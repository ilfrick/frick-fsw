/*
 * frick-fsw - HAL contract: CAN / CANopen / CAN-FD backend
 *
 * Classic CAN (ISO 11898-1 / CAN 2.0 A/B) and CAN-FD share one backend
 * contract; the project supplies a documented CANopen subset layered on it
 * (SRS-REQ-029). On the MPFS, classic CAN is an MSS resource while CAN-FD is a
 * fabric device reached through the FIC (see docs/development-plan.md).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_CAN_H
#define FF_HAL_CAN_H

#include "common/ff_types.h"

typedef struct FF_CanBackend FF_CanBackend_t;
typedef void (*FF_CanRxCb_t)(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len, void *ctx);

struct FF_CanBackend
{
    const char *name;
    uint8_t supports_fd;
    FF_Status_t (*init)(void);
    /* fd = 0 classic; fd = 1 CAN-FD (data_bitrate used when fd != 0). */
    FF_Status_t (*configure)(uint32_t bitrate, uint8_t fd, uint32_t data_bitrate);
    FF_Status_t (*transmit)(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len);
    void (*set_rx_callback)(FF_CanRxCb_t cb, void *ctx);
};

/* CANopen application-layer subset defined by SRS-REQ-063 over a CAN backend;
 * this interface does not assert full CiA 301/302/402 conformance. */
typedef struct FF_CanOpenStack FF_CanOpenStack_t;
struct FF_CanOpenStack
{
    const char *name;
    FF_Status_t (*init)(FF_CanBackend_t *can, uint8_t node_id);
    FF_Status_t (*sdo_write)(uint8_t node, uint16_t index, uint8_t sub, const uint8_t *data,
                             uint8_t len);
    FF_Status_t (*sdo_read)(uint8_t node, uint16_t index, uint8_t sub, uint8_t *data, uint8_t *len);
    FF_Status_t (*pdo_send)(uint8_t node, uint16_t cob_id, const uint8_t *data, uint8_t len);
};

#endif /* FF_HAL_CAN_H */
