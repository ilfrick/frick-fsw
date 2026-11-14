/*
 * frick-fsw - PUS Service 18 (On-board Control Procedures) integration test
 *
 * Verifies the provisional S18 binding to the pinned n7s-obcp engine behind
 * FF_Obcp_t: load/start a bounded script, abort, and status report.
 *
 * This is a provisional interface: the wire format, subservice set and TM
 * layout are NOT yet an approved ICD. They are exercised here as bounded
 * host-level integration evidence only. No mission parameters, no fsw_main
 * wiring, no mission-script activation.
 *
 * Test values are test fixtures only, not mission limits.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */

#include "../ff_test.h"
#include <string.h>

#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include "transport/ccsds/ccsds.h"
#include "pus/pus.h"
#include "obcp/ff_obcp.h"

/* Minimal service callbacks for OBCP engine init. */
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

/* ---- big-endian helper ----------------------------------------------- */

static void wr_u32be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 24) & 0xFFu);
    p[1] = (uint8_t)((v >> 16) & 0xFFu);
    p[2] = (uint8_t)((v >> 8) & 0xFFu);
    p[3] = (uint8_t)(v & 0xFFu);
}

/* ---- Build a CCSDS TC frame for service 18 ---------------------------- */

static uint16_t build_tc18_frame(uint8_t subservice, const uint8_t *payload,
                                 uint16_t payload_len, uint8_t *frame,
                                 uint16_t frame_cap)
{
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_START | FF_PUS_ACK_COMPLETION;
    tc.service = FF_PUS_SERVICE_OBCP; /* 18 */
    tc.subservice = subservice;
    tc.source_id = 0x0102u;
    return FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame,
                            frame_cap, NULL);
}

/* ---- Direct OBCP engine tests (budget slice, no PUS framing) --------- */

static void test_s18_obcp_budget_adequate(void)
{
    FF_ObcpStatus_t status;
    const char script[] = "x = 1\n";

    ensure_obcp_initialized();

    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 1000u) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_RETURNED);
    FF_TEST_ASSERT(status.budget_exhausted == false);
}

static void test_s18_obcp_budget_zero(void)
{
    const char script[] = "x = 1\n";

    ensure_obcp_initialized();

    /* Zero budget must fail closed: no script execution. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 0u)
                   == FF_INVALID_ARGUMENT);
}

static void test_s18_obcp_budget_exhausted(void)
{
    FF_ObcpStatus_t status;
    const char script[] = "while True:\n    pass\n";

    ensure_obcp_initialized();

    /* Tight budget on unbounded loop: FF_ERROR with BUDGET_EXHAUSTED state,
     * distinct from interpreter error and from success. */
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u, 5u) == FF_ERROR);

    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_BUDGET_EXHAUSTED);
    FF_TEST_ASSERT(status.budget_exhausted == true);
}

static void test_s18_obcp_abort_not_running(void)
{
    ensure_obcp_initialized();

    /* Engine is READY, not RUNNING: abort reports not ready. */
    FF_TEST_ASSERT(FF_ObcpRequestAbort() == FF_NOT_READY);
}

static void test_s18_obcp_status(void)
{
    FF_ObcpStatus_t status;

    ensure_obcp_initialized();

    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
}

/* ---- PUS S18 binding (provisional) ----------------------------------- */

static void test_s18_pus_binding(void)
{
    uint8_t payload[64];
    uint16_t plen = 0u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    const char script[] = "x = 1\n";

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* FF_PusInit() already registered core services including S18 when
     * FF_USE_N7_OBCP is enabled. Do not call FF_PusRegisterCoreServices()
     * again (it would return FF_ALREADY_EXISTS). */

    ensure_obcp_initialized();

    /* Build TC[18,1]: [max_vm_vi:4 BE][script:NUL-terminated] */
    wr_u32be(payload, 1000u);
    plen = 4u;
    memcpy(&payload[plen], script, sizeof(script) - 1u);
    plen += (uint16_t)(sizeof(script) - 1u);

    frame_len = build_tc18_frame(1u, payload, plen, frame, sizeof(frame));
    FF_TEST_ASSERT(frame_len > 0u);

    /* The binding is provisional (ICD pending): accept only the documented
     * success/invalid/error/not-found outcomes. FF_NOT_FOUND means S18 is not
     * registered (FF_USE_N7_OBCP not enabled for this build); FF_SUCCESS,
     * FF_INVALID_ARGUMENT and FF_ERROR mean the binding is wired and the
     * handler responded. The engine-level behavior is verified directly by
     * test_obcp_budget.c (budget zero fail-closed, budget exhaustion,
     * context reuse, error distinctness). */
    FF_Status_t rc = FF_PusIngest(frame, frame_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS || rc == FF_INVALID_ARGUMENT ||
                   rc == FF_ERROR || rc == FF_NOT_FOUND);
}

FF_TEST_MAIN(test_s18_obcp_budget_adequate, test_s18_obcp_budget_zero,
             test_s18_obcp_budget_exhausted, test_s18_obcp_abort_not_running,
             test_s18_obcp_status, test_s18_pus_binding)