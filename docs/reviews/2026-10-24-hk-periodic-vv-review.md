# Independent Senior SW V&V Engineer Review Report
## HK Periodic Emission Implementation (NC-EXT-17)

**Date:** 2026-10-24
**Reviewer:** Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer
**Scope:** Source code, requirements, tests, and traceability for the HK periodic emission feature
**Files under review (verified SHA256):**
- `src/pus/pus_services.c`: 360937e04e0fced837b39d633beb785bc9a575cbad7de49876eef9fea2491f7d
- `src/pus/pus.h`: e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd
- `tests/unit/test_pus.c`: 8b92cd845235723644904205d7a79d1582bad553440be0663d2bca9b67e091c3
- `tests/unit/test_pus_fu.c`: 9da8ea819a132e0ad647c79d9f03ad9d971025d2409b28afbe40074083034cb2
- `tests/unit/test_r2_gap.c`: beea686fc98b1844bfa463dcacfdbf888eb63fa38c27f105716ddc14b9bb25ad
- `tests/unit/test_r3_gap.c`: 5425a3feb512345b2d6e59578b2143e8356e22cdcec0e137c146aa674ff0e0e6

---

## Executive Summary

**Overall Disposition:** **FAIL** — Critical functional defects and test gaps prevent acceptance. The periodic emission feature is not verified; the positive emission path is untested, the scheduler integration is absent, the TC[3,1] payload format diverges from the ICD without authority disposition, and a buffer over-read exists in the create handler.

**Gate status after this review:** All affected gates remain RED. No commit/push authorized.

---

## 1. Positive Time-Due TM Emission — NOT EXERCISED IN TESTS

### Finding: Major — V&V-FIND-001
**Test gap:** The test `test_hk_periodic_emission` in `test_pus.c` (lines 391–511) calls `FF_PusEmitPeriodicHk()` exactly once at line 437, immediately after enabling the report, and asserts `FF_ERROR` (no emission because period has not elapsed). The test **never** advances time or simulates a period elapse, then calls `FF_PusEmitPeriodicHk()` again to verify that a periodic TM[3,25] is actually emitted when due.

**Evidence:**
- Lines 436–437: `/* Call periodic emission immediately - should not emit because period hasn't elapsed. */ FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);`
- No subsequent test step manipulates `FF_TimeSetUtcNs`/`FF_TimeToneSignal` or mocks `FF_TimeGetCuc` to make `now >= due`.
- The test then disables the report (line 457) and verifies no emission — but the positive emission case is never reached.
- `test_r3_gap.c::test_pus_hk_s3_lifecycle` (lines 408–480) exercises the one-shot TC[3,27] path and enable/disable flags but **does not call `FF_PusEmitPeriodicHk()` at all**.

**Impact:** The core functional claim — "periodic HK reports are emitted when their period elapses" — has **zero objective evidence**. A test that only checks the "not due" branch achieves no coverage of the emission logic (lines 397–442 in `pus_services.c`).

**Required fix:** Add a test case that:
1. Creates a report with a short period (e.g., 1 s).
2. Enables it (which sets `last_emission_time = now`).
3. Advances the time base by ≥ period (via `FF_TimeSetUtcNs` or `FF_TimeToneSignal`).
4. Calls `FF_PusEmitPeriodicHk()` and verifies `FF_SUCCESS` + TM[3,25] emitted with correct report_id and provider data.
5. Verifies `last_emission_time` is updated to the emission time (not the due time).

---

## 2. Scheduler/App Periodic Integration — ABSENT

### Finding: Major — V&V-FIND-002
**No caller of `FF_PusEmitPeriodicHk()` exists in the codebase.**

**Evidence:**
- `pus.h` declares `FF_Status_t FF_PusEmitPeriodicHk(void);` (line 83).
- `pus_services.c` implements it (lines 376–446).
- `grep -r "FF_PusEmitPeriodicHk" src/` returns **only** the declaration and definition.
- `src/apps/app_scheduler.c` (table-driven dispatcher for SB messages) does not reference it.
- `src/core/sched/ff_sched.c` does not reference it.
- `src/main/fsw_main.c` does not call it in any initialization or cyclic task.
- No RTEMS timer, POSIX timer, or scheduler entry is configured to invoke it.

**Impact:** Even if the implementation were correct, the function is **dead code** in the flight build. Periodic HK emission will never occur on target.

**Required fix:** The system design must designate a caller (e.g., a 1 Hz RTEMS timer task, the scheduler's tick, or a dedicated HK manager task) and wire the call. This is a **system-level integration decision**, not a unit-test fix. The V&V plan must capture this as a verification objective at the system/integration level (SIL/PIL/HIL), not unit level.

---

## 3. Enable/Disable/One-Shot Semantics — PARTIAL CORRECTNESS; INITIALIZATION BUG

### Finding: Major — V&V-FIND-003
**Inconsistent `last_emission_time` initialization between create and enable.**

**Code evidence (`pus_services.c`):**
- `svc3_create` (lines 225–234): Creates report with `enabled = true`, `period_s = period`, `last_emission_coarse = 0`, `last_emission_fine = 0`.
- `svc3_enable` (lines 262–267): Sets `enabled = true` **and** `last_emission_time = now` (via `FF_TimeGetCuc`).

**Defect:** A report created with `period_s > 0` is enabled by default but has `last_emission_time = 0`. On the first call to `FF_PusEmitPeriodicHk()`:
- `due_coarse = 0 + period_s`, `due_fine = 0`.
- `time_ge(now, due)` will be **true** as soon as `now >= period_s` seconds since epoch.
- Since the system boots at UTC ~0 (per `FF_TimeInit` correlating to Unix epoch 0), the first periodic emission will fire **immediately** (or after `period_s` seconds of uptime), not after one full period from creation/enable.

**Contrast:** `svc3_enable` correctly initializes `last_emission_time = now`, so a disabled-then-enabled report waits one full period.

**Test coverage:** The test `test_hk_periodic_emission` creates a report with `period_s = 10` (line 405), enables it (line 431), then immediately calls `FF_PusEmitPeriodicHk()` (line 437). At that point, `now` is near 0 (test doesn't advance time), so `now < 10` and the test passes `FF_ERROR` — but this is **accidental**, not by correct initialization. If the test advanced time by 11 seconds, the bug would manifest as an immediate emission.

**Required fix:** `svc3_create` must initialize `last_emission_time` to current time (via `FF_TimeGetCuc`) when `period_s > 0`, consistent with `svc3_enable`. Or, set `enabled = false` by default in create and require an explicit enable TC (aligning with ECSS-E-ST-70-41C where enable is a separate command). The current hybrid (create = enabled, but last_emission = 0) is inconsistent and unsafe.

---

## 4. Rollover/Time Arithmetic and Error States

### Finding: Minor — V&V-FIND-004
**`time_add_period` ignores `fine` and does not handle coarse wrap.**

**Code (`pus_services.c`, lines 363–369):**
```c
static void time_add_period(uint32_t coarse, uint16_t fine, uint32_t period_s,
                            uint32_t *out_coarse, uint16_t *out_fine)
{
    *out_coarse = coarse + period_s;
    *out_fine = fine;
}
```

**Issues:**
1. **Fine time ignored:** The period is whole seconds only. If `fine != 0`, the due time is effectively truncated to the previous second boundary. For a 1 Hz periodic report, this introduces up to 1 second of phase drift per period.
2. **Coarse wrap:** `coarse + period_s` uses unsigned 32-bit wrap semantics. `time_ge` (lines 355–361) compares `coarse1 > coarse2` then `fine1 >= fine2`. After wrap, `coarse` becomes small, so `time_ge(now, due)` incorrectly returns false until `now` also wraps (~136 years). For the mission lifetime this is likely acceptable, but it is a latent defect if the mission exceeds 2^32 seconds or if `period_s` is large.
3. **No error return:** The function cannot signal overflow. If `period_s == 0`, the caller skips the report (line 387), but `time_add_period` is not called in that case.

**Test coverage:** No test exercises `time_add_period` directly or via `FF_PusEmitPeriodicHk` with non-zero `fine` or wrap conditions.

**Required fix:** Either document the whole-second assumption as a design constraint (and verify `fine` is always 0 at emission instants), or implement proper 48-bit time addition with carry. Add a static assertion or analysis that wrap is not credible within mission duration.

---

## 5. TC[3,1] Payload: 2-Byte ICD vs 6-Byte Implementation — CRITICAL MISMATCH

### Finding: Major — V&V-FIND-005
**The implementation reads a 4-byte `period_s` field that the ICD does not define, and the test provides only 3 bytes causing a buffer over-read.**

**ICD §4 (FRICK-ICD-001 rev 1.5, line 109):**
| Service | Subservice | Name | TC/TM | Data content |
| 3 | 1 | create housekeeping report definition | TC | report id[1] + provider id[1] |

**Implementation (`pus_services.c`, lines 214–219):**
```c
if (len < 3u) { return FF_BAD_SIZE; }
uint8_t report_id = d[0];
uint8_t provider = d[1];
uint32_t period_s = rd_u32(&d[2]);  // Reads 4 bytes: d[2], d[3], d[4], d[5]
```

**Test (`test_pus.c`, line 405):**
```c
uint8_t payload[3] = {1u, 1u, 10u};  // Only 3 bytes: report_id, provider, period_s (1 byte!)
```

**Analysis:**
- The handler expects **6 bytes** minimum (1 + 1 + 4), but checks `len < 3`.
- The test provides **3 bytes**, so `rd_u32(&d[2])` reads `d[2]=0x10`, `d[3]=0x00` (uninitialized stack), `d[4]=0x00`, `d[5]=0x00` → `period_s = 0x10000000` (268,435,456 seconds ≈ 8.5 years).
- This is a **buffer over-read** (CWE-125) and a **silent logic error** — the period is not 10 seconds as the test author intended.
- The ICD was updated in rev 1.3 (2026-10-17) to list subservice codes but **the TC[3,1] payload row still says 2 bytes**. Rev 1.4 (2026-10-17) added "provisional-profile" notes but did not correct the TC[3,1] payload.
- ADR-0020 (provisional PUS and scheduler policies) is cited as the authority for the Service 3 subservice allocation, but the TC[3,1] payload extension is not documented there.

**Authority boundary:** Per the ICD §1 (line 13–14): "This ICD documents the implemented space-to-ground and ground-to-space packet interface as an internal reference profile for SRS-REQ-017/SSS-REQ-014. It is **not an approved mission tailoring baseline**; applicable profile values and acceptance rules require approval by the authorized mission/customer authority."

**V&V position:** The implementation diverges from the documented ICD without a recorded authority decision. The test is invalid (exercises undefined behavior). This must be resolved by:
1. Either updating the ICD to reflect the 6-byte payload (report_id[1] + provider_id[1] + period_s[4]) and obtaining authority approval, OR
2. Reverting the implementation to the 2-byte payload and moving `period_s` to a separate TC (e.g., a new subservice or an extended create), OR
3. Documenting the divergence as a controlled NC with authority disposition.

**Do not claim ECSS authorizes this private wire format.** The ICD explicitly states it is provisional and not an approved profile. ECSS-E-ST-70-41C requires mission tailoring and an approved ICD; this framework document is not that ICD (ICD §4 provisional note, lines 138–151).

---

## 6. Source/Requirements/Tests Traceability

### Finding: Minor — V&V-FIND-006
**Traceability gaps in `docs/requirements/traceability.yaml`.**

| Requirement | Implementation | Tests | Gap |
|-------------|---------------|-------|-----|
| SRS-REQ-008 (HK create/enable/disable/remove/report) | `pus_services.c`, `app_hk.c` | `test_pus.c`, `test_r3_gap.c`, `test_pus_fu.c` | Periodic emission (`FF_PusEmitPeriodicHk`) is **not traced** to any requirement. The function exists but no SRS/SSS requirement mandates periodic generation. |
| SRS-REQ-008 | `pus_services.c::svc3_create` | `test_pus.c::test_hk_periodic_emission` | TC[3,1] payload mismatch (see V&V-FIND-005) — the test does not match the implementation's expected payload. |
| — | `pus_services.c::FF_PusEmitPeriodicHk` | — | No requirement maps to this function. No test verifies its positive path. |

**Impact:** The verification cross-reference cannot demonstrate that the periodic emission feature satisfies any approved requirement. The feature appears to be an undeclared implementation addition.

---

## 7. Test Case PASS/FAIL/NOT RUN Summary

| Test Case | File | Status | Notes |
|-----------|------|--------|-------|
| `test_hk_periodic_emission` | `test_pus.c` | **FAIL** (logic) | Does not exercise positive emission; test payload causes buffer over-read; passes only by accident (time not advanced). |
| `test_hk_standard_subservice_assignments` | `test_pus_fu.c` | **PASS** (nominal) | Exercises create/enable/disable/delete/one-shot via TC ingestion; does not call `FF_PusEmitPeriodicHk`. |
| `test_pus_hk_s3_lifecycle` | `test_r3_gap.c` | **PASS** (nominal) | Exercises S3 lifecycle via TC ingestion; does not call `FF_PusEmitPeriodicHk`. |
| `test_ncext_24_provider_length_bounded` | `test_pus_fu.c` | **PASS** | Provider length validation; unrelated to periodic emission. |
| Unit tests for `time_add_period` / `time_ge` | — | **NOT RUN** | No dedicated tests for the time arithmetic helpers. |
| Integration test calling `FF_PusEmitPeriodicHk` from scheduler/timer | — | **NOT RUN** | No such test exists; no caller exists in the codebase. |

---

## 8. Disposition Recommendation

### Immediate Actions Required (Blockers)

| ID | Action | Owner | Gate |
|----|--------|-------|------|
| V&V-FIND-001 | Add positive periodic emission test case with time advance and TM[3,25] oracle | FSW Engineer | Unit test gate |
| V&V-FIND-002 | Define and implement system-level caller for `FF_PusEmitPeriodicHk` (timer/scheduler/task) | Systems + FSW | Integration gate (SIL/PIL) |
| V&V-FIND-003 | Fix `svc3_create` to initialize `last_emission_time = now` when `period_s > 0` (or change create to disabled-by-default) | FSW Engineer | Unit test gate |
| V&V-FIND-005 | Resolve TC[3,1] payload discrepancy: update ICD + authority approval, OR revert implementation, OR record controlled NC | Requirements Authority + FSW | ICD baseline gate |
| V&V-FIND-006 | Add traceability entries for `FF_PusEmitPeriodicHk` and periodic emission requirement | FSW Engineer | Traceability gate (N4) |

### Deferred / Non-Blocking (Record as NC)

| ID | Action | Owner |
|----|--------|-------|
| V&V-FIND-004 | Document/fix `time_add_period` fine handling and coarse wrap analysis | FSW Engineer (NC-EXT-XXX) |

---

## 9. Independent Delta Review Requirement

Per N7 and N20, **all fixes above must undergo independent delta review** by the Senior SW V&V Engineer (this role) and the Senior SW Product Assurance Engineer (`aeronautics-space-sw-pa-engineer`) before any commit/push. The review must cover:
- Corrected source code
- Updated tests with positive emission evidence
- Updated ICD with authority disposition on TC[3,1] payload
- Updated traceability.yaml
- Re-run of `make build`, `make test`, `make verify-lean`, `make trace`

**No commit/push until delta review is complete and all gates GREEN.**

---

## 10. Authority Statement

This review is an **AI analysis role** per the V&V persona instructions. It has no human credentials, regulatory delegation, proprietary-standard entitlement, test-witness authority, independence by mere declaration, or power to approve qualification, acceptance, certification, compliance, anomaly closure, or flight readiness. Safety-critical conclusions require qualified independent human review and the approvals defined by the governing baseline.

**Evidence ledger:** All findings reference specific source lines, test lines, ICD clauses, and traceability entries as recorded above. Raw evidence (source files, test files, ICD, traceability.yaml) is retained in the working tree per N11.

---

*End of Report*