/*
 * frick-fsw - Static RAM pool + reference DMA helper (PolarFire SoC), HAL backend (M4)
 *
 * Bounded static pool over a caller-provided region (usually DDR). No heap
 * allocation. Single instance (the HAL mem-pool contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/mem/ff_mem_static.h"

static FF_MemStaticPool_t *g_pool;

static void *mem_alloc(size_t n)
{
    uintptr_t cur;
    uintptr_t aligned;
    uintptr_t limit;

    if ((g_pool == NULL) || (n == 0u)) {
        return NULL;
    }
    cur = (uintptr_t)g_pool->cur;
    aligned = (cur + (FF_MEM_ALIGN - 1u)) & ~((uintptr_t)FF_MEM_ALIGN - 1u);
    limit = (uintptr_t)g_pool->base + g_pool->size;
    if ((aligned > limit) || (n > (limit - aligned))) {
        return NULL; /* exhausted */
    }
    g_pool->cur = (uint8_t *)(aligned + n);
    g_pool->pool.used = (uint32_t)((uintptr_t)g_pool->cur - (uintptr_t)g_pool->base);
    return (void *)aligned;
}

static void mem_reset(void)
{
    if (g_pool != NULL) {
        g_pool->cur = g_pool->base;
        g_pool->pool.used = 0u;
    }
}

FF_Status_t FF_MemStaticPoolInit(FF_MemStaticPool_t *p, const char *name, void *base, uint32_t size)
{
    if ((p == NULL) || (base == NULL) || (size == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    p->base = (uint8_t *)base;
    p->cur = (uint8_t *)base;
    p->size = size;
    p->pool.name = name;
    p->pool.base = base;
    p->pool.total = size;
    p->pool.used = 0u;
    p->pool.alloc = mem_alloc;
    p->pool.reset = mem_reset;
    g_pool = p;
    return FF_SUCCESS;
}

/* Reference DMA helper: identity virtual<->physical translation (no MMU on the
 * U54s as used here). Cache maintenance is a no-op in this reference backend;
 * platforms with cache-management operations must provide a real FF_Dma_t (the
 * Cadence GEM driver takes the FF_Dma_t by injection, so this is a seam). */
static uint64_t dma_virt_to_phys(const void *p)
{
    return (uint64_t)(uintptr_t)p;
}

static void *dma_phys_to_virt(uint64_t pa)
{
    return (void *)(uintptr_t)pa;
}

static void dma_cache_flush(const void *p, size_t n)
{
    (void)p;
    (void)n;
}

static void dma_cache_invalidate(const void *p, size_t n)
{
    (void)p;
    (void)n;
}

FF_Dma_t FF_DmaIdentity = {
    "dma-identity", dma_virt_to_phys, dma_phys_to_virt, dma_cache_flush, dma_cache_invalidate,
};
