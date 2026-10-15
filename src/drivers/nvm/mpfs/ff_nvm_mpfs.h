/*
 * frick-fsw - MSS eNVM non-volatile memory driver (PolarFire SoC), HAL backend (M4)
 *
 * Implements the HAL non-volatile-memory contract (FF_Nvm_t, key/value by id) for
 * the Microchip PolarFire SoC MSS embedded NVM (eNVM), used for bounded on-board
 * parameters. MMIO access is injected (FF_NvmBus_t, byte-wide) so the register
 * logic is testable on the host and exercised on the emulated SoC in the Renode
 * functional test (milestone M11).
 *
 * Layout: a fixed slot table (FF_NVM_MAX_IDS slots of FF_NVM_SLOT_SIZE bytes);
 * each slot stores one length byte (0..FF_NVM_MAX_VALUE) followed by the payload.
 * Static, no allocation; bounded ids and bounded values.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_NVM_MPFS_H
#define FF_NVM_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_mem.h"

typedef struct
{
    void *ctx;
    uint8_t (*read8)(void *ctx, uint32_t off);
    void (*write8)(void *ctx, uint32_t off, uint8_t val);
} FF_NvmBus_t;

#define FF_NVM_MAX_IDS 32u
#define FF_NVM_SLOT_SIZE 32u /* bytes per id: 1 length byte + up to 31 payload */
#define FF_NVM_MAX_VALUE (FF_NVM_SLOT_SIZE - 1u)
/* Scratch offset used by the MMIO self-test, outside the parameter slot table. */
#define FF_NVM_SCRATCH_OFF (FF_NVM_MAX_IDS * FF_NVM_SLOT_SIZE)

typedef struct
{
    FF_Nvm_t nvm; /* public HAL interface (must be first) */
    FF_NvmBus_t bus;
    uint8_t present[FF_NVM_MAX_IDS];
} FF_NvmMpfs_t;

/* Bind the instance and complete the FF_Nvm_t vtable. */
FF_Status_t FF_NvmMpfsSetup(FF_NvmMpfs_t *n, const FF_NvmBus_t *bus);

/* MMIO self-test (scratch round trip): used by the host unit test and by the
 * Renode functional check (the eNVM DATA region at 0x20220000 is writable). */
FF_Status_t FF_NvmMpfsSelfTest(FF_NvmMpfs_t *n);

#endif /* FF_NVM_MPFS_H */