/*
 * frick-fsw - Telemetry Output (TO) application
 *
 * Installs the PUS telemetry sink that pushes encoded TM packets onto the
 * link. Analogue of the cFS Telemetry Output lab application.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "app.h"
#include "core/pal/pal.h"
#include "transport/link/link.h"
#include "pus/pus.h"

static void ff_to_sink(const uint8_t *packet, uint16_t len)
{
    (void)FF_LinkSend(packet, len);
    FF_PalLog("TM -> link, %u bytes (S%u)\n", (unsigned)len,
              (unsigned)(len >= 8u ? packet[7] : 0u));
}

FF_Status_t FF_AppTmOutputInit(void)
{
    FF_PusSetTmSink(ff_to_sink);
    return FF_SUCCESS;
}

void FF_AppTmOutputMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "TO app telemetry sink installed");
    for (;;) {
        /* Bound deferred callback work per cycle; deferred events produced by
         * callbacks are drained on a later iteration, never recursively. */
        (void)FF_EvsDispatchPending(8u);
        FF_PalTaskDelayMs(1000u);
    }
}
