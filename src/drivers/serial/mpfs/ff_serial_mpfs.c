/*
 * frick-fsw - MSS MMUART serial driver (PolarFire SoC), HAL backend (M4)
 *
 * NS16550 (wide-register MPFS MMUART) register logic. Static, no allocation.
 * Single instance (the HAL serial contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/serial/mpfs/ff_serial_mpfs.h"

static FF_SerialMpfs_t *g_ser; /* single bound instance */

#define FF_SERIAL_SPIN_LIMIT 1000000u

/* Line Control Register: word length (bits 0-1), stop bits (bit 2), parity
 * (bit 3 enable, bit 4 select: 0 = odd, 1 = even). `parity`: 0 = none,
 * 1 = odd, 2 = even. */
static FF_Status_t serial_init(uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits)
{
    uint8_t lcr;

    (void)baud; /* divisor programming is board-clock specific (see GAP-A5) */
    if ((g_ser == NULL) || (data_bits < 5u) || (data_bits > 8u) || (parity > 2u)) {
        return FF_INVALID_ARGUMENT;
    }
    lcr = (uint8_t)((data_bits - 5u) & FF_UART_LCR_WLEN_MASK);
    if (stop_bits == 2u) {
        lcr |= FF_UART_LCR_STOP2;
    }
    if (parity != 0u) {
        lcr |= FF_UART_LCR_PEN;
        if (parity == 2u) {
            lcr |= FF_UART_LCR_EPS; /* even */
        }
    }
    g_ser->bus.write8(g_ser->bus.ctx, FF_UART_REG_LCR, lcr);
    return FF_SUCCESS;
}

static FF_Status_t serial_write(const uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if ((g_ser == NULL) || ((buf == NULL) && (len != 0u))) {
        return FF_INVALID_ARGUMENT;
    }
    for (uint16_t i = 0u; i < len; i++) {
        uint32_t spin = 0u;
        while (
            ((g_ser->bus.read8(g_ser->bus.ctx, FF_UART_REG_LSR) & FF_UART_LSR_THR_EMPTY) == 0u) &&
            (spin < FF_SERIAL_SPIN_LIMIT)) {
            spin++;
        }
        g_ser->bus.write8(g_ser->bus.ctx, FF_UART_REG_THR, buf[i]);
    }
    return FF_SUCCESS;
}

static FF_Status_t serial_read(uint8_t *buf, uint16_t cap, uint16_t *got, uint32_t timeout_ms)
{
    uint16_t n = 0u;
    (void)timeout_ms;
    if ((g_ser == NULL) || (buf == NULL) || (got == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    while (n < cap) {
        uint32_t spin = 0u;
        while (
            ((g_ser->bus.read8(g_ser->bus.ctx, FF_UART_REG_LSR) & FF_UART_LSR_DATA_READY) == 0u) &&
            (spin < FF_SERIAL_SPIN_LIMIT)) {
            spin++;
        }
        if (spin >= FF_SERIAL_SPIN_LIMIT) {
            break; /* timeout: return what was received */
        }
        buf[n] = g_ser->bus.read8(g_ser->bus.ctx, FF_UART_REG_THR);
        n++;
    }
    *got = n;
    return FF_SUCCESS;
}

static FF_Status_t serial_flush(void)
{
    uint32_t spin = 0u;
    if (g_ser == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    while (((g_ser->bus.read8(g_ser->bus.ctx, FF_UART_REG_LSR) & FF_UART_LSR_TX_EMPTY) == 0u) &&
           (spin < FF_SERIAL_SPIN_LIMIT)) {
        spin++;
    }
    return FF_SUCCESS;
}

/* RS-485 half-duplex direction control: the transceiver DE/RE pin is board
 * specific (a GPIO); the driver records the request for the integrator (the
 * actual DE/RE toggle is wired when a control line is bound). */
static FF_Status_t serial_set_rs485(uint8_t enable, uint32_t de_assert_us)
{
    if (g_ser == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    g_ser->rs485 = enable;
    g_ser->de_assert_us = de_assert_us;
    return FF_SUCCESS;
}

FF_Status_t FF_SerialMpfsScratchSelfTest(FF_SerialMpfs_t *s)
{
    uint8_t v;
    if (s == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    s->bus.write8(s->bus.ctx, FF_UART_REG_SCR, 0xA5u);
    v = s->bus.read8(s->bus.ctx, FF_UART_REG_SCR);
    return (v == 0xA5u) ? FF_SUCCESS : FF_ERROR;
}

FF_Status_t FF_SerialMpfsSetup(FF_SerialMpfs_t *s, const FF_SerialBus_t *bus)
{
    if ((s == NULL) || (bus == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    s->bus = *bus;
    s->rs485 = 0u;
    s->de_assert_us = 0u;
    s->serial.name = "mpfs-mmuart";
    s->serial.init = serial_init;
    s->serial.write = serial_write;
    s->serial.read = serial_read;
    s->serial.flush = serial_flush;
    s->serial.set_rs485 = serial_set_rs485;
    g_ser = s;
    return FF_SUCCESS;
}
