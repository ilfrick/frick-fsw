/*
 * frick-fsw - XW-1 middleware contract tests against the external-watchdog model
 *
 * The SUT is the PRODUCTION watchdog path: `FF_PalWatchdogInit`, `.Feed`,
 * `.Check` and `.Reset` from src/core/pal/pal_watchdog.c are linked unchanged
 * from frick_fsw_core. Only the three device hooks (`ff_pal_wd_hw_arm/feed/
 * reset`) are replaced at link time by tests/stubs/ff_test_extwd_pal.c, which
 * drives tests/stubs/ff_test_extwd_model.c. The production backends
 * (pal_posix.c / pal_rtems.c) are NOT modified and not even reached.
 *
 * Time is owned by the harness (`ff_extwd_advance_ms`), never by the SUT, so an
 * expiry cannot be produced by the software under test.
 *
 * Requirement mapping (see docs/vv/external-watchdog-test-contract.md §5):
 *   SRS-REQ-073  arming, feeding, independent expiry
 *   SRS-REQ-075  single reset path, at most one reset
 *   SRS-REQ-074  not asserted here - covered by the existing report-before-reset
 *                evidence (N22-TS5-50, test_es_hs_lifecycle.c); recorded as a
 *                cross-reference, not as new evidence in this file.
 *
 * HOST CONTRACT TEST ONLY: not hardware, not HIL, not emulated-target, not N18.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "ff_test.h"
#include "core/pal/pal.h"
#include "ff_test_extwd_model.h"

/* M1 - arming through the production API arms the device (SRS-REQ-073). */
static void test_mw_arm_reaches_the_device(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(80u) == FF_SUCCESS);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_ARMED);
    /* The SUT's timeout became the device window: the requirement's
     * "timeout from the configuration table" path is exercised. */
    FF_TEST_ASSERT(ff_extwd_window_ms() == 80u);
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 80u);
}

/* M2 - a valid feed through the production API reloads the device deadline. */
static void test_mw_feed_reloads_the_device_deadline(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(80u) == FF_SUCCESS);

    ff_extwd_advance_ms(50u);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    /* NEGATIVE CONTROL: the deadline moved from 80 to 130 - a SUT that only
     * counted feeds without reaching the device would leave it at 80. */
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 130u);
    FF_TEST_ASSERT(ff_extwd_feeds() == 1u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 1u);
}

/* M3 - the SUT stops feeding: the device expires independently of the software.
 * This is the property SRS-REQ-073 demands and the production API cannot fake. */
static void test_mw_stopped_feeds_expire_the_device(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(100u) == FF_SUCCESS);

    /* The harness advances the device clock; the SUT is not consulted at all. */
    ff_extwd_advance_ms(60u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_ARMED);
    ff_extwd_advance_ms(40u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
    /* NEGATIVE CONTROL: the SUT's own counters did not manufacture the event -
     * the SUT performed no feed and requested no reset. */
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 0u);
}

/* M4 - an arm failure surfaces through the production API and leaves no silent
 * armed state (SRS-REQ-073, and the fatal policy of app_wd.c). */
static void test_mw_arm_failure_is_reported(void)
{
    ff_extwd_reset();
    ff_extwd_set_fault(FF_EXTWD_FAULT_ARM_FAIL);
    FF_TEST_ASSERT(FF_PalWatchdogInit(100u) != FF_SUCCESS);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_DISARMED);
    /* NEGATIVE CONTROL: the SUT did not record itself as armed - a subsequent
     * feed is refused with FF_NOT_READY rather than silently accepted. */
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_NOT_READY);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
}

/* M5 - a feed failure surfaces through the production API and does not reload,
 * so the device expires on the original deadline. */
static void test_mw_feed_failure_is_reported_and_expiry_follows(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(100u) == FF_SUCCESS);
    ff_extwd_advance_ms(50u);
    ff_extwd_set_fault(FF_EXTWD_FAULT_FEED_FAIL);

    FF_TEST_ASSERT(FF_PalWatchdogFeed() != FF_SUCCESS);
    /* NEGATIVE CONTROL: the device deadline is unchanged, and expiry follows. */
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 100u);
    ff_extwd_advance_ms(50u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
}

/* M6 - single reset path, at most one device reset (SRS-REQ-075): the SUT may
 * ask again, the device asserts once. */
static void test_mw_single_reset_path_at_most_once(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(100u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PalWatchdogReset() == FF_SUCCESS);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    /* The second request is refused by the device and reported to the SUT. */
    FF_TEST_ASSERT(FF_PalWatchdogReset() != FF_SUCCESS);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    /* NEGATIVE CONTROL: the SUT's request counter and the device's assertion
     * counter are different observables, and only the device's is the reset. */
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 2u);
}

/* M7 - a stuck feed (success reported, device not reloaded) still expires: the
 * contract cannot be satisfied by the SUT's opinion of its own health. */
static void test_mw_stuck_feed_still_expires(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(100u) == FF_SUCCESS);
    ff_extwd_set_fault(FF_EXTWD_FAULT_STUCK_FEED);

    /* Four "successful" feeds inside the window (t = 0, 25, 50, 75) ... */
    for (uint32_t i = 0u; i < 4u; i++) {
        FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
        ff_extwd_advance_ms(25u);
    }
    /* ... then the window elapses with no reload: the device dies anyway. */
    ff_extwd_advance_ms(25u);
    /* NEGATIVE CONTROL: four "successful" feeds and the device still expired. */
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 4u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
}

/* M8 - a feed at exactly the deadline reaches an expired device: the production
 * API reports NOT_READY (the adapter's E_STATE translation) and nothing is
 * reloaded (contract §2, XW1-F2). */
static void test_mw_feed_at_exact_deadline_is_not_ready(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(100u) == FF_SUCCESS);
    ff_extwd_advance_ms(100u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);

    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_NOT_READY);
    /* NEGATIVE CONTROL: no feed recorded on either side, no reload. */
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(ff_extwd_feeds() == 0u);
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 100u);
}

/* M9 - after the device has expired, the SUT's explicit reset uses the same
 * single mechanism and is answered as already asserted (SRS-REQ-075 "same
 * mechanism", XW1-F6). */
static void test_mw_reset_after_expiry_shares_the_mechanism(void)
{
    ff_extwd_reset();
    FF_TEST_ASSERT(FF_PalWatchdogInit(50u) == FF_SUCCESS);
    ff_extwd_advance_ms(50u);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);

    /* The device already asserted the reset: the SUT's request is refused by
     * the device, so no second assertion happens. */
    FF_TEST_ASSERT(FF_PalWatchdogReset() == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 1u);
}

FF_TEST_MAIN(test_mw_arm_reaches_the_device, test_mw_feed_reloads_the_device_deadline,
             test_mw_stopped_feeds_expire_the_device, test_mw_arm_failure_is_reported,
             test_mw_feed_failure_is_reported_and_expiry_follows,
             test_mw_single_reset_path_at_most_once, test_mw_stuck_feed_still_expires,
             test_mw_feed_at_exact_deadline_is_not_ready,
             test_mw_reset_after_expiry_shares_the_mechanism)
