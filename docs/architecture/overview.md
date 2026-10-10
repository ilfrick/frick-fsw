# Architecture Overview

| Document id | FRICK-ARCH-0001 |
| ----------- | --------------- |
| Revision    | 0.1 |
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

## 2. Component overview

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

## 3. Boot sequence

1. PAL init -> SB init -> EVS init -> ES init -> TBL init -> TIME init ->
   board init -> mission tables init (fsw_main.c).
2. PUS init with the open backend; link init (loopback/UART).
3. EVS listener forwards every event to PUS service 5 telemetry.
4. Six applications are registered with ES and started as tasks.

## 4. Threading and determinism

- Each application runs as an independent task with a fixed stack and a
  subscription pipe of fixed depth; all memory is static (no heap after init,
  SRS-REQ-015/SSS-REQ-010).
- Inter-app communication only via the Software Bus and events; no shared
  mutable state outside explicit services.

## 5. Related documents

- `software-bus.md`, `pus-layer.md`, `platform.md`, `applications.md`,
  `mapping-cfs.md`, `docs/interfaces/ICD.md`, `docs/adr/`.