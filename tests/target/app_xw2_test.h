/*
 * frick-fsw - XW-2 test-only harness entry points
 *
 * TEST-ONLY (FF_ENABLE_EXTWD_TARGET_TEST). Declared here rather than in
 * src/apps/app.h so that no production header changes for this increment.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_APP_XW2_TEST_H
#define FF_APP_XW2_TEST_H

#include "core/common/ff_types.h"

/* Runs XW-T1..XW-T12 in the single-threaded init context, before FF_EsStartAll. */
FF_Status_t FF_AppXw2RunPreInit(void);
/* Runs XW-B1 from a task, after the applications have started. */
FF_Status_t FF_AppXw2RunPostBoot(void);
/* Task entry point (registered by the XW-2 entry point). */
void FF_AppXw2TestMain(void *arg);

#endif /* FF_APP_XW2_TEST_H */
