# Independent Senior SW Safety Engineer Review Report
## HK Periodic Emission Implementation (NC-EXT-17)

**Date:** 2026-10-24
**Reviewer:** Senior Aeronautics and Space Software Safety Engineer (independent context)
**Scope:** Safety assessment of the HK periodic emission feature per the changed candidate files
**Files under review (verified SHA256):**
- `src/pus/pus_services.c`: 360937e04e0fced837b39d633beb785bc9a575cbad7de49876eef9fea2491f7d
- `src/pus/pus.h`: e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd
- `tests/unit/test_pus.c`: 8b92cd845235723644904205d7a79d1582bad553440be0663d2bca9b67e091c3
- `tests/unit/test_pus_fu.c`: 9da8ea819a132e0ad647c79d9f03ad9d971025d2409b28afbe40074083034cb2
- `tests/unit/test_r2_gap.c`: beea686fc98b1844bfa463dcacfdbf888eb63fa38c27f105716ddc14b9bb25ad
- `tests/unit/test_r3_gap.c`: 5425a3feb512345b2d6e59578b2143e8356e22cdcec0e137c146aa674ff0e0e6

**Applicable norms:** N1-N22 (AGENTS.md), ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C)
**Related reviews:** FRICK-REVIEW-0386 (docs), FRICK-REVIEW-0406 (PA F-01..F-18), FRICK-REVIEW-0418 (PA final delta), V&V review 2026-10-24-hk-periodic-vv-review.md
**Constraints:** N18 Renode FAIL (NC-EXT-198) — no target evidence; NC-PAL-01 OPEN — no hardware watchdog binding; No ECSS compliance claim for private wire format; No NC closure or commit/push; Escalate exact missing authority inputs

---

## Executive Summary

**Overall Safety Disposition: FAIL — Critical safety defects and missing hazard analysis prevent any safety acceptance.** The periodic HK emission feature, if ever activated, introduces multiple credible hazard pathways: buffer over-read causing undefined period values, immediate/unscheduled emissions due to initialization defect, unbounded TM bandwidth consumption, silent data loss, and an unauthorized wire protocol. No hazard analysis exists. No safety case supports this feature. The feature is dead code (no caller) but the latent defects would manifest if a scheduler/timer integration is added later without correcting these findings first.

**Gate status after this review:** All affected safety gates remain RED. No commit/push authorized. No safety argument can be constructed from the current evidence.

---

## Safety Findings

### SF-01: Major — TC[3,1] Buffer Over-Read Causing Uncontrolled Period Value

**Hazard:** H_SW_UNDEF_PERIOD — HK report created with a period derived from uninitialized stack memory, leading to unpredictable emission timing (immediate, extremely frequent, or decades-long).

**Evidence:**
- ICD §4 (FRICK-ICD-001 rev 1.5, line 109): TC[3,1] payload = `report id[1] + provider id[1]` (2 bytes).
- Implementation (`pus_services.c:214-219`): Checks `len < 3u`, then reads `uint32_t period_s = rd_u32(&d[2])` (4 bytes at offsets 2,3,4,5).
- Test (`test_pus.c:405`): Sends `uint8_t payload[3] = {1u, 1u, 10u}` (3 bytes only).
- Result: `rd_u32(&d[2])` reads `d[2]=0x10`, `d[3..5]=uninitialized stack` → `period_s` = garbage (e.g., `0x10000000` ≈ 8.5 years).

**Safety consequence:** A ground operator sending a nominal 2-byte create command (per ICD) would trigger a buffer over-read. The resulting period is undefined — could be 0 (continuous emission, TM bandwidth saturation), 1 (1 Hz continuous), or millions of seconds (effectively disabled). The operator has no feedback that the command was misinterpreted. The TM[3,25] emission rate becomes a safety-critical uncontrolled variable.

**Root cause:** Implementation extended the TC[3,1] payload without updating the ICD, obtaining interface authority approval, or adding a length check for the extended format. The test was written against the extended format but with insufficient bytes, masking the defect.

**Required safety action:**
1. Immediate: Add a hard length check `if (len != 6u) return FF_BAD_SIZE;` in `svc3_create` for the extended format, OR revert to 2-byte payload and move period to a separate command.
2. Authority decision: Interface authority must approve or reject the 6-byte TC[3,1] payload. The ICD explicitly states it is "not an approved mission tailoring baseline" (§1, lines 13-14) and "every service/subservice row... is a provisional framework default... not an approved mission PUS profile" (§4 provisional note, lines 138-151).
3. Do not claim ECSS authorizes this private wire format. ECSS-E-ST-70-41C requires an approved ICD; this framework document is not that ICD.

---

### SF-02: Major — `svc3_create` Initializes `last_emission_time = 0` with `enabled = true`

**Hazard:** H_SW_IMMEDIATE_EMISSION — First periodic emission fires at `now >= period_s` since epoch (boot), not after one full period from enable. For a 10 s period, the first emission occurs at ~10 s uptime regardless of when the report was created/enabled.

**Evidence:**
- `pus_services.c:225-234` (`svc3_create`): `enabled = true; last_emission_coarse = 0; last_emission_fine = 0;`
- `pus_services.c:262-267` (`svc3_enable`): Correctly sets `last_emission_time = now` via `FF_TimeGetCuc`.
- `pus_services.c:394-399` (`FF_PusEmitPeriodicHk`): `due = last_emission + period_s`; emits when `now >= due`.

**Scenario:** Report created at boot with `period_s = 10`. At `t=11 s` uptime, `FF_PusEmitPeriodicHk()` emits immediately (since `due = 0 + 10 = 10`). The operator expects first emission at `t = create_time + 10 s`, but gets it at `t = 10 s` since boot. If the report is created at `t = 1000 s` with `period_s = 10`, `due = 10` is already in the past — emission fires on the very next scheduler tick.

**Safety consequence:** Emission timing is not controlled by the operator's enable command. Ground timeline correlation is broken. For safety-critical HK (e.g., thermal, power, propulsion parameters), the first emission may occur before the subsystem is initialized, reporting invalid data as valid. The "enable" command (TC[3,5]) is the only way to get correct phase — but create already enables.

**Required safety action:** `svc3_create` must initialize `last_emission_time = now` when `period_s > 0`, consistent with `svc3_enable`. Alternatively, create with `enabled = false` and require explicit TC[3,5] (aligning with ECSS where enable is a separate command). The current hybrid is inconsistent and unsafe.

---

### SF-03: Major — No Caller of `FF_PusEmitPeriodicHk()` Exists (Dead Code with Latent Defects)

**Hazard:** H_SW_DEAD_CODE_LATENT — The periodic emission function is implemented but never called. If a future integration adds a caller (scheduler, timer, task) without fixing SF-01, SF-02, SF-04, the latent defects activate.

**Evidence:**
- `grep -r "FF_PusEmitPeriodicHk" src/` returns only declaration (`pus.h:83`) and definition (`pus_services.c:376`).
- `src/apps/app_scheduler.c`, `src/core/sched/ff_sched.c`, `src/main/fsw_main.c` — no reference.
- No RTEMS timer, POSIX timer, or cyclic task configured to invoke it.

**Safety consequence:** The feature is currently inert (dead code). However, the project roadmap (M10/M11/M15, scheduler integration, NC-EXT-17 authority disposition) implies a future integration. The V&V review (2026-10-24-hk-periodic-vv-review.md, V&V-FIND-002) classifies this as "system-level integration decision, not a unit-test fix." Safety requires that **before any integration**, the latent defects are resolved and a hazard analysis is performed.

**Required safety action:**
1. Document the integration intent and interface contract (caller, rate, priority, stack, memory protection).
2. Perform a hazard analysis (see SF-07) before designing the integration.
3. Gate any integration behind resolution of SF-01, SF-02, SF-04, SF-05, SF-07.

---

### SF-04: Major — `time_add_period` Ignores Fine Time and Has No Coarse Wrap Handling

**Hazard:** H_SW_TIME_DRIFT_WRAP — Phase drift up to 1 second per period; incorrect due-time comparison after 32-bit coarse wrap (~136 years, but also if `period_s` is large or time is set to a future epoch).

**Evidence:**
- `pus_services.c:363-369`: `time_add_period` adds `period_s` to `coarse` only; `fine` unchanged.
- `time_ge` (lines 355-361): compares coarse then fine. After wrap, `coarse` becomes small → `time_ge(now, due)` returns false until `now` also wraps.
- No error return or saturation.

**Safety consequence:**
- **Drift:** For a 1 Hz report, each period loses the `fine` fraction. After N periods, phase drifts by up to N seconds. Ground correlation degrades.
- **Wrap:** If mission uses a non-zero epoch (e.g., GPS time ~1.3B seconds), `coarse + period_s` wraps at `2^32`. A report with `period_s = 1` created near wrap boundary will have `due_coarse` wrap to a small value. `time_ge(now, due)` will be false for ~136 years. The report stops emitting permanently.
- No test exercises non-zero `fine` or wrap conditions.

**Required safety action:** Implement proper 48-bit time addition with carry, or formally document and verify that `fine` is always 0 at emission instants and wrap is not credible within mission duration (with analysis). Add static assertions.

---

### SF-05: Major — HK Periodic Emission Is a Software-Only Time Source Feeding Ground-Visible TM

**Hazard:** H_SW_GROUND_MISINTERPRETATION — Wrong period → ground misinterprets spacecraft state; missing emission → silent data loss; no independent monitor.

**Evidence:**
- `FF_PusEmitPeriodicHk` is a pure software function driven by `FF_TimeGetCuc` (software time base).
- TM[3,25] is emitted directly to the TM sink → ground.
- No heartbeat, no sequence count in HK reports, no ground-side timeout monitor implemented.
- `FF_PusSendTm` failure in the emission loop (line 431-436) continues to next report but does not signal ground of the loss.

**Safety consequence:**
- **Wrong period (SF-01, SF-02):** Ground sees emissions at unexpected rates. A 1 Hz report emitting at 10 Hz saturates TM bandwidth; a 1 Hz report emitting once per day looks like a frozen subsystem.
- **Silent loss:** If `FF_PusSendTm` fails (queue full, link down, CRC error), the report is skipped, `last_emission_time` is **not updated** (line 440-441 only on success), so the next call retries — but ground has no indication of the gap. If the failure is persistent, the report stops emitting silently.
- **No independent monitor:** The watchdog (NC-PAL-01 OPEN) does not monitor HK emission. The health monitoring app (`app_hs.c`) checks application heartbeats, not periodic HK reports.

**Required safety action:**
1. Add a sequence counter or timestamp echo in each HK report (TM[3,25] payload) so ground can detect gaps/duplicates.
2. Define a ground-side monitor requirement (gap detection, alerting).
3. On `FF_PusSendTm` failure, emit an EVS event (service 5) so the loss is visible in the event stream.
4. Consider a "stale data" indicator if emission is overdue by > 2x period.

---

### SF-06: Major — ICD §4 Does NOT Authorize 6-Byte TC[3,1]; Private Extension Without Authority

**Hazard:** H_SW_UNAUTHORIZED_INTERFACE — Flight software accepts a wire format not approved by the interface authority, creating configuration management and interoperability risk.

**Evidence:**
- ICD §4 line 109: TC[3,1] = `report id[1] + provider id[1]` (2 bytes).
- Implementation requires 6 bytes (2 + 4 for `period_s`).
- ICD §1 lines 13-14: "This ICD documents the implemented... interface as an internal reference profile... **not an approved mission tailoring baseline**; applicable profile values and acceptance rules require approval by the authorized mission/customer authority."
- ICD §4 provisional note lines 138-151: "Every service/subservice row... is a provisional framework default... **not an approved mission PUS profile**... A mission profile/ICD authority must accept or replace these values before any operational use."
- ADR-0020 (provisional PUS and scheduler policies) is cited but does not document the TC[3,1] payload extension.

**Safety consequence:** The flight software behaves differently from the documented interface. A ground system built to the ICD will send 2-byte commands; the flight software will read garbage for `period_s`. The mismatch will only be discovered in integration test or operations. This violates configuration management and interface control principles essential for safety.

**Required safety action:** Escalate to the PUS Interface Authority for disposition. Three resolution paths:
1. Authority approves 6-byte TC[3,1] payload → update ICD, fix length check to `len == 6`, fix tests.
2. Authority rejects extension → revert `svc3_create` to 2-byte payload; move `period_s` to a separate subservice (e.g., TC[3,2] or extended create) with authority approval.
3. Authority defers → record as controlled NC with explicit residual risk acceptance by authorized human. **Do not proceed with integration until dispositioned.**

---

### SF-07: Major — No Hazard Analysis for Periodic HK Emission

**Hazard:** H_SW_NO_HAZARD_ANALYSIS — Safety-critical periodic emission feature has no hazard analysis covering rate, priority inversion, TM bandwidth saturation, event queue overflow, common-cause failures.

**Evidence:**
- No safety analysis document references periodic HK emission.
- No FMEA/FMECA entry for `FF_PusEmitPeriodicHk`.
- No assessment of:
  - **Rate:** Maximum aggregate HK emission rate across all reports (16 reports × min period). No rate limiting in emission loop.
  - **Priority inversion:** `FF_PusEmitPeriodicHk` calls provider callbacks (unbounded execution time) and `FF_PusSendTm` (may block on TM queue). If called from a high-priority timer, it can starve lower-priority tasks or be starved.
  - **TM bandwidth saturation:** Unlimited reports × high rate can fill TM queue/bandwidth, delaying critical TC acceptance TM or event reports.
  - **Event queue overflow:** Provider failures or `FF_PusSendTm` failures generate no events currently, but if added (per SF-05), could flood EVS.
  - **Common cause:** Single `FF_TimeGetCuc` call for all reports — if time base fails, all periodic emissions fail together.
  - **Scheduler interaction:** No analysis of jitter, missed ticks, or scheduler overload.

**Safety consequence:** The feature cannot be assessed for safety. Per ECSS-Q-ST-80C Rev.2 and ECSS-E-ST-40C Rev.1, a safety-critical software function (HK feeds ground situational awareness) requires a hazard analysis before implementation and integration. The absence of analysis is itself a safety finding.

**Required safety action:** Commission a hazard analysis (PHA/FMEA) covering:
1. Functional hazard assessment: incorrect period, missed emission, extra emission, wrong data, delayed emission.
2. System-level effects: TM bandwidth, CPU load, priority inversion, queue overflow, ground misinterpretation.
3. Mitigation requirements: rate limiting, priority assignment, watchdog monitoring, ground gap detection, sequence integrity.
4. Interface with system safety (SSS-REQ-008 health monitoring, watchdog, FDIR).

**Do not integrate until hazard analysis is complete and mitigations are implemented and verified.**

---

### SF-08: Minor — Test Evidence Does Not Cover Safety-Critical Paths

**Hazard:** H_SW_UNVERIFIED_SAFETY_PATHS — Unit tests exercise only nominal/not-due paths; positive emission, error handling, time advance, and boundary conditions are untested.

**Evidence:**
- `test_hk_periodic_emission` (test_pus.c:391-511): Calls `FF_PusEmitPeriodicHk` once at `now ≈ 0`, asserts `FF_ERROR`. Never advances time, never verifies positive emission.
- `test_pus_hk_s3_lifecycle` (test_r3_gap.c:408-480): Exercises TC[3,27] one-shot and enable/disable via TC ingestion; **never calls `FF_PusEmitPeriodicHk`**.
- No tests for: time advance + emission, provider failure during periodic emission, `FF_PusSendTm` failure, wrap, fine time, disabled report not emitted, period_s=0 not emitted.

**Safety consequence:** The V&V review (V&V-FIND-001, 003, 004) identifies these as functional test gaps. From a safety perspective, **untested paths are assumed defective**. The positive emission path (the only path that produces ground-visible TM) has zero test coverage.

**Required safety action:** Safety-critical paths require requirement-based tests with explicit oracles (N22). Add tests for all emission outcomes before any integration.

---

## Escalation Items (Require Human Authority Decision)

| ID | Item | Receiving Authority | Status |
|----|------|---------------------|--------|
| ESC-HK-01 | TC[3,1] payload format: 2-byte (ICD) vs 6-byte (implementation). Approve 6-byte, revert to 2-byte + separate command, or accept residual risk. | PUS Interface Authority / Mission ICD Authority | **AWAITING OWNER** |
| ESC-HK-02 | Periodic HK emission integration: designate caller (timer/scheduler/task), rate, priority, stack, memory protection, watchdog monitoring. | Systems Engineering / Avionics Authority | **AWAITING OWNER** |
| ESC-HK-03 | Hazard analysis commissioning and acceptance for periodic HK emission (rate, bandwidth, priority inversion, silent loss, ground monitor). | System Safety Authority | **AWAITING OWNER** |
| ESC-HK-04 | Ground-side HK gap detection / sequence integrity requirement (TM[3,25] payload enhancement). | Ground Systems / Mission Operations Authority | **AWAITING OWNER** |
| ESC-HK-05 | Authorization to proceed with scheduler/timer integration of `FF_PusEmitPeriodicHk` given current latent defects (SF-01, SF-02, SF-04). | Authorized Release / Safety Authority | **AWAITING OWNER** |

**Note:** Per FRICK-REVIEW-0406 F-06 (NC-EXT-172), escalation records must contain recipient, date, question set, and response. No such record exists for the above items. The escalation register `docs/status/escalations.md` tracks NC-EXT-166..183; these HK items must be added there with the same discipline.

---

## Disposition Recommendation

| Item | Disposition | Rationale |
|------|-------------|-----------|
| **Periodic HK emission feature (as implemented)** | **REJECT** — Do not integrate, do not enable, do not test on target. | Multiple critical defects (SF-01, SF-02), no hazard analysis (SF-07), unauthorized interface (SF-06), dead code with latent defects (SF-03). |
| **TC[3,1] create handler** | **REWORK** — Fix buffer over-read (hard length check or revert payload) before any further test or integration. | SF-01 is a memory safety defect (CWE-125) with uncontrolled safety consequences. |
| **`svc3_create` initialization** | **REWORK** — Initialize `last_emission_time = now` when `period_s > 0`. | SF-02 breaks operator intent and ground timeline correlation. |
| **`time_add_period`** | **REWORK or DOCUMENT WITH ANALYSIS** — Proper 48-bit addition or verified wrap/drift analysis. | SF-04 is a latent defect; acceptable only with formal analysis for mission duration. |
| **Traceability** | **ADD** — Trace `FF_PusEmitPeriodicHk` to a safety-marked SRS/SSS requirement. | V&V-FIND-006: feature has no requirement parent. |
| **Tests** | **EXTEND** — Add positive emission, error, boundary, and time-advance tests per N22. | SF-08: untested safety paths. |
| **Hazard analysis** | **COMMISSION** — Before any integration design. | SF-07: mandatory per ECSS-Q-ST-80C for safety-critical functions. |

**No commit/push until:**
1. SF-01 and SF-02 are corrected in source.
2. ESC-HK-01 (TC[3,1] payload authority) is dispositioned and ICD updated.
3. ESC-HK-03 (hazard analysis) is commissioned with a plan and milestones.
4. All fixes undergo independent delta review per N7/N20 (this role + PA role).
5. `make build`, `make test`, `make verify-lean`, `make trace` all GREEN on the corrected tree.

---

## Evidence Ledger

All findings reference specific source lines, test lines, ICD clauses, and traceability entries as recorded above. Raw evidence (source files, test files, ICD, traceability.yaml, V&V review, PA reviews) is retained in the working tree per N11.

| Finding | Source Evidence | ICD/Requirement | Test Evidence |
|---------|----------------|-----------------|---------------|
| SF-01 | `pus_services.c:214-219`, `rd_u32` macro | ICD §4 line 109 (2 bytes) | `test_pus.c:405` (3 bytes sent) |
| SF-02 | `pus_services.c:225-234` vs `262-267` | ECSS-E-ST-70-41C enable command | `test_pus.c:403-437` (accidental pass) |
| SF-03 | `grep -r FF_PusEmitPeriodicHk src/` | — | No integration test exists |
| SF-04 | `pus_services.c:363-369`, `355-361` | — | No dedicated tests |
| SF-05 | `pus_services.c:376-446`, `app_hs.c` | SSS-REQ-008, SRS-REQ-008 | No ground monitor test |
| SF-06 | ICD §1 lines 13-14, §4 lines 109, 138-151 | ECSS-E-ST-70-41C tailoring | ADR-0020 does not cover this |
| SF-07 | No safety analysis document found | ECSS-Q-ST-80C, ECSS-E-ST-40C | — |
| SF-08 | `test_pus.c:391-511`, `test_r3_gap.c:408-480` | N22 (nominal + out-of-bound) | Positive path NOT RUN |

---

## Authority Statement

This review is an **AI analysis role** per the Senior Aeronautics and Space Software Product Assurance Engineer persona instructions. It has no human credentials, regulatory delegation, proprietary-standard entitlement, test-witness authority, independence by mere declaration, or power to approve qualification, acceptance, certification, compliance, anomaly closure, flight readiness, or **safety risk acceptance**. Safety-critical conclusions require qualified independent human review and the approvals defined by the governing baseline (ECSS, mission authority, regulatory).

The findings herein are **objective evidence of safety defects and gaps**. They do not constitute a safety case, a safety assessment report, or an authorization to proceed. The authorized human safety authority must review this evidence, commission the hazard analysis (ESC-HK-03), resolve the interface authority items (ESC-HK-01, ESC-HK-02, ESC-HK-04), and explicitly accept or reject the residual risks before any integration or flight use.

---

## History

| Revision | Date       | Author    | Change |
| -------- | ---------- | --------- | ------ |
| 1.0      | 2026-10-24 | frick-fsw | Initial safety review of HK periodic emission (NC-EXT-17); findings SF-01..SF-08, escalations ESC-HK-01..05 |

---

*End of Report*