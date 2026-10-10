# Software Bus Architecture

| Document id | FRICK-ARCH-0002 |
| ----------- | --------------- |
| Revision    | 0.1 |
| Date        | 2026-10-10 |

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
- **Overflow policy**: publish never blocks the publisher; a full pipe drops
  the new message and raises an EVS error event.

## 3. Interface (summary)

| Function | Behaviour |
| -------- | --------- |
| `FF_SbInit` | initialise bus and static pools |
| `FF_SbCreatePipe(id, depth)` | create a bounded pipe |
| `FF_SbSubscribe(msg_id, pipe)` | exact or wildcard subscription |
| `FF_SbPublish(msg_id, payload, len)` | copy into every matching pipe |
| `FF_SbReceive(pipe, out, timeout)` | FIFO dequeue on the subscriber task |

## 4. Determinism

All pools are static tables sized at compile time; publish/lock are guarded by
the PAL mutex. Single-publish losslessness for subscribed pipes bounded at
`FF_SB_QUEUE_DEPTH_MAX` is proved in `Frick.SoftwareBus`
(`fits_queue_when_bounded`, SRS-REQ-001/SSS-REQ-002).

## 5. Traceability

SSS-REQ-002, SRS-REQ-001, SRS-REQ-002, SRS-REQ-015.