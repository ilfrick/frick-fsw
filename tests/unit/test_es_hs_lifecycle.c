/*
 * frick-fsw - ES/HS application-lifecycle host integration harness
 * (S8-VV-FU; gap GAP-R3-1 from FRICK-REVIEW-0076 / independent assessment T-08)
 *
 * Before this suite, ES lifecycle and Health & Safety detection had only
 * simulator-smoke evidence. This harness runs the real ES registry, the real
 * POSIX PAL task layer and the real HS monitoring pass on the host, with the
 * EVS listener seam as the independent oracle (the same seam the flight wiring
 * uses to forward events to PUS service 5).
 *
 * Fault cases exercised: PAL task-create resource exhaustion driving an
 * application to FF_APP_FAILED, task exit (entry returns), and the HS
 * FAILED-application report. The oracle is the observable event stream
 * (event id, type, application id, text), not internal state alone.
 *
 * Contract limits (never asserted as requirements here): the state transition
 * on task return/abort and the invalid-id/saturation return contracts are not
 * fixed by any clause (TS-6 `N22-TS6-41`/`N22-TS6-36`, NC-EXT-47 §7.2); those
 * sub-cases are recorded as observations only. Detection latency and the
 * external watchdog window are mission inputs (TS-5 `N22-TS5-52`, TS-6
 * `N22-TS6-31`). GAP-R3-1 stays OPEN until the independent R2/R3 delta review
 * is recorded; nothing here closes an NC or approves a phase.
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
#include <string.h>

/* ------------------------------------------------------------------ */
/* Independent oracle: capture the EVS event stream at the listener    */
/* seam (the production observation point for HS output).              */
/* ------------------------------------------------------------------ */

#define HS_CAP_MAX 64u

static FF_EvsEvent_t g_cap[HS_CAP_MAX];
static uint16_t g_cap_count;

static void hs_capture(const FF_EvsEvent_t *ev)
{
    if (g_cap_count < HS_CAP_MAX) {
        g_cap[g_cap_count] = *ev;
        g_cap_count++;
    }
}

static void cap_reset(void)
{
    (void)memset(g_cap, 0, sizeof(g_cap));
    g_cap_count = 0u;
}

static uint16_t cap_count_id(uint16_t event_id)
{
    uint16_t n = 0u;
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if (g_cap[i].event_id == event_id) {
            n++;
        }
    }
    return n;
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

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

/* Dummy application entries: a host task whose entry returns immediately is
 * the host-side model of a terminated task (PAL keeps the slot until delete). */
static void hs_dummy_entry(void *arg)
{
    (void)arg;
}

/* Fill every remaining PAL task slot so the next task create fails. Returns
 * the number of filler tasks created; the loop is bounded by the PAL slot
 * count plus one. */
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
    return created; /* PAL never reported exhaustion: bounded stop, flagged by caller */
}

static void hs_init_modules(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsInit() == FF_SUCCESS);
    FF_EvsSetListener(hs_capture);
    cap_reset();
}

/* ------------------------------------------------------------------ */
/* 1. Nominal: heartbeat every monitoring pass, no false FAILED report */
/* ------------------------------------------------------------------ */

static void test_hs_heartbeat_no_fault(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t ok_id = FF_INVALID_ID;
    FF_AppId_t hs_id = FF_INVALID_ID;

    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "ok_app";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &ok_id) == FF_SUCCESS);
    cfg.name = "hs_app";
    cfg.entry = FF_AppHsMain;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &hs_id) == FF_SUCCESS);

    /* Start only the worker: the HS task itself is stepped directly by the
     * harness so the pass is deterministic (no 5000 ms cadence wait). */
    FF_TEST_ASSERT(FF_EsStartApp(ok_id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetState(ok_id) == FF_APP_RUNNING);
    FF_TEST_ASSERT(FF_EsGetState(hs_id) == FF_APP_LOADED); /* registered, not started */

    /* Nominal oracle (SRS-REQ-014 "detect failed applications"; SSS-REQ-008
     * "generate heartbeat events"): a RUNNING and a LOADED application are not
     * health anomalies, so no application-error event may be raised, and each
     * monitoring pass raises exactly one heartbeat for the HS application. */
    cap_reset();
    FF_AppHsStep(hs_id);
    FF_AppHsStep(hs_id);

    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_ERROR) == 0u);
    FF_TEST_ASSERT(cap_count_id(0x0502u) == 2u); /* one heartbeat per pass */
    FF_TEST_ASSERT(cap_count_id_type(0x0502u, FF_EVS_INFO) == 2u);
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if (g_cap[i].event_id == 0x0502u) {
            FF_TEST_ASSERT(g_cap[i].app_id == hs_id);
        }
    }

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* 2. Lifecycle on the real host PAL: start, idempotency, task exit     */
/* ------------------------------------------------------------------ */

static void test_es_lifecycle_host_tasks(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t a = FF_INVALID_ID;
    FF_AppId_t b = FF_INVALID_ID;

    FF_PalTaskDelayMs(20u); /* let previous-test tasks terminate before re-init */
    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "worker_a";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &a) == FF_SUCCESS);
    cfg.name = "worker_b";
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &b) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_EsGetAppCount() == 2u);
    FF_TEST_ASSERT(FF_EsGetId("worker_a") == a);
    FF_TEST_ASSERT(FF_EsGetState(a) == FF_APP_LOADED);

    cap_reset();
    FF_TEST_ASSERT(FF_EsStartAll() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetState(a) == FF_APP_RUNNING);
    FF_TEST_ASSERT(FF_EsGetState(b) == FF_APP_RUNNING);
    /* Independent-task oracle: one APP_STARTED event per application. */
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_STARTED) == 2u);

    /* Idempotent restart (SRS-REQ-014 state tracking; TS-5 N22-TS5-38). */
    FF_TEST_ASSERT(FF_EsStartApp(a) == FF_SUCCESS);
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_STARTED) == 2u); /* no second start */

    /* Task exit observation: both entries return immediately. Observation
     * only - no clause defines the state transition on task return/abort
     * (TS-6 N22-TS6-41, NC-EXT-47 §7.2), so this records current behaviour
     * (state stays RUNNING, HS raises no spurious report) and is not a
     * requirement oracle. */
    FF_PalTaskDelayMs(20u);
    FF_TEST_ASSERT(FF_EsGetState(a) == FF_APP_RUNNING);
    cap_reset();
    FF_AppHsStep(a);
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_ERROR) == 0u);

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* 3. Fault cases: start failure -> FAILED, HS reports it exactly once  */
/*    per monitoring pass, naming the failed application id              */
/* ------------------------------------------------------------------ */

static void test_es_start_failure_and_hs_detection(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t ok_id = FF_INVALID_ID;
    FF_AppId_t bad_id = FF_INVALID_ID;
    FF_AppId_t hs_id = FF_INVALID_ID;
    uint16_t fillers;

    FF_PalTaskDelayMs(20u); /* let previous-test tasks terminate before re-init */
    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "ok_app";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &ok_id) == FF_SUCCESS);
    cfg.name = "hs_app";
    cfg.entry = FF_AppHsMain;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &hs_id) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_EsStartApp(ok_id) == FF_SUCCESS);

    /* Fault injection: exhaust the PAL task slots, then start one more
     * application. The start must fail and ES must track the failure. */
    fillers = hs_fill_pal_slots();
    FF_TEST_ASSERT(fillers < 64u); /* exhaustion reached before the bounded stop */

    cfg.name = "bad_app";
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &bad_id) == FF_SUCCESS);
    cap_reset();
    FF_TEST_ASSERT(FF_EsStartApp(bad_id) == FF_ERROR);
    /* SRS-REQ-014: ES tracks states; a failed start is observable as
     * FF_APP_FAILED (the failed-start path, TS-5 N22-TS5-51 fault case). */
    FF_TEST_ASSERT(FF_EsGetState(bad_id) == FF_APP_FAILED);
    /* The failure is also reported at the event seam. */
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_ERROR) >= 1u);

    /* HS detection oracle (SRS-REQ-014 "Health & Safety shall detect failed
     * applications"; SSS-REQ-008 "monitor application health"): one pass
     * reports exactly the FAILED application, by id, as a CRITICAL event, and
     * still emits exactly one heartbeat. The RUNNING application is not
     * reported (no false positive). */
    cap_reset();
    FF_AppHsStep(hs_id);
    FF_TEST_ASSERT(cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u);
    FF_TEST_ASSERT(cap_hs_reports(bad_id));
    FF_TEST_ASSERT(!cap_hs_reports(ok_id));
    FF_TEST_ASSERT(cap_count_id(0x0502u) == 1u);

    /* Second pass: the FAILED application is reported again (one report per
     * monitoring pass while the failure persists). */
    cap_reset();
    FF_AppHsStep(hs_id);
    FF_TEST_ASSERT(cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u);
    FF_TEST_ASSERT(cap_hs_reports(bad_id));
    FF_TEST_ASSERT(cap_count_id(0x0502u) == 1u);

    /* Persistence: the state is unchanged by observation. */
    FF_TEST_ASSERT(FF_EsGetState(bad_id) == FF_APP_FAILED);
    FF_TEST_ASSERT(FF_EsGetState(ok_id) == FF_APP_RUNNING);

    FF_EvsSetListener(NULL);
}

FF_TEST_MAIN(test_hs_heartbeat_no_fault, test_es_lifecycle_host_tasks,
             test_es_start_failure_and_hs_detection)
