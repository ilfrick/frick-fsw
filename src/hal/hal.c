/*
 * frick-fsw - Hardware/OS abstraction layer (HAL) core registry implementation
 *
 * Bounded, static, allocation-free registry (norm N10). No dynamic memory is
 * used at any time (SRS-REQ-015 / SSS-REQ-010).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "hal/hal.h"
#include "fsw_config.h"
#include <string.h>

static const char *g_names[FF_HAL_CLASS_COUNT][FF_HAL_MAX_BACKENDS];
static void *g_backends[FF_HAL_CLASS_COUNT][FF_HAL_MAX_BACKENDS];
static uint16_t g_count[FF_HAL_CLASS_COUNT];

static int ff_hal_class_ok(FF_HalClass_t cls)
{
    return ((int)cls >= 0) && ((int)cls < (int)FF_HAL_CLASS_COUNT);
}

FF_Status_t FF_HalInit(void)
{
    uint16_t c;
    uint16_t i;

    for (c = 0u; c < (uint16_t)FF_HAL_CLASS_COUNT; c++) {
        g_count[c] = 0u;
        for (i = 0u; i < (uint16_t)FF_HAL_MAX_BACKENDS; i++) {
            g_names[c][i] = NULL;
            g_backends[c][i] = NULL;
        }
    }
    return FF_SUCCESS;
}

FF_Status_t FF_HalRegister(FF_HalClass_t cls, const char *name, void *backend)
{
    uint16_t c = (uint16_t)cls;
    uint16_t i;

    if (!ff_hal_class_ok(cls) || (name == NULL) || (backend == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    for (i = 0u; i < g_count[c]; i++) {
        if (strcmp(g_names[c][i], name) == 0) {
            return FF_ALREADY_EXISTS;
        }
    }
    if (g_count[c] >= (uint16_t)FF_HAL_MAX_BACKENDS) {
        return FF_ERROR; /* class full */
    }
    g_names[c][g_count[c]] = name;
    g_backends[c][g_count[c]] = backend;
    g_count[c]++;
    return FF_SUCCESS;
}

void *FF_HalGet(FF_HalClass_t cls, const char *name)
{
    uint16_t c = (uint16_t)cls;
    uint16_t i;

    if (!ff_hal_class_ok(cls) || (name == NULL)) {
        return NULL;
    }
    for (i = 0u; i < g_count[c]; i++) {
        if (strcmp(g_names[c][i], name) == 0) {
            return g_backends[c][i];
        }
    }
    return NULL;
}

uint16_t FF_HalCount(FF_HalClass_t cls)
{
    return ff_hal_class_ok(cls) ? g_count[(uint16_t)cls] : 0u;
}

uint16_t FF_HalTotal(void)
{
    uint16_t c;
    uint16_t total = 0u;

    for (c = 0u; c < (uint16_t)FF_HAL_CLASS_COUNT; c++) {
        total = (uint16_t)(total + g_count[c]);
    }
    return total;
}
