# Review Report — Independent R1 review of the R2/R3 test-gap suites

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0077                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior SW development engineer)    |
| Reference revision                | `8f62746` (branch `main`), 2026-10-11  |
| Items under review                | `tests/unit/test_r2_gap.c`, `tests/unit/test_r3_gap.c`, `tests/CMakeLists.txt` (registration), `docs/requirements/traceability.yaml` (tests-evidence update), and the reports FRICK-REVIEW-0075 (`docs/reviews/2026-10-11-test-coverage-r2-report.md`) and FRICK-REVIEW-0076 (`docs/reviews/2026-10-11-requirements-test-gap-r3-report.md`) |
| Date                              | 2026-10-11                             |
| Scope                             | Build/pass in the real build, direct check counts, meaningfulness (non-tautology, error paths, no undefined order), genuine gap closure (EVS/ES, SB saturation/drop, TBL revert, PUS S3/S20, S1 codes, TM counter wrap), clang-format/MISRA orientation, and any new `src/` defect the tests expose. |
| Independence declaration          | Reviewer authored, integrated and executed none of the reviewed test suites, the CMake edit, the manifest edit or the reports; fresh session with no author context. Only this report file was created; no other tracked file was modified (builds written under git-ignored `/tmp/opencode` and `build/`). |
| Norms applied                     | N1, N3, N4, N5, N6, N9, N10, N11, N15 (see `docs/rules.md`) |

## 1. Scope

This review independently re-verifies the two test suites added by the R2/R3
test-gap increment at HEAD `8f62746`, their registration and their traceability
manifest evidence, and re-checks the factual claims of the two task reports.

**Included.** (a) A clean CMake configure + build and a full `ctest`; (b) direct
execution of the two suites to confirm the reported check counts; (c)
meaningfulness of the assertions (public-API exercise, real return-code/behaviour
checks, real negative paths, absence of undefined-order reliance); (d) spot
closure of the claimed gaps; (e) `clang-format --dry-run --Werror` and a
`cppcheck --addon=misra` scan of the two files in the pinned `frick-vv:local`
image; (f) independent probing of the `src/` modules under test.

**Excluded.** On-target/flight-hardware evidence (GAP-A5); third-party
components; the Lean proofs; the *process* obligations of the increment
(covered by the Product Assurance review FRICK-REVIEW-0078); and the correctness
of `src/` beyond the branches the new tests claim to close. The manifest
update is checked for evidence-link accuracy, not for a full N4 re-convergence.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Reports under review: FRICK-REVIEW-0075, FRICK-REVIEW-0076
- Independent companion: FRICK-REVIEW-0078 (PA, norm N20)
- Harness: `tests/ff_test.h`; harness contract `FF_TEST_ASSERT` / `FF_TEST_MAIN`
- ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Verification performed (independent, reproducible)

| # | Check | Command / method | Result |
| - | ----- | ---------------- | ------ |
| V1 | Clean build (default configs) | `cmake -S . -B /tmp/opencode/ffreview -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build …` | **PASS**, 0 warnings/errors |
| V2 | Full suite | `ctest --test-dir build/native --output-on-failure` | **16/16 PASS** (incl. `r2_gap`, `r3_gap`) |
| V3 | Check counts | `./build/native/tests/test_r2_gap`; `…/test_r3_gap` | **487 checks / 0 failures**; **198 checks / 0 failures** — matches both reports exactly |
| V4 | Config variants | `FF_USE_LIBM=OFF` build+ctest; `-DFF_TEST_NO_HAL=1` compile+run; `FF_USE_LIBM=0` compile+run | 15/15 PASS; r2_gap = **474** / **483** checks — matches FRICK-REVIEW-0075 §5 |
| V5 | Style | `clang-format --dry-run --Werror tests/unit/test_r2_gap.c tests/unit/test_r3_gap.c` (frick-vv:local) | **clean** |
| V6 | MISRA orientation | `cppcheck --addon=misra … tests/unit/test_r2_gap.c tests/unit/test_r3_gap.c` (frick-vv:local) | **0 findings** |
| V7 | Trace gate | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** 96/55/27/14 |
| V8 | Meaningfulness probes | recompile each PUS routine as a standalone `FF_TEST_MAIN` (r2 + r3) | r3 routines **isolate clean**; **3 r2 routines fail in isolation** (see R1-02) |
| V9 | Public-API defect probe | minimal program against `FF_Tbl_{Register,Load,Activate}` | **confirms a validation-bypass defect in `tbl.c`** (see R1-01) |

All findings below are supported by direct file/gate evidence.

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | **Major** | `src/core/tbl/tbl.c`; verified by `test_r2_gap.c` / `test_r3_gap.c`; SRS-REQ-013 / SSS-REQ-009 | **Table validation is bypassed by a failed load after any prior successful load.** `FF_TblLoad` copies `src` into the staging buffer **before** validating (`tbl.c:66`) and, on validator rejection, returns `FF_ERROR` **without clearing `g_tables[h].staged`** (`tbl.c:69-72`). `FF_TblActivate` then commits the *validator-rejected* staging image when `staged` is still true (`tbl.c:83,87`), and itself never clears `staged`. A ground/ops load that fails validation can therefore be activated into the live image. The new suites were added specifically to close the TBL validate/revert gap but both only test "invalid load when nothing is staged" (`test_r2_gap.c:350-353`, `test_r3_gap.c:741-744`), so they **miss the branch and mask the defect**. | Probe: load-good → activate → load-bad (`FF_ERROR`) → activate returns `FF_SUCCESS` and `active.magic=0xBAD0BAD0` instead of `FF_NOT_READY`/unchanged; `src/core/tbl/tbl.c:57-90`; failing branch absent from `tests/unit/test_r2_gap.c:330-374` and `tests/unit/test_r3_gap.c:724-783`; `tests/unit/test_tbl.c` happy-path only | In `FF_TblLoad`, validate a temporary copy (or validate before publishing to staging) and **clear `staged`** on validation failure; clear `staged` after a successful `FF_TblActivate`. Add a regression test asserting that an invalid load after a successful load leaves the active image unchanged and the next `FF_TblActivate` returns `FF_NOT_READY`. Re-run N1/N3/N4 and re-review. Owner: FSW. |
| 2 | Minor | `tests/unit/test_r2_gap.c` (claim at line 12); N6/N10 | **The suite is order-dependent, contradicting its own header claim.** `FF_PusInit()` resets only `pus.c` state (`pus.c:29-41`); it does **not** reset the `pus_services.c` registries (`pus_services.c:63-68`). `test_pus_service3_hk`, `test_pus_service8_function` and `test_pus_service20_parameters` therefore rely on the HK provider / functions / param setter registered earlier by `test_pus_core_gaps`. Run in isolation they fail. The file states "no test-order dependency (each routine re-initialises the module it exercises)" (`test_r2_gap.c:10-12`), which is false. | Standalone probes against the shipped object: `test_pus_service3_hk` → 3 failures; `test_pus_service8_function` → 5 failures; `test_pus_service20_parameters` → 5 failures (`pus_ingest(...) == FF_SUCCESS` etc.). Fixed registered order passes only because of leaked cross-test state. | Make each routine self-contained: register its providers/setters/functions inside the routine (as `test_r3_gap.c` already does), or add a reset that `FF_PusInit` honours. Correct the header comment. |
| 3 | Minor | `tests/CMakeLists.txt:11,55` + `test_r2_gap.c:453-479,951`; N6/N15 | **Regression: `-DFF_BUILD_HAL=OFF` no longer links.** `test_r2_gap` is registered unconditionally and its HAL section is guarded by `FF_TEST_NO_HAL`, which the build never defines; with `FF_BUILD_HAL=OFF` `hal.c` is excluded from `frick_fsw_core`, so the link fails on `FF_HalInit`/`FF_HalGet`/`FF_HalRegister`/… Previously only `test_hal` referenced HAL and it was gated, so this configuration built. The R2 report explicitly required the guard (`FRICK-REVIEW-0075 §8 item 1`). | `cmake -DFF_BUILD_HAL=OFF … && cmake --build …` → `FAILED: tests/test_r2_gap … undefined reference to 'FF_HalInit'` (and 12 more); `tests/CMakeLists.txt:11` (unconditional target) vs `:15` (gated `test_hal`). | Gate the HAL section on `FF_BUILD_HAL`/add `target_compile_definitions(test_r2_gap PRIVATE FF_TEST_NO_HAL=1)` when `FF_BUILD_HAL` is off, exactly as the R2 report instructed. |
| 4 | Minor | `docs/requirements/traceability.yaml` SRS-REQ-050 (~:476), SRS-REQ-041 (~:395), SSS-REQ-010 (~:589); N4 | **Over-traced evidence links.** `tests/unit/test_r2_gap.c` was added as *test evidence* for (a) SRS-REQ-050 (N7/ESA ASN.1 backend) although the file sets `FF_PusBackendOpen` and contains no N7 symbol — FRICK-REVIEW-0075's own GAP-R2-4 states the N7 backend is not exercised; (b) SRS-REQ-041 (MISRA) and SSS-REQ-010 (static analysis), whose evidence is a scan; a scanned artefact is an input, not a verification of the requirement. | `grep -n 'n7\|adapter' tests/unit/test_r2_gap.c` = none; `test_r2_gap.c:669` (`FF_PusBackendOpen`); FRICK-REVIEW-0075 §6 GAP-R2-4; manifest lines above. | Remove the `test_r2_gap.c` link from SRS-REQ-050 (or justify), and either revert or explicitly document the scan-input convention for SRS-REQ-041/SSS-REQ-010. Agrees with PA-78-01/PA-78-02. |
| 5 | Comment | `src/pus/pus.c` (root cause of R1-02); SRS-REQ-052 | `FF_PusInit()` is not idempotent with respect to the PUS service registries: re-initialising leaves HK reports, providers, setters and functions from the previous session, and repeated registration silently consumes slots (16 per table). Today `fsw_main` calls `FF_PusInit` once, so there is no operational failure, but the contract "init resets state" is incomplete and is the reason R1-02 exists. | `src/pus/pus.c:29-41`; `src/pus/pus_services.c:63-68`. | Either reset the service registries in `FF_PusInit` (preferred) or document that they are process-lifetime and provide the reset elsewhere. |
| 6 | Comment | `src/core/tbl/tbl.c:105-116`, `src/core/pal/pal_posix.c:283-289` | Inconsistent return codes for invalid inputs, which the tests codify without flagging: `FF_TblDump(bad_handle,…)` returns `FF_INVALID_ARGUMENT` while `FF_TblLoad/Activate/Revert(bad_handle)` return `FF_NOT_FOUND`; `FF_PalQueuePut(valid_id, NULL, 0)` returns `FF_NOT_FOUND` (not `FF_INVALID_ARGUMENT`). | `test_r2_gap.c:305-307,320,373`; `test_r2_gap.c:909`. | Harmonise or document; a test may legitimately pin observed behaviour, but a short comment noting the asymmetry would avoid it being read as intended design. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 5. Meaningfulness assessment (asked explicitly)

- **Public API, real behaviour.** Every assertion calls a public API and checks a
  real return code or observable state; no assertion is a constant/`true` literal.
  Representative checks: EVS ring wrap (`test_r2_gap.c:88-91`), ES
  lifecycle/ids (`:143-170`), SB `FF_NO_RESOURCE` saturation (`:240-248`), SB
  drop counter (`:225-228`), TBL revert/`FF_NOT_READY` (`:322-327`), CCSDS
  CRC/TM-not-TC rejections (`:548-561`), TM counter wrap `0x3FFF→0`
  (`test_r3_gap.c:356-388`), S1 codes `8/9/20` (`test_r3_gap.c:280-303`).
- **No tautologies / no always-pass tests.** Probed the full "clean-up" question
  by recompiling routines standalone; nothing passes vacuously. The r3 routines
  are self-contained and isolate clean.
- **Negative paths asserted.** Malformed TC → `FF_PARSE_ERROR` + EVS→S5(3)
  (`test_r3_gap.c:344-349`); unknown ids/subservices; validator rejection; queue
  full/timeout; invalid handles/classes. Genuinely negative, not merely
  "not-success".
- **Undefined order.** `test_r3_gap.c` is order-robust (verified by isolation).
  `test_r2_gap.c` is **not** (R1-02), despite claiming otherwise.
- **Check-count claims.** 487 and 198 reproduced exactly, including the
  474/483 configuration variants — FRICK-REVIEW-0075 §5 numbers are accurate.

## 6. Gap-closure spot-check

| Claimed gap | Closed? | Evidence |
| ----------- | ------- | -------- |
| EVS listener + circular ring | Yes | `test_r2_gap.c:55-102` |
| ES registry/lifecycle | Yes | `test_r2_gap.c:113-171` |
| SB wildcard + saturation + drop | Yes | `test_r2_gap.c:202-249`; `test_r3_gap.c:198-238` |
| TBL revert / dump | Yes | `test_r2_gap.c:272-374`; `test_r3_gap.c:724-783` |
| TBL validation barrier | **Partially — misses failed-load-after-success** (R1-01) | `test_r2_gap.c:350-353`; `test_r3_gap.c:741-744` |
| PUS S3 housekeeping | Yes | `test_r2_gap.c:712-753`; `test_r3_gap.c:408-476` |
| PUS S8 / S20 | Yes | `test_r2_gap.c:755-807`; `test_r3_gap.c:611-697` |
| PUS S3/S20 (FRICK-REVIEW-0076 Major #1/#2) | Yes | as above |
| PUS S1 verification codes (8/9/20) | Yes | `test_r3_gap.c:271-327` |
| TM counter wrap | Yes | `test_r3_gap.c:356-388` |
| EVS→S5 propagation | Yes | `test_r3_gap.c:514-566` |
| Time MET/UTC + S9 on-wire | Yes | `test_r2_gap.c:413-447`; `test_r3_gap.c:482-508` |
| HAL invalid-class | Yes | `test_r2_gap.c:454-478` |
| Math NaN/Inf | Yes | `test_r2_gap.c:830-838` |
| PAL lifecycle/queue/mutex/task | Yes | `test_r2_gap.c:856-946` |

## 7. Cross-check with the companion review

The norm-N20 Product Assurance review (FRICK-REVIEW-0078) independently reached
five Minor + one Comment on process/record accuracy. I concur with its
traceability findings (PA-78-01/02, mirrored here as R1-04) and note two
additional technical findings it did not cover: the Major `tbl.c` defect
(R1-01) and the test-order/HAL-build issues (R1-02/R1-03). PA-78 concludes "no
finding defeats a requirement or a safety barrier"; R1-01 **does**, so this R1
review and PA-78 do not reach the same verdict and the divergence must be
resolved by re-review after the defect is fixed (norm N12).

## 8. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED — one or more Major findings**

**Rationale.** The two suites themselves are technically sound: they build
clean, pass 16/16, reproduce their check counts exactly, exercise the public API
with real behaviour/return-code checks, cover real negative paths, and are
clang-format/MISRA clean. However, this review surfaced a **Major defect in
`src/core/tbl/tbl.c`** (R1-01) — a table-validation bypass — in a module the new
suites were added to strengthen, and the suites do **not** detect it. The TBL
"validate/atomic" gap that FRICK-REVIEW-0075/0076 claim to close is therefore
not fully closed, and the increment must not be accepted as-is for an ECSS
class-B baseline. R1-02 and R1-03 are correctness/robustness defects of the test
artefacts themselves.

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Senior SW dev engineer (fresh context) | 2026-10-11 | REJECTED (1 Major, 3 Minor, 2 Comment) |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `src/core/tbl/tbl.c` / SRS-REQ-013, SSS-REQ-009 | validation bypass: failed load after success keeps `staged=true` and stages rejected data | fix `FF_TblLoad`/`FF_TblActivate`; add regression test | probe + new test green; gates re-run | FSW | open | pending |
| R1-02 | Minor | `tests/unit/test_r2_gap.c` | order-dependent; header "no test-order dependency" false | make PUS routines self-contained; fix comment | isolation re-run green | FSW/V&V | open | pending |
| R1-03 | Minor | `tests/CMakeLists.txt` / `test_r2_gap.c` | `FF_BUILD_HAL=OFF` build regression | gate HAL section | HAL-off build green | FSW | open | pending |
| R1-04 | Minor | `traceability.yaml` SRS-REQ-050/041, SSS-REQ-010 | over-traced evidence links | remove/justify | manifest re-read; `trace.py` PASS | FSW | open | pending |
| R1-05 | Comment | `src/pus/pus.c` | `FF_PusInit` does not reset service registries | reset or document | code review | FSW | open | pending |
| R1-06 | Comment | `tbl.c` / `pal_posix.c` | inconsistent invalid-input return codes | harmonise or document | code review | FSW | open | pending |

## 11. Confidence

```text
Decision/product and configuration: frick-fsw @ 8f62746; ECSS criticality B target.
Claim type: mixed (Fact for build/gate results and file contents; Inference for the
  severity call on R1-01).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1,
  ECSS-Q-ST-80C Rev.2. Customer tailoring matrix not available to this reviewer.
Evidence consulted: test sources; CMakeLists.txt; traceability.yaml; both task reports;
  live gates (clean configure/build, ctest, clang-format, cppcheck-MISRA, trace.py);
  standalone recompiles for order-independence; a minimal public-API probe for tbl.c.
Cross-check and reconciliation: FRICK-REVIEW-0078 (PA). Agreement on traceability;
  divergence on the Major (this review found a src defect PA excluded from its scope).
Confidence: High for R1-02/R1-03/R1-04 (reproduced locally) and High for R1-01's
  existence (deterministic probe), Medium for its operational reachability today
  (FF_TblLoad has no in-tree PUS caller yet, so the barrier is API-level, not yet
  command-reachable).
Basis and detection limits: all reviewed files read in full; gates executed; the
  defect probe is deterministic. Stack/timing behaviour and on-target behaviour not
  assessed (out of scope).
Missing/inaccessible evidence: customer tailoring; certified MISRA checker; RTEMS
  cross/on-target run for this increment (n/a: tests+records change).
Assumptions and expiry: assumes the reviewed revision is the merge baseline; recheck
  after the R1-01 fix or on any change to tbl.c / the two suites. Recheck by 2027-04-11.
Required independent human reviewer/authority: project engineering/PA/release
  authority; this agent cannot approve, close NCs, or accept residual risk.
```

---

Committed and pushed with the reviewed change (norm N11). The
`docs/reviews/README.md` review-log row and the `docs/vv/verification-results.md`
cycle-log row for FRICK-REVIEW-0077 are the integrator's obligation and are not
written here.