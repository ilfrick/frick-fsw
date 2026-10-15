/*
 * frick-fsw - Event Services implementation
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "evs.h"
#include "core/pal/pal.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static FF_EvsEvent_t g_log[FF_EVS_LOG_DEPTH];
static FF_EvsEvent_t g_notify_queue[FF_EVS_LOG_DEPTH];
static uint32_t g_count;
static uint32_t g_notify_head;
static uint32_t g_notify_tail;
static uint32_t g_notify_count;
static FF_EvsNotifyStats_t g_notify_stats;
static FF_EvsListener_t g_listener;
static FF_MutexId_t g_lock;
static bool g_dispatching;

FF_Status_t FF_EvsInit(void)
{
    memset(g_log, 0, sizeof(g_log));
    memset(g_notify_queue, 0, sizeof(g_notify_queue));
    g_count = 0u;
    g_notify_head = 0u;
    g_notify_tail = 0u;
    g_notify_count = 0u;
    memset(&g_notify_stats, 0, sizeof(g_notify_stats));
    g_listener = NULL;
    g_dispatching = false;
    return FF_PalMutexCreate(&g_lock, "evs");
}

void FF_EvsSetListener(FF_EvsListener_t listener)
{
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    g_listener = listener;
    FF_PalMutexUnlock(g_lock);
}

static void ff_evs_format(FF_EvsEvent_t *evt, uint16_t event_id, FF_EvsType_t type, FF_AppId_t app,
                          const char *fmt, va_list ap)
{
    memset(evt, 0, sizeof(*evt));
    evt->event_id = event_id;
    evt->type = (uint8_t)type;
    evt->app_id = app;
    evt->time_ns = FF_PalGetMonotonicNs();
    (void)vsnprintf(evt->text, sizeof(evt->text), fmt, ap);
}

static void ff_evs_log_locked(const FF_EvsEvent_t *evt)
{
    g_log[g_count % FF_EVS_LOG_DEPTH] = *evt;
    g_count++;
}

FF_Status_t FF_EvsSend(uint16_t event_id, FF_EvsType_t type, FF_AppId_t app, const char *fmt, ...)
{
    FF_EvsEvent_t evt;
    FF_EvsListener_t listener;
    va_list ap;

    va_start(ap, fmt);
    ff_evs_format(&evt, event_id, type, app, fmt, ap);
    va_end(ap);

    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    ff_evs_log_locked(&evt);
    listener = g_listener;
    FF_PalMutexUnlock(g_lock);

    /* Preserve the ordinary EVS contract: listener delivery is synchronous. */
    if (listener != NULL) {
        listener(&evt);
    }
    return FF_SUCCESS;
}

FF_Status_t FF_EvsSendDeferred(uint16_t event_id, FF_EvsType_t type, FF_AppId_t app,
                               const char *fmt, ...)
{
    FF_EvsEvent_t evt;
    FF_Status_t lock_status;
    va_list ap;

    va_start(ap, fmt);
    ff_evs_format(&evt, event_id, type, app, fmt, ap);
    va_end(ap);

    lock_status = FF_PalMutexTryLock(g_lock);
    if (lock_status != FF_SUCCESS) {
        return lock_status;
    }
    ff_evs_log_locked(&evt);
    if (g_listener == NULL) {
        FF_PalMutexUnlock(g_lock);
        return FF_SUCCESS;
    }
    if (g_notify_count == FF_EVS_LOG_DEPTH) {
        /* The log retains the event; the stats make notification loss observable. */
        g_notify_stats.dropped++;
        FF_PalMutexUnlock(g_lock);
        return FF_NO_RESOURCE;
    }
    g_notify_queue[g_notify_tail] = evt;
    g_notify_tail = (g_notify_tail + 1u) % FF_EVS_LOG_DEPTH;
    g_notify_count++;
    g_notify_stats.queued++;
    FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

#ifdef FF_TEST_HOOKS
FF_Status_t FF_EvsTestLock(void)
{
    return FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
}

FF_Status_t FF_EvsTestUnlock(void)
{
    return FF_PalMutexUnlock(g_lock);
}
#endif

uint32_t FF_EvsDispatchPending(uint32_t max_events)
{
    uint32_t dispatched = 0u;
    uint32_t batch_limit;

    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    if (g_dispatching) {
        FF_PalMutexUnlock(g_lock);
        return 0u;
    }
    g_dispatching = true;
    batch_limit = (g_notify_count < max_events) ? g_notify_count : max_events;
    FF_PalMutexUnlock(g_lock);

    while (dispatched < batch_limit) {
        FF_EvsEvent_t evt;
        FF_EvsListener_t listener;

        FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
        listener = g_listener;
        if ((listener == NULL) || (g_notify_count == 0u)) {
            FF_PalMutexUnlock(g_lock);
            break;
        }
        evt = g_notify_queue[g_notify_head];
        g_notify_head = (g_notify_head + 1u) % FF_EVS_LOG_DEPTH;
        g_notify_count--;
        FF_PalMutexUnlock(g_lock);

        listener(&evt);

        FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
        g_notify_stats.dispatched++;
        FF_PalMutexUnlock(g_lock);
        dispatched++;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    g_dispatching = false;
    FF_PalMutexUnlock(g_lock);
    return dispatched;
}

FF_Status_t FF_EvsGetNotifyStats(FF_EvsNotifyStats_t *stats)
{
    if (stats == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    *stats = g_notify_stats;
    FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

uint32_t FF_EvsGetLog(FF_EvsEvent_t *out, uint32_t max_events)
{
    uint32_t n = 0u;
    if (out == NULL) {
        return 0u;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    uint32_t total = g_count;
    uint32_t start = (total > FF_EVS_LOG_DEPTH) ? (total - FF_EVS_LOG_DEPTH) : 0u;
    for (uint32_t i = start; (i < total) && (n < max_events); i++) {
        out[n++] = g_log[i % FF_EVS_LOG_DEPTH];
    }
    FF_PalMutexUnlock(g_lock);
    return n;
}

uint32_t FF_EvsGetTotalCount(void)
{
    uint32_t c;
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    c = g_count;
    FF_PalMutexUnlock(g_lock);
    return c;
}
