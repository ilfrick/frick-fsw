# Architecture Overview

| Document id | FRICK-ARCH-0001 |
| ----------- | --------------- |
| Revision    | 0.1.1 |
| Date        | 2026-10-10 |

## 1. System context

frick-fsw executes on the Microchip **RT PolarFire SoC RTPFS160ZT** (FCVG784)
under **RTEMS 6.2** (embedded brains ECSS pre-qualified baseline, BSP
`riscv/mpfs64imafdc`) and, for development/V&V, as a native POSIX-thread
simulator. It is the command & data handling backbone of a small/medium
spacecraft: it accepts PUS-C telecommands and produces PUS-C telemetry over
CCSDS Space Packets, manages housekeeping, events, time, health & safety and
configuration tables.

```
 ground station <--UART/loopback--> frick-fsw
                                        |
     +----------------------------------+----------------------------------+
     | frick-fsw framework                                                     |
     |  +--------+  +----+  +----+  +----+  +-----+  +------+                |
     |  | PAL    |  | SB |  | EVS|  | ES |  | TBL |  | TIME |   core         |
     |  +--------+  +----+  +----+  +----+  +-----+  +------+                |
     |  +--------------+  +-------------------+  +------------------+        |
     |  | PUS layer (S1,S3,S5,S6,S8,S9,S17,S20)|  | CCSDS codec+CRC  |        |
     |  +--------------+  +-------------------+  +------------------+        |
     |  +------------------+  +------------------+                            |
     |  | link (loopback/uart)| | apps (6 tasks)  |                         |
     |  +------------------+  +------------------+                            |
     +------------------------------------------------------------------------+
     Host simulator: POSIX threads + loopback     Target: RTEMS 6 RISC-V + UART
```

## 2. Component architecture (Mermaid)

The layered view below is the machine-checkable companion of the ASCII sketch
in section 1; it renders on GitHub and Gitea.

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

## 3. Component overview

| Component | Responsibility | Key files |
| --------- | -------------- | --------- |
| PAL (portable abstraction layer) | threads, time, queues, logs, consoles for POSIX and RTEMS | `src/core/pal/` |
| Software Bus (SB) | publish/subscribe with fixed-size pipes and wildcard subscriptions | `src/core/sb/` |
| Event Services (EVS) | event log and event listeners (PUS S5 forwarding) | `src/core/evs/` |
| Executive Services (ES) | application registry, lifecycle, reset counters | `src/core/es/` |
| Table Services (TBL) | load / validate / activate of mission tables | `src/core/tbl/` |
| Time Services (TIME) | UTC correlation, CUC encoding | `src/core/time/` |
| CCSDS transport | Space Packet encode/decode, CRC-16/CCITT-FALSE | `src/transport/ccsds/` |
| Link layer | byte-stream transport (loopback / UART) | `src/transport/link/` |
| PUS layer | TC dispatch, S1 verification, TM emission, open backend vtable | `src/pus/` |
| Applications | scheduler, command ingest, TM output, HK, HS, sample telemetry | `src/apps/` |
| Board support | RTPFS160ZT init, boot core selection | `src/bsp/mpfs/` |

## 4. Boot sequence

1. PAL init -> SB init -> EVS init -> ES init -> TBL init -> TIME init ->
   board init -> mission tables init (fsw_main.c).
2. PUS init with the open backend; link init (loopback/UART).
3. EVS listener forwards every event to PUS service 5 telemetry.
4. Six applications are registered with ES and started as tasks.

## 5. Threading and determinism

- Each application runs as an independent task with a fixed stack and a
  subscription pipe of fixed depth; all memory is static (no heap after init,
  SRS-REQ-015/SSS-REQ-010).
- Inter-app communication only via the Software Bus and events; no shared
  mutable state outside explicit services.

## 6. Related documents

- `software-bus.md`, `pus-layer.md`, `platform.md`, `applications.md`,
  `mapping-cfs.md`, `docs/interfaces/ICD.md`, `docs/adr/`.

## 7. History

| Rev   | Date       | Author    | Reason |
| ----- | ---------- | --------- | ------ |
| 0.1   | 2026-10-10 | frick-fsw | Initial architecture overview |
| 0.1.1 | 2026-10-10 | frick-fsw | Added the Mermaid component architecture diagram (norm N8) |