/*
 * frick-fsw - Static RAM pool + reference DMA helper (PolarFire SoC), HAL backend (M4)
 *
 * Implements the HAL memory contract (FF_MemPool_t, FF_Dma_t; hal_mem.h) for the
 * Microchip PolarFire SoC: a bounded, allocation-free-from-heap static RAM pool
 * (bump allocator over a caller-provided region, typically DDR) and a reference
 * DMA address-translation/cache helper. Exercised on the emulated SoC DDR in the
 * Renode functional test (milestone M11).
 *
 * FF_MemPool_t.alloc has no context parameter, so the pool is a bounded
 * single-instance backend (FF_MemStaticPoolInit binds it).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_MEM_STATIC_H
#define FF_MEM_STATIC_H

#include "common/ff_types.h"
#include "hal/hal_mem.h"
#include <stddef.h>

#define FF_MEM_ALIGN 8u

/* The HAL mem-pool contract has no context parameter, so the pool is a bounded
 * single-instance backend: FF_MemStaticPoolInit() binds the one instance (a
 * second call silently rebinds; the driver is not re-entrant). alloc(0) returns
 * NULL (a zero-size allocation is a no-op request). */
typedef struct
{
    FF_MemPool_t pool; /* public HAL interface (must be first) */
    uint8_t *base;
    uint8_t *cur;
    uint32_t size;
} FF_MemStaticPool_t;

/* Initialise the pool over [base, base+size). Returns FF_SUCCESS or
 * FF_INVALID_ARGUMENT on NULL/zero-length. */
FF_Status_t FF_MemStaticPoolInit(FF_MemStaticPool_t *p, const char *name, void *base,
                                 uint32_t size);

/* Reference DMA helper (identity virt<->phys). NOTE: the cache-maintenance hooks
 * are a seam and are NOT implemented in this reference backend; a real DMA
 * master on a caching core requires correct cache operations (RISC-V
 * cache-management/`fence`), which is on-target work (GAP-A5). Bind a real
 * FF_Dma_t (the Cadence GEM driver takes it by injection) when DMA is used. */
extern FF_Dma_t FF_DmaIdentity;

#endif /* FF_MEM_STATIC_H */
