# Software Description — frick-fsw Framework and Repository

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ENG-0007                         |
| Revision                          | 0.2                                    |
| Date                              | 2026-10-12                             |
| Audience                          | humans and agents using or evaluating the framework and the accessory repository elements |
| Norms                             | N2 (English), N8 (current documentation), N16 (per-document revision history) |
| Requirements                      | none introduced by this document; it describes the implemented baseline traced in `docs/requirements/SRS.md` and `docs/requirements/traceability.yaml` |

## 1. Purpose, scope and naming note

This document is the **comprehensive user-facing description of the frick-fsw
framework and of the accessory repository elements**. It is the first place a
new user or evaluator reads to understand:

- what the framework is (and is not), and which layers are implemented in this
  repository versus interface-only, planned, or user-provided;
- what the accessory elements beside `src/` do (`config/`, `tests/`,
  `verification/lean/`, `tools/`, `assets/`, `board/`, `cmake/toolchains/`,
  `.github/`, `.gitea/`, `docs/`);
- how to build, run and verify the framework, with pointers to the entry points
  (`AGENTS.md`, `README.md`) and to the detailed engineering guides;
- the rule that **this document is updated at every release** (§6).

**Naming note.** The project lead and users referred to this document as the
"**DDJF**" (a comprehensive user-facing description of the framework and of the
accessory repository elements, kept current at every release). **No repository
file defines an acronym "DDJF"** (verified 2026-10-12: no occurrence of "DDJF"
in the repository), so this document is registered under the descriptive title
above, with document id **FRICK-ENG-0007**. If a project acronym is wanted, it
must be defined and registered by the lead (e.g. in `docs/glossary.md`) in a
later revision; this revision does not invent one.

Scope boundaries (norm N8): this is **documentation only**. It introduces no
new normative requirement, modifies no code, build file, test, script or
evidence, and does not claim certification, qualification or flight-readiness.
Where a capability is not implemented, the document says so and references the
known gaps (GAP-A1, GAP-A4, GAP-A5; `docs/quality/class-a-gaps.md`).

## 2. What frick-fsw is

frick-fsw is a **cFS-inspired flight-software middleware (framework), not a
mission application**, for a small/medium spacecraft OBC:

- **PUS-C** (ECSS-E-ST-70-41C) TM/TC over **CCSDS Space Packets**
  (CCSDS 133.0-B-2), with a CRC-16/CCITT-FALSE trailer;
- target platform: **Microchip RT PolarFire SoC RTPFS160ZT** (FCVG784, RISC-V)
  under **RTEMS 6.2** (open-source RTEMS project, BSP `riscv/mpfs64imafdc`);
- development/platform-independent V&V: the **same C sources** run as a native
  POSIX-thread host simulator, and on the Renode emulated PolarFire SoC and the
  QEMU generic RISC-V target;
- target ECSS criticality **B**, with the residual gaps to level A documented
  (`docs/quality/ecss-criticality-b.md`, `docs/quality/class-a-gaps.md`);
- open source under **AGPL-3.0-only** (norm N13; `LICENSE`, `NOTICE`,
  `docs/quality/licensing.md`).

The authoritative "what is included" matrix (implemented / interface-only /
planned / user-provided) is kept in `README.md` ("Project scope"); the product
boundary and milestone plan are in `docs/development-plan.md`
(FRICK-PLAN-0001) and the roadmap in `docs/roadmap.md` (FRICK-RM-0001).

The six applications under `src/apps/` are **reference applications**
(demonstration and V&V), **not** part of the middleware contract:
`docs/architecture/applications.md` (FRICK-ARCH-0005), and the
`src/apps/app_renode_test.c` functional-test app is registered only when
`FF_ENABLE_RENODE_TEST=ON`.

### 2.1 What frick-fsw is not

To prevent over-reading the scope, the framework is explicitly **not**:

- **a mission application** — the six `src/apps/*` are reference/demo and V&V
  apps; mission software is written by users on the SB/PUS/HAL seams;
- **a TCP/IP stack** — it owns the `FF_NetStack_t` contract and ships an opt-in
  lwIP adapter only; **PUS-C over IP is not implemented end-to-end** (§3.7);
- **a TSN / gPTP stack** — it ships a MAC-level `FF_TsnBackend_t` contract and a
  GEM backend implementing 802.1Q/Qbv plus an 802.1AS *time-read seam*; the full
  gPTP stack and the fabric CoreTSN path are milestone M8 (§3.7);
- **a file-transfer (CFDP/PUS-23) or on-board-autonomy implementation** — these
  are planned milestones (M15/M12/M16; see the README "Project scope" matrix);
- **a crypto implementation** — the CCSDS SDLS sub-layer is middleware with a
  **pluggable** cryptographic backend; the certified algorithm, hardware
  keystore and ground key management are **GAP-A14** (`src/security/`,
  `docs/architecture/security-sdls.md`);
- **a flight-hardware deliverable** — the BSP is bring-up plus reference
  drivers; on-target netif/PHY, timing and stack behaviour are **GAP-A5**;
- **a certified or qualified product by itself** — it targets ECSS criticality
  **B** with the residual gaps to level A documented (norm N5;
  `docs/quality/ecss-criticality-b.md`, `docs/quality/class-a-gaps.md`); this
  document makes **no** airworthiness, certification or qualification claim.

## 3. The framework

This section describes the middleware and its seams. Every row is traceable to
a repository path; the detailed design is `docs/design/software-design.md`
(FRICK-SDD-001) and the architecture notes are `docs/architecture/`.

### 3.1 Middleware core

| Component | Responsibility | Location |
| --------- | -------------- | -------- |
| PAL — Portable Abstraction Layer | OS primitives: tasks, mutexes, queues, time, console; POSIX (host + RTEMS) and RTEMS bootstrap | `src/core/pal/pal.h`, `pal_posix.c`, `pal_rtems.c` |
| SB — Software Bus | publish/subscribe with fixed-size pipes and wildcard subscriptions (`FF_SB_WILDCARD` = `0xFFFF`) | `src/core/sb/sb.h` |
| EVS — Event Services | event log, listeners, forwarding to PUS service 5 | `src/core/evs/evs.h` |
| ES — Executive Services | application registry, lifecycle, reset counters | `src/core/es/es.h` |
| TBL — Table Services | load / validate / atomically activate / revert / dump mission tables | `src/core/tbl/tbl.h` |
| TIME — Time Services | MET/UTC, source selection, 1 Hz tone discipline, CUC timecode, PPS backend vtable | `src/core/time/time.h`, `time.c`, `pps.h`, `pps.c`; `docs/architecture/time-and-timecode.md` |
| PUS-C layer | TC dispatch (`FF_PusIngest`), S1 verification, TM emission (`FF_PusSendTm`), provider hooks (S3/S6/S8/S20), backend vtable `FF_Pus_Backend_t` | `src/pus/pus.h`, `pus.c`, `pus_services.c`, `pus_types.h`, `src/pus/adapters/` |
| CCSDS transport | Space Packet encode/decode + CRC-16/CCITT-FALSE | `src/transport/ccsds/` |
| Link layer | byte-stream transport of CCSDS frames: loopback and UART backends, run-time selection `FF_LinkSelect` | `src/transport/link/link.h`, `link.c`, `link_loopback.c`, `link_uart.c` |

PUS-C services shipped (open backend catalogue in `docs/interfaces/ICD.md`
§4): **1** request verification (framework-generated), **3** housekeeping
(provider-registered), **5** event reporting (via EVS), **6** memory management
(region-bounded dumps), **8** function management (provider-registered),
**9** time management (CUC), **11** scheduling (stub returning
`FF_NOT_IMPLEMENTED`, `src/pus/pus_services.c`), **17** test (ping/pong),
**20** parameter management (provider-registered). The default PUS product is
the **N7 Space / ESA `asn1-pusc-lib` ASN.1/ACN backend**
(`FF_PusBackendN7`, ASN1SCC-compiled at build time, `FF_USE_PUS_ASN1=ON`
default; ADR-0016); the open CCSDS backend is the fallback; PUSopen is an
optional commercial adapter (user-supplied). See `docs/architecture/pus-layer.md`
(FRICK-ARCH-0003) and `docs/adr/ADR-0003`, `docs/adr/ADR-0016`.

### 3.2 HAL contracts (interface deliverables, milestone M3)

The framework depends only on the **HAL contracts** in `src/hal/` — bounded,
allocation-free registry (`src/hal/hal.c`, `hal.h`) plus one header per
resource family: Ethernet (`hal_eth.h`: `FF_EthMac_t`, `FF_NetIf_t`,
SRS-REQ-026), network stack (`hal_net.h`: `FF_NetStack_t`, SRS-REQ-027), TSN
(`hal_tsn.h`: `FF_TsnBackend_t`, SRS-REQ-028), CAN/CANopen (`hal_can.h`),
serial (`hal_serial.h`), SPI/I2C (`hal_spi.h`), PCIe (`hal_pcie.h`), GPIO
(`hal_gpio.h`), SpaceWire (`hal_spacewire.h`), memory/storage (`hal_mem.h`).
The catalogue and the integrator obligations (buffer ownership, DMA/cache
coherence, ISR vs task context, no dynamic allocation, bounded latency,
endianness, error model, half-duplex RS-485, packet vs stream framing) are in
`docs/architecture/hardware-abstraction.md` (FRICK-ARCH-0008).

### 3.3 Reference drivers (milestone M4 reference implementations)

Reference drivers implement HAL/link contracts and are **separable/optional**
(`FF_BUILD_DRIVERS`, default ON):

| Driver | Contract | Location | Notes |
| ------ | -------- | -------- | ----- |
| GEM Ethernet MAC | `FF_EthMac_t` | `src/drivers/eth/gem/ff_gem.{c,h}` | static, allocation-free, host-testable (mock MMIO); SRS-REQ-036; on-target PHY/link = GAP-A5 |
| GEM native TSN backend | `FF_TsnBackend_t` | `src/drivers/eth/gem/ff_gem_tsn.{c,h}` | 802.1Q/Qbv + an 802.1AS time-read seam; 802.1Qav returns `FF_NOT_IMPLEMENTED`; SRS-REQ-038 |
| Loopback network stack | `FF_NetStack_t` | `src/drivers/net/ff_net_loopback.{c,h}` | bounded in-process socket API; SRS-REQ-039 |
| lwIP adapter | `FF_NetStack_t` | `src/hal/net/lwip/ff_net_lwip.{c,h}` | opt-in (`FF_USE_LWIP=ON`); third-party pinned lwIP; SRS-REQ-040 |
| MSS GPIO | `FF_Gpio_t` | `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` | SRS-REQ-047; host test + Renode `hal_gpio=PASS` |
| MSS MMUART serial | `FF_Serial_t` | `src/drivers/serial/mpfs/ff_serial_mpfs.c` | SRS-REQ-048; host test + Renode `hal_serial=PASS` |
| Static RAM pool + DMA | `FF_MemPool_t`, `FF_Dma_t` | `src/drivers/mem/ff_mem_static.c` | SRS-REQ-049; host test + Renode `hal_mem=PASS` |
| MSS eNVM NVM | `FF_Nvm_t` | `src/drivers/nvm/mpfs/ff_nvm_mpfs.c` | SRS-REQ-055; host test + Renode `hal_nvm=PASS` |
| MSS QSPI NOR MTD | `FF_Mtd_t` | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c` | SRS-REQ-056; host test; on-SoC QSPI check pending the controller command driver (M11 follow-up) |
| MSS CAN (classic) | `FF_CanBackend_t` | `src/drivers/can/mpfs/ff_can_mpfs.c` | SRS-REQ-057; host test + Renode `hal_can=PASS`; CAN-FD is a fabric device |
| MSS I2C | `FF_I2c_t` | `src/drivers/i2c/mpfs/ff_i2c_mpfs.c` | SRS-REQ-058; host test + Renode `hal_i2c=PASS` |

The board bring-up is `src/bsp/mpfs/` (`board.h`, `board.c`); the run-time
wire-up of the core, providers, link and applications is
`src/main/fsw_main.c`. The current driver inventory with host/Renode evidence
is also in `README.md` ("Low-level drivers") and `docs/vv/tool-qualification.md`
(T3/T17).

### 3.4 Reference applications (not the framework)

Six reference apps register with ES: **CMD_INGEST** (TC ingest →
`FF_PusIngest`), **TM_OUTPUT** (PUS TM sink → link), **SCHEDULER**, **HK**
(housekeeping via providers), **HS** (health & safety heartbeat), **SAMPLE**
(telemetry generator); plus the conditional **RENODE_TEST** app for norm N18
(`src/apps/app.h`, `src/apps/app_*.c`, `src/main/fsw_main.c`,
`docs/architecture/applications.md`). These are **examples**; mission software
is written by users on the SB/PUS/HAL seams
(`docs/engineering/user-integration-manual.md`).

### 3.5 Security — CCSDS SDLS

The framework provides the **CCSDS SDLS** (CCSDS 355.0-B) sub-layer as
middleware: SDLS processing, security-association database and key
lifecycle/rotation, through a **pluggable cryptographic backend**
(`src/security/ff_sdls.{c,h}`, `src/security/ff_crypto_test.{c,h}`). The
middleware does **not** implement a cipher; the certified algorithm, hardware
keystore, IV-uniqueness analysis and ground key management remain **GAP-A14**
(`docs/architecture/security-sdls.md`, FRICK-ARCH-0010; `docs/adr/ADR-0013`).

### 3.6 Mathematics facade

A portable facade over the platform `libm` for applications that need
floating-point math: `src/math/ff_math.{c,h}` (`FF_MathSqrt/Sin/Cos/...`,
`FF_MathDeg2Rad`, `FF_MathClamp`, `FF_MathIsFinite`; SRS-REQ-033). The
framework core uses integer arithmetic only. Policy, determinism and
WCET/stack cautions: `docs/architecture/numerics.md` and `docs/adr/ADR-0012`.

### 3.7 Networking and TSN position (summary)

- **Framework or separate?** The framework owns only the *contracts*:
  `FF_NetStack_t` (`src/hal/hal_net.h`, SRS-REQ-027) and the Ethernet MAC /
  net-if contracts `FF_EthMac_t` / `FF_NetIf_t` (`src/hal/hal_eth.h`,
  SRS-REQ-026). Every TCP/IP **stack** is separate and optional.
- **TCP/IP**: the framework ships the `FF_NetStack_t` contract and a loopback
  reference stack (`FF_NetStackLoopback`); **lwIP** is a third-party
  (BSD-3-Clause) adapter pinned to the release tag `STABLE-2_2_1_RELEASE`,
  fetched on demand (`tools/scripts/fetch_third_party.sh`) and built only with
  `FF_USE_LWIP=ON` (default **OFF**); Tuxera TCP/IP and RTEMS `libbsd` are
  **user-supplied**.
- **PUS-C over IP is planned, not implemented**: no `FF_Link_t` TCP/IP/TSN
  backend, no segmentation/reassembly, no EtherType/port allocation in
  `docs/interfaces/ICD.md`, and the ground tool is serial-only.
- **TSN**: `FF_TsnBackend_t` + the GEM native backend implement
  802.1Q / 802.1Qbv plus an 802.1AS *time-read seam* (a hardware clock read,
  not the gPTP protocol); 802.1Qav returns `FF_NOT_IMPLEMENTED`; the full
  gPTP stack and the fabric CoreTSN path are planned at **M8**.

User-facing details, tables, "how to use" steps and evidence are in
`docs/engineering/user-integration-manual.md` (§8 TCP/IP over PUS-C incl. §8.1,
§9 TSN incl. §9.1, §10 which stack is included) and
`docs/architecture/network-stack.md` (FRICK-ARCH-0012).

### 3.8 Board support and start-up sequence

- **BSP** — `src/bsp/mpfs/` (`board.h`, `board.c`) exposes `FF_BoardName()` and
  `FF_BoardInit()`, the single board bring-up hook for the RTPFS160ZT. It is a
  bring-up placeholder: the on-target board/peripheral work is **GAP-A5** (and
  the PPS hardware capture is **GAP-A7**).
- **Main / start-up** — `src/main/fsw_main.c` (`ff_fsw_main`) is the **shared**
  bring-up sequence for the host simulator and the RTEMS target (RTEMS
  `POSIX_Init`, SRS-REQ-037). Order: banner; `FF_PalInit` → `FF_SbInit` →
  `FF_EvsInit` → `FF_EsInit` → `FF_TblInit` → `FF_TimeInit` → `FF_BoardInit` →
  `FF_MissionTablesInit`; then `FF_PusInit` + backend selection (default
  `FF_PusBackendN7`); `FF_LinkInit`; install the EVS listener; register the
  reference apps (`CMD_INGEST`, `TM_OUTPUT`, `SCHEDULER`, `HK`, `HS`, `SAMPLE`,
  and conditionally `RENODE_TEST`); `FF_EsStartAll`; then an idle loop.
- The RTEMS application configuration table (bounded `CONFIGURE_*`) lives in
  `config/rtems/rtems_config.c`; platform specifics in `config/platform/`.

### 3.9 Interface contracts — where they live and their status

The framework depends only on the contracts below; a mission supplies or selects
a backend behind each. "Implemented" = a reference backend is in this repository;
"interface-only" = the contract is delivered, the backend is planned or supplied
by the mission; "user-supplied" = deliberately left to the integrator.

| Contract | Header / location | Status in this repo | Reference backend |
| -------- | ----------------- | ------------------- | ----------------- |
| PAL (tasks/mutex/queue/time/console) | `src/core/pal/pal.h` | **Implemented** | POSIX (host + RTEMS) + RTEMS bootstrap |
| `FF_Pus_Backend_t` (PUS product) | `src/pus/pus.h` | **Implemented** (default N7/ESA ASN.1; open CCSDS fallback) | `FF_PusBackendN7`, `FF_PusBackendOpen`; PUSopen optional (user-supplied) |
| `FF_Link_t` (byte-stream CCSDS frame transport) | `src/transport/link/link.h` | **Implemented** (loopback + UART only) | `FF_LinkLoopbackBackend`, `FF_LinkUartBackend`; TCP/IP/SpaceWire/radio planned |
| `FF_PpsBackend_t` (1 Hz tone) | `src/core/time/pps.h` | **Implemented** (software); hardware capture = GAP-A7 | `FF_PpsSwBackend` |
| `FF_CryptoBackend_t` (SDLS crypto) | `src/security/ff_sdls.h` | **Interface + reference test backend**; certified crypto/HSM = GAP-A14 | `FF_CryptoTestBackend`; mission crypto/HSM user-supplied |
| `FF_EthMac_t`, `FF_NetIf_t` (Ethernet MAC / net-if) | `src/hal/hal_eth.h` | **Implemented** (contract; GEM MAC reference) | `src/drivers/eth/gem/ff_gem.{c,h}` |
| `FF_NetStack_t` (pluggable TCP/IP stack) | `src/hal/hal_net.h` | **Implemented** (contract; loopback + opt-in lwIP adapter) | `FF_NetStackLoopback`; `FF_NetStackLwip` (`FF_USE_LWIP`) |
| `FF_TsnBackend_t` (deterministic Ethernet) | `src/hal/hal_tsn.h` | **Implemented** (contract; GEM backend: 802.1Q/Qbv + 802.1AS time-read seam; 802.1Qav `FF_NOT_IMPLEMENTED`; gPTP/CoreTSN = M8) | `FF_TsnGem_t` (`src/drivers/eth/gem/ff_gem_tsn.{c,h}`) |
| `FF_CanBackend_t` (CAN/CANopen) | `src/hal/hal_can.h` | **Implemented** (classic CAN reference; CAN-FD = fabric device) | `src/drivers/can/mpfs/ff_can_mpfs.c` |
| `FF_Serial_t` (serial/RS-422/RS-485) | `src/hal/hal_serial.h` | **Implemented** | `src/drivers/serial/mpfs/ff_serial_mpfs.c` |
| `FF_Spi_t`, `FF_I2c_t` (SPI/I2C) | `src/hal/hal_spi.h` | **I2C implemented; SPI interface-only** (planned M4/M11) | I2C `src/drivers/i2c/mpfs/ff_i2c_mpfs.c`; SPI planned |
| `FF_Gpio_t` (GPIO/IRQ) | `src/hal/hal_gpio.h` | **Implemented** | `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` |
| `FF_Pcie_t` (PCIe) | `src/hal/hal_pcie.h` | **Interface-only** (planned) | — |
| SpaceWire (ECSS-E-ST-50-12/51/52/53) | `src/hal/hal_spacewire.h` | **Interface-only** (planned) | — |
| `FF_MemPool_t`, `FF_Dma_t`, `FF_Mtd_t`, `FF_Nvm_t`, `FF_BlockDev_t` | `src/hal/hal_mem.h` | **Implemented** (RAM pool/DMA/MTD/NVM reference; block device planned) | `src/drivers/mem/ff_mem_static.c`, `src/drivers/mtd/mpfs/ff_mtd_mpfs.c`, `src/drivers/nvm/mpfs/ff_nvm_mpfs.c` |

The HAL registry (`src/hal/hal.c`, `hal.h`) is bounded and allocation-free; the
integrator obligations (buffer ownership, DMA/cache coherence, ISR vs task
context, no dynamic allocation, bounded latency, endianness, error model) are in
`docs/architecture/hardware-abstraction.md` (FRICK-ARCH-0008). The authoritative
implemented / interface-only / planned / user-provided matrix is the README
"Project scope" table; the per-driver host/Renode evidence is in `README.md`
("Low-level drivers") and `docs/vv/tool-qualification.md`.

## 4. Accessory repository elements

What surrounds the framework sources, and why:

| Element | Content | Purpose |
| ------- | ------- | ------- |
| `config/` | single configuration point `fsw_config.h`; platform headers `config/platform/` (`platform_native.h`, `platform_mpfs.h`, `platform_qemu.h`); RTEMS application configuration `config/rtems/rtems_config.c`; mission tables `config/tables/` (`fsw_tables.{c,h}`) | one place to tailor sizes, APIDs, link device, platform specifics (norm N8) |
| `tests/` | C unit tests `tests/unit/*.c` on the minimal assertion harness `tests/ff_test.h`; registered in `tests/CMakeLists.txt`; executed by ctest (V&V tool T3) | requirement-level unit verification on the host (norm N11) |
| `verification/lean/` | Lean 4 project: `verification/lean/Frick/` modules (`Ccsds`, `Common`, `Hal`, `LeanOmega`, `Pus`, `Requirements`, `Sdls`, `SoftwareBus`, `Time`), `Main.lean`, `lakefile.lean`, pinned `lean-toolchain` | formal verification of requirement theorems (norm N1) |
| `tools/scripts/` | V&V tools: `trace.py` (traceability gate T6), `coverage.py` (T12), `metrics.py` (status panel T13), `run_sim.sh` (T4/T9), `setup_rtems.sh` (T10), `renode_env_selftest.sh` / `renode_vv.sh` (T17), `emulated_run.sh` (T15/T16), `qemu_vv.sh` (T20), `gen_pus_asn1.sh` (T19), `misra.sh` (T14), `fetch_third_party.sh`, `ci_rtems_cross_build.sh` | all V&V scripts are qualified per norm N10 in `docs/vv/tool-qualification.md` |
| `tools/gsw/` | ground station `ff_gsw.py` (T7): PUS-C TC injection and TM decode **over serial only** (qualified tool) | operator/automation interaction with the flight software |
| `tools/docker/` | V&V image `Dockerfile` (T1); Renode environment `renode/` (T17); open RTEMS toolchain image `rtems/` (T18); ASN1SCC generator image `asn1scc/` (T19); Gitea `act_runner/` deployment | reproducible V&V/CI environments (norms N6/N10) |
| `assets/` | project marks (logo) for codename "Eärendil" (`assets/README.md`) | branding/marketing material, no functional role |
| `board/` | device-tree source `mpfs-rtpfs160zt.dts` | board description template for the RTPFS160ZT |
| `cmake/toolchains/` | RTEMS toolchain files: `rtems-riscv-mpfs.cmake` (target `riscv/mpfs64imafdc`), `rtems-riscv-generic.cmake` (QEMU `riscv/rv64imafdc`) | cross-compilation for target and emulated-target builds |
| `.github/workflows/`, `.gitea/workflows/` | CI pipelines `ci.yml` (both forges) and the nightly `nightly.yml` (GitHub) | continuous gates: format, build, test, smoke, Lean, trace, docs, RTEMS cross build, Renode (norm N6/N7/N11) |
| `docs/` | the engineering documentation set described in §4.1 | norms N8/N16 documentation, reviews, status |
| Root accessory files | `Makefile` (developer shortcuts), `CMakeLists.txt` (build), `CMakePresets.json`, `Doxyfile` (API docs, `make docs`), `LICENSE`/`NOTICE` (AGPL-3.0-only), `README.md` (human entry point), `AGENTS.md` (agent entry point), `CHANGELOG.md` (project history, norm N16), `CONTRIBUTING.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md`, `CITATION.cff`, `.clang-format`/`.clang-tidy`/`.editorconfig` (style/static analysis) | entry points, build, style, licence, community |

**Manual register of V&V tools**: `docs/vv/tool-qualification.md`
(FRICK-VV-TOOLS-001) lists T1..T20 with qualification level and independence;
a change to a registered script invalidates its qualification until re-review
(norm N10).

### 4.1 Organization of `docs/`

| Directory / file | Content |
| ---------------- | ------- |
| `docs/rules.md` | non-derogable norms N1-N20 (FRICK-RULES-001) |
| `docs/tooling.md` | approach and tooling, incl. CI evidence markers (FRICK-TOOLING-001) |
| `docs/glossary.md` | project glossary (FRICK-GLOS-001) |
| `docs/requirements/` | backwards ECSS requirements: `SSS.md`, `SRS.md`, `traceability.yaml` (manifest), `traceability.md` (generated view) — norms N3/N4 |
| `docs/architecture/` | overview, pus-layer, software-bus, platform, applications, time-and-timecode, hardware-abstraction, rtems-integration, network-stack, numerics, security-sdls, mapping-cfs, component-gap-analysis, pus-c-from-scratch-assessment |
| `docs/design/` | SDD `software-design.md` (FRICK-SDD-001) |
| `docs/interfaces/` | ICD `ICD.md` (FRICK-ICD-001): PUS-C/CCSDS ground interface |
| `docs/adr/` | architecture decision records ADR-0001..0016 (ADR-0015 is unused) |
| `docs/quality/` | ECSS criticality B baseline, class-A gaps (GAP-A1..A14), compliance matrix, configuration management (SCMP), product assurance, safety and FDIR, MISRA process, licensing |
| `docs/vv/` | verification plan, verification results, validation plan, tool qualification, emulated-target, renode-environment |
| `docs/engineering/` | guides: independent review (reviewer-agent profiles R1/R2/R3/R4/PA), driver development, PUS service development, user & integration manual, coding style, this software description |
| `docs/planning/` | sprint plan (FRICK-PLAN-0002) |
| `docs/status/` | `handover.md` (resume point), `loop-audit.md` (norm N19), `status.json` (semaphores) |
| `docs/reviews/` | independent review reports and log (norms N9/N11/N12/N17/N20) |
| `docs/verification/` | formal-verification description |
| `docs/diagrams/` | diagram asset directory (currently empty; diagrams live inline in the docs) |

## 5. Building, running and verifying

Full one-command pipeline and per-tool commands are in `AGENTS.md` §3
(agent entry point) and `README.md` "Quickstart" (human entry point). Summary
of the `make` shortcuts (wired in `Makefile`):

| Command | Effect |
| ------- | ------ |
| `make configure build test` | host simulator + unit tests (ctest) |
| `make run` | boot the native simulator (six reference apps) |
| `make verify-lean` | Lean 4 formal verification (norm N1) |
| `make trace` | traceability convergence gate (norm N4) |
| `make coverage metrics` | gcov coverage + README status panel |
| `make ci-local` | whole cycle in the pinned Docker V&V image |
| `make renode-env`, `make renode-image`, `make renode` | Renode V&V environment self-test / RTEMS image / functional run (norm N18, `RENODE_TEST: PASS`) |
| `make qemu-image`, `make qemu` | QEMU `virt` generic-BSP emulated-target check (M10) |
| `make pus-asn1` | ASN1SCC generation for the N7/ESA PUS-C ASN.1 default backend |
| `make rtems-toolchain`, `make rtems-docker`, `make rtems` | open RTEMS 6.2 toolchain image / cross compile in image / cross compile with host toolchain |

Platform / target matrix (same C sources, different platform headers under
`config/platform/`):

| Target | Selection | Build / run | Purpose |
| ------ | --------- | ----------- | ------- |
| Host simulator (native POSIX) | `-DFF_PLATFORM=native` (default) | `make configure build test` / `make run` | development, unit tests, smoke |
| RTEMS 6.2 on PolarFire SoC (`riscv/mpfs64imafdc`) | `-DFF_PLATFORM=rtems` (`FRICK_PLATFORM_MPFS`) | `make rtems` / `make renode` | target cross build; Renode functional validation (norm N18) |
| RTEMS 6.2 generic RISC-V (QEMU `virt`) | `-DFF_PLATFORM=rtems-generic` (`FRICK_PLATFORM_QEMU`) | `make qemu-image qemu` | emulated-target check without MPFS peripherals (M10) |

Verification without flight hardware is described in
`docs/development-plan.md` §5 and `docs/vv/verification-plan.md`; the residual
on-silicon items are the documented class-A gaps (GAP-A5 and related) rather
than silent omissions. The authoritative per-cycle evidence is recorded in
`docs/vv/verification-results.md` and `docs/status/status.json` (norm N11).

## 6. Release currency rule (this document is updated at every release)

**Rule.** This document is a **living user-facing description** and shall be
**updated at every release** so that §2 (what frick-fsw is), §3 (the
framework), §4 (accessory repository elements) and §5 (build/run/verify)
describe the released baseline accurately.

- **Trigger (release).** At every release preparation, the release owner (or
  agent) checks this document against the release tree: source/directory map,
  implemented/planned status in §3.7 (networking and TSN in particular),
  accessory elements, build/verify commands and evidence references. Any drift
  is corrected in the release commit(s).
- **Trigger (incremental).** Any change between releases that alters the
  framework boundary, the repository layout, the build/verify commands, or the
  implemented/planned status of any capability updates this document **in the
  same commit** (norms N8 and N16 already require the affected documentation to
  change with the change; this section makes the obligation explicit for this
  document).
- **Procedure.** Each update bumps the header revision and date, and adds a row
  to the `## History` table below with rev, date, author and reason. The change
  origin (release preparation, test failure, review finding, requirement delta,
  tooling change) is recorded in the reason column (norm N16). The release
  notes in `CHANGELOG.md` cross-reference this document's revision.
- **Review.** This document is reviewed by the same independent documentation
  (R4, norm N17) and product-assurance (PA, norm N20) reviewers on every change
  that touches it, like the other engineering documents.

**Release-check trigger box**

```text
Release checklist entry:
[ ] FRICK-ENG-0007 revised/verified: framework §3, accessory elements §4,
    build/verify §5 and the implemented/planned status §3.7 reflect the
    released tree; History row added.
```

This rule is a documentation-keeping commitment aligned with norms N8/N16; it
does not create new product requirements or certification claims.

## 7. Related documents

- `AGENTS.md` (agent entry point, norms and quickstart), `README.md` (human
  entry point and scope matrix), `docs/rules.md` (norms N1-N20),
  `docs/tooling.md` (approach and tools).
- `docs/engineering/user-integration-manual.md` (FRICK-ENG-0005) — how to
  integrate applications, drivers and third-party stacks, incl. §8 TCP/IP over
  PUS-C (and §8.1 "how to use"), §9 TSN (and §9.1 "how to use"), §10 which
  TCP/IP stack is included.
- `docs/architecture/network-stack.md` (FRICK-ARCH-0012) — the net-stack
  contract and backends with the PUS-C-over-IP status note.
- `docs/engineering/driver-development.md` (FRICK-ENG-0003),
  `docs/engineering/service-development.md` (FRICK-ENG-0004),
  `docs/engineering/coding-style.md` (FRICK-ENG-0006).
- `docs/architecture/overview.md` (FRICK-ARCH-0001),
  `docs/design/software-design.md` (FRICK-SDD-001), `docs/interfaces/ICD.md`
  (FRICK-ICD-001), `docs/development-plan.md` (FRICK-PLAN-0001),
  `docs/roadmap.md` (FRICK-RM-0001).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | frick-fsw | Initial comprehensive user-facing software description of the framework and accessory repository elements; named "DDJF" by the user, registered under the descriptive title (no repository acronym defined); release-currency rule added (lead directive, documentation sprint) |
| 0.2 | 2026-10-12 | frick-fsw | Expand for users: add §2.1 "what frick-fsw is not", §3.8 BSP and start-up sequence, §3.9 interface-contract inventory (location + implemented/interface-only/planned/user-supplied), a §5 platform build/target matrix, and the exact lwIP pin (`STABLE-2_2_1_RELEASE`)/BSD-3-Clause and §8.1/§9.1 "how to use" cross-references in §3.7 |