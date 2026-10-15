/*
 * frick-fsw - Software Bus / EVS parameterized structural characterization
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t g_listener_calls;

static void sweep_listener(const FF_EvsEvent_t *event)
{
    (void)event;
    g_listener_calls++;
}

static bool parse_uint(const char *text, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(text, &end, 10);
    if ((errno != 0) || (end == text) || (*end != '\0') || (parsed > UINT32_MAX)) {
        return false;
    }
    *value = (uint32_t)parsed;
    return true;
}

static int run_profile(uint32_t fanout, uint32_t depth, uint32_t overflow_publishes,
                       bool dispatch_sweep)
{
    const FF_MsgId_t fill_msg_id = 0x1000u;
    const uint32_t overflow_events = fanout * overflow_publishes;
    const uint32_t expected_queued =
        (overflow_events < FF_EVS_LOG_DEPTH) ? overflow_events : FF_EVS_LOG_DEPTH;
    const uint32_t expected_log = expected_queued;
    const uint32_t expected_dropped = overflow_events - expected_queued;
    FF_Msg_t msg;
    FF_SbStats_t sb_stats;
    FF_EvsNotifyStats_t evs_stats;
    FF_EvsEvent_t log[FF_EVS_LOG_DEPTH];
    uint32_t expected_dispatch = 0u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_EvsSetListener(sweep_listener);
    g_listener_calls = 0u;

    for (uint32_t i = 0u; i < fanout; i++) {
        const FF_AppId_t app = (FF_AppId_t)(100u + i);
        FF_TEST_ASSERT(FF_SbCreatePipe(app, (uint16_t)depth) == FF_SUCCESS);
        FF_TEST_ASSERT(FF_SbSubscribe(FF_SB_WILDCARD, app) == FF_SUCCESS);
    }

    memset(&msg, 0, sizeof(msg));
    msg.msg_id = fill_msg_id;
    for (uint32_t i = 0u; i < depth; i++) {
        FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_SbGetStats(&sb_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(sb_stats.published == depth);
    FF_TEST_ASSERT(sb_stats.delivered == fanout * depth);
    FF_TEST_ASSERT(sb_stats.dropped == 0u);

    /* Select the smallest integral burst count that exceeds EVS's configured
     * deferred-notification capacity for this fan-out. */
    for (uint32_t burst = 0u; burst < overflow_publishes; burst++) {
        msg.msg_id = (FF_MsgId_t)(0x2000u + burst);
        FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    }

    FF_TEST_ASSERT(overflow_events <= FF_EVS_LOG_DEPTH + fanout);
    FF_TEST_ASSERT(FF_SbGetStats(&sb_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(sb_stats.published == depth + overflow_publishes);
    FF_TEST_ASSERT(sb_stats.delivered == fanout * depth);
    FF_TEST_ASSERT(sb_stats.dropped == overflow_events);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == overflow_events);

    FF_TEST_ASSERT(FF_EvsGetLog(log, FF_EVS_LOG_DEPTH) == expected_log);
    for (uint32_t i = 0u; i < expected_log; i++) {
        const uint32_t global_index = overflow_events - expected_log + i;
        const uint32_t expected_burst = global_index / fanout;
        const uint32_t expected_pipe = global_index % fanout;
        char expected_message_id[16];

        (void)snprintf(expected_message_id, sizeof(expected_message_id), "0x%04X",
                       (unsigned)(0x2000u + expected_burst));
        FF_TEST_ASSERT(log[i].event_id == FF_EVS_SB_ERR);
        FF_TEST_ASSERT(log[i].app_id == (FF_AppId_t)(100u + expected_pipe));
        FF_TEST_ASSERT(strstr(log[i].text, expected_message_id) != NULL);
    }

    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(evs_stats.queued == expected_queued);
    FF_TEST_ASSERT(evs_stats.dispatched == 0u);
    FF_TEST_ASSERT(evs_stats.dropped == expected_dropped);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(FF_EVS_LOG_DEPTH) == expected_queued);
    expected_dispatch = expected_queued;
    FF_TEST_ASSERT(g_listener_calls == expected_dispatch);

    if (dispatch_sweep) {
        /* For limits below queue capacity, leave one event pending after the
         * bounded dispatch and verify the remainder is still dispatchable.
         * At the maximum bound, the queue cannot hold more than the limit. */
        for (uint32_t batch = 1u; batch <= FF_EVS_LOG_DEPTH; batch++) {
            const uint32_t prior_queued = evs_stats.queued;
            const uint32_t pending = (batch < FF_EVS_LOG_DEPTH) ? batch + 1u : batch;
            for (uint32_t i = 0u; i < pending; i++) {
                FF_TEST_ASSERT(FF_EvsSendDeferred((uint16_t)(0xA000u + batch), FF_EVS_INFO, 7u,
                                                  "dispatch batch sweep") == FF_SUCCESS);
            }
            FF_TEST_ASSERT(FF_EvsDispatchPending(batch) == batch);
            expected_dispatch += batch;
            FF_TEST_ASSERT(g_listener_calls == expected_dispatch);
            FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_stats) == FF_SUCCESS);
            FF_TEST_ASSERT(evs_stats.queued == prior_queued + pending);
            FF_TEST_ASSERT(evs_stats.dispatched == expected_dispatch);
            FF_TEST_ASSERT(evs_stats.dropped == expected_dropped);
            if (batch < FF_EVS_LOG_DEPTH) {
                FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 1u);
                expected_dispatch++;
                FF_TEST_ASSERT(g_listener_calls == expected_dispatch);
                FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_stats) == FF_SUCCESS);
                FF_TEST_ASSERT(evs_stats.dispatched == expected_dispatch);
                FF_TEST_ASSERT(evs_stats.queued == prior_queued + pending);
                FF_TEST_ASSERT(evs_stats.dropped == expected_dropped);
            }
        }
    }

    (void)printf("SB_EVS_PROFILE fanout=%u depth=%u overflow_publishes=%u "
                 "overflow_events=%u notify_capacity=%u notify_dropped=%u "
                 "dispatch_sweep=%u dispatch_calls=%u checks=%d failures=%d\n",
                 (unsigned)fanout, (unsigned)depth, (unsigned)overflow_publishes,
                 (unsigned)overflow_events, (unsigned)FF_EVS_LOG_DEPTH, (unsigned)expected_dropped,
                 dispatch_sweep ? 1u : 0u, (unsigned)expected_dispatch, ff_test_checks,
                 ff_test_failures);
    return (ff_test_failures == 0) ? 0 : 1;
}

int main(int argc, char **argv)
{
    uint32_t fanout;
    uint32_t depth;
    uint32_t overflow_publishes;
    uint32_t saturation_publishes;
    bool dispatch_sweep = false;

    if ((argc == 2) && (strcmp(argv[1], "--describe") == 0)) {
        (void)printf("SB_EVS_CONFIG max_pipes=%u max_depth=%u evs_depth=%u\n",
                     (unsigned)FF_SB_MAX_PIPES, (unsigned)FF_SB_QUEUE_DEPTH_MAX,
                     (unsigned)FF_EVS_LOG_DEPTH);
        return 0;
    }
    if ((argc < 3) || (argc > 5)) {
        (void)fprintf(stderr, "usage: %s FANOUT DEPTH [OVERFLOW_PUBLISHES] [--dispatch-sweep]\n",
                      argv[0]);
        return 2;
    }
    if (!parse_uint(argv[1], &fanout) || !parse_uint(argv[2], &depth) || (fanout == 0u) ||
        (fanout > FF_SB_MAX_PIPES) || (depth == 0u) || (depth > FF_SB_QUEUE_DEPTH_MAX)) {
        (void)fprintf(stderr, "fanout/depth outside configured structural bounds\n");
        return 2;
    }
    saturation_publishes = (FF_EVS_LOG_DEPTH + fanout) / fanout;
    overflow_publishes = saturation_publishes;
    if (argc >= 4) {
        if (strcmp(argv[3], "--dispatch-sweep") == 0) {
            dispatch_sweep = true;
        }
        else if (!parse_uint(argv[3], &overflow_publishes) ||
                 (overflow_publishes > saturation_publishes)) {
            (void)fprintf(stderr, "overflow-publish count exceeds saturation boundary\n");
            return 2;
        }
    }
    if (argc == 5) {
        if (strcmp(argv[4], "--dispatch-sweep") != 0) {
            (void)fprintf(stderr, "unknown option: %s\n", argv[4]);
            return 2;
        }
        dispatch_sweep = true;
    }
    return run_profile(fanout, depth, overflow_publishes, dispatch_sweep);
}
