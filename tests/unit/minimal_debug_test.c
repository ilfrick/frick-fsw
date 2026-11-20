/* SPDX-License-Identifier: MPL-2.0 */
#include "core/pal/pal.h"
#include "pus/pus.h"
#include "core/time/time.h"
#include "../ff_test.h"
#include <stdio.h>
#include <string.h>

static volatile bool g_block_sink = true;
static volatile bool g_sink_blocked = false;
static uint8_t g_rx_buf[8 * 256];
static uint16_t g_rx_count = 0;
static FF_TaskId_t g_block_task_id;

static void blocking_tm_sink(const uint8_t *packet, uint16_t len)
{
    printf("SINK: called with %u bytes\n", len);
    fflush(stdout);
    if ((g_rx_count + len) < sizeof(g_rx_buf)) {
        memcpy(&g_rx_buf[g_rx_count], packet, len);
        g_rx_count = (uint16_t)(g_rx_count + len);
    }

    if (g_block_sink) {
        g_sink_blocked = true;
        printf("SINK: blocking...\n");
        fflush(stdout);
        while (g_block_sink) {
            FF_PalTaskDelayMs(1);
        }
        printf("SINK: unblocked\n");
        fflush(stdout);
    }
}

static void blocking_task_fn(void *arg)
{
    (void)arg;
    printf("TASK: calling FF_PusSendTm\n");
    fflush(stdout);
    FF_Status_t st = FF_PusSendTm(5, 1, (uint8_t[]){0xAA, 0xBB}, 2);
    printf("TASK: FF_PusSendTm returned %d\n", st);
    fflush(stdout);
    if (st != FF_SUCCESS) {
        return;
    }
    printf("TASK: done\n");
    fflush(stdout);
}

static void test_minimal_debug(void)
{
    printf("MAIN: init\n");
    fflush(stdout);
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_Status_t st = FF_PusInit();
    printf("MAIN: FF_PusInit = %d\n", st);
    fflush(stdout);
    st = FF_PusSetBackend(&FF_PusBackendOpen);
    printf("MAIN: FF_PusSetBackend = %d\n", st);
    fflush(stdout);
    st = FF_PusSetTmSink(blocking_tm_sink);
    printf("MAIN: FF_PusSetTmSink = %d\n", st);
    fflush(stdout);
    printf("MAIN: PUS ready\n");
    fflush(stdout);

    st = FF_PalTaskCreate(&g_block_task_id, "blk_sink", blocking_task_fn, NULL,
                          4096, 10);
    printf("MAIN: FF_PalTaskCreate = %d\n", st);
    fflush(stdout);

    printf("MAIN: waiting for sink blocked\n");
    fflush(stdout);
    uint32_t wait_cycles = 0;
    while (!g_sink_blocked && wait_cycles < 2000) {
        FF_PalTaskDelayMs(5);
        wait_cycles++;
    }
    printf("MAIN: g_sink_blocked = %d, wait_cycles = %u\n", g_sink_blocked, wait_cycles);
    fflush(stdout);

    g_block_sink = false;
    FF_TEST_ASSERT(FF_PalTaskDelete(g_block_task_id) == FF_SUCCESS);
    printf("MAIN: done\n");
    fflush(stdout);
}

FF_TEST_MAIN(test_minimal_debug)
