#!/usr/bin/env python3
"""frick-fsw - coverage and V&V status panel generator.

Computes requirement/test/Lean coverage from the traceability manifest and the
review/V&V semaphores from docs/status/status.json, then renders (and can
update) the metrics panel in README.md between the METRICS-BEGIN/END markers.

Usage:
  python3 tools/scripts/metrics.py            # print panel to stdout
  python3 tools/scripts/metrics.py --readme   # update README.md in place

If build/coverage/coverage.json exists (produced by `make coverage`) the code
line coverage is reported; otherwise the metric is shown as "not measured".

Qualified as V&V tool (norm N10); any behaviour change goes through the full
cycle (norm N15).
"""
import argparse
import collections
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MANIFEST = os.path.join(ROOT, "docs", "requirements", "traceability.yaml")
STATUS = os.path.join(ROOT, "docs", "status", "status.json")
COVERAGE = os.path.join(ROOT, "build", "coverage", "coverage.json")
README = os.path.join(ROOT, "README.md")

MARK_BEGIN = "<!-- METRICS-BEGIN -->"
MARK_END = "<!-- METRICS-END -->"

GREEN, YELLOW, RED = "green", "yellow", "red"
STATE_ICON = {GREEN: "\U0001F7E2 (GREEN)", YELLOW: "\U0001F7E1 (YELLOW)",
              RED: "\U0001F534 (RED)"}


def parse_manifest(path):
    """Minimal parser for docs/requirements/traceability.yaml items."""
    items = []
    cur = None
    field = None
    with open(path, encoding="utf-8") as fh:
        for raw in fh:
            line = raw.rstrip("\n")
            m = re.match(r"^  - id: (\S+)$", line)
            if m:
                if cur:
                    items.append(cur)
                cur = {"id": m.group(1), "level": "", "tests": 0, "lean": 0,
                       "implementation": 0}
                field = None
                continue
            if cur is None:
                continue
            m = re.match(r"^    ([a-z_]+):", line)
            if m:
                field = m.group(1)
                val = line.split(":", 1)[1].strip()
                if field in ("tests", "lean", "implementation"):
                    cur[field] = 1 if val not in ("[]", "") else 0
                else:
                    cur[field] = val
                continue
            m = re.match(r"^    - ", line)
            if m and field in ("tests", "lean", "implementation") and cur[field]:
                cur[field] += 1
    if cur:
        items.append(cur)
    return items


def read_status(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def overall(gates, deferred):
    if any(v == RED for v in gates.values()):
        return RED
    if deferred or any(v == YELLOW for v in gates.values()):
        return YELLOW
    return GREEN


def render(items, status, coverage):
    srs = [i for i in items if i["level"] == "srs"]
    sss = [i for i in items if i["level"] == "sss"]
    gaps = [i for i in items if i["level"] == "gap"]
    total = len(items)
    with_tests = sum(1 for i in items if i["tests"])
    with_lean = sum(1 for i in items if i["lean"])
    with_impl = sum(1 for i in items if i["implementation"])
    gates = status["gates"]
    st = overall(gates, status["deferred"])

    pct = lambda n, d: "100%" if d and n == d else (f"{100.0 * n / d:.1f}%" if d else "n/a")

    lines = []
    lines.append(MARK_BEGIN)
    lines.append("")
    lines.append("## Status panel - coverage and review/V&V traffic lights")
    lines.append("")
    lines.append(f"Maintained by `tools/scripts/metrics.py` (norm N8/N10); last "
                 f"updated {status['meta']['updated']}.")
    lines.append("")
    lines.append("Legend: **" + STATE_ICON[GREEN] + "** all pass - **"
                 + STATE_ICON[YELLOW] + "** issues deferred by the user - **"
                 + STATE_ICON[RED] + "** failures.")
    lines.append("")
    lines.append("### Semaphores")
    lines.append("")
    lines.append("| Gate | State |")
    lines.append("| ---- | ----- |")
    for name, state in gates.items():
        lines.append(f"| {name.replace('_', ' ')} | {STATE_ICON[state]} |")
    lines.append("")
    lines.append(f"**Overall: {STATE_ICON[st]}**")
    if status["deferred"]:
        lines.append("")
        lines.append("Issues deferred by the user:")
        for d in status["deferred"]:
            lines.append(f"- {d.get('id', '')} - {d.get('title', '')}")
    else:
        lines.append("")
        lines.append("Issues deferred by the user: none.")
    lines.append("")
    lines.append("### Coverage")
    lines.append("")
    lines.append("| Metric | Value |")
    lines.append("| ------ | ----- |")
    lines.append(f"| Requirements in manifest | {len(items)} (srs={len(srs)}, sss={len(sss)}, gap={len(gaps)}) |")
    lines.append(f"| Requirements with implementation | {with_impl}/{total} |")
    lines.append(f"| Test coverage (requirements with tests) | {with_tests}/{total} ({pct(with_tests, total)}) |")
    lines.append(f"| Formal verification coverage (requirements with Lean) | {with_lean}/{total} ({pct(with_lean, total)}) |")
    if coverage:
        lines.append(f"| Code line coverage | {coverage['lines_covered']}/{coverage['lines_total']} "
                     f"({pct(coverage['lines_covered'], coverage['lines_total'])}) - "
                     f"functions {coverage.get('funcs_covered', 'n/a')}/{coverage.get('funcs_total', 'n/a')} |")
    else:
        lines.append("| Code line coverage | not measured - run `make coverage` to generate |")
    lines.append("")
    lines.append(MARK_END)
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--readme", action="store_true", help="update README.md in place")
    args = ap.parse_args()

    if not os.path.exists(MANIFEST) or not os.path.exists(STATUS):
        print("error: manifest or status.json missing", file=sys.stderr)
        return 1
    items = parse_manifest(MANIFEST)
    status = read_status(STATUS)
    coverage = None
    if os.path.exists(COVERAGE):
        with open(COVERAGE, encoding="utf-8") as fh:
            coverage = json.load(fh)
    panel = render(items, status, coverage)

    if args.readme:
        with open(README, encoding="utf-8") as fh:
            content = fh.read()
        if MARK_BEGIN not in content or MARK_END not in content:
            print("error: README markers missing", file=sys.stderr)
            return 1
        start = content.index(MARK_BEGIN)
        end = content.index(MARK_END) + len(MARK_END)
        content = content[:start] + panel + content[end:]
        with open(README, "w", encoding="utf-8") as fh:
            fh.write(content)
        print(f"README.md updated ({len(items)} requirements, overall "
              f"{status and overall(status['gates'], status['deferred'])})")
    else:
        print(panel)
    return 0


if __name__ == "__main__":
    sys.exit(main())