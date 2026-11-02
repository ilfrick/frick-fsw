/*
 * frick-fsw - XW-2 test-only harness: abstract external-watchdog contract on the
 * emulated target
 *
 * TEST-ONLY. Compiled only when FF_ENABLE_EXTWD_TARGET_TEST is ON, into the
 * separate executable `frick_fsw_xw2`; it is never part of `frick_fsw`, and no
 * file under `src/` or `config/` is modified by this increment.
 *
 * What it does: it exercises the PRODUCTION watchdog API (`FF_PalWatchdogInit`,
 * `.Feed`, `.Check`, `.Reset` from the unchanged `src/core/pal/pal_watchdog.c`)
 * against the validated abstract model `tests/stubs/ff_test_extwd_model.c`, which
 * is reached through the link-time wrappers of the three device hooks
 * (`tests/stubs/ff_test_extwd_pal.c`, selected with -Wl,--wrap=...). The device
 * seam is substituted; the production code path is not.
 *
 * Clock discipline (contract §2/§5.1): the model's clock is advanced ONLY by this
 * harness, and only while the calling context is the single-threaded init thread
 * (XW-T*), so every expectation there is exact. The post-boot case (XW-B1) runs in
 * a task and asserts feed COUNTS only - never a deadline - because from that point
 * the SUT and the harness share the scheduler.
 *
 * Expectations are derived from the SUT's own configured timeout
 * (`FF_GetMissionCfg()->watchdog_timeout_s`), used as a TEST PARAMETER. No mission
 * value is asserted: the 8 s table entry remains an ESC-7 placeholder.
 *
 * Evidence grammar: the existing `TGTREQ:` lines, verified by the existing
 * fail-closed tools (separate manifest `tools/scripts/xw2_manifest.json`).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "app_xw2_test.h"

#include "core/pal/pal.h"
#include "core/time/time.h"
#include "tables/fsw_tables.h"
#include "ff_test_extwd_model.h"

#include <stdbool.h>
#include <stdint.h>

/* ---- evidence lines ------------------------------------------------- */

static void xw2_line(const char *status, const char *rid, const char *cid, const char *detail)
{
    FF_PalLog("TGTREQ: %s:%s:%s:%s\n", rid, cid, status, detail);
}

static void xw2_pass(const char *rid, const char *cid, const char *detail)
{
    xw2_line("PASS", rid, cid, detail);
}

static void xw2_fail(const char *rid, const char *cid, const char *detail)
{
    xw2_line("FAIL", rid, cid, detail);
}

/* ---- test parameter: the SUT's own derived timeout ------------------- */

static uint32_t xw2_timeout_ms(void)
{
    const FF_MissionCfg_t *cfg = FF_GetMissionCfg();

    if ((cfg == NULL) || (cfg->watchdog_timeout_s == 0u)) {
        return 0u;
    }
    return (uint32_t)cfg->watchdog_timeout_s * 1000u;
}

/* ---- XW-T1..XW-T12: pre-init, single-threaded, authored clock ------- */

static void xw2_case_t1(uint32_t t)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T1", "init_failed");
        return;
    }
    if ((ff_extwd_state() != FF_EXTWD_ARMED) || (ff_extwd_window_ms() != t) ||
        (ff_extwd_deadline_ms() != t) || (FF_PalWatchdogGetTimeoutMs() != t)) {
        xw2_fail("SRS-REQ-073", "XW-T1", "device_not_armed_with_sut_timeout");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T1", "armed_window_eq_sut_timeout");
}

static void xw2_case_t2(uint32_t t, uint32_t f)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T2", "init_failed");
        return;
    }
    ff_extwd_advance_ms(f);
    if (FF_PalWatchdogFeed() != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T2", "feed_failed");
        return;
    }
    if ((ff_extwd_deadline_ms() != (f + t)) || (ff_extwd_feeds() != 1u) ||
        (FF_PalWatchdogGetFeeds() != 1u)) {
        xw2_fail("SRS-REQ-073", "XW-T2", "deadline_not_reloaded");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T2", "feed_reloaded_deadline");
}

static void xw2_case_t3_t4(uint32_t t)
{
    /* T3: one tick before the deadline the device is still armed. */
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T3", "init_failed");
        xw2_fail("SRS-REQ-073", "XW-T4", "init_failed");
        return;
    }
    ff_extwd_advance_ms(t - 1u);
    if ((ff_extwd_state() != FF_EXTWD_ARMED) || (ff_extwd_expiries() != 0u)) {
        xw2_fail("SRS-REQ-073", "XW-T3", "expired_before_deadline");
    }
    else {
        xw2_pass("SRS-REQ-073", "XW-T3", "armed_at_deadline_minus_1");
    }

    /* T4: at the deadline (inclusive) the device expires and resets once. */
    ff_extwd_advance_ms(1u);
    if ((ff_extwd_state() != FF_EXTWD_RESET_ASSERTED) || (ff_extwd_expiries() != 1u) ||
        (ff_extwd_resets() != 1u)) {
        xw2_fail("SRS-REQ-073", "XW-T4", "boundary_or_reset_count");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T4", "expired_at_boundary_reset_once");
}

static void xw2_case_t5(uint32_t t)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T5", "init_failed");
        return;
    }
    ff_extwd_advance_ms(t + t + 5u); /* one step across many windows */
    if ((ff_extwd_expiries() != 1u) || (ff_extwd_state() != FF_EXTWD_RESET_ASSERTED)) {
        xw2_fail("SRS-REQ-073", "XW-T5", "jump_double_expiry");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T5", "one_step_one_expiry");
}

static void xw2_case_t6(uint32_t t)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T6", "init_failed");
        return;
    }
    ff_extwd_advance_ms(t);
    uint32_t feeds_before = ff_extwd_feeds();
    uint16_t sut_feeds_before = FF_PalWatchdogGetFeeds();
    if (FF_PalWatchdogFeed() == FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T6", "feed_accepted_after_expiry");
        return;
    }
    if ((ff_extwd_feeds() != feeds_before) || (FF_PalWatchdogGetFeeds() != sut_feeds_before) ||
        (ff_extwd_state() != FF_EXTWD_RESET_ASSERTED)) {
        xw2_fail("SRS-REQ-073", "XW-T6", "expiry_resurrected");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T6", "late_feed_refused_no_resurrection");
}

static void xw2_case_t7(uint32_t t)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-075", "XW-T7", "init_failed");
        return;
    }
    bool first_ok = (FF_PalWatchdogReset() == FF_SUCCESS);
    bool second_ok = (FF_PalWatchdogReset() == FF_SUCCESS);
    if (!first_ok) {
        xw2_fail("SRS-REQ-075", "XW-T7", "first_reset_refused");
        return;
    }
    if (second_ok) {
        xw2_fail("SRS-REQ-075", "XW-T7", "second_reset_accepted");
        return;
    }
    if ((ff_extwd_resets() != 1u) || (FF_PalWatchdogGetResets() != 2u)) {
        xw2_fail("SRS-REQ-075", "XW-T7", "assertion_or_request_count");
        return;
    }
    xw2_pass("SRS-REQ-075", "XW-T7", "one_device_assertion_two_requests");
}

static void xw2_case_t8(uint32_t t)
{
    ff_extwd_reset();
    ff_extwd_set_fault(FF_EXTWD_FAULT_ARM_FAIL);
    FF_Status_t st = FF_PalWatchdogInit(t);
    ff_extwd_set_fault(FF_EXTWD_FAULT_NONE);
    if (st == FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T8", "arm_failure_hidden");
        return;
    }
    if ((ff_extwd_state() != FF_EXTWD_DISARMED) || (FF_PalWatchdogFeed() != FF_NOT_READY)) {
        xw2_fail("SRS-REQ-073", "XW-T8", "silent_armed_state");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T8", "arm_failure_reported_no_silent_arm");
}

static void xw2_case_t9(uint32_t t, uint32_t f)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T9", "init_failed");
        return;
    }
    ff_extwd_advance_ms(f);
    uint32_t deadline = ff_extwd_deadline_ms();
    ff_extwd_set_fault(FF_EXTWD_FAULT_FEED_FAIL);
    FF_Status_t st = FF_PalWatchdogFeed();
    ff_extwd_set_fault(FF_EXTWD_FAULT_NONE);
    if (st == FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T9", "feed_failure_hidden");
        return;
    }
    if (ff_extwd_deadline_ms() != deadline) {
        xw2_fail("SRS-REQ-073", "XW-T9", "failed_feed_reloaded_deadline");
        return;
    }
    ff_extwd_advance_ms(deadline - ff_extwd_now_ms());
    if (ff_extwd_expiries() != 1u) {
        xw2_fail("SRS-REQ-073", "XW-T9", "no_expiry_after_feed_failure");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T9", "feed_failure_reported_expiry_follows");
}

static void xw2_case_t10(uint32_t t, uint32_t f)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T10", "init_failed");
        return;
    }
    ff_extwd_set_fault(FF_EXTWD_FAULT_STUCK_FEED);
    if (FF_PalWatchdogFeed() != FF_SUCCESS) {
        ff_extwd_set_fault(FF_EXTWD_FAULT_NONE);
        xw2_fail("SRS-REQ-073", "XW-T10", "stuck_feed_refused");
        return;
    }
    ff_extwd_advance_ms(t + f); /* deadline is still the original t */
    ff_extwd_set_fault(FF_EXTWD_FAULT_NONE);
    if ((ff_extwd_expiries() != 1u) || (ff_extwd_feeds() != 1u)) {
        xw2_fail("SRS-REQ-073", "XW-T10", "stuck_feed_prevented_expiry");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T10", "stuck_feed_still_expires");
}

static void xw2_case_t11(uint32_t t)
{
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T11", "init_failed");
        return;
    }
    ff_extwd_set_fault(FF_EXTWD_FAULT_CLOCK_JUMP);
    ff_extwd_advance_ms(t + 1u);
    ff_extwd_set_fault(FF_EXTWD_FAULT_NONE);
    if ((ff_extwd_jumps() != 1u) || (ff_extwd_expiries() != 1u)) {
        xw2_fail("SRS-REQ-073", "XW-T11", "jump_mode_not_observable");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T11", "jump_observable_and_single_expiry");
}

static void xw2_case_t12(uint32_t t)
{
    /* The SUT's own diagnostic runs on the RTEMS monotonic clock. It is NOT a
     * deadline assertion (the model clock is the authored one). */
    ff_extwd_reset();
    if (FF_PalWatchdogInit(t) != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T12", "init_failed");
        return;
    }
    if (FF_PalWatchdogCheck() != FF_SUCCESS) {
        xw2_fail("SRS-REQ-073", "XW-T12", "check_failed");
        return;
    }
    xw2_pass("SRS-REQ-073", "XW-T12", "sut_check_runs_on_rtems_clock");
}

/* ---- pre-init entry point ------------------------------------------- */

FF_Status_t FF_AppXw2RunPreInit(void)
{
    uint32_t t = xw2_timeout_ms();

    if (t == 0u) {
        xw2_fail("SRS-REQ-073", "XW-T1", "no_configured_timeout");
        return FF_ERROR;
    }
    /* Test parameter derived from the SUT's own timeout (FF_AppWdPlan uses T/4);
     * no mission value is asserted anywhere below. */
    uint32_t f = t / 4u;

    xw2_case_t1(t);
    xw2_case_t2(t, f);
    xw2_case_t3_t4(t);
    xw2_case_t5(t);
    xw2_case_t6(t);
    xw2_case_t7(t);
    xw2_case_t8(t);
    xw2_case_t9(t, f);
    xw2_case_t10(t, f);
    xw2_case_t11(t);
    xw2_case_t12(t);

    /* Leave the device disarmed: the production feeder will arm it when the WD
     * application starts (the flight path, unchanged). */
    ff_extwd_reset();
    return FF_SUCCESS;
}

/* ---- XW-B1: post-boot observation (counts only, never a deadline) ---- */

FF_Status_t FF_AppXw2RunPostBoot(void)
{
    const uint32_t window_ms = 3000u;
    uint32_t t = xw2_timeout_ms();

    if (t == 0u) {
        xw2_fail("SRS-REQ-073", "XW-B1", "no_configured_timeout");
        return FF_ERROR;
    }
    uint32_t f = t / 4u;
    uint32_t before = ff_extwd_feeds();

    FF_PalTaskDelayMs(window_ms);
    FF_PalLog("XW2_DBG: post_delay\n");
    FF_PalLog("XW2_DBG: post-delay wake\n");

    uint32_t after = ff_extwd_feeds();

    if (ff_extwd_state() != FF_EXTWD_ARMED) {
        xw2_fail("SRS-REQ-073", "XW-B1", "device_not_armed");
        return FF_ERROR;
    }
    if ((after - before) < 1u) {
        xw2_fail("SRS-REQ-073", "XW-B1", "no_feed_observed");
        return FF_ERROR;
    }
    /* The wrapper is the only live backend: the device's feed count must equal
     * the SUT's own counter (both increment once per successful feed). */
    if (ff_extwd_feeds() != FF_PalWatchdogGetFeeds()) {
        xw2_fail("SRS-REQ-073", "XW-B1", "device_and_sut_feed_counts_differ");
        return FF_ERROR;
    }
    FF_PalLog("TGTREQ-OBS: SRS-REQ-073:XW-B1:feeds=%u sut_feeds=%u feed_ms=%u\n", (unsigned)after,
              (unsigned)FF_PalWatchdogGetFeeds(), (unsigned)f);
    xw2_pass("SRS-REQ-073", "XW-B1", "production_feeder_reaches_the_device");
    return FF_SUCCESS;
}

void FF_AppXw2TestMain(void *arg)
{
    (void)arg;

    FF_PalLog("XW2_DBG: task_entry\n");
    (void)FF_AppXw2RunPostBoot();

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
