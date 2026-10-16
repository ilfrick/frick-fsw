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
#include <stdint.h>
#include <string.h>

/* Reference polling tick. Bounds the maximum release latency observed by the
 * dispatcher; the mission value is a controlled input (FRICK-ARCH-0018). */
#define FF_SCHED_TICK_MS 100u

/* Heartbeat period; HS watches app liveness independently (SSS-REQ-030). */
#define FF_SCHED_HEARTBEAT_MS 5000u

/* EVS event ids local to the scheduler application. */
#define FF_SCHED_EVT_ACTIVATED 0x0601u
#define FF_SCHED_EVT_INVALID_TABLE 0x0602u
#define FF_SCHED_EVT_MISSED_SLOT 0x0603u
#define FF_SCHED_EVT_HEARTBEAT 0x0604u
#define FF_SCHED_EVT_DISPATCH_FAIL 0x0605u

static FF_SchedTable_t g_sched_image;
static FF_TblHandle_t g_sched_tbl;

/* Project reference schedule (rates are illustrative configuration, not a
 * mission profile). */
static void ff_sched_entries_init(void)
{
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 3u;
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
    g_sched_image = t;
}

void FF_AppSchedulerMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    uint32_t reported_missed = 0u;
    uint32_t reported_publish_errors = 0u;
    uint32_t last_heartbeat_ms = 0u;

    ff_sched_entries_init();

    /* Table Services holds the validated image (registration validates through
     * FF_SchedValidate); activation is separate and atomic. */
    g_sched_tbl = FF_TblRegister(&(FF_Tbl_Def_t){.name = "sample_sched",
                                                 .image = &g_sched_image,
                                                 .size = sizeof(g_sched_image),
                                                 .validate = FF_SchedValidate});
    if (g_sched_tbl == FF_TBL_INVALID_HANDLE) {
        (void)FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_ERROR, app, "SCH: table registration failed");
    }

    if (FF_SchedActivate(&g_sched_image, sizeof(g_sched_image)) != FF_SUCCESS) {
        (void)FF_EvsSend(FF_SCHED_EVT_INVALID_TABLE, FF_EVS_ERROR, app,
                         "SCH: schedule table rejected, dispatch inhibited");
    }
    else {
        FF_SchedInit(FF_PalGetMonotonicNs());
        (void)FF_EvsSend(FF_SCHED_EVT_ACTIVATED, FF_EVS_INFO, app, "SCH app started, %u entries",
                         (unsigned)FF_SchedActiveCount());
    }

    for (;;) {
        FF_PalTaskDelayMs(FF_SCHED_TICK_MS);
        uint64_t now = FF_PalGetMonotonicNs();
        uint32_t elapsed_ms;

        /* Release due messages; the dispatcher publishes them and the app keeps
         * no large receive buffer on its 16 KiB task stack. */
        (void)FF_SchedPoll(now, NULL, (uint16_t)FF_SCHED_MAX_ENTRIES, app);

        /* Late/missed-slot evidence (SRS-REQ-071): report only new skips, so a
         * persistent overload cannot flood EVS at scheduler tick rate. */
        FF_SchedStats_t st;
        FF_SchedGetStats(&st);
        if (st.missed != reported_missed) {
            (void)FF_EvsSend(FF_SCHED_EVT_MISSED_SLOT, FF_EVS_WARNING, app,
                             "SCH: %u release slots skipped (total %u)",
                             (unsigned)(st.missed - reported_missed), (unsigned)st.missed);
            reported_missed = st.missed;
        }
        if (st.publish_errors != reported_publish_errors) {
            (void)FF_EvsSend(FF_SCHED_EVT_DISPATCH_FAIL, FF_EVS_ERROR, app,
                             "SCH: %u dispatch failures (total %u)",
                             (unsigned)(st.publish_errors - reported_publish_errors),
                             (unsigned)st.publish_errors);
            reported_publish_errors = st.publish_errors;
        }

        /* Heartbeat so Health & Safety can detect a stopped/hung scheduler. */
        elapsed_ms = (uint32_t)((now / 1000000ULL) - last_heartbeat_ms);
        if (elapsed_ms >= FF_SCHED_HEARTBEAT_MS) {
            last_heartbeat_ms = (uint32_t)(now / 1000000ULL);
            (void)FF_EvsSend(FF_SCHED_EVT_HEARTBEAT, FF_EVS_INFO, app, "SCH heartbeat");
        }
    }
}
