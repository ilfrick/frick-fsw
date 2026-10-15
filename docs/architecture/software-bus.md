# Software Bus Architecture

| Document id | FRICK-ARCH-0002 |
| ----------- | --------------- |
| Revision    | 0.8 |
| Date        | 2026-10-14 |

## 1. Purpose

The Software Bus (SB) delivers message copies from publishers to
subscribers, cFS-SB style, with static allocation.

## 2. Model

- **Message identifiers** follow the cFS convention `0xMMSS`
  (mission/sub-system). `FF_SB_WILDCARD` (`0xFFFF`) matches any message id.
- **Pipes**: bounded FIFO queues of messages (depth tunable via
  `FF_SB_*` in `config/fsw_config.h`); a pipe is owned by one subscriber task.
- **Subscriptions**: a pipe subscribes to exact message ids and/or the
  wildcard; delivery copies the message into every matching pipe (SRS-REQ-001).
- **Ordering**: FIFO per pipe (SRS-REQ-002).
- **Overflow policy**: publish never waits for the SB mutex; if it is busy,
  `FF_SbPublish` returns `FF_TIMEOUT` before changing statistics or delivering
  to any pipe. For each matching subscriber, `FF_PalQueueTryPut` acquires that
  queue mutex once. If it is busy, only that copy is dropped, `dropped`
  increments, fan-out continues to the remaining bounded subscription set, and
  `FF_SbPublish` returns `FF_TIMEOUT` after processing all matches. A full pipe
  is distinct: it returns `FF_BUFFER_FULL` from PAL, drops only that copy,
  increments `dropped`, and preserves the existing `FF_SbPublish` status
  (`FF_SUCCESS` when no queue lock was busy). Both cases request a deferred EVS
  error (`FF_EVS_SB_ERR`, `FF_EVS_ERROR`) identifying the subscriber, message,
  and whether its queue was busy or full through `FF_EvsSendDeferred`. Ordinary
  `FF_EvsSend` retains synchronous listener delivery; only the SB overflow path
  selects deferred delivery. Deferred EVS uses a non-waiting lock attempt: a
  busy EVS lock returns `FF_TIMEOUT` without changing the EVS log, notification
  queue, or notification statistics. A full deferred queue retains the event in
  its circular log, increments `FF_EvsNotifyStats_t.dropped`, and returns
  `FF_NO_RESOURCE`. The SB publisher does not propagate either secondary EVS
  status. Therefore SB `FF_SUCCESS` means the SB fan-out/accounting completed;
  it does **not** claim EVS enqueue or ground forwarding. Busy/saturated EVS
  reporting can be lost and SRS-REQ-011's every-event forwarding requirement is
  not demonstrated; NC-EXT-76 remains open for authority-owned overload policy.
  The TM-output task calls `FF_EvsDispatchPending` with a limit of eight events
  per iteration. Callbacks run outside SB/EVS locks; callback-produced events
  are delivered on a later dispatch iteration, with no recursive callback
  chain. SB and matched-pipe enqueue use non-waiting mutex acquisition for the
  publisher. This characterizes those lock-acquisition paths only; it does not
  establish queue-operation WCET or whole-publish timing. Requirement inputs
  still needed for SRS-REQ-011 are maximum event rate,
  maximum burst, minimum dispatch service and maximum forwarding latency. Their
   mission values are TBD by the authorized requirements owner; none are invented.
   A host-only sweep of fan-out, configured queue depth, overflow bursts through
   EVS saturation, and dispatch batch limits is published in
   `sb-evs-characterization.md` (FRICK-ARCH-0019). It is structural framework
   characterization only—not a mission profile, WCET, target-timing, or
   ground-forwarding guarantee; NC-EXT-75/76 remain open.
- **Initialization dependency**: initialize EVS before making SB available for
  publish, so overflow reporting has a valid EVS log and listener path.

## 3. Interface (summary)

| Function | Behaviour |
| -------- | --------- |
| `FF_SbInit` | initialise bus and static pools |
| `FF_SbCreatePipe(id, depth)` | create a bounded pipe |
| `FF_SbSubscribe(msg_id, pipe)` | exact or wildcard subscription |
| `FF_SbPublish(msg)` | try-lock SB; busy returns `FF_TIMEOUT` with no fan-out/statistics mutation; queue-mutex busy skips that copy, continues fan-out, requests a deferred busy event and returns `FF_TIMEOUT`; full pipe requests a distinct deferred full event and otherwise preserves `FF_SUCCESS` (secondary EVS status is not reflected by SB result) |
| `FF_SbReceive(pipe, out, timeout)` | FIFO dequeue on the subscriber task |

## 4. Determinism

All pools are static tables sized at compile time; publish/lock are guarded by
the PAL mutex. Single-publish losslessness for subscribed pipes bounded at
`FF_SB_QUEUE_DEPTH_MAX` is proved in `Frick.SoftwareBus`
(`fits_queue_when_bounded`, SRS-REQ-001/SSS-REQ-002).

## 5. Traceability

SSS-REQ-002, SRS-REQ-001, SRS-REQ-002, SRS-REQ-015.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial Software Bus note (norm N8/N16) |
| 0.2 | 2026-10-14 | frick-fsw | Specify overflow accounting/event identity and EVS-before-SB initialization; notify outside the SB mutex |
| 0.3 | 2026-10-14 | frick-fsw | Defer EVS listener callbacks to bounded TM-output dispatch; specify notification-queue overflow accounting and status |
| 0.4 | 2026-10-14 | frick-fsw | Limit deferred EVS delivery to SB overflow; retain synchronous `FF_EvsSend` semantics, preserve the no-block requirement and record its mutex-evidence gap |
| 0.5 | 2026-10-14 | frick-fsw | Apply user-approved non-waiting SB/EVS deferred lock attempts; specify no-side-effect timeout and retain the SRS-REQ-011 forwarding-policy gap and mission-bound TBDs |
| 0.6 | 2026-10-14 | frick-fsw | Add portable non-waiting queue-mutex acquisition; specify partial fan-out, dropped accounting, distinct busy/full EVS messages and `FF_TIMEOUT` for busy queue contention; SRS-REQ-072 remains open |
| 0.7 | 2026-10-14 | frick-fsw | Link the user-facing SB/EVS configured structural sweep and state explicitly that it provides no mission-envelope or timing guarantee |
| 0.8 | 2026-10-14 | frick-fsw | Clarify publisher lock behavior without implying whole-operation wait-freedom or WCET; remove ambiguous phrasing after R4 review |
