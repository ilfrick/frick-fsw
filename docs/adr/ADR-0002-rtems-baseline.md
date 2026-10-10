# ADR-0002 - RTEMS 6.2 open-source baseline on RTPFS160ZT

| Status | Accepted (revised 2026-10-10: open-source RTEMS baseline) |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
The flight target is the Microchip Radiation-Tolerant PolarFire SoC FPGA
RTPFS160ZT (RISC-V RV64GC U54 cluster, FCVG784). The RTOS choice must provide
determinism, a proven space track record and ECSS-relevant qualification
evidence.

## Decision
Use the **open-source RTEMS 6.2** release from the RTEMS project
(`gitlab.rtems.org`, release tag `6.2`), BSP `riscv/mpfs64imafdc`, toolchain
installed with `rtems-source-builder` from the RTEMS project
(`tools/scripts/setup_rtems.sh`). No vendor-specific or pre-built commercial
baseline/toolchain is used: the sources and the source builder are the upstream
open-source RTEMS ones.

## Consequences
- Cross toolchain prefix `riscv-rtems6-` (see
  `cmake/toolchains/rtems-riscv-mpfs.cmake`).
- The same framework sources build on the host POSIX simulator and for this
  target via the PAL (SSS-REQ-011).
- On-target qualification (RTEMS BSP tests, HWIL) is GAP-A5 until flight
  hardware is available.

## Traceability
SSS-REQ-001, SRS-REQ-016.