/*
 * frick-fsw - MSS QSPI NOR flash MTD driver unit tests (milestone M4)
 *
 * Exercises the byte-addressed NOR MTD contract (read/write/erase, sector-aligned
 * erase) on the host with an independent mock byte array - SRS-REQ-056. The on-SoC
 * QSPI check needs the board repl (`MPFS_QSPI` + `Micron_MT25Q`) - M11 follow-up.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/mtd/mpfs/ff_mtd_mpfs.h"
#include <string.h>

#define MOCK_MTD_SIZE 8192u /* two 4 KiB sectors */

typedef struct
{
    uint8_t mem[MOCK_MTD_SIZE];
} MockMtd_t;

static MockMtd_t g_mem;
static FF_MtdMpfs_t g_drv;

static uint8_t mock_read8(void *ctx, uint32_t off)
{
    MockMtd_t *m = (MockMtd_t *)ctx;
    return (off < sizeof(m->mem)) ? m->mem[off] : 0u;
}

static void mock_write8(void *ctx, uint32_t off, uint8_t val)
{
    MockMtd_t *m = (MockMtd_t *)ctx;
    if (off < sizeof(m->mem)) {
        m->mem[off] = val;
    }
}

static void test_mtd(void)
{
    FF_MtdBus_t bus = {&g_mem, mock_read8, mock_write8};
    const uint8_t val[4] = {0xDEu, 0xADu, 0xBEu, 0xEFu};
    uint8_t out[8];
    uint32_t i;

    memset(&g_mem, 0, sizeof(g_mem));

    /* Setup validation. */
    FF_TEST_ASSERT(FF_MtdMpfsSetup(NULL, &bus, MOCK_MTD_SIZE, 4096u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, NULL, MOCK_MTD_SIZE, 4096u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, &bus, 0u, 4096u) == FF_INVALID_ARGUMENT);
    /* size not a multiple of the sector size */
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, &bus, 4097u, 4096u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, &bus, MOCK_MTD_SIZE, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, &bus, MOCK_MTD_SIZE, 4096u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.mtd.size == MOCK_MTD_SIZE);
    FF_TEST_ASSERT(g_drv.mtd.sector_size == 4096u);

    /* Write/read round trip. */
    FF_TEST_ASSERT(g_drv.mtd.write(100u, val, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.mtd.read(100u, out, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(memcmp(out, val, 4u) == 0);

    /* Bounds errors. */
    FF_TEST_ASSERT(g_drv.mtd.read(MOCK_MTD_SIZE, out, 1u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_drv.mtd.write((uint32_t)(MOCK_MTD_SIZE - 2u), val, 4u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_drv.mtd.read(0u, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.mtd.write(0u, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.mtd.read(0u, out, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.mtd.write(0u, val, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.mtd.read(MOCK_MTD_SIZE + 1u, out, 1u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_drv.mtd.erase(MOCK_MTD_SIZE, 4096u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, &bus, (2u * 1024u * 1024u) + 4096u, 4096u) ==
                   FF_INVALID_ARGUMENT); /* size > FF_MTD_MAX_SIZE */
    FF_TEST_ASSERT(FF_MtdMpfsSetup(&g_drv, &bus, MOCK_MTD_SIZE, 4096u) == FF_SUCCESS);

    /* Erase is sector aligned; erasing sets 0xFF. */
    FF_TEST_ASSERT(g_drv.mtd.erase(1u, 4096u) == FF_INVALID_ARGUMENT); /* off unaligned */
    FF_TEST_ASSERT(g_drv.mtd.erase(0u, 100u) == FF_INVALID_ARGUMENT);  /* len unaligned */
    FF_TEST_ASSERT(g_drv.mtd.erase(0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.mtd.erase(0u, 4096u) == FF_SUCCESS);
    for (i = 0u; i < 4096u; i++) {
        FF_TEST_ASSERT(g_mem.mem[i] == 0xFFu);
    }
    /* The erased sector leaves the second sector untouched. */
    FF_TEST_ASSERT(g_drv.mtd.write(4200u, val, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.mtd.read(4200u, out, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(memcmp(out, val, 4u) == 0);
    FF_TEST_ASSERT(g_drv.mtd.erase(0u, 8192u) == FF_SUCCESS); /* whole device */

    /* MMIO self-test. */
    FF_TEST_ASSERT(FF_MtdMpfsSelfTest(&g_drv) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mem.mem[0] == 0x5Au);
    FF_TEST_ASSERT(FF_MtdMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
}

FF_TEST_MAIN(test_mtd)