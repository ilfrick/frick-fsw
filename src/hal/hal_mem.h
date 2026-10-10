/*
 * frick-fsw - HAL contract: memory and storage resources
 *
 * RAM pools and DMA (with the RISC-V cache-coherency obligations), NOR/flash
 * MTD, non-volatile memory (eNVM/sNVM/uPROM) and generic block devices
 * (eMMC/SD/SSD-NVMe) (SRS-REQ-032). All allocation is static/bounded
 * (SSS-REQ-010).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_HAL_MEM_H
#define FF_HAL_MEM_H

#include "common/ff_types.h"
#include <stddef.h>

/* Static RAM pool (no dynamic allocation). */
typedef struct FF_MemPool FF_MemPool_t;
struct FF_MemPool
{
    const char *name;
    void *base;
    uint32_t total;
    uint32_t used;
    void *(*alloc)(size_t n);
    void (*reset)(void);
};

/* DMA helper: address translation and cache maintenance (RISC-V). */
typedef struct FF_Dma FF_Dma_t;
struct FF_Dma
{
    const char *name;
    uint64_t (*virt_to_phys)(const void *p);
    void *(*phys_to_virt)(uint64_t pa);
    void (*cache_flush)(const void *p, size_t n);
    void (*cache_invalidate)(const void *p, size_t n);
};

/* NOR/flash memory technology device, addressed in bytes. */
typedef struct FF_Mtd FF_Mtd_t;
struct FF_Mtd
{
    const char *name;
    uint32_t size;
    uint32_t sector_size;
    FF_Status_t (*read)(uint32_t off, uint8_t *buf, uint32_t len);
    FF_Status_t (*write)(uint32_t off, const uint8_t *buf, uint32_t len);
    FF_Status_t (*erase)(uint32_t off, uint32_t len);
};

/* Non-volatile parameter memory (eNVM/sNVM/uPROM), key/value by id. */
typedef struct FF_Nvm FF_Nvm_t;
struct FF_Nvm
{
    const char *name;
    FF_Status_t (*read)(uint16_t id, uint8_t *buf, uint16_t cap, uint16_t *len);
    FF_Status_t (*write)(uint16_t id, const uint8_t *buf, uint16_t len);
    FF_Status_t (*erase)(uint16_t id);
};

/* Generic block device (eMMC, SD, SSD/NVMe), addressed in blocks. */
typedef struct FF_BlockDev FF_BlockDev_t;
struct FF_BlockDev
{
    const char *name;
    uint32_t block_size;
    uint64_t num_blocks;
    FF_Status_t (*read)(uint64_t lba, uint8_t *buf, uint32_t blocks);
    FF_Status_t (*write)(uint64_t lba, const uint8_t *buf, uint32_t blocks);
    FF_Status_t (*flush)(void);
};

#endif /* FF_HAL_MEM_H */
