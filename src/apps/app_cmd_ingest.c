/*
 * frick-fsw - Command Ingest (CI) application
 *
 * Reads CCSDS TC frames from the link and feeds them to the PUS layer.
 * Analogue of the cFS Command Ingest lab application.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "app.h"
#include "fsw_config.h"
#include "core/evs/evs.h"
#include "transport/link/link.h"
#include "pus/pus.h"

void FF_AppCmdIngestMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    uint8_t frame[FF_PUS_MAX_TC_FRAME];

    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "CI app listening on link '%s'",
                     (FF_LinkGet() != NULL) ? FF_LinkGet()->name : "?");

    for (;;) {
        uint16_t len = 0u;
        if (FF_LinkRecv(frame, (uint16_t)sizeof(frame), &len, 200u) == FF_SUCCESS) {
            if (len >= FF_PUS_TC_SEC_HDR_LEN) {
                (void)FF_PusIngest(frame, len);
            }
            else {
                (void)FF_EvsSend(FF_EVS_PUS_TC_ERR, FF_EVS_WARNING, app,
                                 "short TC frame (%u bytes)", (unsigned)len);
            }
        }
    }
}
