#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""Golden-vector tests for the ground-support CCSDS/PUS packet codec."""
# Copyright (C) 2026 Ilfrick

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.gsw.ff_gsw import (  # noqa: E402
    PING_REPLY,
    TmStreamDecoder,
    build_ping_packet,
    crc16,
    decode_tm,
    parse_ack_stages,
    ping_responses_valid,
    read_tm_until_ping_reply,
)


def build_test_tm(service, subservice, payload=b"", seq_count=0):
    sec = bytes([0x20, service, subservice]) + bytes(10)
    pdl = len(sec) + len(payload) + 2 - 1
    header = bytes.fromhex("0820") + (0xC000 | (seq_count & 0x3FFF)).to_bytes(2, "big") + pdl.to_bytes(2, "big")
    frame = header + sec + payload
    checksum = crc16(frame)
    return frame + bytes([checksum >> 8, checksum & 0xFF])


class GroundToolPacketTests(unittest.TestCase):
    def test_ping_packet_matches_flight_wire_layout(self):
        expected = bytes.fromhex("1810c000000a2b1101010201020304f72b")
        packet = build_ping_packet()
        self.assertEqual(packet, expected)
        self.assertEqual(crc16(packet), 0)

    def test_ping_ack_stage_selection_sets_exact_tc_flags(self):
        flags, expected = parse_ack_stages("accept,start,completion")
        self.assertEqual((flags, expected), (0x0B, (1, 3, 7)))
        self.assertEqual(parse_ack_stages("progress"), (0x04, (5,)))
        self.assertEqual(parse_ack_stages(""), (0, ()))
        self.assertEqual(parse_ack_stages("none"), (0, ()))
        self.assertEqual(build_ping_packet(ack_flags=flags), build_ping_packet())
        self.assertEqual(build_ping_packet(ack_flags=0x04)[6] & 0x0F, 0x04)
        with self.assertRaises(ValueError):
            parse_ack_stages("accept,accept")
        with self.assertRaises(ValueError):
            parse_ack_stages("unknown")

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

    def test_incremental_decoder_retains_split_packet(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        decoder = TmStreamDecoder()
        self.assertEqual(decoder.feed(packet[:9]), [])
        self.assertEqual(decoder.buffered_bytes, 9)
        self.assertEqual(decoder.feed(packet[9:]), [(3, 25, b"abc")])
        self.assertEqual(decoder.buffered_bytes, 0)

    def test_incremental_decoder_handles_coalesced_packets_and_timeout(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        decoder = TmStreamDecoder()
        self.assertEqual(decoder.feed(packet[:7]), [])
        self.assertEqual(decoder.feed(b""), [])  # serial timeout; retain partial frame
        self.assertEqual(decoder.buffered_bytes, 7)
        self.assertEqual(
            decoder.feed(packet[7:] + packet), [(3, 25, b"abc"), (3, 25, b"abc")]
        )
        self.assertEqual(decoder.buffered_bytes, 0)

    def test_incremental_decoder_buffer_is_bounded_and_resynchronizes(self):
        packet = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        decoder = TmStreamDecoder()
        self.assertEqual(decoder.feed(b"\x00" * 4096 + packet), [(3, 25, b"abc")])
        self.assertLessEqual(decoder.buffered_bytes, decoder.max_packet)

    def test_ping_reader_ignores_unrelated_tm_and_waits_for_matching_reply(self):
        unrelated = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        reply = bytes.fromhex("0820c000001220110200010102000000050006010203043e97")
        self.assertEqual(decode_tm(reply), [PING_REPLY])

        fragments = iter([unrelated[:8], unrelated[8:] + reply[:11], reply[11:]])
        ticks = iter([0.0, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06])
        packets = read_tm_until_ping_reply(lambda _size, _timeout: next(fragments), 1.0,
                                           clock=lambda: next(ticks))
        self.assertEqual(packets, [(3, 25, b"abc"), PING_REPLY])

    def test_ping_reader_waits_for_requested_ack_after_ping_reply(self):
        reply = bytes.fromhex("0820c000001220110200010102000000050006010203043e97")
        completion = build_test_tm(1, 7, bytes.fromhex("010200010001110100"), seq_count=1)
        fragments = iter([reply, completion])
        packets = read_tm_until_ping_reply(
            lambda _size, _timeout: next(fragments), 1.0,
            expected_ack_subservices=(7,))
        self.assertEqual([packet[:2] for packet in packets], [(17, 2), (1, 7)])
        self.assertTrue(ping_responses_valid(packets, (7,)))
        self.assertFalse(ping_responses_valid(packets + [(1, 7, b"duplicate")], (7,)))

    def test_ping_reader_times_out_on_partial_reply(self):
        reply = bytes.fromhex("0820c000001220110200010102000000050006010203043e97")
        fragments = iter([reply[:8], b"", b"", b""])
        ticks = iter([0.0, 0.25, 0.5, 0.75, 1.0])
        packets = read_tm_until_ping_reply(lambda _size, _timeout: next(fragments), 1.0,
                                           clock=lambda: next(ticks))
        self.assertEqual(packets, [])

    def test_ping_reader_bounds_capture_and_deadlines_plausible_incomplete_candidate(self):
        unrelated = bytes.fromhex("0820c0000011200319000101020000000500066162631aed")
        reply = bytes.fromhex("0820c000001220110200010102000000050006010203043e97")
        fragments = iter([unrelated * 70 + reply])
        ticks = iter([0.0, 0.01, 0.02])
        packets = read_tm_until_ping_reply(lambda _size, _timeout: next(fragments), 0.2,
                                           clock=lambda: next(ticks))
        self.assertLessEqual(len(packets), 64)
        self.assertIn(PING_REPLY, packets)

        # A syntactically plausible, maximum-length header can contain later
        # bytes as payload; the stream decoder must not guess an inner boundary.
        candidate = bytes.fromhex("0820c000040e")
        fragments = iter([candidate + reply, b"", b""])
        ticks = iter([0.0, 0.1, 0.2])
        packets = read_tm_until_ping_reply(lambda _size, _timeout: next(fragments), 0.2,
                                           clock=lambda: next(ticks))
        self.assertEqual(packets, [])

    def test_ping_reader_rejects_reply_returned_after_deadline(self):
        reply = bytes.fromhex("0820c000001220110200010102000000050006010203043e97")
        now = [0.0]

        def late_read(_size, poll_timeout):
            self.assertLessEqual(poll_timeout, 0.1)
            now[0] = 1.01
            return reply

        self.assertEqual(read_tm_until_ping_reply(late_read, 1.0, clock=lambda: now[0]), [])


if __name__ == "__main__":
    unittest.main()
