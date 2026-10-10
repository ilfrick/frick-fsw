/*
 * frick-fsw - PUS core: dispatch, verification and telemetry emission
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "pus.h"
#include "core/time/time.h"
#include "core/pal/pal.h"
#include <string.h>

#define FF_PUS_TM_SCRATCH (FF_PUS_MAX_TM_FRAME + 32u)

typedef struct
{
    bool used;
    uint8_t service;
    uint8_t subservice;
    FF_Pus_Handler_t handler;
} ff_pus_svc_t;

static ff_pus_svc_t g_svcs[FF_PUS_MAX_SERVICES];
static const FF_Pus_Backend_t *g_backend;
static FF_Pus_TmSink_t g_tm_sink;
static uint16_t g_tm_counter;
static uint32_t g_tc_count;
static bool g_ready;

FF_Status_t FF_PusInit(void)
{
    memset(g_svcs, 0, sizeof(g_svcs));
    g_backend = NULL;
    g_tm_sink = NULL;
    g_tm_counter = 0u;
    g_tc_count = 0u;
    g_ready = false;

    if (FF_PusRegisterCoreServices() != FF_SUCCESS) {
        return FF_ERROR;
    }
    g_ready = true;
    return FF_SUCCESS;
}

FF_Status_t FF_PusSetBackend(const FF_Pus_Backend_t *backend)
{
    if (backend == NULL || backend->encode_tm == NULL || backend->decode_tc == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if ((backend->init != NULL) && (backend->init() != FF_SUCCESS)) {
        return FF_ERROR;
    }
    g_backend = backend;
    return FF_SUCCESS;
}

const FF_Pus_Backend_t *FF_PusGetBackend(void)
{
    return g_backend;
}

void FF_PusSetTmSink(FF_Pus_TmSink_t sink)
{
    g_tm_sink = sink;
}

FF_Status_t FF_PusRegisterService(uint8_t service, uint8_t subservice, FF_Pus_Handler_t handler)
{
    if (handler == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    for (uint16_t i = 0; i < FF_PUS_MAX_SERVICES; i++) {
        if (!g_svcs[i].used) {
            g_svcs[i].used = true;
            g_svcs[i].service = service;
            g_svcs[i].subservice = subservice;
            g_svcs[i].handler = handler;
            return FF_SUCCESS;
        }
    }
    return FF_NO_RESOURCE;
}

static FF_Pus_Handler_t ff_pus_find(uint8_t service, uint8_t subservice)
{
    for (uint16_t i = 0; i < FF_PUS_MAX_SERVICES; i++) {
        if (g_svcs[i].used && (g_svcs[i].service == service) &&
            (g_svcs[i].subservice == subservice)) {
            return g_svcs[i].handler;
        }
    }
    return NULL;
}

uint16_t FF_PusNextTmCounter(void)
{
    uint16_t c = g_tm_counter;
    g_tm_counter = (uint16_t)((g_tm_counter + 1u) & 0x3FFFu);
    return c;
}

uint16_t FF_PusPeekTmCounter(void)
{
    return g_tm_counter;
}
uint32_t FF_PusGetTcCount(void)
{
    return g_tc_count;
}

FF_Status_t FF_PusSendTm(uint8_t service, uint8_t subservice, const uint8_t *data, uint16_t len)
{
    uint8_t scratch[FF_PUS_TM_SCRATCH];
    FF_Pus_TmHdr_t hdr;
    uint16_t out_len = 0u;
    uint32_t coarse = 0u;
    uint16_t fine = 0u;

    if (g_backend == NULL || g_backend->encode_tm == NULL) {
        return FF_NOT_READY;
    }
    if ((data == NULL) && (len != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len > FF_PUS_MAX_TM_FRAME) {
        return FF_BAD_SIZE;
    }

    (void)FF_TimeGetCuc(&coarse, &fine);
    memset(&hdr, 0, sizeof(hdr));
    hdr.version = FF_PUS_VERSION;
    hdr.service = service;
    hdr.subservice = subservice;
    hdr.counter = FF_PusNextTmCounter();
    hdr.destination_id = FF_MISSION_ID;
    hdr.time_coarse_s = coarse;
    hdr.time_fine = fine;

    if (g_backend->encode_tm(&hdr, data, len, scratch, sizeof(scratch), &out_len) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (g_tm_sink != NULL) {
        g_tm_sink(scratch, out_len);
    }
    return FF_SUCCESS;
}

FF_Status_t FF_PusSendVerification(const FF_Pus_TcHdr_t *tc, FF_Pus_Stage_t stage, uint8_t success,
                                   uint8_t code)
{
    uint8_t payload[5];
    uint8_t subservice;

    if (tc == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Service 1 subservice: 1 accept. ok, 2 accept fail, 3 start ok, 4 start fail,
     * 5 progress, 7 completion ok, 8 completion fail. */
    switch (stage) {
    case FF_PUS_STAGE_ACCEPTANCE:
        subservice = success ? 1u : 2u;
        break;
    case FF_PUS_STAGE_START:
        subservice = success ? 3u : 4u;
        break;
    case FF_PUS_STAGE_PROGRESS:
        subservice = 5u;
        break;
    case FF_PUS_STAGE_COMPLETION:
        subservice = success ? 7u : 8u;
        break;
    default:
        return FF_INVALID_ARGUMENT;
    }

    payload[0] = (uint8_t)(tc->source_id >> 8);
    payload[1] = (uint8_t)(tc->source_id & 0xFFu);
    payload[2] = tc->service;
    payload[3] = tc->subservice;
    payload[4] = code;
    return FF_PusSendTm(FF_PUS_SERVICE_VERIFICATION, subservice, payload, sizeof(payload));
}

FF_Status_t FF_PusEmitEvent(uint16_t event_id, uint8_t evs_type, const uint8_t *data, uint16_t len)
{
    (void)event_id;
    if (len > (FF_PUS_MAX_TM_FRAME - 3u)) {
        return FF_BAD_SIZE;
    }
    if (evs_type == FF_EVS_INFO) {
        return FF_PusSendTm(FF_PUS_SERVICE_EVENT, 1u, data, len);
    }
    if (evs_type == FF_EVS_WARNING) {
        return FF_PusSendTm(FF_PUS_SERVICE_EVENT, 2u, data, len);
    }
    return FF_PusSendTm(FF_PUS_SERVICE_EVENT, 3u, data, len);
}

FF_Status_t FF_PusIngest(const uint8_t *raw, uint16_t len)
{
    FF_Pus_TcHdr_t tc;
    const uint8_t *data = NULL;
    uint16_t data_len = 0u;
    uint16_t apid = 0u;
    FF_Pus_Handler_t handler;
    FF_Status_t rc;
    bool ack_accept;
    bool ack_start;
    bool ack_complete;
    bool ack_progress;

    if (!g_ready || (raw == NULL) || (g_backend == NULL)) {
        return FF_NOT_READY;
    }
    if (g_backend->decode_tc(raw, len, &apid, &tc, &data, &data_len) != FF_SUCCESS) {
        (void)FF_EvsSend(FF_EVS_PUS_TC_ERR, FF_EVS_ERROR, 0u, "PUS decode failure");
        return FF_PARSE_ERROR;
    }

    g_tc_count++;
    ack_accept = (tc.ack_flags & FF_PUS_ACK_ACCEPTANCE) != 0u;
    ack_start = (tc.ack_flags & FF_PUS_ACK_START) != 0u;
    ack_progress = (tc.ack_flags & FF_PUS_ACK_PROGRESS) != 0u;
    ack_complete = (tc.ack_flags & FF_PUS_ACK_COMPLETION) != 0u;

    if (apid != FF_APID_TC) {
        if (ack_accept) {
            (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_ACCEPTANCE, 0u,
                                         FF_PUS_FAIL_ILLEGAL_APID);
        }
        return FF_ERROR;
    }
    if (ack_accept) {
        (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_ACCEPTANCE, 1u, 0u);
    }

    handler = ff_pus_find(tc.service, tc.subservice);
    if (handler == NULL) {
        uint8_t code =
            (tc.service == 0u) ? FF_PUS_FAIL_UNKNOWN_SERVICE : FF_PUS_FAIL_UNKNOWN_SUBSERVICE;
        if (ack_complete) {
            (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_COMPLETION, 0u, code);
        }
        return FF_NOT_FOUND;
    }

    if (ack_start) {
        (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_START, 1u, 0u);
    }
    if (ack_progress) {
        (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_PROGRESS, 1u, 0u);
    }

    rc = handler(&tc, data, data_len);

    if (ack_complete) {
        (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_COMPLETION, (rc == FF_SUCCESS) ? 1u : 0u,
                                     (rc == FF_SUCCESS) ? 0u : FF_PUS_FAIL_EXECUTION);
    }
    return rc;
}
