/*
 * frick-fsw - lwIP sys_arch port for the RTEMS target
 *
 * Implements the lwIP 2.x system abstraction (NO_SYS=0) over the RTEMS classic
 * API, so the lwIP network-stack adapter (SRS-REQ-040) can be built for the
 * RTEMS target (`FF_USE_LWIP=ON`). The pinned lwIP ships no RTEMS port
 * (NC-S6-01); this file, together with `arch/cc.h` and `arch/sys_arch.h`, is
 * that port. It is used only for the RTEMS target - the host build keeps using
 * the upstream lwIP unix port.
 *
 * RTEMS primitives used (bounded; no dynamic memory allocation beyond the RTEMS
 * kernel objects):
 *   - semaphores : rtems_semaphore_create() counting / binary semaphores;
 *   - mutexes    : binary semaphore with priority inheritance;
 *   - mailboxes  : rtems_message_queue_create() of `void *` messages;
 *   - threads    : rtems_task_create()/rtems_task_start();
 *   - critical sections: rtems_interrupt_disable()/rtems_interrupt_enable();
 *   - time       : rtems_clock_get_ticks_since_boot()/..._per_second().
 *
 * Runtime prerequisites (RTEMS application configuration, owned by the platform
 * bring-up):
 *   - the classic API must provide enough semaphores, message queues and tasks
 *     for the configured lwIP object counts (see config/rtems/rtems_config.c).
 *
 * See docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "lwip/opt.h"

#if NO_SYS
#error "the frick-fsw RTEMS sys_arch port requires NO_SYS=0 (threaded lwIP)"
#endif

#include "lwip/err.h"
#include "lwip/sys.h"

#include <rtems.h>

/* Human-readable base name for the RTEMS objects created by this port. */
#define FRICK_LWIP_OBJ_NAME rtems_build_name('L', 'W', 'I', 'P')

/* Bounded sizing. A mailbox depth of 0 would create an unusable RTEMS message
 * queue, so a positive default is always used. */
#define FRICK_LWIP_DEFAULT_MBOX_SIZE 8
#define FRICK_LWIP_MIN_THREAD_STACK (8 * 1024)
#define FRICK_LWIP_DEFAULT_THREAD_STACK (8 * 1024)

/* Number of concurrently starting lwIP threads supported by the static start
 * pool below (lwIP normally creates a single tcpip thread). */
#define FRICK_LWIP_THREAD_SLOTS 8

/* RTEMS task priority bounds (0 is the most urgent, 255 the least urgent). */
#define FRICK_LWIP_PRIO_MIN 1
#define FRICK_LWIP_PRIO_MAX 254

/* ------------------------------------------------------------------ */
/* Static start records handed from sys_thread_new() to the task entry */
/* ------------------------------------------------------------------ */
typedef struct
{
    lwip_thread_fn fn;
    void *arg;
    volatile int in_use;
} frick_lwip_thread_slot_t;

static frick_lwip_thread_slot_t frick_lwip_thread_slots[FRICK_LWIP_THREAD_SLOTS];

/* ------------------------------------------------------------------ */
/* Thread-creation diagnostics (NC-151-05)                            */
/* ------------------------------------------------------------------ */
/* sys_thread_new() must not fail silently: a task-create failure is a
 * configuration error (insufficient CONFIGURE_MAXIMUM_TASKS / workspace) that
 * would otherwise leave a dead stack with no diagnostic (tcpip_init() ignores
 * the returned handle). The port records every failure - a bounded counter and
 * the last RTEMS status code - readable through the accessors below. A message
 * is emitted only when lwIP debug is enabled (the RTEMS arch/cc.h maps
 * LWIP_PLATFORM_DIAG to the console), so no printf is compiled into the
 * default flight image. */
static volatile unsigned int frick_lwip_thread_failures;
static volatile int frick_lwip_thread_last_status;

unsigned int frick_lwip_port_thread_failures(void)
{
    return frick_lwip_thread_failures;
}

int frick_lwip_port_last_thread_status(void)
{
    return frick_lwip_thread_last_status;
}

static void frick_lwip_thread_failed(int status)
{
    frick_lwip_thread_failures++;
    frick_lwip_thread_last_status = status;
#if LWIP_DEBUG
    LWIP_PLATFORM_DIAG(("lwIP: sys_thread_new failed (RTEMS status 0x%x)\n", (unsigned int)status));
#endif
}

/* --------------------------------------------------------------- */
/* Internal helpers                                               */
/* --------------------------------------------------------------- */

/* Convert a positive millisecond timeout to RTEMS ticks, rounding up so a
 * non-zero millisecond timeout never becomes an immediate (0 tick) timeout. */
static rtems_interval frick_lwip_ms_to_ticks(u32_t ms)
{
    uint64_t rate = (uint64_t)rtems_clock_get_ticks_per_second();
    rtems_interval ticks;

    if (rate == 0u) {
        return 1u;
    }
    ticks = (rtems_interval)(((uint64_t)ms * rate + 999u) / 1000u);
    if (ticks == 0u) {
        ticks = 1u;
    }
    return ticks;
}

/* Map the lwIP thread priority to an RTEMS task priority. The lwIP priority is
 * used directly (both use "smaller value = more urgent"), clamped to the valid
 * RTEMS range so a caller cannot create an out-of-range task. */
static rtems_task_priority frick_lwip_task_prio(int prio)
{
    int p = prio;

    if (p < FRICK_LWIP_PRIO_MIN) {
        p = FRICK_LWIP_PRIO_MIN;
    }
    else if (p > FRICK_LWIP_PRIO_MAX) {
        p = FRICK_LWIP_PRIO_MAX;
    }
    return (rtems_task_priority)p;
}

/* Build an RTEMS four-character object name from an lwIP thread name. */
static rtems_name frick_lwip_task_name(const char *name)
{
    char chars[4] = {' ', ' ', ' ', ' '};
    size_t i = 0u;

    if (name != NULL) {
        while ((i < 4u) && (name[i] != '\0')) {
            chars[i] = name[i];
            i++;
        }
    }
    return rtems_build_name(chars[0], chars[1], chars[2], chars[3]);
}

/* Reserve a static start record. */
static frick_lwip_thread_slot_t *frick_lwip_slot_acquire(void)
{
    frick_lwip_thread_slot_t *slot = NULL;
    sys_prot_t level;
    size_t i;

    level = sys_arch_protect();
    for (i = 0u; i < FRICK_LWIP_THREAD_SLOTS; i++) {
        if (frick_lwip_thread_slots[i].in_use == 0) {
            frick_lwip_thread_slots[i].in_use = 1;
            slot = &frick_lwip_thread_slots[i];
            break;
        }
    }
    sys_arch_unprotect(level);
    return slot;
}

/* --------------------------------------------------------------- */
/* Initialisation and time                                        */
/* --------------------------------------------------------------- */

void sys_init(void)
{
    /* Nothing to initialise: the RTEMS objects are created in the individual
     * sys_*_new() calls and the start-record pool is statically zeroed. */
}

u32_t sys_now(void)
{
    uint64_t ticks = (uint64_t)rtems_clock_get_ticks_since_boot();
    uint64_t rate = (uint64_t)rtems_clock_get_ticks_per_second();

    if (rate == 0u) {
        return (u32_t)ticks;
    }
    return (u32_t)((ticks * 1000u) / rate);
}

u32_t sys_jiffies(void)
{
    return (u32_t)rtems_clock_get_ticks_since_boot();
}

/* Random source for lwIP (TCP initial sequence numbers / ephemeral ports). A
 * bounded xorshift keeps no libc state and cannot block. */
unsigned int lwip_rtems_rand(void)
{
    static u32_t state;
    static int seeded;

    if (seeded == 0) {
        state = (u32_t)rtems_clock_get_ticks_since_boot() ^ 0xA5A5A5A5u;
        if (state == 0u) {
            state = 0x1234567u;
        }
        seeded = 1;
    }
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return (unsigned int)state;
}

/* --------------------------------------------------------------- */
/* Critical sections                                              */
/* --------------------------------------------------------------- */

sys_prot_t sys_arch_protect(void)
{
    rtems_interrupt_level level;

    rtems_interrupt_disable(level);
    return (sys_prot_t)level;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    rtems_interrupt_enable((rtems_interrupt_level)pval);
}

/* --------------------------------------------------------------- */
/* Semaphores (counting; lwIP uses an initial count of 0 or 1)     */
/* --------------------------------------------------------------- */

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    rtems_status_code sc;

    if (sem == NULL) {
        return ERR_ARG;
    }
    sem->id = RTEMS_ID_NONE;
    sc = rtems_semaphore_create(FRICK_LWIP_OBJ_NAME, (uint32_t)count,
                                RTEMS_COUNTING_SEMAPHORE | RTEMS_PRIORITY | RTEMS_LOCAL, 0,
                                &sem->id);
    if (sc != RTEMS_SUCCESSFUL) {
        sem->id = RTEMS_ID_NONE;
        return ERR_MEM;
    }
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
    if ((sem != NULL) && (sem->id != RTEMS_ID_NONE)) {
        (void)rtems_semaphore_delete(sem->id);
        sem->id = RTEMS_ID_NONE;
    }
}

void sys_sem_signal(sys_sem_t *sem)
{
    if ((sem != NULL) && (sem->id != RTEMS_ID_NONE)) {
        (void)rtems_semaphore_release(sem->id);
    }
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    rtems_status_code sc;

    if ((sem == NULL) || (sem->id == RTEMS_ID_NONE)) {
        return SYS_ARCH_TIMEOUT;
    }
    if (timeout == 0u) {
        sc = rtems_semaphore_obtain(sem->id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    }
    else {
        sc = rtems_semaphore_obtain(sem->id, RTEMS_WAIT, frick_lwip_ms_to_ticks(timeout));
    }
    return (sc == RTEMS_SUCCESSFUL) ? 1u : SYS_ARCH_TIMEOUT;
}

/* --------------------------------------------------------------- */
/* Mutexes (binary semaphore with priority inheritance)            */
/* --------------------------------------------------------------- */

err_t sys_mutex_new(sys_mutex_t *mutex)
{
    rtems_status_code sc;

    if (mutex == NULL) {
        return ERR_ARG;
    }
    mutex->id = RTEMS_ID_NONE;
    sc = rtems_semaphore_create(FRICK_LWIP_OBJ_NAME, 1,
                                RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY, 0,
                                &mutex->id);
    if (sc != RTEMS_SUCCESSFUL) {
        mutex->id = RTEMS_ID_NONE;
        return ERR_MEM;
    }
    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    if ((mutex != NULL) && (mutex->id != RTEMS_ID_NONE)) {
        (void)rtems_semaphore_obtain(mutex->id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    }
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    if ((mutex != NULL) && (mutex->id != RTEMS_ID_NONE)) {
        (void)rtems_semaphore_release(mutex->id);
    }
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    if ((mutex != NULL) && (mutex->id != RTEMS_ID_NONE)) {
        (void)rtems_semaphore_delete(mutex->id);
        mutex->id = RTEMS_ID_NONE;
    }
}

/* --------------------------------------------------------------- */
/* Mailboxes (RTEMS message queue of `void *` messages)            */
/* --------------------------------------------------------------- */

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    uint32_t depth = (size > 0) ? (uint32_t)size : (uint32_t)FRICK_LWIP_DEFAULT_MBOX_SIZE;
    rtems_status_code sc;

    if (mbox == NULL) {
        return ERR_ARG;
    }
    mbox->id = RTEMS_ID_NONE;
    sc = rtems_message_queue_create(FRICK_LWIP_OBJ_NAME, depth, sizeof(void *),
                                    RTEMS_FIFO | RTEMS_LOCAL, &mbox->id);
    if (sc != RTEMS_SUCCESSFUL) {
        mbox->id = RTEMS_ID_NONE;
        return ERR_MEM;
    }
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    if ((mbox != NULL) && (mbox->id != RTEMS_ID_NONE)) {
        (void)rtems_message_queue_delete(mbox->id);
        mbox->id = RTEMS_ID_NONE;
    }
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    if ((mbox == NULL) || (mbox->id == RTEMS_ID_NONE)) {
        return;
    }
    /* rtems_message_queue_send() returns RTEMS_TOO_MANY when the queue is full
     * (it never blocks), so a full queue is handled by yielding until a slot is
     * free. This keeps sys_mbox_post() non-failing as lwIP requires. */
    for (;;) {
        rtems_status_code sc = rtems_message_queue_send(mbox->id, &msg, sizeof(msg));

        if (sc == RTEMS_SUCCESSFUL) {
            break;
        }
        if (sc != RTEMS_TOO_MANY) {
            break; /* invalid/deleted queue: cannot make progress */
        }
        (void)rtems_task_wake_after(1);
    }
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    rtems_status_code sc;

    if ((mbox == NULL) || (mbox->id == RTEMS_ID_NONE)) {
        return ERR_MEM;
    }
    sc = rtems_message_queue_send(mbox->id, &msg, sizeof(msg));
    return (sc == RTEMS_SUCCESSFUL) ? ERR_OK : ERR_MEM;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    /* rtems_message_queue_send() is callable from interrupt context and never
     * blocks (RTEMS_TOO_MANY when full), which is the try-post-from-ISR
     * contract. */
    return sys_mbox_trypost(mbox, msg);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    void *dummy = NULL;
    void **dst;
    size_t size;
    rtems_status_code sc;

    if ((mbox == NULL) || (mbox->id == RTEMS_ID_NONE)) {
        if (msg != NULL) {
            *msg = NULL;
        }
        return SYS_ARCH_TIMEOUT;
    }
    dst = (msg != NULL) ? msg : &dummy;
    size = sizeof(*dst);
    if (timeout == 0u) {
        sc = rtems_message_queue_receive(mbox->id, dst, &size, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    }
    else {
        sc = rtems_message_queue_receive(mbox->id, dst, &size, RTEMS_WAIT,
                                         frick_lwip_ms_to_ticks(timeout));
    }
    if (sc != RTEMS_SUCCESSFUL) {
        if (msg != NULL) {
            *msg = NULL;
        }
        return SYS_ARCH_TIMEOUT;
    }
    return 1u;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummy = NULL;
    void **dst;
    size_t size;
    rtems_status_code sc;

    if ((mbox == NULL) || (mbox->id == RTEMS_ID_NONE)) {
        if (msg != NULL) {
            *msg = NULL;
        }
        return SYS_MBOX_EMPTY;
    }
    dst = (msg != NULL) ? msg : &dummy;
    size = sizeof(*dst);
    sc = rtems_message_queue_receive(mbox->id, dst, &size, RTEMS_NO_WAIT, 0);
    if (sc != RTEMS_SUCCESSFUL) {
        if (msg != NULL) {
            *msg = NULL;
        }
        return SYS_MBOX_EMPTY;
    }
    return 0u;
}

/* --------------------------------------------------------------- */
/* Threads (RTEMS tasks)                                          */
/* --------------------------------------------------------------- */

/* Task entry point: recover the thread function/argument, release the start
 * record, then run the lwIP thread body. */
static void frick_lwip_thread_entry(rtems_task_argument argument)
{
    frick_lwip_thread_slot_t *slot = (frick_lwip_thread_slot_t *)argument;
    lwip_thread_fn fn = slot->fn;
    void *arg = slot->arg;
    sys_prot_t level;

    level = sys_arch_protect();
    slot->in_use = 0;
    sys_arch_unprotect(level);

    fn(arg);
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize,
                            int prio)
{
    sys_thread_t result;
    frick_lwip_thread_slot_t *slot;
    size_t stack = FRICK_LWIP_DEFAULT_THREAD_STACK;
    rtems_mode modes;
    rtems_id id = RTEMS_ID_NONE;
    rtems_status_code sc;

    result.id = RTEMS_ID_NONE;

    if (stacksize > 0) {
        stack = (size_t)stacksize;
    }
    if (stack < FRICK_LWIP_MIN_THREAD_STACK) {
        stack = FRICK_LWIP_MIN_THREAD_STACK;
    }

    slot = frick_lwip_slot_acquire();
    if (slot == NULL) {
        /* lwIP expects sys_thread_new() not to fail; exhausting the bounded
         * start pool is a configuration error. Record it (NC-151-05) and return
         * an invalid handle. */
        frick_lwip_thread_failed((int)RTEMS_TOO_MANY);
        return result;
    }
    slot->fn = thread;
    slot->arg = arg;

    modes =
        (rtems_mode)(RTEMS_PREEMPT | RTEMS_NO_TIMESLICE | RTEMS_NO_ASR | RTEMS_INTERRUPT_LEVEL(0));
    sc = rtems_task_create(frick_lwip_task_name(name), frick_lwip_task_prio(prio), stack, modes,
                           RTEMS_LOCAL, &id);
    if (sc != RTEMS_SUCCESSFUL) {
        frick_lwip_thread_failed((int)sc);
        slot->in_use = 0;
        return result;
    }
    sc = rtems_task_start(id, frick_lwip_thread_entry, (rtems_task_argument)slot);
    if (sc != RTEMS_SUCCESSFUL) {
        frick_lwip_thread_failed((int)sc);
        (void)rtems_task_delete(id);
        slot->in_use = 0;
        return result;
    }
    result.id = id;
    return result;
}
