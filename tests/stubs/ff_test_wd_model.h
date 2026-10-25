/*
 * frick-fsw - Host watchdog model fixture: peripheral register semantics
 *
 * TEST DOUBLE - NOT A BINDING. This file is a host-side fixture used only by
 * tests/unit/test_wd_host_contract.c. It performs no MMIO, no register write,
 * no device access and contains no binding logic. It is not a port of any
 * vendor driver and not a model of the flight device. It is never linked into
 * frick_fsw_core and cannot reach a flight or target build. The MPFS/MSS
 * binding is NOT WIRED (NC-PAL-01).
 *
 * WHAT THIS FIXTURE IS: an explicit state machine that implements the
 * peripheral-side register semantics recorded in FRICK-ARCH-0024 rev 1.5
 * (F-1..F-8) so that the *model* behaviour can be pinned by tests. It holds a
 * register file, a down-counter, a write observer and the flags a peripheral
 * would expose. It contains no sequencing, no policy and no software logic:
 * the caller issues the writes, this fixture decides their effect.
 *
 * WHAT THIS FIXTURE IS NOT: a source of device facts. Every value it holds is
 * a synthetic test parameter. The model semantics are classified in
 * docs/vv/host-wd-model-contract-addendum-2026-10-22.md: documented-and-cited,
 * unresolved-and-parameterised, or model-only. No lock rule, window-enable
 * mechanism, gating value, clock, prescaler, timeout, cadence, margin,
 * instance or reset scope is a device truth here.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_TEST_WD_MODEL_H
#define FF_TEST_WD_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The published refresh-key constant of the cited public example driver
 * (FRICK-ARCH-0024 rev 1.5, F-3/F-7). The flight device's key is not
 * established (M-1): this is a documented constant, not a mission value. */
#define FF_TEST_WD_REFRESH_KEY 0xDEADC0DEu

/* Register identifiers. The names and the 0x00..0x18 offsets are those of the
 * cited public example driver (F-1/F-2); their applicability to the flight
 * device is not established (M-1). */
typedef enum {
    FF_TEST_WD_REG_REFRESH = 0,
    FF_TEST_WD_REG_CONTROL,
    FF_TEST_WD_REG_STATUS,
    FF_TEST_WD_REG_TIME,
    FF_TEST_WD_REG_MSVP,
    FF_TEST_WD_REG_TRIGGER,
    FF_TEST_WD_REG_FORCE
} ff_test_wd_reg_t;

/* Lock rule variants. The upstream documentation contradicts itself (F-2,
 * M-11): one reading locks the configuration registers after any write to the
 * offset range 0x00..0x0c, the other after a write into TIME. Both are
 * implemented; NEITHER is device truth. */
typedef enum {
    FF_TEST_WD_LOCK_AFTER_CFG_BLOCK = 0,
    FF_TEST_WD_LOCK_AFTER_TIME = 1
} ff_test_wd_lock_rule_t;

/* Per-write disposition recorded by the write observer. Precedence is
 * gated -> locked -> window -> key (documented in the addendum). */
typedef enum {
    FF_TEST_WD_DISP_APPLIED = 0,
    FF_TEST_WD_DISP_IGNORED_LOCKED,
    FF_TEST_WD_DISP_REFUSED_GATED,
    FF_TEST_WD_DISP_REFUSED_WINDOW,
    FF_TEST_WD_DISP_REFUSED_KEY,
    /* Out-of-range accessor result: a query for a write that does not exist
     * must never report the benign "applied" token. */
    FF_TEST_WD_DISP_INVALID
} ff_test_wd_disposition_t;

/* --- state ------------------------------------------------------------- */

void ff_test_wd_model_reset(void);
void ff_test_wd_model_set_lock_rule(ff_test_wd_lock_rule_t rule);
void ff_test_wd_model_set_gated(bool gated);
void ff_test_wd_model_set_window_enabled(bool enabled);

/* Register access. Writes carry the peripheral semantics; reads return the
 * current register content. */
void ff_test_wd_model_write(ff_test_wd_reg_t reg, uint32_t value);
uint32_t ff_test_wd_model_read(ff_test_wd_reg_t reg);

/* Advances the peripheral clock by the given number of synthetic ticks. The
 * tick domain has NO defined relation to any device timebase and to the PAL's
 * millisecond domain. */
void ff_test_wd_model_advance_ticks(uint32_t ticks);

/* --- independent observables ------------------------------------------- */

uint32_t ff_test_wd_model_counter(void);
bool ff_test_wd_model_clock_running(void);
bool ff_test_wd_model_locked(void);
bool ff_test_wd_model_forbidden_status(void);
bool ff_test_wd_model_timeout_interrupt(void);
bool ff_test_wd_model_mvrp_interrupt(void);
/* True when the write observer dropped a record (its capacity was exceeded).
 * The fixture never drops evidence silently: the flag is asserted clear by
 * the cases. */
bool ff_test_wd_model_write_overflowed(void);

/* --- write observer ---------------------------------------------------- */

size_t ff_test_wd_model_write_count(void);
ff_test_wd_reg_t ff_test_wd_model_write_reg(size_t index);
uint32_t ff_test_wd_model_write_value(size_t index);
ff_test_wd_disposition_t ff_test_wd_model_write_disposition(size_t index);
/* State observed before and after the recorded write: the register's own value
 * for the configuration registers, the counter for a refresh write. */
uint32_t ff_test_wd_model_write_state_before(size_t index);
uint32_t ff_test_wd_model_write_state_after(size_t index);

#endif /* FF_TEST_WD_MODEL_H */
