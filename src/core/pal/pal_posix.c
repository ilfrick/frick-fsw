/*
 * frick-fsw - Platform Abstraction Layer: POSIX implementation
 *
 * Used on the Linux host simulator and on RTEMS 6 (which provides a POSIX
 * threads / message queue surface). No dynamic memory allocation is performed
 * after FF_PalInit().
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#define _GNU_SOURCE
#include "pal.h"

#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define FF_PAL_MAX_TASKS 32u
#define FF_PAL_MAX_MUTEXES 32u
#define FF_PAL_MAX_QUEUES 32u
#define FF_PAL_MAX_QUEUE_DEPTH 32u
#define FF_PAL_MAX_ITEM_SIZE 1024u

typedef struct
{
    bool used;
    pthread_t thread;
    FF_PalTaskFunc_t fn;
    void *arg;
    char name[FF_PAL_NAME_LEN];
} ff_task_slot_t;

typedef struct
{
    bool used;
    pthread_mutex_t mtx;
    char name[FF_PAL_NAME_LEN];
} ff_mutex_slot_t;

typedef struct
{
    bool used;
    char name[FF_PAL_NAME_LEN];
    pthread_mutex_t mtx;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    uint16_t depth;
    uint16_t item_size;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint64_t dropped;
    uint8_t storage[FF_PAL_MAX_QUEUE_DEPTH][FF_PAL_MAX_ITEM_SIZE];
} ff_queue_slot_t;

static ff_task_slot_t g_tasks[FF_PAL_MAX_TASKS];
static ff_mutex_slot_t g_mutexes[FF_PAL_MAX_MUTEXES];
static ff_queue_slot_t g_queues[FF_PAL_MAX_QUEUES];

static pthread_mutex_t g_alloc_lock = PTHREAD_MUTEX_INITIALIZER;
static int64_t g_utc_offset_ns = 0;
static bool g_initialized = false;

/* ------------------------------------------------------------------ */

static uint64_t ff_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void ff_abs_deadline(struct timespec *ts, uint32_t timeout_ms)
{
    clock_gettime(CLOCK_MONOTONIC, ts);
    ts->tv_sec += (time_t)(timeout_ms / 1000u);
    ts->tv_nsec += (long)((timeout_ms % 1000u) * 1000000u);
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_sec += 1;
        ts->tv_nsec -= 1000000000L;
    }
}

/* ------------------------------------------------------------------ */

FF_Status_t FF_PalInit(void)
{
    if (g_initialized) {
        return FF_SUCCESS;
    }
    memset(g_tasks, 0, sizeof(g_tasks));
    memset(g_mutexes, 0, sizeof(g_mutexes));
    memset(g_queues, 0, sizeof(g_queues));
    g_utc_offset_ns = 0;
    FF_PalPlatformInit();
    g_initialized = true;
    return FF_SUCCESS;
}

#ifndef FRICK_PLATFORM_MPFS
FF_Status_t FF_PalPlatformInit(void)
{
    return FF_SUCCESS;
}
#endif

void FF_PalShutdown(void)
{
    g_initialized = false;
}

/* ---- Tasks ------------------------------------------------------- */

static void *ff_task_trampoline(void *arg)
{
    ff_task_slot_t *slot = (ff_task_slot_t *)arg;
    slot->fn(slot->arg);
    return NULL;
}

FF_Status_t FF_PalTaskCreate(FF_TaskId_t *id, const char *name, FF_PalTaskFunc_t fn, void *arg,
                             uint32_t stack_bytes, uint8_t priority)
{
    FF_TaskId_t idx;
    (void)stack_bytes;

    if ((id == NULL) || (fn == NULL)) {
        return FF_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&g_alloc_lock);
    for (idx = 0; (idx < FF_PAL_MAX_TASKS) && g_tasks[idx].used; idx++) {
        /* find a free slot */
    }
    if (idx >= FF_PAL_MAX_TASKS) {
        pthread_mutex_unlock(&g_alloc_lock);
        return FF_NO_RESOURCE;
    }
    g_tasks[idx].used = true;
    g_tasks[idx].fn = fn;
    g_tasks[idx].arg = arg;
    strncpy(g_tasks[idx].name, name != NULL ? name : "task", FF_PAL_NAME_LEN - 1);
    pthread_mutex_unlock(&g_alloc_lock);

    {
        pthread_attr_t attr;
        struct sched_param sp;
        pthread_attr_init(&attr);
        if (stack_bytes > 0) {
            (void)pthread_attr_setstacksize(&attr, stack_bytes);
        }
        memset(&sp, 0, sizeof(sp));
        sp.sched_priority = (int)priority;
        (void)pthread_attr_setschedparam(&attr, &sp);
        if (pthread_create(&g_tasks[idx].thread, &attr, ff_task_trampoline, &g_tasks[idx]) != 0) {
            pthread_attr_destroy(&attr);
            g_tasks[idx].used = false;
            return FF_ERROR;
        }
        pthread_attr_destroy(&attr);
    }

#if defined(__linux__)
    (void)pthread_setname_np(g_tasks[idx].thread, name != NULL ? name : "task");
#endif
    *id = idx;
    return FF_SUCCESS;
}

FF_Status_t FF_PalTaskDelete(FF_TaskId_t id)
{
    if (id >= FF_PAL_MAX_TASKS || !g_tasks[id].used) {
        return FF_NOT_FOUND;
    }
    pthread_cancel(g_tasks[id].thread);
    pthread_join(g_tasks[id].thread, NULL);
    g_tasks[id].used = false;
    return FF_SUCCESS;
}

void FF_PalTaskDelayMs(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000u);
    ts.tv_nsec = (long)((ms % 1000u) * 1000000u);
    nanosleep(&ts, NULL);
}

void FF_PalTaskDelayTicks(uint32_t ticks)
{
    FF_PalTaskDelayMs(ticks);
}
void FF_PalTaskYield(void)
{
    sched_yield();
}
void FF_PalTaskExit(void)
{
    pthread_exit(NULL);
}

FF_TaskId_t FF_PalTaskSelf(void)
{
    pthread_t me = pthread_self();
    for (FF_TaskId_t i = 0; i < FF_PAL_MAX_TASKS; i++) {
        if (g_tasks[i].used && pthread_equal(g_tasks[i].thread, me)) {
            return i;
        }
    }
    return FF_INVALID_ID;
}

/* ---- Mutexes ----------------------------------------------------- */

FF_Status_t FF_PalMutexCreate(FF_MutexId_t *id, const char *name)
{
    FF_MutexId_t idx;
    if (id == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    pthread_mutex_lock(&g_alloc_lock);
    for (idx = 0; (idx < FF_PAL_MAX_MUTEXES) && g_mutexes[idx].used; idx++) {
    }
    if (idx >= FF_PAL_MAX_MUTEXES) {
        pthread_mutex_unlock(&g_alloc_lock);
        return FF_NO_RESOURCE;
    }
    g_mutexes[idx].used = true;
    strncpy(g_mutexes[idx].name, name != NULL ? name : "mtx", FF_PAL_NAME_LEN - 1);
    pthread_mutex_init(&g_mutexes[idx].mtx, NULL);
    pthread_mutex_unlock(&g_alloc_lock);
    *id = idx;
    return FF_SUCCESS;
}

FF_Status_t FF_PalMutexLock(FF_MutexId_t id, uint32_t timeout_ms)
{
    int rc;
    if (id >= FF_PAL_MAX_MUTEXES || !g_mutexes[id].used) {
        return FF_NOT_FOUND;
    }
    if (timeout_ms == FF_PAL_WAIT_FOREVER) {
        rc = pthread_mutex_lock(&g_mutexes[id].mtx);
    }
    else {
        struct timespec ts;
        ff_abs_deadline(&ts, timeout_ms);
        rc = pthread_mutex_timedlock(&g_mutexes[id].mtx, &ts);
    }
    return (rc == 0) ? FF_SUCCESS : FF_TIMEOUT;
}

FF_Status_t FF_PalMutexUnlock(FF_MutexId_t id)
{
    if (id >= FF_PAL_MAX_MUTEXES || !g_mutexes[id].used) {
        return FF_NOT_FOUND;
    }
    return (pthread_mutex_unlock(&g_mutexes[id].mtx) == 0) ? FF_SUCCESS : FF_ERROR;
}

FF_Status_t FF_PalMutexDelete(FF_MutexId_t id)
{
    if (id >= FF_PAL_MAX_MUTEXES || !g_mutexes[id].used) {
        return FF_NOT_FOUND;
    }
    pthread_mutex_destroy(&g_mutexes[id].mtx);
    g_mutexes[id].used = false;
    return FF_SUCCESS;
}

/* ---- Queues ------------------------------------------------------ */

FF_Status_t FF_PalQueueCreate(FF_QueueId_t *id, const char *name, uint16_t depth,
                              uint16_t item_size)
{
    FF_QueueId_t idx;
    if ((id == NULL) || (depth == 0u) || (item_size == 0u) || (depth > FF_PAL_MAX_QUEUE_DEPTH) ||
        (item_size > FF_PAL_MAX_ITEM_SIZE)) {
        return FF_INVALID_ARGUMENT;
    }
    pthread_mutex_lock(&g_alloc_lock);
    for (idx = 0; (idx < FF_PAL_MAX_QUEUES) && g_queues[idx].used; idx++) {
    }
    if (idx >= FF_PAL_MAX_QUEUES) {
        pthread_mutex_unlock(&g_alloc_lock);
        return FF_NO_RESOURCE;
    }
    {
        ff_queue_slot_t *q = &g_queues[idx];
        memset(q, 0, sizeof(*q));
        q->used = true;
        q->depth = depth;
        q->item_size = item_size;
        strncpy(q->name, name != NULL ? name : "q", FF_PAL_NAME_LEN - 1);
        pthread_mutex_init(&q->mtx, NULL);
        pthread_cond_init(&q->not_empty, NULL);
        pthread_cond_init(&q->not_full, NULL);
    }
    pthread_mutex_unlock(&g_alloc_lock);
    *id = idx;
    return FF_SUCCESS;
}

FF_Status_t FF_PalQueuePut(FF_QueueId_t id, const void *item, uint32_t timeout_ms)
{
    ff_queue_slot_t *q;
    int rc = 0;
    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used || item == NULL) {
        return FF_NOT_FOUND;
    }
    q = &g_queues[id];
    pthread_mutex_lock(&q->mtx);
    while (q->count >= q->depth) {
        if (timeout_ms == 0u) {
            q->dropped++;
            pthread_mutex_unlock(&q->mtx);
            return FF_BUFFER_FULL;
        }
        if (timeout_ms == FF_PAL_WAIT_FOREVER) {
            rc = pthread_cond_wait(&q->not_full, &q->mtx);
        }
        else {
            struct timespec ts;
            ff_abs_deadline(&ts, timeout_ms);
            rc = pthread_cond_timedwait(&q->not_full, &q->mtx, &ts);
        }
        if (rc == ETIMEDOUT) {
            q->dropped++;
            pthread_mutex_unlock(&q->mtx);
            return FF_TIMEOUT;
        }
    }
    memcpy(q->storage[q->head], item, q->item_size);
    q->head = (uint16_t)((q->head + 1u) % q->depth);
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mtx);
    return FF_SUCCESS;
}

FF_Status_t FF_PalQueueGet(FF_QueueId_t id, void *item, uint32_t timeout_ms)
{
    ff_queue_slot_t *q;
    int rc = 0;
    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used || item == NULL) {
        return FF_NOT_FOUND;
    }
    q = &g_queues[id];
    pthread_mutex_lock(&q->mtx);
    while (q->count == 0u) {
        if (timeout_ms == 0u) {
            pthread_mutex_unlock(&q->mtx);
            return FF_TIMEOUT;
        }
        if (timeout_ms == FF_PAL_WAIT_FOREVER) {
            rc = pthread_cond_wait(&q->not_empty, &q->mtx);
        }
        else {
            struct timespec ts;
            ff_abs_deadline(&ts, timeout_ms);
            rc = pthread_cond_timedwait(&q->not_empty, &q->mtx, &ts);
        }
        if (rc == ETIMEDOUT) {
            pthread_mutex_unlock(&q->mtx);
            return FF_TIMEOUT;
        }
    }
    memcpy(item, q->storage[q->tail], q->item_size);
    q->tail = (uint16_t)((q->tail + 1u) % q->depth);
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mtx);
    return FF_SUCCESS;
}

FF_Status_t FF_PalQueueDelete(FF_QueueId_t id)
{
    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used) {
        return FF_NOT_FOUND;
    }
    pthread_mutex_lock(&g_queues[id].mtx);
    g_queues[id].used = false;
    pthread_mutex_unlock(&g_queues[id].mtx);
    pthread_mutex_destroy(&g_queues[id].mtx);
    pthread_cond_destroy(&g_queues[id].not_empty);
    pthread_cond_destroy(&g_queues[id].not_full);
    return FF_SUCCESS;
}

uint16_t FF_PalQueueCount(FF_QueueId_t id)
{
    uint16_t n;
    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used) {
        return 0u;
    }
    pthread_mutex_lock(&g_queues[id].mtx);
    n = g_queues[id].count;
    pthread_mutex_unlock(&g_queues[id].mtx);
    return n;
}

/* ---- Time -------------------------------------------------------- */

uint64_t FF_PalGetMonotonicNs(void)
{
    return ff_now_ns();
}
uint64_t FF_PalGetUtcNs(void)
{
    return (uint64_t)((int64_t)ff_now_ns() + g_utc_offset_ns);
}
FF_Status_t FF_PalSetUtcNs(uint64_t utc_ns)
{
    g_utc_offset_ns = (int64_t)utc_ns - (int64_t)ff_now_ns();
    return FF_SUCCESS;
}
uint32_t FF_PalGetCpuLoadPpt(void)
{
    return 0u;
}

/* ---- Console ----------------------------------------------------- */

void FF_PalLog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (void)vfprintf(stdout, fmt, ap);
    (void)fflush(stdout);
    va_end(ap);
}

void FF_PalPanic(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (void)fprintf(stderr, "\n[FF-PANIC] ");
    (void)vfprintf(stderr, fmt, ap);
    (void)fprintf(stderr, "\n");
    (void)fflush(stderr);
    va_end(ap);
    abort();
}
