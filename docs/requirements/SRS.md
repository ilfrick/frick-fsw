# Software Requirements Specification (SRS)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SRS-001                          |
| Revision                          | 1.19                                   |
| Date                              | 2026-10-12                             |
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
  exercises the pluggable network-stack interface without external dependencies
  (bounded, static, in-process loopback), for the unit tests and the emulated-target
  (Renode) functional check. `method: Tst`.
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
- **SRS-REQ-044** — *Renode functional-validation environment*: a Renode-based V&V
  environment running the middleware on an emulated PolarFire SoC shall be
  provided, with a middleware test application that emits a checkable result, and
  the functional validation in that environment shall pass for every change
  (norm N18). `method: Tst | Dem`.
- **SRS-REQ-045** — *OBCP engine (uPython/MicroPython)*: the framework shall
  integrate an engine that executes On-Board Control Procedures written in
  uPython/MicroPython behind an `FF_Obcp_t` contract, with OBCP modules bound to
  SB/EVS/Time/PUS (datapool, events, time, packets, control), bounded resources
  and safety (stack/heap accounting, execution-time budget, abort, reporting;
  OBCP state/verification via PUS S18).
  First integration candidate: `n7space/n7s-obcp` (MIT, ESA-funded); otherwise an
  ECSS-compliant engine. `method: Tst | Dem`. (milestone M12)
- **SRS-REQ-046** — *Reference mission services*: the framework shall provide, as
  optional reference applications/services on the PUS, an **equipment manager**
  (device command/monitoring/reconfiguration, redundancy) and **fault management**
  (limit checking, stored command sequences, memory/file management).
  `method: Tst | Dem`. (milestone M13)
- **SRS-REQ-047** — *Reference MSS GPIO driver*: the framework shall provide a
  reference HAL backend for the Microchip PolarFire SoC MSS GPIO (`FF_Gpio_t`),
  register-based and allocation-free, validated by a host unit test (mock MMIO)
  and by an emulated-MMIO functional check on the Renode PolarFire SoC.
  `method: Tst | Dem`. (milestone M4/M11)
- **SRS-REQ-048** — *Reference MSS MMUART serial driver*: a reference HAL backend
  for the Microchip PolarFire SoC MSS MMUART (`FF_Serial_t`; RS-422/RS-485 via an
  external transceiver), register-based (4-byte-spaced MMUART registers) and
  allocation-free, validated by a host unit test (mock MMIO) and an emulated-MMIO
  check on the Renode PolarFire SoC. `method: Tst | Dem`. (milestone M4/M11)
- **SRS-REQ-049** — *Reference static RAM pool and DMA helper*: a bounded,
  heap-free `FF_MemPool_t` (bump allocator over a caller-provided region) and a
  reference `FF_Dma_t` (address translation + cache-maintenance seam), validated
  by a host unit test and exercised over the emulated DDR in the Renode
  functional test. `method: Tst | Dem`. (milestone M4/M11)
- **SRS-REQ-050** — *PUS-C ASN.1 (N7/ESA) default backend*: the framework shall
  use the N7 Space / ESA `asn1-pusc-lib` (ASN.1/ACN PUS-C model, compiled with the
  ESA ASN1SCC compiler) as the **default** PUS-C basis behind `FF_Pus_Backend_t`,
  with the local open backend and the PUSopen(R) adapter retained as alternatives.
  `method: Tst | Dem`. (norm N19 reconciliation)
- **SRS-REQ-051** — *Multicore / SMP*: the framework shall run on the PolarFire
  SoC 4×U54 cluster under **RTEMS SMP** (multi-core tasking, per-core interrupts,
  cache-coherent shared data, shared-memory Software Bus), validated on the
  emulated SoC. `method: Tst | Dem`. (milestone M14)
- **SRS-REQ-052** — *Fundamental PUS-C service set*: the framework shall
  implement the fundamental PUS-C service behaviour (S1/3/5/6/8/9/11/12/17/
  18-PUS-surface/19/20) in MISRA-clean in-house C behind `FF_Pus_Backend_t`, on
  the default ASN.1/ACN data model, with a bounded on-board parameter datapool;
  every in-scope subservice shall be tested and its pure logic modelled in Lean.
  `method: Tst | Anl`. (milestone M15)
- **SRS-REQ-053** — *CFDP and virtual filestore*: the framework shall implement a
  **CFDP** entity (CCSDS 727.0-B) delivering files to/from the ground, and a
  bounded **virtual filestore (VFS)** with a pluggable storage backend
  (RAM/flash/eMMC/SSD); PUS-C **S23** file management shall operate on the VFS.
  `method: Tst | Dem`. (milestone M15)

- **SRS-REQ-054** — *In-house OBCP engine*: the framework shall provide an in-house,
  MISRA-oriented OBCP engine implementing the `FF_Obcp_t` contract for
  uPython/MicroPython On-Board Control Procedures (bounded resource accounting,
  isolation, execution-time budget, abort, reporting; OBCP state/management via
  PUS S18), developed within the project as the ECSS-compliant fallback to the
  integrated third-party engine (SRS-REQ-045); engine development is milestone
  **M16**. `method: Tst | Dem`. (milestone M16)

- **SRS-REQ-055** — *Reference NVM backend (eNVM)*: the framework shall provide a
  reference non-volatile-memory backend implementing the HAL memory contract
  (`FF_Nvm_t`, key/value by id) over the PolarFire SoC MSS embedded NVM (eNVM),
  with bounded slots and bounded ids. `method: Tst | Dem`. (milestone M4/M11)

- **SRS-REQ-056** — *Reference NOR/QSPI MTD backend*: the framework shall provide a
  reference NOR/flash memory-technology-device backend implementing the HAL memory
  contract (`FF_Mtd_t`: byte-addressed read/write, sector-aligned erase) over the
  PolarFire SoC MSS QSPI NOR flash, with a bounded window. `method: Tst | Inp`
  (host-tested; the live on-SoC QSPI check is not yet in place, see the pending
  QSPI controller command driver). (milestone M4/M11)

- **SRS-REQ-057** — *Reference MSS CAN driver*: the framework shall provide a
  reference HAL CAN backend for the Microchip PolarFire SoC MSS CAN controllers
  (`FF_CanBackend_t`, classic CAN 2.0 A/B frame transmit and receive dispatch,
  and set-up) over bounded TX/RX buffers (32 each), register-based and
  allocation-free, with CAN-FD rejected (FD is a fabric device reached through
  the FIC, not an MSS CAN feature). The HAL CAN contract carries no receive entry
  point, so received frames are dispatched by the driver-specific poll
  (`FF_CanMpfsPoll`); bit timing is a reference derivation from the configured
  MSS CAN clock (per-rate accuracy and on-target timing are tracked, GAP-A5).
  Validated by a host unit test (mock MMIO) and by an emulated-MMIO register
  check on the Renode PolarFire SoC. `method: Tst | Dem`. (milestone M4/M11)

- **SRS-REQ-058** — *Reference MSS I2C driver*: the framework shall provide a
  reference HAL I2C backend for the Microchip PolarFire SoC MSS I2C (CoreI2C)
  controllers through one bound instance (`FF_I2c_t`, polled master: `init`,
  `write`, `read`, `write_read`) over an injected byte-addressed MMIO bus,
  register-based and allocation-free, where `init(clock_hz)` selects the CoreI2C
  serial-clock divider for the requested SCL and the SI poll is bounded. The
  serial-clock divider is a reference derivation from the configured MSS I2C
  (APB) reference clock (per-rate accuracy and on-target SCL timing are tracked,
  GAP-A5); the minimum achievable SCL is the slowest divider (PCLK/960,
  ~104.17 kHz at the 100 MHz reference), so a requested rate below it is not
  honoured and the caller must request a rate at or above that minimum. Failures
  are reported as NACK → `FF_ERROR`, SI-poll timeout → `FF_TIMEOUT`, invalid
  arguments → `FF_INVALID_ARGUMENT` and pre-init use → `FF_NOT_READY`. The
  backend is a polled master only (7-bit addressing; no slave mode, SMBus, clock
  stretching, bus recovery, STOP-completion confirmation or interrupt path) and
  is not re-entrant (calls must be serialised by the caller). Validated by a host
  unit test (behavioural MMIO mock) and by an emulated-MMIO register check on
  the Renode PolarFire SoC. `method: Tst | Dem`. (milestone M4/M11)
- **SRS-REQ-069** — *Reference MSS SPI driver*: the framework shall provide a
  reference HAL SPI backend for the Microchip PolarFire SoC MSS SPI (CoreSPI)
  controllers behind `FF_Spi_t` (polled master: `init`, `transfer`, `cs_assert`,
  `cs_release`) over an injected 32-bit MMIO bus, register-based and
  allocation-free, where `init(clock_hz, mode, cs)` selects the CoreSPI clock
  divider for the requested SCL and the status poll is bounded; failures are
  reported as status-poll timeout → `FF_TIMEOUT`, invalid arguments →
  `FF_INVALID_ARGUMENT` and pre-init use → `FF_NOT_READY`. The clock divider is a
  reference derivation from the configured MSS SPI (APB) reference clock (per-rate
  accuracy and on-target SCL timing are tracked, GAP-A5); the backend is a polled
  master only (8-bit frames; no DMA, interrupt or enhanced-framing path) and is
  not re-entrant (calls must be serialised by the caller). Validated by a host
  unit test (behavioural MMIO mock) and by an emulated-MMIO register check on
  the Renode PolarFire SoC. `method: Tst | Dem`. (milestone M4/M11)

### Extended transport and fieldbus (milestone M4-residual / M8)

- **SRS-REQ-059** — *PUS-C link segmentation and reassembly*: the framework
  shall provide bounded, allocation-free segmentation and reassembly of PUS-C /
  CCSDS frames for stream- or packet-oriented IP and TSN links, with the
  encapsulation profile selected per the mission ICD (profile = mission input,
  TBC; owner: M4-residual). `method: Tst`.
- **SRS-REQ-060** — *IP-based PUS-C link backend (TCP/UDP)*: the framework shall provide a
  reference `FF_Link_t` backend that carries PUS-C TM/TC over an IP-based link
  (TCP or UDP as selected by the mission ICD; the reference uses UDP datagrams)
  using the network-stack contract (`FF_NetStack_t`), selectable with
  `FF_LinkSelect`. `method: Tst | Dem`.
- **SRS-REQ-061** — *TSN PUS-C link backend*: the framework shall provide a
  reference `FF_Link_t` backend that carries PUS-C TM/TC over the TSN-scheduled
  Ethernet path (`FF_TsnBackend_t` + GEM), selectable with `FF_LinkSelect`.
  `method: Tst | Dem`.
- **SRS-REQ-062** — *CAN-FD fabric backend*: the framework shall provide a
  reference CAN-FD backend behind `FF_CanBackend_t` reached through the generic
  fabric (FIC/AXI) device contract. `method: Tst | Dem`.
- **SRS-REQ-063** — *CANopen reference stack*: the framework shall provide a
  reference CANopen (CiA 301/302/402) stack implementing the `FF_CanOpenStack_t`
  contract over a CAN backend. `method: Tst | Dem`.
- **SRS-REQ-064** — *RS-485 half-duplex direction control*: the framework shall
  implement the serial contract's `set_rs485` direction control by driving a bound
  DE/RE control line with the requested assert timing. `method: Tst`.
- **SRS-REQ-065** — *PUSopen(R) documented adapter*: the framework shall provide a
  documented adapter implementing the PUS product contract over the commercial
  PUSopen(R) library (the commercial library itself is user-supplied).
  `method: Tst | Inp`.
- **SRS-REQ-066** — *GEM TSN shaping and filtering completion*: the framework
  shall extend the GEM native TSN backend with 802.1Qav (credit-based shaping),
  802.1Qci (per-stream filtering/policing) and 802.1Qbb (priority flow control);
  feasibility on the GEM register subset used by the reference backend (which does
  not currently expose CBS) = TBC (owner: M8). `method: Tst | Inp`.
- **SRS-REQ-067** — *Generic fabric-device contract*: the framework shall define a
  generic fabric-device (FIC/AXI) contract for resources reached through the FPGA
  fabric, on which the fabric CAN-FD/SpaceWire/TSN backends are built.
  `method: Inp`.
- **SRS-REQ-068** — *gPTP (802.1AS) and fabric CoreTSN completion*: the framework
  shall complete the deterministic-Ethernet feature set: a full 802.1AS gPTP stack
  (BMCA, Sync/Follow_Up, PDelay, servo) and the fabric CoreTSN path (multi-port
  switch / 802.1CB FRER). `method: Tst | Dem`.

## 6. Derived requirements

The following are derived from the applicability of ECSS Category B and from
mission tailoring of PUS-C; their derivation justification is recorded in the
traceability manifest (`derived: true`), per norm N4.

- **SRS-REQ-017** (derived) — TM/TC frames use CCSDS Space Packets (APPID
  0x010 / 0x020) with mission identifier 0x01. `derived justification: ECSS-E-ST-70-41C tailoring, doc ICD-GND`.
- **SRS-REQ-018** (derived) — the reference build uses the N7/ESA ASN.1 backend by
  default (SRS-REQ-050); the open backend is an alternative; PUSopen(R) is an
  optional drop-in adapter (planned, SRS-REQ-065). `derived justification:
  ADR-0003/ADR-0016`.
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
| 1.4 | 2026-10-10 | frick-fsw  | N18/M10: add SRS-REQ-044 Renode functional-validation environment + test app |
| 1.5 | 2026-10-10 | frick-fsw  | M12/M13: add SRS-REQ-045 OBCP engine (uPython/MicroPython) and SRS-REQ-046 reference mission services |
| 1.6 | 2026-10-10 | frick-fsw  | M4/M11: add SRS-REQ-047 reference MSS GPIO driver |
| 1.7 | 2026-10-10 | frick-fsw  | M4/M11: add SRS-REQ-048 reference MSS MMUART serial driver |
| 1.8 | 2026-10-11 | frick-fsw  | M4/M11: add SRS-REQ-049 reference static RAM pool + DMA helper |
| 1.9 | 2026-10-11 | frick-fsw  | N19/M14: add SRS-REQ-050 PUS-C ASN.1 (N7/ESA) default backend, SRS-REQ-051 multicore/SMP |
| 1.10 | 2026-10-11 | frick-fsw  | M15: add SRS-REQ-052 fundamental PUS-C service set (from FRICK-ARCH-0015) |
| 1.11 | 2026-10-11 | frick-fsw  | M15: add SRS-REQ-053 CFDP + virtual filestore |
| 1.12 | 2026-10-11 | frick-fsw  | M16: add SRS-REQ-054 in-house OBCP engine |
| 1.13 | 2026-10-11 | frick-fsw  | M4/M11: add SRS-REQ-055 reference eNVM NVM backend |
| 1.14 | 2026-10-11 | frick-fsw  | M4/M11: add SRS-REQ-056 reference NOR/QSPI MTD backend |
| 1.15 | 2026-10-11 | frick-fsw  | M4/M11: add SRS-REQ-057 reference MSS CAN driver |
| 1.16 | 2026-10-11 | frick-fsw  | M4/M11: SRS-REQ-057 review NCs (receive via driver-specific poll, bounded TX/RX buffers, bit-timing basis/GAP-A5); align SRS-REQ-055 method to Tst\|Dem and note SRS-REQ-056 host-only |
| 1.17 | 2026-10-12 | frick-fsw  | M4/M11: add SRS-REQ-058 reference MSS I2C driver (polled CoreI2C master; serial-clock divider reference derivation/GAP-A5) |
| 1.18 | 2026-10-12 | frick-fsw  | Restore explicit M4 scope: add SRS-REQ-059/060/061 PUS-C segmentation + TCP/IP + TSN link backends; SRS-REQ-062/063/064 CAN-FD/CANopen/RS-485 DE; SRS-REQ-065 PUSopen adapter; SRS-REQ-066 GEM TSN Qav/Qci/Qbb; SRS-REQ-067 generic fabric-device contract; SRS-REQ-068 gPTP/CoreTSN (SSS parents held in the manifest) — FRICK-REVIEW-0137 R3-05/R3-07/R3-08/R3-09 |
| 1.19 | 2026-10-12 | frick-fsw  | M4/M11: add SRS-REQ-069 reference MSS SPI driver (polled CoreSPI master; clock-divider reference derivation/GAP-A5) |
