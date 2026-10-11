# ECSS Criticality Class A - Gap Analysis

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-A-001                       |
| Revision                          | 0.5                                    |
| Date                              | 2026-10-11                             |
| Norm                              | N5 (gaps to reach class A documented)  |
| Tracking                          | GAP-A1..A14 in `docs/requirements/traceability.yaml` (GAP-A1b is a sub-item of GAP-A1, not separately tracked) |

The current baseline targets ECSS criticality **B** (`ecss-criticality-b.md`).
The following gaps must be closed, together with the class-B baseline, to
deliver this software at criticality **A**. Each gap is tracked in the
traceability manifest (level `gap`, `justification` records the disposition).

## GAP-A1 - Deep formal verification

**Status:** open (class-A formal verification work package)

- Arbitrary-length CRC-16/CCITT-FALSE induction (currently proved by
  `native_decide` on reference frames; see `verification/lean/Frick/Ccsds.lean`).
- Full formal embedding of the C dispatch/memory model into Lean, and
  verification of the boot and app-lifecycle contracts.
- Propositional and functional correctness proofs for every SRS requirement
  that is today justified by `lean_note`.

**Closure plan / exit criteria (when will GAP-A1 be closed):**
- **(a) C refinement:** the pure-logic C modules are proven to implement the Lean
  model — via **Frama-C/CBMC** (deductive/verification) or a whole-program
  extraction — not merely "mirrored definitionally close".
- **(b) Requirement discharge:** every SRS requirement is discharged by a theorem
  or, where not formalisable, a justified `lean_note` accepted in independent
  review (no silent waiver).
- **Milestone linkage:** work package **WP-FV-100 / OBJ-15**, executed from **M5**
  (documentation/formal program) and **completed within M8**; it is a **gated
  work package, not a calendar date** — it does not start before the M4/M8 core
  (drivers/network) is stable, and it must be complete before any **class-A**
  claim is made.
- **Owner:** frick-fsw. **Tracking:** this document + `docs/status/loop-audit.md`
  (norm N19). **Tooling:** Frama-C/CBMC are build-time analysis tools (not
  distributed); Lean 4 core (no Mathlib).

## GAP-A1b - ASN1SCC generation in the build (PUS-C ASN.1 default; sub-item of GAP-A1)

The PUS-C ASN.1 default (SRS-REQ-050) requires the **ASN1SCC** compiler at build
time; ASN1SCC is not distributed as a tool in the V&V image. Handled by
`tools/docker/asn1scc/Dockerfile` (pinned) + `tools/scripts/gen_pus_asn1.sh`
(tool T19); generation is validated and the code is git-ignored.
**Delivered (ADR-0016):** the generated code is consumed by CMake and
`FF_PusBackendN7` is the default (`FF_USE_PUS_ASN1=ON`); the former residual is
closed. Tracked as a sub-item of GAP-A1 (no separate manifest entry); not a
class-A flight gap (build-time tool, re-qualified as T19).

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

- MISRA-C:2012 compliance evidence: an open-source **cppcheck MISRA scan**
  (`tools/scripts/misra.sh`, advisory → blocking) and a declared **coding style**
  (`.clang-format`, `docs/engineering/coding-style.md`) are in place; a
  **certified MISRA checker** and the formal compliance report remain the gap.
- WCET and stack-depth analysis for the RTEMS/MPFS target.
- Runtime platform qualification ("lowest terms") for the toolchain builder and
  BSP.
- Failure-mode and fault-injection test campaigns.

## GAP-A5 - On-target qualification

**Status:** open (blocked on hardware)

- RTEMS BSP qualification tests on the RTPFS160ZT board. **Emulated execution**
  (Renode `polarfire-soc` / QEMU `riscv/rv64imafdc`, milestone M10,
  `docs/vv/emulated-target.md`) partially mitigates this without hardware; the
  real-silicon residual remains open.
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

## GAP-A8 - Software Development Plan formalisation

**Status:** open (process artefact)

- The planning content (lifecycle, tailoring, cycle, reviews, gates) exists in
  `docs/development-plan.md`, `docs/roadmap.md` and `docs/tooling.md`; the
  ECSS-SDP formal title/tailoring record (with the criticality-B tailoring of
  each process step) is closable by extending those documents.

## GAP-A9 - SDD detailed design completeness

**Status:** open (documentation work package)

- `docs/design/software-design.md` gives the architectural and module-level
  design. Per-function detailed design (data structures, algorithms, error
  cases) for every module is closable incrementally.

## GAP-A10 - Full software safety analysis (FMEA/FMECA)

**Status:** open (needs system hazard inputs)

- `docs/quality/safety-and-fdir.md` is a preliminary contribution analysis; a
  full FMEA/FMECA requires the system-level hazard inputs from the programme.

## GAP-A11 - Independent verification of unit tests by a third party

**Status:** open (organisational, related to GAP-A3)

- Unit-test and evidence review today is by independent-context agents (R2);
  third-party/organisational independent verification of the test campaigns is
  a company/programme function.

## GAP-A12 - Operations manual and TM/TC database (OPD)

**Status:** open (mission tailoring)

- The PUS service catalogue and the integration manual exist, but a mission
  operations manual and the ground TM/TC database are mission-specific and not
  part of the framework deliverable.

## GAP-A13 - Maintenance and obsolescence plan

**Status:** open (programme work package)

- Corrective/adaptive maintenance and third-party obsolescence are addressed by
  the SCMP (`docs/quality/configuration-management.md` §6) and licensing review;
  a formal long-term maintenance/obsolescence plan is a programme artefact.

## GAP-A14 - SDLS cryptographic algorithm qualification and key store

**Status:** open (mission/HW specific)

- The middleware implements the CCSDS SDLS security-association and
  key-management software and the SDLS processing, but does not implement a
  cipher. A **certified cryptographic algorithm** (CCSDS 352.0-B: e.g.
  AES-GCM/CMAC), a **hardware keystore** with a physically protected zeroisation
  path, an **IV/counter uniqueness analysis**, and the **ground
  key-management/key-distribution procedures** are mission/ground artefacts and
  must be qualified independently. See `docs/architecture/security-sdls.md`.

## Closure policy

Each gap has an owner and an acceptance criterion assigned by the programme; the
class-A work packages are mapped in `docs/quality/ecss-compliance-matrix.md` and
scheduled in `docs/roadmap.md`. Gap closure is recorded in this document, in the
traceability manifest and in the V&V results, and is reviewed independently per
norm N9/N12 before the class-A baseline can be claimed.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial gap analysis GAP-A1..A6 (norm N5) |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: add GAP-A7 on-target PPS and TAI/CCSDS epoch |
| 0.3 | 2026-10-10 | frick-fsw | ECSS compliance assessment: add GAP-A8..A13 (SDP, SDD depth, FMEA, third-party IVV, OPD, maintenance) |
| 0.5 | 2026-10-11 | frick-fsw | GAP-A1b reframed as a sub-item of GAP-A1 (delivered, ADR-0016); tracking clarified (PA-08) |
| 0.4 | 2026-10-10 | frick-fsw | M7: add GAP-A14 (SDLS crypto algorithm qualification and key store) |
