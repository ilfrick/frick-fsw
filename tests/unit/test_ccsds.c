/*
 * frick-fsw - CCSDS codec unit tests
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "transport/ccsds/ccsds.h"
#include <string.h>

static void test_crc(void)
{
    const uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t c1 = FF_CcsdsCrc16(data, 4u);
    uint16_t c2 = FF_CcsdsCrc16(data, 4u);
    FF_TEST_ASSERT(c1 == c2);
    FF_TEST_ASSERT(c1 != 0u);
    /* CRC of the CRC must vanish: checksum property. */
    uint8_t with_crc[6];
    memcpy(with_crc, data, 4u);
    with_crc[4] = (uint8_t)(c1 >> 8);
    with_crc[5] = (uint8_t)(c1 & 0xFFu);
    FF_TEST_ASSERT(FF_CcsdsCrc16(with_crc, 6u) == 0u);
}

static void test_tm_roundtrip(void)
{
    uint8_t out[128];
    uint16_t out_len = 0u;
    uint16_t apid = 0u;
    FF_Pus_TmHdr_t tm;
    const uint8_t *data = NULL;
    uint16_t data_len = 0u;

    memset(&tm, 0, sizeof(tm));
    tm.version = 2u;
    tm.service = 3u;
    tm.subservice = 25u;
    tm.counter = 7u;
    tm.destination_id = 1u;
    tm.time_coarse_s = 1234u;
    tm.time_fine = 0x8000u;

    const uint8_t payload[] = {0xAA, 0xBB, 0xCC};
    FF_TEST_ASSERT(FF_CcsdsEncodeTm(0x020u, &tm, payload, sizeof(payload), out, sizeof(out),
                                    &out_len) == FF_SUCCESS);
    FF_TEST_ASSERT(out_len == (FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN + 3u + 2u));

    FF_TEST_ASSERT(FF_CcsdsDecodeTm(out, out_len, &apid, &tm, &data, &data_len) == FF_SUCCESS);
    FF_TEST_ASSERT(apid == 0x020u);
    FF_TEST_ASSERT(tm.service == 3u);
    FF_TEST_ASSERT(tm.subservice == 25u);
    FF_TEST_ASSERT(tm.counter == 7u);
    FF_TEST_ASSERT(tm.time_coarse_s == 1234u);
    FF_TEST_ASSERT(data_len == 3u);
    FF_TEST_ASSERT(memcmp(data, payload, 3u) == 0);
}

static void test_corruption_detected(void)
{
    uint8_t out[64];
    uint16_t out_len = 0u;
    FF_Pus_TmHdr_t tm;
    memset(&tm, 0, sizeof(tm));
    tm.version = 2u;
    tm.service = 17u;
    tm.subservice = 2u;

    FF_TEST_ASSERT(FF_CcsdsEncodeTm(0x020u, &tm, NULL, 0u, out, sizeof(out), &out_len) ==
                   FF_SUCCESS);
    out[10] ^= 0xFFu; /* corrupt a byte */
    uint16_t apid;
    FF_Pus_TmHdr_t d;
    const uint8_t *data;
    uint16_t dlen;
    FF_TEST_ASSERT(FF_CcsdsDecodeTm(out, out_len, &apid, &d, &data, &dlen) == FF_CRC_ERROR);
}

FF_TEST_MAIN(test_crc, test_tm_roundtrip, test_corruption_detected)
