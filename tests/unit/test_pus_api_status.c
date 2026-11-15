/*
 * frick-fsw - NC-EXT-251 TDD RED test: status-return API for FF_PusSetTmSink
 * and FF_PusInstallPanicReporter
 *
 * Tests that the API returns FF_Status_t (not void) so callers can detect
 * initialization/registration failures. This test MUST compile and pass once
 * the implementation signatures match the header.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include "pus/pus.h"

/* Simple TM sink */
static void dummy_tm_sink(const uint8_t *packet, uint16_t len)
{
    (void)packet;
    (void)len;
}

/* Simple direct writer for panic reporter */
static FF_Status_t dummy_direct_writer(const uint8_t *frame, uint16_t len, uint64_t deadline_ns)
{
    (void)frame;
    (void)len;
    (void)deadline_ns;
    return FF_SUCCESS;
}

static void test_pus_set_tm_sink_returns_status(void)
{
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* FF_PusSetTmSink must return FF_Status_t (not void) */
    rc = FF_PusSetTmSink(dummy_tm_sink);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* Calling with NULL should also return a status */
    rc = FF_PusSetTmSink(NULL);
    FF_TEST_ASSERT(rc == FF_SUCCESS); /* NULL is allowed */

    FF_TEST_PRINT_SUMMARY("test_pus_set_tm_sink_returns_status");
}

static void test_pus_install_panic_reporter_returns_status(void)
{
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* FF_PusInstallPanicReporter must return FF_Status_t (not void) */
    rc = FF_PusInstallPanicReporter(dummy_direct_writer);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* Calling with NULL should return an error status */
    rc = FF_PusInstallPanicReporter(NULL);
    FF_TEST_ASSERT(rc != FF_SUCCESS); /* NULL not allowed */

    FF_TEST_PRINT_SUMMARY("test_pus_install_panic_reporter_returns_status");
}

FF_TEST_MAIN(test_pus_set_tm_sink_returns_status, test_pus_install_panic_reporter_returns_status)