/*
 * frick-fsw - PUS layer integration test
 *
 * Builds a CCSDS TC (PUS service 17, connection test / ping) with the open
 * backend, feeds it to FF_PusIngest and checks the emitted telemetry stream
 * contains the acceptance (1,1), completion (1,7) and ping reply (17,2)
 * packets.
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

static uint8_t g_rx[8 * 128];
static uint16_t g_rx_count;
static uint8_t g_region[64];

static bool stream_has_memory_bytes(uint32_t offset, uint16_t length)
{
    uint16_t pos = 0u;
    while ((pos + 6u) <= g_rx_count) {
        uint16_t packet_pdl = (uint16_t)(((uint16_t)g_rx[pos + 4u] << 8) | g_rx[pos + 5u]);
        uint16_t packet_len = (uint16_t)(6u + packet_pdl + 1u);
        if (packet_len > (g_rx_count - pos)) {
            return false;
        }
        uint16_t apid = 0u;
        FF_Pus_TmHdr_t hdr;
        const uint8_t *data = NULL;
        uint16_t data_len = 0u;
        if ((FF_CcsdsDecodeTm(&g_rx[pos], packet_len, &apid, &hdr, &data, &data_len) ==
             FF_SUCCESS) &&
            (hdr.service == FF_PUS_SERVICE_MEMORY) && (hdr.subservice == 6u) &&
            (data_len == length) && (memcmp(data, &g_region[offset], length) == 0)) {
            return true;
        }
        pos = (uint16_t)(pos + packet_len);
    }
    return false;
}

static void tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx)) {
        memcpy(&g_rx[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }
}

static bool stream_has(uint8_t service, uint8_t subservice)
{
    /* TM secondary header service/subservice are at offsets 7 and 8. */
    uint16_t pos = 0u;
    while (pos < g_rx_count) {
        if (g_rx[pos + 7] == service && g_rx[pos + 8] == subservice) {
            return true;
        }
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx[pos + 4] << 8) | g_rx[pos + 5]);
        pos = (uint16_t)(pos + 6u + pdl + 1u);
    }
    return false;
}

/* Count the (service, subservice) telemetry packets in the captured stream. */
static uint16_t stream_count(uint8_t service, uint8_t subservice)
{
    uint16_t pos = 0u;
    uint16_t count = 0u;
    while ((pos + 9u) <= g_rx_count) {
        if ((g_rx[pos + 7] == service) && (g_rx[pos + 8] == subservice)) {
            count = (uint16_t)(count + 1u);
        }
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx[pos + 4] << 8) | g_rx[pos + 5]);
        uint16_t step = (uint16_t)(6u + pdl + 1u);
        if ((step < 7u) || ((uint16_t)(pos + step) <= pos)) {
            break;
        }
        pos = (uint16_t)(pos + step);
    }
    return count;
}

static void test_ping_roundtrip(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);
    g_rx_count = 0u;

    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_START | FF_PUS_ACK_COMPLETION;
    tc.service = FF_PUS_SERVICE_TEST; /* 17 */
    tc.subservice = 1u;               /* ping */
    tc.source_id = 0x0102u;

    const uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t frame[128];
    uint16_t frame_len = 0u;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);

    /* Expected TM sequence: 1,1 acceptance - 1,3 start - 1,7 completion - 17,2 reply */
    FF_TEST_ASSERT(stream_has(1u, 1u));
    FF_TEST_ASSERT(stream_has(1u, 3u));
    FF_TEST_ASSERT(stream_has(1u, 7u));
    FF_TEST_ASSERT(stream_has(17u, 2u));

#if defined(FF_USE_PUS_ASN1) && (FF_USE_PUS_ASN1 == 1)
    /* SRS-REQ-050: the configured default N7 adapter must preserve reference
     * framing. The pinned generated common types are built into this target;
     * full service-specific ASN.1 payload mapping remains planned. */
    {
        FF_Pus_TmHdr_t h;
        uint8_t a[128];
        uint8_t b[128];
        uint16_t la = 0u;
        uint16_t lb = 0u;

        memset(&h, 0, sizeof(h));
        h.version = FF_PUS_VERSION;
        h.service = FF_PUS_SERVICE_TEST;
        h.subservice = 2u;
        FF_TEST_ASSERT(FF_PusBackendOpen.encode_tm(&h, payload, sizeof(payload), a, sizeof(a),
                                                   &la) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusBackendN7.encode_tm(&h, payload, sizeof(payload), b, sizeof(b), &lb) ==
                       FF_SUCCESS);
        FF_TEST_ASSERT(la == lb);
        FF_TEST_ASSERT(la > 0u);
        FF_TEST_ASSERT(memcmp(a, b, la) == 0);
        /* Verify explicit adapter selection; fsw_main selects N7 when the
         * default FF_USE_PUS_ASN1=ON configuration is built. */
        FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendN7) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusGetBackend() == &FF_PusBackendN7);
        FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    }
#endif
}

/* ---- Service 6 : logical-region dump and byte-offset bounds (SRS-REQ-009) */

/* The PUS codec reads multi-byte TC fields in network (big-endian) order. */
static void wr_u32be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 24) & 0xFFu);
    p[1] = (uint8_t)((v >> 16) & 0xFFu);
    p[2] = (uint8_t)((v >> 8) & 0xFFu);
    p[3] = (uint8_t)(v & 0xFFu);
}

static void wr_u16be(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)((v >> 8) & 0xFFu);
    p[1] = (uint8_t)(v & 0xFFu);
}

static void test_memory_dump_bounded(void)
{
    uint8_t payload[7];
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    FF_Pus_TcHdr_t tc;

    /* Fill the region with a deterministic, index-dependent pattern first.
     * With the previous zero-filled static the content oracle compared zeros
     * against zeros, so a handler reading the wrong offset still matched
     * (independent V&V finding F02: mutant "offset ignored" survived). */
    for (uint16_t i = 0u; i < sizeof(g_region); i++) {
        g_region[i] = (uint8_t)(0xA5u ^ (uint8_t)(i & 0xFFu));
    }

    FF_TEST_ASSERT(FF_PusRegisterMemoryRegion(1u, g_region, sizeof(g_region)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterMemoryRegion(1u, g_region, sizeof(g_region)) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(FF_PusRegisterMemoryRegion(0u, g_region, sizeof(g_region)) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusRegisterMemoryRegion(2u, NULL, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusRegisterMemoryRegion(2u, g_region, 0u) == FF_INVALID_ARGUMENT);

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = FF_PUS_SERVICE_MEMORY; /* 6 */
    tc.subservice = 5u;                 /* dump (reply TM is 6,6) */
    tc.source_id = 0x0102u;

    /* Provisional profile: region ID (1), byte offset (4), and size (2).
     * In-region dump succeeds without serializing a host pointer. */
    g_rx_count = 0u;
    payload[0] = 1u;
    wr_u32be(&payload[1], 4u);
    wr_u16be(&payload[5], 4u);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(6u, 6u));
    FF_TEST_ASSERT(stream_has_memory_bytes(4u, 4u));

    /* The full region and the final byte are both valid boundaries. */
    g_rx_count = 0u;
    payload[0] = 1u;
    wr_u32be(&payload[1], 0u);
    wr_u16be(&payload[5], sizeof(g_region));
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(6u, 6u));
    FF_TEST_ASSERT(stream_has_memory_bytes(0u, sizeof(g_region)));
    g_rx_count = 0u;
    wr_u32be(&payload[1], sizeof(g_region) - 1u);
    wr_u16be(&payload[5], 1u);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(6u, 6u));
    FF_TEST_ASSERT(stream_has_memory_bytes(sizeof(g_region) - 1u, 1u));

    /* Offset+length outside region is rejected: no dump TM, completion failure. */
    {
        g_rx_count = 0u;
        payload[0] = 1u;
        wr_u32be(&payload[1], 62u);
        wr_u16be(&payload[5], 4u);
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(!stream_has(6u, 6u));
        FF_TEST_ASSERT(stream_has(1u, 8u)); /* completion failure */
    }

    /* ---- NC-EXT-18: the two refusal causes must be distinguishable in code.
     * The wire behaviour is unchanged by design (ICD §4: both causes surface as
     * S1(1,8) with FF_PUS_FAIL_EXECUTION); only the software-visible status
     * differs, so a caller can tell "no such region" from "outside the region".
     * Adversarial offsets are chosen so that a naive 32-bit `offset + length`
     * would wrap and pass the check. ---- */
    {
        FF_Status_t st_unknown;
        FF_Status_t st_out_of_range;

        /* Unknown region id, offset that would fit the registered region. */
        g_rx_count = 0u;
        payload[0] = 0xFFu;
        wr_u32be(&payload[1], 0u);
        wr_u16be(&payload[5], 4u);
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        st_unknown = FF_PusIngest(frame, frame_len);

        /* Registered region, first offset past the end. */
        g_rx_count = 0u;
        payload[0] = 1u;
        wr_u32be(&payload[1], sizeof(g_region));
        wr_u16be(&payload[5], 1u);
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        st_out_of_range = FF_PusIngest(frame, frame_len);

        FF_TEST_ASSERT(st_unknown == FF_NOT_FOUND);
        FF_TEST_ASSERT(st_out_of_range == FF_INVALID_ARGUMENT);
        /* The defect under NC-EXT-18: both used to be FF_ERROR. */
        FF_TEST_ASSERT(st_unknown != st_out_of_range);

        /* Region resolution precedes the range check: an unknown region with an
         * adversarial offset is still NOT_FOUND, never a range refusal. */
        g_rx_count = 0u;
        payload[0] = 0xFFu;
        wr_u32be(&payload[1], 0xFFFFFFFFu);
        wr_u16be(&payload[5], 4u);
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
        FF_TEST_ASSERT(!stream_has(6u, 6u));

        /* Adversarial in-region offsets: `offset + length` would overflow 32 bits,
         * so only a subtraction-based check refuses them. None may read memory or
         * emit a dump. */
        static const uint32_t adversarial_offsets[3] = {0xFFFFFFFFu, 0xFFFFFFFEu, 0xFFFFFFFDu};
        for (uint16_t k = 0u; k < 3u; k++) {
            g_rx_count = 0u;
            payload[0] = 1u;
            wr_u32be(&payload[1], adversarial_offsets[k]);
            wr_u16be(&payload[5], 4u);
            FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                            sizeof(frame), &frame_len) == FF_SUCCESS);
            FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT);
            FF_TEST_ASSERT(!stream_has(6u, 6u));
            FF_TEST_ASSERT(stream_has(1u, 8u));
        }

        /* Last valid byte is still accepted (the boundary is not off by one). */
        g_rx_count = 0u;
        payload[0] = 1u;
        wr_u32be(&payload[1], sizeof(g_region) - 1u);
        wr_u16be(&payload[5], 1u);
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(stream_has_memory_bytes(sizeof(g_region) - 1u, 1u));

        /* One byte past the end with a valid length is refused. */
        g_rx_count = 0u;
        payload[0] = 1u;
        wr_u32be(&payload[1], sizeof(g_region) - 1u);
        wr_u16be(&payload[5], 2u);
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(!stream_has(6u, 6u));
    }

    /* Unknown region identifiers are rejected, even when the offset fits. */
    g_rx_count = 0u;
    payload[0] = 2u;
    wr_u32be(&payload[1], 0u);
    wr_u16be(&payload[5], 4u);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_NOT_FOUND);
    FF_TEST_ASSERT(!stream_has(6u, 6u));
    FF_TEST_ASSERT(stream_has(1u, 8u));

    /* Zero-length dumps are rejected, including at offset zero. */
    g_rx_count = 0u;
    payload[0] = 1u;
    wr_u32be(&payload[1], 0u);
    wr_u16be(&payload[5], 0u);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_BAD_SIZE);
    FF_TEST_ASSERT(!stream_has(6u, 6u));
    FF_TEST_ASSERT(stream_has(1u, 8u));

    /* Oversized data requests are rejected before any memory read. */
    g_rx_count = 0u;
    payload[0] = 1u;
    wr_u32be(&payload[1], 0u);
    wr_u16be(&payload[5], (uint16_t)(FF_PUS_MAX_TM_FRAME + 1u));
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_BAD_SIZE);
    FF_TEST_ASSERT(!stream_has(6u, 6u));
}

/* ---- Service 9 : time management (SRS-REQ-010, R3 NC-1) -------------
 * PUS-C on-wire time uses the 6-octet CUC T-field with the P-field implicit.
 * A 7-octet input that includes an explicit P-field (the self-describing form
 * produced by FF_TimeCucEncode) is rejected so that the P byte can never be
 * mis-parsed as the MSB of the coarse field. */

static void test_service9_time(void)
{
    uint8_t payload[7];
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    FF_Pus_TcHdr_t tc;
    uint32_t coarse = 0u;
    uint16_t fine = 0u;

    FF_TimeInit();

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = FF_PUS_SERVICE_TIME; /* 9 */
    tc.subservice = 1u;               /* set time */
    tc.source_id = 0x0102u;

    /* Valid 6-octet T-field: coarse 5 s, fine 0. */
    g_rx_count = 0u;
    wr_u32be(&payload[0], 5u);
    wr_u16be(&payload[4], 0u);
    frame_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, 6u, frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(stream_has(1u, 1u));
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 5u); /* deterministic: whole-second margin */

    /* 7-octet input leading with an explicit P-field (0x2E) must be rejected
     * with FF_BAD_SIZE: if it were parsed, 0x2E would become coarse MSB. */
    g_rx_count = 0u;
    payload[0] = 0x2Eu;
    wr_u32be(&payload[1], 0x000000FFu); /* would mis-read as coarse 255 */
    wr_u16be(&payload[5], 0u);
    frame_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, 7u, frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_BAD_SIZE);
    FF_TEST_ASSERT(stream_has(1u, 8u)); /* completion failure */
    /* time unchanged: coarse not overwritten by the attempted mis-parse */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 5u);
}

/* ---- S1 Service 1 verification: missing subservices (NC-EXT-15) ---------- */

/* Test that start failure (subservice 4) and progress failure (subservice 6)
 * are correctly emitted by FF_PusSendVerification. */
static void test_verification_missing_subservices(void)
{
    /* Setup: initialize PUS with the open backend. */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* Test 1: Start failure (subservice 4) - stage=START, success=0 */
    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_START, 0u, 0u) == FF_SUCCESS);

    /* Test 2: Progress failure (subservice 6) - stage=PROGRESS, success=0 */
    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_PROGRESS, 0u, 0u) == FF_SUCCESS);

    /* Test 3: Verify existing subservices still work */
    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_ACCEPTANCE, 1u, 0u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_ACCEPTANCE, 0u, 0u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_START, 1u, 0u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_PROGRESS, 1u, 0u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_COMPLETION, 1u, 0u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_PusSendVerification(&(FF_Pus_TcHdr_t){.version = FF_PUS_VERSION,
                                                            .ack_flags = 0,
                                                            .service = FF_PUS_SERVICE_VERIFICATION,
                                                            .subservice = 0,
                                                            .source_id = 0x0102u},
                                          FF_PUS_STAGE_COMPLETION, 0u, 0u) == FF_SUCCESS);
}

/* ---- S3 Service 3 : periodic housekeeping emission (NC-EXT-17) ----- */

/* Dummy HK provider that returns fixed data. */
static FF_Status_t dummy_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
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

static void test_hk_periodic_emission(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    FF_PusSetTmSink(tm_sink);
    g_rx_count = 0u;

    /* Register HK provider. */
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(1u, dummy_hk_provider) == FF_SUCCESS);

    /* Deterministic fixture base time so the due instant is exactly testable.
     * Synthetic test value, not a mission time parameter. */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(1000ULL * 1000000000ULL) == FF_SUCCESS);

    /* Create HK report with period = 10 seconds. */
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {1u, 1u, 0u, 0u,
                              0u, 10u}; /* report_id=1, provider_id=1, period_s=10 (big-endian) */
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

    /* Enable the report (initializes last_emission_time to now). */
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[1] = {1u}; /* report_id=1 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 5u; /* enable */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }

    /* Negative: at the enable instant the period has not elapsed, nothing is due. */
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Positive: at the due instant (base 1000 s + period 10 s) exactly one
     * (3,25) housekeeping report is emitted and the call reports success. */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(1010ULL * 1000000000ULL) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);

    /* Re-arm: the last-emission time advanced, so an immediate second call is
     * not due and emits nothing (a stale timestamp would double-emit). */
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Second period: one further period elapsed emits again. */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(1020ULL * 1000000000ULL) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);

    /* One-shot report request (TC[3,27]) should still work regardless of periodic state. */
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[2] = {1u, 1u}; /* N=1, report_id=1 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 27u; /* report request */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }

    /* Disable the report. */
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[1] = {1u}; /* report_id=1 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 6u; /* disable */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }

    /* Periodic emission must not emit a disabled report. */
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Report with period_s=0 (one-shot only) should not be emitted periodically. */
    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[6] = {2u, 1u, 0u, 0u, 0u, 0u}; /* report_id=2, provider_id=1, period_s=0 */
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

    {
        FF_Pus_TcHdr_t tc;
        uint8_t payload[1] = {2u}; /* report_id=2 */
        uint8_t frame[128];
        uint16_t frame_len = 0u;
        memset(&tc, 0, sizeof(tc));
        tc.version = FF_PUS_VERSION;
        tc.ack_flags = 0u;
        tc.service = FF_PUS_SERVICE_HK;
        tc.subservice = 5u; /* enable */
        tc.source_id = 0x0102u;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame,
                                        sizeof(frame), &frame_len) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
    }

    /* Periodic emission must not emit a period_s=0 (one-shot only) report,
     * even long after any period would have elapsed. */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(5000ULL * 1000000000ULL) == FF_SUCCESS);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);
}

/* ---- S3 Service 3 : wrap/step regression for the periodic due test
 *      (NC-EXT-207) ------------------------------------------------ */

/* Sends a TC to the housekeeping service with the given subservice/payload. */
static void hk_send_tc(uint8_t subservice, const uint8_t *payload, uint16_t payload_len)
{
    FF_Pus_TcHdr_t tc;
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = 0u;
    tc.service = FF_PUS_SERVICE_HK;
    tc.subservice = subservice;
    tc.source_id = 0x0102u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
}

/* Sets the synthetic time so that FF_TimeGetCuc reads exactly
 * (coarse, fine). fine must be a multiple of 128: the fine field is
 * (ns % 1e9) * 65536 / 1e9, which on the integer 1e9 ns grid is exact only in
 * steps of 128. Synthetic test instants, not mission time parameters. */
static void set_time_cuc(uint32_t coarse, uint16_t fine)
{
    uint64_t ns = (uint64_t)coarse * 1000000000ULL;
    ns += ((uint64_t)fine * 1000000000ULL) / 65536ULL;
    FF_TEST_ASSERT(FF_TimeSetUtcNs(ns) == FF_SUCCESS);
}

/* Common PusInit preamble for the NC-EXT-207 tests; returns nothing. */
static void hk_wrap_test_setup(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(tm_sink);
    g_rx_count = 0u;
}

/* Coarse-wrap straddle: the due instant (last + 10) wraps below last, so a
 * raw coarse comparison reports the report as immediately due. Expected:
 * one emission exactly at the wrapped due instant (u == period), and at
 * most one emission per period thereafter (no emit-on-every-call burst).
 * Also pins the sub-second tie-break (u == period is due only when
 * now_fine >= last_fine) and the late-caller case (u > period). */
static void test_hk_coarse_wrap_due(void)
{
    hk_wrap_test_setup();
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(4u, dummy_hk_provider) == FF_SUCCESS);

    /* Base instant one coarse tick below the 32-bit wrap, fine = 8192
     * (exact dyadic encoding: 125,000,000 ns fraction). Synthetic test
     * instant, not a mission time parameter. */
    set_time_cuc(0xFFFFFFFEu, 8192u);

    {
        const uint8_t payload[6] = {10u, 4u, 0u, 0u, 0u, 10u}; /* id=10, prov=4, period=10 s */
        hk_send_tc(1u, payload, sizeof(payload));
    }
    {
        const uint8_t payload[1] = {10u};
        hk_send_tc(5u, payload, 1u); /* enable: last_emission = 0xFFFFFFFE:8192 */
    }

    /* NC-EXT-207 reproduction point: the wrapped due instant is coarse 8,
     * numerically below now (0xFFFFFFFE). The pre-fix raw comparison
     * reported the report due at every call until the pre-wrap window
     * passed; the due test must instead wait for u == 10. */
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Still not due after +5 s and +9 s (u = 5, u = 9 < period). */
    set_time_cuc(3u, 8192u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);
    set_time_cuc(7u, 8192u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Exactly u == period: due only when now_fine >= last_fine. The
     * sub-second tie-break of the (coarse, fine) comparison is preserved:
     * fine 4096 < last fine 8192 -> not yet due. */
    set_time_cuc(8u, 4096u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Same coarse tick, fine now equal to the last emission's fine: the
     * wrapped due instant emits exactly once. */
    set_time_cuc(8u, 8192u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);

    /* Re-arm: an immediate re-call is not due (no emit-on-every-call burst,
     * which is the TM-flood failure mode this correction removes). */
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Late caller: after idle time far past the due instant (u = 13 > 10),
     * exactly one emission, and the immediate re-call is not due. */
    set_time_cuc(21u, 8192u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Second period: after the late-caller emission re-based
     * last_emission to coarse 21, the next due instant is coarse 31
     * (u == 10 again, measured from the re-based instant); one emission. */
    set_time_cuc(31u, 8192u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);

    /* Park the report so later tests in this binary are unaffected. */
    {
        const uint8_t payload[1] = {10u};
        hk_send_tc(6u, payload, 1u);
    }
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);
}

/* Large unvalidated period: period_s = 0xFFFFFFFF wraps the due instant to
 * last - 1, so the pre-fix comparison flooded TM on every call from a single
 * valid-format command. With the wrap-safe formulation the elapsed window is
 * capped below 2^31 s, so such a period is not decidable and holds the
 * report unserved (no emission, no flood). Maximum-period validation is not
 * made here: it remains authority-owned (NC-EXT-208, ESC-HK-06). */
static void test_hk_large_period_held_unserved(void)
{
    hk_wrap_test_setup();
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(5u, dummy_hk_provider) == FF_SUCCESS);

    set_time_cuc(1000u, 0u);
    {
        /* id=11, prov=5, period=0xFFFFFFFF (big-endian); the extreme value
         * must still be *accepted* by TC[3,1]: no range validation is added
         * by this fix. */
        const uint8_t payload[6] = {11u, 5u, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
        hk_send_tc(1u, payload, sizeof(payload));
    }
    {
        const uint8_t payload[1] = {11u};
        hk_send_tc(5u, payload, 1u); /* enable: last_emission = 1000:0 */
    }

    /* Three calls across three coarse ticks: never due under the wrap-safe
     * formulation (pre-fix: due = 999 < now -> emitted on every call). */
    for (uint32_t k = 0u; k < 3u; k++) {
        g_rx_count = 0u;
        FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
        FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);
        set_time_cuc(1001u + k, 0u);
    }

    /* Park the report so later tests in this binary are unaffected. */
    {
        const uint8_t payload[1] = {11u};
        hk_send_tc(6u, payload, 1u);
    }
}

/* Backward time step: the due test must wait for the interval to re-elapse
 * instead of treating a stepped-back now as overdue, and a step whose
 * modular elapsed lands in the undecided band (u >= 2^31 s) must be treated
 * conservatively as "wait" (identical resolution to a backward step).
 * Pre-fix behaviour for small steps was already benign; this is the
 * regression guard that kills the wrap-guard-eating mutation. */
static void test_hk_backward_step_no_emission(void)
{
    hk_wrap_test_setup();
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(6u, dummy_hk_provider) == FF_SUCCESS);

    set_time_cuc(2000u, 0u);
    {
        const uint8_t payload[6] = {12u, 6u, 0u, 0u, 0u, 10u}; /* id=12, prov=6, period=10 s */
        hk_send_tc(1u, payload, sizeof(payload));
    }
    {
        const uint8_t payload[1] = {12u};
        hk_send_tc(5u, payload, 1u); /* enable: last_emission = 2000:0 */
    }

    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Small backward step (now 5 s before last): wait, never emit. */
    set_time_cuc(1995u, 0u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Forward again: due again exactly at u == period. */
    set_time_cuc(2009u, 0u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);
    set_time_cuc(2010u, 0u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);

    /* Backward step into the undecided band: last = 2010, now = 1000, the
     * modular elapsed is 2^32 - 1010 >= 2^31 s, which a signed difference
     * cannot distinguish from a backward step of 1010 s. The conservative
     * resolution is "wait": no emission (a wrap-guard-eating mutant emits
     * here, and is killed by this assertion). */
    set_time_cuc(1000u, 0u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 0u);

    /* Forward again, long after the (re-scheduled) due instant: one
     * late-caller emission at u = 90 > period. */
    set_time_cuc(2100u, 0u);
    g_rx_count = 0u;
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(3u, 25u) == 1u);

    /* Park the report. */
    {
        const uint8_t payload[1] = {12u};
        hk_send_tc(6u, payload, 1u);
    }
}

/* ---- HM-P2 - forbidden refresh window (host model) ---------- */

FF_TEST_MAIN(test_ping_roundtrip, test_memory_dump_bounded, test_service9_time,
             test_verification_missing_subservices, test_hk_periodic_emission,
             test_hk_coarse_wrap_due, test_hk_large_period_held_unserved,
             test_hk_backward_step_no_emission)

/* ---- S1 Service 1 verification: missing subservices (NC-EXT-15) ---------- */

/* Test that start failure (subservice 4) and progress failure (subservice 6)
 * are correctly emitted by FF_PusSendVerification. */
