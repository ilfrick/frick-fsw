# Software Requirements Specification (SRS)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SRS-001                          |
| Revision                          | 1.3                                    |
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

### Time base (milestone M2)

- **SRS-REQ-021** — *Time source selection*: the time base shall support an
  internal (CPU clock), a tone (external 1 Hz / PPS) and a ground-correlation
  source; the active source shall be selectable at run time by command and be
  reportable. Selecting a source is an explicit authority change: after the
  switch the exported CUC is continuous with the newly selected source and may
  step by at most one second at the switch instant; across a tone boundary the
  exported CUC seconds shall not regress under a non-negative scheduled leap
  change. `method: Tst | Anl`.
- **SRS-REQ-022** — *Tone discipline*: each valid 1 Hz tone shall advance the
  tone second counter (the disciplined second epoch) by exactly one second and
  strictly increase the tone epoch; non-monotonic tones shall be rejected. Under
  TONE mode the exported CUC coarse seconds shall equal the tone second counter
  plus the accumulated leap offset (SRS-REQ-023); the first tone establishes the
  first disciplined second boundary (`latched correlation + 1 s`). `method: Tst | Anl`.
- **SRS-REQ-023** — *Scheduled leap second*: a pending leap change (single
  second at a time) shall be inert: it shall never alter the exported CUC while
  pending, and shall be applied and cleared exactly at the first valid tone that
  follows; if no valid tone ever arrives the pending leap shall remain pending
  and shall never be applied. `method: Tst | Anl`.
- **SRS-REQ-024** — *CUC framing*: the CCSDS unsegmented time code shall exist
  in two exchange forms: (a) the self-describing 7-octet code (P-field 0x2E,
  4-octet coarse, 2-octet fine) implemented by `FF_TimeCucEncode/Decode`, which
  shall be lossless and shall reject wrong-length and wrong-P-field inputs; and
  (b) the PUS-C on-wire 6-octet T-field (P-field implicit by interface
  agreement, see ICD §5) used in the TM secondary header and service 9
  payloads, which the service layer shall accept only in its exact 6-octet
  length. `method: Tst | Anl`.

### Hardware abstraction (milestone M3)

- **SRS-REQ-025** — *HAL registry*: the abstraction layer shall provide a
  bounded, allocation-free registry that lets a backend be registered under a
  stable name for a resource class; an invalid class or a duplicate name shall
  be rejected and the registered count shall never exceed the configured
  capacity (`FF_HAL_MAX_BACKENDS`). `method: Tst | Anl`.
- **SRS-REQ-026** — *Ethernet MAC and net interface contract*: the framework
  shall define a contract for an Ethernet MAC and a network interface that binds
  a MAC to a network stack, independent of the specific PHY/MAC/controller.
  `method: Inp`.
- **SRS-REQ-027** — *Pluggable network stack contract*: the framework shall
  address the network only through a stack-independent contract, so that a user
  can integrate their own TCP/IP stack (for example Tuxera or an open stack)
  without modifying the framework. `method: Inp | Anl`.
- **SRS-REQ-028** — *TSN backend contract*: the framework shall define a backend
  contract for deterministic Ethernet (IEEE 802.1Q VLAN/PCP, 802.1Qbv, 802.1Qav,
  802.1AS), decoupled from the MAC and the stack. `method: Inp`.
- **SRS-REQ-029** — *CAN/CANopen/CAN-FD contract*: the framework shall define a
  CAN backend contract covering classic CAN (ISO 11898-1, CAN 2.0 A/B) and
  CAN-FD, plus a CANopen (CiA 301/302/402) stack contract layered on it.
  `method: Inp`.
- **SRS-REQ-030** — *Serial RS-422/RS-485 contract*: the framework shall define a
  serial contract with half-duplex RS-485 transceiver direction control.
  `method: Inp`.
- **SRS-REQ-031** — *SPI, I2C, PCIe and GPIO contracts*: the framework shall
  define SPI, I2C, PCIe and GPIO (with interrupt) contracts; the GPIO interrupt
  contract shall state that an ISR defers work to a task. `method: Inp`.
- **SRS-REQ-032** — *SpaceWire and memory/storage contracts*: the framework
  shall define SpaceWire (ECSS-E-ST-50-12 link, 50-51 protocol id, 50-52 RMAP,
  50-53 CCSDS packet transfer) and memory/storage contracts (RAM pool, DMA,
  NOR/flash MTD, non-volatile memory, block device). `method: Inp`.

### Numerical support (milestone M6)

- **SRS-REQ-033** — *Mathematics facade*: the framework shall expose a
  portable mathematics facade over the platform `libm` (elementary functions
  `sqrt/sin/cos/tan/atan2/pow/exp/log/fabs/fmod` plus angle and clamp helpers),
  shall link `libm`, and shall document the behaviour for out-of-domain
  arguments (for example `sqrt(x<0)=NaN`, `log(x<=0)=-inf`). Results shall be
  within the accuracy of the platform `libm`; the unit tests assert within a
  documented tolerance (<= 1e-9 for the covered inputs). `method: Tst | Inp`.

### Data-link security (milestone M7)

- **SRS-REQ-034** — *SDLS processing*: the security sub-layer shall apply a
  configurable SDLS security service (authentication-only, encryption-only,
  authenticated-encryption) to a data unit, producing and validating the SDLS
  header (SPI + IV) and message authentication code, through a **pluggable**
  cryptographic backend that the middleware does not implement. `method: Tst | Inp`.
- **SRS-REQ-035** — *Security-association and key management*: the sub-layer
  shall maintain a bounded, SPI-indexed security-association database and manage
  the key lifecycle: derive session keys from master keys, rotate them (the key
  epoch shall strictly increase and the usage counter reset; rotation on frame
  and time thresholds), and zeroise key material on rotation and removal.
  `method: Tst | Anl`.

### Reference low-level implementations (milestone M4)

- **SRS-REQ-036** — *Ethernet MAC reference driver (Cadence GEM)*: the framework
  shall provide a reference implementation of the Ethernet MAC HAL contract
  (`FF_EthMac_t`) for the Cadence GEM of the Microchip PolarFire SoC MSS, with
  static, allocation-free descriptor/buffer handling and host-testable register
  and descriptor logic (injected MMIO + DMA translation). TSN (802.1Q/Qbv/Qav/
  802.1AS, native on the GEM) is delivered through `FF_TsnBackend_t` in a
  separate planned increment. `method: Tst | Inp`.
- **SRS-REQ-037** — *RTEMS application integration*: the framework shall provide
  a complete RTEMS 6.2 integration for the `riscv/mpfs64imafdc` BSP — an
  application configuration table (bounded `CONFIGURE_*` incl. the POSIX objects
  required by the PAL) and the `POSIX_Init` entry point running the shared
  bring-up sequence — so that the same sources build for the RTEMS target.
  `method: Inp` (on-target demonstration is GAP-A5).
- **SRS-REQ-038** — *GEM native TSN backend*: the framework shall provide a
  reference TSN backend (`FF_TsnBackend_t`) over the Cadence GEM native TSN
  block: 802.1Q VLAN/PCP classification, 802.1Qbv time-aware scheduling (ENST
  gate control lists) and 802.1AS time from the GEM TSU; credit-based shaping
  (802.1Qav) is documented as not exposed in the GEM register subset. `method: Tst | Inp`.
- **SRS-REQ-039** — *Reference network stack*: the framework shall provide a
  reference implementation of the network-stack contract (`FF_NetStack_t`) that
  exercises the pluggable TCP/IP interface without external dependencies
  (bounded, static, in-process loopback), for unit tests and the host simulator.
  `method: Tst`.
- **SRS-REQ-040** — *lwIP network-stack adapter*: the framework shall provide a
  reference adapter implementing the network-stack contract (`FF_NetStack_t`)
  over the lwIP TCP/IP stack (socket API), so a mission can use lwIP without
  changing the middleware; built with `FF_USE_LWIP`. `method: Tst | Inp`.

### Code quality (milestone M9)

- **SRS-REQ-041** — *MISRA C:2012 compliance*: the C sources shall be MISRA
  C:2012 compliant; violations shall be fixed or raised as documented deviations
  per `docs/quality/misra-c.md`. `method: Inp | Anl`.
- **SRS-REQ-042** — *Coding style*: the C sources shall follow the single
  declared frick-fsw C style (`.clang-format` normative), applied and enforced
  codebase-wide by the format gate (`docs/engineering/coding-style.md`).
  `method: Inp`.
- **SRS-REQ-043** — *Emulated-target verification*: the project shall provide an
  emulated-target verification path that runs the RTEMS target binary on an
  emulated PolarFire SoC (Renode `polarfire-soc`) and/or QEMU
  (`riscv/rv64imafdc`), verifying the boot sequence, the PAL/RTEMS backend and
  the drivers without hardware, documented as a partial mitigation of GAP-A5.
  The execution path is **advisory** (runs when the emulator is available;
  records SKIP otherwise) and is qualified as a V&V tool (N10). `method: Dem | Inp`.

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
| 0.2 | 2026-10-10 | frick-fsw  | Milestone M2: add SRS-REQ-021..024 time source, tone, leap, CUC framing |
| 0.3 | 2026-10-10 | frick-fsw  | R3 review closure: SRS-REQ-021 switch semantics, SRS-REQ-022 tone second counter, SRS-REQ-023 pending/no-tone clause, SRS-REQ-024 two CUC exchange forms |
| 0.4 | 2026-10-10 | frick-fsw  | M3: add SRS-REQ-025..032 hardware abstraction (HAL registry, Ethernet/net/TSN, CAN, serial, SPI/I2C/PCIe/GPIO, SpaceWire, memory/storage) |
| 0.5 | 2026-10-10 | frick-fsw  | M6: add SRS-REQ-033 mathematics facade (libm) |
| 0.6 | 2026-10-10 | frick-fsw  | M7: add SRS-REQ-034 SDLS processing and SRS-REQ-035 SA/key management |
| 0.7 | 2026-10-10 | frick-fsw  | M4: add SRS-REQ-036 Ethernet MAC reference driver (Cadence GEM) |
| 0.8 | 2026-10-10 | frick-fsw  | M4: add SRS-REQ-037 RTEMS application integration |
| 0.9 | 2026-10-10 | frick-fsw  | M4: add SRS-REQ-038 GEM native TSN backend |
| 1.0 | 2026-10-10 | frick-fsw  | M4: add SRS-REQ-039 reference network stack |
| 1.1 | 2026-10-10 | frick-fsw  | M4: add SRS-REQ-040 lwIP network-stack adapter |
| 1.2 | 2026-10-10 | frick-fsw  | M9: add SRS-REQ-041 MISRA C:2012 compliance and SRS-REQ-042 coding style |
| 1.3 | 2026-10-10 | frick-fsw  | M10: add SRS-REQ-043 emulated-target verification (Renode/QEMU) |