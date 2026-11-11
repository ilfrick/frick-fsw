/*
 * frick-fsw - Platform Abstraction Layer: RTEMS 6 specific bootstrap
 *
 * On RTEMS the generic POSIX implementation (pal_posix.c) already provides
 * tasks, mutexes and message queues, and also the common FF_PalPanic path.
 * This translation unit contains the glue that is genuinely RTEMS specific:
 * the POSIX API configuration check, the POSIX_Init() application entry point
 * and the hardware-watchdog binding.
 *
 * MPFS MSS watchdog binding (WDOG0 / E51)
 * ---------------------------------------
 * WDOG0 is the only one of the five MSS watchdogs that can reset the MSS; the
 * other four only interrupt the E51 (MSS TRM DS60001702S 3.12.12, recorded as
 * FRICK-ARCH-0024 F-18). The register layout is the verified WATCHDOG_TypeDef
 * map (FRICK-ARCH-0024 F-1/F-3):
 *
 *   0x00 REFRESH  (key 0xDEADC0DE reloads the counter)
 *   0x04 CONTROL  (bit 0 = enable)
 *   0x08 STATUS   (bit 4 = LOCKED in the pinned model)
 *   0x0C TIME     (load value, 24-bit in the model; driver cap 16 777 200)
 *   0x10 MSVP     (maximum value at which a refresh is permitted)
 *   0x14 TRIGGER  (timeout-interrupt threshold, 12-bit; max 4095)
 *   0x18 FORCE    (key 0xDEAD forces a reset after a timeout)
 *
 * The configuration order is TRIGGER -> MSVP -> TIME -> CONTROL, followed by a
 * read-back of the three value registers BEFORE the first keyed REFRESH. The
 * read-back is mandatory because TIME/MSVP/TRIGGER become write-once after the
 * configuration: a wrong configuration is not recoverable in software
 * (FRICK-ARCH-0024 F-12).
 *
 * Authorization boundary
 * ----------------------
 * The flight-target watchdog clock, gating and mission parameters are not
 * established (NC-PAL-01, FRICK-ARCH-0024 M-1/M-2/M-8). This build therefore
 * enables the binding ONLY for the Renode functional-validation image
 * (FF_ENABLE_RENODE_TEST), using the pinned model's explicitly emulated clock
 * (FF_WDOG_EMU_CLK_HZ) and the mission table timeout. Flight builds keep the
 * three hooks explicitly NOT IMPLEMENTED until an authorised hardware
 * configuration and target/HIL evidence exist. The emulated configuration is
 * not a flight parameter and establishes no target behaviour.
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

#if defined(FRICK_PLATFORM_MPFS) && defined(FF_ENABLE_RENODE_TEST) && (FF_ENABLE_RENODE_TEST == 1)

/* ---- MPFS MSS WDOG0 hardware binding (emulated N18 configuration) -------
 * The register map and bounds are the verified values documented in the file
 * header. FF_WDOG0_BASE, the offsets, the keys and the field bounds come from
 * config/platform/platform_mpfs.h. The clock is the pinned Renode model's
 * explicitly emulated value, not a flight parameter (NC-PAL-01).
 */

#define FF_WDOG_REG(offset) (*(volatile uint32_t *)(uintptr_t)(FF_WDOG0_BASE + (offset)))

static bool g_wdog_initialized;
static uint32_t g_wdog_timeout_ms;

static uint32_t ff_wdog_read(uint32_t offset)
{
    return FF_WDOG_REG(offset);
}

static void ff_wdog_write(uint32_t offset, uint32_t value)
{
    FF_WDOG_REG(offset) = value;
}

/* Convert a millisecond timeout into emulated model ticks. Returns 0 when the
 * value is not representable in the cited driver's TIME field. */
static uint32_t ff_wdog_ticks_from_ms(uint32_t timeout_ms)
{
    uint64_t ticks;

    if (timeout_ms == 0u) {
        return 0u;
    }
    ticks = ((uint64_t)timeout_ms * (uint64_t)FF_WDOG_EMU_CLK_HZ) / 1000u;
    if (ticks == 0u) {
        ticks = 1u;
    }
    if (ticks > (uint64_t)FF_WDOG_TIME_MAX) {
        return 0u;
    }
    return (uint32_t)ticks;
}

FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms)
{
    uint32_t time_ticks;
    uint32_t trigger_ticks;
    uint32_t msvp_ticks;
    uint32_t control;

    if (timeout_ms == 0u) {
        return FF_INVALID_ARGUMENT;
    }

    if (g_wdog_initialized) {
        /* The configuration registers are write-once: re-arming with a
         * different timeout cannot be honoured. Keep the already-armed value
         * and refresh it instead of pretending to reconfigure. */
        if (timeout_ms != g_wdog_timeout_ms) {
            FF_PalLog("watchdog: WDOG0 already armed with %u ms, cannot re-arm %u ms\n",
                      (unsigned)g_wdog_timeout_ms, (unsigned)timeout_ms);
            return FF_ERROR;
        }
        return ff_pal_wd_hw_feed();
    }

    time_ticks = ff_wdog_ticks_from_ms(timeout_ms);
    if (time_ticks == 0u) {
        FF_PalLog("watchdog: timeout %u ms is not representable (TIME max %lu)\n",
                  (unsigned)timeout_ms, (unsigned long)FF_WDOG_TIME_MAX);
        return FF_INVALID_ARGUMENT;
    }

    trigger_ticks = FF_WDOG_EMU_TRIGGER_TICKS;
    if ((trigger_ticks > FF_WDOG_TRIGGER_MAX) || (trigger_ticks >= time_ticks)) {
        FF_PalLog("watchdog: invalid TRIGGER %lu for TIME %lu\n", (unsigned long)trigger_ticks,
                  (unsigned long)time_ticks);
        return FF_INVALID_ARGUMENT;
    }

    /* MSVP equal to TIME keeps the forbidden-refresh window disabled: a
     * refresh is permitted while the counter is <= MSVP, which is always true.
     * This is the emulated profile's explicit choice; the flight value is an
     * open owner input (FRICK-ARCH-0024 M-6). */
    msvp_ticks = time_ticks;

    /* Documented configuration order. */
    ff_wdog_write(FF_WDOG_TRIGGER_OFF, trigger_ticks);
    ff_wdog_write(FF_WDOG_MSVP_OFF, msvp_ticks);
    ff_wdog_write(FF_WDOG_TIME_OFF, time_ticks);
    ff_wdog_write(FF_WDOG_CONTROL_OFF, FF_WDOG_CONTROL_ENABLE);

    /* Read back BEFORE the first REFRESH (F-12): a mismatch is fatal because
     * the configuration is locked after this point. */
    control = ff_wdog_read(FF_WDOG_CONTROL_OFF);
    if ((ff_wdog_read(FF_WDOG_TIME_OFF) != time_ticks) ||
        (ff_wdog_read(FF_WDOG_MSVP_OFF) != msvp_ticks) ||
        (ff_wdog_read(FF_WDOG_TRIGGER_OFF) != trigger_ticks) ||
        ((control & FF_WDOG_CONTROL_ENABLE) == 0u)) {
        FF_PalLog("watchdog: WDOG0 read-back mismatch (TIME=%lu MSVP=%lu TRIGGER=%lu "
                  "CONTROL=0x%08lX)\n",
                  (unsigned long)ff_wdog_read(FF_WDOG_TIME_OFF),
                  (unsigned long)ff_wdog_read(FF_WDOG_MSVP_OFF),
                  (unsigned long)ff_wdog_read(FF_WDOG_TRIGGER_OFF), (unsigned long)control);
        return FF_ERROR;
    }

    /* First keyed reload. A zero read-back means the counter did not start. */
    ff_wdog_write(FF_WDOG_REFRESH_OFF, FF_WDOG_REFRESH_KEY);
    if (ff_wdog_read(FF_WDOG_REFRESH_OFF) == 0u) {
        FF_PalLog("watchdog: WDOG0 did not reload after the initial refresh\n");
        return FF_ERROR;
    }

    g_wdog_timeout_ms = timeout_ms;
    g_wdog_initialized = true;

    FF_PalLog("watchdog: WDOG0 armed (emulated profile: timeout %u ms, TIME=%lu, "
              "TRIGGER=%lu, MSVP=TIME, clk=%lu Hz)\n",
              (unsigned)timeout_ms, (unsigned long)time_ticks, (unsigned long)trigger_ticks,
              (unsigned long)FF_WDOG_EMU_CLK_HZ);
    return FF_SUCCESS;
}

FF_Status_t ff_pal_wd_hw_feed(void)
{
    uint32_t before;
    uint32_t after;

    if (!g_wdog_initialized) {
        return FF_NOT_READY;
    }

    before = ff_wdog_read(FF_WDOG_REFRESH_OFF);
    ff_wdog_write(FF_WDOG_REFRESH_OFF, FF_WDOG_REFRESH_KEY);
    after = ff_wdog_read(FF_WDOG_REFRESH_OFF);

    /* The counter is a down-counter reloaded by the keyed write. A reload is
     * observable as a value greater than the pre-write value; a counter that
     * is not running reads back unchanged (equal is accepted). A strictly
     * lower value means the write did not reload it. */
    if ((after == 0u) || (after < before)) {
        FF_PalLog("watchdog: WDOG0 refresh not observed (before=%lu after=%lu)\n",
                  (unsigned long)before, (unsigned long)after);
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

FF_Status_t ff_pal_wd_hw_reset(void)
{
    /* Single recovery path: stop refreshing and let WDOG0 reach zero. The
     * cited semantics defer a software reset request to counter zero; the
     * function therefore does not return. */
    if (!g_wdog_initialized) {
        /* Best effort before the arm: program the shortest representable
         * timeout and start it, so that a panic during initialisation still
         * ends in a hardware reset. */
        ff_wdog_write(FF_WDOG_TRIGGER_OFF, 0u);
        ff_wdog_write(FF_WDOG_MSVP_OFF, 1u);
        ff_wdog_write(FF_WDOG_TIME_OFF, 1u);
        ff_wdog_write(FF_WDOG_CONTROL_OFF, FF_WDOG_CONTROL_ENABLE);
        ff_wdog_write(FF_WDOG_REFRESH_OFF, FF_WDOG_REFRESH_KEY);
        g_wdog_initialized = true;
    }

    FF_PalLog("watchdog: WDOG0 reset requested (MSS scope); waiting for counter zero\n");
    for (;;) {
        /* Do not refresh. The independent down-counter performs the reset. */
    }

    return FF_SUCCESS; /* Unreachable. */
}

#else /* not the emulated N18 build: the flight configuration is not authorised */

/*
 * Flight-target hooks. The owner has rejected operating without a watchdog
 * (ESC-10), so this is a fatal initialisation failure, not a degraded mode.
 * It remains NOT IMPLEMENTED until the target clock, gating, mission timeout
 * and HIL evidence are authorised (NC-PAL-01).
 */
FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms)
{
    (void)timeout_ms;
    FF_PalLog("watchdog: MPFS WDOG0 binding not configured for this build "
              "(NC-PAL-01: flight parameters require owner authorisation)\n");
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t ff_pal_wd_hw_feed(void)
{
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t ff_pal_wd_hw_reset(void)
{
    FF_PalLog("watchdog: reset requested but no authorised MPFS binding is wired "
              "(NC-PAL-01)\n");
    return FF_NOT_IMPLEMENTED;
}

#endif /* FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST */

#endif /* FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU */
