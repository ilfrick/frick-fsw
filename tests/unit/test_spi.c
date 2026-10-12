/*
 * frick-fsw - MSS SPI driver unit tests (milestone M4)
 *
 * Exercises the CoreSPI register/transfer logic on the host with a mock
 * double-word register file that mirrors the documented MSS SPI register map and
 * models the TX/RX path: a write to TXDATA enqueues the transmitted byte, sets
 * RXDATARCVD and (loopback) makes that byte available in RXDATA; a read of
 * RXDATA returns the queued byte and clears RXDATARCVD, so the driver's polled
 * sequence (write TXDATA, poll STATUS for RXDATARCVD, read RXDATA) is driven
 * end-to-end. The mock reuses the driver's register-map constants: it is a
 * convenient behavioural/phase-level fake, not an independent oracle (it cannot
 * discriminate a wrong CONTROL/STATUS bit assignment, and it models a perfect
 * loopback rather than a real SPI slave). The layout is cross-checked only at
 * the proddable register offsets by the Renode `SPI.MPFS_SPI` model (the
 * Microchip `mss_spi.h` / `mss_spi_regs.h` headers are not revision-controlled
 * in this repository) and the Renode functional test exercises the emulated
 * MMIO (milestone M11); see docs/vv/renode-environment.md for the emulator scope
 * limits.
 * - SRS-REQ-069 (SPI backend; SRS-REQ-031 contract).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/spi/mpfs/ff_spi_mpfs.h"
#include <string.h>

typedef struct
{
    uint32_t control;
    uint32_t framesize;
    uint32_t clkrate;
    uint32_t ssel;
    uint32_t tx_latch;     /* last value written to TXDATA */
    uint32_t tx_count;     /* frames transmitted */
    uint32_t rx_valid;     /* RXDATARCVD */
    uint32_t rx_fifo;      /* queued RX byte */
    uint32_t rx_count;     /* frames read from RXDATA */
    uint32_t no_rx;        /* fault injection: TXDATA never makes a frame available */
    uint32_t no_rx_status; /* fault injection: a frame is latched but STATUS hides it */
    uint32_t loopback;     /* echo the transmitted byte back (default on) */
    uint32_t ssel_writes;
    uint8_t tx[64];
    uint32_t tx_len;
} MockSpi_t;

static MockSpi_t g_mock;
static FF_SpiMpfs_t g_drv;
/* static storage: FF_SpiMpfs_t keeps a pointer to the bus descriptor */
static FF_SpiBus_t g_bus;

static void mock_reset(MockSpi_t *m)
{
    (void)memset(m, 0, sizeof(*m));
    m->control = 0x80000102u; /* CoreSPI CONTROL reset value */
    m->framesize = 4u;        /* CoreSPI FRAMESIZE reset value */
    m->loopback = 1u;
}

static uint32_t mock_status(const MockSpi_t *m)
{
    uint32_t st = FF_SPI_ST_TXFIFOEMPTY;

    if (m->tx_count != 0u) {
        st |= FF_SPI_ST_TXDATASENT;
    }
    if ((m->rx_valid != 0u) && (m->no_rx_status == 0u)) {
        st |= FF_SPI_ST_RXDATARCVD;
    }
    else {
        st |= FF_SPI_ST_RXFIFOEMPTY;
    }
    if (m->ssel != 0u) {
        st |= FF_SPI_ST_SSEL;
    }
    return st;
}

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    MockSpi_t *m = (MockSpi_t *)ctx;
    uint32_t v;

    if ((off & 3u) != 0u) {
        return 0u;
    }
    switch (off) {
    case FF_SPI_REG_CONTROL:
        v = m->control;
        break;
    case FF_SPI_REG_FRAMESIZE:
        v = m->framesize;
        break;
    case FF_SPI_REG_STATUS:
        v = mock_status(m);
        break;
    case FF_SPI_REG_RXDATA:
        v = m->rx_fifo & 0xFFu;
        m->rx_valid = 0u; /* reading RXDATA pops the frame */
        m->rx_count = m->rx_count + 1u;
        break;
    case FF_SPI_REG_CLKRATE:
        v = m->clkrate;
        break;
    case FF_SPI_REG_SSEL:
        v = m->ssel;
        break;
    default:
        v = 0u;
        break;
    }
    return v;
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    MockSpi_t *m = (MockSpi_t *)ctx;

    if ((off & 3u) != 0u) {
        return;
    }
    switch (off) {
    case FF_SPI_REG_CONTROL:
        m->control = val;
        break;
    case FF_SPI_REG_FRAMESIZE:
        m->framesize = val;
        break;
    case FF_SPI_REG_CLKRATE:
        m->clkrate = val;
        break;
    case FF_SPI_REG_SSEL:
        m->ssel = val;
        m->ssel_writes = m->ssel_writes + 1u;
        break;
    case FF_SPI_REG_TXDATA:
        m->tx_latch = val & 0xFFu;
        m->tx_count = m->tx_count + 1u;
        if (m->tx_len < (uint32_t)sizeof(m->tx)) {
            m->tx[m->tx_len] = (uint8_t)(val & 0xFFu);
            m->tx_len = m->tx_len + 1u;
        }
        /* Model a one-deep RX FIFO that is NOT overwritten while a previous
         * frame is still unread: this is what lets the test discriminate a
         * driver that fails to drain RXDATA (a stale byte would then be
         * returned by the next transfer). */
        if ((m->no_rx == 0u) && (m->rx_valid == 0u)) {
            m->rx_fifo = (m->loopback != 0u) ? (val & 0xFFu) : 0u;
            m->rx_valid = 1u;
        }
        break;
    case FF_SPI_REG_CMD:
        if ((val & FF_SPI_CMD_RXFIFORST) != 0u) {
            m->rx_valid = 0u;
            m->rx_fifo = 0u;
        }
        break;
    case FF_SPI_REG_RXDATA:
    case FF_SPI_REG_STATUS:
        /* read-only */
        break;
    default:
        /* out of range */
        break;
    }
}

static void mock_bus_init(void)
{
    g_bus.ctx = &g_mock;
    g_bus.read32 = mock_read32;
    g_bus.write32 = mock_write32;
}

static void mock_bind(void)
{
    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&g_drv, &g_bus) == FF_SUCCESS);
}

static void test_setup_and_init(void)
{
    /* static: Setup() binds the instance into the module-global g_spi, so the
     * object must outlive the binding (avoids a dangling pointer after return). */
    static FF_SpiMpfs_t drv;
    FF_SpiBus_t bad;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpiMpfsSetup(NULL, &g_bus) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&drv, NULL) == FF_INVALID_ARGUMENT);
    /* A partially populated bus (missing hooks) is rejected. */
    bad.ctx = &g_mock;
    bad.read32 = NULL;
    bad.write32 = mock_write32;
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&drv, &bad) == FF_INVALID_ARGUMENT);
    bad.read32 = mock_read32;
    bad.write32 = NULL;
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&drv, &bad) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&drv, &g_bus) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.spi.name != NULL);
    FF_TEST_ASSERT(strcmp(drv.spi.name, "mpfs-spi") == 0);
    FF_TEST_ASSERT(drv.spi.init != NULL);
    FF_TEST_ASSERT(drv.spi.transfer != NULL);
    FF_TEST_ASSERT(drv.spi.cs_assert != NULL);
    FF_TEST_ASSERT(drv.spi.cs_release != NULL);

    /* Not ready before init. */
    FF_TEST_ASSERT(drv.spi.transfer(NULL, NULL, 1u) == FF_NOT_READY);

    /* init argument validation. */
    FF_TEST_ASSERT(drv.spi.init(0u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(drv.spi.init(1000000u, 4u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(drv.spi.init(1000000u, 0u, 8u) == FF_INVALID_ARGUMENT);

    FF_TEST_ASSERT(drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    /* Core enabled (ENABLE|MASTER), mode 0, FRAMESIZE 8, SSEL released. */
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_SPI_REG_CONTROL) &
                    (FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER)) ==
                   (FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER));
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_SPI_REG_CONTROL) & FF_SPI_CONTROL_MODE_MASK) == 0u);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_FRAMESIZE) == 8u);
    /* 1 MHz target at the 100 MHz APB reference: PCLK / (2 * 50). */
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 49u);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_SSEL) == 0u);
    FF_TEST_ASSERT(drv.ready == 1u);
    FF_TEST_ASSERT(drv.clock_hz == 1000000u);
    FF_TEST_ASSERT(drv.mode == 0u);
    FF_TEST_ASSERT(drv.cs == 0u);

    /* Mode is programmed into CONTROL bits 2-3. */
    FF_TEST_ASSERT(drv.spi.init(1000000u, 3u, 7u) == FF_SUCCESS);
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_SPI_REG_CONTROL) & FF_SPI_CONTROL_MODE_MASK) ==
                   (3u << FF_SPI_CONTROL_MODE_SHIFT));
    FF_TEST_ASSERT(drv.mode == 3u);
    FF_TEST_ASSERT(drv.cs == 7u);
}

static void test_clkrate_selection(void)
{
    mock_bind();
    /* PCLK = 100 MHz, SCL = PCLK / (2 * (CLKRATE + 1)): the smallest CLKRATE
     * whose SCL does not exceed the request is selected, clamped to [0, 255]. */
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 49u); /* PCLK/(2*50) */
    FF_TEST_ASSERT(g_drv.spi.init(5000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 9u); /* PCLK/(2*10) */
    FF_TEST_ASSERT(g_drv.spi.init(25000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 1u); /* PCLK/(2*2) */
    /* Fastest achievable SCL is PCLK/2 (CLKRATE 0). */
    FF_TEST_ASSERT(g_drv.spi.init(50000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 0u);
    FF_TEST_ASSERT(g_drv.spi.init(100000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 0u);
    /* 40 MHz: CLKRATE 0 gives 50 MHz (too fast), so CLKRATE 1 -> 25 MHz. */
    FF_TEST_ASSERT(g_drv.spi.init(40000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 1u);
    /* Boundary just below PCLK/2: CLKRATE 0 is too fast, CLKRATE 1 is selected. */
    FF_TEST_ASSERT(g_drv.spi.init(49999999u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 1u);
    /* 20 MHz: CLKRATE 2 -> 16.66 MHz (CLKRATE 1 -> 25 MHz is too fast). */
    FF_TEST_ASSERT(g_drv.spi.init(20000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 2u);
    /* Very low target: no divider achieves it, so the slowest is used. */
    FF_TEST_ASSERT(g_drv.spi.init(100000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 255u);
    FF_TEST_ASSERT(g_drv.spi.init(1u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 255u);
    /* Minimum achievable SCL at the 100 MHz reference. */
    FF_TEST_ASSERT(FF_SPI_MIN_SCL_HZ == 195312u);
}

static void test_transfer_tx_rx(void)
{
    const uint8_t tx[3] = {0x11u, 0x22u, 0x33u};
    uint8_t rx[3] = {0u, 0u, 0u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.spi.cs_assert(0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_SSEL) == 1u);
    FF_TEST_ASSERT(g_drv.spi.transfer(tx, rx, 3u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.tx_len == 3u);
    FF_TEST_ASSERT(g_mock.tx[0] == 0x11u);
    FF_TEST_ASSERT(g_mock.tx[1] == 0x22u);
    FF_TEST_ASSERT(g_mock.tx[2] == 0x33u);
    FF_TEST_ASSERT(g_mock.tx_count == 3u);
    FF_TEST_ASSERT(g_mock.rx_count == 3u);
    FF_TEST_ASSERT(rx[0] == 0x11u);
    FF_TEST_ASSERT(rx[1] == 0x22u);
    FF_TEST_ASSERT(rx[2] == 0x33u);
    FF_TEST_ASSERT(g_drv.spi.cs_release(0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_SSEL) == 0u);
}

static void test_transfer_rx_only(void)
{
    uint8_t rx[2] = {0xFFu, 0xFFu};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    /* tx == NULL: frames of 0x00 are transmitted. */
    FF_TEST_ASSERT(g_drv.spi.transfer(NULL, rx, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.tx_len == 2u);
    FF_TEST_ASSERT(g_mock.tx[0] == 0x00u);
    FF_TEST_ASSERT(g_mock.tx[1] == 0x00u);
    FF_TEST_ASSERT(rx[0] == 0x00u); /* loopback of the transmitted frame */
    FF_TEST_ASSERT(rx[1] == 0x00u);
}

static void test_transfer_tx_only(void)
{
    const uint8_t tx[2] = {0xA1u, 0xB2u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    /* rx == NULL: received frames are drained (read and discarded) so that no
     * stale frame is left for the next transfer. */
    FF_TEST_ASSERT(g_drv.spi.transfer(tx, NULL, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.tx_len == 2u);
    FF_TEST_ASSERT(g_mock.tx[0] == 0xA1u);
    FF_TEST_ASSERT(g_mock.tx[1] == 0xB2u);
    FF_TEST_ASSERT(g_mock.rx_count == 2u); /* both frames were read from RXDATA */
    FF_TEST_ASSERT(g_mock.rx_valid == 0u); /* no stale frame remains */
}

static void test_transfer_zero_len(void)
{
    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.spi.transfer(NULL, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.tx_count == 0u);
}

static void test_transfer_timeout(void)
{
    const uint8_t tx[1] = {0x01u};
    uint8_t rx[1] = {0u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    g_mock.no_rx = 1u; /* the controller never signals a received frame */
    FF_TEST_ASSERT(g_drv.spi.transfer(tx, rx, 1u) == FF_TIMEOUT);
    FF_TEST_ASSERT(g_mock.tx_count == 1u); /* the frame was still transmitted */
}

static void test_cs_control(void)
{
    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.spi.cs_assert(0u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_SSEL) == 1u);
    FF_TEST_ASSERT(g_drv.spi.cs_assert(3u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_SSEL) == 8u);
    FF_TEST_ASSERT(g_drv.spi.cs_release(3u) == FF_SUCCESS);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_SSEL) == 0u);
    /* Chip select out of range is rejected. */
    FF_TEST_ASSERT(g_drv.spi.cs_assert(8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.spi.cs_release(8u) == FF_INVALID_ARGUMENT);
}

static void test_transfer_drains_rx(void)
{
    static const uint8_t tx1[1] = {0x11u};
    static const uint8_t tx2[1] = {0x22u};
    uint8_t rx[1];
    uint32_t rx_before;

    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);

    /* A tx-only transfer must drain the received frame (read RXDATA)... */
    rx_before = g_mock.rx_count;
    FF_TEST_ASSERT(g_drv.spi.transfer(tx1, NULL, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.rx_count == (rx_before + 1u));

    /* ...so the next full-duplex transfer returns the NEW frame, not a stale
     * one (regression for the R1-01 one-frame RX shift). */
    rx[0] = 0u;
    FF_TEST_ASSERT(g_drv.spi.transfer(tx2, rx, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(rx[0] == 0x22u);
}

static void test_transfer_timeout_drains(void)
{
    static const uint8_t tx[1] = {0x33u};
    static const uint8_t tx2[1] = {0x44u};
    uint8_t rx[1];

    mock_bind();
    FF_TEST_ASSERT(g_drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    /* Force a timeout: a frame is latched but STATUS hides RXDATARCVD. */
    g_mock.no_rx_status = 1u;
    FF_TEST_ASSERT(g_drv.spi.transfer(tx, NULL, 1u) == FF_TIMEOUT);
    /* The timeout must have reset the RX FIFO (FF_SPI_CMD_RXFIFORST)... */
    FF_TEST_ASSERT(g_mock.rx_valid == 0u);
    /* ...so a following normal transfer returns its own frame, not the stale
     * latched one (regression for the D-01 error-path shift). */
    g_mock.no_rx_status = 0u;
    rx[0] = 0u;
    FF_TEST_ASSERT(g_drv.spi.transfer(tx2, rx, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(rx[0] == 0x44u);
}

static void test_cs_preinit(void)
{
    /* static: Setup() binds the instance into the module-global g_spi. */
    static FF_SpiMpfs_t drv;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&drv, &g_bus) == FF_SUCCESS);
    /* Bound but not init'd: chip-select control reports FF_NOT_READY. */
    FF_TEST_ASSERT(drv.spi.cs_assert(0u) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.spi.cs_release(0u) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.spi.init(1000000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.spi.cs_assert(0u) == FF_SUCCESS);
}

static void test_self_test(void)
{
    /* static: Setup() binds the instance into the module-global g_spi, so the
     * object must outlive the binding (avoids a dangling pointer after return). */
    static FF_SpiMpfs_t drv;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpiMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SpiMpfsSetup(&drv, &g_bus) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SpiMpfsSelfTest(&drv) == FF_SUCCESS);
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_SPI_REG_CONTROL) &
                    (FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER)) ==
                   (FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER));
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_CLKRATE) == 49u);
    FF_TEST_ASSERT(mock_read32(&g_mock, FF_SPI_REG_FRAMESIZE) == 8u);
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_SPI_REG_STATUS) & FF_SPI_ST_RXFIFOEMPTY) != 0u);

    /* A self-test on an unbound instance is rejected. */
    {
        FF_SpiMpfs_t other;
        (void)memset(&other, 0, sizeof(other));
        FF_TEST_ASSERT(FF_SpiMpfsSelfTest(&other) == FF_INVALID_ARGUMENT);
    }
}

FF_TEST_MAIN(test_setup_and_init, test_clkrate_selection, test_transfer_tx_rx,
             test_transfer_rx_only, test_transfer_tx_only, test_transfer_zero_len,
             test_transfer_timeout, test_transfer_drains_rx, test_transfer_timeout_drains,
             test_cs_control, test_cs_preinit, test_self_test)
