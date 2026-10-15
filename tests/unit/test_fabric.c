/*
 * frick-fsw - Fabric (FIC/AXI) device backend unit tests (milestone M4/S4c)
 *
 * Exercises the HAL fabric contract's window/range enforcement (SRS-REQ-067) on
 * the host against an independent mock MMIO region. The mock records every
 * address the driver touches, so the tests can assert the two properties that
 * matter: an accepted access stays inside its device window, and a refused
 * access issues **no** MMIO at all. Nominal and out-of-bound cases carry
 * explicit expected results (norm N22).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/fabric/mpfs/ff_fabric_mpfs.h"
#include <string.h>

#define MOCK_BASE 0x40000000u /* fabric window of the first catalogue device */
#define MOCK_SPAN 0x40u

static uint8_t g_mem[MOCK_SPAN];
static uint32_t g_min_addr;
static uint32_t g_max_addr;
static int g_bus_calls;
static int g_bus_out; /* accesses that would have left the mock region */
static uint32_t g_ro_addr;
static int g_ro_enabled;

static int mock_fits(uint32_t addr, uint32_t width)
{
    if (addr < MOCK_BASE) {
        return 0;
    }
    if ((addr - MOCK_BASE) >= MOCK_SPAN) {
        return 0;
    }
    if ((MOCK_SPAN - (addr - MOCK_BASE)) < width) {
        return 0;
    }
    return 1;
}

static void mock_note(uint32_t addr, uint32_t width)
{
    g_bus_calls++;
    if (!mock_fits(addr, width)) {
        g_bus_out++;
        return;
    }
    if ((g_bus_calls == 1) || (addr < g_min_addr)) {
        g_min_addr = addr;
    }
    if ((addr + width) > g_max_addr) {
        g_max_addr = addr + width;
    }
}

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    uint32_t v = 0u;
    (void)ctx;
    mock_note(off, 4u);
    if (mock_fits(off, 4u)) {
        memcpy(&v, &g_mem[off - MOCK_BASE], 4u);
    }
    return v;
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    mock_note(off, 4u);
    if (!mock_fits(off, 4u)) {
        return;
    }
    if (g_ro_enabled && (off == g_ro_addr)) {
        return; /* read-only register: the write is silently dropped */
    }
    memcpy(&g_mem[off - MOCK_BASE], &val, 4u);
}

static uint8_t mock_read8(void *ctx, uint32_t off)
{
    uint8_t v = 0u;
    (void)ctx;
    mock_note(off, 1u);
    if (mock_fits(off, 1u)) {
        v = g_mem[off - MOCK_BASE];
    }
    return v;
}

static void mock_write8(void *ctx, uint32_t off, uint8_t val)
{
    (void)ctx;
    mock_note(off, 1u);
    if (!mock_fits(off, 1u)) {
        return;
    }
    g_mem[off - MOCK_BASE] = val;
}

static void reset_mock(void)
{
    memset(g_mem, 0, sizeof(g_mem));
    g_min_addr = 0xFFFFFFFFu;
    g_max_addr = 0u;
    g_bus_calls = 0;
    g_bus_out = 0;
    g_ro_enabled = 0;
    g_ro_addr = 0u;
}

/* Catalogue used by the tests: two devices, 16-byte register spans. The mock
 * region is wider than each span, so an access that escaped a device window
 * would still land in the mock and be visible as a bus call outside the span. */
static const FF_FabricMap_t g_map[] = {
    {FF_FABRIC_DEV_CANFD, MOCK_BASE, 0x10u},
    {FF_FABRIC_DEV_SPW, MOCK_BASE + 0x20u, 0x10u},
};

static FF_FabricMpfs_t g_fab;
static FF_FabricMpfs_t g_other;

static const FF_FabricMap_t g_map_bad_duplicate[] = {
    {FF_FABRIC_DEV_CANFD, MOCK_BASE, 0x10u},
    {FF_FABRIC_DEV_CANFD, MOCK_BASE + 0x20u, 0x10u},
};
static const FF_FabricMap_t g_map_bad_zero[] = {
    {FF_FABRIC_DEV_CANFD, MOCK_BASE, 0u},
};
static const FF_FabricMap_t g_map_bad_wrap[] = {
    {FF_FABRIC_DEV_CANFD, 0xFFFFFFF0u, 0x20u},
};

static void test_fabric_setup(void)
{
    FF_FabricBus_t bus = {NULL, mock_read32, mock_write32, mock_read8, mock_write8};
    FF_FabricBus_t bad = {NULL, mock_read32, mock_write32, mock_read8, NULL};

    reset_mock();
    FF_TEST_ASSERT(FF_FabricMpfsSetup(NULL, &bus, g_map, 2u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, NULL, g_map, 2u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bad, g_map, 2u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, NULL, 2u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map, 0u) == FF_INVALID_ARGUMENT);

    /* Out-of-bound catalogues must be rejected before anything is bound. */
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map, (uint16_t)(FF_FABRIC_MAP_MAX + 1u)) ==
                   FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map_bad_zero, 1u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map_bad_wrap, 1u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map_bad_duplicate, 2u) == FF_ALREADY_EXISTS);

    /* Nominal bind completes the contract. */
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.name != NULL);
    FF_TEST_ASSERT(g_fab.fabric.init != NULL);
    FF_TEST_ASSERT(g_fab.fabric.open != NULL);
    FF_TEST_ASSERT(g_fab.fabric.attach_irq != NULL);
    FF_TEST_ASSERT(g_fab.map_count == 2u);
    FF_TEST_ASSERT(g_fab.ready == 0u);

    /* A second instance can be bound as well (single *bound* instance, but the
     * descriptor itself is plain data). */
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_other, &bus, g_map, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fab, &bus, g_map, 2u) == FF_SUCCESS); /* rebind */
}

static void test_fabric_open_close(void)
{
    FF_FabricWindow_t w = {0u, 0u, 0u, 0u};
    FF_FabricWindow_t w2;
    uint32_t v = 0u;

    /* Before init(): no window can be opened and no access is issued. */
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &w) == FF_NOT_READY);
    w.id = FF_FABRIC_DEV_CANFD;
    w.base = MOCK_BASE;
    w.size = 0x10u;
    w.open = 1u;
    FF_TEST_ASSERT(g_fab.ready == 0u);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0u, &v) == FF_NOT_READY);
    FF_TEST_ASSERT(g_bus_calls == 0); /* refused on state: no MMIO at all */

    FF_TEST_ASSERT(g_fab.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.ready == 1u);

    /* Unknown device id: reported and counted, nothing opened. */
    FF_TEST_ASSERT(g_fab.fabric.open(0x7Fu, &w2) == FF_NOT_FOUND);
    FF_TEST_ASSERT(g_fab.misses == 1u);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, NULL) == FF_INVALID_ARGUMENT);

    /* Nominal open fills the handle from the catalogue. */
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &w) == FF_SUCCESS);
    FF_TEST_ASSERT(w.id == FF_FABRIC_DEV_CANFD);
    FF_TEST_ASSERT(w.base == MOCK_BASE);
    FF_TEST_ASSERT(w.size == 0x10u);
    FF_TEST_ASSERT(w.open == 1u);

    /* The IRQ line is validated then reported as board work. */
    FF_TEST_ASSERT(g_fab.fabric.attach_irq(&w, 1u, NULL, NULL) == FF_NOT_IMPLEMENTED);

    /* Close is explicit and cleared state is visible. */
    FF_TEST_ASSERT(g_fab.fabric.close(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.close(&w) == FF_SUCCESS);
    FF_TEST_ASSERT(w.open == 0u);
    FF_TEST_ASSERT(w.id == 0u);
    FF_TEST_ASSERT(w.base == 0u);
    FF_TEST_ASSERT(w.size == 0u);
    FF_TEST_ASSERT(g_fab.fabric.close(&w) == FF_NOT_READY);
    FF_TEST_ASSERT(g_fab.fabric.attach_irq(&w, 1u, NULL, NULL) == FF_NOT_READY);

    /* Access after close is refused without MMIO. */
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0u, &v) == FF_NOT_READY);
    FF_TEST_ASSERT(g_bus_calls == 0);
}

static void test_fabric_access(void)
{
    FF_FabricWindow_t w;
    uint32_t v = 0u;
    int calls;
    uint32_t max_before;

    reset_mock();
    FF_TEST_ASSERT(g_fab.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &w) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_SPW, &w) == FF_SUCCESS);
    FF_TEST_ASSERT(w.base == MOCK_BASE + 0x20u);

    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &w) == FF_SUCCESS);

    /* Word round trip at the window edges (0 and size-4). */
    FF_TEST_ASSERT(g_fab.fabric.write32(&w, 0u, 0xA5A5A5A5u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0xA5A5A5A5u);
    FF_TEST_ASSERT(g_fab.fabric.write32(&w, 0x0Cu, 0x00000011u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0x0Cu, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0x11u);

    /* Byte access, including the last addressable byte. */
    FF_TEST_ASSERT(g_fab.fabric.write8(&w, 0x0Fu, 0x5Au) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.read8(&w, 0x0Fu, (uint8_t *)&v) == FF_SUCCESS);
    FF_TEST_ASSERT((v & 0xFFu) == 0x5Au);

    /* Accepted accesses stayed inside the device window. */
    FF_TEST_ASSERT(g_bus_out == 0);
    FF_TEST_ASSERT(g_min_addr >= MOCK_BASE);
    FF_TEST_ASSERT(g_max_addr <= (MOCK_BASE + 0x10u));

    /* Out-of-bound offsets: refused, and no MMIO is issued for them. */
    calls = g_bus_calls;
    max_before = g_max_addr;
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0x10u, &v) == FF_INVALID_ARGUMENT); /* first byte past */
    FF_TEST_ASSERT(g_fab.fabric.write32(&w, 0x10u, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0x0Du, &v) == FF_INVALID_ARGUMENT); /* 0x0D+4 > size  */
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0x0Eu, &v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0xFFFFFFFCu, &v) == FF_INVALID_ARGUMENT); /* wraps */
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0xFFFFFFFFu, &v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read8(&w, 0x10u, (uint8_t *)&v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read8(&w, 0xFFFFFFFFu, (uint8_t *)&v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.write8(&w, 0x10u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_bus_calls == calls);
    FF_TEST_ASSERT(g_max_addr == max_before);
    FF_TEST_ASSERT(g_bus_out == 0);

    /* Misalignment and NULL out-pointers are refused as well. */
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0x02u, &v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.write32(&w, 0x02u, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read8(&w, 0u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_bus_calls == calls);
    FF_TEST_ASSERT(g_bus_out == 0);
    FF_TEST_ASSERT(g_fab.rejected >= 10u);
}

static void test_fabric_forged_window(void)
{
    FF_FabricWindow_t w;
    FF_FabricWindow_t forged;
    uint32_t v = 0u;
    int calls;

    reset_mock();
    FF_TEST_ASSERT(g_fab.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &w) == FF_SUCCESS);

    /* A handle whose span/base/id no longer matches the catalogue is refused:
     * the caller cannot widen its own window. */
    forged = w;
    forged.size = MOCK_SPAN; /* widened to cover the whole mock region */
    calls = g_bus_calls;
    FF_TEST_ASSERT(g_fab.fabric.read32(&forged, 0x20u, &v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.write32(&forged, 0x20u, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_fab.fabric.read8(&forged, 0x20u, (uint8_t *)&v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_bus_calls == calls);

    forged = w;
    forged.base = MOCK_BASE + 4u; /* moved */
    FF_TEST_ASSERT(g_fab.fabric.read32(&forged, 0u, &v) == FF_INVALID_ARGUMENT);

    forged = w;
    forged.id = FF_FABRIC_DEV_CORETSN; /* not in this catalogue */
    FF_TEST_ASSERT(g_fab.fabric.read32(&forged, 0u, &v) == FF_INVALID_ARGUMENT);

    /* An unopened handle is a state error, not a range error. */
    forged = w;
    forged.open = 0u;
    FF_TEST_ASSERT(g_fab.fabric.read32(&forged, 0u, &v) == FF_NOT_READY);

    /* The genuine handle still works. */
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(g_bus_out == 0);
}

static void test_fabric_self_test(void)
{
    FF_FabricWindow_t w;
    uint32_t v = 0u;

    reset_mock();
    FF_TEST_ASSERT(g_fab.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &w) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.write32(&w, 0u, 0x0F0F0F0Fu) == FF_SUCCESS);

    /* Wrong instance and NULL out-pointer are refused. */
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_other, &w, 0u, 0xFFFFFFFFu, &v) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 0u, 0xFFFFFFFFu, NULL) ==
                   FF_INVALID_ARGUMENT);

    /* Nominal: the register round-trips, the original value is restored and
     * reported back. */
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 0u, 0xFFFFFFFFu, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0x0F0F0F0Fu);
    FF_TEST_ASSERT(g_fab.fabric.read32(&w, 0u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0x0F0F0F0Fu);

    /* A masked probe leaves the unmasked bits untouched. */
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 0u, 0x000000FFu, &v) == FF_SUCCESS);
    FF_TEST_ASSERT((v & 0xFFFFFF00u) == 0x0F0F0F00u);

    /* mask == 0 changes nothing and is accepted. */
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 0u, 0u, &v) == FF_SUCCESS);

    /* Out-of-bound offset propagates the accessor status. */
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 0x10u, 1u, &v) == FF_INVALID_ARGUMENT);

    /* A read-only register cannot be probed: the self-test reports FF_ERROR and
     * does not claim success. */
    g_ro_enabled = 1;
    g_ro_addr = MOCK_BASE + 4u;
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 4u, 0xFFFFFFFFu, &v) == FF_ERROR);
    g_ro_enabled = 0;

    /* A closed window propagates FF_NOT_READY. */
    FF_TEST_ASSERT(g_fab.fabric.close(&w) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricMpfsScratchSelfTest(&g_fab, &w, 0u, 1u, &v) == FF_NOT_READY);
    FF_TEST_ASSERT(g_bus_out == 0);
}

static void test_fabric_multi_device(void)
{
    FF_FabricWindow_t a;
    FF_FabricWindow_t b;
    uint32_t v = 0u;

    reset_mock();
    FF_TEST_ASSERT(g_fab.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_CANFD, &a) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.open(FF_FABRIC_DEV_SPW, &b) == FF_SUCCESS);

    /* The two windows are disjoint: writing one does not disturb the other. */
    FF_TEST_ASSERT(g_fab.fabric.write32(&a, 0u, 0x11111111u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.write32(&b, 0u, 0x22222222u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fab.fabric.read32(&a, 0u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0x11111111u);
    FF_TEST_ASSERT(g_fab.fabric.read32(&b, 0u, &v) == FF_SUCCESS);
    FF_TEST_ASSERT(v == 0x22222222u);

    /* The device window of `b` ends at MOCK_BASE + 0x30 and the mock region at
     * MOCK_BASE + 0x40: the last 16 bytes are not owned by any device, and
     * neither window can reach them. */
    FF_TEST_ASSERT(g_fab.fabric.read32(&b, 0x10u, &v) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_max_addr <= (MOCK_BASE + 0x30u));
    FF_TEST_ASSERT(g_bus_out == 0);
}

FF_TEST_MAIN(test_fabric_setup, test_fabric_open_close, test_fabric_access,
             test_fabric_forged_window, test_fabric_self_test, test_fabric_multi_device)
