/*
 * frick-fsw - Platform Abstraction Layer: RTEMS 6 specific bootstrap
 *
 * On RTEMS the generic POSIX implementation (pal_posix.c) already provides
 * tasks, mutexes and message queues. This translation unit only contains the
 * glue that is genuinely RTEMS specific: the POSIX API configuration check,
 * the RTEMS clock and the panic/reboot path.
 *
 * Compiled only for the RTEMS platforms (FRICK_PLATFORM_MPFS, the flight target,
 * or FRICK_PLATFORM_QEMU, the generic-BSP QEMU emulated-target build, M10).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "pal.h"

#if defined(FRICK_PLATFORM_MPFS) || defined(FRICK_PLATFORM_QEMU)

#include <rtems.h>

/*
 * RTEMS POSIX API prerequisites. The flight configuration file
 * (see config/rtems/rtems_config.c) provides:
 *
 *   CONFIGURE_MAXIMUM_POSIX_THREADS        32
 *   CONFIGURE_MAXIMUM_POSIX_TIMERS         8
 *   CONFIGURE_MAXIMUM_POSIX_KEYS           8
 *   CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES 8
 *   CONFIGURE_POSIX_INIT_THREAD_STACK_SIZE (256 * 1024)
 *
 * In RTEMS 6 POSIX mutexes and condition variables are self-contained (no
 * CONFIGURE_* option); the *_MUTEXES / *_CONDITION_VARIABLES options are
 * obsolete since RTEMS 5.1.
 */
FF_Status_t FF_PalPlatformInit(void)
{
    /* Confirm the POSIX API is actually configured at build time
     * (RTEMS_POSIX_API is defined by the RTEMS headers/configuration). */
#if !defined(RTEMS_POSIX_API)
#error "RTEMS POSIX API must be enabled for frick-fsw"
#endif
    return FF_SUCCESS;
}

/*
 * Called from the RTEMS POSIX initialization thread (see src/main/fsw_main.c).
 * It hands control to the generic PAL initialization and main entry point.
 */
extern int ff_fsw_main(void);

void *POSIX_Init(void *argument)
{
    (void)argument;
    (void)ff_fsw_main();
    rtems_fatal(RTEMS_FATAL_SOURCE_EXIT, 0);
    return NULL;
}

#endif /* FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU */
