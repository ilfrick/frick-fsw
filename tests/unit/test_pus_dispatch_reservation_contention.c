/*
 * frick-fsw - NC-EXT-251 TDD GREEN test: dispatch reservation contention
 *
 * Tests that when a TC is in the acceptance phase (holds dispatch mutex),
 * another TC receives FF_BUSY immediately.
 *
 * Uses a slow TM sink to extend the acceptance phase window.
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

/* ---- Test infrastructure ---------------------------------------------- */

static uint8_t g_rx_buf[8 * 256];
static uint16_t g_rx_count;
static volatile bool g_sink_blocked = false;
static volatile bool g_tc1_in_acceptance = false;
static FF_TaskId_t g_block_task_id;
static FF_MutexId_t g_sink_mutex;

/* TM sink that blocks on first call (simulating slow ACK emission) */
static void blocking_tm_sink(const uint8_t *packet, uint16_t len)
{
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }

    /* Block on first call to simulate slow ACK emission */
    if (!g_sink_blocked) {
        g_sink_blocked = true;
        /* Signal that TC1 is in acceptance phase */
        g_tc1_in_acceptance = true;
        /* Wait until unblocked */
        while (g_sink_blocked) {
            FF_PalTaskDelayMs(1);
        }
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

/* Blocking task function - ingests TC1 which will block in sink during ACK emission */
static void blocking_task_fn(void *arg)
{
    uint8_t *tc_frame = (uint8_t *)arg;
    uint16_t tc_len = (uint16_t)(((uint16_t)tc_frame[4] << 8) | tc_frame[5]);
    tc_len = (uint16_t)(tc_len + 7u);
    FF_PusIngest(tc_frame, tc_len);
}

/* ---- Main test -------------------------------------------------------- */

static void test_pus_dispatch_reservation_contention(void)
{
    uint8_t tc_frame1[256];
    uint16_t tc_len1;
    uint8_t tc_frame2[256];
    uint16_t tc_len2;
    FF_Status_t rc;

    /* Initialize PUS stack */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(blocking_tm_sink);

    /* Register handlers for custom service 21 */
    FF_TEST_ASSERT(FF_PusRegisterService(21, 1, simple_handler) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterService(21, 2, simple_handler) == FF_SUCCESS);

    /* Create mutex for sink coordination */
    FF_TEST_ASSERT(FF_PalMutexCreate(&g_sink_mutex, "sink_mtx") == FF_SUCCESS);

    /* Reset capture buffer */
    g_rx_count = 0u;
    memset(g_rx_buf, 0, sizeof(g_rx_buf));
    g_sink_blocked = false;
    g_tc1_in_acceptance = false;

    /* Build TC1: requests all four ACK stages, uses service 21 subservice 1 */
    build_tc_full_ack(21, 1, 0x0102, (uint8_t[]){0x01, 0x02, 0x03, 0x04}, 4, tc_frame1, &tc_len1);

    /* Build TC2: also requests all ACK stages, uses service 21 subservice 2 */
    build_tc_full_ack(21, 2, 0x0103, (uint8_t[]){0xAA, 0xBB, 0xCC, 0xDD}, 4, tc_frame2, &tc_len2);

    /* Spawn task that ingests TC1 - will block in sink during first ACK emission */
    FF_TEST_ASSERT(FF_PalTaskCreate(&g_block_task_id, "tc1", blocking_task_fn, tc_frame1, 4096,
                                    10) == FF_SUCCESS);

    /* Wait for TC1 to enter acceptance phase (sink blocked) */
    uint32_t wait_cycles = 0;
    while (!g_tc1_in_acceptance && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    FF_TEST_ASSERT(g_tc1_in_acceptance);

    printf("TC1 in acceptance phase, attempting TC2...\n");
    fflush(stdout);

    /* Now attempt TC2 from main thread - should get FF_BUSY because
     * TC1 holds the dispatch mutex during acceptance phase */
    rc = FF_PusIngest(tc_frame2, tc_len2);

    /* GREEN PHASE ASSERTION: TC2 must get FF_BUSY because dispatch mutex
     * is held by TC1 during acceptance phase */
    printf("TC2 returned %d, expected FF_BUSY=%d\n", rc, FF_BUSY);
    fflush(stdout);
    FF_TEST_ASSERT(rc == FF_BUSY);

    /* Verify TC2's S1 ACKs were NOT emitted (it got FF_BUSY before acceptance).
     * Note: TC1's acceptance ACK (service 1, subservice 1) was already emitted
     * before TC2 was attempted, so stream_count(1,1) will be 1 from TC1.
     * We verify TC2's ACKs by checking completion ACK which TC1 hasn't reached yet. */
    FF_TEST_ASSERT(stream_count(1, 7) == 0); /* TC2 completion success - not emitted */

    /* Now unblock the sink to let TC1 complete */
    g_sink_blocked = false;

    /* Wait for TC1 to complete */
    FF_PalTaskDelayMs(500);

    /* Verify TC1's ACKs were emitted (acceptance and start at minimum) */
    FF_TEST_ASSERT(stream_count(1, 1) == 1); /* TC1 acceptance success */
    FF_TEST_ASSERT(stream_count(1, 3) == 1); /* TC1 start success */

    /* Clean up */
    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    FF_PalMutexDelete(g_sink_mutex);

    FF_TEST_PRINT_SUMMARY("test_pus_dispatch_reservation_contention");
}

FF_TEST_MAIN(test_pus_dispatch_reservation_contention)
