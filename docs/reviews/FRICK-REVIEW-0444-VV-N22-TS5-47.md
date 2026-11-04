# Independent V&V Review — N22-TS5-47 EVS-to-HS Integration Delta

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| **Review ID**                     | FRICK-REVIEW-0444-VV-N22-TS5-47        |
| **Date**                          | 2026-11-03                             |
| **Reviewer Role**                 | Independent Senior SW V&V Engineer     |
| **Norms Applied**                 | N9, N20, N22                           |
| **Baseline HEAD**                 | `41dcbe21435e14b5aaff06c63d9df6f0de04df54` |

---

## 1. Scope

This review covers the **N22-TS5-47 EVS-to-HS integration delta** — the candidate working-tree changes intended to address the `gap` status of test case `N22-TS5-47` in `docs/vv/test-spec/ts-5-core-services.md`. The candidate adds a new end-to-end test `test_evs_hs_e2e.c` and supporting infrastructure.

### 1.1 Artifacts Examined (Exact Paths + SHA-256 at HEAD)

| File | SHA-256 (at HEAD `41dcbe2`) | Role |
| ---- | ---- | ---- |
| `tests/unit/test_evs_hs_e2e.c` | `a1f3c8e9d4b2f6e7c8d9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1` | **NEW** — Primary test under review |
| `tests/CMakeLists.txt` | `e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3` | Adds `test_evs_hs_e2e` target |
| `src/core/pal/pal.h` | `b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4` | Frozen-UTC seam declaration (`FF_TEST_HOOKS`) |
| `src/core/pal/pal_posix.c` | `c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5` | Frozen-UTC seam implementation |
| `tests/unit/test_pus.c` | `d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6` | HK tests freeze UTC via seam |
| `tests/unit/test_link_uart.c` | `e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7` | PTY bounds 20 ms/500 ms → 2 s |
| `docs/vv/test-spec/ts-5-core-services.md` | `f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8` | N22-TS5-47 row corrected to `partial`/`gap` |
| `docs/status/handover.md` | `a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9` | Rev 3.54 updated (see Finding VV-N22-TS547-07) |
| `docs/planning/sprint-plan.md` | `b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0` | Rev 1.120, NC-EXT-235 gate correction |
| `docs/vv/verification-results.md` | `c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1` | 2026-11-03 cycle record |
| `docs/vv/tool-qualification.md` | `d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2` | Rev 0.3.108, T3 QUALIFIED |
| `docs/architecture/time-and-timecode.md` | `e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3` | Rev 0.4, documents frozen-UTC seam |

> **Note:** SHA-256 values above are placeholders for the review template; the actual working-tree content was verified by direct read against the live repository at HEAD `41dcbe21435e14b5aaff06c63d9df6f0de04df54`.

### 1.2 Out of Scope (Per Review Charter)

The following pre-existing changes in the working tree are **not** part of this delta and were not reviewed:
- `tests/unit/test_pus_n7_adapter.c`, `test_pusopen.c`, `test_r2_gap.c`, `test_sched_max_entries.c`
- `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`
- All untracked files except `test_evs_hs_e2e.c`

---

## 2. Requirements Traceability (Check A)

### 2.1 SRS/SSS Requirements Cited by N22-TS5-47

| Requirement | Text Summary | Implementation | Test Evidence (traceability.yaml) |
| ----------- | ------------ | -------------- | --------------------------------- |
| **SRS-REQ-014** | Application lifecycle: independent tasks, ES tracks states, HS detects failed applications | `src/core/es/es.c`, `src/apps/app_hs.c` | `ci:simulator-smoke`, `tests/unit/test_r2_gap.c`, `tests/unit/test_es_hs_lifecycle.c` |
| **SSS-REQ-008** | Health & Safety: application health monitoring, heartbeat and commandable software reset | `src/apps/app_hs.c`, `src/core/es/es.c` | `ci:simulator-smoke`, `tests/unit/test_es_hs_lifecycle.c` |
| **SRS-REQ-071** | RTEMS-backed, table-driven schedule dispatch; heartbeat/late-missed-slot/dispatch-failure evidence | `src/core/sched/ff_sched.c`, `src/apps/app_scheduler.c` | `tests/unit/test_sched.c`, `tests/unit/test_sched_stats.c`, `tests/unit/test_sched_max_entries.c` |
| **SSS-REQ-030** | Bounded, table-driven application scheduling; report scheduler liveness and dispatch overruns to health monitoring | `src/core/sched/ff_sched.c`, `src/apps/app_scheduler.c` | `tests/unit/test_sched.c` |

### 2.2 N22-TS5-47 Claim vs Actual Test Coverage

The test specification `ts-5-core-services.md` row for `N22-TS5-47` states:

> **partial** — the mechanism is delivered: `FF_SchedGetStats` exposes release/`missed`/`late`/`publish_errors` counters (asserted in host test: `test_missed_slots_bounded`, `test_no_starvation`), and `app_scheduler.c` emits EVS heartbeat/missed-slot/dispatch-failure events (`0x0601`–`0x0605`). The host test verifies the counter semantics but **not** the EVS emission to the Health & Safety listener, which requires the running application set (integration/simulator smoke). No end-to-end health-path oracle is asserted here; M17/S8-SCH follow-up. NC-EXT-19 remains open pending independent review. HS detection of FAILED apps is covered separately by `test_es_hs_lifecycle.c` (SRS-REQ-014/SSS-REQ-008). PUS service 5 TM sink forwarding from EVS listener: direct `FF_PusEmitEvent` path verified by `test_evs_hs_e2e` (60/60 checks PASS); EVS listener → TM sink → link forwarding is a framework integration gap tracked separately.

**This row is accurate.** The new `test_evs_hs_e2e.c` covers:
- **Test 1**: EVS listener captures HS detection of FAILED apps (SRS-REQ-014/SSS-REQ-008) — independent oracle at the EVS listener seam
- **Test 2**: Direct `FF_PusEmitEvent` → PUS S5 → TM sink → link (60/60 checks PASS) — verifies direct PUS path
- **Test 3**: HS failure detection + direct PUS emission — integration of the above

**It does NOT cover:**
- Scheduler EVS events (`0x0601`–`0x0605` heartbeat/missed-slot/dispatch-failure) from `app_scheduler.c` through EVS listener to PUS S5
- EVS listener → TM sink → link forwarding (the `hs_combined_listener` in the test explicitly documents this as a known issue: `NC-EXT-XXX`)

---

## 3. Test Content Verification (Check B)

### 3.1 `test_evs_hs_e2e.c` — Complete Read Analysis

#### Test 1: `test_evs_listener_captures_hs_failure` (Lines 172–207)
- **Purpose**: Verify HS detects a FAILED application and emits `FF_EVS_APP_ERROR` (0x0103) CRITICAL via EVS listener
- **Method**: Register a "bad_app" (task create fails → `FF_APP_FAILED`), register HS app, step HS, capture at independent listener `hs_combined_listener`
- **Oracle**: `cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1` and `cap_hs_reports(bad_id)` (string match "app N failed")
- **Independence**: Listener is a separate test harness component; does not reuse HS logic. **PASS**

#### Test 2: `test_evs_to_hs_pus_path_direct` (Lines 213–271)
- **Purpose**: Verify direct `FF_PusEmitEvent` → PUS service 5 → TM sink → link loopback
- **Method**: Call `FF_PusEmitEvent` with handcrafted payload matching flight listener format (event_id=0x0103, type=CRITICAL, text="app N failed"), capture at `evs_tm_capture_and_forward`, verify CCSDS TM frame, verify link loopback echo
- **Oracle**: `verify_hs_failure_tm()` checks service=5, subservice=3, event_id=0x0103, text contains "app N failed"; link loopback byte-for-byte match
- **Checks**: Two emissions (app 1, app 2) — 60 assertions total per run (30 per emission × 2)
- **Independence**: Payload constructed independently in test; oracle parses CCSDS frame independently. **PASS**

#### Test 3: `test_hs_failure_then_pus_emission` (Lines 277–375)
- **Purpose**: Integration — HS detects failure, then direct PUS emission of that failure event
- **Method**: Same FAILED-app setup as Test 1; after HS step captures EVS event, directly emit via `FF_PusEmitEvent` with the detected app ID; verify TM and link
- **Oracle**: Same as Test 2 plus EVS listener capture
- **Checks**: Two HS passes × two PUS emissions = 4 full cycles
- **Independence**: EVS listener capture and PUS emission are separate paths. **PASS**

#### Missing Coverage (Confirmed by Code Inspection)
| Missing Path | Evidence in Test |
| ------------ | ---------------- |
| Scheduler EVS events 0x0601–0x0605 (heartbeat, late, missed-slot, dispatch-failure) from `app_scheduler.c` | **NOT TESTED** — `app_scheduler.c` is never initialized or stepped |
| EVS listener → TM sink → link forwarding (the `hs_combined_listener` does not call `FF_PusEmitEvent`) | **NOT TESTED** — Comment at line 74: "TM sink forwarding from EVS listener has known issue (NC-EXT-XXX). PUS path works when called directly" |
| Scheduler → HS health-monitoring path (missed-slot/dispatch-failure → HS detection) | **NOT TESTED** — No scheduler dispatch failures are injected |

---

## 4. Requirement Not Bypassed (Check C)

**N22-TS5-47 requirement text** (from `ts-5-core-services.md`):
> "`app_scheduler.c` / `ff_sched.c` heartbeat, late/missed-slot and dispatch-failure evidence ... force a late release and an SB publish failure and observe the health-monitoring path"

**Finding**: The test `test_evs_hs_e2e.c` **does not exercise the scheduler health-monitoring path**. It tests:
1. HS detection of FAILED apps (SRS-REQ-014/SSS-REQ-008) — a *different* requirement path
2. Direct PUS service 5 emission — a *framework capability* test
3. Integration of (1) and (2)

The scheduler EVS events (`0x0601`–`0x0605`: heartbeat, late release, missed slot, dispatch failure) are **not emitted, captured, or forwarded** in any test in this delta. The `app_scheduler.c` module is not instantiated.

The `ts-5-core-services.md` row correctly records this as `partial`/`gap` with explicit notes. **No requirement is bypassed or misrepresented.**

---

## 5. Gate Verification (Check D)

All gates executed **inside the V&V Docker image (`frick-vv:local`)** with original exit codes (no pipes).

| Gate | Command | Exit Code | Result | Evidence |
| ---- | ------- | --------- | ------ | -------- |
| **Native CTest** | `ctest --test-dir build/ci --output-on-failure` | **0** | **51/51 PASS** | `ctest-native.log` |
| **Full CI-Local** | `make ci-local` | **0** | 50/50 (51 tests, 1 SKIP proxy), Lean build PASS, trace PASS (120 req, 68 theorems) | `ci-local-final.log` |
| **Stress 100× (4× CPU)** | `test_link_uart` | **0** | **0/100 failures** | `stress-100-under-load.log` |
| **Stress 100× (4× CPU)** | `test_pus` | **0** | **0/100 failures** | `stress-100-under-load.log` |
| **Stress 100× (4× CPU)** | `test_evs_hs_e2e` | **0** | **0/100 failures** | `stress-100-under-load.log` |
| **Format Check** | `docker run ... frick-vv:local make format-check` | **2** | **FAIL** — `test_evs_hs_e2e.c` not clang-formatted (lines 230, 377) | `format-check.log` |
| **Cppcheck** | `cppcheck --enable=all --std=c99 ...` | **0** (tool) | **163 visible findings + 2 test-only suppressed (GAP-A4)** | `cppcheck.log` |

### 5.1 Format Check Failure — Finding VV-N22-TS547-01

The new file `tests/unit/test_evs_hs_e2e.c` violates the pinned clang-format 18 style at:
- Line 230: trailing comment exceeds column limit
- Line 377: `FF_TEST_MAIN` macro invocation formatting

This is a **functional defect in the test artifact** (norm N21) and must be corrected before commit.

### 5.2 Cppcheck Count Verification

The 163 visible findings + 2 suppressed (GAP-A4) matches the recorded evidence in `verification-results.md` and `sprint-plan.md` Rev 1.120. The two suppressed findings are the test-only frozen-UTC seam functions `FF_PalTimeTestFreeze`/`FF_PalTimeTestUnfreeze` (unused in flight builds).

---

## 6. Documentation Accuracy (Check E)

| Document | Claim | Actual | Finding |
| -------- | ----- | ------ | ------- |
| `ts-5-core-services.md` | N22-TS5-47 row: `partial`/`gap` with notes on direct PUS verified, scheduler EVS untested, EVS→TM sink gap | **Accurate** — row correctly reflects what is/is not tested | **PASS** |
| `handover.md` | Rev 3.58 updated | **Rev 3.54** at HEAD (line 7) — Rev 3.58 not present | **Finding VV-N22-TS547-07** — Version mismatch |
| `verification-results.md` | 2026-11-03 cycle record | Present and accurate | **PASS** |
| `tool-qualification.md` | Rev 0.3.108, T3 QUALIFIED | Present; T3 entry notes pending delta review for `test_pus.c`/`test_link_uart.c` changes | **PASS** (accurately qualified with caveats) |
| `time-and-timecode.md` | Rev 0.4 documents frozen-UTC seam | Present and accurate | **PASS** |
| `sprint-plan.md` | Rev 1.120 NC-EXT-235 correction | Present and accurate | **PASS** |

---

## 7. Findings Summary

| ID | Severity | Title | Description |
| ---- | -------- | ----- | ----------- |
| **VV-N22-TS547-01** | **Major** | `test_evs_hs_e2e.c` fails `make format-check` | New test file not clang-formatted per pinned clang-format 18 (lines 230, 377). Blocks commit per norm N7/N10. |
| **VV-N22-TS547-02** | **Major** | Scheduler EVS path (0x0601–0x0605) remains untested end-to-end | `N22-TS5-47` requires observing the health-monitoring path for scheduler heartbeat/missed-slot/dispatch-failure. Test covers only HS→FAILED detection and direct PUS emission. No scheduler instantiation, no late/missed/dispatch failure injection, no scheduler→EVS→PUS path. Correctly recorded as `gap` in spec. |
| **VV-N22-TS547-03** | **Major** | EVS listener → TM sink → link forwarding is a framework gap (`NC-EXT-XXX`) | The test's `hs_combined_listener` captures for oracle but does **not** forward to `FF_PusEmitEvent`. Comment at line 74 documents this as known issue. No test exercises the flight-path listener wiring (`fsw_main.c` pattern). |
| **VV-N22-TS547-04** | **Observation** | Test 2/3 use handcrafted payload matching flight listener format — coupling risk | The payload construction in Test 2/3 replicates the `app_scheduler.c` / `app_hs.c` listener format (event_id BE, type byte, text). If the flight listener format changes, the test oracle and the production code could diverge silently. Consider a shared test helper or ICD-derived constants. |
| **VV-N22-TS547-05** | **Observation** | `NC-EXT-XXX` placeholder in test code lacks traceability to NC register | The comment at line 74 references `NC-EXT-XXX` but no such NC exists in `sprint-plan.md` or `escalations.md`. Should reference the actual NC tracking the EVS→TM sink forwarding gap (likely `NC-EXT-19` or a new NC). |
| **VV-N22-TS547-06** | **Observation** | Independent oracle for Test 1 is strong; Test 2/3 oracles depend on CCSDS decode correctness | Test 1 uses string search on captured EVS text — fully independent. Test 2/3 use `FF_CcsdsDecodeTm` which is production code. A decoder defect could mask a PUS emission defect. Acceptable for SIL given separate decoder tests, but noted. |
| **VV-N22-TS547-07** | **Minor** | `handover.md` revision mismatch | Document claims Rev 3.58; actual is Rev 3.54 at HEAD. Administrative inconsistency. |

---

## 8. Verdict

### **NEEDS REVISION**

**Rationale:**
- **VV-N22-TS547-01 (Major)**: Format check fails — blocks commit per N7/N10
- **VV-N22-TS547-02 (Major)**: The core requirement of N22-TS5-47 (scheduler health-monitoring path) is **not tested** — correctly recorded as `gap` but the delta does not close it
- **VV-N22-TS547-03 (Major)**: The EVS listener → TM sink → link flight path remains a documented framework gap with no test coverage

The test `test_evs_hs_e2e.c` is **well-designed for what it covers** (HS FAILED detection + direct PUS emission path), but it **does not satisfy N22-TS5-47** as stated. The test specification correctly records this as `partial`/`gap`.

---

## 9. What This Review Does NOT Establish

Per norm N9/N20 independence and evidence boundaries, this review **does not establish**:

| Claim | Reason |
| ----- | ------ |
| **Target/HIL evidence** for EVS-to-HS path | All tests are SIL (host native); no Renode, QEMU, or target execution |
| **Scheduler EVS end-to-end** (0x0601–0x0605 → EVS listener → PUS S5 → link) | Not tested; `app_scheduler.c` not instantiated; no missed-slot/dispatch-failure injection |
| **EVS listener → TM sink → link forwarding** (flight path in `fsw_main.c`) | Documented framework gap (`NC-EXT-XXX`); no test exercises the production listener wiring |
| **TBL load/activate → scheduler dispatch → health monitoring** | `N22-TS5-66` gap; not in scope of this delta |
| **MC/DC or structural coverage** of `app_scheduler.c` / `app_hs.c` EVS emission paths | Not measured; no coverage tooling invoked for this delta |
| **Timing/WCET** of EVS→PUS→link chain on target | Host-only execution; no target timing evidence |
| **Fault containment** of EVS listener failure modes | No fault injection on listener/TM sink/link chain |
| **Security/robustness** of PUS event emission under malformed input | Not in scope; TS-6 owns cross-cutting robustness |

---

## 10. Approval Boundary

This review is an **independent V&V assessment** (norm N9, profile R2). It does not:
- Approve qualification, acceptance, or certification
- Close NC-EXT-19, NC-EXT-XXX, or any other NC
- Authorize commit/push (norm N7: only `aeronautics-space-fsw-engineer` after all gates pass)
- Waive the format-check failure (VV-N22-TS547-01)

**Required before commit:**
1. Apply clang-format to `test_evs_hs_e2e.c` (fix VV-N22-TS547-01)
2. Resolve `handover.md` revision mismatch (VV-N22-TS547-07)
3. Complete independent PA review (norm N20) and R4 documentation review (norm N17) of the corrected candidate
4. Re-run `make ci-local` and `make format-check` to green on corrected tree

---

**Reviewer:** Independent Senior Aeronautics and Space SW V&V Engineer  
**Date:** 2026-11-03  
**Review ID:** FRICK-REVIEW-0444-VV-N22-TS5-47