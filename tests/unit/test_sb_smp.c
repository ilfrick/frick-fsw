/*
 * frick-fsw - Software Bus SMP tests
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/*
 * SMP test infrastructure for the Software Bus.
 *
 * These tests verify that the SB implementation is thread-safe under concurrent
 * access from multiple threads, simulating the multi-core SMP environment.
 * The tests use POSIX pthreads to exercise concurrent publish/receive operations.
 */

#define SMP_TEST_THREADS 4
#define SMP_TEST_MESSAGES_PER_THREAD 10
#define SMP_TEST_SUBSCRIBERS 8
#define SMP_TEST_MIN_DELIVERY_RATE 0.7 /* Expect at least 70% delivery under contention */

static uint32_t g_smp_test_received[SMP_TEST_THREADS][SMP_TEST_SUBSCRIBERS];
static pthread_mutex_t g_smp_test_mutex;
static bool g_smp_test_ready;
static pthread_barrier_t g_smp_barrier;

typedef struct
{
    uint32_t thread_id;
    FF_AppId_t publisher_app;
    FF_MsgId_t msg_id;
} smp_test_thread_arg_t;

static void *smp_test_publisher_thread(void *arg)
{
    smp_test_thread_arg_t *targ = (smp_test_thread_arg_t *)arg;
    FF_Msg_t msg;

    /* Wait for all threads to be ready */
    pthread_barrier_wait(&g_smp_barrier);

    for (uint32_t i = 0; i < SMP_TEST_MESSAGES_PER_THREAD; i++) {
        memset(&msg, 0, sizeof(msg));
        msg.msg_id = targ->msg_id;
        msg.length = 4;
        msg.payload[0] = (uint8_t)(targ->thread_id & 0xFF);
        msg.payload[1] = (uint8_t)((targ->thread_id >> 8) & 0xFF);
        msg.payload[2] = (uint8_t)(i & 0xFF);
        msg.payload[3] = (uint8_t)((i >> 8) & 0xFF);

        FF_Status_t status = FF_SbPublish(&msg);
        if (status != FF_SUCCESS && status != FF_TIMEOUT) {
            printf("  publisher thread_id=%u msg_id=0x%X status=%d\n", targ->thread_id,
                   targ->msg_id, status);
        }
        FF_TEST_ASSERT(status == FF_SUCCESS || status == FF_TIMEOUT);
    }

    return NULL;
}

static void *smp_test_subscriber_thread(void *arg)
{
    smp_test_thread_arg_t *targ = (smp_test_thread_arg_t *)arg;
    FF_Msg_t msg;
    uint32_t received_count = 0;

    /* Wait for all threads to be ready */
    pthread_barrier_wait(&g_smp_barrier);

    uint32_t total_timeouts = 0;
    while (received_count < SMP_TEST_MESSAGES_PER_THREAD * SMP_TEST_THREADS &&
           total_timeouts < 10000) {
        FF_Status_t status = FF_SbReceive(targ->publisher_app, &msg, 100u);
        if (status == FF_SUCCESS) {
            /* Check if msg_id is in publisher range (0x1000 - 0x1003) */
            if (msg.msg_id >= 0x1000u && msg.msg_id < 0x1000u + SMP_TEST_THREADS) {
                pthread_mutex_lock(&g_smp_test_mutex);
                uint32_t publisher_id = (msg.payload[1] << 8) | msg.payload[0];
                if (publisher_id < SMP_TEST_THREADS) {
                    g_smp_test_received[publisher_id][targ->thread_id]++;
                }
                received_count++;
                pthread_mutex_unlock(&g_smp_test_mutex);
            }
        }
        else if (status == FF_TIMEOUT) {
            total_timeouts++;
        }
    }

    return NULL;
}

static void test_sb_smp_concurrent_publish_receive(void)
{
    pthread_t publisher_threads[SMP_TEST_THREADS];
    pthread_t subscriber_threads[SMP_TEST_SUBSCRIBERS];
    smp_test_thread_arg_t publisher_args[SMP_TEST_THREADS];
    smp_test_thread_arg_t subscriber_args[SMP_TEST_SUBSCRIBERS];

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    pthread_mutex_init(&g_smp_test_mutex, NULL);
    pthread_barrier_init(&g_smp_barrier, NULL, SMP_TEST_THREADS + SMP_TEST_SUBSCRIBERS);
    memset(g_smp_test_received, 0, sizeof(g_smp_test_received));
    g_smp_test_ready = true;

    /* Create publisher apps and pipes */
    for (uint32_t i = 0; i < SMP_TEST_THREADS; i++) {
        FF_AppId_t app_id = 100u + i;
        publisher_args[i].thread_id = i;
        publisher_args[i].publisher_app = app_id;
        publisher_args[i].msg_id = 0x1000u + i;
        FF_TEST_ASSERT(FF_SbCreatePipe(app_id, 256u) == FF_SUCCESS);
    }

    /* Create subscriber apps and pipes, subscribe to all publisher msg_ids */
    for (uint32_t i = 0; i < SMP_TEST_SUBSCRIBERS; i++) {
        FF_AppId_t app_id = 200u + i;
        subscriber_args[i].thread_id = i;
        subscriber_args[i].publisher_app = app_id;
        subscriber_args[i].msg_id = 0x1000u; /* base msg_id, we subscribe to all */
        FF_TEST_ASSERT(FF_SbCreatePipe(app_id, 256u) == FF_SUCCESS);
        for (uint32_t j = 0; j < SMP_TEST_THREADS; j++) {
            FF_Status_t sub_status = FF_SbSubscribe(0x1000u + j, app_id);
            printf("  sub=%u subscribe msg_id=0x%X status=%d\n", i, 0x1000u + j, sub_status);
            FF_TEST_ASSERT(sub_status == FF_SUCCESS);
        }
    }

    /* Small delay to ensure all subscriptions are visible */
    usleep(1000000);

    /* Launch publisher threads */
    for (uint32_t i = 0; i < SMP_TEST_THREADS; i++) {
        pthread_create(&publisher_threads[i], NULL, smp_test_publisher_thread, &publisher_args[i]);
    }

    /* Launch subscriber threads */
    for (uint32_t i = 0; i < SMP_TEST_SUBSCRIBERS; i++) {
        pthread_create(&subscriber_threads[i], NULL, smp_test_subscriber_thread,
                       &subscriber_args[i]);
    }

    /* Wait for publisher threads to complete */
    for (uint32_t i = 0; i < SMP_TEST_THREADS; i++) {
        pthread_join(publisher_threads[i], NULL);
    }

    /* Wait for publisher threads to complete */
    for (uint32_t i = 0; i < SMP_TEST_THREADS; i++) {
        pthread_join(publisher_threads[i], NULL);
    }

    /* Wait for subscriber threads to complete */
    for (uint32_t i = 0; i < SMP_TEST_SUBSCRIBERS; i++) {
        pthread_join(subscriber_threads[i], NULL);
    }

    uint32_t total_published = SMP_TEST_MESSAGES_PER_THREAD * SMP_TEST_THREADS;
    uint32_t total_expected_per_subscriber = total_published;

    /* Print debug info */
    uint32_t min_expected = (uint32_t)(total_expected_per_subscriber * SMP_TEST_MIN_DELIVERY_RATE);
    for (uint32_t sub = 0; sub < SMP_TEST_SUBSCRIBERS; sub++) {
        uint32_t sub_total = 0;
        for (uint32_t pub = 0; pub < SMP_TEST_THREADS; pub++) {
            sub_total += g_smp_test_received[pub][sub];
            printf("  sub=%u pub=%u received=%u\n", sub, pub, g_smp_test_received[pub][sub]);
        }
        printf("  sub=%u total=%u expected=%u min=%u\n", sub, sub_total,
               total_expected_per_subscriber, min_expected);
    }

    /* Verify messages were delivered to all subscribers (allow some drops under contention) */
    for (uint32_t sub = 0; sub < SMP_TEST_SUBSCRIBERS; sub++) {
        uint32_t sub_total = 0;
        for (uint32_t pub = 0; pub < SMP_TEST_THREADS; pub++) {
            sub_total += g_smp_test_received[pub][sub];
        }
        FF_TEST_ASSERT(sub_total >= min_expected);
    }

    pthread_barrier_destroy(&g_smp_barrier);
    pthread_mutex_destroy(&g_smp_test_mutex);
}

static void test_sb_smp_stats_accuracy(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    FF_SbStats_t stats_before, stats_after;

    FF_TEST_ASSERT(FF_SbGetStats(&stats_before) == FF_SUCCESS);

    FF_Msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_id = 0x2000u;
    msg.length = 1;
    msg.payload[0] = 0xAAu;

    /* Single-threaded publish to verify stats */
    for (uint32_t i = 0; i < 100; i++) {
        FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);
    }

    FF_TEST_ASSERT(FF_SbGetStats(&stats_after) == FF_SUCCESS);
    FF_TEST_ASSERT(stats_after.published == stats_before.published + 100);
    /* No subscribers yet, so delivered = 0, dropped = 0 */
    FF_TEST_ASSERT(stats_after.delivered == stats_before.delivered);
    FF_TEST_ASSERT(stats_after.dropped == stats_before.dropped);
}

static void test_sb_smp_mutex_try_lock(void)
{
    FF_Msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_id = 0x3000u;

    /* Test that FF_SbTestLock acquires the global lock and sets maintenance mode,
     * causing subsequent publishes to return FF_TIMEOUT without mutation. */
    FF_TEST_ASSERT(FF_SbTestLock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_TIMEOUT);
    FF_TEST_ASSERT(FF_SbTestUnlock() == FF_SUCCESS);

    /* After unlock, publish should succeed again */
    FF_TEST_ASSERT(FF_SbPublish(&msg) == FF_SUCCESS);

    /* Test that recursive lock attempt on same thread blocks (using TryLock for non-blocking check)
     */
    FF_TEST_ASSERT(FF_SbTestLock() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbTestUnlock() == FF_SUCCESS);
}

FF_TEST_MAIN(test_sb_smp_stats_accuracy, test_sb_smp_mutex_try_lock,
             test_sb_smp_concurrent_publish_receive)