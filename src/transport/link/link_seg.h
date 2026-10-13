/*
 * frick-fsw - PUS-C link segmentation/reassembly (M4-residual, SRS-REQ-059)
 *
 * Bounded, allocation-free framing for stream-oriented transports (TCP).
 * Provides a 2-byte big-endian length prefix so a byte stream can be split
 * into discrete CCSDS/PUS-C frames. Datagram-oriented transports (UDP) and
 * frame-oriented transports (Ethernet/TSN) use the datagram/frame boundary
 * directly and do not invoke this layer.
 *
 * Reference encapsulation profile (mission ICD selects):
 *   TCP stream  : [uint16_t length_be][frame bytes]
 *   UDP datagram: [frame bytes]  (no prefix; datagram boundary delimits)
 *   TSN Ethernet: [Eth hdr][frame bytes][pad to 64] (EtherType 0x88F5)
 *
 * The framing byte order, prefix width and max frame are compile-time
 * constants so integrators can adjust via fsw_config.h overrides.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_LINK_SEG_H
#define FF_LINK_SEG_H

#include "common/ff_types.h"
#include "link.h"

/* Configurable limits (override in fsw_config.h if needed). */
#ifndef FF_LINK_SEG_MAX_FRAME
#ifdef FF_PUS_MAX_TM_FRAME
#define FF_LINK_SEG_MAX_FRAME FF_PUS_MAX_TM_FRAME
#else
#define FF_LINK_SEG_MAX_FRAME 1024u
#endif
#endif

/* Encapsulation profile identifier (reference default). */
typedef enum {
    FF_LINK_ENCAP_UDP, /* UDP: each datagram carries one frame                 */
    FF_LINK_ENCAP_TCP, /* TCP: 2-byte BE length prefix + frame                 */
    FF_LINK_ENCAP_ETH, /* Ethernet/TSN: Eth hdr + EtherType 0x88F5 + frame     */
} FF_LinkEncapProfile_t;

/* Length-prefix width for stream framing (reference default: 2 bytes). */
#ifndef FF_LINK_SEG_PREFIX_LEN
#define FF_LINK_SEG_PREFIX_LEN 2u
#endif

/* Byte-stream I/O contract that the seg layer wraps. */
typedef struct
{
    FF_Status_t (*write_bytes)(void *ctx, const uint8_t *data, uint16_t len);
    FF_Status_t (*read_bytes)(void *ctx, uint8_t *data, uint16_t cap, uint16_t *len,
                              uint32_t timeout_ms);
    void *ctx;
} FF_LinkByteStream_t;

/* Segmentation/reassembly context. Holds a partial RX buffer so frames that
 * span multiple stream reads are reassembled correctly. */
typedef struct
{
    FF_LinkByteStream_t stream; /* underlying byte I/O                */
    uint8_t rx_partial[FF_LINK_SEG_PREFIX_LEN + FF_LINK_SEG_MAX_FRAME];
    uint16_t rx_len; /* bytes currently held in rx_partial            */
} FF_LinkSeg_t;

/* Initialise the segmentation layer over a byte stream. */
void FF_LinkSegInit(FF_LinkSeg_t *seg, FF_LinkByteStream_t stream);

/* Send one frame with the length-prefix encapsulation profile. */
FF_Status_t FF_LinkSegSendFrame(FF_LinkSeg_t *seg, const uint8_t *frame, uint16_t len);

/* Receive one frame, reassembling across stream boundaries. */
FF_Status_t FF_LinkSegRecvFrame(FF_LinkSeg_t *seg, uint8_t *frame, uint16_t cap, uint16_t *len,
                                uint32_t timeout_ms);

#endif /* FF_LINK_SEG_H */
