/*
 * frick-fsw - XW-2 test-only entry point (emulated-target watchdog contract)
 *
 * TEST-ONLY. Compiled only with FF_ENABLE_EXTWD_TARGET_TEST=ON into the separate
 * executable `frick_fsw_xw2`. It mirrors the flight sequence of ff_fsw_main()
 * (which stays byte-identical and is not linked here) and adds the XW-2 harness:
 *
 *   framework init -> PUS -> link -> reporter/TM sink -> HK registration
 *     -> register the seven production apps + XW2 harness
 *     -> XW-T1..XW-T12 (single-threaded, authored clock)
 *     -> FF_EsStartAll()  (the PRODUCTION feeder arms the device here)
 *     -> XW-B1 (counts only)
 *
 * The device seam is substituted at link time by -Wl,--wrap of exactly the three
 * ff_pal_wd_hw_* hooks (tests/stubs/ff_test_extwd_pal.c + the validated model).
 * No file under src/ or config/ is modified, and no #if is added to production.
 *
 * NOT: a device, HIL, N18, all-I/O, timing or mission evidence.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
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
#include "app_xw2_test.h"
#include <string.h>

#if !FF_ENABLE_EXTWD_TARGET_TEST
#error "fsw_main_xw2.c is only valid with FF_ENABLE_EXTWD_TARGET_TEST=1"
#endif

static void ff_xw2_event_listener(const FF_EvsEvent_t *evt)
{
    uint8_t payload[FF_PUS_MAX_TM_FRAME];
    uint16_t text_len = 0u;
    uint16_t total;

    while ((text_len < sizeof(evt->text)) && (evt->text[text_len] != '\0')) {
        text_len++;
    }
    if (text_len > (FF_PUS_MAX_TM_FRAME - 3u)) {
        text_len = FF_PUS_MAX_TM_FRAME - 3u;
    }
    payload[0] = (uint8_t)(evt->event_id >> 8);
    payload[1] = (uint8_t)(evt->event_id & 0xFFu);
    payload[2] = evt->type;
    memcpy(&payload[3], evt->text, text_len);
    total = (uint16_t)(text_len + 3u);

    (void)FF_PusEmitEvent(evt->event_id, evt->type, payload, total);
}

int ff_fsw_main(void)
{
    FF_AppId_t id;

    FF_PalLog("\n");
    FF_PalLog("+-----------------------------------------------------------+\n");
    FF_PalLog("|  frick-fsw \"%s\" v%s                           |\n", FF_CODENAME,
              FF_VERSION_STRING);
    FF_PalLog("|  XW-2 external-watchdog contract image (TEST-ONLY)        |\n");
    FF_PalLog("|  platform : %-49s |\n", FF_PLATFORM_NAME);
    FF_PalLog("+-----------------------------------------------------------+\n");
    FF_PalLog("\n");

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

    FF_EvsSetListener(ff_xw2_event_listener);
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
    /* Harness app: lowest priority so XW-B1 observes the production feeder. */
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){"XW2_TEST", FF_AppXw2TestMain, 24 * 1024, 75, 4},
                           NULL);

    FF_PalLog("frick-fsw: %u applications registered\n", (unsigned)FF_EsGetAppCount());

    /* XW-T1..XW-T12 run before the applications start: the calling context is
     * single-threaded and the model clock is advanced only here. */
    (void)FF_AppXw2RunPreInit();

    if (FF_EsStartAll() != FF_SUCCESS) {
        FF_PalPanic("application start failed");
    }

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
