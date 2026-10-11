# Review Report - NVM backend (documentation, R4 / N17) - delta re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0088                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | R4 (SW documentation)                  |
| Reference revision                | `03962d6` (NVM backend: close R1/R2/R4/PA review findings (N12)) |
| Items under review                | the corrective delta of FRICK-REVIEW-0083: `docs/design/software-design.md` (Revision + §6 matrix + History), `docs/status/handover.md` (M11 remaining list, History/Revision), `docs/status/loop-audit.md` (PUS-C row, Revision + History), `docs/vv/tool-qualification.md` (header + T17 row + History), `docs/vv/verification-results.md` (check count), `CHANGELOG.md`, `docs/requirements/traceability.md` (planned-count wording), `README.md` (design-traceability metric) |
| Date                              | 2026-10-11                             |
| Scope                             | Delta re-review of the seven Minor findings DOC-1..DOC-7 and the Comment DOC-8 of FRICK-REVIEW-0083 (plus the adjacent Comments DOC-9/DOC-10 needed to decide closure of 0083), against the corrective commit `03962d6` at HEAD. Re-ran the CI `docs` required-document and document-id-uniqueness blocks, `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`, `python3 tools/scripts/metrics.py --check`, `ctest`, and measured the `test_nvm` check count from the source and the built binary. Excluded: code correctness (R1), V&V adequacy (R2), requirement singularity (R3/N14) and the PA verdict (N20, FRICK-REVIEW-0084). |
| Independence declaration          | Reviewer had no part in authoring, executing or reviewing the corrective change; no prior context. FRICK-REVIEW-0083 was read as the finding source; the sibling reports (0081/0082/0084) were not relied on for any verdict here. |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N2, N3, N4, N8, N11, N12, N16, N17, N19 |

## 1. Scope

Delta re-review (norm N12) of the documentation corrections applied in
`03962d6` (HEAD) to close the findings of the first R4/N17 review
(FRICK-REVIEW-0083, issued against `0eb009d`). This report re-derives the
verdict for each finding DOC-1..DOC-8 independently and states whether the
corrective delta also closes the remaining Comments of 0083 (DOC-9, DOC-10),
which is required to close the review under N12.

No code, V&V-strength or PA judgement is made here. Where a documentation
statement depends on a measured artifact (a test-binary check count, a
manifest count), the value was reproduced by this reviewer rather than copied
from the commit message.

## 2. References

- Project norms: `docs/rules.md` (N1-N20); contribution gate `AGENTS.md`
- Prior review under closure: `docs/reviews/2026-10-11-nvm-r4-review.md` (FRICK-REVIEW-0083)
- Corrective commit: `03962d6` (`git show 03962d6`)
- Traceability manifest: `docs/requirements/traceability.yaml`; gate `tools/scripts/trace.py`
- Metrics panel generator: `tools/scripts/metrics.py`; CI gates `.github/workflows/ci.yml`
- Review template: `docs/reviews/review-template.md`
- ECSS-E-ST-40C Rev.1 Annex F `<6>` (design DRD), ECSS-Q-ST-80C Rev.2 (PA/tool records)

## 3. Verification performed (independent)

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** - `97 requirements, 56 srs, 27 sss, 14 gaps, 51 lean theorems checked` (exit 0); `SSS-REQ-020 <- 8 srs children` |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** - "metrics panel: current" |
| CI `docs` required-document block | extracted from `.github/workflows/ci.yml`, run locally | **PASS** - no missing document |
| CI `docs` id-uniqueness block | same block | **PASS** - `documentation set: OK (136 unique document ids)` (125 at 0083 + the 4 committed review reports + this report's id added post-check) |
| Unit-test check count (N11) | `grep -c FF_TEST_ASSERT tests/unit/test_nvm.c`; `./build/native/tests/test_nvm` | **36** and `checks=36 failures=0` (matches the corrective records) |
| ctest | `ctest --test-dir build/native` | **PASS** - 17/17 tests passed |
| DOC-1 design trace | `grep -n "SRS-REQ-055\|nvm/mpfs" docs/design/software-design.md`; README metric | **PASS** - SDD §6 row and `README.md:54` `56/56 (100%)` |
| DOC-6 handover History | `sed`/`grep` over `docs/status/handover.md` | **FAIL** - see DEL-1 |
| DOC-8 planned wording | `sed` `docs/requirements/traceability.md:18-24`; manifest parent links | **PASS** - "Six ... SRS-REQ-045/046/051/052/053/054 with their SSS parents SSS-REQ-022..027"; parent links verified in the manifest |

## 4. Findings

### 4.1 Disposition of the FRICK-REVIEW-0083 findings

| # (0083) | Severity | Finding (abridged) | Delta evidence | Disposition |
| ------- | -------- | ------------------ | -------------- | ----------- |
| DOC-1 | Minor | SDD §6 matrix omitted `nvm/mpfs` + SRS-REQ-055 (design trace 55/56) | `docs/design/software-design.md:113` now `src/drivers/{eth/gem,gpio/mpfs,serial/mpfs,mem,nvm/mpfs,net}` + `...SRS-REQ-049, SRS-REQ-055`; Revision `0.2 -> 0.3` + History `0.3`; `README.md:54` `Design traceability 56/56 (100%)` | **CLOSED** |
| DOC-2 | Minor | handover remaining-M11 list still contained `NVM` | `docs/status/handover.md:107` now `.../SPACEWIRE/BLOCK.` (NVM removed; matches development-plan 8-pending / loop-audit `8 classes pending`) | **CLOSED** |
| DOC-3 | Minor | T17 register row not updated with `hal_nvm` though History 0.3.9 claimed it | `docs/vv/tool-qualification.md:41` T17 "Used for evidence of" now `.../hal_eth/hal_net/hal_nvm`; the `ci:renode` mapping (`:44`, `:71`) carries no per-HAL enumeration, so no further change was required | **CLOSED** |
| DOC-4 | Minor | header `Revision` 0.3.6 lagged History 0.3.9 | `docs/vv/tool-qualification.md:7` header `0.3.10` equals the latest History row `0.3.10` (`:108`) | **CLOSED** |
| DOC-5 | Minor | recorded `test_nvm` check count wrong (14 vs measured) | `CHANGELOG.md:15` and `docs/vv/verification-results.md:38` now `36 checks`; independently reproduced (`grep -c` = 36; binary `checks=36 failures=0`) | **CLOSED** |
| DOC-6 | Minor | handover counts changed without a History row / revision bump | unchanged: Revision still `1.2` (`:7`), body says `97 (56 srs / 27 sss / 14 gap)` (`:117`) and M11 5/13, but the latest History row `1.2` (`:184`) still records `counts 96/55/27/14`; `03962d6` touched only `:107` | **OPEN** - see DEL-1 |
| DOC-7 | Minor | loop-audit content changed without History row; baseline e1def89 (94) vs reported counts | `docs/status/loop-audit.md` Revision `0.5 -> 0.6` (`:7`) + History row `0.6` (`:158`); baseline still `e1def89` (`:9`) with current counts 97 - the residual baseline mismatch is the pre-existing NC `PA-66-6` (`docs/reviews/2026-10-11-m16-pa-review.md:166`), which the finding itself directed to keep open | **CLOSED** (History row added; baseline re-baseline correctly remains tracked as `PA-66-6`) |
| DOC-8 | Comment | "Four of the requirements" listed twelve | `docs/requirements/traceability.md:20-22` now "Six of the requirements (SRS-REQ-045/046/051/052/053/054 with their SSS parents SSS-REQ-022/023/024/025/026/027)"; the six parent links verified in the manifest | **CLOSED** |
| DOC-9 | Comment | stale PUS-C ASN.1 "not wired/default" row | `docs/status/loop-audit.md:41` now "reconciled (build-time generation; `FF_PusBackendN7` default, ADR-0016)" - consistent with `:137`/handover | **CLOSED** |
| DOC-10 | Comment | residual "NVM as future work" wording | unchanged: `docs/status/handover.md:125` "S1 (NVM/NOR + M10 QEMU)", `:129` "**NVM (eNVM)** or **MEM/NOR MTD**"; `README.md:407-408` "NVM/NOR + M10 QEMU" | **OPEN** - see DEL-2 |

### 4.2 Residual findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| DEL-1 | **Minor** | `docs/status/handover.md:7,117,184` (N16/N8); residual of FRICK-REVIEW-0083 DOC-6 | **The resume point's history was still not advanced, although its content is current and contradicts its own History.** The body now reports `97 (56 srs / 27 sss / 14 gap)` and M11 `5/13` with NVM done, but `Revision` remains `1.2` and the latest History row `1.2` still records `counts 96/55/27/14` and predates the NVM increment. The corrective commit `03962d6` modified only the remaining-M11 line (`:107`), not the header/History. 0083 DOC-6 recommended a `1.3` row (`97/56/27/14`; M11 5/13) and a Revision bump; this was not applied. | `docs/status/handover.md:7` (`Revision 1.2`), `:117` (`97 (56 srs / 27 sss / 14 gap)`), `:184` (`1.2 ... counts 96/55/27/14`); `git show 03962d6 -- docs/status/handover.md` (single hunk at `:107`) | Add a `1.3` History row recording the NVM increment (counts 97/56/27/14; M11 5/13; SRS-REQ-055) and bump the header `Revision` to `1.3`. |
| DEL-2 | Comment | `docs/status/handover.md:125,129`; `README.md:407-408` (N8); residual of FRICK-REVIEW-0083 DOC-10 | **NVM is still framed as upcoming work.** With NVM delivered (SRS-REQ-055, M11 5/13), the "next HAL class" list still leads with "**NVM (eNVM)** or **MEM/NOR MTD**" and the focus line still scopes Sprint 1 as "NVM/NOR + M10 QEMU". Sprint 1 legitimately includes NOR, but the "next" ordering is stale for NVM. | `docs/status/handover.md:125,129`; `README.md:407-408` | Drop NVM from the "next" suggestions (keep NOR/QSPI as the S1 remainder). No derogation (Comment). |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier. **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified. **Comment** =
observation/recommendation, no derogation.

### 4.3 Positive confirmations

- **Seven of the eight required corrections are effective.** DOC-1..DOC-5,
  DOC-7 and DOC-8 are demonstrably closed at HEAD, and the two Comments DOC-9
  and (partially) DOC-10 area are reconciled (DOC-9 fully).
- **All documentary gates converge at HEAD**: `trace.py` `PASS
  97/56/27/14/51`; `metrics.py --check` exit 0; the CI `docs`
  required/uniqueness block passes (136 unique ids); `ctest` 17/17.
- **The check-count correction is now honest and reproducible**: the records
  quote `36 checks`, which equals both the source (`36` `FF_TEST_ASSERT`) and
  the built binary (`checks=36 failures=0`). No residual "14" remains in the
  live records (only inside the superseded review reports).
- **DOC-1 removed the visible regression**: the SDD §6 "Reference drivers" row
  now carries `nvm/mpfs` + `SRS-REQ-055`, and the README design-traceability
  metric reads `56/56 (100%)` - no `55/56` remains in any live document.
- **Header/History reconciliation is coherent** in `tool-qualification.md`
  (`0.3.10` == `0.3.10`) and `loop-audit.md` (`0.6` == `0.6`); `traceability.md`
  (`0.24`) and `verification-results` (`0.26`) headers still match their
  histories.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - one residual Minor (DEL-1 / 0083 DOC-6) and one residual Comment (DEL-2 / 0083 DOC-10)
- [ ] REJECTED - one or more Major findings

The corrective delta is substantively effective: all five design/record/value
corrections requested as Minor (DOC-1..DOC-5) and the two record-discipline
corrections DOC-7/DOC-8 are closed, the PUS-C reconciliation (DOC-9) holds, and
every documented gate converges at HEAD. No Major is raised within R4's scope.

**FRICK-REVIEW-0083 is NOT closed.** Under N12 a review closes only when every
non-conformance is resolved. One Minor (DOC-6) remains genuinely uncorrected at
HEAD and one Comment (DOC-10) remains unaddressed. The review can be closed
once DEL-1 is corrected and DEL-2 is either corrected or explicitly accepted as
an observation.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N17, N9)  | independent agent, FRICK-REVIEW-0088 | 2026-10-11 | APPROVED WITH COMMENTS (1 Minor residual, 1 Comment residual) |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| DOC-1 | Minor | `docs/design/software-design.md:113`; SRS-REQ-055; N3/N8 | SDD §6 matrix omitted `nvm/mpfs` + SRS-REQ-055 | corrected in `03962d6` | SDD §6 row + `README.md:54` 56/56 | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-2 | Minor | `docs/status/handover.md:107`; N8 | remaining-M11 list still contained `NVM` | corrected in `03962d6` | `handover.md:107` | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-3 | Minor | `docs/vv/tool-qualification.md:41`; N10/N16 | T17 row not updated with `hal_nvm` | corrected in `03962d6` | `tool-qualification.md:41` | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-4 | Minor | `docs/vv/tool-qualification.md:7`; N16 | header `Revision` lagged History | corrected in `03962d6` | header `0.3.10` == History `0.3.10` | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-5 | Minor | `CHANGELOG.md:15`; `docs/vv/verification-results.md:38`; N8/N11 | `test_nvm` count wrong | corrected in `03962d6` | 36 = source = binary | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-6 | Minor | `docs/status/handover.md:7,117,184`; N16 | counts changed without History row / revision bump | not corrected | DEL-1 | frick-fsw | **open** | FRICK-REVIEW-0088 |
| DOC-7 | Minor | `docs/status/loop-audit.md`; N19/N16 | audit changed without History row; baseline mismatch | corrected in `03962d6` (History 0.6); baseline re-baseline tracked under `PA-66-6` | loop-audit Revision/History `0.6` | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-8 | Comment | `docs/requirements/traceability.md:20-22`; N8 | "Four ..." listed twelve | corrected in `03962d6` | wording + manifest parent links | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-9 | Comment | `docs/status/loop-audit.md:41`; N19 | stale PUS-C ASN.1 row | corrected in `03962d6` | row reads "reconciled ... ADR-0016" | frick-fsw | **closed** | FRICK-REVIEW-0088 |
| DOC-10 | Comment | `README.md:407-408`, `docs/status/handover.md:125,129`; N8 | NVM still framed as upcoming work | not corrected | DEL-2 | frick-fsw | **open** | FRICK-REVIEW-0088 |

## 8. Confidence

```text
Product/configuration and decision supported: frick-fsw @ 03962d6 (corrective
  delta of the reference eNVM NVM backend, SRS-REQ-055); documentation
  completeness/consistency for criticality B.
Information item/revision/status: FRICK-REVIEW-0088 (this report), first issue.
Claim type: mixed - gate results and dispositions are Fact (reproduced commands);
  severity of the residual is judgement.
Applicable baseline/tailoring: docs/rules.md N1-N20; ECSS-E-ST-40C Rev.1 Annex F
  <6>, ECSS-Q-ST-80C Rev.2; contract tailoring not inspected (none supplied).
Sources and access level: full public repository at HEAD 03962d6; full normative
  ECSS text not re-fetched for this review.
Independent cross-check and conflict reconciliation: trace.py, metrics.py --check,
  the CI docs blocks, ctest and the test_nvm count re-run/measured by this
  reviewer; finding dispositions re-derived from the working tree, not from the
  commit message.
Confidence: High for the gate results and the DOC-1..DOC-5/DOC-7/DOC-8 closures
  and for the DOC-6 residual (byte-level contradiction in handover.md);
  Medium for comparative severity, following project precedent.
Assumptions, TBD/TBC, detection limits and stale-data risk: the delta addresses
  only the documentation findings of FRICK-REVIEW-0083; the R1/R2/PA findings are
  outside this report's adjudication. The loop-audit baseline/PA-66-6 mismatch is
  deliberately left to the PA track (not re-raised here).
Consequences if wrong: an uncorrected DOC-6/DEL-1 leaves the authoritative resume
  point self-contradictory (body 97 vs History 96) and misleads the next agent;
  FRICK-REVIEW-0083 would remain open and block N12/N17 closure of the increment.
Required technical owner/reviewer/approver: R4 for these NCs; integrator to
  correct DEL-1 (and optionally DEL-2) and log this report; PA (N20) gate.
Recheck trigger/date: on the corrective commit for DEL-1 and on closure of
  FRICK-REVIEW-0083; recheck earlier if the review set changes.
```

---

Committed and pushed with the reviewed change (norm N11); the review-log row in
`docs/reviews/README.md` is to be added by the integrator (this review was
instructed to modify no file other than this report).