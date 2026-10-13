/*
 * frick-fsw - GEM native TSN backend (PolarFire SoC MSS)
 *
 * Implements the HAL TSN contract (FF_TsnBackend_t) on the *native* TSN block of
 * the Cadence GEM: 802.1Q VLAN/PCP classification (screener type 2), 802.1Qbv
 * time-aware scheduling (ENST registers) and 802.1AS time from the GEM TSU.
 * Register offsets/fields per the Cadence GEM layout (ff_gem_regs.h, source
 * Linux macb.h). Credit-based shaping (802.1Qav) is not exposed in the GEM
 * register subset used here and returns FF_NOT_IMPLEMENTED (documented).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_GEM_TSN_H
#define FF_GEM_TSN_H

#include "common/ff_types.h"
#include "hal/hal_tsn.h"
#include "drivers/eth/gem/ff_gem.h"

/* One Qbv gate entry (traffic class schedule). Times are in nanoseconds. */
typedef struct
{
    uint8_t queue;       /* traffic class / queue (0..7)        */
    uint8_t start_sec;   /* cycle start, seconds part (0..3)    */
    uint32_t start_nsec; /* cycle start, nanoseconds part       */
    uint32_t on_nsec;    /* gate-open duration (ns)             */
    uint32_t off_nsec;   /* gate-closed duration (ns)           */
} FF_TsnGate_t;

typedef struct
{
    FF_TsnBackend_t tsn; /* public HAL interface (must be first) */
    FF_GemBus_t bus;
    uint32_t base;
    uint32_t speed_mbps;     /* link speed for ENST time scaling (10/100/1000) */
    uint8_t qci_entry_count; /* number of 802.1Qci stream filters programmed */
} FF_TsnGem_t;

FF_Status_t FF_TsnGemSetup(FF_TsnGem_t *tsn, const FF_GemBus_t *bus, uint32_t base,
                           uint32_t speed_mbps);

#endif /* FF_GEM_TSN_H */
