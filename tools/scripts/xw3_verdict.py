#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""XW-3 independent telemetry oracle and whole-run verdict.

The capture is the raw byte stream QEMU wrote to its serial console while one or
more telecommands were delivered on the SAME serial interface (the real external
path: `/dev/console` RX -> `link_uart` -> CMD_INGEST -> PUS -> `link_uart` TX).

This tool is DERIVED FROM THE CONTROLLED INTERFACE, NOT FROM THE FLIGHT CODEC:

  * it implements CRC-16/CCITT-FALSE itself (poly 0x1021, init 0xFFFF, no
    reflection), per ICD §3;
  * it parses the CCSDS primary header at fixed offsets per ICD §3 (version 0,
    type 0 for TM, secondary-header flag 1, APID 0x020, PDL = data length - 1);
  * it reads the 13-octet PUS TM secondary header per ICD §3 (version, service,
    subservice, 14-bit counter, destination id, coarse/fine time);
  * it derives the expected request from the INJECTED TC OCTETS (also per ICD
    §3: 5-octet TC secondary header, so service/subservice/source id and the
    user data come from the stimulus, never from the capture);
  * it never correlates by packet position and never compares the capture with a
    copy of itself.

Asserted (see `docs/vv/xw3-tc-oracle-matrix.md`):

  accept mode  S17.2 exists exactly once and carries the TC user data octet for
               octet (SRS-REQ-012); exactly one S1.1, one S1.3 and one S1.7 and
               no other S1 subservice (SRS-REQ-006, by service/subservice only);
               the injected golden frame's own CRC verifies (SRS-REQ-004
               stimulus self-check); every accepted packet has APID 0x020, CCSDS
               version 0, type 0, secondary-header flag 1, a self-consistent PDL
               and a matching CRC (SRS-REQ-004); TM counters strictly increase
               modulo 16384 in capture order (SRS-REQ-007).
  reject mode  no S17.2 and no S1.1/S1.3/S1.7 (the frame was not executed,
               SRS-REQ-004/005) while the capture proves the serial path was live
               (at least one CRC-valid TM packet).
  observe mode no set-shape assertion (duplicate TC, split frame): the observed
               counts are recorded because the ICD does not fix the behaviour.

Deliberately NOT asserted (authority pending / open NC):

  * the numeric S1 payload layout (TC type/subtype/errcode) - recorded as an
    observation; the S1 packet is counted by service/subservice only;
  * the S1.8 acceptance-of-failure reporting for a malformed frame (NC-EXT-04):
    absence or presence is an observation, not an assertion;
  * duplicate-TC response behaviour and split-frame reassembly (not fixed by the
    ICD) - observations only.

Identity binding: the run metadata written by `qemu_xw3_vv.sh` must carry the run
nonce, the launched ELF SHA-256 and the capture SHA-256; all three are recomputed
here and compared. A capture carried over from another run (stale) or bound to a
different image therefore fails.

Whole-run verdict (`summary` subcommand): every required case of the manifest
must be present with its expected outcome; a missing case, a case whose result
contradicts the manifest, a fatal console marker, a QEMU process code outside
{0, 124}, or an identity mismatch fails the run. With `--run-index` the summary
is additionally bound to the run identity: the run-index must match the live
manifest/oracle hashes and every case's run-meta nonce/ELF and capture hash must
match. There is no SKIP=0 path.

Exit codes: 0 = all checks pass; 1 = verification failure; 2 = usage error.
Never 0 when nothing was received.

Copyright (C) 2026 Ilfrick
SPDX-License-Identifier: MPL-2.0
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import target_run_verdict as trv  # shared console fatal-marker scan (T27)

TM_APID = 0x020
TC_APID = 0x010
CCSDS_HDR = 6
PUS_TM_SEC = 13
PUS_TC_SEC = 5
COUNTER_MOD = 16384
QEMU_ACCEPTED = (0, 124)
MAX_PACKET = 4096
ANSI_RE = re.compile(rb"\x1b\[[0-9;?]*[A-Za-z]")
HEX_RE = re.compile(r"^[0-9a-fA-F\s]+$")


# --------------------------------------------------------------------------
# independent primitives
# --------------------------------------------------------------------------
def crc16_ccitt_false(data: bytes) -> int:
    """CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF, no reflection), ICD §3."""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def strip_ansi(data: bytes) -> bytes:
    return ANSI_RE.sub(b"", data)


def parse_hex(text: str) -> bytes:
    cleaned = " ".join(text.split())
    if not HEX_RE.match(cleaned):
        raise ValueError("not a hex octet string")
    return bytes.fromhex(cleaned)


def decode_primary(frame: bytes) -> dict:
    word0 = (frame[0] << 8) | frame[1]
    return {
        "version": (word0 >> 13) & 0x7,
        "type": (word0 >> 12) & 0x1,
        "sec_hdr": (word0 >> 11) & 0x1,
        "apid": word0 & 0x07FF,
        "seq_flags": (frame[2] >> 6) & 0x3,
        "seq_count": ((frame[2] & 0x3F) << 8) | frame[3],
        "pdl": (frame[4] << 8) | frame[5],
    }


def parse_tc(frame: bytes) -> dict:
    """TC structure per ICD §3 (5-octet PUS secondary header)."""
    if len(frame) < CCSDS_HDR + PUS_TC_SEC:
        raise ValueError("TC shorter than primary + secondary header")
    hdr = decode_primary(frame)
    total = CCSDS_HDR + hdr["pdl"] + 1
    if total != len(frame):
        raise ValueError(f"TC length {len(frame)} inconsistent with PDL {hdr['pdl']}")
    sec = frame[CCSDS_HDR:CCSDS_HDR + PUS_TC_SEC]
    user = frame[CCSDS_HDR + PUS_TC_SEC:len(frame) - 2]
    return {
        "primary": hdr,
        "pus_version": (sec[0] >> 4) & 0xF,
        "ack_flags": sec[0] & 0xF,
        "service": sec[1],
        "subservice": sec[2],
        "source_id": (sec[3] << 8) | sec[4],
        "user_data": user,
        "crc_ok": crc16_ccitt_false(frame[:-2]) == ((frame[-2] << 8) | frame[-1]),
    }


def scan_tm(stream: bytes) -> dict:
    """Find CCSDS TM packets by header fields and own CRC (never by position)."""
    packets: list = []
    crc_failures: list = []
    header_failures: list = []
    truncated_tail = 0
    index = 0
    limit = len(stream) - (CCSDS_HDR + PUS_TM_SEC + 2)
    while index <= limit:
        if stream[index] != 0x08 or stream[index + 1] != 0x20:
            index += 1
            continue
        pdl = (stream[index + 4] << 8) | stream[index + 5]
        total = CCSDS_HDR + pdl + 1
        if total < CCSDS_HDR + PUS_TM_SEC + 2 or total > MAX_PACKET:
            header_failures.append(index)
            index += 1
            continue
        if index + total > len(stream):
            truncated_tail += 1
            break
        frame = stream[index:index + total]
        hdr = decode_primary(frame)
        if hdr["apid"] != TM_APID or hdr["version"] != 0 or hdr["type"] != 0 or hdr["sec_hdr"] != 1:
            header_failures.append(index)
            index += 1
            continue
        if crc16_ccitt_false(frame[:-2]) != ((frame[-2] << 8) | frame[-1]):
            crc_failures.append(index)
            index += 1
            continue
        packets.append(frame)
        index += total
    return {
        "packets": packets,
        "crc_failures": crc_failures,
        "header_failures": header_failures,
        "truncated_tail": truncated_tail,
    }


def decode_tm(frame: bytes) -> dict:
    sec = frame[CCSDS_HDR:CCSDS_HDR + PUS_TM_SEC]
    return {
        "service": sec[1],
        "subservice": sec[2],
        "counter": ((sec[3] << 8) | sec[4]) & (COUNTER_MOD - 1),
        "destination_id": (sec[5] << 8) | sec[6],
        "coarse": int.from_bytes(sec[7:11], "big"),
        "fine": (sec[11] << 8) | sec[12],
        "payload": frame[CCSDS_HDR + PUS_TM_SEC:-2],
        "length": len(frame),
    }


# --------------------------------------------------------------------------
# identity binding
# --------------------------------------------------------------------------
def bind_identity(run_meta_path: Path, capture: bytes, nonce: str, elf_sha: str) -> list:
    reasons: list = []
    try:
        meta = json.loads(run_meta_path.read_text(encoding="utf-8"))
    except OSError as exc:
        return [f"run_meta_unreadable:{exc.__class__.__name__}"]
    except json.JSONDecodeError:
        return ["run_meta_malformed"]
    if not isinstance(meta, dict):
        return ["run_meta_not_an_object"]
    if meta.get("run_nonce") != nonce:
        reasons.append("nonce_mismatch")
    if meta.get("elf_sha256") != elf_sha:
        reasons.append("elf_mismatch")
    actual = hashlib.sha256(capture).hexdigest()
    if meta.get("capture_sha256") != actual:
        reasons.append("capture_sha_mismatch")
    # Malformed evidence fails closed: the size must be present (not merely
    # "consistent when present") and must equal the bytes actually read.
    recorded_size = meta.get("capture_bytes")
    if recorded_size is None:
        reasons.append("capture_size_missing")
    elif not isinstance(recorded_size, int) or isinstance(recorded_size, bool) \
            or recorded_size != len(capture):
        reasons.append("capture_size_mismatch")
    qemu_exit = meta.get("qemu_exit")
    if not isinstance(qemu_exit, int):
        reasons.append("qemu_exit_missing")
    elif qemu_exit not in QEMU_ACCEPTED:
        reasons.append(f"qemu_process_failure:{qemu_exit}")
    return reasons


# --------------------------------------------------------------------------
# oracle
# --------------------------------------------------------------------------
def verify(capture: bytes, tc: bytes, mode: str) -> dict:
    reasons: list = []
    checks: list = []
    observations: dict = {}

    def check(check_id: str, ok: bool, detail: str) -> None:
        checks.append({"id": check_id, "ok": bool(ok), "detail": detail})
        if not ok:
            reasons.append(f"{check_id}:{detail}")

    try:
        req = parse_tc(tc)
    except ValueError as exc:
        return {
            "ok": False,
            "mode": mode,
            "reasons": [f"tc_unparsable:{exc}"],
            "checks": [],
            "counts": {},
            "observations": {},
        }

    check("TC-STRUCT", req["primary"]["version"] == 0
          and req["primary"]["type"] == 1 and req["primary"]["sec_hdr"] == 1
          and req["primary"]["apid"] == TC_APID,
          f"version={req['primary']['version']} apid=0x{req['primary']['apid']:03x} "
          f"type={req['primary']['type']}")
    if mode != "reject":
        # The stimulus self-check for the accepted/observed cases: the injected
        # golden frame's own CRC must verify under this independent
        # implementation, so a response set can never be attributed to a frame
        # the controlled interface would reject.
        check("TC-CRC-VALID", req["crc_ok"] is True, f"crc_ok={req['crc_ok']}")

    stream = strip_ansi(capture)
    scan = scan_tm(stream)
    decoded = [decode_tm(p) for p in scan["packets"]]

    counts: dict = {}
    for item in decoded:
        key = (item["service"], item["subservice"])
        counts[key] = counts.get(key, 0) + 1

    observations["tm_packets"] = len(decoded)
    observations["services"] = {f"S{k[0]}.{k[1]}": v for k, v in sorted(counts.items())}
    observations["s1_8_reported"] = counts.get((1, 8), 0) > 0
    observations["tm_crc_failures"] = len(scan["crc_failures"])
    observations["tm_header_failures"] = len(scan["header_failures"])
    observations["truncated_tail"] = scan["truncated_tail"]
    observations["destination_ids"] = sorted({d["destination_id"] for d in decoded})
    observations["s1_payload_prefixes"] = [
        d["payload"][:5].hex().upper() for d in decoded if d["service"] == 1
    ]
    observations["s1_payload_prefixes_note"] = (
        "recorded only: the numeric S1 payload layout is authority-pending"
    )

    # The unchanged command-ingest application announces the backend it
    # listens on. Asserting the UART backend ties the response set to the
    # external path: the bytes were not consumed by an in-image loopback.
    check("LINK-UART", b"CI app listening on link 'uart'" in stream,
          "ingest app announces the uart backend")
    check("TM-CRC", not scan["crc_failures"], f"crc_failures={len(scan['crc_failures'])}")
    check("TM-HEADER", not scan["header_failures"],
          f"header_failures={len(scan['header_failures'])}")

    if mode == "reject":
        # The path must have been live, otherwise "no response" proves nothing.
        check("CAPTURE-LIVE", len(decoded) >= 1, f"tm_packets={len(decoded)}")
        check("NO-S17.2", counts.get((17, 2), 0) == 0, f"count={counts.get((17, 2), 0)}")
        for sub in (1, 3, 7):
            check(f"NO-S1.{sub}", counts.get((1, sub), 0) == 0, f"count={counts.get((1, sub), 0)}")
        check("TC-CRC-INVALID", req["crc_ok"] is False, f"crc_ok={req['crc_ok']}")
    elif mode == "observe":
        check("CAPTURE-LIVE", len(decoded) >= 1, f"tm_packets={len(decoded)}")
        observations["s17_2_count"] = counts.get((17, 2), 0)
        observations["s1_acceptance_count"] = counts.get((1, 1), 0)
        observations["s1_start_count"] = counts.get((1, 3), 0)
        observations["s1_completion_count"] = counts.get((1, 7), 0)
    else:  # accept
        check("S17.2-COUNT", counts.get((17, 2), 0) == 1, f"count={counts.get((17, 2), 0)}")
        check("S1.1-COUNT", counts.get((1, 1), 0) == 1, f"count={counts.get((1, 1), 0)}")
        check("S1.3-COUNT", counts.get((1, 3), 0) == 1, f"count={counts.get((1, 3), 0)}")
        check("S1.7-COUNT", counts.get((1, 7), 0) == 1, f"count={counts.get((1, 7), 0)}")
        extra_s1 = sorted(
            f"S1.{sub}" for (svc, sub) in counts if svc == 1 and sub not in (1, 3, 7)
        )
        check("S1-SET", not extra_s1, f"extra={extra_s1}")
        s17 = [d for d in decoded if (d["service"], d["subservice"]) == (17, 2)]
        payload_ok = len(s17) == 1 and s17[0]["payload"] == req["user_data"]
        got = s17[0]["payload"].hex().upper() if len(s17) == 1 else "n/a"
        check("S17.2-PAYLOAD", payload_ok, f"got={got} want={req['user_data'].hex().upper()}")

    counter_breaks = []
    prev = None
    for item in decoded:
        if prev is not None:
            delta = (item["counter"] - prev) % COUNTER_MOD
            if delta == 0:
                counter_breaks.append(item["counter"])
        prev = item["counter"]
    check("TM-COUNTER", not counter_breaks, f"breaks={counter_breaks[:4]}")

    return {
        "ok": not reasons,
        "mode": mode,
        "reasons": reasons,
        "checks": checks,
        "counts": {f"S{k[0]}.{k[1]}": v for k, v in sorted(counts.items())},
        "request": {
            "service": req["service"],
            "subservice": req["subservice"],
            "source_id": req["source_id"],
            "user_data_hex": req["user_data"].hex().upper(),
            "crc_ok": req["crc_ok"],
        },
        "observations": observations,
    }


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------
def cmd_verify(args: argparse.Namespace) -> int:
    try:
        capture = Path(args.capture).read_bytes()
    except OSError as exc:
        print(f"[xw3] usage error: {exc}", file=sys.stderr)
        return 2
    try:
        tc = parse_hex(args.tc)
    except ValueError as exc:
        print(f"[xw3] usage error: --tc {exc}", file=sys.stderr)
        return 2

    identity = bind_identity(Path(args.run_meta), capture, args.nonce, args.elf_sha256)
    oracle = verify(capture, tc, args.mode)
    console_text = capture.decode("utf-8", errors="replace")
    fatal = trv.scan_fatal(console_text)

    result = "PASS" if (oracle["ok"] and not identity and fatal["ok"]) else "FAIL"
    report = {
        "result": result,
        "mode": args.mode,
        "capture_sha256": hashlib.sha256(capture).hexdigest(),
        "capture_bytes": len(capture),
        "identity_reasons": identity,
        "console_fatal": fatal,
        "oracle": oracle,
    }

    print("=" * 72)
    print("frick-fsw XW-3 external TC/TM oracle (tools/scripts/xw3_verdict.py)")
    print("=" * 72)
    print(f"mode: {args.mode}   capture: {len(capture)} bytes   sha256 {report['capture_sha256']}")
    print(f"request: S{oracle['request']['service']}.{oracle['request']['subservice']} "
          f"src=0x{oracle['request']['source_id']:04x} "
          f"user_data={oracle['request']['user_data_hex']} "
          f"tc_crc_ok={oracle['request']['crc_ok']}")
    print(f"identity: {'OK' if not identity else identity}")
    print(f"console fatal markers: {len(fatal['hits'])} (ok={fatal['ok']})")
    for hit in fatal["hits"]:
        print(f"  F: line {hit['line']}: {hit['marker']} :: {hit['text']}")
    print("-" * 72)
    for item in oracle["checks"]:
        print(f"  [{'ok ' if item['ok'] else 'FAIL'}] {item['id']}: {item['detail']}")
    print(f"observed services: {oracle['counts']}")
    obs = oracle["observations"]
    print(f"observed: s1_8_reported={obs.get('s1_8_reported')} "
          f"truncated_tail={obs.get('truncated_tail')} "
          f"crc_failures={obs.get('tm_crc_failures')} "
          f"header_failures={obs.get('tm_header_failures')}")
    if oracle["reasons"]:
        print("reasons:")
        for reason in oracle["reasons"]:
            print(f"  E: {reason}")
    print("-" * 72)
    print(f"oracle: {'PASS' if oracle['ok'] else 'FAIL'}   "
          f"identity: {'PASS' if not identity else 'FAIL'}   "
          f"console: {'PASS' if fatal['ok'] else 'FAIL'}")
    print(f"RESULT: {result}")

    if args.json:
        try:
            Path(args.json).write_text(json.dumps(report, indent=2, sort_keys=True) + "\n",
                                       encoding="utf-8")
        except OSError as exc:
            print(f"[xw3] cannot write {args.json}: {exc}", file=sys.stderr)
            return 2
    return 0 if result == "PASS" else 1


def _sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_staged_summary(path: Path) -> dict:
    """Read and validate the staged case-only summary.

    Args:
        path: Path to the staging JSON file (summary.case.json).

    Returns:
        Parsed summary dict if valid object and PASS has reasons=[]; otherwise
        a FAIL dict with appropriate reason:
        - staging_unreadable: file could not be read
        - staging_malformed: file is not valid JSON
        - staging_not_an_object: JSON is valid but not a dict
        - staging_inconsistent_pass: result is PASS but reasons is not exactly []
    """
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as exc:
        return {"result": "FAIL", "reasons": [f"staging_unreadable:{exc.__class__.__name__}"]}

    try:
        data = json.loads(text)
    except json.JSONDecodeError:
        return {"result": "FAIL", "reasons": ["staging_malformed"]}

    if not isinstance(data, dict):
        return {"result": "FAIL", "reasons": ["staging_not_an_object"]}

    # PASS is only valid when reasons is exactly an empty list;
    # the producer cmd_summary emits PASS only with reasons=[].
    # Any PASS with nonempty/missing/other reasons is inconsistent.
    if data.get("result") == "PASS" and data.get("reasons") != []:
        return {"result": "FAIL", "reasons": ["staging_inconsistent_pass"]}

    return data


def apply_gate_result(summary: dict, gate_reason: str) -> dict:
    """Apply gate decision to a staged summary.

    Preserves PASS only when input result is PASS and gate_reason is empty.
    Otherwise result is FAIL. If no gate_reason but result is FAIL, ensures
    'case_verdict_failed' is in reasons (added if no other reason present).

    Args:
        summary: Staged summary dict with 'result' and 'reasons' keys.
        gate_reason: Gate failure reason string (empty if no gate failure).

    Returns:
        Updated summary dict with result and reasons adjusted.
    """
    result = dict(summary)  # shallow copy to avoid mutating caller's dict
    reasons = list(result.get("reasons", []))

    if gate_reason:
        # Any gate failure forces FAIL
        result["result"] = "FAIL"
        if gate_reason not in reasons:
            reasons.append(gate_reason)
    else:
        # No gate failure: preserve PASS only if already PASS
        if result.get("result") == "PASS":
            result["result"] = "PASS"
        else:
            # FAIL staging stays FAIL; ensure there's at least one reason
            result["result"] = "FAIL"
            if not reasons:
                reasons.append("case_verdict_failed")

    result["reasons"] = reasons
    return result


def atomic_publish(final_path: Path, data: dict) -> None:
    """Write JSON to a temp sibling and atomically replace the final file.

    If os.replace fails, the pre-existing final file (if any) is left unchanged
    and the temp file is removed. The caller must handle the exception.
    """
    import tempfile
    final_path = Path(final_path)
    final_path.parent.mkdir(parents=True, exist_ok=True)
    # Create temp sibling in the same directory for atomic replace on same filesystem
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", dir=final_path.parent,
        prefix=final_path.name + ".tmp", suffix=".json", delete=False
    ) as tmp:
        tmp_path = Path(tmp.name)
        json.dump(data, tmp, indent=2, sort_keys=True)
        tmp.write("\n")
    try:
        os.replace(tmp_path, final_path)
    except OSError:
        # Clean up temp on failure; leave original untouched
        try:
            tmp_path.unlink()
        except OSError:
            pass
        raise


def cmd_summary(args: argparse.Namespace) -> int:
    """Whole-run verdict over the per-case verdicts of one run directory.

    With ``--run-index`` (the real runner always passes it) the summary is bound
    to the run identity: the run-index must carry the same manifest/oracle hashes
    as the live tools, and every manifest case must carry a run-meta nonce/ELF
    equal to the run-index and a capture hash equal to the actual capture. This
    closes the post-hoc substitution gap: a copied verdict/capture set from
    another passing run no longer produces an identical PASS summary.
    """
    manifest_path = Path(args.manifest)
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    cases = manifest.get("cases", [])
    required = [c for c in cases if c.get("required", True) and c.get("mode") != "observe"]
    observations = [c for c in cases if c.get("mode") == "observe"]
    run_dir = Path(args.run_dir)

    reasons: list = []
    seen: dict = {}

    run_index = None
    if args.run_index:
        try:
            run_index = json.loads(Path(args.run_index).read_text(encoding="utf-8"))
        except OSError:
            reasons.append("run_index_unreadable")
        except json.JSONDecodeError:
            reasons.append("run_index_malformed")
        if run_index is not None and not isinstance(run_index, dict):
            reasons.append("run_index_not_an_object")
            run_index = None
    if isinstance(run_index, dict):
        if run_index.get("manifest_sha256") != _sha256_file(manifest_path):
            reasons.append("manifest_mismatch")
        if run_index.get("oracle_tool_sha256") != _sha256_file(Path(__file__).resolve()):
            reasons.append("oracle_mismatch")

    for case in cases:
        case_dir = run_dir / "cases" / case["case"]
        verdict_path = case_dir / "verdict.json"
        if not verdict_path.exists():
            reasons.append(f"missing_case:{case['case']}")
            continue
        try:
            data = json.loads(verdict_path.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            reasons.append(f"malformed_case:{case['case']}")
            continue
        seen[case["case"]] = data
        if isinstance(run_index, dict):
            meta_path = case_dir / "run-meta.json"
            try:
                meta = json.loads(meta_path.read_text(encoding="utf-8"))
            except OSError:
                reasons.append(f"case_meta_unreadable:{case['case']}")
            except json.JSONDecodeError:
                reasons.append(f"case_meta_malformed:{case['case']}")
            else:
                if not isinstance(meta, dict):
                    reasons.append(f"case_meta_not_an_object:{case['case']}")
                else:
                    if meta.get("run_nonce") != run_index.get("run_nonce"):
                        reasons.append(f"case_nonce_mismatch:{case['case']}")
                    if meta.get("elf_sha256") != run_index.get("elf_sha256"):
                        reasons.append(f"case_elf_mismatch:{case['case']}")
                    capture_path = case_dir / "capture.bin"
                    if not capture_path.exists():
                        reasons.append(f"case_capture_missing:{case['case']}")
                    elif meta.get("capture_sha256") != _sha256_file(capture_path):
                        reasons.append(f"case_capture_mismatch:{case['case']}")
        expected = case.get("expect")
        if case.get("mode") == "observe":
            if data.get("result") != "PASS":
                reasons.append(f"observation_invalid:{case['case']}:{data.get('result')}")
            continue
        if data.get("result") != expected:
            reasons.append(f"outcome_mismatch:{case['case']}:{data.get('result')}!={expected}")

    for case in required:
        if case["case"] not in seen:
            reasons.append(f"required_case_absent:{case['case']}")

    print("=" * 72)
    print("frick-fsw XW-3 whole-run verdict")
    print("=" * 72)
    for case in cases:
        data = seen.get(case["case"])
        state = data.get("result") if data else "MISSING"
        tag = "recorded" if case.get("mode") == "observe" else f"expect {case.get('expect')}"
        print(f"  {case['case']:<8} {case.get('mode'):<10} {state:<8} ({tag})")
    print("-" * 72)
    print(f"CASES: {len(seen)}/{len(cases)} present ({len(required)} required, "
          f"{len(observations)} observation)")
    if reasons:
        for reason in reasons:
            print(f"  E: {reason}")
        print("RESULT: FAIL")
        if args.json:
            Path(args.json).write_text(
                json.dumps({"result": "FAIL", "reasons": reasons}, indent=2, sort_keys=True) + "\n",
                encoding="utf-8")
        return 1
    print("RESULT: PASS")
    if args.json:
        Path(args.json).write_text(
            json.dumps({"result": "PASS", "reasons": []}, indent=2, sort_keys=True) + "\n",
            encoding="utf-8")
    return 0


def main(argv: list) -> int:
    parser = argparse.ArgumentParser(description="XW-3 external TC/TM oracle and run verdict")
    sub = parser.add_subparsers(dest="command", required=True)

    verify_p = sub.add_parser("verify", help="verify one capture against one injected TC")
    verify_p.add_argument("--capture", required=True)
    verify_p.add_argument("--tc", required=True, help="injected TC octets (hex)")
    verify_p.add_argument("--mode", required=True, choices=["accept", "reject", "observe"])
    verify_p.add_argument("--run-meta", required=True)
    verify_p.add_argument("--nonce", required=True)
    verify_p.add_argument("--elf-sha256", required=True)
    verify_p.add_argument("--json", default=None)
    verify_p.set_defaults(func=cmd_verify)

    summary_p = sub.add_parser("summary", help="whole-run verdict over a run directory")
    summary_p.add_argument("--run-dir", required=True)
    summary_p.add_argument("--manifest", required=True)
    summary_p.add_argument("--run-index", default=None,
                           help="bind the summary to the run identity (real runs)")
    summary_p.add_argument("--json", default=None)
    summary_p.set_defaults(func=cmd_summary)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
