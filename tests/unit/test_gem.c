/*
 * frick-fsw - Cadence GEM Ethernet driver unit tests (milestone M4)
 *
 * Exercises the GEM register/descriptor logic on the host with a mock MMIO bus
 * (SRS-REQ-036). The mock models the documented Cadence GEM behaviour (NCR
 * TSTART -> TSR COMP; RX descriptors carry the frame length), so it is not
 * derived from the driver. Real PHY/link behaviour requires the board (GAP-A5).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/eth/gem/ff_gem.h"
#include "drivers/eth/gem/ff_gem_regs.h"
#include <string.h>

#define MOCK_REGS 512u

typedef struct
{
    uint32_t reg[MOCK_REGS / 4u];
} MockCtx_t;

static MockCtx_t g_mock;
static uint16_t g_rx_len;
static uint16_t g_rx_hits;

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    MockCtx_t *m = (MockCtx_t *)ctx;
    return m->reg[off / 4u];
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    MockCtx_t *m = (MockCtx_t *)ctx;
    switch (off) {
    case FF_GEM_NCR:
        m->reg[off / 4u] = val;
        if ((val & (1u << FF_GEM_NCR_TSTART)) != 0u) {
            /* transmit start -> transmit complete (documented behaviour) */
            m->reg[FF_GEM_TSR / 4u] |= (1u << FF_GEM_TSR_COMP);
        }
        break;
    default:
        m->reg[off / 4u] = val;
        break;
    }
}

static void on_rx(const uint8_t *frame, uint16_t len, void *ctx)
{
    (void)frame;
    (void)ctx;
    g_rx_len = len;
    g_rx_hits++;
}

static void test_gem_init(void)
{
    FF_Gem_t gem;
    FF_GemConfig_t cfg;
    static const uint8_t want[6] = {0x02u, 0x00u, 0x11u, 0x22u, 0x33u, 0x44u};
    uint8_t mac[6];
    uint32_t ncr;
    uint32_t ncfgr;

    memset(&g_mock, 0, sizeof(g_mock));
    memset(&cfg, 0, sizeof(cfg));
    cfg.bus.ctx = &g_mock;
    cfg.bus.read32 = mock_read32;
    cfg.bus.write32 = mock_write32;
    memcpy(cfg.mac, want, 6u);
    cfg.speed_mbps = 1000u;
    cfg.full_duplex = 1u;

    FF_TEST_ASSERT(FF_GemSetup(&gem, &cfg) == FF_SUCCESS);
    FF_TEST_ASSERT(gem.mac.init(&gem) == FF_SUCCESS);
    ncr = mock_read32(&g_mock, FF_GEM_NCR);
    FF_TEST_ASSERT((ncr & (1u << FF_GEM_NCR_RE)) != 0u);
    FF_TEST_ASSERT((ncr & (1u << FF_GEM_NCR_TE)) != 0u);
    ncfgr = mock_read32(&g_mock, FF_GEM_NCFGR);
    FF_TEST_ASSERT((ncfgr & (1u << FF_GEM_NCFGR_GBE)) != 0u); /* gigabit */
    FF_TEST_ASSERT((ncfgr & (1u << FF_GEM_NCFGR_FD)) != 0u);  /* full duplex */
    FF_TEST_ASSERT(gem.mac.get_mac_address(&gem, mac) == FF_SUCCESS);
    FF_TEST_ASSERT(memcmp(mac, want, 6u) == 0);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_GEM_SA1B) == 0x22110002u);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_GEM_SA1T) == 0x4433u);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_GEM_RBQP) == (uint32_t)(uintptr_t)&gem.rx_desc[0]);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_GEM_RBQP_HI) ==
                   (uint32_t)((uint64_t)(uintptr_t)&gem.rx_desc[0] >> 32));
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_GEM_TBQP) == (uint32_t)(uintptr_t)&gem.tx_desc[0]);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_GEM_TBQP_HI) ==
                   (uint32_t)((uint64_t)(uintptr_t)&gem.tx_desc[0] >> 32));
    /* ADDR64 bit is set in DMACFG to enable 64-bit queue addressing. */
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_GEM_DMACFG) & (1u << FF_GEM_DMACFG_ADDR64)) != 0u);
}

static void test_gem_transmit(void)
{
    FF_Gem_t gem;
    FF_GemConfig_t cfg;
    const uint8_t frame[24] = "gem-transmit-test-frame";
    uint32_t tx = 0u, rx = 0u, err = 0u;

    memset(&g_mock, 0, sizeof(g_mock));
    memset(&cfg, 0, sizeof(cfg));
    cfg.bus.ctx = &g_mock;
    cfg.bus.read32 = mock_read32;
    cfg.bus.write32 = mock_write32;
    cfg.speed_mbps = 1000u;
    cfg.full_duplex = 1u;

    FF_GemSetup(&gem, &cfg);
    gem.mac.init(&gem);
    FF_TEST_ASSERT(gem.mac.transmit(&gem, frame, sizeof(frame)) == FF_SUCCESS);
    FF_TEST_ASSERT(gem.mac.transmit(&gem, frame, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(gem.mac.get_stats(&gem, &tx, &rx, &err) == FF_SUCCESS);
    FF_TEST_ASSERT(tx == 1u);
    FF_TEST_ASSERT(err == 1u); /* the rejected zero-length call counted one error */
}

static void test_gem_rx(void)
{
    FF_Gem_t gem;
    FF_GemConfig_t cfg;

    memset(&g_mock, 0, sizeof(g_mock));
    memset(&cfg, 0, sizeof(cfg));
    cfg.bus.ctx = &g_mock;
    cfg.bus.read32 = mock_read32;
    cfg.bus.write32 = mock_write32;

    g_rx_len = 0u;
    g_rx_hits = 0u;
    FF_GemSetup(&gem, &cfg);
    gem.mac.init(&gem);
    gem.mac.set_rx_callback(&gem, on_rx, NULL);
    FF_TEST_ASSERT(FF_GemPollRx(&gem) == 0u); /* nothing pending */

    /* The hardware would set USED and the frame length in the descriptor. */
    gem.rx_desc[0].addr = (uint32_t)(uintptr_t)gem.rx_buf[0] | (1u << FF_GEM_RX_USED);
    gem.rx_desc[0].ctrl = 42u;
    FF_TEST_ASSERT(FF_GemPollRx(&gem) == 1u);
    FF_TEST_ASSERT(g_rx_hits == 1u);
    FF_TEST_ASSERT(g_rx_len == 42u);
    FF_TEST_ASSERT((gem.rx_desc[0].addr & (1u << FF_GEM_RX_USED)) == 0u); /* re-armed */
    FF_TEST_ASSERT(FF_GemPollRx(&gem) == 0u);
}

FF_TEST_MAIN(test_gem_init, test_gem_transmit, test_gem_rx)
