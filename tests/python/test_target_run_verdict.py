#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Fixture tests for tools/scripts/target_run_verdict.py (T1 F1/R8).

The point of these tests is the separation that the first T1 delivery got wrong:
a console in which every case passed **and** the image then panicked must be a
FAILED whole run. The fatal block used below is copied verbatim from the
retained console `build/t1-run-20261026T052802Z/console.log`; no marker is
invented. Synthetic evidence exercises the classifier only and is never target
evidence.
"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
VERDICT = ROOT / "tools" / "scripts" / "target_run_verdict.py"
MANIFEST = ROOT / "tools" / "scripts" / "target_req_manifest.json"

# Case lines as emitted by src/apps/app_req_test.c (nine required cases, three
# declared gaps after the F2 correction).
GOOD_CASES = [
    "TGTREQ: SRS-REQ-012:L1:PASS:per_frame_offsets_ok_S17.2_exact_negctl_3/3",
    "TGTREQ-OBS: SRS-REQ-005:L2:S1.8_payload=01022A0109 (implementation-defined layout; NC-EXT-14/15)",
    "TGTREQ: SRS-REQ-005:L2:PASS:one_S1.8_by_header_no_S17.2",
    "TGTREQ: SRS-REQ-005:L3:PASS:one_S1.8_by_header_no_S17.2",
    "TGTREQ: SRS-REQ-004:L4:PASS:parse_error_no_handler_no_S17.2",
    "TGTREQ: SRS-REQ-005:L5:PASS:one_S1.2_by_header_no_handler",
    "TGTREQ: SRS-REQ-003:L6:PASS:1024_echoed_1025_refused",
    "TGTREQ: SRS-REQ-010:L7:PASS:CUC_set_value_at_coarse_resolution",
    "TGTREQ: SRS-REQ-009:L8:PASS:dump_exact_statuses_distinct",
    "TGTREQ: SRS-REQ-005:L10:PASS:chain_not_wedged",
    "TGTREQ-GAP: SRS-REQ-004:L4-S18:NC-EXT-04: S1.8 for a malformed TC is not asserted",
    "TGTREQ-GAP: SRS-REQ-005:L8-WIRE:NC-EXT-18: on-wire refusal failure code is interface-authority owned",
    "TGTREQ-GAP: SRS-REQ-008:L9:NC-EXT-17: 6-byte TC[3,1] create is an unapproved extension (ICD defines 2 octets)",
]

# Verbatim from the retained console (build/t1-run-20261026T052802Z/console.log).
FATAL_BLOCK = """watchdog: NOT WIRED on RTEMS (timeout 8000 ms) - NC-PAL-01
[FF-PANIC] watchdog: init failed

*** EXIT STATUS NOT ZERO ***
exit source: 5 (RTEMS_FATAL_SOURCE_EXIT)
exit code: 1 (0x00000001)
"""


def good_log() -> str:
    return "frick-fsw: 8 applications registered\n" + "\n".join(GOOD_CASES) + "\n"


class VerdictTest(unittest.TestCase):
    def run_verdict(self, console_text, manifest=MANIFEST):
        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            console = tmpdir / "console.log"
            console.write_text(console_text, encoding="utf-8")
            proc = subprocess.run(
                [sys.executable, str(VERDICT), "--console", str(console),
                 "--manifest", str(manifest), "--json", str(tmpdir / "v.json"),
                 "--nonce", "deadbeef"],
                capture_output=True, text=True, timeout=60,
            )
            payload = None
            out = tmpdir / "v.json"
            if out.exists():
                payload = json.loads(out.read_text(encoding="utf-8"))
            return proc, payload

    def test_good_log_without_fatal_passes(self):
        proc, payload = self.run_verdict(good_log())
        self.assertEqual(proc.returncode, 0, msg=proc.stdout + proc.stderr)
        self.assertIn("COMPONENT: PASS 9/9", proc.stdout)
        self.assertIn("WHOLE-RUN: PASS", proc.stdout)
        self.assertEqual(payload["result"], "PASS")
        self.assertEqual(payload["run_nonce"], "deadbeef")

    def test_same_log_plus_retained_fatal_block_fails(self):
        """The F1 regression: identical PASS lines, then the real fatal block."""
        proc, payload = self.run_verdict(good_log() + FATAL_BLOCK)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("COMPONENT: PASS 9/9", proc.stdout)
        self.assertIn("WHOLE-RUN: FAIL (watchdog fatal)", proc.stdout)
        self.assertIn("NOT a target PASS", proc.stdout)
        self.assertEqual(payload["result"], "FAIL")
        self.assertFalse(payload["whole_run"]["ok"])
        self.assertTrue(payload["component"]["ok"])

    def test_fatal_before_any_case_fails(self):
        proc, payload = self.run_verdict(FATAL_BLOCK + good_log())
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("WHOLE-RUN: FAIL", proc.stdout)
        # Component lines are still present, so the component result is PASS;
        # the whole-run result must still be FAIL.
        self.assertEqual(payload["result"], "FAIL")

    def test_general_rtems_fatal_markers_fail(self):
        """R2-01: the non-EXIT RTEMS fatal formats must be recognised."""
        for block in ("*** FATAL ***\nfatal source: 2 (RTEMS_FATAL_SOURCE_ASSERT)\n",
                      "fatal source: 6 (RTEMS_FATAL_SOURCE_STACK_CHECKER)\n"):
            proc, payload = self.run_verdict(good_log() + block)
            self.assertEqual(proc.returncode, 1, msg=proc.stdout)
            self.assertEqual(payload["result"], "FAIL")
            self.assertFalse(payload["whole_run"]["ok"])

    def test_aggregate_marker_only_fails(self):
        proc, _ = self.run_verdict("RENODE_TEST: PASS\n")
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("COMPONENT: FAIL 0/9", proc.stdout)
        self.assertIn("missing_case", proc.stdout)

    def test_component_failure_without_fatal_fails(self):
        partial = good_log().replace(
            "TGTREQ: SRS-REQ-005:L3:PASS:one_S1.8_by_header_no_S17.2\n", "")
        proc, payload = self.run_verdict(partial)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("missing_case:SRS-REQ-005:L3", proc.stdout)
        self.assertTrue(payload["whole_run"]["ok"])
        self.assertFalse(payload["component"]["ok"])

    def test_usage_error_when_console_missing(self):
        with tempfile.TemporaryDirectory() as tmp:
            proc = subprocess.run(
                [sys.executable, str(VERDICT), "--console", str(Path(tmp) / "nope"),
                 "--manifest", str(MANIFEST)],
                capture_output=True, text=True, timeout=60,
            )
            self.assertEqual(proc.returncode, 2, msg=proc.stderr)


class ManifestShapeTest(unittest.TestCase):
    """The required set is nine cases after the F2 correction, with L9 a gap."""

    def test_manifest_requires_nine_cases_and_l9_is_a_gap(self):
        data = json.loads(MANIFEST.read_text(encoding="utf-8"))
        self.assertEqual(sorted(c["case"] for c in data["cases"]),
                         ["L1", "L10", "L2", "L3", "L4", "L5", "L6", "L7", "L8"])
        gaps = {g["case"]: g for g in data["gaps"]}
        self.assertIn("L9", gaps)
        self.assertIn("NC-EXT-17", gaps["L9"]["reason"])
        self.assertNotIn("L9", [c["case"] for c in data["cases"]])


if __name__ == "__main__":
    unittest.main(verbosity=2)
