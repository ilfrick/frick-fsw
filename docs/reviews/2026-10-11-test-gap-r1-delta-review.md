# Review Report — Independent R1 delta re-review (norm N12) of the FRICK-REVIEW-0077 findings

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0079                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior SW development engineer)    |
| Reference revision                | `e0c7ad8` (branch `main`), 2026-10-11  |
| Items under review                | The fix for FRICK-REVIEW-0077: `src/core/tbl/tbl.c` (R1-01 Major), `src/pus/pus_services.c` + `tests/unit/test_r2_gap.c` (R1-02), `tests/CMakeLists.txt` (R1-03), `docs/requirements/traceability.yaml` (R1-04), and the new regression in `tests/unit/test_r3_gap.c` |
| Date                              | 2026-10-11                             |
| Scope                             | Norm N12 delta re-review: re-apply the rules to the affected scope and re-verify each closed/fixed finding (R1-01…R1-04) by independent reproduction at HEAD, plus re-run the project gates (build, `ctest`, `trace.py`, `clang-format`). |
| Independence declaration          | Reviewer authored, integrated and executed none of the reviewed fix, tests or records; fresh session with no author context. Only this report file was created; no other tracked file was modified (probes and worktrees written under the git-ignored/temporary `/tmp/opencode`). |
| Norms applied                     | N1, N3, N4, N6, N9, N10, N11, N12, N15, N16 (see `docs/rules.md`) |

## 1. Scope

This is the norm-N12 delta re-review that closes the loop on the R1 review
FRICK-REVIEW-0077 (`docs/reviews/2026-10-11-test-gap-r1-review.md`, verdict
REJECTED — 1 Major + 3 Minor + 2 Comment) whose findings were fixed at HEAD
`e0c7ad8` ("Fix TBL validation bypass (Major) + make new tests order-independent
(R1 findings)").

**Included.** Independent re-verification, at `e0c7ad8`, of each R1 finding:

- **R1-01 (Major, `src/core/tbl/tbl.c`)** — reproduce the original public-API
  probe (good load → activate → bad load → activate) and confirm the active
  image is unchanged and `FF_TblActivate` returns `FF_NOT_READY`; confirm the
  new regression in `tests/unit/test_r3_gap.c` is **discriminating** by reverting
  the one-line fix in a throwaway worktree.
- **R1-02 (Minor, `test_r2_gap.c`)** — confirm `FF_PusRegisterCoreServices`
  resets the registries (`FF_PusInit` idempotent) and that each PUS routine
  registers its own providers; run `test_r2_gap` in isolation and in the full
  `ctest`; **independently** isolate every routine in the suite.
- **R1-03 (Minor, `tests/CMakeLists.txt`)** — build with `-DFF_BUILD_HAL=OFF`.
- **R1-04 (Minor, `traceability.yaml`)** — confirm the over-traced evidence links
  are removed.

**Excluded.** The technical content of `src/` beyond the branches these findings
touch; the Lean proofs; the process findings of the companion PA review
FRICK-REVIEW-0078 (owned by N20); on-target/flight-hardware evidence (GAP-A5);
and the two original task reports FRICK-REVIEW-0075/0076 (already reviewed).

## 2. References

- Project norms: `docs/rules.md` (N1–N20), especially N4, N6, N9, N11, N12, N15.
- Prior review: FRICK-REVIEW-0077 (this delta closes its findings).
- Companion: FRICK-REVIEW-0078 (PA, norm N20).
- Traceability manifest: `docs/requirements/traceability.yaml`.
- Harness: `tests/ff_test.h`; harness contract `FF_TEST_ASSERT` / `FF_TEST_MAIN`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025); ECSS
  criticality B target.

## 3. Verification performed (independent, reproducible, at `e0c7ad8`)

| # | Check | Command / method | Result |
| - | ----- | ---------------- | ------ |
| D1 | Build | `cmake --build build/native` | **PASS**, no work pending; clean reconfigure/rebuild from scratch also **PASS** with 0 warnings |
| D2 | Full suite | `ctest --test-dir build/native --output-on-failure` | **16/16 PASS** (incl. `r2_gap`, `r3_gap`) |
| D3 | Check counts | `./build/native/tests/test_r2_gap`; `…/test_r3_gap` | **487 / 0** and **201 / 0** — `r3_gap` grew 198→201 by the +3-check regression |
| D4 | Trace gate (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (96 reqs / 55 srs / 27 sss / 14 gaps / 51 Lean theorems) |
| D5 | Style | `clang-format --dry-run --Werror` on the four changed C files | **clean** (exit 0) |
| D6 | **R1-01** public-API probe | standalone `FF_Tbl{Register,Load,Activate}` probe linked to HEAD's `libfrick_fsw_core.a` | **PASS**: bad load → `FF_ERROR`; subsequent activate → `FF_NOT_READY`; active image unchanged (`magic=0x52334346 gain=100`) |
| D7 | **R1-01** discrimination | throwaway worktree, one-line fix removed, rebuilt | **FAILS as required**: activate after bad load → `FF_SUCCESS`; committed image `magic=0xBAD0BAD0 gain=999`; 3 assertions fail in `test_r3_gap` |
| D8 | **R1-02** isolation (suite) | `ctest --test-dir build/native -R '^r2_gap$'` | **1/1 PASS** |
| D9 | **R1-02** isolation (per routine) | every routine recompiled as a standalone `FF_TEST_MAIN` (21 routines) | **all 21 isolate clean** — incl. the 3 formerly failing (`S3` 19, `S8` 9, `S20` 11 checks) |
| D10 | **R1-02** idempotence | `FF_PusInit` → `FF_PusRegisterCoreServices` reset path | confirmed: registries zeroed at init (`pus_services.c:423-431`); sole caller is `FF_PusInit` (`pus.c:38`) |
| D11 | **R1-03** HAL-off | `cmake -DFF_BUILD_HAL=OFF … && cmake --build` then `ctest` | **PASS**: links; **15/15 PASS**; `FF_TEST_NO_HAL` present in `build.ninja`; `test_r2_gap` = 474 checks |
| D12 | **R1-04** links | `grep` of the manifest for the three over-traced links | removed (see §4 R1-04) |

No file under review was executed from the authoring session; all probes were
compiled/linked here against the HEAD objects.

## 4. Findings — delta verdict per FRICK-REVIEW-0077 NC

| R1 NC | Severity | Verdict | Evidence (at `e0c7ad8`) |
| ----- | -------- | ------- | ----------------------- |
| **R1-01** | Major | **CLOSED — fixed and verified** | `FF_TblLoad` now sets `g_tables[h].staged = false` on validator rejection (`src/core/tbl/tbl.c:71`), so a rejected load after a successful one can no longer be activated. Probe D6 at HEAD returns `FF_NOT_READY` and leaves the active image unchanged; reverting only that one line (D7) makes the active image the rejected `0xBAD0BAD0` and the new regression fail (3 assertions at `tests/unit/test_r3_gap.c:782,785,793`). The regression **is discriminating**. |
| **R1-02** | Minor | **CLOSED — fixed and verified** | `FF_PusRegisterCoreServices` now zeroes the six registries (`src/pus/pus_services.c:423-431`), and `FF_PusInit` calls it (`src/pus/pus.c:38`), so init is idempotent; `test_r2_gap.c` gained `pus_register_all()` (`:660`) invoked by `test_pus_service3_hk`/`_service8_function`/`_service20_parameters` (`:734,776,804`). D8 (suite isolated), D9 (all 21 routines isolated), and D2 (full `ctest`) all pass. The header claim "no test-order dependency (each routine re-initialises the module it exercises)" (`test_r2_gap.c:10-12`) is now **true** — verified by exhaustive per-routine isolation. |
| **R1-03** | Minor | **CLOSED — fixed and verified** | `tests/CMakeLists.txt:12-14` defines `FF_TEST_NO_HAL` for `test_r2_gap` when `FF_BUILD_HAL` is off, matching the R2 report's instruction. D11: the HAL-off build links with no `FF_Hal*` undefined references and `ctest` is 15/15 (HAL test correctly gated out). |
| **R1-04** | Minor | **CLOSED — fixed and verified** | `tests/unit/test_r2_gap.c` was removed from the evidence of SRS-REQ-050 (`traceability.yaml:476` → `["tests/unit/test_pus.c","ci:renode"]`), SRS-REQ-041 (`:395` → `["ci:misra"]`) and SSS-REQ-010 (`:589` → `["ci:static-analysis"]`). No NEW over-trace was introduced; the remaining `test_r2_gap.c` links (SRS-REQ-001/005/011/013/014/025; SSS-REQ-002/005/006/009) all map to module families the suite demonstrably exercises. `trace.py` PASS (D4). |

**Assessment of the R1-01 fix itself (does it fully satisfy the recommendation?).**
The recommendation was to *invalidate the staged image on validation failure* and
to *clear `staged` after a successful `FF_TblActivate`*. The first (the
safety-critical part) is implemented. The second was **not** implemented:
`FF_TblActivate` still leaves `staged == true` after a successful commit
(`src/core/tbl/tbl.c:87-91`). This is not a defect in the sense of the original
finding (an idempotent re-activate of already-committed staging data is
harmless, and any subsequent rejected load now clears `staged`), but the
`staged` flag no longer strictly means "an uncommitted validated image awaits
activation". Recorded as REC-79-03 (Comment) for completeness / future
hardening.

**New observations on the re-applied scope (none defeats a requirement):**

| # | Severity | Item | Finding | Evidence | Recommendation |
| - | -------- | ---- | ------- | -------- | -------------- |
| REC-79-01 | **Minor** | `docs/reviews/README.md` (N11/N16) | The fix commit added the reports `docs/reviews/2026-10-11-test-gap-r1-review.md` (0077) and `2026-10-11-test-gap-pa-review.md` (0078) but **no rows were appended to the review log** (`docs/reviews/README.md` last rows are for FRICK-REVIEW-0075/0076). Both reports explicitly deferred the log row to the integrator, which this commit is; N11 requires the review results to be logged with the cycle's change. | `git show e0c7ad8 --stat` (reports added); `grep -n '0077\|0078' docs/reviews/README.md` = none; last log entry = the 0075/0076 task reports | Append the two review-log rows to `docs/reviews/README.md` (and the 0079 row for this delta) in the same cycle (N11). Mirrors the PA-78-04 pattern already accepted for the cycle log. |
| REC-79-02 | Comment | `CHANGELOG.md:27` (N16) | The `### Added` bullet still states `test_r3_gap.c` (**198 checks**), while the fix bumped the suite to **201** (recorded correctly in `verification-results.md:37` and `tool-qualification.md` 0.3.8). The CHANGELOG number is now stale. | `CHANGELOG.md:27` vs measured 201 (D3); `verification-results.md:37` | Correct the figure or mark it as the pre-fix count. |
| REC-79-03 | Comment | `src/core/tbl/tbl.c:71,75,87-91` | (a) The new `staged` write on rejection (`:71`) is outside `g_lock`, as are the existing writes (`:75`, and the read at `:84`) while the `memcpy` is inside the lock; (b) `FF_TblActivate` still does not clear `staged` on success, so the R1-01 recommendation is only partially implemented. Neither currently defeats a requirement (single-threaded today; re-activate is idempotent). | `src/core/tbl/tbl.c` read in full | For robustness/harmonisation, guard the `staged` flag with the mutex and clear it after a successful activate; otherwise document the intended semantics. No re-review blocking. |

No finding above is Major; none defeats a requirement or a safety barrier.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — R1-01 Major closed; R1-02/R1-03/R1-04 closed; 1 Minor + 2 Comments to track**
- [ ] REJECTED — one or more Major findings

**Rationale.** The FRICK-REVIEW-0077 **Major (R1-01) is fixed and independently
verified**: the original bypass probe now yields the correct `FF_NOT_READY` with
an unchanged active image, and the new regression is *discriminating* (reverting
the one-line fix reproduces the defect and fails the test). R1-02 (order
dependence), R1-03 (`FF_BUILD_HAL=OFF` link regression) and R1-04 (over-traced
evidence links) are also closed with reproduced evidence; the associated gates
are green (build, 16/16 `ctest`, `trace.py` PASS, `clang-format` clean). The
remaining items are a records omission (review-log rows, REC-79-01 Minor) and
two low-risk Comments; none invalidates the fix or a requirement.

**FRICK-REVIEW-0077 status:** **CLOSED** — the single Major (R1-01) is fixed and
verified; R1-02/R1-03/R1-04 are closed; R1-05 (Comment, `FF_PusInit` idempotence)
is *also* now addressed by this fix; R1-06 (Comment, invalid-input return-code
asymmetry) remains a standing observation, not a blocker.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Senior SW dev engineer (fresh context) | 2026-10-11 | APPROVED WITH COMMENTS (Major closed; 1 Minor + 2 Comments) |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `src/core/tbl/tbl.c` / SRS-REQ-013, SSS-REQ-009 | validation bypass on failed-load-after-success | clear `staged` on rejection; add regression | probe D6 PASS; regression discriminating D7; `r3_gap` 201/0 | FSW | **closed** | FRICK-REVIEW-0079 |
| R1-02 | Minor | `tests/unit/test_r2_gap.c` | order-dependent; header claim false | reset registries in `FF_PusInit`; self-register providers | 21/21 routines isolate clean (D9); full+isolated `ctest` PASS | FSW/V&V | **closed** | FRICK-REVIEW-0079 |
| R1-03 | Minor | `tests/CMakeLists.txt` / `test_r2_gap.c` | `FF_BUILD_HAL=OFF` link regression | gate HAL section | HAL-off build PASS; 15/15 (D11) | FSW | **closed** | FRICK-REVIEW-0079 |
| R1-04 | Minor | `traceability.yaml` SRS-REQ-050/041, SSS-REQ-010 | over-traced evidence links | remove link | links gone (`:476`, `:395`, `:589`); `trace.py` PASS | FSW | **closed** | FRICK-REVIEW-0079 |
| R1-05 | Comment | `src/pus/pus.c` | `FF_PusInit` did not reset service registries | reset or document | registries reset (`pus_services.c:423-431`); `pus.c:38` | FSW | **closed** | FRICK-REVIEW-0079 |
| R1-06 | Comment | `tbl.c` / `pal_posix.c` | inconsistent invalid-input return codes | harmonise or document | code review (not changed by the fix) | FSW | open | standing observation |
| REC-79-01 | Minor | `docs/reviews/README.md` (N11/N16) | no review-log rows for 0077/0078 | append log rows | log re-read | FSW/integrator | open | this report |
| REC-79-02 | Comment | `CHANGELOG.md:27` (N16) | `test_r3_gap` count stale (198 vs 201) | correct the figure | record re-read | FSW | open | this report |
| REC-79-03 | Comment | `src/core/tbl/tbl.c` | `staged` flag not mutex-guarded; activate does not clear `staged` on success | harmonise/document | code review | FSW | open | this report |

## 8. Confidence

```text
Decision/product and configuration: frick-fsw @ e0c7ad8 (HEAD, main); ECSS criticality B target.
Claim type: mixed (Fact for gate results, probes and file contents; Inference for the residual
  severity of REC-79-01/03).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1 (30 Apr 2025),
  ECSS-Q-ST-80C Rev.2 (30 Apr 2025). Customer tailoring matrix not available to this reviewer.
Evidence consulted: git diff of e0c7ad8; src/core/tbl/tbl.c; src/pus/pus.c; src/pus/pus_services.c;
  tests/unit/test_r2_gap.c; tests/unit/test_r3_gap.c; tests/CMakeLists.txt; traceability.yaml;
  CHANGELOG.md; verification-results.md; tool-qualification.md; docs/reviews/README.md. Live gates:
  build, ctest (full, isolated, HAL-off), trace.py, clang-format. Independent probes: public-API TBL
  probe at HEAD and on the reverted worktree; 21 per-routine isolation builds.
Cross-check and reconciliation: R1-01 fix vs the original FRICK-REVIEW-0077 probe (defect present ->
  absent); CHANGELOG/verification-results/tool-qualification counts reconciled to measured 201.
Confidence: High for the closure of R1-01..R1-05 and for R1-03/R1-04 (deterministically reproduced).
  Medium for REC-79-01 (process/integration obligation, may be pending a separate integrator step).
Basis and detection limits: all files under review read in full; gates executed; the defect probe and
  its inverse are deterministic. Stack/timing, concurrency and on-target behaviour not assessed (out of
  scope); the mutex observation (REC-79-03a) is reasoned from the source, not stress-tested.
Missing/inaccessible evidence: customer tailoring matrix; certified MISRA checker; RTEMS cross/on-target
  run (not applicable to a fix+tests+records delta).
Assumptions and expiry/recheck date: assumes e0c7ad8 is the merge baseline and that the only intended
  change in scope is the FRICK-REVIEW-0077 fix. Recheck on any change to tbl.c, pus_services.c, the two
  suites, tests/CMakeLists.txt or the manifest. Recheck by 2027-04-11.
Required independent human reviewer/authority: project engineering/PA/release authority; this agent
  cannot approve, close NCs, or accept residual risk.
```

---

Committed and pushed with the reviewed change (norm N11). The
`docs/reviews/README.md` review-log row and the `docs/vv/verification-results.md`
cycle-log row for FRICK-REVIEW-0079 are the integrator's obligation and are not
written here (see REC-79-01, which also covers the still-missing 0077/0078 rows).