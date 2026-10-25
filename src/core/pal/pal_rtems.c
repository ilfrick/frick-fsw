/*
 * frick-fsw - Platform Abstraction Layer: RTEMS 6 specific bootstrap
 *
 * On RTEMS the generic POSIX implementation (pal_posix.c) already provides
 * tasks, mutexes and message queues, and also the common FF_PalPanic path.
 * This translation unit contains the glue that is genuinely RTEMS specific:
 * the POSIX API configuration check, the POSIX_Init() application entry point
 * and the hardware-watchdog binding.
 *
 * The MPFS MSS watchdog binding is NOT WIRED yet: the hooks below report
 * FF_NOT_IMPLEMENTED so the missing protection stays visible and is tracked by
 * NC-PAL-01. Until it is wired, FF_PalPanic falls through to abort() on the
 * flight target (uncontrolled, no hardware-dependent reset).
 *
 * Compiled only for the RTEMS platforms (FRICK_PLATFORM_MPFS, the flight target,
 * or FRICK_PLATFORM_QEMU, the generic-BSP QEMU emulated-target build, M10).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
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

/* ---- Hardware watchdog binding (MPFS MSS watchdog) ----------------
 * NOT WIRED (NC-PAL-01). The RTPFS160ZT MSS watchdog arm/feed/reset sequence
 * must be implemented against the verified Microchip BSP/datasheet reference
 * and validated on target (Renode has no watchdog model, GAP-A5). Until then
 * these hooks report FF_NOT_IMPLEMENTED: the software harness still tracks the
 * feed deadline and the pre-reset event report still runs, but no hardware
 * reset is guaranteed. */
FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms)
{
    FF_PalLog("watchdog: NOT WIRED on RTEMS (timeout %u ms) - NC-PAL-01\n", (unsigned)timeout_ms);
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t ff_pal_wd_hw_feed(void)
{
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t ff_pal_wd_hw_reset(void)
{
    FF_PalLog("watchdog: reset requested but MPFS binding NOT WIRED - NC-PAL-01\n");
    return FF_NOT_IMPLEMENTED;
}

#endif /* FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU */
