/*
 * frick-fsw - HAL contract: GPIO and interrupt handling
 *
 * Digital IO with edge/level interrupt registration (SRS-REQ-031). The
 * interrupt-callback contract states the ISR/task rule used across the
 * framework: an ISR must do the minimum and defer work to a task (see
 * docs/architecture/hardware-abstraction.md §4).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_GPIO_H
#define FF_HAL_GPIO_H

#include "common/ff_types.h"

typedef enum { FF_GPIO_IN = 0, FF_GPIO_OUT, FF_GPIO_AF } FF_GpioMode_t;

typedef enum {
    FF_GPIO_RISING = 0,
    FF_GPIO_FALLING,
    FF_GPIO_BOTH,
    FF_GPIO_LEVEL_HIGH,
    FF_GPIO_LEVEL_LOW
} FF_GpioEdge_t;

typedef struct FF_Gpio FF_Gpio_t;
typedef void (*FF_GpioIrqCb_t)(uint8_t pin, void *ctx);

struct FF_Gpio
{
    const char *name;
    FF_Status_t (*init)(void);
    FF_Status_t (*configure)(uint8_t pin, FF_GpioMode_t mode);
    FF_Status_t (*write)(uint8_t pin, uint8_t value);
    FF_Status_t (*read)(uint8_t pin, uint8_t *value);
    /* Register an interrupt callback. ISRs must defer to a task. */
    FF_Status_t (*attach_irq)(uint8_t pin, FF_GpioEdge_t edge, FF_GpioIrqCb_t cb, void *ctx);
    FF_Status_t (*detach_irq)(uint8_t pin);
};

#endif /* FF_HAL_GPIO_H */
