/*
 * frick-fsw - Software Bus implementation
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "sb.h"
#include "core/pal/pal.h"
#include <string.h>
#include <stdio.h>

typedef struct
{
    bool used;
    FF_AppId_t app;
    FF_QueueId_t queue;
} ff_sb_pipe_t;

typedef struct
{
    bool used;
    FF_MsgId_t msg_id;
    FF_AppId_t app;
} ff_sb_sub_t;

static ff_sb_pipe_t g_pipes[FF_SB_MAX_PIPES];
static ff_sb_sub_t g_subs[FF_SB_MAX_SUBSCRIPTIONS];
static FF_SbStats_t g_stats;
static FF_MutexId_t g_bus_lock;
static bool g_ready;

FF_Status_t FF_SbInit(void)
{
    memset(g_pipes, 0, sizeof(g_pipes));
    memset(g_subs, 0, sizeof(g_subs));
    memset(&g_stats, 0, sizeof(g_stats));
    if (FF_PalMutexCreate(&g_bus_lock, "sb") != FF_SUCCESS) {
        return FF_ERROR;
    }
    g_ready = true;
    return FF_SUCCESS;
}

static ff_sb_pipe_t *ff_sb_find_pipe(FF_AppId_t app)
{
    for (uint16_t i = 0; i < FF_SB_MAX_PIPES; i++) {
        if (g_pipes[i].used && (g_pipes[i].app == app)) {
            return &g_pipes[i];
        }
    }
    return NULL;
}

FF_Status_t FF_SbCreatePipe(FF_AppId_t app, uint16_t depth)
{
    if (!g_ready) {
        return FF_NOT_READY;
    }
    if ((depth == 0u) || (depth > FF_SB_QUEUE_DEPTH_MAX)) {
        depth = FF_SB_DEFAULT_DEPTH;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);

    if (ff_sb_find_pipe(app) != NULL) {
        FF_PalMutexUnlock(g_bus_lock);
        return FF_ALREADY_EXISTS;
    }
    ff_sb_pipe_t *slot = NULL;
    for (uint16_t i = 0; i < FF_SB_MAX_PIPES; i++) {
        if (!g_pipes[i].used) {
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
    slot->used = true;
    slot->app = app;
    FF_PalMutexUnlock(g_bus_lock);
    return FF_SUCCESS;
}

FF_Status_t FF_SbSubscribe(FF_MsgId_t msg_id, FF_AppId_t subscriber)
{
    if (!g_ready) {
        return FF_NOT_READY;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    if (ff_sb_find_pipe(subscriber) == NULL) {
        FF_PalMutexUnlock(g_bus_lock);
        return FF_NOT_FOUND; /* subscribe to a non existing pipe */
    }
    for (uint16_t i = 0; i < FF_SB_MAX_SUBSCRIPTIONS; i++) {
        if (!g_subs[i].used) {
            g_subs[i].used = true;
            g_subs[i].msg_id = msg_id;
            g_subs[i].app = subscriber;
            FF_PalMutexUnlock(g_bus_lock);
            return FF_SUCCESS;
        }
    }
    FF_PalMutexUnlock(g_bus_lock);
    return FF_NO_RESOURCE;
}

FF_Status_t FF_SbUnsubscribe(FF_MsgId_t msg_id, FF_AppId_t subscriber)
{
    if (!g_ready) {
        return FF_NOT_READY;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    for (uint16_t i = 0; i < FF_SB_MAX_SUBSCRIPTIONS; i++) {
        if (g_subs[i].used && (g_subs[i].msg_id == msg_id) && (g_subs[i].app == subscriber)) {
            g_subs[i].used = false;
            FF_PalMutexUnlock(g_bus_lock);
            return FF_SUCCESS;
        }
    }
    FF_PalMutexUnlock(g_bus_lock);
    return FF_NOT_FOUND;
}

FF_Status_t FF_SbPublish(const FF_Msg_t *msg)
{
    if (!g_ready || (msg == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    g_stats.published++;
    for (uint16_t i = 0; i < FF_SB_MAX_SUBSCRIPTIONS; i++) {
        if (!g_subs[i].used) {
            continue;
        }
        if ((g_subs[i].msg_id == msg->msg_id) || (g_subs[i].msg_id == FF_SB_WILDCARD)) {
            ff_sb_pipe_t *pipe = ff_sb_find_pipe(g_subs[i].app);
            if (pipe == NULL) {
                continue;
            }
            if (FF_PalQueuePut(pipe->queue, msg, 0u) == FF_SUCCESS) {
                g_stats.delivered++;
            }
            else {
                g_stats.dropped++;
            }
        }
    }
    FF_PalMutexUnlock(g_bus_lock);
    return FF_SUCCESS;
}

FF_Status_t FF_SbReceive(FF_AppId_t app, FF_Msg_t *msg, uint32_t timeout_ms)
{
    if (!g_ready || (msg == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    ff_sb_pipe_t *pipe = ff_sb_find_pipe(app);
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
    FF_PalMutexLock(g_bus_lock, FF_PAL_WAIT_FOREVER);
    *stats = g_stats;
    FF_PalMutexUnlock(g_bus_lock);
    return FF_SUCCESS;
}
