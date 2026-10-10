/*
 * frick-fsw - GEM native TSN backend implementation (PolarFire SoC MSS)
 *
 * See ff_gem_tsn.h. Register layout per the Cadence GEM (ff_gem_regs.h).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "drivers/eth/gem/ff_gem_tsn.h"
#include "drivers/eth/gem/ff_gem_regs.h"

#define FF_TSN_MAX_QUEUES 8u

static uint32_t tsn_rd(FF_TsnGem_t *t, uint32_t off)
{
    return t->bus.read32(t->bus.ctx, t->base + off);
}

static void tsn_wr(FF_TsnGem_t *t, uint32_t off, uint32_t val)
{
    t->bus.write32(t->bus.ctx, t->base + off, val);
}

static FF_Status_t tsn_init(void *self)
{
    FF_TsnGem_t *t = (FF_TsnGem_t *)self;
    if ((t->bus.read32 == NULL) || (t->bus.write32 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    return FF_SUCCESS;
}

/* 802.1Q: map a VLAN priority (PCP) to queue 0 via screener type 2 entry 0.
 * The VLAN id is recorded by the mission's screener EtherType config. */
static FF_Status_t tsn_set_vlan(void *self, uint16_t vid, uint8_t pcp)
{
    FF_TsnGem_t *t = (FF_TsnGem_t *)self;
    uint32_t scr;

    (void)vid;
    if (pcp > 7u) {
        return FF_INVALID_ARGUMENT;
    }
    scr = (0u << FF_GEM_SCRT2_QUEUE_SHIFT) | ((uint32_t)pcp << FF_GEM_SCRT2_VLANPR_SHIFT) |
          (1u << FF_GEM_SCRT2_VLANEN_SHIFT);
    tsn_wr(t, FF_GEM_SCRT2, scr);
    return FF_SUCCESS;
}

/* Convert a duration in nanoseconds to the ENST hardware unit (a link-speed
 * scaled byte count; see Linux macb enst_ns_to_hw_units: ns*speed/8000). */
static uint32_t tsn_ns_to_hw(FF_TsnGem_t *t, uint32_t ns)
{
    uint64_t hw = ((uint64_t)ns * (uint64_t)t->speed_mbps + 7999ULL) / 8000ULL;
    return (uint32_t)hw;
}

/* 802.1Qbv: program the gate control list. `schedule` is an array of
 * FF_TsnGate_t and `len` its size in bytes. */
static FF_Status_t tsn_configure_tas(void *self, const uint8_t *schedule, uint16_t len)
{
    FF_TsnGem_t *t = (FF_TsnGem_t *)self;
    const FF_TsnGate_t *gates = (const FF_TsnGate_t *)(const void *)schedule;
    uint16_t count = (uint16_t)(len / sizeof(FF_TsnGate_t));
    uint32_t enable = 0u;
    uint16_t i;

    if ((schedule == NULL) || (count == 0u) || (count > FF_TSN_MAX_QUEUES)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Disable all queues before reconfiguring (avoid stale enable state). */
    tsn_wr(t, FF_GEM_ENST_CONTROL, 0u);

    for (i = 0u; i < count; i++) {
        const FF_TsnGate_t *g = &gates[i];
        uint32_t start;
        uint32_t on_hw;
        uint32_t off_hw;

        if ((g->queue >= FF_TSN_MAX_QUEUES) || (g->start_sec > 3u) ||
            ((g->start_nsec & ~FF_GEM_ENST_START_NSEC_MASK) != 0u)) {
            return FF_INVALID_ARGUMENT; /* would truncate the gate fields */
        }
        on_hw = tsn_ns_to_hw(t, g->on_nsec);
        off_hw = tsn_ns_to_hw(t, g->off_nsec);
        if ((on_hw > FF_GEM_ENST_TIME_MASK) || (off_hw > FF_GEM_ENST_TIME_MASK)) {
            return FF_INVALID_ARGUMENT; /* would truncate the gate fields */
        }
        start = ((uint32_t)g->start_sec << FF_GEM_ENST_START_SEC_SHIFT) |
                (g->start_nsec & FF_GEM_ENST_START_NSEC_MASK);
        tsn_wr(t, FF_GEM_ENST_START_TIME(g->queue), start);
        tsn_wr(t, FF_GEM_ENST_ON_TIME(g->queue), on_hw);
        tsn_wr(t, FF_GEM_ENST_OFF_TIME(g->queue), off_hw);
        enable |= (1u << g->queue);
    }
    /* ENST_CONTROL bits [15:0] enable queues (bit q); [31:16] disable. */
    tsn_wr(t, FF_GEM_ENST_CONTROL, enable & 0xFFFFu);
    return FF_SUCCESS;
}

/* 802.1Qav credit-based shaper: not exposed in the GEM register subset used
 * here (reference limitation; see docs/architecture/hardware-abstraction.md). */
static FF_Status_t tsn_configure_cbs(void *self, uint8_t traffic_class, uint32_t idle_slope_bps)
{
    (void)self;
    (void)traffic_class;
    (void)idle_slope_bps;
    return FF_NOT_IMPLEMENTED;
}

/* 802.1AS: read the GEM 1588 timer (seconds 47:0, nanoseconds 29:0). */
static FF_Status_t tsn_get_time_ns(void *self, uint64_t *out_ns)
{
    FF_TsnGem_t *t = (FF_TsnGem_t *)self;
    uint64_t sec;
    uint64_t ns;

    if (out_ns == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    sec = ((uint64_t)tsn_rd(t, FF_GEM_TSH) << FF_GEM_TSH_SHIFT) | (uint64_t)tsn_rd(t, FF_GEM_TSL);
    ns = (uint64_t)(tsn_rd(t, FF_GEM_TN) & FF_GEM_TN_MASK);
    *out_ns = (sec * 1000000000ULL) + ns;
    return FF_SUCCESS;
}

FF_Status_t FF_TsnGemSetup(FF_TsnGem_t *tsn, const FF_GemBus_t *bus, uint32_t base,
                           uint32_t speed_mbps)
{
    if ((tsn == NULL) || (bus == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    tsn->bus = *bus;
    tsn->base = base;
    tsn->speed_mbps = speed_mbps;
    tsn->tsn.name = "gem-tsn";
    tsn->tsn.init = tsn_init;
    tsn->tsn.set_vlan = tsn_set_vlan;
    tsn->tsn.configure_tas = tsn_configure_tas;
    tsn->tsn.configure_cbs = tsn_configure_cbs;
    tsn->tsn.get_time_ns = tsn_get_time_ns;
    return FF_SUCCESS;
}
