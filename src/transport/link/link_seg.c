/*
 * frick-fsw - PUS-C link segmentation/reassembly implementation
 *
 * Reference encapsulation profile for stream-oriented transports (TCP):
 * each frame is prefixed with a 2-byte big-endian length field.
 *
 * The RX path buffers partial frames in rx_partial[] until the full frame
 * (including the length prefix) has been read. All buffers are statically
 * allocated (norm N15 / SRS-REQ-015); no dynamic allocation.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "fsw_config.h"
#include "link_seg.h"
#include "common/ff_types.h"
#include <string.h>

void FF_LinkSegInit(FF_LinkSeg_t *seg, FF_LinkByteStream_t stream)
{
    if (seg == NULL) {
        return;
    }
    seg->stream = stream;
    seg->rx_len = 0u;
    (void)memset(seg->rx_partial, 0, sizeof(seg->rx_partial));
}

FF_Status_t FF_LinkSegSendFrame(FF_LinkSeg_t *seg, const uint8_t *frame, uint16_t len)
{
    if ((seg == NULL) || (seg->stream.write_bytes == NULL) || (frame == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len > FF_LINK_SEG_MAX_FRAME) {
        return FF_BAD_SIZE;
    }

    /* Write the 2-byte big-endian length prefix. */
    uint8_t prefix[FF_LINK_SEG_PREFIX_LEN];
    prefix[0] = (uint8_t)((len >> 8) & 0xFFu);
    prefix[1] = (uint8_t)(len & 0xFFu);

    FF_Status_t status = seg->stream.write_bytes(seg->stream.ctx, prefix, FF_LINK_SEG_PREFIX_LEN);
    if (status != FF_SUCCESS) {
        return status;
    }
    return seg->stream.write_bytes(seg->stream.ctx, frame, len);
}

FF_Status_t FF_LinkSegRecvFrame(FF_LinkSeg_t *seg, uint8_t *frame, uint16_t cap, uint16_t *len,
                                uint32_t timeout_ms)
{
    if ((seg == NULL) || (seg->stream.read_bytes == NULL) || (frame == NULL) || (len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }

    /* Phase 1: if we don't yet have the length prefix, read bytes until we do. */
    while (seg->rx_len < FF_LINK_SEG_PREFIX_LEN) {
        uint16_t got = 0u;
        uint16_t room = (uint16_t)(FF_LINK_SEG_PREFIX_LEN - seg->rx_len);
        uint8_t tmp[FF_LINK_SEG_PREFIX_LEN];
        FF_Status_t st = seg->stream.read_bytes(seg->stream.ctx, tmp, room, &got, timeout_ms);
        if (st != FF_SUCCESS) {
            return st;
        }
        if (got == 0u) {
            return FF_TIMEOUT; /* non-blocking read returned nothing */
        }
        for (uint16_t i = 0u; i < got; i++) {
            seg->rx_partial[seg->rx_len + i] = tmp[i];
        }
        seg->rx_len = (uint16_t)(seg->rx_len + got);
    }

    /* Phase 2: decode the frame length from the prefix. */
    uint16_t frame_len = ((uint16_t)seg->rx_partial[0] << 8) | (uint16_t)seg->rx_partial[1];
    if (frame_len > FF_LINK_SEG_MAX_FRAME) {
        return FF_BAD_SIZE;
    }
    if (frame_len > cap) {
        return FF_BUFFER_FULL;
    }

    /* Phase 3: if we already have the entire frame in rx_partial, copy and return. */
    uint16_t have_payload = (uint16_t)(seg->rx_len - FF_LINK_SEG_PREFIX_LEN);
    if (have_payload >= frame_len) {
        (void)memcpy(frame, &seg->rx_partial[FF_LINK_SEG_PREFIX_LEN], frame_len);
        *len = frame_len;
        /* Discard consumed bytes; keep any remaining partial data. */
        uint16_t remaining = (uint16_t)(seg->rx_len - FF_LINK_SEG_PREFIX_LEN - frame_len);
        if (remaining > 0u) {
            (void)memmove(seg->rx_partial, &seg->rx_partial[FF_LINK_SEG_PREFIX_LEN + frame_len],
                          remaining);
        }
        seg->rx_len = remaining;
        return FF_SUCCESS;
    }

    /* Phase 4: copy what we have, then read the rest from the stream. */
    if (have_payload > 0u) {
        (void)memcpy(frame, &seg->rx_partial[FF_LINK_SEG_PREFIX_LEN], have_payload);
    }

    uint16_t total = have_payload;
    while (total < frame_len) {
        uint16_t got = 0u;
        uint16_t want = (uint16_t)(frame_len - total);
        FF_Status_t st =
            seg->stream.read_bytes(seg->stream.ctx, &frame[total], want, &got, timeout_ms);
        if (st != FF_SUCCESS) {
            return st;
        }
        if (got == 0u) {
            /* Partial data: preserve the length prefix + whatever payload
             * has been read so far in rx_partial so the next call resumes. */
            if (total > 0u) {
                (void)memcpy(&seg->rx_partial[FF_LINK_SEG_PREFIX_LEN], frame, total);
            }
            seg->rx_len = (uint16_t)(FF_LINK_SEG_PREFIX_LEN + total);
            *len = total;
            return FF_TIMEOUT;
        }
        total = (uint16_t)(total + got);
    }

    *len = total;
    seg->rx_len = 0u;
    return FF_SUCCESS;
}
