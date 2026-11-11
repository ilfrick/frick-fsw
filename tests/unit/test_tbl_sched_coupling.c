/*
 * frick-fsw - TBL load/activate → scheduler dispatch coupling test (N22-TS5-66)
 *
 * Verifies the end-to-end chain: FF_TblLoad → FF_TblActivate →
 * FF_SchedActivate + FF_SchedInit → dispatcher uses new table.
 *
 * Cases 1-3 exercise the TBL hook contract itself with a test-local callback.
 * Cases 4-5 exercise the production wiring in FF_AppSchedulerMain
 * (src/apps/app_scheduler.c): a Table Services activation must reconfigure the
 * live dispatcher without the callback touching single-writer dispatcher state,
 * and an invalid image must leave the active schedule untouched.
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
#include <pthread.h>
#include <string.h>

/* Test application ID */
#define FF_TEST_APP_ID 1u

/* EVS event capture buffer */
#define FF_TEST_EVS_CAP 64u
static FF_EvsEvent_t g_captured_events[FF_TEST_EVS_CAP] __attribute__((used));
static uint16_t g_captured_count __attribute__((used));

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

/* Test schedule image with 2 entries */
static FF_SchedTable_t g_test_sched_image;
static FF_TblHandle_t g_test_tbl_handle = FF_TBL_INVALID_HANDLE;

/* Initialize test schedule image */
static void init_test_sched_image(uint32_t period1_ms, uint32_t period2_ms)
{
    FF_SchedTable_t t;
    memset(&t, 0, sizeof(t));
    t.magic = FF_SCHED_MAGIC;
    t.count = 2u;
    t.entries[0].msg_id = 0x0B01u;
    t.entries[0].period_ms = period1_ms;
    t.entries[0].payload_len = 1u;
    t.entries[0].payload[0] = 0x11u;
    t.entries[1].msg_id = 0x0B02u;
    t.entries[1].period_ms = period2_ms;
    t.entries[1].payload_len = 1u;
    t.entries[1].payload[1] = 0x22u;
    g_test_sched_image = t;
}

/* TBL activation callback for test */
static volatile bool g_cb_called = false;

static __attribute__((used)) void test_tbl_activate_cb(FF_TblHandle_t handle, const void *image,
                                                       size_t size)
{
    (void)handle;
    g_cb_called = true;
    const FF_SchedTable_t *sched = (const FF_SchedTable_t *)image;
    FF_Status_t st = FF_SchedActivate(sched, size);
    if (st != FF_SUCCESS) {
        /* Error - but we can't send EVS from callback context easily */
    }
    else {
        FF_SchedInit(FF_PalGetMonotonicNs());
    }
}

/* Initialize test environment */
static void test_init(void)
{
    reset_capture();
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);
    FF_SchedInit(0u); /* Initialize with empty table first */
    FF_SchedResetStats();
    FF_EvsSetListener(test_evs_listener);
    FF_PalMonotonicTestSet(0u);
    FF_SchedTestResetExit();
}

/* Cleanup test environment */
static void test_cleanup(void)
{
    FF_EvsSetListener(NULL);
    FF_PalMonotonicTestUnfreeze();
}

/* Test 1: TBL load/activate triggers scheduler re-initialization */
__attribute__((used)) void test_tbl_sched_coupling_basic(void)
{
    test_init();

    /* Register test schedule table with activation callback */
    init_test_sched_image(1000u, 2000u);
    g_test_tbl_handle = FF_TblRegister(&(FF_Tbl_Def_t){.name = "test_sched",
                                                       .image = &g_test_sched_image,
                                                       .size = sizeof(FF_SchedTable_t),
                                                       .validate = FF_SchedValidate,
                                                       .on_activate = test_tbl_activate_cb});
    FF_TEST_ASSERT(g_test_tbl_handle != FF_TBL_INVALID_HANDLE);

    /* Load the schedule image */
    FF_TEST_ASSERT(FF_TblLoad(g_test_tbl_handle, &g_test_sched_image, sizeof(FF_SchedTable_t)) ==
                   FF_SUCCESS);

    /* Activate - should trigger callback → FF_SchedActivate + FF_SchedInit */
    FF_TEST_ASSERT(FF_TblActivate(g_test_tbl_handle) == FF_SUCCESS);
    FF_TEST_ASSERT(g_cb_called);

    /* Verify scheduler is now active with the new table */
    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);

    /* Advance time and poll - should dispatch entries from the new table */
    FF_PalMonotonicTestSet(1000ULL * 1000000ULL); /* 1 second */
    uint64_t now = FF_PalGetMonotonicNs();
    (void)FF_SchedPoll(now, NULL, (uint16_t)FF_SCHED_MAX_ENTRIES, FF_TEST_APP_ID);

    test_cleanup();
}

/* Test 2: TBL revert does not affect active scheduler until next activate */
__attribute__((used)) void test_tbl_sched_coupling_revert(void)
{
    test_init();

    init_test_sched_image(1000u, 2000u);
    g_test_tbl_handle = FF_TblRegister(&(FF_Tbl_Def_t){.name = "test_sched2",
                                                       .image = &g_test_sched_image,
                                                       .size = sizeof(FF_SchedTable_t),
                                                       .validate = FF_SchedValidate,
                                                       .on_activate = test_tbl_activate_cb});
    FF_TEST_ASSERT(g_test_tbl_handle != FF_TBL_INVALID_HANDLE);

    /* Load and activate initial table */
    FF_TEST_ASSERT(FF_TblLoad(g_test_tbl_handle, &g_test_sched_image, sizeof(FF_SchedTable_t)) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblActivate(g_test_tbl_handle) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);

    /* Create a different schedule image */
    FF_SchedTable_t alt_sched;
    memset(&alt_sched, 0, sizeof(alt_sched));
    alt_sched.magic = FF_SCHED_MAGIC;
    alt_sched.count = 1u;
    alt_sched.entries[0].msg_id = 0x0C01u;
    alt_sched.entries[0].period_ms = 500u;
    alt_sched.entries[0].payload_len = 1u;
    alt_sched.entries[0].payload[0] = 0x33u;

    /* Load alternative image but don't activate */
    FF_TEST_ASSERT(FF_TblLoad(g_test_tbl_handle, &alt_sched, sizeof(FF_SchedTable_t)) ==
                   FF_SUCCESS);

    /* Revert - should restore original image in staging but not affect active scheduler */
    FF_TEST_ASSERT(FF_TblRevert(g_test_tbl_handle) == FF_SUCCESS);

    /* Active scheduler should still have original 2 entries */
    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);

    /* Revert clears staged flag; must load again before activate */
    FF_TEST_ASSERT(FF_TblLoad(g_test_tbl_handle, &g_test_sched_image, sizeof(FF_SchedTable_t)) ==
                   FF_SUCCESS);

    /* Now activate the reverted table (which is the original) */
    FF_TEST_ASSERT(FF_TblActivate(g_test_tbl_handle) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);

    test_cleanup();
}

/* Test 3: Invalid table validation rejects activation */
__attribute__((used)) void test_tbl_sched_coupling_invalid(void)
{
    test_init();

    init_test_sched_image(1000u, 2000u);
    g_test_tbl_handle = FF_TblRegister(&(FF_Tbl_Def_t){.name = "test_sched3",
                                                       .image = &g_test_sched_image,
                                                       .size = sizeof(FF_SchedTable_t),
                                                       .validate = FF_SchedValidate,
                                                       .on_activate = test_tbl_activate_cb});
    FF_TEST_ASSERT(g_test_tbl_handle != FF_TBL_INVALID_HANDLE);

    /* Create invalid schedule (bad magic) */
    FF_SchedTable_t bad_sched = g_test_sched_image;
    bad_sched.magic = 0xDEADBEEFu;

    /* Load bad image - validation should fail */
    FF_TEST_ASSERT(FF_TblLoad(g_test_tbl_handle, &bad_sched, sizeof(FF_SchedTable_t)) == FF_ERROR);

    /* Activation should fail because staging failed */
    FF_TEST_ASSERT(FF_TblActivate(g_test_tbl_handle) == FF_NOT_READY);

    test_cleanup();
}

/* ---- Production-path harness (FF_AppSchedulerMain) -------------------- */

static pthread_t g_sched_thread;
static bool g_sched_thread_running;

static void *sched_thread_entry(void *arg)
{
    (void)arg;
    g_sched_thread_running = true;
    FF_AppSchedulerMain((void *)(uintptr_t)FF_TEST_APP_ID);
    g_sched_thread_running = false;
    return NULL;
}

/* Block until the scheduler task enters a fresh controlled delay, i.e. the
 * previous loop iteration (delay → now → apply pending → poll) has completed.
 * Every entry is consumed once, so injected time is observed by exactly one
 * iteration and consecutive steps cannot be coalesced. */
static void sync_scheduler(void)
{
    FF_PalTaskDelayMsTestWait();
}

/* Advance the frozen monotonic clock by advance_ms and release the scheduler
 * task for one loop iteration. The caller must be synchronised (see
 * sync_scheduler) before stepping. */
static void step_scheduler(uint32_t advance_ms)
{
    uint64_t current = FF_PalGetMonotonicNs();

    FF_PalMonotonicTestSet(current + ((uint64_t)advance_ms * 1000000ULL));
    FF_PalTaskDelayMsTestStep();
}

/* Run exactly one scheduler iteration and wait for it to complete. */
static void run_scheduler_tick(uint32_t advance_ms)
{
    step_scheduler(advance_ms);
    sync_scheduler();
}

static void start_scheduler_thread(void)
{
    FF_PalTaskDelayMsTestEnable();
    FF_TEST_ASSERT(pthread_create(&g_sched_thread, NULL, sched_thread_entry, NULL) == 0);
    /* Wait until the application has completed startup (table registration and
     * initial activation) and is blocked in its first controlled delay. */
    sync_scheduler();
}

static void stop_scheduler_thread(void)
{
    FF_SchedTestRequestExit();
    FF_PalTaskDelayMsTestStep();
    FF_TEST_ASSERT(pthread_join(g_sched_thread, NULL) == 0);
    FF_SchedTestResetExit();
    FF_PalTaskDelayMsTestDisable();
}

/* Build a two-entry schedule image for the production path. */
static void build_two_entry_image(FF_SchedTable_t *t)
{
    memset(t, 0, sizeof(*t));
    t->magic = FF_SCHED_MAGIC;
    t->count = 2u;
    t->entries[0].msg_id = 0x0A11u;
    t->entries[0].period_ms = 1000u;
    t->entries[0].payload_len = 1u;
    t->entries[0].payload[0] = 0x21u;
    t->entries[1].msg_id = 0x0A12u;
    t->entries[1].period_ms = 2000u;
    t->entries[1].payload_len = 1u;
    t->entries[1].payload[0] = 0x22u;
}

/* Test 4: a Table Services activation reconfigures the live dispatcher */
__attribute__((used)) void test_tbl_sched_production_activate(void)
{
    FF_SchedTable_t next;
    FF_SchedStats_t st_before;
    FF_SchedStats_t st_after;
    FF_TblHandle_t handle;

    test_init();
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);

    /* Startup path activated the project reference schedule (4 entries). */
    handle = FF_SchedTestTableHandle();
    FF_TEST_ASSERT(handle != FF_TBL_INVALID_HANDLE);
    FF_TEST_ASSERT(FF_SchedActiveCount() == 4u);

    build_two_entry_image(&next);
    FF_TEST_ASSERT(FF_TblLoad(handle, &next, sizeof(next)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblActivate(handle) == FF_SUCCESS);

    /* The callback must not touch single-writer dispatcher state: the swap is
     * deferred to the scheduler task's next tick. */
    FF_TEST_ASSERT(FF_SchedActiveCount() == 4u);

    reset_capture();
    run_scheduler_tick(FF_SCHED_TICK_MS);

    FF_TEST_ASSERT(FF_SchedActiveCount() == 2u);
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_ACTIVATED);
    FF_TEST_ASSERT(evt != NULL);
    FF_TEST_ASSERT(evt->type == FF_EVS_INFO);
    FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
    FF_TEST_ASSERT(strcmp(evt->text, "SCH: table activated, 2 entries") == 0);
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_ACTIVATED) == 1u);

    /* Dispatch now follows the new table: only the 1000 ms entry is due after
     * one 1000 ms tick, and its release target restarted at the apply instant. */
    FF_SchedGetStats(&st_before);
    run_scheduler_tick(1000u);
    FF_SchedGetStats(&st_after);
    FF_TEST_ASSERT((st_after.dispatched - st_before.dispatched) == 1u);

    stop_scheduler_thread();
    test_cleanup();
}

/* Test 5: invalid/out-of-bound images leave the active schedule untouched */
__attribute__((used)) void test_tbl_sched_production_invalid(void)
{
    FF_SchedTable_t bad;
    FF_TblHandle_t handle;

    test_init();
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);

    handle = FF_SchedTestTableHandle();
    FF_TEST_ASSERT(handle != FF_TBL_INVALID_HANDLE);
    FF_TEST_ASSERT(FF_SchedActiveCount() == 4u);

    /* Wrong length is rejected by the table registry. */
    build_two_entry_image(&bad);
    FF_TEST_ASSERT(FF_TblLoad(handle, &bad, sizeof(bad) - 1u) == FF_BAD_SIZE);

    /* Bad magic is rejected by FF_SchedValidate during load. */
    bad.magic = 0xDEADBEEFu;
    FF_TEST_ASSERT(FF_TblLoad(handle, &bad, sizeof(bad)) == FF_ERROR);

    /* Nothing valid is staged, so activation is refused. */
    FF_TEST_ASSERT(FF_TblActivate(handle) == FF_NOT_READY);

    reset_capture();
    run_scheduler_tick(FF_SCHED_TICK_MS);

    /* Dispatcher still runs the previously activated table, and no
     * re-activation event was emitted. */
    FF_TEST_ASSERT(FF_SchedActiveCount() == 4u);
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_ACTIVATED) == 0u);

    stop_scheduler_thread();
    test_cleanup();
}

/* Explicit forward declarations for FF_TEST_MAIN */
__attribute__((used)) void test_tbl_sched_coupling_basic(void);
__attribute__((used)) void test_tbl_sched_coupling_revert(void);
__attribute__((used)) void test_tbl_sched_coupling_invalid(void);
__attribute__((used)) void test_tbl_sched_production_activate(void);
__attribute__((used)) void test_tbl_sched_production_invalid(void);

FF_TEST_MAIN(test_tbl_sched_coupling_basic, test_tbl_sched_coupling_revert,
             test_tbl_sched_coupling_invalid, test_tbl_sched_production_activate,
             test_tbl_sched_production_invalid);
