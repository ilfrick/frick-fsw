# ECSS Criticality Class A - Gap Analysis

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-A-001                       |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norm                              | N5 (gaps to reach class A documented)  |
| Tracking                          | GAP-A1..A6 in `docs/requirements/traceability.yaml` |

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

## Closure policy

Each gap has an owner, a work package and an acceptance criterion set in the
roadmap (`docs/roadmap.md`). Gap closure is recorded in this document, in the
traceability manifest and in the V&V results, and is reviewed independently per
norm N9/N12 before the class-A baseline can be claimed.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial gap analysis GAP-A1..A6 (norm N5) |