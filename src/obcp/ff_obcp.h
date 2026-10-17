/*
 * frick-fsw - N7 Space OBCP engine integration contract
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_OBCP_H
#define FF_OBCP_H

#include "common/ff_types.h"
#include "obcpengine.h"

#define FF_OBCP_SCRIPT_MAX_BYTES 4096u

typedef enum {
    FF_OBCP_STATE_UNINITIALIZED = 0,
    FF_OBCP_STATE_READY,
    FF_OBCP_STATE_RUNNING,
    FF_OBCP_STATE_RETURNED,
    FF_OBCP_STATE_ABORT_REQUESTED,
    FF_OBCP_STATE_ERROR
} FF_ObcpState_t;

/*
 * Caller-owned, statically allocated resources and service bindings for the
 * pinned single-context N7 engine. The context callbacks are the integration
 * seam for the mission's datapool, EVS, Time, packet and control services.
 */
typedef struct
{
    obcp_engine_context_t services;
    char *heap;
    size_t heap_size;
    uint8_t *packet_buffer;
    uint32_t packet_buffer_size;
} FF_ObcpConfig_t;

typedef struct
{
    FF_ObcpState_t state;
    /* The upstream execution API result is reported verbatim; it is not an
     * independent script-success oracle at the pinned revision. */
    bool engine_reported_success;
    bool abort_requested;
} FF_ObcpStatus_t;

FF_Status_t FF_ObcpInit(const FF_ObcpConfig_t *config);
FF_Status_t FF_ObcpExecutePy(const char *script, size_t script_len);
/* Abort is cooperative: it is observed at the next obcpcontrol.beginstep(). */
FF_Status_t FF_ObcpRequestAbort(void);
FF_Status_t FF_ObcpGetStatus(FF_ObcpStatus_t *status);

#endif /* FF_OBCP_H */
