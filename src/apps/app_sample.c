/*
 * frick-fsw - Sample application
 *
 * Demonstrates the publish/subscribe development model: it subscribes to the
 * scheduler messages and to the housekeeping message, logs them and echoes
 * back a counter on the bus.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/sb/sb.h"
#include <string.h>

#define FF_SAMPLE_IN_MSG 0x0A01u
#define FF_SAMPLE_HK_MSG 0x0301u
#define FF_SAMPLE_ECHO_MSG 0x0B01u

static uint32_t g_echo_counter;

void FF_AppSampleMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    FF_Msg_t msg;

    (void)FF_SbSubscribe(FF_SAMPLE_IN_MSG, app);
    (void)FF_SbSubscribe(FF_SAMPLE_HK_MSG, app);

    FF_PalLog("SAMPLE app started\n");
    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "SAMPLE app ready");

    for (;;) {
        if (FF_SbReceive(app, &msg, 1000u) != FF_SUCCESS) {
            continue;
        }
        g_echo_counter++;
        (void)FF_EvsSend((uint16_t)(0x0B00u + (uint16_t)(g_echo_counter & 0xFFu)), FF_EVS_INFO, app,
                         "SAMPLE: recv 0x%04X len %u", (unsigned)msg.msg_id, (unsigned)msg.length);

        memset(&msg, 0, sizeof(msg));
        msg.msg_id = FF_SAMPLE_ECHO_MSG;
        msg.apid = app;
        msg.time_ns = FF_PalGetMonotonicNs();
        msg.length = sizeof(g_echo_counter);
        memcpy(msg.payload, &g_echo_counter, sizeof(g_echo_counter));
        (void)FF_SbPublish(&msg);
    }
}