# HK Periodic Emission — Safety Delta Review (NC-EXT-17)

**Date:** 2026-10-24  
**Reviewer:** Senior Aeronautics and Space Software Safety Engineer (Independent)  
**Review Type:** Delta review of fixed candidate against previous safety findings  
**Candidate Files (verified SHA256):**
- `src/pus/pus_services.c`: 6f435d8fb55fded1feabbb67ebf0e00b31776e2a3816e16972c3172efd60ed44
- `src/pus/pus.h`: e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd
- `tests/unit/test_pus.c`: 2b9dc12f7d68f664e571baad91e78223ed523cdf02cf2a8c2429a5a27cc886ca
- `tests/unit/test_pus_fu.c`: 4e7177266062288e54efc12a105a66c886cf667a551971b29a3ddb8b61c2981f
- `tests/unit/test_r2_gap.c`: 6111cd610b06039eb9140c5b53ecb42875867057fad0245fb3823fef12abfab4
- `tests/unit/test_r3_gap.c`: 4c7d6c38e37a67d2cac9ca880d1e58c4420aebcb087c1805ab5b0dfd28c31260

**Previous Review:** FRICK-REVIEW-0076 (R3) / safety findings SF-01..SF-08  
**Escalation Items:** ESC-HK-01..05 (require human authority)  
**Constraints:** N18 Renode FAIL (NC-EXT-198), NC-PAL-01 OPEN, no ECSS compliance claim for private wire format, no NC closure or commit/push

---

## 1. Summary of Findings

| Finding | Previous Severity | Delta Status | Disposition |
|---------|-------------------|--------------|-------------|
| SF-01: TC[3,1] buffer over-read | Major | **FIXED** | Code now validates `len >= 6` before reading 6 bytes |
| SF-02: `svc3_create` init defect (`last_emission_time = 0` with `enabled = true`) | Major | **FIXED** | `last_emission_coarse/fine` initialized to current time via `FF_TimeGetCuc` |
| SF-03: No caller / dead code with latent defects | Major | **DEFERRED (Authority)** | `FF_PusEmitPeriodicHk()` exists but no caller in candidate; ESC-HK-02 pending |
| SF-04: `time_add_period` fine time ignored + wrap handling | Major | **FIXED** | New implementation uses 64-bit arithmetic, handles fine overflow and coarse wrap |
| SF-05: Software-only time source → ground TM misinterpretation | Major | **DEFERRED (Authority)** | Private wire format acknowledged; ESC-HK-04 pending |
| SF-06: ICD §4 does not authorize 6-byte TC[3,1] | Major | **DEFERRED (Authority)** | Code comment explicitly flags as "authority-pending extension (NC-EXT-17)"; ESC-HK-01 pending |
| SF-07: No hazard analysis for periodic HK | Major | **DEFERRED (Authority)** | No hazard analysis in candidate; ESC-HK-03 pending |
| SF-08: Test evidence gaps | Minor | **PARTIALLY FIXED** | Positive path test added but **does not test emission after period elapses** (time not advanced) |

---

## 2. Detailed Verification per Finding

### 2.1 SF-01: TC[3,1] Buffer Over-Read — **FIXED**

**Previous Issue:** `svc3_create` read 6 bytes (`d[0]..d[5]`) without validating `len >= 6`.

**Candidate Evidence (`pus_services.c:211-222`):**
```c
static FF_Status_t svc3_create(const FF_Pus_TcHdr_t *tc, const uint8_t *d, uint16_t len)
{
    (void)tc;
    /* Provisional TC[3,1] payload: 6 bytes (report_id, provider_id, period_s[4]).
     * ICD §4 defines 2 bytes; this 6-byte form is an authority-pending extension
     * tracked under NC-EXT-17. Do not claim ECSS compliance for this wire format. */
    if (len < 6u) {
        return FF_BAD_SIZE;
    }
    uint8_t report_id = d[0];
    uint8_t provider = d[1];
    uint32_t period_s = rd_u32(&d[2]);
    ...
}
```

**Assessment:** The length check `if (len < 6u) return FF_BAD_SIZE;` precedes all payload accesses. The buffer over-read is eliminated. **PASS**.

---

### 2.2 SF-02: `svc3_create` Init Defect — **FIXED**

**Previous Issue:** `last_emission_time` initialized to 0 while `enabled = true`, causing immediate (time=0) periodic emission on first scheduler tick.

**Candidate Evidence (`pus_services.c:228-242`):**
```c
for (uint16_t i = 0; i < FF_PUS_MAX_HK_REPORTS; i++) {
    if (!g_hk_reports[i].used) {
        g_hk_reports[i].used = true;
        g_hk_reports[i].report_id = report_id;
        g_hk_reports[i].provider_id = provider;
        g_hk_reports[i].enabled = true;
        g_hk_reports[i].period_s = period_s;
        /* Initialize last emission time to current time so the first
         * periodic emission occurs after one full period (matches svc3_enable). */
        (void)FF_TimeGetCuc(&g_hk_reports[i].last_emission_coarse,
                            &g_hk_reports[i].last_emission_fine);
        return FF_SUCCESS;
    }
}
```

**Consistency Check:** `svc3_enable` (lines 260-277) uses the identical initialization pattern.

**Assessment:** The initialization now correctly sets `last_emission_time = now`, so the first periodic emission occurs after one full period. **PASS**.

---

### 2.3 SF-03: No Caller / Dead Code — **DEFERRED (Authority Required)**

**Previous Issue:** `FF_PusEmitPeriodicHk()` had no caller in the codebase, making the periodic emission logic dead code with unexercised latent defects.

**Candidate Evidence:** The function `FF_PusEmitPeriodicHk()` is implemented at `pus_services.c:384-454` and declared in `pus.h:83`. **No caller exists in any of the six candidate files.**

**Escalation Item ESC-HK-02:** Scheduler integration authority for `FF_PusEmitPeriodicHk()` caller.

**Assessment:** The implementation is present and no longer has the latent defects (SF-02, SF-04 fixed), but it remains uncalled. This is an **architectural integration decision** requiring human authority (scheduler design, timer configuration, task assignment). **DEFERRED — ESC-HK-02 OPEN**.

---

### 2.4 SF-04: `time_add_period` Fine Time Ignored + Wrap Handling — **FIXED**

**Previous Issue:** Original `time_add_period` ignored the `fine` field and did not handle coarse wrap.

**Candidate Evidence (`pus_services.c:368-377`):**
```c
/* Time addition: adds period_s seconds to (coarse, fine), result in (coarse, fine).
 * Handles fine-field overflow (fine is 16-bit, 0..65535 representing fractional seconds).
 * Coarse wrap (uint32_t) is naturally handled by modular arithmetic. */
static void time_add_period(uint32_t coarse, uint16_t fine, uint32_t period_s, uint32_t *out_coarse,
                            uint16_t *out_fine)
{
    uint64_t total_fine = (uint64_t)fine + ((uint64_t)period_s * 65536ULL);
    *out_coarse = coarse + period_s + (uint32_t)(total_fine >> 16);
    *out_fine = (uint16_t)(total_fine & 0xFFFFu);
}
```

**Verification:**
- Fine field (16-bit, 65536 ticks/second) is correctly incorporated via `period_s * 65536ULL`.
- Fine overflow (`total_fine >> 16`) carries into coarse.
- Coarse addition uses `uint32_t` modular arithmetic (natural wrap).
- Comparison helper `time_ge` (lines 360-366) correctly compares `(coarse, fine)` tuples.

**Assessment:** The time arithmetic is now correct for periodic scheduling. **PASS**.

---

### 2.5 SF-05: Software-Only Time Source → Ground TM Misinterpretation — **DEFERRED (Authority Required)**

**Previous Issue:** Periodic HK uses software-only time source (`FF_TimeGetCuc`) but emits TM that ground interprets as UTC-synchronized CUC, risking misinterpretation of period/drift.

**Candidate Evidence:** The code comment at `pus_services.c:214-216` explicitly states:
> "Provisional TC[3,1] payload: 6 bytes... ICD §4 defines 2 bytes; this 6-byte form is an authority-pending extension tracked under NC-EXT-17. Do not claim ECSS compliance for this wire format."

**Escalation Item ESC-HK-04:** Ground TM misinterpretation risk from private wire format.

**Assessment:** The implementation acknowledges the non-compliant wire format. The risk remains until ICD §4 is formally extended or a compliant alternative is adopted. **DEFERRED — ESC-HK-04 OPEN**.

---

### 2.6 SF-06: ICD §4 Does Not Authorize 6-Byte TC[3,1] — **DEFERRED (Authority Required)**

**Previous Issue:** TC[3,1] create payload uses 6 bytes (report_id, provider_id, period_s[4]) but ICD §4 defines only 2 bytes.

**Candidate Evidence:** Same comment as SF-05. The 6-byte format is documented as "authority-pending extension (NC-EXT-17)".

**Escalation Item ESC-HK-01:** ICD §4 TC[3,1] format authority decision.

**Assessment:** No ICD change evidenced in candidate. The wire format deviation is explicitly flagged and not claimed as compliant. **DEFERRED — ESC-HK-01 OPEN**.

---

### 2.7 SF-07: No Hazard Analysis for Periodic HK — **DEFERRED (Authority Required)**

**Previous Issue:** No hazard analysis covering periodic HK emission (timing errors, duplicate emission, missed emission, buffer exhaustion, ground misinterpretation).

**Candidate Evidence:** No hazard analysis artifacts in any candidate file. The implementation adds functional logic but no safety analysis.

**Escalation Item ESC-HK-03:** Hazard analysis for periodic HK emission.

**Assessment:** Safety analysis is a separate work product requiring system safety authority. **DEFERRED — ESC-HK-03 OPEN**.

---

### 2.8 SF-08: Test Evidence Gaps — **PARTIALLY FIXED**

**Previous Issue:** No test exercised the positive periodic emission path (period elapsed → emission occurs).

**Candidate Evidence (`test_pus.c:391-512`, `test_hk_periodic_emission`):**

The test:
1. Registers HK provider ✓
2. Creates HK report with `period_s = 10` ✓
3. Enables report (initializes `last_emission_time = now`) ✓
4. Calls `FF_PusEmitPeriodicHk()` **immediately** → expects `FF_ERROR` (not due) ✓
5. Tests one-shot TC[3,27] works regardless of periodic state ✓
6. Disables report → periodic emission returns `FF_ERROR` ✓
7. Tests `period_s = 0` reports not emitted periodically ✓

**Critical Gap:** The test **does not advance time** or mock `FF_TimeGetCuc` to simulate period elapsed. The positive emission path (period elapsed → `FF_PusEmitPeriodicHk()` returns `FF_SUCCESS` with TM emitted) is **not tested**.

**Test Coverage Matrix:**
| Scenario | Tested? |
|----------|---------|
| Period not elapsed → no emission | ✅ Yes |
| Disabled report → no emission | ✅ Yes |
| `period_s = 0` → no periodic emission | ✅ Yes |
| One-shot TC[3,27] works when disabled | ✅ Yes |
| **Period elapsed → emission occurs** | ❌ **No** |
| Multiple reports, staggered periods | ❌ No |
| Provider failure during periodic emission | ❌ No |
| TM send failure during periodic emission | ❌ No |
| Time wrap (coarse overflow) | ❌ No |

**Assessment:** The test adds valuable negative/guardrail coverage but **does not close the positive emission path gap**. **PARTIAL PASS — RESIDUAL GAP**.

---

## 3. Escalation Status Summary

| Escalation ID | Description | Status | Required Authority |
|---------------|-------------|--------|-------------------|
| ESC-HK-01 | ICD §4 TC[3,1] format authority decision | **OPEN** | ICD Control Board / Mission System Engineer |
| ESC-HK-02 | Scheduler integration for `FF_PusEmitPeriodicHk()` caller | **OPEN** | FSW Architect / Scheduler Owner |
| ESC-HK-03 | Hazard analysis for periodic HK emission | **OPEN** | System Safety Engineer |
| ESC-HK-04 | Ground TM misinterpretation risk from private wire format | **OPEN** | Ground System Engineer / ICD Control Board |
| ESC-HK-05 | SRS requirement for periodic HK emission | **OPEN** | Requirements Authority / Senior Systems Engineer |

**All five escalation items remain OPEN.** None are resolved by the candidate changes.

---

## 4. Additional Observations

### 4.1 Test `test_r3_gap.c` Also Uses 6-Byte TC[3,1] Payload
`test_r3_gap.c:423-431` constructs the same 6-byte create payload, confirming the wire format is exercised in integration tests but still flagged as non-compliant.

### 4.2 `FF_PusEmitPeriodicHk()` Return Code Semantics
- Returns `FF_SUCCESS` if **at least one** report emitted successfully
- Returns `FF_ERROR` if **no reports were due/emitted**
- Returns specific error code if a send failed (last failure)

This is reasonable but should be documented in the header (currently only a brief comment at `pus.h:81-83`).

### 4.3 Interaction with `svc3_report` (TC[3,27])
The implementation correctly treats TC[3,27] as independent of periodic state (line 317-318 comment). Verified in `test_pus.c:440-455` and `test_r3_gap.c:456-474`.

### 4.4 No Renode / Target Evidence (N18)
**Constraint acknowledged:** N18 Renode FAIL (NC-EXT-198) — no target evidence for the periodic emission path on the emulated PolarFire SoC. The host tests exercise the logic but not the target timing behavior, scheduler integration, or interrupt context.

---

## 5. Disposition Recommendation

### 5.1 Acceptable for Continued Development (with constraints)
The candidate **fixes the code-level defects (SF-01, SF-02, SF-04)** and adds **partial test coverage (SF-08)**. The remaining findings (SF-03, SF-05, SF-06, SF-07) are **authority-dependent** and properly flagged/escalated.

### 5.2 Conditions Before Any Release Claim
1. **ESC-HK-01 resolved:** ICD §4 formally extended or compliant wire format adopted.
2. **ESC-HK-02 resolved:** Scheduler integration implemented and tested (caller for `FF_PusEmitPeriodicHk()`).
3. **ESC-HK-03 resolved:** Hazard analysis completed and reviewed.
4. **ESC-HK-04 resolved:** Ground system acceptance of wire format or format changed.
5. **ESC-HK-05 resolved:** SRS requirement traced to implementation.
6. **Test gap closed:** Positive periodic emission path tested (time advance / mock).
7. **N18 satisfied:** Renode functional validation passes on target.
8. **NC-PAL-01 resolved:** Hardware watchdog binding verified.

### 5.3 Immediate Actions
- [ ] Add positive emission test case (mock `FF_TimeGetCuc` or advance test time)
- [ ] Document `FF_PusEmitPeriodicHk()` return code contract in `pus.h`
- [ ] Record all five ESC-HK items in project risk register with owners and due dates
- [ ] Do **not** claim ECSS compliance for periodic HK until ESC-HK-01/03/04 resolved
- [ ] Do **not** close NC-EXT-17

---

## 6. Confidence Assessment

| Aspect | Confidence | Basis |
|--------|------------|-------|
| SF-01 fixed | **High** | Direct code inspection; length check before access |
| SF-02 fixed | **High** | Direct code inspection; `FF_TimeGetCuc` called at creation/enable |
| SF-04 fixed | **High** | Direct code inspection; 64-bit arithmetic, correct carry/wrap logic |
| SF-03 deferred | **High** | No caller in any candidate file; architectural decision required |
| SF-05/06 deferred | **High** | Explicit comment in code; no ICD change evidenced |
| SF-07 deferred | **High** | No hazard analysis artifact in candidate |
| SF-08 partial | **Medium** | Test exists but lacks time-advance for positive path; gap clearly identified |
| Overall candidate readiness for integration | **Medium** | Code defects fixed; authority gaps and test gap remain; target evidence absent |

---

## 7. Sign-Off

**Reviewer:** Senior Aeronautics and Space Software Safety Engineer (Independent)  
**Date:** 2026-10-24  
**Disposition:** **CONDITIONAL ACCEPTANCE FOR DEVELOPMENT CONTINUATION** — Code defects fixed; authority escalations and test gap remain open; no release/flight claim permitted.

**Next Review:** After ESC-HK-01..05 resolution and positive emission test added.

---

*This review records objective evidence assessment only. It does not certify compliance, approve safety cases, accept residual risk, authorize release, or declare flightworthiness. Independent qualified humans retain those decisions.*