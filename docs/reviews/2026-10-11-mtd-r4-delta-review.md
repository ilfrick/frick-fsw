# Review Report — Reference NOR/QSPI MTD backend (SRS-REQ-056), documentation (N17) — delta re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0097                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R4 (SW documentation)                  |
| Reference revision                | cb2576d (MTD increment: remove vacuous Renode check, commit reviews, correct QSPI claims) |
| Items under review                | corrective delta of FRICK-REVIEW-0093 over: `docs/vv/tool-qualification.md`, `docs/status/loop-audit.md`, `docs/development-plan.md`, `docs/status/handover.md`, `docs/design/software-design.md`, `docs/requirements/traceability.md`, `docs/vv/verification-results.md`, `README.md`, `docs/vv/renode-environment.md`, `CHANGELOG.md`, `docs/reviews/README.md`, `docs/requirements/traceability.yaml` |
| Date                              | 2026-10-11                             |
| Scope                             | Verification of closure of the seven Minor findings R4-93-01..07 and the four Comments R4-93-08..11 of FRICK-REVIEW-0093 at HEAD `cb2576d`; re-check of the N17 criteria: single metrics panel, header `Revision` vs `History` across the touched docs, requirement/coverage counts (98/57/27/14), stale "QSPI pending/done" contradictions, English-only (norm N2), relative links/anchors; re-run of the CI docs `required`/document-id-uniqueness block, `tools/scripts/trace.py` and `tools/scripts/metrics.py --check`. Excluded: driver/V&V correctness (R1/R2), requirement singularity (R3/N14) and the PA verdict (N20, FRICK-REVIEW-0094). |
| Independence declaration          | Independent R4 reviewer (fresh context); no part in authoring/executing the reviewed items. FRICK-REVIEW-0093 read as the finding source; the sibling reports 0091/0092/0094 were not relied on for any verdict, only to interpret the changed basis (the `hal_mtd` Renode check was removed as vacuous). |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N2, N4, N8, N11, N12, N16, N17, N19 |

## 1. Scope

FRICK-REVIEW-0093 issued **7 Minor** findings (R4-93-01..07) and **4 Comments**
(R4-93-08..11) against the Sprint 1.2 revision `a3cdd68`. The corrective commit
`cb2576d` (HEAD) was, however, primarily driven by the sibling code/V&V/PA
reviews 0091/0092/0094, whose Major/Minor finding was that the Renode `hal_mtd`
check was **vacuous** (the QSPI window at `0x21000000` is not mapped by the
loaded CPU repl) and was therefore **removed**. That changes the premise of the
R4 findings that recommended "6/13 + `hal_mtd=PASS`": with the on-SoC QSPI check
removed, MTD is **host-only** and the M11 peripheral-bound count legitimately
stays **5/13** (NVM being the fifth). This re-review therefore distinguishes
(a) findings whose requested edit is now moot (superseded) from (b) findings
whose defect persists at HEAD.

Reproduction performed on the reviewed revision (all green):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
  → `traceability: PASS (98 requirements, 57 srs, 27 sss, 14 gaps, 51 lean theorems checked)`.
- CI docs block (required-documents + document-id uniqueness, copied verbatim from
  `.github/workflows/ci.yml:159-217`) → `documentation set: OK (146 unique document ids)`.
- Reviewer-agent persona drift block → `reviewer-agent personas: bodies in sync`.
- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`.
- Single metrics panel: `grep -c METRICS-BEGIN` = 1, `grep -c METRICS-END` = 1 (`README.md:39/94`); panel body byte-identical to `metrics.py` stdout.
- Relative-link scan over the reviewed files → 0 broken links.
- English-only scan → compliant (non-ASCII limited to typographic symbols and the
  status-panel emoji/legends, both pre-existing and deliberate).

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C (software information/DRDs, incl. SRS + SDD Annex F),
  ECSS-Q-ST-80C (product assurance/data), ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Finding source: `docs/reviews/2026-10-11-mtd-r4-review.md` (FRICK-REVIEW-0093)
- Review template: `docs/reviews/review-template.md`

## 3. Findings

### 3.1 Closure of the FRICK-REVIEW-0093 findings

| # | Prior NC | Severity | Original finding | Status at `cb2576d` | Evidence |
| - | -------- | -------- | ---------------- | ------------------- | -------- |
| 1 | R4-93-01 | Minor | `tool-qualification.md` header `Revision` vs History (`0.3.10` vs `0.3.11`) | **NOT CLOSED** — the fix edited the `0.3.11` History row (dropped the `hal_mtd` claim) but did **not** advance the header; header is still `0.3.10` against a `0.3.11` History row | `docs/vv/tool-qualification.md:7` (`Revision | 0.3.10`) vs `:109` (`| 0.3.11 | ... test_mtd.c ...`); diff of `cb2576d` |
| 2 | R4-93-02 | Minor | `loop-audit.md` content changed without advancing `Revision`/`History` (N16/N19) | **NOT CLOSED** — `cb2576d` changed the file (§2 M4 row + new tracked disposition) but `Revision` is still `0.6` and the last History row is still `0.6`; no `0.7` row | `docs/status/loop-audit.md:7`, `:27`, `:149` vs last row `:159` |
| 3 | R4-93-03 | Minor | `development-plan.md` Revision/History not advanced; M11 row stale vs M4 row (N16/N8) | **NOT CLOSED** — the file was not touched by `cb2576d`; M4 still reads "QSPI/NOR MTD done" (`:295`) while M11 still reads "in progress (5/13 peripheral-bound)" with no MTD in its done/pending enumeration (`:302`), and the Revision (`0.15`) equals the last History row (`0.15`) with no MTD-progress row. The requested "6/13 + `hal_mtd`" edit is **superseded** (MTD is not Renode-verified), so the residual defect is the missing Revision/History row and the M11↔M4 wording reconciliation, not a count change | `docs/development-plan.md:7`, `:295`, `:302`; `git show --stat cb2576d` (file absent) |
| 4 | R4-93-04 | Minor | `handover.md` §4 stale vs §5/History (N8/N16) | **PARTIALLY CLOSED — remains open** — `§5` step 1 was corrected (`:130` now "NOR/QSPI MTD driver done, on-SoC QSPI check pending"), which removes the §4↔§5 MTD contradiction. But `:63` still says "unit tests (**14/14**)" (now 18/18) and `:126` still says "next classes: **NOR MTD**, CAN, ...", contradicting the corrected `:130`. §4's `5/13`/no-`hal_mtd` (`:97`,`:106`) are now defensible (on-SoC MTD check removed) | `docs/status/handover.md:63`, `:97`, `:106`, `:126` vs corrected `:130`; diff of `cb2576d` |
| 5 | R4-93-05 | Minor | `verification-results.md` "Remaining (M4)" still lists QSPI, contradicting the delivery row | **NOT CLOSED** — `:277` still lists "CAN/SPI/I2C/**QSPI**/MMC/eMMC/storage/PCIe/SpaceWire"; additionally the History row `:410` still claims "`hal_mtd=PASS` in Renode", directly contradicting the removal of that check in this same commit | `docs/vv/verification-results.md:277`, `:410` vs `:39`; diff of `cb2576d` |
| 6 | R4-93-06 | Minor | `README.md` project-scope table + focus stale (N8) | **NOT CLOSED** — `:137` still lists `QSPI/NOR` under **Planned** and omits NVM/NOR from **Implemented now**, contradicting the drivers table at `:186`; the focus sentence `:408` still reads "NVM done; NOR/CAN" | `README.md:137`, `:408` vs `:186`; diff of `cb2576d` (only `:54/:55` and `:186` changed) |
| 7 | R4-93-07 | Minor | SRS-REQ-056 not allocated in the SDD design matrix (design 56/57) | **CLOSED** — SDD §6 now lists `SRS-REQ-056` and `mtd/mpfs` (`:113`), History `0.4` added (`:151`), header `0.4`=`0.4`; README design-traceability now `57/57 (100%)` (`:54`) | `docs/design/software-design.md:113`, `:151`; `README.md:54`; `metrics.py` |
| 8 | R4-93-08 | Comment | `renode-environment.md` HAL coverage "(4/13)" stale | **NOT CLOSED** — still "(4/13)" (`:88`). Correct value now is `5/13` (NVM), with MTD recorded host-only/on-SoC-pending; the "6/13" suggestion is superseded | `docs/vv/renode-environment.md:88` |
| 9 | R4-93-09 | Comment | `traceability.md` prose "Six … documented-only" vs 52/57 | **NOT CLOSED** — prose still says "Six of the requirements (SRS-REQ-045/046/051/052/053/054 …) … documented-only", but SRS-REQ-052 carries `tests/unit/test_r3_gap.c` and a non-doc implementation, so it is not documented-only | `docs/requirements/traceability.md:20-23`; `docs/requirements/traceability.yaml:489-495` |
| 10 | R4-93-10 | Comment | `CHANGELOG.md` duplicate `### Added` in `[Unreleased]` | **NOT CLOSED** — two `### Added` headings remain | `CHANGELOG.md:10` and `:37` |
| 11 | R4-93-11 | Comment | Review-log row for FRICK-REVIEW-0093 not appended | **CLOSED** — the four MTD review rows (R1/R2/R4/PA) are now present in the log | `docs/reviews/README.md` (4 new rows added by `cb2576d`) |

### 3.2 N17 cross-cutting checks on the delta

| Check | Result |
| ----- | ------ |
| Single metrics panel | **PASS** — exactly one `METRICS-BEGIN`/`METRICS-END` pair (both in `README.md`); `metrics.py --check` → `metrics panel: current`; panel body identical to generator stdout |
| Counts consistent 98/57/27/14 | **PASS** — `trace.py` 98/57/27/14/51; README panel `98 (srs=57, sss=27, gap=14)`; `loop-audit.md:49` "(98 reqs)"; `handover.md:66` "98/57/27/14/51"; `verification-results.md:39` "98 reqs / 57 srs / 27 sss / 14 gap" (only the historical History rows and the S0 exit-criterion target carry older figures, which is expected) |
| Design traceability 57/57 | **PASS** — `README.md:54` 100%, SDD §6 covers SRS-REQ-056 |
| Header `Revision` vs `History` | **FAIL (one file)** — `tool-qualification.md` header `0.3.10` ≠ last History `0.3.11` (R4-93-01). `loop-audit.md` (0.6/0.6), `development-plan.md` (0.15/0.15) and `handover.md` (1.4/1.4) are internally equal but were changed in this loop without a new History row (R4-93-02/-03/-04); `software-design.md` (0.4/0.4, row added) and `traceability.md` (0.25/0.25) are consistent |
| Stale "QSPI pending/done" contradictions | **FAIL (residual)** — `README.md:137` (QSPI/NOR still "Planned"), `README.md:408` ("NOR/CAN" as next), `verification-results.md:277` (QSPI in Remaining) and `:410` ("`hal_mtd=PASS` in Renode") still present alongside the corrected rows |
| English-only (N2) | **PASS** — no non-English content |
| Links/anchors | **PASS** — 0 broken relative links in the reviewed set |
| CI docs required + uniqueness | **PASS** — `documentation set: OK (146 unique document ids)` |
| Persona drift (N9) | **PASS** — `reviewer-agent personas: bodies in sync` |
| Trace convergence (N4) | **PASS** — `traceability: PASS (98/57/27/14, 51 theorems)` |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Graded conclusion**

- **FRICK-REVIEW-0093 is NOT closed.** Of its 7 Minor findings, only
  **R4-93-07** (SDD design allocation / design traceability 57/57) is closed;
  R4-93-04 is partially corrected (one line) and R4-93-01/-02/-03/-05/-06 remain
  open. Of its 4 Comments, R4-93-11 is closed (review-log row) while R4-93-08/-09/-10
  remain open. The corrective commit `cb2576d` was dominated by the sibling
  R1/R2/PA findings (removal of the vacuous `hal_mtd` Renode check) and did not
  close the R4 documentation NCs; the docs in the affected scope must be
  re-corrected and re-reviewed (norm N12) before 0093 can be closed.
- **This delta re-review (R4, N17): APPROVED WITH COMMENTS.** No Major finding.
  The substantive delta is honest and directionally correct: the vacuous Renode
  QSPI claim has been removed from `README.md:186`, `CHANGELOG.md`, the
  `traceability.yaml` `lean_note`, `verification-results.md:39`, `handover.md:130`
  and `sprint-plan.md`; the SDD design matrix is completed (57/57); and the
  mandatory gates are all green at HEAD (`trace.py` PASS 98/57/27/14/51; CI docs
  required+uniqueness OK, 146 ids; `metrics.py --check` current; single metrics
  panel; no broken links; English-only). The residual findings are documentary
  non-conformities — a header/History drift, three unchanged revision lists, and
  five stale "QSPI pending/done" sentences — that must be corrected to compliance
  (N12) before the increment is closed.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) / PA (N20) | R4 (Senior SW documentation engineer) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R4-97-01 | Minor | `tool-qualification.md:7` vs `:109` (N16) — prior R4-93-01 | Header `0.3.10` ≠ last History `0.3.11` | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-02 | Minor | `loop-audit.md` (N16/N19) — prior R4-93-02 | Content changed at `:27`/`:149`; no `Revision`/`History` advance (still 0.6) | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-03 | Minor | `development-plan.md` (N16/N8) — prior R4-93-03 | Not updated: M4 "QSPI/NOR MTD done" vs M11 "5/13" without MTD; no Revision/History row | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-04 | Minor | `handover.md` (N8/N16) — prior R4-93-04 | `:63` "(14/14)" stale (18/18); `:126` still "next classes: NOR MTD" contradicting corrected `:130` | partly fixed (`:130`); residual open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-05 | Minor | `verification-results.md:277`,`:410` (N8) — prior R4-93-05 | "Remaining (M4)" still lists QSPI; History `:410` still claims `hal_mtd=PASS` in Renode | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-06 | Minor | `README.md:137`,`:408` (N8) — prior R4-93-06 | Scope table still lists QSPI/NOR as Planned and omits NVM/NOR from Implemented now; focus sentence still "NVM done; NOR/CAN" | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-07 | Minor | `docs/design/software-design.md:113` (N3) — prior R4-93-07 | SRS-REQ-056 SDD allocation / design 57/57 | corrected | SDD `:113`/`:151`; `README.md:54` | frick-fsw | **closed** | FRICK-REVIEW-0097 |
| R4-97-08 | Comment | `renode-environment.md:88` (N8) — prior R4-93-08 | HAL coverage still "(4/13)" (should be 5/13; MTD host-only) | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-09 | Comment | `traceability.md:20-23` — prior R4-93-09 | "Six … documented-only" vs SRS-REQ-052 partially delivered | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-10 | Comment | `CHANGELOG.md:10`,`:37` — prior R4-93-10 | Duplicate `### Added` in `[Unreleased]` | open | — | frick-fsw | open | FRICK-REVIEW-0097 |
| R4-97-11 | Comment | `docs/reviews/README.md` — prior R4-93-11 | Review-log row for FRICK-REVIEW-0093 | appended | `docs/reviews/README.md` | frick-fsw | **closed** | FRICK-REVIEW-0097 |

---

This delta re-review was scoped to the writing of this report (the review-log row
for FRICK-REVIEW-0097 and the `verification-results.md` review-record row are
intentionally **not** added here; they follow with the corrective change under the
review-log procedure, norm N11).