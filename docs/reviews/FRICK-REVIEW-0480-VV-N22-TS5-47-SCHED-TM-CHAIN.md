# Independent V&V Review — N22-TS5-47 Scheduler EVS→TM Chain (test_sched_tm_chain.c)

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| **Review ID**                     | FRICK-REVIEW-0480-VV-N22-TS5-47-SCHED-TM-CHAIN |
| **Date**                          | 2026-11-12                             |
| **Reviewer Role**                 | Independent Senior SW V&V Engineer (norm N9 / R2 profile, norm N10) |
| **Norms Applied**                 | N1, N2, N3, N4, N6, N7, N9, N10, N11, N15, N16, N17, N20, N21, N22 |
| **Baseline HEAD**                 | `93be62b` (`N1: read CUC time before enabling HK report in svc3_create/svc3_enable`) |
| **Artifact under review**         | `tests/unit/test_sched_tm_chain.c` (NEW, untracked at HEAD) |
| **Related changes**               | `tests/CMakeLists.txt` (test target registration + `sched_tm_chain` CTest, labels `n22;s8-sch`); `docs/vv/test-spec/ts-5-core-services.md` rev 1.17 (N22-TS5-47 row `partial` → `covered`); `docs/vv/verification-results.md` (`sched_tm_chain` PASS, 50 checks / 0 failures) |
| **Production code changes**       | None (`src/` unchanged) |
| **Test scripts / tools added**    | None (only a new test `.c` file; no `tools/` changes; no T3 impact; no N10 re-qualification trigger beyond normal test-file review) |

---

## 1. Scope and charter

This review assesses whether `tests/unit/test_sched_tm_chain.c` provides sufficient, correct, deterministic objective evidence for test case **N22-TS5-47**:

> `app_scheduler.c` / `ff_sched.c` heartbeat, late/missed-slot and dispatch-failure evidence — force a late release and an SB publish failure and observe the health-monitoring path.

The test exercises the full production chain:

`FF_AppSchedulerMain → FF_EvsSend → chain_listener (mirror of ff_event_listener in fsw_main.c) → FF_PusEmitEvent → PUS S5 → FF_AppTmOutputInit/ff_to_sink (production sink) → FF_LinkSend → link loopback → decode and assert exact event_id/type/app/payload`

Three test cases: heartbeat (`0x0604`/INFO), missed-slot (`0x0603`/WARNING), dispatch-failure (`0x0605`/ERROR).

This review does **not** close NC-EXT-19 (authority-owned, mission schedule/timing/policy inputs), does **not** establish target/HIL evidence, and does **not** authorize commit/push (norm N7: only `aeronautics-space-fsw-engineer` after all gates and full sprint scope).

---

## 2. Evidence verified against production code (Check A/B)

### 2.1 Chain wiring confirmed by source inspection

| Chain stage | Production source | Test mirror | Verified |
|---|---|---|---|
| EVS listener registration | `fsw_main.c:86` `FF_EvsSetListener(ff_event_listener)` | `test_sched_tm_chain.c:171` `FF_EvsSetListener(chain_listener)` | ✓ |
| EVS → PUS S5 emission | `fsw_main.c:33-43` `ff_event_listener` calls `FF_PusEmitEvent(evt->event_id, evt->type, text, text_len)` | `test:46-56` `chain_listener` identical body | ✓ |
| S5 header encoding | `pus.c:356-388` `FF_PusEmitEvent`: `event id[2 BE]` \| `type[1]` \| aux data; subservice INFO=1/WARNING=2/ERROR=3 (`pus.c:378-386`) | `test:79` range-checks `frame[8] in [1..3]`; `test:174` installs production sink | ✓ |
| PUS TM → TM sink | `pus.c:164-203` `FF_PusSendTm` calls `g_tm_sink(scratch, out_len)` | `test:175` `FF_AppTmOutputInit()` installs `ff_to_sink` | ✓ |
| TM sink → link | `app_tm_output.c:16-21` `ff_to_sink` → `FF_LinkSend`; host default link = loopback (`link.c:14-23`) | `test:58-61` `pop_tm_frame` → `FF_LinkLoopbackPop` | ✓ |
| Frame decode layout | `ccsds.c:194-217` `FF_CcsdsEncodeTm`: primary hdr [0-5], TM sec hdr [6-18], data [19-], CRC trailer | `test:63-91` manual parse matches production encoding exactly | ✓ |

### 2.2 Event ID / type / payload oracle correctness

| Event | ID (sched_evt.h) | Type (app_scheduler.c) | Expected payload | Test assertion | Correct? |
|---|---|---|---|---|---|
| Heartbeat | `0x0604` (:26) | `FF_EVS_INFO` (`app_scheduler.c:243-245`) | `"SCH heartbeat"` (`sched_evt.h:37`) | EVS: `:249-251`; S5: `:255-256` | ✓ exact ID/type/app/text |
| Missed-slot | `0x0603` (:25) | `FF_EVS_WARNING` (`app_scheduler.c:228-230`) | `"SCH: 3 release slots skipped (total 3)"` (`sched_evt.h:36`) | EVS: `:273-277`; S5: `:281-283` | ✓ exact ID/type/app/text |
| Dispatch-fail | `0x0605` (:27) | `FF_EVS_ERROR` (`app_scheduler.c:233-237`) | `"SCH: %u dispatch failures (total %u)"` (`sched_evt.h:38`) | EVS: `:304-310`; S5: `:313-314` (id+type only) | ✓ ID/type/app; S5 text not asserted (see VV-R2-002) |

- `app_id == FF_TEST_APP_ID (1)` asserted at EVS level for all three cases (`:249`, `:274`, `:307`). The scheduler is started with app ID 1 (`sched_thread_entry` → `FF_AppSchedulerMain((void *)(uintptr_t)FF_TEST_APP_ID)`). ✓
- S5 subservice range `frame[8] in [1..3]` matches `FF_PusEmitEvent` subservice mapping (`pus.c:378-386`). ✓
- Event IDs `0x0601`–`0x0605` per `sched_evt.h:23-27`; test covers `0x0603/0x0604/0x0605` as required by N22-TS5-47 (`0x0601` ACTIVATED is not in scope). ✓

### 2.3 Requirement coverage sufficiency for N22-TS5-47

N22-TS5-47 requires forcing **a late release** and **an SB publish failure** and observing the health-monitoring path:

- Late release → missed-slot counter → `FF_SCHED_EVT_MISSED_SLOT` → end-to-end S5 TM: covered by `test_sched_tm_chain_missed_slot` (advance 4500 ms past 1000/3000 ms period entries → `missed` goes 0→3 → one MISSED_SLOT event).
- SB publish failure → `publish_errors` counter → `FF_SCHED_EVT_DISPATCH_FAIL` → end-to-end S5 TM: covered by `test_sched_tm_chain_dispatch_fail` (hold `FF_SbTestLock` so `FF_SbPublish` returns `FF_TIMEOUT` → `publish_errors` 0→4 → one DISPATCH_FAIL event).
- Heartbeat → health/liveness evidence: covered by `test_sched_tm_chain_heartbeat` (advance to 5100 ms ≥ `FF_SCHED_HEARTBEAT_MS` → HEARTBEAT event).

**Verdict:** N22-TS5-47 test coverage is **sufficient**. The requirement moves from `partial` to `covered` (ts-5-core-services.md rev 1.17). This closes the test gap previously recorded in FRICK-REVIEW-0444 (which found the scheduler EVS path untested end-to-end).

---

## 3. Determinism (Check C)

The test is deterministic:

- Single scheduler thread (`pthread`) running `FF_AppSchedulerMain`, gated by `FF_PalTaskDelayMsTest` hooks (`pal_posix.c:233-245`).
- Time is frozen (`FF_PalMonotonicTestSet(0)`, `test:172`) and advanced in discrete steps via `advance_time_ms` + `FF_PalTaskDelayMsTestWait/Step` (`test:144-155`). Each `run_scheduler_tick` consumes exactly one scheduler iteration (seam consumes each block notification, `pal_posix.c:270-282`).
- Scheduler start/stop is synchronized: `start_scheduler_thread` enables hooks + `pthread_create`; `stop_scheduler_thread` sets exit flag, steps once, `pthread_join` (`test:127-142`). `pthread_join` establishes happens-before for capture reads.
- Event capture buffer (`g_chain_captured`, `g_chain_count`) is written by the scheduler thread via the listener and read only after `pthread_join`, so no data race in practice.

Minor caveat: `start_scheduler_thread` uses a fixed 10 ms `nanosleep` (`test:131-132`) before the first `FF_PalTaskDelayMsTestWait()`. This is a liveness margin only: `FF_PalTaskDelayMsTestWait` blocks until the scheduler reaches the delay hook, so there is no deadlock or correctness dependency on the sleep. Not a defect.

**Verdict:** Deterministic. Single-threaded scheduler with a controlled time seam. ✓

---

## 4. Tool qualification (Check D)

- No new test scripts or tools added (only `tests/unit/test_sched_tm_chain.c`, a test `.c` file; no `tools/` changes).
- No T3 impact; no N10 re-qualification trigger beyond normal test-file review (norm N10 / N9: testware reviewed as part of the normal test cycle; compiled with `-Wall -Wextra -Werror` via `tests/CMakeLists.txt:205-207`).
- Test registered in `tests/CMakeLists.txt:29-32` and `tests/CMakeLists.txt:244-245` (CTest `sched_tm_chain`, labels `n22;s8-sch`), linked against `frick_fsw_core` + `Threads::Threads`, compile definitions `FF_TEST_HOOKS`.

**Verdict:** No tool-qualification impact. ✓

---

## 5. Regression check (Check E)

- Production code `src/` unchanged → no production regression possible from this change.
- CTest exit 0 (all gates pass): build 0 warnings, `sched_tm_chain` 50 checks / 0 failures, `trace.py` PASS (120/76/30/14/68), Lean `lake build`, format-check PASS, license OK.
- Existing tests unchanged; `sched_tm_chain` is additive (new CTest target).

**Verdict:** No regression. ✓

---

## 6. Findings

### VV-R2-001 — Minor — Heartbeat oracle reuses shared format constant (common-mode coupling)

- **File:lines:** `tests/unit/test_sched_tm_chain.c:251, 256` (EVS and S5 oracles both compare against `FF_SCHED_EVT_HEARTBEAT_FMT`); `src/apps/sched_evt.h:37` (shared constant also used by `src/apps/app_scheduler.c:245`).
- **Description:** Both legs of the heartbeat oracle (EVS listener text comparison and S5 TM text comparison) use the same shared constant `FF_SCHED_EVT_HEARTBEAT_FMT`. If that constant were semantically wrong in both production and test, the end-to-end oracle would pass while emitting an incorrect payload. This is accepted single-source-of-truth practice, but it is common-mode for the *semantic* content of the heartbeat payload.
- **Recommended disposition:** Hardcode the expected heartbeat text independently in at least one oracle leg (e.g. the S5 `find_s5_event` text argument), or formally accept the shared-constant common-mode as SIL-adequate. Track as an observation; does not weaken the end-to-end *delivery* evidence.
- **Blocks commit/push:** No.

### VV-R2-002 — Minor — PUS TM secondary-header subservice not exactly asserted

- **File:lines:** `tests/unit/test_sched_tm_chain.c:79` (`find_s5_frame` range-checks `frame[8] in [1..3]` only); `tests/unit/test_sched_tm_chain.c:216-225` (`find_s5_event` compares `event_id` + data-field `type` only, never `frame[offset+8]` against the expected subservice); `src/pus/pus.c:378-386` (subservice mapping INFO=1/WARNING=2/ERROR=3).
- **Description:** The test verifies the data-field type byte and ranges the subservice byte to [1..3], but never asserts the exact PUS TM secondary-header subservice (INFO→1, WARNING→2, ERROR→3). A subservice-mapping defect (e.g. INFO encoded as subservice 3) would not be caught by this test. Event-identity header encoding is separately discriminated by `test_pus_event_identity.c` (NC-EXT-237), but the subservice mapping itself is not exactly asserted here.
- **Recommended disposition:** Add an `expected_subservice` parameter to `find_s5_event` and assert `frame[offset + 8] == expected_subservice` (heartbeat→1, missed-slot→2, dispatch-fail→3). Track as Minor hardening.
- **Blocks commit/push:** No.

### VV-R2-003 — Minor — Heartbeat test lacks event-count assertion and exclusivity

- **File:lines:** `tests/unit/test_sched_tm_chain.c:246-256` (heartbeat test); contrast `test:281` (`count_event(MISSED_SLOT) == 1`) and `test:313` (`count_event(DISPATCH_FAIL) == 1`).
- **Description:** The missed-slot and dispatch-fail tests assert `count_event(...) == 1u`; the heartbeat test asserts only existence (`find_event != NULL`) and does not assert `count_event(HEARTBEAT) == 1u` nor exclusivity. Because the heartbeat test advances to 5100 ms, the schedule table (1000/3000 ms entries) concurrently emits ACTIVATED (0x0601) at startup and MISSED_SLOT (0x0603) at the 5100 ms tick, so multiple frames are present in the loopback buffer. `find_s5_event` correctly searches all frames, but the test does not assert that heartbeat occurs exactly once or that no unexpected event class appears.
- **Recommended disposition:** Add `FF_TEST_ASSERT(count_event(FF_SCHED_EVT_HEARTBEAT) == 1u)` for consistency with the other two cases. Exclusivity is not required by N22-TS5-47 (delivery, not silence, is the claim), so this is a consistency hardening only.
- **Blocks commit/push:** No.

### VV-R2-004 — Minor — `chain_listener` discards `FF_PusEmitEvent` return value

- **File:lines:** `tests/unit/test_sched_tm_chain.c:55` (`(void)FF_PusEmitEvent(...)`); mirrors `src/main/fsw_main.c:42` production `ff_event_listener`.
- **Description:** The listener ignores the `FF_PusEmitEvent` return status, identical to production. If PUS were not ready or the TM sink not installed, events would be silently dropped with no test failure. The test installs the production sink (`FF_AppTmOutputInit`, `test:175`) before starting the scheduler, and the frame-presence assertion (`find_s5_event`) would catch a silent drop, so residual exposure is low.
- **Recommended disposition:** Either assert `FF_PusEmitEvent == FF_SUCCESS` in the listener, or document acceptance of mirroring production behavior (delivery already asserted via S5 frame presence). Non-blocking observation.
- **Blocks commit/push:** No.

### VV-R2-005 — Minor — Non-atomic shared capture counter

- **File:lines:** `tests/unit/test_sched_tm_chain.c:39` (`static uint16_t g_chain_count;`), `test:48-49` (listener increments), `test:94-102` / `test:104-113` (test reads after `pthread_join`).
- **Description:** `g_chain_count` / `g_chain_captured` are plain `static` (non-`_Atomic`, non-`volatile`) and are written by the scheduler thread via the listener and read by the test thread. `pthread_join` in `stop_scheduler_thread` establishes happens-before, so there is no data race in practice; however, under norm N9 testware review, an atomic/qualified counter would make the intent explicit and preclude future mis-use.
- **Recommended disposition:** Qualify `g_chain_count` as `_Atomic uint16_t` (or `volatile`) for clarity. No defect observed in current execution.
- **Blocks commit/push:** No.

---

## 7. Context items (not findings of this review)

| Item | Status | Bearing on this review |
|---|---|---|
| **NC-EXT-19** | OPEN, authority-owned (mission schedule/timing/policy inputs) | Not closed by this test; remains for authority disposition. Not a blocker for commit/push per established NC disposition practice. |
| **NC-EXT-236** | EVIDENCE DELIVERED (production sink chain via `test_tm_output_chain.c`) | Related but separate; this test adds the scheduler-EVS→S5 leg through the same production sink. |
| **NC-EXT-237** | FIXED IN SOURCE + discriminating test (`test_pus_event_identity.c`); OPEN pending PA acceptance | Event_id S5-header encoding now emitter-owned (`pus.c:356-388`); relied upon by this test's event-ID oracle. |
| **N22-TS5-47 test gap** | CLOSED by this test (TS-5 rev 1.17: `partial` → `covered`) | This review's primary subject; evidence sufficient. |
| **Target/HIL evidence** | OPEN (GAP-A5; NC-EXT-19/90) | All evidence here is SIL host execution; no target timing/HIL claim is made or required for commit. |

---

## 8. Gate summary

| Gate | Result | Evidence |
|---|---|---|
| Build (`make build`) | PASS, 0 warnings | `tests/CMakeLists.txt:205-207` `-Wall -Wextra -Werror` |
| CTest (`sched_tm_chain`) | PASS, 50 checks / 0 failures | `docs/vv/verification-results.md` |
| Full CSet | PASS (exit 0) | CTest exit 0; no regression (src/ unchanged) |
| Lean `lake build` | PASS | norm N1/N3 unaffected (no generated code) |
| Traceability (`trace.py`) | PASS (120/76/30/14/68) | N22-TS5-47 is a test-spec case ID, not a traceability.yaml requirement ID; SRS-REQ-071/SSS-REQ-030 already traced |
| Format check | PASS | pinned clang-format 18 |
| License (N13) | OK | MPL-2.0 header present |
| Tool qualification | No impact | No new scripts/tools; normal test-file review only |

---

## 9. Verdict

**PASS — no V&V finding blocks commit/push.**

- Test coverage for **N22-TS5-47 is sufficient**: late release (missed-slot), SB publish failure (dispatch-failure), and heartbeat are all observed end-to-end through the production EVS→PUS S5→TM sink→link chain, with exact event ID/type/app/payload oracles.
- **Oracles are correct** against `sched_evt.h`, `app_scheduler.c`, `pus.c`, `ccsds.c`, and `app_tm_output.c`; confirmed by source inspection (frame layout, subservice mapping, event IDs).
- **Deterministic**: single scheduler thread, controlled frozen/injected time seam, pthread join synchronization.
- **Tool qualification**: no new scripts/tools, no T3/N10 re-qualification trigger.
- **No regression**: CTest exit 0, production code unchanged.
- Five Minor findings (VV-R2-001 … VV-R2-005) are oracle-hardening recommendations; none invalidate the evidence and none block commit/push.

**Pre-commit reminders (norm N7 / N11):**
1. Commit/push only by `aeronautics-space-fsw-engineer` after all applicable gates and full sprint scope pass.
2. Retain this review and the passing gate logs (`verification-results.md`, CTest log, trace log) with the completed compliant cycle; do not bypass any failed gate.
3. Resolve VV-R2-001 … VV-R2-005 as Minor follow-up NCs per norm N21 (they do not stop this commit); NC-EXT-19 remains with its authority owner.

---

**Reviewer:** Independent Senior Aeronautics and Space SW Verification & Validation Engineer (R2 profile)  
**Date:** 2026-11-12  
**Review ID:** FRICK-REVIEW-0480-VV-N22-TS5-47-SCHED-TM-CHAIN  
**Authority note:** This review is an independent V&V assessment only. It does not approve qualification, acceptance, certification, anomaly closure, or flight readiness, and does not close NC-EXT-19.
