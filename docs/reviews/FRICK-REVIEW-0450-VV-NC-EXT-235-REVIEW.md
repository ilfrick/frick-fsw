# Independent V&V Re-Review — NC-EXT-235 Corrected Candidate (norm N9/N20 independence)

| Field | Value |
|---|---|
| **Document ID** | FRICK-REVIEW-0450-VV-NC-EXT-235-REVIEW |
| **Reviewer profile** | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer (norm N9, profile R2) |
| **Review session** | Independent sub-agent session; no author context inherited |
| **Reference revision** | `HEAD = 41dcbe21435e14b5aaff06c63d9df6f0de04df54` **plus** the working-tree candidate set for NC-EXT-235 |
| **Date** | 2026-11-03 |
| **Items under review** | NC-EXT-235 disposition evidence; tool-qualification.md T3 tracking; FRICK-REVIEW-0439 (VV-NC235-05 Major); FRICK-REVIEW-0443 (R2 T3 delta); FRICK-REVIEW-0440 (PA 7 Major); FRICK-REVIEW-0441 (R4 PASS WITH COMMENTS); evidence package `docs/vv/evidence/2026-11-03-gate-diagnosis/` |
| **Independence declaration** | I did not author, edit, build, test, or commit any item under review. I inspected the live tree, git history, review artifacts, and evidence logs, and recomputed published file hashes. I did not execute the test suite. |
| **Norms applied** | N1–N22 (focused: N3, N4, N6, N8, N9, N10, N11, N12, N15, N16, N17, N18, N20, N21, N22); baselines: ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, NASA-STD-8739.8B, IEEE 1012-2024 (definitions reconciled per project glossary) |

---

## 1. Scope

**In scope.** The NC-EXT-235 corrected candidate defined as `41dcbe2` plus exactly:
- `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` — frozen-UTC seam under `FF_TEST_HOOKS`
- `tests/unit/test_pus.c` — HK due tests freeze UTC for their duration; unfreeze at test end
- `tests/unit/test_link_uart.c` — positive PTY receive bounds raised from 20 ms / 500 ms to 2 s; 2 ms negative cases preserved
- `docs/planning/sprint-plan.md` — Rev 1.119 failure record, Rev 1.120 verification record, NC-EXT-235 entry
- `docs/vv/evidence/2026-11-03-gate-diagnosis/` — complete evidence package with `assets.sha256`

**Out of scope (separate N22-TS5-47 increment).** `tests/unit/test_evs_hs_e2e.c`, `docs/vv/test-spec/ts-5-core-services.md` (N22-TS5-47 row).

---

## 2. Summary of Prior Reviews

| Review | ID | Verdict | Key Finding on T3 Re-qualification |
|---|---|---|---|
| V&V (initial) | FRICK-REVIEW-0439 | PASS WITH COMMENTS | **VV-NC235-05 (Major):** T3 re-qualification pending; two scripts changed; register correctly records NOT FULLY QUALIFIED |
| R2 T3 delta | FRICK-REVIEW-0443 | **PASS WITH COMMENTS** | R2 independent delta review of `test_pus.c` and `test_link_uart.c` **completed**; hosted execution record **retained**; no tool under `tools/` changed; no T3 criterion/oracle changed beyond the two scripts |
| PA | FRICK-REVIEW-0440 | **NEEDS REVISION** | 7 Major findings (PA-NC235-01..07); PA-NC235-05 = T3 re-qualification (same Major as VV-NC235-05) |
| R4 Documentation | FRICK-REVIEW-0441 | **PASS WITH COMMENTS** | 0 Blocker, 0 Major, 1 Minor (handover.md currency) |

---

## 3. Verification of T3 Re-qualification Tracking

### 3.1 tool-qualification.md rev 0.3.107 (as cited in FRICK-REVIEW-0439)
- **T3 register entry (line 26)**: Explicitly lists `test_pus.c` and `test_link_uart.c` under T3 scope.
- **History entry 0.3.107 (line 250)**: Records "**T3 re-qualification required: two test scripts under T3 changed — `tests/unit/test_pus.c` and `tests/unit/test_link_uart.c`. ... T3 remains NOT FULLY QUALIFIED pending independent delta review (R2) of the two modified scripts and a retained hosted execution record.**"
- **Status**: Correctly tracked the obligation; did **not** claim qualification.

### 3.2 tool-qualification.md rev 0.3.108 (current in working tree)
- **T3 status column** shows a mixed entry: the first part states "**QUALIFIED** — R2 delta review FRICK-REVIEW-0443-R2-T3-NC-EXT-235.md (PASS WITH COMMENTS); hosted execution record retained..." but the second part (after a pipe delimiter) still states "**NOT FULLY QUALIFIED — NC-EXT-235 delta: `test_pus.c` and `test_link_uart.c` modified; independent delta review (R2) pending; hosted execution record pending**".
- **Assessment**: The second clause is **stale** and inconsistent with the completed R2 review (FRICK-REVIEW-0443). The R2 review explicitly confirmed both criteria are satisfied (R2 delta review complete + hosted execution record retained). The register should be updated to reflect QUALIFIED status per the R2 review's recommended update (History entry 0.3.108).

### 3.3 FRICK-REVIEW-0443 (R2 T3 delta) — Confirmed PASS WITH COMMENTS
The independent R2 review of the T3 delta found:
- **R2-T3-235-01/02/03/04/06/10 (Observations, positive)**: Frozen-UTC seam correctly implemented and bounded; tie-break branch `now_fine < last_fine` meaningfully exercised; UART 2 ms negative cases preserved; changes minimal and confined to test harness; RTEMS cross-build verifies seam exclusion; test quality N22 satisfied.
- **R2-T3-235-05 (Observation)**: Hosted execution record complete — `make ci-local` exit 0 (50/50, Lean, trace), native CTest rerun 50/50, xw1-host rerun build+CTest 50/50, format-check PASS, stress 0/100 under 4× load, RTEMS cross-build PASS with seam exclusion verified.
- **R2-T3-235-07 (Observation)**: T3 register correctly reflects re-qualification obligation (at the time of that review).
- **R2-T3-235-08/09 (Minor)**: Stale pre-edit logs unlabelled; gate build config not fully captured.
- **Verdict**: **PASS WITH COMMENTS** — "This review completes the independent delta review of the two modified scripts. With this review recorded, the R2 delta review criterion is satisfied. Hosted execution record is retained... No tool under `tools/` was changed, and no T3 criterion/oracle was changed beyond the two scripts."
- **Recommended register update**: T3 status revised to QUALIFIED (both R2 delta review and hosted execution record criteria satisfied).

---

## 4. Findings

| ID | Severity | Title | Summary |
|---|---|---|---|
| **VV-NC235-RE-01** | **Major (residual)** | T3 register status inconsistent in rev 0.3.108 | The T3 status column in `tool-qualification.md` rev 0.3.108 contains a stale "NOT FULLY QUALIFIED — independent delta review (R2) pending" clause that contradicts the completed FRICK-REVIEW-0443. The register must be updated to QUALIFIED per the R2 review's recommended History entry 0.3.108. |
| **VV-NC235-RE-02** | **Major (residual from PA)** | PA review FRICK-REVIEW-0440 verdict NEEDS REVISION with 7 Major findings | PA-NC235-01 (stale xw1-host/native CTest evidence), -02 (incomplete failure record), -03 (evidence not self-contained), -04 (verification-results.md not updated), -05 (T3 re-qualification), -06 (no RTEMS cross-build after PAL change), -07 (build config not recorded) remain open. **Note**: FRICK-REVIEW-0439 noted that PA-NC235-01, -03, -04, -06 are addressed in the evidence package as of the V&V review; PA-NC235-02 and -07 remain as Minor; PA-NC235-05 is the same Major as VV-NC235-05 (now satisfied by FRICK-REVIEW-0443). The PA review itself has not been re-run. |
| **VV-NC235-RE-03** | Minor | Stale pre-edit logs in evidence directory not labelled SUPERSEDED | Confirmed by both FRICK-REVIEW-0439 (VV-NC235-01) and FRICK-REVIEW-0443 (R2-T3-235-08). The original `ctest-native.log`, `ctest-xw1-host.log`, `build-native.log`, `build-xw1-host.log` (mtimes 07:56–07:57) predate the final `pal_posix.c` edit (08:00:27) and remain unmarked. |
| **VV-NC235-RE-04** | Minor | Gate build configuration not fully captured in all logs | Confirmed by FRICK-REVIEW-0439 (VV-NC235-07) and FRICK-REVIEW-0443 (R2-T3-235-09). xw1-host `FF_*` set documented; native CI and RTEMS effective `CMakeCache.txt` values not explicitly captured. |
| **VV-NC235-RE-05** | Observation (positive) | Test quality N22 satisfied for corrected suites | Frozen-UTC seam preserves `now_fine < last_fine` branch coverage; UART 2 ms negative cases preserved; 2 s positive bounds test framing semantics, not latency. No test-logic bypass introduced. Confirmed by VV-NC235-08 and R2-T3-235-10. |
| **VV-NC235-RE-06** | Observation | RTEMS cross-build confirms seam exclusion | `build-rtems-docker.log` shows exit 0; `FF_PalTimeTestFreeze/Unfreeze` symbols NOT PRESENT in final ELF; `FF_TEST_HOOKS` not defined for `FF_PLATFORM=rtems`. Flight build integrity substantiated. |
| **VV-NC235-RE-07** | Observation | Evidence self-containment achieved | `assets.sha256` covers all 24 files; JSON records copied into evidence directory; exclusion sentence corrected. |
| **VV-NC235-RE-08** | Observation | verification-results.md cycle record complete and honest | FRICK-VV-GATE-20261103 section present with accurate verdict "CYCLE COMPLETE WITH OPEN FINDINGS", disclosing N18 FAIL, no NC closed, no sprint completed, Major PA findings blocking commit/push. |

---

## 5. Verdict

### **PASS WITH COMMENTS**

The NC-EXT-235 corrected candidate is **technically sound and minimally scoped**:

- The two test-harness timing defects (UTC quantization flakiness in `test_pus` HK due tests; PTY scheduling bounds in `test_link_uart`) are correctly diagnosed and corrected.
- The frozen-UTC seam is compile-time test-only (`FF_TEST_HOOKS`, absent from flight/RTEMS builds), does not alter production due-test or framing logic, and preserves the critical tie-break branch coverage (`now_fine < last_fine`).
- The UART positive bounds are relaxed to 2 s (still failing on hang), while the 2 ms negative cases are preserved.
- Host gate evidence (`ci-local` 50/50, trace 120/76/30/14/68, format 0, cppcheck 163, stress 0/100) is bound to the final source.
- RTEMS cross-build passes and confirms the seam is absent from the flight ELF.
- **The independent R2 T3 delta review (FRICK-REVIEW-0443) is complete with PASS WITH COMMENTS**, satisfying the Major T3 re-qualification finding (VV-NC235-05 / PA-NC235-05). The hosted execution record is retained in `docs/vv/evidence/2026-11-03-gate-diagnosis/` with hash manifest.

**However, the following must be resolved before commit/push (per N7/N9/N10/N12/N15/N20):**

1. **T3 register update (Major — VV-NC235-RE-01):** `docs/vv/tool-qualification.md` T3 status column must be updated to QUALIFIED with History entry 0.3.108 reflecting completion of FRICK-REVIEW-0443 and the retained hosted execution record. The stale "NOT FULLY QUALIFIED — independent delta review (R2) pending" clause must be removed.
2. **PA review Major findings (FRICK-REVIEW-0440):** The PA review verdict remains **NEEDS REVISION**. While several findings are addressed in the current evidence package (PA-NC235-01, -03, -04, -06 per FRICK-REVIEW-0439; PA-NC235-05 satisfied by FRICK-REVIEW-0443), the PA review itself has not been re-run on the corrected content. A corrected-content PA re-review is required.
3. **Stale log labelling and config capture (Minor — VV-NC235-RE-03/04):** Pre-edit logs labelled `SUPERSEDED`; each gate log records its effective `FF_*` build configuration (or `CMakeCache.txt` hash).
4. **N18 remains FAIL:** NC-EXT-198 / NC-PAL-01 (MPFS watchdog binding unwired) unchanged; no target/HIL evidence exists. GAP-A5 (Renode fidelity) remains.
5. **No NC closed, no sprint completed:** The candidate correctly makes no closure claims.

---

## 6. What This Review Does NOT Establish

- **No NC closure.** NC-EXT-235 remains **CORRECTED — PENDING INDEPENDENT REVIEW** (Major evidence claim / Minor test harness). Closure requires the Major findings above to be resolved and the corrected content re-reviewed by PA.
- **No T3 qualification until register updated.** Although the R2 delta review (FRICK-REVIEW-0443) is complete and the hosted execution record is retained, the `tool-qualification.md` register still contains a stale NOT FULLY QUALIFIED clause. T3 qualification is not formally established until the register is updated to QUALIFIED with the proper History entry.
- **No N18 / flightworthiness / target timing evidence.** The RTEMS cross-build passes but only proves the seam is absent; it does not exercise the target, the watchdog binding (NC-PAL-01), or any HIL/N18 condition. GAP-A5 (Renode fidelity) remains.
- **No requirement compliance beyond the tested gates.** The 50/50 CTest pass on host configurations demonstrates that the corrected test suites execute without failure on those configurations. It does not prove absence of defects in untested paths, untested requirements, or target-specific behaviour.
- **No release, certification, or approval authority.** This is an independent V&V assessment of evidence and process only. The owner/authorized human authority retains every disposition, risk-acceptance, NC-closure, and release decision.
- **No resolution of PA review FRICK-REVIEW-0440.** That review's NEEDS REVISION verdict stands until a corrected-content PA re-review is completed.

---

## 7. Confidence and Uncertainty

- **High confidence** for findings based on direct file/git/log inspection, recomputed hashes, source-code analysis of test logic, and the four prior independent review artifacts.
- **Medium confidence** for precise execution timestamps (filesystem mtimes are the only time evidence; logs carry `*_EXIT` markers but not full ISO timestamps).
- **Low confidence** for anything depending on the unretained pre-fix probe runs ("3/60", "200/200", "20/200") — these are documented as absent and not used as evidence in this review.

---

## 8. Follow-up Entry Criteria for Commit/Push

A commit/push can proceed when:

1. `docs/vv/tool-qualification.md` T3 status updated to QUALIFIED with History entry 0.3.108 per FRICK-REVIEW-0443 recommendation.
2. PA review FRICK-REVIEW-0440 Major findings resolved and corrected-content PA re-review completes with PASS.
3. Stale pre-edit logs in evidence directory labelled `SUPERSEDED` or removed.
4. Each gate log records its effective `FF_*` build configuration (or `CMakeCache.txt` hash).
5. All applicable independent reviews (V&V, R4, PA) are complete and indexed.

---

**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (independent R2 profile)  
**Date:** 2026-11-03  
**Session:** Independent sub-agent; no author context inherited