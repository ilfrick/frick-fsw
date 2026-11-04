#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Fixture tests for tools/scripts/uart_tm_verify.py (T2 §3).

The telemetry frames here are **independently specified golden bytes**, built
from the documented layout by this file's own encoder — not captured from the
target. They exercise the verifier's decision only: they are not target evidence
and not compiler/decoder qualification.

Discriminating controls required by the brief: an empty capture and a
wrong-payload frame must both be rejected.
"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
VERIFIER = ROOT / "tools" / "scripts" / "uart_tm_verify.py"
TM_APID = 0x020


def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def tm_frame(service: int, subservice: int, counter: int, payload: bytes) -> bytes:
    """Build a TM frame from the documented layout (independent of the C codec)."""
    pdl = 13 + len(payload) + 2 - 1  # CCSDS: data field octets - 1
    w0 = (0 << 13) | (0 << 12) | (1 << 11) | TM_APID  # version 0, TM, sec hdr, APID
    frame = bytearray()
    frame += w0.to_bytes(2, "big")
    frame += ((3 << 14) | 0).to_bytes(2, "big")  # seq flags 3, ccsds counter 0
    frame += pdl.to_bytes(2, "big")
    frame += bytes([(2 << 4) | 0])              # PUS version 2
    frame += bytes([service, subservice])
    frame += counter.to_bytes(2, "big")
    frame += (0x01).to_bytes(2, "big")          # destination id
    frame += (1000).to_bytes(4, "big")          # CUC coarse
    frame += (0).to_bytes(2, "big")             # CUC fine
    frame += payload
    frame += crc16(bytes(frame)).to_bytes(2, "big")
    return bytes(frame)


def u1_stream(base_counter: int = 7, s17_payload: bytes = b"\x01\x02\x03\x04",
              include: tuple = ((1, 1), (1, 3), (17, 2), (1, 7)),
              counter_step: int = 1) -> bytes:
    out = bytearray(b"\r\n+--- banner noise ---+\r\n")
    counter = base_counter
    for service, subservice in include:
        payload = s17_payload if (service, subservice) == (17, 2) else b"\x01\x02\x01\x01\x00"
        out += tm_frame(service, subservice, counter, payload)
        counter += counter_step
    return bytes(out)


class UartTmVerifyTest(unittest.TestCase):
    def run_verifier(self, capture: bytes):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "capture.bin"
            path.write_bytes(capture)
            proc = subprocess.run(
                [sys.executable, str(VERIFIER), "--capture", str(path),
                 "--json", str(Path(tmp) / "out.json")],
                capture_output=True, text=True, timeout=60,
            )
            return proc

    def test_complete_u1_set_passes(self):
        proc = self.run_verifier(u1_stream())
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)
        self.assertIn("uart-tm: PASS", proc.stdout)

    def test_empty_capture_is_rejected(self):
        proc = self.run_verifier(b"")
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("no_tm_received", proc.stdout)
        self.assertIn("uart-tm: FAIL", proc.stdout)

    def test_wrong_payload_is_rejected(self):
        proc = self.run_verifier(u1_stream(s17_payload=b"\x01\x02\x03\x05"))
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("S17.2_payload_mismatch", proc.stdout)

    def test_missing_case_is_rejected(self):
        proc = self.run_verifier(u1_stream(include=((1, 1), (1, 3), (17, 2))))
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("count_S1.7=0", proc.stdout)

    def test_duplicate_case_is_rejected(self):
        proc = self.run_verifier(u1_stream(include=((1, 1), (1, 1), (1, 3), (17, 2), (1, 7))))
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("count_S1.1=2", proc.stdout)

    def test_failed_stage_is_rejected(self):
        proc = self.run_verifier(u1_stream(include=((1, 1), (1, 3), (17, 2), (1, 7), (1, 8))))
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("unexpected_S1.8", proc.stdout)

    def test_counter_break_is_rejected(self):
        """Counters that skip a value must be rejected (frames are CRC-valid)."""
        proc = self.run_verifier(u1_stream(counter_step=2))
        self.assertEqual(proc.returncode, 1, msg=proc.stdout)
        self.assertIn("counter_not_incremental_at_9", proc.stdout)

    def test_ansi_noise_is_tolerated(self):
        noisy = b"\x1b[0m\x1b[2J" + u1_stream() + b"\x1b[0m"
        proc = self.run_verifier(noisy)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
