/*
 * frick-fsw - Scheduler (SCH) application
 *
 * Table-driven dispatch of periodic Software Bus messages through the bounded
 * application schedule dispatcher (core/sched/ff_sched.c, M17). Analogue of the
 * cFS Scheduler application, but table based: the reference table lives in
 * config/tables/sample_schedule and is only a project reference profile, not a
 * mission schedule.
 *
 * RTEMS owns runnable-task selection and priority preemption; this application
 * only decides when configured SB messages are released (SSS-REQ-030). Mission
 * rates, jitter tolerance, late-slot policy and recovery authority remain
 * mission-controlled inputs (FRICK-ARCH-0018).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/sched/ff_sched.h"
#include "core/tbl/tbl.h"
#include "sched_evt.h"
#include <stdint.h>
#include <string.h>

static FF_SchedTable_t g_sched_image;
static FF_TblHandle_t g_sched_tbl = FF_TBL_INVALID_HANDLE;

/* Table Services activation runs in command/TBL context, while the dispatcher
 * state (core/sched/ff_sched.c) is single-writer and owned by this task. The
 * activation callback therefore only snapshots the newly activated image into
 * this mutex-protected slot; the scheduler task applies it at the top of its
 * next tick (N22-TS5-66). The callback is bounded, performs no I/O and never
 * calls back into TBL services. */
static FF_SchedTable_t g_sched_pending_image;
static bool g_sched_reconfig_pending;
static FF_MutexId_t g_sched_reconfig_lock;
static bool g_sched_reconfig_ready;
static FF_AppId_t g_sched_app_id;

/* Project reference schedule (rates are illustrative configuration, not a
 * mission profile). */
static void ff_sched_entries_init(void)
{
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 4u;
    t.entries[0].msg_id = 0x0A01u;
    t.entries[0].period_ms = 1000u;
    t.entries[0].payload_len = 2u;
    t.entries[0].payload[0] = 0x00u;
    t.entries[0].payload[1] = 0x01u; /* sample: recover AOCS */
    t.entries[1].msg_id = 0x0A02u;
    t.entries[1].period_ms = 5000u;
    t.entries[1].payload_len = 1u;
    t.entries[1].payload[0] = 0x42u;
    t.entries[2].msg_id = 0x0A03u;
    t.entries[2].period_ms = 10000u;
    t.entries[2].payload_len = 4u;
    t.entries[2].payload[0] = 0x01u;
    t.entries[2].payload[1] = 0x02u;
    t.entries[2].payload[2] = 0x03u;
    t.entries[2].payload[3] = 0x04u;
    t.entries[3].msg_id = 0x0A04u;
    t.entries[3].period_ms = 3000u;
    t.entries[3].payload_len = 1u;
    t.entries[3].payload[0] = 0x99u;
    g_sched_image = t;
}

#ifdef FF_TEST_HOOKS
/* Test-only exit flag for FF_AppSchedulerMain. When set, the main loop will
 * exit after the current iteration. This allows deterministic testing of the
 * scheduler application event emission paths. */
volatile bool g_sched_test_exit_request = false;

void FF_SchedTestRequestExit(void)
{
    g_sched_test_exit_request = true;
}

void FF_SchedTestResetExit(void)
{
    g_sched_test_exit_request = false;
}

FF_TblHandle_t FF_SchedTestTableHandle(void)
{
    return g_sched_tbl;
}
#endif

/* TBL activation callback: bounded snapshot only (see the pending-slot comment
 * above). Rejects any activation that does not target this application's
 * registered table, so a future co-located table cannot silently reconfigure
 * dispatch. */
static void ff_sched_tbl_activated(FF_TblHandle_t handle, const void *image, size_t size)
{
    if (!g_sched_reconfig_ready) {
        (void)FF_EvsSend(FF_SCHED_EVT_RECFG_NOT_READY, FF_EVS_WARNING, g_sched_app_id,
                         FF_SCHED_EVT_RECFG_NOT_READY_FMT);
        return;
    }
    if ((handle != g_sched_tbl) || (size != sizeof(g_sched_image))) {
        return;
    }
    /* TBL validated the staged image before activation; re-validate the active
     * copy so a corrupt buffer cannot reach the dispatcher (defence in depth). */
    if (FF_SchedValidate(image, size) != FF_SUCCESS) {
        return;
    }
    /* Bounded wait (5 ms) to avoid priority inversion in command context.
     * The scheduler task uses a non-blocking try-lock and holds the lock only
     * for the memcpy, so a short timeout is sufficient. */
    if (FF_PalMutexLock(g_sched_reconfig_lock, 5u) != FF_SUCCESS) {
        (void)FF_EvsSend(FF_SCHED_EVT_RECFG_NOT_READY, FF_EVS_WARNING, g_sched_app_id,
                         FF_SCHED_EVT_RECFG_NOT_READY_FMT);
        return;
    }
    memcpy(&g_sched_pending_image, image, sizeof(g_sched_pending_image));
    g_sched_reconfig_pending = true;
    (void)FF_PalMutexUnlock(g_sched_reconfig_lock);
}

/* Apply a pending TBL activation in dispatcher (single-writer) context. The
 * non-blocking lock keeps a scheduler tick from ever waiting on command
 * context: if the slot is busy, the activation is applied on the next tick. */
static void ff_sched_apply_pending(uint64_t now_ns, FF_AppId_t app)
{
    FF_SchedTable_t pending;
    bool have_pending = false;

    if (!g_sched_reconfig_ready) {
        return;
    }
    if (FF_PalMutexTryLock(g_sched_reconfig_lock) != FF_SUCCESS) {
        return;
    }
    if (g_sched_reconfig_pending) {
        pending = g_sched_pending_image;
        g_sched_reconfig_pending = false;
        have_pending = true;
    }
    (void)FF_PalMutexUnlock(g_sched_reconfig_lock);

    if (!have_pending) {
        return;
    }
    if (FF_SchedActivate(&pending, sizeof(pending)) != FF_SUCCESS) {
        (void)FF_EvsSend(FF_SCHED_EVT_INVALID_TABLE, FF_EVS_ERROR, app,
                         FF_SCHED_EVT_INVALID_TABLE_FMT);
        return;
    }
    FF_SchedInit(now_ns);
    (void)FF_EvsSend(FF_SCHED_EVT_ACTIVATED, FF_EVS_INFO, app, FF_SCHED_EVT_REACTIVATED_FMT,
                     (unsigned)FF_SchedActiveCount());
}

void FF_AppSchedulerMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    volatile uint32_t reported_missed = 0u;
    volatile uint32_t reported_publish_errors = 0u;
    uint32_t last_heartbeat_ms = 0u;

    ff_sched_entries_init();

    /* Store app ID for callback use (callback runs in TBL/command context). */
    g_sched_app_id = app;

    /* Reconfiguration slot must exist before the table is registered, because
     * registration publishes the activation callback. */
    g_sched_reconfig_pending = false;
    g_sched_reconfig_ready =
        (FF_PalMutexCreate(&g_sched_reconfig_lock, "sched-recfg") == FF_SUCCESS);
    if (!g_sched_reconfig_ready) {
        (void)FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_ERROR, app,
                         "SCH: reconfiguration lock unavailable, table activation not applied");
    }

    /* Table Services holds the validated image (registration validates through
     * FF_SchedValidate); activation is separate and atomic. A later activation
     * is applied by this task through ff_sched_tbl_activated/ff_sched_apply_pending. */
    g_sched_tbl = FF_TblRegister(&(FF_Tbl_Def_t){.name = "sample_sched",
                                                 .image = &g_sched_image,
                                                 .size = sizeof(g_sched_image),
                                                 .validate = FF_SchedValidate,
                                                 .on_activate = ff_sched_tbl_activated});
    if (g_sched_tbl == FF_TBL_INVALID_HANDLE) {
        (void)FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_ERROR, app, "SCH: table registration failed");
    }

    if (FF_SchedActivate(&g_sched_image, sizeof(g_sched_image)) != FF_SUCCESS) {
        (void)FF_EvsSend(FF_SCHED_EVT_INVALID_TABLE, FF_EVS_ERROR, app,
                         FF_SCHED_EVT_INVALID_TABLE_FMT);
    }
    else {
        FF_SchedInit(FF_PalGetMonotonicNs());
        (void)FF_EvsSend(FF_SCHED_EVT_ACTIVATED, FF_EVS_INFO, app, FF_SCHED_EVT_ACTIVATED_FMT,
                         (unsigned)FF_SchedActiveCount());
    }

    for (;;) {
#ifdef FF_TEST_HOOKS
        FF_PalTaskDelayMsTest(FF_SCHED_TICK_MS);
#else
        FF_PalTaskDelayMs(FF_SCHED_TICK_MS);
#endif
        uint64_t now = FF_PalGetMonotonicNs();
        uint32_t elapsed_ms;

        /* Apply a table activation requested since the last tick before any
         * release decision, so the new table takes effect atomically. */
        ff_sched_apply_pending(now, app);

        /* Release due messages; the dispatcher publishes them and the app keeps
         * no large receive buffer on its 16 KiB task stack. */
        (void)FF_SchedPoll(now, NULL, (uint16_t)FF_SCHED_MAX_ENTRIES, app);

        /* Late/missed-slot evidence (SRS-REQ-071): report only new skips, so a
         * persistent overload cannot flood EVS at scheduler tick rate. */
        FF_SchedStats_t st;
        FF_SchedGetStats(&st);
        if (st.missed != reported_missed) {
            (void)FF_EvsSend(FF_SCHED_EVT_MISSED_SLOT, FF_EVS_WARNING, app,
                             FF_SCHED_EVT_MISSED_SLOT_FMT, (unsigned)(st.missed - reported_missed),
                             (unsigned)st.missed);
            reported_missed = st.missed;
        }
        if (st.publish_errors != reported_publish_errors) {
            (void)FF_EvsSend(FF_SCHED_EVT_DISPATCH_FAIL, FF_EVS_ERROR, app,
                             FF_SCHED_EVT_DISPATCH_FAIL_FMT,
                             (unsigned)(st.publish_errors - reported_publish_errors),
                             (unsigned)st.publish_errors);
            reported_publish_errors = st.publish_errors;
        }

        /* Heartbeat so Health & Safety can detect a stopped/hung scheduler. */
        elapsed_ms = (uint32_t)((now / 1000000ULL) - last_heartbeat_ms);
        if (elapsed_ms >= FF_SCHED_HEARTBEAT_MS) {
            last_heartbeat_ms = (uint32_t)(now / 1000000ULL);
            (void)FF_EvsSend(FF_SCHED_EVT_HEARTBEAT, FF_EVS_INFO, app, FF_SCHED_EVT_HEARTBEAT_FMT);
        }

#ifdef FF_TEST_HOOKS
        if (g_sched_test_exit_request) {
            break;
        }
#endif
    }
}
