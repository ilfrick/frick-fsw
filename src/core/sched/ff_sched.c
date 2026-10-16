/*
 * frick-fsw - Application schedule dispatcher (M17)
 *
 * See ff_sched.h for the contract, bounds and authority boundary. This module
 * is deliberately I/O-free (it publishes through the SB interface only) so the
 * release decision is deterministic and host-testable with an injected clock.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "core/sched/ff_sched.h"

#include <string.h>

/* Double-buffered schedule images. g_active is the index the dispatcher reads;
 * an activation fills the *inactive* buffer and then publishes the new index,
 * so a reader always observes a complete, consistent table (single-writer
 * contract). Multi-core memory ordering beyond this single-writer publish is
 * not established here and is tracked with the platform/SMP work (M14). */
static FF_SchedTable_t g_images[2];
static volatile uint32_t g_active;
static uint8_t g_has_active;

/* Absolute next-release target per active entry, in the monotonic ns base. */
static uint64_t g_next_due_ns[FF_SCHED_MAX_ENTRIES];

static FF_SchedStats_t g_stats;

#define FF_NS_PER_MS 1000000ULL

static FF_Status_t ff_sched_check_image(const FF_SchedTable_t *t, size_t size)
{
    if (t == NULL) {
        return FF_ERROR;
    }
    if (size != sizeof(FF_SchedTable_t)) {
        return FF_ERROR;
    }
    if (t->magic != FF_SCHED_MAGIC) {
        return FF_ERROR;
    }
    if (t->count > FF_SCHED_MAX_ENTRIES) {
        return FF_ERROR;
    }
    for (uint16_t i = 0u; i < t->count; i++) {
        const FF_SchedEntry_t *e = &t->entries[i];
        if (e->period_ms == 0u) {
            return FF_ERROR;
        }
        if ((e->payload_len > FF_SCHED_PAYLOAD_MAX) || (e->payload_len > FF_SB_MSG_PAYLOAD_MAX)) {
            return FF_ERROR;
        }
        for (uint16_t j = 0u; j < i; j++) {
            if (t->entries[j].msg_id == e->msg_id) {
                return FF_ERROR; /* duplicate/conflicting entry */
            }
        }
    }
    return FF_SUCCESS;
}

FF_Status_t FF_SchedValidate(const void *image, size_t size)
{
    return ff_sched_check_image((const FF_SchedTable_t *)image, size);
}

FF_Status_t FF_SchedActivate(const void *image, size_t size)
{
    if (ff_sched_check_image((const FF_SchedTable_t *)image, size) != FF_SUCCESS) {
        g_stats.invalid_activations++;
        return FF_ERROR;
    }
    /* Fill the inactive buffer, then publish the index (single-writer swap). */
    uint32_t next = (uint32_t)((g_active + 1u) & 1u);
    memcpy(&g_images[next], image, sizeof(FF_SchedTable_t));
    g_active = next;
    g_has_active = 1u;
    g_stats.activations++;

    /* A freshly activated table restarts its absolute release targets from the
     * dispatcher epoch; the caller re-initialises with FF_SchedInit(). */
    for (uint16_t i = 0u; i < FF_SCHED_MAX_ENTRIES; i++) {
        g_next_due_ns[i] = 0u;
    }
    return FF_SUCCESS;
}

uint16_t FF_SchedActiveCount(void)
{
    if (g_has_active == 0u) {
        return 0u;
    }
    return g_images[g_active].count;
}

void FF_SchedInit(uint64_t now_ns)
{
    const FF_SchedTable_t *t = &g_images[g_active];
    if (g_has_active == 0u) {
        return;
    }
    for (uint16_t i = 0u; i < t->count; i++) {
        /* First release is one full period after the epoch. */
        g_next_due_ns[i] = now_ns + ((uint64_t)t->entries[i].period_ms * FF_NS_PER_MS);
    }
    for (uint16_t i = t->count; i < FF_SCHED_MAX_ENTRIES; i++) {
        g_next_due_ns[i] = 0u;
    }
}

uint16_t FF_SchedPoll(uint64_t now_ns, FF_Msg_t *out, uint16_t max_out, FF_AppId_t app)
{
    if ((g_has_active == 0u) || (max_out == 0u)) {
        return 0u;
    }
    const FF_SchedTable_t *t = &g_images[g_active];
    uint16_t emitted = 0u;

    for (uint16_t i = 0u; (i < t->count) && (emitted < max_out); i++) {
        const FF_SchedEntry_t *e = &t->entries[i];
        uint64_t period_ns = (uint64_t)e->period_ms * FF_NS_PER_MS;
        uint64_t due = g_next_due_ns[i];

        if ((due == 0u) || (now_ns < due)) {
            continue;
        }

        /* Absolute release targets: advance by whole periods to the first
         * target strictly after "now". Slots crossed without release are
         * counted, never burst-released (bounded work per poll). */
        uint64_t over = now_ns - due;
        uint64_t skip = over / period_ns;
        if (skip > 0u) {
            g_stats.missed += (uint32_t)skip;
            g_stats.late_polls++;
        }
        g_next_due_ns[i] = due + ((skip + 1u) * period_ns);

        FF_Msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_id = e->msg_id;
        msg.apid = app;
        msg.time_ns = now_ns;
        msg.length = e->payload_len;
        if (e->payload_len > 0u) {
            memcpy(msg.payload, e->payload, e->payload_len);
        }
        if (FF_SbPublish(&msg) != FF_SUCCESS) {
            g_stats.publish_errors++;
        }
        /* dispatched counts release slots consumed, including releases whose
         * publish failed (those are additionally counted in publish_errors), so
         * the schedule accounting stays meaningful without a live bus. */
        g_stats.dispatched++;
        if (out != NULL) {
            out[emitted] = msg;
        }
        emitted++;
    }
    return emitted;
}

void FF_SchedGetStats(FF_SchedStats_t *out)
{
    if (out == NULL) {
        return;
    }
    *out = g_stats;
}

void FF_SchedResetStats(void)
{
    memset(&g_stats, 0, sizeof(g_stats));
}
