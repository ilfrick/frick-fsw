/*
 * frick-fsw - Portable hardware-watchdog (FDIR reset recovery) harness
 *
 * This translation unit implements the platform-independent part of the
 * watchdog contract declared in pal.h: the arm/feed/expiry state machine and
 * its observability counters. The hardware binding is provided by the
 * platform translation unit through the three ff_pal_wd_hw_* hooks:
 *
 *   - pal_posix.c : simulated backend on the host simulator (records the
 *                   arm/feed/reset requests, performs no hardware reset), so
 *                   the mechanism can be stimulated deterministically in tests;
 *   - pal_rtems.c : NOT WIRED. The MPFS MSS watchdog binding is deferred and
 *                   tracked by NC-PAL-01; the hooks report FF_NOT_IMPLEMENTED.
 *
 * Rationale (ECSS-E-ST-70-11 5.7.5.2): the recovery action must be
 * hardware-generated so that it is independent of the on-board software it
 * resets, an event report is generated before the reset is enforced, and a
 * commanded software reset uses the same single recovery path as a timeout.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "pal.h"

#include <stdbool.h>

/* Platform binding, provided by exactly one platform translation unit. */
FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms);
FF_Status_t ff_pal_wd_hw_feed(void);
FF_Status_t ff_pal_wd_hw_reset(void);

static bool g_armed;
static uint32_t g_timeout_ms;
static uint64_t g_last_feed_ns;
static uint32_t g_feeds;
static uint32_t g_resets;

FF_Status_t FF_PalWatchdogInit(uint32_t timeout_ms)
{
    FF_Status_t st;
    if (timeout_ms == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    st = ff_pal_wd_hw_arm(timeout_ms);
    if (st != FF_SUCCESS) {
        g_armed = false;
        g_timeout_ms = 0u;
        return st;
    }
    g_timeout_ms = timeout_ms;
    g_last_feed_ns = FF_PalGetMonotonicNs();
    g_feeds = 0u;
    g_resets = 0u;
    g_armed = true;
    return FF_SUCCESS;
}

FF_Status_t FF_PalWatchdogFeed(void)
{
    FF_Status_t st;
    if (!g_armed) {
        return FF_NOT_READY;
    }
    st = ff_pal_wd_hw_feed();
    if (st != FF_SUCCESS) {
        return st;
    }
    g_last_feed_ns = FF_PalGetMonotonicNs();
    g_feeds++;
    return FF_SUCCESS;
}

FF_Status_t FF_PalWatchdogCheck(void)
{
    uint64_t now;
    uint64_t limit_ns;

    if (!g_armed) {
        return FF_NOT_READY;
    }
    now = FF_PalGetMonotonicNs();
    limit_ns = (uint64_t)g_timeout_ms * 1000000ULL;
    if ((now - g_last_feed_ns) > limit_ns) {
        return FF_TIMEOUT;
    }
    return FF_SUCCESS;
}

FF_Status_t FF_PalWatchdogReset(void)
{
    g_resets++;
    return ff_pal_wd_hw_reset();
}

uint32_t FF_PalWatchdogGetFeeds(void)
{
    return g_feeds;
}

uint32_t FF_PalWatchdogGetResets(void)
{
    return g_resets;
}

uint32_t FF_PalWatchdogGetTimeoutMs(void)
{
    return g_timeout_ms;
}

#ifdef FF_TEST_HOOKS
FF_Status_t FF_PalWatchdogTestStarve(uint32_t elapsed_ms)
{
    if (!g_armed) {
        return FF_NOT_READY;
    }
    g_last_feed_ns = FF_PalGetMonotonicNs() - ((uint64_t)elapsed_ms * 1000000ULL);
    return FF_SUCCESS;
}
#endif
