/*
 * frick-fsw - Executive Services (ES)
 *
 * Application registry, task creation and lifecycle management. The analogue
 * of the cFS Executive Services. Applications are described by a static
 * FF_AppConfig_t and started as deterministic tasks of the PAL.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_ES_H
#define FF_ES_H

#include "common/ff_types.h"
#include "fsw_config.h"
#include "core/pal/pal.h"

typedef enum { FF_APP_LOADED = 0, FF_APP_RUNNING, FF_APP_STOPPED, FF_APP_FAILED } FF_AppState_t;

typedef struct
{
    const char *name;
    FF_PalTaskFunc_t entry;
    uint32_t stack_bytes;
    uint8_t priority;
    uint16_t queue_depth;
} FF_AppConfig_t;

FF_Status_t FF_EsInit(void);
FF_Status_t FF_EsRegisterApp(const FF_AppConfig_t *cfg, FF_AppId_t *out_id);
FF_Status_t FF_EsStartApp(FF_AppId_t id);
FF_Status_t FF_EsStartAll(void);
FF_AppId_t FF_EsGetId(const char *name);
FF_AppState_t FF_EsGetState(FF_AppId_t id);
uint32_t FF_EsGetAppCount(void);

#endif /* FF_ES_H */
