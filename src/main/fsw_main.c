/*
 * frick-fsw - Flight software main sequence
 *
 * Boot and initialisation sequence (cFS style):
 *
 *   1. PAL         - platform / RTOS bootstrap
 *   2. SB / EVS    - Software Bus and Event Services
 *   3. ES          - application registry
 *   4. TBL / TIME  - Table and Time Services
 *   5. PUS         - packet utilisation layer, open backend
 *   6. Link        - TM/TC transport
 *   7. Apps        - registration and start
 *
 * SPDX-License-Identifier: AGPL-3.0-only
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

static void ff_event_listener(const FF_EvsEvent_t *evt)
{
    uint8_t  payload[FF_PUS_MAX_TM_FRAME];
    uint16_t text_len = 0u;
    uint16_t total;

    while ((text_len < sizeof(evt->text)) && (evt->text[text_len] != '\0'))
    {
        text_len++;
    }
    if (text_len > (FF_PUS_MAX_TM_FRAME - 3u))
    {
        text_len = FF_PUS_MAX_TM_FRAME - 3u;
    }
    payload[0] = (uint8_t)(evt->event_id >> 8);
    payload[1] = (uint8_t)(evt->event_id & 0xFFu);
    payload[2] = evt->type;
    memcpy(&payload[3], evt->text, text_len);
    total = (uint16_t)(text_len + 3u);

    (void)FF_PusEmitEvent(evt->event_id, evt->type, payload, total);
}

static void ff_print_banner(void)
{
    FF_PalLog("\n");
    FF_PalLog("+-----------------------------------------------------------+\n");
    FF_PalLog("|  frick-fsw \"%s\" v%s                           |\n", FF_CODENAME, FF_VERSION_STRING);
    FF_PalLog("|  cFS-style flight software, PUS TM/TC, RTEMS on RISC-V     |\n");
    FF_PalLog("|  platform : %-49s |\n", FF_PLATFORM_NAME);
    FF_PalLog("+-----------------------------------------------------------+\n");
    FF_PalLog("\n");
}

int ff_fsw_main(void)
{
    FF_AppId_t id;

    ff_print_banner();

    if ((FF_PalInit() != FF_SUCCESS) ||
        (FF_SbInit() != FF_SUCCESS) ||
        (FF_EvsInit() != FF_SUCCESS) ||
        (FF_EsInit() != FF_SUCCESS) ||
        (FF_TblInit() != FF_SUCCESS) ||
        (FF_TimeInit() != FF_SUCCESS) ||
        (FF_BoardInit() != FF_SUCCESS) ||
        (FF_MissionTablesInit() != FF_SUCCESS))
    {
        FF_PalPanic("core initialisation failed");
    }

    /* PUS layer with the default open backend. */
    if ((FF_PusInit() != FF_SUCCESS) ||
        (FF_PusSetBackend(&FF_PusBackendOpen) != FF_SUCCESS))
    {
        FF_PalPanic("PUS initialisation failed");
    }

    if (FF_LinkInit() != FF_SUCCESS)
    {
        FF_PalPanic("link initialisation failed");
    }

    /* Forward every on-board event to ground as PUS service 5 telemetry. */
    FF_EvsSetListener(ff_event_listener);
    (void)FF_AppTmOutputInit();
    (void)FF_AppHkRegister();

    /* Application registration (name, entry, stack, priority, queue depth). */
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){ "CMD_INGEST", FF_AppCmdIngestMain, 32 * 1024, 40, 8 }, &id);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){ "TM_OUTPUT",  FF_AppTmOutputMain, 16 * 1024, 30, 4 }, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){ "SCHEDULER", FF_AppSchedulerMain, 16 * 1024, 25, 4 }, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){ "HK",        FF_AppHkMain,       16 * 1024, 50, 4 }, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){ "HS",        FF_AppHsMain,       16 * 1024, 35, 4 }, NULL);
    (void)FF_EsRegisterApp(&(FF_AppConfig_t){ "SAMPLE",    FF_AppSampleMain,   16 * 1024, 60, 8 }, NULL);

    FF_PalLog("frick-fsw: %u applications registered\n", (unsigned)FF_EsGetAppCount());

    if (FF_EsStartAll() != FF_SUCCESS)
    {
        FF_PalPanic("application start failed");
    }

    /* Idle / background loop. */
    for (;;)
    {
        FF_PalTaskDelayMs(5000u);
    }
}