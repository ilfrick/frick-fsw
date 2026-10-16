/*
 * frick-fsw - Command Ingest (CI) application
 *
 * Reads CCSDS TC frames from the link and feeds them to the PUS layer.
 * Analogue of the cFS Command Ingest lab application.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "app.h"
#include "fsw_config.h"
#include "transport/ccsds/ccsds.h"
#include "core/evs/evs.h"
#include "transport/link/link.h"
#include "pus/pus.h"

FF_Status_t FF_AppCmdIngestReceiveOne(FF_AppId_t app, uint32_t timeout_ms)
{
    /* FF_PUS_MAX_TC_FRAME bounds the PUS payload; reserve space for the CCSDS
     * primary header, PUS TC secondary header and optional CRC as well. */
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t len = 0u;
    FF_Status_t status = FF_LinkRecv(frame, (uint16_t)sizeof(frame), &len, timeout_ms);

    if (status != FF_SUCCESS) {
        return status;
    }
    if (len > (uint16_t)sizeof(frame)) {
        (void)FF_EvsSend(FF_EVS_PUS_TC_ERR, FF_EVS_WARNING, app, "oversized TC frame (%u bytes)",
                         (unsigned)len);
        return FF_BAD_SIZE;
    }
    if (len >= FF_PUS_TC_SEC_HDR_LEN) {
        return FF_PusIngest(frame, len);
    }
    (void)FF_EvsSend(FF_EVS_PUS_TC_ERR, FF_EVS_WARNING, app, "short TC frame (%u bytes)",
                     (unsigned)len);
    return FF_BAD_SIZE;
}

void FF_AppCmdIngestMain(void *arg)
{
    FF_AppId_t app = (FF_AppId_t)(uintptr_t)arg;
    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, app, "CI app listening on link '%s'",
                     (FF_LinkGet() != NULL) ? FF_LinkGet()->name : "?");

    for (;;) {
        (void)FF_AppCmdIngestReceiveOne(app, 200u);
    }
}
