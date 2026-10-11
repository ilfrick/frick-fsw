/*
 * frick-fsw - Platform configuration: Microchip RT PolarFire SoC RTPFS160ZT
 *             running RTEMS 6.2 (open-source RTEMS project)
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_PLATFORM_MPFS_H
#define FF_PLATFORM_MPFS_H

#define FF_PLATFORM_NAME "rtpfs160zt-rtems"
#define FF_HAS_POSIX 1 /* RTEMS 6 exposes a POSIX pthread/mqueue API */
#define FF_HAS_RTEMS 1

/* U54 application cores are RV64GC, little endian. */
#define FF_ENDIAN_LITTLE 1

/* U54 core frequency; 625 MHz is the RT PolarFire SoC maximum for RV64GC. */
#define FF_CPU_HZ 625000000u
#define FF_MEMORY_POOL_SIZE (256u * 1024u * 1024u)

/* Command/telemetry link: UART0 of the MSS (renamed after the mission board). */
#define FF_LINK_UART_DEVICE "/dev/console"
#define FF_LINK_BAUDRATE 115200u

/* Board specific hardware identifiers */
#define FF_BOARD_NAME "RT PolarFire SoC RTPFS160ZT (FCVG784)"
#define FF_MPFS_BOOT_HARTID 1u /* U54 #0 boots the flight software */

/* MSS GPIO port 0 (Microchip PolarFire SoC GPIO). Used by the reference GPIO
 * driver; exercised on the emulated SoC in the Renode functional test (M11). */
#define FF_GPIO0_BASE 0x20120000u
#define FF_GPIO0_PINS 14u /* MSS GPIO0 has 14 pins (GPIO1=24, GPIO2=32) */

/* GEM Ethernet MAC 0 (Cadence GEM). Used by the ETH HAL check on the emulated
 * SoC (M11); the reference driver is `src/drivers/eth/gem/`. */
#define FF_GEM0_BASE 0x20110000u
#define FF_GEM_MODID_OFF 0xFCu /* Module ID/revision (read-only identification) */

/* MSS MMUART0 (NS16550, wide registers). Used by the reference serial driver;
 * exercised on the emulated SoC in the Renode functional test (M11). UART1
 * (0x20100000) is the console. */
#define FF_MMUART0_BASE 0x20000000u

/* MSS eNVM (embedded non-volatile memory) DATA region (writable). Used by the
 * reference NVM driver; exercised on the emulated SoC in the Renode functional
 * test (M11). The config/control block is at 0x20200000. */
#define FF_ENVM0_BASE 0x20220000u

/* MSS CAN 0 (classic CAN 2.0 A/B). Used by the reference CAN driver; exercised
 * on the emulated SoC in the Renode functional test (M11). CAN-FD is a fabric
 * device reached through the FIC, not a feature of the MSS CAN controller. */
#define FF_CAN0_BASE 0x2010C000u

#endif /* FF_PLATFORM_MPFS_H */
