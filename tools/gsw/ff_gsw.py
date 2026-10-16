#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""frick-fsw Ground Support Tool (GSW).

Sends PUS-C telecommands and decodes PUS-C telemetry over a serial port (target
UART) or the host loopback log. TC construction and TM parsing are covered by
fixed packet vectors; the live serial path uses a bounded incremental TM decoder
that retains incomplete frames across reads. Fragmentation/resynchronization
tests live in `tests/python/test_ff_gsw.py` (tool register T7). An end-to-end link
with an independent ground segment is not yet demonstrated.
Keep this script under review when changed (norm N10).

Commands:
  ff_gsw.py --connect /dev/ttyUSB0 --baud 115200            open interactive read
  ff_gsw.py --connect /dev/ttyUSB0 --ping --ack accept,start,completion
  ff_gsw.py --file tm.bin --decode                            decode a TM log

Packet layout: CCSDS Space Packet with CRC-16/CCITT-FALSE trailer (see
docs/interfaces/ICD.md).

Copyright (C) 2026 Ilfrick
https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
"""
from __future__ import annotations

import argparse
import struct
import sys
import time

FF_APID_TC = 0x010
FF_APID_TM = 0x020
FF_SERVICE_TEST = 17
MAX_TM_PACKET = 6 + 13 + 1024 + 2
MIN_TM_PDL = 14  # 13-octet PUS TM secondary header plus CRC
SERIAL_READ_POLL_SECONDS = 0.1
PING_REPLY = (FF_SERVICE_TEST, 2, b"\x01\x02\x03\x04")
MAX_PING_TELEMETRY_PACKETS = 64


def crc16(data: bytes, crc: int = 0xFFFF) -> int:
    """CRC-16/CCITT-FALSE compatible with the on-board codec."""
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def build_ping_packet(source_id: int = 0x0102, seq: int = 0) -> bytes:
    """Build a PUS service 17 subservice 1 (connection test) TC frame."""
    ack = 0x01 | 0x02 | 0x08  # acceptance + start + completion
    sec = bytes([((2 << 4) | ack), 0x11, 0x01, (source_id >> 8) & 0xFF, source_id & 0xFF])
    payload = b"\x01\x02\x03\x04"
    raw = sec + payload
    pdl = len(raw) + 2 - 1  # packet-data field includes the CRC; PDL is length minus one
    hdr = struct.pack(">HHH", (1 << 12) | (1 << 11) | FF_APID_TC,
                      0xC000 | (seq & 0x3FFF), pdl)
    frame = hdr + raw
    c = crc16(frame)
    return frame + bytes([(c >> 8) & 0xFF, c & 0xFF])


def decode_tm(stream: bytes) -> list:
    """Decode CCSDS TM frames in the byte stream into (service, subservice, data)."""
    out = []
    pos = 0
    while pos + 7 <= len(stream):
        (word, seq, _pdl) = struct.unpack_from(">HHH", stream, pos)
        apid = word & 0x07FF
        if (((word >> 13) & 0x07) != 0 or ((word >> 12) & 0x01) != 0 or
                ((word >> 11) & 0x01) != 1 or (seq >> 14) != 0x03 or
                apid != FF_APID_TM):
            pos += 1
            continue
        pdl = _pdl
        end = pos + 7 + pdl
        if end > len(stream):
            # The input is a finite log buffer. Do not let a malformed header
            # hide a later valid frame in the same buffer; resynchronize by one
            # octet. A live incremental receiver must retain incomplete bytes
            # separately before calling this finite-buffer decoder.
            pos += 1
            continue
        frame = stream[pos:end]
        if (pdl < 14 or crc16(frame) != 0 or (frame[6] >> 4) != 2):
            # 13-byte PUS-C TM secondary header + 2-byte CRC, version 2.
            pos += 1
            continue
        svc = frame[7]
        sub = frame[8]
        data = frame[19 : len(frame) - 2]
        out.append((svc, sub, data))
        pos = end
    return out


class TmStreamDecoder:
    """Bounded incremental TM decoder retaining incomplete packets between reads."""

    def __init__(self, max_packet: int = MAX_TM_PACKET):
        if max_packet < 21:
            raise ValueError("max_packet must fit a minimum PUS-C TM packet")
        self.max_packet = max_packet
        self._buffer = bytearray()

    @property
    def buffered_bytes(self) -> int:
        """Number of bytes retained while waiting for a complete packet."""
        return len(self._buffer)

    def feed(self, chunk: bytes) -> list:
        """Accept a read fragment and return all complete valid TM packets."""
        packets = []
        offset = 0
        while offset < len(chunk):
            packets.extend(self._drain())
            free = self.max_packet - len(self._buffer)
            if free <= 0:
                # A full-size buffer cannot be a valid incomplete packet. Drop
                # one octet to recover if a malformed candidate made no progress.
                del self._buffer[0]
                free = 1
            count = min(free, len(chunk) - offset)
            self._buffer.extend(chunk[offset : offset + count])
            offset += count
            packets.extend(self._drain())
        return packets

    def _drain(self) -> list:
        packets = []
        while len(self._buffer) >= 6:
            word, seq, pdl = struct.unpack_from(">HHH", self._buffer, 0)
            apid = word & 0x07FF
            valid_header = (
                ((word >> 13) & 0x07) == 0
                and ((word >> 12) & 0x01) == 0
                and ((word >> 11) & 0x01) == 1
                and (seq >> 14) == 0x03
                and apid == FF_APID_TM
            )
            packet_len = 7 + pdl
            if not valid_header or pdl < MIN_TM_PDL or packet_len > self.max_packet:
                del self._buffer[0]
                continue
            if len(self._buffer) < packet_len:
                break
            frame = bytes(self._buffer[:packet_len])
            if crc16(frame) != 0 or (frame[6] >> 4) != 2:
                del self._buffer[0]
                continue
            packets.append((frame[7], frame[8], frame[19:-2]))
            del self._buffer[:packet_len]
        return packets


def read_tm_until_ping_reply(read, timeout: float, clock=time.monotonic) -> list:
    """Read bounded fragments until the expected S17 echo reply or deadline."""
    deadline = clock() + timeout
    decoder = TmStreamDecoder()
    packets = []
    while True:
        remaining = deadline - clock()
        if remaining <= 0:
            break
        fragment = read(1024, min(remaining, SERIAL_READ_POLL_SECONDS))
        if clock() >= deadline:
            break
        for packet in decoder.feed(fragment):
            if len(packets) == MAX_PING_TELEMETRY_PACKETS:
                del packets[0]
            packets.append(packet)
            if packet == PING_REPLY:
                return packets
    return packets


def main() -> int:
    ap = argparse.ArgumentParser(description="frick-fsw ground support tool")
    ap.add_argument("--connect", help="serial device, e.g. /dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--ping", action="store_true", help="send a PUS S17 ping TC")
    ap.add_argument("--file", help="binary TM log to decode")
    ap.add_argument("--decode", action="store_true", help="decode TM packets")
    ap.add_argument("--timeout", type=float, default=3.0)
    args = ap.parse_args()
    if args.timeout <= 0:
        ap.error("--timeout must be positive")

    if args.decode:
        with open(args.file or "-", "rb") as f:
            tm = decode_tm(f.read())
        print(f"decoded {len(tm)} TM packets:")
        for svc, sub, data in tm:
            print(f"  S{svc}.{sub} payload={data.hex()}")
        return 0

    if args.ping:
        frame = build_ping_packet()
        print(f"TC frame ({len(frame)} B): {frame.hex()}")
        if not args.connect:
            print("(no --connect given; frame size/CRC self-check only)")
            if crc16(frame) != 0:
                print("ERROR: built frame fails CRC")
                return 1
            return 0
        try:
            import serial  # pyserial
        except ImportError:
            print("ERROR: pyserial not installed (pip install pyserial)")
            return 1
        with serial.Serial(args.connect, args.baud,
                           timeout=min(args.timeout, SERIAL_READ_POLL_SECONDS)) as ser:
            ser.write(frame)

            def read_fragment(size, poll_timeout):
                ser.timeout = poll_timeout
                return ser.read(size)

            packets = read_tm_until_ping_reply(read_fragment, args.timeout)
        print(f"received {len(packets)} TM packets")
        for svc, sub, data in packets:
            print(f"  S{svc}.{sub} payload={data.hex()}")
        if not any(packet == PING_REPLY for packet in packets):
            print("WARNING: no matching service 17 ping reply received before timeout")
            return 1
        return 0

    ap.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
