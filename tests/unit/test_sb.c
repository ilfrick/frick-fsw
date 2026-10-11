/*
 * frick-fsw - Software Bus unit tests
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include <string.h>

static void test_delivery(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    FF_AppId_t a1 = 1u, a2 = 2u;
    FF_TEST_ASSERT(FF_SbCreatePipe(a1, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(a2, 4u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbSubscribe(0x0A01u, a1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A01u, a2) == FF_SUCCESS);

    FF_Msg_t m;
    memset(&m, 0, sizeof(m));
    m.msg_id = 0x0A01u;
    m.length = 2u;
    m.payload[0] = 0xDEu;
    m.payload[1] = 0xADu;
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);

    FF_Msg_t r;
    memset(&r, 0, sizeof(r));
    FF_TEST_ASSERT(FF_SbReceive(a1, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x0A01u);
    FF_TEST_ASSERT(r.length == 2u);
    FF_TEST_ASSERT(r.payload[0] == 0xDEu && r.payload[1] == 0xADu);

    FF_TEST_ASSERT(FF_SbReceive(a2, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x0A01u);

    /* No message left in queues on a non matching ID. */
}

static void test_filtering(void)
{
    FF_AppId_t a3 = 3u;
    FF_TEST_ASSERT(FF_SbCreatePipe(a3, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A02u, a3) == FF_SUCCESS);

    FF_Msg_t m;
    memset(&m, 0, sizeof(m));
    m.msg_id = 0x0A03u; /* different id: must NOT be delivered */
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);

    FF_Msg_t r;
    FF_TEST_ASSERT(FF_SbReceive(a3, &r, 10u) == FF_TIMEOUT);
}

static void test_stats(void)
{
    FF_SbStats_t s;
    FF_TEST_ASSERT(FF_SbGetStats(&s) == FF_SUCCESS);
    FF_TEST_ASSERT(s.published >= 2u);
    FF_TEST_ASSERT(s.delivered >= 2u);
}

FF_TEST_MAIN(test_delivery, test_filtering, test_stats)
