# Review Report — Milestone M16 (in-house OBCP engine development): documentation delta re-review (N17/N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0073                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R4 (Senior SW documentation engineer, N17) |
| Reference revision                | `039e47f` (branch `main`) + working-tree delta (`README.md`, `docs/design/software-design.md`, `tools/scripts/metrics.py`); 2026-10-11 |
| Items under review                | Closure of the 7 Minor findings DOC-1..DOC-7 of `FRICK-REVIEW-0065` (`docs/reviews/2026-10-11-m16-docs-r4-review.md`), plus the delta fixes: `README.md` (single generated panel), `docs/requirements/traceability.md`, `docs/status/handover.md`, `docs/development-plan.md`, `docs/architecture/component-gap-analysis.md`, `docs/status/loop-audit.md`, `docs/roadmap.md`, and the new SDD §6 (`docs/design/software-design.md`) + `tools/scripts/metrics.py` |
| Date                              | 2026-10-11                             |
| Scope                             | Delta re-review (norm N12): confirm each DOC-1..7 disposition against the current working tree with file:line evidence; re-check document consistency (header `Revision` vs latest `## History`, stale counts, dangling links/TOC anchors, a single metrics panel, English-only N2, `traceability.md` mirror fidelity, milestone/execution order); re-run the CI `docs` required-document + document-id-uniqueness blocks, `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`, and the `metrics.py --check` currency gate. Excluded: code/V&V correctness (R1/R2), requirement singularity/boundedness (R3), and the concurrent PA review (N20). |
| Independence declaration          | Reviewer took no part in authoring or fixing the M16 change; this is a fresh independent session with no prior context (a re-instantiation of profile R4 under norm N9, not the author of FRICK-REVIEW-0065). Read-only review: only this report file was created; no other repository file was modified. |
| Norms applied                     | N2, N3, N4, N8, N9, N11, N12, N16, N17 (see `docs/rules.md`) |

## 1. Scope

This is the **delta re-review** of `FRICK-REVIEW-0065` (R4/N17), which recorded
7 Minor findings (DOC-1..DOC-7) and 3 Comments (DOC-8..DOC-10) on the M16
documentation addition. Norm N12 requires that non-conformances be resolved by
re-applying all rules to the affected scope and re-reviewing the deltas until
full compliance; this report does exactly that for the documentation scope.

Reviewed the current tree at `039e47f` **including the uncommitted working-tree
delta** (`git status`: `M README.md`, `M docs/design/software-design.md`,
`M tools/scripts/metrics.py`) that carries the consolidated metrics panel and the
new SDD §6 requirements->design matrix.

Verified specifically:

1. **DOC-1..DOC-7 closure** — each prior finding disposed with evidence.
2. **Header `Revision` vs latest `## History`** across the touched documents
   (`handover.md`, `traceability.md`, `development-plan.md`,
   `component-gap-analysis.md`, `loop-audit.md`, `roadmap.md`, `software-design.md`).
3. **Stale counts** — no `94`/`54`/`26` current count outside historical history rows.
4. **Links/TOC anchors** — every relative link and in-document anchor in the touched
   docs resolves; the removed `Requirements set (current)` section leaves no dangling TOC entry.
5. **Single metrics panel** — exactly one `METRICS-BEGIN/END` block, `metrics.py` stdout byte-identical.
6. **English-only (N2)**.
7. **`traceability.md` mirrors the manifest** — counts, top-down matrix, SRS/SSS rows, planned note.
8. **Gates** — CI `docs` required-document + document-id-uniqueness blocks,
   `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`,
   `python3 tools/scripts/metrics.py --check`.

Assumptions: `traceability.yaml` is the authoritative single source of truth; the
README panel between `METRICS-BEGIN/END` is `tools/scripts/metrics.py` output;
the working-tree delta is part of the change under review.

## 2. References

- Project norms: `docs/rules.md` (N1-N20), in particular N2 (English), N4
  (bidirectional traceability), N8 (complete/current docs), N11 (records committed
  each cycle), N12 (NC closure loop), N16 (revision history), N17 (this review).
- Prior review under disposition: `docs/reviews/2026-10-11-m16-docs-r4-review.md`
  (FRICK-REVIEW-0065).
- Traceability manifest: `docs/requirements/traceability.yaml`; human-readable
  mirror `docs/requirements/traceability.md` (FRICK-TRACE-001).
- Touched docs: `README.md`, `docs/status/handover.md`,
  `docs/development-plan.md`, `docs/architecture/component-gap-analysis.md`,
  `docs/status/loop-audit.md`, `docs/roadmap.md`, `docs/design/software-design.md`,
  `tools/scripts/metrics.py`.
- Standards context: ECSS-E-ST-40C (software design/DRD Annex F <6>), ECSS-Q-ST-80C,
  ECSS-E-ST-70-41C (PUS-C, S18), CCSDS 133.0-B-2.

## 3. Findings

### 3.1 Prior findings (FRICK-REVIEW-0065) disposition

| Prior # | Severity | Status | Evidence (current tree) |
| ------- | -------- | ------ | ----------------------- |
| DOC-1 | Minor | **CLOSED** | The `## Requirements set (current)` prose paragraph is removed; the only requirement-set counts now live in the generated panel. `grep -rn "Requirements set" README.md` → none; counts stated once at `README.md:61` (`96 (srs=55, sss=27, gap=14)`) and match `metrics.py`. No stale `54/26` prose remains. |
| DOC-2 | Minor | **PARTIALLY CLOSED — residual** | Counts regenerated (`traceability.md:18` = `SRS = 55, SSS = 27, GAP = 14`), new rows present (`:54` SSS-REQ-027, `:114` SRS-REQ-054, `:151` SSS-REQ-027), SSS summary completed (`:148-151`), planned note updated (`:21`). **Residuals:** (a) header `Revision 0.23` (`:7`) is not matched by any History row — the latest History rows are `0.21` (`:204`), a **duplicate `0.21`** (`:205`) and `0.22` (`:206`); (b) the §1 top-down matrix omits SRS-REQ-050 under SSS-REQ-015 (`:42`), see DR-06. |
| DOC-3 | Minor | **PARTIALLY CLOSED — residual** | M16 added to §4 (`handover.md:114`) and the sprint pointer added to §5 (`:123-124`). **Residual:** header `Revision 1.1` (`:7`) is one behind the latest History row `1.2` (`:183`), see DR-01. |
| DOC-4 | Minor | **PARTIALLY CLOSED — residual** | OBJ order fixed (DOC-8) and header bumped to `0.15` (`development-plan.md:7`). **Residual:** the History has a **duplicate `0.13`** (`:331`,`:332`) and **no `0.15` row**; the latest History is `0.14` (`:333`), see DR-03. |
| DOC-5 | Minor | **CLOSED** | `component-gap-analysis.md` header `Revision 0.2` / `Date 2026-10-11` (`:7-8`), `Requirements` field extended with SSS-REQ-027 and SRS-REQ-054 (`:9`), and History row `0.2` added (`:128`) — header matches latest History. |
| DOC-6 | Minor | **NOT CLOSED** | Baseline still `e1def89` (`loop-audit.md:9`); `git show e1def89:docs/requirements/traceability.yaml \| grep -c "^  - id:"` → **94**, while the audit reports `compliant (96 reqs)` (`:49`). Header `Revision 0.4` (`:7`) is also one behind the latest History `0.5` (`:157`). See DR-04 and DR-05. |
| DOC-7 | Minor | **CLOSED** | `roadmap.md` `## M16` moved after `## M15` (`:192` follows `:181`); the section-order defect is gone (execution order still conveyed by the in-text statement). Header `Revision 0.17` (`:5`) matches the latest History row `0.17`. |
| DOC-8 (Comment) | Comment | **CLOSED** | `development-plan.md` OBJ table now in numeric order: OBJ-22 (`:57`), OBJ-23 (`:58`), OBJ-24 (`:59`). |
| DOC-9 (Comment) | Comment | **CLOSED** | `SRS-REQ-054` method aligned to `Tst \| Dem` (`SRS.md:278`) matching `SSS-REQ-027` `Tst \| Dem` (`SSS.md:123`). |
| DOC-10 (Comment) | Comment | **OPEN (advisory)** | `OBJ-20` still reads "…fallback: an ECSS-compliant engine…" (`development-plan.md:55`) and the loop-audit M12 row still reads "n7s-obcp / ECSS engine" (`loop-audit.md:35`), i.e. the M12/M16 reframe is still not propagated there. No derogation; remains advisory. |

### 3.2 New findings (introduced/remaining in the delta)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| DR-01 | Minor | `docs/status/handover.md` (N16) | **Header `Revision` behind the History.** The M16 change added History row `1.2` but bumped the header only to `1.1`, so the authoritative resume point's header revision does not evidence its latest change. | `handover.md:7` (`Revision 1.1`) vs `:183` (latest row `1.2`). | Set the header to `1.2` (or add the M16 row at `1.1` and renumber), so header == latest History. |
| DR-02 | Minor | `docs/requirements/traceability.md` (N16) | **Header `Revision` mismatched and duplicate revision number.** Header is `0.23` but the History has no `0.23` row; there are two rows both labelled `0.21`. | `traceability.md:7` (`Revision 0.23`); `:204` and `:205` (both `0.21`); `:206` (`0.22`). | Renumber: the M16 regeneration row should be `0.23` (distinct from the `0.21`/`0.22` M11 rows); keep revision numbers unique and header == latest row. |
| DR-03 | Minor | `docs/development-plan.md` (N16) | **Header `Revision` beyond the History and duplicate revision number.** Header is `0.15` but the latest History row is `0.14`; the M16 row was inserted with the already-used label `0.13`. | `development-plan.md:7` (`Revision 0.15`); `:331` and `:332` (both `0.13`); `:333` (`0.14`). | Renumber the M16 row to `0.15` (unique) and keep header == latest History. |
| DR-04 | Minor | `docs/status/loop-audit.md` (N16) | **Header `Revision` behind the History.** Header remains `0.4` while the latest History row is `0.5`. | `loop-audit.md:7` (`Revision 0.4`) vs `:157` (latest row `0.5`). | Bump the header to `0.5`. |
| DR-05 | Minor | `docs/status/loop-audit.md` (N19/N8) | **Audit baseline not reconciled with its own counts** (prior DOC-6, unchanged). Baseline `e1def89` corresponds to 94 requirements, but the audit reports `compliant (96 reqs)`; the numbers are still not tied to the revision they claim. | `loop-audit.md:9` (`Baseline commit e1def89`); `git show e1def89:docs/requirements/traceability.yaml` count = **94**; `:49` (`96 reqs`); `trace.py` → 96/55/27/14. | Re-baseline the audit at the current audited revision (e.g. `039e47f`) or annotate the counts row as "counts at `<named revision>`"; make baseline and audited numbers agree. |
| DR-06 | Minor | `docs/requirements/traceability.md` §1 (N4/N8) | **Human mirror inconsistent with the manifest in the top-down matrix.** SSS-REQ-015 lists only `SRS-REQ-018`, but the manifest gives SSS-REQ-015 two children: SRS-REQ-018 **and SRS-REQ-050**. The mirror's §1 therefore enumerates only 54 of 55 SRS while its own count line says 55. | `traceability.md:42` (`\| SSS-REQ-015 (PUS backends) \| SRS-REQ-018 \|`); `traceability.yaml:471-473` (`SRS-REQ-050` … `parent_sss: SSS-REQ-015`); `trace.py` → `SSS-REQ-015 <- 2 srs children`. | Add `SRS-REQ-050` to the SSS-REQ-015 row of §1. (Pre-existing, but the M16 regeneration was the occasion to make the mirror faithful.) |
| DR-C1 | Comment | `docs/status/handover.md` (N8) | The resume point's §4 is anchored "as of commit `967f563`" while the tree is at `039e47f`; the resume point should reflect the current head. | `handover.md:87` (`Current state (as of commit 967f563…)`). | Refresh the "as of commit" marker/state to the current head. |
| DR-C2 | Comment | M12 reframe propagation | Same residual as prior DOC-10: `OBJ-20` and the loop-audit M12 row still use the old "fallback: an ECSS-compliant engine" phrasing rather than "integrate an engine (M12) + in-house engine (M16)". | `development-plan.md:55`; `loop-audit.md:35`. | Reframe consistently with `roadmap.md:164-182` / `README.md:142`. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.3 Verification performed (evidence)

| Check | Result |
| ----- | ------ |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — `96 requirements, 55 srs, 27 sss, 14 gaps, 51 lean theorems` (EXIT 0). |
| CI `docs` required-document block (`.github/workflows/ci.yml:152-197`) | **PASS** — no missing documents. |
| CI `docs` document-id-uniqueness block | **PASS** — `documentation set: OK (123 unique document ids)`, no duplicates. |
| `python3 tools/scripts/metrics.py --check` | **PASS** — `metrics panel: current` (EXIT 0). |
| `python3 tools/scripts/metrics.py` vs README panel | **BYTE-IDENTICAL** (`README.md:39-94`), including `Requirements in manifest 96 (srs=55, sss=27, gap=14)`, `Design traceability 55/55 (100%)`. |
| One metrics panel only | **PASS** — exactly one `METRICS-BEGIN`/`METRICS-END` in the repo (both in `README.md`); no other doc carries a panel. |
| Header `Revision` vs latest `## History` | **MATCH**: `component-gap-analysis.md` (0.2/0.2), `roadmap.md` (0.17/0.17), `software-design.md` (0.2/0.2). **MISMATCH**: `handover.md` (1.1/1.2, DR-01), `traceability.md` (0.23/0.22 + dup 0.21, DR-02), `development-plan.md` (0.15/0.14 + dup 0.13, DR-03), `loop-audit.md` (0.4/0.5, DR-04). |
| Stale current counts (`94`/`54`/`26`) in touched docs | **None** — the only `94/54/26` occurrence is the historical History row `handover.md:181` (`counts refreshed 94/54/26/14/51`), correctly historical. |
| Relative links / in-document anchors (touched docs) | **All resolve** — 0 broken links/anchors across `README.md`, `traceability.md`, `handover.md`, `development-plan.md`, `component-gap-analysis.md`, `loop-audit.md`, `roadmap.md`, `software-design.md`, `sprint-plan.md`, `CHANGELOG.md`; the removed `Requirements set (current)` TOC anchor is gone and no reference to it remains in `README.md`. |
| `traceability.md` mirror vs manifest | Counts, all 55 SRS rows (`§2`), all 27 SSS rows (`§3`), and the planned note are present; **§1 top-down omits SRS-REQ-050 under SSS-REQ-015** (DR-06). |
| SDD §6 requirements->design matrix | All **55** SRS ids referenced (matches the `55/55` panel claim); M12/M13/M14/M15/M16 planned components listed (`software-design.md:118-123`). |
| `status.json` M16 | **`"M16": "planned"`**; sprint `total 26 / done 0 / in_progress 1` consistent with the panel. |
| English-only (N2) | **OK** — non-ASCII characters are punctuation/symbols (em dash, `×`, `→`, `·`, `§`, traffic-light emoji) and the pre-existing "Eärendil" codename; all M16 content is English. |

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Graded conclusion**

- **R4 (SW documentation, N17): APPROVED WITH COMMENTS.**

The substantive M16 documentation delta is sound: the stale README prose panel is
**gone** (DOC-1 closed) and the requirement-set counts now come from a single
generated panel that is byte-identical to `metrics.py` (single panel confirmed);
the human traceability mirror has been regenerated with the new counts and the
SSS-REQ-027 / SRS-REQ-054 rows and a completed SSS summary; `component-gap-analysis.md`
(DOC-5), `roadmap.md` M16 ordering (DOC-7) and the OBJ-24 ordering (DOC-8) are
corrected and internally consistent; the new SDD §6 requirements->design matrix
covers all 55 SRS; the gates are green (`trace.py` PASS 96/55/27/14/51; CI `docs`
required + uniqueness PASS; `metrics.py --check` PASS); there are no dangling
links/anchors and no non-English content.

**However, `FRICK-REVIEW-0065` is NOT closed.** Three of its Minor findings were
only partially corrected and one is untouched, because the fixes introduced a
**systematic revision-history defect**: the header `Revision` and the latest
`## History` row disagree in `handover.md` (DR-01), `traceability.md` (DR-02),
`development-plan.md` (DR-03) and `loop-audit.md` (DR-04), and two of those files
carry a **duplicate revision number** (`0.21` ×2; `0.13` ×2), contrary to norm
N16. Separately, prior **DOC-6 remains open**: the loop-audit baseline `e1def89`
still corresponds to 94 requirements while the audit reports 96 (DR-05); and the
human mirror's §1 top-down matrix still omits SRS-REQ-050 (DR-06). These are
Minor non-conformities (no Major; none defeats a requirement or a safety barrier),
but norm N12 requires them to be corrected and re-verified before the M16
documentation loop can be closed.

Overall: **APPROVED WITH COMMENTS** (0 Major, 6 Minor, 2 Comments). Prior NCs
DOC-1, DOC-5, DOC-7 closed; DOC-2, DOC-3, DOC-4 partially closed; DOC-6 open.

**Confidence:** **High.** Every disposition is backed by direct file:line evidence
and by re-running the gates in the current tree; the one judgement call (whether a
header/History divergence is a defect) follows the convention already established
and applied in FRICK-REVIEW-0065. No paywalled or secondary sources were relied on.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | Independent documentation review agent (fresh, independent context) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| DOC-1 | Minor | `README.md` | Stale requirement-set prose counts | closed: prose panel removed; counts only in generated panel | `grep "Requirements set" README.md` → none; `README.md:61` matches `metrics.py` | frick-fsw | **closed** | FRICK-REVIEW-0073 |
| DOC-2 | Minor | `docs/requirements/traceability.md` | Human matrix not regenerated | partially closed: counts/rows/history updated, but rev numbering inconsistent | `traceability.md:7,18,54,114,151,204-206` | frick-fsw | **open** (see DR-02) | FRICK-REVIEW-0073 |
| DOC-3 | Minor | `docs/status/handover.md` | Header rev vs History; M16 not recorded | partially closed: M16 in §4/§5, header still behind History | `handover.md:7,114,123-124,183` | frick-fsw | **open** (see DR-01) | FRICK-REVIEW-0073 |
| DOC-4 | Minor | `docs/development-plan.md` | Revision history not bumped for OBJ-24/§6 | partially closed: header bumped, History row mislabelled/duplicated | `development-plan.md:7,331-333` | frick-fsw | **open** (see DR-03) | FRICK-REVIEW-0073 |
| DOC-5 | Minor | `docs/architecture/component-gap-analysis.md` | Rev/date/history/`Requirements` not updated | closed | `component-gap-analysis.md:7-9,128` | frick-fsw | **closed** | FRICK-REVIEW-0073 |
| DOC-6 | Minor | `docs/status/loop-audit.md` | Baseline `e1def89` (94 reqs) vs reported 96 | not closed | `loop-audit.md:9,49`; `git show e1def89…` → 94 | frick-fsw | **open** (see DR-05) | FRICK-REVIEW-0073 |
| DOC-7 | Minor | `docs/roadmap.md` | M16 section out of milestone order | closed | `roadmap.md:181,192`; header/History 0.17/0.17 | frick-fsw | **closed** | FRICK-REVIEW-0073 |
| DOC-8 | Comment | `docs/development-plan.md` | OBJ table out of numeric order | closed | `development-plan.md:57-59` | frick-fsw | **closed** | FRICK-REVIEW-0073 |
| DOC-9 | Comment | `SRS-REQ-054` / `SSS-REQ-027` | Method `Tst` vs `Tst \| Dem` | closed | `SRS.md:278` = `SSS.md:123` = `Tst \| Dem` | frick-fsw | **closed** | FRICK-REVIEW-0073 |
| DOC-10 | Comment | M12 reframe propagation | OBJ-20 / loop-audit M12 old framing | advisory, not addressed | `development-plan.md:55`; `loop-audit.md:35` | frick-fsw | **open** (advisory) | FRICK-REVIEW-0073 (DR-C2) |
| DR-01 | Minor | `docs/status/handover.md` | Header `1.1` ≠ latest History `1.2` | pending | `handover.md:7,183` | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-02 | Minor | `docs/requirements/traceability.md` | Header `0.23` unmatched; duplicate `0.21` | pending | `traceability.md:7,204-206` | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-03 | Minor | `docs/development-plan.md` | Header `0.15` unmatched; duplicate `0.13` | pending | `development-plan.md:7,331-333` | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-04 | Minor | `docs/status/loop-audit.md` | Header `0.4` ≠ latest History `0.5` | pending | `loop-audit.md:7,157` | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-05 | Minor | `docs/status/loop-audit.md` | Audit baseline `e1def89` (94) vs reported 96 | pending | `loop-audit.md:9,49`; manifest at `e1def89` = 94 | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-06 | Minor | `docs/requirements/traceability.md` §1 | SSS-REQ-015 row omits SRS-REQ-050 (mirror ≠ manifest) | pending | `traceability.md:42` vs `traceability.yaml:471-473`; `trace.py` → 2 children | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-C1 | Comment | `docs/status/handover.md` | "as of commit 967f563" stale | advisory | `handover.md:87` | frick-fsw | **open** | FRICK-REVIEW-0073 |
| DR-C2 | Comment | M12 reframe propagation | Same as DOC-10 | advisory | `development-plan.md:55`; `loop-audit.md:35` | frick-fsw | **open** | FRICK-REVIEW-0073 |

---

Committed and pushed with the change under re-review (norm N11). The
`docs/reviews/README.md` index row for FRICK-REVIEW-0073 is to be appended when
this report is committed, per the review-log procedure (`docs/reviews/README.md`).