/*
 * frick-fsw - MPFS PCIe driver (PolarFire SoC), HAL backend (milestone M4/M11)
 *
 * Implements the HAL PCIe contract (FF_Pcie_t) for the Microchip PolarFire
 * SoC PCIe root complex (PLIC 149 for pcie0 @ 0x53004000, PLIC 150 for
 * pcie1 @ 0x53005000). The Renode `PCI.MPFS_PCIe` model presents an
 * ECAM-compatible configuration space (4 KB per function) plus MPFS-specific
 * registers. No physical endpoint is attached in the emulated board (fabric
 * path), so the driver performs **register round-trips only** (like the
 * SPI/I2C/TSN checks) and does not implement enumeration, BAR mapping, or DMA
 * in this increment.
 *
 * MMIO access is injected (FF_PcieBus_t, 32-bit addressed) so the register
 * logic is testable on the host (mock bus) and the registers are exercised on
 * the emulated SoC in the Renode functional test (milestone M11).
 *
 * Register map (ECAM-compatible 4 KB configuration space per function + MPFS
 * controller registers). This layout is a **reference mapping**, cross-checked
 * at the proddable register offsets against the Renode `PCI.MPFS_PCIe` model:
 *   - ECAM config space per function: 0x000-0xFFF (standard PCI config header)
 *   - Controller registers (MPFS-specific): at offset 0x1000+ within the 4 KB
 *     window (vendor-specific capability space). The Renode model provides
 *     read/write access to the full 4 KB; the standard PCI config header at
 *     0x000 (VendorID/DeviceID/Command/Status/ClassCode/RevisionID/...) is
 *     the primary self-test surface.
 *
 * Known limitations of this reference backend (all bounded, allocation-free):
 *   - Configuration-space access only: no enumeration, no BAR mapping, no DMA
 *     (SRS-REQ-031/032 contract subset; full PCIe subsystem is GAP-A5).
 *   - Single root-complex instance bound statically (the HAL contract carries
 *     no per-instance context); a second root complex needs a separate binary
 *     or contract extension.
 *   - ECAM address calculation is hard-coded for bus 0, device 0, function 0
 *     (the emulated root complex); multi-function/multi-device is not handled.
 *   - Interrupt/MSI path is stubbed (returns FF_NOT_IMPLEMENTED); the PLIC
 *     lines (149/150) are declared in platform_mpfs.h for completeness.
 *   - The MMIO self-test is a bring-up check with configuration side effects
 *     (it runs init() and reads/writes config space): run it before normal
 *     operation and not against concurrent traffic.
 *   - The injected bus descriptor is copied by value (FF_PcieMpfs_t.bus).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_PCIE_MPFS_H
#define FF_PCIE_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_pcie.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host).
 * 32-bit: the MPFS PCIe config space and controller registers are 32-bit. */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
} FF_PcieBus_t;

/* MPFS PCIe register map (ECAM config space + MPFS controller registers).
 * The config space is 4 KB per function (standard PCI layout at 0x000-0x0FF
 * for the common header, 0x100-0xFFF for capabilities). The MPFS-specific
 * controller registers start at 0x1000 within the 4 KB window. */
#define FF_PCIE_REG_VENDOR_DEVICE_ID 0x000u /* Vendor ID (15:0) | Device ID (31:16) */
#define FF_PCIE_REG_COMMAND_STATUS 0x004u   /* Command (15:0) | Status (31:16) */
#define FF_PCIE_REG_CLASS_REVISION 0x008u   /* Class Code (23:8) | Revision ID (7:0) */
#define FF_PCIE_REG_BIST_HEADER                                                                    \
    0x00Cu /* BIST (7:0) | Header Type (15:8) | Latency Timer (23:16) | Cache Line (31:24) */
#define FF_PCIE_REG_BAR0 0x010u
#define FF_PCIE_REG_BAR1 0x014u
#define FF_PCIE_REG_BAR2 0x018u
#define FF_PCIE_REG_BAR3 0x01Cu
#define FF_PCIE_REG_BAR4 0x020u
#define FF_PCIE_REG_BAR5 0x024u
#define FF_PCIE_REG_SUBSYSTEM 0x02Cu /* Subsystem Vendor ID (15:0) | Subsystem ID (31:16) */
#define FF_PCIE_REG_CAP_PTR 0x034u   /* Capabilities Pointer (7:0) */
#define FF_PCIE_REG_INTERRUPT 0x03Cu /* Interrupt Pin (15:8) | Interrupt Line (7:0) */

/* MPFS-specific controller registers (offset 0x1000+ within the 4 KB window).
 * These are reference offsets from the Renode model and MPFS TRM. */
#define FF_PCIE_REG_CTRL 0x1000u      /* Controller control */
#define FF_PCIE_REG_STATUS 0x1004u    /* Controller status */
#define FF_PCIE_REG_ECAM_BASE 0x1008u /* ECAM base address (for multi-root) */
#define FF_PCIE_REG_MSI_CTRL 0x1010u  /* MSI control */
#define FF_PCIE_REG_MSI_ADDR 0x1014u  /* MSI address */
#define FF_PCIE_REG_MSI_DATA 0x1018u  /* MSI data */

/* CTRL register bits (reference). */
#define FF_PCIE_CTRL_ENABLE (1u << 0)
#define FF_PCIE_CTRL_ECAM_EN (1u << 1)
#define FF_PCIE_CTRL_LINK_UP (1u << 2) /* read-only: link status */

/* Standard PCI config header fields (byte offsets within config space). */
#define FF_PCIE_CFG_VENDOR_ID 0x00u
#define FF_PCIE_CFG_DEVICE_ID 0x02u
#define FF_PCIE_CFG_COMMAND 0x04u
#define FF_PCIE_CFG_STATUS 0x06u
#define FF_PCIE_CFG_REVISION_ID 0x08u
#define FF_PCIE_CFG_CLASS_CODE 0x09u
#define FF_PCIE_CFG_CACHE_LINE 0x0Cu
#define FF_PCIE_CFG_LATENCY_TIMER 0x0Du
#define FF_PCIE_CFG_HEADER_TYPE 0x0Eu
#define FF_PCIE_CFG_BIST 0x0Fu
#define FF_PCIE_CFG_BAR0 0x10u
#define FF_PCIE_CFG_SUBSYSTEM_VENDOR 0x2Cu
#define FF_PCIE_CFG_SUBSYSTEM_ID 0x2Eu
#define FF_PCIE_CFG_CAP_PTR 0x34u
#define FF_PCIE_CFG_INT_LINE 0x3Cu
#define FF_PCIE_CFG_INT_PIN 0x3Du

/* ECAM address calculation for the MPFS PCIe root complex.
 * Base + (bus << 20) | (device << 15) | (function << 12) | offset.
 * The emulated board has only root complex at bus 0, device 0, function 0. */
#define FF_PCIE_ECAM_OFFSET(bus, dev, fn, off)                                                     \
    ((((uint32_t)(bus)) << 20) | (((uint32_t)(dev)) << 15) | (((uint32_t)(fn)) << 12) |            \
     ((uint32_t)(off)))

/* Serial clock target used by the MMIO self-test. */
#define FF_PCIE_SELFTEST_RC 0u

typedef struct
{
    FF_Pcie_t pcie;   /* public HAL interface (must be first) */
    FF_PcieBus_t bus; /* injected MMIO bus (copied by value) */
    uint32_t base;    /* root complex base address */
    uint8_t rc;       /* root complex index (0 or 1) */
    uint8_t ready;    /* init() has completed */
} FF_PcieMpfs_t;

/* Bind the instance and complete the FF_Pcie_t vtable. Returns FF_INVALID_ARGUMENT
 * if p, bus, or either bus hook (read32/write32) is NULL, or rc >= 2. */
FF_Status_t FF_PcieMpfsSetup(FF_PcieMpfs_t *p, const FF_PcieBus_t *bus, uint32_t base, uint8_t rc);

/* MMIO self-test: initialise the root complex, read the VendorID/DeviceID at
 * config offset 0x00, write and read back the Command register (0x04), verify
 * the round-trip. Used by the host unit test and the Renode functional check
 * (pcie0 at FF_PCIE0_BASE).
 *
 * Scope note: the emulated PCI.MPFS_PCIe has no endpoint attached, so no
 * enumeration or BAR assignment occurs; the self-test exercises the ECAM
 * configuration-space read/write path only. The full PCIe subsystem (link
 * training, enumeration, DMA, MSI) is GAP-A5.
 *
 * This is a bring-up check with configuration side effects: it runs init() and
 * modifies the Command register. Run it before normal operation. */
FF_Status_t FF_PcieMpfsSelfTest(FF_PcieMpfs_t *p);

#endif /* FF_PCIE_MPFS_H */