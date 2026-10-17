/*
 * frick-fsw - N7 Space OBCP engine adapter
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "obcp/ff_obcp.h"

#include <string.h>

#include "core/pal/pal.h"

static FF_MutexId_t g_obcp_mutex = FF_INVALID_ID;
static FF_ObcpState_t g_state = FF_OBCP_STATE_UNINITIALIZED;
static uint32_t g_abort_requested;
static bool g_engine_reported_success;
static bool g_initialized;
static char g_script[FF_OBCP_SCRIPT_MAX_BYTES + 1u];
static char *g_heap;
static size_t g_heap_size;
static uint8_t *g_packet_buffer;
static uint32_t g_packet_buffer_size;
static obcp_engine_context_t g_services;

static void ff_obcp_set_state(FF_ObcpState_t state)
{
    __atomic_store_n(&g_state, state, __ATOMIC_RELEASE);
}

static bool ff_obcp_beginstep(const uint32_t step_id)
{
    if (__atomic_load_n(&g_abort_requested, __ATOMIC_ACQUIRE) != 0u) {
        return false;
    }
    if (g_services.obcp_beginstep == NULL) {
        return false;
    }
    return g_services.obcp_beginstep(step_id);
}

static bool ff_obcp_endstep(const uint32_t step_id, const bool success)
{
    if (g_services.obcp_endstep == NULL) {
        return false;
    }
    return g_services.obcp_endstep(step_id, success);
}

FF_Status_t FF_ObcpInit(const FF_ObcpConfig_t *config)
{
    obcp_engine_context_t context;

    if (config == NULL || config->heap == NULL || config->heap_size == 0u ||
        config->packet_buffer == NULL || config->packet_buffer_size == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    if (config->services.obcp_read_int_parameter == NULL ||
        config->services.obcp_write_int_parameter == NULL ||
        config->services.obcp_read_enum_parameter == NULL ||
        config->services.obcp_write_enum_parameter == NULL ||
        config->services.obcp_read_float_parameter == NULL ||
        config->services.obcp_write_float_parameter == NULL ||
        config->services.obcp_read_bool_parameter == NULL ||
        config->services.obcp_write_bool_parameter == NULL ||
        config->services.obcp_send_event == NULL || config->services.obcp_write == NULL ||
        config->services.obcp_beginstep == NULL || config->services.obcp_endstep == NULL ||
        config->services.obcp_wait == NULL || config->services.obcp_waituntil == NULL ||
        config->services.obcp_gettime == NULL ||
        config->services.obcp_is_packet_available == NULL ||
        config->services.obcp_get_channel_with_packet_available == NULL ||
        config->services.obcp_can_send_packet == NULL ||
        config->services.obcp_send_packet == NULL || config->services.obcp_receive_packet == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_initialized) {
        return FF_ALREADY_EXISTS;
    }

    context = config->services;
    g_services = config->services;
    context.obcp_beginstep = ff_obcp_beginstep;
    context.obcp_endstep = ff_obcp_endstep;

    if (FF_PalMutexCreate(&g_obcp_mutex, "obcp") != FF_SUCCESS) {
        return FF_NO_RESOURCE;
    }
    if (!obcpengine_init(&context)) {
        (void)FF_PalMutexDelete(g_obcp_mutex);
        g_obcp_mutex = FF_INVALID_ID;
        return FF_ERROR;
    }
    g_heap = config->heap;
    g_heap_size = config->heap_size;
    g_packet_buffer = config->packet_buffer;
    g_packet_buffer_size = config->packet_buffer_size;
    __atomic_store_n(&g_abort_requested, 0u, __ATOMIC_RELEASE);
    __atomic_store_n(&g_engine_reported_success, false, __ATOMIC_RELEASE);
    ff_obcp_set_state(FF_OBCP_STATE_READY);
    __atomic_store_n(&g_initialized, true, __ATOMIC_RELEASE);
    return FF_SUCCESS;
}

FF_Status_t FF_ObcpExecutePy(const char *script, size_t script_len)
{
    size_t i;
    bool engine_result;
    FF_Status_t lock_status;

    if (!__atomic_load_n(&g_initialized, __ATOMIC_ACQUIRE) || script == NULL || script_len == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    if (script_len > FF_OBCP_SCRIPT_MAX_BYTES) {
        return FF_BAD_SIZE;
    }
    for (i = 0u; i < script_len; ++i) {
        if (script[i] == '\0') {
            return FF_INVALID_ARGUMENT;
        }
    }
    lock_status = FF_PalMutexTryLock(g_obcp_mutex);
    if (lock_status != FF_SUCCESS) {
        return FF_BUSY;
    }

    memcpy(g_script, script, script_len);
    g_script[script_len] = '\0';
    __atomic_store_n(&g_abort_requested, 0u, __ATOMIC_RELEASE);
    __atomic_store_n(&g_engine_reported_success, false, __ATOMIC_RELEASE);
    ff_obcp_set_state(FF_OBCP_STATE_RUNNING);

    if (!obcpengine_provide_buffer(g_packet_buffer, g_packet_buffer_size)) {
        ff_obcp_set_state(FF_OBCP_STATE_ERROR);
        (void)FF_PalMutexUnlock(g_obcp_mutex);
        return FF_ERROR;
    }

    engine_result = obcpengine_execute_py(g_script, g_heap, g_heap_size);
    __atomic_store_n(&g_engine_reported_success, engine_result, __ATOMIC_RELEASE);
    ff_obcp_set_state((__atomic_load_n(&g_abort_requested, __ATOMIC_ACQUIRE) != 0u)
                          ? FF_OBCP_STATE_ABORT_REQUESTED
                          : (engine_result ? FF_OBCP_STATE_RETURNED : FF_OBCP_STATE_ERROR));
    (void)FF_PalMutexUnlock(g_obcp_mutex);

    /* At the pinned upstream revision this API returns true after the VM call
     * without propagating interpreter exceptions. The status is therefore
     * execution-returned, not proof that the procedure's actions succeeded. */
    return engine_result ? FF_SUCCESS : FF_ERROR;
}

FF_Status_t FF_ObcpRequestAbort(void)
{
    if (!__atomic_load_n(&g_initialized, __ATOMIC_ACQUIRE)) {
        return FF_NOT_READY;
    }
    if (__atomic_load_n(&g_state, __ATOMIC_ACQUIRE) != FF_OBCP_STATE_RUNNING) {
        return FF_NOT_READY;
    }
    __atomic_store_n(&g_abort_requested, 1u, __ATOMIC_RELEASE);
    return FF_SUCCESS;
}

FF_Status_t FF_ObcpGetStatus(FF_ObcpStatus_t *status)
{
    if (!__atomic_load_n(&g_initialized, __ATOMIC_ACQUIRE) || status == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    status->state = __atomic_load_n(&g_state, __ATOMIC_ACQUIRE);
    status->engine_reported_success = __atomic_load_n(&g_engine_reported_success, __ATOMIC_ACQUIRE);
    status->abort_requested = (__atomic_load_n(&g_abort_requested, __ATOMIC_ACQUIRE) != 0u);
    return FF_SUCCESS;
}
