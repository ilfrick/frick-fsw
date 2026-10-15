/*
 * frick-fsw - Fabric (FIC/AXI) device backend (PolarFire SoC), HAL backend (M4/S4c)
 *
 * Reference implementation of the HAL fabric contract (FF_Fabric_t): resolve a
 * board catalogue entry (device id -> AXI base + register span), open a bounded
 * window on it and perform word/byte register access through an injected MMIO
 * bus (FF_FabricBus_t), so window/range and register access are host-tested
 * through mocks. The available Renode model does not include a fabric CAN-FD
 * peripheral (NC-S4c-01/GAP-A5).
 *
 * MMIO semantics: the bus receives an **absolute fabric address** (catalogue
 * base + window-relative offset). A platform hook adds its own physical base
 * from `ctx` (the FIC address space is board specific: the fixed MSS memory map
 * in config/platform/platform_mpfs.h contains no fabric-attached peripheral),
 * while a host mock maps the same address onto a byte array. Catalogue values
 * for the real FIC windows are board work (GAP-A5).
 *
 * Known limitations of this reference backend (all bounded, allocation-free):
 *   - single instance bound statically (the HAL contract carries no context);
 *   - the catalogue is not copied (Setup stores the pointer): the descriptor
 *     array must outlive the binding;
 *   - no interrupt delivery: attach_irq returns FF_NOT_IMPLEMENTED (wiring
 *     fabric lines into the PLIC is board work, GAP-A5);
 *   - rejected accesses are counted for telemetry but never trap: the caller
 *     receives the status and no MMIO is issued;
 *   - the window handle is caller-owned data, not a capability: it is
 *     re-validated against the catalogue on every access, so a handle whose
 *     id/base/span does not match the catalogue is refused.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_FABRIC_MPFS_H
#define FF_FABRIC_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_fabric.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host).
 * The address is the absolute fabric address = catalogue base + window offset. */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
    uint8_t (*read8)(void *ctx, uint32_t off);
    void (*write8)(void *ctx, uint32_t off, uint8_t val);
} FF_FabricBus_t;

/* Largest catalogue the start-up validation accepts (the FIC exposes a small
 * fixed number of windows; the cap makes the duplicate-id scan bounded). */
#define FF_FABRIC_MAP_MAX 16u

/* One catalogue entry: a fabric device and the register span it owns. `size`
 * must be non-zero and `base + size` must not wrap the 32-bit fabric address
 * space (both checked by Setup). */
typedef struct
{
    uint32_t id;
    uint32_t base;
    uint32_t size;
} FF_FabricMap_t;

typedef struct
{
    FF_Fabric_t fabric;        /* public HAL interface (must be first)     */
    FF_FabricBus_t bus;        /* injected MMIO access (copied by value)   */
    const FF_FabricMap_t *map; /* catalogue (not copied; must outlive)     */
    uint16_t map_count;
    uint16_t misses;   /* open() calls with an unknown device id           */
    uint16_t rejected; /* accessors refused (state/alignment/range)        */
    uint8_t ready;     /* init() completed                                 */
} FF_FabricMpfs_t;

/* Bind the instance and complete the FF_Fabric_t vtable. Returns
 * FF_INVALID_ARGUMENT if f, bus (or any of its four hooks), map is NULL, if
 * map_count is 0, or if the catalogue holds a zero-sized span, a span that
 * wraps the 32-bit address space, or a duplicate device id (FF_ALREADY_EXISTS). */
FF_Status_t FF_FabricMpfsSetup(FF_FabricMpfs_t *f, const FF_FabricBus_t *bus,
                               const FF_FabricMap_t *map, uint16_t map_count);

/* MMIO self-test (host unit test): read the register
 * at window-relative `off`, flip the bits selected by `mask`, verify the
 * read-back, then restore the original value and verify that too. Transcript of
 * the value is returned in `*value`. `mask` must select writable bits of a
 * register without side effects on the device (an unused GPIO output bit is the
 * usual choice); on target, run it before normal operation.
 *
 * Returns FF_INVALID_ARGUMENT for a wrong instance/NULL out-pointer, the
 * accessor status when an access is refused, and FF_ERROR when the register
 * does not honour the probe or the restore. */
FF_Status_t FF_FabricMpfsScratchSelfTest(FF_FabricMpfs_t *f, const FF_FabricWindow_t *w,
                                         uint32_t off, uint32_t mask, uint32_t *value);

#endif /* FF_FABRIC_MPFS_H */
