/*
 * frick-fsw - Scheduler maximum entries boundary test (M17)
 *
 * Verifies that the scheduler correctly handles FF_SCHED_MAX_ENTRIES
 * entries with unique periods, all dispatched in a single poll when
 * their periods elapse.
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

static void test_scheduler_max_entries(void)
{
    /* Boundary test: activate a table with FF_SCHED_MAX_ENTRIES entries,
     * each with a unique period, and poll with max_out == FF_SCHED_MAX_ENTRIES.
     * All entries should be dispatched in a single poll when their period elapses. */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    /* Fill with unique periods to exercise all slots */
    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x2000u + i);
        t.entries[i].period_ms = (uint16_t)(1000u + i); /* unique periods 1000..1000+N-1 */
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    /* Advance to the longest period so all entries are due */
    uint64_t max_period_ns = (uint64_t)(1000u + FF_SCHED_MAX_ENTRIES - 1u) * FF_MS;
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint16_t n = FF_SchedPoll(max_period_ns, out, FF_SCHED_MAX_ENTRIES, 1u);
    FF_TEST_ASSERT(n == FF_SCHED_MAX_ENTRIES);

    /* Verify all msg_ids are present */
    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        bool found = false;
        for (uint16_t j = 0u; j < n; j++) {
            if (out[j].msg_id == (FF_MsgId_t)(0x2000u + i)) {
                found = true;
                FF_TEST_ASSERT(out[j].length == 1u);
                FF_TEST_ASSERT(out[j].payload[0] == (uint8_t)i);
                break;
            }
        }
        FF_TEST_ASSERT(found);
    }

    /* Stats should reflect all dispatches */
    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == FF_SCHED_MAX_ENTRIES);
    FF_TEST_ASSERT(st.activations == 1u);
}

FF_TEST_MAIN(test_scheduler_max_entries)
