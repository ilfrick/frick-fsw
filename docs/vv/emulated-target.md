# Emulated-Target Verification (Renode / QEMU)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-EMU-001                       |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
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
| **Renode** | full PolarFire SoC emulation | official Microchip/Antmicro platforms `polarfire-soc` and `icicle-kit`: 5 RiscV64 CPUs (E51 `rv64imac` + 4x U54 `rv64gc`), CLINT, PLIC, PDMA, 5x NS16550 UART, SD/eMMC, 2x SPI, 2x I2C, 2x CAN, 2x CadenceGEM (+ PHY), 3x GPIO, watchdog/RTC/timer, QSPI controller, eNVM, USB, 2x PCIe, crypto, DDR mock; runs the same unmodified binaries; GDB + headless test scripts; UART analyzer for console capture; FPGA fabric co-simulation (Verilator). Runs the **MPFS image** (`make rtems`). |
| **QEMU** | generic RISC-V run | `qemu-system-riscv64 -M virt -nographic -bios <rtems.exe>` runs the RTEMS **generic `riscv/rv64imafdc` BSP** (a separate build from the MPFS image); no MPFS peripherals. |

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
- CI: a job builds the RTEMS image and, when Renode/QEMU is available, runs it
  and checks the boot banner + app registration; evidence recorded in
  `docs/vv/verification-results.md`.
- Tool qualification (N10): Renode and QEMU registered (T15/T16).

## 6. Gaps linkage

GAP-A5 (on-target) is **partially mitigated** by emulated execution; the residual
(real hardware, timing, environment) remains open and is documented in
`docs/quality/class-a-gaps.md`.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial emulated-target V&V note (Renode PolarFire SoC + QEMU); milestone M10 |
