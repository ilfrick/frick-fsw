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

static void test_scheduler_max_entries_activation_reject(void)
{
    /* count == FF_SCHED_MAX_ENTRIES + 1 must be rejected.
     * We only populate FF_SCHED_MAX_ENTRIES entries but set count to max+1. */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES + 1u;

    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x3000u + i);
        t.entries[i].period_ms = 1000u;
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_ERROR);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.invalid_activations == 1u);
}

static void test_scheduler_max_entries_partial_poll(void)
{
    /* max_out < FF_SCHED_MAX_ENTRIES with all entries due -> partial emission and carry-over */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x4000u + i);
        t.entries[i].period_ms = (uint16_t)(1000u + i);
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    uint64_t max_period_ns = (uint64_t)(1000u + FF_SCHED_MAX_ENTRIES - 1u) * FF_MS;
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];

    /* First poll: emit half */
    uint16_t n = FF_SchedPoll(max_period_ns, out, FF_SCHED_MAX_ENTRIES / 2u, 1u);
    FF_TEST_ASSERT(n == FF_SCHED_MAX_ENTRIES / 2u);

    /* Second poll: emit the rest */
    n = FF_SchedPoll(max_period_ns, out, FF_SCHED_MAX_ENTRIES / 2u, 1u);
    FF_TEST_ASSERT(n == FF_SCHED_MAX_ENTRIES / 2u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == FF_SCHED_MAX_ENTRIES);
    FF_TEST_ASSERT(st.activations == 1u);
}

static void test_scheduler_max_entries_duplicate_msgid(void)
{
    /* Duplicate msg_id within max-entries table must be rejected */
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = 0x5000u; /* all same */
        t.entries[i].period_ms = (uint16_t)(1000u + i);
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_ERROR);
}

static void test_scheduler_max_entries_zero_period(void)
{
    /* Zero period_ms in any max-entries slot must be rejected */
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x6000u + i);
        t.entries[i].period_ms = (i == 0u) ? 0u : (uint16_t)(1000u + i);
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_ERROR);
}

static void test_scheduler_max_entries_payload_too_large(void)
{
    /* payload_len > FF_SCHED_PAYLOAD_MAX at max entries must be rejected */
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x7000u + i);
        t.entries[i].period_ms = (uint16_t)(1000u + i);
        t.entries[i].payload_len = (uint16_t)(FF_SCHED_PAYLOAD_MAX + 1u);
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_ERROR);
}

static void test_scheduler_max_entries_poll_before_init(void)
{
    /* Poll before FF_SchedInit with max entries active -> zero dispatch */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x8000u + i);
        t.entries[i].period_ms = (uint16_t)(1000u + i);
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    /* Do NOT call FF_SchedInit */

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint16_t n = FF_SchedPoll(1000000000000uLL, out, FF_SCHED_MAX_ENTRIES, 1u);
    FF_TEST_ASSERT(n == 0u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 0u);
    FF_TEST_ASSERT(st.activations == 1u);
}

static void test_scheduler_max_entries_multi_poll(void)
{
    /* Multiple polls with max entries to exercise missed-slot accounting and late-poll stats.
     * Use uniform period for predictable missed-slot behavior. */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = FF_SCHED_MAX_ENTRIES;

    /* Use uniform period (1000ms) for predictable missed-slot behavior */
    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        t.entries[i].msg_id = (FF_MsgId_t)(0x9000u + i);
        t.entries[i].period_ms = 1000u; /* uniform 1s period */
        t.entries[i].payload_len = 1u;
        t.entries[i].payload[0] = (uint8_t)i;
    }

    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint64_t period_ns = 1000u * FF_MS;

    /* Poll 1: at 1 period, all due, emit half */
    uint16_t n = FF_SchedPoll(period_ns, out, FF_SCHED_MAX_ENTRIES / 2u, 1u);
    FF_TEST_ASSERT(n == FF_SCHED_MAX_ENTRIES / 2u);

    /* Poll 2: same time, emit rest */
    n = FF_SchedPoll(period_ns, out, FF_SCHED_MAX_ENTRIES / 2u, 1u);
    FF_TEST_ASSERT(n == FF_SCHED_MAX_ENTRIES / 2u);

    /* Poll 3: jump 2 periods ahead (to 3s), all entries missed one slot (2s was missed) */
    n = FF_SchedPoll(3u * 1000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u);
    FF_TEST_ASSERT(n == FF_SCHED_MAX_ENTRIES);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 2u * FF_SCHED_MAX_ENTRIES); /* 8+8+16 = 32 = 2*16 */
    FF_TEST_ASSERT(st.missed == FF_SCHED_MAX_ENTRIES); /* one missed slot per entry */
    FF_TEST_ASSERT(st.late_polls == FF_SCHED_MAX_ENTRIES); /* one late poll per entry */
}

FF_TEST_MAIN(test_scheduler_max_entries, test_scheduler_max_entries_activation_reject, test_scheduler_max_entries_partial_poll,
             test_scheduler_max_entries_duplicate_msgid, test_scheduler_max_entries_zero_period,
             test_scheduler_max_entries_payload_too_large, test_scheduler_max_entries_poll_before_init,
             test_scheduler_max_entries_multi_poll)
