/*
 * frick-fsw - HAL contract: SPI and I2C master buses
 *
 * SPI and I2C also act as transports for external controllers that the MPFS
 * MSS does not provide natively (e.g. a CAN-FD controller), so keeping the
 * contract generic is deliberate (SRS-REQ-031).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_SPI_H
#define FF_HAL_SPI_H

#include "common/ff_types.h"

typedef struct FF_Spi FF_Spi_t;
struct FF_Spi
{
    const char *name;
    FF_Status_t (*init)(uint32_t clock_hz, uint8_t mode, uint8_t cs);
    FF_Status_t (*transfer)(const uint8_t *tx, uint8_t *rx, uint16_t len);
    FF_Status_t (*cs_assert)(uint8_t cs);
    FF_Status_t (*cs_release)(uint8_t cs);
};

typedef struct FF_I2c FF_I2c_t;
struct FF_I2c
{
    const char *name;
    FF_Status_t (*init)(uint32_t clock_hz);
    FF_Status_t (*write)(uint8_t addr7, const uint8_t *buf, uint16_t len);
    FF_Status_t (*read)(uint8_t addr7, uint8_t *buf, uint16_t cap, uint16_t *got);
    FF_Status_t (*write_read)(uint8_t addr7, const uint8_t *w, uint16_t wlen, uint8_t *r,
                              uint16_t rcap, uint16_t *rgot);
};

#endif /* FF_HAL_SPI_H */
