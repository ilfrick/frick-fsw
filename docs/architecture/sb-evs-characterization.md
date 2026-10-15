# SB/EVS Framework Structural Characterization

| Field | Value |
|---|---|
| Document ID | FRICK-ARCH-0019 |
| Revision | 0.1 |
| Date | 2026-10-14 |
| Status | Exploratory host characterization; not a mission requirement or qualification result |
| Related records | NC-EXT-75, NC-EXT-76; `docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.csv` |

## Purpose and limits

This document reports what the current SB/EVS implementation does at its
configured **structural** limits. It is intended to help framework users select
candidate configurations and design their own tests. It does not establish a
mission event-arrival profile, throughput guarantee, WCET, target timing,
link-availability guarantee, successful ground receipt, or compliance with
SRS-REQ-011/SRS-REQ-072. It does not close NC-EXT-75 or NC-EXT-76.

No mission values are assumed. `Bmax`, `Rmax`, `Dmin`, `Lmax`, service interval,
burst/link profile, ground-receipt boundary, post-acceptance loss-detection
point, counter range/wrap/reset semantics, and reporting opportunity remain
TBD for accountable Systems/Operations authority input. Local log/statistics
are not evidence of ground receipt.

## Swept configuration space

Using the current `config/fsw_config.h` values (`FF_SB_MAX_PIPES=32`,
`FF_SB_QUEUE_DEPTH_MAX=32`, `FF_EVS_LOG_DEPTH=64`), the host harness evaluated:

1. **Fan-out:** every integer 1–32 matched SB pipes.
2. **Per-pipe queue depth:** every integer 1–32.
3. **Full-queue overflow burst:** every integer publish count from zero through
   the first count that generates more than 64 overflow notifications for that
   fan-out. The saturation count is `ceil(65 / fanout)`, equivalently
   `(64 + fanout) / fanout` in integer arithmetic. This produces 9,984
   fan-out/depth/burst profiles.
4. **EVS dispatch bound:** every integer `max_events` from 1–64. For bounds
   1–63, one more event than the bound is pending; the test checks that exactly
   the bound is dispatched and the remaining event is dispatched afterward.
   At the maximum bound 64, the queue capacity is also 64, so it is filled and
   drained completely.

Each profile asserts the accepted SB copies, SB drop count, EVS log count and
retained suffix, deferred notifications queued/dropped, and callbacks after
explicit dispatch. The runner is `tools/scripts/sb_evs_structural_sweep.py`;
the executable oracle is `tests/unit/test_sb_sweep.c`. Reproduce the same matrix
and write the retained evidence copy with:

```sh
cmake -S . -B build/sb-sweep -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/sb-sweep --target test_sb_sweep
python3 tools/scripts/sb_evs_structural_sweep.py \
  build/sb-sweep/tests/test_sb_sweep \
  --output docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.csv
```

The Make target `make sb-evs-sweep` uses the script's default output under
`build/`; it does not overwrite the retained evidence copy. Before publishing
an output file, the runner checks the executable's compiled limits, validates
each requested profile against its result, serializes to a temporary file,
re-reads and audits the CSV, then atomically renames it.

## Observed structural boundary

- For every fan-out/depth pair, filling each pipe to its configured depth
  accepts `fanout × depth` copies without SB drops.
- The smallest overflow burst that exceeds the 64-entry deferred EVS queue
  generates 65–96 overflow events, depending on fan-out. The queue accepts 64;
  the remainder is counted in `FF_EvsNotifyStats_t.dropped` (1–32).
- The EVS circular log records the overflow events and returns its newest 64
  when more than 64 have been logged. The test checks expected message-ID and
  application-ID ordering in the retained suffix.
- For every dispatch limit 1–63, the batch sweep queues one more event than the
  limit, verifies exactly the limit is dispatched and one remains, then verifies
  that remainder dispatches. At limit 64, the queue's configured capacity is 64,
  so the test fills and drains all 64. Profiles with fewer pending events also
  call dispatch with the configured maximum and assert that only pending events
  are dispatched.
- A separate deterministic contention test holds all 32 matched pipe queue
  mutexes; publish returns `FF_TIMEOUT`, records 32 drops, and deferred busy
  reports are dispatched after unlocking. That all-lock injection is a structural
  corner case, not a probabilistic timing or partial-lock-subset campaign.

These are finite-buffer and explicit-call observations on the native host.
They contain no elapsed-time arrival model, concurrent producer scheduling,
target measurement, flight link, or ground segment. The word “boundary” here
means only the observed capacity/saturation boundary for the exact compiled
configuration and test oracle.

## Characterization result and trace

Fresh run: 9,984 profiles passed with zero assertion failures; the dispatch
bound sweep 1–64 passed, including excess-pending checks below capacity. Its
explicit dispatch-sweep CSV record accounts for 2,207 callbacks; the `dispatch_calls`
column summed over all 9,985 CSV records is 341,151 because each structural
profile also dispatches its accepted pending notifications. The CSV records all
tested parameters and outcomes. The evidence note records build identity and
SHA-256 values. This
result is useful framework characterization, but its harness/script remains an
internal exploratory verification tool pending its applicable independent
review/qualification. It must not be cited as a mission-approved operating
envelope.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.1 | 2026-10-14 | frick-fsw | Publish configured SB/EVS structural parameter sweep for framework users; distinguish capacity characterization from mission performance and leave NC-EXT-75/76 authority-owned items open |
