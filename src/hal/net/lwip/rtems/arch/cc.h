/*
 * frick-fsw - lwIP compiler/architecture abstraction for the RTEMS target
 *
 * Minimal lwIP `arch/cc.h` for the RTEMS 6.2 / RISC-V (rv64) flight target.
 * It provides the compiler/platform macros that lwIP requires beyond the
 * portable defaults: byte order, diagnostics, assertions and the random
 * source used for TCP initial sequence numbers. Part of the in-repo RTEMS
 * `sys_arch` port (SRS-REQ-040, NC-S6-01); it is used only when FF_USE_LWIP=ON
 * and the platform is RTEMS - the host build keeps using the upstream lwIP
 * unix port.
 *
 * See docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_LWIP_RTEMS_ARCH_CC_H
#define FF_LWIP_RTEMS_ARCH_CC_H

#include <stdio.h>
#include <sys/time.h>

#include <rtems.h>

/* The RISC-V target is little endian. */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* newlib provides <sys/time.h> (and thus struct timeval) on RTEMS. */
#define LWIP_TIMEVAL_PRIVATE 0

/* Diagnostics: lwIP debug output goes to the console (stdout). Compiled out
 * unless LWIP_DEBUG is enabled. */
#define LWIP_PLATFORM_DIAG(x)                                                                      \
    do {                                                                                           \
        printf x;                                                                                  \
    } while (0)

/* Assertions are fatal: an lwIP invariant violation is a bounded, non-recoverable
 * condition and must stop the image through the RTEMS fatal error handler. */
#define LWIP_PLATFORM_ASSERT(x)                                                                    \
    do {                                                                                           \
        printf("lwIP assertion \"%s\" failed at %s:%d\n", (x), __FILE__, (int)__LINE__);           \
        rtems_fatal_error_occurred(0xDEADBEEFu);                                                   \
    } while (0)

/* newlib provides <errno.h> on RTEMS. Use the literal-include form: on RTEMS
 * the token `errno` is a function-like macro, so a macro include directive
 * would be expanded. */
#define LWIP_ERRNO_STDINCLUDE 1

/* Random source (TCP ISN / ephemeral ports). The implementation lives in the
 * RTEMS sys_arch port and keeps no libc rand() state. */
unsigned int lwip_rtems_rand(void);
#define LWIP_RAND() lwip_rtems_rand()

#endif /* FF_LWIP_RTEMS_ARCH_CC_H */
