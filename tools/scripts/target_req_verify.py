#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""frick-fsw requirement-keyed target-evidence verifier (T1, norms N10/N22).

Reads the console log produced by the in-image requirement test application
(``src/apps/app_req_test.c``) and decides whether every case required by a
manifest was actually asserted PASS on the target.

Evidence grammar (one line per case; the console may interleave other output):

    TGTREQ: <SRS-ID>:<CASE>:PASS[:<detail>]
    TGTREQ: <SRS-ID>:<CASE>:FAIL:<detail>
    TGTREQ: <SRS-ID>:<CASE>:SKIP:<reason>
    TGTREQ-OBS: <SRS-ID>:<CASE>:<free text>     (observation only, never coverage)
    TGTREQ-GAP: <SRS-ID>:<CASE>:<reason>        (oracle unimplemented/undetermined)

Decision (fail-closed):
  exit 0 **only if** every manifest case appears exactly once with ``PASS`` and
  there is no ``FAIL``, no ``SKIP``, no unexpected case, no duplicate case line,
  no malformed/truncated evidence line, and no required case also listed as a
  gap. Anything else exits 1 with a machine-readable reason list.

  A case listed in the manifest ``gaps`` is never counted as covered: a ``PASS``
  for it is a failure, and a gap line for a case the manifest does not declare
  as a gap is an unexpected case.

  A bare ``RENODE_TEST: PASS`` aggregate marker with no per-case evidence is a
  failure: the aggregate string is exactly the false-positive pattern that this
  verifier replaces (ASSESSMENT §12 F2).

Exit codes: 0 = verified PASS set; 1 = verification failure; 2 = usage error.
Pure stdlib and pure text parsing: it does not import or execute project code.

Copyright (C) 2026 Ilfrick
SPDX-License-Identifier: MPL-2.0
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field

STATUSES = ("PASS", "FAIL", "SKIP")
PREFIX = "TGTREQ:"
OBS_PREFIX = "TGTREQ-OBS:"
GAP_PREFIX = "TGTREQ-GAP:"

# ANSI/control noise that a serial console may carry.
ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
CTRL_RE = re.compile(r"[\x00-\x08\x0b\x0c\x0e-\x1f\x7f]")


@dataclass
class CaseRecord:
    requirement: str
    case: str
    status: str
    detail: str
    line_no: int


@dataclass
class Result:
    ok: bool = False
    reasons: list = field(default_factory=list)
    cases: list = field(default_factory=list)      # CaseRecord
    gaps: list = field(default_factory=list)       # (requirement, case, reason)
    observations: int = 0
    required_total: int = 0
    required_pass: int = 0
    status_totals: dict = field(default_factory=dict)
    per_requirement: dict = field(default_factory=dict)


def normalize_rid(rid: str) -> str:
    """Drop the optional FRICK- prefix so manifest and console agree."""
    rid = rid.strip()
    return rid[len("FRICK-"):] if rid.startswith("FRICK-") else rid


def clean_line(raw: str) -> str:
    line = ANSI_RE.sub("", raw)
    line = CTRL_RE.sub("", line)
    return line.rstrip("\r\n")


def parse_manifest(text: str) -> tuple[list, list, list]:
    """Return (cases, gaps, errors). cases: [(rid, case)]; gaps: [(rid, case, reason)]."""
    errors: list = []
    try:
        data = json.loads(text)
    except json.JSONDecodeError as exc:
        return [], [], [f"manifest_malformed:{exc.msg}@{exc.lineno}"]
    if not isinstance(data, dict):
        return [], [], ["manifest_not_object"]

    cases: list = []
    for i, item in enumerate(data.get("cases") or []):
        if not isinstance(item, dict) or "requirement" not in item or "case" not in item:
            errors.append(f"manifest_case_invalid:{i}")
            continue
        rid = normalize_rid(str(item["requirement"]))
        case = str(item["case"]).strip()
        if not rid or not case:
            errors.append(f"manifest_case_empty:{i}")
            continue
        cases.append((rid, case))

    gaps: list = []
    for i, item in enumerate(data.get("gaps") or []):
        if not isinstance(item, dict) or "case" not in item:
            errors.append(f"manifest_gap_invalid:{i}")
            continue
        rid = normalize_rid(str(item.get("requirement", "")))
        case = str(item["case"]).strip()
        reason = str(item.get("reason", "")).strip()
        if not case:
            errors.append(f"manifest_gap_empty:{i}")
            continue
        gaps.append((rid, case, reason))

    seen = set()
    for rid, case in cases:
        if (rid, case) in seen:
            errors.append(f"manifest_case_duplicate:{rid}:{case}")
        seen.add((rid, case))
    for rid, case, _ in gaps:
        if (rid, case) in seen:
            errors.append(f"manifest_case_in_both_sets:{rid}:{case}")

    return cases, gaps, errors


def parse_log(text: str) -> tuple[list, list, list, list]:
    """Return (case_records, gap_records, observations, parse_errors)."""
    records: list = []
    gaps: list = []
    observations = 0
    errors: list = []

    for idx, raw in enumerate(text.splitlines(), start=1):
        line = clean_line(raw).strip()
        if not line:
            continue
        if line.startswith(OBS_PREFIX):
            observations += 1
            continue
        if line.startswith(GAP_PREFIX):
            rest = line[len(GAP_PREFIX):].strip()
            parts = rest.split(":", 2)
            if len(parts) < 3 or not parts[0].strip() or not parts[1].strip():
                errors.append(f"malformed_line:{idx}")
                continue
            gaps.append((normalize_rid(parts[0]), parts[1].strip(), parts[2].strip()))
            continue
        if not line.startswith(PREFIX):
            continue
        rest = line[len(PREFIX):].strip()
        parts = rest.split(":", 3)
        if len(parts) < 3:
            errors.append(f"malformed_line:{idx}")
            continue
        rid, case, status = parts[0].strip(), parts[1].strip(), parts[2].strip().upper()
        detail = parts[3].strip() if len(parts) > 3 else ""
        if not rid or not case:
            errors.append(f"malformed_line:{idx}")
            continue
        if status not in STATUSES:
            errors.append(f"malformed_line:{idx}")
            continue
        records.append(CaseRecord(normalize_rid(rid), case, status, detail, idx))

    return records, gaps, observations, errors


def verify(log_text: str, cases: list, gaps: list) -> Result:
    res = Result()
    res.required_total = len(cases)

    records, gap_records, observations, parse_errors = parse_log(log_text)
    res.observations = observations
    res.cases = records
    res.gaps = gap_records
    res.reasons.extend(parse_errors)

    required = {(rid, case) for rid, case in cases}
    declared_gaps = {(rid, case) for rid, case, _ in gaps}

    if not log_text.strip():
        res.reasons.append("empty_log")

    seen: dict = {}
    for rec in records:
        key = (rec.requirement, rec.case)
        if key in seen:
            res.reasons.append(f"duplicate_case_line:{rec.requirement}:{rec.case}")
            continue
        seen[key] = rec
        if rec.status == "FAIL":
            res.reasons.append(f"fail_case:{rec.requirement}:{rec.case}:{rec.detail}")
        elif rec.status == "SKIP":
            res.reasons.append(f"skip_case:{rec.requirement}:{rec.case}:{rec.detail}")
        if key in declared_gaps:
            res.reasons.append(f"gap_case_reported_as_covered:{rec.requirement}:{rec.case}")
        elif key not in required:
            res.reasons.append(f"unexpected_case:{rec.requirement}:{rec.case}")

    for rid, case in cases:
        rec = seen.get((rid, case))
        if rec is None:
            res.reasons.append(f"missing_case:{rid}:{case}")
        elif rec.status == "PASS":
            res.required_pass += 1

    gap_seen: dict = {}
    for rid, case, _ in gap_records:
        key = (rid, case)
        if key in gap_seen:
            res.reasons.append(f"duplicate_gap_line:{rid}:{case}")
            continue
        gap_seen[key] = True
        if key not in declared_gaps:
            res.reasons.append(f"undeclared_gap_case:{rid}:{case}")

    if ("RENODE_TEST: PASS" in log_text or "RENODE_TEST:PASS" in log_text) and not records:
        res.reasons.append("aggregate_marker_only")

    # totals for the summary
    for rec in records:
        res.status_totals[rec.status] = res.status_totals.get(rec.status, 0) + 1
        req = res.per_requirement.setdefault(rec.requirement, {})
        req[rec.status] = req.get(rec.status, 0) + 1

    res.reasons = sorted(set(res.reasons))
    res.ok = not res.reasons and res.required_pass == res.required_total and res.required_total > 0
    if res.required_total == 0:
        res.reasons.append("manifest_no_cases")
        res.ok = False
    return res


def print_summary(res: Result) -> None:
    print("=" * 72)
    print("frick-fsw target requirement evidence (tools/scripts/target_req_verify.py)")
    print("=" * 72)
    print("Status totals:")
    for status in STATUSES:
        print(f"  {status:5s} {res.status_totals.get(status, 0)}")
    print(f"  OBS   {res.observations} (observations, never coverage)")
    print(f"  GAP   {len(res.gaps)} (declared gaps, never coverage)")
    print("Per requirement:")
    for rid in sorted(res.per_requirement):
        counts = res.per_requirement[rid]
        text = " ".join(f"{k}={v}" for k, v in sorted(counts.items()))
        print(f"  {rid}: {text}")
    if res.reasons:
        print("Reasons:")
        for reason in res.reasons:
            print(f"  E: {reason}")
    print("-" * 72)
    print(f"COVERAGE: {res.required_pass}/{res.required_total} required cases")
    print("target-evidence: PASS" if res.ok else "target-evidence: FAIL")


def main(argv: list) -> int:
    parser = argparse.ArgumentParser(description="Verify requirement-keyed target evidence")
    parser.add_argument("--log", required=True, help="console log captured from the target run")
    parser.add_argument("--manifest", required=True, help="required-case manifest (JSON)")
    parser.add_argument("--json", dest="json_out", default=None, help="also write the summary as JSON")
    args = parser.parse_args(argv)

    try:
        with open(args.manifest, "r", encoding="utf-8", errors="replace") as handle:
            manifest_text = handle.read()
        with open(args.log, "r", encoding="utf-8", errors="replace") as handle:
            log_text = handle.read()
    except OSError as exc:
        print(f"[target_req_verify] usage error: {exc}", file=sys.stderr)
        return 2

    cases, gaps, m_errors = parse_manifest(manifest_text)
    res = verify(log_text, cases, gaps)
    res.reasons.extend(m_errors)
    res.reasons = sorted(set(res.reasons))
    if m_errors:
        res.ok = False

    print_summary(res)

    if args.json_out:
        payload = {
            "ok": res.ok,
            "required_total": res.required_total,
            "required_pass": res.required_pass,
            "status_totals": res.status_totals,
            "per_requirement": res.per_requirement,
            "observations": res.observations,
            "gaps": [{"requirement": r, "case": c, "reason": why} for r, c, why in res.gaps],
            "cases": [
                {
                    "requirement": rec.requirement,
                    "case": rec.case,
                    "status": rec.status,
                    "detail": rec.detail,
                    "line": rec.line_no,
                }
                for rec in res.cases
            ],
            "reasons": res.reasons,
        }
        try:
            with open(args.json_out, "w", encoding="utf-8") as handle:
                json.dump(payload, handle, indent=2, sort_keys=True)
                handle.write("\n")
        except OSError as exc:
            print(f"[target_req_verify] cannot write {args.json_out}: {exc}", file=sys.stderr)
            return 2

    return 0 if res.ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
