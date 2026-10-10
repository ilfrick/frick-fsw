/*
 * frick-fsw - Housekeeping (HK) application
 *
 * Publishes housekeeping parameters and registers PUS service 3 / 20 data
 * providers. Analogue of the cFS Housekeeping application.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/es/es.h"
#include "core/sb/sb.h"
#include "core/time/time.h"
#include "pus/pus.h"
#include <string.h>

#define FF_HK_STRUCT_ID 1u

/* Message identifiers published by the housekeeping app. */
#define FF_HK_MSG_ID 0x0301u

static bool ff_hk_build(uint8_t *out, uint16_t *len)
{
    uint32_t uptime_ms = FF_TimeGetMetMs();
    uint32_t tc_count  = FF_PusGetTcCount();
    uint16_t tm_count  = FF_PusPeekTmCounter();
    uint8_t  apps      = (uint8_t)FF_EsGetAppCount();
    uint32_t events    = FF_EvsGetTotalCount();

    if (*len < 15u) { return false; }
    out[0] = (uint8_t)(uptime_ms >> 24); out[1] = (uint8_t)(uptime_ms >> 16);
    out[2] = (uint8_t)(uptime_ms >> 8);  out[3] = (uint8_t)uptime_ms;
    out[4] = (uint8_t)(tc_count >> 24);  out[5] = (uint8_t)(tc_count >> 16);
    out[6] = (uint8_t)(tc_count >> 8);   out[7] = (uint8_t)tc_count;
    out[8] = (uint8_t)(tm_count >> 8);   out[9] = (uint8_t)tm_count;
    out[10] = apps;
    out[11] = (uint8_t)(events >> 24);   out[12] = (uint8_t)(events >> 16);
    out[13] = (uint8_t)(events >> 8);    out[14] = (uint8_t)events;
    *len = 15u;
    return true;
}

static FF_Status_t ff_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    return ff_hk_build(out, len) ? FF_SUCCESS : FF_BAD_SIZE;
}

FF_Status_t FF_AppHkRegister(void)
{
    /* PUS service 3 : housekeeping report for structure 1. */
    if (FF_PusRegisterHkProvider(FF_HK_STRUCT_ID, ff_hk_provider) != FF_SUCCESS)
    {
        return FF_ERROR;
    }
    if (FF_PusRegisterParamProvider(FF_HK_STRUCT_ID, ff_hk_provider) != FF_SUCCESS)
    {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

void FF_AppHkMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    FF_Msg_t   msg;

    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "HK app running");

    for (;;)
    {
        uint8_t  payload[16];
        uint16_t len = sizeof(payload);
        (void)ff_hk_build(payload, &len);
        memset(&msg, 0, sizeof(msg));
        msg.msg_id  = FF_HK_MSG_ID;
        msg.apid    = app;
        msg.time_ns = FF_PalGetMonotonicNs();
        msg.length  = len;
        memcpy(msg.payload, payload, len);
        (void)FF_SbPublish(&msg);

        FF_PalTaskDelayMs(1000u);
    }
}
