/*
 * frick-fsw - Software Bus configured-limit stress profile
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include <string.h>

static uint32_t g_listener_calls;
static uint32_t g_listener_depth;
static uint32_t g_listener_max_depth;

static void stress_listener(const FF_EvsEvent_t *event)
{
    (void)event;
    g_listener_depth++;
    if (g_listener_depth > g_listener_max_depth) {
        g_listener_max_depth = g_listener_depth;
    }
    g_listener_calls++;
    g_listener_depth--;
}

/* Stress profile uses configured structural maxima, not mission event-rate or
 * latency requirements. It holds all 32 subscriber queue locks, then fills 32
 * queues of depth 32, then causes three 32-event overflow bursts against EVS. */
static void test_max_configured_fanout_and_evs_saturation_boundary(void)
{
    const FF_MsgId_t msg_id = 0x0AFFu;
    const uint32_t pipes = FF_SB_MAX_PIPES;
    FF_Msg_t msg;
    FF_SbStats_t sb_stats;
    FF_EvsNotifyStats_t evs_stats;
    FF_EvsEvent_t log[FF_EVS_LOG_DEPTH];

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_EvsSetListener(stress_listener);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;

    for (uint32_t i = 0u; i < pipes; i++) {
        const FF_AppId_t app = (FF_AppId_t)(100u + i);
        FF_TEST_ASSERT(FF_SbCreatePipe(app, FF_SB_QUEUE_DEPTH_MAX) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_SbSubscribe(FF_SB_WILDCARD, app) == FF_SUCCESS);
    }

    memset(&msg, 0, sizeof(msg));
    msg.msg_id = msg_id;

    /* Worst structural contention profile: hold every matched queue lock.
     * Publish must account one drop per subscriber and return before release. */
    for (uint32_t i = 0u; i < pipes; i++) {
        FF_TEST_ASSERT(FF_SbTestQueueLock((FF_AppId_t)(100u + i)) == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_SbGetStats(&sb_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(sb_stats.published == 1u);
    FF_TEST_ASSERT(sb_stats.delivered == 0u);
    FF_TEST_ASSERT(sb_stats.dropped == pipes);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(evs_stats.queued == pipes);
    FF_TEST_ASSERT(evs_stats.dispatched == 0u && evs_stats.dropped == 0u);
    for (uint32_t i = 0u; i < pipes; i++) {
        FF_TEST_ASSERT(FF_SbTestQueueUnlock((FF_AppId_t)(100u + i)) == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_EvsDispatchPending(pipes) == pipes);
    FF_TEST_ASSERT(g_listener_calls == pipes);
    FF_TEST_ASSERT(g_listener_max_depth == 1u);

    /* Fill every configured subscriber queue exactly to capacity. */
    for (uint32_t i = 0u; i < FF_SB_QUEUE_DEPTH_MAX; i++) {
        FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_SbGetStats(&sb_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(sb_stats.published == FF_SB_QUEUE_DEPTH_MAX + 1u);
    FF_TEST_ASSERT(sb_stats.delivered == pipes * FF_SB_QUEUE_DEPTH_MAX);
    FF_TEST_ASSERT(sb_stats.dropped == pipes);

    /* Three full-queue bursts create 96 SB drops and event-log entries. EVS
     * accepts the first 64 notifications and accounts for the remaining 32;
     * with 32 earlier busy-lock events, the ring now contains 128 records and
     * retains their newest 64. */
    for (uint32_t burst = 0u; burst < 3u; burst++) {
        /* A distinct wildcard-routed message id identifies each burst in the
         * event text, making the retained-log suffix independently testable. */
        msg.msg_id = (FF_MsgId_t)(0x0B00u + burst);
        FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_SbGetStats(&sb_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(sb_stats.published == FF_SB_QUEUE_DEPTH_MAX + 4u);
    FF_TEST_ASSERT(sb_stats.delivered == pipes * FF_SB_QUEUE_DEPTH_MAX);
    FF_TEST_ASSERT(sb_stats.dropped == pipes * 4u);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == pipes * 4u);
    FF_TEST_ASSERT(FF_EvsGetLog(log, FF_EVS_LOG_DEPTH) == FF_EVS_LOG_DEPTH);
    for (uint32_t i = 0u; i < FF_EVS_LOG_DEPTH; i++) {
        FF_TEST_ASSERT(log[i].event_id == FF_EVS_SB_ERR);
        FF_TEST_ASSERT(log[i].app_id == (FF_AppId_t)(100u + (i % pipes)));
        FF_TEST_ASSERT(strstr(log[i].text, "pipe full") != NULL);
        FF_TEST_ASSERT(strstr(log[i].text, (i < pipes) ? "0x0B01" : "0x0B02") != NULL);
    }
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(evs_stats.queued == pipes + FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(evs_stats.dispatched == pipes);
    FF_TEST_ASSERT(evs_stats.dropped == pipes * 3u - FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(g_listener_calls == pipes);

    FF_TEST_ASSERT(FF_EvsDispatchPending(FF_EVS_LOG_DEPTH) == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(g_listener_calls == pipes + FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(g_listener_max_depth == 1u);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(evs_stats.dispatched == pipes + FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(evs_stats.dropped == pipes * 3u - FF_EVS_LOG_DEPTH);
}

FF_TEST_MAIN(test_max_configured_fanout_and_evs_saturation_boundary)
