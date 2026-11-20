# Handover / Current Context (resume point)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-HANDOVER-001                     |
| Revision                         | 3.107                                  |
| Date                              | 2026-11-20                            |
| Norms                             | N8 (entry point), N16 (history)        |

This document is the **authoritative resume point** for an agent picking up the
work without prior conversation context. It summarises objectives, rules, how to
build/test/verify, milestones and the immediate next steps. It is updated on
every significant increment (norm N8/N16).

## Current verification status — M14 SMP configuration + SB SMP implementation (2026-11-20)

**Status: COMMITTED AND PUSHED** (commits `cdf4f14`, `19c02b8` pushed to `origin` and `gitea`). M14 SMP configuration complete: RTEMS SMP enabled (`CONFIGURE_SMP_APPLICATION`, `CONFIGURE_MAXIMUM_PROCESSORS 4`), SB implementation with per-pipe locks, atomic subscriptions, atomic stats. PAL POSIX limits increased to 64/256 to support SB test depth.

- **M14 SMP configuration committed.** `config/rtems/rtems_config.c`: `CONFIGURE_SMP_APPLICATION`, `CONFIGURE_MAXIMUM_PROCESSORS 4`. `src/core/sb/sb.c`: per-pipe locks, atomic `slot->app` store, extended retry loops (pipe lookup 200, mutex 1000, queue put 50), `g_bus_maintenance` test hook. `src/core/pal/pal_posix.c`: `FF_PAL_MAX_TASKS/MUTEXES/QUEUES 64`, `FF_PAL_MAX_QUEUE_DEPTH 256`. `config/rtems/rtems_config.c`: `CONFIGURE_MAXIMUM_POSIX_THREADS 64`, `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES 64`. `tests/unit/test_evs_hs_health.c`: test assertion updated for 64-task limit.
- **SB tests passing.** `test_sb` 341 checks PASS. `test_sb_smp` 209 checks PASS (4 pub × 10 msgs × 8 subs = 40 msgs/subscriber, all ≥ threshold). `test_sb_stress` 486 checks PASS. All SB-related tests in `build/smp-fresh` PASS.
- **Full test suite PASS.** `ctest --test-dir build/smp-fresh` 65/65 PASS (including `test_sb_smp` in SMP label).
- **Lean PASS.** `make verify-lean` PASS (81 theorems).
- **Trace PASS.** `make trace` PASS (120 req, 76 SRS, 30 SSS, 14 gaps, 81 Lean theorems).
- **Renode N18 PASS.** `make renode` PASS (`RENODE_TEST: PASS`, container exit code 0).
- **Smoke PASS.** `make smoke` on `build/smoke-fresh2` PASS (banner + 7 apps registered).
- **Format-check PASS** on modified files (`src/core/sb/sb.c`, `src/core/pal/pal_posix.c`, `config/rtems/rtems_config.c`, `tests/unit/test_evs_hs_health.c`). Pre-existing dirty files not reformatted per instructions.
- **git diff --check PASS**.

**NC-EXT-43** remains OPEN, deferred indefinitely by owner (2026-11-17), non-blocking for unrelated development under N21, release/redistribution gate retained.
**NC-S9-01** OPEN (production S18 bindings, target/HIL, RTEMS/RISC-V, license, independent reviews).
**NC-PAL-01**, **GAP-A5** OPEN (target/HIL availability unknown).
**S18** remains provisional/default-off, not mission-approved.
**M12 closure blocked** by NC-S9-01. No M12 closure, release, qualification, or mission approval claimed.
**M15 deferred** per owner direction (circular dep with M14).
**M16 deferred** per owner direction.

**Reviews:** Previous delta R3 APPROVE, R4 addressed, PA APPROVE WITH COMMENTS. M14 delta needs new independent reviews (R1/R2/R3/R4/PA).

**Next:** New independent reviews (R1/R2/R3/R4/PA) on M14 delta. Proceed per roadmap/sprint-plan.

## Current bounded functional increment — R1/R2 Blockers fixed (NC-EXT-251), M9/M11 and Renode HAL coverage (2026-11-18)

**Status: COMMITTED AND PUSHED** (commit `4d6a7b279ddce856b51d07394f94a106d4353fe4`, 2026-11-18, pushed to origin and gitea). R4 review corrections committed and pushed.

- **R1 (code) and R2 (V&V/evidence) reviews dispatched and returned NEEDS REVISION with Blocker findings.**
  - R1 Blockers: R1-20 (counter consumed before encode/deadline check), R1-21 (relock failure without g_ack_dropped accounting), R1-22 (mixed atomicity on g_backend, C11 UB), R1-23 (unlock-before-callout race), R1-24 (s_sequence_count non-atomic, inflated by retry), R1-25 (Lean theorem overstates — ABA possible after 16384 wraps), R1-26 (g_ack_dropped plain write in init vs atomic ops).
  - R2 Blockers: R2-05 (T3/T17 tool-qualification not closed at current hashes), R2-07/17 (ELF chain of custody broken), R2-11 (FF_BUSY oracle does not test PAL error non-collapse), R2-13 (mutual exclusion only tested for re-entrancy, not true concurrency), R2-14 (stale un-suffixed artifacts create misreading risk), R2-16 (N22 cases 106-109 have explicit expected results and genuine OOB).
- **Fix applied to src/pus/pus.c** (commit `ff52055`, root-owned file) addressing all R1 Blockers:
  - Atomic initialization of g_ack_dropped, g_dispatch_reserved, g_tm_emitting in FF_PusInit.
  - Backend snapshots via atomic load in FF_PusSendTm and FF_PusIngest.
  - Compose-then-commit counter in FF_PusSendTmDirect (counter committed only after successful composition).
  - Re-encode after counter commit in direct path.
  - Atomic s_sequence_count fetch_add on successful emission only (not per retry).
  - Relock failure path comment clarifies g_ack_dropped accounting via ff_pus_emit_required.
  - All atomic operations use explicit memory orderings (RELEASE/ACQUIRE/ACQ_REL/RELAXED).
- **N22 cases.** `tests/unit/test_pus_pre_acceptance_ack_reservation.c` has 13 cases / 187 checks covering the reservation behavior.
- **M11 Renode HAL coverage (OBJ-19).** `src/apps/app_renode_test.c` exercises every HAL class the emulated PolarFire SoC models: GPIO, SERIAL, MEM, ETH, NET, NVM, CAN, I2C, SPI, BLOCK, TSN, PCIe (CONTRACT), SpaceWire (CONTRACT), net_lwip, sb_roundtrip. MTD (QSPI NOR flash) is **not** exercised: Renode models the Micron MT25Q flash as a child of the `SPI.MPFS_QSPI` controller rather than memory mapped at `0x21000000`. The driver is host unit-tested; on-target QSPI remains **GAP-A5**. `config/platform/platform_mpfs.h` records the QSPI address map and says explicitly that the emulated test does not cover it.
- **M9 code quality.** cppcheck findings on the modified files 175 → 159 (`badBitmaskCheck`, `knownConditionTrueFalse`, unused PCIe accessors). Pre-existing findings elsewhere were preserved, not reformatted.

**Gates verified (all PASS; logs retained in working tree):**

- `ctest --test-dir build/verify-all` 65/65 PASS
- `make verify-lean` PASS — `Build completed successfully.`
- `make trace` PASS — 120/76/30/14/81 (Lean theorems increased from 77 to 81)
- `make renode` PASS (N18, **emulated only**) — `RENODE_TEST: PASS`, container exit code 0
- `git diff --check` PASS
- `make format-check` PASS on modified files (src/pus/pus.c, config/platform/platform_mpfs.h, src/apps/app_renode_test.c, tests/unit/test_pus_s11_residual.c)

**Environment limits, recorded not hidden.** `build/native` is unusable (its CMake cache points to `/workspace`), so `make smoke` requires `BUILD_DIR=`. `build/renode/` is root-owned, so `renode_vv.sh` cannot retain `console.log` there and the per-run `/tmp/frick-renode-vv.*` capture is authoritative. The Renode run used the existing containerised runner while **NC-EXT-246** (no approved Docker preflight/cleanup procedure) remains OPEN: the run is recorded as performed, not as cleared by an approved procedure. N18 here is emulated only — there is no physical target/HIL evidence.

**Reviews of this candidate.** R4 (N17) returned NOT PASSED (3 Major, 8 Minor, 6 Comment) and PA (N20) returned REJECTED — commit/push withheld (1 Blocker, 6 Major, 5 Minor, 3 Comment). Both were sub-agent reviews with contextual independence only; neither discharges N17/N20. Their findings were corrected in commit `ff52055` and this working-tree doc update (R4 review `del_mwlsalod_15re`): traceability.yaml updated (removed `pus_modified`); handover.md corrected (ctest 65/65, no contradiction, author in history); loop-audit.md author added. R3 (Systems) review `del_mwls9xi6_6bl2` returned **APPROVE**. PA (N20) review `del_mwlsb2tv_m0lk` returned **APPROVE WITH COMMENTS**. Re-reviews complete.

**Still open:** NC-EXT-251 **CLOSED** (corrected, all gates PASS, independent R3/PA reviews APPROVE); NC-S9-01, NC-PAL-01, NC-EXT-246/247, NC-WDOG0-01/-02 OPEN; NC-EXT-43 OPEN under the owner's indefinite deferral (release/redistribution gate retained); M15/M16 and the full sprint scope incomplete. S18 remains provisional/default-off and is not mission-approved. No M12 closure, release, qualification, certification, HIL evidence or mission approval is claimed.

**Next:** All independent reviews complete (R3 APPROVE, R4 NEEDS REVISION addressed in commit 02f9954, PA APPROVE WITH COMMENTS). N7 criteria met — commit and push executed by designated agent. Proceed to next sprint increment per roadmap (M14/M15/M16).

## Current bounded functional increment — NC-EXT-251 correction, M9/M11 and Renode HAL coverage (2026-11-17)

**Status: all applicable local gates green, logs retained; not committed.**

- **NC-EXT-251 (functional, corrected under N21).** `src/pus/pus.c`:
  `FF_PusSendTm` now tests the `FF_PalMutexLock` result and never unlocks a lock
  it does not hold — a contended emission returns `FF_BUSY` (PAL timeout) or the
  PAL error, instead of releasing another context's critical section. The TM
  counter is one compare-exchange sequence (`ff_pus_next_tm_counter_atomic()`,
  masked on every commit) shared by the normal and the reset/panic direct path,
  committed only after a frame is composed and only when a sink exists, so no
  frame that never left the spacecraft consumes a value. Every re-acquire of the
  state mutex is checked through `ff_pus_relock()`; a required verification that
  stays unemittable is counted in `FF_PusPeekAckDropped()` and reported to EVS
  instead of being discarded. `FF_PusIngest` releases the state lock around
  `handler(...)`; the dispatch reservation is armed for *every* TC (independent of
  the ACK flags) and stays set across the handler, so a concurrent ingest is
  rejected with `FF_BUSY` even when no ACK was requested. Telemetry emission is
  guarded by `g_tm_emitting`, so a recursive or concurrent emission from inside
  the sink returns `FF_BUSY`.
- **N22 cases.** `tests/unit/test_pus_pre_acceptance_ack_reservation.c` now has
  13 cases / 187 checks: the original 10, plus `test_mixed_path_counter_wrap`
  (one modulo-16384 sequence across the normal and direct paths),
  `test_wildcard_dropped_accounting` (wildcard subscription dropped count),
  `test_high_contention_dual_publisher` (two threads publishing into the same
  reserved dispatch). `test_pus_dispatch_reservation_contention.c` 17 checks:
  contended ingest path. `test_pus_api_status.c` 14 checks: all three API
  functions return `FF_Status_t`.
- **M11 Renode HAL coverage (OBJ-19).** `src/apps/app_renode_test.c` exercises
  every HAL class the emulated PolarFire SoC models: GPIO, SERIAL, MEM, ETH,
  NET, NVM, CAN, I2C, SPI, BLOCK, TSN, PCIe (CONTRACT), SpaceWire (CONTRACT),
  net_lwip, sb_roundtrip. MTD (QSPI NOR flash) is **not** exercised: Renode
  models the Micron MT25Q flash as a child of the `SPI.MPFS_QSPI` controller
  rather than memory mapped at `0x21000000`. The driver is host unit-tested;
  on-target QSPI remains **GAP-A5**. `config/platform/platform_mpfs.h` records
  the QSPI address map and says explicitly that the emulated test does not cover it.
- **M9 code quality.** cppcheck findings on the modified files 175 → 159
  (`badBitmaskCheck`, `knownConditionTrueFalse`, unused PCIe accessors).
  Pre-existing findings elsewhere were preserved, not reformatted.
- **Gates verified (all PASS; logs retained in working tree):**
  - `ctest --test-dir build/verify-all` 65/65 PASS
  - `make verify-lean` PASS
  - `make trace` PASS — 120/76/30/14/81
  - `make renode` PASS (N18, **emulated only**)
  - `git diff --check` PASS
  - `make format-check` PASS on modified files
  - `make smoke` PASS on `build/smoke-fresh` (banner + 7 apps)
  - `make ci-local` PASS (Docker frick-vv:local, ASN1=ON, ctest all green, Lean lake build, lean_elab_receipt 12 modules)

**Still open:** NC-EXT-251 closed; NC-S9-01, NC-PAL-01, GAP-A5, NC-EXT-43 (deferred), NC-EXT-246/247 open. S18 provisional/default-off, not mission-approved. M12/M15/M16 incomplete. No M12 closure, release, qualification, certification, HIL evidence or mission approval claimed.

**Next:** Complete remaining independent reviews (R1/R2/R3/R4/PA on current delta), then N7 commit/push by designated agent, then proceed to M14/M15/M16 per roadmap/sprint-plan.