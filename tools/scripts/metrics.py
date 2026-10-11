#!/usr/bin/env python3
"""frick-fsw - single status panel: completion, coverage and V&V traffic lights.

Computes, from the traceability manifest, docs/status/status.json, the sprint
plan, the review log, the SDD and the coverage report:
  * completion metrics: sprints, SSS requirements satisfied, SRS requirements
    delivered, design traceability, review streams closed;
  * coverage: manifest counts, verification evidence, Lean coverage, code
    statement/function coverage;
  * the review/V&V semaphores.
It then renders (and can update) the ONE panel in README.md between the
METRICS-BEGIN/END markers.

Usage:
  python3 tools/scripts/metrics.py            # print panel to stdout
  python3 tools/scripts/metrics.py --readme   # update README.md in place

Definitions (also printed in the panel note): a requirement is **implemented**
when its implementation list has a non-`docs/` artefact; **verified** when its
evidence list is non-empty and not exactly `ci:docs`; **delivered** when both
hold. An SSS requirement is **satisfied** when all its child SRS are delivered
(else when it is itself delivered). **Design traceability** counts the SRS
requirements referenced in the SDD. **Review streams** group the review log by
milestone/PA token (latest verdict wins).

Qualified as V&V tool (norm N10, register T13); any behaviour change goes
through the full cycle (norm N15).
"""
import argparse
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MANIFEST = os.path.join(ROOT, "docs", "requirements", "traceability.yaml")
STATUS = os.path.join(ROOT, "docs", "status", "status.json")
COVERAGE = os.path.join(ROOT, "build", "coverage", "coverage.json")
README = os.path.join(ROOT, "README.md")
DESIGN = os.path.join(ROOT, "docs", "design", "software-design.md")
REVIEWS = os.path.join(ROOT, "docs", "reviews", "README.md")

MARK_BEGIN = "<!-- METRICS-BEGIN -->"
MARK_END = "<!-- METRICS-END -->"

GREEN, YELLOW, RED = "green", "yellow", "red"
STATE_ICON = {GREEN: "\U0001F7E2 (GREEN)", YELLOW: "\U0001F7E1 (YELLOW)",
              RED: "\U0001F534 (RED)"}

LIST_FIELDS = ("tests", "lean", "implementation")


def _split_list(rest):
    """Parse an inline YAML flow list or a bare scalar into a list of strings."""
    rest = rest.strip()
    if rest.startswith("[") and rest.endswith("]"):
        inner = rest[1:-1].strip()
        if not inner:
            return []
        toks = re.findall(r'"[^"]*"|\'[^\']*\'|[^,]+', inner)
        return [t.strip().strip('"').strip("'") for t in toks if t.strip()]
    return [rest.strip('"').strip("'")] if rest else []


def parse_manifest(path):
    """Parse docs/requirements/traceability.yaml items into dicts with lists."""
    items = []
    cur = None
    field = None
    lines = open(path, encoding="utf-8").read().splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        m = re.match(r"^  - id: (\S+)$", line)
        if m:
            if cur:
                items.append(cur)
            cur = {"id": m.group(1), "level": "", "parent_sss": "",
                   "implementation": [], "tests": [], "lean": []}
            field = None
            i += 1
            continue
        if cur is not None:
            m = re.match(r"^    ([a-z_]+):(.*)$", line)
            if m:
                field = m.group(1)
                rest = m.group(2).strip()
                if field in LIST_FIELDS:
                    if rest:
                        cur[field] = _split_list(rest)
                    else:
                        cont = []
                        j = i + 1
                        while j < len(lines) and re.match(r"^      - ", lines[j]):
                            cont.append(lines[j].split("-", 1)[1].strip().strip('"').strip("'"))
                            j += 1
                        cur[field] = cont
                        i = j - 1
                else:
                    cur[field] = rest.strip('"').strip("'")
                    field = None
        i += 1
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


def _is_real_impl(paths):
    return any(not p.startswith("docs/") and not p.startswith("ci:") for p in paths)


def _is_verified(tests):
    return bool(tests) and tests != ["ci:docs"]


def _review_keys(line):
    m = re.search(r"\bM\d+\b", line)
    if m:
        return m.group(0)
    m = re.search(r"\bN\d+\b", line)
    if m:
        return m.group(0)
    if line.strip().upper().startswith("PA"):
        return "PA"
    base = re.sub(r"\b(delta|final|closure|re-review|review|round)\b", "", line.lower())
    return re.sub(r"[^a-z0-9]+", " ", base).strip() or "other"


def review_streams(path):
    """Group the review log into scope streams; return (closed, total)."""
    recs = []
    for order, line in enumerate(open(path, encoding="utf-8")):
        if not line.startswith("| 20"):
            continue
        cols = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cols) < 4:
            continue
        scope, concl = cols[1], cols[3].upper()
        if re.search(r"\bPA\b", scope) or scope.strip().upper().startswith("PA"):
            key = "PA"
        else:
            key = _review_keys(scope)
        recs.append((order, key, concl))
    keys = [r[1] for r in recs]
    _TOKEN = re.compile(r"^(M\d+|N\d+|PA)$")

    def canon(k):
        if _TOKEN.match(k):
            return k  # milestone / PA / N tokens are exact (no M1/M10 over-merge)
        cands = [x for x in keys if not _TOKEN.match(x)
                 and (x.startswith(k) or k.startswith(x))]
        return min(cands, key=len) if cands else k

    streams = {}
    for _, k, c in recs:
        streams[canon(k)] = c  # latest row wins
    closed = sum(1 for c in streams.values()
                 if "APPROVED" in c or "CLOSED" in c or "PASS" in c)
    return closed, len(streams)


def compute(items, status, coverage):
    srs = [i for i in items if i["level"] == "srs"]
    sss = [i for i in items if i["level"] == "sss"]
    gaps = [i for i in items if i["level"] == "gap"]
    total = len(items)
    with_impl = sum(1 for i in items if i["implementation"])
    with_tests = sum(1 for i in items if i["tests"])
    with_lean = sum(1 for i in items if i["lean"])

    delivered = {i["id"]: (_is_real_impl(i["implementation"]) and _is_verified(i["tests"]))
                 for i in srs}
    children = {}
    for i in srs:
        p = i.get("parent_sss", "")
        if p:
            children.setdefault(p, []).append(i["id"])
    sss_ok = 0
    for s in sss:
        kids = children.get(s["id"])
        if kids:
            sss_ok += 1 if all(delivered.get(k, False) for k in kids) else 0
        else:
            sss_ok += 1 if (_is_real_impl(s["implementation"]) and _is_verified(s["tests"])) else 0

    design_ok = 0
    if os.path.exists(DESIGN):
        sdd = open(DESIGN, encoding="utf-8").read()
        design_ok = sum(1 for i in srs if i["id"] in sdd)

    rev_closed, rev_total = review_streams(REVIEWS) if os.path.exists(REVIEWS) else (0, 0)
    spr = status.get("sprints", {})
    return {
        "total": total, "srs_total": len(srs), "sss_total": len(sss), "gap_total": len(gaps),
        "with_impl": with_impl, "with_tests": with_tests, "with_lean": with_lean,
        "srs_delivered": sum(1 for v in delivered.values() if v), "sss_ok": sss_ok,
        "design_ok": design_ok, "rev_closed": rev_closed, "rev_total": rev_total,
        "spr_total": spr.get("total", 0), "spr_done": spr.get("done", 0),
        "cov": coverage,
    }


def render(items, status, coverage):
    c = compute(items, status, coverage)
    gates = status["gates"]
    st = overall(gates, status["deferred"])

    pct = lambda n, d: "100%" if d and n == d else (f"{100.0 * n / d:.1f}%" if d else "n/a")

    L = []
    L.append(MARK_BEGIN)
    L.append("")
    L.append("## Status panel - completion, coverage and review/V&V traffic lights")
    L.append("")
    L.append(f"Maintained by `tools/scripts/metrics.py` (V&V tool T13, norm N8/N10); "
             f"last updated {status['meta']['updated']}. The completion and coverage "
             f"metrics are generated from `docs/status/status.json`, the traceability "
             f"manifest, the sprint plan, the review log and the coverage report, and are **refreshed every cycle** "
             f"(norms N8/N11/N19). Single source of truth: "
             f"`docs/requirements/traceability.yaml` (mirrored in `traceability.md`).")
    L.append("")
    L.append("Legend: **" + STATE_ICON[GREEN] + "** all pass - **"
             + STATE_ICON[YELLOW] + "** issues deferred by the user - **"
             + STATE_ICON[RED] + "** failures.")
    L.append("")
    L.append("### Completion")
    L.append("")
    L.append("| Metric | Value | Definition |")
    L.append("| ------ | ----- | ---------- |")
    L.append(f"| Sprints completed | {c['spr_done']}/{c['spr_total']} "
             f"({pct(c['spr_done'], c['spr_total'])}) | `docs/status/status.json` + `docs/planning/sprint-plan.md` (FRICK-PLAN-0002) |")
    L.append(f"| SSS requirements satisfied | {c['sss_ok']}/{c['sss_total']} "
             f"({pct(c['sss_ok'], c['sss_total'])}) | all child SRS delivered (else self delivered) |")
    L.append(f"| SRS requirements delivered | {c['srs_delivered']}/{c['srs_total']} "
             f"({pct(c['srs_delivered'], c['srs_total'])}) | implemented (non-doc) + verified (non-`ci:docs`) |")
    L.append(f"| Design traceability | {c['design_ok']}/{c['srs_total']} "
             f"({pct(c['design_ok'], c['srs_total'])}) | SRS referenced in the SDD requirements->design matrix (ECSS design DRD Annex F, SDD section 6) |")
    L.append(f"| Review streams closed | {c['rev_closed']}/{c['rev_total']} "
             f"({pct(c['rev_closed'], c['rev_total'])}) | review-log streams by milestone/PA, latest verdict APPROVED/CLOSED/PASS |")
    L.append("")
    L.append("### Coverage")
    L.append("")
    L.append("| Metric | Value | Definition |")
    L.append("| ------ | ----- | ---------- |")
    L.append(f"| Requirements in manifest | {c['total']} (srs={c['srs_total']}, "
             f"sss={c['sss_total']}, gap={c['gap_total']}) | `traceability.yaml` |")
    L.append(f"| Requirements with implementation | {c['with_impl']}/{c['total']} "
             f"({pct(c['with_impl'], c['total'])}) | manifest implementation list non-empty (incl. `docs/`) |")
    L.append(f"| Requirements with test/CI evidence | {c['with_tests']}/{c['total']} "
             f"({pct(c['with_tests'], c['total'])}) | manifest tests list non-empty (incl. planned `ci:docs`) |")
    L.append(f"| Formal verification coverage (Lean) | {c['with_lean']}/{c['total']} "
             f"({pct(c['with_lean'], c['total'])}) | manifest Lean link non-empty (norm N1) |")
    if coverage:
        L.append(f"| Code statement (line) coverage | {coverage['lines_covered']}/{coverage['lines_total']} "
                 f"({pct(coverage['lines_covered'], coverage['lines_total'])}) | gcov |")
        L.append(f"| Code function coverage | {coverage.get('funcs_covered', 'n/a')}/{coverage.get('funcs_total', 'n/a')} "
                 f"({pct(coverage.get('funcs_covered', 0), coverage.get('funcs_total', 0))}) | gcov |")
    else:
        L.append("| Code statement (line) coverage | not measured | run `make coverage` |")
    L.append("| Branch / decision (MC-DC) coverage | not measured | **GAP-A4** (certified checker / WCET) |")
    L.append("")
    L.append("### Semaphores")
    L.append("")
    L.append("| Gate | State |")
    L.append("| ---- | ----- |")
    for name, state in gates.items():
        L.append(f"| {name.replace('_', ' ')} | {STATE_ICON[state]} |")
    L.append("")
    L.append(f"**Overall: {STATE_ICON[st]}**")
    if status["deferred"]:
        L.append("")
        L.append("Issues deferred by the user:")
        for d in status["deferred"]:
            L.append(f"- {d.get('id', '')} - {d.get('title', '')}")
    else:
        L.append("")
        L.append("Issues deferred by the user: none.")
    L.append("")
    L.append("> **Metrics note.** ECSS does not fix universal numeric thresholds; the "
             "figures above are the completion/coverage indicators commonly adopted for "
             "software of this criticality, computed by `tools/scripts/metrics.py` "
             "(T13). *Requirements traceability* is mandated by ECSS-E-ST-40C 5.8.3.x as "
             "a **matrix** (Annex R applies it at criticality B; in-source requirement "
             "tagging is a permitted means, not a requirement). *Design traceability* "
             "follows the SDD DRD (Annex F <6>); the SDD carries the "
             "requirements->design matrices (SDD section 6). Common practice at "
             "criticality B is **100% statement + branch/decision coverage with justified "
             "exceptions**; the measured statement coverage (see the Coverage table "
             "above) and the absent "
             "branch/MC-DC and WCET results are tracked as **GAP-A4**. *Review streams* "
             "and *sprint* figures are process indicators, and the completion "
             "percentages are **indicators, not claims of ECSS compliance or "
             "qualification**.")
    L.append("")
    L.append(MARK_END)
    return "\n".join(L)


def _toc_cleanup(content):
    """Remove the standalone 'Requirements set (current)' section and its TOC entry."""
    content = re.sub(r"\n## Requirements set \(current\)\n\n.*?\n\n(?=## )", "\n",
                     content, count=1, flags=re.S)
    content = re.sub(r"[ \t]*- \[Requirements set \(current\)\]\(#[^)]*\)\n", "",
                     content, count=1)
    return content


def _normalize(panel):
    """Drop the cross-environment code-coverage rows before comparison."""
    return "\n".join(l for l in panel.splitlines()
                     if "Code statement" not in l and "Code function" not in l)


def check(items, status, coverage):
    """Verify the README panel is current (ignoring code-coverage drift)."""
    content = open(README, encoding="utf-8").read()
    if MARK_BEGIN not in content or MARK_END not in content:
        print("error: README markers missing", file=sys.stderr)
        return 1
    block = content[content.index(MARK_BEGIN):content.index(MARK_END) + len(MARK_END)]
    fresh = render(items, status, coverage)
    if _normalize(block) != _normalize(fresh):
        print("error: README metrics panel is stale - run 'make metrics'",
              file=sys.stderr)
        return 1
    if block != fresh:
        print("warning: README code-coverage rows differ from this environment "
              "(cross-compiler drift); rerun 'make metrics' where the panel is "
              "generated (norm N8)")
    else:
        print("metrics panel: current")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--readme", action="store_true", help="update README.md in place")
    ap.add_argument("--check", action="store_true",
                    help="verify the README panel is current (exit 1 if stale)")
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

    if args.check:
        return check(items, status, coverage)

    panel = render(items, status, coverage)
    if args.readme:
        with open(README, encoding="utf-8") as fh:
            content = fh.read()
        if MARK_BEGIN not in content or MARK_END not in content:
            print("error: README markers missing", file=sys.stderr)
            return 1
        start = content.index(MARK_BEGIN)
        end = content.index(MARK_END) + len(MARK_END)
        content = _toc_cleanup(content[:start] + panel + content[end:])
        with open(README, "w", encoding="utf-8") as fh:
            fh.write(content)
        print(f"README.md updated ({len(items)} requirements, overall "
              f"{status and overall(status['gates'], status['deferred'])})")
    else:
        print(panel)
    return 0


if __name__ == "__main__":
    sys.exit(main())
