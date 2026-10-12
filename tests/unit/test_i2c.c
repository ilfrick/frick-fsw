/*
 * frick-fsw - MSS I2C driver unit tests (milestone M4)
 *
 * Exercises the CoreI2C register/transaction logic on the host with a mock
 * byte-register file that mirrors the documented MSS I2C register map and models
 * the hardware phase state machine: the controller advances one I2C phase when
 * the software clears SI (the Microchip CoreI2C ISR convention) and reports the
 * result through the Status state code, so the driver's polled sequence (START,
 * address, data / receive, repeated START, STOP) is driven end-to-end against a
 * small register-based slave model. The mock reuses the driver's register-map
 * constants: it is a convenient behavioural/phase-level fake, not an independent
 * oracle (it cannot discriminate a wrong CTRL/SI bit assignment, and it acts on
 * STA/STO before applying the SI write-gate that real CoreI2C imposes). The
 * layout is cross-checked only at the proddable register offsets by the Renode
 * `I2C.MPFS_I2C` model (the Microchip `mss_i2c.h` / `mss_i2c_regs.h` headers are
 * not revision-controlled in this repository) and the Renode functional test
 * exercises the emulated MMIO (milestone M11); see
 * docs/vv/renode-environment.md for the emulator scope limits.
 * - SRS-REQ-058.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/i2c/mpfs/ff_i2c_mpfs.h"
#include <string.h>

#define MOCK_SLAVE_REGS 8u

typedef struct
{
    uint8_t reg[8]; /* ADDR (3), SMBUS (4), FREQ (5), GLITCH (6), SLAVE1 (7) */
    uint8_t state;  /* Status state code */
    uint8_t si;     /* SI interrupt flag */
    uint8_t ens1;
    uint8_t aa;
    uint8_t ctrl_latch; /* ENS1|AA|CR0|CR1|CR2 as last written */
    uint8_t div_code;   /* decoded CR2:CR1:CR0 */
    uint8_t data_latch; /* last value written to DATA */
    uint8_t rx_byte;    /* last byte received by the master */
    uint8_t no_si;      /* fault injection: never raise SI */
    /* register-based slave model */
    uint8_t slave_addr; /* 7-bit address */
    uint8_t slave_reg[MOCK_SLAVE_REGS];
    uint8_t sreg_idx;  /* slave register pointer */
    uint8_t nack_data; /* fault injection: NACK data bytes */
    uint8_t wr[32];
    uint16_t wr_len;
    uint16_t data_phase; /* data bytes in the current write phase */
    uint32_t starts;
    uint32_t stops;
    uint32_t nacks; /* bytes the master NACKed while receiving */
} MockI2c_t;

static void mock_raise_si(MockI2c_t *m)
{
    if (m->no_si == 0u) {
        m->si = 1u;
    }
}

static void mock_reset(MockI2c_t *m)
{
    (void)memset(m, 0, sizeof(*m));
    m->state = FF_I2C_ST_IDLE;
    m->si = 1u;            /* SI is asserted until the software first clears it */
    m->slave_addr = 0x50u; /* a representative 7-bit device address */
}

static uint8_t mock_read8(void *ctx, uint32_t off)
{
    MockI2c_t *m = (MockI2c_t *)ctx;

    if ((off & 3u) != 0u) {
        return 0u;
    }
    if (off == FF_I2C_REG_CTRL) {
        return (uint8_t)(m->ctrl_latch | ((m->si != 0u) ? FF_I2C_CTRL_SI : 0u));
    }
    if (off == FF_I2C_REG_STATUS) {
        return m->state;
    }
    if (off == FF_I2C_REG_DATA) {
        return m->rx_byte;
    }
    if (off > FF_I2C_REG_SLAVE1_ADDR) {
        return 0u;
    }
    return m->reg[off / 4u];
}

/* Advance one phase (software cleared SI). */
static void mock_step(MockI2c_t *m)
{
    switch (m->state) {
    case FF_I2C_ST_START:
    case FF_I2C_ST_RESTART: {
        uint8_t addr7 = (uint8_t)(m->data_latch >> 1);
        uint8_t rw = (uint8_t)(m->data_latch & 1u);
        if (addr7 != m->slave_addr) {
            m->state = (rw == 0u) ? FF_I2C_ST_MT_SLA_NACK : FF_I2C_ST_MR_SLA_NACK;
        }
        else if (rw == 0u) {
            m->state = FF_I2C_ST_MT_SLA_ACK;
            m->data_phase = 0u;
        }
        else {
            m->state = FF_I2C_ST_MR_SLA_ACK; /* address ACKed; no byte yet */
        }
        break;
    }
    case FF_I2C_ST_MT_SLA_ACK:
    case FF_I2C_ST_MT_DATA_ACK:
        if (m->nack_data != 0u) {
            m->state = FF_I2C_ST_MT_DATA_NACK;
            break;
        }
        if (m->data_phase == 0u) {
            m->sreg_idx = m->data_latch; /* first byte = target register pointer */
        }
        else if (m->sreg_idx < MOCK_SLAVE_REGS) {
            m->slave_reg[m->sreg_idx] = m->data_latch;
            m->sreg_idx = (uint8_t)(m->sreg_idx + 1u);
        }
        else {
            /* beyond the slave register file: ignored */
        }
        if (m->wr_len < (uint16_t)sizeof(m->wr)) {
            m->wr[m->wr_len] = m->data_latch;
            m->wr_len = (uint16_t)(m->wr_len + 1u);
        }
        m->data_phase = (uint16_t)(m->data_phase + 1u);
        m->state = FF_I2C_ST_MT_DATA_ACK;
        break;
    case FF_I2C_ST_MR_SLA_ACK:
    case FF_I2C_ST_MR_DATA_ACK:
    case FF_I2C_ST_MR_DATA_NACK:
        if (m->sreg_idx < MOCK_SLAVE_REGS) {
            m->rx_byte = m->slave_reg[m->sreg_idx];
            m->sreg_idx = (uint8_t)(m->sreg_idx + 1u);
        }
        else {
            m->rx_byte = 0xFFu; /* beyond the register file */
        }
        if (m->aa == 0u) {
            m->nacks = m->nacks + 1u;
            m->state = FF_I2C_ST_MR_DATA_NACK;
        }
        else {
            m->state = FF_I2C_ST_MR_DATA_ACK;
        }
        break;
    default:
        return; /* IDLE / NACK / lost arbitration: no phase advance */
    }
    mock_raise_si(m);
}

static void mock_ctrl(MockI2c_t *m, uint8_t val)
{
    uint8_t prev_si = m->si;

    m->div_code = (uint8_t)((((val & FF_I2C_CTRL_CR0) != 0u) ? 1u : 0u) |
                            (((val & FF_I2C_CTRL_CR1) != 0u) ? 2u : 0u) |
                            (((val & FF_I2C_CTRL_CR2) != 0u) ? 4u : 0u));
    m->ens1 = ((val & FF_I2C_CTRL_ENS1) != 0u) ? 1u : 0u;
    m->aa = ((val & FF_I2C_CTRL_AA) != 0u) ? 1u : 0u;
    m->ctrl_latch = (uint8_t)(val & (FF_I2C_CTRL_ENS1 | FF_I2C_CTRL_AA | FF_I2C_CTRL_CR0 |
                                     FF_I2C_CTRL_CR1 | FF_I2C_CTRL_CR2));

    if ((val & FF_I2C_CTRL_STA) != 0u) {
        m->state = (m->state == FF_I2C_ST_IDLE) ? FF_I2C_ST_START : FF_I2C_ST_RESTART;
        m->starts = m->starts + 1u;
        mock_raise_si(m);
        return;
    }
    if ((val & FF_I2C_CTRL_STO) != 0u) {
        m->state = FF_I2C_ST_IDLE;
        m->stops = m->stops + 1u;
        mock_raise_si(m);
        return;
    }
    if (((val & FF_I2C_CTRL_SI) == 0u) && (prev_si != 0u)) {
        m->si = 0u;   /* the software cleared the interrupt flag */
        mock_step(m); /* launch the prepared phase (no-op when idle) */
    }
}

static void mock_write8(void *ctx, uint32_t off, uint8_t val)
{
    MockI2c_t *m = (MockI2c_t *)ctx;

    if ((off & 3u) != 0u) {
        return;
    }
    if (off == FF_I2C_REG_CTRL) {
        mock_ctrl(m, val);
    }
    else if (off == FF_I2C_REG_DATA) {
        m->data_latch = val;
    }
    else if (off == FF_I2C_REG_STATUS) {
        /* read-only */
    }
    else if (off <= FF_I2C_REG_SLAVE1_ADDR) {
        m->reg[off / 4u] = val;
    }
    else {
        /* out of range */
    }
}

static FF_I2cMpfs_t g_drv;
static MockI2c_t g_mock;

static void mock_bind(void)
{
    FF_I2cBus_t bus = {&g_mock, mock_read8, mock_write8};

    mock_reset(&g_mock);
    FF_TEST_ASSERT(FF_I2cMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
}

static void test_setup_and_init(void)
{
    /* static: Setup() binds the instance into the module-global g_i2c, so the
     * object must outlive the binding (avoids a dangling pointer after return). */
    static FF_I2cMpfs_t drv;
    FF_I2cBus_t bus = {&g_mock, mock_read8, mock_write8};

    FF_I2cBus_t bad = {&g_mock, NULL, mock_write8};

    mock_reset(&g_mock);
    FF_TEST_ASSERT(FF_I2cMpfsSetup(NULL, &bus) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_I2cMpfsSetup(&drv, NULL) == FF_INVALID_ARGUMENT);
    /* A partially populated bus (missing hooks) is rejected. */
    FF_TEST_ASSERT(FF_I2cMpfsSetup(&drv, &bad) == FF_INVALID_ARGUMENT);
    bad.read8 = mock_read8;
    bad.write8 = NULL;
    FF_TEST_ASSERT(FF_I2cMpfsSetup(&drv, &bad) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_I2cMpfsSetup(&drv, &bus) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.i2c.name != NULL);
    FF_TEST_ASSERT(strcmp(drv.i2c.name, "mpfs-i2c") == 0);
    FF_TEST_ASSERT(drv.i2c.init != NULL);
    FF_TEST_ASSERT(drv.i2c.write != NULL);
    FF_TEST_ASSERT(drv.i2c.read != NULL);
    FF_TEST_ASSERT(drv.i2c.write_read != NULL);

    /* Not ready before init. */
    FF_TEST_ASSERT(drv.i2c.write(0x50u, NULL, 0u) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.i2c.read(0x50u, (uint8_t *)&bus, 1u, NULL) == FF_NOT_READY);

    /* init argument validation. */
    FF_TEST_ASSERT(drv.i2c.init(0u) == FF_INVALID_ARGUMENT);

    FF_TEST_ASSERT(drv.i2c.init(100000u) == FF_SUCCESS);
    /* Core enabled (ENS1) and the APB-clock divider programmed in CTRL. */
    FF_TEST_ASSERT((mock_read8(&g_mock, FF_I2C_REG_CTRL) & FF_I2C_CTRL_ENS1) != 0u);
    FF_TEST_ASSERT(g_mock.ens1 == 1u);
    /* 100 kHz target at the 100 MHz APB reference: the slowest divider (960). */
    FF_TEST_ASSERT(g_mock.div_code == 4u);
    FF_TEST_ASSERT(drv.ready == 1u);
}

static void test_divider_selection(void)
{
    mock_bind();
    /* 100 MHz APB: 60 -> 1666666, 120 -> 833333, 160 -> 625000, 192 -> 520833,
     * 224 -> 446428, 256 -> 390625, 960 -> 104166. The fastest rate not above
     * the request is selected; below every rate the slowest divider is used. */
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 4u); /* PCLK/960 */
    FF_TEST_ASSERT(g_drv.i2c.init(104167u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 4u); /* PCLK/960 (104166 <= 104167) */
    /* Boundary of the minimum achievable SCL (104166 = PCLK/960 at 100 MHz). */
    FF_TEST_ASSERT(g_drv.i2c.init(104166u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 4u); /* PCLK/960 (104166 <= 104166) */
    FF_TEST_ASSERT(g_drv.i2c.init(625000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 3u); /* PCLK/160 (625000 <= 625000) */
    /* 1 MHz class: PCLK/120 = 833333 <= 1000000. */
    FF_TEST_ASSERT(g_drv.i2c.init(1000000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 5u); /* PCLK/120 */
    FF_TEST_ASSERT(g_drv.i2c.init(400000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 0u); /* PCLK/256 (390625 <= 400000) */
    FF_TEST_ASSERT(g_drv.i2c.init(500000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 1u); /* PCLK/224 (446428 <= 500000) */
    FF_TEST_ASSERT(g_drv.i2c.init(2000000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 6u); /* PCLK/60 (1666666 <= 2000000) */
    /* Very low target: no divider achieves it, so the slowest is used. */
    FF_TEST_ASSERT(g_drv.i2c.init(10000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.div_code == 4u); /* PCLK/960 */
}

static void test_divider_table(void)
{
    uint8_t i;

    FF_TEST_ASSERT(FF_I2C_DIV_COUNT == 7u);
    FF_TEST_ASSERT(FF_I2C_MIN_SCL_HZ == 104166u); /* PCLK/960 at the reference */
    for (i = 1u; i < FF_I2C_DIV_COUNT; i++) {
        /* ascending by divisor */
        FF_TEST_ASSERT(ff_i2c_div_table[i].div > ff_i2c_div_table[i - 1u].div);
        FF_TEST_ASSERT(ff_i2c_div_table[i].code != ff_i2c_div_table[i - 1u].code);
        FF_TEST_ASSERT(ff_i2c_div_table[i].code <= 6u);
        FF_TEST_ASSERT(ff_i2c_div_table[i].div > 0u);
    }
}

static void test_write_transaction(void)
{
    const uint8_t data[3] = {0x01u, 0x02u, 0x03u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(400000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.i2c.write(0x50u, data, 3u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.starts == 1u);
    FF_TEST_ASSERT(g_mock.stops == 1u);
    FF_TEST_ASSERT(g_mock.wr_len == 3u);
    FF_TEST_ASSERT(g_mock.wr[0] == 0x01u);
    FF_TEST_ASSERT(g_mock.wr[1] == 0x02u);
    FF_TEST_ASSERT(g_mock.wr[2] == 0x03u);
    /* First data byte is the slave register pointer (0x01), then two writes. */
    FF_TEST_ASSERT(g_mock.slave_reg[1] == 0x02u);
    FF_TEST_ASSERT(g_mock.slave_reg[2] == 0x03u);
    FF_TEST_ASSERT(g_mock.state == FF_I2C_ST_IDLE);

    /* Address-only transaction (len == 0, NULL buffer) is accepted. */
    FF_TEST_ASSERT(g_drv.i2c.write(0x50u, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.wr_len == 3u); /* no data bytes added */
    FF_TEST_ASSERT(g_mock.starts == 2u);
    FF_TEST_ASSERT(g_mock.stops == 2u);

    /* Extended address range is rejected. */
    FF_TEST_ASSERT(g_drv.i2c.write(0x80u, data, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.i2c.write(0x50u, NULL, 1u) == FF_INVALID_ARGUMENT);
}

static void test_write_address_nack(void)
{
    const uint8_t data[1] = {0xAAu};

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    g_mock.slave_addr = 0x51u; /* no device at 0x50 */
    FF_TEST_ASSERT(g_drv.i2c.write(0x50u, data, 1u) == FF_ERROR);
    FF_TEST_ASSERT(g_mock.stops == 1u); /* the bus is still released */
    FF_TEST_ASSERT(g_mock.wr_len == 0u);
}

static void test_write_data_nack(void)
{
    const uint8_t data[2] = {0x10u, 0x20u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    g_mock.nack_data = 1u;
    FF_TEST_ASSERT(g_drv.i2c.write(0x50u, data, 2u) == FF_ERROR);
    FF_TEST_ASSERT(g_mock.stops == 1u);
}

static void test_read_transaction(void)
{
    uint8_t buf[4];
    uint16_t got = 0u;

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    g_mock.slave_reg[0] = 0xA1u;
    g_mock.slave_reg[1] = 0xA2u;
    g_mock.slave_reg[2] = 0xA3u;
    g_mock.slave_reg[3] = 0xA4u;
    g_mock.sreg_idx = 0u;

    FF_TEST_ASSERT(g_drv.i2c.read(0x50u, buf, 4u, &got) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 4u);
    FF_TEST_ASSERT(buf[0] == 0xA1u);
    FF_TEST_ASSERT(buf[1] == 0xA2u);
    FF_TEST_ASSERT(buf[2] == 0xA3u);
    FF_TEST_ASSERT(buf[3] == 0xA4u);
    FF_TEST_ASSERT(g_mock.starts == 1u);
    FF_TEST_ASSERT(g_mock.stops == 1u);
    /* Exactly one NACK: the last received byte. */
    FF_TEST_ASSERT(g_mock.nacks == 1u);
}

static void test_read_single_byte(void)
{
    uint8_t buf[1];
    uint16_t got = 0u;

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    g_mock.slave_reg[0] = 0x5Au;
    g_mock.sreg_idx = 0u;

    FF_TEST_ASSERT(g_drv.i2c.read(0x50u, buf, 1u, &got) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 1u);
    FF_TEST_ASSERT(buf[0] == 0x5Au);
    FF_TEST_ASSERT(g_mock.nacks == 1u); /* the only byte is NACKed */
}

static void test_read_nack(void)
{
    uint8_t buf[2];
    uint16_t got = 7u;

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    g_mock.slave_addr = 0x51u; /* no device at 0x50 */
    FF_TEST_ASSERT(g_drv.i2c.read(0x50u, buf, 2u, &got) == FF_ERROR);
    FF_TEST_ASSERT(got == 0u);
    FF_TEST_ASSERT(g_mock.stops == 1u);

    /* Argument validation. */
    FF_TEST_ASSERT(g_drv.i2c.read(0x50u, NULL, 2u, &got) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.i2c.read(0x50u, buf, 0u, &got) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.i2c.read(0x80u, buf, 2u, &got) == FF_INVALID_ARGUMENT);
}

static void test_write_read_transaction(void)
{
    uint8_t r[2];
    uint16_t rgot = 0u;
    const uint8_t w[1] = {0x02u}; /* register pointer */

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(400000u) == FF_SUCCESS);
    g_mock.slave_reg[2] = 0x33u;
    g_mock.slave_reg[3] = 0x44u;

    FF_TEST_ASSERT(g_drv.i2c.write_read(0x50u, w, 1u, r, 2u, &rgot) == FF_SUCCESS);
    FF_TEST_ASSERT(rgot == 2u);
    FF_TEST_ASSERT(r[0] == 0x33u);
    FF_TEST_ASSERT(r[1] == 0x44u);
    FF_TEST_ASSERT(g_mock.starts == 2u); /* START then repeated START */
    FF_TEST_ASSERT(g_mock.stops == 1u);
    FF_TEST_ASSERT(g_mock.nacks == 1u);

    /* wlen == 0 behaves like a plain read from the current pointer. */
    rgot = 0u;
    FF_TEST_ASSERT(g_drv.i2c.write_read(0x50u, NULL, 0u, r, 2u, &rgot) == FF_SUCCESS);
    FF_TEST_ASSERT(rgot == 2u);

    /* Argument validation. */
    FF_TEST_ASSERT(g_drv.i2c.write_read(0x50u, w, 1u, NULL, 2u, &rgot) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.i2c.write_read(0x50u, w, 1u, r, 0u, &rgot) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.i2c.write_read(0x50u, NULL, 1u, r, 2u, &rgot) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.i2c.write_read(0x80u, w, 1u, r, 2u, &rgot) == FF_INVALID_ARGUMENT);
}

static void test_si_timeout(void)
{
    const uint8_t data[1] = {0x01u};
    uint16_t got = 0u;
    uint8_t buf[1];

    mock_bind();
    FF_TEST_ASSERT(g_drv.i2c.init(100000u) == FF_SUCCESS);
    g_mock.no_si = 1u; /* the controller never signals a completed phase */
    FF_TEST_ASSERT(g_drv.i2c.write(0x50u, data, 1u) == FF_TIMEOUT);
    FF_TEST_ASSERT(g_drv.i2c.read(0x50u, buf, 1u, &got) == FF_TIMEOUT);
    FF_TEST_ASSERT(g_drv.i2c.write_read(0x50u, data, 1u, buf, 1u, &got) == FF_TIMEOUT);
}

static void test_self_test(void)
{
    /* static: Setup() binds the instance into the module-global g_i2c, so the
     * object must outlive the binding (avoids a dangling pointer after return). */
    static FF_I2cMpfs_t drv;
    FF_I2cBus_t bus = {&g_mock, mock_read8, mock_write8};

    mock_reset(&g_mock);
    FF_TEST_ASSERT(FF_I2cMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_I2cMpfsSetup(&drv, &bus) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_I2cMpfsSelfTest(&drv) == FF_SUCCESS);
    /* The slave-address registers are restored to zero. */
    FF_TEST_ASSERT(mock_read8(&g_mock, FF_I2C_REG_ADDR) == 0u);
    FF_TEST_ASSERT(mock_read8(&g_mock, FF_I2C_REG_SLAVE1_ADDR) == 0u);
    FF_TEST_ASSERT((mock_read8(&g_mock, FF_I2C_REG_CTRL) & FF_I2C_CTRL_ENS1) != 0u);
    FF_TEST_ASSERT(mock_read8(&g_mock, FF_I2C_REG_STATUS) == FF_I2C_ST_IDLE);

    /* A self-test on an unbound instance is rejected. */
    {
        FF_I2cMpfs_t other;
        (void)memset(&other, 0, sizeof(other));
        FF_TEST_ASSERT(FF_I2cMpfsSelfTest(&other) == FF_INVALID_ARGUMENT);
    }
}

FF_TEST_MAIN(test_setup_and_init, test_divider_selection, test_divider_table,
             test_write_transaction, test_write_address_nack, test_write_data_nack,
             test_read_transaction, test_read_single_byte, test_read_nack,
             test_write_read_transaction, test_si_timeout, test_self_test)
