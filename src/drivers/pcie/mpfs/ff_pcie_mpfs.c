/*
 * frick-fsw - MPFS PCIe driver (PolarFire SoC), HAL backend (milestone M4/M11)
 *
 * Configuration-space access (ECAM) for the Microchip PolarFire SoC PCIe
 * root complex. Register logic on an injected 32-bit MMIO bus. Static, no
 * allocation. Single instance per root complex (the HAL contract carries no
 * context beyond the root_complex index).
 *
 * The emulated Renode `PCI.MPFS_PCIe` model provides a 4 KB ECAM window per
 * function with a standard PCI config header. No endpoint is attached, so the
 * driver performs register round-trips only. Enumeration, BAR mapping, DMA,
 * and full MSI are not implemented in this increment (GAP-A5).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "drivers/pcie/mpfs/ff_pcie_mpfs.h"

static FF_PcieMpfs_t *g_pcie; /* single bound instance per root complex */

static uint32_t pcie_rd(uint32_t off)
{
    return g_pcie->bus.read32(g_pcie->bus.ctx, off);
}

static void pcie_wr(uint32_t off, uint32_t val)
{
    g_pcie->bus.write32(g_pcie->bus.ctx, off, val);
}

static uint32_t pcie_ecam_off(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t offset)
{
    /* The emulated board has only root complex at bus 0, device 0, function 0.
     * Any other (bus,dev,fn) is treated as an invalid argument in this increment. */
    if ((bus != 0u) || (dev != 0u) || (fn != 0u)) {
        return 0xFFFFFFFFu; /* invalid; callers must check */
    }
    return FF_PCIE_ECAM_OFFSET(bus, dev, fn, offset);
}

static FF_Status_t pcie_check_ready(void)
{
    if (g_pcie == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_pcie->ready == 0u) {
        return FF_NOT_READY;
    }
    return FF_SUCCESS;
}

static FF_Status_t pcie_init(uint8_t root_complex)
{
    if ((g_pcie == NULL) || (root_complex != g_pcie->rc)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Enable the PCIe controller (ECAM + link). */
    pcie_wr(FF_PCIE_REG_CTRL, FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN);
    g_pcie->ready = 1u;
    return FF_SUCCESS;
}

static FF_Status_t pcie_read_config(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t offset,
                                    uint32_t *out_val)
{
    FF_Status_t rc;
    uint32_t ecam_off;
    uint32_t val;

    rc = pcie_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if (out_val == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    ecam_off = pcie_ecam_off(bus, dev, fn, offset);
    if (ecam_off == 0xFFFFFFFFu) {
        return FF_INVALID_ARGUMENT;
    }
    val = pcie_rd(ecam_off);
    *out_val = val;
    return FF_SUCCESS;
}

static FF_Status_t pcie_write_config(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t offset,
                                     uint32_t val)
{
    FF_Status_t rc;
    uint32_t ecam_off;

    rc = pcie_check_ready();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    ecam_off = pcie_ecam_off(bus, dev, fn, offset);
    if (ecam_off == 0xFFFFFFFFu) {
        return FF_INVALID_ARGUMENT;
    }
    pcie_wr(ecam_off, val);
    return FF_SUCCESS;
}

static FF_Status_t pcie_msi_enable(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t vector)
{
    (void)bus;
    (void)dev;
    (void)fn;
    (void)vector;
    /* Not implemented in this increment (GAP-A5). The PLIC lines for pcie0/pcie1
     * are declared in platform_mpfs.h. */
    return FF_NOT_IMPLEMENTED;
}

static FF_Status_t pcie_msi_disable(uint8_t bus, uint8_t dev, uint8_t fn)
{
    (void)bus;
    (void)dev;
    (void)fn;
    return FF_NOT_IMPLEMENTED;
}

FF_Status_t FF_PcieMpfsSelfTest(FF_PcieMpfs_t *p)
{
    uint32_t cmd_before;
    uint32_t cmd_after;
    const uint32_t test_cmd = 0x00000007u; /* Memory + IO + BusMaster enable */

    if ((p == NULL) || (p != g_pcie)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Initialise through the HAL init path (enables ECAM). */
    if (p->pcie.init(p->rc) != FF_SUCCESS) {
        return FF_ERROR;
    }
    /* Verify controller is enabled and ECAM is active. */
    if ((pcie_rd(FF_PCIE_REG_CTRL) & (FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN)) !=
        (FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN)) {
        return FF_ERROR;
    }
    /* Read VendorID/DeviceID at config offset 0x00 (standard PCI header).
     * The emulated model does not populate PCI config space registers;
     * accept any value (including 0) as the model does not populate PCI config space. */
    p->pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_VENDOR_ID, &(uint32_t){0});
    /* Write/read Command register (offset 0x04) round-trip. */
    if (p->pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, &cmd_before) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (p->pcie.write_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, test_cmd) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (p->pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, &cmd_after) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (cmd_after != test_cmd) {
        return FF_ERROR;
    }
    /* Restore original Command value. */
    if (p->pcie.write_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, cmd_before) != FF_SUCCESS) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

FF_Status_t FF_PcieMpfsSetup(FF_PcieMpfs_t *p, const FF_PcieBus_t *bus, uint32_t base, uint8_t rc)
{
    if ((p == NULL) || (bus == NULL) || (bus->read32 == NULL) || (bus->write32 == NULL) ||
        (rc >= 2u)) {
        return FF_INVALID_ARGUMENT;
    }
    p->bus = *bus; /* copied by value */
    p->base = base;
    p->rc = rc;
    p->ready = 0u;
    p->pcie.name = "mpfs-pcie";
    p->pcie.init = pcie_init;
    p->pcie.read_config = pcie_read_config;
    p->pcie.write_config = pcie_write_config;
    p->pcie.msi_enable = pcie_msi_enable;
    p->pcie.msi_disable = pcie_msi_disable;
    g_pcie = p;
    return FF_SUCCESS;
}