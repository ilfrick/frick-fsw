/*
 * frick-fsw - Executive Services implementation
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "es.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include <string.h>

typedef struct
{
    bool used;
    FF_AppId_t id;
    char name[FF_ES_APP_NAME_LEN];
    FF_PalTaskFunc_t entry;
    uint32_t stack_bytes;
    uint8_t priority;
    FF_TaskId_t task;
    FF_AppState_t state;
} ff_es_app_t;

static ff_es_app_t g_apps[FF_ES_MAX_APPS];
static uint16_t g_app_count;

FF_Status_t FF_EsInit(void)
{
    memset(g_apps, 0, sizeof(g_apps));
    g_app_count = 0u;
    return FF_SUCCESS;
}

FF_Status_t FF_EsRegisterApp(const FF_AppConfig_t *cfg, FF_AppId_t *out_id)
{
    if ((cfg == NULL) || (cfg->name == NULL) || (cfg->entry == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (strlen(cfg->name) >= FF_ES_APP_NAME_LEN) {
        return FF_NAME_TOO_LONG;
    }
    for (uint16_t i = 0; i < FF_ES_MAX_APPS; i++) {
        if (!g_apps[i].used) {
            FF_AppId_t id = (FF_AppId_t)(i + 1u); /* app ids are 1-based */
            g_apps[i].used = true;
            g_apps[i].id = id;
            g_apps[i].entry = cfg->entry;
            g_apps[i].stack_bytes = cfg->stack_bytes;
            g_apps[i].priority = cfg->priority;
            g_apps[i].state = FF_APP_LOADED;
            (void)strncpy(g_apps[i].name, cfg->name, FF_ES_APP_NAME_LEN - 1);

            if (FF_SbCreatePipe(id, cfg->queue_depth) != FF_SUCCESS) {
                g_apps[i].used = false;
                return FF_ERROR;
            }
            g_app_count++;
            if (out_id != NULL) {
                *out_id = id;
            }
            return FF_SUCCESS;
        }
    }
    return FF_NO_RESOURCE;
}

static ff_es_app_t *ff_es_find(FF_AppId_t id)
{
    for (uint16_t i = 0; i < FF_ES_MAX_APPS; i++) {
        if (g_apps[i].used && (g_apps[i].id == id)) {
            return &g_apps[i];
        }
    }
    return NULL;
}

FF_Status_t FF_EsStartApp(FF_AppId_t id)
{
    ff_es_app_t *app = ff_es_find(id);
    if (app == NULL) {
        return FF_NOT_FOUND;
    }
    if (app->state == FF_APP_RUNNING) {
        return FF_SUCCESS;
    }
    if (FF_PalTaskCreate(&app->task, app->name, app->entry, (void *)(uintptr_t)app->id,
                         app->stack_bytes, app->priority) != FF_SUCCESS) {
        app->state = FF_APP_FAILED;
        (void)FF_EvsSend(FF_EVS_APP_ERROR, FF_EVS_ERROR, id, "app %s start failed", app->name);
        return FF_ERROR;
    }
    app->state = FF_APP_RUNNING;
    (void)FF_EvsSend(FF_EVS_APP_STARTED, FF_EVS_INFO, id, "app %s started", app->name);
    return FF_SUCCESS;
}

FF_Status_t FF_EsStartAll(void)
{
    FF_Status_t rc = FF_SUCCESS;
    for (uint16_t i = 0; i < FF_ES_MAX_APPS; i++) {
        if (g_apps[i].used) {
            if (FF_EsStartApp(g_apps[i].id) != FF_SUCCESS) {
                rc = FF_ERROR;
            }
        }
    }
    return rc;
}

FF_AppId_t FF_EsGetId(const char *name)
{
    if (name == NULL) {
        return FF_INVALID_ID;
    }
    for (uint16_t i = 0; i < FF_ES_MAX_APPS; i++) {
        if (g_apps[i].used && (strcmp(g_apps[i].name, name) == 0)) {
            return g_apps[i].id;
        }
    }
    return FF_INVALID_ID;
}

FF_AppState_t FF_EsGetState(FF_AppId_t id)
{
    ff_es_app_t *app = ff_es_find(id);
    return (app != NULL) ? app->state : FF_APP_FAILED;
}

uint32_t FF_EsGetAppCount(void)
{
    return g_app_count;
}
