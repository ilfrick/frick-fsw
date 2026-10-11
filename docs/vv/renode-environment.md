# Renode V&V Environment (PolarFire SoC)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RENODE-001                    |
| Revision                          | 0.7                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N6, N10, N18                           |
| Requirement                       | SRS-REQ-044                            |

## 1. Purpose

A **Renode** based V&V environment that runs the frick-fsw middleware on an
**emulated Microchip PolarFire SoC** (norm **N18**): the same RTEMS binary that
targets the RTPFS160ZT board is **designed to be executed** against the Renode
`polarfire-soc` platform, providing functional validation without hardware
(partial GAP-A5 mitigation).

## 2. Environment

| Element | Value |
| ------- | ----- |
| Emulator | Renode (official Antmicro image, pinned by digest `sha256:7905c3…`) |
| Platform | `@platforms/cpus/polarfire-soc.repl` (E51 + 4x U54, UART/GEM/CAN/SPI/I2C/GPIO/PCIe/SD/eNVM/timers) |
| Image build | `docker build -t frick-renode:local tools/docker/renode` |
| Scripts | `tools/docker/renode/*.resc`, `tools/scripts/renode_vv.sh` |
| Tool qualification | T17 (`docs/vv/tool-qualification.md`) |

## 3. Test application (on the middleware)

`src/apps/app_renode_test.c` (`FF_AppRenodeTestMain`) is a middleware
application used for the Renode functional test: it exercises the **Software
Bus** (publish/subscribe round trip), **Event Services** and **Time Services**
and, on the target platform, a **HAL self-check** (MSS GPIO MMIO round trip,
milestone M11), then prints a machine-checkable marker:

```
[renode-test] hal_gpio=PASS
[renode-test] hal_serial=PASS
[renode-test] hal_mem=PASS
[renode-test] sb_roundtrip=PASS
RENODE_TEST: PASS
```

It is registered **only** when `FF_ENABLE_RENODE_TEST=1` (CMake option
`FF_ENABLE_RENODE_TEST=ON`), so the default reference set remains six
applications and the host smoke is unchanged.

## 4. Running

```bash
# 0. Environment self-test (no toolchain needed): Renode runs + platform loads.
make renode-env

# 1. Build the RTEMS image with the Renode test application (needs the RTEMS
#    toolchain in /opt/rtems/6, from the open-source RTEMS project); output
#    build/renode/frick_fsw_renode.elf. The nightly workflow installs the
#    open-source RTEMS toolchain (tools/scripts/setup_rtems.sh), builds this
#    image and runs the validation.
make renode-image

# 2. Run the emulated functional validation (norm N18):
make renode            # tools/scripts/renode_vv.sh
```

`renode-image` configures an RTEMS cross build with `FF_ENABLE_RENODE_TEST=ON`
(the CMake option that compiles and registers the test application; the default
build keeps six applications). `renode_vv.sh` runs the pinned Renode image
headless on the `polarfire-soc` platform, loads the image, captures the UART and
greps for `RENODE_TEST: PASS`. See also `make renode-env`
(`tools/docker/renode/renode_env.resc`), the platform-load self-test.

## 5. Validation status (verified)

- **Environment** (`make renode-env`, tool T17): the pinned Renode image runs,
  the `polarfire-soc` platform loads and exits cleanly (bounded, reproducible).
- **Functional run (real)** (`make renode`, norm N18): with the open-source
  RTEMS 6.2 toolchain image (`tools/docker/rtems`, T18) the middleware image is
  built (`make renode-image`) and run on the emulated PolarFire SoC; the console
  shows the banner and `RENODE_TEST: PASS`. Boot: U54_1 (hart 1) at the image
  entry `0x1000000000` (PolarFire SoC/HSS high DDR alias), console UART
  `mmuart1`. Recorded in `docs/vv/verification-results.md`.
- **HAL coverage (milestone M11, in progress):** the functional run also exercises
  HAL backends on the emulated peripherals: **GPIO** (MSS GPIO, SRS-REQ-047),
  **SERIAL** (MSS MMUART, SRS-REQ-048) and **MEM** (static RAM pool + reference
  DMA, SRS-REQ-049). The app checks each and the marker is `PASS` only when all
  succeed (`hal_gpio=PASS`, `hal_serial=PASS`, `hal_mem=PASS`). The MEM pool is
  exercised over the in-image RAM (emulated DDR); the reference DMA cache hooks
  are a seam (real cache ops = GAP-A5). Further HAL classes are added per M11.
- **Gate**: `renode_vv.sh` **SKIPs** (exit 0) when the RTEMS image is absent
  (per-commit proxy) and **fails (exit 1)** when the image is present but the
  marker is missing; the nightly workflow performs the real run. Norm N18 is
  **in force**.

## 6. Gap linkage

Partial mitigation of GAP-A5; real-silicon residual (timing/WCET/radiation/errata)
stays open (GAP-A4/A6).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial Renode V&V environment (norm N18); test application |
| 0.2 | 2026-10-10 | frick-fsw | Review FRICK-REVIEW-0036 fixes: test app receives inline (marker PASS), CMake option + `renode-image`, `make renode-env` self-test, `renode_vv.sh` fails on missing marker, status wording |
| 0.3 | 2026-10-10 | frick-fsw | Align the RTOS baseline to the open-source RTEMS 6.2 project; the nightly workflow installs the open RTEMS toolchain (`setup_rtems.sh`) and runs the Renode validation (N18 blocking path) |
| 0.4 | 2026-10-10 | frick-fsw | Real toolchain + run: boot U54_1 at 0x1000000000, console mmuart1, `make renode` -> `RENODE_TEST: PASS`; N18 in force |
| 0.5 | 2026-10-10 | frick-fsw | M11 started: MSS GPIO HAL backend exercised in the run (`hal_gpio=PASS`); marker includes HAL checks |
| 0.6 | 2026-10-10 | frick-fsw | M11: MSS MMUART serial HAL backend exercised (`hal_serial=PASS`) |
| 0.7 | 2026-10-11 | frick-fsw | M11: static RAM pool + DMA HAL backend exercised (`hal_mem=PASS`) |
