/*
 * frick-fsw - Watchdog feeder application (FDIR reset recovery)
 *
 * Arms the hardware watchdog once, after framework initialisation has
 * completed, and feeds it periodically at a quarter of the configured timeout.
 * If this task stops being scheduled (hung scheduler, dead task, stuck
 * interrupt path) the feed stops and the hardware watchdog resets the
 * processor. That is the single recovery path also used by an explicit
 * software reset request (FF_PalPanic -> FF_PalWatchdogReset).
 *
 * The platform binding is abstracted (see pal.h): the host simulator uses a
 * simulated backend so the mechanism is testable and no reset occurs. On the
 * MPFS target the WDOG0 binding is implemented only for the emulated N18 build
 * (FF_ENABLE_RENODE_TEST); the flight-target parameters are not yet authorised
 * (NC-PAL-01), so flight builds still fail the arm explicitly.
 *
 * Timeout source: the mission configuration table (config/tables), field
 * watchdog_timeout_s. The default is 10 s, a provisional representative GEO
 * value (NC-EXT-238); the mission-authority value is still an open owner
 * input. The feed interval is derived (timeout/4) so the margin is a property
 * of the requirement, not an independent constant.
 *
 * The feed cadence uses ABSOLUTE monotonic deadlines (FF_PalTaskDelayUntilNs),
 * not cumulative relative delays: relative delays accumulate wake-up jitter
 * and would slowly stretch the effective period (NC-EXT-187). A 100 ms
 * execution/jitter budget is assumed inside the T/4 interval; it is not a
 * separate timer and does not change the derived feed period.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "tables/fsw_tables.h"

#define FF_WD_FEED_DIVISOR 4u

/* Derive the watchdog timeout and the feed period from the mission table.
 *
 * Pure, bounded and side-effect free. This is the *same* arithmetic that the
 * flight main loop performs, extracted so that the SRS-REQ-073 property "the
 * feed period is no greater than one quarter of the timeout" and the clause
 * "the timeout is taken from the mission configuration table" are objectively
 * verifiable in a host test. It follows the pattern already used for the
 * health & safety seams (FF_AppHsStep / FF_AppHsRegister / FF_AppHsReset).
 *
 * No flight behaviour is changed by this extraction: FF_AppWdMain performs
 * exactly the same computation, in the same order, with the same values and
 * the same rejection condition.
 *
 * Returns FF_INVALID_ARGUMENT when the mission table does not carry a usable
 * watchdog timeout (missing table or zero timeout) or when either output
 * pointer is NULL.
 */
FF_Status_t FF_AppWdPlan(const FF_MissionCfg_t *cfg, uint32_t *timeout_ms, uint32_t *feed_ms)
{
    uint32_t timeout;
    uint32_t feed;

    if ((cfg == NULL) || (cfg->watchdog_timeout_s == 0u) || (timeout_ms == NULL) ||
        (feed_ms == NULL)) {
        return FF_INVALID_ARGUMENT;
    }

    /* watchdog_timeout_s is a 16 bit field in seconds: the product fits in
     * 32 bits (max 65535 s -> 65535000 ms). */
    timeout = (uint32_t)cfg->watchdog_timeout_s * 1000u;
    feed = timeout / FF_WD_FEED_DIVISOR;
    if (feed == 0u) {
        feed = 1u;
    }

    *timeout_ms = timeout;
    *feed_ms = feed;
    return FF_SUCCESS;
}

void FF_AppWdMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    const FF_MissionCfg_t *cfg = FF_GetMissionCfg();
    /* Initialised so that no indeterminate value can reach the arm call even
     * if FF_PalPanic ever returned; it currently never returns (abort after
     * the reset request), and a zero timeout is itself rejected by the PAL. */
    uint32_t timeout_ms = 0u;
    uint32_t feed_ms = 0u;
    uint64_t feed_ns;
    uint64_t next_ns;

    if (FF_AppWdPlan(cfg, &timeout_ms, &feed_ms) != FF_SUCCESS) {
        FF_PalPanic("watchdog: invalid mission timeout");
    }

    if (FF_PalWatchdogInit(timeout_ms) != FF_SUCCESS) {
        FF_PalPanic("watchdog: init failed");
    }

    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "WD armed: timeout %u ms, feed %u ms",
                     (unsigned)timeout_ms, (unsigned)feed_ms);

    /* Absolute cadence: the n-th feed targets start + n*feed_ns, so a late
     * wake-up does not shift every later feed (no cumulative drift). */
    feed_ns = (uint64_t)feed_ms * 1000000ULL;
    next_ns = FF_PalGetMonotonicNs() + feed_ns;

    for (;;) {
        if (FF_PalWatchdogFeed() != FF_SUCCESS) {
            FF_PalPanic("watchdog: feed failed");
        }

        if (FF_PalTaskDelayUntilNs(next_ns) != FF_SUCCESS) {
            FF_PalPanic("watchdog: feed scheduling failed");
        }

        /* Advance by exactly one period. If the deadline has already passed
         * by more than one period (overrun), re-anchor instead of issuing a
         * burst of catch-up feeds: a burst would hide the overrun and add no
         * protection. The re-anchored gap stays below the timeout. */
        next_ns += feed_ns;
        {
            uint64_t now = FF_PalGetMonotonicNs();
            if (now >= next_ns) {
                next_ns = now + feed_ns;
            }
        }
    }
}
