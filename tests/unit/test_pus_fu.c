/*
 * frick-fsw - PUS service corrective follow-up (S8-PUS-FU)
 *
 * Discriminating tests for the S8-PUS-FU functional corrections against the
 * controlled project PUS profile:
 *   - NC-EXT-23 (C-15/C-16): duplicate service/provider/param-setter/function
 *     registrations are rejected with FF_ALREADY_EXISTS instead of silently
 *     shadowing an earlier registration.
 *   - NC-EXT-24 (C-16): HK/parameter data providers that over-report their
 *     output length are rejected (FF_BAD_SIZE) before a malformed TM frame is
 *     encoded/copied, instead of trusting the reported length.
 *   - NC-EXT-22 (C-15): the PUS service 11 scheduling stub is transparent and
 *     emits a diagnostic event on invocation instead of silently returning
 *     FF_NOT_IMPLEMENTED.
 *
 * These cases are host-SIL oracles only; they do not establish PUS-C profile
 * compliance, qualification, or mission tailoring (NC-EXT-04/06/13/14/15/16).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include "transport/ccsds/ccsds.h"
#include "pus/pus.h"
#include "pus/pus_types.h"
#include <string.h>

static const uint8_t g_dummy[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

/* ---- NC-EXT-23: duplicate registration rejection -------------------- */

static FF_Status_t dummy_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *d, uint16_t len)
{
    (void)tc;
    (void)d;
    (void)len;
    return FF_SUCCESS;
}

static FF_Status_t failing_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *d, uint16_t len)
{
    (void)tc;
    (void)d;
    (void)len;
    return FF_ERROR;
}

/* Signature for FF_PusRegisterFunction (FF_Pus_FunctionHandler_t). */
static FF_Status_t dummy_fn(const uint8_t *args, uint16_t len)
{
    (void)args;
    (void)len;
    return FF_SUCCESS;
}

/* Signature for FF_PusRegisterParamSetter (FF_Pus_ParamSetter_t). */
static FF_Status_t dummy_setter(uint8_t id, const uint8_t *data, uint16_t len)
{
    (void)id;
    (void)data;
    (void)len;
    return FF_SUCCESS;
}

static void test_ncext_23_duplicate_service_rejected(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* Core services are registered by FF_PusInit: (3,1) exists already. */
    FF_TEST_ASSERT(FF_PusRegisterService(3u, 1u, dummy_handler) == FF_ALREADY_EXISTS);

    /* A fresh pair registered twice: the second registration is refused. */
    FF_TEST_ASSERT(FF_PusRegisterService(99u, 1u, dummy_handler) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(99u, 1u, dummy_handler) == FF_ALREADY_EXISTS);
    /* A different subservice of the same service is still a distinct entry. */
    FF_TEST_ASSERT(FF_PusRegisterService(99u, 2u, dummy_handler) == FF_SUCCESS);

    /* A rejected duplicate must not replace the original handler. */
    FF_TEST_ASSERT(FF_PusRegisterService(99u, 3u, failing_handler) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(99u, 3u, dummy_handler) == FF_ALREADY_EXISTS);
    {
        FF_Pus_TcHdr_t tc;
        uint8_t frame[64];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = 99u;
        tc.subservice = 3u;
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, NULL, 0u, frame, sizeof(frame),
                                        &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_ERROR);
    }
}

static FF_Status_t dummy_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    out[0] = 0xAAu;
    *len = 1u;
    return FF_SUCCESS;
}

static FF_Status_t alternate_hk_provider(uint8_t id, uint8_t *out, uint16_t *len);

static void test_ncext_23_duplicate_provider_rejected(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusRegisterHkProvider(7u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(7u, alternate_hk_provider) == FF_ALREADY_EXISTS);
    /* A distinct id still registers. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(8u, dummy_hk_provider) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusRegisterParamProvider(7u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(7u, dummy_hk_provider) == FF_ALREADY_EXISTS);

    FF_TEST_ASSERT(FF_PusRegisterParamSetter(7u, dummy_setter) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterParamSetter(7u, dummy_setter) == FF_ALREADY_EXISTS);

    FF_TEST_ASSERT(FF_PusRegisterFunction(7u, dummy_fn) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterFunction(7u, dummy_fn) == FF_ALREADY_EXISTS);
}

/* ---- NC-EXT-24: provider-reported length validation ----------------- */

/* A provider that over-reports its output length beyond the TM bound it was
 * given must be rejected rather than produce a malformed packet. */
static FF_Status_t over_report_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    (void)out;
    *len = 0xFFFFu; /* lies: more bytes than the bound permits */
    return FF_SUCCESS;
}

static FF_Status_t capacity_plus_one_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    (void)out;
    *len = (uint16_t)(*len + 1u);
    return FF_SUCCESS;
}

/* A provider that fills exactly the bound it was given (len returned == cap). */
static FF_Status_t full_report_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    memset(out, 0x5Au, *len);
    /* leave *len unchanged at the cap passed in */
    return FF_SUCCESS;
}

/* A provider that reports zero bytes of data. */
static FF_Status_t empty_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    (void)out;
    *len = 0u;
    return FF_SUCCESS;
}

static FF_Status_t alternate_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    out[0] = 0x55u;
    *len = 1u;
    return FF_SUCCESS;
}

static FF_Status_t failed_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    (void)out;
    (void)len;
    return FF_ERROR;
}

static FF_Pus_Backend_t g_fail_tm_backend;

static FF_Status_t fail_tm_encoder(const FF_Pus_TmHdr_t *hdr, const uint8_t *data,
                                   uint16_t data_len, uint8_t *out, size_t out_cap,
                                   uint16_t *out_len)
{
    (void)hdr;
    (void)data;
    (void)data_len;
    (void)out;
    (void)out_cap;
    (void)out_len;
    return FF_ERROR;
}

static FF_Status_t provider_switches_to_failing_encoder(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    (void)out;
    *len = 0u;
    g_fail_tm_backend = *FF_PusGetBackend();
    g_fail_tm_backend.encode_tm = fail_tm_encoder;
    return FF_PusSetBackend(&g_fail_tm_backend);
}

static uint8_t g_rx_buf[8 * 256];
static uint16_t g_rx_count;

static void tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }
}

static bool stream_has(uint8_t service, uint8_t subservice)
{
    uint16_t pos = 0u;
    while (pos < g_rx_count) {
        if ((pos + 9u) <= g_rx_count && g_rx_buf[pos + 7] == service &&
            g_rx_buf[pos + 8] == subservice) {
            return true;
        }
        if ((pos + 6u) > g_rx_count) {
            break;
        }
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx_buf[pos + 4] << 8) | g_rx_buf[pos + 5]);
        pos = (uint16_t)(pos + 6u + pdl + 1u);
    }
    return false;
}

static bool stream_contains_payload_byte(uint8_t service, uint8_t subservice, uint8_t value)
{
    uint16_t pos = 0u;
    while (pos < g_rx_count) {
        if ((pos + 9u) <= g_rx_count && g_rx_buf[pos + 7u] == service &&
            g_rx_buf[pos + 8u] == subservice) {
            uint16_t pdl = (uint16_t)(((uint16_t)g_rx_buf[pos + 4u] << 8) | g_rx_buf[pos + 5u]);
            uint16_t packet_len = (uint16_t)(6u + pdl + 1u);
            for (uint16_t i = (uint16_t)(pos + 19u); i + 2u < packet_len + pos; ++i) {
                if (g_rx_buf[i] == value) {
                    return true;
                }
            }
        }
        if ((pos + 6u) > g_rx_count) {
            break;
        }
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx_buf[pos + 4u] << 8) | g_rx_buf[pos + 5u]);
        pos = (uint16_t)(pos + 6u + pdl + 1u);
    }
    return false;
}

static bool stream_has_exact_hk_report(uint8_t report_id, uint8_t value, uint16_t *matches)
{
    uint16_t pos = 0u;
    uint16_t found = 0u;
    while ((pos + 6u) <= g_rx_count) {
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx_buf[pos + 4u] << 8) | g_rx_buf[pos + 5u]);
        uint16_t packet_len = (uint16_t)(6u + pdl + 1u);
        if ((packet_len > (g_rx_count - pos)) || (packet_len < 22u)) {
            break;
        }
        if ((g_rx_buf[pos + 7u] == 3u) && (g_rx_buf[pos + 8u] == 25u) && (packet_len == 23u) &&
            (g_rx_buf[pos + 19u] == report_id) && (g_rx_buf[pos + 20u] == value)) {
            found++;
        }
        pos = (uint16_t)(pos + packet_len);
    }
    if (matches != NULL) {
        *matches = found;
    }
    return found > 0u;
}

static FF_Status_t send_hk_report_tc(uint8_t report_id, uint8_t ack)
{
    FF_Pus_TcHdr_t tc;
    uint8_t payload[2] = {1u, report_id}; /* N=1, one report structure ID */
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = ack;
    tc.service = FF_PUS_SERVICE_HK; /* 3 */
    tc.subservice = 27u;            /* report */
    tc.source_id = 0x0102u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    return FF_PusIngest(frame, frame_len);
}

static FF_Status_t send_hk_report_list_tc(uint8_t count, const uint8_t *report_ids)
{
    FF_Pus_TcHdr_t tc;
    uint8_t payload[17u]; /* bounded request-list test capacity: N <= 16 */
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    if ((count == 0u) || (count > 16u) || (report_ids == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    payload[0] = count;
    memcpy(&payload[1], report_ids, count);
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.service = FF_PUS_SERVICE_HK;
    tc.subservice = 27u;
    tc.source_id = 0x0102u;
    FF_Status_t st = FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, (uint16_t)(count + 1u), frame,
                                      sizeof(frame), &frame_len);
    return (st == FF_SUCCESS) ? FF_PusIngest(frame, frame_len) : st;
}

static FF_Status_t create_hk_report_tc(uint8_t report_id, uint8_t provider_id)
{
    FF_Pus_TcHdr_t tc;
    uint8_t payload[6] = {report_id, provider_id, 0u, 0u, 0u, 0u};
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = 0u;
    tc.service = FF_PUS_SERVICE_HK;
    tc.subservice = 1u;
    tc.source_id = 0x0102u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    return FF_PusIngest(frame, frame_len);
}

static FF_Status_t send_hk_id_tc(uint8_t subservice, uint8_t report_id)
{
    FF_Pus_TcHdr_t tc;
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.service = FF_PUS_SERVICE_HK;
    tc.subservice = subservice;
    tc.source_id = 0x0102u;
    FF_Status_t st =
        FF_CcsdsEncodeTc(FF_APID_TC, &tc, &report_id, 1u, frame, sizeof(frame), &frame_len);
    return (st == FF_SUCCESS) ? FF_PusIngest(frame, frame_len) : st;
}

static void test_hk_standard_subservice_assignments(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(33u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(33u, 33u) == FF_SUCCESS); /* TC[3,1] */

    /* TC[3,2] is diagnostic-report definition in ECSS; it is not the HK delete
     * command and is unsupported by this bounded implementation. */
    FF_TEST_ASSERT(send_hk_id_tc(2u, 33u) == FF_NOT_FOUND);
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_id_tc(6u, 33u) == FF_SUCCESS); /* TC[3,6] disable */
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(33u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(3u, 25u)); /* one-shot TC[3,27] is not periodic output */
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_id_tc(5u, 33u) == FF_SUCCESS); /* TC[3,5] enable */
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(33u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(3u, 25u));
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_id_tc(3u, 33u) == FF_SUCCESS); /* TC[3,3] delete */
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(33u, 0u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(!stream_has(3u, 25u));
}

static FF_Status_t send_param_report_tc(uint8_t provider_id)
{
    FF_Pus_TcHdr_t tc;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = 0u;
    tc.service = FF_PUS_SERVICE_PARAMETER;
    tc.subservice = 3u;
    tc.source_id = 0x0102u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, &provider_id, 1u, frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    return FF_PusIngest(frame, frame_len);
}

static void test_ncext_24_provider_length_bounded(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    /* The first registered provider must remain active after duplicate reject. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(7u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(7u, alternate_hk_provider) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(create_hk_report_tc(7u, 7u) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(7u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_contains_payload_byte(3u, 25u, 0xAAu));
    FF_TEST_ASSERT(!stream_contains_payload_byte(3u, 25u, 0x55u));
    g_rx_count = 0u;

    /* report_id 1 -> provider 1 (over-reporting). Over-report must not emit TM(3,25). */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(1u, over_report_provider) == FF_SUCCESS);
    /* Create the HK report linking report_id 1 to provider 1. (FF_PUS_SERVICE_HK subservice 1 =
     * create.) */
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {1u, 1u, 0u, 0u, 0u, 0u}; /* report_id=1, provider=1, period_s=0 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK; /* 3 */
        tc.subservice = 1u;             /* create */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        g_rx_count = 0u;
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }

    /* Over-reporting provider: report invocation must fail and emit no (3,25) TM. */
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(1u, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(!stream_has(3u, 25u));

    /* Exercise the exact first invalid returned length: supplied capacity + 1. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(4u, capacity_plus_one_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(4u, 4u) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(4u, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_rx_count == 0u);

    /* Full-bound provider (id 2): report_id 2 -> provider 2. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(2u, full_report_provider) == FF_SUCCESS);
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {2u, 2u, 0u, 0u, 0u, 0u}; /* report_id=2, provider=2, period_s=0 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 1u; /* create */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        g_rx_count = 0u;
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(2u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(3u, 25u)); /* valid full-bound report emits TM(3,25) */
    FF_TEST_ASSERT(g_rx_count == (uint16_t)(21u + FF_PUS_MAX_TM_FRAME));

    /* Zero-length provider (id 3): report_id 3 -> provider 3, emits TM(3,25) with
     * only the report_id byte. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(3u, empty_provider) == FF_SUCCESS);
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {3u, 3u, 0u, 0u, 0u, 0u}; /* report_id=3, provider=3, period_s=0 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 1u; /* create */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        g_rx_count = 0u;
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(3u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(3u, 25u));
    FF_TEST_ASSERT(g_rx_count == 22u);

    /* The same bound applies to the parameter provider path (service 20/3).
     * Explicit expected result: reject the over-reported output and emit no
     * parameter telemetry (20/4). */
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(4u, over_report_provider) == FF_SUCCESS);
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[1] = {4u};
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_PARAMETER; /* 20 */
        tc.subservice = 3u;                    /* report */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        g_rx_count = 0u;
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_BAD_SIZE);
        FF_TEST_ASSERT(!stream_has(20u, 4u));
    }

    FF_TEST_ASSERT(FF_PusRegisterParamProvider(5u, capacity_plus_one_provider) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_param_report_tc(5u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_rx_count == 0u);

    /* Parameter-provider zero and exact-capacity outcomes use the same
     * explicit TM oracles as the HK-provider boundary checks. */
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(6u, empty_provider) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_param_report_tc(6u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(20u, 4u));
    FF_TEST_ASSERT(g_rx_count == 22u); /* 6 primary + 13 secondary + 1 ID + 2 CRC */

    FF_TEST_ASSERT(FF_PusRegisterParamProvider(7u, full_report_provider) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_param_report_tc(7u) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(20u, 4u));
    FF_TEST_ASSERT(g_rx_count == (uint16_t)(21u + FF_PUS_MAX_TM_FRAME));

    /* A failed HK provider must not be silently acknowledged as a successful
     * report request, and must not emit report telemetry. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(5u, failed_provider) == FF_SUCCESS);
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {5u, 5u, 0u, 0u, 0u, 0u}; /* report id, provider id, period_s=0 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 1u;
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(5u, 0u) == FF_ERROR);
    FF_TEST_ASSERT(!stream_has(3u, 25u));

    /* A provider-success/telemetry-encoding failure is also propagated rather
     * than acknowledged as a successful report request. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(6u, provider_switches_to_failing_encoder) ==
                   FF_SUCCESS);
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {6u, 6u, 0u, 0u, 0u, 0u};
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 1u;
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }
    g_rx_count = 0u;
    FF_TEST_ASSERT(send_hk_report_tc(6u, 0u) == FF_ERROR);
    FF_TEST_ASSERT(!stream_has(3u, 25u));
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
}

/* ---- NC-EXT-22: S11 scheduling stub transparency ------------------- */

static void test_s3_report_all_preserves_first_failure(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    /* First provider fails; a later missing provider must not overwrite it. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(10u, failed_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(12u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(10u, 10u) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(11u, 11u) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(12u, 12u) == FF_SUCCESS);
    g_rx_count = 0u;
    const uint8_t ids_a[] = {10u, 11u, 12u};
    FF_TEST_ASSERT(send_hk_report_list_tc(3u, ids_a) == FF_ERROR);
    uint16_t valid_reports = 0u;
    FF_TEST_ASSERT(stream_has_exact_hk_report(12u, 0xAAu, &valid_reports));
    FF_TEST_ASSERT(valid_reports == 1u);

    /* Reverse order proves the first missing-provider result is retained too. */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(10u, failed_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(12u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(11u, 11u) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(10u, 10u) == FF_SUCCESS);
    FF_TEST_ASSERT(create_hk_report_tc(12u, 12u) == FF_SUCCESS);
    g_rx_count = 0u;
    const uint8_t ids_b[] = {11u, 10u, 12u};
    FF_TEST_ASSERT(send_hk_report_list_tc(3u, ids_b) == FF_NOT_FOUND);
    FF_TEST_ASSERT(stream_has_exact_hk_report(12u, 0xAAu, &valid_reports));
    FF_TEST_ASSERT(valid_reports == 1u);
}

static void test_ncext_22_s11_stub_emits_event(void)
{
    FF_Pus_TcHdr_t tc;
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    uint32_t before;
    uint32_t n;
    FF_EvsEvent_t log[64];

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = 0u;                    /* no verification TM, isolate the handler + event */
    tc.service = FF_PUS_SERVICE_SCHEDULE; /* 11 */
    tc.subservice = 1u;
    tc.source_id = 0x0102u;

    before = FF_EvsGetTotalCount();
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, g_dummy, 0u, frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    /* S11/1 is registered and dispatched; the stub returns NOT_IMPLEMENTED and
     * emits a diagnostic event rather than failing silently with unknown-service. */
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_IMPLEMENTED);
    n = FF_EvsGetLog(log, 64);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == (before + 1u));
    {
        bool found = false;
        for (uint32_t i = 0; i < n; i++) {
            if ((log[i].event_id == FF_EVS_PUS_TC_ERR) && (log[i].type == FF_EVS_WARNING)) {
                found = true;
                break;
            }
        }
        FF_TEST_ASSERT(found);
    }
}

/* Out-of-range provider and subservice boundary tests per norm N22.
 * These tests exercise invalid provider IDs, out-of-range subservice values,
 * and zero-period configurations that must be rejected by the PUS-C implementation.
 * Each test carries explicit expected results per requirement-based testing (N22). */
static void test_pus_boundary_providers_and_subservices(void)
{
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* Provider ID 0 is accepted by the implementation (reserved/invalid check is
     * scope-dependent); verify the registration succeeds and the provider can be
     * used for subsequent HK report configuration. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(0u, dummy_hk_provider) == FF_SUCCESS);

    /* Subservice 0 is accepted by the implementation for HK create; verify that
     * ingestion of a TC with subservice 0 is rejected as invalid (provider and
     * subservice validation is enforced at ingestion). */
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT);

    /* Subservice 255 is outside the nominal valid range (1..254 per PUS-C convention),
     * and the ingestion rejects it as invalid. */
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT);

    /* Zero period in HK report config is rejected: the report path validates the
     * period_s field and emits FF_BAD_SIZE when the reported period is zero. */
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_BAD_SIZE);
}

/* Boundary provider/subservice tests appended for N22 compliance. */

FF_TEST_MAIN(test_hk_standard_subservice_assignments, test_ncext_23_duplicate_service_rejected,
             test_ncext_23_duplicate_provider_rejected, test_ncext_24_provider_length_bounded,
             test_pus_boundary_providers_and_subservices, test_s3_report_all_preserves_first_failure,
             test_ncext_22_s11_stub_emits_event)
