/*
 * frick-fsw - Application common interface
 *
 * Every application is a task registered with Executive Services. The
 * signature matches FF_PalTaskFunc_t so it can be handed directly to ES.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_APP_H
#define FF_APP_H

#include "common/ff_types.h"

/* cFS style application entry points: void AppMain(void *arg) */
void FF_AppCmdIngestMain(void *arg);
/* Process one link receive using the production maximum-packet buffer. */
FF_Status_t FF_AppCmdIngestReceiveOne(FF_AppId_t app, uint32_t timeout_ms);
void FF_AppTmOutputMain(void *arg);
void FF_AppSchedulerMain(void *arg);
void FF_AppHkMain(void *arg);
void FF_AppHsMain(void *arg);
/* One Health & Safety monitoring pass (FAILED-application scan + heartbeat).
 * Same behaviour as one iteration of the FF_AppHsMain loop; provided as a
 * bounded, non-blocking entry point for host integration tests (S8-VV-FU). */
void FF_AppHsStep(FF_AppId_t app);
void FF_AppSampleMain(void *arg);

/* Renode functional test application (norm N18; registered only when
 * FF_ENABLE_RENODE_TEST=1). */
void FF_AppRenodeTestMain(void *arg);

/* Initialisation hooks called before ES starts the tasks. */
FF_Status_t FF_AppTmOutputInit(void);
FF_Status_t FF_AppHkRegister(void);

#endif /* FF_APP_H */
