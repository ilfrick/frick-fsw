/*
 * frick-fsw - HAL contract: PCIe endpoint/root complex
 *
 * Generic PCIe contract (enumeration, BAR access, DMA, MSI) so the middleware
 * can use a PCIe-attached device (e.g. an NVMe SSD, SRS-REQ-031/032) without
 * depending on the host controller specifics.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_HAL_PCIE_H
#define FF_HAL_PCIE_H

#include "common/ff_types.h"

typedef struct FF_Pcie FF_Pcie_t;
struct FF_Pcie
{
    const char *name;
    FF_Status_t (*init)(uint8_t root_complex);
    FF_Status_t (*enumerate)(void);
    FF_Status_t (*map_bar)(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t bar, void **out_base,
                           uint32_t *out_size);
    FF_Status_t (*dma_write)(uint64_t dst, const void *src, uint32_t len);
    FF_Status_t (*dma_read)(void *dst, uint64_t src, uint32_t len);
    FF_Status_t (*enable_msi)(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t vector);
};

#endif /* FF_HAL_PCIE_H */
