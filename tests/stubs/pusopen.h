/*
 * frick-fsw - controlled PUSopen SDK test double interface
 *
 * This header is used only by test_pusopen. It models the subset of the
 * user-supplied commercial SDK API referenced by the adapter; it is not vendor
 * source and does not establish compatibility with any PUSopen release.
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_TEST_STUB_PUSOPEN_H
#define FF_TEST_STUB_PUSOPEN_H

#include <stddef.h>
#include <stdint.h>

typedef enum { PUSOPEN_OK = 0, PUSOPEN_ERROR = -1 } PUSopen_Status_t;

typedef struct
{
    uint8_t service;
    uint8_t subservice;
    uint16_t counter;
    uint32_t time_s;
    uint16_t time_f;
    uint8_t ack_flags;
    uint16_t source_id;
    uint8_t *data;
    size_t data_len;
} PUSopen_Packet_t;

PUSopen_Status_t PUSopen_Init(void);
PUSopen_Status_t PUSopen_EncodeTm(const PUSopen_Packet_t *pkt, uint8_t *out, size_t out_cap,
                                  size_t *written);
PUSopen_Status_t PUSopen_DecodeTc(const uint8_t *raw, uint16_t len, PUSopen_Packet_t *pkt);

#endif /* FF_TEST_STUB_PUSOPEN_H */
