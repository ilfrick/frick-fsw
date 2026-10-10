/*
 * frick-fsw - Event Services (EVS)
 *
 * Central event reporting, the analogue of the cFS Event Services (EVS).
 * Events are written to a circular on-board log and forwarded to an optional
 * listener (in flight this listener feeds PUS service 5, service 1
 * verification and the Health & Safety application).
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_EVS_H
#define FF_EVS_H

#include "common/ff_types.h"
#include "fsw_config.h"

typedef enum {
    FF_EVS_INFO = 1,
    FF_EVS_WARNING = 2,
    FF_EVS_ERROR = 3,
    FF_EVS_CRITICAL = 4
} FF_EvsType_t;

typedef struct
{
    uint16_t event_id;
    uint8_t type;
    uint8_t reserved;
    uint16_t app_id;
    uint64_t time_ns;
    char text[FF_EVS_TEXT_LEN];
} FF_EvsEvent_t;

typedef void (*FF_EvsListener_t)(const FF_EvsEvent_t *event);

FF_Status_t FF_EvsInit(void);
void FF_EvsSetListener(FF_EvsListener_t listener);
FF_Status_t FF_EvsSend(uint16_t event_id, FF_EvsType_t type, FF_AppId_t app, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));
uint32_t FF_EvsGetLog(FF_EvsEvent_t *out, uint32_t max_events);
uint32_t FF_EvsGetTotalCount(void);

#endif /* FF_EVS_H */
