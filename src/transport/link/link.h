/*
 * frick-fsw - Link layer (TM/TC physical + data link)
 *
 * A link moves opaque CCSDS frames between the flight software and the
 * outside world (UART, SpaceWire, IP, TSN, radio). Stream framing is backend-
 * specific; the UART backend retains partial and coalesced data and returns one
 * complete packet per recv call. The transport codec is
 * link agnostic; this header defines the frame-oriented byte-stream contract
 * plus reference backends: an in-memory loopback (simulator/tests), UART,
 * IP (UDP/TCP, SRS-REQ-060), and TSN over GEM (SRS-REQ-061).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_LINK_H
#define FF_LINK_H

#include "common/ff_types.h"

typedef struct
{
    const char *name;
    FF_Status_t (*init)(void);
    FF_Status_t (*send)(const uint8_t *data, uint16_t len);
    FF_Status_t (*send_until)(const uint8_t *data, uint16_t len, uint64_t deadline_ns);
    FF_Status_t (*recv)(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms);
} FF_Link_t;

FF_Status_t FF_LinkInit(void);
FF_Status_t FF_LinkSelect(const FF_Link_t *link);
const FF_Link_t *FF_LinkGet(void);
FF_Status_t FF_LinkSend(const uint8_t *data, uint16_t len);
FF_Status_t FF_LinkSendUntil(const uint8_t *data, uint16_t len, uint64_t deadline_ns);
FF_Status_t FF_LinkRecv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms);

/* Loopback helpers (simulator + unit tests) */
FF_Status_t FF_LinkLoopbackPush(const uint8_t *data, uint16_t len);
FF_Status_t FF_LinkLoopbackPop(uint8_t *data, uint16_t cap, uint16_t *len);

extern const FF_Link_t FF_LinkLoopbackBackend;
extern const FF_Link_t FF_LinkUartBackend;
extern const FF_Link_t FF_LinkIpBackend;
extern const FF_Link_t FF_LinkTsnBackend;

#endif /* FF_LINK_H */
