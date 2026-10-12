# Renode V&V Environment (PolarFire SoC)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RENODE-001                    |
| Revision                          | 0.16                                   |
| Date                              | 2026-10-12                             |
| Norms                             | N6, N10, N18                           |
| Requirement                       | SRS-REQ-044                            |

## 1. Purpose

A **Renode** based V&V environment that runs the frick-fsw middleware on an
**emulated Microchip PolarFire SoC** (norm **N18**): the same RTEMS binary that
targets the RTPFS160ZT board is **designed to be executed** against the Renode
PolarFire SoC board (`beaglev-fire`, which includes the SoC platform), providing
functional validation without hardware
(partial GAP-A5 mitigation).

## 2. Environment

| Element | Value |
| ------- | ----- |
| Emulator | Renode (official Antmicro image, pinned by digest `sha256:7905c3…`) |
| Platform | `@platforms/boards/beaglev-fire.repl` (includes `cpus/polarfire-soc.repl`: E51 + 4x U54, UART/GEM/CAN/SPI/I2C/GPIO/PCIe/SD/eNVM/timers; the board adds the **QSPI NOR flash** `SPI.MPFS_QSPI` + `SPI.Micron_MT25Q`, `MPFS_SystemServices`, a board button on GPIO0) |
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
[renode-test] hal_eth=PASS
[renode-test] hal_net=PASS
[renode-test] hal_nvm=PASS
[renode-test] hal_can=PASS
[renode-test] hal_i2c=PASS
[renode-test] hal_spi=PASS
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
headless on the PolarFire SoC board platform (`beaglev-fire`), loads the image,
captures the UART and
greps for `RENODE_TEST: PASS`. See also `make renode-env`
(`tools/docker/renode/renode_env.resc`), the platform-load self-test.

## 5. Validation status (verified)

- **Environment** (`make renode-env`, tool T17): the pinned Renode image runs,
  the PolarFire SoC board platform (`beaglev-fire`) loads and exits cleanly
  (bounded, reproducible).
- **Functional run (real)** (`make renode`, norm N18): with the open-source
  RTEMS 6.2 toolchain image (`tools/docker/rtems`, T18) the middleware image is
  built (`make renode-image`) and run on the emulated PolarFire SoC; the console
  shows the banner and `RENODE_TEST: PASS`. Boot: U54_1 (hart 1) at the image
  entry `0x1000000000` (PolarFire SoC/HSS high DDR alias), console UART
  `mmuart1`. Recorded in `docs/vv/verification-results.md`.
- **HAL coverage (milestone M11, in progress):** the functional run exercises
  HAL backends and, additionally, a **contract check**:
  - **Peripheral-bound** (9/13): **GPIO** (MSS GPIO, SRS-REQ-047), **SERIAL**
    (MSS MMUART, SRS-REQ-048), **MEM** (static RAM pool + reference DMA,
    SRS-REQ-049), **ETH** (Cadence GEM identification, SRS-REQ-036), **NVM**
    (MSS eNVM data region write/read round trip, SRS-REQ-055), **CAN**
    (MSS CAN register round trips, SRS-REQ-057), **I2C** (MSS I2C register
    round trips, SRS-REQ-058), **SPI** (MSS SPI register round trips,
    SRS-REQ-069) and **BLOCK** (MSS MMC/SD register round trips, SRS-REQ-070).
    The app checks
    each and the marker is `PASS` only when all succeed (`hal_gpio=PASS`,
    `hal_serial=PASS`, `hal_mem=PASS`, `hal_eth=PASS`, `hal_nvm=PASS`,
    `hal_can=PASS`, `hal_i2c=PASS`, `hal_spi=PASS`, `hal_block=PASS`). The
    MEM pool is exercised over the in-image RAM (emulated DDR); the reference
    DMA cache hooks are a seam (real cache ops = GAP-A5). The ETH, NVM, CAN and
    I2C checks are MMIO identification/round-trip checks; the full driver path is
    host-tested (the reference DMA seam applies to MEM/ETH only, not to I2C).
    *Scope limit (CAN):* the emulated `MPFS_CAN` models only the AMR identifier
    field, not the AMR RTR/IDE mask bits (bits 1-2); the register check therefore
    uses the fully-modelled registers, and the RX acceptance filter is verified in
    the host unit test (identifier domain).
    *Scope limit (I2C):* the emulated `I2C.MPFS_I2C` does not model the
    Control-register clock-divider bits (CR2:CR1:CR0) — writing them logs a benign
    "unhandled write" warning and they cannot be read back — and it advances the
    transaction on register writes rather than on the SI clear the hardware uses,
    so the register check asserts the core-enable bit (`ENS1`), the idle status
    code, and round-trips the two slave-address registers only; the full
    transaction path (including the serial-clock divider selection and re-derived
    SCL) is verified in the host unit test.
    *Scope limit (SPI):* the emulated `SPI.MPFS_SPI` has no registered slave
    peripheral, so a real master transfer would fault; the register check
    round-trips the control/clock/frame-size/status registers only, and the full
    transfer path (TX/RX, chip-select control, clock-divider selection) is
    verified in the host unit test.
    *Scope limit (BLOCK):* the emulated `SD.MPFS_SDController` has no SD card
    attached, so a real command would log "no SD card" and do nothing; the
    register check round-trips the block-size/block-count, argument and
    capability registers only, and the full transfer path (CMD17/18 read,
    CMD24/25 write, data-buffer flow) is verified in the host unit test.
  - **Contract check** (not peripheral-bound): **NET** — a socket round trip
    through the pluggable `FF_NetStack_t` contract using the reference loopback
    stack (`hal_net=PASS`; in-process).

  Further peripheral HAL classes are added per M11.
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
| 0.8 | 2026-10-11 | frick-fsw | M11: ETH (Cadence GEM identification) exercised (`hal_eth=PASS`); console capture via container-local path + docker cp |
| 0.9 | 2026-10-11 | frick-fsw | M11: NET (pluggable TCP/IP, loopback stack) exercised (`hal_net=PASS`) |
| 0.10 | 2026-10-11 | frick-fsw | V&V environment switched to the BeagleV-Fire board repl (QSPI NOR + system services); NVM (`hal_nvm`) added to the HAL coverage (5/13) |
| 0.11 | 2026-10-11 | frick-fsw | M11: CAN (`hal_can`) added to the HAL coverage (6/13; SRS-REQ-057) |
| 0.12 | 2026-10-11 | frick-fsw | Note the CAN scope limit: the emulated `MPFS_CAN` does not model the AMR RTR/IDE mask bits (R4-0108-11) |
| 0.13 | 2026-10-12 | frick-fsw | M11: I2C (`hal_i2c`) added to the HAL coverage (7/13; SRS-REQ-058); note the emulated `I2C.MPFS_I2C` scope limit (clock-divider bits not modelled; advances on register writes) |
| 0.14 | 2026-10-12 | frick-fsw | Correct the I2C clause: the check is an MMIO round trip with the full driver path host-tested (no I2C DMA path; the reference DMA seam is MEM/ETH only) — FRICK-REVIEW-0129 R2-04 |
| 0.15 | 2026-10-12 | frick-fsw | M11: SPI (`hal_spi`) added to the HAL coverage (8/13; SRS-REQ-069); note the emulated `SPI.MPFS_SPI` scope limit (no registered slave → register round-trips only; the transfer path is host-tested) |
| 0.16 | 2026-10-12 | frick-fsw | M11: BLOCK (`hal_block`) added to the HAL coverage (9/13; SRS-REQ-070); note the emulated `SD.MPFS_SDController` scope limit (no SD card attached → register round-trips only; the transfer path is host-tested) |
