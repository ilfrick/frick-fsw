# Emulated-Target Verification (Renode / QEMU)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-EMU-001                       |
| Revision                          | 0.3                                    |
| Date                              | 2026-10-12                             |
| Norms                             | N5, N6, N10                            |
| Requirement                       | SRS-REQ-043                            |

## 1. Purpose

Between the host POSIX simulator and the real RTPFS160ZT board, run the **actual
RTEMS target binary** on an **emulated PolarFire SoC** to verify the boot
sequence, the PAL/RTEMS backend and the drivers against emulated peripherals —
closing part of GAP-A5 without hardware.

## 2. Tools

| Tool | Role | Notes |
| ---- | ---- | ----- |
| **Renode** | full PolarFire SoC emulation | official Microchip/Antmicro platform `polarfire-soc` (and the `beaglev-fire` board repl used by this project, `@platforms/boards/beaglev-fire.repl`): 5 RISC-V64 CPUs (E51 `rv64imac_zicsr_zifencei`, no F/D; 4x U54 `rv64gc_zicsr_zifencei` = `rv64imafdc`), CLINT, PLIC, PDMA, 5x NS16550 UART, SD/eMMC, 2x SPI, 2x I2C, 2x CAN, 2x CadenceGEM (+ PHY), 3x GPIO, watchdog/RTC/timer, eNVM, USB, 2x PCIe, crypto, DDR mock; the **board repl** additionally instantiates the QSPI NOR (`SPI.MPFS_QSPI` + `SPI.Micron_MT25Q`) and `MPFS_SystemServices` (the open `polarfire-soc` platform alone has no QSPI/MT25Q device). Runs the same unmodified binaries; GDB + headless test scripts; UART analyzer for console capture; FPGA fabric co-simulation (Verilator). Runs the **MPFS image** (`make rtems`). |
| **QEMU** | generic RISC-V run | `qemu-system-riscv64 -M virt -nographic -bios <rtems.exe>` runs the RTEMS **generic `riscv/rv64imafdc` BSP** (a separate build from the MPFS image; toolchain file `cmake/toolchains/rtems-riscv-generic.cmake`, targets `make qemu-image` / `make qemu-image-docker`); no MPFS peripherals. The generic BSP links at the QEMU `virt` RAM base `0x80000000`, so `-bios` boots it directly. |

## 3. Scope of verification

- **QEMU**: RTEMS boot, PAL/RTEMS tasking, mutex/queue/clock, `POSIX_Init` ->
  `ff_fsw_main` bring-up, application start (generic `riscv/rv64imafdc` BSP; no
  MPFS peripherals).
- **Renode (`polarfire-soc`)**: the **MPFS image**, plus drivers against emulated
  MSS peripherals (UART, GEM Ethernet, CAN, SPI, I2C, GPIO, PCIe, SD/eMMC);
  tone/PPS and time paths; TM/TC over an emulated link (UART analyzer captures
  the console).
- Both run committed binaries produced by the RTEMS cross build (Renode: the
  MPFS `make rtems` image; QEMU: a generic `rv64imafdc` RTEMS image).

## 4. What it does NOT verify (gaps)

- Real silicon behaviour: electrical/radiation (GAP-A6), errata, exact timing.
- WCET/stack on the real core (GAP-A4).
- Peripheral-model fidelity; fabric IP (M8 co-simulation only).
- Emulator qualification is required (N10).

## 5. Integration plan

- `tools/scripts/emulated_run.sh`: run the RTEMS binary on QEMU and/or Renode
  (`polarfire-soc`) headless when the tool is present. **Advisory**: it reports
  what ran (SKIP when absent) and does not itself constitute verification; the
  emulator is qualified as a tool (T15/T16) and evidence is recorded in
  `docs/vv/verification-results.md` when a run is available.
- `tools/scripts/qemu_vv.sh` (`make qemu`, tool T20): the **binding** QEMU check.
  Builds/loads the generic-BSP image (`make qemu-image`/`qemu-image-docker`) and
  runs it on `qemu-system-riscv64 -M virt` in the toolchain image (which carries
  QEMU + the `rv64imafdc` BSP), captures the UART console to
  `build/rtems-generic/qemu-console.log`, and checks the boot banner +
  application registration. SKIP proxy when the image is absent; FAIL when
  present but the banner is missing.
- CI: the per-commit `ci-emulated-run` step runs `emulated_run.sh` (advisory;
  T15/T16) and the **nightly** workflow (`.github/workflows/nightly.yml`) builds
  the generic image and runs the QEMU check (`make qemu`, T20); evidence recorded
  in `docs/vv/verification-results.md`.
- Tool qualification (N10): Renode and QEMU registered (T15/T16) and the QEMU
  functional check (**T20**, run by the nightly workflow). The QEMU generic build
  carries **no MPFS peripherals**, so
  the QEMU path establishes RTEMS boot / PAL / tasking / framework bring-up only
  (no driver verification); driver verification is Renode's (and on-target = GAP-A5).
  Note: the generic image still links the reference driver translation units
  (`FF_BUILD_DRIVERS` default ON) but none is bound or touched on QEMU (no MMIO
  access), so the evidence is unaffected.

## 6. Gaps linkage

GAP-A5 (on-target) is **partially mitigated** by emulated execution; the residual
(real hardware, timing, environment) remains open and is documented in
`docs/quality/class-a-gaps.md`.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial emulated-target V&V note (Renode PolarFire SoC + QEMU); milestone M10 |
| 0.2 | 2026-10-11 | frick-fsw | M10 closed: QEMU generic-BSP (`riscv/rv64imafdc`) run added (`make qemu-image`/`make qemu`, tool T20, `qemu_vv.sh`); toolchain image (T18) gains the rv64imafdc BSP + QEMU |
| 0.3 | 2026-10-12 | frick-fsw | Review NCs: per-core ISA corrected (E51 no F/D; U54 `rv64imafdc`), QSPI/MT25Q attributed to the board repl, QEMU driver-scope clarified, T20 in the tool list; generic `qemu-virt-rtems` platform (no MPFS board module) |
