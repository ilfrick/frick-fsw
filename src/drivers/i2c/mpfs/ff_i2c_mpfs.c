/*
 * frick-fsw - MSS I2C driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Polled I2C master over the Microchip PolarFire SoC MSS I2C (CoreI2C), register
 * logic on an injected byte MMIO bus. Static, no allocation. Single instance
 * (the HAL I2C contract carries no context).
 *
 * Transaction shape (the Microchip CoreI2C ISR convention, applied in-line): the
 * driver clears SI to launch each phase and polls the Control register for the
 * next SI, then reads the Status state code and either loads the next byte or
 * issues STOP. See the header for the register map, the serial-clock derivation
 * and the known limitations.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/i2c/mpfs/ff_i2c_mpfs.h"

FF_STATIC_ASSERT(FF_I2C_MPFS_CLK_HZ > 0u, i2c_clk_nonzero);

static FF_I2cMpfs_t *g_i2c; /* single bound instance */

/* CoreI2C serial-clock dividers, ascending by divisor: the first entry whose
 * serial clock does not exceed the requested rate is selected (if none fits, the
 * slowest divider is used). Code = CR2:CR1:CR0 (see i2c_ctrl_div_bits). */
const FF_I2cDiv_t ff_i2c_div_table[FF_I2C_DIV_COUNT] = {
    {6u, 60u},  /* PCLK/60  */
    {5u, 120u}, /* PCLK/120 */
    {3u, 160u}, /* PCLK/160 */
    {2u, 192u}, /* PCLK/192 */
    {1u, 224u}, /* PCLK/224 */
    {0u, 256u}, /* PCLK/256 */
    {4u, 960u}, /* PCLK/960 */
};

static uint8_t i2c_rd(uint32_t off)
{
    return g_i2c->bus.read8(g_i2c->bus.ctx, off);
}

static void i2c_wr(uint32_t off, uint8_t val)
{
    g_i2c->bus.write8(g_i2c->bus.ctx, off, val);
}

/* Map the 3-bit CoreI2C divider code onto the Control-register CR2:CR1:CR0 bits
 * (Microchip MSS_I2C_init writes the code's bits 0/1/2 to CR0/CR1/CR2). */
static uint8_t i2c_ctrl_div_bits(uint8_t code)
{
    uint8_t v = 0u;

    if ((code & 0x1u) != 0u) {
        v |= FF_I2C_CTRL_CR0;
    }
    if ((code & 0x2u) != 0u) {
        v |= FF_I2C_CTRL_CR1;
    }
    if ((code & 0x4u) != 0u) {
        v |= FF_I2C_CTRL_CR2;
    }
    return v;
}

/* Select the fastest divider whose serial clock does not exceed the requested
 * rate. When the request is below every achievable rate the slowest divider is
 * selected (the minimum achievable SCL is FF_I2C_MIN_SCL_HZ, ~104.17 kHz at the
 * 100 MHz reference); such a request cannot be honoured and init() still returns
 * FF_SUCCESS, so the caller must request a rate >= FF_I2C_MIN_SCL_HZ. */
static uint8_t i2c_pick_div(uint32_t target)
{
    uint8_t i;

    for (i = 0u; i < FF_I2C_DIV_COUNT; i++) {
        uint32_t scl = FF_I2C_MPFS_CLK_HZ / (uint32_t)ff_i2c_div_table[i].div;
        if (scl <= target) {
            return ff_i2c_div_table[i].code;
        }
    }
    return ff_i2c_div_table[FF_I2C_DIV_COUNT - 1u].code;
}

/* Poll until the controller signals a completed phase (SI set). */
static FF_Status_t i2c_wait_si(uint8_t *st)
{
    uint32_t i;

    for (i = 0u; i < FF_I2C_POLL_MAX; i++) {
        if ((i2c_rd(FF_I2C_REG_CTRL) & FF_I2C_CTRL_SI) != 0u) {
            *st = i2c_rd(FF_I2C_REG_STATUS);
            return FF_SUCCESS;
        }
    }
    return FF_TIMEOUT;
}

/* Clear SI, launching the phase prepared in the Control/Data registers. */
static void i2c_clear_si(uint8_t extra)
{
    i2c_wr(FF_I2C_REG_CTRL, (uint8_t)(g_i2c->ctrl | extra));
}

static FF_Status_t i2c_check_ready(void)
{
    if (g_i2c == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_i2c->ready == 0u) {
        return FF_NOT_READY;
    }
    return FF_SUCCESS;
}

/* clock_hz is the target serial clock (SCL) frequency; the serial-clock divider
 * is derived from the APB reference clock (FF_I2C_MPFS_CLK_HZ). */
static FF_Status_t i2c_init(uint32_t clock_hz)
{
    uint8_t code;

    if ((g_i2c == NULL) || (clock_hz == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    code = i2c_pick_div(clock_hz);
    g_i2c->ctrl = (uint8_t)(i2c_ctrl_div_bits(code) | FF_I2C_CTRL_ENS1);
    g_i2c->ready = 1u;
    i2c_wr(FF_I2C_REG_CTRL, g_i2c->ctrl);
    return FF_SUCCESS;
}

/* START (or, on a bus already in a master phase, repeated START). */
static FF_Status_t i2c_start(void)
{
    uint8_t st;

    i2c_wr(FF_I2C_REG_CTRL, (uint8_t)(g_i2c->ctrl | FF_I2C_CTRL_STA));
    if (i2c_wait_si(&st) != FF_SUCCESS) {
        return FF_TIMEOUT;
    }
    if ((st != FF_I2C_ST_START) && (st != FF_I2C_ST_RESTART)) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

static void i2c_stop(void)
{
    if (g_i2c != NULL) {
        i2c_wr(FF_I2C_REG_CTRL, (uint8_t)(g_i2c->ctrl | FF_I2C_CTRL_STO));
    }
}

/* Load a byte and clear SI to transmit it; require the expected ACK state. */
static FF_Status_t i2c_send(uint8_t byte, uint8_t st_ack)
{
    uint8_t st;

    i2c_wr(FF_I2C_REG_DATA, byte);
    i2c_clear_si(0u);
    if (i2c_wait_si(&st) != FF_SUCCESS) {
        return FF_TIMEOUT;
    }
    if (st != st_ack) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

/* Transmit SLA+R and wait for the address ACK. */
static FF_Status_t i2c_begin_read(uint8_t addr7)
{
    uint8_t st;

    i2c_wr(FF_I2C_REG_DATA, (uint8_t)((addr7 << 1) | 1u));
    i2c_clear_si(0u);
    if (i2c_wait_si(&st) != FF_SUCCESS) {
        return FF_TIMEOUT;
    }
    if (st != FF_I2C_ST_MR_SLA_ACK) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

/* Receive cap bytes (AA is asserted for every byte but the last). On entry the
 * controller has ACKed SLA+R (state MR_DATA_ACK pending); on a success the
 * caller issues STOP. Returns FF_TIMEOUT / FF_ERROR on a failed phase. */
static FF_Status_t i2c_receive(uint8_t *buf, uint16_t cap, uint16_t *got)
{
    uint16_t i;
    uint8_t st;

    /* Clear SI to receive the first byte, ACKing it when more are wanted. */
    i2c_clear_si((cap > 1u) ? FF_I2C_CTRL_AA : 0u);
    for (i = 0u; i < cap; i++) {
        if (i2c_wait_si(&st) != FF_SUCCESS) {
            return FF_TIMEOUT;
        }
        if ((st != FF_I2C_ST_MR_DATA_ACK) && (st != FF_I2C_ST_MR_DATA_NACK)) {
            return FF_ERROR;
        }
        buf[i] = i2c_rd(FF_I2C_REG_DATA);
        if ((i + 1u) < cap) {
            /* ACK the next byte unless it is the last one. */
            uint8_t aa = (((i + 1u) < (cap - 1u)) ? FF_I2C_CTRL_AA : 0u);
            i2c_clear_si(aa);
        }
    }
    if (got != NULL) {
        *got = cap;
    }
    return FF_SUCCESS;
}

static FF_Status_t i2c_write(uint8_t addr7, const uint8_t *buf, uint16_t len)
{
    uint16_t i;
    FF_Status_t rc;

    rc = i2c_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if (((buf == NULL) && (len != 0u)) || (addr7 > FF_I2C_ADDR7_MAX)) {
        return FF_INVALID_ARGUMENT;
    }

    rc = i2c_start();
    if (rc == FF_SUCCESS) {
        rc = i2c_send((uint8_t)((addr7 << 1) | 0u), FF_I2C_ST_MT_SLA_ACK);
    }
    for (i = 0u; (i < len) && (rc == FF_SUCCESS); i++) {
        rc = i2c_send(buf[i], FF_I2C_ST_MT_DATA_ACK);
    }
    i2c_stop();
    return rc;
}

static FF_Status_t i2c_read(uint8_t addr7, uint8_t *buf, uint16_t cap, uint16_t *got)
{
    FF_Status_t rc;

    rc = i2c_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if ((buf == NULL) || (cap == 0u) || (addr7 > FF_I2C_ADDR7_MAX)) {
        return FF_INVALID_ARGUMENT;
    }
    if (got != NULL) {
        *got = 0u;
    }

    rc = i2c_start();
    if (rc == FF_SUCCESS) {
        rc = i2c_begin_read(addr7);
    }
    if (rc == FF_SUCCESS) {
        rc = i2c_receive(buf, cap, got);
    }
    i2c_stop();
    return rc;
}

static FF_Status_t i2c_write_read(uint8_t addr7, const uint8_t *w, uint16_t wlen, uint8_t *r,
                                  uint16_t rcap, uint16_t *rgot)
{
    uint16_t i;
    uint8_t st;
    FF_Status_t rc;

    rc = i2c_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if ((r == NULL) || (rcap == 0u) || (addr7 > FF_I2C_ADDR7_MAX) ||
        ((w == NULL) && (wlen != 0u))) {
        return FF_INVALID_ARGUMENT;
    }
    if (rgot != NULL) {
        *rgot = 0u;
    }
    if (wlen == 0u) {
        return i2c_read(addr7, r, rcap, rgot);
    }

    rc = i2c_start();
    if (rc == FF_SUCCESS) {
        rc = i2c_send((uint8_t)((addr7 << 1) | 0u), FF_I2C_ST_MT_SLA_ACK);
    }
    for (i = 0u; (i < wlen) && (rc == FF_SUCCESS); i++) {
        rc = i2c_send(w[i], FF_I2C_ST_MT_DATA_ACK);
    }
    if (rc == FF_SUCCESS) {
        /* Repeated START, then SLA+R. */
        i2c_wr(FF_I2C_REG_CTRL, (uint8_t)(g_i2c->ctrl | FF_I2C_CTRL_STA));
        if (i2c_wait_si(&st) != FF_SUCCESS) {
            rc = FF_TIMEOUT;
        }
        else if (st != FF_I2C_ST_RESTART) {
            rc = FF_ERROR;
        }
    }
    if (rc == FF_SUCCESS) {
        rc = i2c_begin_read(addr7);
    }
    if (rc == FF_SUCCESS) {
        rc = i2c_receive(r, rcap, rgot);
    }
    i2c_stop();
    return rc;
}

FF_Status_t FF_I2cMpfsSelfTest(FF_I2cMpfs_t *c)
{
    if ((c == NULL) || (c != g_i2c)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Enable the core through the HAL init path (serial-clock divider + ENS1). */
    if (c->i2c.init(FF_I2C_SELFTEST_SCL_HZ) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if ((c->bus.read8(c->bus.ctx, FF_I2C_REG_CTRL) & FF_I2C_CTRL_ENS1) == 0u) {
        return FF_ERROR;
    }
    /* No transaction has been started: the bus must report idle. */
    if (c->bus.read8(c->bus.ctx, FF_I2C_REG_STATUS) != FF_I2C_ST_IDLE) {
        return FF_ERROR;
    }
    /* Slave address 0 round trip. */
    c->bus.write8(c->bus.ctx, FF_I2C_REG_ADDR, 0xA5u);
    if (c->bus.read8(c->bus.ctx, FF_I2C_REG_ADDR) != 0xA5u) {
        return FF_ERROR;
    }
    c->bus.write8(c->bus.ctx, FF_I2C_REG_ADDR, 0u);
    /* Slave address 1 round trip. */
    c->bus.write8(c->bus.ctx, FF_I2C_REG_SLAVE1_ADDR, 0x5Au);
    if (c->bus.read8(c->bus.ctx, FF_I2C_REG_SLAVE1_ADDR) != 0x5Au) {
        return FF_ERROR;
    }
    c->bus.write8(c->bus.ctx, FF_I2C_REG_SLAVE1_ADDR, 0u);
    return FF_SUCCESS;
}

FF_Status_t FF_I2cMpfsSetup(FF_I2cMpfs_t *c, const FF_I2cBus_t *bus)
{
    if ((c == NULL) || (bus == NULL) || (bus->read8 == NULL) || (bus->write8 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    c->bus = *bus;
    c->ctrl = 0u;
    c->ready = 0u;
    c->i2c.name = "mpfs-i2c";
    c->i2c.init = i2c_init;
    c->i2c.write = i2c_write;
    c->i2c.read = i2c_read;
    c->i2c.write_read = i2c_write_read;
    g_i2c = c;
    return FF_SUCCESS;
}
