/*
 * frick-fsw - MSS eNVM non-volatile memory driver (PolarFire SoC), HAL backend (M4)
 *
 * Bounded slot table over the eNVM byte-addressed region. Static, no allocation.
 * Single instance (the HAL NVM contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/nvm/mpfs/ff_nvm_mpfs.h"

static FF_NvmMpfs_t *g_nvm; /* single bound instance */

static uint32_t nvm_off(uint16_t id)
{
    return (uint32_t)id * FF_NVM_SLOT_SIZE;
}

static FF_Status_t nvm_write(uint16_t id, const uint8_t *buf, uint16_t len)
{
    uint32_t off;
    uint16_t i;

    if ((g_nvm == NULL) || (id >= FF_NVM_MAX_IDS) || ((buf == NULL) && (len != 0u)) ||
        (len > FF_NVM_MAX_VALUE)) {
        return FF_INVALID_ARGUMENT;
    }
    off = nvm_off(id);
    g_nvm->bus.write8(g_nvm->bus.ctx, off, (uint8_t)len);
    for (i = 0u; i < len; i++) {
        g_nvm->bus.write8(g_nvm->bus.ctx, off + 1u + i, buf[i]);
    }
    g_nvm->present[id] = 1u;
    return FF_SUCCESS;
}

static FF_Status_t nvm_read(uint16_t id, uint8_t *buf, uint16_t cap, uint16_t *len)
{
    uint32_t off;
    uint16_t n;
    uint16_t i;

    if ((g_nvm == NULL) || (id >= FF_NVM_MAX_IDS) || (buf == NULL) || (len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    off = nvm_off(id);
    n = g_nvm->bus.read8(g_nvm->bus.ctx, off);
    if (n > FF_NVM_MAX_VALUE) {
        n = FF_NVM_MAX_VALUE; /* defensively clamp a corrupt length byte */
    }
    if (cap < n) {
        return FF_BAD_SIZE;
    }
    for (i = 0u; i < n; i++) {
        buf[i] = g_nvm->bus.read8(g_nvm->bus.ctx, off + 1u + i);
    }
    *len = n;
    return FF_SUCCESS;
}

static FF_Status_t nvm_erase(uint16_t id)
{
    if ((g_nvm == NULL) || (id >= FF_NVM_MAX_IDS)) {
        return FF_INVALID_ARGUMENT;
    }
    g_nvm->bus.write8(g_nvm->bus.ctx, nvm_off(id), 0u);
    g_nvm->present[id] = 0u;
    return FF_SUCCESS;
}

FF_Status_t FF_NvmMpfsSelfTest(FF_NvmMpfs_t *n)
{
    uint8_t v;

    if (n == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Scratch round trip, outside the parameter slot table (id space untouched). */
    n->bus.write8(n->bus.ctx, FF_NVM_SCRATCH_OFF, 1u);
    n->bus.write8(n->bus.ctx, FF_NVM_SCRATCH_OFF + 1u, 0xA5u);
    v = n->bus.read8(n->bus.ctx, FF_NVM_SCRATCH_OFF + 1u);
    return (v == 0xA5u) ? FF_SUCCESS : FF_ERROR;
}

FF_Status_t FF_NvmMpfsSetup(FF_NvmMpfs_t *n, const FF_NvmBus_t *bus)
{
    uint16_t i;

    if ((n == NULL) || (bus == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    n->bus = *bus;
    for (i = 0u; i < FF_NVM_MAX_IDS; i++) {
        n->present[i] = 0u;
    }
    n->nvm.name = "mpfs-envm";
    n->nvm.read = nvm_read;
    n->nvm.write = nvm_write;
    n->nvm.erase = nvm_erase;
    g_nvm = n;
    return FF_SUCCESS;
}