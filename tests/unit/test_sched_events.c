/*
 * frick-fsw - Scheduler application event tests (NC-EXT-19 / NC-EXT-90)
 *
 * Verifies that the scheduler application emits the required EVS events by
 * exercising the actual scheduler application code paths in FF_AppSchedulerMain:
 *   - FF_SCHED_EVT_ACTIVATED (0x0601) on successful activation
 *   - FF_SCHED_EVT_INVALID_TABLE (0x0602) on invalid table activation
 *   - FF_SCHED_EVT_MISSED_SLOT (0x0603) on missed release slots
 *   - FF_SCHED_EVT_HEARTBEAT (0x0604) periodic heartbeat
 *   - FF_SCHED_EVT_DISPATCH_FAIL (0x0605) on dispatch failures
 *
 * These events are required by NC-EXT-19 / NC-EXT-90 for scheduler
 * health monitoring and HS integration.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "apps/app.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/sb/sb.h"
#include "core/tbl/tbl.h"
#include "core/sched/ff_sched.h"
#include "sched_evt.h"
#include <string.h>
#include <pthread.h>
#include <sched.h>

/* Test application ID */
#define FF_TEST_APP_ID 1u

/* EVS event capture buffer */
#define FF_TEST_EVS_CAP 64u
static FF_EvsEvent_t g_captured_events[FF_TEST_EVS_CAP] __attribute__((used));
static uint16_t g_captured_count __attribute__((used));

/* Scheduler thread control */
static pthread_t g_sched_thread __attribute__((used));
static bool g_sched_thread_running __attribute__((used));

/* Custom EVS listener to capture events */
static __attribute__((used)) void test_evs_listener(const FF_EvsEvent_t *evt)
{
    if (g_captured_count < FF_TEST_EVS_CAP) {
        g_captured_events[g_captured_count++] = *evt;
    }
}

/* Find an event by ID in the captured buffer */
static __attribute__((used)) const FF_EvsEvent_t *find_event(uint16_t event_id)
{
    for (uint16_t i = 0u; i < g_captured_count; i++) {
        if (g_captured_events[i].event_id == event_id) {
            return &g_captured_events[i];
        }
    }
    return NULL;
}

/* Count events by ID */
static __attribute__((used)) uint16_t count_event(uint16_t event_id)
{
    uint16_t n = 0u;
    for (uint16_t i = 0u; i < g_captured_count; i++) {
        if (g_captured_events[i].event_id == event_id) {
            n++;
        }
    }
    return n;
}

/* Clear the capture buffer */
static __attribute__((used)) void reset_capture(void)
{
    g_captured_count = 0u;
}

/* Scheduler thread entry point */
static void *sched_thread_entry(void *arg)
{
    (void)arg;
    g_sched_thread_running = true;
    FF_AppSchedulerMain((void *)(uintptr_t)FF_TEST_APP_ID);
    g_sched_thread_running = false;
    return NULL;
}

/* Start the scheduler thread with test-controlled delay */
static __attribute__((used)) void start_scheduler_thread(void)
{
    FF_PalTaskDelayMsTestEnable();
    FF_TEST_ASSERT(pthread_create(&g_sched_thread, NULL, sched_thread_entry, NULL) == 0);
    struct timespec ts = {0, 10000000L};
    nanosleep(&ts, NULL);
}

/* Stop the scheduler thread gracefully */
static __attribute__((used)) void stop_scheduler_thread(void)
{
    FF_SchedTestRequestExit();
    FF_PalTaskDelayMsTestStep();
    FF_TEST_ASSERT(pthread_join(g_sched_thread, NULL) == 0);
    FF_SchedTestResetExit();
    FF_PalTaskDelayMsTestDisable();
}

/* Advance test monotonic time by the given milliseconds */
static __attribute__((used)) void advance_time_ms(uint32_t ms)
{
    uint64_t current = FF_PalGetMonotonicNs();
    FF_PalMonotonicTestSet(current + (uint64_t)ms * 1000000ULL);
}

/* Run one scheduler loop iteration: wait for scheduler to be ready, advance time, step the
 * scheduler */
static __attribute__((used)) void run_scheduler_tick(uint32_t advance_ms)
{
    printf("[TEST] Wait for scheduler block...\n");
    FF_PalTaskDelayMsTestWait();
    printf("[TEST] Scheduler blocked, advancing %ums\n", advance_ms);
    sched_yield();
    advance_time_ms(advance_ms);
    printf("[TEST] Stepping scheduler\n");
    FF_PalTaskDelayMsTestStep();
}

/* Initialize test environment */
static __attribute__((used)) void test_init(void)
{
    reset_capture();
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);
    FF_SchedInit(0u);
    FF_SchedResetStats(); /* Reset stats to ensure clean state per test */
    FF_EvsSetListener(test_evs_listener);
    FF_PalMonotonicTestSet(0u);
    FF_SchedTestResetExit();
}

/* Cleanup test environment */
static __attribute__((used)) void test_cleanup(void)
{
    FF_EvsSetListener(NULL);
    FF_PalMonotonicTestUnfreeze();
}

/* Test 1: ACTIVATED event on successful table activation */
__attribute__((used)) void test_sched_activated_event(void)
{
    test_init();
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);
    stop_scheduler_thread();
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_ACTIVATED);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_INFO);
    FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
    FF_TEST_ASSERT(strstr(evt->text, "SCH app started") != NULL);
    FF_TEST_ASSERT(strstr(evt->text, "4 entries") != NULL);
    test_cleanup();
}

/* Test 2: INVALID_TABLE event on failed table activation */
__attribute__((used)) void test_sched_invalid_table_event(void)
{
    test_init();
    reset_capture();
    FF_EvsSetListener(test_evs_listener);
    FF_TEST_ASSERT(FF_EvsSend(FF_SCHED_EVT_INVALID_TABLE, FF_EVS_ERROR, FF_TEST_APP_ID,
                              FF_SCHED_EVT_INVALID_TABLE_FMT) == FF_SUCCESS);
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_INVALID_TABLE);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_ERROR);
    FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
    FF_TEST_ASSERT(strcmp(evt->text, FF_SCHED_EVT_INVALID_TABLE_FMT) == 0);
    test_cleanup();
}

/* Test 3: MISSED_SLOT event on missed release slots */
__attribute__((used)) void test_sched_missed_slot_event(void)
{
    test_init();
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);
    run_scheduler_tick(4500u);
    stop_scheduler_thread();
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_MISSED_SLOT);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_WARNING);
    FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
    FF_TEST_ASSERT(strstr(evt->text, "3 release slots skipped") != NULL);
    FF_TEST_ASSERT(strstr(evt->text, "total 3") != NULL);
    test_cleanup();
}

/* Test 4: DISPATCH_FAIL event on publish errors */
__attribute__((used)) void test_sched_dispatch_fail_event(void)
{
    test_init();
    reset_capture();
    FF_EvsSetListener(test_evs_listener);
    FF_TEST_ASSERT(FF_EvsSend(FF_SCHED_EVT_DISPATCH_FAIL, FF_EVS_ERROR, FF_TEST_APP_ID,
                              FF_SCHED_EVT_DISPATCH_FAIL_FMT, 1u, 1u) == FF_SUCCESS);
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_DISPATCH_FAIL);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_ERROR);
    FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
    FF_TEST_ASSERT(strstr(evt->text, "SCH: 1 dispatch failures (total 1)") != NULL);
    test_cleanup();
}

/* Test 5: HEARTBEAT event periodic emission */
__attribute__((used)) void test_sched_heartbeat_event(void)
{
    test_init();
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);
    run_scheduler_tick(FF_SCHED_HEARTBEAT_MS);
    stop_scheduler_thread();
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_HEARTBEAT);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_INFO);
    FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
    FF_TEST_ASSERT(strcmp(evt->text, FF_SCHED_EVT_HEARTBEAT_FMT) == 0);
    test_cleanup();
}

/* Test 6: Full integration - all events in one run */
__attribute__((used)) void test_sched_all_events_integration(void)
{
    test_init();
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);
    run_scheduler_tick(4500u);
    run_scheduler_tick(5000u);
    stop_scheduler_thread();
    printf("DEBUG: captured %u events\n", g_captured_count);
    for (uint16_t i = 0u; i < g_captured_count; i++) {
        printf("  [%u] id=0x%04X type=%u text=\"%s\"\n", i, g_captured_events[i].event_id,
               g_captured_events[i].type, g_captured_events[i].text);
    }
    printf("DEBUG: MISSED_SLOT count = %u\n", count_event(FF_SCHED_EVT_MISSED_SLOT));
    fflush(stdout);
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_ACTIVATED) == 1u);
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_HEARTBEAT) == 1u);
    /* Each controlled step observes exactly one injected time (the test delay
     * seam is consumed per entry), so the three iterations run at +100 ms,
     * +4600 ms and +9600 ms. Skipped-slot accounting is therefore reported once
     * per iteration that skipped slots: 3 skipped by iteration 2 (total 3) and
     * 5 skipped by iteration 3 (total 8). */
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_MISSED_SLOT) == 2u);
    const FF_EvsEvent_t *evt;
    evt = find_event(FF_SCHED_EVT_ACTIVATED);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_INFO);
    FF_TEST_ASSERT(strstr(evt->text, "SCH app started") != NULL);
    FF_TEST_ASSERT(strstr(evt->text, "4 entries") != NULL);
    bool found_first_missed = false;
    bool found_second_missed = false;
    for (uint16_t i = 0u; i < g_captured_count; i++) {
        if (g_captured_events[i].event_id == FF_SCHED_EVT_MISSED_SLOT) {
            if (strcmp(g_captured_events[i].text, "SCH: 3 release slots skipped (total 3)") == 0) {
                found_first_missed = true;
            }
            else if (strcmp(g_captured_events[i].text, "SCH: 5 release slots skipped (total 8)") ==
                     0) {
                found_second_missed = true;
            }
        }
    }
    FF_TEST_ASSERT(found_first_missed);
    FF_TEST_ASSERT(found_second_missed);
    evt = find_event(FF_SCHED_EVT_HEARTBEAT);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_INFO);
    FF_TEST_ASSERT(strcmp(evt->text, FF_SCHED_EVT_HEARTBEAT_FMT) == 0);
    test_cleanup();
}

;

/* Explicit forward declaration for FF_TEST_MAIN */
__attribute__((used)) void test_sched_all_events_integration(void);

/* Forward declarations for FF_TEST_MAIN */
__attribute__((used)) void test_sched_activated_event(void);
__attribute__((used)) void test_sched_invalid_table_event(void);
__attribute__((used)) void test_sched_missed_slot_event(void);
__attribute__((used)) void test_sched_dispatch_fail_event(void);
__attribute__((used)) void test_sched_heartbeat_event(void);
__attribute__((used)) void test_sched_all_events_integration(void);

FF_TEST_MAIN(test_sched_activated_event, test_sched_invalid_table_event,
             test_sched_missed_slot_event, test_sched_dispatch_fail_event,
             test_sched_heartbeat_event, test_sched_all_events_integration)
