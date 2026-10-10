/*
 * frick-fsw - On-board configuration tables implementation
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "fsw_tables.h"
#include "core/tbl/tbl.h"
#include <string.h>

static FF_MissionCfg_t g_cfg;

static FF_Status_t ff_cfg_validate(const void *image, size_t size)
{
    const FF_MissionCfg_t *c = (const FF_MissionCfg_t *)image;
    if ((size != sizeof(FF_MissionCfg_t)) || (c->magic != FF_MISSION_CFG_MAGIC)) {
        return FF_ERROR;
    }
    if ((c->hk_period_ms == 0u) || (c->watchdog_timeout_s == 0u)) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

FF_Status_t FF_MissionTablesInit(void)
{
    memset(&g_cfg, 0, sizeof(g_cfg));
    g_cfg.magic = FF_MISSION_CFG_MAGIC;
    g_cfg.hk_period_ms = 1000u;
    g_cfg.telemetry_period_ms = 1000u;
    g_cfg.scheduler_tick_ms = 100u;
    g_cfg.watchdog_timeout_s = 30u;

    FF_TblHandle_t h = FF_TblRegister(&(FF_Tbl_Def_t){.name = "mission_cfg",
                                                      .image = &g_cfg,
                                                      .size = sizeof(g_cfg),
                                                      .validate = ff_cfg_validate});
    return (h == FF_TBL_INVALID_HANDLE) ? FF_ERROR : FF_SUCCESS;
}

const FF_MissionCfg_t *FF_GetMissionCfg(void)
{
    return &g_cfg;
}