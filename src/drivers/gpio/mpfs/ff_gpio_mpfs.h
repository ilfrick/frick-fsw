/*
 * frick-fsw - MSS GPIO driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Implements the HAL GPIO contract (FF_Gpio_t) for the Microchip PolarFire SoC
 * MSS GPIO ports (GPIOPort.MPFS_GPIO). MMIO access is injected (FF_GpioBus_t)
 * so the register logic is testable on the host (mock bus) and reusable on the
 * target (real MMIO, exercised in the Renode functional test, milestone M11).
 *
 * Register map (Microchip PolarFire SoC MSS GPIO):
 *   - GPIO_CFG[pin] : 0x00 + pin*4 (per-pin config; bit 5 = output enable)
 *   - GPIO_IN       : 0x84 (pin input value)
 *   - GPIO_OUT      : 0x88 (pin output value)
 *
 * The HAL GPIO contract has no per-instance context, so the driver is a bounded
 * single-instance backend: FF_GpioMpfsSetup() binds the instance. FF_GPIO_MPFS_PINS
 * is the register-array size (32); the caller must use pins valid for the port
 * (e.g. GPIO0 exposes 14).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_GPIO_MPFS_H
#define FF_GPIO_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_gpio.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host). */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
} FF_GpioBus_t;

/* MPFS MSS GPIO register map (Microchip PolarFire SoC; matches the Renode
 * GPIOPort.MPFS_GPIO model). */
#define FF_GPIO_REG_IN 0x84u                        /* pin input value */
#define FF_GPIO_REG_OUT 0x88u                       /* pin output value */
#define FF_GPIO_CFG_OFF(pin) (0x00u + ((pin) * 4u)) /* per-pin config */
#define FF_GPIO_CFG_OUT_EN (1u << 0)                /* per-pin output (direction) enable */
#define FF_GPIO_CFG_IN_EN (1u << 1)                 /* per-pin input (direction) enable */

#define FF_GPIO_MPFS_PINS 32u

typedef struct
{
    FF_Gpio_t gpio; /* public HAL interface (must be first) */
    FF_GpioBus_t bus;
    uint32_t dir; /* shadow direction: 1 = output */
    uint8_t pins; /* valid pin count of the bound port (1..FF_GPIO_MPFS_PINS) */
} FF_GpioMpfs_t;

/* Bind the instance and complete the FF_Gpio_t vtable. `pins` is the number of
 * valid pins of the bound port (GPIO0=14, GPIO1=24, GPIO2=32). Returns
 * FF_SUCCESS, or FF_INVALID_ARGUMENT on a NULL argument or out-of-range pins. */
FF_Status_t FF_GpioMpfsSetup(FF_GpioMpfs_t *g, const FF_GpioBus_t *bus, uint8_t pins);

#endif /* FF_GPIO_MPFS_H */
