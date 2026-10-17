# ADR-0020 - Provisional public-reference PUS and scheduler policies

| Status | Accepted as a framework engineering reference; not a mission baseline |
| ------ | --------------------------------------------------------------- |
| Date   | 2026-10-16 |

## Context

The S8 corrective work has unresolved PUS profile and scheduler time-boundary
inputs (NC-EXT-14..19 and NC-EXT-90). The project owner directed the FSW author
to select examples from public industry sources, apply them, and document the
choice with an appropriate disclaimer. This ADR records a bounded framework
reference so implementation work can proceed without representing the choice as
a mission authority decision.

## Decision

### PUS reference

- Use **ECSS-E-ST-70-41C (15 April 2016)** as the normative public reference for
  generic PUS-C service semantics in corrective implementation and tests.
- Use the public PUSopen service documentation as a secondary implementation
  cross-check only, never as a normative source or proof of interoperability.
- For Service 1, use the ECSS request-identifier composition (source identifier,
  destination identifier, and sequence count or request name) and the ECSS
  distinction between requested successful-verification reports and failed
  verification reports.
- For Service 3, use the ECSS standardized service/subservice definitions and
  report semantics where the framework implements them; do not invent periodic
  report timing or mission report allocations. The bounded TC[3,27] handler
  accepts a provisional `N:u8` followed by exactly N report-structure IDs;
  one-shot requests are honored independently of periodic-generation status.
- For the framework's bounded Service 6 dump path, use a provisional payload
  profile of `region_id:u8` (zero reserved), `byte_offset:u32` big-endian, and
  `length:u16` big-endian. Map the ID to a configured logical memory region;
  validate `offset <= region_size` and `length <= region_size - offset` before
  access. The host/target pointer is never placed on the wire. This explicit
  project choice follows the public ECSS logical-memory-ID/byte-address model
  and the public PUSopen logical-memory example, but the field widths/layout
  remain a **provisional project profile**, not a claim that ECSS mandates them
  or that they match any mission ICD.

### Scheduler time exhaustion reference

- Use checked, non-wrapping deadline arithmetic. If an initial or subsequent
  absolute release target cannot be represented in the `uint64_t` monotonic
  nanosecond domain, do not wrap it into an apparently early time.
- The provisional fail-closed behavior is to disable that entry for the current
  dispatcher initialization epoch and increment a visible time-exhaustion
  counter. Only an explicit `FF_SchedInit` reinitializes release targets;
  activation clears targets and therefore must be followed by `FF_SchedInit`
  before dispatch resumes. The missed-slot diagnostic counter saturates at
  `UINT32_MAX` rather than wrapping. No automatic restart, reset, or mission
  safe-state action is implied.
- This follows the general checked-time-arithmetic pattern exposed by public
  operating-system APIs (for example Linux `timespec64_add_safe`). It does not
  adopt Linux kernel timekeeping or claim RTEMS target behavior.

## Disclaimer and applicability boundary

These are **provisional framework defaults for implementation and host
verification only**. ECSS-E-ST-70-41C explicitly treats PUS service selection
and tailoring as mission-specific and expects the result in the Space-to-Ground
Interface Control Document. This ADR is not that ICD and does not establish
system-authority approval, a mission PUS profile, ECSS compliance, packet
interoperability, safe memory access authorization, scheduler schedulability,
target timing, or flight suitability. NC-EXT-14..19/90 remain open until the
relevant authority accepts or replaces these defaults and the applicable
independent reviews and evidence pass. Mission procedures and mission use must
not be enabled from this ADR alone.

## Public sources checked 2026-10-16

1. ECSS, **ECSS-E-ST-70-41C – Telemetry and telecommand packet utilization**,
   15 April 2016. Issuer page and downloadable standard:
   <https://ecss.nl/standard/ecss-e-st-70-41c-space-engineering-telemetry-and-telecommand-packet-utilization-15-april-2016/>.
    Relevant text: scope/tailoring; §5.4.3.3 logical memories, identifiers,
    alignment, size, operations and byte addressing; §5.4.11.2.1(c) request
    identifier; §§6.1 and 6.3 Service 1 and Service 3; §6.6 Service 6.
   Retrieved issuer PDF SHA-256:
   `fed627dae2aa2a1e9dbfb873861002fbee40a37a3883019d6701af34852ea51d`.
2. PUSopen, **PUS Services**, public 1.3.19 documentation, accessed
   2026-10-16: <https://pusopen.com/api/pus.html>. Version 1.3.19 as identified
   by the served documentation. Used only as an implementation cross-check; it
   documents service-specific supported messages and a logical memory ID model,
   but is not the governing project profile. No archival copy/hash is retained.
3. Linux kernel documentation, **Driver Basics — Time and timer routines**,
   served docs version 7.3.0-rc7, accessed 2026-10-16:
   <https://docs.kernel.org/driver-api/basics.html>. Public API reference for
   `timespec64_add_safe`; used only as an example of checked time arithmetic.
   No archival copy/hash is retained.
4. RTEMS 6.2 C User Manual, **Clock Manager — Introduction**, accessed
   2026-10-16: <https://docs.rtems.org/docs/6.2/c-user/clock/introduction.html>.
   Confirms the distinction between real-time and monotonic clocks; does not
   establish the project's clock range or exhaustion policy. No archival
   copy/hash is retained.

## Change history

| Revision | Date | Author | Change |
| -------- | ---- | ------ | ------ |
| 0.1 | 2026-10-16 | frick-fsw | Record owner-directed provisional public references and explicit mission-authority disclaimer |
