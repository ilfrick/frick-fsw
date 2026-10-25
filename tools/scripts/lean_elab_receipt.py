#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""NC-EXT-174 source-bound Lean elaboration receipt generator.

For every ``verification/lean/Frick/**/*.lean`` module (enumerated exactly
once by :mod:`validator_checkpoint`), this tool performs an *explicit*
per-module elaboration with the pinned Lake/Lean toolchain -- including
modules with no theorems and modules outside the default build graph -- and
only then writes a receipt binding

  * the module name and source SHA-256,
  * the produced ``.olean`` path, SHA-256 and size,
  * the exact compiler argv, working directory and exit status,
  * the Lean toolchain / lakefile / lake-manifest SHA-256 values,
  * the Lean version string and declared Lake package manifests,
  * a combined ``config_sha256``, generation timestamp and generator version.

The receipt is written atomically to
``verification/lean/.lake/build/elab_receipt.json`` and is *only* written when
every discovered module elaborated successfully (exit status 0) and produced a
non-empty regular-file ``.olean``.  Any failure, skip, empty module set,
symlink, unreadable entry, duplicate module or non-regular/empty output
aborts the run with a diagnostic and leaves the previous receipt untouched.

This script does not run the default ``lake build`` graph; a default-graph
build can silently omit orphan modules (NC-EXT-120/NC-EXT-174).

Usage::

    python3 tools/scripts/lean_elab_receipt.py [--root DIR] [--receipt PATH]

Exit 0 = receipt written; 1 = refused (no receipt written).

Receipt schema ``nc-ext-174/1`` is documented in
``tools/scripts/validator_checkpoint.py`` and in ``docs/vv/tool-qualification.md``.
"""
from __future__ import annotations

import argparse
import json
import os
import stat
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import validator_checkpoint as nc174  # noqa: E402  (path established above)

PRODUCER_VERSION = nc174.GENERATOR_VERSION
COMPILER_LAUNCHER = ("lake", "env", "lean")


class Refused(RuntimeError):
    """A fail-closed condition that must prevent writing the receipt."""


def _sha256_file_checked(root: Path, rel: str) -> str:
    path = root / rel
    if path.is_symlink():
        raise Refused(f"symlink_rejected: {rel}")
    try:
        info = os.lstat(path)
    except FileNotFoundError:
        raise Refused(f"config_file_missing: {rel}") from None
    except OSError as exc:
        raise Refused(f"config_file_unreadable: {rel} ({exc.strerror or exc})") from None
    if not stat.S_ISREG(info.st_mode):
        raise Refused(f"config_file_not_regular: {rel}")
    try:
        return nc174.sha256_file(path)
    except OSError as exc:
        raise Refused(f"config_file_unreadable: {rel} ({exc.strerror or exc})") from None


def _lean_version(root: Path) -> tuple[str, dict[str, Any]]:
    """Run ``lake env lean --version`` and return (version string, argv record)."""
    argv = list(COMPILER_LAUNCHER) + ["--version"]
    try:
        proc = subprocess.run(
            argv,
            cwd=str(root / nc174.LEAN_ROOT_REL),
            capture_output=True,
            text=True,
            check=False,
            timeout=300,
        )
    except (OSError, subprocess.SubprocessError) as exc:
        raise Refused(f"compiler_unavailable: {exc}") from None
    if proc.returncode != 0:
        raise Refused(
            f"compiler_version_failed: exit={proc.returncode}"
            f" argv={' '.join(argv)}"
        )
    first_line = (proc.stdout or proc.stderr).strip().splitlines()
    if not first_line:
        raise Refused("compiler_version_empty: lake env lean --version")
    version = first_line[0].strip()
    if not nc174._LEAN_VERSION_RE.search(version):
        raise Refused(f"compiler_version_unparsed: {version}")
    return version, {
        "compiler_argv": argv,
        "compiler_cwd": nc174.LEAN_ROOT_REL,
        "exit_status": proc.returncode,
        "stdout_sha256": nc174.sha256_bytes((proc.stdout or "").encode("utf-8")),
        "stderr_sha256": nc174.sha256_bytes((proc.stderr or "").encode("utf-8")),
    }


def build_receipt(root: Path) -> dict[str, Any]:
    root = root.resolve()
    lean_root = root / nc174.LEAN_ROOT_REL
    if not lean_root.is_dir() or lean_root.is_symlink():
        raise Refused(f"lean_root_unavailable: {nc174.LEAN_ROOT_REL}")

    toolchain_sha = _sha256_file_checked(root, nc174.TOOLCHAIN_REL)
    lakefile_sha = _sha256_file_checked(root, nc174.LAKEFILE_REL)
    lake_manifest_sha = _sha256_file_checked(root, nc174.LAKE_MANIFEST_REL)

    modules, diags = nc174.enumerate_source_modules(root)
    if diags:
        raise Refused("; ".join(diags))
    if not modules:
        raise Refused(f"source_inventory_empty: {nc174.SOURCE_REL}")

    lean_version, version_record = _lean_version(root)

    build_lib = root / nc174.BUILD_LIB_REL
    if build_lib.is_symlink() or not build_lib.is_dir():
        raise Refused(f"build_lib_unavailable: {nc174.BUILD_LIB_REL}")

    order, order_diags = nc174.elaboration_order(modules)
    if order_diags:
        raise Refused("; ".join(order_diags))
    by_name = {module.module: module for module in modules}

    # NC-EXT-174 F1: every enumerated output is removed before elaboration, and
    # modules are elaborated in dependency order, so no module can be compiled
    # against an olean left behind by an earlier run.
    cleared: list[str] = []
    for module_name in order:
        stem = nc174.module_rel_stem(module_name)
        for suffix in (".olean", ".ilean"):
            stale = root / nc174.BUILD_LIB_REL / f"{stem}{suffix}"
            if stale.is_symlink():
                raise Refused(f"olean_clear_failed: {module_name} ({stale.name} is a symlink)")
            try:
                stale.unlink()
            except FileNotFoundError:
                pass
            except OSError as exc:
                raise Refused(
                    f"olean_clear_failed: {module_name} ({exc.strerror or exc})"
                ) from None
            if stale.exists():
                raise Refused(f"olean_clear_failed: {module_name} ({stale.name} still present)")
        cleared.append(module_name)

    records: dict[str, Any] = {}
    for index, module_name in enumerate(order):
        module = by_name[module_name]
        lean_rel = module.path.relative_to(lean_root).as_posix()
        olean_rel_in_build = Path(lean_rel).with_suffix(".olean").as_posix()
        ilean_rel_in_build = Path(lean_rel).with_suffix(".ilean").as_posix()
        olean_rel = f"{nc174.BUILD_LIB_REL}/{olean_rel_in_build}"
        olean_path = root / olean_rel
        try:
            olean_path.parent.mkdir(parents=True, exist_ok=True)
        except OSError as exc:
            raise Refused(
                f"olean_output_dir_unwritable: {olean_rel} ({exc.strerror or exc})"
            ) from None

        argv = list(COMPILER_LAUNCHER) + [
            "-o",
            f".lake/build/lib/{olean_rel_in_build}",
            "-i",
            f".lake/build/lib/{ilean_rel_in_build}",
            lean_rel,
        ]
        try:
            proc = subprocess.run(
                argv,
                cwd=str(lean_root),
                capture_output=True,
                text=True,
                check=False,
                timeout=1800,
            )
        except (OSError, subprocess.SubprocessError) as exc:
            raise Refused(
                f"module_compile_error: {module.module} ({exc})"
            ) from None
        if proc.returncode != 0:
            detail = (proc.stderr or proc.stdout or "").strip().splitlines()
            tail = detail[-1] if detail else ""
            raise Refused(
                f"module_compile_failed: {module.module}"
                f" exit={proc.returncode} argv={' '.join(argv)}"
                + (f" stderr={tail}" if tail else "")
            )

        if olean_path.is_symlink():
            raise Refused(f"olean_not_regular: {module.module} ({olean_rel})")
        try:
            ost = os.lstat(olean_path)
        except FileNotFoundError:
            raise Refused(f"olean_missing: {module.module} ({olean_rel})") from None
        except OSError as exc:
            raise Refused(
                f"olean_unreadable: {module.module} ({olean_rel})"
                f" ({exc.strerror or exc})"
            ) from None
        if not stat.S_ISREG(ost.st_mode):
            raise Refused(f"olean_not_regular: {module.module} ({olean_rel})")
        try:
            olean_bytes = olean_path.read_bytes()
        except OSError as exc:
            raise Refused(
                f"olean_unreadable: {module.module} ({olean_rel})"
                f" ({exc.strerror or exc})"
            ) from None
        if not olean_bytes:
            raise Refused(f"olean_empty: {module.module} ({olean_rel})")

        records[module.module] = {
            "source": module.rel,
            "source_sha256": module.source_sha256,
            "olean": olean_rel,
            "olean_sha256": nc174.sha256_bytes(olean_bytes),
            "olean_size": len(olean_bytes),
            "compiler_cwd": nc174.LEAN_ROOT_REL,
            "compiler_argv": argv,
            "exit_status": proc.returncode,
            "status": "success",
            "elaboration_index": index,
            "stdout_sha256": nc174.sha256_bytes((proc.stdout or "").encode("utf-8")),
            "stderr_sha256": nc174.sha256_bytes((proc.stderr or "").encode("utf-8")),
        }

    dependencies = nc174.package_dependencies(root)
    for dep in dependencies:
        if dep.get("manifest_present") is not True:
            raise Refused(
                f"dependency_manifest_missing: {dep.get('name')} ({dep.get('path')})"
            )

    return {
        "schema_version": nc174.SCHEMA_VERSION,
        "generator": nc174.GENERATOR_ID,
        "generator_version": PRODUCER_VERSION,
        "generated_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "lean_root": nc174.LEAN_ROOT_REL,
        "build_lib": nc174.BUILD_LIB_REL,
        "lean_toolchain_sha256": toolchain_sha,
        "lakefile_sha256": lakefile_sha,
        "lake_manifest_sha256": lake_manifest_sha,
        "lean_version": lean_version,
        "dependencies": dependencies,
        "config_sha256": nc174.compute_config_sha256(
            toolchain_sha, lakefile_sha, lake_manifest_sha, lean_version, dependencies
        ),
        "compiler_version_probe": version_record,
        "outputs_cleared": True,
        "elaboration_order": order,
        "modules": records,
    }


def _write_and_publish(
    root: Path, receipt_path: Path, payload: bytes
) -> list[str]:
    """Write a temp receipt, self-validate it, then atomically publish."""
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(
        mode="wb",
        prefix=f".{receipt_path.name}.",
        suffix=".tmp",
        dir=str(receipt_path.parent),
        delete=False,
    )
    tmp_path = Path(handle.name)
    try:
        with handle:
            handle.write(payload)
            handle.flush()
            os.fsync(handle.fileno())
        diags, _ = nc174.validate_receipt(root, tmp_path)
        if diags:
            return diags
        os.replace(tmp_path, receipt_path)
        return []
    finally:
        if tmp_path.exists():
            try:
                tmp_path.unlink()
            except OSError:
                pass


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Generate the NC-EXT-174 source-bound elaboration receipt."
    )
    parser.add_argument(
        "--root", default=None, help="repository root (default: inferred from script)"
    )
    parser.add_argument("--receipt", default=None, help="receipt output path")
    args = parser.parse_args(argv)

    root = (
        Path(args.root).resolve()
        if args.root
        else Path(__file__).resolve().parents[2]
    )
    receipt_path = (
        Path(args.receipt).resolve() if args.receipt else root / nc174.RECEIPT_REL
    )

    try:
        receipt = build_receipt(root)
        payload = (
            json.dumps(receipt, indent=2, sort_keys=True) + "\n"
        ).encode("utf-8")
        # Self-check before publishing: the consumer contract must accept the
        # receipt we are about to write.  Nothing is published on failure, so a
        # previous receipt is never clobbered by a rejected run.
        diags = _write_and_publish(root, receipt_path, payload)
        if diags:
            raise Refused("receipt_self_check_failed: " + "; ".join(diags))
    except Refused as exc:
        print(f"NC-EXT-174 receipt REFUSED: {exc}", file=sys.stderr)
        print("elab-receipt: FAIL (no receipt written)", file=sys.stderr)
        return 1

    print(f"receipt: {receipt_path}")
    print(f"modules elaborated: {len(receipt['modules'])}")
    print(f"receipt sha256: {nc174.sha256_bytes(payload)}")
    print("elab-receipt: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
