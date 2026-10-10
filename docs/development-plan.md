# Development Plan

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-PLAN-0001                        |
| Revision                          | 0.8                                    |
| Date                              | 2026-10-10                             |
| Status                            | living plan; updated every cycle (norm N8/N16) |
| Applicable norms                  | N1-N18 (`docs/rules.md`)               |

## 1. Product boundary: a middleware, not an application

frick-fsw is a **flight-software middleware (framework)**, not a mission
application. It provides the command & data handling backbone — PAL, Software
Bus, Event Services, Executive Services, Table Services, Time Services, CCSDS
codec, link layer, PUS-C services — and the **interfaces** through which users
integrate their own:

- **mission applications** (the six `src/apps/*` are *reference* applications
  used for demonstration and V&V, not mission software), and
- **low-level drivers / board support** for their hardware, and
- **third-party stacks** (e.g. TCP/IP, CANopen, TSN) via pluggable backends.

Everything above the hardware abstraction seam is delivered; everything below
the seam is either an **interface this repo defines** (so a user can plug their
own implementation) or a **reference implementation kept in a separate,
optional partition** (so a user can take only what they need). This plan makes
that boundary explicit and tracks it as milestones; the concise milestone view
is in `docs/roadmap.md`.

## 2. Objectives (traceable to user requirements)

| Id | Objective | Milestone |
| -- | --------- | --------- |
| OBJ-01 | State the middleware boundary in README, repo description and AGENTS. | M3 |
| OBJ-02 | Ship a **User & Integration Manual** for humans and agents (app model, ES/SB/EVS, HK/param/function/memory providers, time, link, build/test, traceability). | M3 |
| OBJ-03 | Complete the **PUS-C service extension guide** (standardised vs custom services, provider hooks, worked example). | M3 |
| OBJ-04 | Define the **hardware/OS abstraction (HAL) interfaces** (contracts only, no drivers) for every bus and memory resource below. | M3 |
| OBJ-05 | Make **TCP/IP and TSN pluggable**: a stack-independent network contract so a user can plug Tuxera, lwIP, RTEMS libbsd, or their own. | M3 (contract) / M4 (backends) |
| OBJ-06 | Apply the **pluggable-backend philosophy to every resource** (bus, storage, GPIO, clock), not only the network. | M3 |
| OBJ-07 | **Partition the low-level developments** in the repository and docs so users can opt into some and not others (selectable CMake targets). | M3 (structure) / M4 (implementations) |
| OBJ-08 | Document the **V&V approach completely** so humans and agents know exactly what is verified, how, and what is not (gaps). | M3 |
| OBJ-09 | Fix the **architecture diagrams** to focus on the software layers running on the PolarFire SoC, marking graphically which layers are implemented in this repository and which are not. | M3 |
| OBJ-10 | Implement the **MPFS low-level layers** behind the HAL (native MSS peripherals), so the middleware runs on the target. | M4 |
| OBJ-11 | Deliver a complete **1 Gb/s Ethernet driver** on the MPFS MSS GEM, integrated with the middleware link/network layer. | M4 |
| OBJ-12 | Keep the full cycle (norms N1-N18), independent reviews (R1/R2/R3) and the documentation review (R4) applied to every increment. | all |
| OBJ-13 | Integrate a mathematics library (libm) behind a portable facade with determinism and licensing policy. | M6 |
| OBJ-14 | Integrate CCSDS SDLS (355.0-B) middleware software: SDLS processing, security-association database and key management/rotation, with a pluggable cryptographic backend. | M7 |
| OBJ-15 | Formal completeness (WP-FV-100): raise Lean coverage to 100% of generated requirements (theorem or explicit `lean_note`), mirror the pure-logic modules in Lean, and add C-level proof tooling (Frama-C/CBMC) alongside. | M5/M8 |
| OBJ-16 | Complete deterministic Ethernet: full 802.1AS gPTP stack and the fabric CoreTSN path (multi-port switch / 802.1CB FRER). | M8 |
| OBJ-17 | Declare and enforce a single C coding style codebase-wide and drive MISRA C:2012 compliance (documented deviations; certified checker = GAP-A4). | M9 |
| OBJ-18 | Emulated-target verification: run the RTEMS binary on Renode (PolarFire SoC) and QEMU to verify boot/PAL/drivers without hardware (partial GAP-A5 mitigation). | M10 |

## 3. Resource and bus interface catalogue

The HAL (OBJ-04) defines one **contract per resource**. "MPFS" indicates the
Microchip RT PolarFire SoC RTPFS160ZT resource; "MSS" = native microprocessor
subsystem, "fabric" = FPGA fabric (reachable via the Fabric Interface
Controllers, FIC). Interfaces are defined for all rows in M3; reference
implementations that are native to the MSS are delivered in M4, while
fabric-attached resources are delivered as a **generic fabric-device contract +
backend** (OBJ-07), without writing FPGA IP.

| Resource | Requested protocol/feature | MPFS resource | Native/fabric | HAL contract | Reference backend (M4) |
| -------- | -------------------------- | ------------- | ------------- | ------------ | ---------------------- |
| Ethernet | 1 Gb/s (OBJ-11), TCP/IP | GEM x2 (RGMII/SGMII) | MSS | `FF_EthMac_t` + `FF_NetIf_t` | GEM driver + RTEMS libbsd/lwIP |
| Ethernet | TSN: 802.1Q (VLAN/PCP), 802.1Qbv, 802.1Qav, 802.1Qci/Qbb, 802.1AS, 1588, 802.1CB | GEM x2 (native TSN block) | MSS (fabric CoreTSN only for multi-port switch/FRER) | `FF_TsnBackend_t` | GEM TSN driver |
| Ethernet | pluggable stack (Tuxera / open) | — | — | `FF_NetStack_t` (`socket`/`netif` vtable) | libbsd + lwIP adapters |
| CAN | classic CAN 2.0 A/B (ISO 11898-1) | CAN x2 | MSS | `FF_CanBackend_t` | CAN controller driver |
| CAN | CANopen (CiA 301/302/402) | (over CAN) | — | `FF_CanOpenStack_t` | CANopen stack adapter |
| CAN | CAN-FD | — | fabric via FIC | `FF_CanBackend_t` (FD mode) | generic fabric CAN-FD backend |
| Serial | RS-422 | MMUART x5 + transceiver | MSS | `FF_Serial_t` | MMUART driver |
| Serial | RS-485 (half-duplex, EIA/TIA-485, DE) | MMUART + GPIO DE | MSS | `FF_Serial_t` (DE control) | MMUART + GPIO driver |
| SPI | SPI master/slave | SPI x2 | MSS | `FF_Spi_t` | SPI driver |
| I2C | I2C master | I2C x2 | MSS | `FF_I2c_t` | I2C driver |
| PCIe | PCIe Gen2 EP/RC (enum, BAR, DMA, MSI) | PCIe (XCVR0) | hard block | `FF_Pcie_t` | PCIe driver |
| GPIO | digital IO + IRQ | GPIO (MSS x3 + fabric) | MSS + fabric | `FF_Gpio_t` | GPIO/PLIC driver |
| SpaceWire | ECSS-E-ST-50-12 (link), 50-51 (PID), 50-52 (RMAP), 50-53 (CCSDS PTP) | — | fabric via FIC | `FF_SpaceWire_t` + `FF_Rmap_t` | generic fabric SpaceWire backend |
| Memory | RAM (main) | MSS DDR (DDR3/4/LPDDR4) | MSS | `FF_MemPool_t` + `FF_Dma_t` | static pool + DDR/DMA |
| Memory | NOR flash (boot/config) | QSPI-XIP NOR + eNVM | MSS | `FF_Mtd_t` | QSPI NOR MTD |
| Memory | NVM (persistent params) | eNVM + sNVM + uPROM | MSS/fabric | `FF_Nvm_t` | eNVM/sNVM backend |
| Memory | eMMC / SD | MMC 5.1 (eMMC, SD, SDIO) | MSS | `FF_BlockDev_t` | MMC driver + FTL |
| Memory | SSD | via PCIe NVMe / USB | PCIe/USB | `FF_BlockDev_t` | NVMe backend |
| Time | RTC / timer / watchdog | RTC x1, timer x2, watchdog x5 | MSS | PAL clock (`FF_PalGet*`) + BSP watchdog | RTC/timer driver (M4) |
| Mathematics | elementary functions (sqrt/sin/cos/atan2/pow/…) | toolchain `libm` | host glibc / RTEMS newlib | `FF_Math*` facade (`src/math/`) | libm (M6) |
| Security | SDLS (CCSDS 355.0-B): auth/enc/auth-enc + key management | crypto algorithm/HSM | mission/HW | `FF_CryptoBackend_t`, `FF_Sa_t` (`src/security/`) | middleware SDLS core (M7) |

## 4. Milestones

Every milestone is delivered through the non-derogable cycle (norm N15):
requirements (N3) -> implementation -> Lean model (N1) -> tests (N11) ->
traceability (N4) -> documentation (N8) -> independent review (N9/N10/N14) ->
documentation review (N17) -> full cycle green -> commit and push to both
remotes (N7).

### M1 — Baseline (done)

cFS-inspired core, PUS-C services 1/3/5/6/8/9/11/17/20, CCSDS codec, link
layer, six reference applications, Lean models, CI/CD on GitHub and Gitea.
Released as v0.1.0.

### M2 — Time base: PPS capture and CUC timecode (done)

PPS backend vtable + software backend, time-source selection, tone discipline,
leap-at-boundary, CUC framing (7-octet self-describing P + 6-octet PUS on-wire
T). Requirements SSS-REQ-016, SRS-REQ-021..024; NCs from R1/R2/R3 resolved and
delta-re-reviewed (see `docs/reviews/2026-10-10-m2-*-delta-review.md`).

### M3 — Middleware integration & hardware-abstraction interfaces (in progress)

Deliverables (**interfaces and documentation; no drivers**):

- HAL contract headers under `src/hal/` for every row of section 3, with the
  generic, stack-independent `FF_NetStack_t` and `FF_TsnBackend_t` contracts
  (OBJ-04/OBJ-05/OBJ-06). Header-only contracts; no implementation.
- `docs/architecture/hardware-abstraction.md`: the HAL design and the
  "specificities the middleware needs" (buffer ownership, DMA + RISC-V cache
  coherency, ISR/task split, bounded latency, no dynamic allocation, endianness,
  error model, half-duplex/DE, packet-vs-stream framing).
- Repository/publication partitioning (OBJ-07): separate, selectable CMake
  targets and directories (`src/hal/`, `src/drivers/`, `src/bsp/mpfs/`) with a
  documented `FF_HAL_*`/driver CMake option set, so a user takes only what they
  need.
- **User & Integration Manual** `docs/engineering/user-integration-manual.md`
  (OBJ-02) and completed `docs/engineering/service-development.md` (OBJ-03).
- **V&V approach** documentation (OBJ-08): what is verified (unit/Lean/smoke),
  what is not (hardware gaps), and how without flight hardware.
- Corrected **architecture diagrams** (OBJ-09): software layers on the MPFS,
  graphically marked implemented-in-this-repo vs interface-only vs user-provided.
- README/AGENTS **middleware statement** (OBJ-01) and the development-plan link.

### M4 — MPFS low-level layers and 1 Gb/s Ethernet (in progress)

Deliverables (reference implementations behind the M3 contracts):

- `src/bsp/mpfs/` extended with the native MSS peripheral implementations
  (MMUART/RS-422/RS-485, CAN 2.0, SPI, I2C, QSPI NOR MTD, MMC/eMMC/SD block
  device, GPIO/IRQ, DDR/DMA, RTC/timer) behind the HAL (OBJ-10).
- **1 Gb/s Ethernet driver** on the GEM (`src/drivers/eth/`), integrated with
  the middleware: `FF_EthMac_t` -> `FF_NetIf_t` -> link layer, plus the
  pluggable `FF_NetStack_t` adapters (libbsd, lwIP) and the `FF_TsnBackend_t`
  seam (OBJ-11/OBJ-05).
- Generic **fabric-device contract** backend path (FIC/AXI) for fabric-attached
  resources (CAN-FD, SpaceWire/RMAP, TSN), no FPGA IP (OBJ-07).
- IP + TSN over PUS-C (segmentation/reassembly, TCP/IP and TSN link backends)
  and the on-board file transfer set (virtual filestore, PUS service 23, CFDP),
  absorbed here per the agreed plan.

### M6 — Mathematics library integration (libm) (in progress)

Deliver a documented, portable mathematics facade over the platform `libm`
(`src/math/ff_math.{c,h}`, `FF_USE_LIBM`), so applications can use elementary
math functions (geometry, attitude, transforms) with explicit determinism and
licensing implications. Requirements SSS-REQ-018 / SRS-REQ-033; ADR-0012;
policy `docs/architecture/numerics.md`. The core remains integer/deterministic.

### M7 — CCSDS SDLS (Space Data Link Security) (in progress)

Middleware-side link-layer security: SDLS processing, security-association
database and key management/rotation, with a pluggable cryptographic backend
(`src/security/ff_sdls.{c,h}`). Requirements SSS-REQ-019 / SRS-REQ-034/-035;
ADR-0013; `docs/architecture/security-sdls.md`. The certified algorithm, HSM
keystore, IV-uniqueness analysis and ground key management are GAP-A14.

### M8 — Deterministic Ethernet completion: gPTP (802.1AS) and fabric CoreTSN (planned)

Full 802.1AS gPTP stack (BMCA, Sync/Follow_Up, PDelay, servo disciplining the
GEM TSU) and the fabric CoreTSN path (multi-port switch / 802.1CB FRER) via the
FIC generic fabric-device contract. Requirements, Lean models where feasible,
tests, docs; on-target timing = GAP-A5. (OBJ-16.)

### M9 — Code quality: MISRA C:2012 and coding style (in progress)

Declare a single C coding style (`.clang-format` normative) applied and enforced
codebase-wide (`docs/engineering/coding-style.md`), and drive MISRA C:2012
compliance with the cppcheck MISRA scan (`tools/scripts/misra.sh`, CI advisory →
blocking) and a documented deviation process (`docs/quality/misra-c.md`).
Certified MISRA checker + formal report = GAP-A4. (OBJ-17.)

### M10 — Emulated-target verification: Renode (PolarFire SoC) and QEMU (planned)

Run the RTEMS target binary on an emulated PolarFire SoC (Renode `polarfire-soc`)
and/or QEMU (`riscv/rv64imafdc`) to verify boot, PAL/RTEMS and drivers without
hardware; partially mitigates GAP-A5. `docs/vv/emulated-target.md`,
`tools/scripts/emulated_run.sh`, requirement SRS-REQ-043. (OBJ-18.)

### M5 — Documentation and status program (in progress)

Living documentation: README index, documentation index, repository map, PUS
catalogue, driver inventory, status panel and this development plan (norm N8).

## 5. Verification without flight hardware (summary)

Full detail in `docs/vv/verification-plan.md` and
`docs/vv/verification-results.md`; the approach in one paragraph:

- the **same C sources** run as a native POSIX-thread simulator and are unit
  tested (method Tst) with deterministic assertions;
- **Lean 4 core** (no Mathlib; integer arithmetic only, no math library) proves
  the requirement-level properties of the model (method Anl), linked in the
  traceability manifest;
- **integration/smoke** boots six reference applications and exercises the link;
- **cross-compilation** to RTEMS 6.2 `riscv/mpfs64imafdc` proves portability;
- **independent reviews** (R1/R2/R3/R4) and NC closure per norm N12/N17.

Not verified without hardware (documented class-A gaps, not hidden): real MSS
registers, throughput/latency, DMA and cache coherency, ISR latencies, RTEMS
scheduling on silicon (GAP-A5/A7, GAP-A4 stack/WCET). The HAL seam confines this
residual risk to the BSP/driver layer; board bring-up closes it.

## 6. Objective-to-milestone status

| Milestone | Objectives | Status |
| --------- | ---------- | ------ |
| M1 | baseline middleware | done |
| M2 | time base | done |
| M3 | OBJ-01..OBJ-09 | in progress |
| M4 | OBJ-07, OBJ-10, OBJ-11, OBJ-05 (backends), filestore/CFDP/IP/TSN | in progress |
| M5 | documentation & status program | in progress |
| M6 | OBJ-13 (mathematics library integration) | in progress |
| M7 | OBJ-14 (CCSDS SDLS security + key management) | in progress |
| M8 | OBJ-16 (deterministic Ethernet completion: gPTP 802.1AS + fabric CoreTSN) | planned |
| M9 | OBJ-17 (code quality: MISRA C:2012 + coding style) | in progress |
| M10 | OBJ-18 (emulated-target verification: Renode + QEMU) | planned |

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial development plan: middleware boundary, objectives OBJ-01..OBJ-12, resource/bus interface catalogue, M1..M5 milestones |
| 0.2 | 2026-10-10 | frick-fsw | M3 status set to in progress (HAL interfaces delivered) |
| 0.3 | 2026-10-10 | frick-fsw | Add M6 (mathematics library integration, libm); objective OBJ-13 |
| 0.4 | 2026-10-10 | frick-fsw | Add M7 (CCSDS SDLS security + key management); objective OBJ-14 |
| 0.5 | 2026-10-10 | frick-fsw | Correct TSN resource: the MPFS GEM has a native TSN block (Qbv/Qav/Qci/Qbb/802.1AS/1588/CB); fabric CoreTSN only for multi-port switch/FRER |
| 0.6 | 2026-10-10 | frick-fsw | Add M8 (gPTP 802.1AS + fabric CoreTSN); objective OBJ-16 |
| 0.7 | 2026-10-10 | frick-fsw | Add M9 (code quality: MISRA + coding style); objective OBJ-17 |
| 0.8 | 2026-10-10 | frick-fsw | Add M10 (emulated-target verification); objective OBJ-18 |
