# Time, Timecode and PPS

| Document id | FRICK-ARCH-0006 |
| ----------- | --------------- |
| Revision    | 0.1 |
| Date        | 2026-10-10 |
| Requirements | SSS-REQ-007, SRS-REQ-010 |

This document describes how frick-fsw keeps time, how the on-board time is
encoded on the wire (CUC), and the current state of **PPS** (pulse per second)
handling.

## 1. Time sources

frick-fsw separates a **monotonic** clock from a **correlated UTC** clock.

| Concept | API | Meaning |
| ------- | --- | ------- |
| Mission elapsed time (MET) | `FF_TimeGetMetNs` / `FF_TimeGetMetMs` | nanoseconds since boot, monotonic, never adjusted |
| UTC correlation | `FF_TimeGetUtcNs` / `FF_TimeSetUtcNs` | nanoseconds since the mission epoch (currently the Unix epoch) |
| CUC | `FF_TimeGetCuc` | CCSDS Unsegmented Time Code derived from UTC |
| Leap seconds | `FF_TimeSetLeapSeconds` / `FF_TimeGetLeapSeconds` | static offset applied when building the CUC coarse field |

Implementation:

- `src/core/pal/pal_posix.c` — `FF_PalGetMonotonicNs` reads
  `clock_gettime(CLOCK_MONOTONIC)`. UTC is a pure software offset:
  `FF_PalGetUtcNs = monotonic + g_utc_offset_ns`, and `FF_PalSetUtcNs` sets the
  offset so that UTC equals the requested value at that instant.
- `src/core/pal/pal_rtems.c` — on RTEMS 6 the same POSIX surface is used; the
  monotonic clock is the RTEMS clock.
- `src/core/time/time.c` — mission-level API above the PAL; `FF_TimeInit`
  correlates UTC with the mission epoch (Unix epoch 0 in the baseline).
- `src/pus/pus.c` — every TM packet is timestamped with the current CUC
  (`FF_TimeGetCuc`) in `FF_PusSendTm`.
- `src/pus/pus_services.c` — PUS service 9 handlers.
- `src/transport/ccsds/ccsds.c` — serialises the CUC into the TM secondary
  header (`FF_CcsdsEncodeTm`).

## 2. Timecode format (CUC)

The on-board time is encoded as a **CCSDS Unsegmented Time Code (CUC)**: a
4-byte coarse field (whole seconds) followed by a 2-byte fine field (binary
fraction of a second), for a fixed 6-byte time code. This is the time code
carried in the PUS-C TM secondary header.

| Field | Bytes | Encoding |
| ----- | ----- | -------- |
| coarse | 4 | seconds (big-endian), mission epoch + leap-second offset |
| fine | 2 | fraction of a second in units of 2^-16 s (big-endian), ~15.26 us |

Conversion (`src/core/time/time.c`, `src/pus/pus_services.c`):

```text
coarse = utc_ns / 1e9 + leap_seconds
fine   = (utc_ns % 1e9) * 65536 / 1e9

utc_ns = coarse * 1e9 + fine * 1e9 / 65536      (service 9,1 set-time)
```

In the TM secondary header the fields are at offsets 7..10 (coarse) and 11..12
(fine); the PUS TM secondary header is therefore 13 bytes
(`FF_PUS_TM_SEC_HDR_LEN`). See `docs/interfaces/ICD.md` for the byte layout.

**Is the timecode CUC?** Yes. The coarse+fine pair is the reference CUC time
code with a 2^-16 s fractional resolution; it is set and reported through PUS
service 9 (subservices 1 and 2), satisfying SSS-REQ-007 and SRS-REQ-010.

## 3. PPS (pulse per second)

**PPS is not implemented in the current baseline.** There is no PPS interrupt,
no hardware time capture and no external time source anywhere in the tree: UTC
is a software offset over the monotonic clock, set at boot and optionally
changed by ground through PUS service 9,1. This is sufficient for the host
simulator and for early integration, but a real spacecraft needs an external
reference to bound the drift between the local oscillator and UTC.

### 3.1 What a PPS driver must do

To discipline the clock with a 1 Hz PPS edge the flight software must:

1. capture the PPS rising edge (RTEMS interrupt/GPIO pin or a timer capture
   register);
2. latch the monotonic counter at the edge (`FF_PalGetMonotonicNs`);
3. compare the latched counter with the expected period and steer or set the
   UTC correlation (`FF_TimeSetUtcNs`), optionally after combining it with a
   coarse on-board time code received from the ground or a time broadcaster;
4. optionally announce the correction as an event (EVS) and expose the lock
   state through housekeeping.

This is a low-level driver and is developed exactly like any other driver; see
`docs/engineering/driver-development.md`. A PPS/time driver is a natural
first board driver because it is small, observable and testable in isolation.

## 4. Open points (class-A gap)

- No PPS input and no oscillator drift model (SSS-REQ-007 covers only
  correlation and encoding).
- Leap seconds are a manually set static offset; there is no automatic
  announcement handling (e.g. PUS service 9,1 with a leap-second change).
- The mission epoch is the Unix epoch in the baseline; a real mission fixes the
  epoch in the mission tables.

These points are tracked with the other class-A gaps in
`docs/quality/class-a-gaps.md`.

## 5. History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial time/timecode/PPS note (norm N8) |
