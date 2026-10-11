# Review Report - NVM backend increment (documentation, R4 / N17)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0083                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | R4 (SW documentation)                  |
| Reference revision                | `0eb009d` (Sprint 1.1, reference eNVM NVM backend, SRS-REQ-055) |
| Items under review                | `docs/requirements/SRS.md` (SRS-REQ-055, rev 1.13 + History), `docs/requirements/traceability.yaml` + `traceability.md` (mirror, counts 97/56/27/14), `README.md` (drivers row + metrics panel), `docs/development-plan.md` (M11 5/13), `docs/status/loop-audit.md` (M11 row, N3 count), `docs/status/handover.md` (counts, M11), `CHANGELOG.md`, `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md` (T3) |
| Date                              | 2026-10-11                             |
| Scope                             | Documentation completeness/consistency of the SRS-REQ-055 delta: exactly one generated metrics panel; header `Revision` vs `History` across every touched document; requirement counts (97/56/27/14) consistent everywhere; absence of stale "4/13" / "9 classes pending"; link/anchor validity; English-only (N2). Re-ran the CI `docs` required-document and document-id-uniqueness blocks and `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`. Excluded: code/V&V correctness (R1/R2), requirement singularity/boundedness (R3/N14), and the product-assurance verdict (N20, FRICK-REVIEW-0084). |
| Independence declaration          | Reviewer had no part in authoring, executing or reviewing this increment; no prior context. The concurrent sibling reports (R1/FRICK-REVIEW-0081, R2/FRICK-REVIEW-0082) were read only after this review's findings were independently derived. |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N2, N3, N4, N8, N11, N16, N17, N19 |

## 1. Scope

Reviewed the documentation changed/generated for the reference eNVM NVM backend
(SRS-REQ-055) at HEAD `0eb009d`. The check was documentary: whether the affected
controlled documents are internally consistent, mutually consistent, current and
complete, and whether the machine gates (`trace.py`, the CI `docs` block,
`metrics.py --check`) still converge.

In scope are only the documents listed above. Code correctness, the driver's
boundedness, the Renode oracle strength, requirement wording and the PA verdict
are other reviewers' scope and are not adjudicated here (they are referenced
where a documentation statement depends on them). The concurrent R1/R2/PA
reports for the same increment are untracked working-tree files at the time of
this review; this report does not depend on them.

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Traceability manifest: `docs/requirements/traceability.yaml`; gate `tools/scripts/trace.py`
- Metrics panel generator: `tools/scripts/metrics.py`; CI gates `.github/workflows/ci.yml`
- Review template: `docs/reviews/review-template.md`
- ECSS-E-ST-40C Rev.1 Annex F <6> (design DRD), ECSS-Q-ST-80C Rev.2 (PA/tool records)
- Sibling concurrent reviews: FRICK-REVIEW-0081 (R1), FRICK-REVIEW-0082 (R2), FRICK-REVIEW-0084 (PA)

## 3. Verification performed (independent)

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** - `97 requirements, 56 srs, 27 sss, 14 gaps, 51 lean theorems checked` (exit 0); `SSS-REQ-020 <- 8 srs children` |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** - "metrics panel: current" (README block byte-consistent) |
| CI `docs` required-document block | extracted from `.github/workflows/ci.yml:157-217`, run locally | **PASS** - no missing document |
| CI `docs` id-uniqueness block | same block | **PASS** - `documentation set: OK (132 unique document ids)` |
| Metrics-panel uniqueness | `grep` README | **PASS** - exactly one panel, `<!-- METRICS-BEGIN -->` (`README.md:39`) .. `<!-- METRICS-END -->` (`:94`); no second counts panel and no `Requirements set (current)` prose |
| Counts across touched docs | `grep` | **PASS** - current counts 97/56/27/14 agree in README panel (`:61`), handover (`:117`), loop-audit (`:49`), traceability.md (`:18`), verification-results (`:38`) |
| Stale `4/13` / `9 classes pending` | `grep` | **PASS with one exception** - loop-audit (`:34`) and development-plan (`:302`) correctly say 5/13 and 8 pending; `CHANGELOG.md:16` `4/13 -> 5/13` and `:303` are a legitimate transition/historical record; the handover retains a stale NVM entry (DOC-2) |
| Header `Revision` vs `History` | `grep` | **PASS** for SRS (1.13), traceability.md (0.24), verification-results (0.26); **FAIL** for tool-qualification (header 0.3.6 vs History 0.3.9, DOC-4); handover/loop-audit content changed without a History row (DOC-6/DOC-7) |
| Local links/anchors | markdown target existence scan over the touched docs | **PASS** - no broken local link targets; no new `](...)` links introduced by the change |
| English-only (N2) | diff scan | **PASS** - all new/changed prose is English ("Eärendil" is a project codename) |

## 4. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| DOC-1 | **Minor** | `docs/design/software-design.md` §6; SRS-REQ-055 (N3/N8) | **The SDD requirements->design matrix was not updated for the new requirement/design element.** §6 states "every software requirement is allocated to a design element", but the "Reference drivers" row still lists `src/drivers/{eth/gem,gpio/mpfs,serial/mpfs,mem,net}` and SRS-REQ-036/038/040/047/048/049 only; neither the new `nvm/mpfs` path nor SRS-REQ-055 appears. This is the visible cause of the README "Design traceability 55/56 (98.2%)" regression and is not recorded as a tracked gap. | `docs/design/software-design.md:94-97,113`; `README.md:54`; `git show --stat 0eb009d` does not include the SDD; `grep -n SRS-REQ-055 docs/design/software-design.md` -> none | Add `nvm/mpfs` + SRS-REQ-055 to the SDD §6 reference-drivers row, or explicitly record the deferral; N8 requires affected docs updated in the same change. |
| DOC-2 | **Minor** | `docs/status/handover.md:107` (N8/N19) | **Internal contradiction in the same paragraph: NVM is recorded as done and also listed as remaining.** The change added NVM to the done set (`:97-101`, `:106` `hal_nvm=PASS`), but `:107` still reads "Remaining M11 peripheral classes: NET(lwIP-on-GEM)/TSN/CAN/SPI/I2C/PCIE/SPACEWIRE/BLOCK/**NVM**". development-plan was correctly updated (NVM removed) and loop-audit correctly says 8 pending; the handover was not, so its remaining list has 9 entries instead of 8. | `docs/status/handover.md:97-107`; `docs/development-plan.md:302`; `docs/status/loop-audit.md:34` | Remove `NVM` from the handover remaining-M11 list. |
| DOC-3 | **Minor** | `docs/vv/tool-qualification.md` T17 row (N10/N16) | **The T17 register row's evidence description was not updated to match its own history.** History `0.3.9` states "T17 Renode functional check gains `hal_nvm`", but the T17 "Used for evidence of" cell still enumerates only `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`. Since T17's location includes `src/apps/app_renode_test.c` (which now carries the NVM clause), the register is internally inconsistent. | `docs/vv/tool-qualification.md:41` vs `:107` | Add `hal_nvm` to the T17 row (and the `ci:renode` mapping at `:71`) in a corrective commit. |
| DOC-4 | **Minor** | `docs/vv/tool-qualification.md` header (N16) | **Header `Revision` lags the History.** Header `Revision` is `0.3.6`, while History already carries `0.3.7`, `0.3.8` and now `0.3.9`. (Pre-existing at `0.3.6` vs `0.3.8`; the change extended it to `0.3.9` without advancing the header.) | `docs/vv/tool-qualification.md:8`; `:105-107` | Advance the header revision to the latest history row as part of the N16 record discipline. |
| DOC-5 | **Minor** | `CHANGELOG.md`, `docs/vv/verification-results.md` (N8/N11) | **The recorded unit-test check count is wrong.** Both records state `test_nvm` has "14 checks"; the harness counts executed `FF_TEST_ASSERT` macros and the built binary prints `checks=29 failures=0` (`tests/unit/test_nvm.c` has 29 `FF_TEST_ASSERT`). The V&V record is therefore inaccurate. | `./build/native/tests/test_nvm` -> `checks=29 failures=0`; `grep -c FF_TEST_ASSERT tests/unit/test_nvm.c` = 29; `CHANGELOG.md:15`; `docs/vv/verification-results.md:38` | Correct to **29** in both records and quote check counts from harness output rather than maintaining them by hand. |
| DOC-6 | **Minor** | `docs/status/handover.md` History (N16) | **The resume point's history was not advanced although its content changed.** The body count was updated to 97 (`:117`) but no History row was added: `Revision` stays `1.2` and the latest row `1.2` (`:184`) still records "counts 96/55/27/14". The authoritative resume point therefore contradicts itself. | `docs/status/handover.md:7,117,184`; `git show 0eb009d -- docs/status/handover.md` (no revision/history change) | Add a `1.3` History row (counts 97/56/27/14; M11 5/13) and bump `Revision`. |
| DOC-7 | **Minor** | `docs/status/loop-audit.md` (N19/N16) | **The loop audit's content changed without a History row and its counts remain untied to its declared baseline.** The M11 row (`:34`) and the N3 count (`:49` `97 reqs`) were updated, but `Revision` stays `0.5`, no History row was added (`:157` remains the latest), and the declared `Baseline commit e1def89` (`:9`) corresponds to **94** requirements. The baseline/count mismatch is the already-open NC DR-05 / PA-66-6 and is now wider (94 vs 97). | `docs/status/loop-audit.md:9,34,49,157`; `git show e1def89:docs/requirements/traceability.yaml \| grep -c '^  - id:'` = 94; `trace.py` -> 97 | Add a History row for this audit update and re-baseline (or annotate counts "at revision X") so baseline and reported counts agree; keep DR-05/PA-66-6 open until then. |
| DOC-8 | Comment | `docs/requirements/traceability.md:20-21` (N8) | **"Four of the requirements" lists twelve.** The planned-requirement note enumerates SRS-REQ-045/046/051/052/053/054 and SSS-REQ-022..027 (12 ids) but says "Four". Pre-existing, not introduced by this change, but the file was regenerated in this commit and the count word remains wrong. | `docs/requirements/traceability.md:20-21` | Replace "Four" with the correct number (12, or "several") or drop the numeric word. |
| DOC-9 | Comment | `docs/status/loop-audit.md:41` (N19) | **Stale PUS-C ASN.1 objective row.** `:41` still states the N7/ESA library is "only fetched ... NOT wired into build/runtime; default stays the local CCSDS open backend", whereas `:137`, `docs/status/handover.md:93-96`, ADR-0016 and the README state PUS-C ASN.1 is the default. Pre-existing; this loop touched the M11 row and N3 count but did not reconcile this row. | `docs/status/loop-audit.md:41` vs `:137`; `docs/status/handover.md:93-96` | Reconcile the PUS-C row (wired as default per ADR-0016) in the next loop-audit update. |
| DOC-10 | Comment | `README.md`, `docs/status/handover.md` (N8) | **Residual "NVM as future work" wording.** `README.md:407-408` and `handover.md:125` describe current focus as "Sprint 1 (… NVM/NOR + M10 QEMU)" and `handover.md:129` still suggests "**NVM (eNVM)** or **MEM/NOR MTD**" as the next class. Sprint 1's scope legitimately includes NVM+NOR, but the "next" ordering is now stale for NVM (delivered). | `README.md:406-408`; `docs/status/handover.md:125,129` | Refresh the "next" suggestions to drop NVM (keep NOR/QSPI as the S1 remainder). |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier. **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified. **Comment** =
observation/recommendation, no derogation.

### 4.1 Positive confirmations

- **One metrics panel only.** The README holds a single generated block
  (`README.md:39-94`); no duplicate counts panel and no `Requirements set
  (current)` prose (the M16 R4 DOC-1 correction holds).
- **Counts are consistent** across all touched documents and the manifest
  (97/56/27/14; SRS = 56), and `trace.py` reproduces them exactly.
- **`traceability.md` mirror regeneration is correct in substance**: the §1
  top-down row for SSS-REQ-020 now carries SRS-REQ-055 (`:47`), the §2 SRS table
  has the SRS-REQ-055 row (`:115`), the counts line reads `SRS = 56` (`:18`) and
  History `0.24` matches the header (`:208`).
- **No stale `4/13` or `9 classes pending`** in the live status documents;
  `CHANGELOG.md:16` is a legitimate `4/13 -> 5/13` transition statement.
- **English-only (N2)**, no broken links/anchors, and the CI `docs`
  required/uniqueness blocks pass (132 unique document ids).

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings (DOC-1..DOC-7) tracked to closure
- [ ] REJECTED - one or more Major findings

The documentation delta is substantially correct and internally consistent on
its core claims: the requirement, its SSS parent link, the traceability mirror,
the counts, the single metrics panel and the M11 progress statements are right,
and all documentary gates pass. The findings are completeness/records defects,
none of which invalidates the requirement or a safety barrier, so no Major is
raised within R4's scope.

Two caveats belong to other gates and are recorded here for the integrator:
(a) DOC-1 is a genuine ECSS design-trace gap (N3/N8) that must be closed or
tracked; and (b) the **process** Major raised by PA (FRICK-REVIEW-0084 F1 -
the increment was pushed without its committed review set, currently missing a
logged R4/N17 review and unlogged R1/R2 reports) is outside R4's documentary
adjudication but is a precondition of the N20 gate. The documentation verdict
can only be finalised once DOC-1..DOC-7 are corrected and re-reviewed (N12) and
this report and its siblings are committed and logged (N11).

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N17, N9)  | independent agent, FRICK-REVIEW-0083 | 2026-10-11 | APPROVED WITH COMMENTS (7 Minor, 3 Comment) |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| DOC-1 | Minor | `docs/design/software-design.md:113`; SRS-REQ-055; N3/N8 | SDD §6 matrix omits `nvm/mpfs` + SRS-REQ-055 (design traceability 55/56) | pending |  | frick-fsw | open | pending |
| DOC-2 | Minor | `docs/status/handover.md:107`; N8 | Remaining-M11 list still contains `NVM` after NVM done | pending |  | frick-fsw | open | pending |
| DOC-3 | Minor | `docs/vv/tool-qualification.md:41`; N10/N16 | T17 row not updated with `hal_nvm` though history 0.3.9 claims it | pending |  | frick-fsw | open | pending |
| DOC-4 | Minor | `docs/vv/tool-qualification.md:8`; N16 | Header `Revision` 0.3.6 lags History 0.3.9 | pending |  | frick-fsw | open | pending |
| DOC-5 | Minor | `CHANGELOG.md:15`, `docs/vv/verification-results.md:38`; N8/N11 | `test_nvm` check count stated 14, measured 29 | pending |  | frick-fsw | open | pending |
| DOC-6 | Minor | `docs/status/handover.md`; N16 | Counts changed to 97 without a History row / revision bump | pending |  | frick-fsw | open | pending |
| DOC-7 | Minor | `docs/status/loop-audit.md`; N19/N16 | Audit content changed without History row; baseline e1def89 (94) vs reported 97 (NC DR-05) | pending |  | frick-fsw | open | pending |
| DOC-8 | Comment | `docs/requirements/traceability.md:20-21`; N8 | "Four of the requirements" lists twelve | pending |  | frick-fsw | open | pending |
| DOC-9 | Comment | `docs/status/loop-audit.md:41`; N19 | Stale PUS-C ASN.1 "not wired/default" row | pending |  | frick-fsw | open | pending |
| DOC-10 | Comment | `README.md:407-408`, `docs/status/handover.md:125,129`; N8 | NVM still framed as upcoming work | pending |  | frick-fsw | open | pending |

## 8. Confidence

```text
Product/configuration and decision supported: frick-fsw @ 0eb009d (reference eNVM
  NVM backend, SRS-REQ-055); documentation completeness/consistency for criticality B.
Information item/revision/status: FRICK-REVIEW-0083 (this report), first issue.
Claim type: mixed - Findings are Fact (reproduced commands); severity is judgement.
Applicable baseline/tailoring: docs/rules.md N1-N20; ECSS-E-ST-40C Rev.1 Annex F <6>,
  ECSS-Q-ST-80C Rev.2; contract tailoring not inspected (none supplied to reviewer).
Sources and access level: full public repository at HEAD plus public ECSS standard
  metadata; full normative ECSS text not re-fetched for this review.
Independent cross-check and conflict reconciliation: trace.py, metrics.py --check and
  the CI docs blocks re-run by this reviewer; check counts measured from the built
  binaries; concurrent sibling reports (0081/0082/0084) read only after findings were
  derived - their overlap corroborates DOC-1..DOC-5 independently.
Confidence: High for the gate results and the reproduced counts (97/56/27/14, 29 vs 14);
  Medium for comparative severity, which follows project precedent (design-trace and
  records gaps graded Minor, tracked by sprint-plan S0 / NC DR-05).
Assumptions, TBD/TBC, detection limits and stale-data risk: the R1/R2/PA reports are
  untracked working-tree files and could change the process picture; the reviewers'
  reports themselves are not documentation under this review's adjudication.
Consequences if wrong: an uncorrected DOC-1 would leave a requirement without a
  design-trace entry at criticality B; uncorrected DOC-5 leaves an inaccurate V&V record.
Required technical owner/reviewer/approver: R4 for these NCs; integrator to commit the
  review set and log it; PA (N20) to re-assess the gate.
Recheck trigger/date: on the corrective commit (counts, SDD, handover, tool register)
  and on the N12 delta re-review; recheck earlier if the review set changes.
```

---

Committed and pushed with the reviewed change (norm N11); the review-log row in
`docs/reviews/README.md` is to be added by the integrator (this review was
instructed to modify no file other than this report).