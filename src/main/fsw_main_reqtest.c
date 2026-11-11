/*
 * frick-fsw - Test-only flight main for the T1 requirement-keyed target suite
 *
 * This entry point exists because `src/main/fsw_main.c` is FROZEN by the T1
 * brief (byte-identical at handover): the requirement-test application cannot
 * be registered by a conditional line inside the flight main. Instead, this
 * file mirrors the flight sequence of `ff_fsw_main()` and adds the REQ_TEST
 * application, so that a test-only image (`frick_fsw_reqtest`, built with
 * -DFF_ENABLE_TARGET_REQ_TEST=ON) runs the T1 suite on the target.
 *
 * Consequences that are stated rather than hidden:
 *   - The initialisation order below is a COPY of the flight sequence. If the
 *     flight main changes, this file must be re-mirrored; the T1 handover
 *     records the duplication and leaves the alternative (a conditional
 *     registration inside `fsw_main.c`) as a decision for the owner.
 *   - The production executable and its application count are unaffected: this
 *     file is compiled into a separate executable only when the option is ON.
 *   - Watchdog/safety initialisation is not modified, bypassed or weakened:
 *     the WD application is registered exactly as in flight, and a watchdog
 *     abort is reported as the real result.
 *
 * See docs/vv/target-io-coverage.md and the T1 handover for the evidence scope
 * (logical chain only; not MPFS peripheral evidence, not N18).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "fsw_main.h"
#include "fsw_config.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include "core/es/es.h"
#include "core/tbl/tbl.h"
#include "core/time/time.h"
#include "bsp/mpfs/board.h"
#include "transport/link/link.h"
#include "pus/pus.h"
#include "apps/app.h"
#include "tables/fsw_tables.h"
#include <string.h>

#if !FF_ENABLE_TARGET_REQ_TEST
#error "fsw_main_reqtest.c is only valid with FF_ENABLE_TARGET_REQ_TEST=1"
#endif

static void ff_reqtest_event_listener(const FF_EvsEvent_t *evt)
{
    uint16_t text_len = 0u;

    while ((text_len < sizeof(evt->text)) && (evt->text[text_len] != '\0')) {
        text_len++;
    }
    /* FF_PusEmitEvent owns the service 5 report header (event id + type). */
    (void)FF_PusEmitEvent(evt->event_id, evt->type, (const uint8_t *)evt->text, text_len);
}

static void ff_reqtest_banner(void)
{
    FF_PalLog("\n");
    FF_PalLog("+-----------------------------------------------------------+\n");
    FF_PalLog("|  frick-fsw \"%s\" v%s                           |\n", FF_CODENAME,
              FF_VERSION_STRING);
    FF_PalLog("|  T1 requirement-keyed target suite (test-only image)      |\n");
    FF_PalLog("|  platform : %-49s |\n", FF_PLATFORM_NAME);
    FF_PalLog("+-----------------------------------------------------------+\n");
    FF_PalLog("\n");
}

int ff_fsw_main(void)
{
    FF_AppId_t id;

    ff_reqtest_banner();

    if ((FF_PalInit() != FF_SUCCESS) || (FF_EvsInit() != FF_SUCCESS) ||
        (FF_SbInit() != FF_SUCCESS) || (FF_EsInit() != FF_SUCCESS) ||
        (FF_TblInit() != FF_SUCCESS) || (FF_TimeInit() != FF_SUCCESS) ||
        (FF_BoardInit() != FF_SUCCESS) || (FF_MissionTablesInit() != FF_SUCCESS)) {
        FF_PalPanic("core initialisation failed");
    }

    if ((FF_PusInit() != FF_SUCCESS) ||
#if defined(FF_USE_PUS_ASN1) && (FF_USE_PUS_ASN1 == 1)
        (FF_PusSetBackend(&FF_PusBackendN7) != FF_SUCCESS)
#else
        (FF_PusSetBackend(&FF_PusBackendOpen) != FF_SUCCESS)
#endif
    ) {
        FF_PalPanic("PUS initialisation failed");
    }

    if (FF_LinkInit() != FF_SUCCESS) {
        FF_PalPanic("link initialisation failed");
    }

    FF_EvsSetListener(ff_reqtest_event_listener);
    FF_PusInstallPanicReporter(FF_LinkSendUntil);
    (void)FF_AppTmOutputInit();
    (void)FF_AppHkRegister();

    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"CMD_INGEST", FF_AppCmdIngestMain, 32 * 1024, 40, 8},
                           &id);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"TM_OUTPUT", FF_AppTmOutputMain, 16 * 1024, 30, 4},
                           NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"SCHEDULER", FF_AppSchedulerMain, 16 * 1024, 25, 4},
                           NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"HK", FF_AppHkMain, 16 * 1024, 50, 4}, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"HS", FF_AppHsMain, 16 * 1024, 35, 4}, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"WD", FF_AppWdMain, 8 * 1024, 70, 2}, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"SAMPLE", FF_AppSampleMain, 16 * 1024, 60, 8}, NULL);
    /* Highest priority so the suite runs before any other application can emit
     * telemetry into the capture window or trigger a later failure path. */
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"REQ_TEST", FF_AppReqTestMain, 24 * 1024, 20, 4},
                           NULL);

    FF_PalLog("frick-fsw: %u applications registered\n", (unsigned)FF_EsGetAppCount());

    /* Deterministic T1 evidence: run the suite before the other applications
     * start, so a later start-up failure (for example the unwired-watchdog
     * abort, NC-PAL-01) cannot truncate the evidence. FF_AppReqTestRun() is
     * idempotent, so the registered REQ_TEST application does not repeat it. */
    (void)FF_AppReqTestRun();

    if (FF_EsStartAll() != FF_SUCCESS) {
        FF_PalPanic("application start failed");
    }

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
