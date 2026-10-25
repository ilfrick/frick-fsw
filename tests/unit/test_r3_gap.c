/*
 * frick-fsw - System / integration gap tests (R3 review FRICK-REVIEW-0076)
 *
 * Requirement-level gap closure for the R3 (Senior Systems Engineer) analysis of
 * how the system is tested at the requirement / system level. These tests
 * exercise end-to-end flows through the public APIs for behaviours that were
 * previously backed only by `ci:docs`, `ci:simulator-smoke` or weak unit
 * evidence:
 *
 *   SRS-REQ-001  SB exact + wildcard publish/subscribe fan-out
 *   SRS-REQ-002  SB FIFO ordering on a single pipe
 *   SRS-REQ-005  PUS unknown service / subservice / malformed TC -> S1 failure
 *   SRS-REQ-006  PUS verification: exactly one S1 packet per acknowledged stage
 *   SRS-REQ-007  PUS TM counter strictly increases and wraps modulo 16384
 *   SRS-REQ-008  PUS S3 housekeeping create/report/enable/disable/delete
 *   SRS-REQ-010  PUS S9 set time then report time (CUC on the wire)
 *   SRS-REQ-011  EVS event -> PUS S5 TM (listener wiring as in fsw_main.c)
 *   SRS-REQ-013  TBL atomic load/activate/revert lifecycle
 *   SRS-REQ-052  PUS S8 function management and S20 parameter set/report
 *                (implemented in the reference core; previously ci:docs)
 *
 * Self-contained: public APIs + the ff_test.h harness only. It does not modify
 * the framework, nor the shared test registration (see the R3 report for the
 * one-line CMake wiring the FSW engineer must apply).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include "core/tbl/tbl.h"
#include "transport/ccsds/ccsds.h"
#include "pus/pus.h"
#include "fsw_config.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Telemetry capture and inspection helpers                            */
/* ------------------------------------------------------------------ */

#define R3_RX_CAP 8192u

static uint8_t g_rx[R3_RX_CAP];
static uint16_t g_rx_count;

static void tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((uint16_t)(g_rx_count + len) <= R3_RX_CAP) {
        (void)memcpy(&g_rx[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }
}

static void rx_reset(void)
{
    (void)memset(g_rx, 0, sizeof(g_rx));
    g_rx_count = 0u;
}

/* CCSDS total frame length = 7 + Packet Data Length (header 6 + PDL field). */
static uint16_t rx_frame_len(uint16_t pos)
{
    uint16_t pdl = (uint16_t)(((uint16_t)g_rx[pos + 4u] << 8) | (uint16_t)g_rx[pos + 5u]);
    return (uint16_t)(7u + pdl);
}

static bool rx_find(uint8_t service, uint8_t subservice, const uint8_t **data, uint16_t *data_len)
{
    uint16_t pos = 0u;

    while ((uint16_t)(pos + 7u) <= g_rx_count) {
        uint16_t flen = rx_frame_len(pos);
        uint16_t apid = 0u;
        FF_Pus_TmHdr_t hdr;
        const uint8_t *d = NULL;
        uint16_t dl = 0u;

        if (flen > (uint16_t)(g_rx_count - pos)) {
            break;
        }
        (void)memset(&hdr, 0, sizeof(hdr));
        if (FF_CcsdsDecodeTm(&g_rx[pos], flen, &apid, &hdr, &d, &dl) == FF_SUCCESS) {
            if ((hdr.service == service) && (hdr.subservice == subservice)) {
                if (data != NULL) {
                    *data = d;
                }
                if (data_len != NULL) {
                    *data_len = dl;
                }
                return true;
            }
        }
        pos = (uint16_t)(pos + flen);
    }
    return false;
}

static uint16_t rx_count_svc(uint8_t service, uint8_t subservice)
{
    uint16_t pos = 0u;
    uint16_t n = 0u;

    while ((uint16_t)(pos + 7u) <= g_rx_count) {
        uint16_t flen = rx_frame_len(pos);
        uint16_t apid = 0u;
        FF_Pus_TmHdr_t hdr;
        const uint8_t *d = NULL;
        uint16_t dl = 0u;

        if (flen > (uint16_t)(g_rx_count - pos)) {
            break;
        }
        (void)memset(&hdr, 0, sizeof(hdr));
        if (FF_CcsdsDecodeTm(&g_rx[pos], flen, &apid, &hdr, &d, &dl) == FF_SUCCESS) {
            if ((hdr.service == service) && (hdr.subservice == subservice)) {
                n++;
            }
        }
        pos = (uint16_t)(pos + flen);
    }
    return n;
}

static uint16_t rx_total(void)
{
    uint16_t pos = 0u;
    uint16_t n = 0u;

    while ((uint16_t)(pos + 7u) <= g_rx_count) {
        uint16_t flen = rx_frame_len(pos);

        if (flen > (uint16_t)(g_rx_count - pos)) {
            break;
        }
        n++;
        pos = (uint16_t)(pos + flen);
    }
    return n;
}

/* ------------------------------------------------------------------ */
/* Common setup / TC helpers                                           */
/* ------------------------------------------------------------------ */

static void r3_push_setup(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);
    rx_reset();
}

static uint16_t encode_tc(uint8_t service, uint8_t subservice, uint8_t ack, const uint8_t *data,
                          uint16_t data_len, uint8_t *frame, size_t frame_cap)
{
    FF_Pus_TcHdr_t tc;
    uint16_t frame_len = 0u;

    (void)memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = ack;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = 0x0102u;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, data, data_len, frame, frame_cap,
                                    &frame_len) == FF_SUCCESS);
    return frame_len;
}

static void wr_u32be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 24) & 0xFFu);
    p[1] = (uint8_t)((v >> 16) & 0xFFu);
    p[2] = (uint8_t)((v >> 8) & 0xFFu);
    p[3] = (uint8_t)(v & 0xFFu);
}

static uint32_t rd_u32be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* EVS -> PUS S5 listener used by the malformed-TC and event-propagation tests. */
static void r3_evs_to_pus(const FF_EvsEvent_t *evt);

/* ------------------------------------------------------------------ */
/* SRS-REQ-001 / SRS-REQ-002 : Software Bus fan-out and FIFO ordering  */
/* ------------------------------------------------------------------ */

static void test_sb_wildcard_fanout(void)
{
    FF_AppId_t exact = 0x11u;
    FF_AppId_t wild = 0x12u;
    FF_AppId_t other = 0x13u;
    FF_Msg_t m;
    FF_Msg_t r;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbCreatePipe(exact, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(wild, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(other, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0B01u, exact) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(FF_SB_WILDCARD, wild) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0B02u, other) == FF_SUCCESS);

    (void)memset(&m, 0, sizeof(m));
    m.msg_id = 0x0B01u;
    m.length = 3u;
    m.payload[0] = 0x11u;
    m.payload[1] = 0x22u;
    m.payload[2] = 0x33u;
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);

    (void)memset(&r, 0, sizeof(r));
    FF_TEST_ASSERT(FF_SbReceive(exact, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x0B01u);
    FF_TEST_ASSERT(r.length == 3u);
    FF_TEST_ASSERT((r.payload[0] == 0x11u) && (r.payload[2] == 0x33u));

    /* The wildcard subscriber receives the same message (SRS-REQ-001). */
    (void)memset(&r, 0, sizeof(r));
    FF_TEST_ASSERT(FF_SbReceive(wild, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x0B01u);

    /* A subscriber to a different identifier receives nothing. */
    (void)memset(&r, 0, sizeof(r));
    FF_TEST_ASSERT(FF_SbReceive(other, &r, 10u) == FF_TIMEOUT);
}

static void test_sb_fifo_order(void)
{
    FF_AppId_t a = 0x21u;
    FF_Msg_t m;
    FF_Msg_t r;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(a, 8u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0B03u, a) == FF_SUCCESS);

    for (uint8_t i = 0u; i < 3u; i++) {
        (void)memset(&m, 0, sizeof(m));
        m.msg_id = 0x0B03u;
        m.length = 1u;
        m.payload[0] = i;
        FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);
    }

    for (uint8_t i = 0u; i < 3u; i++) {
        (void)memset(&r, 0, sizeof(r));
        FF_TEST_ASSERT(FF_SbReceive(a, &r, 100u) == FF_SUCCESS);
        FF_TEST_ASSERT(r.msg_id == 0x0B03u);
        FF_TEST_ASSERT(r.payload[0] == i); /* FIFO order (SRS-REQ-002) */
    }
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-005 / SRS-REQ-006 : PUS verification contract               */
/* ------------------------------------------------------------------ */

static void test_pus_verification_failures(void)
{
    uint8_t frame[128];
    uint16_t flen;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    r3_push_setup();

    /* Unknown service (service 0) -> completion failure code 8. */
    flen = encode_tc(0u, 0u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION, NULL, 0u, frame,
                     sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_FOUND);
    FF_TEST_ASSERT(rx_find(1u, 1u, &d, &dl));
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));
    FF_TEST_ASSERT(dl == 5u);
    FF_TEST_ASSERT(d[4] == FF_PUS_FAIL_UNKNOWN_SERVICE);

    /* Unknown subservice -> completion failure code 9. */
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_TEST, 0x7Fu, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     NULL, 0u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_FOUND);
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));
    FF_TEST_ASSERT(d[4] == FF_PUS_FAIL_UNKNOWN_SUBSERVICE);

    /* Service 11 scheduling stub -> NOT_IMPLEMENTED -> failure code 20. */
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_SCHEDULE, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     NULL, 0u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));
    FF_TEST_ASSERT(d[4] == FF_PUS_FAIL_EXECUTION);

    /* Acceptance-only: exactly one service 1 packet, no completion (SRS-REQ-006). */
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_SCHEDULE, 1u, FF_PUS_ACK_ACCEPTANCE, NULL, 0u, frame,
                     sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(rx_total() == 1u);
    FF_TEST_ASSERT(rx_count_svc(1u, 1u) == 1u);
    FF_TEST_ASSERT(rx_count_svc(1u, 7u) == 0u);
    FF_TEST_ASSERT(rx_count_svc(1u, 8u) == 0u);

    /* Ping with acceptance+completion pairs one accept and one completion. */
    rx_reset();
    {
        const uint8_t payload[] = {0x0Au, 0x0Bu};
        flen = encode_tc(FF_PUS_SERVICE_TEST, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                         payload, sizeof(payload), frame, sizeof(frame));
    }
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_count_svc(1u, 1u) == 1u);
    FF_TEST_ASSERT(rx_count_svc(1u, 7u) == 1u);
    FF_TEST_ASSERT(rx_find(17u, 2u, &d, &dl));
    FF_TEST_ASSERT((dl == 2u) && (d[0] == 0x0Au) && (d[1] == 0x0Bu));
}

static void test_pus_malformed_tc_event(void)
{
    uint8_t frame[128];
    uint16_t flen;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;
    const uint8_t payload[] = {0x01u, 0x02u, 0x03u, 0x04u};

    r3_push_setup();
    FF_EvsSetListener(r3_evs_to_pus);

    flen = encode_tc(FF_PUS_SERVICE_TEST, 1u, FF_PUS_ACK_ACCEPTANCE, payload, sizeof(payload),
                     frame, sizeof(frame));
    frame[flen - 3u] = (uint8_t)(frame[flen - 3u] ^ 0xFFu); /* break the CRC-covered body */

    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_PARSE_ERROR);
    /* The malformed TC raises an EVS error that is forwarded to PUS S5 (sub 3). */
    FF_TEST_ASSERT(rx_find(5u, 3u, &d, &dl));
    FF_TEST_ASSERT(dl >= 3u);
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_ERROR);
    FF_TEST_ASSERT(rx_count_svc(1u, 1u) == 0u); /* nothing was executed */
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-007 : PUS telemetry counter                                 */
/* ------------------------------------------------------------------ */

static void test_pus_tm_counter(void)
{
    uint16_t first;
    uint16_t prev;
    uint16_t wrap_prev = 0xFFFFu;
    uint32_t bad = 0u;

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    first = FF_PusNextTmCounter();
    FF_TEST_ASSERT(first == 0u);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == 1u);

    /* Walk one full counter cycle: every value is strictly increasing and
     * bounded, and the cycle wraps from 0x3FFF back to 0 (SRS-REQ-007). */
    prev = first;
    for (uint32_t i = 0u; i < 16390u; i++) {
        uint16_t v = FF_PusNextTmCounter();

        if (v >= 16384u) {
            bad++;
        }
        if (v != (uint16_t)((prev + 1u) & 0x3FFFu)) {
            bad++;
        }
        if (v == 0u) {
            wrap_prev = prev;
            break;
        }
        prev = v;
    }
    FF_TEST_ASSERT(bad == 0u);
    FF_TEST_ASSERT(wrap_prev == 0x3FFFu);
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-008 : PUS S3 housekeeping lifecycle                         */
/* ------------------------------------------------------------------ */

static FF_Status_t r3_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    if (*len < 4u) {
        return FF_BAD_SIZE;
    }
    out[0] = 0xAAu;
    out[1] = 0xBBu;
    out[2] = 0xCCu;
    out[3] = 0xDDu;
    *len = 4u;
    return FF_SUCCESS;
}

static void test_pus_hk_s3_lifecycle(void)
{
    const uint8_t report_id = 0x05u;
    const uint8_t provider_id = 0x21u;
    uint8_t frame[128];
    uint16_t flen;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;
    uint8_t create_payload[6];
    uint8_t one[1];
    uint8_t report_request[2];

    r3_push_setup();
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(provider_id, r3_hk_provider) == FF_SUCCESS);

    /* Create the report (3,1).
     * Provisional 6-byte payload: report_id, provider_id, period_s[4] (big-endian).
     * ICD §4 defines 2 bytes; this is an authority-pending extension (NC-EXT-17). */
    create_payload[0] = report_id;
    create_payload[1] = provider_id;
    create_payload[2] = 0u;
    create_payload[3] = 0u;
    create_payload[4] = 0u;
    create_payload[5] = 0u; /* period_s = 0 (one-shot only) */
    flen = encode_tc(FF_PUS_SERVICE_HK, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     create_payload, sizeof(create_payload), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(1u, 7u, &d, &dl));

    /* Duplicate create is rejected with a completion failure. */
    flen = encode_tc(FF_PUS_SERVICE_HK, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     create_payload, sizeof(create_payload), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));

    /* Report on request (3,27) produces (3,25) with the provider image. */
    rx_reset();
    one[0] = report_id;
    report_request[0] = 1u;
    report_request[1] = report_id;
    flen = encode_tc(FF_PUS_SERVICE_HK, 27u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     report_request, sizeof(report_request), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(3u, 25u, &d, &dl));
    FF_TEST_ASSERT(dl == 5u);
    FF_TEST_ASSERT(d[0] == report_id);
    FF_TEST_ASSERT((d[1] == 0xAAu) && (d[2] == 0xBBu) && (d[3] == 0xCCu) && (d[4] == 0xDDu));

    /* Disable periodic generation (3,6); a one-shot (3,27) still reports. */
    flen = encode_tc(FF_PUS_SERVICE_HK, 6u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION, one,
                     sizeof(one), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_HK, 27u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     report_request, sizeof(report_request), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(3u, 25u, &d, &dl));

    /* Enable periodic generation (3,5); one-shot remains available. */
    flen = encode_tc(FF_PUS_SERVICE_HK, 5u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION, one,
                     sizeof(one), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_HK, 27u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     report_request, sizeof(report_request), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(3u, 25u, &d, &dl));

    /* Delete (3,3); a second delete is a completion failure (not found). */
    flen = encode_tc(FF_PUS_SERVICE_HK, 3u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION, one,
                     sizeof(one), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_HK, 3u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION, one,
                     sizeof(one), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_FOUND);
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-010 : PUS S9 set + report time                              */
/* ------------------------------------------------------------------ */

static void test_pus_service9_set_report(void)
{
    uint8_t frame[128];
    uint16_t flen;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;
    uint8_t payload[6];

    r3_push_setup();

    /* Set UTC = 5 s via the 6-octet PUS-C T-field (9,1). */
    wr_u32be(&payload[0], 5u);
    payload[4] = 0u;
    payload[5] = 0u;
    flen = encode_tc(FF_PUS_SERVICE_TIME, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     payload, sizeof(payload), frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);

    /* Report time (9,2) echoes the coarse CUC seconds on the wire. */
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_TIME, 2u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION, NULL,
                     0u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(9u, 2u, &d, &dl));
    FF_TEST_ASSERT(dl == 6u);
    FF_TEST_ASSERT(rd_u32be(&d[0]) == 5u);
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-011 : EVS event propagated to PUS S5                        */
/* ------------------------------------------------------------------ */

static void r3_evs_to_pus(const FF_EvsEvent_t *evt)
{
    uint8_t payload[FF_PUS_MAX_TM_FRAME];
    uint16_t text_len = 0u;
    uint16_t total;

    while ((text_len < sizeof(evt->text)) && (evt->text[text_len] != '\0')) {
        text_len++;
    }
    if (text_len > (FF_PUS_MAX_TM_FRAME - 3u)) {
        text_len = (uint16_t)(FF_PUS_MAX_TM_FRAME - 3u);
    }
    payload[0] = (uint8_t)(evt->event_id >> 8);
    payload[1] = (uint8_t)(evt->event_id & 0xFFu);
    payload[2] = evt->type;
    (void)memcpy(&payload[3], evt->text, text_len);
    total = (uint16_t)(text_len + 3u);

    (void)FF_PusEmitEvent(evt->event_id, evt->type, payload, total);
}

static void test_pus_event_s5_propagation(void)
{
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    r3_push_setup();
    FF_EvsSetListener(r3_evs_to_pus); /* same wiring as src/main/fsw_main.c */

    rx_reset();
    FF_TEST_ASSERT(FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, 0u, "boot") == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(5u, 1u, &d, &dl));
    FF_TEST_ASSERT(dl >= 3u);
    FF_TEST_ASSERT((d[0] == 0x01u) && (d[1] == 0x01u));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_INFO);

    rx_reset();
    FF_TEST_ASSERT(FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_WARNING, 0u, "warn %d", 2) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(5u, 2u, &d, &dl));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_WARNING);

    rx_reset();
    FF_TEST_ASSERT(FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_ERROR, 0u, "err") == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(5u, 3u, &d, &dl));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_ERROR);

    FF_TEST_ASSERT(FF_EvsGetTotalCount() >= 3u);
    {
        FF_EvsEvent_t log[4];
        uint32_t n = FF_EvsGetLog(log, 4u);
        FF_TEST_ASSERT(n == 3u); /* all events retained in the circular log */
    }
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-052 : PUS S20 parameter set/report and S8 functions         */
/* ------------------------------------------------------------------ */

static uint8_t g_param[4];
static uint16_t g_param_len;

static FF_Status_t r3_param_set(uint8_t id, const uint8_t *data, uint16_t len)
{
    (void)id;
    if (len > sizeof(g_param)) {
        return FF_BAD_SIZE;
    }
    (void)memcpy(g_param, data, len);
    g_param_len = len;
    return FF_SUCCESS;
}

static FF_Status_t r3_param_get(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    if (*len < g_param_len) {
        return FF_BAD_SIZE;
    }
    (void)memcpy(out, g_param, g_param_len);
    *len = g_param_len;
    return FF_SUCCESS;
}

static FF_Status_t r3_fn_ok(const uint8_t *args, uint16_t len)
{
    (void)args;
    (void)len;
    return FF_SUCCESS;
}

static FF_Status_t r3_fn_bad(const uint8_t *args, uint16_t len)
{
    (void)args;
    (void)len;
    return FF_ERROR;
}

static void test_pus_param_s20_roundtrip(void)
{
    const uint8_t setter_id = 0x31u;
    const uint8_t provider_id = 0x32u;
    uint8_t frame[128];
    uint16_t flen;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;
    uint8_t payload[8];

    r3_push_setup();
    g_param_len = 0u;
    FF_TEST_ASSERT(FF_PusRegisterParamSetter(setter_id, r3_param_set) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(provider_id, r3_param_get) == FF_SUCCESS);

    /* Set parameter (20,1). */
    payload[0] = setter_id;
    payload[1] = 0xDEu;
    payload[2] = 0xADu;
    payload[3] = 0xBEu;
    payload[4] = 0xEFu;
    flen = encode_tc(FF_PUS_SERVICE_PARAMETER, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     payload, 5u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(1u, 7u, &d, &dl));

    /* Report parameter (20,3) -> (20,4) with the stored image. */
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_PARAMETER, 3u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     &provider_id, 1u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(20u, 4u, &d, &dl));
    FF_TEST_ASSERT(dl == 5u);
    FF_TEST_ASSERT(d[0] == provider_id);
    FF_TEST_ASSERT((d[1] == 0xDEu) && (d[2] == 0xADu) && (d[3] == 0xBEu) && (d[4] == 0xEFu));

    /* Unknown parameter id -> completion failure. */
    rx_reset();
    payload[0] = 0x7Eu;
    flen = encode_tc(FF_PUS_SERVICE_PARAMETER, 3u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     payload, 1u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_FOUND);
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));
}

static void test_pus_function_s8(void)
{
    const uint8_t fn_ok_id = 0x41u;
    const uint8_t fn_bad_id = 0x42u;
    uint8_t frame[128];
    uint16_t flen;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;
    uint8_t payload[4];

    r3_push_setup();
    FF_TEST_ASSERT(FF_PusRegisterFunction(fn_ok_id, r3_fn_ok) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterFunction(fn_bad_id, r3_fn_bad) == FF_SUCCESS);

    /* Successful invocation -> (8,2) status 0. */
    payload[0] = fn_ok_id;
    payload[1] = 0x09u;
    payload[2] = 0x08u;
    flen = encode_tc(FF_PUS_SERVICE_FUNCTION, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     payload, 3u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_find(8u, 2u, &d, &dl));
    FF_TEST_ASSERT(dl == 2u);
    FF_TEST_ASSERT((d[0] == fn_ok_id) && (d[1] == 0u));

    /* Failing invocation -> (8,2) status 1 plus completion failure. */
    rx_reset();
    flen = encode_tc(FF_PUS_SERVICE_FUNCTION, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     &fn_bad_id, 1u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_ERROR);
    FF_TEST_ASSERT(rx_find(8u, 2u, &d, &dl));
    FF_TEST_ASSERT((d[0] == fn_bad_id) && (d[1] == 1u));
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));

    /* Unknown function id -> completion failure. */
    rx_reset();
    payload[0] = 0x5Au;
    flen = encode_tc(FF_PUS_SERVICE_FUNCTION, 1u, FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION,
                     payload, 1u, frame, sizeof(frame));
    FF_TEST_ASSERT(FF_PusIngest(frame, flen) == FF_NOT_FOUND);
    FF_TEST_ASSERT(rx_find(1u, 8u, &d, &dl));
}

/* ------------------------------------------------------------------ */
/* SRS-REQ-013 : Table Services atomic activate / revert               */
/* ------------------------------------------------------------------ */

typedef struct
{
    uint32_t magic;
    uint16_t gain;
    uint16_t bias;
} r3_cfg_t;

#define R3_CFG_MAGIC 0x52334346u /* 'R3CF' */

static uint8_t g_tbl_image[sizeof(r3_cfg_t)];

static FF_Status_t r3_tbl_validate(const void *img, size_t size)
{
    const r3_cfg_t *t = (const r3_cfg_t *)img;

    if ((size != sizeof(r3_cfg_t)) || (t->magic != R3_CFG_MAGIC)) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

static void test_tbl_atomic_activate_revert(void)
{
    r3_cfg_t staged;
    r3_cfg_t dumped;
    size_t dumped_len = sizeof(dumped);
    FF_TblHandle_t h;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);

    (void)memset(&g_tbl_image, 0, sizeof(g_tbl_image));
    h = FF_TblRegister(&(FF_Tbl_Def_t){.name = "r3_cfg",
                                       .image = g_tbl_image,
                                       .size = sizeof(g_tbl_image),
                                       .validate = r3_tbl_validate});
    FF_TEST_ASSERT(h != FF_TBL_INVALID_HANDLE);

    /* Invalid staged image: validation fails, nothing becomes activatable. */
    (void)memset(&staged, 0, sizeof(staged));
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_ERROR);
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_NOT_READY);

    /* Valid load stages but does not activate (atomicity, SRS-REQ-013). */
    staged.magic = R3_CFG_MAGIC;
    staged.gain = 100u;
    staged.bias = 7u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_SUCCESS);
    {
        const r3_cfg_t *cur = (const r3_cfg_t *)FF_TblGet(h, NULL);
        FF_TEST_ASSERT(cur->magic != R3_CFG_MAGIC); /* active image untouched */
    }

    FF_TEST_ASSERT(FF_TblActivate(h) == FF_SUCCESS);
    {
        const r3_cfg_t *cur = (const r3_cfg_t *)FF_TblGet(h, NULL);
        FF_TEST_ASSERT((cur->magic == R3_CFG_MAGIC) && (cur->gain == 100u));
    }

    /* A second staged load must not disturb the active image until activated. */
    staged.gain = 200u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_SUCCESS);
    {
        const r3_cfg_t *cur = (const r3_cfg_t *)FF_TblGet(h, NULL);
        FF_TEST_ASSERT(cur->gain == 100u);
    }

    /* Dump returns the active (not the staged) image. */
    (void)memset(&dumped, 0, sizeof(dumped));
    FF_TEST_ASSERT(FF_TblDump(h, &dumped, &dumped_len) == FF_SUCCESS);
    FF_TEST_ASSERT(dumped_len == sizeof(dumped));
    FF_TEST_ASSERT(dumped.gain == 100u);

    /* Regression (FRICK-REVIEW-0077 Major, SRS-REQ-013): after an activate, a
     * rejected load must invalidate the staged image - activate must NOT commit
     * the rejected image and must return FF_NOT_READY. */
    staged.magic = 0u; /* invalid magic -> validator rejects */
    staged.gain = 999u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_ERROR);
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_NOT_READY);
    {
        const r3_cfg_t *cur = (const r3_cfg_t *)FF_TblGet(h, NULL);
        FF_TEST_ASSERT(cur->gain == 100u); /* rejected image never committed */
    }

    /* Revert discards the staged image; activate then reports NOT_READY. */
    FF_TEST_ASSERT(FF_TblRevert(h) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_NOT_READY);
    {
        const r3_cfg_t *cur = (const r3_cfg_t *)FF_TblGet(h, NULL);
        FF_TEST_ASSERT((cur->magic == R3_CFG_MAGIC) && (cur->gain == 100u));
    }
}

/* ------------------------------------------------------------------ */
/* Test driver                                                         */
/* ------------------------------------------------------------------ */

FF_TEST_MAIN(test_sb_wildcard_fanout, test_sb_fifo_order, test_pus_verification_failures,
             test_pus_malformed_tc_event, test_pus_tm_counter, test_pus_hk_s3_lifecycle,
             test_pus_service9_set_report, test_pus_event_s5_propagation,
             test_pus_param_s20_roundtrip, test_pus_function_s8, test_tbl_atomic_activate_revert)
