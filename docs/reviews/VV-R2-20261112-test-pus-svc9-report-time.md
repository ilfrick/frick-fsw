# Independent SW V&V Review — PUS service 9 time-report unit test (`test_service9_report_time`)

| Field | Value |
|---|---|
| Review ID | VV-R2-20261112-TEST-PUS-SVC9-REPORT-TIME |
| Reviewer profile | Senior Aeronautics and Space SW V&V Engineer (independent-context R2, norm N9/N10) |
| Repository | /home/nicola/fsw/frick-fsw, branch `main`, HEAD `29cba32` |
| Change under review | Uncommitted working tree: `tests/unit/test_pus.c` — added `test_service9_report_time` plus helper `stream_has_time_report`. Verifies PUS TC[9,2] (report time) → TM[9,2] end-to-end: nominal frozen-time report, time-advance re-report, unknown-subservice refusal (subsystem 99 → `FF_NOT_FOUND`). No `src/` production files modified — verifies existing `svc9_report_time`. |
| Date | 2026-11-12 |
| Governing norms | N1-N22 (docs/rules.md); in particular N10 (test-script/tool qualification), N21 (sequencing), N22 (nominal + out-of-bound expected-result tests), N7 (commit/push authority). ECSS-E-ST-40C Rev.1 / ECSS-Q-ST-80C Rev.2 context. ECSS-E-ST-70-41C (PUS-C) service 9 reference. |
| Criticality | ECSS Class B target (project target; no controlled mission assignment evidenced — NC-EXT-5/11 class) |
| Scope | `tests/unit/test_pus.c` only (helper lines 91-120; test function lines 510-556; registration line 1230). Host-executable CTest unit test; NOT target/HIL/Renode/QEMU; NOT GAP-A5/NC-PAL-01/OBCP; no production-code behavior change. Existing NCs NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 untouched. |

> **Authority statement.** This review is an independent analysis artifact. It has no approval, qualification, acceptance, certification, or flight-readiness authority. Commit/push authorization (norm N7), safety/security acceptance, PA sign-off (norm N20), and any residual-risk decision remain with the designated FSW engineer and the human authorities. This report may recommend "do not proceed"; it may not authorize proceeding.

## 1. Change under review (verified diff)

`git diff tests/unit/test_pus.c` adds:

- Helper `stream_has_time_report(uint32_t coarse, uint16_t fine)` at lines 91-120: scans the captured TM stream for a packet with service `FF_PUS_SERVICE_TIME` (9), subservice `2`, and a 6-octet CUC payload `{coarse[4] BE, fine[2] BE}` at TM payload offset 19.
- Test `test_service9_report_time` at lines 510-556:
  1. **Nominal time report** (lines 518-538): freeze time (`FF_PalTimeTestFreeze`), set UTC = 42 s (`FF_TimeSetUtcNs`), build TC[9,2] with `ack_flags = 0`, ingest, assert `stream_has_time_report(42u, 0u)`.
  2. **Time-advance re-report** (lines 540-546): set UTC = 100 s, re-ingest the same TC frame, assert `stream_has_time_report(100u, 0u)` — coarse tracks the new time.
  3. **Unknown subservice refusal** (lines 548-555): retune `tc.subservice` to 99, ingest, assert `FF_PusIngest == FF_NOT_FOUND` and no TM[9,2] in the stream; unfreeze time.
- Test registered in `FF_TEST_MAIN` at line 1230.
- Total: ~13 new checks, 263 → 277 checks, 0 failures (per change context).

## 2. Evidence basis

| ID | Evidence | Relevance |
|---|---|---|
| E-1 | `tests/unit/test_pus.c:91-120` | `stream_has_time_report` oracle: service byte 7, subservice byte 8, CUC payload bytes 19-24 |
| E-2 | `tests/unit/test_pus.c:510-556` | New test: 3 cases, explicit expected results (norm N22), frozen time, exact return codes |
| E-3 | `src/pus/pus_services.c:590-602` | Production `svc9_report_time`: reads CUC via `FF_TimeGetCuc`, writes coarse/fine big-endian (`wr_u32`/`wr_u16`, lines 599-600), emits `FF_PusSendTm(FF_PUS_SERVICE_TIME, 2u, payload, 6)` |
| E-4 | `src/pus/pus_services.c:84-95` | `wr_u32`/`wr_u16` confirmed big-endian: `p[0] = (v >> 24)`, `p[1] = (v >> 16)`, … |
| E-5 | `src/pus/pus.c:436-446` | `FF_PusIngest` dispatch: unknown (service,subservice) → `ff_pus_find == NULL` → returns `FF_NOT_FOUND` (ack_flags = 0 so no verification TM) |
| E-6 | `src/transport/ccsds/ccsds.c:200-207` + `ccsds.h:19`, `pus_types.h:70-71` | TM layout confirmed: primary header 6 bytes (`FF_CCSDS_HDR_LEN`), TM secondary header 13 bytes (`FF_PUS_TM_SEC_HDR_LEN`); service at byte 7, subservice at byte 8, payload at byte 19 (= 6 + 13) |
| E-7 | `src/core/time/time.c:209-227` | `FF_TimeGetCuc`: `coarse = ns / 1e9 + leap`, `fine = (ns % 1e9) * 65536 / 1e9`; for 42e9 ns → coarse 42, fine 0; for 100e9 ns → coarse 100, fine 0 |
| E-8 | `src/core/pal/pal_posix.c:604-613` | `FF_PalTimeTestFreeze`/`Unfreeze`: test-only seam under `#ifdef FF_TEST_HOOKS`; sets global `g_utc_test_frozen_ns` / flag |
| E-9 | `tests/ff_test.h:17-24` | `FF_TEST_ASSERT`: non-aborting, increments `ff_test_checks`/`ff_test_failures`; `main` returns non-zero iff any failure |
| E-10 | Reported gate results (change context) | `make build` exit 0 / 0 warnings; CTest exit 0; Lean `lake build`; trace PASS (120/76/30/14/68); format-check PASS; license OK |
| E-11 | `tests/CMakeLists.txt:16, 110` | `test_pus` target from `unit/test_pus.c`, CTest target, host-only |
| E-12 | Independent re-verification (this review) | Static comparison of test expected values against `svc9_report_time`, `FF_PusIngest` dispatch, TM encoder, `FF_TimeGetCuc`; all oracles confirmed byte-exact |

Rebuild/re-execution from current sources was not possible in this environment (build tree owned by another user, stale `/workspace` CMakeCache). The review rests on source-level oracle verification (E-1…E-7, E-12) plus the reported gate results (E-10), which are internally consistent with the source inspection.

## 3. Norm-by-norm confirmation

| Item | Question | Result | Evidence |
|---|---|---|---|
| 1 | Coverage sufficiency (nominal + advance + out-of-bound) | Adequate for claimed scope | E-2: 3 cases each with explicit expected results; production behavior exercised end-to-end |
| 2 | Oracle correctness (exact CUC bytes, exact return codes) | Verified byte-exact | E-1/E-3/E-4/E-6/E-7: payload at bytes 19-24 big-endian; `FF_NOT_FOUND` for unknown subservice |
| 3 | Determinism (single-threaded, frozen time, no concurrency) | Verified | E-2: `FF_PalTimeTestFreeze` at line 521, exact UTC values, no threads/random/timing |
| 4 | Tool qualification (no new scripts/tools, no T3 impact, no N10 re-qualification) | No impact | Change context: no new test scripts; `test_pus` is existing CTest target; T3 untouched |
| 5 | Regression (existing test behavior, CTest exit 0) | None | E-10: CTest exit 0; all prior checks retained |
| 6 | Blocking V&V findings | None | All findings Minor/Comment; no Major |

## 4. Findings

### VV-R2-001 — Minor — CUC fine field always 0; sub-second time-report path not exercised

**Description.** Both time-report assertions use `fine = 0` (`stream_has_time_report(42u, 0u)` at line 538; `stream_has_time_report(100u, 0u)` at line 546). `svc9_report_time` writes the fine field from `FF_TimeGetCuc` (line 600: `wr_u16(&payload[4], fine)`), and `FF_TimeGetCuc` computes `fine = (ns % 1e9) * 65536 / 1e9` (`time.c:224`), which is exact only on the 128-ns dyadic grid (`set_time_cuc` comment at `test_pus.c:830-832`). The test never exercises a non-zero fine value, so the fine-byte encoding (`wr_u16` big-endian at `pus_services.c:86-87`) and sub-second CUC quantization are not independently verified by this test.

**File:line references.** `tests/unit/test_pus.c:538, 546` (fine hard-coded 0); `src/pus/pus_services.c:600` (fine written to payload); `src/core/time/time.c:224` (fine computation).

**Observed result.** Nominal and advance cases pass with fine = 0 (gate E-10). No assertion covers `fine != 0`.

**Blocks commit/push?** No.

**Disposition.** Extend the test (or a sibling case) with a non-zero fine instant — e.g. `set_time_cuc(42u, 8192u)` (125,000,000 ns fraction, exact dyadic) — and assert `stream_has_time_report(42u, 8192u)`. Track as a verification improvement under norm N21; functional success path verified, sub-second path tracked, does not stop this change.

---

### VV-R2-002 — Minor — Test isolation: `FF_PalTimeTestUnfreeze()` is after all assertions; a failed assertion leaves time frozen for subsequent tests

**Description.** `test_service9_report_time` freezes time at line 521 (`FF_PalTimeTestFreeze(0u)`) and only unfreezes at line 555, after every assertion. `FF_TEST_ASSERT` is non-aborting (`ff_test.h:20-23`: increments `ff_test_failures` and continues). If any assertion between lines 522 and 554 fails (e.g. `stream_has_time_report` at 538 or 546), line 555 never executes and the UTC seam remains frozen (`g_utc_test_frozen = 1`, `pal_posix.c:604-613`). A later `FF_TimeSetUtcNs` then only updates the frozen value and never restores the offset, so subsequent tests in the same process (`test_verification_missing_subservices` line 562, `test_hk_periodic_emission` line 648 which re-freezes at 666, `test_hk_coarse_wrap_due` line 862, etc.) execute against a contaminated time base. This is the same failure-safe-teardown gap previously flagged for the existing freeze/unfreeze tests (PA-NC235-06 in FRICK-REVIEW-0440; finding references `test_pus.c:517, :657, :704, :797, :838, :904`). The new test extends an existing, accepted pattern rather than introducing a new class of defect.

**File:line references.** `tests/unit/test_pus.c:521` (freeze), `:555` (unfreeze after all assertions); `:538, :546` (assertions whose failure would skip unfreeze); `tests/ff_test.h:17-24` (non-aborting assert); `src/core/pal/pal_posix.c:604-613` (seam globals).

**Observed result.** All 13 checks pass in current execution order (gate E-10), so unfreeze runs and no contamination is observed. The latent isolation defect is unexercised by the passing run.

**Blocks commit/push?** No.

**Disposition.** Add a guaranteed teardown: capture the freeze state and unfreeze in a cleanup path, or re-freeze/re-init at the start of each dependent test, so a failing assertion cannot leave the clock frozen for later tests. Alternatively record the limitation as an accepted observation under norm N21 (pattern consistent with existing tests; tracked, does not stop this change).

---

### VV-R2-003 — Minor — Test ordering/initialization coupling: `test_service9_report_time` does not initialize the PUS layer itself

**Description.** `test_service9_report_time` (line 510) calls only `FF_TimeInit()` (line 518). It does **not** call `FF_PusInit()`, `FF_PusSetBackend(&FF_PusBackendOpen)`, or `FF_PusSetTmSink(tm_sink)`. It relies on `test_ping_roundtrip` (line 187) having run earlier in `FF_TEST_MAIN` order (line 1230) and having initialized the global PUS backend, TM sink, and service registry. `FF_PusSendTm` (`pus.c:173-176`) returns `FF_NOT_READY` if `g_backend == NULL`, and the TM sink would be NULL if never set, so `stream_has_time_report` would find an empty stream. If `FF_TEST_MAIN` order changes, `test_ping_roundtrip` is removed, or tests are selectively executed, this test fails for reasons unrelated to service 9. Same pre-existing coupling as `test_service9_time` (line 460, also lacks PUS initialization).

**File:line references.** `tests/unit/test_pus.c:510-556` (test body, no PUS init); `:187-195` (`test_ping_roundtrip` performs `FF_PusInit`, `FF_PusSetBackend`, `FF_PusSetTmSink`); `:1230-1234` (`FF_TEST_MAIN` order: ping → memory → svc9_time → svc9_report_time → …); `src/pus/pus.c:173-176` (`FF_NOT_READY` when `g_backend == NULL`).

**Observed result.** Passes in current order because `test_ping_roundtrip` runs first (gate E-10). Fragile to test-list reordering.

**Blocks commit/push?** No.

**Disposition.** Add the PUS initialization preamble (`FF_PalInit`, `FF_EvsInit`, `FF_TimeInit`, `FF_PusInit`, `FF_PusSetBackend`, `FF_PusSetTmSink`) to `test_service9_report_time` so the test is order-independent, consistent with `test_ping_roundtrip`, `test_memory_dump_bounded`, `test_param_set_bounded`, and `test_function_execute_bounded`. Track under norm N21; does not stop this change.

## 5. Norm-by-norm summary (compliance statement)

| Norm | Requirement | Assessment |
|---|---|---|
| N10 | Test scripts/V&V tools reviewed & qualified per ECSS | **N/A — no new scripts/tools; existing CTest target only; no T3 impact; no re-qualification trigger** |
| N21 | Functional bugs corrected immediately; other findings as NCs for later disposition | **Compliant — all findings are Minor, tracked, none block this change** |
| N22 | Nominal + out-of-bound cases vs explicit expected results | **Compliant — nominal (538), advance (546), out-of-bound subservice (553-554), each with explicit expected results** |
| N7 | Only designated FSW engineer commits/pushes after all gates | **Not a V&V gate finding — commit/push authority remains with designated agent** |
| N20 | Independent PA review | **Deferred to PA (N20) profile; this is the R2 V&V review only** |

## 6. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Summary.** The test coverage is sufficient for the claimed scope (nominal time report, time-advance re-report, unknown-subservice refusal), each with explicit expected results per norm N22. The test oracles are correct and byte-exact: service at TM byte 7, subservice at byte 8, 6-octet big-endian CUC payload at bytes 19-24, matching `svc9_report_time` (`pus_services.c:590-602`), the TM encoder (`ccsds.c:200-207`), and `FF_TimeGetCuc` (`time.c:209-227`). The test is deterministic: single-threaded, time frozen via `FF_PalTimeTestFreeze`, exact UTC instants, no concurrency or randomization. Tool qualification is unaffected: no new scripts/tools, no T3 impact, no N10 re-qualification trigger. No regression: CTest exit 0, all prior checks retained, build clean with 0 warnings.

Three Minor findings (VV-R2-001 to VV-R2-003) concern coverage of the sub-second fine field, test-isolation teardown on assertion failure, and test-ordering/initialization coupling. None is Major; none blocks commit/push. They should be corrected or justified under norm N21 before or as part of the next compliant cycle.

No V&V finding blocks commit/push.

## 7. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|---|---|---|---|
| R2 (SW V&V, norm N9/N10) | Independent Senior SW V&V Engineer | 2026-11-12 | APPROVED WITH COMMENTS (3x Minor, none blocking) |

## 8. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|---|---|---|---|---|---|---|---|---|
| VV-R2-001 | Minor | `test_pus.c:538, 546` | CUC fine field always 0; sub-second path untested | Add non-zero fine case (`set_time_cuc(42, 8192)`) | Re-run CTest; trace PASS | V&V engineer | Open | Next cycle |
| VV-R2-002 | Minor | `test_pus.c:521, 555` | `FF_PalTimeTestUnfreeze` after all assertions; failure leaves clock frozen | Guaranteed teardown (unfreeze on failure path) or document accepted pattern | Re-run CTest; confirm subsequent tests unaffected | V&V engineer | Open | Next cycle |
| VV-R2-003 | Minor | `test_pus.c:510-556` | No PUS init; coupled to `test_ping_roundtrip` order | Add PUS initialization preamble to test | Re-run CTest; confirm order-independence | V&V engineer | Open | Next cycle |

---

Committed and pushed with the reviewed change (norm N11) — only by the designated `aeronautics-space-fsw-engineer` agent after all gates/norms pass (norm N7).
