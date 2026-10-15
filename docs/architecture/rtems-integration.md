# RTEMS Integration

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0011                        |
| Revision                          | 0.2                                    |
| Date                              | 2026-10-13                             |
| Norms                             | N3, N4, N8                             |
| Requirements                      | SSS-REQ-001, SRS-REQ-037               |

## 1. Purpose

This document describes how frick-fsw is integrated with **RTEMS 6.2** on the
Microchip RT PolarFire SoC RTPFS160ZT (BSP `riscv/mpfs64imafdc`): the PAL
interface, the RTEMS configuration, the build, the initialisation sequence, the
ISR/task rules and how the integration is verified.

## 2. Interface: the PAL

All OS access goes through the **Portable Abstraction Layer** (`src/core/pal/pal.h`):
tasks, mutexes, message queues, time, console, log, panic. Two backends exist:

| File | Role |
| ---- | ---- |
| `src/core/pal/pal_posix.c` | reference implementation over the POSIX API (used on the host and on RTEMS, which provides POSIX threads/mutexes/mqueues/semaphores/time) |
| `src/core/pal/pal_rtems.c` | RTEMS-specific glue: POSIX API configuration check and the `POSIX_Init()` application entry point |

On RTEMS, `POSIX_Init()` calls the shared bring-up `ff_fsw_main()`
(`src/main/fsw_main.c`) and then `rtems_fatal(EXIT)`. The host uses `main()`
(`src/main/main_native.c`, excluded on RTEMS via `FF_HAS_RTEMS`).

## 3. RTEMS configuration

`config/rtems/rtems_config.c` is the single translation unit carrying the RTEMS
Configuration Table (`CONFIGURE_*` + `<rtems/confdefs.h>`) for the
`riscv/mpfs64imafdc` BSP. It is bounded and static (no unlimited objects):

- console + clock drivers;
- classic objects: tasks, timers, semaphores;
- POSIX objects required by the PAL: threads (32), timers (8), keys (8), message
  queues (8), file descriptors (16); POSIX mutexes and condition variables are
  self-contained in RTEMS 6 (no `CONFIGURE_*` option);
- `CONFIGURE_POSIX_INIT_THREAD_TABLE` with a 256 KiB stack, so `POSIX_Init` is
  the entry point;
- tick = 1 ms.

The PAL confirms at build time that the POSIX API is enabled
(`#if !defined(RTEMS_POSIX_API) #error`). "RTEMS 6.2" refers to the RTEMS
Source Builder release tag `6.2` from the RTEMS project; the kernel may report
an internal version string (e.g. `6.0.0`), which is normal for RTEMS releases.

## 4. Build

The baseline is the **open-source RTEMS 6.2** release from the RTEMS project
(`gitlab.rtems.org`, source-builder ref `6.2`); no vendor/commercial baseline or
pre-built toolchain is used. Toolchain: `tools/scripts/setup_rtems.sh` builds the
riscv toolchain and the RTEMS 6.2 kernel + `riscv/mpfs64imafdc` BSP with the RTEMS
Source Builder into `$RTEMS_HOME/6` (default `/opt/rtems/6`), or run the build in
the Docker environment (`tools/docker/rtems/Dockerfile`). The base image is
digest-pinned, but apt resolution and the RSB release reference are mutable;
bit-for-bit reproducibility is not established (NC-EXT-40). Then:

```bash
RTEMS_HOME=/opt/rtems make rtems        # cmake with cmake/toolchains/rtems-riscv-mpfs.cmake
# or, using the Docker environment without a host toolchain (NC-EXT-40):
make rtems-toolchain                    # build the open RTEMS 6.2 toolchain image (T18)
make rtems-docker                       # cross compile inside the image
```

The toolchain file selects `riscv-rtems6-gcc` with `-march=rv64imafdc -mabi=lp64d
-mcmodel=medany`, the BSP sysroot, links with `-qrtems` (RTEMS 6 provides the
linker script/memory map), and does **not** override the load address. The RTEMS
build adds `config/rtems/rtems_config.c`; `main_native.c` is excluded.

## 5. Initialisation sequence

```
RTEMS boot -> POSIX_Init()            [src/core/pal/pal_rtems.c]
  -> ff_fsw_main()                    [src/main/fsw_main.c]
       -> FF_PalInit / FF_PalPlatformInit (checks POSIX API)
       -> SB, EVS, ES, TBL, TIME, board, mission tables
       -> PUS (open CCSDS backend, default; optional N7/ESA ASN.1), link
       -> register + start the applications
```

## 6. ISR / task rules

PAL mutexes are blocking primitives and are **not ISR-safe**. Interrupt handlers
must do the minimum and defer work to a task (this rule is applied by the time
base, the GPIO HAL contract and the SDLS/registry modules). See
`docs/engineering/driver-development.md`.

## 7. Verification

- **Cross-compilation** `riscv/mpfs64imafdc`: `tools/scripts/ci_rtems_cross_build.sh`
  performs the **real** cross build (and **fails** on any build error) when the
  toolchain is present; the per-commit CI job is a documented **proxy** that
  SKIPs (exit 0) when the toolchain is absent, while the scheduled/unattended
  workflow installs the toolchain and performs the real build.
- **Host POSIX**: the same sources build as the native simulator; logic is
  verified there.
- **Runtime on target** (link, PHY, timing, stack/WCET): not exercised without the
  board -> **GAP-A5**.

## 8. Gaps

GAP-A5 (on-target bring-up), GAP-A4 (WCET/stack), GAP-A7 (on-target PPS). The
per-commit cross build is a documented proxy when the toolchain is absent; the
scheduled/unattended workflow installs the toolchain and performs the real
cross build. Making the per-commit build real (toolchain in the V&V image) is
recommended for a release.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial RTEMS integration guide (config table, entry point, build, verification) |
| 0.2 | 2026-10-13 | frick-fsw | Disclose mutable apt/RSB inputs and remove unsubstantiated reproducibility claim (NC-EXT-40). |
