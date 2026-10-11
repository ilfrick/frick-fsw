#!/usr/bin/env python3
"""frick-fsw Ground Support Tool (GSW).

Sends PUS-C telecommands and decodes PUS-C telemetry over a serial port (target
UART) or the host loopback log. It is a qualified V&V tool (see
docs/vv/tool-qualification.md, T7): keep this script under review when changed
(norm N10).

Commands:
  ff_gsw.py --connect /dev/ttyUSB0 --baud 115200            open interactive read
  ff_gsw.py --connect /dev/ttyUSB0 --ping --ack accept,start,completion
  ff_gsw.py --file tm.bin --decode                            decode a TM log

Packet layout: CCSDS Space Packet with CRC-16/CCITT-FALSE trailer (see
docs/interfaces/ICD.md).

Copyright (C) 2026 Ilfrick
https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
SPDX-License-Identifier: AGPL-3.0-only
"""
from __future__ import annotations

import argparse
import struct
import sys

FF_APID_TC = 0x010
FF_APID_TM = 0x020
FF_SERVICE_TEST = 17


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
    sec = bytes([0x02, 0x00 | ack, 0x11, 0x01, (source_id >> 8) & 0xFF, source_id & 0xFF])
    payload = b"\x01\x02\x03\x04"
    raw = sec + payload
    pdl = len(raw)
    hdr = struct.pack(">HHH", (0x1 << 13) | (0 << 12) | (1 << 11) | (0x01 << 8) | FF_APID_TC,
                      0x0000 | (seq & 0x3FFF), pdl)
    frame = hdr + raw
    c = crc16(frame)
    return frame + bytes([(c >> 8) & 0xFF, c & 0xFF])


def decode_tm(stream: bytes) -> list:
    """Decode CCSDS TM frames in the byte stream into (service, subservice, data)."""
    out = []
    pos = 0
    while pos + 7 <= len(stream):
        (word, _seq, _pdl) = struct.unpack_from(">HHH", stream, pos)
        apid = word & 0x07FF
        if apid != FF_APID_TM or pos + 6 > len(stream):
            pos += 1
            continue
        pdl = _pdl
        end = pos + 6 + pdl
        if end > len(stream):
            break
        frame = stream[pos:end]
        if crc16(frame) != 0:
            pos += 1
            continue
        svc = frame[6]
        sub = frame[7]
        data = frame[10:6 + pdl - 2] if 6 + pdl - 2 >= 10 else b""
        out.append((svc, sub, data))
        pos = end
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description="frick-fsw ground support tool")
    ap.add_argument("--connect", help="serial device, e.g. /dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--ping", action="store_true", help="send a PUS S17 ping TC")
    ap.add_argument("--file", help="binary TM log to decode")
    ap.add_argument("--decode", action="store_true", help="decode TM packets")
    ap.add_argument("--timeout", type=float, default=3.0)
    args = ap.parse_args()

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
        with serial.Serial(args.connect, args.baud, timeout=args.timeout) as ser:
            ser.write(frame)
            reply = ser.read(1024)
        packets = decode_tm(reply)
        print(f"received {len(packets)} TM packets")
        for svc, sub, data in packets:
            print(f"  S{svc}.{sub} payload={data.hex()}")
        if not any(svc == FF_SERVICE_TEST for svc, _, _ in packets):
            print("WARNING: no service 17 reply received")
            return 1
        return 0

    ap.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
