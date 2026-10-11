/*
 * frick-fsw - Static RAM pool + DMA helper unit tests (milestone M4)
 *
 * Exercises the FF_MemPool_t bump allocator and the reference FF_Dma_t on the
 * host (SRS-REQ-049). The pool is exercised over the emulated DDR in the Renode
 * functional test (milestone M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/mem/ff_mem_static.h"
#include <string.h>

#define POOL_BYTES 256u

static uint8_t g_area[POOL_BYTES + 16u]; /* +16 so over-read is detectable */
static FF_MemStaticPool_t g_pool;

static void test_mem(void)
{
    void *a;
    void *b;
    uint8_t *p;

    memset(g_area, 0, sizeof(g_area));

    FF_TEST_ASSERT(FF_MemStaticPoolInit(NULL, "x", g_area, POOL_BYTES) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MemStaticPoolInit(&g_pool, "mem0", NULL, POOL_BYTES) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MemStaticPoolInit(&g_pool, "mem0", g_area, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_MemStaticPoolInit(&g_pool, "mem0", g_area, POOL_BYTES) == FF_SUCCESS);
    FF_TEST_ASSERT(g_pool.pool.total == POOL_BYTES);
    FF_TEST_ASSERT(g_pool.pool.used == 0u);

    /* Allocation is 8-byte aligned and non-overlapping. */
    a = g_pool.pool.alloc(1u);
    b = g_pool.pool.alloc(8u);
    FF_TEST_ASSERT(a != NULL);
    FF_TEST_ASSERT(b != NULL);
    FF_TEST_ASSERT(FF_MEM_ALIGN == 8u); /* literal contract */
    FF_TEST_ASSERT(((uintptr_t)a % 8u) == 0u);
    FF_TEST_ASSERT(((uintptr_t)b % 8u) == 0u);
    FF_TEST_ASSERT((uintptr_t)b >= (uintptr_t)a + FF_MEM_ALIGN);
    FF_TEST_ASSERT(g_pool.pool.used > 0u);

    /* Writable memory. */
    p = (uint8_t *)a;
    p[0] = 0x5Au;
    FF_TEST_ASSERT(*p == 0x5Au);

    /* Exhaustion returns NULL (bounded, no overflow). */
    FF_TEST_ASSERT(g_pool.pool.alloc(0u) == NULL);
    FF_TEST_ASSERT(g_pool.pool.alloc(POOL_BYTES * 2u) == NULL);

    /* Reset returns the whole region. */
    g_pool.pool.reset();
    FF_TEST_ASSERT(g_pool.pool.used == 0u);
    FF_TEST_ASSERT(g_pool.pool.alloc(POOL_BYTES) != NULL);

    /* Reference DMA helper: identity translation. */
    FF_TEST_ASSERT(FF_DmaIdentity.virt_to_phys != NULL);
    FF_TEST_ASSERT(FF_DmaIdentity.virt_to_phys(a) == (uint64_t)(uintptr_t)a);
    FF_TEST_ASSERT(FF_DmaIdentity.phys_to_virt((uint64_t)(uintptr_t)a) == a);
    FF_DmaIdentity.cache_flush(a, 8u);      /* no-op, must not crash */
    FF_DmaIdentity.cache_invalidate(a, 8u); /* no-op */

    /* Exact-fit allocation exhausts the region without overflow. */
    g_pool.pool.reset();
    FF_TEST_ASSERT(g_pool.pool.alloc(POOL_BYTES) != NULL);
    FF_TEST_ASSERT(g_pool.pool.alloc(1u) == NULL);
}

FF_TEST_MAIN(test_mem)
