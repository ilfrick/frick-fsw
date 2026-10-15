/*
 * frick-fsw - HAL contract: generic fabric-device (FIC/AXI) access
 *
 * Milestone M4 (S4c). The PolarFire SoC keeps many peripherals in the MSS, but
 * resources that live in the FPGA fabric - the CAN-FD controller, SpaceWire,
 * the CoreTSN switch - are reached through the Fabric Interface Controller
 * (FIC), which exposes one AXI window per device. This contract is the seam
 * between "open a device window" and "access registers inside it", so a fabric
 * driver never hard-codes a physical address and a board supplies its own
 * catalogue (device id -> AXI base + register span) without modifying the
 * framework (SRS-REQ-067). The fabric CAN-FD backend (SRS-REQ-062), the
 * SpaceWire backend and the fabric CoreTSN path (SRS-REQ-068) are built on it.
 *
 * Access rules, enforced by the backends and covered by the host unit test and
 * by the Lean model (verification/lean/Frick/Fabric.lean):
 *   - a window is opened by device id and closed explicitly; without init() and
 *     an open window no MMIO access is issued;
 *   - offsets are window-relative and checked against the catalogue span with
 *     an overflow-free range test, so an access can never leave its device
 *     window, whatever the offset;
 *   - 32-bit accesses are word aligned (no misaligned AXI access);
 *   - no dynamic allocation at any time (norm N10, SRS-REQ-015).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_FABRIC_H
#define FF_HAL_FABRIC_H

#include "common/ff_types.h"

/* Logical fabric device identifier: the board catalogue key (not a physical
 * address). Well-known ids are given below; a board may use its own. */
typedef uint32_t FF_FabricDevId_t;

/* A window onto one fabric device. Produced by open(), consumed by the accessors
 * and cleared by close(); treat it as an opaque handle (the fields describe the
 * catalogue entry the backend resolved). */
typedef struct
{
    FF_FabricDevId_t id; /* catalogue key                                  */
    uint32_t base;       /* AXI base address of the device                 */
    uint32_t size;       /* register span in bytes (bounded window)        */
    uint8_t open;        /* 1 while the window is open, else 0             */
} FF_FabricWindow_t;

/* Fabric interrupt line callback (deferred to a task by the receiver, see
 * docs/architecture/hardware-abstraction.md). */
typedef void (*FF_FabricIrqCb_t)(uint8_t vector, void *ctx);

typedef struct FF_Fabric FF_Fabric_t;
struct FF_Fabric
{
    const char *name;
    FF_Status_t (*init)(void);
    /* Resolve a catalogue entry and open a window on it. FF_NOT_FOUND for an
     * unknown device id; FF_INVALID_ARGUMENT when out is NULL. */
    FF_Status_t (*open)(FF_FabricDevId_t id, FF_FabricWindow_t *out);
    /* Close a window (idempotent). FF_NOT_READY when it is not open. */
    FF_Status_t (*close)(FF_FabricWindow_t *w);
    /* Window-relative register access. `off` is relative to the device base
     * and is range checked against the catalogue span; `val` must be non-NULL.
     * 32-bit access requires a 4-byte aligned offset. */
    FF_Status_t (*read32)(const FF_FabricWindow_t *w, uint32_t off, uint32_t *val);
    FF_Status_t (*write32)(const FF_FabricWindow_t *w, uint32_t off, uint32_t val);
    FF_Status_t (*read8)(const FF_FabricWindow_t *w, uint32_t off, uint8_t *val);
    FF_Status_t (*write8)(const FF_FabricWindow_t *w, uint32_t off, uint8_t val);
    /* Bind an interrupt line to a window. Not provided by every backend. */
    FF_Status_t (*attach_irq)(const FF_FabricWindow_t *w, uint8_t vector, FF_FabricIrqCb_t cb,
                              void *ctx);
};

/* Well-known catalogue keys (logical; a board may add its own). */
#define FF_FABRIC_DEV_CANFD 0x01u   /* fabric CAN-FD controller (SRS-REQ-062) */
#define FF_FABRIC_DEV_SPW 0x02u     /* fabric SpaceWire                       */
#define FF_FABRIC_DEV_CORETSN 0x03u /* fabric CoreTSN switch (802.1CB)        */

/* The overflow-free range test the access rules imply is a backend detail (it
 * is private to the fabric backends); the required behaviour - "no access
 * leaves the device window, whatever the offset" - is what callers depend on. */

#endif /* FF_HAL_FABRIC_H */
