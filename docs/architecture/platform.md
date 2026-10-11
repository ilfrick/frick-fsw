# Platform and Portability Architecture

| Document id | FRICK-ARCH-0004 |
| ----------- | --------------- |
| Revision    | 0.4 |
| Date        | 2026-10-12 |

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
| qemu-virt-rtems | `pal_rtems.c`  | generic RTEMS 6.2 `riscv/rv64imafdc` on QEMU `virt` (emulated-target verification, M10; `FF_PLATFORM=rtems-generic`); no MPFS peripherals |

Platform-specific configuration: `config/platform/platform_native.h`,
`config/platform/platform_mpfs.h` and `config/platform/platform_qemu.h`; CMake
selects via `FF_PLATFORM` (`native` | `rtems` | `rtems-generic`, where the RTEMS
values define `FRICK_PLATFORM_MPFS` / `FRICK_PLATFORM_QEMU` respectively).

## 3. Board support

`src/bsp/mpfs/board.c` initialises the board context for the RTPFS160ZT
(FCVG784): boot core selection (`FF_MPFS_BOOT_HARTID=1`, first U54), CPU
frequency (625 MHz), UART console (`/dev/console`, 115200 baud). The device
tree template lives in `board/mpfs-rtpfs160zt.dts`.

RTEMS application integration (configuration table + `POSIX_Init` entry point,
build and verification) is documented in
[`rtems-integration.md`](rtems-integration.md); the configuration table is
`config/rtems/rtems_config.c` and the POSIX/RTEMS glue is `pal_rtems.c`.

## 4. Memory model

Static allocation everywhere; no heap usage after init (SRS-REQ-015 /
SSS-REQ-010). `FF_MEMORY_POOL_SIZE` reserves the target memory budget.

## 5. Traceability

SSS-REQ-001, SRS-REQ-015, SRS-REQ-016.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial platform and portability note (norm N8/N16) |
| 0.2 | 2026-10-10 | frick-fsw | M4: note GEM native TSN (Qbv/Qav/Qci/Qbb/802.1AS/1588/CB); fabric CoreTSN only for multi-port/FRER |
| 0.3 | 2026-10-10 | frick-fsw | M4: reference RTEMS application integration (rtems_config.c, POSIX_Init) |
| 0.4 | 2026-10-12 | frick-fsw | M10: add the generic `qemu-virt-rtems` platform (`FF_PLATFORM=rtems-generic`, `platform_qemu.h`) |
