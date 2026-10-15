/*
 * frick-fsw - Cadence GEM Ethernet driver (PolarFire SoC MSS), HAL backend
 *
 * Implements the HAL Ethernet MAC contract (FF_EthMac_t) for the two Gigabit
 * Ethernet MACs in the Microchip PolarFire SoC MSS. MMIO access and DMA address
 * translation are injected (FF_GemBus_t + HAL FF_Dma_t) so the register and
 * descriptor logic is testable on the host and reusable on the target.
 *
 * Design: static, bounded, allocation-free (norm N10); a small static
 * descriptor ring with per-descriptor buffers. The register/descriptor layout
 * follows the Cadence GEM (see ff_gem_regs.h). On-target PHY/link validation is
 * GAP-A5; TSN uses the GEM's native block (see docs/architecture/
 * hardware-abstraction.md).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_GEM_H
#define FF_GEM_H

#include "common/ff_types.h"
#include "hal/hal_eth.h"
#include "hal/hal_mem.h"

#ifndef FF_GEM_MTU
#define FF_GEM_MTU 1536u
#endif
#ifndef FF_GEM_TX_DESC
#define FF_GEM_TX_DESC 4u
#endif
#ifndef FF_GEM_RX_DESC
#define FF_GEM_RX_DESC 4u
#endif

/* MMIO access injected by the platform (real MMIO on target, mock on host). */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
} FF_GemBus_t;

/* GEM DMA buffer descriptor (2 words: buffer address, control/status). */
typedef struct
{
    uint32_t addr; /* buffer physical address (RX: + USED/WRAP bits) */
    uint32_t ctrl; /* TX: FRMLEN|LAST|USED ; RX: FRMLEN|SOF|EOF           */
} FF_GemDesc_t;

typedef struct
{
    FF_GemBus_t bus;
    FF_Dma_t *dma; /* address translation + cache (may be NULL on host) */
    uint32_t base; /* register base offset (0 for one MAC)              */
    uint8_t mac[6];
    uint32_t speed_mbps; /* 10 | 100 | 1000 */
    uint8_t full_duplex;
} FF_GemConfig_t;

typedef struct
{
    FF_EthMac_t mac; /* public HAL interface (must be first) */
    FF_GemConfig_t cfg;
    FF_GemDesc_t tx_desc[FF_GEM_TX_DESC];
    FF_GemDesc_t rx_desc[FF_GEM_RX_DESC];
    uint8_t tx_buf[FF_GEM_TX_DESC][FF_GEM_MTU];
    uint8_t rx_buf[FF_GEM_RX_DESC][FF_GEM_MTU];
    uint16_t tx_head;
    uint16_t rx_head;
    uint32_t tx_frames;
    uint32_t rx_frames;
    uint32_t errors;
    FF_EthRxCb_t rx_cb;
    void *rx_ctx;
} FF_Gem_t;

/* Initialise the driver instance and the FF_EthMac_t vtable. The driver does
 * not power up the MAC; FF_EthMac_t.init() does. */
FF_Status_t FF_GemSetup(FF_Gem_t *gem, const FF_GemConfig_t *cfg);

/* Poll the receive path and dispatch any completed frame to the registered
 * callback (call periodically from a task; the driver is poll-mode). Returns
 * the number of frames dispatched. */
uint16_t FF_GemPollRx(FF_Gem_t *gem);

#endif /* FF_GEM_H */
