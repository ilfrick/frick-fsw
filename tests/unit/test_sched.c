/*
 * frick-fsw - Application schedule dispatcher unit tests (M17)
 *
 * Covers SSS-REQ-030 / SRS-REQ-071: table validation, atomic activation,
 * per-entry periodic release, the NC-EXT-19 starvation regression (a longer
 * period must not be starved by a shorter one), bounded per-poll work, absolute
 * release targets (no cumulative drift), missed-slot accounting and real SB
 * delivery. Nominal and out-of-bound cases carry explicit expected results
 * (norm N22). Time is injected, so the results are deterministic; this is host
 * evidence only and does not establish target jitter, WCET or RTEMS behaviour.
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
    return t;
}

/* Deterministic dispatch driver: advance a virtual clock in fixed steps. */
static uint32_t ff_run_ticks(uint64_t start_ns, uint32_t ticks, uint32_t step_ms, uint32_t *per_msg,
                             uint32_t per_msg_cap)
{
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint32_t total = 0u;
    uint64_t now = start_ns;
    for (uint32_t k = 0u; k < ticks; k++) {
        now += (uint64_t)step_ms * FF_MS;
        uint16_t n = FF_SchedPoll(now, out, (uint16_t)FF_SCHED_MAX_ENTRIES, 1u);
        for (uint16_t i = 0u; i < n; i++) {
            if (per_msg != NULL) {
                for (uint32_t m = 0u; m < per_msg_cap; m++) {
                    if (out[i].msg_id == (FF_MsgId_t)(0x0A01u + m)) {
                        per_msg[m]++;
                    }
                }
            }
            total++;
        }
    }
    return total;
}

static void test_validate_rejects(void)
{
    FF_SchedTable_t t = ff_test_table(3u);

    FF_TEST_ASSERT(FF_SchedValidate(NULL, sizeof(t)) == FF_ERROR);
    FF_TEST_ASSERT(FF_SchedValidate(&t, sizeof(t) - 1u) == FF_ERROR);
    FF_TEST_ASSERT(FF_SchedValidate(&t, sizeof(t) + 1u) == FF_ERROR);

    FF_SchedTable_t bad = t;
    bad.magic = 0xDEADBEEFu;
    FF_TEST_ASSERT(FF_SchedValidate(&bad, sizeof(bad)) == FF_ERROR);

    bad = t;
    bad.count = (uint16_t)(FF_SCHED_MAX_ENTRIES + 1u);
    FF_TEST_ASSERT(FF_SchedValidate(&bad, sizeof(bad)) == FF_ERROR);

    bad = t;
    bad.entries[1].period_ms = 0u; /* zero period is out of bounds */
    FF_TEST_ASSERT(FF_SchedValidate(&bad, sizeof(bad)) == FF_ERROR);

    bad = t;
    bad.entries[1].payload_len = (uint16_t)(FF_SCHED_PAYLOAD_MAX + 1u);
    FF_TEST_ASSERT(FF_SchedValidate(&bad, sizeof(bad)) == FF_ERROR);

    bad = t;
    bad.entries[2].msg_id = bad.entries[0].msg_id; /* duplicate entry */
    FF_TEST_ASSERT(FF_SchedValidate(&bad, sizeof(bad)) == FF_ERROR);

    FF_TEST_ASSERT(FF_SchedValidate(&t, sizeof(t)) == FF_SUCCESS);

    /* Zero entries is a legal (inert) table. */
    FF_SchedTable_t empty;
    memset(&empty, 0, sizeof(empty));
    empty.magic = FF_SCHED_MAGIC;
    empty.count = 0u;
    FF_TEST_ASSERT(FF_SchedValidate(&empty, sizeof(empty)) == FF_SUCCESS);

    /* Maximum-size table is accepted. */
    FF_SchedTable_t full = ff_test_table(3u);
    full.count = FF_SCHED_MAX_ENTRIES;
    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        full.entries[i].msg_id = (FF_MsgId_t)(0x1000u + i);
        full.entries[i].period_ms = 1000u;
        full.entries[i].payload_len = 1u;
        full.entries[i].payload[0] = (uint8_t)i;
    }
    FF_TEST_ASSERT(FF_SchedValidate(&full, sizeof(full)) == FF_SUCCESS);
}

static void test_activate_atomic(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t good = ff_test_table(2u);
    FF_SchedTable_t bad = ff_test_table(2u);
    bad.magic = 0u;
    bad.entries[0].msg_id = 0x0B01u;
    bad.entries[0].payload[0] = 0xEEu;

    FF_TEST_ASSERT(FF_SchedActivate(&good, sizeof(good)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);

    /* A rejected activation must not disturb the active table. */
    FF_TEST_ASSERT(FF_SchedActivate(&bad, sizeof(bad)) == FF_ERROR);
    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);

    /* Polling after rejection must dispatch the old image's contents, never
     * the distinct message/payload from the rejected image. */
    FF_SchedInit(0u);
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    FF_TEST_ASSERT(FF_SchedPoll(1000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_TEST_ASSERT(out[0].msg_id == good.entries[0].msg_id);
    FF_TEST_ASSERT(out[0].msg_id != bad.entries[0].msg_id);
    FF_TEST_ASSERT(out[0].length == good.entries[0].payload_len);
    FF_TEST_ASSERT(out[0].payload[0] == good.entries[0].payload[0]);
    FF_TEST_ASSERT(out[0].payload[1] == good.entries[0].payload[1]);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.activations == 1u);
    FF_TEST_ASSERT(st.invalid_activations == 1u);
}

static void test_first_release_boundary(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u); /* 1000 ms */
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);

    FF_SchedInit(0u);
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];

    /* Before the period elapses nothing is released. */
    FF_TEST_ASSERT(FF_SchedPoll(999u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
    /* Exactly at the period the first release happens (inclusive boundary). */
    FF_TEST_ASSERT(FF_SchedPoll(1000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_TEST_ASSERT(out[0].msg_id == 0x0A01u);
    FF_TEST_ASSERT(out[0].length == 2u);
    FF_TEST_ASSERT(out[0].payload[0] == 0xAAu);
    FF_TEST_ASSERT(out[0].time_ns == 1000u * FF_MS);
}

/* NC-EXT-19 regression: the old stub used one shared timestamp, so the 1 s
 * entry reset it every tick and the 5 s / 10 s entries never fired at all. */
static void test_no_starvation(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(3u);
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    uint32_t per[3] = {0u, 0u, 0u};
    uint32_t total = ff_run_ticks(0u, 30u, 1000u, per, 3u);

    FF_TEST_ASSERT(per[0] == 30u); /* 30 s / 1000 ms */
    FF_TEST_ASSERT(per[1] == 6u);  /* 30 s / 5000 ms */
    FF_TEST_ASSERT(per[2] == 3u);  /* 30 s / 10000 ms - the starvation case */
    FF_TEST_ASSERT(total == 39u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 39u);
    FF_TEST_ASSERT(st.missed == 0u); /* exact 1 s polls skip nothing */
}

static void test_no_cumulative_drift(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u); /* 1000 ms */
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    /* Irregular polling (137 ms steps) over 10 s: release count must equal
     * floor(elapsed / period) because targets are absolute, not accumulated. */
    uint32_t per[1] = {0u};
    (void)ff_run_ticks(0u, 73u, 137u, per, 1u); /* 73 * 137 ms = 10001 ms */
    FF_TEST_ASSERT(per[0] == 10u);
}

static void test_missed_slots_bounded(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u); /* 1000 ms */
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    /* Jump 3.5 periods ahead in one poll: exactly one bounded release, three
     * skipped slots recorded, and no catch-up burst. */
    FF_TEST_ASSERT(FF_SchedPoll(4500u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_TEST_ASSERT(out[0].msg_id == t.entries[0].msg_id);
    FF_TEST_ASSERT(out[0].length == t.entries[0].payload_len);
    FF_TEST_ASSERT(out[0].payload[0] == t.entries[0].payload[0]);
    FF_TEST_ASSERT(out[0].payload[1] == t.entries[0].payload[1]);
    FF_TEST_ASSERT(out[0].time_ns == 4500u * FF_MS);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.missed == 3u);
    FF_TEST_ASSERT(st.late_polls == 1u);

    /* Next release is the absolute target 5000 ms, not now + period. */
    FF_TEST_ASSERT(FF_SchedPoll(4999u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
    FF_TEST_ASSERT(FF_SchedPoll(5000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_TEST_ASSERT(out[0].msg_id == t.entries[0].msg_id);
    FF_TEST_ASSERT(out[0].length == t.entries[0].payload_len);
    FF_TEST_ASSERT(out[0].payload[0] == t.entries[0].payload[0]);
    FF_TEST_ASSERT(out[0].payload[1] == t.entries[0].payload[1]);
    FF_TEST_ASSERT(out[0].time_ns == 5000u * FF_MS);
}

static void test_max_out_bound(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(3u); /* all due at 10 s */
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    /* Three entries are due but the caller only offers room for two. */
    FF_TEST_ASSERT(FF_SchedPoll(10000u * FF_MS, out, 2u, 1u) == 2u);
    /* The remaining one is released on the next poll. */
    FF_TEST_ASSERT(FF_SchedPoll(10000u * FF_MS, out, 2u, 1u) == 1u);

    /* max_out == 0 releases nothing and consumes no release slot. */
    FF_TEST_ASSERT(FF_SchedPoll(20000u * FF_MS, out, 0u, 1u) == 0u);
}

static void test_null_out_allowed(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(3u);
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    /* The application path passes out == NULL: releases still occur and are
     * counted, which keeps the 16 KiB scheduler task stack free of a large
     * receive buffer. All three entries are due at 10 s; jumping straight from
     * the epoch to 10 s skips 9 slots of the 1 s entry and 1 of the 5 s entry,
     * which must be counted rather than burst-released. */
    FF_TEST_ASSERT(FF_SchedPoll(10000u * FF_MS, NULL, FF_SCHED_MAX_ENTRIES, 1u) == 3u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 3u);
    FF_TEST_ASSERT(st.missed == 10u);
}

static void test_scheduler_zero_entries(void)
{
    /* Scheduler with zero entries in the table: poll must return 0 and not
     * crash or exhibit undefined behaviour. This is an out-of-bound case
     * per N22: valid invocation with an empty table. */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 0u;
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint16_t n = FF_SchedPoll(10000u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u);
    FF_TEST_ASSERT(n == 0u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 0u);
    FF_TEST_ASSERT(st.activations == 1u);
}

static void test_scheduler_activate_zero_count(void)
{
    /* Activate a table with count == 0: the scheduler must initialise without
     * error and poll must return 0 for any due time. */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 0u;
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    /* Advance past the epoch */
    uint16_t n = FF_SchedPoll(1u * FF_MS, out, FF_SCHED_MAX_ENTRIES, 1u);
    FF_TEST_ASSERT(n == 0u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 0u);
    FF_TEST_ASSERT(st.activations == 1u);
}

static void test_scheduler_poll_before_activation(void)
{
    /* Poll before any activation must return 0 regardless of the due time. */
    FF_SchedResetStats();
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 0u;
    /* Do not activate */

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint16_t n = FF_SchedPoll(1000000000000uLL, out, FF_SCHED_MAX_ENTRIES, 1u);
    FF_TEST_ASSERT(n == 0u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 0u);
    FF_TEST_ASSERT(st.activations == 0u);
}

static void test_scheduler_max_out_zero(void)
{
    /* Poll with max_out == 0 must return 0 and not write to the output buffer. */
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u);
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    uint16_t n = FF_SchedPoll(10000u * FF_MS, out, 0u, 1u);
    FF_TEST_ASSERT(n == 0u);

    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.dispatched == 0u);
}

static void test_before_init_no_dispatch(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u);
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    /* After activation the targets are re-initialised by FF_SchedInit; polling
     * with a time before the epoch must not release anything. */
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    FF_TEST_ASSERT(FF_SchedPoll(0u, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
}

static void test_time_exhaustion_does_not_wrap(void)
{
    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(1u); /* 1000 ms */
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);

    /* An initial deadline that cannot fit disables this entry for this epoch. */
    FF_SchedInit(UINT64_MAX - (500u * FF_MS));
    FF_Msg_t out[FF_SCHED_MAX_ENTRIES];
    FF_TEST_ASSERT(FF_SchedPoll(UINT64_MAX, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
    FF_SchedStats_t st;
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.time_exhaustions == 1u);

    /* A valid first release may dispatch once, but its overflowing next target
     * must not wrap and cause another early dispatch. */
    FF_SchedResetStats();
    FF_SchedInit(UINT64_MAX - (1500u * FF_MS));
    uint64_t due = UINT64_MAX - (500u * FF_MS);
    FF_TEST_ASSERT(FF_SchedPoll(due, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_TEST_ASSERT(out[0].msg_id == t.entries[0].msg_id);
    FF_TEST_ASSERT(FF_SchedPoll(UINT64_MAX, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.time_exhaustions == 1u);
    FF_TEST_ASSERT(st.dispatched == 1u);

    /* A representable deadline exactly at UINT64_MAX remains a valid release. */
    FF_SchedResetStats();
    FF_SchedInit(UINT64_MAX - (1000u * FF_MS));
    FF_TEST_ASSERT(FF_SchedPoll(UINT64_MAX - 1u, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
    FF_TEST_ASSERT(FF_SchedPoll(UINT64_MAX, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_TEST_ASSERT(out[0].msg_id == t.entries[0].msg_id);
    FF_TEST_ASSERT(FF_SchedPoll(UINT64_MAX, out, FF_SCHED_MAX_ENTRIES, 1u) == 0u);
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.time_exhaustions == 1u);
    FF_TEST_ASSERT(st.dispatched == 1u);

    /* Huge elapsed time cannot wrap the 32-bit missed-slot diagnostic. */
    FF_SchedResetStats();
    FF_SchedInit(0u);
    FF_TEST_ASSERT(FF_SchedPoll(UINT64_MAX, out, FF_SCHED_MAX_ENTRIES, 1u) == 1u);
    FF_SchedGetStats(&st);
    FF_TEST_ASSERT(st.missed == UINT32_MAX);
    FF_TEST_ASSERT(st.time_exhaustions == 1u);
}

static void test_real_sb_delivery(void)
{
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(1u, 16u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A02u, 1u) == FF_SUCCESS);

    FF_SchedResetStats();
    FF_SchedTable_t t = ff_test_table(2u);
    FF_TEST_ASSERT(FF_SchedActivate(&t, sizeof(t)) == FF_SUCCESS);
    FF_SchedInit(0u);

    uint32_t per[2] = {0u, 0u};
    (void)ff_run_ticks(0u, 5u, 1000u, per, 2u);

    /* The 5 s entry must be delivered over the real bus exactly once. */
    FF_Msg_t got;
    uint32_t received = 0u;
    while (FF_SbReceive(1u, &got, 0u) == FF_SUCCESS) {
        FF_TEST_ASSERT(got.msg_id == 0x0A02u);
        received++;
    }
    FF_TEST_ASSERT(received == 1u);
    FF_TEST_ASSERT(got.payload[0] == 0x42u);
    FF_TEST_ASSERT(got.apid == 1u);
}

FF_TEST_MAIN(test_validate_rejects, test_activate_atomic, test_first_release_boundary,
             test_no_starvation, test_no_cumulative_drift, test_missed_slots_bounded,
             test_max_out_bound, test_null_out_allowed, test_before_init_no_dispatch,
             test_time_exhaustion_does_not_wrap, test_real_sb_delivery, test_scheduler_zero_entries,
             test_scheduler_activate_zero_count, test_scheduler_poll_before_activation,
             test_scheduler_max_out_zero)
