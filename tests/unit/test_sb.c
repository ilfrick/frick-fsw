/*
 * frick-fsw - Software Bus unit tests
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
static bool g_listener_reenter;
static bool g_listener_reenter_dispatch;
static uint32_t g_listener_nested_dispatch_result;
static FF_Msg_t g_listener_reentry_message;
static FF_EvsEvent_t g_listener_event;

static void test_event_listener(const FF_EvsEvent_t *event)
{
    (void)event;
    g_listener_depth++;
    if (g_listener_depth > g_listener_max_depth) {
        g_listener_max_depth = g_listener_depth;
    }
    g_listener_calls++;
    g_listener_event = *event;
    if (g_listener_reenter_dispatch) {
        g_listener_reenter_dispatch = false;
        g_listener_nested_dispatch_result = FF_EvsDispatchPending(FF_EVS_LOG_DEPTH);
    }
    if (g_listener_reenter) {
        g_listener_reenter = false;
        (void)FF_SbPublish(&g_listener_reentry_message);
    }
    g_listener_depth--;
}

static void test_delivery(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    FF_AppId_t a1 = 1u, a2 = 2u;
    FF_TEST_ASSERT(FF_SbCreatePipe(a1, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(a2, 4u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbSubscribe(0x0A01u, a1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A01u, a2) == FF_SUCCESS);

    FF_Msg_t m;
    memset(&m, 0, sizeof(m));
    m.msg_id = 0x0A01u;
    m.length = 2u;
    m.payload[0] = 0xDEu;
    m.payload[1] = 0xADu;
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);

    FF_Msg_t r;
    memset(&r, 0, sizeof(r));
    FF_TEST_ASSERT(FF_SbReceive(a1, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x0A01u);
    FF_TEST_ASSERT(r.length == 2u);
    FF_TEST_ASSERT(r.payload[0] == 0xDEu && r.payload[1] == 0xADu);

    FF_TEST_ASSERT(FF_SbReceive(a2, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x0A01u);

    /* No message left in queues on a non matching ID. */
}

static void test_filtering(void)
{
    FF_AppId_t a3 = 3u;
    FF_TEST_ASSERT(FF_SbCreatePipe(a3, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A02u, a3) == FF_SUCCESS);

    FF_Msg_t m;
    memset(&m, 0, sizeof(m));
    m.msg_id = 0x0A03u; /* different id: must NOT be delivered */
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);

    FF_Msg_t r;
    FF_TEST_ASSERT(FF_SbReceive(a3, &r, 10u) == FF_TIMEOUT);
}

static void test_stats(void)
{
    FF_SbStats_t s;
    FF_TEST_ASSERT(FF_SbGetStats(&s) == FF_SUCCESS);
    FF_TEST_ASSERT(s.published >= 2u);
    FF_TEST_ASSERT(s.delivered >= 2u);
}

static void test_full_pipe_drop_raises_evs_error(void)
{
    const FF_AppId_t subscriber = 4u;
    const FF_AppId_t observer = 5u;
    FF_Msg_t first;
    FF_Msg_t overflow;
    FF_Msg_t received;
    FF_SbStats_t stats;
    FF_EvsEvent_t events[FF_EVS_LOG_DEPTH];

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;
    g_listener_reenter = false;
    memset(&g_listener_event, 0, sizeof(g_listener_event));
    FF_EvsSetListener(test_event_listener);
    FF_TEST_ASSERT(FF_SbCreatePipe(subscriber, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(observer, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A04u, subscriber) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A04u, observer) == FF_SUCCESS);

    memset(&first, 0, sizeof(first));
    first.msg_id = 0x0A04u;
    first.length = 1u;
    first.payload[0] = 0x11u;
    overflow = first;
    overflow.payload[0] = 0x22u;

    FF_TEST_ASSERT(FF_SbPublish(&first) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbReceive(observer, &received, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(received.payload[0] == first.payload[0]);
    FF_TEST_ASSERT(FF_SbPublish(&overflow) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbGetStats(&stats) == FF_SUCCESS);
    FF_TEST_ASSERT(stats.published == 2u);
    FF_TEST_ASSERT(stats.delivered == 3u);
    FF_TEST_ASSERT(stats.dropped == 1u);

    FF_TEST_ASSERT(FF_SbReceive(subscriber, &received, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(received.msg_id == first.msg_id);
    FF_TEST_ASSERT(received.payload[0] == first.payload[0]);
    FF_TEST_ASSERT(FF_SbReceive(subscriber, &received, 0u) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_SbReceive(observer, &received, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(received.payload[0] == overflow.payload[0]);

    FF_TEST_ASSERT(FF_EvsGetTotalCount() == 1u);
    FF_TEST_ASSERT(FF_EvsGetLog(events, FF_EVS_LOG_DEPTH) == 1u);
    FF_TEST_ASSERT(events[0].event_id == FF_EVS_SB_ERR);
    FF_TEST_ASSERT(events[0].type == FF_EVS_ERROR);
    FF_TEST_ASSERT(events[0].app_id == subscriber);
    FF_TEST_ASSERT(strstr(events[0].text, "pipe full") != NULL);
    FF_TEST_ASSERT(strstr(events[0].text, "0x0A04") != NULL);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 1u);
    FF_TEST_ASSERT(g_listener_calls == 1u);
    FF_TEST_ASSERT(g_listener_event.event_id == FF_EVS_SB_ERR);
    FF_TEST_ASSERT(g_listener_event.type == (uint8_t)FF_EVS_ERROR);
    FF_TEST_ASSERT(g_listener_event.app_id == subscriber);
    FF_TEST_ASSERT(strstr(g_listener_event.text, "0x0A04") != NULL);
}

static void test_deferred_listener_reentry_is_bounded(void)
{
    const FF_AppId_t subscriber = 6u;
    FF_Msg_t msg;
    FF_EvsNotifyStats_t stats;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(subscriber, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A05u, subscriber) == FF_SUCCESS);
    FF_EvsSetListener(test_event_listener);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;
    memset(&g_listener_event, 0, sizeof(g_listener_event));
    memset(&msg, 0, sizeof(msg));
    msg.msg_id = 0x0A05u;
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    g_listener_reentry_message = msg;
    g_listener_reenter = true;
    g_listener_reenter_dispatch = true;
    g_listener_nested_dispatch_result = UINT32_MAX;

    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 1u);
    FF_TEST_ASSERT(g_listener_calls == 1u);
    FF_TEST_ASSERT(g_listener_max_depth == 1u);
    FF_TEST_ASSERT(g_listener_nested_dispatch_result == 0u);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == 2u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 1u);
    FF_TEST_ASSERT(g_listener_calls == 2u);
    FF_TEST_ASSERT(g_listener_max_depth == 1u);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&stats) == FF_SUCCESS);
    FF_TEST_ASSERT(stats.queued == 2u && stats.dispatched == 2u && stats.dropped == 0u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 0u);
}

static void test_event_notification_queue_saturation_is_reported(void)
{
    FF_EvsNotifyStats_t stats;
    FF_EvsEvent_t log[FF_EVS_LOG_DEPTH];
    uint32_t i;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_EvsSetListener(test_event_listener);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;
    memset(&g_listener_event, 0, sizeof(g_listener_event));
    for (i = 0u; i < FF_EVS_LOG_DEPTH; i++) {
        FF_TEST_ASSERT(FF_EvsSendDeferred((uint16_t)i, FF_EVS_ERROR, 1u, "queued") == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_EvsSendDeferred(0xFFFFu, FF_EVS_ERROR, 1u, "overflow") == FF_NO_RESOURCE);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == FF_EVS_LOG_DEPTH + 1u);
    FF_TEST_ASSERT(FF_EvsGetLog(log, FF_EVS_LOG_DEPTH) == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(log[FF_EVS_LOG_DEPTH - 1u].event_id == 0xFFFFu);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&stats) == FF_SUCCESS);
    FF_TEST_ASSERT(stats.queued == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(stats.dispatched == 0u && stats.dropped == 1u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(FF_EVS_LOG_DEPTH + 1u) == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(g_listener_calls == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(g_listener_max_depth == 1u);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&stats) == FF_SUCCESS);
    FF_TEST_ASSERT(stats.dispatched == FF_EVS_LOG_DEPTH && stats.dropped == 1u);
}

static void test_sb_overflow_event_is_logged_when_evs_queue_is_full(void)
{
    const FF_AppId_t subscriber = 7u;
    FF_Msg_t msg;
    FF_EvsEvent_t log[FF_EVS_LOG_DEPTH];
    FF_EvsNotifyStats_t stats;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_EvsSetListener(test_event_listener);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;
    FF_TEST_ASSERT(FF_SbCreatePipe(subscriber, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A06u, subscriber) == FF_SUCCESS);

    memset(&msg, 0, sizeof(msg));
    msg.msg_id = 0x0A06u;
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    for (uint32_t i = 0u; i < FF_EVS_LOG_DEPTH; i++) {
        FF_TEST_ASSERT(FF_EvsSendDeferred((uint16_t)i, FF_EVS_INFO, 1u, "queued") == FF_SUCCESS);
    }

    /* SB preserves its publish result while EVS exposes lost forwarding. */
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == FF_EVS_LOG_DEPTH + 1u);
    FF_TEST_ASSERT(FF_EvsGetLog(log, FF_EVS_LOG_DEPTH) == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(log[FF_EVS_LOG_DEPTH - 1u].event_id == FF_EVS_SB_ERR);
    FF_TEST_ASSERT(log[FF_EVS_LOG_DEPTH - 1u].app_id == subscriber);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&stats) == FF_SUCCESS);
    FF_TEST_ASSERT(stats.queued == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(stats.dispatched == 0u && stats.dropped == 1u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(FF_EVS_LOG_DEPTH + 1u) == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(g_listener_calls == FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&stats) == FF_SUCCESS);
    FF_TEST_ASSERT(stats.dispatched == FF_EVS_LOG_DEPTH && stats.dropped == 1u);
}

static void test_publish_lock_contention_has_no_side_effects(void)
{
    const FF_AppId_t subscriber = 8u;
    FF_Msg_t msg;
    FF_Msg_t received;
    FF_SbStats_t before;
    FF_SbStats_t after;
    FF_EvsNotifyStats_t evs_before;
    FF_EvsNotifyStats_t evs_after;
    FF_EvsEvent_t log_event;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(subscriber, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A07u, subscriber) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_before) == FF_SUCCESS);
    memset(&msg, 0, sizeof(msg));
    msg.msg_id = 0x0A07u;
    FF_TEST_ASSERT(FF_SbGetStats(&before) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbTestLock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_SbTestUnlock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbGetStats(&after) == FF_SUCCESS);
    FF_TEST_ASSERT(after.published == before.published);
    FF_TEST_ASSERT(after.delivered == before.delivered);
    FF_TEST_ASSERT(after.dropped == before.dropped);
    FF_TEST_ASSERT(FF_SbReceive(subscriber, &received, 0u) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == 0u);
    FF_TEST_ASSERT(FF_EvsGetLog(&log_event, 1u) == 0u);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&evs_after) == FF_SUCCESS);
    FF_TEST_ASSERT(evs_after.queued == evs_before.queued);
    FF_TEST_ASSERT(evs_after.dispatched == evs_before.dispatched);
    FF_TEST_ASSERT(evs_after.dropped == evs_before.dropped);
}

static void test_deferred_notification_lock_contention_is_reported_without_mutation(void)
{
    FF_EvsNotifyStats_t before;
    FF_EvsNotifyStats_t after;
    FF_EvsEvent_t events[2];
    uint32_t log_count;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_EvsSetListener(test_event_listener);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&before) == FF_SUCCESS);
    log_count = FF_EvsGetTotalCount();
    FF_TEST_ASSERT(FF_EvsTestLock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsSendDeferred(0xA55Au, FF_EVS_ERROR, 9u, "contended") == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_EvsTestUnlock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == log_count);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&after) == FF_SUCCESS);
    FF_TEST_ASSERT(after.queued == before.queued);
    FF_TEST_ASSERT(after.dispatched == before.dispatched);
    FF_TEST_ASSERT(after.dropped == before.dropped);
    FF_TEST_ASSERT(FF_EvsGetLog(events, 2u) == 0u);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 0u);
    FF_TEST_ASSERT(FF_EvsSendDeferred(0xA55Au, FF_EVS_ERROR, 9u, "available") == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == log_count + 1u);
    FF_TEST_ASSERT(FF_EvsGetLog(events, 2u) == 1u);
    FF_TEST_ASSERT(events[0].event_id == 0xA55Au);
}

static void test_sb_overflow_does_not_wait_for_evs_lock(void)
{
    const FF_AppId_t subscriber = 10u;
    FF_Msg_t msg;
    FF_Msg_t received;
    FF_SbStats_t sb_stats;
    FF_EvsNotifyStats_t notify_before;
    FF_EvsNotifyStats_t notify_after;
    FF_EvsEvent_t event;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_EvsSetListener(test_event_listener);
    g_listener_calls = 0u;
    FF_TEST_ASSERT(FF_SbCreatePipe(subscriber, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0A08u, subscriber) == FF_SUCCESS);
    memset(&msg, 0, sizeof(msg));
    msg.msg_id = 0x0A08u;
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&notify_before) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsTestLock() == FF_SUCCESS);
    /* The first copy fills the pipe; the second is accounted as a drop while
     * its deferred EVS report encounters the deliberately-held EVS mutex. */
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsTestUnlock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbGetStats(&sb_stats) == FF_SUCCESS);
    FF_TEST_ASSERT(sb_stats.published == 2u);
    FF_TEST_ASSERT(sb_stats.delivered == 1u);
    FF_TEST_ASSERT(sb_stats.dropped == 1u);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == 0u);
    FF_TEST_ASSERT(FF_EvsGetLog(&event, 1u) == 0u);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&notify_after) == FF_SUCCESS);
    FF_TEST_ASSERT(notify_after.queued == notify_before.queued);
    FF_TEST_ASSERT(notify_after.dispatched == notify_before.dispatched);
    FF_TEST_ASSERT(notify_after.dropped == notify_before.dropped);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_SbReceive(subscriber, &received, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbReceive(subscriber, &received, 0u) == FF_TIMEOUT);
}

static void test_busy_subscriber_queue_skips_copy_and_continues_fanout(void)
{
    const FF_AppId_t busy_app = 11u;
    const FF_AppId_t available_app = 12u;
    const FF_MsgId_t msg_id = 0x0A09u;
    FF_Msg_t msg;
    FF_Msg_t received;
    FF_SbStats_t before;
    FF_SbStats_t after;
    FF_EvsNotifyStats_t notify_before;
    FF_EvsNotifyStats_t notify_after;
    FF_EvsEvent_t event;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_EvsSetListener(test_event_listener);
    g_listener_calls = 0u;
    g_listener_depth = 0u;
    g_listener_max_depth = 0u;
    memset(&g_listener_event, 0, sizeof(g_listener_event));
    FF_TEST_ASSERT(FF_SbCreatePipe(busy_app, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(available_app, 2u) == FF_SUCCESS);
    /* Subscription order makes the held pipe the first bounded fan-out item. */
    FF_TEST_ASSERT(FF_SbSubscribe(msg_id, busy_app) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(msg_id, available_app) == FF_SUCCESS);
    memset(&msg, 0, sizeof(msg));
    msg.msg_id = msg_id;
    msg.payload[0] = 0x5Au;
    FF_TEST_ASSERT(FF_SbGetStats(&before) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&notify_before) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbTestQueueLock(busy_app) == FF_SUCCESS);
    /* Publish must finish while the queue mutex remains held: there is no wait
     * for this lock, and later subscribers still receive their bounded copy. */
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_SbGetStats(&after) == FF_SUCCESS);
    FF_TEST_ASSERT(after.published == before.published + 1u);
    FF_TEST_ASSERT(after.delivered == before.delivered + 1u);
    FF_TEST_ASSERT(after.dropped == before.dropped + 1u);
    FF_TEST_ASSERT(FF_SbReceive(available_app, &received, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(received.msg_id == msg_id && received.payload[0] == 0x5Au);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == 1u);
    FF_TEST_ASSERT(FF_EvsGetLog(&event, 1u) == 1u);
    FF_TEST_ASSERT(event.event_id == FF_EVS_SB_ERR);
    FF_TEST_ASSERT(event.type == FF_EVS_ERROR);
    FF_TEST_ASSERT(event.app_id == busy_app);
    FF_TEST_ASSERT(strstr(event.text, "queue busy") != NULL);
    FF_TEST_ASSERT(strstr(event.text, "0x0A09") != NULL);
    FF_TEST_ASSERT(g_listener_calls == 0u);
    FF_TEST_ASSERT(FF_EvsGetNotifyStats(&notify_after) == FF_SUCCESS);
    FF_TEST_ASSERT(notify_after.queued == notify_before.queued + 1u);
    FF_TEST_ASSERT(notify_after.dispatched == notify_before.dispatched);
    FF_TEST_ASSERT(notify_after.dropped == notify_before.dropped);
    FF_TEST_ASSERT(FF_SbTestQueueUnlock(busy_app) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbReceive(busy_app, &received, 0u) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_EvsDispatchPending(1u) == 1u);
    FF_TEST_ASSERT(g_listener_calls == 1u);
    FF_TEST_ASSERT(g_listener_event.event_id == FF_EVS_SB_ERR);
    FF_TEST_ASSERT(g_listener_event.app_id == busy_app);
    FF_TEST_ASSERT(strstr(g_listener_event.text, "queue busy") != NULL);
}

static void test_pal_try_put_distinguishes_busy_from_full(void)
{
    FF_QueueId_t queue;
    uint8_t item = 0xA5u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueCreate(&queue, "tryput", 1u, (uint16_t)sizeof(item)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueTestLock(queue) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueTryPut(queue, &item) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_PalQueueTestUnlock(queue) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueTryPut(queue, &item) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueTryPut(queue, &item) == FF_BUFFER_FULL);
}

FF_TEST_MAIN(test_delivery, test_filtering, test_stats, test_full_pipe_drop_raises_evs_error,
             test_deferred_listener_reentry_is_bounded,
             test_event_notification_queue_saturation_is_reported,
             test_sb_overflow_event_is_logged_when_evs_queue_is_full,
             test_publish_lock_contention_has_no_side_effects,
             test_deferred_notification_lock_contention_is_reported_without_mutation,
             test_sb_overflow_does_not_wait_for_evs_lock,
             test_busy_subscriber_queue_skips_copy_and_continues_fanout,
             test_pal_try_put_distinguishes_busy_from_full)
