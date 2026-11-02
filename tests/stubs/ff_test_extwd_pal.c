/*
 * frick-fsw - XW-1 test-only PAL adapter: the production watchdog seam driven
 * by the abstract external-watchdog model
 *
 * TEST-ONLY. This file provides `__wrap_ff_pal_wd_hw_*`, selected at link time
 * with `-Wl,--wrap=` in the test binary only (the same technique the project
 * already uses for `nanosleep`/`write`). Nothing here is compiled into
 * `frick_fsw_core`, the flight image, or any target build, and the production
 * backend (src/core/pal/pal_posix.c, pal_rtems.c) is untouched.
 *
 * What it does: it is the *device side* of the abstract contract. The SUT keeps
 * calling the unchanged production `FF_PalWatchdogInit/Feed/Reset` functions,
 * which call the wrapped hooks; the hooks translate the call into the model and
 * translate the model's answer back into the project status type. The timeout
 * the SUT passes to `arm` becomes the model window, so the requirement's
 * "timeout from the mission configuration table" path is exercised without the
 * model ever knowing a mission value.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "core/common/ff_types.h"
#include "ff_test_extwd_model.h"

/* The production seam declarations (pal_watchdog.c defines these; the wrapper
 * replaces them at link time). */
FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms);
FF_Status_t ff_pal_wd_hw_feed(void);
FF_Status_t ff_pal_wd_hw_reset(void);

FF_Status_t __wrap_ff_pal_wd_hw_arm(uint32_t timeout_ms)
{
    ff_extwd_set_window_ms(timeout_ms);
    switch (ff_extwd_arm()) {
    case FF_EXTWD_OK:
        return FF_SUCCESS;
    case FF_EXTWD_E_ARM:
        return FF_ERROR; /* reported to the SUT: arm failure is never hidden */
    default:
        return FF_ERROR;
    }
}

FF_Status_t __wrap_ff_pal_wd_hw_feed(void)
{
    switch (ff_extwd_feed()) {
    case FF_EXTWD_OK:
        return FF_SUCCESS;
    case FF_EXTWD_E_FEED:
        return FF_ERROR;
    case FF_EXTWD_E_STATE:
        return FF_NOT_READY;
    default:
        return FF_ERROR;
    }
}

FF_Status_t __wrap_ff_pal_wd_hw_reset(void)
{
    switch (ff_extwd_request_reset()) {
    case FF_EXTWD_OK:
        return FF_SUCCESS;
    case FF_EXTWD_E_RESET:
        return FF_ALREADY_EXISTS; /* the device already asserted the reset */
    default:
        return FF_ERROR;
    }
}
