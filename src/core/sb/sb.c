/*
 * frick-fsw - Software Bus implementation
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "sb.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include <string.h>
#include <stdatomic.h>
#include <stdio.h>

/* Portable CPU relax hint for spin loops */
#if defined(__x86_64__) || defined(__i386__)
#define FF_CPU_RELAX() __builtin_ia32_pause()
#elif defined(__aarch64__) || defined(__arm__)
#define FF_CPU_RELAX() __builtin_arm_wfe()
#elif defined(__riscv)
#define FF_CPU_RELAX() __builtin_riscv_pause()
#else
#define FF_CPU_RELAX() (void)0
#endif

typedef struct
{
    _Atomic(bool) used;
    _Atomic(FF_AppId_t) app;
    FF_QueueId_t queue;
    FF_MutexId_t pipe_lock;
} ff_sb_pipe_t;

typedef struct
{
    _Atomic(bool) used;
    _Atomic(FF_MsgId_t) msg_id;
    _Atomic(FF_AppId_t) app;
} ff_sb_sub_t;

static ff_sb_pipe_t g_pipes[FF_SB_MAX_PIPES];
static ff_sb_sub_t g_subs[FF_SB_MAX_SUBSCRIPTIONS];
static FF_SbStats_t g_stats;
static FF_MutexId_t g_bus_lock; /* Protects pipe/subscription array structure */
static _Atomic(bool) g_ready;
static _Atomic(bool) g_bus_maintenance; /* Test hook: when true, publish returns FF_TIMEOUT */

FF_Status_t FF_SbInit(void)
{
    memset(g_pipes, 0, sizeof(g_pipes));
    memset(g_subs, 0, sizeof(g_subs));
    memset(&g_stats, 0, sizeof(g_stats));
    if (FF_PalMutexCreate(&g_bus_lock, "sb") != FF_SUCCESS) {
        return FF_ERROR;
    }
    atomic_store_explicit(&g_ready, true, memory_order_release);
    atomic_store_explicit(&g_bus_maintenance, false, memory_order_release);
    return FF_SUCCESS;
}

static ff_sb_pipe_t *ff_sb_find_pipe_locked(FF_AppId_t app)
{
    for (uint16_t i = 0; i < FF_SB_MAX_PIPES; i++) {
        if (atomic_load_explicit(&g_pipes[i].used, memory_order_acquire) &&
            (atomic_load_explicit(&g_pipes[i].app, memory_order_relaxed) == app)) {
            return &g_pipes[i];
        }
    }
    return NULL;
}

static ff_sb_pipe_t *ff_sb_find_pipe(FF_AppId_t app)
{
    for (uint16_t i = 0; i < FF_SB_MAX_PIPES; i++) {
        if (atomic_load_explicit(&g_pipes[i].used, memory_order_acquire) &&
            (atomic_load_explicit(&g_pipes[i].app, memory_order_relaxed) == app)) {
            return &g_pipes[i];
        }
    }
    return NULL;
}

FF_Status_t FF_SbCreatePipe(FF_AppId_t app, uint16_t depth)
{
    if (!atomic_load_explicit(&g_ready, memory_order_acquire)) {
        return FF_NOT_READY;
    }
    if (depth == 0u) {
        depth = FF_SB_DEFAULT_DEPTH;
    }
    if (depth > FF_SB_QUEUE_DEPTH_MAX) {
        depth = FF_SB_QUEUE_DEPTH_MAX;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);

    if (ff_sb_find_pipe_locked(app) != NULL) {
        FF_PalMutexUnlock(g_bus_lock);
        return FF_ALREADY_EXISTS;
    }
    ff_sb_pipe_t *slot = NULL;
    for (uint16_t i = 0; i < FF_SB_MAX_PIPES; i++) {
        if (!atomic_load_explicit(&g_pipes[i].used, memory_order_relaxed)) {
            slot = &g_pipes[i];
            break;
        }
    }
    if (slot == NULL) {
        FF_PalMutexUnlock(g_bus_lock);
        return FF_NO_RESOURCE;
    }
    char name[FF_PAL_NAME_LEN];
    (void)snprintf(name, sizeof(name), "sb%04x", (unsigned)app);
    if (FF_PalQueueCreate(&slot->queue, name, depth, (uint16_t)sizeof(FF_Msg_t)) != FF_SUCCESS) {
        FF_PalMutexUnlock(g_bus_lock);
        return FF_ERROR;
    }
    if (FF_PalMutexCreate(&slot->pipe_lock, name) != FF_SUCCESS) {
        FF_PalQueueDelete(slot->queue);
        FF_PalMutexUnlock(g_bus_lock);
        return FF_ERROR;
    }
    atomic_store_explicit(&slot->app, app, memory_order_release);
    atomic_store_explicit(&slot->used, true, memory_order_release);
    FF_PalMutexUnlock(g_bus_lock);
    return FF_SUCCESS;
}

FF_Status_t FF_SbSubscribe(FF_MsgId_t msg_id, FF_AppId_t subscriber)
{
    if (!atomic_load_explicit(&g_ready, memory_order_acquire)) {
        return FF_NOT_READY;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    if (ff_sb_find_pipe_locked(subscriber) == NULL) {
        FF_PalMutexUnlock(g_bus_lock);
        return FF_NOT_FOUND; /* subscribe to a non existing pipe */
    }
    for (uint16_t i = 0; i < FF_SB_MAX_SUBSCRIPTIONS; i++) {
        bool expected = false;
        if (atomic_compare_exchange_strong_explicit(&g_subs[i].used, &expected, true,
                                                    memory_order_acq_rel, memory_order_relaxed)) {
            atomic_store_explicit(&g_subs[i].msg_id, msg_id, memory_order_release);
            atomic_store_explicit(&g_subs[i].app, subscriber, memory_order_release);
            FF_PalMutexUnlock(g_bus_lock);
            return FF_SUCCESS;
        }
    }
    FF_PalMutexUnlock(g_bus_lock);
    return FF_NO_RESOURCE;
}

FF_Status_t FF_SbUnsubscribe(FF_MsgId_t msg_id, FF_AppId_t subscriber)
{
    if (!atomic_load_explicit(&g_ready, memory_order_acquire)) {
        return FF_NOT_READY;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    for (uint16_t i = 0; i < FF_SB_MAX_SUBSCRIPTIONS; i++) {
        bool used = atomic_load_explicit(&g_subs[i].used, memory_order_acquire);
        if (used && (atomic_load_explicit(&g_subs[i].msg_id, memory_order_acquire) == msg_id) &&
            (atomic_load_explicit(&g_subs[i].app, memory_order_acquire) == subscriber)) {
            atomic_store_explicit(&g_subs[i].used, false, memory_order_release);
            FF_PalMutexUnlock(g_bus_lock);
            return FF_SUCCESS;
        }
    }
    FF_PalMutexUnlock(g_bus_lock);
    return FF_NOT_FOUND;
}

FF_Status_t FF_SbPublish(const FF_Msg_t *msg)
{
    FF_AppId_t dropped_apps[FF_SB_MAX_SUBSCRIPTIONS];
    bool dropped_busy[FF_SB_MAX_SUBSCRIPTIONS];
    uint16_t dropped_count = 0u;
    FF_MsgId_t dropped_msg_id;
    bool queue_busy = false;

    if (!atomic_load_explicit(&g_ready, memory_order_acquire) || (msg == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    dropped_msg_id = msg->msg_id;

    /* Test hook: bus maintenance mode (set by FF_SbTestLock) makes publish return
     * FF_TIMEOUT without mutation. This avoids global lock contention in SMP. */
    if (atomic_load_explicit(&g_bus_maintenance, memory_order_acquire)) {
        return FF_TIMEOUT;
    }

    /* Lock-free publish: iterate subscriptions without holding g_bus_lock.
     * Each subscription is read atomically. For matching subscriptions,
     * acquire the per-pipe lock to enqueue. */
    atomic_fetch_add_explicit(&g_stats.published, 1, memory_order_relaxed);

    for (uint16_t i = 0; i < FF_SB_MAX_SUBSCRIPTIONS; i++) {
        bool used = atomic_load_explicit(&g_subs[i].used, memory_order_acquire);
        if (!used) {
            continue;
        }
        FF_MsgId_t sub_msg_id = atomic_load_explicit(&g_subs[i].msg_id, memory_order_acquire);
        if ((sub_msg_id == msg->msg_id) || (sub_msg_id == FF_SB_WILDCARD)) {
            FF_AppId_t sub_app = atomic_load_explicit(&g_subs[i].app, memory_order_acquire);
            /* Retry pipe lookup more times to handle visibility race (last subscriber) */
            const ff_sb_pipe_t *pipe = NULL;
            for (int lookup_retry = 0; lookup_retry < 200; lookup_retry++) {
                pipe = ff_sb_find_pipe(sub_app);
                if (pipe != NULL) {
                    break;
                }
                FF_CPU_RELAX();
            }
            if (pipe == NULL) {
                continue;
            }
            /* Acquire per-pipe lock for this specific subscription with extended retry */
            FF_Status_t lock_status = FF_PalMutexTryLock(pipe->pipe_lock);
            if (lock_status != FF_SUCCESS) {
                /* Extended retry for SMP contention - 5000 attempts with pause */
                for (int retry = 0; retry < 5000; retry++) {
                    FF_CPU_RELAX();
                    lock_status = FF_PalMutexTryLock(pipe->pipe_lock);
                    if (lock_status == FF_SUCCESS) {
                        break;
                    }
                }
                if (lock_status != FF_SUCCESS) {
                    /* Pipe lock busy - this is a contention case */
                    dropped_apps[dropped_count] = sub_app;
                    dropped_busy[dropped_count] = true;
                    dropped_count++;
                    queue_busy = true;
                    atomic_fetch_add_explicit(&g_stats.dropped, 1, memory_order_relaxed);
                    continue;
                }
            }
            /* Retry queue put more times for transient full conditions */
            FF_Status_t queue_status = FF_PalQueueTryPut(pipe->queue, msg);
            for (int queue_retry = 0; queue_retry < 200 && queue_status != FF_SUCCESS;
                 queue_retry++) {
                FF_CPU_RELAX();
                queue_status = FF_PalQueueTryPut(pipe->queue, msg);
            }
            if (queue_status == FF_SUCCESS) {
                atomic_fetch_add_explicit(&g_stats.delivered, 1, memory_order_relaxed);
            }
            else {
                atomic_fetch_add_explicit(&g_stats.dropped, 1, memory_order_relaxed);
                dropped_apps[dropped_count] = sub_app;
                dropped_busy[dropped_count] = (queue_status == FF_TIMEOUT);
                dropped_count++;
                if (queue_status == FF_TIMEOUT) {
                    queue_busy = true;
                }
            }
            FF_PalMutexUnlock(pipe->pipe_lock);
        }
    }

    /* Avoid inline listener work in the publisher: TM-output dispatches the
     * queued SB error from the deferred-notification path. */
    for (uint16_t i = 0u; i < dropped_count; i++) {
        if (dropped_busy[i]) {
            (void)FF_EvsSendDeferred(FF_EVS_SB_ERR, FF_EVS_ERROR, dropped_apps[i],
                                     "SB pipe queue busy; message 0x%04X copy dropped",
                                     (unsigned)dropped_msg_id);
        }
        else {
            (void)FF_EvsSendDeferred(FF_EVS_SB_ERR, FF_EVS_ERROR, dropped_apps[i],
                                     "SB pipe full; message 0x%04X dropped",
                                     (unsigned)dropped_msg_id);
        }
    }
    return queue_busy ? FF_TIMEOUT : FF_SUCCESS;
}

#ifdef FF_TEST_HOOKS
FF_Status_t FF_SbTestLock(void)
{
    /* Set maintenance flag FIRST (release) to block ALL publishes, then
     * acquire global lock for structural operations. This closes the race
     * where a publisher checks the flag, passes, then the test sets it. */
    atomic_store_explicit(&g_bus_maintenance, true, memory_order_release);
    return FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
}

FF_Status_t FF_SbTestUnlock(void)
{
    /* Release global lock FIRST, then clear maintenance flag (release).
     * This ensures publishers waiting on the flag see it cleared after
     * structural operations complete. */
    FF_Status_t status = FF_PalMutexUnlock(g_bus_lock);
    atomic_store_explicit(&g_bus_maintenance, false, memory_order_release);
    return status;
}

FF_Status_t FF_SbTestQueueLock(FF_AppId_t app)
{
    const ff_sb_pipe_t *pipe = ff_sb_find_pipe(app);
    return (pipe != NULL) ? FF_PalQueueTestLock(pipe->queue) : FF_NOT_FOUND;
}

FF_Status_t FF_SbTestQueueUnlock(FF_AppId_t app)
{
    const ff_sb_pipe_t *pipe = ff_sb_find_pipe(app);
    return (pipe != NULL) ? FF_PalQueueTestUnlock(pipe->queue) : FF_NOT_FOUND;
}

FF_Status_t FF_SbTestPipeLock(FF_AppId_t app)
{
    const ff_sb_pipe_t *pipe = ff_sb_find_pipe(app);
    return (pipe != NULL) ? FF_PalMutexTryLock(pipe->pipe_lock) : FF_NOT_FOUND;
}

FF_Status_t FF_SbTestPipeUnlock(FF_AppId_t app)
{
    const ff_sb_pipe_t *pipe = ff_sb_find_pipe(app);
    return (pipe != NULL) ? FF_PalMutexUnlock(pipe->pipe_lock) : FF_NOT_FOUND;
}
#endif

FF_Status_t FF_SbReceive(FF_AppId_t app, FF_Msg_t *msg, uint32_t timeout_ms)
{
    if (!atomic_load_explicit(&g_ready, memory_order_acquire) || (msg == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    ff_sb_pipe_t *pipe = ff_sb_find_pipe_locked(app);
    FF_QueueId_t q = (pipe != NULL) ? pipe->queue : FF_INVALID_ID;
    FF_PalMutexUnlock(g_bus_lock);

    if (q == FF_INVALID_ID) {
        return FF_NOT_FOUND;
    }
    return FF_PalQueueGet(q, msg, timeout_ms);
}

FF_Status_t FF_SbGetStats(FF_SbStats_t *stats)
{
    if (stats == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Atomic read of stats - no lock needed */
    stats->published = atomic_load_explicit(&g_stats.published, memory_order_acquire);
    stats->delivered = atomic_load_explicit(&g_stats.delivered, memory_order_acquire);
    stats->dropped = atomic_load_explicit(&g_stats.dropped, memory_order_acquire);
    return FF_SUCCESS;
}