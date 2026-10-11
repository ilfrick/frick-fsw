# frick-fsw

![frick-fsw — codename "Eärendil"](assets/logo-wordmark.png)

**Codename "Eärendil" · v0.1.0 · AGPL-3.0-only**

cFS-inspired flight software framework: PUS-C TM/TC over CCSDS Space Packets
for a small/medium spacecraft OBC built on the **Microchip RT PolarFire SoC
RTPFS160ZT** (RISC-V) under **RTEMS 6.2** (open-source RTEMS project), target
ECSS criticality **B**.

**This is a middleware (framework), not a mission application.** It provides the
command & data handling backbone and the interfaces to integrate your own
applications, drivers and third-party stacks; the six `src/apps/*` are reference
applications for demonstration and V&V. See
[`docs/development-plan.md`](docs/development-plan.md) for the product boundary
and the milestone/objective plan.

> Agent-oriented project guide: **`AGENTS.md`**. Non-derogable norms (rules
> N1-N20): **`docs/rules.md`**. Approach & tools: **`docs/tooling.md`**.
> Licence: **AGPL-3.0-only** (`LICENSE`, `docs/quality/licensing.md`).

## Project index (table of contents)

- [Highlights](#highlights)
- [PUS-C services implemented](#pus-c-services-implemented)
- [Low-level drivers](#low-level-drivers)
- [Architecture](#architecture)
- [Development and V&V process (V-model)](#development-and-vv-process-v-model)
- [Quickstart](#quickstart)
- [Requirements set (current)](#requirements-set-current)
- [Documentation index](#documentation-index)
- [Extending the framework](#extending-the-framework)
- [Repository layout](#repository-layout)
- [Contributing](#contributing)
- [Copyright and license](#copyright-and-license)
- [Mirrors](#mirrors)
- [Acknowledgments](#acknowledgments)

<!-- METRICS-BEGIN -->

## Status panel - coverage and review/V&V traffic lights

Maintained by `tools/scripts/metrics.py` (norm N8/N10); last updated 2026-10-11.

Legend: **🟢 (GREEN)** all pass - **🟡 (YELLOW)** issues deferred by the user - **🔴 (RED)** failures.

### Semaphores

| Gate | State |
| ---- | ----- |
| build and unit tests | 🟢 (GREEN) |
| static analysis | 🟢 (GREEN) |
| simulator smoke | 🟢 (GREEN) |
| lean verification | 🟢 (GREEN) |
| traceability | 🟢 (GREEN) |
| ci github | 🟢 (GREEN) |
| ci gitea | 🟢 (GREEN) |
| documentation set | 🟢 (GREEN) |
| documentation review | 🟢 (GREEN) |
| product assurance review | 🟢 (GREEN) |
| loop audit | 🟢 (GREEN) |
| renode functional validation | 🟢 (GREEN) |
| reviews | 🟢 (GREEN) |
| vv records | 🟢 (GREEN) |

**Overall: 🟢 (GREEN)**

Issues deferred by the user: none.

### Coverage

| Metric | Value |
| ------ | ----- |
| Requirements in manifest | 96 (srs=55, sss=27, gap=14) |
| Requirements with implementation | 96/96 |
| Test coverage (requirements with tests) | 82/96 (85.4%) |
| Formal verification coverage (requirements with Lean) | 20/96 (20.8%) |
| Code line coverage | 1917/2920 (65.7%) - functions 257/302 |

<!-- METRICS-END -->

## Highlights

- **PUS-C** (ECSS-E-ST-70-41C) on CCSDS Space Packets (CCSDS 133.0-B-2) with
  CRC-16/CCITT-FALSE trailer; pluggable backend: the N7 Space / ESA ASN.1/ACN
  PUS-C library (`asn1-pusc-lib`, ASN1SCC-compiled) is the **default**
  (`make pus-asn1`, `FF_USE_PUS_ASN1`), the open CCSDS codec is the fallback, and
  a documented commercial PUSopen adapter is optional (`FRICK_USE_PUSOPEN`).
- **cFS-style core**: PAL, Software Bus, Event Services, Executive Services,
  Table Services, Time Services; six reference applications.
- **Same sources** for the native simulator and the RTEMS 6 target
  (`riscv/mpfs64imafdc`, Microchip RTPFS160ZT).
- **Formal verification with Lean 4** (norm N1): CRC, Software Bus, PUS
  counters, CCSDS length bounds — requirement theorems linked from the
  traceability manifest.
- **Reproducible V&V**: Docker image, GitHub Actions + Gitea Actions via
  `act_runner` (norm N6), traceability gate `tools/scripts/trace.py`.
- **Backwards ECSS engineering data**: SSS, SRS, traceability matrices,
  architecture, ADRs, ICD, quality (class B baseline + class A gaps).
- **Security**: CCSDS **SDLS** (355.0-B) with security-association and
  **key management/rotation**, pluggable cryptographic backend (`src/security/`).
- **Mathematics**: portable **libm** facade (`src/math/`) for deterministic,
  licensed use; no `<math.h>` in the deterministic core.
- **Networking**: pluggable TCP/IP (`FF_NetStack_t`; lwIP reference, Tuxera/custom
  by the user) and native **TSN** on the GEM (802.1Q/Qbv/802.1AS), plus a loopback
  backend (`src/drivers/net/`, `src/hal/net/`).
- **Renode + open-source RTEMS 6.2 V&V** (norm N18): functional validation on an
  emulated PolarFire SoC (`make renode-env`, `make renode`).

## Project scope (what is included)

frick-fsw is a **middleware/framework**: the applications under `src/apps/` are
**reference/demo only** — you write your own on the SB/PUS/HAL seams. Consolidated
scope (implemented / interface-only / planned / user-provided):

| Layer / resource | Implemented now (this repo) | Interface-only | Planned | User-provided |
| ---------------- | --------------------------- | -------------- | ------- | ------------- |
| PAL (tasks/mutex/queue/time/console) | POSIX (host + RTEMS) + RTEMS bootstrap | — | — | — |
| Core services | SB, EVS, ES, TBL, Time (+ PPS) | — | — | — |
| PUS-C | services 1/3/5/6/8/9/11/17/20; **default backend N7/ESA ASN.1** | `FF_Pus_Backend_t` | full fundamental set (**M15**); S20/S23 | PUSopen adapter |
| Transport | CCSDS Space Packets, link (loopback/UART) | `FF_Link_t` | — | radio/SpaceWire back-ends |
| HAL contracts | registry + `hal_*.h` | ETH, NET, TSN, CAN, SERIAL, SPI, I2C, PCIE, GPIO, SPACEWIRE, MEM, NVM, BLOCK | — | any backend |
| Drivers | GEM Ethernet + native TSN; MSS GPIO; MSS MMUART; RAM pool/DMA | — | CAN, SPI, I2C, QSPI/NOR, MMC/eMMC/SD, PCIe, SpaceWire, storage (**M4/M11**) | — |
| Networking | lwIP adapter; loopback stack | `FF_NetStack_t` | — | Tuxera/custom stack |
| Security | CCSDS SDLS + key mgmt/rotation | `FF_CryptoBackend_t` | — | mission crypto/HSM |
| Mathematics | libm facade | — | — | — |
| File transfer | — | — | **CFDP + virtual filestore (VFS) + PUS-C S23** (**M15**) | — |
| Autonomy | — | — | **OBCP engine integration (uPython)** (**M12**) + **in-house OBCP engine** (**M16**); equipment manager + fault mgmt (**M13**) | mission apps |
| Execution cores | single U54 | — | **RTEMS SMP / multicore** (**M14**) | — |
| V&V | host simulator; **Renode (open RTEMS)**; trace/Lean/format gates | — | QEMU run (**M10**); certified MISRA checker (**GAP-A4**) | HWIL (**GAP-A5**) |

## PUS-C services implemented

PUS-C (ECSS-E-ST-70-41C) telecommand dispatch and telemetry over the open
CCSDS backend (`src/pus/pus.c`, `src/pus/pus_services.c`):

| Service | Name | Implemented subservices |
| ------- | ---- | ----------------------- |
| 1 | Request verification | 1,2,4,8 acceptance/start/progress/completion (framework-generated) |
| 3 | Housekeeping | 1 create, 2 delete, 3 enable, 4 disable, 27 report (provider-registered) |
| 5 | Event reporting | 1-3 event messages (via EVS listener) |
| 6 | Memory management | 5 dump (region-bounded, registered regions only) |
| 8 | Function management | 1 function invocation (provider-registered) |
| 9 | Time management | 1 set time (CUC), 2 report time (CUC) |
| 11 | Scheduling | 1,2 stub (returns NOT_IMPLEMENTED) |
| 17 | Test | 1 connection test (ping/pong) |
| 20 | Parameter management | 1 set, 3 report (provider-registered) |

Planned extensions (`docs/roadmap.md`): HAL interfaces + integration manuals
(M3, delivered as interfaces); MPFS BSP layers, 1 Gb/s Ethernet driver, PUS
service 23 file management + CFDP, and TCP/IP + TSN transport over PUS-C (M4).

## Low-level drivers

The framework hides the hardware behind the PAL, the transport link(s) and the
**HAL contracts** (`src/hal/hal_*.h`). Reference implementations available:

| Driver | File | Notes |
| ------ | ---- | ----- |
| PAL (POSIX) | `src/core/pal/pal_posix.c` | host simulator and RTEMS 6 POSIX surface |
| PAL (RTEMS) | `src/core/pal/pal_rtems.c` | RTEMS bootstrap (POSIX keys, clock, panic) |
| Link loopback | `src/transport/link/link_loopback.c` | in-memory loopback (simulator/tests) |
| Link UART | `src/transport/link/link_uart.c` | serial link (host `/dev/ttyUSB0` or RTEMS console) |
| Board (MPFS) | `src/bsp/mpfs/board.c` | RTPFS160ZT bring-up placeholder behind `FF_BoardInit` |
| Ethernet (GEM) | `src/drivers/eth/gem/` | Cadence GEM MAC driver behind `FF_EthMac_t` (M4, host-testable; on-target PHY = GAP-A5) |
| TSN (GEM) | `src/drivers/eth/gem/ff_gem_tsn.c` | GEM native TSN backend behind `FF_TsnBackend_t` (802.1Q/Qbv/802.1AS) |
| Net stack (loopback) | `src/drivers/net/ff_net_loopback.c` | reference `FF_NetStack_t`; third-party stacks (lwIP) plug into the same vtable |
| Serial (MSS MMUART) | `src/drivers/serial/mpfs/ff_serial_mpfs.c` | MSS MMUART (NS16550, 4-byte registers) backend behind `FF_Serial_t`; host test (mock MMIO) + Renode MMIO check (M4/M11) |
| GPIO (MSS) | `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` | MSS GPIO backend behind `FF_Gpio_t`; host test (mock MMIO) + Renode MMIO check (M4/M11) |
| Memory (RAM pool + DMA) | `src/drivers/mem/ff_mem_static.c` | static `FF_MemPool_t` + reference `FF_Dma_t`; host test + Renode DDR check (M4/M11) |

GPIO: `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` implements the `FF_Gpio_t` contract
(bus-injected MMIO, allocation-free, single instance), validated by the host unit
test `test_gpio` (mock MMIO) and by an emulated-MMIO round trip on the Renode
`polarfire-soc` platform (milestone M11; SRS-REQ-047).

Other device drivers (SpaceWire, CAN/CANopen, SPI/I2C, PCIe, storage,
PPS) are interface deliverables (M3: HAL contracts) and reference-implementation
deliverables (M4: MPFS BSP + 1 Gb/s Ethernet + stacks). See the detailed
**driver development and integration guide** for humans and agents:
`docs/engineering/driver-development.md` and the
[`docs/development-plan.md`](docs/development-plan.md).

## Architecture

```mermaid
flowchart TB
    classDef impl fill:#c7f9cc,stroke:#2b8a3e,color:#000
    classDef iface fill:#fff3b0,stroke:#b8860b,color:#000
    classDef m4 fill:#ffe0b3,stroke:#d9822b,color:#000
    classDef user fill:#e0e0e0,stroke:#666,color:#000
    classDef hw fill:#ffd6d6,stroke:#c92a2a,color:#000

    GSW["Ground station (tools/gsw/ff_gsw.py)"]:::user
    MISSION["Mission applications - written by users"]:::user

    A1["Reference applications: CMD_INGEST, TM_OUTPUT, SCHEDULER, HK, HS, SAMPLE"]:::impl
    P1["PUS-C services S1 S3 S5 S6 S8 S9 S11 S17 S20 + S1 verification"]:::impl
    C1["Software Bus"]:::impl
    C2["Event Services"]:::impl
    C3["Executive Services"]:::impl
    C4["Table Services"]:::impl
    C5["Time Services"]:::impl
    T1["CCSDS Space Packet codec + CRC-16"]:::impl
    T2["Link layer: loopback / UART"]:::impl
    PL1["PAL - OS abstraction (POSIX / RTEMS)"]:::impl
    H1["HAL registry + contracts (M3): Eth/Net/TSN, CAN, serial, SPI/I2C/PCIe/GPIO, SpaceWire, memory"]:::iface
    B1["BSP + drivers (M4): MPFS peripherals, 1 Gb/s GEM Ethernet"]:::m4
    HW["Microchip RT PolarFire SoC RTPFS160ZT: MSS peripherals + FPGA fabric"]:::hw

    MISSION --> A1
    GSW <-->|"TC / TM frames"| T2
    A1 <--> C1
    P1 <--> C1
    P1 <-->|"TC/TM"| T1
    C2 -.->|"events to S5"| P1
    T1 <--> T2
    C1 --- C5
    C1 --- C4
    C1 --- C3
    C1 --- PL1
    PL1 --- H1
    H1 --- B1
    B1 --- HW
```

**Legend** — 🟩 implemented in this repository · 🟨 interface/contract delivered
here · 🟧 planned reference implementation (M4) · ⬜ user-provided · 🟥 hardware
/ FPGA IP (not in this repository). The same software runs on the host simulator
and on the MPFS target; see `docs/architecture/hardware-abstraction.md`.

See `docs/architecture/overview.md` for the narrative architecture and
`docs/interfaces/ICD.md` for the packet interfaces.

## Development and V&V process (V-model)

```mermaid
flowchart TB
    NEED["Mission and stakeholder needs"]

    subgraph LEFT["Definition and decomposition"]
        direction TB
        SSS["SSS system requirements"]
        SRS["SRS software requirements"]
        DES["Architecture and detailed design"]
        IMP["Implementation - C, config, tables"]
    end

    subgraph RIGHT["Integration and verification"]
        direction TB
        ACC["Acceptance and validation"]
        SYST["System verification - SRS and SSS"]
        INT["Integration verification"]
        UNIT["Unit verification"]
    end

    NEED --> SSS --> SRS --> DES --> IMP
    UNIT --> INT --> SYST --> ACC
    IMP -->|"build and unit tests"| UNIT
    DES -.->|"integration tests"| INT
    SRS -.->|"system tests and traceability gate"| SYST
    SSS -.->|"validation"| ACC
    NEED -.-> ACC

    GATES["Continuous gates on every change - Lean N1, traceability N4, CI and CD N6, docs N8, independent review N9, full cycle N15, revision history N16"]
    IMP -.- GATES
    ACC -.- GATES
```

The full plan, including the per-change cycle, is in
`docs/vv/verification-plan.md`.

## Quickstart

```bash
# Full local pipeline inside the reproducible V&V container (norm N6):
make ci-local

# Or natively:
make configure build test      # build + unit tests (ctest)
make run                       # boot the native simulator (6 apps)
make verify-lean               # lake build (formal verification, norm N1)
make trace                     # traceability convergence gate (norm N4)

# PUS-C ASN.1 default backend (N7/ESA); regenerate the generated C (build-time):
make pus-asn1                  # ASN1SCC generation; default ON (FF_USE_PUS_ASN1)

# Renode V&V environment (norm N18):
make renode-env                # self-test: Renode runs + polarfire-soc platform loads
make renode-image renode       # build the RTEMS image + emulated functional validation

# RTEMS 6.2 cross build (open-source RTEMS; toolchain: tools/scripts/setup_rtems.sh):
make rtems-toolchain           # build the open RTEMS 6.2 toolchain image
make rtems-docker              # cross compile inside the image (no host toolchain)
make rtems
```

## Requirements set (current)

SRS = 54 · SSS = 26 · GAP (to class A) = 14 · Lean theorems = 51. The generated
status panel above is authoritative; the single source of truth is
`docs/requirements/traceability.yaml` (mirrored in
`docs/requirements/traceability.md`). See `docs/requirements/`.

## Repository layout

| Path | Content |
| ---- | ------- |
| `src/` | C framework (core, transport, PUS, apps, BSP, HAL, drivers, math, security, main) |
| `config/` | single configuration point + mission tables + platform headers |
| `tests/` | unit tests (ctest) |
| `verification/lean/` | Lean 4 formal verification project |
| `tools/` | scripts, ground station, Docker V&V environment |
| `assets/` | project marks (logo) for codename "Eärendil" |
| `docs/` | rules, requirements, architecture, ADRs, ICD, quality, V&V, reviews |
| `board/` | device-tree template for the RTPFS160ZT |
| `.github/`, `.gitea/` | CI/CD pipelines |

## Documentation index

- **Rules & norms**: [`docs/rules.md`](docs/rules.md) (N1-N20) ·
  [`AGENTS.md`](AGENTS.md)
- **Requirements**: [`docs/requirements/SSS.md`](docs/requirements/SSS.md) ·
  [`SRS.md`](docs/requirements/SRS.md) ·
  [`traceability.yaml`](docs/requirements/traceability.yaml) ·
  [`traceability.md`](docs/requirements/traceability.md)
- **Architecture**: [`docs/architecture/overview.md`](docs/architecture/overview.md) ·
  [`pus-layer.md`](docs/architecture/pus-layer.md) ·
  [`time-and-timecode.md`](docs/architecture/time-and-timecode.md) ·
  [`hardware-abstraction.md`](docs/architecture/hardware-abstraction.md) ·
  [`rtems-integration.md`](docs/architecture/rtems-integration.md) ·
  [`numerics.md`](docs/architecture/numerics.md) ·
  [`security-sdls.md`](docs/architecture/security-sdls.md) ·
  [`network-stack.md`](docs/architecture/network-stack.md) ·
  [`component-gap-analysis.md`](docs/architecture/component-gap-analysis.md) ·
  [`pus-c-from-scratch-assessment.md`](docs/architecture/pus-c-from-scratch-assessment.md) ·
  [`design/software-design.md`](docs/design/software-design.md) ·
  [`docs/adr/`](docs/adr/) · [`docs/interfaces/ICD.md`](docs/interfaces/ICD.md)
- **Quality**: [`docs/quality/ecss-criticality-b.md`](docs/quality/ecss-criticality-b.md) ·
  [`ecss-compliance-matrix.md`](docs/quality/ecss-compliance-matrix.md) ·
  [`class-a-gaps.md`](docs/quality/class-a-gaps.md) ·
  [`configuration-management.md`](docs/quality/configuration-management.md) ·
  [`product-assurance.md`](docs/quality/product-assurance.md) ·
  [`safety-and-fdir.md`](docs/quality/safety-and-fdir.md) ·
  [`misra-c.md`](docs/quality/misra-c.md) ·
  [`licensing.md`](docs/quality/licensing.md)
- **V&V**: [`docs/vv/verification-plan.md`](docs/vv/verification-plan.md) ·
  [`validation-plan.md`](docs/vv/validation-plan.md) ·
  [`verification-results.md`](docs/vv/verification-results.md) ·
  [`emulated-target.md`](docs/vv/emulated-target.md) ·
  [`renode-environment.md`](docs/vv/renode-environment.md) ·
  [`tool-qualification.md`](docs/vv/tool-qualification.md)
- **Reviews**: [`docs/engineering/independent-review.md`](docs/engineering/independent-review.md) ·
  [`docs/reviews/`](docs/reviews/)
- **Engineering guides**: [`driver-development.md`](docs/engineering/driver-development.md) ·
  [`service-development.md`](docs/engineering/service-development.md) ·
  [`user-integration-manual.md`](docs/engineering/user-integration-manual.md) ·
  [`coding-style.md`](docs/engineering/coding-style.md)
- **Formal verification**: [`docs/verification/formal-verification.md`](docs/verification/formal-verification.md)
- **Planning**: [`docs/development-plan.md`](docs/development-plan.md) ·
  [`docs/roadmap.md`](docs/roadmap.md) · [`docs/tooling.md`](docs/tooling.md)
- **Status & reference**: [`docs/status/status.json`](docs/status/status.json) ·
  [`docs/status/handover.md`](docs/status/handover.md) (resume point) ·
  [`docs/status/loop-audit.md`](docs/status/loop-audit.md) (norm N19 audit) ·
  [`docs/glossary.md`](docs/glossary.md) · [`CHANGELOG.md`](CHANGELOG.md)

## Extending the framework

- Add or integrate a **low-level driver** (link, board/PPS, HAL backend):
  [`docs/engineering/driver-development.md`](docs/engineering/driver-development.md).
- Add or extend a **PUS-C service**:
  [`docs/engineering/service-development.md`](docs/engineering/service-development.md).
- Integrate your own **application**: see the
  [`docs/engineering/user-integration-manual.md`](docs/engineering/user-integration-manual.md).
- Planned milestones: [`docs/development-plan.md`](docs/development-plan.md) ·
  [`docs/roadmap.md`](docs/roadmap.md).

## Contributing

See `CONTRIBUTING.md` (rules N7/N11/N15/N16 apply: full cycle, dual push,
history, reviews). Security: `SECURITY.md`. License: AGPL-3.0-only.

## Copyright and license

Copyright (C) 2026 **Ilfrick**

- GitHub: <https://github.com/ilfrick/frick-fsw>
- Gitea: <https://www.housefz.com/git/ilfrick/frick-fsw>

frick-fsw is free software, released under the **GNU Affero General Public
License, version 3** (`AGPL-3.0-only`, `LICENSE`). Every authored source,
configuration and build file carries the copyright header and
`SPDX-License-Identifier: AGPL-3.0-only`; engineering documentation is covered
by `LICENSE`/`NOTICE`; third-party components retain their own licences
(analysis in `docs/quality/licensing.md`, `NOTICE`).

## Mirrors

- GitHub: https://github.com/ilfrick/frick-fsw
- Gitea (home): https://www.housefz.com/git/ilfrick/frick-fsw

## Acknowledgments

Architectural inspiration from NASA cFS (Draco); RTEMS 6.2 from the open-source
RTEMS project; Lean 4 by the Lean community. The project is independent and contains
no copied cFS/RTEMS code.

**Codename / third-party name disclaimer.** The project name is **frick-fsw**;
"**Eärendil**" is an internal codename taken from J. R. R. Tolkien's legendarium.
A single name is not protected by copyright; the relevant right, if any, is
trademark. This project is **not affiliated with, endorsed by, or associated
with** the Tolkien Estate or Middle-earth Enterprises, includes no Tolkien text,
and does not use the name as a trademark. "TOLKIEN", "THE LORD OF THE RINGS",
"THE HOBBIT", "MIDDLE-EARTH" and many related names are trademarks of their
owners. See `NOTICE` and `docs/quality/licensing.md §6` (incl. contacts for
licensing and the note that this is not legal advice).
