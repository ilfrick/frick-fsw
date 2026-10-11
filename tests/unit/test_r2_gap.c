/*
 * frick-fsw - R2 coverage-gap unit tests (independent V&V, norm N9)
 *
 * Self-contained suite that closes the highest-value uncovered functions and
 * boundary/error branches identified by the R2 gap analysis recorded in
 * docs/reviews/2026-10-11-test-coverage-r2-report.md (FRICK-REVIEW-0075).
 *
 * Scope: pure-logic / registry core modules only - PAL registry surface,
 * Software Bus, Event Services, Executive Services, Table Services, Time,
 * the HAL registry and the CCSDS and PUS codecs. No hardware, no dynamic
 * memory, no test-order dependency (each routine re-initialises the module it
 * exercises).
 *
 * These tests assert already-documented contracts; they do not modify the
 * behaviour under test. Additive evidence for SRS-REQ-011 / SRS-REQ-014 and
 * the error/robustness clauses of the SB, TBL, HAL, CCSDS, PUS and Time
 * requirements.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include "core/es/es.h"
#include "core/tbl/tbl.h"
#include "core/time/time.h"
#if !defined(FF_TEST_NO_HAL)
#include "hal/hal.h"
#endif
#include "transport/ccsds/ccsds.h"
#include "pus/pus.h"
#include "pus/pus_types.h"
#if defined(FRICK_USE_LIBM) && (FRICK_USE_LIBM == 1)
#include "math/ff_math.h"
#include <math.h>
#endif
#include <string.h>

/* ------------------------------------------------------------------ */
/* Event Services (EVS): currently 0% covered - full module exercise   */
/* ------------------------------------------------------------------ */

static uint32_t g_evs_listener_calls;
static FF_EvsEvent_t g_evs_last;

static void evs_listener(const FF_EvsEvent_t *event)
{
    g_evs_listener_calls = g_evs_listener_calls + 1u;
    g_evs_last = *event;
}

static void test_evs_listener_and_ring(void)
{
    FF_EvsEvent_t out[FF_EVS_LOG_DEPTH];
    uint32_t n;
    uint32_t i;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);

    g_evs_listener_calls = 0u;
    memset(&g_evs_last, 0, sizeof(g_evs_last));
    FF_EvsSetListener(evs_listener);

    /* Formatted event reaches the listener with the correct fields. */
    FF_TEST_ASSERT(FF_EvsSend(0x1234u, FF_EVS_WARNING, 7u, "x=%u", 42u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_evs_listener_calls == 1u);
    FF_TEST_ASSERT(g_evs_last.event_id == 0x1234u);
    FF_TEST_ASSERT(g_evs_last.type == (uint8_t)FF_EVS_WARNING);
    FF_TEST_ASSERT(g_evs_last.app_id == 7u);
    FF_TEST_ASSERT(strcmp(g_evs_last.text, "x=42") == 0);
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == 1u);

    /* Single-event read while the log is not yet full (start == 0 branch). */
    n = FF_EvsGetLog(out, FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(n == 1u);
    FF_TEST_ASSERT(out[0].event_id == 0x1234u);

    /* Overrun the circular log by three entries; only the last DEPTH remain. */
    for (i = 0u; i < (FF_EVS_LOG_DEPTH + 3u); i++) {
        FF_TEST_ASSERT(FF_EvsSend((uint16_t)(0x1000u + i), FF_EVS_INFO, 1u, "e%u", i) ==
                       FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_EvsGetTotalCount() == (FF_EVS_LOG_DEPTH + 4u));
    n = FF_EvsGetLog(out, FF_EVS_LOG_DEPTH);
    FF_TEST_ASSERT(n == FF_EVS_LOG_DEPTH);
    /* total - DEPTH = 4; the oldest retained ring entry is index 4 = 0x1003. */
    FF_TEST_ASSERT(out[0].event_id == 0x1003u);

    /* max_events caps the copy; NULL destination returns 0 (no fault). */
    n = FF_EvsGetLog(out, 5u);
    FF_TEST_ASSERT(n == 5u);
    FF_TEST_ASSERT(FF_EvsGetLog(NULL, 5u) == 0u);

    /* A NULL listener disables forwarding without breaking the log. */
    FF_EvsSetListener(NULL);
    FF_TEST_ASSERT(FF_EvsSend(0x1235u, FF_EVS_INFO, 1u, "quiet") == FF_SUCCESS);
    FF_TEST_ASSERT(g_evs_listener_calls == (FF_EVS_LOG_DEPTH + 4u));
}

/* ------------------------------------------------------------------ */
/* Executive Services (ES): only simulator-smoke evidence today       */
/* ------------------------------------------------------------------ */

static void es_dummy_task(void *arg)
{
    (void)arg;
}

static void test_es_registry_and_lifecycle(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t id = FF_INVALID_ID;
    FF_Status_t rc;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetAppCount() == 0u);

    /* Argument and name validation (SRS-REQ-014). */
    FF_TEST_ASSERT(FF_EsRegisterApp(NULL, &id) == FF_INVALID_ARGUMENT);

    memset(&cfg, 0, sizeof(cfg));
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &id) == FF_INVALID_ARGUMENT); /* name + entry NULL */
    cfg.name = "app_one";
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &id) == FF_INVALID_ARGUMENT); /* entry NULL */
    cfg.entry = es_dummy_task;
    cfg.name = "app_name_is_too_long"; /* >= FF_ES_APP_NAME_LEN (16) */
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &id) == FF_NAME_TOO_LONG);

    /* Valid registration: ids are 1-based, state LOADED, name resolvable. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.name = "app_one";
    cfg.entry = es_dummy_task;
    cfg.stack_bytes = 4096u;
    cfg.priority = 1u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &id) == FF_SUCCESS);
    FF_TEST_ASSERT(id == 1u);
    FF_TEST_ASSERT(FF_EsGetAppCount() == 1u);
    FF_TEST_ASSERT(FF_EsGetId("app_one") == id);
    FF_TEST_ASSERT(FF_EsGetId("missing") == FF_INVALID_ID);
    FF_TEST_ASSERT(FF_EsGetId(NULL) == FF_INVALID_ID);
    FF_TEST_ASSERT(FF_EsGetState(id) == FF_APP_LOADED);
    FF_TEST_ASSERT(FF_EsGetState(0xFFFDu) == FF_APP_FAILED);

    /* Starting an unknown app is a clean NOT_FOUND. */
    FF_TEST_ASSERT(FF_EsStartApp(0xFFFDu) == FF_NOT_FOUND);

    /* Start the app: launches a PAL task and moves it to RUNNING; a second
     * start is idempotent (already running). */
    FF_TEST_ASSERT(FF_EsStartApp(id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetState(id) == FF_APP_RUNNING);
    FF_TEST_ASSERT(FF_EsStartApp(id) == FF_SUCCESS);

    /* A second app with a NULL out_id (optional output) and StartAll. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.name = "app_two";
    cfg.entry = es_dummy_task;
    cfg.queue_depth = 2u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, NULL) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetAppCount() == 2u);
    rc = FF_EsStartAll();
    FF_TEST_ASSERT(rc == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetState(FF_EsGetId("app_two")) == FF_APP_RUNNING);
}

/* ------------------------------------------------------------------ */
/* Software Bus (SB): error paths, wildcard, saturation, drop         */
/* ------------------------------------------------------------------ */

static void test_sb_error_paths(void)
{
    FF_Msg_t m;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbPublish(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SbGetStats(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SbReceive(1u, &m, 0u) == FF_NOT_FOUND); /* no pipe     */
    FF_TEST_ASSERT(FF_SbReceive(1u, NULL, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SbSubscribe(0x0010u, 3u) == FF_NOT_FOUND); /* no pipe   */
    FF_TEST_ASSERT(FF_SbUnsubscribe(0x0010u, 3u) == FF_NOT_FOUND);

    /* depth 0 and depth > max fall back to the default depth. */
    FF_TEST_ASSERT(FF_SbCreatePipe(1u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(1u, 4u) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(FF_SbCreatePipe(2u, (uint16_t)(FF_SB_QUEUE_DEPTH_MAX + 1u)) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbSubscribe(0x0010u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbReceive(1u, &m, 0u) == FF_TIMEOUT); /* subscribed, empty */
    FF_TEST_ASSERT(FF_SbUnsubscribe(0x0010u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbUnsubscribe(0x0010u, 1u) == FF_NOT_FOUND);
}

static void test_sb_wildcard_and_drop(void)
{
    FF_Msg_t m;
    FF_Msg_t r;
    FF_SbStats_t s;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);

    FF_TEST_ASSERT(FF_SbCreatePipe(1u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(FF_SB_WILDCARD, 1u) == FF_SUCCESS);

    memset(&m, 0, sizeof(m));
    m.msg_id = 0x2222u;
    m.length = 1u;
    m.payload[0] = 0x5Au;
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);
    memset(&r, 0, sizeof(r));
    FF_TEST_ASSERT(FF_SbReceive(1u, &r, 100u) == FF_SUCCESS);
    FF_TEST_ASSERT(r.msg_id == 0x2222u);
    FF_TEST_ASSERT(r.payload[0] == 0x5Au);

    /* Queue depth 1: the second publish to a full pipe is dropped. */
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbPublish(&m) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbGetStats(&s) == FF_SUCCESS);
    FF_TEST_ASSERT(s.dropped >= 1u);
    FF_TEST_ASSERT(FF_SbReceive(1u, &r, 100u) == FF_SUCCESS);
}

static void test_sb_subscription_saturation(void)
{
    uint16_t i;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbCreatePipe(1u, 2u) == FF_SUCCESS);

    for (i = 0u; i < (uint16_t)FF_SB_MAX_SUBSCRIPTIONS; i++) {
        FF_TEST_ASSERT(FF_SbSubscribe((FF_MsgId_t)(0x3000u + i), 1u) == FF_SUCCESS);
    }
    /* The table is full: the next distinct key is refused. */
    FF_TEST_ASSERT(FF_SbSubscribe(0x3100u, 1u) == FF_NO_RESOURCE);

    /* Freeing one slot makes it reusable. */
    FF_TEST_ASSERT(FF_SbUnsubscribe(0x3000u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbSubscribe(0x3100u, 1u) == FF_SUCCESS);
}

/* ------------------------------------------------------------------ */
/* Table Services (TBL): revert, dump, validation, capacity           */
/* ------------------------------------------------------------------ */

typedef struct
{
    uint32_t magic;
    uint16_t gain;
} r2_tbl_cfg_t;

#define R2_TBL_MAGIC 0x52325531u /* 'R2U1' */

static FF_Status_t r2_tbl_validate(const void *image, size_t size)
{
    const r2_tbl_cfg_t *t = (const r2_tbl_cfg_t *)image;
    if ((size != sizeof(r2_tbl_cfg_t)) || (t->magic != R2_TBL_MAGIC)) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

static void test_tbl_errors_and_revert(void)
{
    r2_tbl_cfg_t active;
    r2_tbl_cfg_t staged;
    FF_Tbl_Def_t def;
    FF_TblHandle_t h;
    size_t sz = 0u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);

    /* Registration validation (no valid handle on any invalid input). */
    FF_TEST_ASSERT(FF_TblRegister(NULL) == FF_TBL_INVALID_HANDLE);
    memset(&def, 0, sizeof(def));
    FF_TEST_ASSERT(FF_TblRegister(&def) == FF_TBL_INVALID_HANDLE); /* name NULL */
    def.name = "t";
    FF_TEST_ASSERT(FF_TblRegister(&def) == FF_TBL_INVALID_HANDLE); /* image NULL */
    def.image = &active;
    FF_TEST_ASSERT(FF_TblRegister(&def) == FF_TBL_INVALID_HANDLE); /* size 0 */
    def.size = FF_TBL_MAX_SIZE + 1u;
    FF_TEST_ASSERT(FF_TblRegister(&def) == FF_TBL_INVALID_HANDLE); /* too big */

    /* Valid registration without a validator (NULL validate branch). */
    memset(&active, 0, sizeof(active));
    def.name = "t";
    def.image = &active;
    def.size = sizeof(active);
    def.validate = NULL;
    h = FF_TblRegister(&def);
    FF_TEST_ASSERT(h != FF_TBL_INVALID_HANDLE);

    /* Load/activate argument and state errors. */
    FF_TEST_ASSERT(FF_TblLoad(h, NULL, sizeof(active)) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_TblLoad(0xFFFEu, &staged, sizeof(staged)) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_TblActivate(0xFFFEu) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_TblRevert(0xFFFEu) == FF_NOT_FOUND);
    staged.magic = 0u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged) - 1u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_NOT_READY); /* nothing staged yet */

    /* Load + activate commits the staging image. */
    staged.magic = 0xABCDEF01u;
    staged.gain = 1u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_SUCCESS);
    FF_TEST_ASSERT(active.magic == 0xABCDEF01u);
    FF_TEST_ASSERT(FF_TblGet(h, &sz) == &active);
    FF_TEST_ASSERT(sz == sizeof(active));
    FF_TEST_ASSERT(FF_TblGet(0xFFFEu, NULL) == NULL);

    /* Revert restores staging from the active image and clears "staged". */
    staged.gain = 99u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblRevert(h) == FF_SUCCESS);
    FF_TEST_ASSERT(active.gain == 1u);                 /* active untouched by load */
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_NOT_READY); /* reverted => not staged */
}

static void test_tbl_validate_and_dump(void)
{
    r2_tbl_cfg_t active;
    r2_tbl_cfg_t staged;
    r2_tbl_cfg_t dump;
    FF_Tbl_Def_t def;
    FF_TblHandle_t h;
    size_t len;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);

    memset(&active, 0, sizeof(active));
    def.name = "cfg";
    def.image = &active;
    def.size = sizeof(active);
    def.validate = r2_tbl_validate;
    h = FF_TblRegister(&def);
    FF_TEST_ASSERT(h != FF_TBL_INVALID_HANDLE);

    /* A validator rejection propagates as FF_ERROR and stages nothing. */
    staged.magic = 0u;
    staged.gain = 5u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_ERROR);

    /* Valid load then activate. */
    staged.magic = R2_TBL_MAGIC;
    staged.gain = 0x1234u;
    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblActivate(h) == FF_SUCCESS);

    /* Dump the active image; capacity and argument checks. */
    memset(&dump, 0, sizeof(dump));
    len = sizeof(dump);
    FF_TEST_ASSERT(FF_TblDump(h, &dump, &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == sizeof(dump));
    FF_TEST_ASSERT(dump.magic == R2_TBL_MAGIC);
    FF_TEST_ASSERT(dump.gain == 0x1234u);

    len = sizeof(dump) - 1u;
    FF_TEST_ASSERT(FF_TblDump(h, &dump, &len) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_TblDump(h, NULL, &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TblDump(h, &dump, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TblDump(0xFFFEu, &dump, &len) == FF_INVALID_ARGUMENT);
}

static void test_tbl_capacity(void)
{
    static uint8_t images[FF_TBL_MAX_TABLES][4];
    char name[FF_TBL_NAME_LEN];
    FF_Tbl_Def_t def;
    uint16_t i;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);

    def.image = images[0];
    def.size = 4u;
    def.validate = NULL;
    for (i = 0u; i < (uint16_t)FF_TBL_MAX_TABLES; i++) {
        (void)snprintf(name, sizeof(name), "t%u", (unsigned)i);
        def.name = name;
        def.image = images[i];
        /* Fresh registry: slots are handed out in order. */
        FF_TEST_ASSERT(FF_TblRegister(&def) == (FF_TblHandle_t)i);
    }
    /* One past capacity: no handle. */
    FF_TEST_ASSERT(FF_TblRegister(&def) == FF_TBL_INVALID_HANDLE);
}

/* ------------------------------------------------------------------ */
/* Time (MET/UTC accessors not yet exercised)                         */
/* ------------------------------------------------------------------ */

static uint64_t r2_later_than(uint64_t prev)
{
    uint64_t t = FF_PalGetMonotonicNs();
    while (t <= prev) {
        t = FF_PalGetMonotonicNs();
    }
    return t;
}

static void test_time_met_and_utc(void)
{
    uint64_t m1;
    uint64_t m2;
    uint64_t u1;
    uint64_t u2;
    const uint64_t anchor = 3600ull * FF_TIME_TONE_NS;
    uint32_t coarse = 0u;
    uint16_t fine = 0u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);

    m1 = FF_TimeGetMetNs();
    m2 = r2_later_than(m1);
    FF_TEST_ASSERT(m2 > m1);
    FF_TEST_ASSERT(FF_TimeGetMetMs() <= (uint32_t)(m2 / 1000000ull));

    /* UTC advances monotonically across two reads. */
    u1 = FF_TimeGetUtcNs();
    u2 = FF_TimeGetUtcNs();
    FF_TEST_ASSERT(u2 >= u1);

    /* A ground-pinned UTC epoch is reflected (bounded by one tone period of
     * elapsed wall time between set and read). */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(anchor) == FF_SUCCESS);
    u1 = FF_TimeGetUtcNs();
    FF_TEST_ASSERT(u1 >= anchor);
    FF_TEST_ASSERT((u1 - anchor) < FF_TIME_TONE_NS);

    /* NULL outputs are tolerated (optional parameters). */
    FF_TEST_ASSERT(FF_TimeGetCuc(NULL, NULL) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, NULL) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(NULL, &fine) == FF_SUCCESS);
}

/* ------------------------------------------------------------------ */
/* HAL registry: invalid-class and multi-class accounting              */
/* ------------------------------------------------------------------ */

#if !defined(FF_TEST_NO_HAL)
static void test_hal_registry_gaps(void)
{
    static int backend_a;
    static int backend_b;

    FF_TEST_ASSERT(FF_HalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_HalTotal() == 0u);

    /* Invalid class: negative enum value and out-of-range value both map to
     * count 0 / lookup NULL; registration is refused. */
    FF_TEST_ASSERT(FF_HalCount((FF_HalClass_t)-1) == 0u);
    FF_TEST_ASSERT(FF_HalCount((FF_HalClass_t)200) == 0u);
    FF_TEST_ASSERT(FF_HalGet((FF_HalClass_t)-1, "x") == NULL);
    FF_TEST_ASSERT(FF_HalGet(FF_HAL_ETH, NULL) == NULL);
    FF_TEST_ASSERT((int)FF_HalRegister((FF_HalClass_t)-1, "x", &backend_a) ==
                   (int)FF_INVALID_ARGUMENT);

    /* Accounting sums across distinct classes. */
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_ETH, "e0", &backend_a) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_MEM, "m0", &backend_b) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_HalCount(FF_HAL_ETH) == 1u);
    FF_TEST_ASSERT(FF_HalCount(FF_HAL_MEM) == 1u);
    FF_TEST_ASSERT(FF_HalTotal() == 2u);
    FF_TEST_ASSERT(FF_HalGet(FF_HAL_MEM, "m0") == &backend_b);
}
#endif /* !FF_TEST_NO_HAL */

/* ------------------------------------------------------------------ */
/* CCSDS Space Packet codec: telecommand direction and decode errors  */
/* ------------------------------------------------------------------ */

static void test_ccsds_telecommand(void)
{
    FF_Pus_TcHdr_t tc;
    FF_Pus_TcHdr_t dec;
    uint8_t frame[128];
    uint8_t tmframe[128];
    uint16_t frame_len = 0u;
    uint16_t tm_len = 0u;
    uint16_t apid = 0u;
    const uint8_t *data = NULL;
    uint16_t data_len = 0u;
    const uint8_t payload[3] = {0x10u, 0x20u, 0x30u};

    /* Argument validation. */
    memset(&tc, 0, sizeof(tc));
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, NULL, payload, 3u, frame, sizeof(frame), &frame_len) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, payload, 3u, NULL, sizeof(frame), &frame_len) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, payload, 3u, frame, sizeof(frame), NULL) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, NULL, 3u, frame, sizeof(frame), &frame_len) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, payload, 3u, frame, 4u, &frame_len) ==
                   FF_BAD_SIZE);

    /* Round trip with a payload. */
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = 17u;
    tc.subservice = 1u;
    tc.source_id = 0x0102u;
    frame_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(frame_len == (FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN + sizeof(payload) + 2u));
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame, frame_len, &apid, &dec, &data, &data_len) == FF_SUCCESS);
    FF_TEST_ASSERT(apid == 0x010u);
    FF_TEST_ASSERT(dec.version == FF_PUS_VERSION);
    FF_TEST_ASSERT(dec.service == 17u);
    FF_TEST_ASSERT(dec.subservice == 1u);
    FF_TEST_ASSERT(dec.source_id == 0x0102u);
    FF_TEST_ASSERT(data_len == sizeof(payload));
    FF_TEST_ASSERT(memcmp(data, payload, sizeof(payload)) == 0);

    /* Optional outputs may be NULL. */
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame, frame_len, NULL, &dec, NULL, NULL) == FF_SUCCESS);

    /* Zero-length payload is legal. */
    frame_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, NULL, 0u, frame, sizeof(frame), &frame_len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame, frame_len, &apid, &dec, &data, &data_len) == FF_SUCCESS);
    FF_TEST_ASSERT(data_len == 0u);

    /* Decode rejections: short buffer, wrong packet type, truncated PDL. */
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(NULL, 0u, &apid, &dec, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame,
                                    (uint16_t)(FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN - 1u),
                                    &apid, &dec, &data, &data_len) == FF_PARSE_ERROR);

    FF_TEST_ASSERT(FF_CcsdsEncodeTm(0x020u, (const FF_Pus_TmHdr_t *)&dec, NULL, 0u, tmframe,
                                    sizeof(tmframe), &tm_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(tmframe, tm_len, &apid, &dec, &data, &data_len) ==
                   FF_PARSE_ERROR); /* a TM frame is not a TC */

    /* Rebuild a valid TC and truncate its declared length. */
    frame_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(0x010u, &tc, payload, sizeof(payload), frame, sizeof(frame),
                                    &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame, (uint16_t)(frame_len - 3u), &apid, &dec, &data,
                                    &data_len) == FF_PARSE_ERROR);

    /* Corrupted CRC is detected. */
    frame[frame_len - 1u] ^= 0xFFu;
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame, frame_len, &apid, &dec, &data, &data_len) ==
                   FF_CRC_ERROR);
}

/* ------------------------------------------------------------------ */
/* PUS core + services: statistics, event emission, S3/S8/S9/S11/S20  */
/* ------------------------------------------------------------------ */

static uint8_t g_pus_rx[8192];
static uint16_t g_pus_rx_len;

static void pus_sink(const uint8_t *packet, uint16_t len)
{
    if ((uint32_t)g_pus_rx_len + (uint32_t)len <= sizeof(g_pus_rx)) {
        memcpy(&g_pus_rx[g_pus_rx_len], packet, len);
        g_pus_rx_len = (uint16_t)(g_pus_rx_len + len);
    }
}

static bool pus_stream_has(uint8_t service, uint8_t subservice)
{
    uint16_t pos = 0u;
    while ((uint32_t)pos + 8u < (uint32_t)g_pus_rx_len) {
        if ((g_pus_rx[pos + 7u] == service) && (g_pus_rx[pos + 8u] == subservice)) {
            return true;
        }
        uint16_t pdl = (uint16_t)(((uint16_t)g_pus_rx[pos + 4u] << 8) | g_pus_rx[pos + 5u]);
        pos = (uint16_t)(pos + 6u + pdl + 1u);
    }
    return false;
}

static FF_Status_t pus_ingest(uint8_t service, uint8_t subservice, const uint8_t *data,
                              uint16_t len)
{
    FF_Pus_TcHdr_t tc;
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t frame_len = 0u;

    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = 0x0102u;
    if (FF_CcsdsEncodeTc(FF_APID_TC, &tc, data, len, frame, sizeof(frame), &frame_len) !=
        FF_SUCCESS) {
        return FF_ERROR;
    }
    return FF_PusIngest(frame, frame_len);
}

static uint8_t g_pus_param;
static bool g_pus_setter_called;

static FF_Status_t pus_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    out[0] = 0xA5u;
    *len = 1u;
    return FF_SUCCESS;
}

static FF_Status_t pus_param_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    out[0] = 0x5Au;
    *len = 1u;
    return FF_SUCCESS;
}

static FF_Status_t pus_param_setter(uint8_t id, const uint8_t *data, uint16_t len)
{
    (void)id;
    g_pus_setter_called = true;
    if (len >= 1u) {
        g_pus_param = data[0];
    }
    return FF_SUCCESS;
}

static bool g_pus_func_called;

static FF_Status_t pus_function_ok(const uint8_t *args, uint16_t len)
{
    (void)args;
    (void)len;
    g_pus_func_called = true;
    return FF_SUCCESS;
}

static FF_Status_t pus_function_fail(const uint8_t *args, uint16_t len)
{
    (void)args;
    (void)len;
    return FF_ERROR;
}

static void pus_register_all(void)
{
    (void)FF_PusRegisterParamProvider(5u, pus_param_provider);
    (void)FF_PusRegisterParamSetter(5u, pus_param_setter);
    (void)FF_PusRegisterFunction(7u, pus_function_ok);
    (void)FF_PusRegisterFunction(8u, pus_function_fail);
    (void)FF_PusRegisterHkProvider(9u, pus_hk_provider);
}

static void test_pus_core_gaps(void)
{
    uint16_t c0;
    uint16_t c1;
    uint32_t tc0;
    const uint8_t payload[2] = {0x01u, 0x02u};

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusGetBackend() == &FF_PusBackendOpen);
    FF_PusSetTmSink(pus_sink);

    /* Counter/statistics accessors. */
    c0 = FF_PusPeekTmCounter();
    FF_TEST_ASSERT(FF_PusNextTmCounter() == c0);
    c1 = FF_PusPeekTmCounter();
    FF_TEST_ASSERT(c1 == (uint16_t)((c0 + 1u) & 0x3FFFu));

    /* Event emission maps EVS severity to service 5 subservices. */
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0101u, FF_EVS_INFO, payload, sizeof(payload)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0102u, FF_EVS_WARNING, payload, sizeof(payload)) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0103u, FF_EVS_ERROR, payload, sizeof(payload)) == FF_SUCCESS);
    FF_TEST_ASSERT(pus_stream_has(5u, 1u));
    FF_TEST_ASSERT(pus_stream_has(5u, 2u));
    FF_TEST_ASSERT(pus_stream_has(5u, 3u));

    /* Oversized event payload is refused before framing. */
    FF_TEST_ASSERT(FF_PusEmitEvent(0u, FF_EVS_INFO, NULL, (uint16_t)(FF_PUS_MAX_TM_FRAME)) ==
                   FF_BAD_SIZE);

    /* Parameter registration contract (NULL handler rejected). */
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(5u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusRegisterParamSetter(5u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusRegisterFunction(7u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(9u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PusRegisterParamProvider(5u, pus_param_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterParamSetter(5u, pus_param_setter) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterFunction(7u, pus_function_ok) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterFunction(8u, pus_function_fail) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(9u, pus_hk_provider) == FF_SUCCESS);

    /* TC counter advances on a successfully decoded command. */
    tc0 = FF_PusGetTcCount();
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(17u, 1u, payload, sizeof(payload)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusGetTcCount() == (tc0 + 1u));
    FF_TEST_ASSERT(pus_stream_has(17u, 2u));
}

static void test_pus_service3_hk(void)
{
    const uint8_t create_ok[2] = {3u, 9u};
    const uint8_t create_bad_prov[2] = {4u, 99u};
    const uint8_t bad_size[1] = {3u};
    const uint8_t sel3[1] = {3u};
    const uint8_t sel4[1] = {4u};

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(pus_sink);
    pus_register_all();

    /* Create: too-short payload, success, duplicate. */
    FF_TEST_ASSERT(pus_ingest(3u, 1u, bad_size, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(pus_ingest(3u, 1u, create_ok, sizeof(create_ok)) == FF_SUCCESS);
    FF_TEST_ASSERT(pus_ingest(3u, 1u, create_ok, sizeof(create_ok)) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(pus_ingest(3u, 1u, create_bad_prov, sizeof(create_bad_prov)) == FF_SUCCESS);

    /* Report all: report 3 emits (3,25); report 4 has no provider => NOT_FOUND. */
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(3u, 27u, NULL, 0u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(pus_stream_has(3u, 25u));

    /* Report one enabled report. */
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(3u, 27u, sel3, sizeof(sel3)) == FF_SUCCESS);
    FF_TEST_ASSERT(pus_stream_has(3u, 25u));

    /* Disable suppresses emission; enable restores it. */
    FF_TEST_ASSERT(pus_ingest(3u, 4u, sel3, sizeof(sel3)) == FF_SUCCESS);
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(3u, 27u, sel3, sizeof(sel3)) == FF_SUCCESS);
    FF_TEST_ASSERT(!pus_stream_has(3u, 25u));
    FF_TEST_ASSERT(pus_ingest(3u, 3u, sel3, sizeof(sel3)) == FF_SUCCESS);

    /* Selector errors and delete semantics. */
    FF_TEST_ASSERT(pus_ingest(3u, 2u, NULL, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(pus_ingest(3u, 3u, NULL, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(pus_ingest(3u, 4u, NULL, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(pus_ingest(3u, 2u, sel4, sizeof(sel4)) == FF_SUCCESS);
    FF_TEST_ASSERT(pus_ingest(3u, 2u, sel4, sizeof(sel4)) == FF_NOT_FOUND);
}

static void test_pus_service8_function(void)
{
    const uint8_t f7[2] = {7u, 0x77u};
    const uint8_t f8[2] = {8u, 0x88u};
    const uint8_t f200[2] = {200u, 0x00u};

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(pus_sink);
    pus_register_all();

    FF_TEST_ASSERT(pus_ingest(8u, 1u, NULL, 0u) == FF_BAD_SIZE);

    g_pus_func_called = false;
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(8u, 1u, f7, sizeof(f7)) == FF_SUCCESS);
    FF_TEST_ASSERT(g_pus_func_called);
    FF_TEST_ASSERT(pus_stream_has(8u, 2u));

    /* A failing function reports its error and a failure status TM. */
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(8u, 1u, f8, sizeof(f8)) == FF_ERROR);
    FF_TEST_ASSERT(pus_stream_has(8u, 2u));

    /* Unknown function id. */
    FF_TEST_ASSERT(pus_ingest(8u, 1u, f200, sizeof(f200)) == FF_NOT_FOUND);
}

static void test_pus_service20_parameters(void)
{
    const uint8_t set5[2] = {5u, 0x11u};
    const uint8_t sel5[1] = {5u};
    const uint8_t sel200[1] = {200u};

    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(pus_sink);
    pus_register_all();

    /* Set: too short, unknown id, success with captured value. */
    FF_TEST_ASSERT(pus_ingest(20u, 1u, NULL, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(pus_ingest(20u, 1u, sel200, sizeof(sel200)) == FF_NOT_FOUND);
    g_pus_setter_called = false;
    g_pus_param = 0u;
    FF_TEST_ASSERT(pus_ingest(20u, 1u, set5, sizeof(set5)) == FF_SUCCESS);
    FF_TEST_ASSERT(g_pus_setter_called);
    FF_TEST_ASSERT(g_pus_param == 0x11u);

    /* Report: too short, unknown id, success emits (20,4). */
    FF_TEST_ASSERT(pus_ingest(20u, 3u, NULL, 0u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(pus_ingest(20u, 3u, sel200, sizeof(sel200)) == FF_NOT_FOUND);
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(20u, 3u, sel5, sizeof(sel5)) == FF_SUCCESS);
    FF_TEST_ASSERT(pus_stream_has(20u, 4u));
}

static void test_pus_service9_and_11(void)
{
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(pus_sink);

    /* Service 9 report time. */
    g_pus_rx_len = 0u;
    FF_TEST_ASSERT(pus_ingest(9u, 2u, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(pus_stream_has(9u, 2u));

    /* Service 11 scheduling is a declared stub: NOT_IMPLEMENTED both ways. */
    FF_TEST_ASSERT(pus_ingest(11u, 1u, NULL, 0u) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(pus_ingest(11u, 2u, NULL, 0u) == FF_NOT_IMPLEMENTED);
}

/* ------------------------------------------------------------------ */
/* Mathematics facade: NaN / infinity helpers (MISRA edge behaviour)  */
/* ------------------------------------------------------------------ */

#if defined(FRICK_USE_LIBM) && (FRICK_USE_LIBM == 1)
static void test_math_nan_inf(void)
{
    double nan_value = FF_MathSqrt(-1.0); /* documented NaN result */

    FF_TEST_ASSERT(isnan(FF_MathClamp(nan_value, 0.0, 1.0)) != 0);
    FF_TEST_ASSERT(FF_MathIsFinite(1.0 / 0.0) == 0u);
    FF_TEST_ASSERT(FF_MathIsFinite(nan_value) == 0u);
    FF_TEST_ASSERT(FF_MathIsFinite(FF_MathExp(0.0)) == 1u);
}
#endif

/* ------------------------------------------------------------------ */
/* PAL surface: lifecycle, tasks, mutex/queue delete, count, time      */
/* ------------------------------------------------------------------ */

static void pal_dummy_task(void *arg)
{
    (void)arg;
}

static void pal_exit_task(void *arg)
{
    (void)arg;
    FF_PalTaskExit(); /* noreturn: terminates this PAL task thread */
}

static void test_pal_lifecycle_and_time(void)
{
    uint64_t utc;
    const uint64_t anchor = 123456789ull;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS); /* idempotent */
    FF_TEST_ASSERT(FF_PalGetCpuLoadPpt() == 0u);
    FF_TEST_ASSERT(FF_PalTaskSelf() == FF_INVALID_ID); /* main is not a PAL task */
    FF_PalTaskDelayTicks(1u);
    FF_PalTaskYield();
    FF_PalLog("%s", ""); /* console path; prints nothing for an empty string */

    FF_TEST_ASSERT(FF_PalSetUtcNs(anchor) == FF_SUCCESS);
    utc = FF_PalGetUtcNs();
    FF_TEST_ASSERT(utc >= anchor);
    FF_TEST_ASSERT((utc - anchor) < FF_TIME_TONE_NS);
}

static void test_pal_mutex_delete(void)
{
    FF_MutexId_t id = 0u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalMutexCreate(NULL, "m") == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalMutexLock(0xFFFFu, 0u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_PalMutexUnlock(0xFFFFu) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_PalMutexDelete(0xFFFFu) == FF_NOT_FOUND);

    FF_TEST_ASSERT(FF_PalMutexCreate(&id, "m") == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalMutexLock(id, FF_PAL_WAIT_FOREVER) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalMutexUnlock(id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalMutexDelete(id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalMutexLock(id, 0u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_PalMutexDelete(id) == FF_NOT_FOUND);
}

static void test_pal_queue(void)
{
    FF_QueueId_t id = 0u;
    uint32_t value = 0u;
    const uint32_t in_value = 0x11223344u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueCreate(NULL, "q", 1u, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalQueueCreate(&id, "q", 0u, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalQueueCreate(&id, "q", 1u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalQueuePut(0xFFFFu, &in_value, 0u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_PalQueueGet(0xFFFFu, &value, 0u) == FF_NOT_FOUND);

    FF_TEST_ASSERT(FF_PalQueueCreate(&id, "q", 1u, (uint16_t)sizeof(uint32_t)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueCount(id) == 0u);
    FF_TEST_ASSERT(FF_PalQueueCount(0xFFFFu) == 0u);
    FF_TEST_ASSERT(FF_PalQueuePut(id, NULL, 0u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_PalQueueGet(id, &value, 0u) == FF_TIMEOUT); /* empty, no block */

    FF_TEST_ASSERT(FF_PalQueuePut(id, &in_value, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueCount(id) == 1u);
    FF_TEST_ASSERT(FF_PalQueuePut(id, &in_value, 0u) == FF_BUFFER_FULL); /* full, no wait   */
    FF_TEST_ASSERT(FF_PalQueuePut(id, &in_value, 1u) == FF_TIMEOUT);     /* full, timed out */

    value = 0u;
    FF_TEST_ASSERT(FF_PalQueueGet(id, &value, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(value == in_value);
    FF_TEST_ASSERT(FF_PalQueueCount(id) == 0u);

    FF_TEST_ASSERT(FF_PalQueueDelete(id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalQueueCount(id) == 0u);
    FF_TEST_ASSERT(FF_PalQueueDelete(id) == FF_NOT_FOUND);
}

static void test_pal_task_delete(void)
{
    FF_TaskId_t id = 0u;

    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalTaskCreate(NULL, "t", pal_dummy_task, NULL, 0u, 1u) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalTaskCreate(&id, "t", NULL, NULL, 0u, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PalTaskDelete(0xFFFDu) == FF_NOT_FOUND);

    FF_TEST_ASSERT(FF_PalTaskCreate(&id, "t", pal_dummy_task, NULL, 0u, 1u) == FF_SUCCESS);
    FF_PalTaskDelayMs(5u);
    FF_TEST_ASSERT(FF_PalTaskDelete(id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalTaskDelete(id) == FF_NOT_FOUND);

    /* A task that self-terminates via FF_PalTaskExit must still be deletable. */
    FF_TEST_ASSERT(FF_PalTaskCreate(&id, "x", pal_exit_task, NULL, 0u, 1u) == FF_SUCCESS);
    FF_PalTaskDelayMs(5u);
    FF_TEST_ASSERT(FF_PalTaskDelete(id) == FF_SUCCESS);
}

FF_TEST_MAIN(test_evs_listener_and_ring, test_es_registry_and_lifecycle, test_sb_error_paths,
             test_sb_wildcard_and_drop, test_sb_subscription_saturation, test_tbl_errors_and_revert,
             test_tbl_validate_and_dump, test_tbl_capacity, test_time_met_and_utc,
#if !defined(FF_TEST_NO_HAL)
             test_hal_registry_gaps,
#endif
             test_ccsds_telecommand, test_pus_core_gaps, test_pus_service3_hk,
             test_pus_service8_function, test_pus_service20_parameters, test_pus_service9_and_11,
#if defined(FRICK_USE_LIBM) && (FRICK_USE_LIBM == 1)
             test_math_nan_inf,
#endif
             test_pal_lifecycle_and_time, test_pal_mutex_delete, test_pal_queue,
             test_pal_task_delete)