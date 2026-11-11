# FRICK-REVIEW-0474-PA-20261105-SCHED-EVENTS

**Independent Product Assurance Review (Norm N20) — Corrected Scheduler Event Tests**

---

## 1. Review Identification

| Field | Value |
|-------|-------|
| **Review ID** | FRICK-REVIEW-0474 |
| **Type** | Independent Product Assurance (N20) |
| **Scope** | `tests/unit/test_sched_events.c` — 5 unit tests for NC-EXT-19/90 scheduler EVS events |
| **Test File SHA-256** | `335d6d46a9a9ee94c87b83108ce72f5e8b813cd62c9481f070addae8da6e54a8` |
| **Date** | 2026-11-05 |
| **Reviewer** | `aeronautics-space-sw-pa-engineer` (independent PA subagent) |
| **Independence** | No involvement in the test design, implementation, or prior reviews of this artifact. Separate context from engineering, V&V, and documentation agents. Reporting line: independent PA function per ECSS-Q-ST-80C Rev.2 §4.2 and project norm N9/N20. |
| **Baseline** | ECSS-Q-ST-80C Rev.2 (2025-04-30) [S1]; ECSS-E-ST-40C Rev.1 (2025-04-30) [S2]; project norms N1–N22 per `AGENTS.md`/`docs/rules.md` |
| **Applicable Requirements** | NC-EXT-19 (scheduler event emission), NC-EXT-90 (time-domain/exhaustion policy open), SRS-REQ-071 (RTEMS-backed schedule dispatch), SSS-REQ-030 (table-driven application scheduling) |

---

## 2. Independence Declaration

This review was performed by an independent PA context with:

- **No authorship or co-authorship** of `test_sched_events.c`, `app_scheduler.c`, or the scheduler EVS event design.
- **No prior review involvement** in FRICK-REVIEW-0452, FRICK-REVIEW-0453, FRICK-REVIEW-0455, or FRICK-REVIEW-0456/0457.
- **Separate tool context** from the engineering agent that corrected the tests and the V&V agent that executed the gate runs.
- **Authority to escalate** findings directly to the authorized human authority per N20/N15; this report does not close NCs, approve residual risk, or authorize release.

---

## 3. Verdict

| Verdict | Severity Counts |
|---------|-----------------|
| **CONDITIONAL PASS** — The 5 required NC-EXT-19/90 events are tested with explicit assertions. Traceability to SRS-REQ-071/SSS-REQ-030 is documented in the test file header. CTest increased from 53→54 as required. | **Critical: 0**  **Major: 2**  **Minor: 3**  **Observations: 2** |

**Conditions for unconditional PASS:**

1. **Major F-0474-01:** Test design tests *event emission mechanism* (`FF_EvsSend` direct invocation), not the *production emission paths* in `app_scheduler.c`. This is a unit-test-of-the-harness, not an end-to-end verification of the scheduler's actual EVS emission logic (NC-EXT-236 related gap).
2. **Major F-0474-02:** Event IDs and payload strings are duplicated in the test file (local `#define`s) rather than imported from `app_scheduler.c`. Divergence risk if production codes change without test update.

---

## 4. Findings Table

| ID | Severity | Category | Criterion (Source) | Observed Fact | Gap / Risk | Recommendation |
|----|----------|----------|-------------------|---------------|------------|----------------|
| F-0474-01 | Major | Test design / N22 | N22: requirement-based tests cover nominal and applicable out-of-bound cases against explicit expected results; test the *production* behaviour, not only the harness | Each test calls `FF_EvsSend(event_id, ...)` directly with hard-coded event IDs and payload strings, then asserts the listener captured them. The production emission paths in `app_scheduler.c:94-136` (activation, missed-slot detection, dispatch-fail detection, heartbeat timer) are **not exercised**. | The tests verify the EVS listener capture mechanism and string matching, not that `app_scheduler.c` emits the correct events under the correct conditions. This is a unit test of the *test harness*, not the *product*. NC-EXT-236 tracks the missing end-to-end EVS-to-HS chain test. | Add an integration test (or extend `test_sched.c`) that exercises `FF_AppSchedulerMain` / `FF_SchedPoll` with injected time/failures and asserts the actual EVS events emitted by the production code paths. |
| F-0474-02 | Major | Traceability / CM | ECSS-Q-ST-80C Rev.2 §5.3.2: configuration identification of test artifacts; bidirectional traceability | Event IDs (`0x0601`–`0x0605`) and payload format strings are **duplicated** in `test_sched_events.c:33-37` and `app_scheduler.c:35-39`. No shared header or single source of truth. | If production event IDs or payload formats change, the test will silently pass against stale expectations (false positive) or fail with misleading diagnostics. | Move event ID definitions and payload format strings to a shared header (e.g., `include/apps/app_scheduler_events.h`) included by both `app_scheduler.c` and `test_sched_events.c`. |
| F-0474-03 | Minor | Test completeness | N22: applicable out-of-bound cases covered | No test for: (a) multiple rapid missed-slot emissions (counter saturation), (b) heartbeat with non-default app ID, (c) event emission when EVS listener is not registered, (d) `FF_EvsSend` return value on buffer full. | Edge cases of the EVS emission path are not verified. The test only covers the nominal single-event capture path. | Add negative/robustness cases per N22: listener not registered, buffer saturation, counter wrap (if applicable). |
| F-0474-04 | Minor | Test determinism | N22: deterministic, no flakiness | Tests use `strstr` for payload matching on `ACTIVATED`, `MISSED_SLOT`, `INVALID_TABLE`, `DISPATCH_FAIL` but `strcmp` (exact match) on `HEARTBEAT`. Inconsistent oracle strategy. | `strstr` passes if the expected substring appears anywhere; `strcmp` requires exact match. Both are valid but the mix is inconsistent and could mask payload format drift on the `strstr` cases. | Standardize on exact-match `strcmp` where the full payload is deterministic (all 5 cases), or document the rationale for substring matching. |
| F-0474-05 | Minor | Evidence integrity | N11/N15: test scripts and review results retained; gate evidence traceable | CTest count increased 53→54 as reported, but **gate11 logs were overwritten** (per scope note). No archived evidence of the 53-test baseline run. | Cannot independently verify the 53→54 delta claim or confirm no other tests were added/removed concurrently. | Archive per-gate test logs with immutable names (timestamp + git SHA) per `docs/vv/verification-results.md` convention. |
| F-0474-06 | Observation | Requirements mapping | SRS-REQ-071/SSS-REQ-030 traceability | Test file header (lines 11-12) cites NC-EXT-19/90, SRS-REQ-071, SSS-REQ-030. Traceability.yaml lines 663-665 and 932 confirm the SRS/SSS linkage. | Adequate at the document level. | No action required. |
| F-0474-07 | Observation | Event severity alignment | `app_scheduler.c` event severity usage | Test assertions match production severity: ACTIVATED=INFO, MISSED_SLOT=WARNING, DISPATCH_FAIL=ERROR, HEARTBEAT=INFO, INVALID_TABLE=ERROR. | Consistent. | No action required. |

---

## 5. Requirement Coverage Matrix

| Requirement | Event ID | Event Name | Test Case | Production Emission Site | Coverage Verdict |
|-------------|----------|------------|-----------|--------------------------|------------------|
| NC-EXT-19 / SRS-REQ-071 / SSS-REQ-030 | 0x0601 | FF_SCHED_EVT_ACTIVATED | `test_sched_activated_event` | `app_scheduler.c:100-101` (after successful `FF_SchedActivate`) | **MECHANISM ONLY** — test invokes `FF_EvsSend` directly; production path not exercised |
| NC-EXT-19 / SRS-REQ-071 / SSS-REQ-030 | 0x0602 | FF_SCHED_EVT_INVALID_TABLE | `test_sched_invalid_table_event` | `app_scheduler.c:95-96` (failed `FF_SchedActivate`) | **MECHANISM ONLY** — test invokes `FF_EvsSend` directly; production path not exercised |
| NC-EXT-19 / SRS-REQ-071 / SSS-REQ-030 | 0x0603 | FF_SCHED_EVT_MISSED_SLOT | `test_sched_missed_slot_event` | `app_scheduler.c:117-121` (missed counter delta) | **MECHANISM ONLY** — test invokes `FF_EvsSend` directly; production path not exercised |
| NC-EXT-19 / SRS-REQ-071 / SSS-REQ-030 | 0x0604 | FF_SCHED_EVT_HEARTBEAT | `test_sched_heartbeat_event` | `app_scheduler.c:133-135` (heartbeat timer) | **MECHANISM ONLY** — test invokes `FF_EvsSend` directly; production path not exercised |
| NC-EXT-19 / SRS-REQ-071 / SSS-REQ-030 | 0x0605 | FF_SCHED_EVT_DISPATCH_FAIL | `test_sched_dispatch_fail_event` | `app_scheduler.c:123-128` (publish_errors counter delta) | **MECHANISM ONLY** — test invokes `FF_EvsSend` directly; production path not exercised |

**All 5 NC-EXT-19/90 events: TESTED AT HARNESS LEVEL (5/5).**  
**Production emission paths: NOT EXERCISED (0/5).**  
→ NC-EXT-19 remains OPEN pending independent review of complete chain (per `handover.md` rev 3.58, `loop-audit.md` line 41).

---

## 6. Gate Evidence Integrity

| Gate | Prior Count | Current Count | Delta | Evidence Integrity |
|------|-------------|---------------|-------|-------------------|
| `make test` (CTest) | 53 | 54 | +1 | **COMPROMISED** — gate11 logs overwritten; no archived baseline to verify the exact delta. The +1 is consistent with adding `test_sched_events` (5 tests) but other changes may have occurred concurrently. |

**N11/N15 Non-conformance:** Test evidence from the gate that established the 53→54 transition was not retained. This finding is recorded for disposition; it does not invalidate the current 54/54 PASS.

---

## 7. Confidence and Uncertainty

| Aspect | Confidence | Basis |
|--------|------------|-------|
| 5 events defined in production code | High | Direct inspection of `app_scheduler.c:35-39` and `test_sched_events.c:33-37` |
| 5 test cases exist and pass | High | CTest 54/54; test file read; `FF_TEST_MAIN` lists 5 cases |
| Tests verify production emission logic | Low | Tests call `FF_EvsSend` directly; `app_scheduler.c` emission paths not executed |
| Event ID/payload consistency | Medium | Manual comparison shows match at time of review; no shared header (F-0474-02) |
| Traceability to NC-EXT-19/90, SRS-REQ-071, SSS-REQ-030 | High | Test header, traceability.yaml, traceability.md all consistent |
| Gate evidence (53→54) | Low | Logs overwritten; delta claimed but not evidenced |

**Assumptions:** The current `app_scheduler.c` is the version that corresponds to the test. No unverified patches between review and commit.

**Expiry/Recheck:** Re-review required if `app_scheduler.c` event IDs, payloads, or emission conditions change; or if NC-EXT-236 end-to-end test is delivered.

---

## 8. Escalation and Human-Approval Boundaries

Per N20/N15, the following require **independent qualified human review** (this agent cannot approve):

- Closure of **NC-EXT-19** (scheduler event emission chain) — depends on F-0474-01 resolution and NC-EXT-236 end-to-end test.
- Acceptance of **F-0474-01** as a known limitation vs. requiring production-path test before sprint closure.
- Disposition of **F-0474-05** (gate evidence integrity) — whether overwritten logs are acceptable for this increment.
- **Residual risk acceptance** for duplicated event definitions (F-0474-02) if shared-header refactor is deferred.

---

## 9. Source Register (Verified 2026-11-05)

| ID | Source | Accessed | Notes |
|----|--------|----------|-------|
| S1 | ECSS-Q-ST-80C Rev.2 (2025-04-30) | 2026-10-11 | Public full text; normative space software PA standard |
| S2 | ECSS-E-ST-40C Rev.1 (2025-04-30) | 2026-10-11 | Public full text; space software engineering lifecycle |
| S3 | `tests/unit/test_sched_events.c` | 2026-11-05 | SHA-256 `335d6d46a9a9ee94c87b83108ce72f5e8b813cd62c9481f070addae8da6e54a8` |
| S4 | `src/apps/app_scheduler.c` | 2026-11-05 | Production event emission paths |
| S5 | `docs/requirements/traceability.yaml` | 2026-11-05 | SRS-REQ-071 (line 663), SSS-REQ-030 (line 932) |
| S6 | `docs/requirements/traceability.md` | 2026-11-05 | SRS-REQ-071 row (line 144), SSS-REQ-030 row (line 188) |
| S7 | `docs/status/handover.md` rev 3.58 | 2026-11-05 | NC-EXT-19 open, NC-EXT-236 tracked |
| S8 | `docs/status/loop-audit.md` | 2026-11-05 | M17 S8-SCH delivered, NC-EXT-19 open (line 41) |
| S9 | CTest output `make test` | 2026-11-05 | 54/54 PASS |

---

**End of Report — FRICK-REVIEW-0474-PA-20261105-SCHED-EVENTS**

*This report is evidence for the independent PA review record. It does not close NCs, approve compliance, or authorize release. Human authority required for all dispositions.*