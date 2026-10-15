#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""Golden-vector tests for the ground-support CCSDS/PUS packet codec."""
# Copyright (C) 2026 Ilfrick

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.gsw.ff_gsw import build_ping_packet, crc16, decode_tm  # noqa: E402


class GroundToolPacketTests(unittest.TestCase):
    def test_ping_packet_matches_flight_wire_layout(self):
        expected = bytes.fromhex("1810c000000a2b1101010201020304f72b")
        packet = build_ping_packet()
        self.assertEqual(packet, expected)
        self.assertEqual(crc16(packet), 0)

    def test_decode_tm_matches_golden_pus_c_packet(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        self.assertEqual(decode_tm(packet), [(3, 25, b"abc")])

    def test_decode_tm_rejects_incomplete_and_bad_crc(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        self.assertEqual(decode_tm(packet[:-1]), [])
        damaged = packet[:-1] + bytes([packet[-1] ^ 0x01])
        self.assertEqual(decode_tm(damaged), [])

    def test_decode_tm_handles_noise_prefix_and_concatenated_packets(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        self.assertEqual(decode_tm(b"\x00" + packet), [(3, 25, b"abc")])
        self.assertEqual(decode_tm(packet + packet), [(3, 25, b"abc"), (3, 25, b"abc")])

    def test_incomplete_candidate_does_not_hide_later_valid_packet(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        malformed = bytes.fromhex("0820c000ffff")
        self.assertEqual(decode_tm(malformed + packet), [(3, 25, b"abc")])


if __name__ == "__main__":
    unittest.main()
