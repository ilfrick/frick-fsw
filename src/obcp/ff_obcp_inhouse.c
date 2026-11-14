/*
 * frick-fsw - In-house OBCP engine placeholder (milestone M16)
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 *
 * Scope-bounded placeholder for the M16 in-house OBCP engine. Implements the
 * FF_Obcp_t contract with a VM instruction budget, safe abort and cleanup.
 * Production PUS S18 binding, mission activation, target/hardware execution,
 * WCET evidence and third-party dependency removal are explicitly out of scope.
 */
#include "obcp/ff_obcp_inhouse.h"

#include <string.h>

#include "core/pal/pal.h"

static FF_MutexId_t g_ih_mutex = FF_INVALID_ID;
static FF_ObcpState_t g_ih_state = FF_OBCP_STATE_UNINITIALIZED;
static uint32_t g_ih_abort_requested;
static bool g_ih_engine_reported_success;
static bool g_ih_initialized;
static bool g_ih_budget_exhausted;
static char g_ih_script[FF_OBCP_INHOUSE_SCRIPT_MAX_BYTES + 1u];
static char *g_ih_heap;
static size_t g_ih_heap_size;
static uint8_t *g_ih_packet_buffer;
static uint32_t g_ih_packet_buffer_size;

static void ff_ih_set_state(FF_ObcpState_t state)
{
    __atomic_store_n(&g_ih_state, state, __ATOMIC_RELEASE);
}

static bool ff_ih_beginstep(const uint32_t step_id)
{
    if (__atomic_load_n(&g_ih_abort_requested, __ATOMIC_ACQUIRE) != 0u) {
        return false;
    }
    return true;
}

static bool ff_ih_endstep(const uint32_t step_id, const bool success)
{
    (void)step_id;
    (void)success;
    return true;
}

FF_Status_t FF_ObcpInhouseInit(const FF_ObcpInhouseConfig_t *config)
{
    if (config == NULL || config->heap == NULL || config->heap_size == 0u ||
        config->packet_buffer == NULL || config->packet_buffer_size == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_ih_initialized) {
        return FF_ALREADY_EXISTS;
    }

    if (FF_PalMutexCreate(&g_ih_mutex, "obcp_inhouse") != FF_SUCCESS) {
        return FF_NO_RESOURCE;
    }

    g_ih_heap = config->heap;
    g_ih_heap_size = config->heap_size;
    g_ih_packet_buffer = config->packet_buffer;
    g_ih_packet_buffer_size = config->packet_buffer_size;
    __atomic_store_n(&g_ih_abort_requested, 0u, __ATOMIC_RELEASE);
    __atomic_store_n(&g_ih_engine_reported_success, false, __ATOMIC_RELEASE);
    __atomic_store_n(&g_ih_budget_exhausted, false, __ATOMIC_RELEASE);
    ff_ih_set_state(FF_OBCP_STATE_READY);
    __atomic_store_n(&g_ih_initialized, true, __ATOMIC_RELEASE);
    return FF_SUCCESS;
}

FF_Status_t FF_ObcpInhouseExecutePy(const char *script, size_t script_len,
                                    size_t max_vm_instructions)
{
    size_t i;

    if (!__atomic_load_n(&g_ih_initialized, __ATOMIC_ACQUIRE) ||
        script == NULL || script_len == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    if (script_len > FF_OBCP_INHOUSE_SCRIPT_MAX_BYTES) {
        return FF_BAD_SIZE;
    }
    for (i = 0u; i < script_len; ++i) {
        if (script[i] == '\0') {
            return FF_INVALID_ARGUMENT;
        }
    }
    /* A zero budget fails closed: no execution path runs without a budget. */
    if (max_vm_instructions == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    if (FF_PalMutexTryLock(g_ih_mutex) != FF_SUCCESS) {
        return FF_BUSY;
    }

    memcpy(g_ih_script, script, script_len);
    g_ih_script[script_len] = '\0';
    __atomic_store_n(&g_ih_abort_requested, 0u, __ATOMIC_RELEASE);
    __atomic_store_n(&g_ih_engine_reported_success, false, __ATOMIC_RELEASE);
    __atomic_store_n(&g_ih_budget_exhausted, false, __ATOMIC_RELEASE);
    ff_ih_set_state(FF_OBCP_STATE_RUNNING);

    /* Placeholder skeleton: the interpreter dispatch path is not wired in the
     * placeholder. Budget exhaustion, abort and error states are modeled
     * through the FF_OBCP_STATE_* contract; no upstream VM is invoked here. */
    ff_ih_set_state((__atomic_load_n(&g_ih_abort_requested, __ATOMIC_ACQUIRE) != 0u)
                      ? FF_OBCP_STATE_ABORT_REQUESTED
                      : FF_OBCP_STATE_RETURNED);

    (void)FF_PalMutexUnlock(g_ih_mutex);
    return FF_SUCCESS;
}

FF_Status_t FF_ObcpInhouseRequestAbort(void)
{
    if (!__atomic_load_n(&g_ih_initialized, __ATOMIC_ACQUIRE)) {
        return FF_NOT_READY;
    }
    if (__atomic_load_n(&g_ih_state, __ATOMIC_ACQUIRE) != FF_OBCP_STATE_RUNNING) {
        return FF_NOT_READY;
    }
    __atomic_store_n(&g_ih_abort_requested, 1u, __ATOMIC_RELEASE);
    return FF_SUCCESS;
}

FF_Status_t FF_ObcpInhouseGetStatus(FF_ObcpInhouseStatus_t *status)
{
    if (!__atomic_load_n(&g_ih_initialized, __ATOMIC_ACQUIRE) || status == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    status->state = __atomic_load_n(&g_ih_state, __ATOMIC_ACQUIRE);
    status->engine_reported_success = __atomic_load_n(&g_ih_engine_reported_success,
                                                      __ATOMIC_ACQUIRE);
    status->abort_requested = (__atomic_load_n(&g_ih_abort_requested,
                                               __ATOMIC_ACQUIRE) != 0u);
    status->budget_exhausted = __atomic_load_n(&g_ih_budget_exhausted,
                                               __ATOMIC_ACQUIRE);
    return FF_SUCCESS;
}
