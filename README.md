# frick-fsw

![frick-fsw — codename "Eärendil"](assets/logo-wordmark.png)

**Codename "Eärendil" · v0.1.0 · AGPL-3.0-only**

cFS-inspired flight software framework: PUS-C TM/TC over CCSDS Space Packets
for a small/medium spacecraft OBC built on the **Microchip RT PolarFire SoC
RTPFS160ZT** (RISC-V) under **RTEMS 6.2** (embedded brains ECSS pre-qualified
baseline), target ECSS criticality **B**.

**This is a middleware (framework), not a mission application.** It provides the
command & data handling backbone and the interfaces to integrate your own
applications, drivers and third-party stacks; the six `src/apps/*` are reference
applications for demonstration and V&V. See
[`docs/development-plan.md`](docs/development-plan.md) for the product boundary
and the milestone/objective plan.

> Agent-oriented project guide: **`AGENTS.md`**. Non-derogable norms (rules
> N1-N17): **`docs/rules.md`**. Approach & tools: **`docs/tooling.md`**.
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

Maintained by `tools/scripts/metrics.py` (norm N8/N10); last updated 2026-10-10.

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
| reviews | 🟢 (GREEN) |
| vv records | 🟢 (GREEN) |

**Overall: 🟢 (GREEN)**

Issues deferred by the user: none.

### Coverage

| Metric | Value |
| ------ | ----- |
| Requirements in manifest | 48 (srs=25, sss=16, gap=7) |
| Requirements with implementation | 48/48 |
| Test coverage (requirements with tests) | 41/48 (85.4%) |
| Formal verification coverage (requirements with Lean) | 16/48 (33.3%) |
| Code line coverage | 845/1446 (58.4%) - functions 114/156 |

<!-- METRICS-END -->

## Highlights

- **PUS-C** (ECSS-E-ST-70-41C) on CCSDS Space Packets (CCSDS 133.0-B-2) with
  CRC-16/CCITT-FALSE trailer; pluggable backend: open codec (default) and
  documented commercial PUSopen adapter (optional, `FRICK_USE_PUSOPEN`).
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
(M3); MPFS BSP layers, 1 Gb/s Ethernet driver, PUS service 23 file management +
CFDP, and TCP/IP + TSN transport over PUS-C (M4).

## Low-level drivers

The framework hides the hardware behind three interfaces. Currently available:

| Driver | File | Notes |
| ------ | ---- | ----- |
| PAL (POSIX) | `src/core/pal/pal_posix.c` | host simulator and RTEMS 6 POSIX surface |
| PAL (RTEMS) | `src/core/pal/pal_rtems.c` | RTEMS bootstrap (POSIX keys, clock, panic) |
| Link loopback | `src/transport/link/link_loopback.c` | in-memory loopback (simulator/tests) |
| Link UART | `src/transport/link/link_uart.c` | serial link (host `/dev/ttyUSB0` or RTEMS console) |
| Board (MPFS) | `src/bsp/mpfs/board.c` | RTPFS160ZT bring-up placeholder behind `FF_BoardInit` |

No external device drivers (SpaceWire, CAN, Ethernet/IP, TSN, PPS) are
implemented yet; they are interface-milestone deliverables (M3: HAL contracts)
and reference-implementation deliverables (M4: MPFS BSP + 1 Gb/s Ethernet). See
the detailed **driver development and integration guide** for humans and agents:
`docs/engineering/driver-development.md` and the
[`docs/development-plan.md`](docs/development-plan.md).

## Architecture

```mermaid
flowchart TB
    GSW["Ground station<br/>tools/gsw/ff_gsw.py"]

    subgraph APPS["Applications - 6 tasks"]
        direction LR
        A1["CMD_INGEST"]
        A2["TM_OUTPUT"]
        A3["SCHEDULER"]
        A4["HK"]
        A5["HS"]
        A6["SAMPLE"]
    end

    subgraph PUS["PUS-C layer - ECSS-E-ST-70-41C"]
        direction LR
        P1["TC dispatch and S1 verification"]
        P2["Services S1 S3 S5 S6 S8 S9 S17 S20"]
        P3["Backend vtable FF_Pus_Backend_t"]
    end

    subgraph CORE["Core services"]
        direction LR
        C1["Software Bus"]
        C2["Event Services"]
        C3["Executive Services"]
        C4["Table Services"]
        C5["Time Services"]
    end

    subgraph TRANSPORT["CCSDS transport"]
        direction LR
        T1["Space Packet codec and CRC-16 CCITT-FALSE"]
        T2["Link layer - loopback and UART"]
    end

    subgraph PLATFORM["Platform"]
        direction LR
        PL1["PAL - POSIX and RTEMS"]
        PL2["BSP RTPFS160ZT boot"]
        PL3["RTEMS 6.2 riscv/mpfs64imafdc"]
    end

    GSW <-->|"TC and TM frames"| T2
    T2 <--> T1
    T1 <--> P3
    P3 <--> P1
    P1 <--> P2
    P2 <--> C1
    C2 -.->|"events to S5"| P2
    C1 <--> A1
    C1 <--> A2
    C1 <--> A3
    C1 <--> A4
    C1 <--> A5
    C1 <--> A6
    C5 --- C1
    C4 --- C1
    C3 --- C1
    C1 --- PL1
    PL1 --- PL2
    PL2 --- PL3
```

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

# RTEMS 6.2 cross build (toolchain: tools/scripts/setup_rtems.sh):
make rtems
```

## Requirements set (current)

SRS = 25 · SSS = 16 · GAP (to class A) = 7 · Lean theorems = 41. The generated
status panel above is authoritative; the single source of truth is
`docs/requirements/traceability.yaml` (mirrored in
`docs/requirements/traceability.md`). See `docs/requirements/`.

## Repository layout

| Path | Content |
| ---- | ------- |
| `src/` | C framework (core, transport, PUS, apps, BSP, main) |
| `config/` | single configuration point + mission tables + platform headers |
| `tests/` | unit tests (ctest) |
| `verification/lean/` | Lean 4 formal verification project |
| `tools/` | scripts, ground station, Docker V&V environment |
| `assets/` | project marks (logo) for codename "Eärendil" |
| `docs/` | rules, requirements, architecture, ADRs, ICD, quality, V&V, reviews |
| `board/` | device-tree template for the RTPFS160ZT |
| `.github/`, `.gitea/` | CI/CD pipelines |

## Documentation index

- **Rules & norms**: [`docs/rules.md`](docs/rules.md) (N1-N17) ·
  [`AGENTS.md`](AGENTS.md)
- **Requirements**: [`docs/requirements/SSS.md`](docs/requirements/SSS.md) ·
  [`SRS.md`](docs/requirements/SRS.md) ·
  [`traceability.yaml`](docs/requirements/traceability.yaml) ·
  [`traceability.md`](docs/requirements/traceability.md)
- **Architecture**: [`docs/architecture/overview.md`](docs/architecture/overview.md) ·
  [`pus-layer.md`](docs/architecture/pus-layer.md) ·
  [`time-and-timecode.md`](docs/architecture/time-and-timecode.md) ·
  [`docs/adr/`](docs/adr/) · [`docs/interfaces/ICD.md`](docs/interfaces/ICD.md)
- **Quality**: [`docs/quality/ecss-criticality-b.md`](docs/quality/ecss-criticality-b.md) ·
  [`class-a-gaps.md`](docs/quality/class-a-gaps.md) ·
  [`licensing.md`](docs/quality/licensing.md)
- **V&V**: [`docs/vv/verification-plan.md`](docs/vv/verification-plan.md) ·
  [`verification-results.md`](docs/vv/verification-results.md) ·
  [`tool-qualification.md`](docs/vv/tool-qualification.md)
- **Reviews**: [`docs/engineering/independent-review.md`](docs/engineering/independent-review.md) ·
  [`docs/reviews/`](docs/reviews/)
- **Engineering guides**: [`driver-development.md`](docs/engineering/driver-development.md) ·
  [`service-development.md`](docs/engineering/service-development.md) ·
  [`user-integration-manual.md`](docs/engineering/user-integration-manual.md)
- **Formal verification**: [`docs/verification/formal-verification.md`](docs/verification/formal-verification.md)
- **Planning**: [`docs/development-plan.md`](docs/development-plan.md) ·
  [`docs/roadmap.md`](docs/roadmap.md) · [`docs/tooling.md`](docs/tooling.md)
- **Status & reference**: [`docs/status/status.json`](docs/status/status.json) ·
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
License, version 3** (`AGPL-3.0-only`, `LICENSE`). Every authored file carries
the copyright header and `SPDX-License-Identifier: AGPL-3.0-only`; third-party
components retain their own licences (analysis in
`docs/quality/licensing.md`, `NOTICE`).

## Mirrors

- GitHub: https://github.com/ilfrick/frick-fsw
- Gitea (home): https://www.housefz.com/git/ilfrick/frick-fsw

## Acknowledgments

Architectural inspiration from NASA cFS (Draco); RTEMS 6.2 baseline by embedded
brains; Lean 4 by the Lean community. The project is independent and contains
no copied cFS/RTEMS code.