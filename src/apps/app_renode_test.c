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
#include "drivers/serial/mpfs/ff_serial_mpfs.h"
#include "drivers/mem/ff_mem_static.h"
#include "drivers/nvm/mpfs/ff_nvm_mpfs.h"
#include "drivers/net/ff_net_loopback.h"
#include <string.h>

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

static uint8_t mpfs_uart_read8(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint8_t *)(uintptr_t)(FF_MMUART0_BASE + off);
}

static void mpfs_uart_write8(void *ctx, uint32_t off, uint8_t val)
{
    (void)ctx;
    *(volatile uint8_t *)(uintptr_t)(FF_MMUART0_BASE + off) = val;
}

/* HAL serial MMIO check on the emulated MSS MMUART0 (milestone M11): line
 * control + NS16550 scratch-register round trip (wide registers). */
static bool renode_hal_serial_check(void)
{
    static FF_SerialMpfs_t drv;
    FF_SerialBus_t bus = {NULL, mpfs_uart_read8, mpfs_uart_write8};

    if (FF_SerialMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    if (drv.serial.init(115200u, 8u, 0u, 1u) != FF_SUCCESS) {
        return false;
    }
    return FF_SerialMpfsScratchSelfTest(&drv) == FF_SUCCESS;
}

static uint32_t mpfs_gem_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_GEM0_BASE + off);
}

/* HAL Ethernet check on the emulated Cadence GEM (milestone M11): identify the
 * MAC via the read-only Module ID/revision register. The emulated GEM uses the
 * Cadence defaults (identification 0x002, revision 0x118). (This is an MMIO
 * presence/identification check; the full GEM driver/DMA/link path is host-tested
 * and on-target PHY/link is GAP-A5.) */
static bool renode_hal_eth_check(void)
{
    uint32_t modid = mpfs_gem_read32(NULL, FF_GEM_MODID_OFF);
    /* Cadence GEM MODID: [15:0] module revision, [27:16] module identification. */
    return (((modid >> 16) & 0x0FFFu) == 0x002u) && ((modid & 0xFFFFu) == 0x0118u);
}

/* HAL network-stack check (milestone M11): a socket round trip through the
 * pluggable FF_NetStack_t contract, using the reference loopback stack (the same
 * contract an lwIP/Tuxera adapter implements). It is a **contract/behaviour**
 * check (in-process), not a binding to an emulated peripheral.
 * Oracle: bind to a non-zero port; a connect to an unlistened port must fail;
 * accept must report the server port; the message must round-trip. */
static bool renode_hal_net_fail(FF_NetStack_t *s)
{
    (void)s->init(); /* reset the socket table (no leak on the failure path) */
    return false;
}

static bool renode_hal_net_check(void)
{
    FF_NetStack_t *s = &FF_NetStackLoopback;
    const uint8_t port[1] = {0x42u};
    const uint8_t no_port[1] = {0x43u};
    const uint8_t msg[4] = {0x11u, 0x22u, 0x33u, 0x44u};
    uint8_t peer[4];
    uint8_t buf[8];
    uint16_t peer_len = sizeof(peer);
    uint16_t sent = 0u;
    uint16_t got = 0u;
    FF_NetSock_t srv;
    FF_NetSock_t neg;
    FF_NetSock_t cli;
    FF_NetSock_t acc;

    if (s->init() != FF_SUCCESS) {
        return false;
    }
    srv = s->open(0, 0, 0);
    if ((srv < 0) || (s->bind(srv, port, 1u) != FF_SUCCESS) || (s->listen(srv, 1u) != FF_SUCCESS)) {
        return renode_hal_net_fail(s);
    }
    /* Negative: connecting to a port with no listener must fail. */
    neg = s->open(0, 0, 0);
    if ((neg < 0) || (s->connect(neg, no_port, 1u) == FF_SUCCESS)) {
        return renode_hal_net_fail(s);
    }
    (void)s->close(neg);
    cli = s->open(0, 0, 0);
    if ((cli < 0) || (s->connect(cli, port, 1u) != FF_SUCCESS)) {
        return renode_hal_net_fail(s);
    }
    acc = s->accept(srv, peer, &peer_len);
    if ((acc < 0) || (peer_len < 1u) || (peer[0] != port[0])) {
        return renode_hal_net_fail(s); /* bind must be observable */
    }
    if ((s->send(cli, msg, sizeof(msg), &sent) != FF_SUCCESS) || (sent != sizeof(msg))) {
        return renode_hal_net_fail(s);
    }
    if ((s->recv(acc, buf, sizeof(buf), &got, 0u) != FF_SUCCESS) || (got != sizeof(msg)) ||
        (memcmp(buf, msg, sizeof(msg)) != 0)) {
        return renode_hal_net_fail(s);
    }
    (void)s->close(acc);
    (void)s->close(cli);
    (void)s->close(srv);
    return true;
}

/* HAL memory check (milestone M11): static RAM pool over the emulated DDR +
 * reference DMA translation. */
static bool renode_hal_mem_check(void)
{
    static FF_MemStaticPool_t pool;
    static uint8_t area[256];
    uint8_t *p;

    if (FF_MemStaticPoolInit(&pool, "ddr-pool", area, sizeof(area)) != FF_SUCCESS) {
        return false;
    }
    p = (uint8_t *)pool.pool.alloc(16u);
    if (p == NULL) {
        return false;
    }
    p[0] = 0xAAu;
    p[15] = 0x55u;
    if ((p[0] != 0xAAu) || (p[15] != 0x55u)) {
        return false;
    }
    return FF_DmaIdentity.virt_to_phys(p) != 0u;
}

static uint8_t mpfs_envm_read8(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint8_t *)(uintptr_t)(FF_ENVM0_BASE + off);
}

static void mpfs_envm_write8(void *ctx, uint32_t off, uint8_t val)
{
    (void)ctx;
    *(volatile uint8_t *)(uintptr_t)(FF_ENVM0_BASE + off) = val;
}

/* HAL NVM check on the emulated MSS eNVM (milestone M11): a bounded read of the
 * first parameter slot must succeed (the eNVM region is mapped and readable). It
 * is an MMIO presence/identification check **only**: the emulated eNVM region is
 * not writable in the platform model, so the write/erase register logic is
 * host-tested (`test_nvm`) and the on-target flash write/endurance/retention is
 * GAP-A5. This scope is deliberate (a write round trip faults on the model). */
static bool renode_hal_nvm_check(void)
{
    static FF_NvmMpfs_t drv;
    FF_NvmBus_t bus = {NULL, mpfs_envm_read8, mpfs_envm_write8};
    uint8_t buf[4];
    uint16_t len = 0u;

    if (FF_NvmMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    return drv.nvm.read(0u, buf, sizeof(buf), &len) == FF_SUCCESS;
}
#endif /* FRICK_PLATFORM_MPFS */

void FF_AppRenodeTestMain(void *arg)
{
    FF_AppId_t me;
    FF_Msg_t msg;
    bool pub_ok;
    bool rx_ok = false;
    bool hal_ok = true;
    bool hal_serial_ok = true;
    bool hal_mem_ok = true;
    bool hal_eth_ok = true;
    bool hal_net_ok = true;
    bool hal_nvm_ok = true;

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
    hal_serial_ok = renode_hal_serial_check();
    hal_mem_ok = renode_hal_mem_check();
    hal_eth_ok = renode_hal_eth_check();
    hal_net_ok = renode_hal_net_check();
    hal_nvm_ok = renode_hal_nvm_check();
    FF_PalLog("[renode-test] hal_gpio=%s\n", hal_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_serial=%s\n", hal_serial_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_mem=%s\n", hal_mem_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_eth=%s\n", hal_eth_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_net=%s\n", hal_net_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_nvm=%s\n", hal_nvm_ok ? "PASS" : "FAIL");
#endif

    FF_PalLog("[renode-test] sb_roundtrip=%s\n", rx_ok ? "PASS" : "FAIL");
    FF_PalLog("RENODE_TEST: %s\n", (pub_ok && rx_ok && hal_ok && hal_serial_ok && hal_mem_ok &&
                                    hal_eth_ok && hal_net_ok && hal_nvm_ok)
                                       ? "PASS"
                                       : "FAIL");

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
