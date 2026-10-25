/*
 * frick-fsw - PAL hardware-watchdog harness unit test
 *
 * Requirement-based test of the portable watchdog contract (ADR-0021,
 * ECSS-E-ST-70-11 5.7.5.2): arm, periodic feed, expiry detection and the
 * single reset-request path. Nominal and out-of-bound cases carry explicit
 * expected results (norm N22). Cases 7 and 8 additionally assert the
 * SRS-REQ-073 derivation from the mission configuration table (timeout source
 * and the "feed period no greater than one quarter of the timeout" clause).
 *
 * The platform binding exercised here is the simulated host backend: it
 * records arm/feed/reset requests and performs no hardware reset, so the
 * mechanism is stimulated deterministically without terminating the test
 * process. That is a backend property, not evidence of MPFS watchdog
 * behaviour: the hardware binding is not wired (NC-PAL-01) and Renode has no
 * watchdog model (GAP-A5).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/tbl/tbl.h"
#include "apps/app.h"
#include "tables/fsw_tables.h"
#include <errno.h>
#include <time.h>

/* External link-time syscall wrapper for deterministic host-only EINTR/error
 * injection. It does not add a callable flight-build hook. The wrapper is
 * compiled only when the linker supports --wrap (see tests/CMakeLists.txt). */
#ifdef FF_TEST_HAVE_NANOSLEEP_WRAP
extern int __real_nanosleep(const struct timespec *req, struct timespec *rem);
static int g_sleep_fault_mode;
static unsigned g_sleep_fault_count;

int __wrap_nanosleep(const struct timespec *req, struct timespec *rem)
{
    if (g_sleep_fault_mode != 0) {
        g_sleep_fault_count++;
        if ((g_sleep_fault_mode == 1) && (g_sleep_fault_count <= 3u)) {
            errno = EINTR;
            if (rem != NULL) {
                *rem = *req;
            }
            return -1;
        }
        if (g_sleep_fault_mode == 2) {
            errno = EIO;
            return -1;
        }
    }
    return __real_nanosleep(req, rem);
}
#else
static int g_sleep_fault_mode;
static unsigned g_sleep_fault_count;
#endif

static void test_monotonic_absolute_wait(void)
{
    uint64_t deadline = FF_PalGetMonotonicNs() + 1000000ULL;
    FF_TEST_ASSERT(FF_PalTaskDelayUntilNs(deadline) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalGetMonotonicNs() >= deadline);
    /* An already-expired absolute deadline must return without adding a
     * relative delay or wrapping the monotonic time domain. */
    FF_TEST_ASSERT(FF_PalTaskDelayUntilNs(1u) == FF_SUCCESS);
}

static void test_absolute_wait_retries_eintr_and_reports_other_errors(void)
{
    uint64_t deadline;
#ifdef FF_TEST_HAVE_NANOSLEEP_WRAP
    g_sleep_fault_mode = 1;
    g_sleep_fault_count = 0u;
    deadline = FF_PalGetMonotonicNs() + 2000000ULL;
    FF_TEST_ASSERT(FF_PalTaskDelayUntilNs(deadline) == FF_SUCCESS);
    FF_TEST_ASSERT(g_sleep_fault_count >= 4u);

    g_sleep_fault_mode = 2;
    g_sleep_fault_count = 0u;
    deadline = FF_PalGetMonotonicNs() + 100000000ULL;
    FF_TEST_ASSERT(FF_PalTaskDelayUntilNs(deadline) == FF_ERROR);
    FF_TEST_ASSERT(g_sleep_fault_count == 1u);
    g_sleep_fault_mode = 0;
#else
    /* Without the syscall harness the fault paths cannot be provoked; the
     * absolute wait is still exercised on its nominal path. */
    deadline = FF_PalGetMonotonicNs() + 2000000ULL;
    FF_TEST_ASSERT(FF_PalTaskDelayUntilNs(deadline) == FF_SUCCESS);
#endif
}

/* 1. Out-of-bound: feeding/checking before the watchdog is armed. */
static void test_watchdog_before_init(void)
{
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_NOT_READY);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_NOT_READY);
    FF_TEST_ASSERT(FF_PalWatchdogGetTimeoutMs() == 0u);
}

/* 2. Out-of-bound: a zero timeout is rejected and does not arm. */
static void test_watchdog_init_rejects_zero(void)
{
    FF_TEST_ASSERT(FF_PalWatchdogInit(0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_NOT_READY);
}

/* Arm/feed backend failures must remain observable through the PAL contract;
 * the injection exists only in FF_TEST_HOOKS host test builds. */
static void test_watchdog_hardware_errors_propagate(void)
{
    FF_PalWatchdogTestSetHardwareResults(FF_NOT_IMPLEMENTED, FF_SUCCESS, FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_NOT_READY);

    FF_PalWatchdogTestSetHardwareResults(FF_SUCCESS, FF_ERROR, FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_ERROR);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS);
    /* A failed feed must not reset the software expiry reference. */
    FF_TEST_ASSERT(FF_PalWatchdogTestStarve(9000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_ERROR);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_TIMEOUT);
    FF_PalWatchdogTestSetHardwareResults(FF_SUCCESS, FF_SUCCESS, FF_ERROR);
    FF_TEST_ASSERT(FF_PalWatchdogReset() == FF_ERROR);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 1u);
    FF_PalWatchdogTestSetHardwareResults(FF_SUCCESS, FF_SUCCESS, FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS);
}

/* 3. Nominal: arm, feed, stay within the deadline. */
static void test_watchdog_nominal(void)
{
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogGetTimeoutMs() == 8000u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 1u);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 2u);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS);
}

/* 4. Out-of-bound: the feed stops past the timeout -> expiry is reported;
 *    resuming the feed restores the nominal state. */
static void test_watchdog_expiry_then_recovery(void)
{
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS);

    /* Simulated missed deadline (8500 ms > 8000 ms timeout). */
    FF_TEST_ASSERT(FF_PalWatchdogTestStarve(8500u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_TIMEOUT);

    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS);
}

/* 5. Boundary: a margin below the timeout is still accepted and a margin
 *    above it is rejected. The margin is deliberately much larger than the
 *    host clock jitter (the injected instant and the check are two separate
 *    reads of the monotonic clock, so the observed age is `elapsed + delta`
 *    with delta the scheduling jitter). A 1 ms margin was previously used and
 *    was not robust: a delta larger than 1 ms flips the "inside" case. The
 *    exact-equality point is not asserted: the monotonic clock advances
 *    between the injection and the check, so an "exactly equal" sample is not
 *    reproducible (norm N22: do not test physically meaningless values). */
static void test_watchdog_expiry_boundary(void)
{
    FF_TEST_ASSERT(FF_PalWatchdogInit(1000u) == FF_SUCCESS);

    /* 100 ms of margin on a 1000 ms timeout: far above host jitter, still on
     * the correct side of the boundary. */
    FF_TEST_ASSERT(FF_PalWatchdogTestStarve(900u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_SUCCESS); /* inside */

    FF_TEST_ASSERT(FF_PalWatchdogTestStarve(1100u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogCheck() == FF_TIMEOUT); /* outside */

    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
}

/* 6. Reset requests use the single path and are counted. The simulated
 *    backend performs no hardware reset and returns, which is exactly the
 *    behaviour the host build must have. */
static void test_watchdog_reset_path(void)
{
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 0u);

    FF_TEST_ASSERT(FF_PalWatchdogReset() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 1u);
    FF_TEST_ASSERT(FF_PalWatchdogReset() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 2u);
}

/* 7. SRS-REQ-073 nominal: the timeout comes from the mission configuration
 *    table field `watchdog_timeout_s`, and the feed period derived from it is
 *    no greater than one quarter of that timeout. This asserts the property
 *    the requirement states, over the table values the mission actually ships
 *    plus a spread of table values (including the smallest usable one). */
static void test_watchdog_plan_from_mission_table(void)
{
    const FF_MissionCfg_t *cfg;
    FF_MissionCfg_t probe;
    uint32_t timeout_ms = 0u;
    uint32_t feed_ms = 0u;

    /* Bring up the table service and the shipped mission table default. */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_MissionTablesInit() == FF_SUCCESS);
    cfg = FF_GetMissionCfg();

    /* The mission table default: 8 s -> 8000 ms, fed every 2000 ms. */
    FF_TEST_ASSERT(cfg != NULL);
    FF_TEST_ASSERT(cfg->watchdog_timeout_s == 8u);
    FF_TEST_ASSERT(FF_AppWdPlan(cfg, &timeout_ms, &feed_ms) == FF_SUCCESS);
    FF_TEST_ASSERT(timeout_ms == 8000u);
    FF_TEST_ASSERT(feed_ms == 2000u);
    FF_TEST_ASSERT(feed_ms <= (timeout_ms / 4u)); /* SRS-REQ-073 quarter period */

    /* Property over representative table values: 1 s .. 60 s. */
    for (uint32_t s = 1u; s <= 60u; s++) {
        probe = *cfg;
        probe.watchdog_timeout_s = (uint16_t)s;
        FF_TEST_ASSERT(FF_AppWdPlan(&probe, &timeout_ms, &feed_ms) == FF_SUCCESS);
        FF_TEST_ASSERT(timeout_ms == ((uint32_t)s * 1000u));
        FF_TEST_ASSERT(feed_ms <= (timeout_ms / 4u));
        FF_TEST_ASSERT(feed_ms > 0u); /* never a zero-length delay */
    }

    /* Largest table value (16 bit field): no 32 bit overflow. */
    probe = *cfg;
    probe.watchdog_timeout_s = 0xFFFFu;
    FF_TEST_ASSERT(FF_AppWdPlan(&probe, &timeout_ms, &feed_ms) == FF_SUCCESS);
    FF_TEST_ASSERT(timeout_ms == 65535000u);
    FF_TEST_ASSERT(feed_ms == (65535000u / 4u));
}

/* 8. SRS-REQ-073 out-of-bound: an unusable mission table is rejected rather
 *    than arming with an undefined timeout (a watchdog armed with a zero or
 *    garbage timeout is not protection). */
static void test_watchdog_plan_rejects_bad_table(void)
{
    const FF_MissionCfg_t *cfg;
    FF_MissionCfg_t probe;
    uint32_t timeout_ms = 12345u;
    uint32_t feed_ms = 6789u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_MissionTablesInit() == FF_SUCCESS);
    cfg = FF_GetMissionCfg();
    FF_TEST_ASSERT(cfg != NULL);

    /* NULL mission table. */
    FF_TEST_ASSERT(FF_AppWdPlan(NULL, &timeout_ms, &feed_ms) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(timeout_ms == 12345u); /* untouched */
    FF_TEST_ASSERT(feed_ms == 6789u);

    /* Zero timeout in the table. */
    probe = *cfg;
    probe.watchdog_timeout_s = 0u;
    FF_TEST_ASSERT(FF_AppWdPlan(&probe, &timeout_ms, &feed_ms) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(timeout_ms == 12345u);
    FF_TEST_ASSERT(feed_ms == 6789u);

    /* A NULL output pointer is rejected instead of returning success while
     * silently writing nothing (the caller would then use indeterminate
     * values). */
    FF_TEST_ASSERT(FF_AppWdPlan(cfg, NULL, &feed_ms) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_AppWdPlan(cfg, &timeout_ms, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(timeout_ms == 12345u);
    FF_TEST_ASSERT(feed_ms == 6789u);
}

FF_TEST_MAIN(test_monotonic_absolute_wait, test_watchdog_before_init,
             test_absolute_wait_retries_eintr_and_reports_other_errors,
             test_watchdog_init_rejects_zero, test_watchdog_hardware_errors_propagate,
             test_watchdog_nominal, test_watchdog_expiry_then_recovery,
             test_watchdog_expiry_boundary, test_watchdog_reset_path,
             test_watchdog_plan_from_mission_table, test_watchdog_plan_rejects_bad_table)
