/*
 * frick-fsw - Health and Safety (HS) application
 *
 * Monitors application and framework health, logs anomalies and provides a
 * recovery action (on-board software reset in flight). Analogue of the cFS
 * Health and Safety application. In the reference build the reset action is a
 * controlled FF_PalPanic so the watchdog (RTEMS/monitor or system controller)
 * can step in.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/es/es.h"
#include "core/sb/sb.h"
#include "pus/pus.h"

/* Receives events published by EVS through the listener. */
#define FF_HS_MSG_EVENT 0x0501u

static void ff_hs_reset(void)
{
    /* Ground requested software reset -> single watchdog reset path. */
    FF_PalPanic("HS: on-board software reset requested");
}

/* Exposed as PUS service 8 function #1. */
static FF_Status_t ff_hs_fn_reset(const uint8_t *args, uint16_t len)
{
    (void)args;
    (void)len;
    ff_hs_reset();
    return FF_SUCCESS; /* not reached */
}

/* Registration seam: keeps the flight registration identical and lets the host
 * integration test drive the real service 8 function 1 (N22-TS5-50). */
void FF_AppHsRegister(void)
{
    (void)FF_PusRegisterFunction(1u, ff_hs_fn_reset);
}

/* Reset-action seam used by the PUS service 8 function and by the host test. */
void FF_AppHsReset(void)
{
    ff_hs_reset();
}

/* One monitoring pass: report every FAILED application, then raise the
 * periodic heartbeat. Extracted so the health-check behaviour can be exercised
 * by a deterministic host integration test without entering the blocking
 * 5000 ms cadence loop (S8-VV-FU; GAP-R3-1). The behaviour is identical to the
 * loop body of FF_AppHsMain. */
void FF_AppHsStep(FF_AppId_t app)
{
    /* Watchdog style check: every app must still be RUNNING. */
    for (FF_AppId_t id = 1u; id <= (FF_AppId_t)FF_EsGetAppCount(); id++) {
        if (FF_EsGetState(id) == FF_APP_FAILED) {
            (void)FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, app, "HS: app %u failed",
                             (unsigned)id);
        }
    }

    /* Periodic heartbeat EVS event (also visible to ground via PUS 5). */
    (void)FF_EvsSend(0x0502u, FF_EVS_INFO, app, "HS heartbeat");
}

void FF_AppHsMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    FF_AppHsRegister();

    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "HS app monitoring");

    for (;;) {
        FF_AppHsStep(app);
        FF_PalTaskDelayMs(5000u);
    }
}
