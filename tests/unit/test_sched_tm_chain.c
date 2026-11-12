/*
 * frick-fsw - N22-TS5-47: Scheduler EVS events end-to-end through production TM chain
 *
 * Verifies end-to-end delivery of scheduler heartbeat/missed-slot/dispatch-failure
 * EVS events to the health-monitoring path (PUS service 5 TM via production sink):
 *
 *   FF_AppSchedulerMain → FF_EvsSend → chain_listener → FF_PusEmitEvent
 *   → PUS S5 encoding → FF_AppTmOutputInit/ff_to_sink → FF_LinkSend
 *   → link loopback → decode and assert exact event_id/type/app/payload
 *
 * Strict TDD: written first (RED), then only the smallest missing production
 * software behavior is implemented for GREEN. No hardware touched. No invented
 * mission rates or recovery policy.
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
#include "core/time/time.h"
#include "pus/pus.h"
#include "transport/link/link.h"
#include "transport/ccsds/ccsds.h"
#include "sched_evt.h"
#include <pthread.h>
#include <string.h>

#define FF_TEST_APP_ID 1u
#define FF_TEST_EVS_CAP 64u

/* Captured EVS events (app_id visible here; stripped from S5 TM payload). */
static FF_EvsEvent_t g_chain_captured[FF_TEST_EVS_CAP];
static uint16_t g_chain_count;

static pthread_t g_sched_thread;

/* Hybrid listener: captures the EVS event (for app_id assertion) and forwards
 * to the production S5 TM chain (for end-to-end health-monitoring-path
 * verification). Mirrors ff_event_listener in src/main/fsw_main.c. */
static void chain_listener(const FF_EvsEvent_t *evt)
{
    if (g_chain_count < FF_TEST_EVS_CAP) {
        g_chain_captured[g_chain_count++] = *evt;
    }
    uint16_t text_len = 0u;
    while ((text_len < sizeof(evt->text)) && (evt->text[text_len] != '\0')) {
        text_len++;
    }
    (void)FF_PusEmitEvent(evt->event_id, evt->type, (const uint8_t *)evt->text, text_len);
}

static bool pop_tm_frame(uint8_t *frame, uint16_t cap, uint16_t *len)
{
    return FF_LinkLoopbackPop(frame, cap, len) == FF_SUCCESS;
}

static bool find_s5_frame(const uint8_t *frame, uint16_t len, uint16_t *out_event_id,
                          uint8_t *out_type, const uint8_t **out_text, uint16_t *out_text_len)
{
    /* Manual CCSDS TM frame parse: primary header (6 bytes) at [0-5],
     * packet data length - 1 at [4-5], PUS TM secondary header (13 bytes)
     * at [6-18], data = event_id[2 BE] + type[1] + text[N] at [19-],
     * 2-byte CRC trailer. */
    if (len < FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN + 3u + 2u) {
        return false;
    }
    uint16_t pdu_len_minus_1 = (uint16_t)((frame[4] << 8) | frame[5]);
    uint16_t total = (uint16_t)(FF_CCSDS_HDR_LEN + pdu_len_minus_1 + 1u);
    if (total > len) {
        return false;
    }
    /* Secondary header: byte 7 = service, byte 8 = subservice. */
    if (frame[7] != 5u || frame[8] < 1u || frame[8] > 3u) {
        return false;
    }
    const uint8_t *d = &frame[FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN];
    uint16_t dl = (uint16_t)(total - FF_CCSDS_HDR_LEN - FF_PUS_TM_SEC_HDR_LEN - 2u);
    if (dl < 3u) {
        return false;
    }
    *out_event_id = (uint16_t)((d[0] << 8) | d[1]);
    *out_type = d[2];
    *out_text = &d[3];
    *out_text_len = (uint16_t)(dl - 3u);
    return true;
}

static const FF_EvsEvent_t *find_event(uint16_t event_id)
{
    for (uint16_t i = 0u; i < g_chain_count; i++) {
        if (g_chain_captured[i].event_id == event_id) {
            return &g_chain_captured[i];
        }
    }
    return NULL;
}

static uint16_t count_event(uint16_t event_id)
{
    uint16_t n = 0u;
    for (uint16_t i = 0u; i < g_chain_count; i++) {
        if (g_chain_captured[i].event_id == event_id) {
            n++;
        }
    }
    return n;
}

static void reset_capture(void)
{
    g_chain_count = 0u;
}

static void *sched_thread_entry(void *arg)
{
    (void)arg;
    FF_AppSchedulerMain((void *)(uintptr_t)FF_TEST_APP_ID);
    return NULL;
}

static void start_scheduler_thread(void)
{
    FF_PalTaskDelayMsTestEnable();
    FF_TEST_ASSERT(pthread_create(&g_sched_thread, NULL, sched_thread_entry, NULL) == 0);
    struct timespec ts = {0, 10000000L};
    nanosleep(&ts, NULL);
}

static void stop_scheduler_thread(void)
{
    FF_SchedTestRequestExit();
    FF_PalTaskDelayMsTestStep();
    FF_TEST_ASSERT(pthread_join(g_sched_thread, NULL) == 0);
    FF_SchedTestResetExit();
    FF_PalTaskDelayMsTestDisable();
}

static void advance_time_ms(uint32_t ms)
{
    uint64_t current = FF_PalGetMonotonicNs();
    FF_PalMonotonicTestSet(current + (uint64_t)ms * 1000000ULL);
}

static void run_scheduler_tick(uint32_t advance_ms)
{
    FF_PalTaskDelayMsTestWait();
    advance_time_ms(advance_ms);
    FF_PalTaskDelayMsTestStep();
}

static void test_chain_init(bool init_sb)
{
    reset_capture();
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkInit() == FF_SUCCESS);
    if (init_sb) {
        FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    }
    FF_SchedInit(0u);
    FF_SchedResetStats();
    FF_EvsSetListener(chain_listener);
    FF_PalMonotonicTestSet(0u);
    FF_SchedTestResetExit();
    /* Install production TM sink (closes N22-TS5-47 gap: EVS→S5→TM sink→link) */
    FF_TEST_ASSERT(FF_AppTmOutputInit() == FF_SUCCESS);
    /* Drain link loopback */
    uint8_t frame[1024];
    uint16_t len = 0u;
    while (FF_LinkLoopbackPop(frame, sizeof(frame), &len) == FF_SUCCESS) {
    }
}

static void test_chain_cleanup(void)
{
    FF_EvsSetListener(NULL);
    FF_PalMonotonicTestUnfreeze();
}

/* Pop all bytes from the link loopback (concatenated CCSDS frames), parse
 * individual frames using the packet data length field (bytes 6-7), and check
 * that one matches the expected event_id/type/text. */
static bool find_s5_event(uint16_t expected_event_id, uint8_t expected_type,
                          const char *expected_text)
{
    uint8_t buf[2048];
    uint16_t buf_len = 0u;

    if (!pop_tm_frame(buf, sizeof(buf), &buf_len)) {
        return false;
    }

    uint16_t offset = 0u;
    while (offset + FF_CCSDS_HDR_LEN <= buf_len) {
        /* CCSDS primary header: packet data length - 1 at bytes 4-5 (big-endian);
         * total frame = 6 + (that value + 1). */
        uint16_t pdu_len_minus_1 = (uint16_t)((buf[offset + 4] << 8) | buf[offset + 5]);
        uint16_t frame_len = (uint16_t)(FF_CCSDS_HDR_LEN + pdu_len_minus_1 + 1u);
        if (offset + frame_len > buf_len) {
            break; /* incomplete trailing frame */
        }

        uint16_t event_id = 0u;
        uint8_t type = 0u;
        const uint8_t *text = NULL;
        uint16_t text_len = 0u;
        if (find_s5_frame(&buf[offset], frame_len, &event_id, &type, &text, &text_len)) {
            if ((event_id == expected_event_id) && (type == expected_type)) {
                if (expected_text != NULL) {
                    if ((text_len == (uint16_t)strlen(expected_text)) &&
                        (memcmp(text, expected_text, text_len) == 0)) {
                        return true;
                    }
                }
                else {
                    return true; /* match on id+type only */
                }
            }
        }

        offset += frame_len;
    }

    return false;
}

/* Test 1: heartbeat event (0x0604, INFO, "SCH heartbeat") through production TM chain */
static void test_sched_tm_chain_heartbeat(void)
{
    test_chain_init(true); /* SB init: heartbeat doesn't need publish to succeed */
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS);      /* now=100ms, startup, no heartbeat */
    run_scheduler_tick(FF_SCHED_HEARTBEAT_MS); /* now=5100ms, elapsed>=5000, heartbeat */
    stop_scheduler_thread();

    /* Assert at EVS listener level (app_id visible here) */
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_HEARTBEAT);
    FF_TEST_ASSERT(evt != NULL);
    if (evt != NULL) {
        FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
        FF_TEST_ASSERT(evt->type == FF_EVS_INFO);
        FF_TEST_ASSERT(strcmp(evt->text, FF_SCHED_EVT_HEARTBEAT_FMT) == 0);
    }

    /* Assert at S5 TM level (end-to-end health-monitoring path) */
    FF_TEST_ASSERT(
        find_s5_event(FF_SCHED_EVT_HEARTBEAT, (uint8_t)FF_EVS_INFO, FF_SCHED_EVT_HEARTBEAT_FMT));

    test_chain_cleanup();
}

/* Test 2: missed-slot event (0x0603, WARNING) through production TM chain */
static void test_sched_tm_chain_missed_slot(void)
{
    test_chain_init(true);
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS); /* now=100ms, startup */
    run_scheduler_tick(4500u);            /* now=4600ms, slots missed */
    stop_scheduler_thread();

    /* Assert at EVS listener level */
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_MISSED_SLOT);
    FF_TEST_ASSERT(evt != NULL);
    if (evt != NULL) {
        FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
        FF_TEST_ASSERT(evt->type == FF_EVS_WARNING);
        FF_TEST_ASSERT(strstr(evt->text, "3 release slots skipped") != NULL);
        FF_TEST_ASSERT(strstr(evt->text, "total 3") != NULL);
    }

    /* Assert at S5 TM level */
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_MISSED_SLOT) == 1u);
    FF_TEST_ASSERT(find_s5_event(FF_SCHED_EVT_MISSED_SLOT, (uint8_t)FF_EVS_WARNING,
                                 "SCH: 3 release slots skipped (total 3)"));

    test_chain_cleanup();
}

/* Test 3: dispatch-failure event (0x0605, ERROR) through production TM chain.
 * Hold the SB bus mutex via the test hook so FF_SbPublish returns FF_TIMEOUT
 * on every publish → publish_errors increments → FF_SCHED_EVT_DISPATCH_FAIL
 * is emitted. */
static void test_sched_tm_chain_dispatch_fail(void)
{
    test_chain_init(true); /* SB initialized, bus mutex exists */
    /* Hold the bus mutex so the scheduler's FF_SbPublish cannot acquire it. */
    FF_TEST_ASSERT(FF_SbTestLock() == FF_SUCCESS);
    start_scheduler_thread();
    run_scheduler_tick(FF_SCHED_TICK_MS); /* now=100ms, startup, no dispatch */
    run_scheduler_tick(15000u);           /* now=15100ms, all entries due, publish fails */
    stop_scheduler_thread();
    FF_TEST_ASSERT(FF_SbTestUnlock() == FF_SUCCESS);

    /* Assert at EVS listener level */
    const FF_EvsEvent_t *evt = find_event(FF_SCHED_EVT_DISPATCH_FAIL);
    FF_TEST_ASSERT(evt != NULL);
    if (evt != NULL) {
        FF_TEST_ASSERT(evt->app_id == FF_TEST_APP_ID);
        FF_TEST_ASSERT(evt->type == FF_EVS_ERROR);
        FF_TEST_ASSERT(strstr(evt->text, "dispatch failures") != NULL);
    }

    /* Assert at S5 TM level */
    FF_TEST_ASSERT(count_event(FF_SCHED_EVT_DISPATCH_FAIL) == 1u);
    FF_TEST_ASSERT(find_s5_event(FF_SCHED_EVT_DISPATCH_FAIL, (uint8_t)FF_EVS_ERROR, NULL));

    test_chain_cleanup();
}

FF_TEST_MAIN(test_sched_tm_chain_heartbeat, test_sched_tm_chain_missed_slot,
             test_sched_tm_chain_dispatch_fail)
