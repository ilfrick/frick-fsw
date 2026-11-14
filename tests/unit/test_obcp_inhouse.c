/*
 * frick-fsw - In-house OBCP engine placeholder test (milestone M16)
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 *
 * Scope-bounded test for the M16 in-house OBCP engine placeholder:
 * nominal execution, zero-budget fail-closed, abort request, out-of-bound
 * arguments. Explicit expected results per norm N22.
 */
#include "ff_test.h"
#include <string.h>

#include "obcp/ff_obcp_inhouse.h"

static char g_heap[4096];
static uint8_t g_packet_buffer[256];

/* Nominal execution: state RETURNED, budget not exhausted, no abort. */
static void test_nominal_execution(void)
{
    FF_ObcpInhouseConfig_t cfg = {
        .heap = g_heap,
        .heap_size = sizeof(g_heap),
        .packet_buffer = g_packet_buffer,
        .packet_buffer_size = sizeof(g_packet_buffer),
    };
    FF_ObcpInhouseStatus_t st;
    const char *script = "print(1)";
    FF_Status_t rc;

    rc = FF_ObcpInhouseInit(&cfg);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    rc = FF_ObcpInhouseExecutePy(script, strlen(script), 100u);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    rc = FF_ObcpInhouseGetStatus(&st);
    FF_TEST_ASSERT(rc == FF_SUCCESS);
    FF_TEST_ASSERT(st.state == FF_OBCP_STATE_RETURNED);
    FF_TEST_ASSERT(st.budget_exhausted == false);
    FF_TEST_ASSERT(st.abort_requested == false);
}

/* Zero budget fails closed: no execution path runs without a budget. */
static void test_zero_budget_fails_closed(void)
{
    FF_ObcpInhouseConfig_t cfg = {
        .heap = g_heap,
        .heap_size = sizeof(g_heap),
        .packet_buffer = g_packet_buffer,
        .packet_buffer_size = sizeof(g_packet_buffer),
    };
    const char *script = "print(1)";
    FF_Status_t rc;

    rc = FF_ObcpInhouseInit(&cfg);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    rc = FF_ObcpInhouseExecutePy(script, strlen(script), 0u);
    FF_TEST_ASSERT(rc == FF_INVALID_ARGUMENT);
}

/* NULL script fails. */
static void test_null_script_fails(void)
{
    FF_ObcpInhouseConfig_t cfg = {
        .heap = g_heap,
        .heap_size = sizeof(g_heap),
        .packet_buffer = g_packet_buffer,
        .packet_buffer_size = sizeof(g_packet_buffer),
    };
    FF_Status_t rc;

    rc = FF_ObcpInhouseInit(&cfg);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    rc = FF_ObcpInhouseExecutePy(NULL, 5u, 100u);
    FF_TEST_ASSERT(rc == FF_INVALID_ARGUMENT);
}

/* Abort request on non-running engine returns FF_NOT_READY. */
static void test_abort_request(void)
{
    FF_ObcpInhouseConfig_t cfg = {
        .heap = g_heap,
        .heap_size = sizeof(g_heap),
        .packet_buffer = g_packet_buffer,
        .packet_buffer_size = sizeof(g_packet_buffer),
    };
    FF_ObcpInhouseStatus_t st;
    const char *script = "print(1)";
    FF_Status_t rc;

    rc = FF_ObcpInhouseInit(&cfg);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    rc = FF_ObcpInhouseExecutePy(script, strlen(script), 100u);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    rc = FF_ObcpInhouseRequestAbort();
    FF_TEST_ASSERT(rc == FF_NOT_READY);

    rc = FF_ObcpInhouseGetStatus(&st);
    FF_TEST_ASSERT(rc == FF_SUCCESS);
    FF_TEST_ASSERT(st.abort_requested == false);
}

/* Get status on uninitialized engine fails. */
static void test_uninitialized_get_status_fails(void)
{
    FF_ObcpInhouseStatus_t st;
    FF_Status_t rc;

    rc = FF_ObcpInhouseGetStatus(&st);
    FF_TEST_ASSERT(rc == FF_INVALID_ARGUMENT);
}

FF_TEST_MAIN(test_nominal_execution, test_zero_budget_fails_closed,
             test_null_script_fails, test_abort_request,
             test_uninitialized_get_status_fails)
