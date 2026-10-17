/*
 * frick-fsw - PUSopen adapter unit tests (SRS-REQ-065)
 *
 * The test compiles the adapter against a controlled API stub; it verifies the
 * adapter's input/output contract and primary-header APID/type checks only.
 * It does not verify compatibility with the proprietary PUSopen SDK or any
 * PUS-C profile/compliance claim.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "pus/pus.h"
#include <pusopen.h>
#include <string.h>

static PUSopen_Status_t g_init_status;
static PUSopen_Status_t g_encode_status;
static PUSopen_Status_t g_decode_status;
static size_t g_encode_written;
static PUSopen_Packet_t g_decode_packet;
static uint16_t g_decode_calls;
static PUSopen_Packet_t g_encode_packet;

PUSopen_Status_t PUSopen_Init(void)
{
    return g_init_status;
}

PUSopen_Status_t PUSopen_EncodeTm(const PUSopen_Packet_t *pkt, uint8_t *out, size_t out_cap,
                                  size_t *written)
{
    size_t i;

    if ((pkt == NULL) || (out == NULL) || (written == NULL)) {
        return PUSOPEN_ERROR;
    }
    g_encode_packet = *pkt;
    if (g_encode_status != PUSOPEN_OK) {
        return g_encode_status;
    }
    if (out_cap < pkt->data_len) {
        return PUSOPEN_ERROR;
    }
    for (i = 0u; i < pkt->data_len; i++) {
        out[i] = pkt->data[i];
    }
    *written = g_encode_written;
    return PUSOPEN_OK;
}

PUSopen_Status_t PUSopen_DecodeTc(const uint8_t *raw, uint16_t len, PUSopen_Packet_t *pkt)
{
    (void)raw;
    (void)len;
    g_decode_calls++;
    if (g_decode_status != PUSOPEN_OK) {
        return g_decode_status;
    }
    *pkt = g_decode_packet;
    return PUSOPEN_OK;
}

static void reset_stub(void)
{
    g_init_status = PUSOPEN_OK;
    g_encode_status = PUSOPEN_OK;
    g_decode_status = PUSOPEN_OK;
    g_encode_written = 3u;
    memset(&g_decode_packet, 0, sizeof(g_decode_packet));
    memset(&g_encode_packet, 0, sizeof(g_encode_packet));
    g_decode_calls = 0u;
}

static void make_tc_primary(uint8_t *raw, uint16_t apid)
{
    uint16_t word = (uint16_t)(0x1000u | (apid & 0x07FFu)); /* TC packet, version 0 */
    raw[0] = (uint8_t)(word >> 8);
    raw[1] = (uint8_t)word;
    raw[2] = 0u;
    raw[3] = 0u;
    raw[4] = 0u;
    raw[5] = 0u;
}

static void test_pusopen_init(void)
{
    reset_stub();
    FF_TEST_ASSERT(FF_PusBackendPusopen.init() == FF_SUCCESS);
    g_init_status = PUSOPEN_ERROR;
    FF_TEST_ASSERT(FF_PusBackendPusopen.init() == FF_ERROR);
}

static void test_pusopen_encode(void)
{
    FF_Pus_TmHdr_t hdr = {FF_PUS_VER_2, 5u, 3u, 0x1234u, 0x5678u, 0x11223344u, 0x7788u};
    uint8_t payload[3] = {0xA1u, 0xB2u, 0xC3u};
    uint8_t out[8] = {0u};
    uint16_t out_len = 0u;

    reset_stub();
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, payload, sizeof(payload), out, sizeof(out),
                                                  &out_len) == FF_SUCCESS);
    FF_TEST_ASSERT(out_len == 3u);
    FF_TEST_ASSERT(out[0] == 0xA1u && out[2] == 0xC3u);
    FF_TEST_ASSERT(g_encode_packet.service == hdr.service);
    FF_TEST_ASSERT(g_encode_packet.subservice == hdr.subservice);
    FF_TEST_ASSERT(g_encode_packet.counter == hdr.counter);
    FF_TEST_ASSERT(g_encode_packet.time_s == hdr.time_coarse_s);
    FF_TEST_ASSERT(g_encode_packet.time_f == hdr.time_fine);
    FF_TEST_ASSERT(g_encode_packet.data_len == sizeof(payload));

    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(NULL, payload, sizeof(payload), out, sizeof(out),
                                                  &out_len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, NULL, 1u, out, sizeof(out), &out_len) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, payload, sizeof(payload), NULL, sizeof(out),
                                                  &out_len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, payload, sizeof(payload), out, sizeof(out),
                                                  NULL) == FF_INVALID_ARGUMENT);

    g_encode_status = PUSOPEN_ERROR;
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, payload, sizeof(payload), out, sizeof(out),
                                                  &out_len) == FF_ERROR);
    g_encode_status = PUSOPEN_OK;
    g_encode_written = sizeof(out) + 1u;
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, payload, sizeof(payload), out, sizeof(out),
                                                  &out_len) == FF_BAD_SIZE);
    g_encode_written = (size_t)UINT16_MAX + 1u;
    FF_TEST_ASSERT(FF_PusBackendPusopen.encode_tm(&hdr, payload, sizeof(payload), out,
                                                  (size_t)UINT16_MAX + 1u,
                                                  &out_len) == FF_BAD_SIZE);
}

static void test_pusopen_decode(void)
{
    uint8_t raw[8] = {0u};
    uint8_t payload[2] = {0x11u, 0x22u};
    uint16_t apid = 0u;
    uint16_t data_len = 0u;
    const uint8_t *data = NULL;
    FF_Pus_TcHdr_t hdr;

    reset_stub();
    make_tc_primary(raw, FF_APID_TC);
    g_decode_packet.ack_flags = 0x0Au;
    g_decode_packet.service = 17u;
    g_decode_packet.subservice = 1u;
    g_decode_packet.source_id = 0x1234u;
    g_decode_packet.data = payload;
    g_decode_packet.data_len = sizeof(payload);

    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, sizeof(raw), &apid, &hdr, &data,
                                                  &data_len) == FF_SUCCESS);
    FF_TEST_ASSERT(apid == FF_APID_TC);
    FF_TEST_ASSERT(hdr.version == FF_PUS_VERSION);
    FF_TEST_ASSERT(hdr.ack_flags == 0x0Au);
    FF_TEST_ASSERT(hdr.service == 17u && hdr.subservice == 1u);
    FF_TEST_ASSERT(hdr.source_id == 0x1234u);
    FF_TEST_ASSERT(data == payload && data_len == sizeof(payload));

    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(NULL, sizeof(raw), &apid, &hdr, &data,
                                                  &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, 5u, &apid, &hdr, &data, &data_len) ==
                   FF_PARSE_ERROR);
    /* Local adapter contract (docs/architecture/pus-layer.md §§6–7): the
     * adapter rejects a non-TC packet or a mismatched APID before delegating to
     * the vendor SDK. This asserts the guard ordering of that local contract; it
     * is not mission/profile compliance verification and does not exercise the
     * vendor SDK path. */
    make_tc_primary(raw, (uint16_t)(FF_APID_TC + 1u));
    {
        uint16_t calls_before = g_decode_calls;
        FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, sizeof(raw), &apid, &hdr, &data,
                                                      &data_len) == FF_PARSE_ERROR);
        FF_TEST_ASSERT(g_decode_calls == calls_before); /* reject before SDK decode */
    }
    raw[0] &= 0xEFu; /* change packet type to TM */
    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, sizeof(raw), &apid, &hdr, &data,
                                                  &data_len) == FF_PARSE_ERROR);

    make_tc_primary(raw, FF_APID_TC);
    g_decode_status = PUSOPEN_ERROR;
    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, sizeof(raw), &apid, &hdr, &data,
                                                  &data_len) == FF_PARSE_ERROR);
    g_decode_status = PUSOPEN_OK;
    g_decode_packet.data = NULL;
    g_decode_packet.data_len = 1u;
    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, sizeof(raw), &apid, &hdr, &data,
                                                  &data_len) == FF_BAD_SIZE);
    g_decode_packet.data = payload;
    g_decode_packet.data_len = (size_t)UINT16_MAX + 1u;
    FF_TEST_ASSERT(FF_PusBackendPusopen.decode_tc(raw, sizeof(raw), &apid, &hdr, &data,
                                                  &data_len) == FF_BAD_SIZE);
}

FF_TEST_MAIN(test_pusopen_init, test_pusopen_encode, test_pusopen_decode)
