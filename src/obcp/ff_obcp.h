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
    FF_OBCP_STATE_ERROR,
    /* Distinct from FF_OBCP_STATE_ERROR: the VM instruction budget was
     * exhausted in the interpreter dispatch path. The engine context remains
     * usable for a subsequent bounded execution. */
    FF_OBCP_STATE_BUDGET_EXHAUSTED,
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
    /* True when the last execution exhausted the VM instruction budget.
     * Cleared on the next FF_ObcpExecutePy call. */
    bool budget_exhausted;
} FF_ObcpStatus_t;

FF_Status_t FF_ObcpInit(const FF_ObcpConfig_t *config);
/* max_vm_instructions must be > 0 on every call; a zero budget fails closed.
 * Returns FF_OBCP_STATE_BUDGET_EXHAUSTED (via FF_ObcpGetStatus) when the
 * interpreter dispatch path exhausts the budget. */
FF_Status_t FF_ObcpExecutePy(const char *script, size_t script_len, size_t max_vm_instructions);
/* Abort is cooperative: it is observed at the next obcpcontrol.beginstep(). */
FF_Status_t FF_ObcpRequestAbort(void);
FF_Status_t FF_ObcpGetStatus(FF_ObcpStatus_t *status);

#endif /* FF_OBCP_H */
