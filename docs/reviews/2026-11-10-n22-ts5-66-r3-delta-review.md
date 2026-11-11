Independent R3 Avionics Systems Review: N22-TS5-66 Delta (TBL Load/Activate → Scheduler Dispatch Coupling)
====================================================================================================

Date: 2026-11-10
Reviewer: Senior Aeronautics and Space Avionics Systems Engineer (R3 — independent sub-agent)

Scope: N22-TS5-66 delta implementing Table Services activation callback coupling to the scheduler dispatcher. Key files: `src/apps/app_scheduler.c`, `tests/unit/test_tbl_sched_coupling.c`, `docs/architecture/scheduling.md` (Rev 0.4/0.5), `docs/vv/test-spec/ts-5-core-services.md` (Rev 1.14, N22-TS5-66 row).

Requirements traced: SRS-REQ-071, SRS-REQ-013, SSS-REQ-030 (parent SSS-REQ-030 → SRS-REQ-071 per traceability.yaml:663-670).
Architecture: `docs/architecture/scheduling.md` (FRICK-ARCH-0018 Rev 0.5, 2026-11-10).
Implementation: `src/apps/app_scheduler.c` (lines 31-50, 95-150, 152-241), `src/core/tbl/tbl.c` (activation callback invocation, lines 89-95).
Tests: `tests/unit/test_tbl_sched_coupling.c` (5 test cases, production-path tests 4-5 under `FF_TEST_HOOKS` controlled-delay seam).

---

## VERDICT

**R3 Review: APPROVED WITH COMMENTS**

The N22-TS5-66 delta correctly implements the TBL load/validate/activate → scheduler dispatch coupling with a bounded one-tick deferral that preserves the single-writer dispatcher contract. Requirements allocation, interface contracts, fault containment, and mode/state behavior are satisfied for the host SIL baseline. The implementation is traceable, testable, and correctly scopes its provisional behaviors.

**Open items preventing full mission authorization (not findings against this delta):**
1. Mission rates, jitter tolerances, late-slot policy, and recovery authority remain TBD (NC-EXT-19, FRICK-ARCH-0018 §1/§5).
2. The one-tick deferral budget (≤ 100 ms at `FF_SCHED_TICK_MS`) is not yet quantified against mission operational needs.
3. Target/HIL evidence absent (GAP-A5); Renode N18 emulated PASS on this tree is host-functional only and does not establish RTPFS160ZT timing or SMP memory ordering (M14).
4. EVS-to-HS end-to-end oracle for the 0x0601 re-activation event is not yet verified in an integration run (`N22-TS5-47` partial).

**No NC closed. No human approval claimed. No commit/push authorized by this review alone.**

---

## FINDINGS SUMMARY

| # | Finding | Severity | Status | Reference |
|---|---------|----------|--------|-----------|
| F1 | Requirements allocation correct: SRS-REQ-071 (validated-before-activation, atomic activate, heartbeat/overrun evidence), SRS-REQ-013 (atomic activate), SSS-REQ-030 (bounded table-driven dispatch, RTEMS separation, health reporting) all satisfied by the coupling design | OK | Verified | traceability.yaml:663-670,145-152; SRS.md:376-385,93-94; SSS.md:167-173 |
| F2 | Interface contracts respected: `FF_TblActivateCb` signature per `tbl.h:24-31`; callback runs in TBL/command context, only snapshots; scheduler task applies in single-writer context via `ff_sched_apply_pending`; mutex protocol uses blocking lock in callback, try-lock in task | OK | Verified | app_scheduler.c:99-116,121-150; tbl.h:24-31; tbl.c:89-95 |
| F3 | Mode/state behavior correct: startup initial activation (lines 183-191), re-activation via callback + next tick (lines 99-150), invalid image handling (TBL load rejects, activate returns `FF_NOT_READY`, active schedule preserved) | OK | Verified | app_scheduler.c:183-191,99-116,121-150; test_tbl_sched_coupling.c:366-401 |
| F4 | Fault containment achieved: callback cannot corrupt dispatcher (pending-slot + mutex); invalid images rejected at TBL load and re-validated in callback; previously active schedule preserved on failed activation; `FF_SchedActivate` atomic double-buffer preserves active table on rejection | OK | Verified | app_scheduler.c:105-109,121-150; tbl.c:64-78,89-95; ff_sched.h:78-82 |
| F5 | Timing deferral bounded to one scheduler tick (100 ms): callback snapshots only; `ff_sched_apply_pending` runs at top of scheduler loop before `FF_SchedPoll`; worst-case latency = one tick + scheduler WCET. Design intent documented in architecture (scheduling.md:158-166) and code comments (lines 31-36,118-120) | OK | Verified | app_scheduler.c:118-120,193-204; scheduling.md:158-166 |
| F6 | TBL load/validate/activate separation respected: `FF_TblLoad` stages + validates; `FF_TblActivate` atomically commits + invokes callback; callback sees only validated active image | OK | Verified | tbl.c:59-97; app_scheduler.c:174-178,99-116 |
| F7 | Health & Safety coupling: re-activation emits `FF_SCHED_EVT_ACTIVATED` (0x0601) with format "SCH: table activated, %u entries" (sched_evt.h:33, app_scheduler.c:148-149). Same event ID as startup activation but distinct format string | OK | Verified | app_scheduler.c:148-149; sched_evt.h:23,31-33; test_tbl_sched_coupling.c:348-353 |
| F8 | One-tick deferral budget not quantified against mission rates/operational needs (NC-EXT-19). The 100 ms `FF_SCHED_TICK_MS` is a project reference value, not a mission-authorized parameter. If a mission requires activation within a tighter bound, this architecture would need a faster tick or immediate-apply path | Observation | Open | scheduling.md:43-44,172-177; NC-EXT-19 |
| F9 | Callback blocks on mutex with `FF_PAL_WAIT_FOREVER` (line 110). Scheduler uses `FF_PalMutexTryLock` (line 129). If scheduler holds the lock and is preempted/delayed, the command-context callback blocks. Lock hold is short (memcpy only), but priority inversion window exists. No priority inheritance on RTEMS POSIX mutex confirmed for this configuration | Minor | Documented | app_scheduler.c:110,129; config/platform/platform_mpfs.h (RTEMS mutex attributes TBD) |
| F10 | Callback silently returns on validation failure (line 107-109) without event emission. Scheduler task also validates and emits `FF_SCHED_EVT_INVALID_TABLE` on `FF_SchedActivate` failure (lines 142-146). Double validation is defense in depth; silent callback return is acceptable but could be noted in architecture | Observation | Documented | app_scheduler.c:105-109,142-146 |
| F11 | `g_sched_reconfig_ready` gate (lines 100,126) prevents callback/apply if mutex creation failed (line 164-165). Fail-safe: no activation applied, no crash. Event `FF_EVS_APP_ERROR` emitted at startup if lock unavailable (lines 167-169). Correct behavior | OK | Verified | app_scheduler.c:164-169,100,126 |
| F12 | Test coverage: 5 cases (3 hook-contract, 2 production-path) cover nominal activation, revert, invalid load, production activate with one-tick deferral verification, and production invalid image preservation. EVS event capture verifies 0x0601 emission and payload. Explicit expected results per N22 | OK | Verified | test_tbl_sched_coupling.c:136-412; ts-5-core-services.md:196 (N22-TS5-66 row) |
| F13 | Traceability: `docs/requirements/traceability.yaml` lists `test_tbl_sched_coupling.c` for both SRS-REQ-013 (line 150) and SRS-REQ-071 (line 668). Bidirectional traceability converged per trace.py | OK | Verified | traceability.yaml:150,668; docs/requirements/traceability.md:86,188 |

---

## DETAILED FINDINGS

### F1 — Requirements Allocation Correctness (OK)
- **SRS-REQ-071** (SRS.md:376-385): "Table images shall be validated before activation and activated atomically. The service shall expose heartbeat, late/missed-slot, and dispatch-failure evidence to Health & Safety / EVS." → Satisfied: TBL load validates (`FF_SchedValidate`), activate atomically commits and invokes callback; scheduler task re-validates (defense in depth), activates via `FF_SchedActivate` (atomic double-buffer), and emits EVS events (0x0601 activation, 0x0602 invalid, 0x0603 missed, 0x0604 heartbeat, 0x0605 dispatch fail).
- **SRS-REQ-013** (SRS.md:93-94): "Table activation shall be atomic (a staged image becomes active only on explicit activate)." → Satisfied: TBL staging isolates load from activate; callback only fires on successful activate; dispatcher applies on next tick.
- **SSS-REQ-030** (SSS.md:167-173): "Bounded, table-driven service that dispatches configured Software Bus messages at their approved monotonic-time rates, while leaving runnable task selection and priority preemption to the native RTEMS scheduler. It shall report scheduler liveness and dispatch overruns to the health-monitoring path." → Satisfied: coupling preserves RTEMS ownership; heartbeat/missed/dispatch-failure events reported to EVS for HS consumption.

### F2 — Interface Contracts (OK)
- **Callback signature**: `FF_TblActivateCb` per `tbl.h:24-31` — `void (*)(FF_TblHandle_t, const void*, size_t)`. Implemented as `ff_sched_tbl_activated` (app_scheduler.c:99-116).
- **Context boundary**: Callback runs in TBL/command context (tbl.c:93-95). It performs only: handle/image/size check, `FF_SchedValidate`, mutex lock, `memcpy` to pending slot, flag set, mutex unlock. No I/O, no blocking beyond the short mutex, no re-entry into TBL.
- **Scheduler task boundary**: `ff_sched_apply_pending` runs in `FF_AppSchedulerMain` loop (line 204), before `FF_SchedPoll`. Uses `FF_PalMutexTryLock` — never blocks the scheduler tick. Copies pending image, calls `FF_SchedActivate` + `FF_SchedInit`, emits event.
- **Mutex protocol**: Single mutex `g_sched_reconfig_lock` protects `g_sched_pending_image` and `g_sched_reconfig_pending`. Callback: blocking lock (must succeed to snapshot). Task: try-lock (skip if busy, retry next tick). This preserves single-writer dispatcher contract (ff_sched.c double-buffer is only touched by task).

### F3 — Mode/State Behavior (OK)
- **Startup**: `FF_AppSchedulerMain` initializes reference schedule (line 159), creates reconfig mutex (lines 163-165), registers table with callback (lines 174-178), performs initial activation directly (lines 183-191) — `FF_SchedActivate` + `FF_SchedInit` + startup event `FF_SCHED_EVT_ACTIVATED_FMT` ("SCH app started, %u entries").
- **Re-activation**: Ground loads new image via `FF_TblLoad`, then `FF_TblActivate`. TBL commits atomically, invokes callback. Callback snapshots validated image. Scheduler task applies on next tick (≤ 100 ms), calls `FF_SchedActivate` + `FF_SchedInit`, emits re-activation event `FF_SCHED_EVT_REACTIVATED_FMT` ("SCH: table activated, %u entries") with same event ID 0x0601.
- **Invalid image**: Wrong length → `FF_TblLoad` returns `FF_BAD_SIZE` (tbl.c:64-65). Bad magic → validator rejects, load returns `FF_ERROR`, staged flag cleared (tbl.c:72-75). Subsequent `FF_TblActivate` returns `FF_NOT_READY` (tbl.c:86-88). Callback not invoked. Active schedule unchanged. Verified by test_tbl_sched_production_invalid (lines 367-401).

### F4 — Fault Containment (OK)
- **Callback cannot corrupt dispatcher**: Pending slot is separate from active dispatcher state. Dispatcher state (`core/sched/ff_sched.c` double-buffer) is only modified by `FF_SchedActivate` in task context.
- **Invalid images never reach dispatcher**: Two validation gates — TBL load validator (`FF_SchedValidate`) and callback re-validation (app_scheduler.c:107). If either fails, activation path stops.
- **Previously active schedule preserved**: `FF_SchedActivate` on invalid image returns `FF_ERROR` and leaves active table untouched (ff_sched.h:78-80). Callback returns early on validation failure. Task `ff_sched_apply_pending` checks `FF_SchedActivate` result and emits error event without changing active state on failure.
- **Common-cause consideration**: Both validation gates use the same `FF_SchedValidate` function. A systematic validator defect would affect both. This is acceptable for the current criticality (class B) but should be noted if diverse validation is later required.

### F5 — Timing/Deferral (OK with Observation F8)
- **Bounded one-tick deferral**: Scheduler tick = `FF_SCHED_TICK_MS` (100 ms, sched_evt.h:19). Activation applied at top of next tick. Worst-case latency = one tick + `ff_sched_apply_pending` WCET + `FF_SchedActivate`/`FF_SchedInit` WCET.
- **Design documented**: Architecture §5.1 (scheduling.md:158-166) and code comments (app_scheduler.c:31-36,118-120) explicitly state the deferral bound and rationale (single-writer preservation).
- **Operational implication**: A ground-commanded schedule change takes effect no sooner than the next scheduler tick. Mission operations must account for this latency in procedures (e.g., timeline uploads, contingency reconfigurations). The deferral budget is not yet traced to a mission requirement (NC-EXT-19).

### F6 — TBL Integration (OK)
- **Load/Validate/Activate separation**: `FF_TblLoad` copies to staging buffer, runs validator, sets staged flag. `FF_TblActivate` checks staged flag, atomically copies staging → active image under mutex, then invokes callback with active image pointer (tbl.c:89-95). Revert restores staging from active and clears staged flag (tbl.c:99-108).
- **Callback registration**: At table registration (app_scheduler.c:174-178), `on_activate = ff_sched_tbl_activated`. Reconfiguration mutex created *before* registration (lines 161-165) so callback has a valid lock.

### F7 — Health & Safety Coupling (OK)
- **Event emission**: On successful application in task context, `FF_EvsSend(FF_SCHED_EVT_ACTIVATED, FF_EVS_INFO, app, FF_SCHED_EVT_REACTIVATED_FMT, count)` (app_scheduler.c:148-149).
- **Event ID**: 0x0601 (`FF_SCHED_EVT_ACTIVATED`, sched_evt.h:23) — same as startup activation but distinct format string ("SCH: table activated, %u entries" vs "SCH app started, %u entries"). Ground can distinguish by format payload.
- **HS consumption**: HS application monitors EVS events (app_hs.c). The 0x0601 event provides visibility into schedule reconfiguration for health monitoring. End-to-end EVS→HS path is tested in `test_es_hs_lifecycle.c` but not specifically for this event (N22-TS5-47 partial).

### F8 — One-Tick Deferral Budget Not Quantified (Observation)
The `FF_SCHED_TICK_MS = 100 ms` is a project reference constant (sched_evt.h:19). FRICK-ARCH-0018 §1/§5 (scheduling.md:43-44,172-177) explicitly states mission rates, jitter limits, late-slot policy, and recovery authority are TBD controlled inputs. NC-EXT-19 remains open for authority-dependent closure. If a mission requires activation latency < 100 ms, this architecture would need either a faster tick or an immediate-apply path (which would break the single-writer contract or require additional synchronization). **Decision required from mission authority on acceptable activation latency.**

### F9 — Callback Blocking Mutex (Minor)
- Callback uses `FF_PalMutexLock(g_sched_reconfig_lock, FF_PAL_WAIT_FOREVER)` (line 110).
- Scheduler task uses `FF_PalMutexTryLock` (line 129).
- **Risk**: If scheduler task holds the mutex (during the brief copy in `ff_sched_apply_pending`) and is preempted by a higher-priority task, or if the scheduler task is delayed (e.g., interrupt storm), the command-context callback blocks indefinitely. The lock hold is ~`memcpy(FF_SchedTable_t)` (max 16 entries × ~40 bytes = ~640 bytes + overhead), so the window is small.
- **Mitigation**: Scheduler uses try-lock, so it never blocks waiting for the callback. Priority inheritance on the RTEMS POSIX mutex is not confirmed for this BSP/configuration. This should be verified in the target RTEMS configuration or documented as an accepted risk for class B.

### F10 — Silent Callback Return on Validation Failure (Observation)
- Callback returns silently if `FF_SchedValidate` fails (lines 107-109). No event emitted.
- Task `ff_sched_apply_pending` also validates and emits `FF_SCHED_EVT_INVALID_TABLE` (0x0602) on `FF_SchedActivate` failure (lines 142-146).
- **Rationale**: The callback re-validation is defense in depth (TBL already validated at load). A validation failure in the callback implies a corruption between TBL commit and callback invocation (e.g., memory corruption). In that case, silent return preserves the active schedule; the task will also detect and report on its next tick. Acceptable but architecture could note this behavior explicitly.

### F11 — Reconfig Ready Gate (OK)
- `g_sched_reconfig_ready` set only after successful mutex creation (line 164-165).
- Both callback (line 100) and apply function (line 126) check this flag and return early if false.
- Startup emits `FF_EVS_APP_ERROR` if mutex creation fails (lines 167-169).
- **Behavior**: If mutex creation fails, no activation can ever be applied, but the initial schedule (activated at startup before registration) continues to run. Fail-safe.

### F12 — Test Coverage (OK)
- **Test 1** (`test_tbl_sched_coupling_basic`): Hook contract — TBL load/activate triggers callback → `FF_SchedActivate` + `FF_SchedInit`, verifies active count and dispatch.
- **Test 2** (`test_tbl_sched_coupling_revert`): Revert does not affect active scheduler until next activate.
- **Test 3** (`test_tbl_sched_coupling_invalid`): Invalid table (bad magic) rejected at load, activate returns `FF_NOT_READY`.
- **Test 4** (`test_tbl_sched_production_activate`): Production path — scheduler thread running, TBL activate defers to next tick (verifies `FF_SchedActiveCount` unchanged before tick, updated after), verifies 0x0601 event emission with correct payload, verifies new table dispatch behavior.
- **Test 5** (`test_tbl_sched_production_invalid`): Production path — wrong length (`FF_BAD_SIZE`), bad magic (`FF_ERROR`), activate returns `FF_NOT_READY`, active schedule preserved, no re-activation event.
- All tests use explicit expected results (oracles) per N22. EVS listener captures events. Controlled-delay seam (`FF_TEST_HOOKS`) provides deterministic tick advancement.

### F13 — Traceability (OK)
- `traceability.yaml:150` lists `test_tbl_sched_coupling.c` for SRS-REQ-013.
- `traceability.yaml:668` lists `test_tbl_sched_coupling.c` for SRS-REQ-071.
- `traceability.md:86` and `:188` map SRS-REQ-071 and SSS-REQ-030 to implementation and tests.
- `make trace` passes (bidirectional traceability converged per norm N4).

---

## OPEN SYSTEM-LEVEL ITEMS (Not Findings Against This Delta)

| Item | Description | Owner | Tracking |
|------|-------------|-------|----------|
| O1 | Mission schedule rates, jitter tolerances, late-slot policy, recovery authority | Systems / Mission Authority | NC-EXT-19, FRICK-ARCH-0018 §1/§5 |
| O2 | One-tick activation deferral budget quantified against mission operational needs | Systems / Mission Authority | NC-EXT-19 |
| O3 | Target/HIL evidence: Renode emulated PASS is host-functional only; RTPFS160ZT timing, SMP memory ordering (M14), watchdog binding (NC-PAL-01) not established | FSW / V&V / PA | GAP-A5, NC-EXT-31, NC-PAL-01 |
| O4 | EVS-to-HS end-to-end oracle for 0x0601 re-activation event | V&V / HS | N22-TS5-47 (partial) |
| O5 | RTEMS mutex priority inheritance configuration for `g_sched_reconfig_lock` | FSW / Platform | Platform configuration review |

---

## RECOMMENDATIONS

| # | Recommendation | Decision Owner | Prerequisites |
|---|---------------|----------------|---------------|
| R1 | Accept this R3 review as satisfying the independent Avionics Systems Engineering review requirement for the N22-TS5-66 delta | FSW Engineer / User | This review documented |
| R2 | Do NOT close NC-EXT-19; it remains open for mission authority inputs (schedule, timing, policy) | Systems + FSW Authority | Mission configuration baseline, target timing tests, ADR-0020 disposition |
| R3 | Update `docs/architecture/scheduling.md` §5.1 to reflect that the TBL coupling increment is delivered and host-verified; deferral budget remains TBD | FSW Engineer | This review recorded |
| R4 | Verify RTEMS POSIX mutex priority inheritance for `g_sched_reconfig_lock` in the target configuration; document as accepted risk or add mitigation if required | FSW Engineer / Platform | RTEMS 6.2 BSP configuration review |
| R5 | Add explicit note in architecture (§5.1) about callback silent-return behavior on validation failure (F10) and the task-side event emission as the observable failure signal | FSW Engineer | This review recorded |
| R6 | No commit/push until applicable gates (build, test, Lean, trace) pass in the designated environment and full sprint scope is complete per N7/N15 | FSW Engineer | CI green in frick-vv:local |

---

## R3 REVIEW CONCLUSION

**APPROVED WITH COMMENTS** — The N22-TS5-66 delta is correctly implemented, traced, and host-verified for the SIL baseline. The bounded one-tick deferral design preserves the single-writer dispatcher contract and achieves fault containment. Requirements SRS-REQ-071, SRS-REQ-013, and SSS-REQ-030 are satisfied at the host level.

**Outstanding items are authority-dependent and do not reflect defects in this delta.** Mission rates, deferral budget, target evidence, and EVS-HS integration remain open per project norms (N7, N15, N18, N21).

**No NC closed. No human approval claimed. No commit/push authorized by this review alone.**

---

Reviewer: Senior Aeronautics and Space Avionics Systems Engineer (R3 — independent sub-agent)  
Date: 2026-11-10  
Authority: This review provides evidence and findings only. It does not approve baselines, close NCs, waive gates, or authorize commit/push.