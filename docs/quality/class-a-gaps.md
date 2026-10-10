# ECSS Criticality Class A - Gap Analysis

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-A-001                       |
| Revision                          | 0.2                                    |
| Date                              | 2026-10-10                             |
| Norm                              | N5 (gaps to reach class A documented)  |
| Tracking                          | GAP-A1..A7 in `docs/requirements/traceability.yaml` |

The current baseline targets ECSS criticality **B** (`ecss-criticality-b.md`).
The following gaps must be closed, together with the class-B baseline, to
deliver this software at criticality **A**. Each gap is tracked in the
traceability manifest (level `gap`, `justification` records the disposition).

## GAP-A1 - Deep formal verification

**Status:** open (work package WP-FV-A)

- Arbitrary-length CRC-16/CCITT-FALSE induction (currently proved by
  `native_decide` on reference frames; see `verification/lean/Frick/Ccsds.lean`).
- Full formal embedding of the C dispatch/memory model into Lean, and
  verification of the boot and app-lifecycle contracts.
- Propositional and functional correctness proofs for every SRS requirement
  that is today justified by `lean_note`.

## GAP-A2 - Full PUS-C service coverage

**Status:** open

- Formalisation and per-service unit coverage for the whole PUS-C service set
  (S1 verification, S3 housekeeping, S5 events, S6 memory, S8 function
  management, S9 time, S17 test, S20 parameter, ...).
- Independent tests for each service and subservice combination, including
  malformed-frame and out-of-range subchecks.

## GAP-A3 - Independent SES and QA organisation

**Status:** open

- Organisationally independent software engineering support (SES) and QA
  function per ECSS-Q-ST-80C; records of independence (norm N9 agents today
  provide the technical independence of reviews, but organisational
  independence is a project/company function).
- Full SES agenda, independent verification of every deliverable, QA audits.

## GAP-A4 - Code assurance and runtime evidence

**Status:** open

- MISRA-C:2012 compliance evidence (today: `-Wall -Wextra`, `.clang-tidy`).
- WCET and stack-depth analysis for the RTEMS/MPFS target.
- Runtime platform qualification ("lowest terms") for the toolchain builder and
  BSP.
- Failure-mode and fault-injection test campaigns.

## GAP-A5 - On-target qualification

**Status:** open (blocked on hardware)

- RTEMS BSP qualification tests on the RTPFS160ZT board.
- Link-level PUS TM/TC verification over the real UART plus loopback on target.
- Hardware-in-the-loop closed-loop scenarios (housekeeping, time, HS recovery).

## GAP-A6 - Environmental / radiation campaign

**Status:** open (out of software scope)

- SEE/radiation characterisation of the RTPFS160ZT and memory scrub schemes.
- Environmental qualification and long-duration soak testing.

## GAP-A7 - On-target PPS capture and CCSDS/TAI epoch

**Status:** open (blocked on hardware)

- A hardware PPS backend for the RTEMS/MPFS target (PLIC GPIO capture ISR
  latching the timestamp and deferring to a task that calls
  `FF_TimeToneSignal()`) is today served by the software backend on the host
  simulator; the driver hook exists but needs the flight board to be validated
  (see `docs/engineering/driver-development.md`). The **edge-qualification
  criterion to be provided by the front-end/driver** at board bring-up is:
  reject edges closer than a configurable fraction of the nominal 1 s tone
  interval (spurious-edge / glitch removal) before the hook; the core enforces
  strict monotonicity and the one-second epoch advance, while tone-spacing
  plausibility remains a front-end responsibility (R1 review NC-06).
- The CUC uses an agency-defined epoch on the simulator; a CCSDS 1958 (level 1)
  / TAI epoch correlation and a leap-second table require on-board validation.

## Closure policy

Each gap has an owner, a work package and an acceptance criterion set in the
roadmap (`docs/roadmap.md`). Gap closure is recorded in this document, in the
traceability manifest and in the V&V results, and is reviewed independently per
norm N9/N12 before the class-A baseline can be claimed.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial gap analysis GAP-A1..A6 (norm N5) |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: add GAP-A7 on-target PPS and TAI/CCSDS epoch |