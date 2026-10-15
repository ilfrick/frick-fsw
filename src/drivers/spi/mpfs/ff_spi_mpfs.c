/*
 * frick-fsw - MSS SPI driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Polled SPI master over the Microchip PolarFire SoC MSS SPI (CoreSPI), register
 * logic on an injected double-word MMIO bus. Static, no allocation. Single
 * instance (the HAL SPI contract carries no context).
 *
 * Transfer shape: for each frame the driver writes TXDATA and polls the Status
 * register for RXDATARCVD, then reads RXDATA. Chip select is driven separately
 * by cs_assert()/cs_release(). See the header for the register map, the
 * serial-clock derivation and the known limitations.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/spi/mpfs/ff_spi_mpfs.h"

FF_STATIC_ASSERT(FF_SPI_MPFS_CLK_HZ > 0u, spi_clk_nonzero);

static FF_SpiMpfs_t *g_spi; /* single bound instance */

static uint32_t spi_rd(uint32_t off)
{
    return g_spi->bus->read32(g_spi->bus->ctx, off);
}

static void spi_wr(uint32_t off, uint32_t val)
{
    g_spi->bus->write32(g_spi->bus->ctx, off, val);
}

/* Reference clock divider: the smallest 8-bit CLKRATE for which the resulting
 * serial clock PCLK / (2 * (CLKRATE + 1)) does not exceed the requested rate,
 * clamped to [0, 255]. This is a reference derivation from the APB reference
 * clock (FF_SPI_MPFS_CLK_HZ); per-rate accuracy and the on-target SCL are
 * confirmed on target (GAP-A5). */
static uint8_t spi_clkrate(uint32_t clock_hz)
{
    uint32_t n;

    /* Defensive: a zero target is rejected by init(); never divide by zero. */
    if (clock_hz == 0u) {
        return 255u;
    }
    /* Fastest achievable SCL is PCLK / 2 (CLKRATE = 0). */
    if (clock_hz >= (FF_SPI_MPFS_CLK_HZ / 2u)) {
        return 0u;
    }
    /* clock_hz < PCLK / 2 here, so 2 * clock_hz < PCLK and neither the divisor
     * nor the ceiling numerator can overflow. */
    n = (FF_SPI_MPFS_CLK_HZ + (2u * clock_hz) - 1u) / (2u * clock_hz); /* ceil(PCLK/d) */
    if (n == 0u) {
        n = 1u; /* defensive: unreachable, PCLK >= 2 * clock_hz here */
    }
    n = n - 1u; /* CLKRATE */
    if (n > 255u) {
        n = 255u; /* clamp to the slowest divider */
    }
    return (uint8_t)n;
}

static FF_Status_t spi_check_ready(void)
{
    if (g_spi == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_spi->ready == 0u) {
        return FF_NOT_READY;
    }
    return FF_SUCCESS;
}

/* Poll until the controller signals a received frame (RXDATARCVD set). */
static FF_Status_t spi_wait_rx(void)
{
    uint32_t i;

    for (i = 0u; i < FF_SPI_POLL_MAX; i++) {
        if ((spi_rd(FF_SPI_REG_STATUS) & FF_SPI_ST_RXDATARCVD) != 0u) {
            return FF_SUCCESS;
        }
    }
    return FF_TIMEOUT;
}

/* clock_hz is the target serial clock (SCL) frequency; the clock divider is
 * derived from the APB reference clock (FF_SPI_MPFS_CLK_HZ). */
static FF_Status_t spi_init(uint32_t clock_hz, uint8_t mode, uint8_t cs)
{
    if ((g_spi == NULL) || (clock_hz == 0u) || (mode > FF_SPI_MODE_MAX) || (cs > FF_SPI_CS_MAX)) {
        return FF_INVALID_ARGUMENT;
    }
    spi_wr(FF_SPI_REG_CONTROL, FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER |
                                   (((uint32_t)mode) << FF_SPI_CONTROL_MODE_SHIFT));
    spi_wr(FF_SPI_REG_FRAMESIZE, FF_SPI_FRAMESIZE_BITS);
    spi_wr(FF_SPI_REG_CLKRATE, (uint32_t)spi_clkrate(clock_hz));
    spi_wr(FF_SPI_REG_SSEL, 0u);
    g_spi->clock_hz = clock_hz;
    g_spi->mode = mode;
    g_spi->cs = cs;
    g_spi->ready = 1u;
    return FF_SUCCESS;
}

/* Full-duplex transfer of len 8-bit frames. tx may be NULL (frames of 0x00 are
 * sent) and/or rx may be NULL (received frames are discarded). Chip select is
 * driven separately by cs_assert()/cs_release(). */
static FF_Status_t spi_transfer(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    uint16_t i;
    uint8_t rx_byte;
    FF_Status_t rc;

    rc = spi_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    for (i = 0u; i < len; i++) {
        spi_wr(FF_SPI_REG_TXDATA, (tx != NULL) ? (uint32_t)tx[i] : 0u);
        rc = spi_wait_rx();
        if (rc != FF_SUCCESS) {
            /* Reset the receive FIFO so a timed-out frame cannot leave a stale
             * byte that would shift the next transfer (closes the R1-01 failure
             * mode on the error path). RXFIFORST is a write-1 self-clearing
             * command bit; the emulated model implements it and the host test
             * `test_transfer_timeout_drains` discriminates it. */
            spi_wr(FF_SPI_REG_CMD, FF_SPI_CMD_RXFIFORST);
            return rc;
        }
        /* Always drain RXDATA to clear the level-triggered RXDATARCVD flag: a
         * tx-only (rx == NULL) or timed-out transfer must not leave a stale
         * received byte that would shift the next full-duplex transfer by one
         * frame. */
        rx_byte = (uint8_t)(spi_rd(FF_SPI_REG_RXDATA) & 0xFFu);
        if (rx != NULL) {
            rx[i] = rx_byte;
        }
    }
    return FF_SUCCESS;
}

static FF_Status_t spi_cs_assert(uint8_t cs)
{
    FF_Status_t rc;

    if (g_spi == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (cs > FF_SPI_CS_MAX) {
        return FF_INVALID_ARGUMENT;
    }
    rc = spi_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    spi_wr(FF_SPI_REG_SSEL, 1u << cs);
    return FF_SUCCESS;
}

static FF_Status_t spi_cs_release(uint8_t cs)
{
    FF_Status_t rc;

    if ((g_spi == NULL) || (cs > FF_SPI_CS_MAX)) {
        return FF_INVALID_ARGUMENT;
    }
    rc = spi_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    spi_wr(FF_SPI_REG_SSEL, 0u);
    return FF_SUCCESS;
}

FF_Status_t FF_SpiMpfsSelfTest(FF_SpiMpfs_t *s)
{
    uint32_t clkrate;

    if ((s == NULL) || (s != g_spi)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Enable the core through the HAL init path (CONTROL/FRAMESIZE/CLKRATE). */
    if (s->spi.init(FF_SPI_SELFTEST_SCL_HZ, 0u, 0u) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if ((s->bus->read32(s->bus->ctx, FF_SPI_REG_CONTROL) &
         (FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER)) !=
        (FF_SPI_CONTROL_ENABLE | FF_SPI_CONTROL_MASTER)) {
        return FF_ERROR;
    }
    clkrate = (uint32_t)spi_clkrate(FF_SPI_SELFTEST_SCL_HZ);
    if ((s->bus->read32(s->bus->ctx, FF_SPI_REG_CLKRATE) & FF_SPI_CLKRATE_MASK) != clkrate) {
        return FF_ERROR;
    }
    if ((s->bus->read32(s->bus->ctx, FF_SPI_REG_FRAMESIZE) & FF_SPI_FRAMESIZE_MASK) !=
        FF_SPI_FRAMESIZE_BITS) {
        return FF_ERROR;
    }
    /* No frame has been transferred: the RX FIFO must be empty. */
    if ((s->bus->read32(s->bus->ctx, FF_SPI_REG_STATUS) & FF_SPI_ST_RXFIFOEMPTY) == 0u) {
        return FF_ERROR;
    }
    /* Restore the configuration (a second init; no bus transaction is run). */
    if (s->spi.init(FF_SPI_SELFTEST_SCL_HZ, 0u, 0u) != FF_SUCCESS) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

FF_Status_t FF_SpiMpfsSetup(FF_SpiMpfs_t *s, const FF_SpiBus_t *bus)
{
    if ((s == NULL) || (bus == NULL) || (bus->read32 == NULL) || (bus->write32 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    s->bus = bus;
    s->clock_hz = 0u;
    s->mode = 0u;
    s->cs = 0u;
    s->ready = 0u;
    s->spi.name = "mpfs-spi";
    s->spi.init = spi_init;
    s->spi.transfer = spi_transfer;
    s->spi.cs_assert = spi_cs_assert;
    s->spi.cs_release = spi_cs_release;
    g_spi = s;
    return FF_SUCCESS;
}
