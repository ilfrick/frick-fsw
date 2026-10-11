#!/usr/bin/env python3
"""frick-fsw traceability convergence gate (norms N1/N3/N4/N10/N11).

Validates docs/requirements/traceability.yaml as the single source of truth:

  * every requirement id appears in the matching document (SRS.md / SSS.md) and
    vice versa (norm N3/N4);
  * every srs entry points to a parent sss entry, and every sss entry has at
    least one child (bidirectional hierarchy);
  * non-derived requirements have implementation, tests/CI evidence and either
    Lean theorems or an explicit lean_note; derived/gap items carry a
    justification (norm N4);
  * implementation and test paths exist; `ci:<name>` evidence is claimed by a
    CI workflow `# evidence:` comment (the marker may write `ci-<name>` or
    `<name>`, several names per line) (norm N6/N10);
  * Lean theorem names are resolvable in verification/lean (norm N1);
  * GAP-* items appear in docs/quality/class-a-gaps.md (norm N5).

Exit code 0 = converged, 1 = gaps to resolve (blocks commit/push).

Self-contained: parses the restricted YAML subset used by the manifest, so it
runs without third-party packages.
"""
from __future__ import annotations

import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

MANIFEST_DEFAULT = "docs/requirements/traceability.yaml"
DOCS = {"srs": "docs/requirements/SRS.md", "sss": "docs/requirements/SSS.md"}
GAP_DOC = "docs/quality/class-a-gaps.md"
LEAN_GLOB = "verification/lean/**/*.lean"
WORKFLOWS = [".github/workflows/ci.yml", ".gitea/workflows/ci.yml"]

ID_RE = re.compile(r"^(?:FRICK-)?((?:SRS|SSS)-REQ-\d+|GAP-(?:REQ-\d+|A\d+))$")
DOC_ID_RE = re.compile(r"(?:FRICK-)?((?:SRS|SSS)-REQ-\d+)")
THEOREM_RE = re.compile(r"^theorem\s+([A-Za-z0-9_]+)", re.MULTILINE)


@dataclass
class Entry:
    id: str
    level: str
    parent_sss: str = ""
    derived: bool = False
    gap: bool = False
    justification: str = ""
    implementation: list = field(default_factory=list)
    tests: list = field(default_factory=list)
    lean: list = field(default_factory=list)
    lean_note: str = ""
    note: str = ""


def parse_flow_list(value: str) -> list:
    """Parse a YAML flow list `["a", "b"]`."""
    value = value.strip()
    if value.startswith("[") and value.endswith("]"):
        inner = value[1:-1]
        # split on commas that are not inside quotes
        parts, buf, quote = [], [], None
        for ch in inner:
            if quote:
                buf.append(ch)
                if ch == quote:
                    quote = None
            elif ch in ('"', "'"):
                quote = ch
                buf.append(ch)
            elif ch == ",":
                parts.append("".join(buf).strip().strip('"').strip("'"))
                buf = []
            else:
                buf.append(ch)
        if "".join(buf).strip():
            parts.append("".join(buf).strip().strip('"').strip("'"))
        return [p for p in (x.strip() for x in parts) if p]
    return []


def parse_scalar(value: str) -> str:
    value = value.strip()
    if value.startswith('"') and value.endswith('"'):
        return value[1:-1]
    if value.startswith("'") and value.endswith("'"):
        return value[1:-1]
    return value


def parse_manifest(text: str) -> list:
    """Parse the restricted manifest subset into Entry objects."""
    entries: list[Entry] = []
    current: Entry | None = None
    is_req_section = False
    for raw in text.splitlines():
        line = raw if isinstance(raw, str) else ""
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped == "requirements:":
            is_req_section = True
            continue
        if not is_req_section:
            continue
        if stripped.startswith("schema_version"):
            continue
        indent = len(line) - len(line.lstrip())
        if indent == 2 and stripped.startswith("- id:"):
            if current:
                entries.append(current)
            rid = parse_scalar(stripped[len("- id:"):].strip())
            current = Entry(id=rid, level="")
            continue
        if indent == 4 and current is not None and ":" in stripped:
            key, _, val = stripped.partition(":")
            key = key.strip()
            val = val.strip()
            if not val:
                continue
            if val.startswith("["):
                lst = parse_flow_list(val)
                setattr(current, key, lst)
            else:
                scalar = parse_scalar(val)
                if key in ("derived", "gap"):
                    setattr(current, key, scalar.lower() == "true")
                else:
                    setattr(current, key, scalar)
    if current:
        entries.append(current)
    return entries


def canonical_rid(rid: str) -> str:
    return re.sub(r"^FRICK-", "", rid)


def doc_ids(path: Path) -> set:
    if not path.exists():
        return set()
    text = path.read_text(encoding="utf-8")
    return {canonical_rid(m) for m in DOC_ID_RE.findall(text)}


def lean_theorems(root: Path) -> set:
    found: set = set()
    for p in sorted(root.glob(LEAN_GLOB)):
        try:
            text = p.read_text(encoding="utf-8")
        except Exception:
            continue
        for m in THEOREM_RE.finditer(text):
            found.add(m.group(1))
    return found


def ci_evidence(root: Path) -> set:
    """Collect CI evidence names claimed by workflow `# evidence:` comments.

    A marker line lists one or more claim names, optionally followed by a
    parenthetical reference list, e.g.

        # evidence: ci-host-build, ci-dual-build (SRS-REQ-016)

    Names are accepted with either the `ci-` / `ci:` prefix or bare, so the
    manifest's `ci:<name>` items match the workflow markers.
    """
    claims: set = set()
    for wf in WORKFLOWS:
        p = root / wf
        if not p.exists():
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        for m in re.finditer(r"#\s*evidence:\s*(.+)", text):
            for tok in re.findall(r"[A-Za-z0-9_\-]+", m.group(1)):
                # only explicit `ci:<name>` / `ci-<name>` tokens claim evidence;
                # requirement references in the marker comment must not count
                # (R2 M2-03 re-qualification).
                if tok.startswith("ci:") or tok.startswith("ci-"):
                    claims.add(tok[3:])
    return claims


def check_path(value: str, root: Path) -> bool:
    if value.startswith("ci:"):
        return True
    return (root / value).exists()


def main() -> int:
    root = Path.cwd()
    manifest = Path(sys.argv[1]) if len(sys.argv) > 1 else root / MANIFEST_DEFAULT
    if not manifest.is_absolute():
        manifest = root / manifest
    if not manifest.exists():
        print(f"[trace] FATAL: manifest not found: {manifest}")
        return 1
    if Path.cwd() != root:
        # caller may run from repo root; anchor root at the manifest's repo.
        root = manifest.parents[1]

    entries = parse_manifest(manifest.read_text(encoding="utf-8"))
    if not entries:
        print("[trace] FATAL: no requirements parsed from manifest")
        return 1

    by_id = {e.id: e for e in entries}
    cans = {canonical_rid(e.id): e for e in entries}
    theorems = lean_theorems(root)
    claims = ci_evidence(root)

    errors: list[str] = []
    warnings: list[str] = []

    # ---- id sanity
    for e in entries:
        m = ID_RE.match(e.id)
        if not m:
            errors.append(f"{e.id}: malformed id (expected SRS/SSS/GAP-REQ-<n>)")
            continue
        kind = m.group(1).split("-")[0]
        if kind == "GAP" and e.level != "gap":
            errors.append(f"{e.id}: level must be 'gap'")
        if kind != "GAP" and e.level == "gap":
            errors.append(f"{e.id}: level 'gap' only for GAP ids")

    # ---- top-down / bottom-up document presence (norm N3/N4)
    srs_doc = doc_ids(root / DOCS["srs"])
    sss_doc = doc_ids(root / DOCS["sss"])
    srs_manual_ids = {
        canonical_rid(e.id) for e in entries if e.level == "srs"
    }
    sss_manual_ids = {
        canonical_rid(e.id) for e in entries if e.level == "sss"
    }
    gap_ids = [e.id for e in entries if e.level == "gap"]

    for rid in sorted(srs_manual_ids - srs_doc):
        errors.append(f"{rid}: in manifest but missing from {DOCS['srs']}")
    for rid in sorted(srs_doc - srs_manual_ids):
        errors.append(f"{rid}: in {DOCS['srs']} but missing from manifest")
    for rid in sorted(sss_manual_ids - sss_doc):
        errors.append(f"{rid}: in manifest but missing from {DOCS['sss']}")
    for rid in sorted(sss_doc - sss_manual_ids):
        errors.append(f"{rid}: in {DOCS['sss']} but missing from manifest")

    # ---- hierarchy (norm N4)
    sss_children: dict[str, int] = {}
    for e in entries:
        if e.level == "srs":
            parent = e.parent_sss
            if not parent:
                errors.append(f"{e.id}: srs entry has no parent_sss")
            elif parent not in by_id:
                errors.append(f"{e.id}: parent_sss {parent} not declared")
            elif by_id[parent].level != "sss":
                errors.append(f"{e.id}: parent_sss {parent} is not level sss")
            else:
                sss_children[parent] = sss_children.get(parent, 0) + 1
    for e in entries:
        if e.level == "sss" and sss_children.get(e.id, 0) == 0:
            errors.append(f"{e.id}: sss entry with no srs child (bottom-up gap)")

    # ---- per-entry completeness
    for e in entries:
        if e.level == "gap":
            if not e.justification:
                errors.append(f"{e.id}: gap entry requires justification")
            continue
        if e.derived:
            if not e.justification:
                errors.append(f"{e.id}: derived entry requires justification")
            if not e.implementation:
                errors.append(f"{e.id}: derived entry requires implementation")
            continue
        # non-derived
        if not e.implementation:
            errors.append(f"{e.id}: missing implementation")
        for impl in e.implementation:
            if not check_path(impl, root):
                errors.append(f"{e.id}: implementation path not found: {impl}")
        if not e.tests:
            errors.append(f"{e.id}: missing tests/CI evidence (norm N6)")
        for t in e.tests:
            if t.startswith("ci:"):
                if t[3:] not in claims:
                    errors.append(
                        f"{e.id}: CI evidence '{t}' not claimed by any workflow"
                        f" (add '# evidence: {t[3:]}' to a workflow)")
            elif not (root / t).exists():
                errors.append(f"{e.id}: test path not found: {t}")
        if not e.lean and not e.lean_note:
            errors.append(
                f"{e.id}: requires Lean theorems or an explicit lean_note"
                " (norm N1/N3)")
        for th in e.lean:
            if th not in theorems:
                errors.append(f"{e.id}: Lean theorem not found: {th}")

    # ---- gaps doc (norm N5)
    gap_path = root / GAP_DOC
    if gap_ids and not gap_path.exists():
        errors.append(f"{GAP_DOC}: missing (GAP items documented, norm N5)")
    elif gap_ids:
        gap_text = gap_path.read_text(encoding="utf-8", errors="replace")
        for g in gap_ids:
            if g not in gap_text:
                errors.append(f"{g}: not referenced in {GAP_DOC}")

    # ---- unlinked Lean req theorems (bottom-up, norm N3)
    linked = {th for e in entries for th in e.lean}
    for th in sorted(theorems):
        if th.startswith("req_") and th not in linked:
            warnings.append(
                f"Lean theorem {th} not linked by any requirement (add a lean link or drop it)")

    # ---- report
    print("=" * 78)
    print("frick-fsw traceability convergence (tools/scripts/trace.py)")
    print("=" * 78)
    print(f"manifest: {manifest}")
    print(f"requirements in manifest: {len(entries)}")
    print(f"  srs={len(srs_manual_ids)} sss={len(sss_manual_ids)} "
          f"gap={len(gap_ids)}")
    print(f"lean theorems found: {len(theorems)}")
    print(f"CI evidence claims: {len(claims)}")
    print()
    print("Hierarchy (bottom-up summary):")
    for e in sorted(entries, key=lambda x: x.id):
        if e.level == "sss":
            kids = [c for c in entries if c.level == "srs" and c.parent_sss == e.id]
            print(f"  {e.id:24s} <- {len(kids):2d} srs children")
    print()
    if warnings:
        print("Warnings:")
        for w in warnings:
            print(f"  W: {w}")
        print()
    if errors:
        print("GAPS FOUND (must be resolved before commit/push):")
        for err in sorted(errors):
            print(f"  E: {err}")
        print("-" * 78)
        print(f"traceability: FAIL  ({len(errors)} errors, {len(warnings)} warnings)")
        return 1
    print(f"traceability: PASS  ({len(entries)} requirements, "
          f"{len(srs_manual_ids)} srs, {len(sss_manual_ids)} sss, "
          f"{len(gap_ids)} gaps, {len(theorems)} lean theorems checked)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
