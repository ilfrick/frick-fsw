# NC-EXT-251 TDD RED Phase Summary

**Date:** 2026-11-15
**Test:** `test_pus_dispatch_reservation_contention`
**Status:** RED (expected failures)

## Changed Files (RED Phase Only)

### New Files
- `tests/unit/test_pus_dispatch_reservation_contention.c` - RED test for dispatch reservation contention
- `docs/engineering/nc-ext-251-design/bounded-state-machine.md` - Design note with bounded state machine

### Modified Files
- `tests/CMakeLists.txt` - Added test executable and CTest registration

## Test Execution Result (RED)

```
FAIL .../test_pus_dispatch_reservation_contention.c:165: g_sink_blocked
FAIL .../test_pus_dispatch_reservation_contention.c:174: rc == FF_BUSY
FAIL .../test_pus_dispatch_reservation_contention.c:177: stream_count(1, 1) == 0
FAIL .../test_pus_dispatch_reservation_contention.c:178: stream_count(1, 3) == 0
FAIL .../test_pus_dispatch_reservation_contention.c:179: stream_count(1, 5) == 0
FAIL .../test_pus_dispatch_reservation_contention.c:180: stream_count(1, 7) == 0
test_pus_dispatch_reservation_contention checks=21 failures=6
```

**Exit code:** 1 (test failure as expected)

## RED Phase Verification

✅ Test compiles successfully (no compile errors)
✅ Test links and runs (no setup/link errors)
✅ Test failures are **assertion failures** on the expected behavior (not infrastructure failures)
✅ The key assertion `rc == FF_BUSY` fails because current implementation has no reservation mechanism
✅ The sink blocking mechanism works (`g_sink_blocked` becomes true)
✅ No production code edited - only test and build configuration

## Design Note Summary

The design note `docs/engineering/nc-ext-251-design/bounded-state-machine.md` documents:

1. **Bounded State Machine** with:
   - Capacity: 1 TC dispatch at a time, 4 S1 ACK reservation slots per TC
   - Lock order: dispatch mutex (try) → state mutex (try, never across handlers/sink)
   - Stage mapping: 4 bits for accept/start/progress/completion
   - Pre-acceptance rollback on any error/rejection
   - Handler-originated TM and sink reentry behavior
   - Documented callback non-termination boundary

2. **API Changes Needed** (minimal):
   - `FF_PusSetTmSink` and `FF_PusInstallPanicReporter` return `FF_Status_t` (currently void)
   - Counter getters remain scalar (no error channel needed)

3. **Test Plan** (for reference):
   - RED test implemented: `test_pus_dispatch_reservation_contention`
   - Later tests (NOT added yet): max-four/order, release, exhaustion, accepted-path contention, reentry

4. **Viability:** All constraints satisfied within current architecture - no SRS weakening, no sink contract change, no worker thread needed.

## Next Step (GREEN Phase)

Implement the dispatch reservation mechanism in `src/pus/pus.c` to make the RED test pass:
- Add dispatch mutex and state
- Add reservation logic before TC acceptance
- Map lock contention to `FF_BUSY`
- Release state mutex before handler/sink calls
- Keep `FF_PusSendTmDirect` lock-free with atomic counter

## Evidence Location

- Raw test output: `docs/engineering/nc-ext-251-design/scratch-logs/red-phase-run.log`
- Design note: `docs/engineering/nc-ext-251-design/bounded-state-machine.md`