# Review Report — Product Assurance review of the R2/R3 test-gap increment (norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0078                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | `8f62746` (branch `main`), 2026-10-11  |
| Items under review                | `tests/unit/test_r2_gap.c`, `tests/unit/test_r3_gap.c`, `tests/CMakeLists.txt`, `docs/requirements/traceability.yaml` (tests evidence), `docs/vv/tool-qualification.md` (T3), `docs/vv/verification-results.md` (0.25), `CHANGELOG.md`, `docs/reviews/README.md`, `docs/reviews/2026-10-11-test-coverage-r2-report.md` (FRICK-REVIEW-0075), `docs/reviews/2026-10-11-requirements-test-gap-r3-report.md` (FRICK-REVIEW-0076) |
| Date                              | 2026-10-11                             |
| Scope                             | Process obligations (N15 full cycle, N4 trace convergence, N10 tool register, N11 records); honesty of the coverage/closure claims; whether the new tests actually improved measured coverage; consistency of the T3 register update; any record that overstates. |
| Independence declaration          | Reviewer took no part in authoring, integrating or executing the reviewed test suites, the manifest edit, the T3 register edit or the records; fresh independent session with no prior author context. Read-only for the repository: only this report file was created. `make coverage`/`make trace`/`ctest`/`metrics.py` were executed read-only (their outputs live under the git-ignored `build/`); no tracked file was modified. |
| Norms applied                     | N4, N6, N8, N9, N10, N11, N12, N15, N16, N17, N20 (see `docs/rules.md`) |

## 1. Scope

This is the norm-N20 Product Assurance review of the test-gap closure increment
delivered at HEAD `8f62746`: the two new host suites
(`tests/unit/test_r2_gap.c`, `tests/unit/test_r3_gap.c`), their CMake
registration, the `traceability.yaml` tests-evidence extension, the tool register
entry T3, and the associated records (verification-results 0.25, CHANGELOG,
review log, and the two task reports FRICK-REVIEW-0075/R2 and 0076/R3).

**Included:** the process obligations the increment triggers, the factual
accuracy of its coverage and closure claims, and whether any record overstates
the evidence produced.

**Excluded:** the technical content of the `src/` modules under test and the
Lean proofs (owned by their own reviews); the exact per-module correctness of
the assertions beyond what the gates exercise; on-target behaviour (GAP-A5);
the review *reports* FRICK-REVIEW-0075/0076 are separately treated from the
*increment* that integrated them (this review does not re-perform R2/R3 review).

**Method.** Read the commit diff and every artifact in scope; executed the
project gates independently; regenerated the coverage metric with the new
suites registered; compared the recorded numbers against the measured ones; and
checked the traceability evidence links against the actual test sources.

**Independence note (log row).** Per the task instruction this review modifies
nothing but this report file. The `docs/reviews/README.md` review-log row and
the `verification-results.md` cycle-log row for FRICK-REVIEW-0078 are therefore
**not** written here and remain the integrator's obligation (norm N11/N17). This
is stated so the omission below is intentional and tracked, not a silent gap.

## 2. References

- Project norms: `docs/rules.md` (N1–N20), especially N4, N10, N11, N15, N20.
- Traceability manifest: `docs/requirements/traceability.yaml`.
- Tool register: `docs/vv/tool-qualification.md` (T2, T3, T13).
- Records: `docs/vv/verification-results.md` (0.25), `CHANGELOG.md`,
  `docs/reviews/README.md`.
- Reports under review: FRICK-REVIEW-0075 (`2026-10-11-test-coverage-r2-report.md`),
  FRICK-REVIEW-0076 (`2026-10-11-requirements-test-gap-r3-report.md`).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025) 5.8.3.x (traceability), ECSS-Q-ST-80C Rev.2
  (30 Apr 2025) 6.2.6.12a (traceability matrices verified at each milestone) and
  6.3.x (product assurance records).

## 3. Verification performed (independent)

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 96 requirements (55 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims (as expected) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** ("warning: README code-coverage rows differ … cross-compiler drift") |
| Unit suite (N6) | `ctest --test-dir build/native` | **16/16 PASS** (incl. `r2_gap`, `r3_gap`) |
| Suite check counts | `./build/native/tests/test_r2_gap`, `test_r3_gap` | **487 checks / 0 failures**, **198 checks / 0 failures** — matches both reports |
| Coverage (measured) | `make coverage` | lines **2811/3938 (71.4%)**, functions **370/375 (98.7%)**; product-only (`src/`) functions **228/233 (97.9%)**, lines 1803/1999 (90.2%) |
| Residual uncovered product functions | gcov scan of `src/**` | exactly **5** remain: `gem_set_link_up`, `n7_decode_tc`, `FF_PalShutdown`, `FF_PalPanic`, `netloop_attach_netif` — precisely the residuals documented in FRICK-REVIEW-0075 §6 (GAP-R2-1/2/3/4) |

**Coverage-claim honesty (assessment).** The R2 report's robust per-function
claim ("all baseline `called 0` functions in G1–G11 are now executed; only the
five functions in §6 remain at zero") is **confirmed independently** — the five
residuals are exactly those documented. The report's *aggregate* figures
(functions 305/310, lines 2294/2985) differ from the authoritative numbers
(370/375, 2811/3938) because the report used an informal merged-gcov pass with a
different denominator; the report **explicitly caveats** this and names
`make coverage` as authoritative. This is honest; the aggregate claim is not
overstated.

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | SRS-REQ-050 (N7/ESA ASN.1 PUS backend) | `tests/unit/test_r2_gap.c` was added to the requirement's `tests:` evidence although the file contains **no reference to the N7 adapter or ASN.1 backend** (`grep n7|adapter tests/unit/test_r2_gap.c` = none). FRICK-REVIEW-0075's own GAP-R2-4 states the N7 backend is *not built* in the coverage configuration. The evidence link is therefore unsupported (over-tracing). | `docs/requirements/traceability.yaml:476`; `test_r2_gap.c` (no N7 symbol); FRICK-REVIEW-0075 §6 GAP-R2-4 | Remove `tests/unit/test_r2_gap.c` from SRS-REQ-050, or justify explicitly (fr. it is not evidence of the N7 backend). Keep the existing `test_pus.c` + `ci:renode` evidence. |
| 2 | Minor | SRS-REQ-041 (MISRA C:2012); SSS-REQ-010 (deterministic memory) | `tests/unit/test_r2_gap.c` was added as *test evidence* for requirements it does not verify: SRS-REQ-041's evidence is a static scan (`ci:misra`), and the new file is merely an *input* to that scan, not a verification of MISRA compliance. Same pattern for SSS-REQ-010 (`ci:static-analysis`). This inflates the tests-evidence list with a file that does not substantiate the requirement. | `traceability.yaml:395` (SRS-REQ-041), `:589` (SSS-REQ-010); `docs/quality/misra-c.md`; `test_r2_gap.c` contains no MISRA/deterministic-memory assertion | Revert these two additions or state the convention (scanned artefact ≠ test evidence) explicitly; a test file should list only requirements whose behaviour it exercises. (SSS-REQ-010 is nearest-justified because `test_r2_gap.c` does exercise PAL runtime paths — record that rationale rather than leaving it implicit.) |
| 3 | Minor | README metrics panel (N8/N11) | The commit refreshed the **completion** rows but **not the code-coverage rows**: the panel still reads lines 1917/2920 (65.7%) and functions 257/302 (85.1%), while the measured coverage after this increment is lines 2811/3938 (71.4%) and functions 370/375 (98.7%). The currency gate did not catch it because `metrics.py --check` **deliberately ignores** the code-coverage rows (`_normalize` drops them), emitting only a warning. The panel is thus stale (it *understates* the improvement). | README.md:65-66; measured `build/coverage/coverage.json`; `tools/scripts/metrics.py:321-345`; `python3 tools/scripts/metrics.py --check` → "warning: … code-coverage rows differ" | Run `make metrics` (regenerates `coverage.json` then rewrites the panel) as part of the cycle, and record the fresh code-coverage rows. Consider making the CI currency gate fail on coverage-row drift when the coverage report is regenerated in the same job. |
| 4 | Minor | `docs/vv/verification-results.md` cycle log (N11/N15) | The record was bumped to 0.25 and gained the two review rows and the history line, but the **Cycle log table gained no row** for this change: the build/ctest/Lean/trace/smoke/static/RTEMS gate results for `8f62746` are not recorded (the last row is the M11 NET change). N11 requires the verification results to be updated every cycle; the gate outcomes are the substance of that record. | `verification-results.md:18-36` (no row for this increment; ends at M11 NET); `git show 8f62746 -- docs/vv/verification-results.md` (only revision/table/history) | Add a cycle-log row: build PASS, ctest 16/16 (r2_gap 487 + r3_gap 198 checks), Lean n/a-unchanged, trace PASS 96/55/27/14/51, smoke/static per the actual run, RTEMS cross proxy/n-a. |
| 5 | Minor | N9/N10 (independent review of the new V&V scripts) | The two reports that approve the new suites were written by the **same reviewers who authored the suites**. FRICK-REVIEW-0075's independence declaration says "Reviewer wrote … none of the reviewed test infrastructure" yet the reviewed set ("`tests/unit/*.c`") and the report's own deliverable include `test_r2_gap.c`; likewise R3 and `test_r3_gap.c`. So the new V&V test scripts (tool T3) have **no independent source review** — only a self-review — even though T3's register entry claims "reviewed + CM". | FRICK-REVIEW-0075 header/§9; FRICK-REVIEW-0076 header/§9; `tool-qualification.md` T3 | Have the new suites source-reviewed by an agent/reviewer independent of their author (or by the FSW integrator), and record it under T3 (N10). State plainly in each report which artefact is the reviewer's own work. |
| 6 | Comment | `verification-results.md` 0.25 history (N16) | The 0.25 history entry appends "…; SDD section 6 design matrix (PA-67-01)" but this change does **not** modify the SDD and does not close NC PA-67-01 (still open, Sprint 0/1). The phrase suggests a closure that did not occur. | `verification-results.md:387`; `git show 8f62746 --stat` (no SDD file); `docs/reviews/2026-10-11-ecss-traceability-pa-review.md` (PA-67-01 open) | Remove the clause or replace it with the accurate statement (e.g. "manifest tests-evidence updated; PA-67-01 remains open"). |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 5. Process-obligation assessment (asked explicitly)

- **N15 (full cycle).** Build and `ctest` 16/16 pass; the change touches only
  `tests/` + records, so Lean/RTEMS/smoke are not candidate gates (no verified
  code changed). The cycle **execution** is credible, but its **record** is
  incomplete (Finding 4). Partially met.
- **N4 (trace convergence).** Verified PASS 96/55/27/14/51. The manifest edit
  wires the new suites to the right requirement families, with two over-traced
  links (Findings 1–2). Mostly met.
- **N10 (tool register).** T3 correctly lists the two new files and the history
  entry 0.3.7 records the change — consistent. The residual gap is the
  independence of the source review (Finding 5), which T3's "reviewed + CM"
  claim assumes.
- **N11 (records).** Reports, CHANGELOG, review-log rows and the verification
  record are committed in the same cycle — good. The verification cycle-log row
  is missing (Finding 4) and the README coverage rows are stale (Finding 3).
- **Honesty of coverage/closure claims.** The suites do **improve** measured
  coverage (all products `called 0` closed except the 5 documented residuals);
  the reports do not overstate this and caveat their aggregate figures. The
  overstatements found are narrow: two traceability evidence links (Findings
  1–2) and one history clause (Finding 6). No false claim of defect-freedom or
  qualification was found.

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — 5 Minor findings tracked to closure, 1 Comment**
- [ ] REJECTED — one or more Major findings

No finding defeats a requirement or a safety barrier, so none is Major. The
increment delivers genuine, independently confirmed coverage improvement and
honest closure reporting; the defects are record-accuracy and evidence-link
issues (N4/N8/N9/N10/N11/N16) that must be corrected or justified under norm
N12 before the records can be considered converged.

## 7. Confidence

```text
Decision/product and configuration: frick-fsw @ 8f62746; ECSS criticality B target.
Claim type: mixed (Fact for gate results and file contents; Inference for process verdict)
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1
  (30 Apr 2025) 5.8.3.x; ECSS-Q-ST-80C Rev.2 (30 Apr 2025) 6.2.6.12a/6.3.x. Customer
  tailoring matrix not available to this reviewer (unchanged from PA-67 assessment).
Evidence consulted: commit diff 8f62746; test sources; traceability.yaml; tool-qualification.md;
  CHANGELOG.md; docs/reviews/README.md; verification-results.md; live gates (trace.py,
  metrics.py, ctest, make coverage); gcov reports under build/coverage/gcov.
Cross-check and reconciliation: measured coverage vs README panel vs report claims;
  trace evidence links vs actual test symbols; delivered-metric delta (SRS 49->50,
  SSS 21->22) reconciled to SRS-REQ-052 / SSS-REQ-025.
Confidence: High for the findings (direct file and gate evidence, reproduced locally).
Basis and detection limits: full diff and all in-scope records read; gates executed;
  coverage regenerated. The "no independent review" finding rests on the reports'
  own authorship/independence declarations, read as written.
Missing/inaccessible evidence: customer tailoring matrix; the RTEMS cross/on-target
  results for this increment were not re-run (not applicable to a tests/ + records change).
Assumptions and expiry/recheck date: assumes the task instruction to modify only this
  report is authoritative; recheck the review-log/cycle-log rows are added by the
  integrator. Recheck on next baseline change or 2027-04-11.
Consequences if wrong: if the over-traced links are intended by a project convention,
  Findings 1-2 downgrade to Comments; the coverage/staleness findings are independent of that.
Required independent human reviewer/authority: project engineering/PA/release authority;
  this agent cannot approve, close NCs, or accept residual risk.
```

## 8. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) | — | | |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0078** | **2026-10-11** | **APPROVED WITH COMMENTS (5 Minor, 1 Comment)** |

## 9. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-78-01 | Minor | SRS-REQ-050 / `traceability.yaml:476` | `test_r2_gap.c` linked as evidence for the N7 ASN.1 backend although it contains no N7 assertions | Remove link or justify | manifest re-read; `trace.py` PASS | frick-fsw | open | pending |
| PA-78-02 | Minor | SRS-REQ-041 / SSS-REQ-010 | unit test listed as evidence for MISRA/static-analysis requirements it does not verify | Revert or document the convention | manifest re-read | frick-fsw | open | pending |
| PA-78-03 | Minor | README metrics panel (N8/N11) | code-coverage rows stale (1917/2920, 257/302) vs measured 2811/3938, 370/375; `--check` ignores coverage rows | `make metrics`; record fresh rows | README panel diff | frick-fsw | open | pending |
| PA-78-04 | Minor | `verification-results.md` cycle log (N11/N15) | no cycle-log row for the increment's gate results | Add the row | record re-read | frick-fsw | open | pending |
| PA-78-05 | Minor | N9/N10 (T3 scripts) | new suites self-reviewed; no independent source review | Independent review of `test_r2_gap.c`/`test_r3_gap.c`; record under T3 | review report + T3 entry | frick-fsw / V&V | open | pending |
| PA-78-06 | Comment | `verification-results.md:387` (N16) | 0.25 history clause implies PA-67-01 handled; it is not | Correct the clause | record re-read | frick-fsw | open | pending |

---

Committed and pushed with the reviewed change (norm N11). The review-log row and
the verification-results cycle-log row for FRICK-REVIEW-0078 are to be added by
the integrator (see §1, Independence note).