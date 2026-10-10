/*
 * frick-fsw - Scheduler (SCH) application
 *
 * Time driven, table-driven dispatch of periodic Software Bus messages.
 * Analogue of the cFS Scheduler lab application, but table based (the table
 * lives in config/tables/sample_schedule).
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/sb/sb.h"
#include "core/tbl/tbl.h"
#include <stdint.h>
#include <string.h>

typedef struct
{
    uint16_t msg_id;
    uint16_t period_ms;
    uint16_t payload_len;
    uint8_t  payload[16];
} ff_sched_entry_t;

typedef struct
{
    uint32_t         magic;
    uint16_t         count;
    ff_sched_entry_t entries[8];
} ff_sched_table_t;

#define FF_SCHED_MAGIC 0x46534831u /* 'FSH1' */

static ff_sched_table_t g_sched_image;
static FF_TblHandle_t   g_sched_tbl;
static uint64_t         g_last_run;

static FF_Status_t ff_sched_validate(const void *image, size_t size)
{
    const ff_sched_table_t *t = (const ff_sched_table_t *)image;
    if ((size != sizeof(*t)) || (t->magic != FF_SCHED_MAGIC) || (t->count > 8u))
    {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

static void ff_sched_entries_init(void)
{
    ff_sched_table_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 3u;
    t.entries[0].msg_id = 0x0A01u; t.entries[0].period_ms = 1000u; t.entries[0].payload_len = 2u;
    t.entries[0].payload[0] = 0x00u; t.entries[0].payload[1] = 0x01u; /* sample: recover AOCS */
    t.entries[1].msg_id = 0x0A02u; t.entries[1].period_ms = 5000u; t.entries[1].payload_len = 1u;
    t.entries[1].payload[0] = 0x42u;
    t.entries[2].msg_id = 0x0A03u; t.entries[2].period_ms = 10000u; t.entries[2].payload_len = 4u;
    t.entries[2].payload[0] = 0x01u; t.entries[2].payload[1] = 0x02u;
    t.entries[2].payload[2] = 0x03u; t.entries[2].payload[3] = 0x04u;
    g_sched_image = t;
}

void FF_AppSchedulerMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;

    ff_sched_entries_init();
    g_sched_tbl = FF_TblRegister(&(FF_Tbl_Def_t){
        .name = "sample_sched", .image = &g_sched_image, .size = sizeof(g_sched_image),
        .validate = ff_sched_validate });
    g_last_run = FF_PalGetMonotonicNs();

    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "SCH app started, %u entries",
                     (unsigned)g_sched_image.count);

    for (;;)
    {
        FF_PalTaskDelayMs(100u);
        uint64_t now = FF_PalGetMonotonicNs();

        for (uint16_t i = 0; i < g_sched_image.count; i++)
        {
            ff_sched_entry_t *e = &g_sched_image.entries[i];
            uint64_t elapsed = (now - g_last_run) / 1000000ULL;
            if (elapsed >= e->period_ms)
            {
                FF_Msg_t msg;
                memset(&msg, 0, sizeof(msg));
                msg.msg_id  = e->msg_id;
                msg.apid    = app;
                msg.time_ns = now;
                msg.length  = e->payload_len;
                memcpy(msg.payload, e->payload, e->payload_len);
                (void)FF_SbPublish(&msg);
                g_last_run = now;
            }
        }
    }
}