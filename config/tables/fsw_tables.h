/*
 * frick-fsw - On-board configuration tables
 *
 * Reference use of the Table Services with an ECSS-style load/validate/
 * activate lifecycle. The values below are the mission default; in flight
 * the table is updated by ground using PUS service 20 or the dedicated
 * memory/table commands.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_FSW_TABLES_H
#define FF_FSW_TABLES_H

#include "common/ff_types.h"

#define FF_MISSION_CFG_MAGIC 0x4D434647u /* 'MCFG' */

typedef struct
{
    uint32_t magic;
    uint16_t hk_period_ms;
    uint16_t telemetry_period_ms;
    uint16_t scheduler_tick_ms;
    uint16_t watchdog_timeout_s;
} FF_MissionCfg_t;

FF_Status_t FF_MissionTablesInit(void);
const FF_MissionCfg_t *FF_GetMissionCfg(void);

#endif /* FF_FSW_TABLES_H */
