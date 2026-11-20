/*
 * frick-fsw - NC-EXT-251 TDD RED test: bounded pre-acceptance ACK reservation
 *
 * Tests the SRS-REQ-006 and owner-approved §14.4 semantics:
 * 1. Reserve every requested S1 ACK before acceptance/mutation
 * 2. No missing ACK after acceptance
 * 3. Try-only state/dispatch/emission locks
 * 4. Contention => FF_BUSY, other PAL errors distinct
 * 5. Mutually exclusive accepted sink calls
 * 6. Handler TM allowed
 * 7. Nested ingest/sink-recursive TM => FF_BUSY
 * 8. No state lock across callbacks
 * 9. Serialized full dispatch
 * 10. Direct TM path bounded/lock-free with shared atomic modulo-16384 counter
 * 11. One modulo-16384 sequence shared by the normal and direct paths
 * 12. Dispatch serialized even when no verification ACK is requested
 * 13. No counter value consumed for a frame that is never transmitted
 *
 * Cases 1-10 were the original RED phase for this correction; cases 11-13 were
 * added as RED for the defects the 2026-11-17 R1/R3 reviews found in the first
 * correction (counter paths not equivalent; reservation armed only for TCs that
 * request the acceptance ACK; a counter value consumed for a frame that was
 * never transmitted). Retained evidence: docs/vv/evidence/
 * 2026-11-17-m9-m11-renode-hal/tdd-nc-ext-251/.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include "transport/ccsds/ccsds.h"
#include "pus/pus.h"
#include <string.h>
#include <stdio.h>

/* ---- Forward declarations ------------------------------------------- */

static void blocking_task_fn(void *arg);
static FF_Status_t counter_direct_writer(const uint8_t *frame, uint16_t len, uint64_t deadline_ns);

/* ---- Test infrastructure ---------------------------------------------- */

static uint8_t g_rx_buf[8 * 256];
static uint16_t g_rx_count;
static volatile bool g_sink_blocked = false;
static volatile bool g_sink_first_call = true;
static volatile bool g_tc1_in_acceptance = false;
static FF_TaskId_t g_block_task_id;
static FF_MutexId_t g_sink_mutex;
static uint16_t g_handler_tm_count = 0;
static volatile bool g_nested_ingest_blocked = false;

/* TM sink that blocks on first call only (simulating slow ACK emission) */
static void blocking_tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }

    /* Block on first call only to simulate slow ACK emission */
    if (g_sink_first_call) {
        g_sink_first_call = false;
        g_sink_blocked = true;
        /* Signal that TC1 is in acceptance phase */
        g_tc1_in_acceptance = true;
        /* Wait until unblocked */
        while (g_sink_blocked) {
            FF_PalTaskDelayMs(1);
        }
    }
}

/* TM sink for nested ingest test */
static void nested_tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }
}

/* TM sink that re-enters FF_PusSendTm (sink-recursive TM emission).
 * The core marks the emission in progress with an atomic guard for the whole
 * frame-composition-plus-handover operation, so a recursive emission must be
 * refused with FF_BUSY rather than nesting the emission path (case 7 of the
 * NC-EXT-251 contract, SRS-REQ-006). */
static volatile FF_Status_t g_sink_recursive_rc = FF_SUCCESS;
static volatile bool g_sink_recursed = false;

static void recursive_tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }
    if (!g_sink_recursed) {
        g_sink_recursed = true;
        g_sink_recursive_rc = FF_PusSendTm(0x15, 0x02, (uint8_t[]){0xBE, 0xEF}, 2);
    }
}

/* Simple handler that does nothing */
static FF_Status_t simple_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *data, uint16_t len)
{
    (void)tc;
    (void)data;
    (void)len;
    return FF_SUCCESS;
}

/* Handler that emits TM */
static FF_Status_t tm_emitting_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *data, uint16_t len)
{
    (void)tc;
    (void)data;
    (void)len;
    g_handler_tm_count++;
    FF_PusSendTm(0x15, 0x01, (uint8_t[]){0xDE, 0xAD}, 2);
    return FF_SUCCESS;
}

/* Handler that attempts nested ingest */
static FF_Status_t nested_ingest_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *data,
                                         uint16_t len)
{
    (void)tc;
    (void)data;
    (void)len;
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Pus_TcHdr_t tc_nested;
    memset(&tc_nested, 0, sizeof(tc_nested));
    tc_nested.version = FF_PUS_VERSION;
    tc_nested.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc_nested.service = 21;
    tc_nested.subservice = 3;
    tc_nested.source_id = 0x0200;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc_nested, (uint8_t[]){0x00}, 1, tc_frame, 256,
                                    &tc_len) == FF_SUCCESS);
    FF_Status_t rc = FF_PusIngest(tc_frame, tc_len);
    g_nested_ingest_blocked = (rc == FF_BUSY);
    return FF_SUCCESS;
}

/* Handler that fails */
static FF_Status_t failing_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *data, uint16_t len)
{
    (void)tc;
    (void)data;
    (void)len;
    return FF_ERROR;
}

static void build_tc_full_ack(uint8_t service, uint8_t subservice, uint16_t source_id,
                              const uint8_t *payload, uint16_t payload_len, uint8_t *frame,
                              uint16_t *frame_len)
{
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags =
        FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_START | FF_PUS_ACK_PROGRESS | FF_PUS_ACK_COMPLETION;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = source_id;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, 256, frame_len) ==
                   FF_SUCCESS);
}

static void build_tc_accept_only(uint8_t service, uint8_t subservice, uint16_t source_id,
                                 const uint8_t *payload, uint16_t payload_len, uint8_t *frame,
                                 uint16_t *frame_len)
{
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = source_id;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, 256, frame_len) ==
                   FF_SUCCESS);
}

static void build_tc_accept_complete(uint8_t service, uint8_t subservice, uint16_t source_id,
                                     const uint8_t *payload, uint16_t payload_len, uint8_t *frame,
                                     uint16_t *frame_len)
{
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = source_id;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, 256, frame_len) ==
                   FF_SUCCESS);
}

static void build_tc_no_ack(uint8_t service, uint8_t subservice, uint16_t source_id,
                            const uint8_t *payload, uint16_t payload_len, uint8_t *frame,
                            uint16_t *frame_len)
{
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = 0;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = source_id;

    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, 256, frame_len) ==
                   FF_SUCCESS);
}

static uint16_t stream_count(uint8_t service, uint8_t subservice)
{
    uint16_t pos = 0u;
    uint16_t count = 0u;
    while ((pos + 9u) <= g_rx_count) {
        if (g_rx_buf[pos + 7] == service && g_rx_buf[pos + 8] == subservice) {
            count++;
        }
        if ((pos + 6u) > g_rx_count)
            break;
        uint16_t pdl = (uint16_t)(((uint16_t)g_rx_buf[pos + 4] << 8) | g_rx_buf[pos + 5]);
        pos = (uint16_t)(pos + 6u + pdl + 1u);
    }
    return count;
}

static void reset_capture(void)
{
    g_rx_count = 0u;
    memset(g_rx_buf, 0, sizeof(g_rx_buf));
    g_sink_blocked = false;
    g_sink_first_call = true;
    g_tc1_in_acceptance = false;
    g_handler_tm_count = 0;
    g_nested_ingest_blocked = false;
}

/* ---- Test 1: Pre-acceptance ACK reservation --------------------------- */

static void test_pre_acceptance_ack_reservation(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 1, simple_handler) == FF_SUCCESS);
    FF_PusSetTmSink(blocking_tm_sink);

    FF_TEST_ASSERT(FF_PalMutexCreate(&g_sink_mutex, "sink_mtx") == FF_SUCCESS);

    reset_capture();

    /* Build TC with all ACK flags */
    build_tc_full_ack(21, 1, 0x0102, (uint8_t[]){0x01, 0x02, 0x03, 0x04}, 4, tc_frame, &tc_len);

    /* Spawn task that ingests TC - will block in sink during first ACK emission */
    FF_TEST_ASSERT(FF_PalTaskCreate(&g_block_task_id, "tc1", blocking_task_fn, tc_frame, 4096,
                                    10) == FF_SUCCESS);

    /* Wait for TC to enter acceptance phase (sink blocked) */
    uint32_t wait_cycles = 0;
    while (!g_tc1_in_acceptance && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    FF_TEST_ASSERT(g_tc1_in_acceptance);

    /* At this point, the acceptance ACK should have been emitted but the
     * sink is blocked. The dispatch reservation should be held.
     * Verify that the acceptance ACK was emitted (service 1, subservice 1). */
    FF_TEST_ASSERT(stream_count(1, 1) == 1);

    /* Unblock the sink to let TC complete */
    g_sink_blocked = false;
    FF_PalTaskDelayMs(500);

    /* Verify all four ACKs were emitted */
    FF_TEST_ASSERT(stream_count(1, 1) == 1); /* acceptance success */
    FF_TEST_ASSERT(stream_count(1, 3) == 1); /* start success */
    FF_TEST_ASSERT(stream_count(1, 5) == 1); /* progress success */
    FF_TEST_ASSERT(stream_count(1, 7) == 1); /* completion success */

    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    FF_PalMutexDelete(g_sink_mutex);

    FF_TEST_PRINT_SUMMARY("test_pre_acceptance_ack_reservation");
}

/* ---- Test 2: No missing ACK after acceptance -------------------------- */

static void test_no_missing_ack_after_acceptance(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 2, simple_handler) == FF_SUCCESS);
    FF_PusSetTmSink(blocking_tm_sink);

    FF_TEST_ASSERT(FF_PalMutexCreate(&g_sink_mutex, "sink_mtx") == FF_SUCCESS);

    reset_capture();

    /* Build TC with acceptance and completion ACK only */
    build_tc_accept_complete(21, 2, 0x0103, (uint8_t[]){0xAA, 0xBB}, 2, tc_frame, &tc_len);

    /* Spawn task */
    FF_TEST_ASSERT(FF_PalTaskCreate(&g_block_task_id, "tc2", blocking_task_fn, tc_frame, 4096,
                                    10) == FF_SUCCESS);

    /* Wait for acceptance phase */
    uint32_t wait_cycles = 0;
    while (!g_tc1_in_acceptance && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    FF_TEST_ASSERT(g_tc1_in_acceptance);

    /* Acceptance ACK should be emitted */
    FF_TEST_ASSERT(stream_count(1, 1) == 1);

    /* Unblock */
    g_sink_blocked = false;
    FF_PalTaskDelayMs(500);

    /* Completion ACK must also be emitted (no missing ACK after acceptance) */
    FF_TEST_ASSERT(stream_count(1, 7) == 1);

    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    FF_PalMutexDelete(g_sink_mutex);

    FF_TEST_PRINT_SUMMARY("test_no_missing_ack_after_acceptance");
}

/* ---- Test 3: Try-only locks - contention returns FF_BUSY -------------- */

static void test_try_only_locks_busy(void)
{
    uint8_t tc_frame1[256], tc_frame2[256];
    uint16_t tc_len1, tc_len2;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 3, simple_handler) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 4, simple_handler) == FF_SUCCESS);
    FF_PusSetTmSink(blocking_tm_sink);

    FF_TEST_ASSERT(FF_PalMutexCreate(&g_sink_mutex, "sink_mtx") == FF_SUCCESS);

    reset_capture();

    build_tc_full_ack(21, 3, 0x0104, (uint8_t[]){0x01}, 1, tc_frame1, &tc_len1);
    build_tc_full_ack(21, 4, 0x0105, (uint8_t[]){0x02}, 1, tc_frame2, &tc_len2);

    /* Spawn TC1 */
    FF_TEST_ASSERT(FF_PalTaskCreate(&g_block_task_id, "tc3a", blocking_task_fn, tc_frame1, 4096,
                                    10) == FF_SUCCESS);

    /* Wait for acceptance phase */
    uint32_t wait_cycles = 0;
    while (!g_tc1_in_acceptance && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    FF_TEST_ASSERT(g_tc1_in_acceptance);

    /* Now try TC2 from main thread - should get FF_BUSY */
    rc = FF_PusIngest(tc_frame2, tc_len2);
    FF_TEST_ASSERT(rc == FF_BUSY);

    /* Verify TC2's ACKs were NOT emitted */
    FF_TEST_ASSERT(stream_count(1, 1) == 1); /* Only TC1's acceptance */

    /* Unblock TC1 */
    g_sink_blocked = false;
    FF_PalTaskDelayMs(500);

    /* TC1 should complete with all ACKs */
    FF_TEST_ASSERT(stream_count(1, 1) == 1);
    FF_TEST_ASSERT(stream_count(1, 3) == 1);
    FF_TEST_ASSERT(stream_count(1, 5) == 1);
    FF_TEST_ASSERT(stream_count(1, 7) == 1);

    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    FF_PalMutexDelete(g_sink_mutex);

    FF_TEST_PRINT_SUMMARY("test_try_only_locks_busy");
}

/* ---- Test 4: Mutually exclusive accepted sink calls ------------------- */

static void test_mutually_exclusive_sink(void)
{
    uint8_t tc_frame1[256], tc_frame2[256];
    uint16_t tc_len1, tc_len2;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 5, simple_handler) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 6, simple_handler) == FF_SUCCESS);
    FF_PusSetTmSink(blocking_tm_sink);

    FF_TEST_ASSERT(FF_PalMutexCreate(&g_sink_mutex, "sink_mtx") == FF_SUCCESS);

    reset_capture();

    build_tc_full_ack(21, 5, 0x0106, (uint8_t[]){0x01}, 1, tc_frame1, &tc_len1);
    build_tc_full_ack(21, 6, 0x0107, (uint8_t[]){0x02}, 1, tc_frame2, &tc_len2);

    /* Spawn TC1 */
    FF_TEST_ASSERT(FF_PalTaskCreate(&g_block_task_id, "tc4a", blocking_task_fn, tc_frame1, 4096,
                                    10) == FF_SUCCESS);

    /* Wait for acceptance phase */
    uint32_t wait_cycles = 0;
    while (!g_tc1_in_acceptance && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    FF_TEST_ASSERT(g_tc1_in_acceptance);

    /* TC2 should get FF_BUSY (mutually exclusive) */
    rc = FF_PusIngest(tc_frame2, tc_len2);
    FF_TEST_ASSERT(rc == FF_BUSY);

    /* Unblock TC1 */
    g_sink_blocked = false;
    FF_PalTaskDelayMs(500);

    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    FF_PalMutexDelete(g_sink_mutex);

    FF_TEST_PRINT_SUMMARY("test_mutually_exclusive_sink");
}

/* ---- Test 5: Handler TM allowed during dispatch ----------------------- */

static void test_handler_tm_allowed(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 7, tm_emitting_handler) == FF_SUCCESS);
    FF_PusSetTmSink(nested_tm_sink); /* Non-blocking sink */

    reset_capture();

    build_tc_accept_complete(21, 7, 0x0108, (uint8_t[]){0x01}, 1, tc_frame, &tc_len);

    rc = FF_PusIngest(tc_frame, tc_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* Handler should have been able to emit TM */
    FF_TEST_ASSERT(g_handler_tm_count == 1);

    /* TC ACKs + handler TM should be emitted */
    FF_TEST_ASSERT(stream_count(1, 1) == 1);       /* acceptance */
    FF_TEST_ASSERT(stream_count(1, 7) == 1);       /* completion */
    FF_TEST_ASSERT(stream_count(0x15, 0x01) == 1); /* handler TM */

    FF_TEST_PRINT_SUMMARY("test_handler_tm_allowed");
}

/* ---- Test 6: Nested ingest => FF_BUSY --------------------------------- */

static void test_nested_ingest_busy(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 8, nested_ingest_handler) == FF_SUCCESS);
    FF_PusSetTmSink(nested_tm_sink);

    reset_capture();

    build_tc_accept_complete(21, 8, 0x0109, (uint8_t[]){0x01}, 1, tc_frame, &tc_len);

    rc = FF_PusIngest(tc_frame, tc_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* Nested ingest from handler should get FF_BUSY */
    FF_TEST_ASSERT(g_nested_ingest_blocked == true);

    FF_TEST_PRINT_SUMMARY("test_nested_ingest_busy");
}

/* ---- Test 7: No state lock across callbacks --------------------------- */

static void test_no_state_lock_across_callbacks(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 9, tm_emitting_handler) == FF_SUCCESS);
    FF_PusSetTmSink(nested_tm_sink);

    reset_capture();

    build_tc_accept_complete(21, 9, 0x0110, (uint8_t[]){0x01}, 1, tc_frame, &tc_len);

    /* First call */
    rc = FF_PusIngest(tc_frame, tc_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* Second call should not be blocked by state lock from handler */
    rc = FF_PusIngest(tc_frame, tc_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* Both should have emitted their ACKs */
    FF_TEST_ASSERT(stream_count(1, 1) == 2);
    FF_TEST_ASSERT(stream_count(1, 7) == 2);
    FF_TEST_ASSERT(g_handler_tm_count == 2);

    FF_TEST_PRINT_SUMMARY("test_no_state_lock_across_callbacks");
}

/* ---- Test 8: Serialized full dispatch --------------------------------- */

static void test_serialized_full_dispatch(void)
{
    uint8_t tc_frame1[256], tc_frame2[256];
    uint16_t tc_len1, tc_len2;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 10, simple_handler) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 11, simple_handler) == FF_SUCCESS);
    FF_PusSetTmSink(blocking_tm_sink);

    FF_TEST_ASSERT(FF_PalMutexCreate(&g_sink_mutex, "sink_mtx") == FF_SUCCESS);

    reset_capture();

    build_tc_full_ack(21, 10, 0x0111, (uint8_t[]){0x01}, 1, tc_frame1, &tc_len1);
    build_tc_full_ack(21, 11, 0x0112, (uint8_t[]){0x02}, 1, tc_frame2, &tc_len2);

    /* Spawn TC1 */
    FF_TEST_ASSERT(FF_PalTaskCreate(&g_block_task_id, "tc8a", blocking_task_fn, tc_frame1, 4096,
                                    10) == FF_SUCCESS);

    /* Wait for acceptance phase */
    uint32_t wait_cycles = 0;
    while (!g_tc1_in_acceptance && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    FF_TEST_ASSERT(g_tc1_in_acceptance);

    /* TC2 should get FF_BUSY */
    rc = FF_PusIngest(tc_frame2, tc_len2);
    FF_TEST_ASSERT(rc == FF_BUSY);

    /* Unblock TC1 */
    g_sink_blocked = false;
    FF_PalTaskDelayMs(500);

    /* Now TC2 should work */
    rc = FF_PusIngest(tc_frame2, tc_len2);
    FF_TEST_ASSERT(rc == FF_SUCCESS);
    FF_PalTaskDelayMs(200);

    /* Both TCs should have completed with all ACKs */
    FF_TEST_ASSERT(stream_count(1, 1) == 2);
    FF_TEST_ASSERT(stream_count(1, 3) == 2);
    FF_TEST_ASSERT(stream_count(1, 5) == 2);
    FF_TEST_ASSERT(stream_count(1, 7) == 2);

    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    FF_PalMutexDelete(g_sink_mutex);

    FF_TEST_PRINT_SUMMARY("test_serialized_full_dispatch");
}

/* ---- Test 9: Direct TM path bounded/lock-free ------------------------- */

static void test_direct_tm_bounded_lockfree(void)
{
    uint8_t direct_frame[256];
    uint16_t direct_len;
    FF_Status_t rc;
    uint16_t counter_start, counter_end;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* Get initial counter value */
    counter_start = FF_PusPeekTmCounter();

    /* Direct writer that just counts */
    uint32_t write_count = 0;

    /* Emit multiple direct TMs */
    for (int i = 0; i < 10; i++) {
        rc =
            FF_PusSendTmDirect(0x14, 0x01, (uint8_t[]){0x01}, 1, UINT64_MAX, counter_direct_writer);
        FF_TEST_ASSERT(rc == FF_SUCCESS);
    }

    counter_end = FF_PusPeekTmCounter();

    /* Counter should have incremented by exactly 10 (modulo 16384) */
    uint16_t diff = (uint16_t)((counter_end - counter_start) & 0x3FFFu);
    FF_TEST_ASSERT(diff == 10);

    FF_TEST_PRINT_SUMMARY("test_direct_tm_bounded_lockfree");
}

/* ---- Test 10: Sink-recursive TM emission => FF_BUSY ------------------- */

static void test_sink_recursive_tm_busy(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 10, simple_handler) == FF_SUCCESS);
    FF_PusSetTmSink(recursive_tm_sink);

    reset_capture();
    g_sink_recursed = false;
    g_sink_recursive_rc = FF_SUCCESS;

    build_tc_accept_complete(21, 10, 0x0111, (uint8_t[]){0x01}, 1, tc_frame, &tc_len);

    /* The outer dispatch must still succeed: a refused recursive emission is a
     * bounded, reported failure of the nested call, not a failure of the TC. */
    rc = FF_PusIngest(tc_frame, tc_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS);

    /* The recursive emission was attempted... */
    FF_TEST_ASSERT(g_sink_recursed == true);
    /* ...and refused with FF_BUSY (expected result per the contract)... */
    FF_TEST_ASSERT(g_sink_recursive_rc == FF_BUSY);
    /* ...and nothing extra was emitted for it. */
    FF_TEST_ASSERT(stream_count(0x15, 0x02) == 0);

    /* The outer ACK pair is intact: exactly one acceptance and one completion,
     * i.e. the refusal did not lose or duplicate the outer verification. */
    FF_TEST_ASSERT(stream_count(1, 1) == 1);
    FF_TEST_ASSERT(stream_count(1, 7) == 1);

    /* The lock is still consistent: a subsequent normal dispatch succeeds. */
    FF_PusSetTmSink(nested_tm_sink);
    reset_capture();
    rc = FF_PusIngest(tc_frame, tc_len);
    FF_TEST_ASSERT(rc == FF_SUCCESS);
    FF_TEST_ASSERT(stream_count(1, 1) == 1);
    FF_TEST_ASSERT(stream_count(1, 7) == 1);

    FF_TEST_PRINT_SUMMARY("test_sink_recursive_tm_busy");
}

/* ---- Test 11: one modulo-16384 sequence shared by both paths --------- */

static void test_mixed_path_counter_wrap(void)
{
    uint16_t v;
    uint16_t bad = 0u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* Walk the normal path to the last value before the wrap. */
    for (uint32_t i = 0u; i < 0x3FFFu; i++) {
        v = FF_PusNextTmCounter();
        if (v != (uint16_t)i) {
            bad++;
        }
    }
    FF_TEST_ASSERT(bad == 0u);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == 0x3FFFu);

    /* Cross the wrap through the reset/panic direct path (no state mutex). */
    FF_TEST_ASSERT(FF_PusSendTmDirect(0x14, 0x01, (uint8_t[]){0x01}, 1, UINT64_MAX,
                                      counter_direct_writer) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSendTmDirect(0x14, 0x01, (uint8_t[]){0x01}, 1, UINT64_MAX,
                                      counter_direct_writer) == FF_SUCCESS);

    /* The stored value must still be inside the modulo-16384 sequence; the
     * direct path must not leave storage above 0x3FFF. */
    FF_TEST_ASSERT(FF_PusPeekTmCounter() < 16384u);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == 1u);

    /* The normal path continues the same sequence from the same value. */
    FF_TEST_ASSERT(FF_PusNextTmCounter() == 1u);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == 2u);

    FF_TEST_PRINT_SUMMARY("test_mixed_path_counter_wrap");
}

/* ---- Test 12: dispatch serialized even without ACK requests ---------- */

static void test_no_ack_dispatch_serialized(void)
{
    uint8_t tc_frame[256];
    uint16_t tc_len;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 4, nested_ingest_handler) == FF_SUCCESS);
    FF_PusSetTmSink(nested_tm_sink);

    reset_capture();

    /* A TC that requests no verification ACK at all: the dispatch reservation
     * must still be armed, otherwise two such TCs would run handlers
     * concurrently and handler state is not internally synchronized. */
    build_tc_no_ack(21, 4, 0x0300, (uint8_t[]){0x00}, 1, tc_frame, &tc_len);
    FF_TEST_ASSERT(FF_PusIngest(tc_frame, tc_len) == FF_SUCCESS);

    /* The handler's nested ingest was refused with FF_BUSY. */
    FF_TEST_ASSERT(g_nested_ingest_blocked == true);
    /* No ACK was requested, so none may be emitted. */
    FF_TEST_ASSERT(g_rx_count == 0u);

    /* The reservation was released: a following dispatch still runs. */
    FF_TEST_ASSERT(FF_PusIngest(tc_frame, tc_len) == FF_SUCCESS);

    /* No required verification was dropped in the nominal path. */
    FF_TEST_ASSERT(FF_PusPeekAckDropped() == 0u);

    FF_TEST_PRINT_SUMMARY("test_no_ack_dispatch_serialized");
}

/* ---- Test 13: no counter consumed for a frame never transmitted ------ */

static void test_no_counter_hole_without_sink(void)
{
    uint16_t before;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* No sink registered: emission is a successful no-op by contract. */
    before = FF_PusPeekTmCounter();
    FF_TEST_ASSERT(FF_PusSendTm(0x15, 0x01, (uint8_t[]){0x11}, 1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSendTm(0x15, 0x01, (uint8_t[]){0x11}, 1) == FF_SUCCESS);
    /* ... and it must not consume counter values: a consumed value with no
     * transmission is a hole the ground reports as a missing packet
     * (SRS-REQ-007; R3 F-4). */
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == before);

    /* A rejected request consumes nothing either. */
    FF_TEST_ASSERT(FF_PusSendTm(0x15, 0x01, NULL, 4) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == before);

    /* With a sink attached, each transmitted frame consumes exactly one value. */
    reset_capture();
    FF_PusSetTmSink(nested_tm_sink);
    FF_TEST_ASSERT(FF_PusSendTm(0x15, 0x01, (uint8_t[]){0x22}, 1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == (uint16_t)(before + 1u));
    FF_TEST_ASSERT(FF_PusSendTm(0x15, 0x01, (uint8_t[]){0x22}, 1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusPeekTmCounter() == (uint16_t)(before + 2u));
    FF_TEST_ASSERT(g_rx_count > 0u);

    FF_TEST_PRINT_SUMMARY("test_no_counter_hole_without_sink");
}

/* ---- Blocking task function ------------------------------------------- */

static void blocking_task_fn(void *arg)
{
    uint8_t *tc_frame = (uint8_t *)arg;
    uint16_t tc_len = (uint16_t)(((uint16_t)tc_frame[4] << 8) | tc_frame[5]);
    tc_len = (uint16_t)(tc_len + 7u);
    FF_PusIngest(tc_frame, tc_len);
}

/* ---- Direct writer for lock-free counter test ------------------------- */

static FF_Status_t counter_direct_writer(const uint8_t *frame, uint16_t len, uint64_t deadline_ns)
{
    (void)frame;
    (void)len;
    (void)deadline_ns;
    return FF_SUCCESS;
}

/* ---- Main test -------------------------------------------------------- */

FF_TEST_MAIN(test_pre_acceptance_ack_reservation, test_no_missing_ack_after_acceptance,
             test_try_only_locks_busy, test_mutually_exclusive_sink, test_handler_tm_allowed,
             test_nested_ingest_busy, test_no_state_lock_across_callbacks,
             test_serialized_full_dispatch, test_direct_tm_bounded_lockfree,
             test_sink_recursive_tm_busy, test_mixed_path_counter_wrap,
             test_no_ack_dispatch_serialized, test_no_counter_hole_without_sink)