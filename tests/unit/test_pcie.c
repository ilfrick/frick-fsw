/*
 * frick-fsw - MPFS PCIe driver unit tests (milestone M4/M11)
 *
 * Exercises the ECAM configuration-space read/write logic on the host with a
 * mock 32-bit register file that mirrors the documented MPFS PCIe register
 * map and models the config-space access path. The mock reuses the driver's
 * register-map constants; it is a behavioural fake for the register/ECAM
 * logic, not an independent oracle. The layout is cross-checked at the
 * proddable register offsets by the Renode `PCI.MPFS_PCIe` model in the
 * emulated functional test (milestone M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/pcie/mpfs/ff_pcie_mpfs.h"
#include <string.h>

typedef struct
{
    uint32_t regs[1024]; /* 4 KB config space (32-bit words) */
    uint32_t ctrl_reg;   /* CTRL register at 0x1000 */
    uint32_t reads;
    uint32_t writes;
} MockPcie_t;

static MockPcie_t g_mock;
static FF_PcieMpfs_t g_drv;
static FF_PcieBus_t g_bus;

static void mock_reset(MockPcie_t *m)
{
    (void)memset(m, 0, sizeof(*m));
    /* Renode PCI.MPFS_PCIe model reset values (reference). */
    m->regs[FF_PCIE_REG_VENDOR_DEVICE_ID >> 2] = 0x12345678u; /* VendorID=0x5678, DeviceID=0x1234 */
    m->regs[FF_PCIE_REG_COMMAND_STATUS >> 2] = 0x00100000u;   /* Status=0x0010, Command=0x0000 */
    m->regs[FF_PCIE_REG_CLASS_REVISION >> 2] = 0x06000001u; /* Class=0x060000 (bridge), Rev=0x01 */
    m->ctrl_reg = 0u;
}

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    MockPcie_t *m = (MockPcie_t *)ctx;
    uint32_t idx = off >> 2;
    uint32_t byte_off = off & 3u;

    if (off >= 0x1000u) {
        /* Controller registers - only aligned access for simplicity. */
        if (byte_off != 0u) {
            return 0u;
        }
        if (off == FF_PCIE_REG_CTRL) {
            return m->ctrl_reg;
        }
        return 0u;
    }
    if (idx < (uint32_t)(sizeof(m->regs) / sizeof(m->regs[0]))) {
        m->reads++;
        if (byte_off == 0u) {
            /* Aligned read - return the word directly. */
            return m->regs[idx];
        }
        /* Unaligned read - combine two adjacent words. */
        if (idx + 1u < (uint32_t)(sizeof(m->regs) / sizeof(m->regs[0]))) {
            uint32_t low = m->regs[idx];
            uint32_t high = m->regs[idx + 1u];
            uint32_t shift = byte_off * 8u;
            return (low >> shift) | (high << (32u - shift));
        }
        return m->regs[idx] >> (byte_off * 8u);
    }
    return 0u;
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    MockPcie_t *m = (MockPcie_t *)ctx;
    uint32_t idx = off >> 2;

    if ((off & 3u) != 0u) {
        return;
    }
    if (off >= 0x1000u) {
        if (off == FF_PCIE_REG_CTRL) {
            m->ctrl_reg = val;
        }
        m->writes++;
        return;
    }
    if (idx < (uint32_t)(sizeof(m->regs) / sizeof(m->regs[0]))) {
        m->regs[idx] = val;
        m->writes++;
    }
}

static void mock_bus_init(void)
{
    g_bus.ctx = &g_mock;
    g_bus.read32 = mock_read32;
    g_bus.write32 = mock_write32;
}

static void mock_bind(uint8_t rc)
{
    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&g_drv, &g_bus, 0u, rc) == FF_SUCCESS);
}

static void test_setup_and_init(void)
{
    static FF_PcieMpfs_t drv;
    FF_PcieBus_t bad;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_PcieMpfsSetup(NULL, &g_bus, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, NULL, 0u, 0u) == FF_INVALID_ARGUMENT);
    bad.ctx = &g_mock;
    bad.read32 = NULL;
    bad.write32 = mock_write32;
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, &bad, 0u, 0u) == FF_INVALID_ARGUMENT);
    bad.read32 = mock_read32;
    bad.write32 = NULL;
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, &bad, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, &g_bus, 0u, 2u) == FF_INVALID_ARGUMENT); /* rc >= 2 */
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, &g_bus, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.pcie.name != NULL);
    FF_TEST_ASSERT(strcmp(drv.pcie.name, "mpfs-pcie") == 0);
    FF_TEST_ASSERT(drv.pcie.init != NULL);
    FF_TEST_ASSERT(drv.pcie.read_config != NULL);
    FF_TEST_ASSERT(drv.pcie.write_config != NULL);
    FF_TEST_ASSERT(drv.pcie.msi_enable != NULL);
    FF_TEST_ASSERT(drv.pcie.msi_disable != NULL);

    /* Not ready before init. */
    FF_TEST_ASSERT(drv.pcie.read_config(0u, 0u, 0u, 0u, NULL) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.pcie.write_config(0u, 0u, 0u, 0u, 0u) == FF_NOT_READY);

    /* init argument validation. */
    FF_TEST_ASSERT(drv.pcie.init(1u) == FF_INVALID_ARGUMENT); /* wrong rc */
    FF_TEST_ASSERT(drv.pcie.init(0u) == FF_SUCCESS);
    /* Controller enabled (ENABLE | ECAM_EN). */
    FF_TEST_ASSERT(
        (mock_read32(&g_mock, FF_PCIE_REG_CTRL) & (FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN)) ==
        (FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN));
    FF_TEST_ASSERT(drv.ready == 1u);
}

static void test_read_config(void)
{
    uint32_t val;

    mock_bind(0u);
    FF_TEST_ASSERT(g_drv.pcie.init(0u) == FF_SUCCESS);

    /* Read VendorID/DeviceID at offset 0x00. */
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_VENDOR_ID, &val) == FF_SUCCESS);
    FF_TEST_ASSERT(val == 0x12345678u);

    /* Read Command/Status register at offset 0x04 (32-bit aligned). */
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, &val) == FF_SUCCESS);
    FF_TEST_ASSERT((val & 0xFFFFu) == 0x0000u); /* Command = 0 */
    FF_TEST_ASSERT((val >> 16) == 0x0010u);     /* Status = 0x0010 */

    /* Read Class/Revision register at offset 0x08 (32-bit aligned, contains Class[23:8] |
     * Rev[7:0]). */
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_CLASS_CODE - 1u, &val) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(val == 0x06000001u); /* Class=0x060000, Rev=0x01 */

    /* Also check mock directly at aligned offset. */
    uint32_t direct = mock_read32(&g_mock, FF_PCIE_REG_CLASS_REVISION);
    FF_TEST_ASSERT(direct == 0x06000001u);

    /* Invalid bus/dev/fn (only 0,0,0 supported). */
    FF_TEST_ASSERT(g_drv.pcie.read_config(1u, 0u, 0u, 0u, &val) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 1u, 0u, 0u, &val) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 0u, 1u, 0u, &val) == FF_INVALID_ARGUMENT);

    /* NULL out_val. */
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 0u, 0u, 0u, NULL) == FF_INVALID_ARGUMENT);
}

static void test_write_config(void)
{
    uint32_t val;

    mock_bind(0u);
    FF_TEST_ASSERT(g_drv.pcie.init(0u) == FF_SUCCESS);

    /* Write Command register (offset 0x04) - 32-bit write updates the full word.
     * The mock does not model read-only Status bits; a full-word write overwrites both. */
    FF_TEST_ASSERT(g_drv.pcie.write_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, 0x00000007u) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.pcie.read_config(0u, 0u, 0u, FF_PCIE_CFG_COMMAND, &val) == FF_SUCCESS);
    FF_TEST_ASSERT(val == 0x00000007u); /* Full word written */

    /* Invalid bus/dev/fn. */
    FF_TEST_ASSERT(g_drv.pcie.write_config(1u, 0u, 0u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.pcie.write_config(0u, 1u, 0u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.pcie.write_config(0u, 0u, 1u, 0u, 0u) == FF_INVALID_ARGUMENT);
}

static void test_msi_stub(void)
{
    mock_bind(0u);
    FF_TEST_ASSERT(g_drv.pcie.init(0u) == FF_SUCCESS);

    /* MSI enable/disable are stubbed (GAP-A5). */
    FF_TEST_ASSERT(g_drv.pcie.msi_enable(0u, 0u, 0u, 0u) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(g_drv.pcie.msi_disable(0u, 0u, 0u) == FF_NOT_IMPLEMENTED);
}

static void test_preinit_access(void)
{
    static FF_PcieMpfs_t drv;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, &g_bus, 0u, 0u) == FF_SUCCESS);
    /* Bound but not init'd: config access reports FF_NOT_READY. */
    FF_TEST_ASSERT(drv.pcie.read_config(0u, 0u, 0u, 0u, &(uint32_t){0}) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.pcie.write_config(0u, 0u, 0u, 0u, 0u) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.pcie.init(0u) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.pcie.read_config(0u, 0u, 0u, 0u, &(uint32_t){0}) == FF_SUCCESS);
}

static void test_self_test(void)
{
    static FF_PcieMpfs_t drv;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_PcieMpfsSetup(&drv, &g_bus, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PcieMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PcieMpfsSelfTest(&drv) == FF_SUCCESS);

    /* Verify self-test left controller enabled and Command restored. */
    FF_TEST_ASSERT(
        (mock_read32(&g_mock, FF_PCIE_REG_CTRL) & (FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN)) ==
        (FF_PCIE_CTRL_ENABLE | FF_PCIE_CTRL_ECAM_EN));
    /* Command should be restored to original (0). */
    FF_TEST_ASSERT((mock_read32(&g_mock, FF_PCIE_REG_COMMAND_STATUS) & 0xFFFFu) == 0u);

    /* Self-test on unbound instance is rejected. */
    {
        FF_PcieMpfs_t other;
        (void)memset(&other, 0, sizeof(other));
        FF_TEST_ASSERT(FF_PcieMpfsSelfTest(&other) == FF_INVALID_ARGUMENT);
    }
}

FF_TEST_MAIN(test_setup_and_init, test_read_config, test_write_config, test_msi_stub,
             test_preinit_access, test_self_test)