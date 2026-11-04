#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Fixture tests for tools/scripts/target_req_verify.py (T1, norms N10/N22).

Every test runs the verifier as a subprocess against a temporary fixture log and
manifest, and asserts the exact exit code plus the exact failure-reason token.
The fixture log is synthetic: it exercises the *verifier parser/decision*, not
target behaviour. Synthetic evidence must never be presented as target evidence.

Also contains the independent golden-frame cross-check required by the T1
brief: the GSW golden TC must decode here as CCSDS TC, APID 0x010, PUS-C
service 17 subservice 1, length 4, with a valid CRC-16/CCITT-FALSE trailer.
This is a Python re-implementation of the framing/CRC rules, deliberately not
the C codec under test.
"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
VERIFIER = ROOT / "tools" / "scripts" / "target_req_verify.py"

# GSW golden TC (tests/python/test_ff_gsw.py, tools/gsw/ff_gsw.py).
GOLDEN_TC = bytes.fromhex("1810C000000A2B1101010201020304F72B")


def good_log(cases):
    return "".join(f"TGTREQ: {rid}:{case}:PASS:ok\n" for rid, case in cases)


DEFAULT_CASES = [
    ("SRS-REQ-012", "L1"),
    ("SRS-REQ-009", "L9"),
]


class VerifierTest(unittest.TestCase):
    def run_verifier(self, log_text, manifest):
        with tempfile.TemporaryDirectory() as tmp:
            tmpdir = Path(tmp)
            log_path = tmpdir / "console.log"
            man_path = tmpdir / "manifest.json"
            log_path.write_text(log_text, encoding="utf-8")
            man_path.write_text(json.dumps(manifest), encoding="utf-8")
            proc = subprocess.run(
                [sys.executable, str(VERIFIER), "--log", str(log_path),
                 "--manifest", str(man_path), "--json", str(tmpdir / "out.json")],
                capture_output=True, text=True, timeout=60,
            )
            return proc

    def assert_reason(self, proc, token, expected_exit=1):
        self.assertEqual(proc.returncode, expected_exit,
                         msg=f"stdout={proc.stdout}\nstderr={proc.stderr}")
        self.assertIn(token, proc.stdout,
                      msg=f"expected reason token {token!r} in:\n{proc.stdout}")

    # ---- positive controls -------------------------------------------
    def test_positive_exact_set(self):
        manifest = {"cases": [{"requirement": r, "case": c} for r, c in DEFAULT_CASES]}
        proc = self.run_verifier(good_log(DEFAULT_CASES), manifest)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)
        self.assertIn("COVERAGE: 2/2", proc.stdout)
        self.assertIn("target-evidence: PASS", proc.stdout)

    def test_declared_gap_with_gap_line_is_not_coverage(self):
        manifest = {
            "cases": [{"requirement": r, "case": c} for r, c in DEFAULT_CASES],
            "gaps": [{"requirement": "SRS-REQ-004", "case": "L4-S18",
                      "reason": "NC-EXT-04"}],
        }
        log_text = good_log(DEFAULT_CASES) + "TGTREQ-GAP: SRS-REQ-004:L4-S18:NC-EXT-04\n"
        proc = self.run_verifier(log_text, manifest)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)
        self.assertIn("COVERAGE: 2/2", proc.stdout)

    def test_observation_lines_never_count(self):
        manifest = {"cases": [{"requirement": r, "case": c} for r, c in DEFAULT_CASES]}
        log_text = good_log(DEFAULT_CASES) + "TGTREQ-OBS: SRS-REQ-005:L2:code=9\n"
        proc = self.run_verifier(log_text, manifest)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)

    def test_frick_prefix_normalization(self):
        manifest = {"cases": [{"requirement": "FRICK-SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier("TGTREQ: SRS-REQ-012:L1:PASS\n", manifest)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)

    # ---- rejections --------------------------------------------------
    def test_missing_case(self):
        manifest = {"cases": [{"requirement": r, "case": c} for r, c in DEFAULT_CASES]}
        log_text = "TGTREQ: SRS-REQ-012:L1:PASS:ok\n"
        proc = self.run_verifier(log_text, manifest)
        self.assert_reason(proc, "missing_case:SRS-REQ-009:L9")
        self.assertIn("COVERAGE: 1/2", proc.stdout)

    def test_duplicate_case_line(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        log_text = "TGTREQ: SRS-REQ-012:L1:PASS:ok\nTGTREQ: SRS-REQ-012:L1:PASS:ok\n"
        proc = self.run_verifier(log_text, manifest)
        self.assert_reason(proc, "duplicate_case_line:SRS-REQ-012:L1")

    def test_fail_case(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier("TGTREQ: SRS-REQ-012:L1:FAIL:crc\n", manifest)
        self.assert_reason(proc, "fail_case:SRS-REQ-012:L1:crc")

    def test_unapproved_skip(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier("TGTREQ: SRS-REQ-012:L1:SKIP:not-run\n", manifest)
        self.assert_reason(proc, "skip_case:SRS-REQ-012:L1:not-run")

    def test_gap_case_reported_as_covered(self):
        manifest = {
            "cases": [{"requirement": "SRS-REQ-012", "case": "L1"}],
            "gaps": [{"requirement": "SRS-REQ-004", "case": "L4-S18",
                      "reason": "NC-EXT-04"}],
        }
        log_text = good_log([("SRS-REQ-012", "L1")]) + "TGTREQ: SRS-REQ-004:L4-S18:PASS\n"
        proc = self.run_verifier(log_text, manifest)
        self.assert_reason(proc, "gap_case_reported_as_covered:SRS-REQ-004:L4-S18")

    def test_unexpected_extra_pass(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        log_text = "TGTREQ: SRS-REQ-012:L1:PASS\nTGTREQ: SRS-REQ-013:L2:PASS\n"
        proc = self.run_verifier(log_text, manifest)
        self.assert_reason(proc, "unexpected_case:SRS-REQ-013:L2")

    def test_malformed_line(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier("TGTREQ: SRS-REQ-012:L1:PA\n", manifest)
        self.assert_reason(proc, "malformed_line:")
        self.assertIn("missing_case:SRS-REQ-012:L1", proc.stdout)

    def test_empty_log(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier("", manifest)
        self.assert_reason(proc, "empty_log")

    def test_manifest_case_in_both_sets(self):
        manifest = {
            "cases": [{"requirement": "SRS-REQ-012", "case": "L1"}],
            "gaps": [{"requirement": "SRS-REQ-012", "case": "L1", "reason": "x"}],
        }
        proc = self.run_verifier(good_log([("SRS-REQ-012", "L1")]), manifest)
        self.assert_reason(proc, "manifest_case_in_both_sets:SRS-REQ-012:L1")

    def test_manifest_case_duplicate(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"},
                              {"requirement": "SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier(good_log([("SRS-REQ-012", "L1")]), manifest)
        self.assert_reason(proc, "manifest_case_duplicate:SRS-REQ-012:L1")

    def test_aggregate_marker_only(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        proc = self.run_verifier("RENODE_TEST: PASS\n", manifest)
        self.assert_reason(proc, "aggregate_marker_only")

    def test_undeclared_gap_line(self):
        manifest = {"cases": [{"requirement": "SRS-REQ-012", "case": "L1"}]}
        log_text = good_log([("SRS-REQ-012", "L1")]) + "TGTREQ-GAP: SRS-REQ-004:L4:why\n"
        proc = self.run_verifier(log_text, manifest)
        self.assert_reason(proc, "undeclared_gap_case:SRS-REQ-004:L4")

    def test_usage_error_when_manifest_missing(self):
        with tempfile.TemporaryDirectory() as tmp:
            proc = subprocess.run(
                [sys.executable, str(VERIFIER), "--log", str(Path(tmp) / "nolog"),
                 "--manifest", str(Path(tmp) / "noman")],
                capture_output=True, text=True, timeout=60,
            )
            self.assertEqual(proc.returncode, 2, msg=proc.stderr)


def crc16_ccitt_false(data: bytes) -> int:
    """CRC-16/CCITT-FALSE: init 0xFFFF, poly 0x1021, no reflection, xorout 0."""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


class GoldenFrameTest(unittest.TestCase):
    """Independent cross-check of the L-series framing assumptions (T1 §4)."""

    def test_golden_tc_decodes_svc17_sub1_with_valid_crc(self):
        frame = GOLDEN_TC
        # CRC appended such that the remainder over the whole frame is zero.
        self.assertEqual(crc16_ccitt_false(frame), 0)

        w0 = (frame[0] << 8) | frame[1]
        self.assertEqual((w0 >> 12) & 0x1, 1, "packet must be a telecommand")
        self.assertEqual(w0 & 0x07FF, 0x010, "APID 0x010")
        seq = (frame[2] << 8) | frame[3]
        self.assertEqual(seq >> 14, 3, "sequence flags = unsegmented")
        # CCSDS 133.0-B-2: the packet data length field holds (octets - 1).
        pdl = (frame[4] << 8) | frame[5]
        self.assertEqual(6 + pdl + 1, len(frame), "CCSDS data length field")
        self.assertEqual(pdl + 1, 5 + 4 + 2, "secondary header + payload + CRC")

        sec = frame[6:11]
        self.assertEqual(sec[0] >> 4, 2, "PUS-C version 2")
        self.assertEqual(sec[1], 17, "service 17")
        self.assertEqual(sec[2], 1, "subservice 1")
        self.assertEqual((sec[3] << 8) | sec[4], 0x0102, "source id 0x0102")
        payload = frame[11:-2]
        self.assertEqual(payload, b"\x01\x02\x03\x04", "ping payload")


if __name__ == "__main__":
    unittest.main(verbosity=2)
