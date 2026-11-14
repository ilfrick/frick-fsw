/*
 * frick-fsw - In-house OBCP engine placeholder (milestone M16)
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 *
 * Scope-bounded placeholder for the M16 in-house OBCP engine. It implements the
 * FF_Obcp_t contract with a VM instruction budget, safe abort and cleanup, and
 * reuses the FF_OBCP_STATE_* state machine already defined in ff_obcp.h.
 *
 * This is a placeholder skeleton: production PUS S18 binding, mission
 * activation, target/hardware execution, WCET evidence and third-party
 * dependency removal are explicitly out of scope and remain authority-owned.
 */
#ifndef FF_OBCP_INHOUSE_H
#define FF_OBCP_INHOUSE_H

#include "common/ff_types.h"
#include "obcp/ff_obcp.h"

#define FF_OBCP_INHOUSE_SCRIPT_MAX_BYTES 4096u

/* In-house engine configuration: statically allocated, caller-owned resources. */
typedef struct
{
    char *heap;
    size_t heap_size;
    uint8_t *packet_buffer;
    uint32_t packet_buffer_size;
} FF_ObcpInhouseConfig_t;

/* In-house engine status: reuses FF_OBCP_STATE_* from ff_obcp.h. */
typedef struct
{
    FF_ObcpState_t state;
    bool engine_reported_success;
    bool abort_requested;
    bool budget_exhausted;
} FF_ObcpInhouseStatus_t;

/* Initialize the in-house engine. max_vm_instructions must be > 0 on every
 * FF_ObcpInhouseExecutePy call; a zero budget fails closed. */
FF_Status_t FF_ObcpInhouseInit(const FF_ObcpInhouseConfig_t *config);

/* Execute a script bounded by max_vm_instructions. Returns FF_OBCP_STATE_* via
 * FF_ObcpInhouseGetStatus. */
FF_Status_t FF_ObcpInhouseExecutePy(const char *script, size_t script_len,
                                    size_t max_vm_instructions);

/* Cooperative abort: observed at the next step boundary. */
FF_Status_t FF_ObcpInhouseRequestAbort(void);

/* Read the current in-house engine status. */
FF_Status_t FF_ObcpInhouseGetStatus(FF_ObcpInhouseStatus_t *status);

#endif /* FF_OBCP_INHOUSE_H */
