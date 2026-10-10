# Software Requirements Specification (SRS)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SRS-001                          |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Applicable standards              | ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 |
| Software criticality (target)     | ECSS Category B                        |
| Non-derogable norms               | N1 (Lean), N2 (English), N3 (SRS/SSS traced), N4 (traceability convergence), N5 (class B + A gaps), N6 (V&V), N7 (dual push) |

This document is generated backwards from the frick-fsw architecture and
implementation. Every requirement is machine-checked against the
traceability manifest `docs/requirements/traceability.yaml` by `tools/scripts/trace.py`
(norm N3/N4), and its verification is connected to a Lean-4 theorem where it
applies (norm N1). Traceability is bidirectional and is re-run at every
change.

Verification method codes: **Tst** (test), **Anl** (analysis), **Inp**
(inspection), **Dem** (demonstration), **MIR** (mission/flight readiness
review).

## 1. General requirements

- **FRICK-SRS-REQ-000** (functional baseline): the software shall implement
  the architecture documented in `docs/architecture/` and shall be buildable
  from a fresh clone with the documented toolchain. `method: Dem`.

## 2. Software Bus requirements

- **SRS-REQ-001** — *Delivery semantics*: a published message shall be copied
  into the input queue of every pipe whose subscription matches the message
  identifier, and of every wildcard subscription. `method: Tst`, `lean: req_srs001_*`.
- **SRS-REQ-002** — *Ordering*: messages delivered on a single pipe shall be
  received in FIFO order. `method: Tst | Anl`.
- **SRS-REQ-003** — *Overflow safety*: for any frame of up to `FF_PUS_MAX_*_FRAME`
  bytes the CCSDS Packet Data Length field shall not overflow (16 bit).
  `method: Anl`, `lean: req_srs003_*`.

## 3. CCSDS / PUS transport requirements

- **SRS-REQ-004** — *Integrity*: every TM and TC frame shall carry a
  CRC-16/CCITT-FALSE trailer; the decoder shall reject frames whose CRC does
  not match. `method: Tst`, `lean: req_srs004_*`.
- **SRS-REQ-005** — *TC acceptance*: a telecommand with an unknown service or
  subservice, or a malformed frame, shall not be executed and shall produce a
  PUS service 1 completion failure with the appropriate error code.
  `method: Tst | Anl`.
- **SRS-REQ-006** — *Verification service*: every acknowledged stage
  (acceptance/start/progress/completion) of an accepted telecommand shall
  produce exactly one service 1 telemetry packet. A TC acknowledged for
  acceptance and completion produces at least two packets.
  `method: Tst`, `lean: req_srs006_*`.
- **SRS-REQ-007** — *TM counter*: the PUS telemetry message type counter
  shall strictly increase and wrap modulo 16384. `method: Anl`, `lean: req_srs007_*`.

## 4. PUS service requirements

- **SRS-REQ-008** — *Housekeeping (S3)*: housekeeping structures shall be
  created/enabled/disabled/removed by ground command and reported on request.
  `method: Tst | Dem`.
- **SRS-REQ-009** — *Memory supervision (S6)*: a memory dump command shall
  only access addresses inside registered memory regions. `method: Tst | Anl`.
- **SRS-REQ-010** — *Time (S9)*: the UTC correlation shall be settable from
  ground and reported in CUC format. `method: Tst | Dem`.
- **SRS-REQ-011** — *Events (S5)*: every on-board event (EVS) shall be
  forwarded to ground as PUS service 5 telemetry. `method: Tst | Dem`.
- **SRS-REQ-012** — *Test (S17)*: a connection-test telecommand (ping) shall
  be answered with the same data on service 17 subservice 2. `method: Tst`.

## 5. Operational requirements

- **SRS-REQ-013** — *Tables*: table activation shall be atomic (a staged image
  becomes active only on explicit activate). `method: Tst`.
- **SRS-REQ-014** — *Lifecycle*: applications shall run as independent tasks;
  Executive Services shall track states; Health & Safety shall detect failed
  applications. `method: Tst | Inp`.
- **SRS-REQ-015** — *Deterministic memory*: the flight software shall not
  allocate dynamic memory after initialisation. `method: Anl | Inp`.
- **SRS-REQ-016** — *Portability*: the same framework source shall build for
  the host simulator and for the RTEMS 6 RTPFS160ZT target via the PAL.
  `method: Dem`.

## 6. Derived requirements

The following are derived from the applicability of ECSS Category B and from
mission tailoring of PUS-C; their derivation justification is recorded in the
traceability manifest (`derived: true`), per norm N4.

- **SRS-REQ-017** (derived) — TM/TC frames use CCSDS Space Packets (APPID
  0x010 / 0x020) with mission identifier 0x01. `derived justification: ECSS-E-ST-70-41C tailoring, doc ICD-GND`.
- **SRS-REQ-018** (derived) — the reference build uses the open backend for
  PUS; PUSopen(R) is an optional drop-in backend. `derived justification: ADR-0003`.
- **SRS-REQ-019** (derived) — configuration is concentrated in
  `config/fsw_config.h` plus mission tables. `derived justification: cFS heritage, ADR-0008`.
- **SRS-REQ-020** (derived) — the project shall maintain an always-current
  documentation set that allows building, running and verifying the framework
  on a developer machine and in CI, by human users and by automated agents
  (norm N6/N8). `derived justification: norms N6/N8; AGENTS.md quickstart and CI jobs act on the docs.`

## 7. Conformance and traceability

- Traceability manifest: `docs/requirements/traceability.yaml`.
- Verification records and matrices: `docs/requirements/traceability.md`.
- Class B target baseline and Class A gap analysis: `docs/quality/ecss-criticality-b.md`,
  `docs/quality/class-a-gaps.md`.
- V&V lifecycle evidence: `docs/vv/verification-plan.md`, `docs/vv/verification-results.md`.

## 8. History

| Rev | Date       | Author     | Reason                          |
| --- | ---------- | ---------- | ------------------------------- |
| 0.1 | 2026-10-10 | frick-fsw  | Initial release (backward from code, Lean-checked) |