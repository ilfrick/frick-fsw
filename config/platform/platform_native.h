/*
 * frick-fsw - Platform configuration: native POSIX host (development / simulator)
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_PLATFORM_NATIVE_H
#define FF_PLATFORM_NATIVE_H

#define FF_PLATFORM_NAME "native-posix"
#define FF_HAS_POSIX 1
#define FF_HAS_RTEMS 0

/* Native builds run on the Linux host, a 64-bit little endian machine. */
#define FF_ENDIAN_LITTLE 1

#define FF_CPU_HZ 24000000u /* nominal, unused on host */
#define FF_MEMORY_POOL_SIZE (16u * 1024u * 1024u)

#define FF_LINK_UART_DEVICE "/dev/ttyUSB0"
#define FF_LINK_BAUDRATE 115200u

#define FF_BOARD_NAME "host simulator"
#define FF_MPFS_BOOT_HARTID 1u

#endif /* FF_PLATFORM_NATIVE_H */
