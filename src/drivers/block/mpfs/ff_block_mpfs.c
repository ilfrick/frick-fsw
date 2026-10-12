/*
 * frick-fsw - MSS MMC/SD block-device driver (PolarFire SoC), HAL backend (M4)
 *
 * Polled single/multi-block buffer-mode host over the Microchip PolarFire SoC
 * MSS MMC/SD controller (SDHCI-style), register logic on an injected
 * double-word MMIO bus. Static, no allocation. Single instance (the HAL
 * block-device contract carries no context).
 *
 * Transfer shape: program BlockSizeBlockCount and Argument1, issue the
 * single/multi-block read/write command through CommandTransferMode, poll
 * ErrorNormalInterruptStatus for completion (error bits -> FF_ERROR), then move
 * the payload through the 32-bit DataBuffer port. See the header for the
 * register map, the reference-mapping/GAP-A5 notes and the known limitations.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "drivers/block/mpfs/ff_block_mpfs.h"

FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MIN <= FF_MMC_BLOCK_SIZE_MAX, block_size_bounds);
/* The transfer block size must fit the 12-bit TBS field (a 4096-byte block
 * would truncate to 0 and silently corrupt the transfer). */
FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK, block_size_fits_tbs);
FF_STATIC_ASSERT(FF_MMC_BCCT_MAX <= FF_MMC_BCCT_MASK, block_count_fits_field);

static FF_BlockMpfs_t *g_block; /* single bound instance */

static uint32_t block_rd(uint32_t off)
{
    return g_block->bus->read32(g_block->bus->ctx, off);
}

static void block_wr(uint32_t off, uint32_t val)
{
    g_block->bus->write32(g_block->bus->ctx, off, val);
}

static FF_Status_t block_check_ready(void)
{
    if (g_block == NULL) {
        return FF_NOT_READY;
    }
    if (g_block->ready == 0u) {
        return FF_NOT_READY;
    }
    return FF_SUCCESS;
}

/* Poll until the controller reports the command and data transfer complete, or
 * an error bit is set (returned as FF_ERROR). Bounded by FF_MMC_POLL_MAX. The
 * handled completion bits are cleared (write-1-to-clear). */
static FF_Status_t block_wait_done(void)
{
    uint32_t i;

    for (i = 0u; i < FF_MMC_POLL_MAX; i++) {
        uint32_t st = block_rd(FF_MMC_REG_ERR_NORMAL_INT_STATUS);
        if ((st & FF_MMC_INT_ERROR_MASK) != 0u) {
            /* Clear the whole status word (write-1-to-clear) before returning. */
            block_wr(FF_MMC_REG_ERR_NORMAL_INT_STATUS, st);
            return FF_ERROR;
        }
        if ((st & FF_MMC_INT_DONE) == FF_MMC_INT_DONE) {
            block_wr(FF_MMC_REG_ERR_NORMAL_INT_STATUS, FF_MMC_INT_DONE);
            return FF_SUCCESS;
        }
    }
    /* Clear the whole status word so a late/stale completion or error bit cannot
     * mis-lead the next transfer. */
    block_wr(FF_MMC_REG_ERR_NORMAL_INT_STATUS, FF_MMC_INT_CLEAR_ALL);
    return FF_TIMEOUT;
}

/* Program the transfer and issue a single/multi-block read (is_read != 0) or
 * write command for `blocks` blocks starting at `arg` (a block address). */
static FF_Status_t block_issue(uint8_t cmd_index, uint32_t arg, uint32_t blocks, uint8_t is_read)
{
    uint32_t multi = (blocks > 1u) ? 1u : 0u;
    uint32_t cmd;

    /* Unmask the completion and error status bits (the emulated controller only
     * sets an interrupt bit when it is enabled). */
    block_wr(FF_MMC_REG_ERR_NORMAL_INT_ENABLE, FF_MMC_INT_DONE | FF_MMC_INT_ERROR_MASK);
    block_wr(FF_MMC_REG_BLOCK_SIZE_COUNT, ((blocks & FF_MMC_BCCT_MASK) << FF_MMC_BCCT_SHIFT) |
                                              (g_block->block_size & FF_MMC_TBS_MASK));
    block_wr(FF_MMC_REG_ARGUMENT1, arg);

    cmd = FF_MMC_CMD_DATA_PRESENT | ((uint32_t)FF_MMC_CMD_RESP_48 << FF_MMC_CMD_RESP_TYPE_SHIFT) |
          ((uint32_t)cmd_index << FF_MMC_CMD_INDEX_SHIFT);
    if (multi != 0u) {
        cmd |= FF_MMC_XFER_BLOCK_COUNT_ENABLE | FF_MMC_XFER_MULTI_BLOCK;
    }
    if (is_read != 0u) {
        cmd |= FF_MMC_XFER_DATA_DIR_READ;
    }
    block_wr(FF_MMC_REG_CMD_XFER_MODE, cmd);

    return block_wait_done();
}

/* Move total_bytes from the controller DataBuffer into buf (read path),
 * little-endian, 4 bytes at a time. total_bytes is a multiple of 4 because
 * block_size is a power of two in [512, 2048]. */
static void block_move_in(uint8_t *buf, uint32_t total_bytes)
{
    uint32_t words = total_bytes / 4u;
    uint32_t i;

    for (i = 0u; i < words; i++) {
        uint32_t w = block_rd(FF_MMC_REG_DATA_BUFFER);
        buf[(i * 4u) + 0u] = (uint8_t)(w & 0xFFu);
        buf[(i * 4u) + 1u] = (uint8_t)((w >> 8) & 0xFFu);
        buf[(i * 4u) + 2u] = (uint8_t)((w >> 16) & 0xFFu);
        buf[(i * 4u) + 3u] = (uint8_t)((w >> 24) & 0xFFu);
    }
}

/* Move total_bytes from buf into the controller DataBuffer (write path),
 * little-endian, 4 bytes at a time. */
static void block_move_out(const uint8_t *buf, uint32_t total_bytes)
{
    uint32_t words = total_bytes / 4u;
    uint32_t i;

    for (i = 0u; i < words; i++) {
        uint32_t w = (uint32_t)buf[(i * 4u) + 0u] | ((uint32_t)buf[(i * 4u) + 1u] << 8) |
                     ((uint32_t)buf[(i * 4u) + 2u] << 16) | ((uint32_t)buf[(i * 4u) + 3u] << 24);
        block_wr(FF_MMC_REG_DATA_BUFFER, w);
    }
}

/* Common argument validation for read/write. Returns FF_SUCCESS when the request
 * is valid for the bound device. */
static FF_Status_t block_check_args(uint64_t lba, uint32_t blocks)
{
    if ((blocks == 0u) || (blocks > FF_MMC_BCCT_MAX)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Argument1 is 32 bits: a larger block address cannot be issued. */
    if (lba > 0xFFFFFFFFu) {
        return FF_INVALID_ARGUMENT;
    }
    if ((lba >= g_block->num_blocks) || ((uint64_t)blocks > (g_block->num_blocks - lba))) {
        return FF_INVALID_ARGUMENT;
    }
    return FF_SUCCESS;
}

static FF_Status_t block_read(uint64_t lba, uint8_t *buf, uint32_t blocks)
{
    uint8_t cmd_index;
    FF_Status_t rc;

    rc = block_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if (buf == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    rc = block_check_args(lba, blocks);
    if (rc != FF_SUCCESS) {
        return rc;
    }

    cmd_index = (blocks > 1u) ? (uint8_t)FF_MMC_CMD_READ_MULTIPLE_BLOCKS
                              : (uint8_t)FF_MMC_CMD_READ_SINGLE_BLOCK;
    rc = block_issue(cmd_index, (uint32_t)lba, blocks, 1u);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    block_move_in(buf, blocks * g_block->block_size);
    return FF_SUCCESS;
}

static FF_Status_t block_write(uint64_t lba, const uint8_t *buf, uint32_t blocks)
{
    uint8_t cmd_index;
    FF_Status_t rc;

    rc = block_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if (buf == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    rc = block_check_args(lba, blocks);
    if (rc != FF_SUCCESS) {
        return rc;
    }

    cmd_index = (blocks > 1u) ? (uint8_t)FF_MMC_CMD_WRITE_MULTIPLE_BLOCKS
                              : (uint8_t)FF_MMC_CMD_WRITE_SINGLE_BLOCK;
    rc = block_issue(cmd_index, (uint32_t)lba, blocks, 0u);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    block_move_out(buf, blocks * g_block->block_size);
    return FF_SUCCESS;
}

/* No write-back cache: there is nothing to flush. A CMD12 stop-transmission /
 * abort path is not implemented (see the header limitations). */
static FF_Status_t block_flush(void)
{
    return FF_SUCCESS;
}

FF_Status_t FF_BlockMpfsSelfTest(FF_BlockMpfs_t *d)
{
    uint32_t cap;

    if ((d == NULL) || (d != g_block)) {
        return FF_INVALID_ARGUMENT;
    }
    /* BlockSizeBlockCount round trip (TBS + BCCT fields). */
    d->bus->write32(d->bus->ctx, FF_MMC_REG_BLOCK_SIZE_COUNT,
                    ((1u & FF_MMC_BCCT_MASK) << FF_MMC_BCCT_SHIFT) |
                        (FF_MMC_SELFTEST_BLOCK_SIZE & FF_MMC_TBS_MASK));
    if (d->bus->read32(d->bus->ctx, FF_MMC_REG_BLOCK_SIZE_COUNT) !=
        (((1u & FF_MMC_BCCT_MASK) << FF_MMC_BCCT_SHIFT) |
         (FF_MMC_SELFTEST_BLOCK_SIZE & FF_MMC_TBS_MASK))) {
        return FF_ERROR;
    }
    d->bus->write32(d->bus->ctx, FF_MMC_REG_BLOCK_SIZE_COUNT, 0u);
    if (d->bus->read32(d->bus->ctx, FF_MMC_REG_BLOCK_SIZE_COUNT) != 0u) {
        return FF_ERROR;
    }
    /* Argument1 round trip. */
    d->bus->write32(d->bus->ctx, FF_MMC_REG_ARGUMENT1, 0xA5A5A5A5u);
    if (d->bus->read32(d->bus->ctx, FF_MMC_REG_ARGUMENT1) != 0xA5A5A5A5u) {
        return FF_ERROR;
    }
    d->bus->write32(d->bus->ctx, FF_MMC_REG_ARGUMENT1, 0u);
    if (d->bus->read32(d->bus->ctx, FF_MMC_REG_ARGUMENT1) != 0u) {
        return FF_ERROR;
    }
    /* Capabilities1 must advertise SDMA and 3.3 V support. */
    cap = d->bus->read32(d->bus->ctx, FF_MMC_REG_CAPABILITIES1);
    if ((cap & (FF_MMC_CAP1_SDMA | FF_MMC_CAP1_VOLTAGE_3V3)) !=
        (FF_MMC_CAP1_SDMA | FF_MMC_CAP1_VOLTAGE_3V3)) {
        return FF_ERROR;
    }
    /* HostControl2 must report the internal clock stable. */
    if ((d->bus->read32(d->bus->ctx, FF_MMC_REG_HOST_CONTROL2) & FF_MMC_HC2_CLOCK_STABLE) == 0u) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

FF_Status_t FF_BlockMpfsSetup(FF_BlockMpfs_t *d, const FF_BlockBus_t *bus, uint32_t block_size,
                              uint64_t num_blocks)
{
    if ((d == NULL) || (bus == NULL) || (bus->read32 == NULL) || (bus->write32 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((block_size < FF_MMC_BLOCK_SIZE_MIN) || (block_size > FF_MMC_BLOCK_SIZE_MAX) ||
        ((block_size & (block_size - 1u)) != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (num_blocks == 0u) {
        return FF_INVALID_ARGUMENT;
    }
    d->bus = bus;
    d->block_size = block_size;
    d->num_blocks = num_blocks;
    d->ready = 1u;
    d->dev.name = "mpfs-mmc";
    d->dev.block_size = block_size;
    d->dev.num_blocks = num_blocks;
    d->dev.read = block_read;
    d->dev.write = block_write;
    d->dev.flush = block_flush;
    g_block = d;
    return FF_SUCCESS;
}
