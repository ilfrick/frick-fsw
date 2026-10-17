/*
 * frick-fsw - Application schedule dispatcher (M17)
 *
 * Bounded, allocation-free, table-driven dispatch of Software Bus messages
 * against a monotonic time base. This is an *application* dispatcher: it
 * decides which configured SB messages are released and when, and deliberately
 * does not select or preempt tasks -- RTEMS owns runnable-task selection and
 * priority preemption (SSS-REQ-030 / SRS-REQ-071).
 *
 * Design and authority boundary: docs/architecture/scheduling.md
 * (FRICK-ARCH-0018). Mission rates, major-frame synchronization source,
 * allowed jitter, task priority/stack, late-slot policy and recovery authority
 * are mission-controlled inputs and remain TBD; the values and policies in
 * this module are project-selected provisional behaviour, not an approved
 * mission profile, and no schedulability claim is made.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_SCHED_H
#define FF_SCHED_H

#include "common/ff_types.h"
#include "core/sb/sb.h"
#include <stddef.h>
#include <stdint.h>

/* Fixed table capacity (static memory; no dynamic allocation anywhere). */
#define FF_SCHED_MAX_ENTRIES 16u

/* Maximum payload carried by one schedule entry (bounded copy source). */
#define FF_SCHED_PAYLOAD_MAX 32u

/* Schedule image magic ('FSH1'). */
#define FF_SCHED_MAGIC 0x46534831u

/* One validated schedule slot. */
typedef struct
{
    FF_MsgId_t msg_id;
    uint16_t period_ms; /* period in ms; 0 is invalid             */
    uint16_t payload_len;
    uint8_t payload[FF_SCHED_PAYLOAD_MAX];
} FF_SchedEntry_t;

/* Complete schedule image; count bounds the number of live entries. */
typedef struct
{
    uint32_t magic;
    uint16_t count; /* <= FF_SCHED_MAX_ENTRIES                   */
    uint16_t reserved;
    FF_SchedEntry_t entries[FF_SCHED_MAX_ENTRIES];
} FF_SchedTable_t;

/* Observable dispatch health (SRS-REQ-071 heartbeat/overrun evidence). */
typedef struct
{
    uint32_t activations;         /* accepted table activations          */
    uint32_t invalid_activations; /* rejected (invalid) activations      */
    uint32_t dispatched;          /* release slots consumed (incl. failed pubs) */
    uint32_t missed;              /* release slots skipped while late    */
    uint32_t late_polls;          /* polls that had to skip a slot       */
    uint32_t publish_errors;      /* FF_SbPublish() failures             */
    uint32_t time_exhaustions;    /* entries disabled rather than wrapping a deadline */
} FF_SchedStats_t;

/*
 * Validate a schedule image without activating it.
 * Rejects: wrong size, bad magic, count > FF_SCHED_MAX_ENTRIES, zero period,
 * payload_len > FF_SCHED_PAYLOAD_MAX or > FF_SB_MSG_PAYLOAD_MAX, and duplicate
 * msg_id entries. Rate divisibility and major-frame alignment are NOT checked
 * here: they are mission inputs that remain TBD (FRICK-ARCH-0018 s2/s5).
 */
FF_Status_t FF_SchedValidate(const void *image, size_t size);

/*
 * Validate then atomically activate an image. On any validation failure the
 * previously active table is left untouched and invalid_activations is
 * incremented; returns FF_ERROR.
 */
FF_Status_t FF_SchedActivate(const void *image, size_t size);

/* Number of entries in the currently active table (0 before first activation). */
uint16_t FF_SchedActiveCount(void);

/*
 * Initialise the dispatcher epoch at now_ns. Releases are scheduled one full
 * period after the epoch, so no message is emitted before its period elapses.
 */
void FF_SchedInit(uint64_t now_ns);

/*
 * Release every entry that is due at now_ns and return the number released
 * (bounded by max_out). out may be NULL when the caller only wants the release
 * side effect and the counters. Each entry releases at most one message per
 * call, so a late poll cannot produce an unbounded catch-up burst: skipped
 * slots are counted in stats.missed and the next release target is advanced by
 * absolute period multiples (no cumulative relative-delay drift).
 * If an absolute target cannot be represented in uint64_t nanoseconds, the
 * entry is disabled until FF_SchedInit and stats.time_exhaustions increments.
 * A new activation clears targets and must be followed by FF_SchedInit before
 * dispatch resumes. stats.missed saturates at UINT32_MAX. These are provisional
 * framework policies, not mission time-domain or recovery approvals.
 */
uint16_t FF_SchedPoll(uint64_t now_ns, FF_Msg_t *out, uint16_t max_out, FF_AppId_t app);

/* Copy the current counters (never NULL). */
void FF_SchedGetStats(FF_SchedStats_t *out);

/* Clear counters (test/support use). */
void FF_SchedResetStats(void);

#endif /* FF_SCHED_H */
