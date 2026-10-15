/*
 * frick-fsw - Platform configuration: generic RISC-V RTEMS on QEMU `virt`
 *
 * Emulated-target verification only (milestone M10, SRS-REQ-043). This platform
 * builds the framework against the generic RTEMS `riscv/rv64imafdc` BSP and runs
 * it on `qemu-system-riscv64 -M virt`; it deliberately does NOT include the
 * Microchip PolarFire SoC board module or any MPFS MMIO base, so the image is
 * genuinely generic (RTEMS/PAL/tasking/framework bring-up only). The flight
 * target remains the PolarFire SoC (config/platform/platform_mpfs.h).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_PLATFORM_QEMU_H
#define FF_PLATFORM_QEMU_H

#define FF_PLATFORM_NAME "qemu-virt-rtems"
#define FF_HAS_POSIX 1 /* RTEMS 6 exposes a POSIX pthread/mqueue API */
#define FF_HAS_RTEMS 1

/* Generic RISC-V (rv64imafdc, little endian). */
#define FF_ENDIAN_LITTLE 1

/* Nominal QEMU `virt` core clock (no hardware clock; timing is not verified). */
#define FF_CPU_HZ 100000000u
#define FF_MEMORY_POOL_SIZE (16u * 1024u * 1024u)

/* Console: the generic RTEMS BSP routes printk/console to the QEMU `virt`
 * NS16550 UART0; the host sees it via `-nographic` stdio. */
#define FF_LINK_UART_DEVICE "/dev/console"
#define FF_LINK_BAUDRATE 115200u

#define FF_BOARD_NAME "QEMU virt (generic rv64imafdc RTEMS)"

#endif /* FF_PLATFORM_QEMU_H */
