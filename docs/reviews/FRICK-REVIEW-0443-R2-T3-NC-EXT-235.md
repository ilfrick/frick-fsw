# Independent R2 V&V Review — T3 Tool Re-qualification Delta for NC-EXT-235 (norm N10)

| Field | Value |
|---|---|
| **Document ID** | FRICK-REVIEW-0443-R2-T3-NC-EXT-235 |
| **Reviewer profile** | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer (norm N9, profile R2) |
| **Review session** | Independent sub-agent session; no author context inherited |
| **Reference revision** | `HEAD = 41dcbe21435e14b5aaff06c63d9df6f0de04df54` plus working-tree candidate set |
| **Date** | 2026-11-03 |
| **Items under review** | T3 register entry changes per NC-EXT-235: `tests/unit/test_pus.c`, `tests/unit/test_link_uart.c`; supporting evidence in `docs/vv/evidence/2026-11-03-gate-diagnosis/` |
| **Independence declaration** | I did not author, edit, build, test, or commit any item under review. I inspected the live tree, git history, and the named logs, and recomputed the published file hashes. I did not execute the test suite. |
| **Norms applied** | N1–N22 (focused: N3, N4, N6, N8, N9, N10, N11, N15, N16, N22); baselines: ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, NASA-STD-8739.8B, IEEE 1012-2024 (definitions reconciled per project glossary) |

---

## 1. Scope

**In scope.** The T3 delta defined by the NC-EXT-235 correction:

- `tests/unit/test_pus.c` — frozen-UTC seam (`FF_PalTimeTestFreeze`/`Unfreeze`) added around HK due-boundary tests (SRS-REQ-071/NC-EXT-207) to eliminate nondeterminism from host PAL UTC fine-field quantization (15.2588 µs LSB). No production logic changed.
- `tests/unit/test_link_uart.c` — positive PTY receive bounds raised from 20 ms / 500 ms to 2 s (bounded; hang still fails). 2 ms negative cases unchanged. No production framing logic changed.
- `docs/vv/tool-qualification.md` rev 0.3.107 — T3 register entry updated to reflect the changes, status set to **NOT FULLY QUALIFIED** pending independent R2 delta review and hosted execution record.

**Out of scope.** Production code (`src/pus/pus_services.c`, `src/transport/link/link_uart.c`), other T3 scripts, other tool registers, NC-EXT-235 process disposition (covered by FRICK-REVIEW-0442 PA and FRICK-REVIEW-0439 V&V).

---

## 2. Artifacts examined (exact paths + hashes)

| Artifact | SHA-256 (verified) |
|---|---|
| `tests/unit/test_pus.c` | `6f68de14635039a35381cb5f6c51accadc3afb2f4d47938778b0b40df4de2c0d` |
| `tests/unit/test_link_uart.c` | `c7307c7d13167c8d20acf59b0d5fa68f440108d196de3dc9ef8de82faabe4252` |
| `docs/vv/tool-qualification.md` | rev 0.3.107, 2026-11-03 |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/ctest-native-rerun.log` | CTEST_NATIVE_EXIT=0, 50/50 PASS |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/ctest-xw1-host-rerun.log` | CTEST_XW1_EXIT=0, 50/50 PASS |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/build-xw1-host-rerun.log` | BUILD_XW1_EXIT=0 |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/ci-local-final.log` | Full `make ci-local` cycle: configure, build, ctest 50/50, Lean build, elab receipt, trace 120/76/30/14/68 |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/stress-100-under-load-detailed.log` | STRESS RESULT: link_uart=0/100 pus=0/100 under 4× CPU load |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/format-check.log` | FORMAT_CHECK_EXIT=0 |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/build-rtems-docker.log` | RTEMS cross-build exit 0; `FF_PalTimeTestFreeze/Unfreeze` symbols NOT PRESENT in final ELF |
| `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md` + `assets.sha256` | Evidence manifest and integrity check |

All five candidate-file hashes in the evidence `README.md` match the live working tree exactly. The `assets.sha256` manifest covers all 24 files in the evidence directory.

---

## 3. Method

1. **Source review** (N10): line-by-line inspection of the two modified test scripts for correctness, traceability to evidence claims, minimality, and preservation of coverage.
2. **Traceability verification**: confirmed bidirectional links from test cases to SRS/NC requirements (SRS-REQ-071, NC-EXT-207 for HK due tests; NC-EXT-235 / S8-FU for UART bounds).
3. **Hosted execution record verification**: confirmed the four required gate logs exist, post-date the final `pal_posix.c` edit (08:00:27), carry explicit exit markers, and show 50/50 PASS on native and xw1-host configurations.
4. **Stress test verification**: confirmed `stress-100-under-load-detailed.log` records 100 iterations of `test_pus` and `test_link_uart` under 4× CPU load with 0 failures.
5. **RTEMS cross-build verification**: confirmed the test seam is correctly excluded from flight builds (`FF_TEST_HOOKS` not defined for `FF_PLATFORM=rtems`).
6. **T3 register check**: verified no tool under `tools/` changed, no T3 criterion/oracle changed beyond the two scripts, and the register honestly reflects **NOT FULLY QUALIFIED** status.

---

## 4. Findings

| ID | Severity | Title | Summary |
|---|---|---|---|
| **R2-T3-235-01** | Observation | Frozen-UTC seam correctly implemented and bounded | `test_pus.c` adds `FF_PalTimeTestFreeze(0u)` at entry and `FF_PalTimeTestUnfreeze()` at exit of `test_hk_periodic_emission` (line 517/657), `test_hk_coarse_wrap_due` (line 704/797), and `test_hk_backward_step_no_emission` (line 850/904). The seam is guarded by `FF_TEST_HOOKS` (CMakeLists.txt:217–220) and absent from RTEMS builds (verified by `nm` in `build-rtems-docker.log`). |
| **R2-T3-235-02** | Observation (positive) | Tie-break branch `now_fine < last_fine` meaningfully exercised | In `test_hk_coarse_wrap_due` (lines 750–763): `set_time_cuc(8u, 4096u)` with last emission fine=8192 → asserts `FF_ERROR` (not due); `set_time_cuc(8u, 8192u)` → asserts `FF_SUCCESS` (due exactly once); immediate re-call → asserts `FF_ERROR` (no burst). The freeze only stabilizes the clock between the enable read and the due read; it does not remove the `now_fine >= last_fine` logic. N22 satisfied. |
| **R2-T3-235-03** | Observation | UART positive bounds relaxed to 2 s; 2 ms negative cases preserved | `test_link_uart.c` lines 245–295: positive receive paths use `2000u` timeout (comment lines 239–243: *"The assertions still fail on a hang; they test framing semantics, not latency."*). Lines 229, 286: negative/timeout cases retain `2u` timeout. Line 275: delayed writer test uses `UINT32_MAX` (validates bounded VTIME path). Hang detection is not weakened. |
| **R2-T3-235-04** | Observation | Changes are minimal and confined to test harness | No production code in `src/pus/pus_services.c` (`time_period_due`) or `src/transport/link/link_uart.c` was modified. The test changes are determinism fixes only. |
| **R2-T3-235-05** | Observation | Hosted execution record complete for the corrected candidate | - `make ci-local`: `ci-local-final.log` shows exit 0, 50/50 CTest, Lean build PASS, elab receipt PASS, trace 120/76/30/14/68. <br> - Native CTest rerun: `ctest-native-rerun.log` shows `CTEST_NATIVE_EXIT=0`, 50/50 PASS. <br> - xw1-host rerun: `build-xw1-host-rerun.log` (`BUILD_XW1_EXIT=0`), `ctest-xw1-host-rerun.log` (`CTEST_XW1_EXIT=0`, 50/50 PASS). <br> - Stress: `stress-100-under-load-detailed.log` shows 0/100 failures for both `link_uart` and `pus` under 4× CPU load. <br> - Format: `format-check.log` shows `FORMAT_CHECK_EXIT=0`. <br> All logs post-date the final `pal_posix.c` edit (08:00:27). |
| **R2-T3-235-06** | Observation | RTEMS cross-build verifies seam exclusion | `build-rtems-docker.log` lines 315–318: exit 0; `FF_PalTimeTestFreeze/Unfreeze` symbols NOT PRESENT in final ELF; `FF_TEST_HOOKS` not defined for `FF_PLATFORM=rtems`. Flight build integrity confirmed. |
| **R2-T3-235-07** | Observation | T3 register correctly reflects re-qualification obligation | `docs/vv/tool-qualification.md` rev 0.3.107 T3 row and History entry 0.3.107 explicitly state: **NOT FULLY QUALIFIED** pending independent delta review (R2) of the two modified scripts and a retained hosted execution record. No T3 criterion/oracle changed beyond the two scripts; no tool under `tools/` changed. |
| **R2-T3-235-08** | Minor | Stale pre-edit logs in evidence directory not labelled `SUPERSEDED` | The original `ctest-native.log`, `ctest-xw1-host.log`, `build-native.log`, `build-xw1-host.log` (mtimes 07:56–07:57) predate the final edit and remain in the evidence directory unmarked. The README cites the rerun logs but does not label the stale logs. Recommend labelling to avoid confusion. |
| **R2-T3-235-09** | Minor | Gate build configuration not fully captured in all logs | xw1-host `FF_*` set is documented in `verification-results.md`; native CI and RTEMS effective `CMakeCache.txt` values (or hashes) are not explicitly captured in the gate logs. Reproducibility gap, not functional. |
| **R2-T3-235-10** | Observation (positive) | Test quality N22 satisfied for corrected suites | Frozen seam preserves boundary coverage; UART 2 ms negative cases preserved; 2 s positive bounds test framing semantics, not latency. No test-logic bypass introduced. |

---

## 5. Verdict

### **PASS WITH COMMENTS**

The T3 delta for NC-EXT-235 is **technically sound, minimally scoped, and correctly evidenced**:

- The two test-harness timing defects (UTC quantization flakiness in `test_pus` HK due tests; PTY scheduling bounds in `test_link_uart`) are correctly diagnosed and corrected.
- The frozen-UTC seam is compile-time test-only (`FF_TEST_HOOKS`, absent from flight/RTEMS builds), does not alter production due-test or framing logic, and preserves the critical tie-break branch coverage (`now_fine < last_fine`).
- The UART positive bounds are relaxed to 2 s (still failing on hang), while the 2 ms negative cases are preserved.
- Hosted execution record is complete: `make ci-local` exit 0 (50/50, Lean, trace), native CTest rerun 50/50, xw1-host rerun build+CTest 50/50, format-check PASS, stress 0/100 under 4× load, RTEMS cross-build PASS with seam exclusion verified.
- T3 register honestly reflects **NOT FULLY QUALIFIED** status pending this R2 review and the retained hosted execution record.

**Residual open items for full T3 qualification:**

1. **This R2 review completes the independent delta review of the two modified scripts.** With this review recorded, the R2 delta review criterion is satisfied.
2. **Hosted execution record is retained** in `docs/vv/evidence/2026-11-03-gate-diagnosis/` with hash manifest (`assets.sha256`).
3. **No tool under `tools/` was changed**, and no T3 criterion/oracle was changed beyond the two scripts.

**This review qualifies the T3 delta only.** It does not confer full T3 qualification without the hosted CI execution record. The T3 register entry in `tool-qualification.md` must be updated to reflect completion of this R2 review and the retained hosted execution record before T3 status can be revised to QUALIFIED.

**This review does not approve release, does not close NC-EXT-235, does not confer qualification, certification, airworthiness, flightworthiness or compliance credit.** It is an independent V&V assessment of the T3 delta evidence and process only. No source, test, plan or evidence file was modified; no commit or push was performed. The owner/authorized human authority retains every disposition, risk-acceptance, NC-closure and release decision.

---

## 6. Confidence and uncertainty

- **High confidence** for findings based on direct file/git/log inspection, recomputed hashes, and source-code analysis of the test logic.
- **Medium confidence** for precise execution timestamps (filesystem mtimes are the only time evidence; logs carry `*_EXIT` markers but not full ISO timestamps).
- **Low confidence** for anything depending on the unretained pre-fix probe runs — these are documented as absent and not used as evidence in this review.

---

## 7. Recommended T3 register update (for the `aeronautics-space-fsw-engineer` upon commit)

In `docs/vv/tool-qualification.md`, append a History entry:

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.3.108 | 2026-11-03 | frick-fsw | **T3 re-qualification delta review completed (R2):** Independent V&V review FRICK-REVIEW-0443-R2-T3-NC-EXT-235 of the two modified scripts (`test_pus.c` frozen-UTC seam for HK due tests NC-EXT-207/235; `test_link_uart.c` positive PTY bounds 20 ms/500 ms → 2 s). Source review PASS; hosted execution record retained (`make ci-local` 50/50 + Lean + trace, native CTest 50/50, xw1-host build+CTest 50/50, stress 0/100 under 4× load, format-check PASS, RTEMS cross-build PASS with seam exclusion verified). No tool under `tools/` changed; no T3 criterion/oracle changed beyond the two scripts. **T3 status revised to QUALIFIED** (both R2 delta review and hosted execution record criteria satisfied). NC-EXT-235 Minor closure criterion "T3 re-qualification completed" evidenced. |

---

**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (independent R2 profile)  
**Date:** 2026-11-03  
**Session:** Independent sub-agent; no author context inherited