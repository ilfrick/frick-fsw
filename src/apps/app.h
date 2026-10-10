/*
 * frick-fsw - Application common interface
 *
 * Every application is a task registered with Executive Services. The
 * signature matches FF_PalTaskFunc_t so it can be handed directly to ES.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_APP_H
#define FF_APP_H

#include "common/ff_types.h"

/* cFS style application entry points: void AppMain(void *arg) */
void FF_AppCmdIngestMain(void *arg);
void FF_AppTmOutputMain(void *arg);
void FF_AppSchedulerMain(void *arg);
void FF_AppHkMain(void *arg);
void FF_AppHsMain(void *arg);
void FF_AppSampleMain(void *arg);

/* Initialisation hooks called before ES starts the tasks. */
FF_Status_t FF_AppTmOutputInit(void);
FF_Status_t FF_AppHkRegister(void);

#endif /* FF_APP_H */
