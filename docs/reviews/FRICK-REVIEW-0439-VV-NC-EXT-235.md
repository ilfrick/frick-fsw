# Independent V&V Review — NC-EXT-235 Corrected Candidate (norm N9/N20 independence)

| Field | Value |
|---|---|
| **Document ID** | FRICK-REVIEW-0439-VV-NC-EXT-235 |
| **Reviewer profile** | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer (norm N9, profile R2) |
| **Review session** | Independent sub-agent session; no author context inherited |
| **Reference revision** | `HEAD = 41dcbe21435e14b5aaff06c63d9df6f0de04df54` **plus** the working-tree candidate set |
| **Date** | 2026-11-03 |
| **Items under review** | NC-EXT-235 disposition; corrected-candidate evidence; `pal.h`/`pal_posix.c` test seam; test edits in `test_pus.c`/`test_link_uart.c`; sprint-plan Rev 1.117–1.120; evidence directory `docs/vv/evidence/2026-11-03-gate-diagnosis/` |
| **Independence declaration** | I did not author, edit, build, test, or commit any item under review. I inspected the live tree, git history, and the named logs, and recomputed the published file hashes. I did not execute the test suite. |
| **Norms applied** | N1–N22 (focused: N3, N4, N6, N8, N9, N10, N11, N15, N16, N18, N20, N21, N22) |
| **Scope baseline** | ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, NASA-STD-8739.8B, IEEE 1012-2024 (definitions reconciled per project glossary) |

---

## 1. Scope

**In scope.** The candidate defined as `41dcbe2` plus exactly:

- `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` — frozen-UTC seam under `FF_TEST_HOOKS`
- `tests/unit/test_pus.c` — HK due tests freeze UTC for their duration; unfreeze at test end
- `tests/unit/test_link_uart.c` — positive PTY receive bounds raised from 20 ms / 500 ms to 2 s; 2 ms negative cases preserved
- `docs/planning/sprint-plan.md` — Rev 1.119 failure record, Rev 1.120 verification record, NC-EXT-235 register entry
- `docs/vv/evidence/2026-11-03-gate-diagnosis/` — complete evidence package with `assets.sha256`

**Out of scope (pre-existing, preserved).** `tests/unit/test_pus_n7_adapter.c`, `test_pusopen.c`, `test_r2_gap.c`, `test_sched_max_entries.c`, `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`, all untracked files not in the evidence directory.

---

## 2. Artifacts examined (exact paths and commands)

Git and working tree:

- `git rev-parse HEAD`, `git status --porcelain=v1`, `git log --oneline -8`
- `git diff` for all ten modified tracked files; `git show HEAD:docs/planning/sprint-plan.md` (pre-correction text)
- `sha256sum` on the five candidate files against `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md` — all five match
- `stat -c '%y %n'` on candidate files, evidence logs, and RTEMS build artifacts
- `grep -E "FF_USE_LWIP|FF_USE_N7_OBCP|FF_USE_PUS_ASN1|FF_BUILD_TESTS|FF_PLATFORM" build/{ci,native,xw1-host}/CMakeCache.txt`

Candidate content:

- `src/core/pal/pal.h` (lines 71–87), `src/core/pal/pal_posix.c` (lines 498–542, 504–522), `CMakeLists.txt` (lines 217–220)
- `tests/unit/test_pus.c` (lines 499–910, 750–798, 866–870, 797, 904), `tests/unit/test_link_uart.c` (lines 230–412), `tests/ff_test.h` (lines 17–43)
- `docs/planning/sprint-plan.md` (Rev 1.119 lines 693, Rev 1.120 lines 694, NC-EXT-235 entry line 322)
- `docs/architecture/time-and-timecode.md` (rev 0.4, lines 25–40)
- `docs/vv/tool-qualification.md` (rev 0.3.107, T3 row line 26, History entry 0.3.107 line 250)
- `docs/vv/verification-results.md` (FRICK-VV-GATE-20261103 section lines 135–201)
- `docs/vv/evidence/2026-11-03-gate-diagnosis/`: `README.md`, `assets.sha256`, `ctest-native-rerun.log`, `ctest-xw1-host-rerun.log`, `build-xw1-host-rerun.log`, `build-rtems-docker.log`, `ci-local-final.log`, `ctest-native.log`, `ctest-xw1-host.log`, `build-native.log`, `build-xw1-host.log`, `format-check.log`, `cppcheck.log`, `cppcheck.txt`, `stress-100-under-load.log`, `stress-100-under-load-detailed.log`, `2026-11-03-ci-pipeline-evidence.json`, `2026-11-03-ci-direct-17633.json`, `2026-11-03-ci-direct-17849.json`, `2026-11-03-pus-reruns.json`

Cache evidence cited by the candidate (now copied into evidence directory):

- `2026-11-03-ci-pipeline-evidence.json` (seq 17401, 17419, 17428)
- `2026-11-03-ci-direct-17633.json` (seq 17633)
- `2026-11-03-ci-direct-17849.json` (seq 17849)
- `2026-11-03-pus-reruns.json` (seq 17795, 17822, 17831, 17840)

No files were modified, no build/test was re-executed, no commit/push was performed.

---

## 3. Candidate identity — verified

The five published SHA-256 values in `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md` match the live working tree exactly. The `assets.sha256` manifest covers all 24 files in the evidence directory. The container image id `sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b` is corroborated by `ci-local-final.log`. Host kernel `7.0.0-38-generic` and 4 CPUs match `uname -r` / `nproc`. **Source identity is established; build configuration and execution timestamps are partially recorded (logs carry `*_EXIT` markers but not full ISO timestamps).**

---

## 4. Required checks — status

### A. Re-run evidence (native CTest, xw1-host, RTEMS cross-build)

| Gate | Log | Exit | Result | Post-dates final `pal_posix.c` edit (08:00:27)? |
|---|---|---|---|---|
| Native CTest rerun | `ctest-native-rerun.log` | 0 | 50/50 PASS | **Yes** (file mtime after edit) |
| xw1-host build rerun | `build-xw1-host-rerun.log` | 0 | 242/245 targets | **Yes** |
| xw1-host CTest rerun | `ctest-xw1-host-rerun.log` | 0 | 50/50 PASS | **Yes** |
| RTEMS cross-build (Docker) | `build-rtems-docker.log` | 0 | ELF linked | **Yes** |

**V&V finding VV-NC235-01 (Observation):** The original `ctest-native.log` (07:56), `ctest-xw1-host.log` (07:57), `build-native.log` (07:56), `build-xw1-host.log` (07:56) predate the final `pal_posix.c` edit. The README table and Rev 1.120 now cite the rerun logs explicitly for the corrected candidate. The stale logs remain in the evidence directory unmarked; recommend labelling them `SUPERSEDED — pre-final-edit` to avoid confusion.

### B. Failure record completeness

| Sequence | Description | JSON location | Status |
|---|---|---|---|
| 17401 | Pipeline-masked `tail` exit 0; excerpt shows `link_uart` FAILED, make exit 8 | `ci-pipeline-evidence.json` | **Copied into evidence dir** |
| 17633 | Direct `make ci-local`, exit 2, CTest 49/50, `pus` failures at `test_pus.c:754,755,760,761` | `ci-direct-17633.json` | **Copied into evidence dir** (output truncated to 2000 chars in JSON) |
| 17795 | Pre-fix binary, `test_hk_backward_step_no_emission` at `test_pus.c:869,870`, `checks=225 failures=2` | `pus-reruns.json` | **Copied into evidence dir** |
| 17849 | Direct `make ci-local`, exit 0, 50/50; candidate binding "not independently established" | `ci-direct-17849.json` | **Copied into evidence dir** |

**V&V finding VV-NC235-02 (Minor):** The failure record is now complete in the evidence directory. However:
- The `ci-direct-17633.json` output is truncated (2000-char limit) and does not show the full `pus` failure context (lines 754–761).
- The probe source (`time_probe.c`) and its log for the "3/60" and "200/200" reproduction claims are not retained anywhere; the candidate documents this absence honestly in Rev 1.119 and the PA review.
- The `link_uart` "20/200" reproduction log is likewise absent.
- The 17795 failure at `test_pus.c:869,870` was not named in the NC-EXT-235 problem statement (which cites only `test_hk_coarse_wrap_due` and `test_hk_periodic_emission`); this is a record-keeping gap, not a technical gap.

### C. Evidence self-containment

- `assets.sha256` covers all 24 files in `docs/vv/evidence/2026-11-03-gate-diagnosis/` including the four JSON records.
- Exclusion sentence in `README.md` line 68: *"all other untracked files shown by `git status` except this evidence directory"* — correct and no longer self-referential.
- All cited JSONs are now inside the evidence directory.

**V&V finding VV-NC235-03 (Observation):** Self-containment achieved. The manifest is a simple `sha256sum` list; consider upgrading to a signed attestation (as done for `2026-11-02-cppcheck-canonical-final/`) for future evidence packages.

### D. verification-results.md updated

- FRICK-VV-GATE-20261103 section present (lines 135–201), with cycle record, candidate identity, gate table, failure history, open findings, residual unproven items, and verdict **"CYCLE COMPLETE WITH OPEN FINDINGS"**.

**V&V finding VV-NC235-04 (Observation):** The record correctly states that N18 remains FAIL, no NC closed, no sprint completed, and that Major PA findings must be addressed before commit/push. This is accurate and compliant with N11/N15.

### E. T3 re-qualification

- `docs/vv/tool-qualification.md` rev 0.3.107 (line 250) records: **"T3 re-qualification required: two test scripts under T3 changed — `tests/unit/test_pus.c` and `tests/unit/test_link_uart.c`. ... T3 remains NOT FULLY QUALIFIED pending independent delta review (R2) of the two modified scripts and a retained hosted execution record."**
- T3 register entry (line 26) lists both scripts explicitly.

**V&V finding VV-NC235-05 (Major):** N10 requires re-qualification of any changed V&V tool before its output is used as evidence. The candidate correctly records the re-qualification obligation and does **not** claim T3 is qualified. The 50/50 results are therefore evidence from a **changed, not-yet-requalified** test set. This is properly disclosed; the Major is that the independent R2 delta review and hosted execution record are still pending.

### F. RTEMS cross-build

- `build-rtems-docker.log` lines 315–318: `RTEMS cross-build exit: 0`; `FF_PalTimeTestFreeze/Unfreeze symbols: NOT PRESENT in final ELF (correctly excluded from flight build)`; `FF_TEST_HOOKS not defined for FF_PLATFORM=rtems (CMakeLists.txt:217-220)`.
- The CMake guard `if(FF_BUILD_TESTS AND NOT FF_PLATFORM MATCHES "^rtems")` at line 218 is correct and verified by inspection.

**V&V finding VV-NC235-06 (Observation):** The post-build verification provides both the exit code and the symbol-absence check (`nm` on the final ELF). This is strong evidence that the test seam is absent from flight/RTEMS builds. The claim is substantiated.

### G. Build configuration recorded

- `ci-local-final.log` contains the full configure command (line 69 in evidence README corroborates the container image and command).
- `verification-results.md` Rev 1.0 table (lines 166–169) documents `FF_USE_LWIP=ON`, `FF_USE_N7_OBCP=ON`, `FF_USE_PUS_ASN1=ON` for the xw1-host gate.
- RTEMS build uses `frick-rtems-toolchain:local` with pinned toolchain image id `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` (recorded in `build-rtems-docker.log` line 7 and `verification-results.md` line 169).

**V&V finding VV-NC235-07 (Minor):** The effective `CMakeCache.txt` values for each gate are not explicitly captured in the evidence logs (only the xw1-host set is documented in the cycle record). For full reproducibility, each gate log should record its effective `FF_*` configuration (or a `CMakeCache.txt` hash). This is a reproducibility gap, not a functional one.

### H. Test quality (N22) — frozen-UTC seam and UART bounds

**Frozen-UTC seam does not make the `u == period` tie-break tautological:**
- `test_pus.c` lines 750–763 (`test_hk_coarse_wrap_due`): The test explicitly exercises `now_fine < last_fine` (line 753: `set_time_cuc(8u, 4096u)` where last emission fine was 8192) and asserts `FF_ERROR` (not due). Then at equal fine (line 760: `set_time_cuc(8u, 8192u)`) it asserts `FF_SUCCESS` (due exactly once). Then immediate re-call asserts `FF_ERROR` again (no burst). The tie-break branch is exercised and the freeze only stabilises the clock between the enable read and the due read; it does not remove the `now_fine >= last_fine` logic.
- `test_pus.c` lines 866–870 (`test_hk_backward_step_no_emission`): Backward step to `1995u` (5 s before last emission at 2000) asserts `FF_ERROR` — the freeze does not affect this path.
- `test_pus.c` lines 797, 904: `FF_PalTimeTestUnfreeze()` called at test end.

**2 s UART bounds still fail on hang; 2 ms negative cases preserved:**
- `test_link_uart.c` lines 234, 286: Negative/timeout cases use `2u` (2 ms) timeout — `FF_TEST_ASSERT(FF_LinkUartBackend.recv(..., 2u) == FF_TIMEOUT)`. These are unchanged.
- Lines 245–295: Positive receive paths use `2000u` (2 s) timeout. Comment at lines 239–243 explains the rationale: *"The assertions still fail on a hang; they test framing semantics, not latency."*
- Line 275: Delayed writer test uses `UINT32_MAX` timeout (validates bounded VTIME path).

**V&V finding VV-NC235-08 (Observation — positive):** The test design correctly preserves the boundary coverage (including the `now_fine < last_fine` branch) and the negative-case tight bounds. The frozen seam is a test-harness determinism aid, not a test-logic bypass. This satisfies N22 for these test suites.

---

## 5. Findings summary

| ID | Severity | Title | Summary |
|---|---|---|---|
| **VV-NC235-01** | Observation | Stale pre-edit logs unmarked | Original `ctest-native.log`, `ctest-xw1-host.log`, `build-native.log`, `build-xw1-host.log` predate final `pal_posix.c` edit; rerun logs exist and are correct. Recommend `SUPERSEDED` label. |
| **VV-NC235-02** | Minor | Failure record probe/log gaps | `ci-direct-17633.json` truncated; probe source/log for "3/60", "200/200", "20/200" claims not retained; 17795 failure at `test_pus.c:869,870` not named in NC problem statement. |
| **VV-NC235-03** | Observation | Evidence self-containment achieved | `assets.sha256` covers all files; JSONs copied in; exclusion sentence fixed. Consider signed attestation for future. |
| **VV-NC235-04** | Observation | verification-results.md cycle record complete | FRICK-VV-GATE-20261103 present with accurate verdict and open findings. |
| **VV-NC235-05** | **Major** | T3 re-qualification pending | Two T3 scripts changed; re-qualification recorded as required and NOT FULLY QUALIFIED; independent R2 delta review and hosted execution record pending. |
| **VV-NC235-06** | Observation | RTEMS cross-build + symbol check done | Exit 0; `FF_PalTimeTestFreeze/Unfreeze` absent from ELF; `FF_TEST_HOOKS` not defined for RTEMS. Claim substantiated. |
| **VV-NC235-07** | Minor | Gate build config not fully captured | xw1-host `FF_*` set documented; native/ci/RTEMS effective configs not explicitly in logs. Reproducibility gap. |
| **VV-NC235-08** | Observation (positive) | Test quality N22 satisfied for corrected suites | Frozen seam preserves `now_fine < last_fine` branch; UART 2 ms negative cases preserved; 2 s positive bounds test framing, not latency. |

---

## 6. Verdict

### **PASS WITH COMMENTS**

The corrected candidate is **technically sound and minimally scoped**:

- The two test-harness timing defects (`test_pus` UTC quantization flakiness; `test_link_uart` PTY scheduling bounds under load) are correctly diagnosed and corrected.
- The frozen-UTC seam is compile-time test-only (`FF_TEST_HOOKS`, absent from flight/RTEMS builds), does not alter production due-test or framing logic, and preserves the critical tie-break branch coverage (`now_fine < last_fine`).
- The UART positive bounds are relaxed to 2 s (still failing on hang), while the 2 ms negative cases are preserved.
- Host gate evidence (`ci-local` 50/50, trace 120/76/30/14/68, format 0, cppcheck 163, stress 0/100) is bound to the final source.
- RTEMS cross-build passes and confirms the seam is absent from the flight ELF.
- Failure records are now retained in the evidence directory with a hash manifest.
- The cycle record in `verification-results.md` honestly discloses all open findings and does not claim NC closure or sprint completion.

**However, the following must be resolved before commit/push (per N7/N9/N10/N12/N15/N20):**

1. **T3 re-qualification (Major — VV-NC235-05):** Independent R2 delta review of `test_pus.c` and `test_link_uart.c` changes, plus a retained hosted execution record, are required before the 50/50 results can be used as qualified V&V evidence.
2. **PA review Major findings (FRICK-REVIEW-0440):** The PA review verdict is **NEEDS REVISION** with 7 Major findings (PA-NC235-01 through PA-NC235-07), primarily concerning evidence-chain completeness (stale logs, missing probe logs, incomplete failure record, missing `verification-results.md` entry at time of PA review, T3 re-qualification, missing RTEMS build, build config capture). *Note: As of this V&V review, items PA-NC235-01, -03, -04, -06 are addressed in the evidence package; PA-NC235-02 (probe/log gaps) and -07 (config capture) remain as Minor; PA-NC235-05 (T3 re-qualification) is the same Major as VV-NC235-05.*
3. **V&V and R4 (N17) independent reviews:** This V&V review is the R2 leg; the R4 documentation review of Rev 1.119/1.120 and the evidence README is still pending.
4. **N18 remains FAIL:** NC-EXT-198 / NC-PAL-01 (MPFS watchdog binding unwired) is unchanged; no target/HIL evidence exists.
5. **No NC closed, no sprint completed:** The candidate correctly makes no closure claims.

---

## 7. What this review does NOT establish

- **No NC closure.** NC-EXT-235 remains **CORRECTED — PENDING INDEPENDENT REVIEW** (Major evidence claim / Minor test harness). Closure requires the Major findings above to be resolved and the corrected content re-reviewed.
- **No T3 qualification.** The test scripts are changed; T3 is explicitly **NOT FULLY QUALIFIED** pending independent R2 delta review and hosted execution record.
- **No N18 / flightworthiness / target timing evidence.** The RTEMS cross-build passes but only proves the seam is absent; it does not exercise the target, the watchdog binding (NC-PAL-01), or any HIL/N18 condition. GAP-A5 (Renode fidelity) remains.
- **No requirement compliance beyond the tested gates.** The 50/50 CTest pass on host configurations demonstrates that the corrected test suites execute without failure on those configurations. It does not prove absence of defects in untested paths, untested requirements, or target-specific behaviour.
- **No release, certification, or approval authority.** This is an independent V&V assessment of evidence and process only. The owner/authorized human authority retains every disposition, risk-acceptance, NC-closure, and release decision.

---

## 8. Confidence and uncertainty

- **High confidence** for findings based on direct file/git/log inspection, recomputed hashes, and source-code analysis of the test logic.
- **Medium confidence** for precise execution timestamps (filesystem mtimes are the only time evidence; logs carry `*_EXIT` markers but not full ISO timestamps).
- **Low confidence** for anything depending on the unretained pre-fix probe runs ("3/60", "200/200", "20/200") — these are documented as absent and not used as evidence in this review.

---

## 9. Follow-up entry criteria for corrected-content re-review

A corrected-content V&V re-review can be opened when:

1. T3 independent R2 delta review of `test_pus.c` and `test_link_uart.c` is complete and recorded in `tool-qualification.md` with review reference.
2. PA review FRICK-REVIEW-0440 Major findings are resolved and the corrected content re-reviewed to PASS.
3. R4 documentation review of Rev 1.119/1.120 and the evidence README is complete.
4. Stale pre-edit logs in the evidence directory are labelled `SUPERSEDED` or removed.
5. Each gate log records its effective `FF_*` build configuration (or `CMakeCache.txt` hash).

---

**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (independent R2 profile)  
**Date:** 2026-11-03  
**Session:** Independent sub-agent; no author context inherited