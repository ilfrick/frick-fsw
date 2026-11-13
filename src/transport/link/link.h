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
    /* Deadline-bounded send used by the pre-reset report path (SRS-REQ-074).
     *
     * `deadline_ns` is an absolute FF_PalGetMonotonicNs() instant. A backend
     * that implements this writer returns:
     *   - FF_SUCCESS only when the whole transfer was accepted before the
     *     deadline;
     *   - FF_TIMEOUT when the deadline had already passed on entry (in which
     *     case nothing is transmitted) or when it elapsed during the transfer;
     *   - the ordinary argument/readiness statuses otherwise (FF_NOT_READY,
     *     FF_INVALID_ARGUMENT, backend error).
     * A backend that temporarily changes a shared device mode must inspect the
     * restoration result after every transfer attempt. If restoration fails,
     * it reports FF_ERROR and stops retrying; the device mode is then uncertain
     * and the caller must not infer that the transfer can safely be retried.
     * Implementations whose underlying transmit call is synchronous cannot
     * preempt a stalled driver; they enforce the deadline on entry and
     * re-check it after the transfer, so an overrun is reported as FF_TIMEOUT
     * rather than as an in-budget delivery. A NULL pointer means the backend
     * has no deadline-aware writer and FF_LinkSendUntil() reports
     * FF_NOT_IMPLEMENTED (NC-EXT-191). */
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
