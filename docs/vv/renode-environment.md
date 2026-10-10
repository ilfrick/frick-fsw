# Renode V&V Environment (PolarFire SoC)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RENODE-001                    |
| Revision                          | 0.1                                    |
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
Bus** (publish/subscribe round trip), **Event Services** and **Time Services**,
then prints a machine-checkable marker:

```
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
#    toolchain in /opt/rtems/6); output build/renode/frick_fsw_renode.elf:
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

## 5. Validation status (honest)

- **Verified now** (`make renode-env`, tool T17): the pinned Renode Docker image
  runs, the `polarfire-soc` platform loads (`System bus created.`) and exits
  cleanly; the check is reproducible from the repo and bounded (cannot hang).
- **Prerequisites to make the gate blocking**: (a) the RTEMS cross toolchain in
  the V&V image to produce `build/renode/frick_fsw_renode.elf`; (b) confirming
  the RTEMS image boot in the `.resc` (the boot-hart PC / multi-hart handling is
  applied in `frick-renode.resc`, mirroring the self-test).
- Until then `renode_vv.sh` is an **advisory proxy** (SKIP/exit 0 when the image
  is absent; it **fails** with exit 1 if the image is present but the marker is
  missing), like the RTEMS cross-build proxy (norm N18 gate). It **must not** be
  read as functional verification until the marker is actually produced.

## 6. Gap linkage

Partial mitigation of GAP-A5; real-silicon residual (timing/WCET/radiation/errata)
stays open (GAP-A4/A6).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial Renode V&V environment (norm N18); test application |
| 0.2 | 2026-10-10 | frick-fsw | Review FRICK-REVIEW-0036 fixes: test app receives inline (marker PASS), CMake option + `renode-image`, `make renode-env` self-test, `renode_vv.sh` fails on missing marker, status wording |
