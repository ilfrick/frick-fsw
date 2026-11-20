/*
 * frick-fsw - host PTY E2E server for the GSW UART test
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "core/evs/evs.h"
#include "core/pal/pal.h"
#include "core/time/time.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"
#include "transport/link/link.h"

#include <stdio.h>
#include <stdlib.h>

#define FF_GSW_E2E_RECV_TIMEOUT_MS 4000u

static FF_Status_t g_tm_send_status = FF_SUCCESS;

static void gsw_e2e_tm_sink(const uint8_t *packet, uint16_t len)
{
    printf("[DEBUG SINK] Called with service=%d, subservice=%d, len=%d\n", packet[7], packet[8], len); fflush(stdout);
    const char *capture_path = getenv("FRICK_E2E_TM_CAPTURE_PATH");
    if (capture_path != NULL) {
        FILE *capture = fopen(capture_path, "ab");
        if (capture == NULL) {
            g_tm_send_status = FF_ERROR;
            return;
        }
        bool write_ok = (fwrite(packet, 1u, len, capture) == len);
        int close_status = fclose(capture);
        if (!write_ok || (close_status != 0)) {
            g_tm_send_status = FF_ERROR;
            return;
        }
    }
    printf("[DEBUG SINK] Calling FF_LinkSend len=%d\n", len); fflush(stdout);
    g_tm_send_status = FF_LinkSend(packet, len);
    printf("[DEBUG SINK] FF_LinkSend returned %d\n", g_tm_send_status); fflush(stdout);
}

int main(void)
{
    uint8_t packet[FF_CCSDS_MAX_TC_PACKET];
    uint16_t packet_len = 0u;
    FF_Status_t st;

    if ((FF_PalInit() != FF_SUCCESS) || (FF_EvsInit() != FF_SUCCESS) ||
        (FF_TimeInit() != FF_SUCCESS) || (FF_PusInit() != FF_SUCCESS)) {
        return 10;
    }
    if (FF_PusSetBackend(&FF_PusBackendOpen) != FF_SUCCESS) {
        return 11;
    }
    if (FF_LinkSelect(&FF_LinkUartBackend) != FF_SUCCESS) {
        return 12;
    }
    FF_PusSetTmSink(gsw_e2e_tm_sink);

    st = FF_LinkRecv(packet, (uint16_t)sizeof(packet), &packet_len, FF_GSW_E2E_RECV_TIMEOUT_MS);
    if (st != FF_SUCCESS) {
        return 13;
    }
    st = FF_PusIngest(packet, packet_len);
    if ((st != FF_SUCCESS) || (g_tm_send_status != FF_SUCCESS)) {
        return 14;
    }
    return 0;
}
