/*
 * frick-fsw - External watchdog model validation (XW-1 §2)
 *
 * Validates tests/stubs/ff_test_extwd_model against AUTHORED expected traces:
 * every expectation below is a literal written from
 * docs/vv/external-watchdog-test-contract.md, never computed by the model, so
 * the oracle cannot be circular.
 *
 * Each case also carries a NEGATIVE CONTROL: an assertion that the same
 * observation is different one step away (just before vs at the deadline,
 * before vs after the reset, one feed vs none). If an implementation is off by
 * one tick, inverted, or forgets to reload, the control fails with it - which is
 * what makes the expectation discriminating rather than tautological.
 *
 * TEST DOUBLE ONLY: no production code is linked here.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "ff_test.h"
#include "ff_test_extwd_model.h"

/* T1 - clock advance with no feed: expiry at the declared boundary, exactly
 * once, and the reset asserted by the device itself. */
static void test_extwd_expiry_at_boundary(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_ARMED);

    /* NEGATIVE CONTROL: one tick before the deadline the device is still armed
     * and has neither expired nor reset. */
    ff_extwd_advance_ms(99u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_ARMED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 0u);
    FF_TEST_ASSERT(ff_extwd_resets() == 0u);

    /* At the deadline (inclusive) the device expires and asserts the reset. */
    ff_extwd_advance_ms(1u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);

    /* NEGATIVE CONTROL: further time does not expire/reset a second time. */
    ff_extwd_advance_ms(500u);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    FF_TEST_ASSERT(!ff_extwd_overflows());
}

/* T2 - a valid feed before the deadline reloads it; the original deadline would
 * have expired but does not. */
static void test_extwd_feed_reloads_deadline(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    ff_extwd_advance_ms(60u);
    FF_TEST_ASSERT(ff_extwd_feed() == FF_EXTWD_OK);
    FF_TEST_ASSERT(ff_extwd_feeds() == 1u);

    /* NEGATIVE CONTROL: crossing the ORIGINAL deadline (t=100) does not expire
     * the device, because the feed moved it to t=160. */
    ff_extwd_advance_ms(40u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_ARMED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 0u);

    /* ... and the reloaded deadline does expire it. */
    ff_extwd_advance_ms(60u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
}

/* T3 - a late feed finds the device already expired and does not resurrect it. */
static void test_extwd_late_feed_does_not_unexpire(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    ff_extwd_advance_ms(150u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);

    /* NEGATIVE CONTROL: the refused feed is observable and changes nothing. */
    FF_TEST_ASSERT(ff_extwd_feed() == FF_EXTWD_E_STATE);
    FF_TEST_ASSERT(ff_extwd_feeds() == 0u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
}

/* T4 - no feed at all: expiry; and an ARM FAILURE never leaves the device
 * silently armed. */
static void test_extwd_arm_failure_has_no_silent_armed_state(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(50u);
    ff_extwd_set_fault(FF_EXTWD_FAULT_ARM_FAIL);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_E_ARM);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_DISARMED);

    /* NEGATIVE CONTROL: a device that is not armed never expires, however long
     * the clock runs - expiry cannot be faked from the outside. */
    ff_extwd_advance_ms(1000u);
    FF_TEST_ASSERT(ff_extwd_expiries() == 0u);
    FF_TEST_ASSERT(ff_extwd_resets() == 0u);

    /* With the fault cleared the same call arms, so the control above is not
     * passing merely because arm is broken. */
    ff_extwd_set_fault(FF_EXTWD_FAULT_NONE);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_ARMED);
}

/* T5 - feed failure is reported and does not reload, so the device expires on
 * the original deadline. */
static void test_extwd_feed_failure_reports_and_still_expires(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    ff_extwd_advance_ms(50u);
    ff_extwd_set_fault(FF_EXTWD_FAULT_FEED_FAIL);
    FF_TEST_ASSERT(ff_extwd_feed() == FF_EXTWD_E_FEED);
    FF_TEST_ASSERT(ff_extwd_feeds() == 0u);

    /* NEGATIVE CONTROL: the refused feed did not move the deadline. */
    ff_extwd_advance_ms(50u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
}

/* T6 - a "stuck" feed (success reported, no reload) still expires: the expiry
 * is the device's, not the software's opinion. */
static void test_extwd_stuck_feed_still_expires(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    ff_extwd_set_fault(FF_EXTWD_FAULT_STUCK_FEED);
    for (uint32_t i = 0u; i < 4u; i++) {
        FF_TEST_ASSERT(ff_extwd_feed() == FF_EXTWD_OK);
        ff_extwd_advance_ms(30u);
    }
    /* NEGATIVE CONTROL: the SUT "fed" four times and the device still died. */
    FF_TEST_ASSERT(ff_extwd_feeds() == 4u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
}

/* T7 - duplicate reset request asserts exactly one reset. */
static void test_extwd_reset_at_most_once(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    FF_TEST_ASSERT(ff_extwd_request_reset() == FF_EXTWD_OK);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);

    /* NEGATIVE CONTROL: a second request is refused and adds nothing. */
    FF_TEST_ASSERT(ff_extwd_request_reset() == FF_EXTWD_E_RESET);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);

    /* Late clock movement after the reset does not expire or reset again. */
    ff_extwd_advance_ms(500u);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    FF_TEST_ASSERT(ff_extwd_expiries() == 0u);
}

/* T8 - a clock jump larger than the window expires once at the deadline (never
 * twice, never retroactively). */
static void test_extwd_clock_jump_expires_once(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    ff_extwd_set_fault(FF_EXTWD_FAULT_CLOCK_JUMP);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    ff_extwd_advance_ms(5000u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    FF_TEST_ASSERT(!ff_extwd_overflows());
    /* F3: the fault mode has its own observable, so it is not a label.
     * NEGATIVE CONTROL (XW1R2-N4): a step NOT larger than the window is not a
     * jump, so the counter does not move - an implementation that counted every
     * advance would fail here. */
    FF_TEST_ASSERT(ff_extwd_jumps() == 1u);
    ff_extwd_advance_ms(50u);
    FF_TEST_ASSERT(ff_extwd_jumps() == 1u);
}

/* T11 - a service at EXACTLY the deadline arrives after the expiry and is
 * refused: no un-expiry, no reload (contract §2, XW1-F2). */
static void test_extwd_feed_at_exact_deadline_is_refused(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);

    ff_extwd_advance_ms(100u); /* exactly the deadline */
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);

    /* NEGATIVE CONTROL: the feed is refused, counts nothing and does not move
     * the deadline (an implementation that settles AFTER the reload would pass
     * the state check but would also have reloaded - both are asserted). */
    FF_TEST_ASSERT(ff_extwd_feed() == FF_EXTWD_E_STATE);
    FF_TEST_ASSERT(ff_extwd_feeds() == 0u);
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 100u);
    FF_TEST_ASSERT(ff_extwd_expiries() == 1u);
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
}

/* T9 - premature reset fault: the device asserts a reset without an expiry, so
 * a SUT that equates "reset" with "expiry" is visible. */
static void test_extwd_premature_reset_fault(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(100u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);
    ff_extwd_set_fault(FF_EXTWD_FAULT_PREMATURE_RESET);

    ff_extwd_advance_ms(10u);
    /* NEGATIVE CONTROL: one reset, zero expiries - the two observables are
     * distinct, so a test cannot use one as a proxy for the other. */
    FF_TEST_ASSERT(ff_extwd_resets() == 1u);
    FF_TEST_ASSERT(ff_extwd_expiries() == 0u);
    FF_TEST_ASSERT(ff_extwd_state() == FF_EXTWD_RESET_ASSERTED);
}

/* T10 - the harness clock is the only time source: the model's deadline is a
 * pure function of the feed instant and the window. */
static void test_extwd_deadline_is_harness_owned(void)
{
    ff_extwd_reset();
    ff_extwd_set_window_ms(40u);
    FF_TEST_ASSERT(ff_extwd_arm() == FF_EXTWD_OK);
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 40u);

    ff_extwd_advance_ms(15u);
    FF_TEST_ASSERT(ff_extwd_feed() == FF_EXTWD_OK);
    /* NEGATIVE CONTROL: the reload is now + window, not a fixed cycle. */
    FF_TEST_ASSERT(ff_extwd_deadline_ms() == 55u);
    FF_TEST_ASSERT(ff_extwd_now_ms() == 15u);
}

FF_TEST_MAIN(test_extwd_expiry_at_boundary, test_extwd_feed_reloads_deadline,
             test_extwd_late_feed_does_not_unexpire,
             test_extwd_arm_failure_has_no_silent_armed_state,
             test_extwd_feed_failure_reports_and_still_expires, test_extwd_stuck_feed_still_expires,
             test_extwd_reset_at_most_once, test_extwd_clock_jump_expires_once,
             test_extwd_premature_reset_fault, test_extwd_deadline_is_harness_owned,
             test_extwd_feed_at_exact_deadline_is_refused)
