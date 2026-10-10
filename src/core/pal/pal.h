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
 *   - pal_rtems.c : RTEMS specific bootstrap (POSIX keys, clock, panic).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
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
void FF_PalTaskDelayTicks(uint32_t ticks);
void FF_PalTaskYield(void);
void FF_PalTaskExit(void) __attribute__((noreturn));
FF_TaskId_t FF_PalTaskSelf(void);

/* ---- Mutexes ----------------------------------------------------- */
FF_Status_t FF_PalMutexCreate(FF_MutexId_t *id, const char *name);
FF_Status_t FF_PalMutexLock(FF_MutexId_t id, uint32_t timeout_ms);
FF_Status_t FF_PalMutexUnlock(FF_MutexId_t id);
FF_Status_t FF_PalMutexDelete(FF_MutexId_t id);

/* ---- Message queues (fixed size items, blocking) ----------------- */
FF_Status_t FF_PalQueueCreate(FF_QueueId_t *id, const char *name, uint16_t depth,
                              uint16_t item_size);
FF_Status_t FF_PalQueuePut(FF_QueueId_t id, const void *item, uint32_t timeout_ms);
FF_Status_t FF_PalQueueGet(FF_QueueId_t id, void *item, uint32_t timeout_ms);
FF_Status_t FF_PalQueueDelete(FF_QueueId_t id);
uint16_t FF_PalQueueCount(FF_QueueId_t id);

/* ---- Time -------------------------------------------------------- */
uint64_t FF_PalGetMonotonicNs(void);
uint64_t FF_PalGetUtcNs(void);
FF_Status_t FF_PalSetUtcNs(uint64_t utc_ns);
uint32_t FF_PalGetCpuLoadPpt(void);

/* ---- Console / logging ------------------------------------------ */
void FF_PalLog(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void FF_PalPanic(const char *fmt, ...) __attribute__((format(printf, 1, 2)))
__attribute__((noreturn));

#endif /* FF_PAL_H */
