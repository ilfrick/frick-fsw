#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""Strict consumer for the NC-EXT-174 source-bound elaboration receipt.

This module is the single implementation of the receipt contract shared by

  * ``tools/scripts/lean_elab_receipt.py`` (producer), and
  * ``tools/scripts/trace.py`` (consumer / gate).

It is pure Python: it never invokes Lean, never builds, and never writes.
A receipt is *evidence produced by an actual per-module elaboration run*; this
validator can only check that the receipt is structurally complete and bound to
the current source, toolchain, configuration and dependency state.

Receipt schema ``nc-ext-174/1`` (JSON object, sorted keys, UTF-8)::

    {
      "schema_version": "nc-ext-174/1",
      "generator": "tools/scripts/lean_elab_receipt.py",
      "generator_version": "1.0.0",
      "generated_at": "2026-10-25T20:00:00Z",
      "lean_root": "verification/lean",
      "build_lib": "verification/lean/.lake/build/lib",
      "lean_toolchain_sha256": "<sha256 of verification/lean/lean-toolchain>",
      "lakefile_sha256": "<sha256 of verification/lean/lakefile.lean>",
      "lake_manifest_sha256": "<sha256 of verification/lean/lake-manifest.json>",
      "lean_version": "Lean (version 4.10.0, ...)",
      "dependencies": [
        {"name": "<package>", "path": ".lake/packages/<package>",
         "manifest_present": true, "manifest_sha256": "<sha256>"}
      ],
      "config_sha256": "<sha256 over the toolchain/lakefile/manifest/version/deps>",
      "modules": {
        "Frick.Ccsds": {
          "source": "verification/lean/Frick/Ccsds.lean",
          "source_sha256": "<sha256>",
          "olean": "verification/lean/.lake/build/lib/Frick/Ccsds.olean",
          "olean_sha256": "<sha256>",
          "olean_size": 90712,
          "compiler_cwd": "verification/lean",
          "compiler_argv": ["lake", "env", "lean", "-o", "...", "-i", "...",
                            "Frick/Ccsds.lean"],
          "exit_status": 0,
          "status": "success",
          "stdout_sha256": "<sha256>",
          "stderr_sha256": "<sha256>"
        }
      }
    }

Diagnostics are stable, prefix-free strings; the gate prints them with an
``NC-EXT-174: `` prefix.  CLI::

    python3 tools/scripts/validator_checkpoint.py [--root DIR] [--receipt PATH]

Exit 0 only when the receipt validates against the tree at ``--root``.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import stat
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

SCHEMA_VERSION = "nc-ext-174/2"
GENERATOR_ID = "tools/scripts/lean_elab_receipt.py"
GENERATOR_VERSION = "1.1.0"

LEAN_ROOT_REL = "verification/lean"
SOURCE_REL = "verification/lean/Frick"
BUILD_LIB_REL = "verification/lean/.lake/build/lib"
RECEIPT_REL = "verification/lean/.lake/build/elab_receipt.json"
TOOLCHAIN_REL = "verification/lean/lean-toolchain"
LAKEFILE_REL = "verification/lean/lakefile.lean"
LAKE_MANIFEST_REL = "verification/lean/lake-manifest.json"
SOURCE_NAMESPACE = "Frick"

_GENERATED_AT_RE = re.compile(r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$")
_LEAN_VERSION_RE = re.compile(r"version\s+([0-9][0-9A-Za-z.\-]*)")
_TOOLCHAIN_TAG_RE = re.compile(r":v?([0-9][0-9A-Za-z.\-]*)\s*$")

REQUIRED_ROOT_STR_FIELDS = (
    "schema_version",
    "generator",
    "generator_version",
    "generated_at",
    "lean_root",
    "build_lib",
    "lean_toolchain_sha256",
    "lakefile_sha256",
    "lake_manifest_sha256",
    "lean_version",
    "config_sha256",
)
REQUIRED_MODULE_STR_FIELDS = (
    "source",
    "source_sha256",
    "olean",
    "olean_sha256",
)


@dataclass(frozen=True)
class SourceModule:
    """One discovered Frick/*.lean source module."""

    module: str
    path: Path
    rel: str
    source_sha256: str
    text: str


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def rel_posix(root: Path, path: Path) -> str:
    return path.relative_to(root).as_posix()


def enumerate_source_modules(root: Path) -> tuple[list[SourceModule], list[str]]:
    """Enumerate ``verification/lean/Frick/**/*.lean`` exactly once.

    Policy: all symlinks (directory or file) are rejected with a diagnostic;
    directory cycles are detected by ``(st_dev, st_ino)``; unreadable or
    non-UTF-8 entries produce controlled diagnostics instead of being skipped;
    duplicate module names are rejected.  An empty inventory is an error.
    """
    diags: list[str] = []
    source_root = root / SOURCE_REL
    try:
        root_stat = os.lstat(source_root)
    except FileNotFoundError:
        return [], [f"source_root_missing: {SOURCE_REL}"]
    except OSError as exc:
        return [], [f"source_root_unreadable: {SOURCE_REL} ({exc.strerror or exc})"]

    if stat.S_ISLNK(root_stat.st_mode):
        return [], [f"symlink_rejected: {SOURCE_REL}"]
    if not stat.S_ISDIR(root_stat.st_mode):
        return [], [f"source_root_not_directory: {SOURCE_REL}"]

    def on_walk_error(exc: OSError) -> None:
        name = SOURCE_REL
        if exc.filename:
            try:
                name = rel_posix(root, Path(exc.filename))
            except ValueError:
                name = str(exc.filename)
        diags.append(f"unreadable_directory: {name}")

    modules: list[SourceModule] = []
    seen_modules: dict[str, str] = {}
    visited: set[tuple[int, int]] = set()

    for dirpath, dirnames, filenames in os.walk(
        source_root, topdown=True, followlinks=False, onerror=on_walk_error
    ):
        dpath = Path(dirpath)
        try:
            dst = os.stat(dpath)
            visited.add((dst.st_dev, dst.st_ino))
        except OSError:
            diags.append(f"unreadable_directory: {rel_posix(root, dpath)}")
            dirnames[:] = []
            continue

        kept: list[str] = []
        for dname in sorted(dirnames):
            full = dpath / dname
            rel = rel_posix(root, full)
            if full.is_symlink():
                diags.append(f"symlink_rejected: {rel}")
                continue
            try:
                st = os.lstat(full)
            except OSError:
                diags.append(f"unreadable_directory: {rel}")
                continue
            ident = (st.st_dev, st.st_ino)
            if ident in visited:
                diags.append(f"cycle_detected: {rel}")
                continue
            if not stat.S_ISDIR(st.st_mode):
                diags.append(f"directory_not_directory: {rel}")
                continue
            visited.add(ident)
            kept.append(dname)
        dirnames[:] = kept

        for fname in sorted(filenames):
            fp = dpath / fname
            rel = rel_posix(root, fp)
            if not fname.endswith(".lean"):
                continue
            if fp.is_symlink():
                diags.append(f"symlink_rejected: {rel}")
                continue
            try:
                fst = os.lstat(fp)
            except OSError:
                diags.append(f"unreadable_source: {rel}")
                continue
            if not stat.S_ISREG(fst.st_mode):
                diags.append(f"source_not_regular: {rel}")
                continue
            try:
                data = fp.read_bytes()
            except OSError:
                diags.append(f"unreadable_source: {rel}")
                continue
            try:
                text = data.decode("utf-8")
            except UnicodeDecodeError:
                diags.append(f"source_not_utf8: {rel}")
                continue
            rel_module = fp.relative_to(source_root).with_suffix("").as_posix()
            module = SOURCE_NAMESPACE + "." + rel_module.replace("/", ".")
            if module in seen_modules:
                diags.append(
                    f"duplicate_module: {module} ({seen_modules[module]}, {rel})"
                )
                continue
            seen_modules[module] = rel
            modules.append(
                SourceModule(
                    module=module,
                    path=fp,
                    rel=rel,
                    source_sha256=sha256_bytes(data),
                    text=text,
                )
            )

    modules.sort(key=lambda item: item.module)
    if not diags and not modules:
        diags.append(f"source_inventory_empty: {SOURCE_REL}")
    return modules, diags


_IMPORT_RE = re.compile(r"^\s*(?:public\s+)?import\s+(.+?)\s*$")


def module_imports(text: str) -> list[str]:
    """Return the module names imported by a Lean source buffer, in order."""
    names: list[str] = []
    for line in text.splitlines():
        match = _IMPORT_RE.match(line)
        if not match:
            continue
        for token in match.group(1).split():
            token = token.strip()
            if token and not token.startswith("--"):
                names.append(token)
    return names


def intra_namespace_imports(module: SourceModule) -> list[str]:
    """Imports that resolve inside the analysed namespace (elaboration deps)."""
    prefix = SOURCE_NAMESPACE + "."
    return [name for name in module_imports(module.text) if name.startswith(prefix)]


def elaboration_order(modules: list[SourceModule]) -> tuple[list[str], list[str]]:
    """Deterministic topological order of the modules by intra-namespace imports.

    Fails closed on a missing import target, a self-import or an import cycle:
    an order that cannot be justified is not an order the producer may claim.
    """
    diags: list[str] = []
    by_name = {module.module: module for module in modules}
    deps: dict[str, set[str]] = {name: set() for name in by_name}
    for name in sorted(by_name):
        for dep in intra_namespace_imports(by_name[name]):
            if dep == name:
                diags.append(f"import_self: {name}")
                continue
            if dep not in by_name:
                diags.append(f"import_target_missing: {name} -> {dep}")
                continue
            deps[name].add(dep)
    if diags:
        return [], diags

    order: list[str] = []
    done: set[str] = set()
    pending = sorted(by_name)
    while pending:
        progressed = False
        for name in list(pending):
            if deps[name] <= done:
                order.append(name)
                done.add(name)
                pending.remove(name)
                progressed = True
                break
        if not progressed:
            diags.append("import_cycle: " + ", ".join(pending))
            return [], diags
    return order, []


def module_rel_stem(module_name: str) -> str:
    """Path stem (relative to the Lean root) of a module's compiler output."""
    prefix = SOURCE_NAMESPACE + "."
    if not module_name.startswith(prefix):
        raise ValueError(f"module outside namespace: {module_name}")
    return SOURCE_NAMESPACE + "/" + module_name[len(prefix):].replace(".", "/")


def canonical_olean_rel(module_name: str) -> str:
    return f"{BUILD_LIB_REL}/{module_rel_stem(module_name)}.olean"


def canonical_argv(module_name: str) -> list[str]:
    stem = module_rel_stem(module_name)
    return [
        "lake",
        "env",
        "lean",
        "-o",
        f".lake/build/lib/{stem}.olean",
        "-i",
        f".lake/build/lib/{stem}.ilean",
        f"{stem}.lean",
    ]



def config_payload(
    lean_toolchain_sha256: str,
    lakefile_sha256: str,
    lake_manifest_sha256: str,
    lean_version: str,
    dependencies: list[dict[str, Any]],
) -> dict[str, Any]:
    return {
        "lean_toolchain_sha256": lean_toolchain_sha256,
        "lakefile_sha256": lakefile_sha256,
        "lake_manifest_sha256": lake_manifest_sha256,
        "lean_version": lean_version,
        "dependencies": dependencies,
    }


def compute_config_sha256(
    lean_toolchain_sha256: str,
    lakefile_sha256: str,
    lake_manifest_sha256: str,
    lean_version: str,
    dependencies: list[dict[str, Any]],
) -> str:
    payload = config_payload(
        lean_toolchain_sha256,
        lakefile_sha256,
        lake_manifest_sha256,
        lean_version,
        dependencies,
    )
    canonical = json.dumps(payload, sort_keys=True, separators=(",", ":"))
    return sha256_bytes(canonical.encode("utf-8"))


def package_dependencies(root: Path) -> list[dict[str, Any]]:
    """Bind the Lake package manifests declared in lake-manifest.json."""
    manifest_path = root / LAKE_MANIFEST_REL
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return []
    packages = manifest.get("packages") if isinstance(manifest, dict) else None
    if not isinstance(packages, list):
        return []
    packages_dir = manifest.get("packagesDir", ".lake/packages")
    deps: list[dict[str, Any]] = []
    for pkg in packages:
        name = ""
        if isinstance(pkg, dict):
            name = str(pkg.get("name", ""))
        elif isinstance(pkg, str):
            name = pkg
        if not name:
            continue
        pkg_rel = f"{LEAN_ROOT_REL}/{packages_dir}/{name}"
        manifest_rel = f"{pkg_rel}/lake-manifest.json"
        mpath = root / manifest_rel
        present = mpath.is_file() and not mpath.is_symlink()
        digest = sha256_file(mpath) if present else ""
        deps.append(
            {
                "name": name,
                "path": pkg_rel,
                "manifest_present": present,
                "manifest_sha256": digest,
            }
        )
    deps.sort(key=lambda d: str(d.get("name", "")))
    return deps


def _require_str(
    data: dict[str, Any], key: str, scope: str, diags: list[str]
) -> str | None:
    if key not in data:
        diags.append(f"receipt_field_missing: {scope}{key}")
        return None
    value = data[key]
    if not isinstance(value, str) or not value:
        diags.append(f"receipt_field_invalid: {scope}{key}")
        return None
    return value


def _is_int(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool)


def validate_receipt(
    root: Path,
    receipt_path: Path | None = None,
    modules: list[SourceModule] | None = None,
) -> tuple[list[str], dict[str, Any] | None]:
    """Validate the receipt against ``root``; return (diagnostics, parsed).

    ``modules`` may carry an already-enumerated inventory so a caller performs
    exactly one traversal per run; when omitted the tree is enumerated here.
    """
    root = root.resolve()
    receipt_path = receipt_path or (root / RECEIPT_REL)
    diags: list[str] = []

    try:
        rstat = os.lstat(receipt_path)
    except FileNotFoundError:
        return [f"receipt_missing: {RECEIPT_REL}"], None
    except OSError:
        return [f"receipt_unreadable: {RECEIPT_REL}"], None
    if stat.S_ISLNK(rstat.st_mode):
        return [f"symlink_rejected: {RECEIPT_REL}"], None
    if not stat.S_ISREG(rstat.st_mode):
        return [f"receipt_not_regular: {RECEIPT_REL}"], None
    try:
        raw = receipt_path.read_bytes()
    except OSError:
        return [f"receipt_unreadable: {RECEIPT_REL}"], None
    try:
        data = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError):
        return [f"receipt_malformed_json: {RECEIPT_REL}"], None
    if not isinstance(data, dict):
        return [f"receipt_not_object: {RECEIPT_REL}"], None

    schema = _require_str(data, "schema_version", "", diags)
    if schema is not None and schema != SCHEMA_VERSION:
        diags.append(f"receipt_schema_unknown: {schema}")

    generator = _require_str(data, "generator", "", diags)
    if generator is not None and generator != GENERATOR_ID:
        diags.append(f"receipt_generator_unknown: {generator}")
    generator_version = _require_str(data, "generator_version", "", diags)
    if generator_version is not None and generator_version != GENERATOR_VERSION:
        diags.append(f"receipt_generator_version_unknown: {generator_version}")
    generated_at = _require_str(data, "generated_at", "", diags)
    if generated_at is not None and not _GENERATED_AT_RE.match(generated_at):
        diags.append(f"receipt_field_invalid: generated_at")
    lean_root = _require_str(data, "lean_root", "", diags)
    if lean_root is not None and lean_root != LEAN_ROOT_REL:
        diags.append(f"receipt_field_invalid: lean_root")
    build_lib = _require_str(data, "build_lib", "", diags)
    if build_lib is not None and build_lib != BUILD_LIB_REL:
        diags.append(f"receipt_field_invalid: build_lib")

    # ---- toolchain / configuration / dependency binding -------------------
    receipt_toolchain = _require_str(data, "lean_toolchain_sha256", "", diags)
    receipt_lakefile = _require_str(data, "lakefile_sha256", "", diags)
    receipt_lake_manifest = _require_str(data, "lake_manifest_sha256", "", diags)
    receipt_version = _require_str(data, "lean_version", "", diags)
    receipt_config = _require_str(data, "config_sha256", "", diags)

    config_actual: dict[str, str] = {}
    for rel, key, label in (
        (TOOLCHAIN_REL, "lean_toolchain_sha256", "toolchain_mismatch"),
        (LAKEFILE_REL, "lakefile_sha256", "config_mismatch"),
        (LAKE_MANIFEST_REL, "lake_manifest_sha256", "config_mismatch"),
    ):
        path = root / rel
        if path.is_symlink():
            diags.append(f"symlink_rejected: {rel}")
            continue
        if not path.is_file():
            diags.append(f"config_file_missing: {rel}")
            continue
        actual = sha256_file(path)
        config_actual[key] = actual
        recorded = data.get(key)
        if isinstance(recorded, str) and recorded != actual:
            diags.append(f"{label}: {key} (receipt={recorded} actual={actual})")

    # lean_version must be a Lean release string matching the pinned toolchain.
    toolchain_path = root / TOOLCHAIN_REL
    if receipt_version is not None and toolchain_path.is_file():
        tag_text = toolchain_path.read_text(encoding="utf-8", errors="replace").strip()
        tag_match = _TOOLCHAIN_TAG_RE.search(tag_text)
        version_match = _LEAN_VERSION_RE.search(receipt_version)
        if not version_match or not tag_match or version_match.group(1) != tag_match.group(1):
            expected = tag_match.group(1) if tag_match else "<unparsed toolchain tag>"
            diags.append(
                f"toolchain_mismatch: lean_version (receipt={receipt_version} expected={expected})"
            )

    deps = data.get("dependencies")
    deps_list: list[dict[str, Any]] = []
    if not isinstance(deps, list):
        diags.append("receipt_field_missing: dependencies")
    else:
        deps_list = [d for d in deps if isinstance(d, dict)]
        if len(deps_list) != len(deps):
            diags.append("receipt_field_invalid: dependencies")
        actual_deps = package_dependencies(root)
        if deps_list != actual_deps:
            diags.append(
                "config_mismatch: dependencies"
                f" (receipt={json.dumps(deps_list, sort_keys=True)}"
                f" actual={json.dumps(actual_deps, sort_keys=True)})"
            )

    if (
        receipt_config is not None
        and len(config_actual) == 3
        and isinstance(data.get("lean_version"), str)
    ):
        computed = compute_config_sha256(
            config_actual["lean_toolchain_sha256"],
            config_actual["lakefile_sha256"],
            config_actual["lake_manifest_sha256"],
            data["lean_version"],
            deps_list,
        )
        if receipt_config != computed:
            diags.append(
                f"config_mismatch: config_sha256 (receipt={receipt_config} actual={computed})"
            )

    # ---- module inventory --------------------------------------------------
    if modules is None:
        modules, source_diags = enumerate_source_modules(root)
        diags.extend(source_diags)
    source_by_name = {m.module: m for m in modules}

    receipt_modules = data.get("modules")
    if not isinstance(receipt_modules, dict):
        diags.append("receipt_field_missing: modules")
        return _dedupe(diags), data

    for name in sorted(source_by_name):
        if name not in receipt_modules:
            diags.append(f"module_missing_from_receipt: {name}")
    for name in sorted(receipt_modules):
        if name not in source_by_name:
            diags.append(f"receipt_module_not_in_source: {name}")

    # ---- elaboration ordering -------------------------------------------
    outputs_cleared = data.get("outputs_cleared")
    if outputs_cleared is not True:
        diags.append(f"receipt_outputs_not_cleared: outputs_cleared={outputs_cleared}")
    recorded_order = data.get("elaboration_order")
    expected_order, order_diags = elaboration_order(modules)
    diags.extend(order_diags)
    if not isinstance(recorded_order, list) or not all(
        isinstance(item, str) for item in recorded_order
    ):
        diags.append("receipt_field_invalid: elaboration_order")
    elif not order_diags:
        if len(recorded_order) != len(expected_order) or set(recorded_order) != set(
            expected_order
        ):
            diags.append("elaboration_order_incomplete: receipt order does not cover the inventory")
        else:
            position = {name: index for index, name in enumerate(recorded_order)}
            for name in recorded_order:
                for dep in intra_namespace_imports(source_by_name[name]):
                    if dep in position and position[dep] >= position[name]:
                        diags.append(
                            f"elaboration_order_invalid: {name} elaborated before dependency {dep}"
                        )
                entry = receipt_modules.get(name)
                if isinstance(entry, dict):
                    index = entry.get("elaboration_index")
                    if not _is_int(index) or index != position[name]:
                        diags.append(
                            f"elaboration_index_inconsistent: {name}"
                            f" (recorded={index} order={position[name]})"
                        )

    build_lib_resolved = (root / BUILD_LIB_REL).resolve()
    for name in sorted(set(source_by_name) & set(receipt_modules)):
        info = receipt_modules[name]
        scope = f"modules.{name}."
        if not isinstance(info, dict):
            diags.append(f"receipt_module_not_object: {name}")
            continue
        source = source_by_name[name]
        try:
            source_lean_rel = source.path.resolve().relative_to(
                (root / LEAN_ROOT_REL).resolve()
            ).as_posix()
        except ValueError:
            source_lean_rel = source.rel

        status = info.get("status")
        if status != "success":
            diags.append(f"module_status_not_success: {name} (status={status})")
        exit_status = info.get("exit_status")
        if not _is_int(exit_status) or exit_status != 0:
            diags.append(
                f"module_exit_status_not_success: {name} (exit={exit_status})"
            )

        argv = info.get("compiler_argv")
        expected_argv = canonical_argv(name)
        if not isinstance(argv, list) or not all(
            isinstance(item, str) and item for item in argv
        ):
            diags.append(f"compiler_argv_invalid: {name}")
        elif argv != expected_argv:
            diags.append(
                f"compiler_argv_not_canonical: {name} (expected={' '.join(expected_argv)})"
            )

        for key in REQUIRED_MODULE_STR_FIELDS:
            _require_str(info, key, scope, diags)

        recorded_cwd = info.get("compiler_cwd")
        if recorded_cwd != LEAN_ROOT_REL:
            diags.append(
                f"compiler_cwd_not_canonical: {name} (recorded={recorded_cwd} expected={LEAN_ROOT_REL})"
            )
        recorded_source = info.get("source")
        if isinstance(recorded_source, str) and recorded_source != source.rel:
            diags.append(
                f"source_path_mismatch: {name}"
                f" (receipt={recorded_source} actual={source.rel})"
            )
        recorded_source_sha = info.get("source_sha256")
        if (
            isinstance(recorded_source_sha, str)
            and recorded_source_sha != source.source_sha256
        ):
            diags.append(
                f"source_hash_mismatch: {name}"
                f" (receipt={recorded_source_sha} actual={source.source_sha256})"
            )

        olean_rel = info.get("olean")
        if not isinstance(olean_rel, str) or not olean_rel:
            continue
        expected_olean_rel = canonical_olean_rel(name)
        if olean_rel != expected_olean_rel:
            diags.append(
                f"olean_path_not_canonical: {name}"
                f" (receipt={olean_rel} expected={expected_olean_rel})"
            )
            continue
        olean_path = root / olean_rel
        try:
            resolved = olean_path.resolve(strict=False)
            contained = resolved.is_relative_to(build_lib_resolved)
        except (OSError, ValueError):
            diags.append(f"olean_outside_build: {name} ({olean_rel})")
            continue
        if not contained:
            diags.append(f"olean_outside_build: {name} ({olean_rel})")
            continue
        if olean_path.is_symlink():
            diags.append(f"olean_not_regular: {name} ({olean_rel})")
            continue
        try:
            ost = os.lstat(olean_path)
        except FileNotFoundError:
            diags.append(f"olean_missing: {name} ({olean_rel})")
            continue
        except OSError:
            diags.append(f"olean_unreadable: {name} ({olean_rel})")
            continue
        if not stat.S_ISREG(ost.st_mode):
            diags.append(f"olean_not_regular: {name} ({olean_rel})")
            continue
        try:
            olean_bytes = olean_path.read_bytes()
        except OSError:
            diags.append(f"olean_unreadable: {name} ({olean_rel})")
            continue
        if len(olean_bytes) == 0:
            diags.append(f"olean_empty: {name} ({olean_rel})")
            continue
        recorded_size = info.get("olean_size")
        if not _is_int(recorded_size) or recorded_size != len(olean_bytes):
            diags.append(
                f"olean_size_mismatch: {name}"
                f" (receipt={recorded_size} actual={len(olean_bytes)})"
            )
        recorded_hash = info.get("olean_sha256")
        actual_hash = sha256_bytes(olean_bytes)
        if isinstance(recorded_hash, str) and recorded_hash != actual_hash:
            diags.append(
                f"olean_hash_mismatch: {name}"
                f" (receipt={recorded_hash} actual={actual_hash})"
            )

    return _dedupe(diags), data


def _dedupe(items: list[str]) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for item in items:
        if item not in seen:
            seen.add(item)
            out.append(item)
    return out


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Validate the NC-EXT-174 source-bound elaboration receipt."
    )
    parser.add_argument(
        "--root",
        default=None,
        help="repository root (default: current working directory)",
    )
    parser.add_argument(
        "--receipt",
        default=None,
        help=f"receipt path (default: <root>/{RECEIPT_REL})",
    )
    args = parser.parse_args(argv)

    root = Path(args.root).resolve() if args.root else Path.cwd().resolve()
    receipt = Path(args.receipt).resolve() if args.receipt else None
    diags, _ = validate_receipt(root, receipt)
    if diags:
        print("NC-EXT-174 elaboration receipt: FAIL")
        for diag in diags:
            print(f"  NC-EXT-174: {diag}")
        print(f"nc174-receipt: FAIL ({len(diags)} diagnostics)")
        return 1
    print(f"NC-EXT-174 elaboration receipt: OK ({RECEIPT_REL})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
