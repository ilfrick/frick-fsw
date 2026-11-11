# FRICK-REVIEW-N22-TS5-66-R4-2026-11-10

## Independent Documentation Review — N22-TS5-66 Delta (TBL load/activate → scheduler dispatch coupling)

| Field | Value |
|-------|-------|
| **Review ID** | FRICK-REVIEW-N22-TS5-66-R4-2026-11-10 |
| **Date** | 2026-11-10 |
| **Reviewer role** | R4 — Senior Aeronautics and Space Software Documentation Engineer (independent context) |
| **Scope** | Read-only independent review of documentation completeness, accuracy, and consistency for the N22-TS5-66 delta (Table Services activation callback coupling to scheduler dispatcher) |
| **Norms invoked** | N8 (docs updated with behavior change), N17 (documentation review on every change), N4 (bidirectional traceability), N22 (test design: nominal + OOB with explicit oracles) |
| **Artifacts reviewed** | See §1 below |

---

## 1. Documentation Artifacts Reviewed (with versions)

| Artifact | Document ID | Revision / Date | Status |
|----------|-------------|-----------------|--------|
| Architecture — Application Scheduling | FRICK-ARCH-0018 | Rev 0.5 / 2026-11-10 | Reviewed |
| Unit test — TBL/Scheduler coupling | — | `tests/unit/test_tbl_sched_coupling.c` (as of working tree) | Reviewed |
| Test Specification TS-5 — Core Services | FRICK-VV-TSPEC-TS5 | Rev 1.15 / 2026-11-10 | Reviewed |
| Traceability Manifest | — | `docs/requirements/traceability.yaml` (working tree) | Reviewed |
| Loop Audit (N19) | FRICK-AUDIT-001 | Rev 10.46 / 2026-11-10 | Reviewed |
| Sprint Plan | FRICK-PLAN-0002 | Rev 1.129 / 2026-11-10 | Reviewed |
| CHANGELOG | — | Entry 3.66/3.67 (2026-11-10) | Reviewed |
| Scheduler Application Source | — | `src/apps/app_scheduler.c` (working tree) | Reviewed for implementation↔doc consistency |

---

## 2. Executive Summary

The N22-TS5-66 delta delivers the TBL load/activate → scheduler dispatch coupling: the scheduler application (`app_scheduler.c`) registers an `FF_TblActivateCb` callback (`ff_sched_tbl_activated`) that snapshots the validated, activated schedule image into a mutex-protected pending slot; the scheduler task applies it on its next tick via `ff_sched_apply_pending` (calling `FF_SchedActivate` + `FF_SchedInit`), preserving the single-writer dispatcher contract with a bounded one-tick deferral. Five test cases in `test_tbl_sched_coupling.c` cover the TBL hook contract (tests 1–3) and production wiring (tests 4–5) with nominal and out-of-bound cases.

**Overall disposition: APPROVED WITH COMMENTS**

The documentation set accurately reflects the implementation, test oracles are grounded in requirements/architecture clauses, traceability links are bidirectional and complete for the delta, status documents correctly record open items (deferral budget, M14, mission rates, target/HIL), and CHANGELOG entries follow the project format. The Minor and Observation findings below do not block the delta but should be addressed in the next documentation cycle.

---

## 3. Findings

### 3.1 Major Findings
*None.*

---

### 3.2 Minor Findings

#### MIN-1: Architecture document (scheduling.md) §5.1 — "TBL refuses the load" claim not fully accurate
**File:** `docs/architecture/scheduling.md`  
**Lines:** 164–166 (Rev 0.5)  
**Text:** *"An invalid image never reaches the dispatcher: TBL refuses the load, activation returns `FF_NOT_READY`, and the previously active table keeps running."*

**Evidence from implementation:**
- `test_tbl_sched_coupling.c` test 3 (`test_tbl_sched_coupling_invalid`, lines 219–242) shows: `FF_TblLoad` with a bad-magic image returns `FF_ERROR` (validator rejection), and subsequent `FF_TblActivate` returns `FF_NOT_READY`.
- `test_tbl_sched_coupling.c` test 5 (`test_tbl_sched_production_invalid`, lines 367–401) shows: `FF_TblLoad` with wrong length returns `FF_BAD_SIZE`; `FF_TblLoad` with bad magic returns `FF_ERROR`; `FF_TblActivate` returns `FF_NOT_READY`.
- The architecture text says "TBL refuses the load" — but the load *succeeds* for bad magic (returns `FF_SUCCESS` for staging), and validation fails *during* load (returning `FF_ERROR`). The staging is invalidated, so activate returns `FF_NOT_READY`. The phrasing "TBL refuses the load" could be read as the load call itself being rejected, which is only true for size mismatches (`FF_BAD_SIZE`), not for validator rejections.

**Recommendation:** Clarify: *"An invalid image never reaches the dispatcher: `FF_TblLoad` either rejects the wrong size (`FF_BAD_SIZE`) or accepts the staging but the validator rejects it (`FF_ERROR`), so `FF_TblActivate` returns `FF_NOT_READY` and the previously active table keeps running."*

---

#### MIN-2: Test specification (ts-5-core-services.md) N22-TS5-66 — Test case references do not match test function names exactly
**File:** `docs/vv/test-spec/ts-5-core-services.md`  
**Lines:** 196 (Rev 1.15), Evidence column for N22-TS5-66  
**Text:** *"Evidence: `tests/unit/test_tbl_sched_coupling.c::test_tbl_sched_production_activate` / `test_tbl_sched_production_invalid`"*

**Evidence from test file:**
- Actual function names (lines 320, 367): `test_tbl_sched_production_activate` and `test_tbl_sched_production_invalid` — **these match**.
- However, the test file's `FF_TEST_MAIN` (lines 410–412) lists the functions in order: `test_tbl_sched_coupling_basic`, `test_tbl_sched_coupling_revert`, `test_tbl_sched_coupling_invalid`, `test_tbl_sched_production_activate`, `test_tbl_sched_production_invalid`.
- The test spec also references "production `FF_AppSchedulerMain` under the `FF_TEST_HOOKS` controlled-delay seam, EVS-listener oracle" — this is accurate.

**Note:** The function names *do* match. This finding is withdrawn upon re-verification.  
**Correction:** No action needed; the reference is accurate.

---

#### MIN-3: Traceability.yaml — SRS-REQ-013 `tests` array missing `test_tbl_sched_coupling.c`
**File:** `docs/requirements/traceability.yaml`  
**Lines:** 145–152 (SRS-REQ-013 entry)  
**Current `tests`:** `["tests/unit/test_tbl.c", "tests/unit/test_r2_gap.c", "tests/unit/test_r3_gap.c", "tests/unit/test_tbl_sched_coupling.c"]`  
**Wait — the file shows it IS included** (line 150). This finding is withdrawn.

---

#### MIN-4: Traceability.yaml — SRS-REQ-071 `lean_note` does not mention the one-tick deferral budget as open
**File:** `docs/requirements/traceability.yaml`  
**Lines:** 663–670 (SRS-REQ-071 entry)  
**Current `lean_note`:** *"Bounded allocation-free table-driven dispatcher delivered (S8-SCH first increment): ... Table Services load/activate is coupled to dispatch (N22-TS5-66): the app's activation callback snapshots the activated image and the scheduler task applies it on its next tick, host-verified with nominal and out-of-bound cases (tests/unit/test_tbl_sched_coupling.c). RTEMS task selection/preemption remains out of scope; mission rates, jitter limits, late-slot policy and recovery authority remain TBD controlled inputs, so no schedulability, jitter or WCET claim is made and no target evidence is implied."*

**Gap:** The `lean_note` mentions the coupling but does not explicitly call out the **one-tick deferral budget** (WCET/latency of the callback-to-apply path) as open, whereas the architecture document (§5.1, lines 172–176) and test spec (N22-TS5-66 scope limit) both identify it as open. The loop-audit (Rev 10.46, line 41) and sprint-plan (Rev 1.129, S8-SCH row) also flag it.

**Recommendation:** Add to `lean_note`: *"The activation callback snapshots the image and the scheduler task applies it on its next tick, introducing a bounded one-tick deferral whose WCET/latency budget, cross-core memory ordering (M14), and mission rates remain open (NC-EXT-19)."*

---

#### MIN-5: Sprint Plan (sprint-plan.md) S8-SCH row — "Latest `make ci-local` passes 43/43 plus Lean/trace (historical, at the 2026-10-30 increment; the current candidate gate is 52/52 as of Rev 1.121 and is still 52/52 at Rev 1.125)"
**File:** `docs/planning/sprint-plan.md`  
**Lines:** S8-SCH row, Evidence Status column (Rev 1.129)  
**Issue:** The text references historical gate counts (43/43, 52/52) from prior revisions (1.121, 1.125) but the current revision is 1.129. This creates ambiguity about the *current* gate status. The CHANGELOG entry 3.66 (2026-11-10) states: *"Host build clean (no warnings), CTest 55/55 PASS, focused scheduler/TBL binaries 50/50 executions over 25 loops, trace PASS (120/76/30/14/68), `lake build` PASS, pinned clang-format 18.1.3 PASS, RTEMS 6.2 cross-build PASS (N18 profile) and **N18 Renode PASS**"*.

**Recommendation:** Update the Evidence Status to reflect the current (2026-11-10) gate results: *"Latest `make ci-local` passes 55/55 CTest, trace PASS (120/76/30/14/68), Lean PASS, format PASS, RTEMS cross-build PASS, N18 Renode PASS (SCH app started, 4 entries)."*

---

#### MIN-6: Loop Audit (loop-audit.md) Rev 10.46 — M17 row references "emulated N18 PASS 2026-11-10" but the N18 run evidence is for the WDOG0 binding (2026-11-09), not the N22-TS5-66 delta
**File:** `docs/status/loop-audit.md`  
**Lines:** 41 (M17 row, Actual implementation column)  
**Text:** *"**TBL load/activate → dispatch coupling (`N22-TS5-66`) delivered 2026-11-10:** the application's `FF_TblActivateCb` re-validates and snapshots the activated image and the scheduler task applies it (`FF_SchedActivate` + `FF_SchedInit`) on its next tick, host-tested with nominal and out-of-bound cases (`test_tbl_sched_coupling.c`, 61 checks); the one-tick deferral budget, mission rates (`N22-TS5-48`) and M14 memory ordering remain open | NC-EXT-19 | tracked: S8-SCH delivered (committed f0e1083); `N22-TS5-66` closed as a test gap; NC-EXT-19/90 open pending independent review and authority inputs; emulated N18 PASS 2026-11-10 (`SCH app started, 4 entries`), target/HIL and timing evidence still GAP-A5"*

**Issue:** The "emulated N18 PASS 2026-11-10 (`SCH app started, 4 entries`)" is recorded in the CHANGELOG entry 3.66 (2026-11-10) as part of the N22-TS5-66 delta verification. However, the loop-audit Rev 10.45 (2026-11-10, line 48 in history) and Rev 10.46 (2026-11-10, line 8) both have the same date. The N18 PASS for the WDOG0 binding was 2026-11-09 (CHANGELOG entry 3.65). The scheduler app starts in both runs. The text is not incorrect but could be clearer that the 2026-11-10 N18 run includes the scheduler coupling.

**Recommendation:** Clarify: *"emulated N18 PASS 2026-11-10 (includes `N22-TS5-66` delta; console shows `SCH app started, 4 entries`)"*

---

#### MIN-7: Architecture document (scheduling.md) Rev 0.5 history table — Rev 0.4 and 0.5 both dated 2026-11-10
**File:** `docs/architecture/scheduling.md`  
**Lines:** 210–216 (History table)  
**Rows:**
- 0.4 | 2026-11-10 | frick-fsw | Record the delivered Table Services coupling (`N22-TS5-66`)...
- 0.5 | 2026-11-10 | frick-fsw | Correct the sprint-closure paragraph...

**Issue:** Two revisions on the same date without a sequence indicator (e.g., 0.4a, 0.4b, or time-of-day). The project norm N16 expects detailed revision history; same-date multiple revisions should be disambiguated.

**Recommendation:** Use `2026-11-10a` / `2026-11-10b` or append time (e.g., `2026-11-10 14:30 UTC`) per project convention.

---

#### MIN-8: Test specification (ts-5-core-services.md) Rev 1.15 — §3.2 coverage summary says "60 covered / 27 gap" but §3 gap mapping lists N22-TS5-66 as "closed as a test gap (2026-11-10)" under NC-EXT-47 §7.3
**File:** `docs/vv/test-spec/ts-5-core-services.md`  
**Lines:** 245–250 (Gap mapping) and 397–398 (Coverage summary)  
**Gap mapping text:** *"N22-TS5-66 — **closed as a test gap (2026-11-10):** the TBL load/activate → dispatch coupling is delivered and host-tested... the bounded one-tick deferral, M14 memory ordering and mission rates remain open under `N22-TS5-48`/NC-EXT-19, which is not closed here."*  
**Coverage summary:** *"`covered` (oracle grounded in a requirement/architecture/interface clause) | 60"* and *"`gap` (no controlled oracle; recorded under NC-EXT-47) | 27"*

**Issue:** The gap mapping says "closed as a test gap" (meaning the test gap is closed, i.e., the case is now covered), but it appears under "NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause" which is the section for `gap` cases. This is confusing. The case N22-TS5-66 *is* now `covered` (per the case table row at line 196, Status = `covered`), so it should not be listed under the gap mapping at all, or should be listed with a clarifying note that it moved from `gap` to `covered`.

**Recommendation:** Remove N22-TS5-66 from the §3 gap mapping list, or move it to a "Residual limits attached to rows that are now `covered`" section (like the paragraph at lines 262–271 for N22-TS5-45/46/77), since the deferral budget/M14/mission rates remain open but the case itself is `covered`.

---

---

### 3.3 Observations

#### OBS-1: Terminology consistency — "deferral" vs "deferred activation" vs "one-tick deferral"
- Architecture (scheduling.md §5.1, line 162): *"bounded one-tick deferral"*
- Architecture (scheduling.md §5.1, line 175): *"the worst-case deferral budget of the one-tick activation apply"*
- Test spec (N22-TS5-66 scope limit): *"bounded deferral of at most one tick whose WCET/latency... remain open"*
- Sprint plan (S8-SCH row): *"the one-tick deferral budget"*
- Loop audit (M17 row): *"the one-tick deferral budget"*
- CHANGELOG 3.66: *"preserving the single-writer dispatcher contract with a bounded one-tick deferral"*

**Assessment:** Terminology is consistent across all reviewed artifacts. Good.

#### OBS-2: Open items clearly documented
All reviewed artifacts explicitly identify the following as open:
- One-tick deferral budget (WCET/latency) — architecture, test spec, sprint plan, loop audit, CHANGELOG
- M14 cross-core memory ordering — architecture, test spec, sprint plan, loop audit, traceability.yaml (SRS-REQ-071 lean_note could be improved, see MIN-4)
- Mission rates / jitter tolerances / late-slot policy / recovery authority — architecture, test spec, sprint plan, loop audit, traceability.yaml
- Target/HIL evidence (GAP-A5) — architecture, test spec, sprint plan, loop audit, CHANGELOG
- N18 Renode evidence is emulated only, not target fidelity — all artifacts

This meets the requirement for clear documentation of open items.

#### OBS-3: N8 compliance — docs updated in same commit as behavior change
The CHANGELOG entry 3.66 lists all documentation updates together with the code change:
- `docs/vv/test-spec/ts-5-core-services.md` (rev 1.15)
- `docs/requirements/traceability.yaml`
- `docs/architecture/scheduling.md` (rev 0.4/0.5)
- `docs/planning/sprint-plan.md` (rev 1.128)
- `docs/status/handover.md` (rev 3.66)
- `docs/status/loop-audit.md` (rev 10.45)
- `docs/vv/verification-results.md`

This satisfies norm N8.

#### OBS-4: English language and terminology
All reviewed documents use consistent English, project terminology (SB, TBL, EVS, ES, HS, PAL, FF_* prefixes), and acronyms. No language issues found.

#### OBS-5: Test specification N22-TS5-66 oracle precision
The test spec (line 196) defines the nominal oracle precisely:
- Before next tick: `FF_SchedActiveCount()` unchanged (callback only snapshots)
- After exactly one tick: active count = newly activated count
- Exactly one `FF_SCHED_EVT_ACTIVATED` (0x0601) with `"SCH: table activated, N entries"`
- Following 1000 ms tick releases exactly one message (`dispatched` +1)

The test implementation (`test_tbl_sched_production_activate`, lines 320–364) verifies all these points exactly. The out-of-bound oracle (line 196) is also precisely defined and implemented in `test_tbl_sched_production_invalid` (lines 367–401). This is a strong example of N22-compliant test specification.

---

## 4. Traceability Verification (N4)

| Link | Verified | Notes |
|------|----------|-------|
| SRS-REQ-071 ↔ `src/core/sched/ff_sched.c`, `src/apps/app_scheduler.c`, `test_tbl_sched_coupling.c` | ✅ | Bidirectional in traceability.yaml |
| SRS-REQ-013 ↔ `src/core/tbl/tbl.c`, `test_tbl_sched_coupling.c` | ✅ | Bidirectional; test file added to SRS-REQ-013 `tests` array |
| SSS-REQ-030 ↔ SRS-REQ-071 (parent) | ✅ | `parent_sss: SSS-REQ-030` on SRS-REQ-071 |
| N22-TS5-66 (test spec) ↔ SRS-REQ-071, SRS-REQ-013 | ✅ | Case table row cites both |
| Architecture §5.1 ↔ Implementation (`app_scheduler.c`) | ✅ | Callback name, mutex, pending slot, apply-on-next-tick all match |
| CHANGELOG ↔ All updated docs | ✅ | Entry 3.66 lists all revised documents |

**Result:** Traceability is bidirectional and complete for the delta.

---

## 5. Disposition

| Category | Count |
|----------|-------|
| Major | 0 |
| Minor | 7 (MIN-1, MIN-4, MIN-5, MIN-6, MIN-7, MIN-8; MIN-2 and MIN-3 withdrawn) |
| Observation | 5 |

**Overall: APPROVED WITH COMMENTS**

The N22-TS5-66 documentation delta is complete, accurate, and consistent. The Minor findings are editorial/clarification items that do not affect the technical correctness of the documentation or the closure of the test gap. They should be addressed in the next documentation cycle (sprint S8-SCH closure or subsequent).

---

## 6. Reviewer Statement

This review was performed read-only against the working tree as of 2026-11-10. No files were modified. All findings are evidence-based with specific file:line references. The review covers documentation completeness, accuracy, and consistency per norms N4, N8, N17, N22. No safety, certification, or release authority is implied.

**Reviewer:** R4 — Senior Aeronautics and Space Software Documentation Engineer (independent context)  
**Date:** 2026-11-10