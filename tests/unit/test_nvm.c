/*
 * frick-fsw - MSS eNVM non-volatile memory driver unit tests (milestone M4)
 *
 * Exercises the bounded eNVM slot table (key/value by id) on the host with an
 * independent mock byte array - SRS-REQ-055. The same driver is exercised against
 * the emulated SoC MMIO in the Renode functional test (milestone M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/nvm/mpfs/ff_nvm_mpfs.h"
#include <string.h>

typedef struct
{
    uint8_t mem[FF_NVM_MAX_IDS * FF_NVM_SLOT_SIZE + 8u]; /* + scratch region */
} MockNvm_t;

static MockNvm_t g_mem;
static FF_NvmMpfs_t g_drv;

static uint8_t mock_read8(void *ctx, uint32_t off)
{
    MockNvm_t *m = (MockNvm_t *)ctx;
    return (off < sizeof(m->mem)) ? m->mem[off] : 0u;
}

static void mock_write8(void *ctx, uint32_t off, uint8_t val)
{
    MockNvm_t *m = (MockNvm_t *)ctx;
    if (off < sizeof(m->mem)) {
        m->mem[off] = val;
    }
}

static void test_nvm(void)
{
    FF_NvmBus_t bus = {&g_mem, mock_read8, mock_write8};
    const uint8_t val[4] = {0x11u, 0x22u, 0x33u, 0x44u};
    uint8_t out[8];
    uint16_t len = 0u;

    memset(&g_mem, 0, sizeof(g_mem));

    /* Setup argument validation. */
    FF_TEST_ASSERT(FF_NvmMpfsSetup(NULL, &bus) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_NvmMpfsSetup(&g_drv, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_NvmMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.nvm.read != NULL);

    /* Write/read round trip by id. */
    FF_TEST_ASSERT(g_drv.nvm.write(3u, val, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.present[3] == 1u);
    FF_TEST_ASSERT(g_drv.nvm.read(3u, out, sizeof(out), &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == 4u);
    FF_TEST_ASSERT(memcmp(out, val, 4u) == 0);

    /* Zero-length value is legal; read returns 0. */
    FF_TEST_ASSERT(g_drv.nvm.write(5u, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.nvm.read(5u, out, sizeof(out), &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == 0u);

    /* Erase clears the length byte. */
    FF_TEST_ASSERT(g_drv.nvm.erase(3u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.present[3] == 0u);
    FF_TEST_ASSERT(g_drv.nvm.read(3u, out, sizeof(out), &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == 0u);

    /* Bounds and argument errors. */
    FF_TEST_ASSERT(g_drv.nvm.write(FF_NVM_MAX_IDS, val, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.nvm.write(1u, val, (uint16_t)FF_NVM_SLOT_SIZE) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.nvm.write(1u, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.nvm.read(1u, NULL, sizeof(out), &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.nvm.read(1u, out, sizeof(out), NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.nvm.read(FF_NVM_MAX_IDS, out, sizeof(out), &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.nvm.erase(FF_NVM_MAX_IDS) == FF_INVALID_ARGUMENT);

    /* A capability smaller than the stored value is a size error. */
    FF_TEST_ASSERT(g_drv.nvm.write(2u, val, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.nvm.read(2u, out, 2u, &len) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_drv.nvm.read(2u, out, 4u, &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == 4u);

    /* Maximum legal value (31 bytes) round-trips. */
    {
        uint8_t big[FF_NVM_MAX_VALUE];
        uint8_t big_out[FF_NVM_MAX_VALUE];
        for (uint16_t i = 0u; i < FF_NVM_MAX_VALUE; i++) {
            big[i] = (uint8_t)i;
        }
        memset(big_out, 0, sizeof(big_out));
        FF_TEST_ASSERT(g_drv.nvm.write(1u, big, FF_NVM_MAX_VALUE) == FF_SUCCESS);
        FF_TEST_ASSERT(g_drv.nvm.read(1u, big_out, sizeof(big_out), &len) == FF_SUCCESS);
        FF_TEST_ASSERT(len == FF_NVM_MAX_VALUE);
        FF_TEST_ASSERT(memcmp(big, big_out, FF_NVM_MAX_VALUE) == 0);
    }

    /* A corrupt length byte (> max) is clamped to the maximum slot payload. */
    {
        uint8_t clamp_out[FF_NVM_MAX_VALUE];
        g_mem.mem[FF_NVM_SLOT_SIZE] = 200u; /* id 1 length byte, out of range */
        FF_TEST_ASSERT(g_drv.nvm.read(1u, out, sizeof(out), &len) == FF_BAD_SIZE);
        FF_TEST_ASSERT(g_drv.nvm.read(1u, clamp_out, sizeof(clamp_out), &len) == FF_SUCCESS);
        FF_TEST_ASSERT(len == FF_NVM_MAX_VALUE);
    }

    /* MMIO self-test. */
    FF_TEST_ASSERT(FF_NvmMpfsSelfTest(&g_drv) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_NvmMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
}

FF_TEST_MAIN(test_nvm)