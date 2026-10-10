# Platform and Portability Architecture

| Document id | FRICK-ARCH-0004 |
| ----------- | --------------- |
| Revision    | 0.1 |
| Date        | 2026-10-10 |

## 1. Purpose

One framework source set must build for the host simulator and for the flight
target (SSS-REQ-011). The Portable Abstraction Layer (PAL) is the seam.

## 2. PAL

`src/core/pal/pal.h` defines operations over RTOS primitives: init, task
create/start/delay, queues, mutexes, logging and panics.

| Platform | Implementation | Notes |
| -------- | -------------- | ----- |
| native   | `pal_posix.c`  | POSIX threads; used by the simulator and unit tests (`FF_PLATFORM=native`) |
| rtpfs160zt | `pal_rtems.c`  | RTEMS 6.2 `riscv/mpfs64imafdc`; POSIX API available on RTEMS (`FF_HAS_POSIX=1`) |

Platform-specific configuration: `config/platform/platform_native.h` and
`config/platform/platform_mpfs.h`; CMake selects via `FF_PLATFORM`.

## 3. Board support

`src/bsp/mpfs/board.c` initialises the board context for the RTPFS160ZT
(FCVG784): boot core selection (`FF_MPFS_BOOT_HARTID=1`, first U54), CPU
frequency (625 MHz), UART console (`/dev/console`, 115200 baud). The device
tree template lives in `board/mpfs-rtpfs160zt.dts`.

## 4. Memory model

Static allocation everywhere; no heap usage after init (SRS-REQ-015 /
SSS-REQ-010). `FF_MEMORY_POOL_SIZE` reserves the target memory budget.

## 5. Traceability

SSS-REQ-001, SRS-REQ-015, SRS-REQ-016.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial platform and portability note (norm N8/N16) |