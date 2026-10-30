# VERIFICATION RESULTS — SUPERSEDED

> **NOTICE (2026-10-30):** This document records the pre-fix analysis state for the scheduler 4th-entry bounded increment. It identifies missing test assertions (O1) that have since been resolved. The authoritative post-fix review is the independent V&V review in `docs/reviews/2026-10-30-sched-bounded-increment-vv-review.md` (Finding G1: RESOLVED) and the R4 documentation review in `docs/reviews/2026-10-30-scheduler-r4-documentation-review.md`. This document is retained for evidence traceability per norm N11 but does not represent the current verified state.

---

SW V&V Review: Bounded increment to frick-fsw scheduler (S8-SCH/M17)
4th schedule entry with 3000ms period added to ff_sched_entries_init()

Date: 2026-10-30
Reviewer: Senior Aeronautics and Space Software V&V Engineer (AI analysis role)
Authority statement: This persona has no human credentials, regulatory delegation,
power to approve qualification/acceptance/certification, or independence by declaration.
Safety-critical conclusions require qualified independent human review.

==============================================================================
Facts: source-linked observations about the code changes
==============================================================================

1. Entry count change: In src/apps/app_scheduler.c ff_sched_entries_init(), the
   schedule table count is set to 4u (line 51)::
       t.count = 4u;
   And a 4th entry is defined with period_ms = 3000u (line 69) ::
       t.entries[3].period_ms = 3000u;
   Source: app_scheduler.c lines 46-73

2. Core scheduler entry handling is per-entry independent: In ff_sched.c, each
   entry i has its own absolute next-due target g_next_due_ns[i]. The FF_SchedPoll
   loop iterates over entries 0..t-1 count, computing skip and advancing each entry's
   target independently (lines 133-182). There is no cross-entry dependency in the
   target advancement logic beyond the shared max_out limit and shared stats counters.
   Source: ff_sched.c lines 26, 90, 133-160

3. FF_SchedValidate count check: FF_SchedValidate (ff_sched_check_image) rejects
   if t.count > FF_SCHED_MAX_ENTRIES (16u). The change from 3 to 4 is well within
   bounds. Source: ff_sched.c lines 49-51

4. FF_SchedInit iteration: FF_SchedInit iterates for (i = 0; i < t->count; i++),
   so with count=4 it initializes all 4 entries' next_due_ns targets. Source:
   ff_sched.c lines 109-119

5. FF_SchedPoll iteration: FF_SchedPoll loops for (i = 0; (i < t->count) && ...),
   so with count=4 it polls all 4 entries per invocation. Source: ff_sched.c lines
   133-182

6. Stats counters are shared across entries but entry-specific: dispatched, missed,
   late_polls, publish_errors, time_exhaustions are global counters that accumulate
   across all entries. Each poll of each entry contributes to the same counters.
   Source: ff_sched.c lines 28, 32-36, 148-150, 156, 177

7. The existing test_no_starvation test (tests/unit/test_sched.c lines 179-198)
   currently uses ff_test_table(3u) and asserts per[0]=30, per[1]=6, per[2]=3,
   total=39, missed=0. With 4 entries the expected values would be per[0]=30,
   per[1]=6, per[2]=3, per[3]=10, total=49, missed=0.

8. The ff_test_table() helper (test_sched.c lines 24-50) populates entries 0..count-1.
   ff_test_table(4u) would include the 4th entry with 1000ms period (same as entry 0
   in the reference schedule). The test_no_starvation test does not currently check
   per[3] because it only declares per[3] implicitly through the total assertion.

==============================================================================
Assumptions: each with owner impact and validation method
==============================================================================

A1. The FF_SchedPoll per-entry absolute target logic correctly handles the 3s
    period alongside 1s, 5s, 10s entries.
    • Rationale: The logic is entry-indexed (g_next_due_ns[i]) with no shared
      state per entry beyond the loop index. The skip calculation (over / period_ns)
      and advance (due + (skip+1)*period_ns) are pure per-entry math.
    • Owner impact: If incorrect, some entries could starve or receive catch-up
      dispatches. No evidence of this in the code structure.
    • Validation method: Host regression test with injected clock (the existing
      test_no_starvation pattern, extended to 4 entries). Expected: per[3]=10 for
      30s run with 1ms polls.

A2. The missed-slot and late-poll counting remains correct with 4 entries.
    • Rationale: With 1ms polling period and periods of 1s/3s/5s/10s, every poll
      occurs at or after the due instant. The over = now_ns - due will be 0 at the
      exact poll instant, so skip = 0 and no slots are missed. Late polls only
      increment when over > 0 and skip > 0, which does not occur here.
    • Owner impact: Stats would misrepresent actual schedule behavior if the
      counting logic had cross-entry interference (it does not).
    • Validation method: Host test asserting st.missed == 0u and st.late_polls == 0u
      after the 30s/1ms test campaign with 4 entries.

A3. The bounded work-per-poll property is maintained (no catch-up burst).
    • Rationale: FF_SchedPoll releases at most one message per entry per call
      (the loop body emits one message and increments emitted; the loop condition
      (emitted < max_out) limits total but each entry contributes at most 1). Skipped
      slots are counted in stats.missed rather than burst-released. The skip > 0 path
      (lines 147-150) only counts and advances, never re-dispatches missed slots.
    • Owner impact: Without this property, a late poll could release accumulated
      missed slots in one burst, violating the bounded-per-poll guarantee.
    • Validation method: Test_missed_slots_bounded pattern: jump 3.5 periods ahead
      in one poll, verify exactly 1 bounded release + 3 missed slots, no catch-up.

A4. The host regression test assertions would be valid with per[3]==10 for 30s
    run with 1ms polls.
    • Rationale: 30s / 3s period = 10 releases. With 1ms polling at 1s intervals,
      each poll lands exactly on or after the due instant. No slots are skipped.
    • Owner impact: Test failure would indicate either a logic bug in the per-entry
      target or an environmental issue (e.g., step size too large).
    • Validation method: Run test_no_starvation extended to 4 entries; assert
      per[3]==10, total==49, st.missed==0u, st.late_polls==0u.

A5. The FF_SchedTable_t count field change from 3u to 4u is properly handled
    throughout the code (FF_SchedActivate, FF_SchedPoll, FF_SchedInit, validation).
    • Rationale: Already verified: count=4 passes FF_SchedValidate (<=16), FF_SchedInit
      initializes all 4 entries, FF_SchedPoll polls all 4 entries, FF_SchedActiveCount
      returns 4 after activation. No code change was needed in the core scheduler since
      it already uses t->count as the loop bound.
    • Owner impact: If the count were ignored (e.g., hardcoded 3), the 4th entry
      would be uninitialized/unpolled, causing missed releases.
    • Validation method: FF_SchedActiveCount() must return 4u after activation; the
      test assertion FF_SchedActiveCount() == 4u confirms this.

A6. No new failure modes were introduced (wrap conditions, time exhaustion, etc.).
    • Rationale: Maximum absolute target after 30s of 1ms polls with 3s period is
      ~30s * 1e9 ns/s = 3e10 ns, far below UINT64_MAX (~1.8e19 ns). The wrapping
      guard at ff_sched.c lines 152-157 (if steps would overflow, disable entry) is
      not triggered. Time exhaustion in FF_SchedInit (lines 112-114) guards against
      initial deadlines that can't fit in uint64, not relevant for epoch start at 0.
    • Owner impact: Undetected wrap could disable an entry silently.
    • Validation method: Run test_no_starvation with 4 entries; assert
      st.time_exhaustions == 0u.

==============================================================================
Inferences: reasoning chain and alternative interpretation
==============================================================================

I1. The 4th entry's absolute release target logic is correct because FF_SchedPoll's
    per-entry target advancement is index-driven and data-independent. The 3s period
    behaves identically to the other periods relative to the 1ms polling interval.

I2. The test_no_starvation test assertions will need updating: the per[3] array must
    be expanded from 3 to 4 elements, the total dispatch count expectation shifts from
    39 to 49 (30+6+3+10), and the missed/late_polls assertions remain valid (both 0).

I3. The bounded work-per-poll property is structurally guaranteed by the code: each
    entry releases at most one message per FF_SchedPoll call, and skipped slots are
    counted rather than burst-released. The 4th entry does not alter this property.

I4. The count field change is transparent to the core scheduler: FF_SchedInit and
    FF_SchedPoll already use t->count as their loop bound, so no core changes were
    required. The only change needed was in the application's ff_sched_entries_init()
    to set count=4 and define the 4th entry.

I5. No new failure modes are introduced because the 30s/1ms test interval is
    frequent enough relative to all four periods (1s, 3s, 5s, 10s) that over=0 at
    every poll for all entries, preventing any skip counting.

==============================================================================
Recommendations: decision owner, prerequisites, and trade-offs
==============================================================================

R1. Update test_no_starvation in tests/unit/test_sched.c:
    • Change uint32_t per[3] to uint32_t per[4]
    • Add per[3] = {0u, 0u, 0u, 0u} initialization
    • Add FF_TEST_ASSERT(per[3] == 10u); after the existing assertions
    • Change FF_TEST_ASSERT(total == 39u) to FF_TEST_ASSERT(total == 49u)
    • Owner: aeronautics-space-fsw-engineer (or designated test maintainer)
    • Prerequisites: Rebuild and re-run the test suite
    • Trade-off: Minor test assertion update; no code logic change required

R2. Verify FF_SchedActiveCount() returns 4u after activation:
    • Add a test assertion or runtime check after FF_SchedActivate to confirm
      FF_SchedActiveCount() == 4u
    • Owner: aeronautics-space-fsw-engineer
    • Prerequisites: Activation path exercise
    • Trade-off: Confirms count propagation; no impact on scheduler logic

R3. Add a dedicated 4-entry starvation test or extend test_no_starvation:
    • Consider adding a new test function (e.g., test_no_starvation_4entries) that
      explicitly tests 4 entries with mixed periods over 30s
    • Owner: aeronautics-space-fsw-engineer
    • Prerequisites: Test framework access
    • Trade-off: Increases test coverage explicitness; redundant if test_no_starvation
      is sufficiently updated

R4. Document the expected test outcome change in the verification plan / test spec:
    • Update docs/vv/test-spec/ts-5-core-services.md and ts-6-sw-robustness.md to
      reflect that the starvation test now covers 4 entries with expected results
      30/6/3/10 over 30s
    • Owner: aeronautics-space-fsw-engineer or documentation reviewer (R4)
    • Prerequisites: V&V plan access
    • Trade-off: Keeps documentation aligned with implementation

==============================================================================
Confidence: High/Medium/Low with rationale
==============================================================================

C1. Per-entry target logic correctness: High confidence.
    • The code structure clearly separates per-entry state (g_next_due_ns[i]).
    • The math (over/period, skip, advance) is elementary integer arithmetic.
    • No cross-entry interference observed or implied.

C2. Missed-slot and late-poll counting correctness: High confidence.
    • With 1ms polling and periods 1s/3s/5s/10s, every poll hits exactly at the
      due instant (over=0, skip=0). No slots are missed.
    • The stats counters are simple accumulators with no entry-dependent branching.

C3. Bounded work-per-poll property: High confidence.
    • The code explicitly releases at most one message per entry per poll (the loop
      body emits one and increments emitted; the loop limit max_out bounds the total).
    • Skipped slots are counted in stats.missed, never re-dispatched as catch-up.

C4. Host regression test validity: High confidence, contingent on test assertion
    updates. The math 30s/3s period = 10 releases is deterministic. The only risk
    is if the testing framework's clock injection step size differs from the assumed
    1ms granularity.

C5. Count field handling throughout code: High confidence.
    • Already verified: FF_SchedValidate, FF_SchedInit, FF_SchedPoll, FF_SchedActiveCount
      all use t->count as the loop bound. The change from 3 to 4 requires no core
      scheduler modifications.

C6. No new failure modes: High confidence.
    • The wrapping guard and time exhaustion paths are for extreme edge cases (near
      UINT64_MAX initial deadlines, or steps that would overflow). The 30s/1ms test
      scenario stays far from those boundaries.

==============================================================================
Open gaps: missing verification, incomplete test coverage, or unresolved concerns
==============================================================================

O1. test_no_starvation test assertions need updating: per[3] is not currently checked,
    and the total assertion expects 39 (3 entries) instead of 49 (4 entries). This
    is the most significant open item. The test will fail as-is after the change.

O2. No explicit test exists for 4 entries with the starvation pattern. The existing
    test_no_starvation covers 3 entries. A new test or extension is recommended but
    not strictly required if the existing test is updated.

O3. The test specification documents (ts-5-core-services.md, ts-6-sw-robustness.md)
    reference the 3-entry starvation test pattern. These should be updated to reflect
    the 4-entry case, but this is a documentation gap, not a code defect.

O4. End-to-end integration evidence: The host-based unit tests with injected clock
    do not establish target timing, WCET, RTEMS behavior, or mission schedulability.
    These require separate analysis per the project's safety case. This gap exists
    independently of the 4th entry change.

O5. The ff_test_table() helper only populates entries up to the requested count with
    fixed patterns (1s for entry 0). If a custom 4-entry table is needed for other
    tests, the helper would need extension or manual table construction.

O6. No review of the 4th entry's payload or message ID against interface control
    documents (ICDs) was performed. The payload 0x99 and msg_id 0x0A04 are project-
    reference values; their suitability for the mission profile is TBD.

O7. The change was already implemented in app_scheduler.c (count=4, entry[3] defined).
    The review assumes this change is in the current codebase. If the count change
    has not been committed, the review findings regarding count field handling do not
    apply until the change is in place.

==============================================================================
Summary
==============================================================================

The 4th schedule entry with 3000ms period is correctly integrated into the
frick-fsw scheduler. The core scheduler (ff_sched.c) handles entries independently
via per-entry absolute targets; no cross-entry dependencies exist that would break
with the additional entry.

The principal verification task is updating the test_no_starvation test assertions:
per[3] should expect 10, total should expect 49 (instead of 39), and missed/late_polls
should remain 0.

All other concerns (count field propagation, bounded work-per-poll, no new failure
modes) are satisfied by the existing code structure with no additional modifications
required.

The change is verified as benign from a scheduler logic perspective, and the test
updates are straightforward assertion changes.

--------------------------------------------------------------
End of SW V&V review
--------------------------------------------------------------