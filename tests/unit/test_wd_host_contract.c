/*
 * frick-fsw - Host watchdog model/contract cases (link-time test double)
 *
 * HOST MODEL / PORTABLE-CONTRACT EVIDENCE ONLY. The executable exercises
 * (a) a host test double - tests/stubs/ff_test_wd_model.c plus the
 * link-wrapped ff_pal_wd_hw_* backend below - and (b) the UNCHANGED
 * production PAL contract (src/core/pal/pal_watchdog.c) on the host. It is
 * NOT the MPFS/MSS binding (which does not exist - NC-PAL-01), NOT the
 * Renode pinned model, and NOT target, HIL, board or register-level evidence.
 * The properties checked here are the internal consistency of the host model
 * with the contract recorded in FRICK-ARCH-0024 rev 1.5 (F-2, F-3/F-7, F-4,
 * F-5, F-8) and the propagation of a backend refusal through the production
 * PAL contract. No register value, timeout, clock, prescaler, instance, reset
 * scope, gating value, window value or lock rule is claimed as a device or
 * mission fact; the model's time/window/clock values are SYNTHETIC TEST
 * PARAMETERS ONLY.
 *
 * The model's tick domain has NO defined relation to the PAL's millisecond
 * domain: the timeout passed to FF_PalWatchdogInit() is never converted into
 * the model's tick domain and is deliberately unrelated to the model's TIME.
 *
 * The FF_TEST_HOOKS status-injection seam (FF_PalWatchdogTestSetHardwareResults)
 * is deliberately NOT used: the link-time wrap bypasses it. This file contains
 * no production code and no binding; the double performs no register access of
 * any kind - it only drives the model fixture.
 *
 * Case scope classes: HM-P1..HM-P6 are HOST-MODEL cases. The HS-P series of
 * FRICK-ARCH-0024 rev 1.5 is binding-scoped; none of those cases is written,
 * run or discharged here. The frozen negative controls NC-1..NC-4 are
 * defined against the binding and are NOT RUN.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "ff_test_wd_model.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Synthetic test parameters. None of these is a mission or device value.
 * ---------------------------------------------------------------------- */

/* Synthetic tick-domain configuration used by the double. */
static uint32_t g_double_time_ticks = 1000u;
static uint32_t g_double_trigger_ticks = 10u;
static uint32_t g_double_msvp_ticks = 900u;

/* The refresh key the double carries. Two separate parameters exist so that a
 * mutation of the NOMINAL FEED PATH (the refresh key omitted on the periodic
 * feed) can be expressed without touching the initial configuration reload. */
static uint32_t g_double_init_key = FF_TEST_WD_REFRESH_KEY;
static uint32_t g_double_feed_key = FF_TEST_WD_REFRESH_KEY;

/* Call counters of the double: the runtime proof that the wrapped backend is
 * in the call path of the production PAL. */
static uint32_t g_double_arm_calls;
static uint32_t g_double_feed_calls;
static uint32_t g_double_reset_calls;

/* The timeout the PAL was asked for. Recorded only: it is never converted
 * into the model's tick domain. */
static uint32_t g_double_arm_timeout_ms;

/* -------------------------------------------------------------------------
 * The link-time test double. It stands in for the absent MPFS/MSS binding:
 * it issues the documented configuration sequence and the keyed refresh and
 * derives the feed outcome from a POST-WRITE READ-BACK (the documented feed
 * rule: on a read-back that shows no reload, report an error). It contains no
 * peripheral semantics - those live in the model fixture.
 * ---------------------------------------------------------------------- */

FF_Status_t __wrap_ff_pal_wd_hw_arm(uint32_t timeout_ms);
FF_Status_t __wrap_ff_pal_wd_hw_feed(void);
FF_Status_t __wrap_ff_pal_wd_hw_reset(void);

FF_Status_t __wrap_ff_pal_wd_hw_arm(uint32_t timeout_ms)
{
    g_double_arm_calls++;
    g_double_arm_timeout_ms = timeout_ms;

    /* The documented configuration sequence of the cited driver (F-2): the
     * trigger, the maximum-value register, the time register, the control
     * register and then the initial reload carrying the key. The order is
     * reproduced so that the write observer has something to observe; it is
     * NOT a device order claim (the binding's order must come from the
     * controlled device documentation, M-1). */
    ff_test_wd_model_write(FF_TEST_WD_REG_TRIGGER, g_double_trigger_ticks);
    ff_test_wd_model_write(FF_TEST_WD_REG_MSVP, g_double_msvp_ticks);
    ff_test_wd_model_write(FF_TEST_WD_REG_TIME, g_double_time_ticks);
    ff_test_wd_model_write(FF_TEST_WD_REG_CONTROL, 1u);
    ff_test_wd_model_write(FF_TEST_WD_REG_REFRESH, g_double_init_key);
    return FF_SUCCESS;
}

FF_Status_t __wrap_ff_pal_wd_hw_feed(void)
{
    uint32_t before;
    uint32_t after;

    g_double_feed_calls++;
    before = ff_test_wd_model_counter();

    ff_test_wd_model_write(FF_TEST_WD_REG_REFRESH, g_double_feed_key);

    /* Feed outcome derived from a post-write read-back, not from the model's
     * write decision. The change condition keeps the criterion from being
     * satisfied by a counter that already sat at the reload value. */
    after = ff_test_wd_model_counter();
    if ((after == ff_test_wd_model_read(FF_TEST_WD_REG_TIME)) && (after != before)) {
        return FF_SUCCESS;
    }
    return FF_ERROR;
}

FF_Status_t __wrap_ff_pal_wd_hw_reset(void)
{
    g_double_reset_calls++;
    /* The device's forced-reset semantics are not modelled and the reset
     * path is not exercised by these cases (HS-P7 stays NOT RUN). The hook
     * fails closed so that it cannot silently satisfy a future case. */
    return FF_NOT_IMPLEMENTED;
}

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

/* Resets the model and the double between cases: the PAL keeps process-global
 * state, so every case starts with a successful FF_PalWatchdogInit (the only
 * path that resets the feed and reset counters). */
static void ff_test_wd_test_reset_all(void)
{
    static bool banner_printed;

    if (!banner_printed) {
        banner_printed = true;
        (void)printf("EVIDENCE CLASS: HOST MODEL / PORTABLE-CONTRACT "
                     "(not binding, not emulator, not target, not hardware)\n");
    }
    ff_test_wd_model_reset();
    g_double_time_ticks = 1000u;
    g_double_trigger_ticks = 10u;
    g_double_msvp_ticks = 900u;
    g_double_init_key = FF_TEST_WD_REFRESH_KEY;
    g_double_feed_key = FF_TEST_WD_REFRESH_KEY;
    g_double_arm_calls = 0u;
    g_double_feed_calls = 0u;
    g_double_reset_calls = 0u;
    g_double_arm_timeout_ms = 0u;
}

/* -------------------------------------------------------------------------
 * HM-P1 - write-once lock enforcement (host model, both lock-rule variants)
 *
 * Healthy control : the configuration writes are applied and read back.
 * Fault/negative  : a write to TIME/TRIGGER issued after the configuration is
 *                   ignored - the read-back is unchanged and the observer
 *                   records the disposition.
 * Discriminating  : the same write on a model that is not configured yet IS
 *                   applied, so "ignored" is distinguishable from "applied".
 * Observable       : register read-back (primary); observer disposition and
 *                   the model's lock state (secondary).
 * ---------------------------------------------------------------------- */
static void test_host_model_lock_enforcement(void)
{
    for (int variant = 0; variant < 2; variant++) {
        ff_test_wd_lock_rule_t rule =
            (variant == 0) ? FF_TEST_WD_LOCK_AFTER_CFG_BLOCK : FF_TEST_WD_LOCK_AFTER_TIME;

        /* Variant-discriminating leg: a control-register write lies inside
         * the offsets-variant's lock range but not inside the TIME-variant's,
         * so the lock state differs between the two upstream readings. This
         * is the only assertion that discriminates the variants. */
        ff_test_wd_test_reset_all();
        ff_test_wd_model_set_lock_rule(rule);
        ff_test_wd_model_write(FF_TEST_WD_REG_CONTROL, 1u);
        if (variant == 0) {
            FF_TEST_ASSERT(ff_test_wd_model_locked());
        }
        else {
            FF_TEST_ASSERT(!ff_test_wd_model_locked());
        }

        /* Discriminating counterpart: before any configuration the same write
         * is applied. */
        ff_test_wd_test_reset_all();
        ff_test_wd_model_set_lock_rule(rule);
        ff_test_wd_model_write(FF_TEST_WD_REG_TIME, 500u);
        FF_TEST_ASSERT(ff_test_wd_model_read(FF_TEST_WD_REG_TIME) == 500u);
        FF_TEST_ASSERT(ff_test_wd_model_write_count() == 1u);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(0u) == FF_TEST_WD_DISP_APPLIED);

        /* Healthy control and negative leg. */
        ff_test_wd_test_reset_all();
        ff_test_wd_model_set_lock_rule(rule);
        FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);

        /* Runtime proof that the wrapped double is in the call path. */
        FF_TEST_ASSERT(g_double_arm_calls == 1u);
        FF_TEST_ASSERT(g_double_arm_timeout_ms == 12345u);
        FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
        FF_TEST_ASSERT(FF_PalWatchdogGetResets() == 0u);
        FF_TEST_ASSERT(FF_PalWatchdogGetTimeoutMs() == 12345u);

        /* Healthy control: the configuration writes were applied. */
        FF_TEST_ASSERT(ff_test_wd_model_read(FF_TEST_WD_REG_TIME) == 1000u);
        FF_TEST_ASSERT(ff_test_wd_model_read(FF_TEST_WD_REG_TRIGGER) == 10u);
        FF_TEST_ASSERT(ff_test_wd_model_read(FF_TEST_WD_REG_MSVP) == 900u);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(0u) == FF_TEST_WD_DISP_APPLIED);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(1u) == FF_TEST_WD_DISP_APPLIED);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(2u) == FF_TEST_WD_DISP_APPLIED);
        FF_TEST_ASSERT(ff_test_wd_model_locked());

        /* Negative: the re-writes are ignored (read-back is the primary
         * observable; the disposition and the recorded state corroborate). */
        ff_test_wd_model_write(FF_TEST_WD_REG_TIME, 500u);
        ff_test_wd_model_write(FF_TEST_WD_REG_TRIGGER, 20u);
        FF_TEST_ASSERT(ff_test_wd_model_read(FF_TEST_WD_REG_TIME) == 1000u);
        FF_TEST_ASSERT(ff_test_wd_model_read(FF_TEST_WD_REG_TRIGGER) == 10u);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(5u) == FF_TEST_WD_DISP_IGNORED_LOCKED);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(6u) == FF_TEST_WD_DISP_IGNORED_LOCKED);
        FF_TEST_ASSERT(ff_test_wd_model_write_state_before(5u) == 1000u);
        FF_TEST_ASSERT(ff_test_wd_model_write_state_after(5u) == 1000u);
        FF_TEST_ASSERT(ff_test_wd_model_write_state_before(6u) == 10u);
        FF_TEST_ASSERT(ff_test_wd_model_write_state_after(6u) == 10u);
    }
}

/* -------------------------------------------------------------------------
 * HM-P2 - forbidden refresh window (host model)
 *
 * Negative        : with the window enabled and the counter above the
 *                   maximum-value register, the refresh is refused - the
 *                   counter is not reloaded, the forbidden status and the
 *                   timeout interrupt are raised and the production feed
 *                   counter does not advance.
 * Healthy control : once the counter has reached the maximum-value register
 *                   the same refresh is permitted and reloads the counter;
 *                   with the window disabled a refresh is permitted at any
 *                   counter value.
 * Observable       : counter read-back (primary); forbidden status, timeout
 *                   interrupt, observer disposition and the production feed
 *                   counter (secondary).
 * ---------------------------------------------------------------------- */
static void test_host_model_forbidden_window(void)
{
    /* Negative leg: the refresh falls inside the forbidden window. */
    ff_test_wd_test_reset_all();
    ff_test_wd_model_set_window_enabled(true);
    FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_double_arm_calls == 1u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);

    /* The counter is moved away from the reload value first, so that a
     * reload and a non-reload are distinguishable. */
    ff_test_wd_model_advance_ticks(50u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() != 1000u);

    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_ERROR);
    FF_TEST_ASSERT(g_double_feed_calls == 1u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u); /* primary: not reloaded */
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);     /* production observable */
    FF_TEST_ASSERT(ff_test_wd_model_forbidden_status());
    FF_TEST_ASSERT(ff_test_wd_model_timeout_interrupt());
    FF_TEST_ASSERT(ff_test_wd_model_write_disposition(5u) == FF_TEST_WD_DISP_REFUSED_WINDOW);

    /* Boundary: the refresh is forbidden only while the counter is strictly
     * greater than the maximum-value register. One tick above it is refused;
     * exactly at it the refresh is permitted, and the maximum-value crossing
     * is witnessed. */
    ff_test_wd_model_advance_ticks(49u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 901u);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_ERROR);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 901u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(ff_test_wd_model_write_disposition(6u) == FF_TEST_WD_DISP_REFUSED_WINDOW);

    ff_test_wd_model_advance_ticks(1u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 900u);
    FF_TEST_ASSERT(ff_test_wd_model_mvrp_interrupt());
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(g_double_feed_calls == 3u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 1u);

    /* Healthy control: below the maximum-value register the refresh is
     * permitted and reloads the counter. */
    ff_test_wd_model_advance_ticks(110u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 890u);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(g_double_feed_calls == 4u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 2u);

    /* Healthy control: with the window disabled (the default) a refresh is
     * permitted at any counter value. */
    ff_test_wd_test_reset_all();
    FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);
    ff_test_wd_model_advance_ticks(50u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 1u);
}

/* -------------------------------------------------------------------------
 * HM-P3 - peripheral gating and clock running/gated (host model)
 *
 * Negative        : while gated, an access is refused and is not reported as
 *                   success, the production feed counter does not advance and
 *                   the peripheral clock does not run (a model-only
 *                   assumption, not a device fact).
 * Healthy control : after ungating, the same feed is accepted, the counter
 *                   reloads and the counter advances with the clock.
 * Observable       : counter value across an explicit tick advance and the
 *                   clock-running flag (primary); the production feed counter
 *                   and the observer disposition (secondary).
 * ---------------------------------------------------------------------- */
static void test_host_model_gating_and_clock(void)
{
    ff_test_wd_test_reset_all();
    FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);
    ff_test_wd_model_advance_ticks(50u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);

    ff_test_wd_model_set_gated(true);
    FF_TEST_ASSERT(!ff_test_wd_model_clock_running());

    /* The peripheral clock does not run while gated. */
    ff_test_wd_model_advance_ticks(100u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);

    /* An access while gated is refused and is not reported as success. */
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_ERROR);
    FF_TEST_ASSERT(g_double_feed_calls == 1u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
    FF_TEST_ASSERT(ff_test_wd_model_write_disposition(5u) == FF_TEST_WD_DISP_REFUSED_GATED);

    /* Healthy control: after ungating the feed is accepted and the clock
     * runs again. */
    ff_test_wd_model_set_gated(false);
    FF_TEST_ASSERT(ff_test_wd_model_clock_running());
    ff_test_wd_model_advance_ticks(10u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 940u);
    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(g_double_feed_calls == 2u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 1u);
}

/* -------------------------------------------------------------------------
 * HM-P4 - refresh-key class: wrong keys are refused (host model)
 *
 * The wrong key is exercised with THREE distinct values, each of them
 * non-zero and different from the documented key. A single-value stimulus
 * would not exclude a model that reloads on everything except one specific
 * wrong value.
 * Observable : counter read-back unchanged and different from the reload
 *              value (primary); the feed status, the production feed counter
 *              and the observer disposition (secondary).
 * ---------------------------------------------------------------------- */
static void test_host_model_wrong_key_class(void)
{
    static const uint32_t wrong_keys[3] = {0x00000001u, 0xDEADC0DFu, 0xFFFFFFFFu};
    size_t i;

    ff_test_wd_test_reset_all();
    FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);
    ff_test_wd_model_advance_ticks(50u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() != 1000u);

    for (i = 0u; i < 3u; i++) {
        g_double_feed_key = wrong_keys[i];
        FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_ERROR);
        FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);
        FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 0u);
        FF_TEST_ASSERT(ff_test_wd_model_write_disposition(5u + i) == FF_TEST_WD_DISP_REFUSED_KEY);
    }

    g_double_feed_key = FF_TEST_WD_REFRESH_KEY;
}

/* -------------------------------------------------------------------------
 * HM-P5 - refresh with the documented key reloads (host model)
 *
 * The counter is moved away from the reload value first, so that the
 * positive case distinguishes a reload from a non-reload. This is the case a
 * key omission on the nominal feed path must break.
 * Observable : counter read-back equal to the reload value (primary); the
 *              feed status, the production feed counter and the observer
 *              disposition (secondary).
 * ---------------------------------------------------------------------- */
static void test_host_model_correct_key_reload(void)
{
    ff_test_wd_test_reset_all();
    FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_double_arm_calls == 1u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);

    ff_test_wd_model_advance_ticks(50u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 950u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() != 1000u);

    FF_TEST_ASSERT(FF_PalWatchdogFeed() == FF_SUCCESS);
    FF_TEST_ASSERT(g_double_feed_calls == 1u);
    FF_TEST_ASSERT(ff_test_wd_model_counter() == 1000u);
    FF_TEST_ASSERT(FF_PalWatchdogGetFeeds() == 1u);
    FF_TEST_ASSERT(ff_test_wd_model_write_disposition(5u) == FF_TEST_WD_DISP_APPLIED);
}

/* -------------------------------------------------------------------------
 * HM-P6 - write-observer capability self-check (host model)
 *
 * HARNESS-CAPABILITY SELF-CHECK, NOT HS-P5 EVIDENCE and NOT NC-3 EVIDENCE:
 * both the actor and the oracle are the fixture. The check is a relative
 * order (subsequence) requirement, so documented surrounding writes may
 * appear before or after the configuration writes.
 * ---------------------------------------------------------------------- */
static void test_host_model_write_observer_order(void)
{
    static const ff_test_wd_reg_t expected[4] = {FF_TEST_WD_REG_TRIGGER, FF_TEST_WD_REG_MSVP,
                                                 FF_TEST_WD_REG_TIME, FF_TEST_WD_REG_CONTROL};
    size_t next = 0u;
    size_t i;

    ff_test_wd_test_reset_all();
    FF_TEST_ASSERT(FF_PalWatchdogInit(12345u) == FF_SUCCESS);
    FF_TEST_ASSERT(ff_test_wd_model_write_count() == 5u);
    FF_TEST_ASSERT(!ff_test_wd_model_write_overflowed());
    /* The observer records the value of each write as well as its order. */
    FF_TEST_ASSERT(ff_test_wd_model_write_value(0u) == 10u);
    FF_TEST_ASSERT(ff_test_wd_model_write_value(2u) == 1000u);

    for (i = 0u; (i < ff_test_wd_model_write_count()) && (next < 4u); i++) {
        if (ff_test_wd_model_write_reg(i) == expected[next]) {
            next++;
        }
    }
    FF_TEST_ASSERT(next == 4u);
}

FF_TEST_MAIN(test_host_model_lock_enforcement, test_host_model_forbidden_window,
             test_host_model_gating_and_clock, test_host_model_wrong_key_class,
             test_host_model_correct_key_reload, test_host_model_write_observer_order)
