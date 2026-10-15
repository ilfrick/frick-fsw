/*
 * frick-fsw - HAL contract: serial (UART / RS-422 / RS-485)
 *
 * RS-422 and RS-485 are UARTs with an external transceiver; RS-485 half-duplex
 * additionally needs transceiver direction control (DE/RE), exposed here
 * (SRS-REQ-030). On the MPFS these map to the MMUART peripherals plus a GPIO.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
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
    /* RS-485 half-duplex direction control (SRS-REQ-064; ignored for
     * RS-422/full duplex, which needs no turnaround).
     *
     * `enable` != 0 asserts the transceiver for transmit (DE); `enable` == 0
     * releases it back to receive (RE) after a bounded transmitter-empty poll
     * and a `de_assert_us` hold supplied on that release call. The argument on
     * an assert call is not cached for later use. If that poll expires, the line is still released
     * after the hold and the call returns FF_TIMEOUT; this reports uncertain TX
     * and does not imply successful transmission. A backend that has a
     * board-specific DE/RE line bound drives it; otherwise the request is
     * recorded for the board integrator (the line is not driven).
     * Values other than 0/1 are rejected with FF_INVALID_ARGUMENT. */
    FF_Status_t (*set_rs485)(uint8_t enable, uint32_t de_assert_us);
};

#endif /* FF_HAL_SERIAL_H */
