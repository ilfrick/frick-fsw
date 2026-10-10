/*
 * frick-fsw - HAL contract: serial (UART / RS-422 / RS-485)
 *
 * RS-422 and RS-485 are UARTs with an external transceiver; RS-485 half-duplex
 * additionally needs transceiver direction control (DE/RE), exposed here
 * (SRS-REQ-030). On the MPFS these map to the MMUART peripherals plus a GPIO.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_HAL_SERIAL_H
#define FF_HAL_SERIAL_H

#include "common/ff_types.h"

typedef struct FF_Serial FF_Serial_t;
struct FF_Serial
{
    const char *name;
    FF_Status_t (*init)(uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits);
    FF_Status_t (*write)(const uint8_t *buf, uint16_t len, uint32_t timeout_ms);
    FF_Status_t (*read)(uint8_t *buf, uint16_t cap, uint16_t *got, uint32_t timeout_ms);
    FF_Status_t (*flush)(void);
    /* RS-485 half-duplex direction control (ignored for RS-422/full duplex). */
    FF_Status_t (*set_rs485)(uint8_t enable, uint32_t de_assert_us);
};

#endif /* FF_HAL_SERIAL_H */
