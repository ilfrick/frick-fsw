/*
 * frick-fsw - S8-PUS-FU S11 residual bounded corrections
 *
 * Implements comprehensive S1 verification end-to-end tests and S3/S9/S20
 * subservice dispatch validation per NC-EXT-14/15/16/18/22.
 *
 * These are host-SIL requirement-based tests (norm N22). They do not establish
 * PUS-C profile compliance, qualification, or mission tailoring.
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
#include <string.h>

/* ---- Common test infrastructure ---------------------------------------- */

static uint8_t g_rx_buf[8 * 256];
static uint16_t g_rx_count;

static void tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }
}

static uint16_t stream_count(uint8_t service, uint8_t subservice)
{
    uint16_t pos = 0u;
    uint16_t count = 0u;
    while ((pos + 9u) <= g_rx_count) {
        if (g_rx_buf[pos + 7] == service && g_rx_buf[pos + 8] == subservice) {
            count++;
        }
        if ((pos + 6u) > g_rx_count)
            break;
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx_buf[pos + 4] << 8) | g_rx_buf[pos + 5]);
        pos = (uint16_t)(pos + 6u + pdl + 1u);
    }
    return count;
}

static void wr_u16be(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

static void wr_u32be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

/* Build a minimal TC frame for the given service/subservice/payload. */
static void build_tc(uint8_t service, uint8_t subservice, uint16_t source_id,
                     const uint8_t *payload, uint16_t payload_len, uint8_t *frame,
                     uint16_t *frame_len)
{
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags =
        FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_START | FF_PUS_ACK_PROGRESS | FF_PUS_ACK_COMPLETION;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = source_id;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, 128, frame_len) ==
                   FF_SUCCESS);
}

/* Dummy HK provider */
static FF_Status_t dummy_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    out[0] = 0xAAu;
    out[1] = 0xBBu;
    *len = 2u;
    return FF_SUCCESS;
}

/* Dummy parameter setter/provider */
static FF_Status_t dummy_param_setter(uint8_t id, const uint8_t *data, uint16_t len)
{
    (void)id;
    (void)data;
    (void)len;
    return FF_SUCCESS;
}

static FF_Status_t dummy_param_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    out[0] = 0xCCu;
    out[1] = 0xDDu;
    *len = 2u;
    return FF_SUCCESS;
}

/* ---- S1 Service 1: comprehensive verification end-to-end ---------------- */

static void test_s1_full_verification_sequence(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);
    g_rx_count = 0u;

    /* Service 17 ping - exercises full S1 verification chain */
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};

    build_tc(FF_PUS_SERVICE_TEST, 1u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);

    /* Verify all 4 verification subservices emitted in order */
    FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);  /* acceptance OK */
    FF_TEST_ASSERT(stream_count(1u, 3u) == 1u);  /* start OK */
    FF_TEST_ASSERT(stream_count(1u, 5u) == 1u);  /* progress OK */
    FF_TEST_ASSERT(stream_count(1u, 7u) == 1u);  /* completion OK */
    FF_TEST_ASSERT(stream_count(17u, 2u) == 1u); /* ping reply */

    /* S1 negative: acceptance failure for unknown APID */
    g_rx_count = 0u;
    FF_Pus_TcHdr_t tc_bad;
    memset(&tc_bad, 0, sizeof(tc_bad));
    tc_bad.version = FF_PUS_VERSION;
    tc_bad.ack_flags = FF_PUS_ACK_ACCEPTANCE;
    tc_bad.service = FF_PUS_SERVICE_TEST;
    tc_bad.subservice = 1u;
    tc_bad.source_id = 0x9999u; /* not FF_APID_TC */

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x1234u, &tc_bad, payload, sizeof(payload), frame,
                                    sizeof(frame), &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_ERROR);
    FF_TEST_ASSERT(stream_count(1u, 2u) == 1u); /* acceptance fail (S1-2) */
}

/* S1: missing subservices - acceptance fail, completion fail for unknown subservice */
static void test_s1_missing_subservices_coverage(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    uint8_t frame[128];
    uint16_t frame_len = 0u;
    uint8_t payload[] = {0xAA, 0xBB};

    /* Test: Force handler failure for S1-2, S1-8 coverage.
     * Use unknown subservice of known service to trigger FF_NOT_FOUND.
     * Note: start/progress verifications are NOT sent when handler is not found
     * (the dispatcher returns early before those stages). */
    g_rx_count = 0u;
    FF_Pus_TcHdr_t tc_fail;
    memset(&tc_fail, 0, sizeof(tc_fail));
    tc_fail.version = FF_PUS_VERSION;
    tc_fail.ack_flags =
        FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_START | FF_PUS_ACK_PROGRESS | FF_PUS_ACK_COMPLETION;
    tc_fail.service = FF_PUS_SERVICE_HK; /* known service */
    tc_fail.subservice = 99u;            /* unknown subservice -> FF_NOT_FOUND */
    tc_fail.source_id = 0x0102u;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc_fail, payload, sizeof(payload), frame,
                                    sizeof(frame), &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);

    /* With completion ack: acceptance OK (1,1) and completion FAIL (1,8) with
     * FF_PUS_FAIL_UNKNOWN_SUBSERVICE. Start/progress verifications are NOT emitted when handler is
     * not found. */
    FF_TEST_ASSERT(stream_count(1u, 1u) == 1u); /* acceptance OK */
    FF_TEST_ASSERT(stream_count(1u, 8u) == 1u); /* completion FAIL (S1-8) */
    FF_TEST_ASSERT(stream_count(1u, 3u) == 0u); /* start NOT sent */
    FF_TEST_ASSERT(stream_count(1u, 5u) == 0u); /* progress NOT sent */
}

/* ---- S3: standard subservice assignment validation (NC-EXT-16) -------- */

static void test_s3_subservice_assignments(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    FF_TEST_ASSERT(FF_PusRegisterHkProvider(1u, dummy_hk_provider) == FF_SUCCESS);
    g_rx_count = 0u;

    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* TC[3,1] create - should succeed */
    {
        uint8_t payload[6] = {10u, 1u, 0u, 0u, 0u, 10u}; /* id=10, prov=1, period=10s */
        build_tc(FF_PUS_SERVICE_HK, 1u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u); /* acceptance OK */
    }

    /* TC[3,5] enable - should succeed for existing report */
    g_rx_count = 0u;
    {
        uint8_t payload[1] = {10u};
        build_tc(FF_PUS_SERVICE_HK, 5u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);
    }

    /* TC[3,6] disable - should succeed */
    g_rx_count = 0u;
    {
        uint8_t payload[1] = {10u};
        build_tc(FF_PUS_SERVICE_HK, 6u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);
    }

    /* TC[3,27] report request - one-shot, should emit HK TM(3,25).
     * Must be called while report still exists (before TC[3,3] delete). */
    g_rx_count = 0u;
    {
        uint8_t payload[2] = {1u, 10u}; /* N=1, report_id=10 */
        build_tc(FF_PUS_SERVICE_HK, 27u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(3u, 25u) == 1u); /* HK report TM emitted */
    }

    /* TC[3,2] (diagnostic) - NOT FOUND per implementation */
    g_rx_count = 0u;
    {
        uint8_t payload[1] = {10u};
        build_tc(FF_PUS_SERVICE_HK, 2u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }

    /* TC[3,3] delete - should succeed after create */
    g_rx_count = 0u;
    {
        uint8_t payload[1] = {10u};
        build_tc(FF_PUS_SERVICE_HK, 3u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);
    }

    /* Verify non-standard subservices return NOT_FOUND */
    const uint8_t invalid_subservices[] = {4u, 7u, 8u, 9u, 10u, 255u};
    for (size_t i = 0; i < sizeof(invalid_subservices); i++) {
        g_rx_count = 0u;
        uint8_t payload[1] = {10u};
        build_tc(FF_PUS_SERVICE_HK, invalid_subservices[i], 0x0102u, payload, sizeof(payload),
                 frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }

    /* TC[3,27] report request for deleted report should return NOT_FOUND */
    g_rx_count = 0u;
    {
        uint8_t payload[2] = {1u, 10u}; /* N=1, report_id=10 (deleted) */
        build_tc(FF_PUS_SERVICE_HK, 27u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
        FF_TEST_ASSERT(stream_count(3u, 25u) == 0u); /* no HK report TM emitted */
    }
}

/* ---- S9: time management subservice validation (NC-EXT-16) ------------ */

static void test_s9_time_subservices(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    g_rx_count = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* TC[9,1] set time - 6 octet CUC T-field */
    {
        uint8_t payload[6];
        wr_u32be(&payload[0], 42u); /* coarse = 42s */
        wr_u16be(&payload[4], 0u);  /* fine = 0 */
        build_tc(FF_PUS_SERVICE_TIME, 1u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u); /* acceptance OK */
        FF_TEST_ASSERT(stream_count(1u, 7u) == 1u); /* completion OK */
    }

    /* TC[9,2] report time - no payload */
    g_rx_count = 0u;
    {
        build_tc(FF_PUS_SERVICE_TIME, 2u, 0x0102u, NULL, 0u, frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);
        FF_TEST_ASSERT(stream_count(1u, 7u) == 1u);
        FF_TEST_ASSERT(stream_count(9u, 2u) == 1u); /* time report TM emitted */
    }

    /* Invalid: 7-octet payload with P-field (should be FF_BAD_SIZE) */
    g_rx_count = 0u;
    {
        uint8_t payload[7];
        payload[0] = 0x2Eu; /* P-field that would mis-parse as coarse MSB */
        wr_u32be(&payload[1], 0u);
        wr_u16be(&payload[5], 0u);
        build_tc(FF_PUS_SERVICE_TIME, 1u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_BAD_SIZE);
        FF_TEST_ASSERT(stream_count(1u, 8u) == 1u); /* completion fail */
    }

    /* Invalid subservice 0 and 255 */
    g_rx_count = 0u;
    {
        build_tc(FF_PUS_SERVICE_TIME, 0u, 0x0102u, NULL, 0u, frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }
    g_rx_count = 0u;
    {
        build_tc(FF_PUS_SERVICE_TIME, 255u, 0x0102u, NULL, 0u, frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }
}

/* ---- S20: on-board parameter subservice validation -------------------- */

static void test_s20_parameter_subservices(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    FF_TEST_ASSERT(FF_PusRegisterParamSetter(5u, dummy_param_setter) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(5u, dummy_param_provider) == FF_SUCCESS);

    g_rx_count = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* TC[20,1] set parameter - should succeed with registered setter */
    {
        uint8_t payload[3] = {5u, 0x12u, 0x34u}; /* param_id=5, value=0x1234 */
        build_tc(FF_PUS_SERVICE_PARAMETER, 1u, 0x0102u, payload, sizeof(payload), frame,
                 &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);
        FF_TEST_ASSERT(stream_count(1u, 7u) == 1u);
    }

    /* TC[20,3] report parameter - should emit TM(20,4) */
    g_rx_count = 0u;
    {
        uint8_t payload[1] = {5u}; /* param_id=5 */
        build_tc(FF_PUS_SERVICE_PARAMETER, 3u, 0x0102u, payload, sizeof(payload), frame,
                 &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(1u, 1u) == 1u);
        FF_TEST_ASSERT(stream_count(1u, 7u) == 1u);
        FF_TEST_ASSERT(stream_count(20u, 4u) == 1u); /* parameter report TM */
    }

    /* Unknown parameter ID */
    g_rx_count = 0u;
    {
        uint8_t payload[1] = {99u};
        build_tc(FF_PUS_SERVICE_PARAMETER, 3u, 0x0102u, payload, sizeof(payload), frame,
                 &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
        FF_TEST_ASSERT(stream_count(1u, 8u) == 1u); /* completion fail */
    }

    /* Invalid subservice 0 and 255 */
    g_rx_count = 0u;
    {
        build_tc(FF_PUS_SERVICE_PARAMETER, 0u, 0x0102u, NULL, 0u, frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }
    g_rx_count = 0u;
    {
        build_tc(FF_PUS_SERVICE_PARAMETER, 255u, 0x0102u, NULL, 0u, frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }
}

/* ---- S11: scheduling stub transparency (NC-EXT-22) -------------------- */

static void test_s11_stub_emits_event(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    g_rx_count = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* S11/1 with no verification ack - isolate the event */
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = 0u;
    tc.service = FF_PUS_SERVICE_SCHEDULE; /* 11 */
    tc.subservice = 1u;
    tc.source_id = 0x0102u;

    uint32_t before = FF_EvsGetTotalCount();
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, NULL, 0u, frame, sizeof(frame), &frame_len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_IMPLEMENTED);

    uint32_t n = FF_EvsGetTotalCount() - before;
    FF_TEST_ASSERT(n >= 1u);

    FF_EvsEvent_t log[64];
    uint32_t got = FF_EvsGetLog(log, 64);
    bool found = false;
    for (uint32_t i = 0; i < got; i++) {
        if (log[i].event_id == FF_EVS_PUS_TC_ERR && log[i].type == FF_EVS_WARNING) {
            found = true;
            break;
        }
    }
    FF_TEST_ASSERT(found); /* diagnostic event emitted */

    /* S11/2 should also be dispatched to the same stub */
    g_rx_count = 0u;
    tc.subservice = 2u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, NULL, 0u, frame, sizeof(frame), &frame_len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_IMPLEMENTED);
}

/* ---- S6: memory management subservice boundaries ---------------------- */

static void test_s6_subservice_boundaries(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    static uint8_t g_region[64];
    for (uint16_t i = 0; i < sizeof(g_region); i++) {
        g_region[i] = (uint8_t)(0xA5u ^ (i & 0xFFu));
    }
    FF_TEST_ASSERT(FF_PusRegisterMemoryRegion(1u, g_region, sizeof(g_region)) == FF_SUCCESS);

    g_rx_count = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* Valid subservice 5 (dump) */
    {
        uint8_t payload[7] = {1u, 0u, 0u, 0u, 0u, 0u, 4u}; /* region=1, offset=0, size=4 */
        build_tc(FF_PUS_SERVICE_MEMORY, 5u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(6u, 6u) == 1u); /* dump reply */
    }

    /* Invalid subservice 0-4, 6-255 should be NOT_FOUND */
    for (uint16_t ss = 0; ss < 256; ss++) {
        if (ss == 5u)
            continue;
        g_rx_count = 0u;
        uint8_t payload[7] = {1u, 0u, 0u, 0u, 0u, 0u, 4u};
        build_tc(FF_PUS_SERVICE_MEMORY, (uint8_t)ss, 0x0102u, payload, sizeof(payload), frame,
                 &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }
}

/* ---- S17: test service boundary --------------------------------------- */

static void test_s17_subservice_boundaries(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    g_rx_count = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* Valid subservice 1 (ping) */
    {
        uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
        build_tc(FF_PUS_SERVICE_TEST, 1u, 0x0102u, payload, sizeof(payload), frame, &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_count(17u, 2u) == 1u);
    }

    /* Invalid subservice 0, 2-255 */
    for (uint16_t ss = 0; ss < 256; ss++) {
        if (ss == 1u)
            continue;
        g_rx_count = 0u;
        uint8_t payload[] = {0x01, 0x02};
        build_tc(FF_PUS_SERVICE_TEST, (uint8_t)ss, 0x0102u, payload, sizeof(payload), frame,
                 &frame_len);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    }
}

/* ---- Unknown service dispatch ----------------------------------------- */

static void test_unknown_service_dispatch(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);

    g_rx_count = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    /* Service 99 - not registered */
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_COMPLETION;
    tc.service = 99u;
    tc.subservice = 1u;
    tc.source_id = 0x0102u;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, NULL, 0u, frame, sizeof(frame), &frame_len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);

    /* With completion ack: should emit S1(1,8) with FF_PUS_FAIL_UNKNOWN_SERVICE */
    FF_TEST_ASSERT(stream_count(1u, 8u) == 1u);
}

/* Main test entry */
FF_TEST_MAIN(test_s1_full_verification_sequence, test_s1_missing_subservices_coverage,
             test_s3_subservice_assignments, test_s9_time_subservices,
             test_s20_parameter_subservices, test_s11_stub_emits_event,
             test_s6_subservice_boundaries, test_s17_subservice_boundaries,
             test_unknown_service_dispatch)