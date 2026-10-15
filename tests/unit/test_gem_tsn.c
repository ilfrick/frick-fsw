/*
 * frick-fsw - GEM native TSN backend unit tests (milestone M4)
 *
 * Exercises the GEM TSN register logic (802.1Q screener, 802.1Qbv ENST,
 * 802.1AS TSU read) with a mock MMIO bus (SRS-REQ-038).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/eth/gem/ff_gem_tsn.h"
#include "drivers/eth/gem/ff_gem_regs.h"
#include <string.h>

#define MOCK_REGS 4096u /* includes the GEM ENST control block at offset 0x0880 */

typedef struct
{
    uint32_t reg[MOCK_REGS / 4u];
} TsnMock_t;

static TsnMock_t g_mock;

static uint32_t mock_rd(void *ctx, uint32_t off)
{
    return ((TsnMock_t *)ctx)->reg[off / 4u];
}

static void mock_wr(void *ctx, uint32_t off, uint32_t val)
{
    ((TsnMock_t *)ctx)->reg[off / 4u] = val;
}

static void test_tsn_vlan_and_time(void)
{
    FF_TsnGem_t t;
    FF_GemBus_t bus;
    uint64_t ns = 0u;
    uint32_t scr;
    FF_TsnGate_t gates[1];

    memset(&g_mock, 0, sizeof(g_mock));
    bus.ctx = &g_mock;
    bus.read32 = mock_rd;
    bus.write32 = mock_wr;

    FF_TEST_ASSERT(FF_TsnGemSetup(&t, &bus, 0u, 1000u) == FF_SUCCESS);
    FF_TEST_ASSERT(t.tsn.init(&t) == FF_SUCCESS);
    FF_TEST_ASSERT(t.tsn.set_vlan(&t, 100u, 5u) == FF_SUCCESS);
    FF_TEST_ASSERT(t.tsn.set_vlan(&t, 100u, 9u) == FF_INVALID_ARGUMENT);
    scr = mock_rd(&g_mock, FF_GEM_SCRT2);
    FF_TEST_ASSERT(((scr >> FF_GEM_SCRT2_VLANPR_SHIFT) & 0x7u) == 5u);
    FF_TEST_ASSERT((scr & (1u << FF_GEM_SCRT2_VLANEN_SHIFT)) != 0u);

    /* 802.1Qbv: one gate for queue 3. */
    memset(gates, 0, sizeof(gates));
    gates[0].queue = 3u;
    gates[0].start_sec = 1u;
    gates[0].start_nsec = 250000u;
    gates[0].on_nsec = 1000u;
    gates[0].off_nsec = 2000u;
    FF_TEST_ASSERT(t.tsn.configure_tas(&t, (const uint8_t *)gates, sizeof(gates)) == FF_SUCCESS);
    FF_TEST_ASSERT((mock_rd(&g_mock, FF_GEM_ENST_CONTROL) & (1u << 3)) != 0u);
    /* ENST time field is a link-speed-scaled byte count: 1000 ns @1G = 125,
     * 2000 ns @1G = 250 (see macb enst_ns_to_hw_units). */
    FF_TEST_ASSERT(mock_rd(&g_mock, FF_GEM_ENST_ON_TIME(3u)) == 125u);
    FF_TEST_ASSERT(mock_rd(&g_mock, FF_GEM_ENST_OFF_TIME(3u)) == 250u);

    /* Out-of-range gate duration (would truncate the 17-bit field). */
    gates[0].on_nsec = 2000000000u; /* hw = 2.5e8 >> 0x1FFFF */
    FF_TEST_ASSERT(t.tsn.configure_tas(&t, (const uint8_t *)gates, sizeof(gates)) ==
                   FF_INVALID_ARGUMENT);
    gates[0].on_nsec = 1000u;

    /* start_sec out of range (validated, not masked). */
    gates[0].on_nsec = 1000u;
    gates[0].start_sec = 4u;
    FF_TEST_ASSERT(t.tsn.configure_tas(&t, (const uint8_t *)gates, sizeof(gates)) ==
                   FF_INVALID_ARGUMENT);
    gates[0].start_sec = 1u;

    /* 802.1AS: TSU seconds 2, nanoseconds 500000000 -> 2.5 s. */
    mock_wr(&g_mock, FF_GEM_TSH, 0u);
    mock_wr(&g_mock, FF_GEM_TSL, 2u);
    mock_wr(&g_mock, FF_GEM_TN, 500000000u);
    FF_TEST_ASSERT(t.tsn.get_time_ns(&t, &ns) == FF_SUCCESS);
    FF_TEST_ASSERT(ns == 2500000000ULL);

    /* 802.1Qav CBS is not exposed in the GEM register subset. */
    FF_TEST_ASSERT(t.tsn.configure_cbs(&t, 1u, 1000000u) == FF_NOT_IMPLEMENTED);

    /* 802.1Qci: per-stream filter via screener type 1 entry 0.
     * Maps dest MAC (00:01:02...) — not used by the GEM screener — to queue 2. */
    {
        const uint8_t dest_mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
        FF_TEST_ASSERT(t.tsn.set_stream_filter(&t, 2u, dest_mac, 100u, 0x0800u, 1u) == FF_SUCCESS);
        FF_TEST_ASSERT(t.tsn.set_stream_filter(&t, 0u, NULL, 100u, 0x0800u, 0u) ==
                       FF_INVALID_ARGUMENT);
        /* Out-of-range queue. */
        FF_TEST_ASSERT(t.tsn.set_stream_filter(&t, 8u, dest_mac, 100u, 0x0800u, 1u) ==
                       FF_INVALID_ARGUMENT);
        /* VID out of range. */
        FF_TEST_ASSERT(t.tsn.set_stream_filter(&t, 2u, dest_mac, 5000u, 0x0800u, 1u) ==
                       FF_INVALID_ARGUMENT);
        /* Disable the stream filter. */
        FF_TEST_ASSERT(t.tsn.set_stream_filter(&t, 2u, dest_mac, 100u, 0x0800u, 0u) == FF_SUCCESS);
    }

    /* 802.1Qbb: PFC not exposed in the GEM register subset. */
    FF_TEST_ASSERT(t.tsn.enable_pfc(&t, 0x01u) == FF_NOT_IMPLEMENTED);

    FF_TEST_ASSERT(t.tsn.get_time_ns(&t, NULL) == FF_INVALID_ARGUMENT);
}

FF_TEST_MAIN(test_tsn_vlan_and_time)
