/*
 * frick-fsw - HAL contract: SpaceWire and RMAP
 *
 * ECSS SpaceWire suite: link (ECSS-E-ST-50-12), protocol identification
 * (50-51), remote memory access (50-52, RMAP) and CCSDS packet transfer
 * (50-53) (SRS-REQ-032). Not native to the MPFS MSS: shipped as a generic
 * fabric-device backend (see docs/development-plan.md).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_SPACEWIRE_H
#define FF_HAL_SPACEWIRE_H

#include "common/ff_types.h"

typedef struct FF_SpaceWire FF_SpaceWire_t;
typedef void (*FF_SpwRxCb_t)(const uint8_t *data, uint16_t len, void *ctx);

struct FF_SpaceWire
{
    const char *name;
    FF_Status_t (*init)(void);
    FF_Status_t (*send)(const uint8_t *data, uint16_t len);
    /* ECSS-E-ST-50-51: send with an explicit protocol identifier. */
    FF_Status_t (*send_pid)(uint8_t protocol_id, const uint8_t *data, uint16_t len);
    void (*set_rx_callback)(FF_SpwRxCb_t cb, void *ctx);
};

/* ECSS-E-ST-50-52: Remote Memory Access Protocol. */
typedef struct FF_Rmap FF_Rmap_t;
struct FF_Rmap
{
    const char *name;
    FF_Status_t (*init)(FF_SpaceWire_t *spw, uint8_t target);
    FF_Status_t (*read)(uint32_t address, uint8_t *resp, uint16_t cap, uint16_t *len);
    FF_Status_t (*write)(uint32_t address, const uint8_t *data, uint16_t len);
    /* Read-modify-write with AND/OR masks (mask_len bytes, 1..255 per ECSS). */
    FF_Status_t (*rmw)(uint32_t address, const uint8_t *and_mask, const uint8_t *or_mask,
                       uint16_t mask_len);
};

#endif /* FF_HAL_SPACEWIRE_H */
