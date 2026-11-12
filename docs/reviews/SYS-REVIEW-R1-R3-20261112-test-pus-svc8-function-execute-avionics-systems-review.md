# Avionics/Systems Review — PUS svc8_function execute-function test coverage (test-only)

| | |
|---|---|
| **Document id** | SYS-REVIEW-R1-R3-20261112-test-pus-svc8-function-execute-avionics-systems-review.md |
| **Reviewer profile** | R1 / R3 (Senior Aeronautics and Space Avionics Systems Engineer), independent review |
| **Independence declaration** | Reviewer had no part in authoring or executing the change under review; fresh context from repository contents only. |
| **Reference revision** | Branch `main`, HEAD `3b04cb2`; working tree modification: `tests/unit/test_pus.c` |
| **Items under review** | `tests/unit/test_pus.c` (new `test_function_execute_bounded`) |
| **Date** | 2026-11-12 |
| **Scope** | Avionics/system-level review only: system-level risk, target/hardware items (NC-PAL-01, GAP-A5), GNC/safety/FDIR/operations interfaces, cross-disciplinary consistency, appropriateness of svc8_function (TC[8,1]) coverage, commit/push blocking findings. Excludes: software V&V detail (R2), documentation (R4), PA (N20). |
| **Norms applied** | N1-N22 (see `docs/rules.md`); ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 as referenced by the project baseline |
| **Authority statement** | This review is engineering advice only. It does not certify, qualify, accept, or release any product. All safety-critical and regulatory decisions remain with authorized humans. |

---

## 1. Summary of the change under review

1. `tests/unit/test_pus.c` — added `test_function_execute_bounded` verifying PUS service 8 subservice 1 (TC[8,1] "execute function") end-to-end through `FF_PusIngest`: nominal execute of function id 7 with args `[0xCC, 0xDD]`, unknown-id rejection (`FF_NOT_FOUND`, handler not called), and empty-payload rejection (`FF_BAD_SIZE`). Test registered in `FF_TEST_MAIN` at `tests/unit/test_pus.c:1153`.

Production code (`src/`) is untouched. No `tests/CMakeLists.txt` change is required for this test because `test_pus` is already an executable (`tests/CMakeLists.txt:16`) and the test is self-registered in `FF_TEST_MAIN`. The change is host-executable only (`test_pus` is a native host unit test). No GNC, safety, FDIR, operations interface, or hardware change.

Per the change context: NC-EXT-19/90, NC-PAL-01, GAP-A5, NC-EXT-211, N22-TS5-47 are NOT touched and remain OPEN.

## 2. System-level findings

| Finding id | Severity | Blocks commit/push | Item / file:line | Finding | Evidence / basis | Recommendation |
|---|---|---|---|---|---|---|
| SYS-R1/R3-001 | Minor | No | `tests/unit/test_pus.c` (whole file) | The change introduces no system-level risk. It is test-only: no production source file under `src/` is modified, no ICD, no timing/power/thermal/radiation/EMC parameter change, no new fault-containment or independence claim. | `git diff` shows only `tests/unit/test_pus.c`; `src/` untouched. All gates reported green (build, CTest, Lean, trace, format, license). | None required. Record as reviewed. |
| SYS-R1/R3-002 | Minor | No | NC-PAL-01 / GAP-A5 / NC-EXT-19/90 / NC-EXT-211 / N22-TS5-47 | NC-PAL-01 (MPFS MSS hardware watchdog binding NOT WIRED), GAP-A5 (target/HIL evidence), NC-EXT-19/90, NC-EXT-211, N22-TS5-47 are NOT touched by this change and remain OPEN. The test is host-native and exercises no PAL watchdog hook, no RTEMS/MPFS binding, and no target peripheral. | `git diff` confirms no `src/core/pal/*`, no `board/*`, no driver file changed. `docs/status/handover.md` records all listed NCs as OPEN. | None required. Do not close or re-scope any OPEN NC on the basis of this test-only change. |
| SYS-R1/R3-003 | Minor | No | GNC / safety / FDIR / operations interfaces | No GNC, safety, FDIR, or operations interface is impacted. PUS service 8 is a ground-commandable function-execute service; only test coverage is added, with no production behaviour change and no new command authority, argument limit, or monitoring path. | `svc8_function` production implementation unchanged at `src/pus/pus_services.c:546-564`; registration `{8u,1u,svc8_function}` at `:705` unchanged. No `src/` change. | None required. If flight use of TC[8,1] is later enabled, it must be re-reviewed for command authority, argument validation, and FDIR interaction. |
| SYS-R1/R3-004 | Minor | No | `tests/unit/test_pus.c:1067-1147` vs existing tests | Cross-disciplinary consistency: `test_function_execute_bounded` follows the established `test_pus.c` patterns — `FF_PalInit/FF_EvsInit/FF_TimeInit/FF_PusInit/FF_PusSetBackend` preamble (`test_ping_roundtrip:159-163`, `test_service9_time:439-443`, `test_param_set_bounded:983-987`), `FF_PusSetTmSink(tm_sink)` (`test_param_set_bounded:988`), `FF_CcsdsEncodeTc` + `FF_PusIngest` end-to-end oracle (`test_memory_dump_bounded:272-274`), explicit nominal and out-of-bound expected results per N22. | Side-by-side comparison with `test_service9_time` and `test_param_set_bounded`. | None required. |
| SYS-R1/R3-005 | Minor (positive) | No | `tests/unit/test_pus.c:1067-1147` vs `src/pus/pus_services.c:546-564` | Adding test coverage for `svc8_function` (PUS service 8 subservice 1, ECSS-E-ST-70-41C "execute function") is appropriate. The test oracle matches the production implementation exactly: (a) nominal `len>=1` with registered id calls `fn(&d[1], len-1)` and returns its status; (b) unknown id returns `FF_NOT_FOUND` and the handler is not called (`svc8_function:563`); (c) `len<1` returns `FF_BAD_SIZE` (`svc8_function:549-550`). TM[8,2] execution report `{func_id, status}` emitted (`svc8_function:556-559`). Coverage spans nominal and applicable out-of-bound cases against explicit expected results (N22). | `src/pus/pus_services.c:546-564` (`svc8_function`), `:705` (registration `{8u,1u,svc8_function}`), `src/pus/pus.h:47` (`FF_Pus_FunctionHandler_t`), `:164-181` (`FF_PusRegisterFunction`). Test: `:1086-1106` (nominal), `:1109-1127` (unknown id), `:1129-1146` (bad size). | None required. |
| SYS-R1/R3-006 | Minor | No | `tests/unit/test_pus.c:44-60`, `:1077-1080` | Minor consistency observation: `g_func_exec_id` is declared (`:45`) and reset (`:1078`) but the `FF_Pus_FunctionHandler_t` signature (`src/pus/pus.h:47`) carries no `func_id`, the handler never writes `g_func_exec_id`, and nothing reads it. Function-ID end-to-end is already covered by `stream_has_function_report(7u,0u)` (`:1106`). No build/system effect (`-Wall -Wextra -Werror` clean; file-scope static). | `src/pus/pus.h:47` (handler signature), test `:1077-1080` (reset), `:1106` (TM report check). | None required (non-blocking). Optionally remove `g_func_exec_id` or retain as harmless reserve; no commit/push impact. |

## 3. Systems conclusion

The change is **test-only** with no production-source modification. From an avionics/system-level perspective:

- **System-level risk:** None introduced (SYS-R1/R3-001).
- **Target/hardware items:** NC-PAL-01, GAP-A5, and the listed OPEN NCs are unaffected (SYS-R1/R3-002).
- **GNC / safety / FDIR / operations:** No interface impact (SYS-R1/R3-003).
- **Cross-disciplinary consistency:** Test is consistent with existing `test_pus.c` patterns (SYS-R1/R3-004).
- **svc8_function coverage:** Appropriate and oracle-faithful to the production implementation, including nominal and out-of-bound legs (SYS-R1/R3-005).
- **Minor consistency observation:** `g_func_exec_id` is declared/reset but unused by the handler; end-to-end func_id is covered by the TM[8,2] report check (SYS-R1/R3-006).

**No finding blocks commit/push.** All findings are Minor and none are blocking. The normal project gates (build, CTest, Lean, trace, format, license) must still pass per N7/N15/N16 before commit/push, and this review does not waive any separate safety, security, regulatory, or contractual gate.

---

## 4. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|---|---|---|---|
| R1/R3 (avionics systems, N9) | Independent subagent (this review) | 2026-11-12 | APPROVED — Minor findings only, none blocking commit/push |

---

## 5. Follow-up records

No NC raised by this review. Existing OPEN NCs (NC-PAL-01, GAP-A5, NC-EXT-19/90, NC-EXT-211, N22-TS5-47) remain open for owner disposition and are not closed by this review.

| NC | Severity | Item / Requirement | Finding | Disposition | Owner | Status |
|---|---|---|---|---|---|---|
| — | — | — | No new NC | Not applicable | — | Not applicable |

---

*Engineering advice only. No airworthiness, human-rating, qualification, release, or flight-readiness approval is implied or granted by this document.*
