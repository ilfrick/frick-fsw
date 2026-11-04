#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""Verify telemetry captured from the external UART path (T2 §3).

The capture is the raw byte stream QEMU wrote to its serial console while the
golden test telecommand was delivered on the same serial interface. This tool
is deliberately independent of the flight C codec: it scans for CCSDS TM
packets, checks the packet data length field and the CRC-16/CCITT-FALSE trailer
with its own implementation, reads the PUS TM header at fixed offsets, and
requires the exact U1 set:

  exactly one S1.1, one S1.3, one S1.7 and one S17.2,
  S17.2 payload exactly ``01 02 03 04``,
  APID ``0x020`` and a valid CRC on every accepted packet,
  PUS counters strictly increasing in capture order (mod 16384),
  and no S1.2 / S1.4 / S1.8 (a failed request would not be the U1 set).
Event telemetry (S5, service 5) is reported as observational: a real boot always
emits application-start events and the pre-reset event, so it is not an
"extra" failure and it is not part of the U1 set.

Exit codes: 0 = U1 set verified; 1 = verification failure (including an empty or
TM-free capture); 2 = usage error. It never exits 0 when nothing was received.

Copyright (C) 2026 Ilfrick
SPDX-License-Identifier: MPL-2.0
"""
from __future__ import annotations

import argparse
import json
import re
import sys

TM_APID = 0x020
CCSDS_HDR = 6
PUS_TM_SEC = 13
S17_2_PAYLOAD = bytes([0x01, 0x02, 0x03, 0x04])
EXPECTED = {(1, 1): 1, (1, 3): 1, (1, 7): 1, (17, 2): 1}
FORBIDDEN = {(1, 2), (1, 4), (1, 8)}
# Event telemetry (service 5) is always present on a real boot (application-start
# events, the pre-reset event). It is reported but is not part of the U1 set and
# is not an "extra" failure; any other service is.
OBSERVATIONAL_SERVICES = {5}
ANSI_RE = re.compile(rb"\x1b\[[0-9;?]*[A-Za-z]")


def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def strip_ansi(data: bytes) -> bytes:
    return ANSI_RE.sub(b"", data)


def scan_packets(stream: bytes) -> list:
    """Find CRC-valid TM packets in an arbitrary byte stream."""
    packets: list = []
    index = 0
    limit = len(stream) - (CCSDS_HDR + PUS_TM_SEC + 2)
    while index <= limit:
        window = stream[index:index + 2]
        if window == bytes([0x08, TM_APID & 0xFF]):
            w0 = (stream[index] << 8) | stream[index + 1]
            apid = w0 & 0x07FF
            sec_flag = (w0 >> 11) & 0x1
            ptype = (w0 >> 12) & 0x1
            version = (w0 >> 13) & 0x7
            pdl = (stream[index + 4] << 8) | stream[index + 5]
            total = CCSDS_HDR + pdl + 1
            if (apid == TM_APID and sec_flag == 1 and ptype == 0 and version == 0
                    and index + total <= len(stream) and total > CCSDS_HDR + 2):
                frame = stream[index:index + total]
                if crc16_ccitt_false(frame) == 0:
                    packets.append(frame)
                    index += total
                    continue
        index += 1
    return packets


def decode(frame: bytes) -> dict:
    service = frame[6 + 1]
    subservice = frame[6 + 2]
    counter = (frame[6 + 3] << 8) | frame[6 + 4]
    payload = frame[CCSDS_HDR + PUS_TM_SEC:-2]
    return {
        "service": service,
        "subservice": subservice,
        "counter": counter & 0x3FFF,
        "payload": payload,
        "length": len(frame),
    }


def verify(capture: bytes) -> dict:
    stream = strip_ansi(capture)
    packets = scan_packets(stream)
    decoded = [decode(p) for p in packets]
    reasons: list = []

    counts: dict = {}
    for item in decoded:
        key = (item["service"], item["subservice"])
        counts[key] = counts.get(key, 0) + 1

    if not packets:
        reasons.append("no_tm_received")

    for key, want in EXPECTED.items():
        got = counts.get(key, 0)
        if got != want:
            reasons.append(f"count_S{key[0]}.{key[1]}={got}")

    for key in sorted(FORBIDDEN):
        if counts.get(key, 0):
            reasons.append(f"unexpected_S{key[0]}.{key[1]}")

    for key in sorted(counts):
        if key in EXPECTED or key in FORBIDDEN:
            continue
        if key[0] in OBSERVATIONAL_SERVICES:
            continue  # reported, not counted against the U1 set
        reasons.append(f"extra_S{key[0]}.{key[1]}")

    s17 = [d for d in decoded if (d["service"], d["subservice"]) == (17, 2)]
    if len(s17) == 1 and s17[0]["payload"] != S17_2_PAYLOAD:
        reasons.append("S17.2_payload_mismatch")

    prev = None
    for item in decoded:
        if prev is not None:
            if item["counter"] != (prev + 1) & 0x3FFF:
                reasons.append(f"counter_not_incremental_at_{item['counter']}")
                break
        prev = item["counter"]

    return {
        "ok": not reasons,
        "reasons": sorted(set(reasons)),
        "packets": len(packets),
        "counts": {f"S{k[0]}.{k[1]}": v for k, v in sorted(counts.items())},
        "payloads": [
            {"service": d["service"], "subservice": d["subservice"],
             "counter": d["counter"], "payload_hex": d["payload"].hex().upper()}
            for d in decoded
        ],
    }


def main(argv: list) -> int:
    parser = argparse.ArgumentParser(description="Verify UART-captured TM against the U1 set")
    parser.add_argument("--capture", required=True, help="raw captured serial byte stream")
    parser.add_argument("--json", dest="json_out", default=None)
    args = parser.parse_args(argv)

    try:
        capture = open(args.capture, "rb").read()
    except OSError as exc:
        print(f"[uart_tm_verify] usage error: {exc}", file=sys.stderr)
        return 2

    result = verify(capture)

    print("=" * 72)
    print("frick-fsw UART telemetry verification (tools/scripts/uart_tm_verify.py)")
    print("=" * 72)
    print(f"capture bytes: {len(capture)}")
    print(f"TM packets accepted (CRC-valid): {result['packets']}")
    if result["counts"]:
        print("services seen: " + ", ".join(f"{k}={v}" for k, v in result["counts"].items()))
    for item in result["payloads"]:
        print(f"  S{item['service']}.{item['subservice']} counter={item['counter']} "
              f"payload={item['payload_hex']}")
    if result["reasons"]:
        print("Reasons:")
        for reason in result["reasons"]:
            print(f"  E: {reason}")
    print("-" * 72)
    print("uart-tm: PASS (U1 set verified)" if result["ok"] else "uart-tm: FAIL")

    if args.json_out:
        try:
            with open(args.json_out, "w", encoding="utf-8") as handle:
                json.dump(result, handle, indent=2, sort_keys=True)
                handle.write("\n")
        except OSError as exc:
            print(f"[uart_tm_verify] cannot write {args.json_out}: {exc}", file=sys.stderr)
            return 2

    return 0 if result["ok"] else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
