# Software System Specification (SSS)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SSS-001                          |
| Revision                          | 0.8                                    |
| Date                              | 2026-10-10                             |
| Applicable standards              | ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 |
| Target platform                   | Microchip RT PolarFire SoC RTPFS160ZT (FCVG784), RISC-V RV64GC |
| RTOS baseline                     | RTEMS 6.2 (open-source RTEMS project) |
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
  RTEMS 6.2 (open-source RTEMS project), using the
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
- **SSS-REQ-007** — *Time management*: UTC correlation and CUC time reporting
  and setting through PUS service 9, independent of any hardware reference
  (CUC encoding/reporting boundary; the disciplined time base and its CUC frame
  tailoring are SSS-REQ-016). `method: Tst | Dem`.
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
- **SSS-REQ-016** — *Hardware-disciplined time base*: the system time shall be
  disciplinable from an external 1 Hz / PPS reference: the software shall
  provide the capability to switch, at run time, between the internal CPU
  clock, an external tone and a ground-correlation source, so that the exported
  CUC time is disciplined at every one-second boundary and scheduled leap-second
  changes are applied exactly at that boundary; the CUC frame tailoring shall
  pin a 4+2 octet agency-epoch frame (P-field 0x2E) for the on-wire forms.
  `method: Tst | Anl`.
- **SSS-REQ-017** — *Hardware abstraction and integration interfaces*: the
  software shall expose a documented hardware abstraction layer (HAL) with
  stable, stack-independent contracts, so that users can integrate their own
  drivers and third-party stacks (Ethernet with pluggable TCP/IP and TSN,
  CAN/CANopen/CAN-FD, RS-422/RS-485, SPI, I2C, PCIe, GPIO, SpaceWire, and
  memory/storage resources) without modifying the framework (OS primitives are
  abstracted separately by the PAL). `method: Inp | Anl`.
- **SSS-REQ-018** — *Numerical support*: the framework shall provide a
  documented, portable numerical/mathematics interface (based on the platform
  `libm`) usable by applications, with the determinism and licensing
  implications documented. `method: Tst | Inp`.
- **SSS-REQ-019** — *Data-link security (SDLS)*: the system shall provide CCSDS
  Space Data Link Security (SDLS, CCSDS 355.0-B) processing for telemetry and
  telecommand, with security-association management and key management
  (session-key derivation, rotation and zeroisation), keeping the cryptographic
  algorithm a pluggable, mission-supplied backend. `method: Tst | Anl | Inp`.
- **SSS-REQ-020** — *Reference low-level implementations*: the framework shall
  provide reference implementations of the HAL contracts for the target platform
  (starting with the Gigabit Ethernet MAC) as separable, optional components,
  keeping the middleware independent of them. `method: Tst | Dem`.
- **SSS-REQ-021** — *Code quality and compliance*: the software shall follow a
  single declared C coding style applied and enforced codebase-wide, and shall
  be MISRA C:2012 compliant with a documented deviation process. `method: Inp`.

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
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: add SSS-REQ-016 hardware-disciplined time base |
| 0.3 | 2026-10-10 | frick-fsw | R3 review closure: SSS-REQ-016 capability wording and SSS-REQ-007/016 boundary |
| 0.4 | 2026-10-10 | frick-fsw | M3: add SSS-REQ-017 hardware abstraction and integration interfaces |
| 0.5 | 2026-10-10 | frick-fsw | M6: add SSS-REQ-018 numerical support (libm facade) |
| 0.6 | 2026-10-10 | frick-fsw | M7: add SSS-REQ-019 data-link security (CCSDS SDLS) |
| 0.7 | 2026-10-10 | frick-fsw | M4: add SSS-REQ-020 reference low-level implementations |
| 0.8 | 2026-10-10 | frick-fsw | M9: add SSS-REQ-021 code quality and compliance (MISRA + coding style) |