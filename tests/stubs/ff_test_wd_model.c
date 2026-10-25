/*
 * frick-fsw - Host watchdog model fixture: peripheral register semantics
 *
 * TEST DOUBLE - NOT A BINDING. See tests/stubs/ff_test_wd_model.h for the
 * scope statement. This translation unit implements only the peripheral-side
 * effects of a register write and the peripheral's own state; it contains no
 * sequencing, no policy, no binding logic and no device access.
 *
 * Semantics implemented (FRICK-ARCH-0024 rev 1.5):
 *   F-1 register identifiers and offsets (cited public driver; applicability
 *       to the flight device is not established - M-1);
 *   F-2 write-once lock of TIME/TRIGGER/MSVP, with BOTH upstream readings
 *       implemented because the upstream documentation contradicts itself
 *       (M-11); neither reading is device truth;
 *   F-3/F-7 refresh key check - a refresh reloads the counter only when it
 *       carries the documented key;
 *   F-4 down-counter reloaded from TIME, timeout interrupt at the TRIGGER
 *       crossing, MVRP interrupt at the MSVP crossing. F-4's fourth clause
 *       ("a reset is generated at zero") is NOT modelled: the counter halts
 *       at zero and the fixture has no reset observable (model-only, C8);
 *   F-5 forbidden refresh window - a refresh is forbidden while the counter is
 *       greater than MSVP and attempting one asserts the timeout interrupt;
 *       the window is disabled by default;
 *   F-8 peripheral gating - a peripheral gating mechanism exists and its bit
 *       values are undefined in the retrieved files. The MODEL-ONLY response
 *       to a gated access (a refusal, ahead of the key and window checks) is
 *       not an F-8 statement (model-only, C2).
 *
 * Model-only behaviour (documented in the addendum, not device facts): the
 * peripheral clock does not run while gated; the tick domain and the
 * synthetic TIME/TRIGGER/MSVP values; the disposition precedence; the
 * benign power-up state.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "ff_test_wd_model.h"

#include <string.h>

#define FF_TEST_WD_MAX_WRITES 64u

typedef struct
{
    ff_test_wd_reg_t reg;
    uint32_t value;
    ff_test_wd_disposition_t disposition;
    uint32_t state_before;
    uint32_t state_after;
} ff_test_wd_write_record_t;

static struct
{
    uint32_t refresh;
    uint32_t control;
    uint32_t status;
    uint32_t time;
    uint32_t msvp;
    uint32_t trigger;
    uint32_t force;
} g_regs;

static uint32_t g_counter;
static bool g_loaded;
static bool g_gated;
static bool g_window_enabled;
static bool g_locked;
static bool g_forbidden_status;
static bool g_timeout_interrupt;
static bool g_mvrp_interrupt;
static ff_test_wd_lock_rule_t g_lock_rule;
static ff_test_wd_write_record_t g_writes[FF_TEST_WD_MAX_WRITES];
static size_t g_write_count;
static bool g_write_overflowed;

static uint32_t ff_test_wd_offset(ff_test_wd_reg_t reg)
{
    switch (reg) {
    case FF_TEST_WD_REG_REFRESH:
        return 0x00u;
    case FF_TEST_WD_REG_CONTROL:
        return 0x04u;
    case FF_TEST_WD_REG_STATUS:
        return 0x08u;
    case FF_TEST_WD_REG_TIME:
        return 0x0cu;
    case FF_TEST_WD_REG_MSVP:
        return 0x10u;
    case FF_TEST_WD_REG_TRIGGER:
        return 0x14u;
    case FF_TEST_WD_REG_FORCE:
    default:
        return 0x18u;
    }
}

static bool ff_test_wd_is_configuration_register(ff_test_wd_reg_t reg)
{
    return (reg == FF_TEST_WD_REG_TIME) || (reg == FF_TEST_WD_REG_MSVP) ||
           (reg == FF_TEST_WD_REG_TRIGGER);
}

/* The state observable of a register: its own content for the configuration
 * registers, the counter for a refresh write, zero elsewhere. */
static uint32_t ff_test_wd_state_of(ff_test_wd_reg_t reg)
{
    switch (reg) {
    case FF_TEST_WD_REG_TIME:
        return g_regs.time;
    case FF_TEST_WD_REG_MSVP:
        return g_regs.msvp;
    case FF_TEST_WD_REG_TRIGGER:
        return g_regs.trigger;
    case FF_TEST_WD_REG_CONTROL:
        return g_regs.control;
    case FF_TEST_WD_REG_STATUS:
        return g_regs.status;
    case FF_TEST_WD_REG_REFRESH:
        return g_counter;
    case FF_TEST_WD_REG_FORCE:
    default:
        return g_regs.force;
    }
}

static uint32_t ff_test_wd_read_register(ff_test_wd_reg_t reg)
{
    switch (reg) {
    case FF_TEST_WD_REG_REFRESH:
        return g_regs.refresh;
    case FF_TEST_WD_REG_CONTROL:
        return g_regs.control;
    case FF_TEST_WD_REG_STATUS:
        return g_regs.status;
    case FF_TEST_WD_REG_TIME:
        return g_regs.time;
    case FF_TEST_WD_REG_MSVP:
        return g_regs.msvp;
    case FF_TEST_WD_REG_TRIGGER:
        return g_regs.trigger;
    case FF_TEST_WD_REG_FORCE:
    default:
        return g_regs.force;
    }
}

static void ff_test_wd_record(ff_test_wd_reg_t reg, uint32_t value,
                              ff_test_wd_disposition_t disposition, uint32_t before)
{
    if (g_write_count >= FF_TEST_WD_MAX_WRITES) {
        g_write_overflowed = true;
        return;
    }
    g_writes[g_write_count].reg = reg;
    g_writes[g_write_count].value = value;
    g_writes[g_write_count].disposition = disposition;
    g_writes[g_write_count].state_before = before;
    g_writes[g_write_count].state_after = ff_test_wd_state_of(reg);
    g_write_count++;
}

void ff_test_wd_model_reset(void)
{
    memset(&g_regs, 0, sizeof(g_regs));
    memset(g_writes, 0, sizeof(g_writes));
    g_counter = 0u;
    g_loaded = false;
    g_gated = false;
    g_window_enabled = false; /* F-5: the window is disabled by default. */
    g_locked = false;
    g_forbidden_status = false;
    g_timeout_interrupt = false;
    g_mvrp_interrupt = false;
    g_lock_rule = FF_TEST_WD_LOCK_AFTER_CFG_BLOCK;
    g_write_count = 0u;
    g_write_overflowed = false;
}

void ff_test_wd_model_set_lock_rule(ff_test_wd_lock_rule_t rule)
{
    g_lock_rule = rule;
}

void ff_test_wd_model_set_gated(bool gated)
{
    g_gated = gated;
}

void ff_test_wd_model_set_window_enabled(bool enabled)
{
    g_window_enabled = enabled;
}

void ff_test_wd_model_write(ff_test_wd_reg_t reg, uint32_t value)
{
    uint32_t before = ff_test_wd_state_of(reg);
    ff_test_wd_disposition_t disposition = FF_TEST_WD_DISP_APPLIED;

    /* Precedence: gated -> locked -> window -> key. */
    if (g_gated) {
        disposition = FF_TEST_WD_DISP_REFUSED_GATED;
    }
    else if (ff_test_wd_is_configuration_register(reg) && g_locked) {
        disposition = FF_TEST_WD_DISP_IGNORED_LOCKED;
    }
    else if (reg == FF_TEST_WD_REG_REFRESH) {
        if (g_window_enabled && (g_counter > g_regs.msvp)) {
            /* F-5: attempting a forbidden refresh asserts the timeout
             * interrupt and leaves the counter untouched. */
            g_forbidden_status = true;
            g_timeout_interrupt = true;
            disposition = FF_TEST_WD_DISP_REFUSED_WINDOW;
        }
        else if (value != FF_TEST_WD_REFRESH_KEY) {
            disposition = FF_TEST_WD_DISP_REFUSED_KEY;
        }
        else {
            g_counter = g_regs.time; /* F-4: reload from TIME. */
            g_loaded = true;
        }
    }
    else {
        switch (reg) {
        case FF_TEST_WD_REG_CONTROL:
            g_regs.control = value;
            break;
        case FF_TEST_WD_REG_STATUS:
            g_regs.status = value;
            break;
        case FF_TEST_WD_REG_TIME:
            g_regs.time = value;
            break;
        case FF_TEST_WD_REG_MSVP:
            g_regs.msvp = value;
            break;
        case FF_TEST_WD_REG_TRIGGER:
            g_regs.trigger = value;
            break;
        case FF_TEST_WD_REG_FORCE:
        default:
            g_regs.force = value;
            break;
        }
    }

    /* The lock engages after an applied write (F-2). */
    if (disposition == FF_TEST_WD_DISP_APPLIED) {
        if ((g_lock_rule == FF_TEST_WD_LOCK_AFTER_CFG_BLOCK) && (ff_test_wd_offset(reg) <= 0x0cu)) {
            g_locked = true;
        }
        else if ((g_lock_rule == FF_TEST_WD_LOCK_AFTER_TIME) && (reg == FF_TEST_WD_REG_TIME)) {
            g_locked = true;
        }
    }

    ff_test_wd_record(reg, value, disposition, before);
}

uint32_t ff_test_wd_model_read(ff_test_wd_reg_t reg)
{
    return ff_test_wd_read_register(reg);
}

void ff_test_wd_model_advance_ticks(uint32_t ticks)
{
    /* Model-only behaviour: the peripheral clock does not run while gated. */
    if (g_gated || !g_loaded) {
        return;
    }
    while ((ticks > 0u) && (g_counter > 0u)) {
        g_counter--;
        ticks--;
        if (!g_mvrp_interrupt && (g_counter <= g_regs.msvp)) {
            g_mvrp_interrupt = true; /* F-4: MVRP interrupt at the crossing. */
        }
        if (!g_timeout_interrupt && (g_counter <= g_regs.trigger)) {
            g_timeout_interrupt = true; /* F-4: timeout interrupt at the crossing. */
        }
    }
}

uint32_t ff_test_wd_model_counter(void)
{
    return g_counter;
}

bool ff_test_wd_model_clock_running(void)
{
    /* Model-only (C9): the clock is modelled as running only once the counter
     * has been loaded by an accepted keyed refresh. */
    return (!g_gated) && g_loaded;
}

bool ff_test_wd_model_locked(void)
{
    return g_locked;
}

bool ff_test_wd_model_forbidden_status(void)
{
    return g_forbidden_status;
}

bool ff_test_wd_model_timeout_interrupt(void)
{
    return g_timeout_interrupt;
}

bool ff_test_wd_model_mvrp_interrupt(void)
{
    return g_mvrp_interrupt;
}

size_t ff_test_wd_model_write_count(void)
{
    return g_write_count;
}

ff_test_wd_reg_t ff_test_wd_model_write_reg(size_t index)
{
    if (index >= g_write_count) {
        return FF_TEST_WD_REG_STATUS;
    }
    return g_writes[index].reg;
}

uint32_t ff_test_wd_model_write_value(size_t index)
{
    if (index >= g_write_count) {
        return 0u;
    }
    return g_writes[index].value;
}

ff_test_wd_disposition_t ff_test_wd_model_write_disposition(size_t index)
{
    if (index >= g_write_count) {
        return FF_TEST_WD_DISP_INVALID;
    }
    return g_writes[index].disposition;
}

uint32_t ff_test_wd_model_write_state_before(size_t index)
{
    if (index >= g_write_count) {
        return 0u;
    }
    return g_writes[index].state_before;
}

uint32_t ff_test_wd_model_write_state_after(size_t index)
{
    if (index >= g_write_count) {
        return 0u;
    }
    return g_writes[index].state_after;
}

bool ff_test_wd_model_write_overflowed(void)
{
    return g_write_overflowed;
}
