/*
 * frick-fsw - Fabric (FIC/AXI) device backend (PolarFire SoC), HAL backend (M4/S4c)
 *
 * Window/range implementation for the HAL fabric contract (SRS-REQ-067).
 * Static, bounded, no allocation at any time (SRS-REQ-015 / norm N10).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/fabric/mpfs/ff_fabric_mpfs.h"

static FF_FabricMpfs_t *g_fab; /* single bound instance */

/* Overflow-free window range test: an access of `width` bytes at the
 * window-relative offset `off` is inside a span of `size` bytes when the offset
 * is inside the span and the remaining span covers `width`. `size - off` is only
 * evaluated after `off < size`, so the subtraction can never wrap - including
 * the caller passing off = 0xFFFFFFFF with any size. The acceptance condition is
 * exactly `off + width <= size` (verified in Frick/Fabric.lean), so no accepted
 * access can reach past the end of the device window. */
static FF_Status_t fabric_range(uint32_t size, uint32_t off, uint32_t width)
{
    if ((width == 0u) || (off >= size)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((size - off) < width) {
        return FF_INVALID_ARGUMENT;
    }
    return FF_SUCCESS;
}

/* Resolve a caller-supplied handle against the catalogue and, when it matches,
 * return the catalogue entry to access through. A handle that is closed or a
 * backend that is not initialised is FF_NOT_READY; an unknown id, or a handle
 * whose id/base/span no longer matches the catalogue, is FF_INVALID_ARGUMENT. */
static FF_Status_t fabric_resolve(const FF_FabricWindow_t *w, const FF_FabricMap_t **entry)
{
    uint16_t i;

    if ((g_fab == NULL) || (w == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((g_fab->ready == 0u) || (w->open == 0u)) {
        return FF_NOT_READY;
    }
    for (i = 0u; i < g_fab->map_count; i++) {
        if (g_fab->map[i].id == w->id) {
            if ((g_fab->map[i].base != w->base) || (g_fab->map[i].size != w->size)) {
                return FF_INVALID_ARGUMENT; /* handle does not match the catalogue */
            }
            *entry = &g_fab->map[i];
            return FF_SUCCESS;
        }
    }
    return FF_INVALID_ARGUMENT; /* unknown device id */
}

static FF_Status_t fabric_init(void)
{
    if (g_fab == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    g_fab->ready = 1u;
    return FF_SUCCESS;
}

static FF_Status_t fabric_open(FF_FabricDevId_t id, FF_FabricWindow_t *out)
{
    uint16_t i;

    if ((g_fab == NULL) || (out == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_fab->ready == 0u) {
        return FF_NOT_READY;
    }
    for (i = 0u; i < g_fab->map_count; i++) {
        if (g_fab->map[i].id == id) {
            out->id = g_fab->map[i].id;
            out->base = g_fab->map[i].base;
            out->size = g_fab->map[i].size;
            out->open = 1u;
            return FF_SUCCESS;
        }
    }
    g_fab->misses++;
    return FF_NOT_FOUND;
}

static FF_Status_t fabric_close(FF_FabricWindow_t *w)
{
    if (w == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (w->open == 0u) {
        return FF_NOT_READY;
    }
    w->open = 0u;
    w->id = 0u;
    w->base = 0u;
    w->size = 0u;
    return FF_SUCCESS;
}

static FF_Status_t fabric_read32(const FF_FabricWindow_t *w, uint32_t off, uint32_t *val)
{
    const FF_FabricMap_t *e = NULL;
    FF_Status_t rc;

    if (g_fab == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    rc = fabric_resolve(w, &e);
    if (rc != FF_SUCCESS) {
        g_fab->rejected++;
        return rc;
    }
    if ((val == NULL) || ((off & 3u) != 0u)) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    if (fabric_range(e->size, off, 4u) != FF_SUCCESS) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    *val = g_fab->bus.read32(g_fab->bus.ctx, e->base + off);
    return FF_SUCCESS;
}

static FF_Status_t fabric_write32(const FF_FabricWindow_t *w, uint32_t off, uint32_t val)
{
    const FF_FabricMap_t *e = NULL;
    FF_Status_t rc;

    if (g_fab == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    rc = fabric_resolve(w, &e);
    if (rc != FF_SUCCESS) {
        g_fab->rejected++;
        return rc;
    }
    if ((off & 3u) != 0u) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    if (fabric_range(e->size, off, 4u) != FF_SUCCESS) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    g_fab->bus.write32(g_fab->bus.ctx, e->base + off, val);
    return FF_SUCCESS;
}

static FF_Status_t fabric_read8(const FF_FabricWindow_t *w, uint32_t off, uint8_t *val)
{
    const FF_FabricMap_t *e = NULL;
    FF_Status_t rc;

    if (g_fab == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    rc = fabric_resolve(w, &e);
    if (rc != FF_SUCCESS) {
        g_fab->rejected++;
        return rc;
    }
    if (val == NULL) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    if (fabric_range(e->size, off, 1u) != FF_SUCCESS) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    *val = g_fab->bus.read8(g_fab->bus.ctx, e->base + off);
    return FF_SUCCESS;
}

static FF_Status_t fabric_write8(const FF_FabricWindow_t *w, uint32_t off, uint8_t val)
{
    const FF_FabricMap_t *e = NULL;
    FF_Status_t rc;

    if (g_fab == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    rc = fabric_resolve(w, &e);
    if (rc != FF_SUCCESS) {
        g_fab->rejected++;
        return rc;
    }
    if (fabric_range(e->size, off, 1u) != FF_SUCCESS) {
        g_fab->rejected++;
        return FF_INVALID_ARGUMENT;
    }
    g_fab->bus.write8(g_fab->bus.ctx, e->base + off, val);
    return FF_SUCCESS;
}

/* Interrupt wiring of a fabric line into the PLIC is board work (GAP-A5): the
 * window is validated so the caller learns the state, and the binding itself is
 * reported as not implemented. */
static FF_Status_t fabric_attach_irq(const FF_FabricWindow_t *w, uint8_t vector,
                                     FF_FabricIrqCb_t cb, void *ctx)
{
    const FF_FabricMap_t *e = NULL;
    FF_Status_t rc;

    (void)vector;
    (void)cb;
    (void)ctx;
    rc = fabric_resolve(w, &e);
    if (rc != FF_SUCCESS) {
        if (g_fab != NULL) {
            g_fab->rejected++;
        }
        return rc;
    }
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t FF_FabricMpfsSetup(FF_FabricMpfs_t *f, const FF_FabricBus_t *bus,
                               const FF_FabricMap_t *map, uint16_t map_count)
{
    uint16_t i;
    uint16_t j;

    if ((f == NULL) || (bus == NULL) || (map == NULL) || (map_count == 0u) ||
        (bus->read32 == NULL) || (bus->write32 == NULL) || (bus->read8 == NULL) ||
        (bus->write8 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (map_count > FF_FABRIC_MAP_MAX) {
        return FF_BAD_SIZE;
    }
    /* Validate the whole catalogue before binding anything, so a rejected
     * Setup never leaves a half-configured backend behind. */
    for (i = 0u; i < map_count; i++) {
        if (map[i].size == 0u) {
            return FF_BAD_SIZE;
        }
        if (map[i].size > (0xFFFFFFFFu - map[i].base)) {
            return FF_BAD_SIZE; /* base + size wraps the fabric address space */
        }
        for (j = (uint16_t)(i + 1u); j < map_count; j++) {
            if (map[j].id == map[i].id) {
                return FF_ALREADY_EXISTS;
            }
        }
    }
    f->bus = *bus;
    f->map = map;
    f->map_count = map_count;
    f->misses = 0u;
    f->rejected = 0u;
    f->ready = 0u;
    f->fabric.name = "mpfs-fabric";
    f->fabric.init = fabric_init;
    f->fabric.open = fabric_open;
    f->fabric.close = fabric_close;
    f->fabric.read32 = fabric_read32;
    f->fabric.write32 = fabric_write32;
    f->fabric.read8 = fabric_read8;
    f->fabric.write8 = fabric_write8;
    f->fabric.attach_irq = fabric_attach_irq;
    g_fab = f;
    return FF_SUCCESS;
}

FF_Status_t FF_FabricMpfsScratchSelfTest(FF_FabricMpfs_t *f, const FF_FabricWindow_t *w,
                                         uint32_t off, uint32_t mask, uint32_t *value)
{
    uint32_t orig = 0u;
    uint32_t probe = 0u;
    uint32_t back = 0u;
    uint8_t probe_ok = 0u;
    FF_Status_t rc;

    if ((f == NULL) || (g_fab != f) || (value == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    rc = g_fab->fabric.read32(w, off, &orig);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    /* Flip the masked bits (read-modify-write): bits outside `mask` - read-only
     * or side-effecting ones - are carried over untouched. */
    probe = (orig & ~mask) | (~orig & mask);
    rc = g_fab->fabric.write32(w, off, probe);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    rc = g_fab->fabric.read32(w, off, &back);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    probe_ok = (uint8_t)(back == probe);
    /* Always attempt restoration after a successful probe write, including
     * when read-back mismatches; self-test must not knowingly leave the device
     * in its test pattern. */
    rc = g_fab->fabric.write32(w, off, orig);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    rc = g_fab->fabric.read32(w, off, &back);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if ((back != orig) || (probe_ok == 0u)) {
        return FF_ERROR; /* the original value could not be restored */
    }
    *value = orig;
    return FF_SUCCESS;
}
