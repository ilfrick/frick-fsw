#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""Whole-run verdict for a T1 target run: component result AND run result.

Two independent questions, two independent answers:

* **COMPONENT** — did every required case emit exactly one PASS line? This is
  decided by the existing per-case logic in ``target_req_verify.py``, imported
  rather than re-implemented so the evidence grammar has a single definition.
* **QEMU-PROCESS** — did the emulator itself exit acceptably (0, or 124 for the
  bounded timeout)? A crash that leaves a clean case console is still a failure.
* **WHOLE-RUN** — did the *image* survive? A run that panics after (or before)
  the case lines is a FAILED run even when every case passed. The markers are
  taken from the retained console text, never invented: ``[FF-PANIC]``,
  ``RTEMS_FATAL_SOURCE_EXIT``, ``*** EXIT STATUS NOT ZERO ***`` and the
  ``exit source:`` / ``exit code:`` block that follows them.

Exit code is 0 **only** when the component set is exactly PASS *and* no fatal
marker is present. A component PASS is never promoted to an overall PASS.

This exists because the first T1 delivery reported ``target PASS`` for a run
that aborted at watchdog init (REVIEW-T1 F1): the QEMU process exit code was 0
and the per-case verifier ignores non-``TGTREQ`` lines, so nothing in the chain
recorded the failure.

Copyright (C) 2026 Ilfrick
SPDX-License-Identifier: MPL-2.0
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import target_req_verify as trv  # noqa: E402  (same-directory companion module)

# RTEMS prints one of two terminal formats: `*** EXIT STATUS NOT ZERO ***` with
# an `exit source:`/`exit code:` block for RTEMS_FATAL_SOURCE_EXIT, and
# `*** FATAL ***` with a `fatal source:` block for every other fatal source
# (assert, exception, heap, SMP, panic, stack checker, invalid free). Both are
# matched, together with the project's own panic text; the set is aligned with
# tools/scripts/renode_vv_verdict.sh so the toolchain has one definition of
# "the image died" (N10 review R2-01).
FATAL_MARKERS = (
    "[FF-PANIC]",
    "RTEMS_FATAL_SOURCE_EXIT",
    "*** EXIT STATUS NOT ZERO ***",
    "*** FATAL ***",
    "fatal source:",
    "rtems_fatal",
    "Fatal error",
)
EXIT_BLOCK_RE = re.compile(r"^\s*(exit source:|exit code:|fatal source:)\s*(.*)$")


def scan_fatal(console_text: str) -> dict:
    """Return the whole-run fatal classification for a captured console."""
    lines = [trv.clean_line(raw) for raw in console_text.splitlines()]
    hits: list = []
    exit_block: list = []
    for idx, line in enumerate(lines, start=1):
        for marker in FATAL_MARKERS:
            if marker in line:
                hits.append({"line": idx, "marker": marker, "text": line.strip()})
        match = EXIT_BLOCK_RE.match(line)
        if match and hits:
            exit_block.append(line.strip())

    reason = ""
    if hits:
        markers = sorted({hit["marker"] for hit in hits})
        if "[FF-PANIC]" in markers:
            reason = "watchdog fatal" if any("watchdog" in h["text"] for h in hits) else "panic"
        elif "RTEMS_FATAL_SOURCE_EXIT" in markers or "*** EXIT STATUS NOT ZERO ***" in markers:
            reason = "RTEMS fatal exit"
        else:
            reason = "RTEMS fatal (non-exit source)"
    return {
        "ok": not hits,
        "hits": hits,
        "exit_block": exit_block,
        "reason": reason,
    }


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    digest.update(path.read_bytes())
    return digest.hexdigest()


def main(argv: list) -> int:
    parser = argparse.ArgumentParser(description="Whole-run verdict for a T1 target run")
    parser.add_argument("--console", required=True, help="console log captured from the target run")
    parser.add_argument("--manifest", required=True, help="required-case manifest (JSON)")
    parser.add_argument("--json", dest="json_out", default=None, help="write the verdict as JSON")
    parser.add_argument("--nonce", default="", help="run nonce to echo into the verdict")
    parser.add_argument("--qemu-exit", type=int, default=None,
                        help="QEMU process exit code (0 and 124/timeout are the accepted codes)")
    args = parser.parse_args(argv)

    try:
        console_path = Path(args.console)
        manifest_path = Path(args.manifest)
        console_text = console_path.read_text(encoding="utf-8", errors="replace")
        manifest_text = manifest_path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        print(f"[target_run_verdict] usage error: {exc}", file=sys.stderr)
        return 2

    cases, gaps, manifest_errors = trv.parse_manifest(manifest_text)
    component = trv.verify(console_text, cases, gaps)
    component.reasons.extend(manifest_errors)
    component.reasons = sorted(set(component.reasons))
    if manifest_errors:
        component.ok = False

    fatal = scan_fatal(console_text)
    qemu_failed = (args.qemu_exit is not None) and (args.qemu_exit not in (0, 124))
    overall_ok = bool(component.ok and fatal["ok"] and not qemu_failed)

    print("=" * 72)
    print("frick-fsw T1 whole-run verdict (tools/scripts/target_run_verdict.py)")
    print("=" * 72)
    print(f"console: {console_path}")
    print(f"manifest: {manifest_path}")
    if args.nonce:
        print(f"run nonce: {args.nonce}")
    print("-" * 72)
    # Component summary (deliberately separate from the whole-run line).
    if component.ok:
        print(f"COMPONENT: PASS {component.required_pass}/{component.required_total}")
    else:
        print(
            f"COMPONENT: FAIL {component.required_pass}/{component.required_total} "
            f"({len(component.reasons)} reason(s))"
        )
        for reason in component.reasons:
            print(f"  E: {reason}")
    print(f"COVERAGE: {component.required_pass}/{component.required_total} required cases")
    if args.qemu_exit is not None:
        state = "FAIL (QEMU process failure)" if qemu_failed else "accepted"
        print(f"QEMU-PROCESS: exit {args.qemu_exit} ({state})")
    if fatal["ok"]:
        print("WHOLE-RUN: PASS (no panic/fatal marker in the console)")
    else:
        print(f"WHOLE-RUN: FAIL ({fatal['reason']})")
        for hit in fatal["hits"]:
            print(f"  F: line {hit['line']}: {hit['marker']} :: {hit['text']}")
        for line in fatal["exit_block"]:
            print(f"  F: {line}")
    print("-" * 72)
    print("RESULT: PASS" if overall_ok else "RESULT: FAIL")
    if component.ok and not fatal["ok"]:
        print("NOTE: component cases passed, but the run aborted - this is NOT a target PASS.")
    if component.ok and fatal["ok"] and qemu_failed:
        print("NOTE: component cases passed and no fatal marker was seen, but the QEMU "
              "process itself failed - this is NOT a target PASS.")

    if args.json_out:
        payload = {
            "run_nonce": args.nonce,
            "console": str(console_path),
            "console_sha256": file_sha256(console_path),
            "manifest": str(manifest_path),
            "manifest_sha256": file_sha256(manifest_path),
            "component": {
                "ok": component.ok,
                "required_total": component.required_total,
                "required_pass": component.required_pass,
                "status_totals": component.status_totals,
                "observations": component.observations,
                "gaps": [
                    {"requirement": r, "case": c, "reason": why}
                    for (r, c, why) in component.gaps
                ],
                "reasons": component.reasons,
            },
            "qemu_exit": args.qemu_exit,
            "qemu_failed": qemu_failed,
            "whole_run": fatal,
            "result": "PASS" if overall_ok else "FAIL",
        }
        try:
            Path(args.json_out).write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n",
                                           encoding="utf-8")
        except OSError as exc:
            print(f"[target_run_verdict] cannot write {args.json_out}: {exc}", file=sys.stderr)
            return 2

    return 0 if overall_ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
