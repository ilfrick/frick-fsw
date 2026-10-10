# Software System Specification (SSS)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SSS-001                          |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Applicable standards              | ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 |
| Target platform                   | Microchip RT PolarFire SoC RTPFS160ZT (FCVG784), RISC-V RV64GC |
| RTOS baseline                     | RTEMS 6.2 (embedded brains ECSS pre-qualified baseline) |
| Software criticality (target)     | ECSS Category B                        |

The Software System Specification fixes the system-level (end item) behaviour
of frick-fsw. It is the top of the bidirectional traceability chain:
`SSS -> SRS -> implementation -> tests -> Lean theorems` and back.

## 1. Mission context

frick-fsw is a generic, cFS-inspired flight software framework intended for a
small to medium spacecraft on-board computer built around the Microchip
Radiation-Tolerant PolarFire SoC FPGA (part number RTPFS160ZT). It provides:

- command and data handling (C&DH),
- packet utilisation service layer (PUS-C) on CCSDS Space Packets,
- housekeeping, event reporting, time, health & safety and table management,
- a deterministic static-allocation execution environment on RTEMS 6.

## 2. System requirements

- **SSS-REQ-001** — *Flight target*: the flight software shall execute on the
  Microchip RT PolarFire SoC RTPFS160ZT, RISC-V U54 application cluster, under
  RTEMS 6.2 (embedded brains ECSS pre-qualified baseline), using the
  `riscv/mpfs64imafdc` BSP as reference port. `method: Dem`.
- **SSS-REQ-002** — *Software bus*: the software shall provide a
  publish/subscribe software bus with configurable pipe depth. `method: Tst | Anl`.
- **SSS-REQ-003** — *TM/TC chain*: the software shall accept PUS-C
  telecommands and produce PUS-C telemetry wrapped in CCSDS Space Packets
  with CRC-16 protection. `method: Tst | Dem`.
- **SSS-REQ-004** — *Request verification*: every acknowledged telecommand
  shall be tracked by PUS service 1 verification telemetry. `method: Tst | Anl`.
- **SSS-REQ-005** — *Housekeeping*: the software shall collect and report
  housekeeping structures (PUS service 3). `method: Tst | Dem`.
- **SSS-REQ-006** — *Event reporting*: on-board events shall be visible to the
  ground (PUS service 5). `method: Tst | Dem`.
- **SSS-REQ-007** — *Time management*: UTC correlation and CUC time encoding
  (PUS service 9). `method: Tst | Dem`.
- **SSS-REQ-008** — *Health & safety*: the software shall monitor application
  health, generate heartbeat events and support a commandable software reset.
  `method: Tst | Dem | MIR`.
- **SSS-REQ-009** — *Tables*: the software shall support load/validate/
  activate of on-board configuration tables. `method: Tst`.
- **SSS-REQ-010** — *Determinism*: no dynamic memory allocation after
  initialisation. `method: Anl | Inp`.
- **SSS-REQ-011** — *Multi-target build*: identical framework sources shall
  build for the host simulator and the flight target. `method: Dem | Anl`.
- **SSS-REQ-012** — *Doc set*: the delivered documentation set (SSS, SRS,
  traceability matrices, V&V records) shall satisfy ECSS Category B, and the
  residual gap to Category A shall be documented. `method: Inp | MIR`.
- **SSS-REQ-013** — *Continuous verification*: every committed change shall
  pass the automated pipeline (build, tests, Lean checks, traceability
  convergence) on both developers and CI. `method: Inp | Dem`.

## 3. Derived system requirements

- **SSS-REQ-014** (derived) — the reference communication link is a UART
  byte stream (`loopback` on host, `uart` on target); the transport codec and
  the link layer are decoupled to allow SpaceWire/radio back-ends later.
  `derived justification: ADR-0003/ADR-0009, ICD-GND`.
- **SSS-REQ-015** (derived) — the framework shall expose both an open PUS
  backend (default) and a documented adapter for the commercial PUSopen(R)
  library. `derived justification: norm N1 shows auditable defaults; ADR-0003`.

## 4. Traceability

See `docs/requirements/traceability.md` and `traceability.yaml`; gaps and
deviations are resolved before every commit (norm N4).

## 5. History

| Rev | Date       | Author    | Reason                          |
| --- | ---------- | --------- | ------------------------------- |
| 0.1 | 2026-10-10 | frick-fsw | Initial release (backward from code) |