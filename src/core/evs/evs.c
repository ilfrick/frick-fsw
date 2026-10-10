/*
 * frick-fsw - Event Services implementation
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "evs.h"
#include "core/pal/pal.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static FF_EvsEvent_t g_log[FF_EVS_LOG_DEPTH];
static uint32_t g_count;
static FF_EvsListener_t g_listener;
static FF_MutexId_t g_lock;

FF_Status_t FF_EvsInit(void)
{
    memset(g_log, 0, sizeof(g_log));
    g_count = 0u;
    g_listener = NULL;
    return FF_PalMutexCreate(&g_lock, "evs");
}

void FF_EvsSetListener(FF_EvsListener_t listener)
{
    g_listener = listener;
}

FF_Status_t FF_EvsSend(uint16_t event_id, FF_EvsType_t type, FF_AppId_t app, const char *fmt, ...)
{
    FF_EvsEvent_t evt;
    va_list ap;

    memset(&evt, 0, sizeof(evt));
    evt.event_id = event_id;
    evt.type = (uint8_t)type;
    evt.app_id = app;
    evt.time_ns = FF_PalGetMonotonicNs();

    va_start(ap, fmt);
    (void)vsnprintf(evt.text, sizeof(evt.text), fmt, ap);
    va_end(ap);

    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    g_log[g_count % FF_EVS_LOG_DEPTH] = evt;
    g_count++;
    FF_PalMutexUnlock(g_lock);

    if (g_listener != NULL) {
        g_listener(&evt);
    }
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
