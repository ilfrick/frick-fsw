Independent V&V Review: S8-VV-FU Residual F-0379-03 / NC-EXT-98 Reset-Path Oracle
=================================================================================

Date: 2026-10-30
Reviewer: Senior Aeronautics and Space Software V&V Engineer (independent sub-agent)

Scope: S8-VV-FU residual F-0379-03 / NC-EXT-98 reset-path oracle in ES/HS lifecycle test harness.

Context: Sprint plan S9 records "ES/HS lifecycle test harness already implemented; need to add specific reset-path test case per NC-EXT-98 disposition". NC-EXT-98 remains OPEN pending residual F-0379-03 and owner disposition (§3.2).

---

## FINDINGS

### 1. Original Finding F-0379-03 (2026-10-18)
**Original claim:** "reset path specified by N22-TS5-50 is unexecuted (no FF_PalPanic stub or invocation exists in tests/; concurrency exercised structurally only)"

**Current status:** **RESOLVED for test existence** — S8-FDIR-01 increment (2026-10-20/21) added `FF_TEST_HOOKS` panic observer seam and 9 reset-path tests in `test_es_hs_lifecycle.c` covering all explicit N22-TS5-50/80/83/86/87 subcases for the commanded reset path.

### 2. Current Reset-Path Test Coverage (9 tests)

| Test | Requirement Coverage | Status |
|------|---------------------|--------|
| `test_hs_reset_single_path` | N22-TS5-50 nominal (nominal commanded reset) | PASS |
| `test_hs_reset_unknown_function` | N22-TS5-50(b) unknown function | PASS |
| `test_hs_reset_missing_function_id` | N22-TS5-50(c) missing function ID | PASS |
| `test_hs_reset_survives_report_failure` | SRS-REQ-074 "failure of notification shall not prevent reset" | PASS |
| `test_hs_reset_survives_missing_notifier` | SRS-REQ-074 "absence of notification shall not prevent reset" | PASS |
| `test_panic_console_stall_and_report_share_one_budget` | N22-TS5-83 shared 250ms deadline | PASS |
| `test_panic_production_link_wiring_delivers_one_frame` | N22-TS5-87 production wiring | PASS |
| `test_link_deadline_writer_semantics` | N22-TS5-83 deadline writer semantics | PASS |
| `test_hs_reset_survives_report_failure` + `test_hs_reset_survives_missing_notifier` | SRS-REQ-074 "failure/absence shall not prevent reset" | PASS |

### 3. Remaining Verification Gaps (NC-EXT-98 correctly remains OPEN)

| Gap | Description | Linked NC | Severity |
|-----|-------------|-----------|----------|
| G-1 | **Report-before-reset ordering not test-observable** — simulated backend returns from reset; oracle sees only final state. No test-level sensor reads reset counter inside notifier. | NC-EXT-132 | Major |
| G-2 | **Lean theorem `req_srs074_report_precedes_reset` is definitional only** — `decide` over `[report, reset]` literal; does not refine to C `pal_posix.c` ordering. | NC-EXT-167 | Major |
| G-3 | **Mandatory-text envelope unresolved** — >96 chars suppresses notifier while still requesting reset. Violates SRS-REQ-074 "shall not be truncated". Authority decision pending. | FRICK-REVIEW-0411/0413/0414, ADR-0021 | Major |
| G-4 | **No concurrency/interleaving assertions** — tests run sequentially; isolation via timing-based delay. F-0379-03 part 2 unaddressed. | F-0379-03 residual | Minor |
| G-5 | **Watchdog feed-failure → panic path not tested** with pre-reset oracle. Same `FF_PalPanic` but different entry point (`FF_AppWdMain` feed failure). | Sprint S9 "specific reset-path test case" | Minor |
| G-6 | 250 ms deadline is host-only observation; does not establish target WCET, link backend time, or RTEMS timing. | ADR-0021, N22-TS5-83 | Minor (scope limit) |
| G-7 | Hardware watchdog binding not wired (NC-PAL-01); target/HIL reset path, independence, and report-before-reset on real link remain gaps. | NC-PAL-01, GAP-A5, N22-TS5-82 | Major (target) |

### 4. Sprint S9 Disposition Status
- Sprint plan S9: "need to add specific reset-path test case per NC-EXT-98 disposition"
- "Owner disposition §3.2 pending" — **disposition not recorded**
- "Owner disposition (§3.2 only)" — disposition not recorded

---

## NC-EXT-98 STATUS: REMAINS OPEN

**Rationale:** The original "reset path unexecuted" finding is resolved (9 tests exist), but verification NCs on the NEW tests are open (NC-EXT-132, NC-EXT-167), concurrency gap persists, mandatory-text envelope unresolved, owner disposition §3.2 not recorded, and sprint S9 explicitly states "need to add specific reset-path test case per NC-EXT-98 disposition."

**Conditions for closure (all must be met):**
1. NC-EXT-132 resolved (test-level ordering sensor added OR inspection+Lean explicitly accepted by authority)
2. NC-EXT-167 resolved (Lean theorem scope documented, not attributed as C-code evidence)
3. Concurrency assertion gap addressed OR explicitly accepted as host-only limitation with owner sign-off
4. Mandatory-text envelope: authority decision recorded
5. Sprint S9 "specific reset-path test case" identified, implemented, and independently reviewed
6. Owner disposition (§3.2) recorded closing the NC

---

## RECOMMENDATIONS

| # | Recommendation | Decision Owner | Prerequisites |
|---|----------------|----------------|---------------|
| R1 | **Do NOT close NC-EXT-98** — verification NCs on the new tests are open, owner disposition pending | FSW / V&V / Owner | All closure conditions met |
| R2 | **Add report-before-reset sensor test** — extend `test_hs_reset_single_path` to read reset counter inside notifier (like `test_panic_console_stall_and_report_share_one_budget` does with `g_observed_resets_at_notifier`) | FSW / V&V | Owner approval for test seam change; closes NC-EXT-132 |
| R3 | **Add watchdog feed-failure reset-path test** — exercise `FF_AppWdMain` feed failure → `FF_PalPanic` → pre-reset report with same oracle depth as commanded reset | FSW / V&V | `FF_TEST_HOOKS` supports this; closes sprint S9 "specific reset-path test case" |
| R4 | **Document Lean theorem scope explicitly** in `traceability.yaml` and TS-5: `req_srs074_report_precedes_reset` is a model-level ordering constraint only | V&V / PA | Update traceability.yaml lean_note; TS-5 case N22-TS5-80 evidence column |
| R5 | **Request authority decision on mandatory-text envelope** (SRS-REQ-074 "not truncated") — current >96 chars suppresses notifier | Systems / Owner | ADR-0021 § "mandatory-text envelope... remains authority decisions" |
| R6 | **Record owner disposition §3.2** for NC-EXT-98, identifying the "specific reset-path test case" in sprint S9 | Owner | Owner availability |

---

## NC-EXT-98 STATUS: **REMAINS OPEN**

**Verdict:** The original "reset path unexecuted" finding is resolved (9 tests exist). However, verification NCs on the new tests (NC-EXT-132, NC-EXT-167), concurrency gap, mandatory-text envelope, owner disposition §3.2 pending, and sprint S9 explicit "need to add specific reset-path test case" mean NC-EXT-98 correctly remains OPEN as a verification-evidence NC.

---

**Reviewer:** Senior Aeronautics and Space Software V&V Engineer (independent sub-agent)  
**Date:** 2026-10-30  
**Authority:** This review provides evidence and findings only. It does not approve baselines, close NCs, waive gates, or authorize commit/push.
