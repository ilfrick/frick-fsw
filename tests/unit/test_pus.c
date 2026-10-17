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
        FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_ERROR);
        FF_TEST_ASSERT(!stream_has(6u, 6u));
        FF_TEST_ASSERT(stream_has(1u, 8u)); /* completion failure */
    }

    /* Unknown region identifiers are rejected, even when the offset fits. */
    g_rx_count = 0u;
    payload[0] = 2u;
    wr_u32be(&payload[1], 0u);
    wr_u16be(&payload[5], 4u);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_ERROR);
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

FF_TEST_MAIN(test_ping_roundtrip, test_memory_dump_bounded, test_service9_time)
