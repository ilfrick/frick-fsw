/*
 * frick-fsw - MSS GPIO driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Register logic for the Microchip PolarFire SoC MSS GPIO ports. Static, no
 * allocation. Single instance (the HAL GPIO contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/gpio/mpfs/ff_gpio_mpfs.h"

static FF_GpioMpfs_t *g_gpio; /* single bound instance */

static FF_Status_t gpio_init(void)
{
    return FF_SUCCESS;
}

/* Configure a pin as input or output by setting/clearing its output-enable bit
 * in the per-pin GPIO_CFG[pin] register. */
static FF_Status_t gpio_configure(uint8_t pin, FF_GpioMode_t mode)
{
    uint32_t cfg;
    uint32_t bit;

    if ((g_gpio == NULL) || (pin >= g_gpio->pins)) {
        return FF_INVALID_ARGUMENT;
    }
    bit = 1u << pin;
    cfg = g_gpio->bus.read32(g_gpio->bus.ctx, FF_GPIO_CFG_OFF(pin));
    if (mode == FF_GPIO_OUT) {
        cfg |= FF_GPIO_CFG_OUT_EN;
        cfg &= ~FF_GPIO_CFG_IN_EN;
        g_gpio->dir |= bit;
    }
    else {
        cfg |= FF_GPIO_CFG_IN_EN;
        cfg &= ~FF_GPIO_CFG_OUT_EN;
        g_gpio->dir &= ~bit;
    }
    g_gpio->bus.write32(g_gpio->bus.ctx, FF_GPIO_CFG_OFF(pin), cfg);
    return FF_SUCCESS;
}

static FF_Status_t gpio_write(uint8_t pin, uint8_t value)
{
    uint32_t out;
    uint32_t bit;

    if ((g_gpio == NULL) || (pin >= g_gpio->pins)) {
        return FF_INVALID_ARGUMENT;
    }
    bit = 1u << pin;
    out = g_gpio->bus.read32(g_gpio->bus.ctx, FF_GPIO_REG_OUT);
    if ((value & 1u) != 0u) {
        out |= bit;
    }
    else {
        out &= ~bit;
    }
    g_gpio->bus.write32(g_gpio->bus.ctx, FF_GPIO_REG_OUT, out);
    return FF_SUCCESS;
}

/* Read the pin level. For an output pin the driven value is read back from the
 * output register (validates the MMIO round trip); for an input pin the input
 * register is read. */
static FF_Status_t gpio_read(uint8_t pin, uint8_t *value)
{
    uint32_t reg;
    uint32_t bit;

    if ((g_gpio == NULL) || (pin >= g_gpio->pins) || (value == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    bit = 1u << pin;
    reg = ((g_gpio->dir & bit) != 0u) ? FF_GPIO_REG_OUT : FF_GPIO_REG_IN;
    *value = (uint8_t)((g_gpio->bus.read32(g_gpio->bus.ctx, reg) >> pin) & 1u);
    return FF_SUCCESS;
}

static FF_Status_t gpio_attach_irq(uint8_t pin, FF_GpioEdge_t edge, FF_GpioIrqCb_t cb, void *ctx)
{
    (void)pin;
    (void)edge;
    (void)cb;
    (void)ctx;
    return FF_NOT_IMPLEMENTED;
}

static FF_Status_t gpio_detach_irq(uint8_t pin)
{
    (void)pin;
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t FF_GpioMpfsSetup(FF_GpioMpfs_t *g, const FF_GpioBus_t *bus, uint8_t pins)
{
    if ((g == NULL) || (bus == NULL) || (pins == 0u) || (pins > FF_GPIO_MPFS_PINS)) {
        return FF_INVALID_ARGUMENT;
    }
    g->bus = *bus;
    g->dir = 0u;
    g->pins = pins;
    g->gpio.name = "mpfs-gpio";
    g->gpio.init = gpio_init;
    g->gpio.configure = gpio_configure;
    g->gpio.write = gpio_write;
    g->gpio.read = gpio_read;
    g->gpio.attach_irq = gpio_attach_irq;
    g->gpio.detach_irq = gpio_detach_irq;
    g_gpio = g;
    return FF_SUCCESS;
}
