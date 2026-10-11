# Time, Timecode and PPS

| Document id | FRICK-ARCH-0006 |
| ----------- | --------------- |
| Revision    | 0.3 |
| Date        | 2026-10-10 |
| Requirements | SSS-REQ-007, SSS-REQ-016, SRS-REQ-010, SRS-REQ-021..024 |

This document describes how frick-fsw keeps time, how the on-board time is
encoded on the wire (CUC), and how the time base is disciplined by a
**1 Hz PPS tone** (milestone M2).

## 1. Time sources

frick-fsw separates a **monotonic** clock from a **correlated UTC** clock and
selects at run time which one feeds the exported CUC time.

| Concept | API | Meaning |
| ------- | --- | ------- |
| Mission elapsed time (MET) | `FF_TimeGetMetNs` / `FF_TimeGetMetMs` | nanoseconds since boot, monotonic, never adjusted |
| UTC correlation | `FF_TimeGetUtcNs` / `FF_TimeSetUtcNs` | nanoseconds since the mission epoch (currently the Unix epoch) |
| Time source | `FF_TimeSetSource` / `FF_TimeGetSource` | `FF_TIME_SOURCE_INTERNAL` (CPU clock), `FF_TIME_SOURCE_TONE` (external 1 Hz / PPS), `FF_TIME_SOURCE_GROUND` (ground correlation) |
| Tone capture | `FF_TimeToneSignal` / `FF_TimeGetToneCount` | 1 Hz edge handling; monotonic latch and CUC jump discipline |
| Leap seconds | `FF_TimeScheduleLeap` / `FF_TimeGetPendingLeap` | pending leap change applied at the next tone boundary |
| CUC | `FF_TimeGetCuc` / `FF_TimeCucEncode` / `FF_TimeCucDecode` | CCSDS Unsegmented Time Code derived from the selected source |

Implementation:

- `src/core/pal/pal_posix.c` — `FF_PalGetMonotonicNs` reads
  `clock_gettime(CLOCK_MONOTONIC)`. UTC is a pure software offset:
  `FF_PalGetUtcNs = monotonic + g_utc_offset_ns`, and `FF_PalSetUtcNs` sets the
  offset so that UTC equals the requested value at that instant.
- `src/core/pal/pal_rtems.c` — on RTEMS 6 the same POSIX surface is used; the
  monotonic clock is the RTEMS clock.
- `src/core/time/time.c` — mission-level API above the PAL: time-source
  selection, tone discipline, leap scheduling, CUC framing (M2).
  `FF_TimeInit` correlates UTC with the mission epoch (Unix epoch 0 in the
  baseline).
- `src/core/time/pps.c` — PPS backend vtable (`FF_PpsRegister`, `FF_PpsStart`,
  `FF_PpsStop`) and the native software backend task that issues a 1 Hz tone on
  the host simulator by calling `FF_TimeToneSignal`; see section 3.
- `src/pus/pus.c` — every TM packet is timestamped with the current CUC
  (`FF_TimeGetCuc`) in `FF_PusSendTm`.
- `src/pus/pus_services.c` — PUS service 9 handlers.
- `src/transport/ccsds/ccsds.c` — serialises the CUC into the TM secondary
  header (`FF_CcsdsEncodeTm`).

### 1.1 Source selection (SRS-REQ-021)

`FF_TimeSetSource` switches between:

| Source | Clocks | Notes |
| ------ | ------ | ----- |
| `FF_TIME_SOURCE_INTERNAL` | MET/UTC | default; UTC = monotonic + offset |
| `FF_TIME_SOURCE_TONE` | tone epoch | UTC = `g_tone_utc_ns + (now - tone_epoch)`, monotonic since latch |
| `FF_TIME_SOURCE_GROUND` | UTC offset | set by PUS service 9,1; `FF_TimeSetUtcNs` selects GROUND when the active source is INTERNAL |

The source enum is a proper, disjoint set (Lean `req_srs021_sources_distinct`,
`req_srs021_source_set_disjoint`); with a non-negative pending leap the CUC
seconds never regress across a tone boundary (Lean
`req_srs021_utc_non_regression`).

**Switch semantics (SRS-REQ-021).** Selecting a source is an explicit authority
change, performed by the operator/ground: after the switch the exported CUC is
continuous with the *newly selected* source definition. At the switch instant
the coarse seconds may therefore step by at most one second (the TONE
correlation carries the `+1 s` first-edge convention of section 2; the other
sources differ by less than one second because each derives from the same
PAL UTC offset at the instant of the switch). No regression guarantee applies
across a switch itself; the guarantee that holds is the *within-source*
non-regression against the tone boundary. Every state transition in
`time.c` is protected by a PAL mutex (`g_time_mtx`), so the switch and the CUC
exports are always coherent (no torn multi-word state, R1 review NC-03).

**Correlation vs active source.** `FF_TimeSetUtcNs` always updates the stored
PAL UTC correlation, but it only changes the active source when it is currently
INTERNAL (then it moves to GROUND). While TONE is active the correlation update
is stored without silently changing the source: the exported CUC continues to
follow the tone epoch until the operator explicitly selects another source with
`FF_TimeSetSource` (no implicit TONE->GROUND re-alignment; R1 delta comment).

## 2. Tone discipline (SRS-REQ-022)

A **tone** is the 1 Hz edge (PPS). `FF_TimeToneSignal(mono_ns)` implements the
discipline:

1. The tone epoch (`mono_ns`) must strictly increase with respect to the last
   accepted tone, otherwise the tone is rejected with `FF_INVALID_ARGUMENT`
   (out-of-order edges are ignored).
2. The **first** tone both latches the correlation and establishes the first
   disciplined second boundary: it sets `g_tone_utc_ns = FF_PalGetUtcNs() + 1 s`
   (the cFS "time at tone will be" convention). Immediately after the first
   edge, TONE-mode UTC therefore reads `latched correlation + 1 s`, and the
   offset is permanent while TONE stays active (it is cancelled by switching
   source, section 1.1).
3. Every further tone adds exactly `FF_TIME_TONE_NS` (1 s) to `g_tone_utc_ns`;
   the tone epoch is replaced by the new edge timestamp.
4. Under the TONE source, UTC is computed as
   `g_tone_utc_ns + (now - epoch)`, so no drift accumulates between tones.

Each valid tone advances the tone second counter by exactly one second and the
tone counter strictly increases (Lean `req_srs022_tone_second_advance`,
`req_srs022_tone_count_strict`). Under TONE mode the exported CUC coarse
seconds are the tone second counter plus the accumulated leap offset
(section 4).

## 3. PPS backends (`src/core/time/pps.c`, `pps.h`)

The tone source is decoupled from the physical edge by a backend vtable:

```c
typedef struct {
    const char* name;
    FF_ReturnCode_t (*init)(void);
    FF_ReturnCode_t (*start)(void);
    FF_ReturnCode_t (*stop)(void);
} FF_PpsBackend_t;
```

- `FF_PpsRegister(backend)` selects the active backend (one at a time).
- `FF_PpsStart` / `FF_PpsStop` run/stop the backend.
- `FF_PpsSwBackend` is the native **software backend**: a PAL task
  (`FF_TIME_TONE_TASK_PRIORITY` = 40) that sleeps
  `FF_TIME_SW_TONE_PERIOD_MS` (1000 ms) and calls `FF_TimeToneSignal` with the
  current monotonic clock. It is disabled by default in the simulator; tests
  drive `FF_TimeToneSignal` directly with controlled timestamps.
- The **RTEMS/MPFS hardware backend** (PLIC GPIO capture ISR) is the board
  integrator's hook (GAP-A7): the ISR latches the monotonic timestamp and
  defers to a task that calls `FF_TimeToneSignal` (task context only: the tone
  state is mutex-protected); see `docs/engineering/driver-development.md`.
- The task priority/stack arguments are effective on the RTEMS target; on the
  host simulator the PAL maps them to the default POSIX scheduling policy and
  they are advisory only (R1 review Comment-01). Edge qualification (rejecting
  spurious edges and enforcing a plausible tone spacing) is deliberately a
  front-end/driver responsibility: the core only enforces strict monotonicity
  and the one-second epoch advance, so the same core serves both the ideal
  software backend and a hardware capture path (R1 review NC-06).

## 4. Leap seconds at the tone boundary (SRS-REQ-023)

Leap seconds are scheduled, never applied mid-second:

- `FF_TimeScheduleLeap(delta)` with `delta ∈ {-1, 0, +1}` stages a pending leap;
  only one second at a time is allowed (`FF_TIME_CUC_MAX_LEAP_S`).
- At the **next tone boundary** the pending change is added to
  `g_leap_seconds` and cleared (`pending = 0`). The CUC coarse field is then
  `utc_ns / 1e9 + g_leap_seconds`, so the leap is never double counted.
- A pending leap is **inert while pending**: it never alters the exported CUC.
  If no valid tone ever arrives it remains pending indefinitely and is never
  applied; it is applied and cleared only at the first valid tone that follows
  (SRS-REQ-023 no-tone clause, unit test `test_leap_pending_without_tone`).
- With a non-negative pending leap the CUC seconds never regress across the
  boundary (Lean `req_srs023_leap_cleared_at_tone`,
  `req_srs023_leap_accumulated_at_tone`, `req_srs021_utc_non_regression`).

## 5. Timecode format (CUC)

On-board time is exchanged as a **CCSDS Unsegmented Time Code (CUC)** per
CCSDS 301.0-B-4, tailored to a 4-octet coarse field (whole seconds) plus a
2-octet fine field (binary fraction of a second). The tailoring is fixed by the
P-field `0x2E` (4 coarse + 2 fine octets, agency epoch). **Two exchange forms
are defined and must not be confused** (R3 review NC-1):

- the **explicit self-describing form** — P-field + T-field, **7 octets**
  (`FF_TIME_CUC_LEN`) — produced and consumed only by
  `FF_TimeCucEncode`/`FF_TimeCucDecode`;
- the **PUS-C on-wire T-field** — **6 octets**, P-field implicit by interface
  agreement — carried in the TM secondary header (section 5.2) and in the PUS
  service 9 payloads. `FF_PusSendTm` and `svc9_set_time`/`svc9_report_time`
  use this form and the service layer accepts only its exact 6-octet length.

### 5.1 P-field (CCSDS 301.0-B-4 §3.2.2)

The P-field byte is **`0x2E`** (`0b00101110`), matching the CCSDS 301.0-B-4
reference bit layout (also used by Orekit):

| Bits | Value | Meaning |
| ---- | ----- | ------- |
| 7 (mask 0x80) | 0 | unsegmented time code |
| 6..4 (mask 0x70) | 010 | time code identification: agency-defined epoch |
| 3..2 (mask 0x0C) | 11 | number of coarse octets minus one (3 → 4 octets) |
| 1..0 (mask 0x03) | 10 | number of fine octets (2) |

### 5.2 T-field

| Field | Bytes | Encoding |
| ----- | ----- | -------- |
| coarse | 4 | seconds big-endian: `utc_ns / 1e9 + g_leap_seconds` |
| fine | 2 | fraction of a second in units of 2^-16 s big-endian (≈15.26 us) |

The **explicit** form is P-field (1) + T-field (6) = **7 octets**
(`FF_TIME_CUC_LEN`). The **PUS-C on-wire** form is the T-field alone = **6
octets** (P-field implicit), as in the TM secondary header and PUS service 9.

Conversion (`src/core/time/time.c`):

```text
coarse = utc_ns / 1e9 + leap_seconds
fine   = (utc_ns % 1e9) * 65536 / 1e9

utc_ns = coarse * 1e9 + fine * 1e9 / 65536      (service 9,1 set-time, 6-octet T-field)
```

`FF_TimeCucEncode` writes `{P, coarse[4] BE, fine[2] BE}` and
`FF_TimeCucDecode` parses it back; a wrong-length input is rejected with
`FF_BAD_SIZE` and an unknown P-field with `FF_PARSE_ERROR`. The coarse/fine
fields are recovered losslessly and the fine fraction of any sub-second count
always fits 16 bits (Lean `req_srs024_cuc_field_roundtrip`,
`req_srs024_fine_frac_bounded`, `req_srs024_coarse_byte_decompose`).

In the TM secondary header the 6-octet T-field is at byte offsets 7..12
(coarse 7..10, fine 11..12); the PUS TM secondary header is therefore 13 bytes
(`FF_PUS_TM_SEC_HDR_LEN`). See `docs/interfaces/ICD.md` for the byte layout.

**Is the timecode CUC?** Yes. Both forms are the same CUC tailoring with a
2^-16 s fractional resolution; the explicit `FF_TimeCucEncode/Decode` form is
used for self-describing interchange, and the implicit 6-octet T-field is set
and reported through PUS service 9 (subservices 1 and 2), satisfying
SSS-REQ-007 and SRS-REQ-010.

## 6. Configuration

Tunable in `config/fsw_config.h`:

| Macro | Value | Meaning |
| ----- | ----- | ------- |
| `FF_TIME_TONE_NS` | 1000000000LL | nominal tone period (1 s) |
| `FF_TIME_CUC_COARSE_OCTETS` | 4 | coarse field length |
| `FF_TIME_CUC_FINE_OCTETS` | 2 | fine field length |
| `FF_TIME_CUC_MAX_LEAP_S` | 1 | max scheduled leap change (single second) |
| `FF_TIME_SW_TONE_PERIOD_MS` | 1000 | software backend period |
| `FF_TIME_TONE_TASK_PRIORITY` | 40 | software tone task priority |

## 7. Open points (class-A gap)

- On-target PPS capture (RTEMS MPFS GPIO/PLIC ISR) and a CCSDS 1958 (level 1) /
  TAI epoch correlation with a leap-second table require the flight hardware
  and an on-board campaign: GAP-A7 in `docs/quality/class-a-gaps.md`. Edge
  plausibility (minimum spacing, glitch rejection) belongs to this on-target
  capture front-end, not to the core time base (R1 review NC-06).
- The mission epoch is the Unix epoch in the baseline; a real mission fixes the
  epoch in the mission tables.

## 8. History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial time/timecode/PPS note (norm N8) |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: time-source selection, tone discipline, leap-at-boundary, CUC P-field/T-field framing, PPS backend vtable |
| 0.3 | 2026-10-10 | frick-fsw | Review closure: source-switch semantics, first-tone +1 s convention, CUC two exchange forms (explicit 7-octet vs PUS 6-octet T-field), leap no-tone clause, atomics, edge-qualification disposition |
