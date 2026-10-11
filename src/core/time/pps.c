/*
 * frick-fsw - PPS / 1 Hz tone backends implementation
 *
 * Single registration slot for a PPS backend, plus the reference software
 * backend used by the host simulator and by tests. A hardware backend
 * (RTEMS GPIO / PLIC capture on the MPFS) is wired by the board integrator by
 * calling FF_PpsRegister() from FF_BoardInit(); the capture ISR must defer to a
 * task that calls FF_TimeToneSignal() (task context only: the time state is
 * mutex-protected and mutexes are not ISR-safe).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "time/pps.h"
#include "time/time.h"
#include "core/pal/pal.h"

static FF_PpsBackend_t *g_pps;
static bool g_sw_running;
static FF_TaskId_t g_sw_task_id;

/* ---- Software tone task ---------------------------------------------- */

static void ff_pps_sw_task(void *arg)
{
    (void)arg;
    while (g_sw_running) {
        FF_PalTaskDelayMs(FF_TIME_SW_TONE_PERIOD_MS);
        if (g_sw_running) {
            (void)FF_TimeToneSignal(FF_PalGetMonotonicNs());
        }
    }
    FF_PalTaskExit();
}

static FF_Status_t ff_pps_sw_init(void)
{
    g_sw_running = false;
    return FF_SUCCESS;
}

static FF_Status_t ff_pps_sw_start(void)
{
    FF_Status_t st;

    if (g_sw_running) {
        return FF_SUCCESS;
    }
    /* The task gate must be visible to the spawned thread immediately, but on
     * create failure the backend must be reset to a fully stopped state
     * (R1 NC-04). */
    g_sw_running = true;
    st = FF_PalTaskCreate(&g_sw_task_id, "pps_sw", ff_pps_sw_task, NULL, 2048u,
                          FF_TIME_TONE_TASK_PRIORITY);
    if (st != FF_SUCCESS) {
        g_sw_running = false;
        g_sw_task_id = FF_INVALID_ID;
    }
    return st;
}

static FF_Status_t ff_pps_sw_stop(void)
{
    if (!g_sw_running) {
        return FF_SUCCESS;
    }
    g_sw_running = false;
    if (g_sw_task_id == FF_INVALID_ID) {
        return FF_INVALID_ARGUMENT;
    }
    return FF_PalTaskDelete(g_sw_task_id);
}

FF_PpsBackend_t FF_PpsSwBackend = {"sw", ff_pps_sw_init, ff_pps_sw_start, ff_pps_sw_stop};

/* ---- Registration ------------------------------------------------------- */

FF_Status_t FF_PpsRegister(FF_PpsBackend_t *backend)
{
    FF_Status_t st;

    if ((backend == NULL) || (backend->name == NULL) || (backend->init == NULL) ||
        (backend->start == NULL) || (backend->stop == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_pps != NULL) {
        return FF_ALREADY_EXISTS;
    }
    g_pps = backend;
    st = backend->init();
    if (st != FF_SUCCESS) {
        g_pps = NULL;
    }
    return st;
}

FF_Status_t FF_PpsStart(void)
{
    if (g_pps == NULL) {
        return FF_NOT_READY;
    }
    return g_pps->start();
}

FF_Status_t FF_PpsStop(void)
{
    if (g_pps == NULL) {
        return FF_NOT_READY;
    }
    return g_pps->stop();
}
