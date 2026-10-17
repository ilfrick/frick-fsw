/*
 * frick-fsw - N7 PUS adapter test against generated ASN1SCC/N7 sources.
 *
 * This target is built only when FF_USE_PUS_ASN1 is enabled. It links the real
 * generated ASN1SCC runtime and N7/ESA CRC wrapper through frick_fsw_core; no
 * controlled replacement headers or CRC doubles are used.
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"

#include <string.h>

static uint8_t g_tc[128];

static void make_tc(uint16_t *length)
{
    FF_Pus_TcHdr_t hdr;
    const uint8_t payload[] = {0x31u, 0x42u};

    memset(&hdr, 0, sizeof(hdr));
    hdr.version = FF_PUS_VERSION;
    hdr.ack_flags = FF_PUS_ACK_COMPLETION;
    hdr.service = FF_PUS_SERVICE_TEST;
    hdr.subservice = 1u;
    hdr.source_id = 0x1234u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &hdr, payload, sizeof(payload), g_tc, sizeof(g_tc),
                                    length) == FF_SUCCESS);
}

static void test_n7_tc_decode_with_generated_crc(void)
{
    FF_Pus_TcHdr_t hdr;
    const uint8_t *data = NULL;
    uint16_t apid = 0u;
    uint16_t data_len = 0u;
    uint16_t length = 0u;

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendN7) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusGetBackend() == &FF_PusBackendN7);
    make_tc(&length);

#if FF_FRAME_WITH_CRC
    /* Valid TC encoded by the reference codec must pass the generated N7/ESA
     * CRC implementation and preserve the decoded PUS header and payload. */
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, length, &apid, &hdr, &data, &data_len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(apid == FF_APID_TC);
    FF_TEST_ASSERT(hdr.service == FF_PUS_SERVICE_TEST && hdr.subservice == 1u);
    FF_TEST_ASSERT(data_len == 2u && data[0] == 0x31u && data[1] == 0x42u);

    /* A one-bit trailer corruption is rejected by the real adapter path. */
    g_tc[length - 1u] ^= 0x01u;
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, length, &apid, &hdr, &data, &data_len) ==
                   FF_CRC_ERROR);
    g_tc[length - 1u] ^= 0x01u;

    /* Structural boundary/type errors precede CRC calculation and are rejected. */
    g_tc[length] = 0u;
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, (uint16_t)(length + 1u), &apid, &hdr, &data,
                                             &data_len) == FF_PARSE_ERROR);
    g_tc[0] ^= 0x10u;
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, length, &apid, &hdr, &data, &data_len) ==
                   FF_PARSE_ERROR);
    g_tc[0] ^= 0x10u;
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, length, &apid, NULL, &data, &data_len) ==
                   FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, 5u, &apid, &hdr, &data, &data_len) ==
                   FF_PARSE_ERROR);
    {
        uint8_t saved_hi = g_tc[4];
        uint8_t saved_lo = g_tc[5];
        g_tc[4] = 0u;
        g_tc[5] = 0u;
        FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, length, &apid, &hdr, &data, &data_len) ==
                       FF_PARSE_ERROR);
        g_tc[4] = saved_hi;
        g_tc[5] = saved_lo;
    }
#else
    /* A CRC-disabled configuration is deliberately not the project's default;
     * this branch documents the compile-time tailoring but does not assert the
     * SRS-REQ-004 CRC-required configuration. */
    FF_TEST_ASSERT(FF_PusBackendN7.decode_tc(g_tc, length, &apid, &hdr, &data, &data_len) ==
                   FF_SUCCESS);
#endif
}

static void test_n7_tm_uses_generated_crc(void)
{
    FF_Pus_TmHdr_t hdr;
    uint8_t payload[] = {0xA5u, 0x5Au};
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    uint16_t decoded_apid = 0u;
    FF_Pus_TmHdr_t decoded_hdr;
    const uint8_t *decoded_data = NULL;
    uint16_t decoded_len = 0u;

    memset(&hdr, 0, sizeof(hdr));
    hdr.version = FF_PUS_VERSION;
    hdr.service = FF_PUS_SERVICE_HK;
    hdr.subservice = 25u;
    hdr.counter = 9u;
    hdr.destination_id = 0x1234u;
    hdr.time_coarse_s = 17u;
    hdr.time_fine = 0x8000u;

    FF_TEST_ASSERT(FF_PusBackendN7.encode_tm(&hdr, payload, sizeof(payload), frame, sizeof(frame),
                                             &frame_len) == FF_SUCCESS);
#if FF_FRAME_WITH_CRC
    /* CRC trailer compatibility check: the trailer emitted by the generated N7
     * CRC routine is compared against the reference codec CRC routine
     * (`FF_CcsdsCrc16`). This is a compatibility check, not an independent CRC
     * implementation (both follow the project CRC semantics), so it cannot
     * detect a common-mode defect in that routine. Then decode the complete
     * frame through the packet codec. */
    FF_TEST_ASSERT(frame_len >= 2u);
    {
        uint16_t expected = FF_CcsdsCrc16(frame, (uint16_t)(frame_len - 2u));
        uint16_t actual =
            (uint16_t)(((uint16_t)frame[frame_len - 2u] << 8) | (uint16_t)frame[frame_len - 1u]);
        FF_TEST_ASSERT(actual == expected);
    }
#endif
    FF_TEST_ASSERT(FF_CcsdsDecodeTm(frame, frame_len, &decoded_apid, &decoded_hdr, &decoded_data,
                                    &decoded_len) == FF_SUCCESS);
    FF_TEST_ASSERT(decoded_apid == FF_APID_TM);
    FF_TEST_ASSERT(decoded_hdr.service == hdr.service && decoded_hdr.subservice == hdr.subservice);
    FF_TEST_ASSERT(decoded_len == sizeof(payload));
    FF_TEST_ASSERT(memcmp(decoded_data, payload, sizeof(payload)) == 0);
}

FF_TEST_MAIN(test_n7_tc_decode_with_generated_crc, test_n7_tm_uses_generated_crc)
