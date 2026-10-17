/*
 * frick-fsw - connectivity test for the integrated N7 OBCP engine
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "ff_test.h"

#include <string.h>

#include "core/pal/pal.h"
#include "obcp/ff_obcp.h"

static int32_t g_parameter_value;
static uint32_t g_parameter_id;
static uint32_t g_enum_id;
static int32_t g_enum_value;
static uint32_t g_float_id;
static float g_float_value;
static uint32_t g_bool_id;
static bool g_bool_value;
static uint32_t g_event_id;
static uint32_t g_step_id;
static bool g_step_ended;
static uint32_t g_write_count;
static uint32_t g_time_wait_ms;
static uint32_t g_wait_until_seconds;
static uint32_t g_wait_until_ms;
static size_t g_output_length;

static bool read_int(const uint32_t id, int32_t *value)
{
    if (id != g_parameter_id || value == NULL) {
        return false;
    }
    *value = g_parameter_value;
    return true;
}

static bool write_int(const uint32_t id, const int32_t value)
{
    g_parameter_id = id;
    g_parameter_value = value;
    g_write_count++;
    return true;
}

static bool read_enum(const uint32_t id, int32_t *value)
{
    if (id != g_enum_id || value == NULL) {
        return false;
    }
    *value = g_enum_value;
    return true;
}

static bool write_enum(const uint32_t id, const int32_t value)
{
    g_enum_id = id;
    g_enum_value = value;
    return true;
}

static bool read_float(const uint32_t id, float *value)
{
    if (id != g_float_id || value == NULL) {
        return false;
    }
    *value = g_float_value;
    return true;
}

static bool write_float(const uint32_t id, const float value)
{
    g_float_id = id;
    g_float_value = value;
    return true;
}

static bool read_bool(const uint32_t id, bool *value)
{
    if (id != g_bool_id || value == NULL) {
        return false;
    }
    *value = g_bool_value;
    return true;
}

static bool write_bool(const uint32_t id, const bool value)
{
    g_bool_id = id;
    g_bool_value = value;
    return true;
}

static void send_event(const uint32_t event_id)
{
    g_event_id = event_id;
}

static bool write_text(const char *text, size_t length)
{
    if (text == NULL) {
        return false;
    }
    g_output_length += length;
    return true;
}

static bool begin_step(const uint32_t step_id)
{
    g_step_id = step_id;
    return true;
}

static bool end_step(const uint32_t step_id, const bool success)
{
    g_step_ended = (step_id == g_step_id) && success;
    return g_step_ended;
}

static bool wait_ms(const uint32_t milliseconds)
{
    g_time_wait_ms = milliseconds;
    return true;
}

static bool wait_until(const uint32_t seconds, const uint32_t milliseconds)
{
    g_wait_until_seconds = seconds;
    g_wait_until_ms = milliseconds;
    return true;
}

static bool get_time(uint32_t *seconds, uint32_t *milliseconds)
{
    if (seconds == NULL || milliseconds == NULL) {
        return false;
    }
    *seconds = 12u;
    *milliseconds = 34u;
    return true;
}

static bool packet_available(const uint32_t channel)
{
    return channel == 2u;
}

static bool get_packet_channel(uint32_t *channel)
{
    if (channel == NULL) {
        return false;
    }
    *channel = 2u;
    return true;
}

static bool can_send_packet(const uint32_t channel)
{
    return channel == 2u;
}

static bool send_packet(const uint32_t channel, const uint32_t length, const char *const data)
{
    return channel == 2u && length == 2u && data != NULL && data[0] == 'O' && data[1] == 'K';
}

static bool receive_packet(const uint32_t channel, uint32_t *length, char *data,
                           const uint32_t timeout_ms)
{
    if (channel != 2u || length == NULL || data == NULL || *length < 2u || timeout_ms != 7u) {
        return false;
    }
    data[0] = 'O';
    data[1] = 'K';
    *length = 2u;
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

static void test_script_connects_services_and_checks_input_bounds(void)
{
    static char heap[65536];
    static uint8_t packet_buffer[128];
    const char script[] =
        "import obcpdatapool, obcpevents, obcpio, obcpcontrol, obcptime, obcppackets\n"
        "obcpdatapool.writeintparameter(100, 42)\n"
        "assert obcpdatapool.readintparameter(100) == 42\n"
        "obcpdatapool.writeenumparameter(101, 3)\n"
        "assert obcpdatapool.readenumparameter(101) == 3\n"
        "obcpdatapool.writefloatparameter(102, 1.25)\n"
        "assert obcpdatapool.readfloatparameter(102) == 1.25\n"
        "obcpdatapool.writeboolparameter(103, True)\n"
        "assert obcpdatapool.readboolparameter(103) is True\n"
        "obcpevents.sendevent(513)\n"
        "obcpio.write('connectivity')\n"
        "obcpcontrol.beginstep(9)\n"
        "obcptime.wait(5)\n"
        "obcpcontrol.endstep(9, True)\n"
        "assert obcppackets.ispacketavailable(2)\n"
        "assert obcppackets.getchannelwithpacketavailable() == 2\n"
        "assert obcppackets.cansendpacket(2)\n"
        "obcppackets.sendpacket(2, 'OK')\n"
        "assert obcppackets.receivepacket(2, 7) == b'OK'\n"
        "obcptime.waituntil(20, 10)\n"
        "assert obcptime.gettime() == (12, 34)\n"
        "obcpevents.sendevent(514)\n";
    obcp_engine_context_t context = make_context();
    FF_ObcpConfig_t config = {0};
    FF_ObcpStatus_t status;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    config.services = context;
    config.heap = heap;
    config.heap_size = sizeof(heap);
    config.packet_buffer = packet_buffer;
    config.packet_buffer_size = sizeof(packet_buffer);
    config.services.obcp_send_packet = NULL;
    FF_TEST_ASSERT(FF_ObcpInit(&config) == FF_INVALID_ARGUMENT);
    config.services = context;
    FF_TEST_ASSERT(FF_ObcpInit(&config) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_READY);
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, sizeof(script) - 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_write_count == 1u && g_parameter_id == 100u && g_parameter_value == 42);
    FF_TEST_ASSERT(g_enum_id == 101u && g_enum_value == 3);
    FF_TEST_ASSERT(g_float_id == 102u && g_float_value == 1.25f);
    FF_TEST_ASSERT(g_bool_id == 103u && g_bool_value);
    /* Event 514 is a completion sentinel after the final Python assertion.
     * It supplies an observable oracle despite the upstream Boolean result. */
    FF_TEST_ASSERT(g_event_id == 514u);
    FF_TEST_ASSERT(g_step_id == 9u);
    FF_TEST_ASSERT(g_step_ended);
    FF_TEST_ASSERT(g_time_wait_ms == 5u);
    FF_TEST_ASSERT(g_wait_until_seconds == 20u && g_wait_until_ms == 10u);
    FF_TEST_ASSERT(g_output_length > 0u);
    FF_TEST_ASSERT(FF_ObcpGetStatus(&status) == FF_SUCCESS);
    FF_TEST_ASSERT(status.state == FF_OBCP_STATE_RETURNED);
    FF_TEST_ASSERT(status.engine_reported_success);
    FF_TEST_ASSERT(FF_ObcpExecutePy(NULL, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_ObcpExecutePy(script, FF_OBCP_SCRIPT_MAX_BYTES + 1u) == FF_BAD_SIZE);
    FF_PalShutdown();
}

FF_TEST_MAIN(test_script_connects_services_and_checks_input_bounds)
