/*
 * frick-fsw - Platform Abstraction Layer: POSIX implementation
 *
 * Used on the Linux host simulator and on RTEMS 6 (which provides a POSIX
 * threads / message queue surface). No dynamic memory allocation is performed
 * after FF_PalInit().
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
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
#include <fcntl.h>
#include <unistd.h>

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

#if !defined(FRICK_PLATFORM_MPFS) && !defined(FRICK_PLATFORM_QEMU)
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

FF_Status_t FF_PalTaskDelayUntilNs(uint64_t deadline_ns)
{
    for (;;) {
        uint64_t now = FF_PalGetMonotonicNs();
        if (now >= deadline_ns) {
            return FF_SUCCESS;
        }
        uint64_t remaining = deadline_ns - now;
        struct timespec req = {(time_t)(remaining / 1000000000ULL),
                               (long)(remaining % 1000000000ULL)};
        while (nanosleep(&req, &req) != 0) {
            if (errno != EINTR) {
                return FF_ERROR;
            }
        }
    }
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

FF_Status_t FF_PalMutexTryLock(FF_MutexId_t id)
{
    int rc;
    if (id >= FF_PAL_MAX_MUTEXES || !g_mutexes[id].used) {
        return FF_NOT_FOUND;
    }
    rc = pthread_mutex_trylock(&g_mutexes[id].mtx);
    if (rc == 0) {
        return FF_SUCCESS;
    }
    return (rc == EBUSY) ? FF_TIMEOUT : FF_ERROR;
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

FF_Status_t FF_PalQueueTryPut(FF_QueueId_t id, const void *item)
{
    ff_queue_slot_t *q;
    int rc;

    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used) {
        return FF_NOT_FOUND;
    }
    if (item == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    q = &g_queues[id];
    rc = pthread_mutex_trylock(&q->mtx);
    if (rc == EBUSY) {
        return FF_TIMEOUT;
    }
    if (rc != 0) {
        return FF_ERROR;
    }
    if (q->count >= q->depth) {
        q->dropped++;
        pthread_mutex_unlock(&q->mtx);
        return FF_BUFFER_FULL;
    }
    memcpy(q->storage[q->head], item, q->item_size);
    q->head = (uint16_t)((q->head + 1u) % q->depth);
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mtx);
    return FF_SUCCESS;
}

#ifdef FF_TEST_HOOKS
FF_Status_t FF_PalQueueTestLock(FF_QueueId_t id)
{
    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used) {
        return FF_NOT_FOUND;
    }
    return (pthread_mutex_lock(&g_queues[id].mtx) == 0) ? FF_SUCCESS : FF_ERROR;
}

FF_Status_t FF_PalQueueTestUnlock(FF_QueueId_t id)
{
    if (id >= FF_PAL_MAX_QUEUES || !g_queues[id].used) {
        return FF_NOT_FOUND;
    }
    return (pthread_mutex_unlock(&g_queues[id].mtx) == 0) ? FF_SUCCESS : FF_ERROR;
}
#endif

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

/* ---- Watchdog hardware binding (host simulator / generic build) -----
 * Simulated backend: it records the arm/feed/reset requests and performs no
 * hardware action, so the FDIR mechanism can be stimulated deterministically
 * in tests. The RTEMS/MPFS hardware binding is provided by pal_rtems.c and is
 * not wired yet (NC-PAL-01). */
#if !defined(FRICK_PLATFORM_MPFS) && !defined(FRICK_PLATFORM_QEMU)
#ifdef FF_TEST_HOOKS
static FF_Status_t g_wd_test_arm_status = FF_SUCCESS;
static FF_Status_t g_wd_test_feed_status = FF_SUCCESS;
static FF_Status_t g_wd_test_reset_status = FF_SUCCESS;

void FF_PalWatchdogTestSetHardwareResults(FF_Status_t arm_status, FF_Status_t feed_status,
                                          FF_Status_t reset_status)
{
    g_wd_test_arm_status = arm_status;
    g_wd_test_feed_status = feed_status;
    g_wd_test_reset_status = reset_status;
}
#endif

FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms)
{
    FF_PalLog("watchdog: simulated backend armed (%u ms); no hardware reset "
              "(NC-PAL-01)\n",
              (unsigned)timeout_ms);
#ifdef FF_TEST_HOOKS
    return g_wd_test_arm_status;
#else
    return FF_SUCCESS;
#endif
}

FF_Status_t ff_pal_wd_hw_feed(void)
{
#ifdef FF_TEST_HOOKS
    return g_wd_test_feed_status;
#else
    return FF_SUCCESS;
#endif
}

FF_Status_t ff_pal_wd_hw_reset(void)
{
#ifdef FF_TEST_HOOKS
    return g_wd_test_reset_status;
#else
    FF_PalLog("watchdog: reset requested (simulated backend; no hardware "
              "reset, NC-PAL-01)\n");
    return FF_SUCCESS;
#endif
}
#endif

/* ---- Pre-reset notification -------------------------------------- */

static FF_Pal_PanicNotifier_t g_panic_notifier = NULL;

/* Bounded console write used by the panic path.
 *
 * The descriptor is switched to non-blocking mode so that a stalled console
 * cannot trap the panic context inside write(), and the descriptor mode is
 * restored before returning so later diagnostics keep the normal semantics.
 *
 * No poll()/select() is used: RTEMS 6.2 provides neither, and a target build
 * that referenced them would not link. A temporarily full descriptor is
 * therefore handled by a short back-off and a retry against the shared
 * deadline, which is bounded by construction.
 */
static void ff_panic_console_write_until(const char *text, size_t length, uint64_t deadline_ns)
{
    int flags = fcntl(STDERR_FILENO, F_GETFL, 0);
    if ((flags < 0) || (fcntl(STDERR_FILENO, F_SETFL, flags | O_NONBLOCK) != 0)) {
        return;
    }
    size_t written = 0u;
    while (written < length) {
        uint64_t now = FF_PalGetMonotonicNs();
        if (now >= deadline_ns) {
            break;
        }
        ssize_t n = write(STDERR_FILENO, text + written, length - written);
        if (n > 0) {
            written += (size_t)n;
            continue;
        }
        if ((n < 0) && (errno == EINTR)) {
            continue;
        }
        if ((n < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            struct timespec pause = {.tv_sec = 0, .tv_nsec = 1000000L};
            (void)nanosleep(&pause, NULL);
            continue;
        }
        break; /* hard error: the reset must not be blocked by the console */
    }
    (void)fcntl(STDERR_FILENO, F_SETFL, flags);
}

void FF_PalSetPanicNotifier(FF_Pal_PanicNotifier_t notifier)
{
    g_panic_notifier = notifier;
}

#ifdef FF_TEST_HOOKS
static FF_Pal_PanicHandler_t g_panic_handler = NULL;

void FF_PalSetPanicHandler(FF_Pal_PanicHandler_t handler)
{
    g_panic_handler = handler;
}
#endif

/* ---- Console ----------------------------------------------------- */

void FF_PalLog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (void)vfprintf(stdout, fmt, ap);
    (void)fflush(stdout);
    va_end(ap);
}

/*
 * Panic / reset-request path (ECSS-E-ST-70-11 5.7.5.2).
 *
 * Order is significant: (1) log, (2) generate the pre-reset event report so
 * that ground is notified BEFORE the reset is enforced, (3) request the single
 * watchdog reset path, (4) the test seam (test builds only), (5) abort.
 *
 * The notifier and the reset request are deliberately unconditional and their
 * failures are ignored: a failed notification must never prevent the reset.
 * Everything here is bounded and lock-free: no EVS/SB/PUS/queue/time mutex is
 * taken, because FF_PalPanic can be entered from any context, including while
 * holding one of those locks.
 */
void FF_PalPanic(const char *fmt, ...)
{
    char msg[FF_EVS_TEXT_LEN + 1u];
    char console_msg[FF_EVS_TEXT_LEN + 16u];
    uint64_t start_ns = FF_PalGetMonotonicNs();
    uint64_t deadline_ns = start_ns + ((uint64_t)FF_PANIC_REPORT_BUDGET_MS * 1000000ULL);
    bool message_complete;
    int message_len;
    va_list ap;

    va_start(ap, fmt);
    message_len = vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    if (message_len < 0) {
        /* Encoding error: the buffer contents are unspecified. Do not use
         * them; report an empty reason instead of undefined text. */
        msg[0] = '\0';
        message_complete = false;
    }
    else {
        message_complete = ((size_t)message_len < sizeof(msg));
    }

    int console_len = snprintf(console_msg, sizeof(console_msg), "\n[FF-PANIC] %s\n", msg);
    if (console_len > 0) {
        size_t bounded_len = (size_t)console_len;
        if (bounded_len >= sizeof(console_msg)) {
            bounded_len = sizeof(console_msg) - 1u;
        }
        ff_panic_console_write_until(console_msg, bounded_len, deadline_ns);
    }

    if (message_complete && (g_panic_notifier != NULL)) {
        (void)g_panic_notifier(msg, deadline_ns);
    }

    (void)FF_PalWatchdogReset();

#ifdef FF_TEST_HOOKS
    if (g_panic_handler != NULL) {
        g_panic_handler(msg);
    }
#endif

    /* The platform did not perform a reset (simulated or unwired binding):
     * never continue silently. */
    abort();
}
