/*
 * frick-fsw - Platform Abstraction Layer (PAL)
 *
 * The PAL is the frick-fsw equivalent of the cFS Operating System
 * Abstraction Layer (OSAL). It exposes a small, deterministic, POSIX-like
 * surface (tasks, mutexes, message queues, time, console) and hides the
 * RTOS. Two implementations are provided:
 *
 *   - pal_posix.c : POSIX pthread/mqueue based, used both on the Linux host
 *                   simulator and on RTEMS 6, which ships a POSIX API.
 *   - pal_rtems.c : RTEMS 6 specific bootstrap (POSIX_API check, the
 *                   POSIX_Init() application entry point, clock, panic; the
 *                   RTEMS configuration table is config/rtems/rtems_config.c,
 *                   see docs/architecture/rtems-integration.md).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_PAL_H
#define FF_PAL_H

#include "common/ff_types.h"
#include "fsw_config.h"

#define FF_PAL_WAIT_FOREVER 0xFFFFFFFFu
#define FF_PAL_NAME_LEN 16u

typedef void (*FF_PalTaskFunc_t)(void *arg);

/* ---- Lifecycle --------------------------------------------------- */
FF_Status_t FF_PalInit(void);
FF_Status_t FF_PalPlatformInit(void);
void FF_PalShutdown(void);

/* ---- Tasks ------------------------------------------------------- */
FF_Status_t FF_PalTaskCreate(FF_TaskId_t *id, const char *name, FF_PalTaskFunc_t fn, void *arg,
                             uint32_t stack_bytes, uint8_t priority);
FF_Status_t FF_PalTaskDelete(FF_TaskId_t id);
void FF_PalTaskDelayMs(uint32_t ms);
/* Wait until an absolute deadline in FF_PalGetMonotonicNs()'s time domain.
 * Returns immediately with FF_SUCCESS when the deadline has passed. */
FF_Status_t FF_PalTaskDelayUntilNs(uint64_t deadline_ns);
void FF_PalTaskDelayTicks(uint32_t ticks);
void FF_PalTaskYield(void);
void FF_PalTaskExit(void) __attribute__((noreturn));
FF_TaskId_t FF_PalTaskSelf(void);

/* ---- Mutexes ----------------------------------------------------- */
FF_Status_t FF_PalMutexCreate(FF_MutexId_t *id, const char *name);
FF_Status_t FF_PalMutexLock(FF_MutexId_t id, uint32_t timeout_ms);
/* Attempts to acquire once without waiting; busy mutexes return FF_TIMEOUT. */
FF_Status_t FF_PalMutexTryLock(FF_MutexId_t id);
FF_Status_t FF_PalMutexUnlock(FF_MutexId_t id);
FF_Status_t FF_PalMutexDelete(FF_MutexId_t id);

/* ---- Message queues (fixed size items, blocking) ----------------- */
FF_Status_t FF_PalQueueCreate(FF_QueueId_t *id, const char *name, uint16_t depth,
                              uint16_t item_size);
FF_Status_t FF_PalQueuePut(FF_QueueId_t id, const void *item, uint32_t timeout_ms);
/* Acquires the queue mutex once; FF_TIMEOUT means busy, FF_BUFFER_FULL means full. */
FF_Status_t FF_PalQueueTryPut(FF_QueueId_t id, const void *item);
FF_Status_t FF_PalQueueGet(FF_QueueId_t id, void *item, uint32_t timeout_ms);
FF_Status_t FF_PalQueueDelete(FF_QueueId_t id);
uint16_t FF_PalQueueCount(FF_QueueId_t id);
#ifdef FF_TEST_HOOKS
FF_Status_t FF_PalQueueTestLock(FF_QueueId_t id);
FF_Status_t FF_PalQueueTestUnlock(FF_QueueId_t id);
#endif

/* ---- Time -------------------------------------------------------- */
uint64_t FF_PalGetMonotonicNs(void);
uint64_t FF_PalGetUtcNs(void);
FF_Status_t FF_PalSetUtcNs(uint64_t utc_ns);
uint32_t FF_PalGetCpuLoadPpt(void);

/* ---- Hardware watchdog (FDIR reset recovery) ---------------------
 * ECSS-E-ST-70-11 5.7.5.2: the recovery action that overcomes an on-board
 * failure is typically hardware-generated so that it is independent of the
 * on-board software it resets. The watchdog is armed with a timeout and must
 * be fed periodically by mission software; if feeding stops (hung scheduler,
 * dead task, stuck interrupt path) the hardware resets the processor.
 *
 * The platform binding is deliberately abstracted (norm N1/N8):
 *   - host builds use a simulated backend that records arm/feed/reset so the
 *     mechanism can be stimulated deterministically in tests;
 *   - the RTEMS/MPFS hardware binding is not wired (NC-PAL-01).
 * FF_PalWatchdogReset() requests the single recovery path used both by a
 * timeout and by an explicit software reset request. It returns only when the
 * platform did not perform a reset (host/dummy or unwired binding); callers
 * must not silently continue in that case.
 */
FF_Status_t FF_PalWatchdogInit(uint32_t timeout_ms);
FF_Status_t FF_PalWatchdogFeed(void);
/* Monotonic-time diagnostic expiry estimate. The current feeder does not call
 * this function; hardware expiry is the independent protection when wired. */
FF_Status_t FF_PalWatchdogCheck(void);
FF_Status_t FF_PalWatchdogReset(void);
uint32_t FF_PalWatchdogGetFeeds(void);
uint32_t FF_PalWatchdogGetResets(void);
uint32_t FF_PalWatchdogGetTimeoutMs(void);

/* Pre-reset notification: the application layer registers a bounded,
 * lock-free reporter (PUS service 5 event report to ground). PAL has no
 * upward dependency on PUS; an unset notifier or a failing notifier never
 * blocks the reset. */
typedef FF_Status_t (*FF_Pal_PanicNotifier_t)(const char *reason, uint64_t deadline_ns);
void FF_PalSetPanicNotifier(FF_Pal_PanicNotifier_t notifier);

#ifdef FF_TEST_HOOKS
/* Host-test-only SUT-interface fault injection; absent from flight builds. */
void FF_PalWatchdogTestSetHardwareResults(FF_Status_t arm_status, FF_Status_t feed_status,
                                          FF_Status_t reset_status);
/* Test seam (absent from production builds): observe FF_PalPanic without
 * terminating the process. The handler runs after the pre-reset notification
 * and after the reset request, so a test can assert both; it must not return
 * normally (it is expected to longjmp). Used for N22-TS5-50. */
typedef void (*FF_Pal_PanicHandler_t)(const char *message);
void FF_PalSetPanicHandler(FF_Pal_PanicHandler_t handler);
/* Deterministically simulate a missed feed deadline (dummy backend only). */
FF_Status_t FF_PalWatchdogTestStarve(uint32_t elapsed_ms);
#endif

/* ---- Console / logging ------------------------------------------ */
void FF_PalLog(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void FF_PalPanic(const char *fmt, ...) __attribute__((format(printf, 1, 2)))
__attribute__((noreturn));

#endif /* FF_PAL_H */
