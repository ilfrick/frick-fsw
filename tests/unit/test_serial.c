/*
 * frick-fsw - MSS MMUART serial driver unit tests (milestone M4)
 *
 * Exercises the NS16550 (wide-register MPFS MMUART) register logic on the host
 * with an independent mock modelled on the documented 4-byte-spaced map
 * (THR 0x00, LCR 0x0C, LSR 0x14, SCR 0x1C) — SRS-REQ-048. The same driver is
 * exercised against the real MMIO in the Renode functional test (milestone M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/serial/mpfs/ff_serial_mpfs.h"
#include <string.h>

#define MOCK_WORDS 8u /* covers 0x00..0x1C */

typedef struct
{
    uint8_t reg[MOCK_WORDS];
    uint8_t tx[16];
    int tx_n;
} MockUart_t;

static MockUart_t g_regs;
static FF_SerialMpfs_t g_drv;

static uint8_t mock_read8(void *ctx, uint32_t off)
{
    MockUart_t *m = (MockUart_t *)ctx;
    return m->reg[(off / 4u) & (MOCK_WORDS - 1u)];
}

static void mock_write8(void *ctx, uint32_t off, uint8_t val)
{
    MockUart_t *m = (MockUart_t *)ctx;
    if ((off / 4u) == (FF_UART_REG_THR / 4u)) {
        if (m->tx_n < 16) {
            m->tx[m->tx_n++] = val;
        }
    }
    else {
        m->reg[(off / 4u) & (MOCK_WORDS - 1u)] = val;
    }
}

static void test_serial(void)
{
    FF_SerialBus_t bus = {&g_regs, mock_read8, mock_write8};
    uint8_t tx[2] = {0xDEu, 0xADu};
    uint8_t rx[4];
    uint16_t got = 0u;

    memset(&g_regs, 0, sizeof(g_regs));
    g_regs.reg[FF_UART_REG_LSR / 4u] = FF_UART_LSR_THR_EMPTY | FF_UART_LSR_TX_EMPTY;

    /* Setup rejects NULL. */
    FF_TEST_ASSERT(FF_SerialMpfsSetup(NULL, &bus) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SerialMpfsSetup(&g_drv, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SerialMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.serial.init != NULL);

    /* init sets the Line Control Register (8 data bits, 1 stop, no parity). */
    FF_TEST_ASSERT(g_drv.serial.init(115200u, 8u, 0u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_WLEN_MASK) == 0x03u);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_PEN) == 0u);

    /* 2 stop bits + even parity (PEN and EPS set). */
    FF_TEST_ASSERT(g_drv.serial.init(115200u, 8u, 2u, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_STOP2) != 0u);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_PEN) != 0u);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_EPS) != 0u);

    /* Odd parity: PEN set, EPS clear. */
    FF_TEST_ASSERT(g_drv.serial.init(115200u, 8u, 1u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_PEN) != 0u);
    FF_TEST_ASSERT((g_regs.reg[FF_UART_REG_LCR / 4u] & FF_UART_LCR_EPS) == 0u);

    FF_TEST_ASSERT(g_drv.serial.init(115200u, 4u, 0u, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.serial.init(115200u, 8u, 3u, 1u) == FF_INVALID_ARGUMENT);

    /* scratch-register MMIO self-test (wide offset). */
    FF_TEST_ASSERT(FF_SerialMpfsScratchSelfTest(&g_drv) == FF_SUCCESS);
    FF_TEST_ASSERT(g_regs.reg[FF_UART_REG_SCR / 4u] == 0xA5u);

    /* write drives the transmitter holding register (wide offset). */
    FF_TEST_ASSERT(g_drv.serial.write(tx, 2u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_regs.tx_n == 2);
    FF_TEST_ASSERT(g_regs.tx[0] == 0xDEu);
    FF_TEST_ASSERT(g_regs.tx[1] == 0xADu);
    FF_TEST_ASSERT(g_drv.serial.write(NULL, 1u, 0u) == FF_INVALID_ARGUMENT);

    /* read returns received bytes when the line status has data ready. */
    g_regs.reg[FF_UART_REG_LSR / 4u] = FF_UART_LSR_DATA_READY;
    g_regs.reg[FF_UART_REG_THR / 4u] = 0x55u;
    FF_TEST_ASSERT(g_drv.serial.read(rx, 1u, &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 1u);
    FF_TEST_ASSERT(rx[0] == 0x55u);
    FF_TEST_ASSERT(g_drv.serial.read(NULL, 1u, &got, 0u) == FF_INVALID_ARGUMENT);

    /* flush and RS-485 direction control. */
    g_regs.reg[FF_UART_REG_LSR / 4u] = FF_UART_LSR_TX_EMPTY;
    FF_TEST_ASSERT(g_drv.serial.flush() == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.serial.set_rs485(1u, 40u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.rs485 == 1u);
}

FF_TEST_MAIN(test_serial)
