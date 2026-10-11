# Review Report — Code-level test coverage: gaps and added tests (R2)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0075                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (Senior SW V&V engineer)            |
| Reference revision                | `b714c56` (branch `main`), 2026-10-11  |
| Items under review                | `tests/` (all `tests/unit/*.c`, `tests/ff_test.h`, `tests/CMakeLists.txt`), `.github/workflows/ci.yml`, `tools/scripts/coverage.py`, `build/coverage/coverage.json` (gcov report), `verification/lean`, `tools/scripts/run_sim.sh`, `src/apps/app_renode_test.c`, `docs/requirements/traceability.yaml`, and the `src/` modules PAL, SB, EVS, ES, TBL, Time, HAL, CCSDS, PUS, Math |
| Date                              | 2026-10-11                             |
| Scope                             | (a) Inventory how the system is currently tested; (b) code/function/branch coverage gap analysis driven by the gcov report; (c) addition of a self-contained unit-test file closing the highest-value CORE gaps; (d) residual gaps with rationale. |
| Independence declaration          | Reviewer wrote and executes none of the reviewed test infrastructure or `src/`; fresh session with no author context. Only two new files were created (`tests/unit/test_r2_gap.c`, this report); **no shared file was modified** (not `tests/CMakeLists.txt`, not existing tests, not `src/`). The CMake wiring, the traceability-manifest update and the `make coverage` regeneration are left to the FSW engineer (§8). |
| Norms applied                     | N1, N3, N4, N5, N6, N9, N10, N11, N15, N20 (see `docs/rules.md`) |

## 1. Scope

This review evaluates the **implementation-code coverage** of frick-fsw at
`b714c56`. It is the code-level companion of the R3
requirements-level review (`FRICK-REVIEW-0076`,
`docs/reviews/2026-10-11-requirements-test-gap-r3-report.md`), run in parallel on
the same revision. Where R3 maps *requirements* to evidence and adds
system/integration flows, R2 works from the **gcov report** and closes
*functions, error paths and boundary branches* that were never executed.

The review was performed with: the existing unit suites under `tests/unit/`
(`test_{ccsds,sb,tbl,pus,time,hal,math,sdls,gem,gem_tsn,netloop,gpio,serial,mem}.c`)
driven by `ctest`; the CI pipeline (`.github/workflows/ci.yml`: host Debug +
Release build, unit tests, AddressSanitizer suite, `lwIP` adapter test, MISRA
scan, emulated-target run, Renode functional validation); the Lean 4 verification
(`make verify-lean`); the native simulator smoke (`frick_fsw` boot); and the
Renode functional-test application (`src/apps/app_renode_test.c`, norm N18). The
coverage baseline is `build/coverage/coverage.json` produced by
`tools/scripts/coverage.py` (gcov line/function aggregation).

**Excluded.** On-target/flight-hardware verification (GAP-A5), third-party
component internals (lwIP, ASN1SCC, `asn1-pusc-lib`), the Lean proof content, and
the CI/emulator environments themselves (evaluated by their own reviews). The
companion R3 review covers requirement mapping and traceability corrections.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- SRS `FRICK-SRS-001`; SSS `FRICK-SSS-001`
- Harness: `tests/ff_test.h`; existing tests `tests/unit/test_*.c`
- Coverage tool: `tools/scripts/coverage.py` (T12); baseline
  `build/coverage/coverage.json` — lines 1917/2920 (65.6%), functions 257/302
  (85.1%)
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2,
  CCSDS 301.0-B-4 (CUC)

## 3. Current test approach (inventory)

**Fact.** The test architecture at `b714c56` has four independent layers:

1. **Host unit tests (`ctest`).** `tests/CMakeLists.txt` builds one executable
   per module on the minimal `tests/ff_test.h` harness (`FF_TEST_ASSERT` /
   `FF_TEST_MAIN`); each executable links the static `frick_fsw_core` library and
   is registered both as a CMake target and a `ctest` test. Registration is
   feature-gated (`FF_BUILD_HAL`, `FF_USE_LIBM`, `FF_BUILD_SDLS`,
   `FF_BUILD_DRIVERS`, `FF_USE_LWIP`). This is the primary functional evidence.
2. **CI pipeline.** `.github/workflows/ci.yml` runs: Debug + Release builds and
   `ctest` on both; an **AddressSanitizer** `ctest` pass over the same suites;
   the `lwIP` adapter fetch/build/test; a `cppcheck` MISRA C:2012 scan;
   `tools/scripts/emulated_run.sh`; the Renode environment self-test and
   functional validation; the Lean `lake build`; the traceability gate
   (`tools/scripts/trace.py`); and a documentation/licence job. A
   `--coverage`/`metrics.py --check` step guards metrics-panel currency.
3. **Formal verification.** `verification/lean` discharges requirement theorems
   (norm N1/N3); invoked by `make verify-lean`.
4. **Emulated-target functional validation.** `make renode` (norm N18) runs
   `src/apps/app_renode_test.c` on the emulated PolarFire SoC, exercising SB
   publish/subscribe, EVS, Time and (on target) a HAL GPIO MMIO round trip, and
   printing `RENODE_TEST: PASS`.

**Assessment.** The approach is sound and layered, but the *host* layer is
unevenly distributed: the gcov report shows entire modules that no `ctest` ever
executes (ES), or where large parts of the public surface are never called
(EVS, PUS services, TBL dump/revert, PAL teardown/inspection, Time MET/UTC).

## 4. Gap analysis (function / branch, with evidence)

**Fact.** Baseline function coverage from `build/coverage/coverage.json` is
257/302 (85.1%); the per-file gcov reports (`build/coverage/gcov/*.gcov`,
dated 2026-10-11 15:28) list the following functions with `called 0` (the
function-body evidence is the `gcov` `called 0 returned 0%` line). The ES module
(`src/core/es/es.c`) has **no `.gcov` report at all**: it was not linked into any
test executable, i.e. 0/8 functions executed.

| # | Module (file) | Uncovered functions / branches (baseline) | Evidence | Severity |
| - | ------------- | ----------------------------------------- | -------- | -------- |
| G1 | `src/core/es/es.c` | **Whole module, 8/8 functions**: `FF_EsInit`, `FF_EsRegisterApp`, `FF_EsStartApp`, `FF_EsStartAll`, `FF_EsGetId`, `FF_EsGetState`, `FF_EsGetAppCount` (+ `ff_es_find`). No unit test; only `ci:simulator-smoke`. | no `es.c.gcov` in `build/coverage/gcov/`; `traceability.yaml` SRS-REQ-014 → `ci:simulator-smoke` | Major |
| G2 | `src/core/evs/evs.c` | 4/5 functions: `FF_EvsSetListener`, `FF_EvsSend`, `FF_EvsGetLog`, `FF_EvsGetTotalCount` (only `FF_EvsInit` reached, via `test_pus.c`). Circular-log wrap, listener dispatch and NULL handling unexecuted. | `evs.c.gcov` `called 0` ×4 | Major |
| G3 | `src/core/sb/sb.c` | `FF_SbUnsubscribe` unexecuted; branch coverage missing: `FF_SbPublish(NULL)`, invalid-app `FF_SbReceive`, subscribe to a non-existent pipe, subscription-table saturation (`FF_NO_RESOURCE`), queue-full `dropped` path, wildcard delivery. | `sb.c.gcov` `called 0`; `test_sb.c` exact-match only | Minor |
| G4 | `src/core/tbl/tbl.c` | `FF_TblRevert`, `FF_TblDump` unexecuted; branches missing: registration size/argument rejects, load wrong-size, validator rejection, activate-before-load, capacity (`FF_TBL_MAX_TABLES`). | `tbl.c.gcov` `called 0` ×2; `test_tbl.c` load/activate only | Minor |
| G5 | `src/core/time/time.c` | `FF_TimeGetMetNs`, `FF_TimeGetMetMs`, `FF_TimeGetUtcNs` unexecuted; `FF_TimeGetCuc(NULL, …)` NULL branches unexercised. | `time.c.gcov` `called 0` ×3 | Minor |
| G6 | `src/core/pal/pal_posix.c` | 10/28 unexecuted: `FF_PalShutdown`, `FF_PalTaskDelayTicks`, `FF_PalTaskYield`, `FF_PalTaskSelf`, `FF_PalMutexDelete`, `FF_PalQueueDelete`, `FF_PalQueueCount`, `FF_PalGetCpuLoadPpt`, `FF_PalLog`, `FF_PalPanic`; queue full/timeout branches (`FF_BUFFER_FULL`, timed-wait `FF_TIMEOUT`). | `pal_posix.c.gcov` `called 0` ×10 | Minor |
| G7 | `src/pus/pus.c` | `FF_PusGetBackend`, `FF_PusPeekTmCounter`, `FF_PusGetTcCount`, `FF_PusEmitEvent` unexecuted. | `pus.c.gcov` `called 0` ×4 | Minor |
| G8 | `src/pus/pus_services.c` | 19/25 unexecuted: `wr_u16`, `wr_u32`, `find_provider`, `reg_provider`, all four register APIs, `svc3_create/delete/enable/disable/report`, `svc8_function`, `svc9_report_time`, `svc11_stub`, `svc20_set`, `svc20_report`. | `pus_services.c.gcov` `called 0` ×19; SRS-REQ-052 → `ci:docs` | Major |
| G9 | `src/transport/ccsds/ccsds.c` | All functions reached (TC decode via `test_pus.c`), but the **error/boundary branches** of `FF_CcsdsEncodeTc`/`FF_CcsdsDecodeTc` (NULL args, undersized buffer, wrong packet type, truncated PDL, malformed CRC) have no direct codec test — `test_ccsds.c` covers TM only. | `test_ccsds.c` scope; `ccsds.c` no direct TC test | Minor |
| G10 | `src/hal/hal.c` | Registry reachable via `test_hal.c`, but invalid-class `FF_HalCount`/`FF_HalGet` and negative-class registration, plus cross-class `FF_HalTotal` accounting, are untested. | `test_hal.c` covers `FF_HAL_ETH`/`FF_HAL_CAN` only | Comment |
| G11 | `src/math/ff_math.c` | Helpers covered; NaN-through-`FF_MathClamp` and infinity `FF_MathIsFinite` edge behaviour untested. | `test_math.c` scope | Comment |
| G12 | `src/pus/adapters/pus_adapter_n7.c` | `n7_decode_tc` unexecuted — the N7 backend is not compiled in the coverage configuration (`FF_USE_PUS_ASN1` requires generated code via `make pus-asn1`). | `pus_adapter_n7.c.gcov` `called 0` ×1 | Comment |

## 5. Tests added (`tests/unit/test_r2_gap.c`)

A single self-contained file using `tests/ff_test.h`, exercising **public APIs
only** (no framework change, no dynamic allocation). It formats clean under the
project `.clang-format` (verified with `clang-format --dry-run --Werror`) and
compiles clean under `-Wall -Wextra` with the project `-Wall`/`-Wextra` host
toolchain. It is configuration-robust: the libm section is guarded by
`FRICK_USE_LIBM`, and the HAL section is guarded by `FF_TEST_NO_HAL` (default
build keeps it).

**Execution result (evidence).** Built and run on the host against
`build/coverage/libfrick_fsw_core.a` on 2026-10-11:

- default (`FF_USE_LIBM` + HAL): **487 checks, 0 failures**;
- no-libm: **483 checks, 0 failures**;
- `-DFF_TEST_NO_HAL`: **474 checks, 0 failures**;
- clean under AddressSanitizer (`-fsanitize=address`): 0 failures, no reports.

| Test routine | Module | What it now covers |
| ------------ | ------ | ------------------ |
| `test_evs_listener_and_ring` | EVS (G2) | `FF_EvsSetListener` / `FF_EvsSend` / `FF_EvsGetLog` / `FF_EvsGetTotalCount`; formatted text, listener clear (NULL), circular overrun (`FF_EVS_LOG_DEPTH + 3` → only the last DEPTH retained, oldest index check), `max_events` capping, NULL-destination returns 0 |
| `test_es_registry_and_lifecycle` | ES (G1) | `FF_EsInit`/`Register`/`StartApp`/`StartAll`/`GetId`/`GetState`/`GetAppCount`; NULL/name-too-long/entry-NULL rejects, 1-based ids, LOADED→RUNNING (idempotent start), `FF_APP_FAILED` for unknown id, `FF_NOT_FOUND` start |
| `test_sb_error_paths` | SB (G3) | `FF_SbPublish(NULL)`, `FF_SbGetStats(NULL)`, `FF_SbReceive` invalid app/NULL, subscribe-to-missing-pipe, depth 0/over-max fallback, duplicate pipe, unsubscribe + re-unsubscribe, empty-queue timeout |
| `test_sb_wildcard_and_drop` | SB (G3) | `FF_SB_WILDCARD` fan-out; queue-full `dropped` statistics path |
| `test_sb_subscription_saturation` | SB (G3) | Subscription-table saturation (`FF_NO_RESOURCE` at 96) and slot reuse after `FF_SbUnsubscribe` |
| `test_tbl_errors_and_revert` | TBL (G4) | `FF_TblRevert`; register size/argument rejects, NULL-src/wrong-size load, activate-before-load (`FF_NOT_READY`), revert clears staging (activate after revert → `FF_NOT_READY`), `FF_TblGet` size/NULL |
| `test_tbl_validate_and_dump` | TBL (G4) | `FF_TblDump` (success/`FF_BAD_SIZE`/NULL args/invalid handle) and validator rejection (`FF_ERROR`) |
| `test_tbl_capacity` | TBL (G4) | `FF_TBL_MAX_TABLES` capacity boundary → `FF_TBL_INVALID_HANDLE` |
| `test_time_met_and_utc` | Time (G5) | `FF_TimeGetMetNs`/`Ms`/`FF_TimeGetUtcNs`; monotonic advance, ground-pinned UTC anchor, `FF_TimeGetCuc` NULL-output branches |
| `test_hal_registry_gaps` | HAL (G10) | Invalid/negative class for `FF_HalCount`/`FF_HalGet`/`FF_HalRegister`, NULL name lookup, cross-class `FF_HalTotal` accounting |
| `test_ccsds_telecommand` | CCSDS (G9) | TC encode/decode round trip; encode NULL/undersized rejects; decode short/wrong-type/truncated-PDL/CRC-error rejects; zero-length payload; NULL optional outputs; CRC of a corrupted frame |
| `test_pus_core_gaps` | PUS (G7) | `FF_PusGetBackend`, `FF_PusPeekTmCounter`/`FF_PusNextTmCounter`, `FF_PusGetTcCount`, `FF_PusEmitEvent` (S5 INFO/WARNING/ERROR mapping + oversize reject); register-API NULL rejects |
| `test_pus_service3_hk` | PUS (G8) | S3 create/duplicate/short, report-all (provider-missing `FF_NOT_FOUND`), report-one, disable/enable suppression, delete + double-delete |
| `test_pus_service8_function` | PUS (G8) | S8 success/failure status TM, unknown id, short payload |
| `test_pus_service20_parameters` | PUS (G8) | S20 setter capture, provider report (`(20,4)`), unknown id, short payload |
| `test_pus_service9_and_11` | PUS (G8) | S9 report-time TM; S11 declared stub `FF_NOT_IMPLEMENTED` (both subservices) |
| `test_math_nan_inf` | Math (G11, guarded) | `FF_MathClamp` NaN passthrough, `FF_MathIsFinite` for inf/NaN |
| `test_pal_lifecycle_and_time` | PAL (G6) | Idempotent `FF_PalInit`, `FF_PalGetCpuLoadPpt`, `FF_PalTaskSelf`, `FF_PalTaskDelayTicks`, `FF_PalTaskYield`, `FF_PalLog`, `FF_PalSetUtcNs`/`GetUtcNs` |
| `test_pal_mutex_delete` | PAL (G6) | `FF_PalMutexDelete` and created-then-locked/unlocked mutex, invalid-id rejects |
| `test_pal_queue` | PAL (G6) | `FF_PalQueueCount`/`Delete`; full→`FF_BUFFER_FULL`, timed full→`FF_TIMEOUT`, empty→`FF_TIMEOUT`, NULL/NULL-id rejects |
| `test_pal_task_delete` | PAL (G6) | `FF_PalTaskCreate` (also NULL name), `FF_PalTaskDelete`, self-terminating task via `FF_PalTaskExit` |

**Effect (informal merged-gcov measurement, 2026-10-11).** Merging the new
executions into the coverage-object `.gcda` and re-running `gcov`, all baseline
`called 0` functions listed in G1–G11 are now executed; only the five functions
in §6 remain at zero. Aggregate over the coverage-build object set (same
measurement method as `coverage.py`/`coverage.json`, i.e. including test and
third-party objects): **functions 305/310 (98.4%), lines 2294/2985 (76.9%)**,
versus the baseline panel 257/302 (85.1%) / 1917/2920 (65.6%). The denominator
grew by the 8 ES functions (+65 lines) that are now *linked and executed* but
were absent from the baseline report; on the common denominator the function
ratio moves from 257/310 (82.9%) to 305/310 (98.4%).

> Caveat (uncertainty): the post-change aggregate above is an informal merged
> `gcov` pass, not the authoritative project metric. The authoritative number is
> regenerated by `make coverage` **after** the FSW engineer registers the target
> (§8). The per-function evidence (baseline `called 0` list → now executed) is
> the robust, reproducible claim.

## 6. Residual gaps (what remains uncovered, with rationale)

| Gap | Item | Rationale |
| --- | ---- | --------- |
| GAP-R2-1 | `FF_PalPanic` (PAL) | Declared `noreturn`; it calls `abort()`. Invoking it in a unit test would terminate the whole `ctest` process; not a host-testable unit. Verify by inspection + RTEMS panic-path integration evidence. |
| GAP-R2-2 | `FF_PalShutdown` (PAL) | Destructive lifecycle teardown that only toggles `g_initialized` with no independent observable contract; a host test would corrupt shared PAL state across the suite. Covered by inspection and the RTEMS integration path. |
| GAP-R2-3 | `gem_set_link_up` (`drivers/eth/gem/ff_gem.c`), `netloop_attach_netif` (`drivers/net/ff_net_loopback.c`) | Driver internals requiring mock-MMIO / netif plumbing; owned by the driver suites (`test_gem.c`, `test_netloop.c`), outside the CORE-registry scope of this R2 task. Recommend a follow-up driver-coverage item. |
| GAP-R2-4 | `n7_decode_tc` (`pus/adapters/pus_adapter_n7.c`) | The N7 ASN.1 backend is not compiled unless `FF_USE_PUS_ASN1=ON` and the generated code exists (`make pus-asn1`); absent from the default coverage build. Covered by the mission backend qualification (GAP-A14) and the encode-parity assertion already in `test_pus.c`. |
| GAP-R2-5 | ES `FF_NO_RESOURCE` (app table full) and SB pipe-table full | Require 32 registered applications / 32 pipes; low safety value, high fixture cost. The subscription-saturation path *is* covered. Track as a scheduler/ES integration item. |
| GAP-R2-6 | On-target timing, reset, PHY/electrical and long-duration behaviours | GAP-A5 by construction; only emulated (Renode) and hardware evidence can close these. |
| GAP-R2-7 | Traceability update | `docs/requirements/traceability.yaml` must add `tests/unit/test_r2_gap.c` to the tests lists of SRS-REQ-001/005/011/013/014/025/041/050 and SSS-REQ-002/005/006/009/010, so the new evidence is traceable (norm N4). Left to the FSW engineer (§8). |

## 7. Conclusion

- [x] APPROVED WITH COMMENTS — the added test file closes the Major/Minor code
  coverage gaps (G1–G11); residual items and the mandatory CMake/manifest wiring
  are tracked to closure in §8/§10.
- The two Major findings (G1 ES, G8 PUS services) and the EVS Major finding (G2)
  are **closed by the added host evidence**; the remaining findings are Minor /
  Comment with documented rationale.

## 8. Required integration (FSW engineer — shared files)

This R2 review was instructed not to modify shared files. Apply exactly the
following (no other change is required):

1. **Register the test** in `tests/CMakeLists.txt`, alongside the unconditional
   core targets (near lines 6–12) and the `add_test` block (near lines 48–52):
   - `add_executable(test_r2_gap unit/test_r2_gap.c)` and append `test_r2_gap`
     to `FF_TEST_TARGETS`;
   - `add_test(NAME r2_gap COMMAND test_r2_gap)`.
   The target assumes the default `FF_BUILD_HAL=ON` (as `test_hal` does); for a
   HAL-disabled configuration also pass
   `target_compile_definitions(test_r2_gap PRIVATE FF_TEST_NO_HAL=1)`.
2. **Update `docs/requirements/traceability.yaml`** tests lists to include
   `tests/unit/test_r2_gap.c` for SRS-REQ-001, -005, -011, -013, -014, -025,
   -041, -050 and SSS-REQ-002, -005, -006, -009, -010 (norm N4).
3. **Build / registration command** (must pass after wiring):

   ```bash
   cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug \
     && cmake --build build/native -j"$(nproc)" \
     && ctest --test-dir build/native --output-on-failure -R r2_gap \
     && python3 tools/scripts/trace.py docs/requirements/traceability.yaml
   ```

   Regenerate the authoritative coverage panel with the new target included:

   ```bash
   make coverage        # rebuilds build/coverage with tests + my test, runs ctest + coverage.py
   make metrics         # refreshes the README status panel
   ```

   Full-suite / pinned-toolchain equivalent:

   ```bash
   make ci-local        # format + build + ctest + Lean + trace in frick-vv:local
   ```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9)       | Senior SW V&V Engineer (fresh context) | 2026-10-11 | APPROVED WITH COMMENTS |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R2-01 | Major | SRS-REQ-014 / `es.c` | ES module had 0% function coverage (not linked by any test) | added `test_es_registry_and_lifecycle` | `test_r2_gap` PASS (487 checks) | FSW (register) + manifest | open until wired | pending |
| R2-02 | Major | SRS-REQ-052 / `pus_services.c` | S3/S8/S20 handler functions unexecuted | added `test_pus_service{3,8,20}_*` | `test_r2_gap` PASS | FSW (register) + manifest | open until wired | pending |
| R2-03 | Major | SRS-REQ-011 / `evs.c` | 4/5 EVS functions unexecuted (log, listener) | added `test_evs_listener_and_ring` | `test_r2_gap` PASS | FSW (register) + manifest | open until wired | pending |
| R2-04 | Minor | SRS-REQ-001 / `sb.c` | `FF_SbUnsubscribe`, wildcard, saturation, drop paths unexecuted | added `test_sb_*` (3 routines) | `test_r2_gap` PASS | FSW (register) + manifest | open | pending |
| R2-05 | Minor | SRS-REQ-013 / `tbl.c` | `FF_TblRevert`, `FF_TblDump`, validator, capacity unexecuted | added `test_tbl_*` (3 routines) | `test_r2_gap` PASS | FSW (register) + manifest | open | pending |
| R2-06 | Minor | SRS-REQ-021 / -024 / `time.c` | `FF_TimeGetMet*`, `FF_TimeGetUtcNs` unexecuted | added `test_time_met_and_utc` | `test_r2_gap` PASS | FSW (register) + manifest | open | pending |
| R2-07 | Minor | SRS-REQ-003 / `ccsds.c` | TC direction + decode error branches untested | added `test_ccsds_telecommand` | `test_r2_gap` PASS | FSW (register) + manifest | open | pending |
| R2-08 | Minor | SRS-REQ-015 / `pal_posix.c` | 10 PAL functions unexecuted | added `test_pal_*` (4 routines); `FF_PalPanic`/`Shutdown` tracked GAP-R2-1/2 | `test_r2_gap` PASS | FSW (register) | open | pending |
| R2-09 | Comment | driver internals | `gem_set_link_up`, `netloop_attach_netif` unexecuted | tracked GAP-R2-3 (driver suite) | — | FSW | open | pending |
| R2-10 | Comment | `pus_adapter_n7.c` | `n7_decode_tc` unexecuted (backend not built by default) | tracked GAP-R2-4 | — | V&V / mission | open | pending |

---

Committed and pushed with the reviewed change (norm N11).