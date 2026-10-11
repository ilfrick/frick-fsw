/*
 * frick-fsw - MSS QSPI NOR flash MTD driver (PolarFire SoC), HAL backend (M4)
 *
 * Implements the HAL memory-technology-device contract (FF_Mtd_t: read/write/
 * erase, byte-addressed) for the Microchip PolarFire SoC MSS QSPI NOR flash, a
 * NOR/flash MTD used for configuration/patch storage. MMIO access is injected
 * (FF_MtdBus_t, byte-wide) so the register logic is testable on the host and
 * exercised on the emulated SoC in the Renode functional test (milestone M11).
 *
 * NOR semantics: erase is sector-granular (off and len must be multiples of the
 * sector size) and sets bytes to 0xFF; a write requires the target to have been
 * erased first (bits only go 1 -> 0). The reference backend performs plain byte
 * writes and leaves erase-before-write to the caller; real on-target flash
 * timing/endurance/retention is GAP-A5.
 *
 * Static, no allocation; bounded window. Single instance (the HAL MTD contract
 * carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_MTD_MPFS_H
#define FF_MTD_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_mem.h"

typedef struct
{
    void *ctx;
    uint8_t (*read8)(void *ctx, uint32_t off);
    void (*write8)(void *ctx, uint32_t off, uint8_t val);
} FF_MtdBus_t;

#define FF_MTD_DEFAULT_SECTOR_SIZE 4096u     /* QSPI NOR 4 KiB sector      */
#define FF_MTD_MAX_SIZE (2u * 1024u * 1024u) /* reference window bound     */

typedef struct
{
    FF_Mtd_t mtd; /* public HAL interface (must be first) */
    FF_MtdBus_t bus;
} FF_MtdMpfs_t;

/* Bind the instance and complete the FF_Mtd_t vtable (size/sector_size). */
FF_Status_t FF_MtdMpfsSetup(FF_MtdMpfs_t *m, const FF_MtdBus_t *bus, uint32_t size,
                            uint32_t sector_size);

/* MMIO scratch round trip used by the host unit test (not part of the HAL
 * contract). The Renode check is a read-presence check (the XIP window is
 * read-only in the model). */
FF_Status_t FF_MtdMpfsSelfTest(FF_MtdMpfs_t *m);

#endif /* FF_MTD_MPFS_H */