/*
 * frick-fsw - MSS MMUART serial driver unit tests (milestone M4)
 *
 * Exercises the NS16550 (wide-register MPFS MMUART) register logic on the host
 * with an independent mock modelled on the documented 4-byte-spaced map
 * (THR 0x00, LCR 0x0C, LSR 0x14, SCR 0x1C) — SRS-REQ-048, plus the RS-485
 * half-duplex DE/RE turnaround sequence with an injected direction line
 * (SRS-REQ-064), nominal and out-of-bound. The same driver is
 * exercised against the real MMIO in the Renode functional test (milestone M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
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

/* RS-485 DE/RE direction-line mock (SRS-REQ-064): records the driven levels and
 * the timed holds so the turnaround sequence is checked against explicit
 * expected values (norm N22). */
typedef struct
{
    int writes[8];
    int write_n;
    uint32_t delays[8];
    int delay_n;
} MockDeRe_t;

static MockDeRe_t g_dere;

static void mock_de_write(void *ctx, uint8_t level)
{
    MockDeRe_t *m = (MockDeRe_t *)ctx;
    if (m->write_n < 8) {
        m->writes[m->write_n++] = (int)level;
    }
}

static void mock_de_delay(void *ctx, uint32_t us)
{
    MockDeRe_t *m = (MockDeRe_t *)ctx;
    if (m->delay_n < 8) {
        m->delays[m->delay_n++] = us;
    }
}

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

    /* flush and RS-485 direction control (no line bound: request recorded). */
    g_regs.reg[FF_UART_REG_LSR / 4u] = FF_UART_LSR_TX_EMPTY;
    FF_TEST_ASSERT(g_drv.serial.flush() == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.serial.set_rs485(1u, 40u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.rs485 == 1u);

    /* --- RS-485 DE/RE direction line (SRS-REQ-064): out-of-bound cases --- */
    memset(&g_dere, 0, sizeof(g_dere));
    {
        FF_SerialDeRe_t line = {&g_dere, mock_de_write, mock_de_delay};
        FF_SerialDeRe_t incomplete = {&g_dere, NULL, mock_de_delay};

        FF_TEST_ASSERT(FF_SerialMpfsBindDeRe(NULL, &line) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(FF_SerialMpfsBindDeRe(&g_drv, NULL) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(FF_SerialMpfsBindDeRe(&g_drv, &incomplete) == FF_INVALID_ARGUMENT);
        incomplete.write = mock_de_write;
        incomplete.delay_us = NULL;
        FF_TEST_ASSERT(FF_SerialMpfsBindDeRe(&g_drv, &incomplete) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(g_drv.dere == NULL); /* nothing bound by the rejected calls */

        /* Nominal bind: the line starts in receive (0). */
        FF_TEST_ASSERT(FF_SerialMpfsBindDeRe(&g_drv, &line) == FF_SUCCESS);
        FF_TEST_ASSERT(g_drv.dere == &line);
        FF_TEST_ASSERT(g_dere.write_n == 1);
        FF_TEST_ASSERT(g_dere.writes[0] == 0);
        FF_TEST_ASSERT(g_drv.de_line == 0u);

        /* Assert: the request is bounded (only 0/1 are accepted) and the line
         * is driven to transmit. */
        FF_TEST_ASSERT(g_drv.serial.set_rs485(2u, 40u) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(g_drv.de_line == 0u); /* rejected: no drive happened */
        FF_TEST_ASSERT(g_drv.serial.set_rs485(1u, 40u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_drv.rs485 == 1u);
        FF_TEST_ASSERT(g_dere.write_n == 2);
        FF_TEST_ASSERT(g_dere.writes[1] == 1);
        FF_TEST_ASSERT(g_drv.de_line == 1u);

        /* Release with an empty transmitter: no drain needed, the requested
         * hold is applied, then the line returns to receive. */
        g_regs.reg[FF_UART_REG_LSR / 4u] = FF_UART_LSR_TX_EMPTY;
        FF_TEST_ASSERT(g_drv.serial.set_rs485(0u, 40u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_dere.write_n == 3);
        FF_TEST_ASSERT(g_dere.writes[2] == 0);
        FF_TEST_ASSERT(g_drv.de_line == 0u);
        FF_TEST_ASSERT(g_dere.delay_n == 1);
        FF_TEST_ASSERT(g_dere.delays[0] == 40u);
        FF_TEST_ASSERT(g_drv.drain_polls == 0u);
        FF_TEST_ASSERT(g_drv.drain_timeouts == 0u);

        /* Release while the transmitter is not empty: the drain poll is
         * bounded, the diagnostic is counted and the bus is still released
         * (fail-safe) so no transceiver is left driving the pair. */
        g_regs.reg[FF_UART_REG_LSR / 4u] = 0u;
        FF_TEST_ASSERT(g_drv.serial.set_rs485(1u, 0u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_drv.serial.set_rs485(0u, 0u) == FF_TIMEOUT);
        FF_TEST_ASSERT(g_drv.de_line == 0u);
        FF_TEST_ASSERT(g_dere.writes[g_dere.write_n - 1] == 0);
        FF_TEST_ASSERT(g_drv.drain_polls == (uint16_t)FF_SERIAL_DRAIN_SPIN_LIMIT);
        FF_TEST_ASSERT(g_drv.drain_timeouts == 1u);
        /* de_assert_us == 0: no hold is applied. */
        FF_TEST_ASSERT(g_dere.delay_n == 1);

        /* A zero hold is legal and never calls the delay hook. */
        g_regs.reg[FF_UART_REG_LSR / 4u] = FF_UART_LSR_TX_EMPTY;
        FF_TEST_ASSERT(g_drv.serial.set_rs485(1u, 1u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_drv.serial.set_rs485(0u, 0u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_dere.delay_n == 1); /* unchanged */
    }

    /* Unbind by setting up the driver again (fresh instance state). */
    FF_TEST_ASSERT(FF_SerialMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.dere == NULL);
}

FF_TEST_MAIN(test_serial)
