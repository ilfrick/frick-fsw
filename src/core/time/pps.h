/*
 * frick-fsw - PPS / 1 Hz tone backend abstraction
 *
 * The hardware PPS capture is a low-level driver (see
 * docs/engineering/driver-development.md). A backend selects the tone source;
 * the driver calls FF_TimeToneSignal() (time.h) from a task (the capture ISR
 * must defer to a task: the tone hook is task context only).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_PPS_H
#define FF_PPS_H

#include "common/ff_types.h"
#include "fsw_config.h"

typedef FF_Status_t (*FF_PpsInitFn_t)(void);
typedef FF_Status_t (*FF_PpsStartFn_t)(void);
typedef FF_Status_t (*FF_PpsStopFn_t)(void);

typedef struct FF_PpsBackend
{
    const char *name;
    FF_PpsInitFn_t init;
    FF_PpsStartFn_t start;
    FF_PpsStopFn_t stop;
} FF_PpsBackend_t;

/* Select the active PPS backend (one registration; the board integrator
 * provides the hardware backend). */
FF_Status_t FF_PpsRegister(FF_PpsBackend_t *backend);
FF_Status_t FF_PpsStart(void);
FF_Status_t FF_PpsStop(void);

/* Reference software backend: a PAL task emitting one tone per second.
 * Suitable for the host simulator and for tests. */
extern FF_PpsBackend_t FF_PpsSwBackend;

#endif /* FF_PPS_H */
