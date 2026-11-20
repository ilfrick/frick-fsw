/* SPDX-License-Identifier: MPL-2.0 */
#include "core/pal/pal.h"
#include "../ff_test.h"
#include <stdio.h>

static FF_MutexId_t g_test_mutex;
static volatile bool g_thread1_holding = false;
static volatile bool g_thread1_done = false;
static FF_TaskId_t g_thread1_id;

static void thread1_fn(void *arg)
{
    (void)arg;
    printf("THREAD1: trying to lock mutex\n");
    fflush(stdout);
    FF_Status_t st = FF_PalMutexLock(g_test_mutex, 0);
    printf("THREAD1: lock returned %d\n", st);
    fflush(stdout);
    
    if (st == FF_SUCCESS) {
        g_thread1_holding = true;
        printf("THREAD1: holding mutex, sleeping 2s\n");
        fflush(stdout);
        FF_PalTaskDelayMs(2000);
        printf("THREAD1: releasing mutex\n");
        fflush(stdout);
        FF_PalMutexUnlock(g_test_mutex);
        g_thread1_holding = false;
    }
    g_thread1_done = true;
}

static void test_mutex_contention(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    
    FF_Status_t st = FF_PalMutexCreate(&g_test_mutex, "test_mtx");
    FF_TEST_ASSERT(st == FF_SUCCESS);
    printf("MAIN: mutex created, id = %u\n", g_test_mutex);
    fflush(stdout);
    
    st = FF_PalTaskCreate(&g_thread1_id, "thr1", thread1_fn, NULL, 4096, 10);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    
    /* Wait for thread1 to acquire mutex */
    while (!g_thread1_holding) {
        FF_PalTaskDelayMs(10);
    }
    printf("MAIN: thread1 is holding mutex\n");
    fflush(stdout);
    
    /* Try to acquire mutex - should fail with FF_TIMEOUT */
    printf("MAIN: trying to trylock mutex\n");
    fflush(stdout);
    st = FF_PalMutexTryLock(g_test_mutex);
    printf("MAIN: trylock returned %d (expected FF_TIMEOUT=%d)\n", st, -4);
    fflush(stdout);
    FF_TEST_ASSERT(st == FF_TIMEOUT);
    
    /* Wait for thread1 to release */
    while (!g_thread1_done) {
        FF_PalTaskDelayMs(10);
    }
    
    /* Now try again - should succeed */
    printf("MAIN: trying to trylock mutex again\n");
    fflush(stdout);
    st = FF_PalMutexTryLock(g_test_mutex);
    printf("MAIN: trylock returned %d (expected FF_SUCCESS=%d)\n", st, 0);
    fflush(stdout);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    
    FF_PalMutexUnlock(g_test_mutex);
    FF_PalMutexDelete(g_test_mutex);
    
    FF_TEST_ASSERT(FF_PalTaskDelete(g_thread1_id) == FF_SUCCESS);
    printf("MAIN: done\n");
    fflush(stdout);
}

FF_TEST_MAIN(test_mutex_contention)
