/*
 * frick-fsw - Scheduler statistics accumulation test (M17)
 *
 * Verifies that scheduler statistics (activations, dispatched, missed,
 * late_polls, time_exhaustions, publish_errors) accumulate correctly
 * across multiple activations unless explicitly reset via
 * FF_SchedResetStats().
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/sched/ff_sched.h"
#include <string.h>

#define FF_MS 1000000ULL

static FF_SchedTable_t ff_test_table(uint16_t count)
{
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = count;
    if (count > 0u) {
        t.entries[0].msg_id = 0x0A01u;
        t.entries[0].period_ms = 1000u;
        t.entries[0].payload_len = 2u;
        t.entries[0].payload[0] = 0xAAu;
        t.entries[0].payload[1] = 0xBBu;
    }
    if (count > 1u) {
        t.entries[1].msg_id = 0x0A02u;
        t.entries[1].period_ms = 5000u;
        t.entries[1].payload_len = 1u;
        t.entries[1].payload[0] = 0x42u;
    }
    if (count > 2u) {
        t.entries[2].msg_id = 0x0A03u;
        t.entries[2].period_ms = 10000u;
        t.entries[2].payload_len = 4u;
        t.entries[2].payload[0] = 0x01u;
    }
    if (count > 3u) {
        t.entries[3].msg_id = 0x0A04u;
        t.entries[3].period_ms = 3000u;
        t.entries[3].payload_len = 1u;
        t.entries[3].payload[0] = 0x99u;
    }
    return t;
}

static void test_stats_accumulate_across_activations(void)
{
    /* Statistics must accumulate across multiple activations unless explicitly
     * reset. This verifies that activations, dispatched, missed, late_polls,
     * time_exhaustions, and publish_errors are cumulative across the scheduler
     * lifetime unless FF_SchedResetStats() is called. */
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u); /* 1000 ms period */

    /* First activation + polling */
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];

    /* First period: 1 release */
    FF_TEST_ASSERT(FF_SchedPoll(1000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    /* Second period: 1 more release */
    FF_TEST_ASSERT(FF_SchedPoll(2000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.activations == 1u);
    FF_TEST_ASSERT(st.dispatched == 2u);
    FF_TEST_ASSERT(st.missed == 0u);
    FF_TEST_ASSERT(st.late_polls == 0u);
    FF_TEST_ASSERT(st.time_exhaustions == 0u);

    /* Second activation: should NOT reset accumulated stats */
    FF_SchedTable_t t2 = ff_test_table(1u);
    t2.entries[0].msg_id = 0x0A05u; /* different msg_id to distinguish */
    FF_TEST_ASSERT(FF_SchedActivate(&t2, sizeof(t2)) == FF_SUCCESS);
    FF_SchedInit(0u);

    /* Advance 1.5 periods: 1 release */
    FF_TEST_ASSERT(FF_SchedPoll(1500u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);

    FF_SchedGetStats(&st);
    /* Activations should be 2, dispatched should be 3 (2+1) */
    FF_TEST_ASSERT(st.activations == 2u);
    FF_TEST_ASSERT(st.dispatched == 3u);
    FF_TEST_ASSERT(st.missed == 0u);
    FF_TEST_ASSERT(st.late_polls == 0u);

    /* Third activation with missed slots to test accumulation of missed/late */
    FF_SchedTable_t t3 = ff_test_table(1u);
    t3.entries[0].msg_id = 0x0A06u;
    FF_TEST_ASSERT(FF_SchedActivate(&t3, sizeof(t3)) == FF_SUCCESS);
    FF_SchedInit(0u);

    /* Jump 3.5 periods: 1 release, 3 missed slots */
    FF_TEST_ASSERT(FF_SchedPoll(4500u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);

    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.activations == 3u);
    FF_TEST_ASSERT(st.dispatched == 4u); /* 2+1+1 */
    FF_TEST_ASSERT(st.missed == 3u);
    FF_TEST_ASSERT(st.late_polls == 1u);

    /* Now test that FF_SchedResetStats() actually resets everything */
    FF_SchedResetStats();
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.activations == 0u);
    FF_TEST_ASSERT(st.dispatched == 0u);
    FF_TEST_ASSERT(st.missed == 0u);
    FF_TEST_ASSERT(st.late_polls == 0u);
    FF_TEST_ASSERT(st.time_exhaustions == 0u);
    FF_TEST_ASSERT(st.publish_errors == 0u);

    /* Verify activations counter still works after reset */
    FF_SchedTable_t t4 = ff_test_table(1u);
    t4.entries[0].msg_id = 0x0A07u;
    FF_TEST_ASSERT(FF_SchedActivate(&t4, sizeof(t4)) == FF_SUCCESS);
    FF_SchedInit(0u);
    FF_TEST_ASSERT(FF_SchedPoll(1000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);

    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.activations == 1u);
    FF_TEST_ASSERT(st.dispatched == 1u);
    FF_TEST_ASSERT(st.missed == 0u);
    FF_TEST_ASSERT(st.late_polls == 0u);
}

FF_TEST_MAIN(test_stats_accumulate_across_activations)
