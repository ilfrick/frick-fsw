# ADR-0002 - RTEMS 6.2 embedded brains baseline on RTPFS160ZT

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
The flight target is the Microchip Radiation-Tolerant PolarFire SoC FPGA
RTPFS160ZT (RISC-V RV64GC U54 cluster, FCVG784). The RTOS choice must provide
determinism, a proven space track record and ECSS-relevant qualification
evidence.

## Decision
Use RTEMS **6.2** from the **embedded brains** ECSS pre-qualified baseline
(release `esa/v6.2`), BSP `riscv/mpfs64imafdc`, toolchain installed with
`rtems-source-builder` (`tools/scripts/setup_rtems.sh`).

## Consequences
- Cross toolchain prefix `riscv-rtems6-` (see
  `cmake/toolchains/rtems-riscv-mpfs.cmake`).
- The same framework sources build on the host POSIX simulator and for this
  target via the PAL (SSS-REQ-011).
- On-target qualification (RTEMS BSP tests, HWIL) is GAP-A5 until flight
  hardware is available.

## Traceability
SSS-REQ-001, SRS-REQ-016.