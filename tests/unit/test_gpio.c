/*
 * frick-fsw - MSS GPIO driver unit tests (milestone M4)
 *
 * Exercises the GPIO register logic on the host with an independent mock MMIO
 * register file that models the Microchip PolarFire SoC MSS GPIO map (per-pin
 * GPIO_CFG at 0x00+pin*4, GPIO_IN=0x84, GPIO_OUT=0x88) — SRS-REQ-047. The same
 * driver is exercised against the real MMIO in the Renode functional test (M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/gpio/mpfs/ff_gpio_mpfs.h"

#define MOCK_WORDS 64u /* covers 0x00..0xFC */

typedef struct
{
    uint32_t reg[MOCK_WORDS];
} MockGpio_t;

static MockGpio_t g_regs;
static FF_GpioMpfs_t g_drv;

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    MockGpio_t *m = (MockGpio_t *)ctx;
    return m->reg[off / 4u];
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    MockGpio_t *m = (MockGpio_t *)ctx;
    m->reg[off / 4u] = val;
}

static void test_gpio(void)
{
    FF_GpioBus_t bus = {&g_regs, mock_read32, mock_write32};
    uint8_t v = 0u;

    for (uint32_t i = 0u; i < MOCK_WORDS; i++) {
        g_regs.reg[i] = 0u;
    }

    FF_TEST_ASSERT(FF_GpioMpfsSetup(&g_drv, &bus, 14u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.gpio.init != NULL);
    FF_TEST_ASSERT(g_drv.gpio.init() == FF_SUCCESS);

    /* Output configure sets bit 0 (output enable) in the per-pin CFG register,
     * not a shared register: neighbouring CFG[2] must stay untouched. */
    FF_TEST_ASSERT(g_drv.gpio.configure(3u, FF_GPIO_OUT) == FF_SUCCESS);
    FF_TEST_ASSERT((g_regs.reg[FF_GPIO_CFG_OFF(3u) / 4u] & FF_GPIO_CFG_OUT_EN) != 0u);
    FF_TEST_ASSERT((g_regs.reg[FF_GPIO_CFG_OFF(3u) / 4u] & FF_GPIO_CFG_IN_EN) == 0u);
    FF_TEST_ASSERT(g_regs.reg[FF_GPIO_CFG_OFF(2u) / 4u] == 0u);
    FF_TEST_ASSERT(g_regs.reg[FF_GPIO_CFG_OFF(1u) / 4u] == 0u);

    /* Output write/read round trip through GPIO_OUT (0x88). */
    FF_TEST_ASSERT(g_drv.gpio.write(3u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT((g_regs.reg[FF_GPIO_REG_OUT / 4u] & (1u << 3)) != 0u);
    FF_TEST_ASSERT(g_drv.gpio.read(3u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 1u);
    FF_TEST_ASSERT(g_drv.gpio.write(3u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.gpio.read(3u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0u);

    /* Input configure sets bit 1 (input enable) and clears output enable; read
     * uses GPIO_IN (0x84). */
    FF_TEST_ASSERT(g_drv.gpio.configure(5u, FF_GPIO_IN) == FF_SUCCESS);
    FF_TEST_ASSERT((g_regs.reg[FF_GPIO_CFG_OFF(5u) / 4u] & FF_GPIO_CFG_IN_EN) != 0u);
    FF_TEST_ASSERT((g_regs.reg[FF_GPIO_CFG_OFF(5u) / 4u] & FF_GPIO_CFG_OUT_EN) == 0u);
    g_regs.reg[FF_GPIO_REG_IN / 4u] = (1u << 5);
    FF_TEST_ASSERT(g_drv.gpio.read(5u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 1u);

    /* Bounds and argument validation (no UB shift before the check). */
    FF_TEST_ASSERT(g_drv.gpio.read(14u, &v) == FF_INVALID_ARGUMENT); /* GPIO0 has 14 pins */
    FF_TEST_ASSERT(g_drv.gpio.read(0xFFu, &v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.gpio.read(0u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.gpio.configure(14u, FF_GPIO_OUT) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.gpio.write(14u, 1u) == FF_INVALID_ARGUMENT);
    /* Setup rejects an invalid pin count. */
    FF_TEST_ASSERT(FF_GpioMpfsSetup(&g_drv, &bus, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_GpioMpfsSetup(&g_drv, &bus, 33u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_GpioMpfsSetup(&g_drv, &bus, 14u) == FF_SUCCESS);

    /* Interrupts are not implemented by this reference driver. */
    FF_TEST_ASSERT(g_drv.gpio.attach_irq(0u, FF_GPIO_RISING, NULL, NULL) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(g_drv.gpio.detach_irq(0u) == FF_NOT_IMPLEMENTED);
}

FF_TEST_MAIN(test_gpio)
