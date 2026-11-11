/*
 * frick-fsw - Scheduler application EVS event definitions
 *
 * Shared definitions for scheduler application EVS event IDs and payload format strings.
 * Used by both app_scheduler.c and test_sched_events.c to ensure consistency.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_SCHED_EVT_H
#define FF_SCHED_EVT_H

#include <stdint.h>

#include "core/tbl/tbl.h"

/* Scheduler application timing constants (shared with tests) */
#define FF_SCHED_TICK_MS 100u
#define FF_SCHED_HEARTBEAT_MS 5000u

/* EVS event IDs for the scheduler application */
#define FF_SCHED_EVT_ACTIVATED 0x0601u
#define FF_SCHED_EVT_INVALID_TABLE 0x0602u
#define FF_SCHED_EVT_MISSED_SLOT 0x0603u
#define FF_SCHED_EVT_HEARTBEAT 0x0604u
#define FF_SCHED_EVT_DISPATCH_FAIL 0x0605u
#define FF_SCHED_EVT_RECFG_NOT_READY 0x0606u

/* EVS event payload format strings (must match app_scheduler.c exactly) */
#define FF_SCHED_EVT_ACTIVATED_FMT "SCH app started, %u entries"
/* Reuses FF_SCHED_EVT_ACTIVATED for a table activation requested through Table
 * Services after startup, so the ground-visible event-id catalogue is unchanged. */
#define FF_SCHED_EVT_REACTIVATED_FMT "SCH: table activated, %u entries"
#define FF_SCHED_EVT_INVALID_TABLE_FMT "SCH: schedule table rejected, dispatch inhibited"
#define FF_SCHED_EVT_MISSED_SLOT_FMT "SCH: %u release slots skipped (total %u)"
#define FF_SCHED_EVT_HEARTBEAT_FMT "SCH heartbeat"
#define FF_SCHED_EVT_DISPATCH_FAIL_FMT "SCH: %u dispatch failures (total %u)"
#define FF_SCHED_EVT_RECFG_NOT_READY_FMT "SCH: activation ignored, reconfiguration not ready"

#ifdef FF_TEST_HOOKS
/* Test-only control functions for FF_AppSchedulerMain (declared in app_scheduler.c) */
void FF_SchedTestRequestExit(void);
void FF_SchedTestResetExit(void);
/* Handle of the schedule table the application registered with Table Services
 * (FF_TBL_INVALID_HANDLE before registration). */
FF_TblHandle_t FF_SchedTestTableHandle(void);
#endif

#endif /* FF_SCHED_EVT_H */