# Review Report — Product Assurance delta review of the R2/R3 test-gap increment (norm N20/N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0080                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | `e0c7ad8` (branch `main`), 2026-10-11  |
| Items under review                | the FRICK-REVIEW-0078 findings and their fixes: `docs/requirements/traceability.yaml` (SRS-REQ-041/050, SSS-REQ-010 tests evidence), `README.md` (coverage rows), `docs/vv/verification-results.md` (cycle-log row + 0.25 history), `docs/vv/tool-qualification.md` (T3 0.3.8), `src/core/tbl/tbl.c` (R1-01 fix), `src/pus/pus_services.c` (R1-02 fix), `tests/CMakeLists.txt` + `tests/unit/test_r2_gap.c` (R1-03 fix), `tests/unit/test_r3_gap.c` (regression), `CHANGELOG.md`, and the companion source reviews `docs/reviews/2026-10-11-test-gap-r1-review.md` (FRICK-REVIEW-0077) and `docs/reviews/2026-10-11-test-gap-r1-delta-review.md` (FRICK-REVIEW-0079) |
| Date                              | 2026-10-11                             |
| Scope                             | Verify that each Minor/Comment of FRICK-REVIEW-0078 is resolved at HEAD, that the independent source review (FRICK-REVIEW-0077) exists and is independent of the R2/R3 authors, that its Major finding is actually fixed (and delta-verified), and re-run the project gates. Record-accuracy and remaining over-tracing only. |
| Independence declaration          | Reviewer took no part in authoring, integrating or executing the reviewed test suites, the `src/` fix, the manifest edit or any record. Fresh independent session with no author context. Read-only for the repository: only this report file was created. `trace.py` / `metrics.py` / `ctest` were executed read-only (outputs under git-ignored `build/`); no tracked file was modified. |
| Norms applied                     | N4, N6, N8, N9, N10, N11, N12, N15, N16, N17, N20 (see `docs/rules.md`) |

## 1. Scope

This is the norm-N12 delta re-review of the norm-N20 review FRICK-REVIEW-0078
(`docs/reviews/2026-10-11-test-gap-pa-review.md`, verdict APPROVED WITH COMMENTS,
5 Minor + 1 Comment) after the fixes delivered at HEAD `e0c7ad8`
("Fix TBL validation bypass (Major) + make new tests order-independent (R1
findings)").

FRICK-REVIEW-0078 left the increment APPROVED WITH COMMENTS but left open six
records (PA-78-01..PA-78-06). In parallel, the independent R1 source review
FRICK-REVIEW-0077 was produced for the same increment (norm N9/N10, addressing
PA-78-05) and **rejected** it on one Major finding (a table-validation bypass in
`src/core/tbl/tbl.c`). HEAD `e0c7ad8` is the combined fix for both review streams.
A companion independent R1 delta re-review, FRICK-REVIEW-0079
(`docs/reviews/2026-10-11-test-gap-r1-delta-review.md`), re-verifies that fix at
`e0c7ad8`; this PA review treats it as a cross-checked input, not as a substitute
for the PA verdict (norm N9: independent profiles, independent sessions).

**Included.** (a) Whether each PA-78-01..06 disposition is actually implemented at
HEAD; (b) whether the independent source review exists, is genuinely independent
of the R2/R3 suite authors, and whether the Major it found is fixed and
delta-verified; (c) independent re-run of the project gates; (d) any remaining
record inaccuracy/over-tracing.

**Excluded.** The technical content of the `src/` modules beyond confirming the
R1-01 fix and its regression; the Lean proofs; on-target behaviour (GAP-A5); the
full correctness of the new suites (the domain of FRICK-REVIEW-0077/0079).

**Method.** Read the HEAD diff and every artefact in scope; executed the gates
independently; read `src/core/tbl/tbl.c` and `src/pus/pus_services.c` to confirm
the fix; cross-checked the independently reproduced results recorded in
FRICK-REVIEW-0079; compared recorded numbers with the measured
`build/coverage/coverage.json`; and re-checked the traceability evidence links
against the actual test sources.

**Independence note (log rows).** Per the task instruction this review modifies
nothing but this report file. The `docs/reviews/README.md` review-log rows for
FRICK-REVIEW-0077/0078/0079/0080 and this review's own `verification-results.md`
cycle-log row remain the integrator's obligation (norm N11/N17). The absence of
the 0077/0078 log rows is itself tracked below (Finding 1).

## 2. References

- Project norms: `docs/rules.md` (N1–N20), especially N4, N9, N10, N11, N12, N15, N20.
- Prior review: `docs/reviews/2026-10-11-test-gap-pa-review.md` (FRICK-REVIEW-0078).
- Independent source review: `docs/reviews/2026-10-11-test-gap-r1-review.md` (FRICK-REVIEW-0077).
- Independent R1 delta review: `docs/reviews/2026-10-11-test-gap-r1-delta-review.md` (FRICK-REVIEW-0079).
- Task reports: FRICK-REVIEW-0075 (R2), FRICK-REVIEW-0076 (R3).
- Traceability manifest: `docs/requirements/traceability.yaml`.
- Tool register: `docs/vv/tool-qualification.md` (T3, 0.3.8).
- Records: `docs/vv/verification-results.md` (cycle log + 0.25), `CHANGELOG.md`,
  `docs/reviews/README.md`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025) 5.8.3.x (traceability), ECSS-Q-ST-80C Rev.2
  (30 Apr 2025) 6.2.6.12a (traceability matrices verified at each milestone) and
  6.3.x (product assurance records).

## 3. Verification performed (independent)

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 96 requirements (55 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims (exit 0) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0**, "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **16/16 PASS** (incl. `r2_gap`, `r3_gap`) |
| Suite check counts | `./build/native/tests/test_r2_gap`, `test_r3_gap` | **487 checks / 0 failures**, **201 checks / 0 failures** |
| Measured coverage | `build/coverage/coverage.json` | lines **2831/3961 (71.5%)**, functions **371/376 (98.7%)** — matches README rows exactly |

## 4. Disposition of FRICK-REVIEW-0078 findings

| NC | Severity | Finding (0078) | Status at HEAD | Evidence |
| -- | -------- | -------------- | -------------- | -------- |
| PA-78-01 | Minor | `test_r2_gap.c` linked as evidence for SRS-REQ-050 (N7 ASN.1 backend) | **CLOSED** — link removed | `docs/requirements/traceability.yaml:476` now `["tests/unit/test_pus.c", "ci:renode"]` |
| PA-78-02 | Minor | unit test listed as evidence for SRS-REQ-041 (MISRA) / SSS-REQ-010 (static) | **CLOSED** — links removed | `traceability.yaml:395` `["ci:misra"]`; `:589` `["ci:static-analysis"]` |
| PA-78-03 | Minor | README code-coverage rows stale (1917/2920, 257/302) | **CLOSED** — rows refreshed to measured values | `README.md:65` `2831/3961 (71.5%)`, `:66` `371/376 (98.7%)`; `metrics.py --check` exit 0; matches `coverage.json` |
| PA-78-04 | Minor | no cycle-log row for the increment's gate results | **CLOSED** — row added | `verification-results.md:37` ("R2/R3 test-gap task + TBL fix") records build/ctest 16/16, Lean 21/21, trace 96/55/27/14, static, RTEMS proxy |
| PA-78-05 | Minor | new V&V scripts self-reviewed; no independent source review | **CLOSED** — independent R1 source review exists and found a Major | `docs/reviews/2026-10-11-test-gap-r1-review.md` (FRICK-REVIEW-0077, R1, fresh context, authored none of the suites) |
| PA-78-06 | Comment | 0.25 history clause implied NC PA-67-01 closure | **ADDRESSED** — clause no longer references PA-67-01 | `verification-results.md:388` now "...; TBL defect fix + `FF_PusInit` idempotence; SDD section 6 requirements->design matrix added" |

**All five Minor findings and the one Comment of FRICK-REVIEW-0078 are closed at
HEAD.** The over-traced links were also independently corroborated by
FRICK-REVIEW-0077's R1-04 and re-confirmed by FRICK-REVIEW-0079's R1-04.

**Independent source review (PA-78-05) — assessment.** FRICK-REVIEW-0077 was
written by profile R1 (Senior SW development engineer), fresh context, with an
explicit declaration that the reviewer "authored, integrated and executed none
of the reviewed test suites" — the suites under review were authored by R2
(`test_r2_gap.c`, FRICK-REVIEW-0075) and R3 (`test_r3_gap.c`, FRICK-REVIEW-0076).
The reviewer profile therefore differs from both suite authors, satisfying the
N9 independence expectation for the T3 source review. The value of that
independence is demonstrated by its outcome: it reproduced the T3 checks,
re-ran the suites in isolation, and found a **Major defect** (`src/core/tbl/tbl.c`
validation bypass) that both the increment's authors and FRICK-REVIEW-0078 had
missed.

**Major fix (R1-01) — assessment.** At HEAD `src/core/tbl/tbl.c:71` clears
`g_tables[h].staged = false` when the validator rejects the loaded image, so a
rejected load can no longer leave a stale `staged=true` that `FF_TblActivate`
(`tbl.c:84`) would commit. The regression added at `tests/unit/test_r3_gap.c:776-785`
loads a valid image, activates it, then performs a rejected load and asserts
`FF_TblActivate` returns `FF_NOT_READY` and the live image is unchanged
(`gain == 100`) — the exact R1-01 scenario. The suite check count rose 198 -> 201
and passes (201/0). The companion R1 delta review FRICK-REVIEW-0079 additionally
confirmed the fix by a standalone public-API probe and by **reverting the
one-line fix in a throwaway worktree**, showing the regression is discriminating
(FRICK-REVIEW-0079 D6/D7). I did not reproduce that worktree probe myself, but the
source read plus the passing regression are consistent with it. R1-02
(`src/pus/pus_services.c:425-433` resets the registries so `FF_PusInit` is
idempotent; every PUS routine in `test_r2_gap.c` now calls `pus_register_all()`)
and R1-03 (`tests/CMakeLists.txt:12-14` sets `FF_TEST_NO_HAL` when `FF_BUILD_HAL=OFF`;
the HAL include and section are guarded at `test_r2_gap.c:32,455,965`) are also
implemented. **The R1 Major is fixed and delta-verified; R1-01..R1-05 are closed
(FRICK-REVIEW-0079).**

## 5. Findings (new)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `docs/reviews/README.md` review index (N11/N17) | The report files for FRICK-REVIEW-0077, FRICK-REVIEW-0078 and the companion FRICK-REVIEW-0079 are present in the tree, but the **review-log index gained no row** for any of them. The prior reports' own closing notes state the log row is the integrator's obligation, and this delta cycle added the `verification-results.md` cycle row but not the review-log rows. The record index is therefore incomplete for the increment. Independently raised as REC-79-01. | `docs/reviews/README.md` ends at the 0075/0076 task reports (lines 76-77); the 0077/0078/0079 report files are present (`git status`); FRICK-REVIEW-0077 closing note; FRICK-REVIEW-0078 §1; FRICK-REVIEW-0079 REC-79-01 | Append review-log rows for FRICK-REVIEW-0077 (R1, REJECTED 1 Major -> closed), FRICK-REVIEW-0078 (PA, APPROVED WITH COMMENTS), FRICK-REVIEW-0079 (R1 delta) and this delta review, in the same commit (N11/N17). |
| 2 | Comment | `CHANGELOG.md:27` (N16) | The `### Added` bullet still states `test_r3_gap.c` (**198 checks**), while the fix bumped the suite to **201** (recorded correctly in `verification-results.md:37` and `tool-qualification.md` 0.3.8). The CHANGELOG figure is stale (understates the delivered suite). Independently raised as REC-79-02. | `CHANGELOG.md:27` vs measured 201 (§3); `verification-results.md:37`; `tool-qualification.md:106` | Update the figure to 201 (or annotate it as the pre-fix count). |
| 3 | Comment | `verification-results.md` 0.25 history (N16) | The revised clause no longer implies NC PA-67-01 closure (the PA-78-06 defect is fixed), but it now attributes "SDD section 6 requirements->design matrix added" to the R2/R3 test-gap task; SDD section 6 was in fact added in the preceding commit `b714c56`, not in the test-gap change. Factually the section exists; only the attribution is imprecise. | `verification-results.md:388`; `git log -S "Design traceability matrix" -- docs/design/software-design.md` = `b714c56` | Optionally reword to reference the commit/delivery window rather than the task, or leave if the entry is read as a release-window summary. |
| 4 | Comment | `src/core/tbl/tbl.c` (R1-01 hardening) | The security defect is fixed, but `staged` is still **not cleared after a successful `FF_TblActivate`** (`tbl.c:87-90`), and the new `staged` write at `tbl.c:71` is outside `g_lock` while the `memcpy` is inside it. Neither is a defect today (single-threaded; a repeated activate merely re-commits the same validated staged image, and any rejected load now clears `staged`), but the flag no longer strictly means "an uncommitted validated image awaits activation". Overlaps REC-79-03. | `src/core/tbl/tbl.c:71,75,84,87-90`; R1-01 recommendation in FRICK-REVIEW-0077 §4; FRICK-REVIEW-0079 REC-79-03 | For robustness, guard the `staged` flag with the mutex and clear it after a successful activate, or document the intended semantics. Not re-review blocking. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 6. Process-obligation assessment

- **N4 (trace convergence).** PASS 96/55/27/14; the two over-traced links of
  PA-78-01/02 are removed; the remaining `test_r2_gap.c`/`test_r3_gap.c` evidence
  links point to requirement families the suites genuinely exercise. Met.
- **N9/N10 (independent review + tool register).** The T3 suites have an
  independent R1 source review (0077) that found and fixed a Major, followed by
  an independent R1 delta re-review (0079) confirming the fix; T3 history 0.3.8
  records the re-qualification and the new check count. Met.
- **N11 (records).** Cycle-log row added and the 0.25 clause corrected — good.
  The review-log index rows for 0077/0078/0079 are still missing (Finding 1). The
  CHANGELOG suite count is stale (Finding 2). Mostly met.
- **N15 (full cycle).** The cycle-log row records build/ctest 16/16, Lean 21/21,
  trace PASS, static clean, RTEMS proxy — consistent with the change scope
  (`tests/` + records + two `src/` lines, no verified/Lean module). Met.
- **N8 (docs current).** README coverage rows now match the measured values and
  `metrics.py --check` passes. Met.
- **Honesty of the closure claim.** The increment no longer overstates: the
  headline "real defect fixed" (FRICK-REVIEW-0077 Major) is true and
  delta-verified, the regression genuinely guards it, and the coverage rows are
  the measured ones. The residual inaccuracies are a stale CHANGELOG count and
  the review-log index omission. No false claim of defect-freedom or
  qualification was found.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — all FRICK-REVIEW-0078 Minor findings closed; 1 new Minor + 3 Comment**
- [ ] REJECTED — one or more Major findings

**FRICK-REVIEW-0078's five Minor findings (PA-78-01..05) are CLOSED and its
Comment (PA-78-06) is ADDRESSED.** The fix independently confirms PA-78-05:
FRICK-REVIEW-0077 (R1, independent of the R2/R3 authors) found a Major in
`src/core/tbl/tbl.c`, and that defect is demonstrably fixed, regression-guarded
and delta-verified at HEAD (FRICK-REVIEW-0079). The one new Minor is a record
completeness item (the missing review-log index rows); the three Comments are
record-accuracy/hardening observations. None is a product defect or defeats a
requirement.

**Does the test-gap increment pass the N20 PA review?** Yes — the increment now
passes the norm-N20 PA review. Its remaining obligations are procedural (norm N12
closure of the R1 findings is recorded by FRICK-REVIEW-0079; the review-log index
and the CHANGELOG count), not substantive; they do not warrant rejection. The
verdict is **APPROVED WITH COMMENTS** pending those record closures.

## 8. Confidence

```text
Decision/product and configuration: frick-fsw @ e0c7ad8; ECSS criticality B target.
Claim type: mixed (Fact for gate results, file contents and dispositions; Inference for
  the process verdict).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1
  (30 Apr 2025) 5.8.3.x; ECSS-Q-ST-80C Rev.2 (30 Apr 2025) 6.2.6.12a/6.3.x. Customer
  tailoring matrix not available to this reviewer (unchanged from PA-67/PA-78).
Evidence consulted: HEAD diff 8f62746..e0c7ad8; test sources; tbl.c/pus_services.c;
  traceability.yaml; README.md; verification-results.md; tool-qualification.md;
  CHANGELOG.md; docs/reviews/README.md; FRICK-REVIEW-0077/0078/0079; live gates
  (trace.py, metrics.py, ctest, coverage.json).
Cross-check and reconciliation: README coverage rows vs coverage.json vs measured;
  trace evidence links vs actual test symbols; PA-78 dispositions vs the HEAD diff;
  R1-01 fix vs its regression and vs the companion FRICK-REVIEW-0079 delta evidence.
Confidence: High for the dispositions, the gate results and the fix (direct file and
  gate evidence, reproduced locally; the discriminating-revert result is cited from
  FRICK-REVIEW-0079, not re-executed here). Medium for the record findings, which rest
  on the review-log index and a stale CHANGELOG figure.
Basis and detection limits: full diff and all in-scope records read; gates executed;
  coverage.json read. The independence of FRICK-REVIEW-0077/0079 is accepted as declared
  (fresh context, no authorship), consistent with the prior PA-78 treatment; the agent
  cannot test session identity.
Missing/inaccessible evidence: customer tailoring matrix; RTEMS cross/on-target run for
  this increment (n/a: the change is tests + records + two C lines, no verified/Lean
  module changed).
Assumptions and expiry/recheck date: assumes the task instruction to modify only this
  report is authoritative; assumes FRICK-REVIEW-0079 (produced in a companion session)
  is integrated with this cycle. Recheck after the review-log index update. Recheck on
  next baseline change or 2027-04-11.
Consequences if wrong: if the project treats the review-log index as integrator-only
  and the CHANGELOG count as historical, Findings 1-2 downgrade to Comments; all PA-78
  closures are independent of that.
Required independent human reviewer/authority: project engineering/PA/release
  authority; this agent cannot approve, close NCs, or accept residual risk.
```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Senior SW dev engineer, FRICK-REVIEW-0077 | 2026-10-11 | REJECTED (1 Major, 3 Minor, 2 Comment) -> fixes at `e0c7ad8` |
| R1 (N9)       | Senior SW dev engineer, FRICK-REVIEW-0079 (delta) | 2026-10-11 | APPROVED WITH COMMENTS (Major closed) |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0080** | **2026-10-11** | **APPROVED WITH COMMENTS (PA-78 Minors closed; 1 new Minor, 3 Comment)** |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-78-01 | Minor | SRS-REQ-050 / `traceability.yaml` | unsupported `test_r2_gap.c` evidence link | link removed | `traceability.yaml:476`; `trace.py` PASS | frick-fsw | **closed** | FRICK-REVIEW-0080 |
| PA-78-02 | Minor | SRS-REQ-041 / SSS-REQ-010 | unsupported evidence links | links removed | `traceability.yaml:395,589` | frick-fsw | **closed** | FRICK-REVIEW-0080 |
| PA-78-03 | Minor | README metrics panel | stale coverage rows | refreshed to measured values | `README.md:65-66`; `metrics.py --check` exit 0; `coverage.json` | frick-fsw | **closed** | FRICK-REVIEW-0080 |
| PA-78-04 | Minor | `verification-results.md` cycle log | missing cycle-log row | row added | `verification-results.md:37` | frick-fsw | **closed** | FRICK-REVIEW-0080 |
| PA-78-05 | Minor | N9/N10 (T3 suites) | no independent source review | independent R1 review; Major found+fixed+delta-verified | FRICK-REVIEW-0077/0079; `tbl.c:71`; `test_r3_gap.c:776-785` | frick-fsw / V&V | **closed** | FRICK-REVIEW-0080 |
| PA-78-06 | Comment | `verification-results.md` 0.25 history | implied PA-67-01 closure | clause corrected | `verification-results.md:388` | frick-fsw | **closed** | FRICK-REVIEW-0080 |
| PA-80-01 | Minor | `docs/reviews/README.md` index | no log rows for FRICK-REVIEW-0077/0078/0079 | add the index rows | README append | frick-fsw | open | pending (dup REC-79-01) |
| PA-80-02 | Comment | `CHANGELOG.md:27` (N16) | stale `test_r3_gap` count (198 vs 201) | correct the figure | `CHANGELOG.md` re-read | frick-fsw | open | pending (dup REC-79-02) |
| PA-80-03 | Comment | `verification-results.md:388` | SDD section 6 attributed to the test-gap task (added in `b714c56`) | optional rewording | record re-read | frick-fsw | open | pending |
| PA-80-04 | Comment | `src/core/tbl/tbl.c` | `staged` not cleared after successful activate; flag write outside `g_lock` | optional hardening document/guard | code review | frick-fsw | open | pending (dup REC-79-03) |

---

Committed and pushed with the reviewed change (norm N11). The
`docs/reviews/README.md` review-log rows and this report's `verification-results.md`
cycle-log row are the integrator's obligation (see §1, Independence note).