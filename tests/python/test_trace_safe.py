#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""Fixture-only regression tests for the NC-EXT-174 receipt contract.

Everything here is synthetic.  A valid fixture proves that the *parser and
binding contract* accepts a well-formed receipt; it is explicitly NOT evidence
that Lean elaborated anything.  Real elaboration evidence is produced only by
``tools/scripts/lean_elab_receipt.py`` in an environment with the pinned
toolchain, which these tests never invoke.

Isolation rules honoured by every test:
  * fresh ``tempfile.TemporaryDirectory`` per test, cleaned up by the context
    manager; no canonical repository path is written, read as a fixture, or
    deleted;
  * no ``make``, ``lake``, compiler, container, network or install command;
  * no import-time side effects beyond adding ``tools/scripts`` to ``sys.path``;
  * assertions check exact diagnostic strings produced by the validator, not
    substrings or basenames.

Run with::

    python3 -m unittest discover -s tests/python -p 'test_trace_safe.py' -v
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = REPO_ROOT / "tools" / "scripts"
TRACE = SCRIPTS / "trace.py"
sys.path.insert(0, str(SCRIPTS))

import validator_checkpoint as nc174  # noqa: E402

TOOLCHAIN = "leanprover/lean4:v4.10.0\n"
LAKEFILE = "-- fixture lakefile (synthetic)\npackage fixture\n"
LAKE_MANIFEST = {
    "version": "1.1.0",
    "packagesDir": ".lake/packages",
    "packages": [],
    "name": "fixture",
    "lakeDir": ".lake",
}
LEAN_VERSION = "Lean (version 4.10.0, commit deadbeef, Release)"
GAP_DOC = "docs/quality/class-a-gaps.md"
MANIFEST = """schema_version: "1.0"

requirements:
  - id: GAP-REQ-1
    level: gap
    justification: "fixture-only, no implementation claim"
"""


class Fixture:
    """A minimal synthetic repository tree plus receipt helpers."""

    def __init__(self, tmp: Path) -> None:
        self.root = tmp
        self.lean = tmp / "verification" / "lean"
        self.frick = self.lean / "Frick"
        self.build_lib = self.lean / ".lake" / "build" / "lib"
        self.receipt = self.lean / ".lake" / "build" / "elab_receipt.json"
        self.lean.mkdir(parents=True, exist_ok=True)
        (self.lean / "lean-toolchain").write_text(TOOLCHAIN, encoding="utf-8")
        (self.lean / "lakefile.lean").write_text(LAKEFILE, encoding="utf-8")
        (self.lean / "lake-manifest.json").write_text(
            json.dumps(LAKE_MANIFEST), encoding="utf-8"
        )
        (tmp / GAP_DOC).parent.mkdir(parents=True, exist_ok=True)
        (tmp / GAP_DOC).write_text("GAP-REQ-1\n", encoding="utf-8")
        (tmp / "traceability.yaml").write_text(MANIFEST, encoding="utf-8")
        self.frick.mkdir(parents=True)
        self.build_lib.mkdir(parents=True)

    # -- source / output helpers ------------------------------------------
    def add_module(self, name: str, text: str, olean: bytes | None = None) -> Path:
        source = self.frick / f"{name}.lean"
        source.write_text(text, encoding="utf-8")
        if olean is not None:
            self.write_olean(name, olean)
        return source

    def write_olean(self, name: str, data: bytes) -> Path:
        path = self.build_lib / "Frick" / f"{name}.olean"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    # -- receipt helpers ---------------------------------------------------
    def module_record(self, name: str) -> dict:
        source = self.frick / f"{name}.lean"
        module_name = f"Frick.{name}"
        olean = self.build_lib / "Frick" / f"{name}.olean"
        olean_bytes = olean.read_bytes()
        rel_olean = olean.relative_to(self.root).as_posix()
        assert rel_olean == nc174.canonical_olean_rel(module_name)
        return {
            "source": source.relative_to(self.root).as_posix(),
            "source_sha256": nc174.sha256_file(source),
            "olean": rel_olean,
            "olean_sha256": nc174.sha256_bytes(olean_bytes),
            "olean_size": len(olean_bytes),
            "compiler_cwd": nc174.LEAN_ROOT_REL,
            "compiler_argv": nc174.canonical_argv(module_name),
            "exit_status": 0,
            "status": "success",
            "stdout_sha256": nc174.sha256_bytes(b""),
            "stderr_sha256": nc174.sha256_bytes(b""),
        }

    def build_receipt(self, names: list[str], **root_overrides) -> dict:
        toolchain_sha = nc174.sha256_file(self.lean / "lean-toolchain")
        lakefile_sha = nc174.sha256_file(self.lean / "lakefile.lean")
        lake_manifest_sha = nc174.sha256_file(self.lean / "lake-manifest.json")
        version = root_overrides.get("lean_version", LEAN_VERSION)
        dependencies = root_overrides.get("dependencies", [])
        receipt = {
            "schema_version": nc174.SCHEMA_VERSION,
            "generator": nc174.GENERATOR_ID,
            "generator_version": nc174.GENERATOR_VERSION,
            "generated_at": "2026-10-25T20:00:00Z",
            "lean_root": nc174.LEAN_ROOT_REL,
            "build_lib": nc174.BUILD_LIB_REL,
            "lean_toolchain_sha256": toolchain_sha,
            "lakefile_sha256": lakefile_sha,
            "lake_manifest_sha256": lake_manifest_sha,
            "lean_version": version,
            "dependencies": dependencies,
            "config_sha256": nc174.compute_config_sha256(
                toolchain_sha,
                lakefile_sha,
                lake_manifest_sha,
                version,
                dependencies,
            ),
            "outputs_cleared": True,
            "elaboration_order": [f"Frick.{name}" for name in names],
            "modules": {
                f"Frick.{name}": dict(
                    self.module_record(name), elaboration_index=index
                )
                for index, name in enumerate(names)
            },
        }
        receipt.update(
            {
                key: value
                for key, value in root_overrides.items()
                if key not in ("lean_version", "dependencies")
            }
        )
        return receipt

    def write_receipt(self, receipt: dict | str) -> Path:
        self.receipt.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(receipt, str):
            self.receipt.write_text(receipt, encoding="utf-8")
        else:
            self.receipt.write_text(
                json.dumps(receipt, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
        return self.receipt

    # -- execution helpers -------------------------------------------------
    def validate(self) -> list[str]:
        diagnostics, _ = nc174.validate_receipt(self.root)
        return diagnostics

    def run_trace(self) -> subprocess.CompletedProcess:
        return subprocess.run(
            [sys.executable, str(TRACE), str(self.root / "traceability.yaml")],
            cwd=str(self.root),
            capture_output=True,
            text=True,
            timeout=60,
            env={**os.environ, "PYTHONDONTWRITEBYTECODE": "1"},
        )


class Nc174ReceiptContractTests(unittest.TestCase):
    def setUp(self) -> None:
        self._tmp = tempfile.TemporaryDirectory(prefix="nc174-fixture-")
        self.addCleanup(self._tmp.cleanup)
        self.fx = Fixture(Path(self._tmp.name))
        self.fx.add_module(
            "Alpha",
            "theorem req_alpha : True := by trivial\n",
            b"\x00" * 128,
        )
        self.fx.add_module(
            "NoTheorems",
            "def helper : Nat := 42\n",
            b"\x01" * 64,
        )

    # -- controls ----------------------------------------------------------
    def test_synthetic_parser_control_is_accepted(self) -> None:
        """SYNTHETIC CONTRACT CONTROL — not compilation evidence."""
        self.fx.write_receipt(self.fx.build_receipt(["Alpha", "NoTheorems"]))
        self.assertEqual(self.fx.validate(), [])
        result = self.fx.run_trace()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("NC-EXT-174", result.stdout)
        self.assertIn("traceability: PASS", result.stdout)

    def test_status_only_receipt_is_rejected(self) -> None:
        self.fx.write_receipt(
            {
                "schema_version": nc174.SCHEMA_VERSION,
                "modules": {
                    "Frick.Alpha": {"status": "success"},
                    "Frick.NoTheorems": {"status": "success"},
                },
            }
        )
        diagnostics = self.fx.validate()
        self.assertIn("receipt_field_missing: generator", diagnostics)
        self.assertIn("receipt_field_missing: config_sha256", diagnostics)
        result = self.fx.run_trace()
        self.assertEqual(result.returncode, 1)
        self.assertIn(
            "  E: NC-EXT-174: receipt_field_missing: generator", result.stdout
        )

    # -- receipt presence / shape -----------------------------------------
    def test_missing_receipt(self) -> None:
        self.assertEqual(
            self.fx.validate(),
            ["receipt_missing: verification/lean/.lake/build/elab_receipt.json"],
        )
        result = self.fx.run_trace()
        self.assertEqual(result.returncode, 1)
        self.assertIn(
            "  E: NC-EXT-174: receipt_missing: "
            "verification/lean/.lake/build/elab_receipt.json",
            result.stdout,
        )

    def test_malformed_json(self) -> None:
        self.fx.write_receipt("{not json")
        self.assertEqual(
            self.fx.validate(),
            ["receipt_malformed_json: verification/lean/.lake/build/elab_receipt.json"],
        )

    def test_unknown_schema_version(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["schema_version"] = "nc-ext-174/999"
        self.fx.write_receipt(receipt)
        self.assertIn("receipt_schema_unknown: nc-ext-174/999", self.fx.validate())

    def test_module_missing_from_receipt(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        del receipt["modules"]["Frick.NoTheorems"]
        self.fx.write_receipt(receipt)
        self.assertIn("module_missing_from_receipt: Frick.NoTheorems", self.fx.validate())

    def test_receipt_module_not_in_source(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Stale"] = receipt["modules"]["Frick.Alpha"]
        self.fx.write_receipt(receipt)
        self.assertIn("receipt_module_not_in_source: Frick.Stale", self.fx.validate())

    # -- source binding ----------------------------------------------------
    def test_source_hash_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        old = receipt["modules"]["Frick.Alpha"]["source_sha256"]
        (self.fx.frick / "Alpha.lean").write_text(
            "theorem req_alpha : True := by trivial\n-- changed\n", encoding="utf-8"
        )
        new = nc174.sha256_file(self.fx.frick / "Alpha.lean")
        self.assertIn(
            f"source_hash_mismatch: Frick.Alpha (receipt={old} actual={new})",
            self.fx.validate(),
        )

    def test_olean_hash_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        self.fx.write_olean("Alpha", b"\xff" * 128)  # same size, different bytes
        actual = nc174.sha256_bytes(b"\xff" * 128)
        recorded = receipt["modules"]["Frick.Alpha"]["olean_sha256"]
        self.assertIn(
            f"olean_hash_mismatch: Frick.Alpha (receipt={recorded} actual={actual})",
            self.fx.validate(),
        )

    def test_olean_size_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Alpha"]["olean_size"] = 999
        self.fx.write_receipt(receipt)
        self.assertIn(
            "olean_size_mismatch: Frick.Alpha (receipt=999 actual=128)",
            self.fx.validate(),
        )

    def test_zero_length_olean(self) -> None:
        self.fx.write_olean("Alpha", b"")
        self.fx.write_receipt(self.fx.build_receipt(["Alpha", "NoTheorems"]))
        self.assertIn(
            "olean_empty: Frick.Alpha "
            "(verification/lean/.lake/build/lib/Frick/Alpha.olean)",
            self.fx.validate(),
        )

    def test_non_regular_olean(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        olean = self.fx.build_lib / "Frick" / "Alpha.olean"
        olean.unlink()
        olean.mkdir()
        self.assertIn(
            "olean_not_regular: Frick.Alpha "
            "(verification/lean/.lake/build/lib/Frick/Alpha.olean)",
            self.fx.validate(),
        )

    def test_olean_outside_build_lib(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Alpha"]["olean"] = (
            "verification/lean/.lake/build/lib/../../../../Frick/Alpha.olean"
        )
        self.fx.write_receipt(receipt)
        self.assertTrue(
            any(
                d.startswith("olean_path_not_canonical: Frick.Alpha")
                for d in self.fx.validate()
            )
        )

    # -- compiler provenance ----------------------------------------------
    def test_non_success_exit_status(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Alpha"]["exit_status"] = 1
        self.fx.write_receipt(receipt)
        self.assertIn(
            "module_exit_status_not_success: Frick.Alpha (exit=1)",
            self.fx.validate(),
        )

    def test_status_not_success(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Alpha"]["status"] = "failed"
        self.fx.write_receipt(receipt)
        self.assertIn(
            "module_status_not_success: Frick.Alpha (status=failed)",
            self.fx.validate(),
        )

    def test_missing_compiler_argv(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Alpha"]["compiler_argv"] = "lake env lean"
        self.fx.write_receipt(receipt)
        self.assertIn("compiler_argv_invalid: Frick.Alpha", self.fx.validate())

    def test_empty_compiler_argv(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["modules"]["Frick.Alpha"]["compiler_argv"] = []
        self.fx.write_receipt(receipt)
        self.assertTrue(
            any(
                d.startswith("compiler_argv_not_canonical: Frick.Alpha")
                for d in self.fx.validate()
            )
        )

    def test_substituted_olean_path(self) -> None:
        """A receipt pointing at another module's olean must be refused."""
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        alpha = receipt["modules"]["Frick.Alpha"]
        no_theorems = receipt["modules"]["Frick.NoTheorems"]
        alpha["olean"] = no_theorems["olean"]
        alpha["olean_sha256"] = no_theorems["olean_sha256"]
        alpha["olean_size"] = no_theorems["olean_size"]
        self.fx.write_receipt(receipt)
        diagnostics = self.fx.validate()
        self.assertIn(
            "olean_path_not_canonical: Frick.Alpha "
            "(receipt=verification/lean/.lake/build/lib/Frick/NoTheorems.olean "
            "expected=verification/lean/.lake/build/lib/Frick/Alpha.olean)",
            diagnostics,
        )

    def test_foreign_olean_outside_build_lib(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        foreign = self.fx.root / "Frick" / "Alpha.olean"
        foreign.parent.mkdir(parents=True, exist_ok=True)
        foreign.write_bytes(b"\x00" * 128)
        alpha = receipt["modules"]["Frick.Alpha"]
        alpha["olean"] = foreign.relative_to(self.fx.root).as_posix()
        self.fx.write_receipt(receipt)
        self.assertTrue(
            any(
                d.startswith("olean_path_not_canonical: Frick.Alpha")
                for d in self.fx.validate()
            )
        )

    def test_generator_version_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["generator_version"] = "0.9.9"
        self.fx.write_receipt(receipt)
        self.assertIn(
            "receipt_generator_version_unknown: 0.9.9", self.fx.validate()
        )

    # -- F1: ordering and fresh outputs ------------------------------------
    def test_outputs_not_cleared_is_rejected(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        del receipt["outputs_cleared"]
        self.fx.write_receipt(receipt)
        self.assertIn(
            "receipt_outputs_not_cleared: outputs_cleared=None", self.fx.validate()
        )

    def test_elaboration_order_missing_is_rejected(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        del receipt["elaboration_order"]
        self.fx.write_receipt(receipt)
        self.assertIn(
            "receipt_field_invalid: elaboration_order", self.fx.validate()
        )

    def test_elaboration_order_violates_imports(self) -> None:
        incl = self.fx.add_module(
            "Incl", "import Frick.Alpha\ntheorem x : True := by trivial\n", b"\x02" * 32
        )
        self.assertIsNotNone(incl)
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems", "Incl"])
        receipt["elaboration_order"] = [
            "Frick.Incl",
            "Frick.Alpha",
            "Frick.NoTheorems",
        ]
        self.fx.write_receipt(receipt)
        self.assertIn(
            "elaboration_order_invalid: Frick.Incl elaborated before dependency "
            "Frick.Alpha",
            self.fx.validate(),
        )

    def test_elaboration_order_covers_inventory(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["elaboration_order"] = ["Frick.Alpha"]
        self.fx.write_receipt(receipt)
        self.assertIn(
            "elaboration_order_incomplete: receipt order does not cover the inventory",
            self.fx.validate(),
        )

    def test_import_cycle_is_refused(self) -> None:
        self.fx.add_module("LoopA", "import Frick.LoopB\n", b"\x03" * 32)
        self.fx.add_module("LoopB", "import Frick.LoopA\n", b"\x04" * 32)
        modules, diagnostics = nc174.enumerate_source_modules(self.fx.root)
        self.assertEqual(diagnostics, [])
        order, order_diags = nc174.elaboration_order(modules)
        self.assertEqual(order, [])
        self.assertEqual(order_diags, ["import_cycle: Frick.LoopA, Frick.LoopB"])

    def test_topological_order_places_dependencies_first(self) -> None:
        self.fx.add_module(
            "Zeta", "import Frick.Common\ntheorem z : True := by trivial\n", b"\x05" * 32
        )
        self.fx.add_module(
            "Common", "theorem c : True := by trivial\n", b"\x06" * 32
        )
        modules, diagnostics = nc174.enumerate_source_modules(self.fx.root)
        self.assertEqual(diagnostics, [])
        order, order_diags = nc174.elaboration_order(modules)
        self.assertEqual(order_diags, [])
        self.assertLess(order.index("Frick.Common"), order.index("Frick.Zeta"))

    # -- toolchain / configuration ----------------------------------------
    def test_toolchain_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        recorded = receipt["lean_toolchain_sha256"]
        (self.fx.lean / "lean-toolchain").write_text(
            "leanprover/lean4:v4.11.0\n", encoding="utf-8"
        )
        actual = nc174.sha256_file(self.fx.lean / "lean-toolchain")
        self.assertIn(
            f"toolchain_mismatch: lean_toolchain_sha256 "
            f"(receipt={recorded} actual={actual})",
            self.fx.validate(),
        )

    def test_lean_version_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        receipt["lean_version"] = "Lean (version 4.11.0, commit deadbeef, Release)"
        self.fx.write_receipt(receipt)
        self.assertIn(
            "toolchain_mismatch: lean_version "
            "(receipt=Lean (version 4.11.0, commit deadbeef, Release) "
            "expected=4.10.0)",
            self.fx.validate(),
        )

    def test_config_mismatch_on_lakefile_change(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        (self.fx.lean / "lakefile.lean").write_text(
            LAKEFILE + "-- changed\n", encoding="utf-8"
        )
        diagnostics = self.fx.validate()
        self.assertTrue(any(
            d.startswith("config_mismatch: lakefile_sha256 (") for d in diagnostics
        ), diagnostics)
        self.assertTrue(any(
            d.startswith("config_mismatch: config_sha256 (") for d in diagnostics
        ), diagnostics)

    # -- traversal policy --------------------------------------------------
    def test_symlink_source_is_rejected(self) -> None:
        (self.fx.frick / "Linked.lean").symlink_to(self.fx.frick / "Alpha.lean")
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        self.assertIn(
            "symlink_rejected: verification/lean/Frick/Linked.lean",
            self.fx.validate(),
        )

    def test_symlink_directory_is_rejected_and_not_traversed(self) -> None:
        sub = self.fx.frick / "sub"
        sub.mkdir()
        (sub / "Loop").symlink_to(self.fx.frick, target_is_directory=True)
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        diagnostics = self.fx.validate()
        self.assertIn(
            "symlink_rejected: verification/lean/Frick/sub/Loop", diagnostics
        )
        modules, enum_diags = nc174.enumerate_source_modules(self.fx.root)
        self.assertEqual([m.module for m in modules], ["Frick.Alpha", "Frick.NoTheorems"])
        self.assertEqual(enum_diags, ["symlink_rejected: verification/lean/Frick/sub/Loop"])

    def test_enumeration_visits_each_module_once(self) -> None:
        modules, diagnostics = nc174.enumerate_source_modules(self.fx.root)
        self.assertEqual(diagnostics, [])
        self.assertEqual(
            [m.module for m in modules], ["Frick.Alpha", "Frick.NoTheorems"]
        )

    def test_missing_source_root(self) -> None:
        empty = Path(tempfile.mkdtemp(prefix="nc174-empty-", dir=self._tmp.name))
        self.assertEqual(
            nc174.enumerate_source_modules(empty),
            ([], ["source_root_missing: verification/lean/Frick"]),
        )

    # -- end-to-end trace fail-closed -------------------------------------
    def test_trace_fails_on_olean_hash_mismatch(self) -> None:
        receipt = self.fx.build_receipt(["Alpha", "NoTheorems"])
        self.fx.write_receipt(receipt)
        self.fx.write_olean("Alpha", b"\xfe" * 128)
        actual = nc174.sha256_bytes(b"\xfe" * 128)
        recorded = receipt["modules"]["Frick.Alpha"]["olean_sha256"]
        result = self.fx.run_trace()
        self.assertEqual(result.returncode, 1)
        self.assertIn(
            f"  E: NC-EXT-174: olean_hash_mismatch: Frick.Alpha "
            f"(receipt={recorded} actual={actual})",
            result.stdout,
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
