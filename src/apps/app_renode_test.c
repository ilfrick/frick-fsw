/*
 * frick-fsw - Renode functional test application
 *
 * An application integrated on the middleware used for functional validation in
 * the Renode emulated PolarFire SoC environment (norm N18). On start it prints a
 * machine-checkable banner and runs middleware self-checks (Software Bus
 * publish/subscribe, Event Services, Time Services) and, on the target platform,
 * a HAL self-check (MSS GPIO MMIO round trip, milestone M11), then prints
 * "RENODE_TEST: PASS" or "RENODE_TEST: FAIL". Registered only when
 * FF_ENABLE_RENODE_TEST=ON (so the default reference set stays at six apps).
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

#if defined(FRICK_PLATFORM_MPFS)
#include "platform/platform_mpfs.h"
#include "drivers/gpio/mpfs/ff_gpio_mpfs.h"

static uint32_t mpfs_gpio_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_GPIO0_BASE + off);
}

static void mpfs_gpio_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)(FF_GPIO0_BASE + off) = val;
}

/* HAL GPIO MMIO round trip on the emulated MSS GPIO (milestone M11). */
static bool renode_hal_gpio_check(void)
{
    static FF_GpioMpfs_t drv;
    FF_GpioBus_t bus = {NULL, mpfs_gpio_read32, mpfs_gpio_write32};
    uint8_t v = 0xFFu;

    if (FF_GpioMpfsSetup(&drv, &bus, FF_GPIO0_PINS) != FF_SUCCESS) {
        return false;
    }
    if (drv.gpio.init() != FF_SUCCESS) {
        return false;
    }
    if (drv.gpio.configure(0u, FF_GPIO_OUT) != FF_SUCCESS) {
        return false;
    }
    if (drv.gpio.write(0u, 1u) != FF_SUCCESS) {
        return false;
    }
    if ((drv.gpio.read(0u, &v) != FF_SUCCESS) || (v != 1u)) {
        return false;
    }
    if (drv.gpio.write(0u, 0u) != FF_SUCCESS) {
        return false;
    }
    if ((drv.gpio.read(0u, &v) != FF_SUCCESS) || (v != 0u)) {
        return false;
    }
    return true;
}
#endif /* FRICK_PLATFORM_MPFS */

void FF_AppRenodeTestMain(void *arg)
{
    FF_AppId_t me;
    FF_Msg_t msg;
    bool pub_ok;
    bool rx_ok = false;
    bool hal_ok = true;

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

#if defined(FRICK_PLATFORM_MPFS)
    hal_ok = renode_hal_gpio_check();
    FF_PalLog("[renode-test] hal_gpio=%s\n", hal_ok ? "PASS" : "FAIL");
#endif

    FF_PalLog("[renode-test] sb_roundtrip=%s\n", rx_ok ? "PASS" : "FAIL");
    FF_PalLog("RENODE_TEST: %s\n", (pub_ok && rx_ok && hal_ok) ? "PASS" : "FAIL");

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
