/*
 * frick-fsw - OBCP VM instruction budget test (M12/S9)
 *
 * Verifies a nonzero, caller-supplied per-execution VM-instruction budget
 * enforced in the interpreter dispatch path: budget exhaustion yields a
 * distinct result, and the same engine context can execute a subsequent
 * bounded script without stale budget/exception state.
 *
 * Test-first slice: this test references the budget-aware
 * FF_ObcpExecutePy(script, len, max_vm_instructions) API and the
 * FF_OBCP_STATE_BUDGET_EXHAUSTED state that the implementation must add.
 * It is expected to fail to compile / fail at runtime before the
 * implementation exists (RED), then pass after implementation (GREEN).
 *
 * Test values are test fixtures only, not mission limits.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */

#include "ff_test.h"
#include <string.h>

#include "core/pal/pal.h"
#include "obcp/ff_obcp.h"

/* Minimal service callbacks -- all 20 required by FF_ObcpInit. */
static bool read_int(const uint32_t id, int32_t *value)
{
    (void)id;
    (void)value;
    return true;
}

static bool write_int(const uint32_t id, const int32_t value)
{
    (void)id;
    (void)value;
    return true;
}

static bool read_enum(const uint32_t id, int32_t *value)
{
    (void)id;
    (void)value;
    return true;
}

static bool write_enum(const uint32_t id, const int32_t value)
{
    (void)id;
    (void)value;
    return true;
}

static bool read_float(const uint32_t id, float *value)
{
    (void)id;
    (void)value;
    return true;
}

static bool write_float(const uint32_t id, const float value)
{
    (void)id;
    (void)value;
    return true;
}

static bool read_bool(const uint32_t id, bool *value)
{
    (void)id;
    (void)value;
    return true;
}

static bool write_bool(const uint32_t id, const bool value)
{
    (void)id;
    (void)value;
    return true;
}

static void send_event(const uint32_t event_id)
{
    (void)event_id;
}

static bool write_text(const char *text, size_t length)
{
    (void)text;
    (void)length;
    return true;
}

static bool begin_step(const uint32_t step_id)
{
    (void)step_id;
    return true;
}

static bool end_step(const uint32_t step_id, const bool success)
{
    (void)step_id;
    (void)success;
    return true;
}

static bool wait_ms(const uint32_t milliseconds)
{
    (void)milliseconds;
    return true;
}

static bool wait_until(const uint32_t seconds, const uint32_t milliseconds)
{
    (void)seconds;
    (void)milliseconds;
    return true;
}

static bool get_time(uint32_t *seconds, uint32_t *milliseconds)
{
    *seconds = 0u;
    *milliseconds = 0u;
    return true;
}

static bool packet_available(const uint32_t channel)
{
    (void)channel;
    return true;
}

static bool get_packet_channel(uint32_t *channel)
{
    *channel = 0u;
    return true;
}

static bool can_send_packet(const uint32_t channel)
{
    (void)channel;
    return true;
}

static bool send_packet(const uint32_t channel, const uint32_t length,
                        const char *const data)
{
    (void)channel;
    (void)length;
    (void)data;
    return true;
}

static bool receive_packet(const uint32_t channel, uint32_t *length, char *data,
                           const uint32_t timeout_ms)
{
    (void)channel;
    (void)length;
    (void)data;
    (void)timeout_ms;
    return true;
}

static obcp_engine_context_t make_context(void)
{
    obcp_engine_context_t context;
    (void)memset(&context, 0, sizeof(context));
    context.obcp_read_int_parameter = read_int;
    context.obcp_write_int_parameter = write_int;
    context.obcp_read_enum_parameter = read_enum;
    context.obcp_write_enum_parameter = write_enum;
    context.obcp_read_float_parameter = read_float;
    context.obcp_write_float_parameter = write_float;
    context.obcp_read_bool_parameter = read_bool;
    context.obcp_write_bool_parameter = write_bool;
    context.obcp_send_event = send_event;
    context.obcp_write = write_text;
    context.obcp_beginstep = begin_step;
    context.obcp_endstep = end_step;
    context.obcp_wait = wait_ms;
    context.obcp_waituntil = wait_until;
    context.obcp_gettime = get_time;
    context.obcp_is_packet_available = packet_available;
    context.obcp_get_channel_with_packet_available = get_packet_channel;
    context.obcp_can_send_packet = can_send_packet;
    context.obcp_send_packet = send_packet;
    context.obcp_receive_packet = receive_packet;
    return context;
}

static void obcp_config_ready(FF_ObcpConfig_t *config,
                              obcp_engine_context_t context)
{
    static char heap[4096];
    static uint8_t packet_buffer[64];
    config->services = context;
    config->heap = heap;
    config->heap_size = sizeof(heap);
    config->packet_buffer = packet_buffer;
    config->packet_buffer_size = sizeof(packet_buffer);
}

/* Ensure the engine is initialized (idempotent: tolerates FF_ALREADY_EXISTS
 * from a prior test case, since g_initialized is process-static). */
static void ensure_obcp_initialized(void)
{
    static bool initialized = false;
    if (!initialized) {
        FF_ObcpConfig_t config;
        obcp_engine_context_t context = make_context();
        obcp_config_ready(&config, context);
        FF_Status_t st = FF_ObcpInit(&config);
        FF_TEST_ASSERT(st == FF_SUCCESS || st == FF_ALREADY_EXISTS);
        initialized = true;
    }
}

/* A zero budget must fail closed: no execution path may run without a
 * nonzero budget. */
static void test_budget_zero_rejected(void)
{
    ensure_obcp_initialized();

    FF_TEST_ASSERT(FF_ObcpExecutePy("x = 1", 5u, 0u) == FF_INVALID_ARGUMENT);
}

/* A simple bounded script with adequate budget must complete successfully. */
static void test_budget_adequate_succeeds(void)
{
    FF_ObcpStatus_t status;
    const char script[] = "x = 1\n";

    ensure_obcp_initialized();

    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 1000u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_RETURNED);
    FF_TEST_ASSERT(status.budget_exhausted == false);
}

/* An unbounded loop with a tight budget must exhaust the budget and produce
 * a distinct budget-exhausted result (not success, not plain error). */
static void test_budget_exhausted(void)
{
    FF_ObcpStatus_t status;
    const char script[] = "while True:\n    pass\n";

    ensure_obcp_initialized();

    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 10u) == FF_ERROR);

    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_BUDGET_EXHAUSTED);
    FF_TEST_ASSERT(status.budget_exhausted == true);
}

/* An interpreter error (e.g. undefined name) must yield FF_OBCP_STATE_ERROR,
 * not FF_OBCP_STATE_BUDGET_EXHAUSTED, proving distinct error classification. */
static void test_budget_error_distinct_from_exhaustion(void)
{
    FF_ObcpStatus_t status;
    const char script[] = "undefined_name + 1\n";

    ensure_obcp_initialized();

    /* Interpreter error with adequate budget: FF_ERROR, state ERROR,
     * budget_exhausted false. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 1000u) == FF_ERROR);

    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_ERROR);
    FF_TEST_ASSERT(status.budget_exhausted == false);
}

/* After budget exhaustion, the same engine context must be reusable for a
 * subsequent bounded script without stale abort/exception/budget state. */
static void test_context_reuse_after_budget_exhaustion(void)
{
    FF_ObcpStatus_t status;
    const char script[] = "while True:\n    pass\n";
    const char benign[] = "y = 2\n";

    ensure_obcp_initialized();

    /* First: exhaust the budget. FF_ERROR with BUDGET_EXHAUSTED. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 5u) == FF_ERROR);
    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_BUDGET_EXHAUSTED);
    FF_TEST_ASSERT(status.budget_exhausted == true);

    /* Then: a benign script must run cleanly on the same context. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(benign, sizeof(benign) - 1u, 1000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_RETURNED);
    FF_TEST_ASSERT(status.budget_exhausted == false);
}

/* After an interpreter error (not budget exhaustion), the same engine
 * context must be reusable for a subsequent bounded script. This exercises
 * the cleanup path: mp_embed_deinit() and thread-local reset must run on the
 * exception branch, otherwise stale embed state can corrupt the next run. */
static void test_context_reuse_after_interpreter_error(void)
{
    FF_ObcpStatus_t status;
    const char err_script[] = "undefined_name + 1\n";
    const char benign[] = "y = 2\n";

    ensure_obcp_initialized();

    /* First: an interpreter error with adequate budget. FF_ERROR, state ERROR,
     * budget_exhausted false. This must NOT be classified as budget exhaustion. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(err_script, sizeof(err_script) - 1u, 1000u) == FF_ERROR);
    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_ERROR);
    FF_TEST_ASSERT(status.budget_exhausted == false);

    /* Then: a benign script must run cleanly on the same context. If the
     * exception branch did not call mp_embed_deinit() / reset thread-local,
     * this execution may fail or return a stale state. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(benign, sizeof(benign) - 1u, 1000u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_RETURNED);
    FF_TEST_ASSERT(status.budget_exhausted == false);
}

FF_TEST_MAIN(test_budget_zero_rejected, test_budget_adequate_succeeds,
             test_budget_exhausted, test_budget_error_distinct_from_exhaustion,
             test_context_reuse_after_budget_exhaustion,
             test_context_reuse_after_interpreter_error)
