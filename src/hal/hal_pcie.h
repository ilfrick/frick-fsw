/*
 * frick-fsw - HAL contract: PCIe endpoint/root complex
 *
 * Generic PCIe contract (configuration space access, MSI) so the middleware
 * can use a PCIe-attached device (e.g. an NVMe SSD, SRS-REQ-031/032) without
 * depending on the host controller specifics.
 *
 * This increment provides configuration-space read/write (ECAM) and MSI
 * enable/disable; DMA and full enumeration are tracked as residuals (GAP-A5).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_PCIE_H
#define FF_HAL_PCIE_H

#include "common/ff_types.h"

typedef struct FF_Pcie FF_Pcie_t;
struct FF_Pcie
{
    const char *name;
    FF_Status_t (*init)(uint8_t root_complex);
    FF_Status_t (*read_config)(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t offset,
                               uint32_t *out_val);
    FF_Status_t (*write_config)(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t offset, uint32_t val);
    FF_Status_t (*msi_enable)(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t vector);
    FF_Status_t (*msi_disable)(uint8_t bus, uint8_t dev, uint8_t fn);
};

#endif /* FF_HAL_PCIE_H */