/*
 * frick-fsw - External watchdog TEST model: abstract device contract
 *
 * TEST DOUBLE - NOT A DEVICE. See ff_test_extwd_model.h for the scope statement
 * and docs/vv/external-watchdog-test-contract.md for the contract it implements.
 * No production header is included: the model depends on fixed-width types only.
 *
 * Semantics implemented (contract §2):
 *   S1  arm from DISARMED sets deadline = now + window; ARMED.
 *   S2  feed while ARMED and now < deadline reloads deadline = now + window.
 *   S3  the harness clock reaching the deadline expires the device exactly once
 *       (inclusive boundary) and asserts the reset exactly once.
 *   S4  feed while EXPIRED / RESET_ASSERTED is refused and never un-expires.
 *   S5  a late feed after the deadline does not cancel the expiry (S3 happens
 *       at the boundary, before the feed is seen).
 *   S6  arm/feed failures report an error and never leave the device silently
 *       ARMED.
 *   S7  the reset is asserted at most once per arm cycle; a second request is
 *       refused with FF_EXTWD_E_RESET.
 *   S8  a clock jump larger than the window expires once, at the deadline.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "ff_test_extwd_model.h"

#define FF_EXTWD_COUNTER_MAX 0xFFFFFFFFu

static struct
{
    ff_extwd_state_t state;
    uint32_t now_ms;
    uint32_t window_ms;
    uint32_t deadline_ms;
    ff_extwd_fault_t fault;
    uint32_t expiries;
    uint32_t resets;
    uint32_t feeds;
    uint32_t jumps;
    bool overflow;
} g_model;

static void ff_extwd_bump(uint32_t *counter)
{
    if (*counter == FF_EXTWD_COUNTER_MAX) {
        g_model.overflow = true;
        return;
    }
    (*counter)++;
}

/* Advance the logical state as far as the clock requires. Called after every
 * clock step and before every observation, so the observable never depends on
 * when the harness looks. */
static void ff_extwd_settle(void)
{
    if ((g_model.state == FF_EXTWD_ARMED) && (g_model.now_ms >= g_model.deadline_ms)) {
        g_model.state = FF_EXTWD_EXPIRED;
        ff_extwd_bump(&g_model.expiries);
        /* The device asserts the reset itself at expiry: this is the property
         * the SUT must never have to perform (SRS-REQ-073 independence). */
        g_model.state = FF_EXTWD_RESET_ASSERTED;
        ff_extwd_bump(&g_model.resets);
    }
}

void ff_extwd_reset(void)
{
    g_model.state = FF_EXTWD_DISARMED;
    g_model.now_ms = 0u;
    g_model.window_ms = FF_EXTWD_DEFAULT_WINDOW_MS;
    g_model.deadline_ms = 0u;
    g_model.fault = FF_EXTWD_FAULT_NONE;
    g_model.expiries = 0u;
    g_model.resets = 0u;
    g_model.feeds = 0u;
    g_model.jumps = 0u;
    g_model.overflow = false;
}

void ff_extwd_set_window_ms(uint32_t window_ms)
{
    g_model.window_ms = window_ms;
}

uint32_t ff_extwd_window_ms(void)
{
    return g_model.window_ms;
}

void ff_extwd_set_fault(ff_extwd_fault_t fault)
{
    g_model.fault = fault;
}

ff_extwd_fault_t ff_extwd_fault(void)
{
    return g_model.fault;
}

void ff_extwd_advance_ms(uint32_t delta_ms)
{
    uint32_t before = g_model.now_ms;

    g_model.now_ms += delta_ms;
    if (g_model.now_ms < before) {
        g_model.overflow = true; /* clock domain wrapped: never silently */
    }
    ff_extwd_settle();

    /* A premature-reset fault asserts the reset without an expiry (S6 family:
     * the device misbehaves, the SUT must not be able to hide it). */
    if ((g_model.fault == FF_EXTWD_FAULT_PREMATURE_RESET) && (g_model.state == FF_EXTWD_ARMED) &&
        (delta_ms != 0u)) {
        g_model.state = FF_EXTWD_RESET_ASSERTED;
        ff_extwd_bump(&g_model.resets);
    }
    if (g_model.fault == FF_EXTWD_FAULT_CLOCK_JUMP) {
        /* S8: a step larger than the window is a clock jump. Counted so the
         * fault mode has a declared observable (XW1-F3). */
        if (delta_ms > g_model.window_ms) {
            ff_extwd_bump(&g_model.jumps);
        }
    }
}

uint32_t ff_extwd_now_ms(void)
{
    ff_extwd_settle();
    return g_model.now_ms;
}

uint32_t ff_extwd_deadline_ms(void)
{
    return g_model.deadline_ms;
}

int ff_extwd_arm(void)
{
    if (g_model.fault == FF_EXTWD_FAULT_ARM_FAIL) {
        return FF_EXTWD_E_ARM; /* state stays DISARMED: no silent armed state */
    }
    if (g_model.state != FF_EXTWD_DISARMED) {
        return FF_EXTWD_E_STATE;
    }
    g_model.deadline_ms = g_model.now_ms + g_model.window_ms;
    g_model.state = FF_EXTWD_ARMED;
    return FF_EXTWD_OK;
}

int ff_extwd_feed(void)
{
    ff_extwd_settle();

    if (g_model.fault == FF_EXTWD_FAULT_FEED_FAIL) {
        return FF_EXTWD_E_FEED; /* no reload */
    }
    if (g_model.state != FF_EXTWD_ARMED) {
        return FF_EXTWD_E_STATE; /* expired/disarmed: never un-expire */
    }
    if (g_model.fault == FF_EXTWD_FAULT_STUCK_FEED) {
        /* Reports success, does not reload: the SUT cannot tell, the deadline
         * still expires. Modelled so a test can prove the expiry is the
         * device's and not a software opinion. */
        ff_extwd_bump(&g_model.feeds);
        return FF_EXTWD_OK;
    }
    g_model.deadline_ms = g_model.now_ms + g_model.window_ms;
    ff_extwd_bump(&g_model.feeds);
    return FF_EXTWD_OK;
}

int ff_extwd_request_reset(void)
{
    if (g_model.state == FF_EXTWD_RESET_ASSERTED) {
        return FF_EXTWD_E_RESET; /* at most one reset per arm cycle */
    }
    g_model.state = FF_EXTWD_RESET_ASSERTED;
    ff_extwd_bump(&g_model.resets);
    return FF_EXTWD_OK;
}

ff_extwd_state_t ff_extwd_state(void)
{
    ff_extwd_settle();
    return g_model.state;
}

uint32_t ff_extwd_expiries(void)
{
    return g_model.expiries;
}

uint32_t ff_extwd_resets(void)
{
    return g_model.resets;
}

uint32_t ff_extwd_feeds(void)
{
    return g_model.feeds;
}

uint32_t ff_extwd_jumps(void)
{
    return g_model.jumps;
}

bool ff_extwd_overflows(void)
{
    return g_model.overflow;
}
