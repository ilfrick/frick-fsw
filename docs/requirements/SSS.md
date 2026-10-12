# Software System Specification (SSS)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SSS-001                          |
| Revision                          | 0.14                                   |
| Date                              | 2026-10-12                             |
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
- **SSS-REQ-022** — *On-board autonomy (OBCP)*: the framework shall be able to
  execute user-provided On-Board Control Procedures written in **uPython/
  MicroPython**, uploaded on board, within bounded resources and with ECSS-compliant
  behaviour (isolation, time budget, abort, reporting; OBCP state/management via
  PUS S18). `method: Tst | Dem`.
- **SSS-REQ-023** — *Reference mission services*: the framework shall provide, as
  optional reference services on the PUS, an **equipment manager** (device
  command/monitoring/reconfiguration, redundancy) and **fault management** (limit
  checking, stored command sequences, memory/file management). `method: Tst | Dem`.
- **SSS-REQ-024** — *Multicore / SMP*: the system shall execute on the target's
  multi-core RISC-V cluster (RTEMS SMP) with correct concurrency and cache
  coherency. `method: Tst | Dem`.
- **SSS-REQ-025** — *Fundamental PUS-C service capability set*: the system shall
  provide the fundamental PUS-C service set (S1/3/5/6/8/9/11/12/17/18-surface/
  19/20) with the associated packet/state behaviour and a bounded on-board
  parameter datapool, on the default ASN.1/ACN data model. `method: Tst | Anl`.
- **SSS-REQ-026** — *On-board file delivery and storage*: the system shall provide
  a **CFDP** entity (CCSDS 727.0-B) delivering files to/from the ground over the
  mission link, on a bounded **virtual filestore (VFS)** with a pluggable storage
  backend (RAM/flash/eMMC/SSD); PUS-C **S23** file management shall operate on the
  same VFS. `method: Tst | Dem`.

- **SSS-REQ-027** — *On-board autonomy — in-house engine*: the system shall
  execute On-Board Control Procedures on an OBCP engine developed within the
  project (in addition to an integrated third-party engine, SSS-REQ-022), within a
  bounded resource budget (bounded engine memory and one execution context per
  OBCP), a bounded per-OBCP execution-time budget and a bounded abort latency,
  with OBCP state exposed for management (PUS S18 seam). `method: Tst | Dem`.
  (milestone M16)

- **SSS-REQ-028** — *PUS-C transport over IP and TSN*: the system shall carry
  PUS-C TM/TC over IP-based links (TCP or UDP per the mission ICD) and over
  TSN-scheduled Ethernet, in addition to the reference UART link, with the active
  link selectable by the integrator and the PUS-C layer kept transport-agnostic.
  `method: Tst | Dem`. (milestone M4-residual)

- **SSS-REQ-029** — *Extended fieldbus and PUS-library coverage*: the system shall
  provide, as optional reference components, CAN-FD (via the generic fabric
  contract), a CANopen (CiA 301/302/402) stack, RS-485 half-duplex direction
  control, and a documented adapter for the commercial PUSopen(R) PUS library.
  `method: Tst | Dem`. (milestone M4-residual)

## 3. Derived system requirements

- **SSS-REQ-014** (derived) — the reference communication link is a UART
  byte stream (`loopback` on host, `uart` on target); the transport codec and
  the link layer are decoupled to allow SpaceWire/radio back-ends later.
  `derived justification: ADR-0003/ADR-0009, ICD-GND`.
- **SSS-REQ-015** (derived) — the framework shall expose the open PUS backend
  and the N7/ESA ASN.1 default backend (the PUSopen(R) adapter is covered by
  SSS-REQ-029). `derived justification: norm N1 shows auditable defaults;
  ADR-0003/ADR-0016`.

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
| 0.9 | 2026-10-10 | frick-fsw | Add SSS-REQ-022 on-board autonomy (OBCP) and SSS-REQ-023 reference mission services |
| 0.10 | 2026-10-11 | frick-fsw | Add SSS-REQ-024 multicore / SMP |
| 0.11 | 2026-10-11 | frick-fsw | Add SSS-REQ-025 fundamental PUS-C service capability set (M15) |
| 0.12 | 2026-10-11 | frick-fsw | M15: add SSS-REQ-026 CFDP + virtual filestore |
| 0.13 | 2026-10-11 | frick-fsw | M16: add SSS-REQ-027 in-house OBCP engine development |
| 0.14 | 2026-10-12 | frick-fsw | Restore explicit M4 scope: SSS-REQ-028 PUS-C over IP/TSN; SSS-REQ-029 extended fieldbus + PUSopen adapter; reconcile SSS-REQ-015 (open + N7/ESA default backends; PUSopen adapter planned, library user-supplied) — FRICK-REVIEW-0137 R3-01/R3-02 |
