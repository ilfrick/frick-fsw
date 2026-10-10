/*
 * frick-fsw - Link layer (TM/TC physical + data link)
 *
 * A link moves opaque CCSDS frames between the flight software and the
 * outside world (UART, SpaceWire, radio). The transport codec is link
 * agnostic; this header only defines the byte-stream contract plus two
 * reference backends: an in-memory loopback (simulator/tests) and a UART.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_LINK_H
#define FF_LINK_H

#include "common/ff_types.h"

typedef struct
{
    const char  *name;
    FF_Status_t (*init)(void);
    FF_Status_t (*send)(const uint8_t *data, uint16_t len);
    FF_Status_t (*recv)(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms);
} FF_Link_t;

FF_Status_t      FF_LinkInit(void);
FF_Status_t      FF_LinkSelect(const FF_Link_t *link);
const FF_Link_t *FF_LinkGet(void);
FF_Status_t      FF_LinkSend(const uint8_t *data, uint16_t len);
FF_Status_t      FF_LinkRecv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms);

/* Loopback helpers (simulator + unit tests) */
FF_Status_t FF_LinkLoopbackPush(const uint8_t *data, uint16_t len);
FF_Status_t FF_LinkLoopbackPop(uint8_t *data, uint16_t cap, uint16_t *len);

extern const FF_Link_t FF_LinkLoopbackBackend;
extern const FF_Link_t FF_LinkUartBackend;

#endif /* FF_LINK_H */
