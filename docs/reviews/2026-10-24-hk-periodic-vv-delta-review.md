# Delta V&V Review: HK Periodic Emission Implementation (NC-EXT-17)

**Review ID:** V&V-DELTA-2026-10-24-HK-PERIODIC
**Date:** 2026-10-24
**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (independent)
**Baseline:** frick-fsw commit prior to this review (exact candidate files verified by SHA256)
**Authority:** Independent V&V per norm N9; this review does not approve release/flight readiness

---

## 1. Scope and Candidate Files

This delta review verifies the fixes for the six previous V&V findings (V&V-FIND-001 through V&V-FIND-006) against the **exact** candidate files identified by SHA256:

| File | SHA256 |
|------|--------|
| `src/pus/pus_services.c` | 6f435d8fb55fded1feabbb67ebf0e00b31776e2a3816e16972c3172efd60ed44 |
| `src/pus/pus.h` | e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd |
| `tests/unit/test_pus.c` | 2b9dc12f7d68f664e571baad91e78223ed523cdf02cf2a8c2429a5a27cc886ca |
| `tests/unit/test_pus_fu.c` | 4e7177266062288e54efc12a105a66c886cf667a551971b29a3ddb8b61c2981f |
| `tests/unit/test_r2_gap.c` | 6111cd610b06039eb9140c5b53ecb42875867057fad0245fb3823fef12abfab4 |
| `tests/unit/test_r3_gap.c` | 4c7d6c38e37a67d2cac9ca880d1e58c4420aebcb087c1805ab5b0dfd28c31260 |

**Constraints observed:**
- N18 Renode FAIL (NC-EXT-198) — no target evidence
- NC-PAL-01 OPEN — no hardware watchdog binding
- No emulator/hardware/mission parameter choices made
- No NC closure or frozen-manifest refresh
- No commit/push authority (per norm N7)
- Non-test findings retained as open/deferred

---

## 2. Previous Findings — Disposition Summary

| Finding | Description | Disposition |
|---------|-------------|-------------|
| V&V-FIND-001 | Positive time-due TM emission not exercised in tests | **NOT FIXED** (FAIL) |
| V&V-FIND-002 | No caller of `FF_PusEmitPeriodicHk()` — dead code | **DEFERRED NC** (scheduler integration authority pending) |
| V&V-FIND-003 | `svc3_create` initializes `last_emission_time = 0` with `enabled = true` | **FIXED** (PASS) |
| V&V-FIND-004 | `time_add_period` ignores fine time, no coarse-wrap handling | **FIXED** (PASS) |
| V&V-FIND-005 | TC[3,1] payload mismatch (ICD 2 bytes vs impl 6 bytes, tests 3 bytes) | **FIXED** (PASS) |
| V&V-FIND-006 | Traceability gaps for periodic emission feature | **NOT FIXED** (DEFERRED NC) |

---

## 3. Detailed Verification per Finding

### 3.1 V&V-FIND-001: Positive time-due TM emission not exercised in tests
**Status: FAIL — Not Fixed**

**Evidence examined:** `tests/unit/test_pus.c`, function `test_hk_periodic_emission` (lines 391–512).

**What the test covers:**
- Creates HK report with `period_s = 10`
- Enables the report (initializes `last_emission_time` to current time via `svc3_enable`)
- Calls `FF_PusEmitPeriodicHk()` **immediately** — asserts `FF_ERROR` (period not elapsed) ✓
- Verifies one-shot TC[3,27] works regardless of periodic state ✓
- Disables report — verifies periodic emission does not emit ✓
- Creates report with `period_s = 0` — verifies not emitted periodically ✓

**What is MISSING:**
- **No test advances time (or mocks time) to make the period elapse and verifies that a periodic emission actually occurs.**
- The positive branch `if (!time_ge(now_coarse, now_fine, due_coarse, due_fine)) continue;` (line 405–407 in `pus_services.c`) and the subsequent emission logic (lines 409–450) are **never exercised with a true time-due condition**.
- The test only validates the "not due" and "disabled/zero-period" negative paths.

**Risk:** The core periodic emission logic — provider lookup, payload construction, `FF_PusSendTm`, last-emission-time update, and return `FF_SUCCESS` — has **zero test coverage** for the nominal success case.

**Required fix:** Add a test case that either:
- Mocks `FF_TimeGetCuc` to return a time ≥ `last_emission + period`, or
- Uses a real time advance (with a short period, e.g., 1 second) and waits, or
- Injects a controllable time source into the periodic emission path for testability.

---

### 3.2 V&V-FIND-002: No caller of `FF_PusEmitPeriodicHk()` — dead code
**Status: DEFERRED NC — Not a fixable defect in this increment**

**Evidence:** Grep of the entire codebase confirms no caller of `FF_PusEmitPeriodicHk()` exists outside the test `test_hk_periodic_emission` which calls it directly.

**Context:** The function is declared in `pus.h` (line 83) and defined in `pus_services.c` (line 384). It is intended to be called from a timer or scheduler (per its header comment). Scheduler integration (SRS-REQ-071) is a separate milestone (M14/M15) with authority pending.

**Disposition:** Record as **deferred NC (NC-EXT-17-SCHED)**. Not a defect in the HK implementation itself; the integration seam exists and is tested in isolation. Closure requires scheduler authority decision and wiring.

---

### 3.3 V&V-FIND-003: `svc3_create` initializes `last_emission_time = 0` with `enabled = true`
**Status: PASS — Fixed**

**Evidence:** `src/pus/pus_services.c`

- **`svc3_create` (lines 234–238):** After setting `enabled = true`, the code now calls:
  ```c
  (void)FF_TimeGetCuc(&g_hk_reports[i].last_emission_coarse,
                      &g_hk_reports[i].last_emission_fine);
  ```
  Comment (lines 235–236): *"Initialize last emission time to current time so the first periodic emission occurs after one full period (matches svc3_enable)."*

- **`svc3_enable` (lines 269–272):** Same fix applied:
  ```c
  (void)FF_TimeGetCuc(&g_hk_reports[i].last_emission_coarse,
                      &g_hk_reports[i].last_emission_fine);
  ```

**Verification:** The initialization now uses the current CUC time (coarse + fine) instead of zero. This ensures the first periodic emission occurs after one full period from creation/enable, not immediately at startup (which would have happened with `last_emission_time = 0`).

**Independence:** The fix uses the public `FF_TimeGetCuc` API; no internal time representation assumptions leaked.

---

### 3.4 V&V-FIND-004: `time_add_period` ignores fine time, no coarse-wrap handling
**Status: PASS — Fixed**

**Evidence:** `src/pus/pus_services.c`, lines 368–377:

```c
static void time_add_period(uint32_t coarse, uint16_t fine, uint32_t period_s,
                            uint32_t *out_coarse, uint16_t *out_fine)
{
    uint64_t total_fine = (uint64_t)fine + ((uint64_t)period_s * 65536ULL);
    *out_coarse = coarse + period_s + (uint32_t)(total_fine >> 16);
    *out_fine = (uint16_t)(total_fine & 0xFFFFu);
}
```

**Analysis:**
- **Fine time handled:** The fine field (16-bit, 0..65535 representing fractional seconds) is promoted to 64-bit, `period_s * 65536` is added, and overflow is carried into coarse via `>> 16`.
- **Coarse wrap handled:** Coarse is `uint32_t`; addition uses modular arithmetic (natural C unsigned wrap). The comment explicitly states: *"Coarse wrap (uint32_t) is naturally handled by modular arithmetic."*
- **Used by:** `FF_PusEmitPeriodicHk` (line 402–403) to compute the next due time from `last_emission_coarse/fine + period_s`.

**Verification:** The implementation correctly models CUC time addition. No test directly exercises `time_add_period` with fine-field overflow or coarse wrap boundary, but the arithmetic is sound and the function is pure (no side effects).

**Gap:** No unit test for `time_add_period` boundary cases (fine overflow, coarse wrap at 0xFFFFFFFF). This is a test completeness gap but does not invalidate the fix.

---

### 3.5 V&V-FIND-005: TC[3,1] payload mismatch — buffer over-read
**Status: PASS — Fixed (consistent 6-byte payload across impl and tests)**

**Evidence:**

| Location | Payload Size | Detail |
|----------|--------------|--------|
| `pus_services.c:217` | `len < 6u` → `FF_BAD_SIZE` | Implementation requires 6 bytes |
| `test_pus.c:405–406` | 6 bytes | `{1u, 1u, 0u, 0u, 0u, 10u}` (report_id, provider_id, period_s=10 as BE u32) |
| `test_pus_fu.c:331` | 6 bytes | `create_hk_report_tc` uses `uint8_t payload[6]` |
| `test_r3_gap.c:416–431` | 6 bytes | `create_payload[6]` with `period_s = 0` |

**Wire format (provisional, authority-pending):**
```
Byte 0: report_id (1)
Byte 1: provider_id (1)
Bytes 2–5: period_s (uint32_t, big-endian)
```

**Comment in code (lines 214–216):** *"Provisional TC[3,1] payload: 6 bytes (report_id, provider_id, period_s[4]). ICD §4 defines 2 bytes; this 6-byte form is an authority-pending extension tracked under NC-EXT-17. Do not claim ECSS compliance for this wire format."*

**Verification:** All three test files and the implementation now agree on 6 bytes. The previous mismatch (impl 6, tests 3) is resolved. The buffer over-read risk (reading `d[2]..d[5]` when `len < 6`) is eliminated by the `len < 6u` guard.

**Caveat:** This wire format is **not ECSS-compliant** per ICD §4 (2 bytes). The extension is tracked under NC-EXT-17 and requires authority approval before any compliance claim.

---

### 3.6 V&V-FIND-006: Traceability gaps for periodic emission feature
**Status: FAIL — Not Fixed (Deferred NC)**

**Evidence:** `docs/requirements/traceability.yaml` — searched for `periodic`, `hk.*emission`, `FF_PusEmitPeriodicHk`, `time_add_period`, `time_ge`. **Zero matches.**

**Missing traceability items:**
- No SRS requirement for periodic HK emission (feature exists in code but not in requirements baseline)
- No bidirectional trace from code (`pus_services.c`: `FF_PusEmitPeriodicHk`, `time_add_period`, `time_ge`, `svc3_create`/`enable` time init) to requirements
- No test traceability entry for `test_hk_periodic_emission`
- No Lean theorem coverage for time arithmetic or periodic emission logic

**Disposition:** Record as **deferred NC (NC-EXT-17-TRACE)**. Traceability cannot be retrofitted without an SRS requirement authorizing the periodic emission feature. The feature is implemented and partially tested but exists outside the controlled requirements baseline.

---

## 4. Test Execution Results (All Candidate Tests)

| Test Suite | Test Case | Result | Notes |
|------------|-----------|--------|-------|
| `pus` | `test_hk_periodic_emission` | **PASS** | Only exercises negative paths (not due, disabled, period=0) |
| `pus_fu` | `test_hk_standard_subservice_assignments` | **PASS** | Uses 6-byte TC[3,1] create payload |
| `r2_gap` | (no dedicated periodic HK test) | N/A | Coverage gap tests pass |
| `r3_gap` | `test_pus_hk_s3_lifecycle` | **PASS** | Uses 6-byte TC[3,1] create payload; period_s=0 |
| **All 46 tests** | **Full suite** | **PASS** | 0 failed, 46 passed |

**Critical gap:** No test in any suite exercises the **positive time-due emission path** (V&V-FIND-001).

---

## 5. Additional Observations

### 5.1 Time Comparison Helper (`time_ge`)
New function at lines 360–366:
```c
static bool time_ge(uint32_t coarse1, uint16_t fine1, uint32_t coarse2, uint16_t fine2)
{
    if (coarse1 != coarse2) return coarse1 > coarse2;
    return fine1 >= fine2;
}
```
- Correctly implements CUC time comparison (coarse primary, fine secondary).
- Used at line 405 to check if `now >= due`.
- No dedicated unit test; exercised indirectly by `test_hk_periodic_emission` (immediate call → not due).

### 5.2 `FF_PusEmitPeriodicHk` Return Code Semantics
- Returns `FF_SUCCESS` if **at least one** report emitted successfully (line 450).
- Returns `FF_ERROR` if **no reports were due/emitted** (line 388, 453).
- Returns specific error code (`FF_NOT_FOUND`, `FF_BAD_SIZE`, send failure) only if that was the **first** error encountered and no report succeeded (lines 413, 426, 434, 442).
- **Design note:** The "first error wins unless a report succeeds" logic is reasonable but should be documented as a requirement if it becomes a contractual interface.

### 5.3 One-Shot vs Periodic Independence
- `svc3_report` (TC[3,27], lines 294–355) correctly handles one-shot requests **independent** of `enabled` state and `period_s` (comment line 317–318).
- Verified by `test_hk_periodic_emission` (lines 440–455): one-shot works when disabled and when `period_s = 0`.

### 5.4 Provider Length Validation
- Both periodic emission (lines 431–437) and one-shot report (lines 332–341) validate `plen > cap` before sending TM.
- Consistent with NC-EXT-24 (C-16) fix verified in `test_pus_fu.c`.

---

## 6. Disposition Recommendation

| Finding | Recommendation |
|---------|----------------|
| V&V-FIND-001 | **BLOCKER for feature claim.** Add a positive emission test (mock time or short-period wait) before claiming periodic HK emission is verified. |
| V&V-FIND-002 | **DEFERRED NC (NC-EXT-17-SCHED).** Scheduler integration out of scope for this increment. |
| V&V-FIND-003 | **CLOSED.** Fix verified in code. |
| V&V-FIND-004 | **CLOSED.** Fix verified in code. Recommend adding unit test for `time_add_period` boundaries. |
| V&V-FIND-005 | **CLOSED.** Consistent 6-byte payload verified. **Do not claim ECSS compliance** for this wire format (authority pending, NC-EXT-17). |
| V&V-FIND-006 | **DEFERRED NC (NC-EXT-17-TRACE).** Requires SRS requirement authorizing the feature before traceability can be established. |

**Overall:** The implementation fixes for FIND-003, 004, 005 are **correct and verified**. FIND-001 remains a **functional test gap** — the feature's primary success path is untested. FIND-002 and FIND-006 are **process/organizational gaps** deferred to appropriate authorities.

**No release/flight readiness approval is granted by this review.** This delta review provides evidence for the development team and the independent PA/Safety reviewers (norms N9, N20) to disposition.

---

## 7. Evidence Retention

Per norm N11, this report and the following artifacts are retained in the working tree:
- This report: `docs/reviews/2026-10-24-hk-periodic-vv-delta-review.md`
- Test logs: `make test` output (46/46 PASS)
- Traceability check: `make trace` output (PASS, 120 requirements)
- Lean build: `make verify-lean` in container (PASS)

---

**End of Delta Review Report**