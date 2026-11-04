#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Tests for atomic_publish helper and XW-3 summary staging (NC-EXT-226)."""

import json, os, sys, tempfile, unittest, ast, re
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "tools" / "scripts"


class AtomicPublishTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.run_dir = Path(self.tmp.name)

    def _import(self):
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import atomic_publish
        return atomic_publish

    def test_atomic_publish_replaces_file(self):
        final = self.run_dir / "summary.json"
        self._import()(final, {"result": "PASS"})
        self.assertEqual(json.loads(final.read_text()), {"result": "PASS"})

    def test_atomic_publish_on_replace_failure_preserves_existing_and_cleans_temp(self):
        final = self.run_dir / "summary.json"
        final.write_text(json.dumps({"result": "FAIL", "reasons": ["gate"]}))
        with patch("os.replace", side_effect=OSError()):
            with self.assertRaises(OSError): self._import()(final, {"result": "PASS"})
        self.assertEqual(json.loads(final.read_text()), {"result": "FAIL", "reasons": ["gate"]})
        self.assertEqual(list(self.run_dir.glob("summary.json.tmp*")), [])

    def test_cmd_summary_writes_to_staging_not_final(self):
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import cmd_summary
        import argparse
        m = self.run_dir / "manifest.json"
        m.write_text(json.dumps({"cases": [{"case": "XW3-A1", "mode": "accept", "expect": "PASS", "required": True}]}))
        d = self.run_dir / "cases" / "XW3-A1"
        d.mkdir(parents=True)
        (d / "verdict.json").write_text(json.dumps({"result": "PASS", "mode": "accept", "capture_sha256": "a"*64, "capture_bytes": 100, "identity_reasons": [], "console_fatal": {"ok": True, "hits": []}, "oracle": {"ok": True, "checks": [], "counts": {"S17.2":1,"S1.1":1,"S1.3":1,"S1.7":1}, "reasons": [], "request": {}, "observations": {}}}))
        (d / "run-meta.json").write_text(json.dumps({"run_nonce": "n", "elf_sha256": "b"*64, "capture_sha256": "a"*64, "capture_bytes": 100}))
        (d / "capture.bin").write_bytes(b"x"*100)
        s = self.run_dir / "summary.case.json"
        rc = cmd_summary(argparse.Namespace(run_dir=str(self.run_dir), manifest=str(m), run_index=None, json=str(s)))
        self.assertEqual(rc, 0)
        self.assertTrue(s.exists())
        self.assertEqual(json.loads(s.read_text())["result"], "PASS")

    def test_pygate_staging_to_final_via_atomic_publish(self):
        atomic_publish = self._import()
        s = self.run_dir / "summary.case.json"
        s.write_text(json.dumps({"result": "PASS", "reasons": []}))
        f = self.run_dir / "summary.json"
        atomic_publish(f, {"result": "PASS", "run_nonce": "n", "elf_sha256": "e"*64, "cases": {}})
        self.assertEqual(json.loads(f.read_text())["run_nonce"], "n")

    def test_pygate_atomic_failure_leaves_staging_and_final_unchanged(self):
        atomic_publish = self._import()
        s = self.run_dir / "summary.case.json"
        s.write_text(json.dumps({"result": "PASS", "reasons": []}))
        f = self.run_dir / "summary.json"
        f.write_text(json.dumps({"result": "FAIL", "reasons": ["gate"]}))
        with patch("os.replace", side_effect=OSError()):
            try: atomic_publish(f, {"result": "PASS", "run_nonce": "n", "elf_sha256": "e"*64, "cases": {}})
            except OSError: pass
        self.assertEqual(json.loads(s.read_text()), {"result": "PASS", "reasons": []})
        self.assertEqual(json.loads(f.read_text()), {"result": "FAIL", "reasons": ["gate"]})

    def test_apply_gate_result_preserves_fail_when_gate_reason_empty(self):
        """FAIL staging summary must remain FAIL even when gate_reason is empty (NC-EXT-226)."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import apply_gate_result

        # FAIL due to missing staging file
        summary = {"result": "FAIL", "reasons": ["staging_unreadable:FileNotFoundError"]}
        result = apply_gate_result(summary, "")
        self.assertEqual(result["result"], "FAIL", "FAIL staging must not become PASS when gate_reason empty")
        self.assertIn("staging_unreadable:FileNotFoundError", result["reasons"])

        # FAIL due to malformed staging JSON
        summary = {"result": "FAIL", "reasons": ["staging_malformed"]}
        result = apply_gate_result(summary, "")
        self.assertEqual(result["result"], "FAIL", "FAIL staging must not become PASS when gate_reason empty")
        self.assertIn("staging_malformed", result["reasons"])

    def test_apply_gate_result_preserves_pass_when_gate_reason_empty(self):
        """PASS staging summary remains PASS when gate_reason is empty."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import apply_gate_result

        summary = {"result": "PASS", "reasons": []}
        result = apply_gate_result(summary, "")
        self.assertEqual(result["result"], "PASS")
        self.assertEqual(result["reasons"], [])

    def test_apply_gate_result_fails_when_gate_reason_provided(self):
        """Any staging becomes FAIL when gate_reason is provided."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import apply_gate_result

        summary = {"result": "PASS", "reasons": []}
        result = apply_gate_result(summary, "runner_gate_failed")
        self.assertEqual(result["result"], "FAIL")
        self.assertIn("runner_gate_failed", result["reasons"])

    def test_apply_gate_result_fail_with_empty_reasons_adds_default(self):
        """FAIL staging with empty reasons gets 'case_verdict_failed' when gate_reason empty."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import apply_gate_result

        summary = {"result": "FAIL", "reasons": []}
        result = apply_gate_result(summary, "")
        self.assertEqual(result["result"], "FAIL")
        self.assertIn("case_verdict_failed", result["reasons"])

    def test_apply_gate_result_fail_preserves_existing_reason(self):
        """FAIL staging with existing reason preserves it when gate_reason empty."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import apply_gate_result

        summary = {"result": "FAIL", "reasons": ["existing_reason"]}
        result = apply_gate_result(summary, "")
        self.assertEqual(result["result"], "FAIL")
        self.assertIn("existing_reason", result["reasons"])
        self.assertNotIn("case_verdict_failed", result["reasons"])

    # --- read_staged_summary contract tests ---
    def test_read_staged_summary_valid_object(self):
        """read_staged_summary returns parsed dict for valid JSON object."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import read_staged_summary

        p = self.run_dir / "staging.json"
        p.write_text(json.dumps({"result": "PASS", "reasons": []}))
        result = read_staged_summary(p)
        self.assertEqual(result, {"result": "PASS", "reasons": []})

    def test_read_staged_summary_missing_file(self):
        """read_staged_summary returns FAIL dict for missing file."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import read_staged_summary

        p = self.run_dir / "nonexistent.json"
        result = read_staged_summary(p)
        self.assertEqual(result["result"], "FAIL")
        self.assertTrue(any(r.startswith("staging_unreadable:") for r in result["reasons"]))

    def test_read_staged_summary_malformed_json(self):
        """read_staged_summary returns FAIL dict for malformed JSON."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import read_staged_summary

        p = self.run_dir / "staging.json"
        p.write_text("{not valid json")
        result = read_staged_summary(p)
        self.assertEqual(result["result"], "FAIL")
        self.assertIn("staging_malformed", result["reasons"])

    def test_read_staged_summary_valid_non_object(self):
        """read_staged_summary returns FAIL dict for valid non-object JSON (array, string, etc.)."""
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import read_staged_summary

        p = self.run_dir / "staging.json"
        p.write_text("[]")
        result = read_staged_summary(p)
        self.assertEqual(result["result"], "FAIL")
        self.assertIn("staging_not_an_object", result["reasons"])

    def test_read_staged_summary_pass_with_reasons_rejected(self):
        """read_staged_summary must reject PASS when reasons is nonempty or absent
        (NC-EXT-226). The producer cmd_summary emits PASS only with reasons=[].

        Both cases must return FAIL with reason staging_inconsistent_pass.
        """
        sys.path.insert(0, str(TOOLS))
        from xw3_verdict import read_staged_summary

        test_cases = [
            # (description, staging_json, expected_result, expected_reason_substring)
            (
                "PASS with nonempty reasons",
                {"result": "PASS", "reasons": ["case_verdict_failed"]},
                "FAIL",
                "staging_inconsistent_pass",
            ),
            (
                "PASS with reasons field absent",
                {"result": "PASS"},
                "FAIL",
                "staging_inconsistent_pass",
            ),
        ]

        for desc, staging_json, exp_result, exp_reason in test_cases:
            with self.subTest(desc):
                p = self.run_dir / "staging.json"
                p.write_text(json.dumps(staging_json))
                result = read_staged_summary(p)
                self.assertEqual(result["result"], exp_result,
                                 f"{desc}: expected result {exp_result}, got {result['result']}")
                self.assertIn(exp_reason, result["reasons"],
                              f"{desc}: expected reason '{exp_reason}' in {result['reasons']}")

    def test_both_pygate_blocks_use_helpers(self):
        """Verify both PYGATE blocks in qemu_xw3_vv.sh import and use read_staged_summary, apply_gate_result, atomic_publish."""
        script_path = TOOLS / "qemu_xw3_vv.sh"
        content = script_path.read_text(encoding="utf-8")

        # Extract PYGATE heredoc blocks using regex
        pygate_blocks = re.findall(r"<<'PYGATE'\n(.*?)\nPYGATE", content, re.DOTALL)
        self.assertEqual(len(pygate_blocks), 2, f"Expected 2 PYGATE blocks, found {len(pygate_blocks)}")

        for i, block in enumerate(pygate_blocks):
            # Parse the block as Python to verify imports and calls
            tree = ast.parse(block)

            # Check imports
            imports = []
            for node in ast.walk(tree):
                if isinstance(node, ast.ImportFrom) and node.module == "xw3_verdict":
                    for alias in node.names:
                        imports.append(alias.name)

            self.assertIn("read_staged_summary", imports, f"Block {i+1} missing read_staged_summary import")
            self.assertIn("apply_gate_result", imports, f"Block {i+1} missing apply_gate_result import")
            self.assertIn("atomic_publish", imports, f"Block {i+1} missing atomic_publish import")

            # Check calls
            calls = []
            for node in ast.walk(tree):
                if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
                    calls.append(node.func.id)

            self.assertIn("read_staged_summary", calls, f"Block {i+1} missing read_staged_summary call")
            self.assertIn("apply_gate_result", calls, f"Block {i+1} missing apply_gate_result call")
            self.assertIn("atomic_publish", calls, f"Block {i+1} missing atomic_publish call")


if __name__ == "__main__":
    unittest.main(verbosity=2)