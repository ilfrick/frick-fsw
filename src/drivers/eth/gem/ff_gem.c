/*
 * frick-fsw - Cadence GEM Ethernet driver implementation (PolarFire SoC MSS)
 *
 * Static, bounded, allocation-free. Register/descriptor layout per the Cadence
 * GEM (see ff_gem_regs.h; source: Linux macb.h / microchip,mpfs-macb binding).
 * On-target PHY/link validation is GAP-A5.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/eth/gem/ff_gem.h"
#include "drivers/eth/gem/ff_gem_regs.h"
#include <stdint.h>

#define FF_GEM_TX_POLL 100000u
/* RXBS = DMA receive buffer size in units of 64 bytes. */
#define FF_GEM_RXBS ((FF_GEM_MTU + 63u) / 64u)

static uint32_t gem_rd(FF_Gem_t *g, uint32_t off)
{
    return g->cfg.bus.read32(g->cfg.bus.ctx, g->cfg.base + off);
}

static void gem_wr(FF_Gem_t *g, uint32_t off, uint32_t val)
{
    g->cfg.bus.write32(g->cfg.bus.ctx, g->cfg.base + off, val);
}

/* Returns the full 64-bit physical address. Co-implemented for NC-S6-03:
 * the return type is uint64_t (was uint32_t) so a DDR address >= 4 GiB is no
 * longer silently truncated. The descriptor addr field remains 32-bit for now
 * (full 3-word 64-bit descriptor format is deferred to M14). */
static uint64_t gem_phys(FF_Gem_t *g, const void *p)
{
    if (g->cfg.dma != NULL) {
        return g->cfg.dma->virt_to_phys(p);
    }
    return (uint64_t)(uintptr_t)p; /* host/test: identity */
}

/* Write a 64-bit queue base address: low word to TBQP/RBQP and high word to
 * TBQP_HI/RBQP_HI (the latter is a no-op on models without 64-bit DMA). */
static void gem_wr_qbqp(FF_Gem_t *g, uint32_t off, uint32_t off_hi, uint64_t addr)
{
    gem_wr(g, off, (uint32_t)(addr & 0xFFFFFFFFu));
    gem_wr(g, off_hi, (uint32_t)((addr >> 32) & 0xFFFFFFFFu));
}

static void gem_flush(FF_Gem_t *g, const void *p, uint32_t n)
{
    if ((g->cfg.dma != NULL) && (g->cfg.dma->cache_flush != NULL)) {
        g->cfg.dma->cache_flush(p, n);
    }
}

static void gem_inval(FF_Gem_t *g, const void *p, uint32_t n)
{
    if ((g->cfg.dma != NULL) && (g->cfg.dma->cache_invalidate != NULL)) {
        g->cfg.dma->cache_invalidate(p, n);
    }
}

static FF_Status_t gem_init(void *self)
{
    FF_Gem_t *g = (FF_Gem_t *)self;
    uint32_t ncfgr;
    uint16_t i;

    if ((g->cfg.bus.read32 == NULL) || (g->cfg.bus.write32 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }

    /* Disable RX/TX and clear statistics (CLRSTAT strobe; GEM has no reset). */
    gem_wr(g, FF_GEM_NCR, (1u << FF_GEM_NCR_CLRSTAT));

    /* Network config: full duplex, 1536-byte frames, FCS removed by HW,
     * 64-bit data bus. Gigabit mode is enabled by GBE (NCFGR bit 10). */
    ncfgr = ((g->cfg.full_duplex != 0u) ? (1u << FF_GEM_NCFGR_FD) : 0u) | (1u << FF_GEM_NCFGR_BIG) |
            (1u << FF_GEM_NCFGR_DRFCS) | (1u << FF_GEM_NCFGR_DBW);
    if (g->cfg.speed_mbps == 1000u) {
        ncfgr |= (1u << FF_GEM_NCFGR_GBE);
    }
    else if (g->cfg.speed_mbps == 100u) {
        ncfgr |= (1u << FF_GEM_NCFGR_SPD); /* SPD: 0=10, 1=100 */
    }
    gem_wr(g, FF_GEM_NCFGR, ncfgr);

    /* DMA receive buffer size (bits 16..23); enable 64-bit addressing
     * (ADDR64, bit 30) so TBQP_HI/RBQP_HI are honoured on hardware with DDR
     * >= 4 GiB. The full 64-bit descriptor format is deferred to M14. */
    gem_wr(g, FF_GEM_DMACFG,
           (1u << FF_GEM_DMACFG_ADDR64) | ((uint32_t)FF_GEM_RXBS << FF_GEM_DMACFG_RXBS));

    /* MAC address (specific address 1, little-endian per GEM). */
    gem_wr(g, FF_GEM_SA1B,
           (uint32_t)g->cfg.mac[0] | ((uint32_t)g->cfg.mac[1] << 8) |
               ((uint32_t)g->cfg.mac[2] << 16) | ((uint32_t)g->cfg.mac[3] << 24));
    gem_wr(g, FF_GEM_SA1T, (uint32_t)g->cfg.mac[4] | ((uint32_t)g->cfg.mac[5] << 8));

    /* Descriptor rings: TX owned by SW (USED=1), RX armed with buffer
     * addresses (last descriptor carries the WRAP bit). The per-descriptor
     * addr field is 32-bit (M14 widens it); the ring base is written 64-bit
     * (NC-S6-03 co-implementation). */
    for (i = 0u; i < FF_GEM_TX_DESC; i++) {
        g->tx_desc[i].addr = (uint32_t)gem_phys(g, g->tx_buf[i]);
        g->tx_desc[i].ctrl = (1u << FF_GEM_TX_USED);
        if (i == (FF_GEM_TX_DESC - 1u)) {
            g->tx_desc[i].ctrl |= (1u << FF_GEM_TX_WRAP);
        }
    }
    for (i = 0u; i < FF_GEM_RX_DESC; i++) {
        g->rx_desc[i].addr = (uint32_t)gem_phys(g, g->rx_buf[i]);
        if (i == (FF_GEM_RX_DESC - 1u)) {
            g->rx_desc[i].addr |= (1u << FF_GEM_RX_WRAP);
        }
        g->rx_desc[i].ctrl = 0u;
    }
    gem_wr_qbqp(g, FF_GEM_TBQP, FF_GEM_TBQP_HI, gem_phys(g, &g->tx_desc[0]));
    gem_wr_qbqp(g, FF_GEM_RBQP, FF_GEM_RBQP_HI, gem_phys(g, &g->rx_desc[0]));

    /* Enable receiver, transmitter and management port; poll mode (no IRQ). */
    gem_wr(g, FF_GEM_NCR, (1u << FF_GEM_NCR_RE) | (1u << FF_GEM_NCR_TE) | (1u << FF_GEM_NCR_MPE));
    gem_wr(g, FF_GEM_IDR, 0xFFFFFFFFu);
    (void)gem_rd(g, FF_GEM_TSR); /* read clears latched status */
    (void)gem_rd(g, FF_GEM_RSR);
    g->tx_head = 0u;
    g->rx_head = 0u;
    return FF_SUCCESS;
}

static FF_Status_t gem_get_mac_address(void *self, uint8_t out[6])
{
    FF_Gem_t *g = (FF_Gem_t *)self;
    uint8_t i;
    if (out == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    for (i = 0u; i < 6u; i++) {
        out[i] = g->cfg.mac[i];
    }
    return FF_SUCCESS;
}

static FF_Status_t gem_set_link_up(void *self, uint8_t up)
{
    FF_Gem_t *g = (FF_Gem_t *)self;
    uint32_t ncr = gem_rd(g, FF_GEM_NCR);
    if (up != 0u) {
        ncr |= (1u << FF_GEM_NCR_RE) | (1u << FF_GEM_NCR_TE);
    }
    else {
        ncr &= ~((1u << FF_GEM_NCR_RE) | (1u << FF_GEM_NCR_TE));
    }
    gem_wr(g, FF_GEM_NCR, ncr);
    return FF_SUCCESS;
}

static FF_Status_t gem_transmit(void *self, const uint8_t *frame, uint16_t len)
{
    FF_Gem_t *g = (FF_Gem_t *)self;
    uint16_t idx = g->tx_head;
    uint16_t i;
    uint32_t spins;

    if ((frame == NULL) || (len == 0u) || (len > FF_GEM_MTU)) {
        g->errors++;
        return FF_BAD_SIZE;
    }
    if ((g->tx_desc[idx].ctrl & (1u << FF_GEM_TX_USED)) == 0u) {
        g->errors++; /* descriptor still owned by DMA: TX ring full */
        return FF_NOT_READY;
    }
    for (i = 0u; i < len; i++) {
        g->tx_buf[idx][i] = frame[i];
    }
    gem_flush(g, g->tx_buf[idx], len);
    g->tx_desc[idx].addr = (uint32_t)gem_phys(g, g->tx_buf[idx]);
    /* USED=0 hands the descriptor to the DMA; LAST marks the frame end and
     * WRAP is set on the last descriptor of the ring. */
    g->tx_desc[idx].ctrl = (uint32_t)(len & FF_GEM_TX_FRMLEN_MASK) | (1u << FF_GEM_TX_LAST);
    if (idx == (FF_GEM_TX_DESC - 1u)) {
        g->tx_desc[idx].ctrl |= (1u << FF_GEM_TX_WRAP);
    }
    gem_flush(g, &g->tx_desc[idx], (uint32_t)sizeof(FF_GemDesc_t));

    gem_wr(g, FF_GEM_NCR, gem_rd(g, FF_GEM_NCR) | (1u << FF_GEM_NCR_TSTART));
    spins = 0u;
    while (((gem_rd(g, FF_GEM_TSR) & (1u << FF_GEM_TSR_COMP)) == 0u) && (spins < FF_GEM_TX_POLL)) {
        spins++;
    }
    gem_wr(g, FF_GEM_NCR, gem_rd(g, FF_GEM_NCR) & ~(1u << FF_GEM_NCR_TSTART));
    if ((gem_rd(g, FF_GEM_TSR) & (1u << FF_GEM_TSR_COMP)) == 0u) {
        /* Timeout: reclaim the descriptor so the ring is not permanently wedged. */
        g->tx_desc[idx].ctrl |= (1u << FF_GEM_TX_USED);
        g->tx_head = (uint16_t)((idx + 1u) % FF_GEM_TX_DESC);
        g->errors++;
        return FF_ERROR;
    }
    (void)gem_rd(g, FF_GEM_TSR); /* read clears latched status */
    gem_inval(g, &g->tx_desc[idx], (uint32_t)sizeof(FF_GemDesc_t));
    g->tx_desc[idx].ctrl |= (1u << FF_GEM_TX_USED);
    g->tx_head = (uint16_t)((idx + 1u) % FF_GEM_TX_DESC);
    g->tx_frames++;
    return FF_SUCCESS;
}

static void gem_set_rx_callback(void *self, FF_EthRxCb_t cb, void *ctx)
{
    FF_Gem_t *g = (FF_Gem_t *)self;
    g->rx_cb = cb;
    g->rx_ctx = ctx;
}

static FF_Status_t gem_get_stats(void *self, uint32_t *tx, uint32_t *rx, uint32_t *err)
{
    FF_Gem_t *g = (FF_Gem_t *)self;
    if (tx != NULL) {
        *tx = g->tx_frames;
    }
    if (rx != NULL) {
        *rx = g->rx_frames;
    }
    if (err != NULL) {
        *err = g->errors;
    }
    return FF_SUCCESS;
}

uint16_t FF_GemPollRx(FF_Gem_t *gem)
{
    uint16_t dispatched = 0u;

    if (gem == NULL) {
        return 0u;
    }
    for (;;) {
        FF_GemDesc_t *d = &gem->rx_desc[gem->rx_head];
        uint16_t len;

        gem_inval(gem, d, (uint32_t)sizeof(FF_GemDesc_t));
        if ((d->addr & (1u << FF_GEM_RX_USED)) == 0u) {
            break; /* no more completed frames */
        }
        len = (uint16_t)(d->ctrl & FF_GEM_RX_FRMLEN_MASK);
        if (len > FF_GEM_MTU) {
            len = FF_GEM_MTU;
        }
        gem_inval(gem, gem->rx_buf[gem->rx_head], len);
        gem->rx_frames++;
        if (gem->rx_cb != NULL) {
            gem->rx_cb(gem->rx_buf[gem->rx_head], len, gem->rx_ctx);
        }
        /* Re-arm the descriptor (clear USED, restore WRAP on the last one). */
        d->addr = (uint32_t)gem_phys(gem, gem->rx_buf[gem->rx_head]);
        if (gem->rx_head == (FF_GEM_RX_DESC - 1u)) {
            d->addr |= (1u << FF_GEM_RX_WRAP);
        }
        gem_flush(gem, d, (uint32_t)sizeof(FF_GemDesc_t));
        gem->rx_head = (uint16_t)((gem->rx_head + 1u) % FF_GEM_RX_DESC);
        dispatched++;
    }
    return dispatched;
}

FF_Status_t FF_GemSetup(FF_Gem_t *gem, const FF_GemConfig_t *cfg)
{
    if ((gem == NULL) || (cfg == NULL) || (cfg->bus.read32 == NULL) || (cfg->bus.write32 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    gem->cfg = *cfg;
    gem->tx_head = 0u;
    gem->rx_head = 0u;
    gem->tx_frames = 0u;
    gem->rx_frames = 0u;
    gem->errors = 0u;
    gem->rx_cb = NULL;
    gem->rx_ctx = NULL;

    gem->mac.name = "gem";
    gem->mac.mtu = FF_GEM_MTU;
    gem->mac.init = gem_init;
    gem->mac.get_mac_address = gem_get_mac_address;
    gem->mac.set_link_up = gem_set_link_up;
    gem->mac.transmit = gem_transmit;
    gem->mac.set_rx_callback = gem_set_rx_callback;
    gem->mac.get_stats = gem_get_stats;
    return FF_SUCCESS;
}
