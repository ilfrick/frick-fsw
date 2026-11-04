#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""XW-3 negative controls: they must fail, for the right reason.

Every control below is executed through the REAL runner
(`tools/scripts/qemu_xw3_vv.sh`, fixture mode) and the REAL verdict path
(`tools/scripts/xw3_verdict.py`). No verifier is mocked and no verdict is
fabricated: the runner is invoked as a subprocess and its exit code plus the
retained `verdict.json` are what is judged.

Two kinds of control:

  real   a genuine QEMU run of the no-wrap control image, which panics at
         watchdog init (fatal BEFORE the response set exists);
  fixture  a capture derived from a real accepted run by a documented,
         deterministic mutation (payload change with a recomputed CRC, removal
         of one packet, appended synthetic fatal marker) or an identity defect
         (stale nonce, wrong ELF, nonzero QEMU exit, empty/truncated capture).
         The fixtures are not evidence of a live run; they exist only to prove
         the verdict discriminates.

The positive replay control (P1) re-feeds the UNMODIFIED accepted capture and
must PASS, so the fixture path is not biased to fail.

Exit: 0 = every control behaved exactly as specified; 1 = at least one
control did not; 2 = missing prerequisite.

Copyright (C) 2026 Ilfrick
SPDX-License-Identifier: MPL-2.0
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import xw3_verdict as oracle  # the oracle's own CRC/parse primitives (cross-checked
#                              against real SUT traffic by the XW3-A1 case)

GOLDEN_HEX = "18 10 C0 00 00 0A 2B 11 01 01 02 01 02 03 04 F7 2B"
STALE_NONCE = "0" * 32
WRONG_ELF = "0" * 64
SYNTHETIC_FATAL = b"\n[FF-PANIC] injected control marker (synthetic)\n"


def packet_spans(stream: bytes) -> list:
    scan = oracle.scan_tm(stream)
    spans = []
    index = 0
    for frame in scan["packets"]:
        start = stream.find(frame, index)
        spans.append((start, start + len(frame), frame))
        index = start + len(frame)
    return spans


def rewrite_frame(capture: bytes, start: int, end: int, frame: bytes) -> bytes:
    return capture[:start] + frame + capture[end:]


def mutate_subservice(capture: bytes, svc: int, sub: int, new_sub: int) -> bytes:
    for start, end, frame in packet_spans(capture):
        decoded = oracle.decode_tm(frame)
        if (decoded["service"], decoded["subservice"]) == (svc, sub):
            body = bytearray(frame[:-2])
            body[oracle.CCSDS_HDR + 2] = new_sub
            crc = oracle.crc16_ccitt_false(bytes(body))
            new_frame = bytes(body) + crc.to_bytes(2, "big")
            return rewrite_frame(capture, start, end, new_frame)
    raise RuntimeError(f"packet S{svc}.{sub} not found in the accepted capture")


def mutate_payload(capture: bytes, svc: int, sub: int) -> bytes:
    for start, end, frame in packet_spans(capture):
        decoded = oracle.decode_tm(frame)
        if (decoded["service"], decoded["subservice"]) == (svc, sub):
            body = bytearray(frame[:-2])
            body[oracle.CCSDS_HDR + oracle.PUS_TM_SEC] ^= 0xFF
            crc = oracle.crc16_ccitt_false(bytes(body))
            new_frame = bytes(body) + crc.to_bytes(2, "big")
            return rewrite_frame(capture, start, end, new_frame)
    raise RuntimeError(f"packet S{svc}.{sub} not found in the accepted capture")


def remove_packet(capture: bytes, svc: int, sub: int) -> bytes:
    for start, end, frame in packet_spans(capture):
        decoded = oracle.decode_tm(frame)
        if (decoded["service"], decoded["subservice"]) == (svc, sub):
            return capture[:start] + capture[end:]
    raise RuntimeError(f"packet S{svc}.{sub} not found in the accepted capture")


def write_fixture(path: Path, capture: bytes, tc: str, *, meta: dict | None,
                  with_meta: bool = True) -> None:
    path.mkdir(parents=True, exist_ok=True)
    (path / "capture.bin").write_bytes(capture)
    (path / "tc.txt").write_text(tc + "\n", encoding="utf-8")
    if with_meta:
        payload = meta if meta is not None else {
            "case": "fixture",
            "variant": "fixture",
            "run_nonce": STALE_NONCE,
            "elf_sha256": WRONG_ELF,
            "capture_sha256": "0" * 64,
            "capture_bytes": len(capture),
            "qemu_exit": 124,
        }
        (path / "run-meta.json").write_text(
            json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def run_runner(runner: Path, fixture: Path, case: str, mode: str, env_extra: dict) -> subprocess.CompletedProcess:
    env = dict(os.environ)
    env.update(env_extra)
    return subprocess.run(
        ["bash", str(runner), "--fixture", str(fixture), "--case", case, "--mode", mode],
        capture_output=True, text=True, env=env,
    )


def control_verdict(stdout: str, case: str) -> dict:
    """Locate the verdict the RUNNER produced for this control."""
    run_dir = None
    for line in stdout.splitlines():
        marker = "run directory: "
        if marker in line:
            run_dir = line.split(marker, 1)[1].strip()
    if run_dir is None:
        return {}
    verdict_path = Path(run_dir) / "cases" / case / "verdict.json"
    if not verdict_path.exists():
        return {}
    return json.loads(verdict_path.read_text(encoding="utf-8"))


def latest_run(root: Path) -> Path:
    runs = sorted(root.glob("build/xw3-run-*"))
    if not runs:
        raise SystemExit("[xw3-controls] MISSING PREREQUISITE: no build/xw3-run-* directory")
    return runs[-1]


def main(argv: list) -> int:
    parser = argparse.ArgumentParser(description="XW-3 negative controls (real runner + real verdict)")
    parser.add_argument("--run-dir", default=None, help="a completed real run (default: latest)")
    parser.add_argument("--keep", action="store_true", help="keep the fixture directory")
    args = parser.parse_args(argv)

    root = Path(__file__).resolve().parents[2]
    runner = root / "tools/scripts/qemu_xw3_vv.sh"
    run_dir = Path(args.run_dir) if args.run_dir else latest_run(root)
    accept_capture = run_dir / "cases/XW3-A1/capture.bin"
    control_capture = run_dir / "cases/XW3-C1/capture.bin"
    if not accept_capture.exists():
        raise SystemExit(f"[xw3-controls] MISSING PREREQUISITE: {accept_capture}")
    if not control_capture.exists():
        raise SystemExit(f"[xw3-controls] MISSING PREREQUISITE: {control_capture}")

    accepted = accept_capture.read_bytes()
    fatal_run = control_capture.read_bytes()
    # A fresh directory per invocation: nothing is deleted (no removal of
    # repository content, and no reuse of a previous control set).
    stamp = run_dir.name.split("-")[-1]
    fixtures = root / "build" / f"xw3-fixtures-{stamp}"
    suffix = 1
    while fixtures.exists():
        fixtures = root / "build" / f"xw3-fixtures-{stamp}-r{suffix}"
        suffix += 1
    fixtures.mkdir(parents=True)

    controls = []

    def add(name, capture, *, mode="accept", env_extra=None, tc=GOLDEN_HEX,
            meta=None, with_meta=True, expect_exit=1, expect_reasons=(),
            expect_fatal=False, note=""):
        path = fixtures / name
        write_fixture(path, capture, tc, meta=meta, with_meta=with_meta)
        controls.append({
            "name": name, "path": path, "mode": mode,
            "env": {"XW3_ALLOW_FIXTURE": "1", **(env_extra or {})},
            "tc": tc, "expect_exit": expect_exit, "expect_reasons": list(expect_reasons),
            "expect_fatal": expect_fatal, "note": note,
        })

    # ---- positive replay: the unmodified accepted capture must PASS ---------
    add("P1-positive-replay", accepted, expect_exit=0,
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        note="unmodified accepted capture, identity rebound to this run")

    # ---- capture-content negative controls ---------------------------------
    add("N1-empty-capture", b"", expect_reasons=["S17.2-COUNT"],
        env_extra={"XW3_FIXTURE_REBIND": "1"}, note="no bytes at all")
    add("N2-stale-capture", accepted,
        env_extra={"XW3_FIXTURE_REBIND": "1", "XW3_FIXTURE_REBIND_NONCE": "0"},
        expect_reasons=["nonce_mismatch"], note="capture bound to another run's nonce")
    add("N3-wrong-elf", accepted,
        env_extra={"XW3_FIXTURE_REBIND": "1", "XW3_FIXTURE_REBIND_ELF": "0"},
        expect_reasons=["elf_mismatch"], note="capture bound to a different image")
    add("N4-wrong-payload", mutate_payload(accepted, 17, 2),
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        expect_reasons=["S17.2-PAYLOAD"], note="S17.2 payload altered, CRC recomputed")
    add("N5-wrong-response", mutate_subservice(accepted, 1, 7, 8),
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        expect_reasons=["S1.7-COUNT"], note="completion ack replaced by a failure ack")
    add("N7-fatal-after-pass", accepted + SYNTHETIC_FATAL,
        env_extra={"XW3_FIXTURE_REBIND": "1"}, expect_fatal=True,
        note="valid response set followed by a fatal marker")
    add("N8-qemu-nonzero-clean-capture", accepted,
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        meta={"case": "fixture", "variant": "fixture", "run_nonce": STALE_NONCE,
              "elf_sha256": WRONG_ELF, "capture_sha256": "0" * 64,
              "capture_bytes": len(accepted), "qemu_exit": 1},
        expect_reasons=["qemu_process_failure:1"],
        note="clean capture, QEMU process exit 1")
    add("N9-timeout124-with-fatal", accepted + SYNTHETIC_FATAL,
        env_extra={"XW3_FIXTURE_REBIND": "1"}, expect_fatal=True,
        note="accepted timeout 124 must not hide a fatal marker")
    add("N10-timeout124-incomplete", remove_packet(accepted, 1, 7),
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        expect_reasons=["S1.7-COUNT"],
        note="accepted timeout 124 must not hide an incomplete response set")
    add("N12-malformed-evidence", accepted[:120], with_meta=False,
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        expect_reasons=["run_meta_unreadable"],
        note="truncated capture and no run metadata")
    add("N13-malformed-meta-missing-size", accepted,
        meta={"case": "fixture", "variant": "fixture", "run_nonce": STALE_NONCE,
              "elf_sha256": WRONG_ELF, "capture_sha256": "0" * 64,
              "qemu_exit": 124},
        expect_reasons=["capture_size_missing"],
        note="valid capture, metadata omits the recorded capture size")

    # ---- real fatal control: the no-wrap image panics before any response ---
    add("N6-fatal-before-pass", fatal_run, expect_fatal=True,
        env_extra={"XW3_FIXTURE_REBIND": "1"},
        note="real capture of the no-wrap control image (watchdog init panic)")

    results = []
    failures = 0
    for control in controls:
        proc = run_runner(runner, control["path"], "XW3-CTL", control["mode"], control["env"])
        data = control_verdict(proc.stdout, "XW3-CTL")
        reasons = data.get("identity_reasons", []) + [
            check.get("id", "") + ":" + check.get("detail", "")
            for check in data.get("oracle", {}).get("checks", []) if not check.get("ok")
        ] + list(data.get("oracle", {}).get("reasons", []))
        fatal_hits = data.get("console_fatal", {}).get("hits", [])
        missing = [want for want in control["expect_reasons"]
                   if not any(want in reason for reason in reasons)]
        fatal_ok = (bool(fatal_hits) == control["expect_fatal"])
        exit_ok = (proc.returncode == control["expect_exit"])
        control_ok = exit_ok and not missing and fatal_ok
        if not control_ok:
            failures += 1
        results.append({
            "control": control["name"],
            "expect_exit": control["expect_exit"],
            "actual_exit": proc.returncode,
            "expect_reasons": control["expect_reasons"],
            "missing_reasons": missing,
            "fatal_as_expected": fatal_ok,
            "ok": control_ok,
            "note": control["note"],
        })
        print(f"[xw3-controls] {control['name']:<32} exit={proc.returncode} "
              f"({'ok' if control_ok else 'CONTROL FAILED'})")
        if not control_ok:
            print(f"    expected exit {control['expect_exit']}, fatal={control['expect_fatal']}")
            print(f"    reasons seen: {reasons[:6]}")
            print(f"    stdout tail: {proc.stdout.strip().splitlines()[-3:]}")

    # ---- the fixture seam must be refused without the explicit flag ---------
    seam = subprocess.run(
        ["bash", str(runner), "--fixture", str(fixtures / "P1-positive-replay"),
         "--case", "XW3-CTL", "--mode", "accept"],
        capture_output=True, text=True, env={k: v for k, v in os.environ.items()
                                             if k != "XW3_ALLOW_FIXTURE"},
    )
    seam_ok = seam.returncode == 2
    if not seam_ok:
        failures += 1
    results.append({"control": "N11a-fixture-seam-refused", "expect_exit": 2,
                    "actual_exit": seam.returncode, "ok": seam_ok,
                    "note": "fixture mode without XW3_ALLOW_FIXTURE must be refused"})
    print(f"[xw3-controls] {'N11a-fixture-seam-refused':<32} exit={seam.returncode} "
          f"({'ok' if seam_ok else 'CONTROL FAILED'})")

    missing_dir = subprocess.run(
        ["bash", str(runner), "--fixture", str(root / "build/definitely-absent"),
         "--case", "XW3-CTL", "--mode", "accept"],
        capture_output=True, text=True,
        env={**os.environ, "XW3_ALLOW_FIXTURE": "1"},
    )
    missing_ok = missing_dir.returncode == 2
    if not missing_ok:
        failures += 1
    results.append({"control": "N11b-missing-prerequisite", "expect_exit": 2,
                    "actual_exit": missing_dir.returncode, "ok": missing_ok,
                    "note": "an absent fixture/image prerequisite must exit 2, never 0"})
    print(f"[xw3-controls] {'N11b-missing-prerequisite':<32} exit={missing_dir.returncode} "
          f"({'ok' if missing_ok else 'CONTROL FAILED'})")

    summary = {
        "run_dir": str(run_dir),
        "controls": results,
        "passed": sum(1 for r in results if r["ok"]),
        "failed": failures,
        "result": "PASS" if failures == 0 else "FAIL",
    }
    (fixtures / "controls.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print("-" * 72)
    print(f"controls: {summary['passed']}/{len(results)} behaved as specified")
    print(f"RESULT: {summary['result']}")
    if not args.keep:
        pass  # fixtures are retained under build/ for review; nothing is deleted
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
