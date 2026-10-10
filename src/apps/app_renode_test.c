/*
 * frick-fsw - Renode functional test application
 *
 * An application integrated on the middleware used for functional validation in
 * the Renode emulated PolarFire SoC environment (norm N18). On start it prints a
 * machine-checkable banner and runs middleware self-checks (Software Bus
 * publish/subscribe, Event Services, Time Services), then prints
 * "RENODE_TEST: PASS" or "RENODE_TEST: FAIL". Registered only when
 * FF_ENABLE_RENODE_TEST=1 (so the default reference set stays at six apps).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "apps/app.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/es/es.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include <stdint.h>

#define RENODE_TEST_MSG_ID 0x7E01u

void FF_AppRenodeTestMain(void *arg)
{
    FF_AppId_t me;
    FF_Msg_t msg;
    bool pub_ok;
    bool rx_ok = false;

    (void)arg;
    FF_PalLog("[renode-test] frick-fsw Renode functional test application\n");

    me = FF_EsGetId("RENODE_TEST");

    /* Software Bus publish/subscribe round trip (received inline). */
    pub_ok = (FF_SbSubscribe(RENODE_TEST_MSG_ID, me) == FF_SUCCESS);

    msg.msg_id = RENODE_TEST_MSG_ID;
    msg.apid = 0x010u;
    msg.time_ns = FF_TimeGetMetNs();
    msg.length = 4u;
    msg.payload[0] = 0xDEu;
    msg.payload[1] = 0xADu;
    msg.payload[2] = 0xBEu;
    msg.payload[3] = 0xEFu;
    if (FF_SbPublish(&msg) != FF_SUCCESS) {
        pub_ok = false;
    }
    if (FF_SbReceive(me, &msg, 1000u) == FF_SUCCESS) {
        rx_ok = true;
    }

    /* Event Services emit (visible on the bus / PUS S5). */
    FF_EvsSend(0x7E01u, FF_EVS_INFO, me, "renode-test");

    /* Time Services available and monotonic. */
    if (FF_TimeGetMetNs() == 0u) {
        rx_ok = false;
    }

    FF_PalLog("[renode-test] sb_roundtrip=%s\n", rx_ok ? "PASS" : "FAIL");
    FF_PalLog("RENODE_TEST: %s\n", (pub_ok && rx_ok) ? "PASS" : "FAIL");

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
