/*
 * frick-fsw - External watchdog TEST model: abstract device contract
 *
 * TEST DOUBLE - NOT A DEVICE, NOT A BINDING, NOT HIL EVIDENCE. This model
 * implements only the *abstract* service/deadline/reset contract of an external
 * independent watchdog (XW-1), as documented in
 * docs/vv/external-watchdog-test-contract.md. It contains no device protocol,
 * no register map, no clock assumption and no mission value. It is never linked
 * into production code and cannot reach a flight or target build.
 *
 * Two properties make it usable as an independent oracle:
 *   * the virtual clock is advanced ONLY by the harness (ff_extwd_advance_ms);
 *     the model never reads the middleware time source, so expiry cannot be
 *     caused by the software under test;
 *   * every observable (state, expiry count, reset count, feed count) is a
 *     function of the service calls and the harness clock alone, so an authored
 *     expected trace can be compared exactly.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_TEST_EXTWD_MODEL_H
#define FF_TEST_EXTWD_MODEL_H

#include <stdbool.h>
#include <stdint.h>

/* Abstract device states. Order is significant: the device only moves forward
 * (DISARMED -> ARMED -> EXPIRED -> RESET_ASSERTED); no transition goes back. */
typedef enum {
    FF_EXTWD_DISARMED = 0,
    FF_EXTWD_ARMED = 1,
    FF_EXTWD_EXPIRED = 2,
    FF_EXTWD_RESET_ASSERTED = 3
} ff_extwd_state_t;

/* Configurable device-side faults (test parameters, not device facts). */
typedef enum {
    FF_EXTWD_FAULT_NONE = 0,
    FF_EXTWD_FAULT_ARM_FAIL = 1,        /* arm refuses, device stays DISARMED */
    FF_EXTWD_FAULT_FEED_FAIL = 2,       /* feed reports an error, no reload */
    FF_EXTWD_FAULT_STUCK_FEED = 3,      /* feed reports success but does not reload */
    FF_EXTWD_FAULT_PREMATURE_RESET = 4, /* device asserts reset before the deadline */
    FF_EXTWD_FAULT_CLOCK_JUMP = 5       /* one advance step larger than a window is
                                         * recorded as a jump (ff_extwd_jumps) */
} ff_extwd_fault_t;

/* Return codes of the device-side operations (int, not the project status type,
 * so the model depends on nothing but fixed-width types). */
#define FF_EXTWD_OK 0
#define FF_EXTWD_E_ARM 1   /* arm refused (fault) */
#define FF_EXTWD_E_FEED 2  /* feed refused (fault) */
#define FF_EXTWD_E_STATE 3 /* operation not valid in this state */
#define FF_EXTWD_E_RESET 4 /* reset already asserted */

/* --- harness control --------------------------------------------------- */

/* Returns the model to DISARMED at t=0 with the default test window. */
void ff_extwd_reset(void);

/* Test window (ms). TEST PARAMETER, default FF_EXTWD_DEFAULT_WINDOW_MS; this is
 * not a mission timeout and must never be reported as one. */
#define FF_EXTWD_DEFAULT_WINDOW_MS 100u
void ff_extwd_set_window_ms(uint32_t window_ms);
uint32_t ff_extwd_window_ms(void);

/* Fault injection. */
void ff_extwd_set_fault(ff_extwd_fault_t fault);
ff_extwd_fault_t ff_extwd_fault(void);

/* Harness-advanced virtual clock: the ONLY way time moves. A step that crosses
 * the deadline expires the device exactly once, at the deadline (never twice,
 * never retroactively). */
void ff_extwd_advance_ms(uint32_t delta_ms);
uint32_t ff_extwd_now_ms(void);
uint32_t ff_extwd_deadline_ms(void);

/* --- device-side operations (what the SUT does through its adapter) ---- */

int ff_extwd_arm(void);
int ff_extwd_feed(void);
/* The SUT's explicit reset request (SRS-REQ-075 single path). Asserts the reset
 * at most once, from any state. */
int ff_extwd_request_reset(void);

/* --- observables ------------------------------------------------------- */

ff_extwd_state_t ff_extwd_state(void);
uint32_t ff_extwd_expiries(void);
uint32_t ff_extwd_resets(void);
uint32_t ff_extwd_feeds(void);
/* Number of advance steps taken while FF_EXTWD_FAULT_CLOCK_JUMP was selected and
 * the step was larger than the window. Observable so the fault mode is proven
 * by a test rather than being a label. */
uint32_t ff_extwd_jumps(void);
/* True when an observable counter saturated instead of wrapping (the model
 * never drops evidence silently). */
bool ff_extwd_overflows(void);

#endif /* FF_TEST_EXTWD_MODEL_H */
