/*
 * frick-fsw - EVS/HS health-path test (N22 test design; scope stated below)
 *
 * What THIS test verifies:
 *   1. the EVS listener captures the HS application-failure event when HS
 *      detects an application in the FAILED state;
 *   2. the direct PUS-C service 5 emission path
 *      (FF_PusEmitEvent -> CCSDS TM -> TM sink -> link loopback), including
 *      INFO/WARNING/CRITICAL subservice selection and payload transparency;
 *   3. nominal and out-of-bound behaviour of FF_PusEmitEvent (oversized
 *      payload, null data with non-zero length, unknown severity mapping).
 *
 * What this test does NOT verify:
 *   - the ASSEMBLED listener -> PUS service 5 -> production TM sink -> link
 *     chain. The listener does forward (src/main/fsw_main.c installs
 *     ff_event_listener, which calls FF_PusEmitEvent), and listener -> sink is
 *     covered by test_r3_gap.c:523-575; this test installs its own sink.
 *     **EVIDENCE DELIVERED**: `test_tm_output_chain.c` (47 checks) now exercises
 *     the assembled chain through the production `FF_AppTmOutputInit`/`ff_to_sink`
 *     pair, closing the NC-EXT-236 gap (NC remains open pending human PA
 *     disposition).
 *   - the scheduler heartbeat / late-missed-slot / dispatch-failure events
 *     (N22-TS5-47); tracked as NC-EXT-19.
 *
 * Known product observation (NC-EXT-237): FF_PusEmitEvent ignores its
 * event_id argument; event identity is carried by the caller-supplied
 * payload. This test therefore asserts payload transparency, not
 * event-identity semantics of the argument.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include "core/es/es.h"
#include "apps/app.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"
#include "transport/link/link.h"
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Independent oracle: capture the EVS event stream at the listener    */
/* seam (the production observation point for HS output).              */
/* ------------------------------------------------------------------ */

#define HS_CAP_MAX 64u

static FF_EvsEvent_t g_cap[HS_CAP_MAX];
static uint16_t g_cap_count;

static void cap_reset(void)
{
    (void)memset(g_cap, 0, sizeof(g_cap));
    g_cap_count = 0u;
}

static uint16_t cap_count_id_type(uint16_t event_id, uint8_t type)
{
    uint16_t n = 0u;
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if ((g_cap[i].event_id == event_id) && (g_cap[i].type == type)) {
            n++;
        }
    }
    return n;
}

/* True when a captured CRITICAL application-error event reports app id `id`
 * ("HS: app <n> failed"). */
static bool cap_hs_reports(uint16_t app_id)
{
    char needle[32];
    int n = snprintf(needle, sizeof(needle), "app %u failed", (unsigned)app_id);
    if ((n < 0) || ((size_t)n) >= sizeof(needle)) {
        return false;
    }
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if ((g_cap[i].event_id == FF_EVS_APP_ERROR) && (g_cap[i].type == FF_EVS_CRITICAL) &&
            (strstr(g_cap[i].text, needle) != NULL)) {
            return true;
        }
    }
    return false;
}

/* Listener used only to capture events for the oracle. It performs no TM
 * forwarding. The listener -> TM sink segment is exercised by
 * test_r3_gap.c:523-575, not by this test. This test installs its own sink
 * that forwards to the loopback link, so the injected sink -> link segment is
 * covered here; what no host test covers is the assembled chain through the
 * production FF_AppTmOutputInit/ff_to_sink pair (NC-EXT-236). */
static void hs_capture_listener(const FF_EvsEvent_t *evt)
{
    if (g_cap_count < HS_CAP_MAX) {
        g_cap[g_cap_count] = *evt;
        g_cap_count++;
    }
}

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

static void hs_dummy_entry(void *arg)
{
    (void)arg;
}

static uint16_t hs_fill_pal_slots(void)
{
    uint16_t created = 0u;
    FF_TaskId_t tid = 0u;

    for (uint16_t i = 0u; i <= 64u; i++) {
        if (FF_PalTaskCreate(&tid, "filler", hs_dummy_entry, NULL, 0u, 0u) != FF_SUCCESS) {
            return created;
        }
        created++;
    }
    return created;
}

static void hs_init_modules(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsInit() == FF_SUCCESS);
    FF_EvsSetListener(hs_capture_listener);
    cap_reset();
}

/* ------------------------------------------------------------------ */
/* TM sink capture for the PUS service 5 path. Also forwards to link.  */
/* ------------------------------------------------------------------ */
static uint8_t g_evs_tm_frame[1024];
static uint16_t g_evs_tm_frame_len;
static uint32_t g_evs_tm_count;

static void evs_tm_capture_and_forward(const uint8_t *frame, uint16_t len)
{
    if (len > sizeof(g_evs_tm_frame)) {
        return;
    }
    (void)memcpy(g_evs_tm_frame, frame, len);
    g_evs_tm_frame_len = len;
    g_evs_tm_count++;
    /* Forward to link (flight path). */
    (void)FF_LinkSend(frame, len);
}

/* Decode a CCSDS TM frame and report PUS service/subservice plus a bounded
 * copy of the payload (NUL-terminated, never reads past `len`). */
static bool decode_tm(const uint8_t *frame, uint16_t len, uint8_t *service, uint8_t *subservice,
                      char *payload_text, size_t payload_text_size, uint16_t *payload_len_out)
{
    uint16_t apid;
    FF_Pus_TmHdr_t hdr;
    const uint8_t *payload;
    uint16_t payload_len;

    if (FF_CcsdsDecodeTm(frame, len, &apid, &hdr, &payload, &payload_len) != FF_SUCCESS) {
        return false;
    }
    if (payload_len >= payload_text_size) {
        return false;
    }
    (void)memcpy(payload_text, payload, payload_len);
    payload_text[payload_len] = '\0';
    *service = hdr.service;
    *subservice = hdr.subservice;
    *payload_len_out = payload_len;
    return true;
}

/* Verify an event-report TM frame: service 5, expected subservice, event id in
 * the first two payload bytes and the failed app id in the text. */
static bool verify_hs_failure_tm(const uint8_t *frame, uint16_t len, uint8_t expected_subservice,
                                 uint16_t failed_app_id)
{
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    uint16_t payload_len = 0u;
    char payload_text[FF_PUS_MAX_TM_FRAME + 1u];
    char needle[32];

    if (!decode_tm(frame, len, &service, &subservice, payload_text, sizeof(payload_text),
                   &payload_len)) {
        return false;
    }
    if (service != 5u || subservice != expected_subservice) {
        return false;
    }
    if (payload_len < 4u) {
        return false;
    }
    uint16_t tm_event_id = (uint16_t)(((uint8_t)payload_text[0] << 8) | (uint8_t)payload_text[1]);
    if (tm_event_id != FF_EVS_APP_ERROR) {
        return false;
    }
    int n = snprintf(needle, sizeof(needle), "app %u failed", (unsigned)failed_app_id);
    if ((n < 0) || ((size_t)n) >= sizeof(needle)) {
        return false;
    }
    return (strstr(payload_text, needle) != NULL);
}

/* Build the flight-format event payload: event_id (2 bytes BE), type, text. */
static uint16_t build_hs_payload(uint8_t *payload, size_t payload_size, uint16_t app_id)
{
    char text[32];
    int n = snprintf(text, sizeof(text), "app %u failed", (unsigned)app_id);
    if ((n < 0) || ((size_t)n >= sizeof(text)) || ((size_t)(3 + n) > payload_size)) {
        return 0u;
    }
    payload[0] = (uint8_t)(FF_EVS_APP_ERROR >> 8);
    payload[1] = (uint8_t)(FF_EVS_APP_ERROR & 0xFFu);
    payload[2] = FF_EVS_CRITICAL;
    (void)memcpy(&payload[3], text, (size_t)n);
    return (uint16_t)(3 + n);
}

/* ------------------------------------------------------------------ */
/* Test 1: EVS listener captures HS failure events                    */
/* ------------------------------------------------------------------ */

static void test_evs_listener_captures_hs_failure(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t bad_id = FF_INVALID_ID;
    FF_AppId_t hs_id = FF_INVALID_ID;
    uint16_t fillers;

    FF_PalTaskDelayMs(20u);
    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "bad_app";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &bad_id) == FF_SUCCESS);

    cfg.name = "hs_app";
    cfg.entry = FF_AppHsMain;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &hs_id) == FF_SUCCESS);

    fillers = hs_fill_pal_slots();
    /* Evidence that the PAL task table is actually exhausted, without binding
     * this oracle to the file-local FF_PAL_MAX_TASKS value in pal_posix.c: the
     * public contract says a create with no free slot returns FF_NO_RESOURCE. */
    FF_TEST_ASSERT(fillers < 64u);
    {
        FF_TaskId_t overflow_id = 0u;
        FF_TEST_ASSERT(FF_PalTaskCreate(&overflow_id, "overflow", hs_dummy_entry, NULL, 0u, 0u) ==
                       FF_NO_RESOURCE);
    }

    cap_reset();
    FF_TEST_ASSERT(FF_EsStartApp(bad_id) == FF_ERROR);
    FF_TEST_ASSERT(FF_EsGetState(bad_id) == FF_APP_FAILED);

    FF_AppHsStep(hs_id);

    FF_TEST_ASSERT(cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u);
    FF_TEST_ASSERT(cap_hs_reports(bad_id));

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* Test 2: Direct PUS service 5 event emission -> TM sink -> link     */
/* ------------------------------------------------------------------ */

static void test_pus_event_direct_path(void)
{
    uint8_t rx[1024];
    uint16_t rx_len;

    FF_PalTaskDelayMs(20u);
    hs_init_modules();

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(evs_tm_capture_and_forward);

    FF_TEST_ASSERT(FF_LinkInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkGet() == &FF_LinkLoopbackBackend);

    for (uint16_t app_id = 1u; app_id <= 2u; app_id++) {
        uint8_t payload[32];
        uint16_t payload_len = build_hs_payload(payload, sizeof(payload), app_id);
        g_evs_tm_count = 0u;
        g_evs_tm_frame_len = 0u;

        FF_TEST_ASSERT(payload_len > 3u);
        FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, payload, payload_len) ==
                       FF_SUCCESS);
        FF_TEST_ASSERT(g_evs_tm_count == 1u);
        FF_TEST_ASSERT(verify_hs_failure_tm(g_evs_tm_frame, g_evs_tm_frame_len, 3u, app_id));

        FF_TEST_ASSERT(FF_LinkLoopbackPop(rx, sizeof(rx), &rx_len) == FF_SUCCESS);
        FF_TEST_ASSERT(rx_len == g_evs_tm_frame_len);
        FF_TEST_ASSERT(memcmp(rx, g_evs_tm_frame, rx_len) == 0);
    }

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* Test 3: severity -> subservice mapping (nominal and unknown)        */
/* ------------------------------------------------------------------ */

static void test_pus_event_severity_mapping(void)
{
    static const struct
    {
        uint8_t type;
        uint8_t expected_subservice;
    } cases[] = {
        {FF_EVS_INFO, 1u},
        {FF_EVS_WARNING, 2u},
        {FF_EVS_CRITICAL, 3u},
        {0xFFu, 3u}, /* unknown severity falls back to CRITICAL (documented) */
    };
    uint8_t payload[4] = {0x01u, 0x02u, 0x03u, 0x04u};
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    uint16_t payload_len = 0u;
    char text[FF_PUS_MAX_TM_FRAME + 1u];

    FF_PalTaskDelayMs(20u);
    hs_init_modules();

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(evs_tm_capture_and_forward);

    for (size_t i = 0u; i < (sizeof(cases) / sizeof(cases[0])); i++) {
        g_evs_tm_count = 0u;
        FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, cases[i].type, payload,
                                       (uint16_t)sizeof(payload)) == FF_SUCCESS);
        FF_TEST_ASSERT(g_evs_tm_count == 1u);
        FF_TEST_ASSERT(decode_tm(g_evs_tm_frame, g_evs_tm_frame_len, &service, &subservice, text,
                                 sizeof(text), &payload_len));
        FF_TEST_ASSERT(service == 5u);
        FF_TEST_ASSERT(subservice == cases[i].expected_subservice);
        FF_TEST_ASSERT(payload_len == sizeof(payload));
        FF_TEST_ASSERT(memcmp(text, payload, sizeof(payload)) == 0);
    }

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* Test 4: FF_PusEmitEvent out-of-bound behaviour                      */
/* ------------------------------------------------------------------ */

static void test_pus_event_out_of_bound(void)
{
    uint8_t payload[8] = {0};

    FF_PalTaskDelayMs(20u);
    hs_init_modules();

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);

    /* Payload budget is FF_PUS_MAX_TM_FRAME - 3; one byte over must be refused. */
    FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, payload,
                                   (uint16_t)(FF_PUS_MAX_TM_FRAME - 2u)) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, payload, UINT16_MAX) ==
                   FF_BAD_SIZE);
    /* Null data with non-zero length is an invalid argument. */
    FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, NULL, 4u) != FF_SUCCESS);
    /* Null data with zero length is accepted (empty report). */
    FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, NULL, 0u) == FF_SUCCESS);

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* Test 5: HS failure detection + direct PUS emission (integration)   */
/* ------------------------------------------------------------------ */

static void test_hs_failure_then_direct_pus_emission(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t bad_id = FF_INVALID_ID;
    FF_AppId_t hs_id = FF_INVALID_ID;
    uint16_t fillers;
    uint8_t rx[1024];
    uint16_t rx_len;

    FF_PalTaskDelayMs(20u);
    hs_init_modules();

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(evs_tm_capture_and_forward);

    FF_TEST_ASSERT(FF_LinkInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkGet() == &FF_LinkLoopbackBackend);

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "bad_app";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &bad_id) == FF_SUCCESS);

    cfg.name = "hs_app";
    cfg.entry = FF_AppHsMain;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &hs_id) == FF_SUCCESS);

    fillers = hs_fill_pal_slots();
    /* Evidence that the PAL task table is actually exhausted, without binding
     * this oracle to the file-local FF_PAL_MAX_TASKS value in pal_posix.c: the
     * public contract says a create with no free slot returns FF_NO_RESOURCE. */
    FF_TEST_ASSERT(fillers < 64u);
    {
        FF_TaskId_t overflow_id = 0u;
        FF_TEST_ASSERT(FF_PalTaskCreate(&overflow_id, "overflow", hs_dummy_entry, NULL, 0u, 0u) ==
                       FF_NO_RESOURCE);
    }

    cap_reset();
    FF_TEST_ASSERT(FF_EsStartApp(bad_id) == FF_ERROR);
    FF_TEST_ASSERT(FF_EsGetState(bad_id) == FF_APP_FAILED);

    for (uint16_t pass = 1u; pass <= 2u; pass++) {
        uint8_t payload[32];
        uint16_t payload_len = build_hs_payload(payload, sizeof(payload), bad_id);

        /* HS detects the failure at the listener seam. */
        cap_reset();
        FF_AppHsStep(hs_id);
        FF_TEST_ASSERT(cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u);
        FF_TEST_ASSERT(cap_hs_reports(bad_id));

        /* Direct PUS emission of the same event (listener forwarding is
         * NC-EXT-236, not exercised here). */
        g_evs_tm_count = 0u;
        g_evs_tm_frame_len = 0u;
        FF_TEST_ASSERT(payload_len > 3u);
        FF_TEST_ASSERT(FF_PusEmitEvent(FF_EVS_APP_ERROR, FF_EVS_CRITICAL, payload, payload_len) ==
                       FF_SUCCESS);
        FF_TEST_ASSERT(g_evs_tm_count == 1u);
        FF_TEST_ASSERT(verify_hs_failure_tm(g_evs_tm_frame, g_evs_tm_frame_len, 3u, bad_id));

        FF_TEST_ASSERT(FF_LinkLoopbackPop(rx, sizeof(rx), &rx_len) == FF_SUCCESS);
        FF_TEST_ASSERT(rx_len == g_evs_tm_frame_len);
        FF_TEST_ASSERT(memcmp(rx, g_evs_tm_frame, rx_len) == 0);
    }

    FF_EvsSetListener(NULL);
}

FF_TEST_MAIN(test_evs_listener_captures_hs_failure, test_pus_event_direct_path,
             test_pus_event_severity_mapping, test_pus_event_out_of_bound,
             test_hs_failure_then_direct_pus_emission)
