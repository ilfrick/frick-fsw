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

#endif /* FF_PLATFORM_MPFS_H */
