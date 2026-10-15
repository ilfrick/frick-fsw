/*
 * frick-fsw - MSS QSPI NOR flash MTD driver (PolarFire SoC), HAL backend (M4)
 *
 * Byte-addressed NOR MTD over an injected MMIO window. Static, no allocation.
 * Single instance (the HAL MTD contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/mtd/mpfs/ff_mtd_mpfs.h"

static FF_MtdMpfs_t *g_mtd; /* single bound instance */

static FF_Status_t mtd_read(uint32_t off, uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if ((g_mtd == NULL) || (buf == NULL) || (len == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((off > g_mtd->mtd.size) || (len > (g_mtd->mtd.size - off))) {
        return FF_BAD_SIZE;
    }
    for (i = 0u; i < len; i++) {
        buf[i] = g_mtd->bus.read8(g_mtd->bus.ctx, off + i);
    }
    return FF_SUCCESS;
}

static FF_Status_t mtd_write(uint32_t off, const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if ((g_mtd == NULL) || (buf == NULL) || (len == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((off > g_mtd->mtd.size) || (len > (g_mtd->mtd.size - off))) {
        return FF_BAD_SIZE;
    }
    for (i = 0u; i < len; i++) {
        g_mtd->bus.write8(g_mtd->bus.ctx, off + i, buf[i]);
    }
    return FF_SUCCESS;
}

static FF_Status_t mtd_erase(uint32_t off, uint32_t len)
{
    uint32_t i;
    uint32_t sector;

    if (g_mtd == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    sector = g_mtd->mtd.sector_size;
    if ((len == 0u) || ((off % sector) != 0u) || ((len % sector) != 0u)) {
        return FF_INVALID_ARGUMENT; /* erase is sector aligned */
    }
    if ((off > g_mtd->mtd.size) || (len > (g_mtd->mtd.size - off))) {
        return FF_BAD_SIZE;
    }
    for (i = 0u; i < len; i++) {
        g_mtd->bus.write8(g_mtd->bus.ctx, off + i, 0xFFu);
    }
    return FF_SUCCESS;
}

FF_Status_t FF_MtdMpfsSelfTest(FF_MtdMpfs_t *m)
{
    uint8_t v;

    if (m == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Scratch round trip at the start of the window. */
    m->bus.write8(m->bus.ctx, 0u, 0x5Au);
    v = m->bus.read8(m->bus.ctx, 0u);
    return (v == 0x5Au) ? FF_SUCCESS : FF_ERROR;
}

FF_Status_t FF_MtdMpfsSetup(FF_MtdMpfs_t *m, const FF_MtdBus_t *bus, uint32_t size,
                            uint32_t sector_size)
{
    if ((m == NULL) || (bus == NULL) || (size == 0u) || (size > FF_MTD_MAX_SIZE) ||
        (sector_size == 0u) || ((size % sector_size) != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    m->bus = *bus;
    m->mtd.name = "mpfs-qspi-nor";
    m->mtd.size = size;
    m->mtd.sector_size = sector_size;
    m->mtd.read = mtd_read;
    m->mtd.write = mtd_write;
    m->mtd.erase = mtd_erase;
    g_mtd = m;
    return FF_SUCCESS;
}