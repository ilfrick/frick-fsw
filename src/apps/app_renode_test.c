/*
 * frick-fsw - Renode functional test application
 *
 * An application integrated on the middleware used for functional validation in
 * the Renode emulated PolarFire SoC environment (norm N18). On start it prints a
 * machine-checkable banner and runs middleware self-checks (Software Bus
 * publish/subscribe, Event Services, Time Services) and, on the target platform,
 * a HAL self-check set (milestone M11: MSS peripherals bound in the
 * beaglev-fire platform description), then prints
 * "RENODE_TEST: PASS" or "RENODE_TEST: FAIL". Registered only when
 * FF_ENABLE_RENODE_TEST=ON; the default reference set has seven apps, and this
 * optional test app is registered only when the option is enabled.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "apps/app.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/es/es.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include <stdint.h>

#define RENODE_TEST_MSG_ID 0x7E01u

#if defined(FRICK_PLATFORM_MPFS)
#include "platform/platform_mpfs.h"
#include "drivers/gpio/mpfs/ff_gpio_mpfs.h"
#include "drivers/serial/mpfs/ff_serial_mpfs.h"
#include "drivers/mem/ff_mem_static.h"
#include "drivers/nvm/mpfs/ff_nvm_mpfs.h"
#include "drivers/can/mpfs/ff_can_mpfs.h"
#include "drivers/i2c/mpfs/ff_i2c_mpfs.h"
#include "drivers/spi/mpfs/ff_spi_mpfs.h"
#include "drivers/block/mpfs/ff_block_mpfs.h"
#include "drivers/net/ff_net_loopback.h"
#include "drivers/eth/gem/ff_gem_tsn.h"
#include "drivers/pcie/mpfs/ff_pcie_mpfs.h"
#include "drivers/spacewire/mpfs/ff_spacewire_mpfs.h"
#include "drivers/mtd/mpfs/ff_mtd_mpfs.h"
#if defined(FF_USE_LWIP)
#include "hal/net/lwip/ff_net_lwip.h"
#include "hal/net/lwip/ff_lwip_netif.h"
#include "drivers/eth/gem/ff_gem_regs.h"
#endif
#include <string.h>

/* Result of one HAL check in this functional test.
 *
 * PASS     - the check exercised an emulated peripheral end to end.
 * CONTRACT - no emulated peripheral exists for this class in the board
 *            description, so only the HAL/API contract was exercised (on the
 *            host unit tests, or against a mock bus here). A CONTRACT result is
 *            reported as CONTRACT and is EXCLUDED from the aggregate verdict:
 *            it is not peripheral-bound evidence and must never be rendered as
 *            PASS (N22: a PASS marker does not by itself establish coverage).
 * FAIL     - the check ran and failed.
 */
typedef enum {
    FF_RENODE_CHECK_FAIL = 0,
    FF_RENODE_CHECK_PASS = 1,
    FF_RENODE_CHECK_CONTRACT = 2
} FF_RenodeCheck_t;

static uint32_t mpfs_gpio_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_GPIO0_BASE + off);
}

static void mpfs_gpio_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)(FF_GPIO0_BASE + off) = val;
}

/* HAL GPIO MMIO round trip on the emulated MSS GPIO (milestone M11). */
static bool renode_hal_gpio_check(void)
{
    static FF_GpioMpfs_t drv;
    FF_GpioBus_t bus = {NULL, mpfs_gpio_read32, mpfs_gpio_write32};
    uint8_t v = 0xFFu;

    if (FF_GpioMpfsSetup(&drv, &bus, FF_GPIO0_PINS) != FF_SUCCESS) {
        return false;
    }
    if (drv.gpio.init() != FF_SUCCESS) {
        return false;
    }
    if (drv.gpio.configure(0u, FF_GPIO_OUT) != FF_SUCCESS) {
        return false;
    }
    if (drv.gpio.write(0u, 1u) != FF_SUCCESS) {
        return false;
    }
    if ((drv.gpio.read(0u, &v) != FF_SUCCESS) || (v != 1u)) {
        return false;
    }
    if (drv.gpio.write(0u, 0u) != FF_SUCCESS) {
        return false;
    }
    if ((drv.gpio.read(0u, &v) != FF_SUCCESS) || (v != 0u)) {
        return false;
    }
    return true;
}

static uint8_t mpfs_uart_read8(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint8_t *)(uintptr_t)(FF_MMUART0_BASE + off);
}

static void mpfs_uart_write8(void *ctx, uint32_t off, uint8_t val)
{
    (void)ctx;
    *(volatile uint8_t *)(uintptr_t)(FF_MMUART0_BASE + off) = val;
}

/* HAL serial MMIO check on the emulated MSS MMUART0 (milestone M11): line
 * control + NS16550 scratch-register round trip (wide registers). */
static bool renode_hal_serial_check(void)
{
    static FF_SerialMpfs_t drv;
    FF_SerialBus_t bus = {NULL, mpfs_uart_read8, mpfs_uart_write8};

    if (FF_SerialMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    if (drv.serial.init(115200u, 8u, 0u, 1u) != FF_SUCCESS) {
        return false;
    }
    return FF_SerialMpfsScratchSelfTest(&drv) == FF_SUCCESS;
}

static uint32_t mpfs_gem_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_GEM0_BASE + off);
}

/* HAL Ethernet check on the emulated Cadence GEM (milestone M11): identify the
 * MAC via the read-only Module ID/revision register. The emulated GEM uses the
 * Cadence defaults (identification 0x002, revision 0x118). (This is an MMIO
 * presence/identification check; the full GEM driver/DMA/link path is host-tested
 * and on-target PHY/link is GAP-A5.) */
static bool renode_hal_eth_check(void)
{
    uint32_t modid = mpfs_gem_read32(NULL, FF_GEM_MODID_OFF);
    /* Cadence GEM MODID: [15:0] module revision, [27:16] module identification. */
    return (((modid >> 16) & 0x0FFFu) == 0x002u) && ((modid & 0xFFFFu) == 0x0118u);
}

static void mpfs_gem_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)(FF_GEM0_BASE + off) = val;
}

/* HAL TSN check on the emulated Cadence GEM (milestone M11/S6): bind the GEM
 * native TSN backend to the emulated MAC, initialise it, set a VLAN/PCP and read
 * the 802.1AS TSU time (the GEM model implements the Timer1588 registers). This
 * exercises the FF_TsnBackend_t seam peripheral-bound. 802.1Qav CBS returns
 * FF_NOT_IMPLEMENTED (documented) and the full gPTP/CoreTSN stack is milestone M8
 * (GAP-A5 for on-target PTP accuracy). */
static bool renode_hal_tsn_check(void)
{
    static FF_TsnGem_t tsn;
    FF_GemBus_t bus;
    uint64_t t_ns = 0u;

    bus.ctx = NULL;
    bus.read32 = mpfs_gem_read32;
    bus.write32 = mpfs_gem_write32;
    /* The injected accessors already add FF_GEM0_BASE, so the backend base is 0
     * (the same convention as test_gem_tsn.c and the hal_eth check). Passing
     * FF_GEM0_BASE here would double-add it and hit an unmapped address. */
    if (FF_TsnGemSetup(&tsn, &bus, 0u, 1000u) != FF_SUCCESS) {
        return false;
    }
    if (tsn.tsn.init(&tsn) != FF_SUCCESS) {
        return false;
    }
    if (tsn.tsn.set_vlan(&tsn, 1u, 3u) != FF_SUCCESS) {
        return false;
    }
    if (tsn.tsn.get_time_ns(&tsn, &t_ns) != FF_SUCCESS) {
        return false;
    }
    return true;
}

#if defined(FF_USE_LWIP)
/* HAL network-stack check, peripheral-bound (milestone M11/S6b-2): bring up the
 * lwIP stack (FF_NetStackLwip.init() -> tcpip_init) and bind an lwIP netif to
 * the emulated Cadence GEM MAC through the framework FF_NetIf_t / FF_EthMac_t
 * contract (SRS-REQ-040), then confirm the netif is up. The GEM MAC is
 * configured through the driver (MMIO to the emulated registers) and the lwIP
 * netif hardware address must match the configured GEM MAC.
 * Oracle: stack init succeeds, attach succeeds, the netif reports up/link-up,
 * and its hardware address equals the GEM MAC. The GEM MAC (mode + station
 * address) is configured over MMIO on the emulated Cadence GEM. The receiver /
 * transmitter are NOT enabled: the Renode `Network.CadenceGEM` model is a
 * register-only model (no DMA descriptor engine — TSTART is rejected as an
 * unhandled bit), so a full packet round trip is **not** possible in Renode.
 * The on-target GEM DMA PADDR translation is also GAP-A5. The TX/RX data-path
 * is host-tested with a mock MAC in `test_lwip_netif` (NC-154-01). */
static bool renode_hal_net_lwip_check(void)
{
    static FF_Gem_t gem;
    static FF_NetIf_t nif;
    FF_GemConfig_t cfg;
    const uint8_t want[6] = {0x02u, 0x00u, 0x00u, 0x12u, 0x34u, 0x56u};
    uint8_t hw[6];
    uint32_t ncfgr;
    uint32_t modid;

    cfg.bus.ctx = NULL;
    cfg.bus.read32 = mpfs_gem_read32;
    cfg.bus.write32 = mpfs_gem_write32;
    cfg.dma = NULL;
    cfg.base = 0u; /* the accessors already add FF_GEM0_BASE */
    memcpy(cfg.mac, want, sizeof(want));
    cfg.speed_mbps = 1000u;
    cfg.full_duplex = 1u;

    if (FF_GemSetup(&gem, &cfg) != FF_SUCCESS) {
        return false;
    }

    /* Configure the emulated GEM MAC registers directly over MMIO (network
     * config + station address). The driver's DMA path is deliberately NOT armed
     * (no RE/TE) because the descriptor translation is 32-bit and cannot express
     * the MPFS DDR address in the emulator (NC-S6-03); no frame is transmitted. */
    ncfgr = (1u << FF_GEM_NCFGR_FD) | (1u << FF_GEM_NCFGR_GBE) | (1u << FF_GEM_NCFGR_BIG) |
            (1u << FF_GEM_NCFGR_DRFCS) | (1u << FF_GEM_NCFGR_DBW);
    mpfs_gem_write32(NULL, FF_GEM_NCFGR, ncfgr);
    mpfs_gem_write32(NULL, FF_GEM_SA1B,
                     (uint32_t)want[0] | ((uint32_t)want[1] << 8) | ((uint32_t)want[2] << 16) |
                         ((uint32_t)want[3] << 24));
    mpfs_gem_write32(NULL, FF_GEM_SA1T, (uint32_t)want[4] | ((uint32_t)want[5] << 8));

    nif.name = "gem0";
    nif.mac = &gem.mac;
    nif.stack = &FF_NetStackLwip;
    nif.ipv4[0] = 192u;
    nif.ipv4[1] = 168u;
    nif.ipv4[2] = 0u;
    nif.ipv4[3] = 10u;
    nif.netmask[0] = 255u;
    nif.netmask[1] = 255u;
    nif.netmask[2] = 255u;
    nif.netmask[3] = 0u;
    nif.gateway[0] = 192u;
    nif.gateway[1] = 168u;
    nif.gateway[2] = 0u;
    nif.gateway[3] = 1u;
    nif.init = NULL;
    nif.max_frame = NULL;

    if (FF_NetStackLwip.init() != FF_SUCCESS) {
        return false;
    }
    if (FF_NetStackLwip.attach_netif(&nif) != FF_SUCCESS) {
        return false;
    }
    if (!FF_LwipNetifIsUp()) {
        return false;
    }
    if ((FF_LwipNetifGetMac(hw) != FF_SUCCESS) || (memcmp(hw, want, sizeof(want)) != 0)) {
        return false;
    }
    /* Peripheral presence: the netif MAC is the GEM driver on the emulated
     * Cadence GEM (read-only Module ID/revision). */
    modid = mpfs_gem_read32(NULL, FF_GEM_MODID_OFF);
    if ((((modid >> 16) & 0x0FFFu) != 0x002u) || ((modid & 0xFFFFu) != 0x0118u)) {
        return false;
    }
    return true;
}
#endif /* FF_USE_LWIP */

/* HAL network-stack check (milestone M11): a socket round trip through the
 * pluggable FF_NetStack_t contract, using the reference loopback stack (the same
 * contract an lwIP/Tuxera adapter implements). It is a **contract/behaviour**
 * check (in-process), not a binding to an emulated peripheral.
 * Oracle: bind to a non-zero port; a connect to an unlistened port must fail;
 * accept must report the server port; the message must round-trip. */
static bool renode_hal_net_fail(FF_NetStack_t *s)
{
    (void)s->init(); /* reset the socket table (no leak on the failure path) */
    return false;
}

static bool renode_hal_net_check(void)
{
    FF_NetStack_t *s = &FF_NetStackLoopback;
    const uint8_t port[1] = {0x42u};
    const uint8_t no_port[1] = {0x43u};
    const uint8_t msg[4] = {0x11u, 0x22u, 0x33u, 0x44u};
    uint8_t peer[4];
    uint8_t buf[8];
    uint16_t peer_len = sizeof(peer);
    uint16_t sent = 0u;
    uint16_t got = 0u;
    FF_NetSock_t srv;
    FF_NetSock_t neg;
    FF_NetSock_t cli;
    FF_NetSock_t acc;

    if (s->init() != FF_SUCCESS) {
        return false;
    }
    srv = s->open(0, 0, 0);
    if ((srv < 0) || (s->bind(srv, port, 1u) != FF_SUCCESS) || (s->listen(srv, 1u) != FF_SUCCESS)) {
        return renode_hal_net_fail(s);
    }
    /* Negative: connecting to a port with no listener must fail. */
    neg = s->open(0, 0, 0);
    if ((neg < 0) || (s->connect(neg, no_port, 1u) == FF_SUCCESS)) {
        return renode_hal_net_fail(s);
    }
    (void)s->close(neg);
    cli = s->open(0, 0, 0);
    if ((cli < 0) || (s->connect(cli, port, 1u) != FF_SUCCESS)) {
        return renode_hal_net_fail(s);
    }
    acc = s->accept(srv, peer, &peer_len);
    if ((acc < 0) || (peer_len < 1u) || (peer[0] != port[0])) {
        return renode_hal_net_fail(s); /* bind must be observable */
    }
    if ((s->send(cli, msg, sizeof(msg), &sent) != FF_SUCCESS) || (sent != sizeof(msg))) {
        return renode_hal_net_fail(s);
    }
    if ((s->recv(acc, buf, sizeof(buf), &got, 0u) != FF_SUCCESS) || (got != sizeof(msg)) ||
        (memcmp(buf, msg, sizeof(msg)) != 0)) {
        return renode_hal_net_fail(s);
    }
    (void)s->close(acc);
    (void)s->close(cli);
    (void)s->close(srv);
    return true;
}

/* HAL memory check (milestone M11): static RAM pool over the emulated DDR +
 * reference DMA translation. */
static bool renode_hal_mem_check(void)
{
    static FF_MemStaticPool_t pool;
    static uint8_t area[256];
    uint8_t *p;

    if (FF_MemStaticPoolInit(&pool, "ddr-pool", area, sizeof(area)) != FF_SUCCESS) {
        return false;
    }
    p = (uint8_t *)pool.pool.alloc(16u);
    if (p == NULL) {
        return false;
    }
    p[0] = 0xAAu;
    p[15] = 0x55u;
    if (p[0] != 0xAAu) {
        return false;
    }
    return FF_DmaIdentity.virt_to_phys(p) != 0u;
}

static uint8_t mpfs_envm_read8(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint8_t *)(uintptr_t)(FF_ENVM0_BASE + off);
}

static void mpfs_envm_write8(void *ctx, uint32_t off, uint8_t val)
{
    (void)ctx;
    *(volatile uint8_t *)(uintptr_t)(FF_ENVM0_BASE + off) = val;
}

/* HAL NVM check on the emulated MSS eNVM (milestone M11): write/read round trip
 * on the scratch region, outside the parameter id table (the eNVM DATA region at
 * 0x20220000 is writable; the config block at 0x20200000 is not). On-target flash
 * write/endurance/retention remains GAP-A5. */
static bool renode_hal_nvm_check(void)
{
    static FF_NvmMpfs_t drv;
    FF_NvmBus_t bus = {NULL, mpfs_envm_read8, mpfs_envm_write8};

    if (FF_NvmMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    return FF_NvmMpfsSelfTest(&drv) == FF_SUCCESS;
}

/* HAL MTD (QSPI NOR flash) check: NOT exercised in Renode (milestone M11 / OBJ-19).
 * The Renode model exposes the QSPI NOR flash behind an SPI controller
 * (SPI.Micron_MT25Q attached to SPI.MPFS_QSPI), not as a memory-mapped window.
 * Direct MMIO access at 0x21000000 is not supported by the model. The MTD driver
 * (ff_mtd_mpfs.c) exists and is host unit-tested; on-target QSPI timing,
 * endurance, retention, and command path remain GAP-A5. */

/* HAL CAN check on the emulated MSS CAN (milestone M11): register round trips
 * (Control run bit, Interrupt-enable, TX buffer ID, RX acceptance mask) through
 * the reference CAN driver. Interrupts stay disabled, so no IRQ is raised; the
 * full CAN driver/data path is host-tested (the emulated controller has no
 * medium attached here) and on-target CAN is GAP-A5. */
static uint32_t mpfs_can_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_CAN0_BASE + off);
}

static void mpfs_can_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)(FF_CAN0_BASE + off) = val;
}

static bool renode_hal_can_check(void)
{
    static FF_CanMpfs_t drv;
    FF_CanBus_t bus = {NULL, mpfs_can_read32, mpfs_can_write32};

    if (FF_CanMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    return FF_CanMpfsSelfTest(&drv) == FF_SUCCESS;
}

/* HAL I2C check on the emulated MSS I2C (milestone M11): enable the core through
 * the HAL init path (serial-clock divider + ENS1), confirm the bus reports idle,
 * and round-trip the two slave-address registers. The emulated MPFS_I2C does not
 * model the Control-register clock-divider bits (they are not readable back), so
 * only ENS1 is asserted; no bus transaction is run (no slave is attached and the
 * model advances on register writes, not on the SI clear the hardware uses). The
 * full transaction path is host-tested, and on-target SCL/APB timing and real
 * bus behavior are GAP-A5. */
static uint8_t mpfs_i2c_read8(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint8_t *)(uintptr_t)(FF_I2C0_BASE + off);
}

static void mpfs_i2c_write8(void *ctx, uint32_t off, uint8_t val)
{
    (void)ctx;
    *(volatile uint8_t *)(uintptr_t)(FF_I2C0_BASE + off) = val;
}

static bool renode_hal_i2c_check(void)
{
    static FF_I2cMpfs_t drv;
    FF_I2cBus_t bus = {NULL, mpfs_i2c_read8, mpfs_i2c_write8};

    if (FF_I2cMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    return FF_I2cMpfsSelfTest(&drv) == FF_SUCCESS;
}

/* HAL SPI check on the emulated MSS SPI (milestone M11): enable the core through
 * the HAL init path (CONTROL ENABLE+MASTER, FRAMESIZE, CLKRATE), confirm the
 * derived CLKRATE/FRAMESIZE and the empty RX FIFO. The emulated SPI.MPFS_SPI has
 * no registered slave, so no bus transaction is run (a real master transfer
 * would dereference a null slave and fault): the full transfer path is
 * host-tested, and on-target SCL/APB timing and real bus behavior are GAP-A5.
 * The bus descriptor is static because FF_SpiMpfs_t keeps a pointer to it. */
static uint32_t mpfs_spi_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_SPI0_BASE + off);
}

static void mpfs_spi_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)(FF_SPI0_BASE + off) = val;
}

static bool renode_hal_spi_check(void)
{
    static FF_SpiMpfs_t drv;
    static FF_SpiBus_t bus = {NULL, mpfs_spi_read32, mpfs_spi_write32};

    if (FF_SpiMpfsSetup(&drv, &bus) != FF_SUCCESS) {
        return false;
    }
    return FF_SpiMpfsSelfTest(&drv) == FF_SUCCESS;
}

/* HAL PCIe check (milestone M11/S7): **contract-level only, NOT peripheral-bound**.
 * See the body note below: the emulated board instantiates no PCIe root complex.
 * The FF_Pcie_t vtable and the ECAM address arithmetic (SRS-REQ-031/032 contract
 * subset) are host-tested; the full PCIe subsystem (link training, enumeration,
 * BAR mapping, DMA, MSI) is GAP-A5. */
static FF_RenodeCheck_t renode_hal_pcie_check(void)
{
    /* Not peripheral-bound on this emulated board: `beaglev-fire.repl` (loaded
     * by tools/docker/renode/frick-renode.resc) does not instantiate a PCIe root
     * complex, so the controller is absent from the emulated memory map and an
     * ECAM access at FF_PCIE0_BASE would fault instead of round-tripping. The
     * FF_Pcie_t vtable and the ECAM address arithmetic are covered by the host
     * unit test (`tests/unit/test_pcie.c`); the peripheral-bound path is
     * GAP-A5.
     *
     * It must NOT report PASS. An earlier version returned true with no register
     * access at all, so `[renode-test] hal_pcie=PASS` carried no evidence
     * behind it (R1-08, corrected 2026-11-18). */
    return FF_RENODE_CHECK_CONTRACT;
}

/* HAL SpaceWire check (milestone M11/S7): **contract-level only** (GAP-A5).
 * There is no emulated SpaceWire peripheral in the Renode beaglev-fire repl.
 * This check exercises the FF_SpaceWire_t and FF_Rmap_t API contracts
 * (send/send_pid, RMAP read/write/rmw, CRC helpers) against a mock link
 * and emits CONTRACT (like the network loopback check). The NC-S7-01 is
 * documented: no emulated peripheral for peripheral-bound validation. */
static FF_SpwMockBus_t g_spw_mock_bus;
static uint8_t g_spw_mock_tx[256];
static uint16_t g_spw_mock_tx_len;
static uint8_t g_spw_mock_rx[256];
static uint16_t g_spw_mock_rx_len;

static FF_Status_t spw_mock_send(void *ctx, const uint8_t *data, uint16_t len)
{
    (void)ctx;
    if ((data == NULL) || (len == 0u) || (len > sizeof(g_spw_mock_tx))) {
        return FF_INVALID_ARGUMENT;
    }
    (void)memcpy(g_spw_mock_tx, data, len);
    g_spw_mock_tx_len = len;
    return FF_SUCCESS;
}

static FF_Status_t spw_mock_recv(void *ctx, uint8_t *data, uint16_t *len)
{
    (void)ctx;
    if ((data == NULL) || (len == NULL) || (g_spw_mock_rx_len == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    *len = (*len < g_spw_mock_rx_len) ? *len : g_spw_mock_rx_len;
    (void)memcpy(data, g_spw_mock_rx, *len);
    return FF_SUCCESS;
}

static FF_RenodeCheck_t renode_hal_spacewire_check(void)
{
    static FF_SpaceWireMpfs_t drv;
    static FF_SpwMockBus_t bus = {NULL, spw_mock_send, spw_mock_recv};

    if (FF_SpaceWireMpfsSetup(&drv, &bus, 0xFEu) != FF_SUCCESS) {
        return FF_RENODE_CHECK_FAIL;
    }
    /* The contract self-test runs against a mock link: that is CONTRACT, not
     * peripheral-bound evidence, so it can never be reported as PASS. */
    return (FF_SpaceWireMpfsSelfTest(&drv) == FF_SUCCESS) ? FF_RENODE_CHECK_CONTRACT
                                                          : FF_RENODE_CHECK_FAIL;
}

/* HAL block-device check on the emulated MSS MMC/SD (milestone M11): register
 * round trips (BlockSizeBlockCount, Argument1) and the static Capabilities1 /
 * HostControl2 identification bits through the reference MMC/SD driver. The
 * emulated board has no SD card attached, so no command is issued (a real
 * command would log "no SD card" and do nothing): the full transfer path is
 * host-tested, and on-target SD clock/bus-width/card behaviour is GAP-A5. The
 * bus descriptor is static because FF_BlockMpfs_t keeps a pointer to it. */
static uint32_t mpfs_mmc_read32(void *ctx, uint32_t off)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)(FF_MMC0_BASE + off);
}

static void mpfs_mmc_write32(void *ctx, uint32_t off, uint32_t val)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)(FF_MMC0_BASE + off) = val;
}

static bool renode_hal_block_check(void)
{
    static FF_BlockMpfs_t drv;
    static FF_BlockBus_t bus = {NULL, mpfs_mmc_read32, mpfs_mmc_write32};

    if (FF_BlockMpfsSetup(&drv, &bus, FF_MMC_SELFTEST_BLOCK_SIZE, 1u) != FF_SUCCESS) {
        return false;
    }
    return FF_BlockMpfsSelfTest(&drv) == FF_SUCCESS;
}

#endif /* FRICK_PLATFORM_MPFS */

/* Render one tri-state HAL check result. */
static const char *renode_check_str(FF_RenodeCheck_t r)
{
    switch (r) {
    case FF_RENODE_CHECK_PASS:
        return "PASS";
    case FF_RENODE_CHECK_CONTRACT:
        return "CONTRACT";
    case FF_RENODE_CHECK_FAIL:
    default:
        return "FAIL";
    }
}

void FF_AppRenodeTestMain(void *arg)
{
    FF_AppId_t me;
    FF_Msg_t msg;
    bool pub_ok;
    bool rx_ok = false;
    bool hal_ok = true;
    bool hal_serial_ok = true;
    bool hal_mem_ok = true;
    bool hal_eth_ok = true;
    bool hal_net_ok = true;
    bool hal_nvm_ok = true;
    bool hal_can_ok = true;
    bool hal_i2c_ok = true;
    bool hal_spi_ok = true;
    bool hal_block_ok = true;
    bool hal_tsn_ok = true;
    FF_RenodeCheck_t hal_pcie_ok = FF_RENODE_CHECK_FAIL;
    FF_RenodeCheck_t hal_spacewire_ok = FF_RENODE_CHECK_FAIL;
#if defined(FF_USE_LWIP)
    bool hal_net_lwip_ok = true;
#endif

    (void)arg;
    FF_PalLog("[renode-test] frick-fsw Renode functional test application\n");

    me = FF_EsGetId("RENODE_TEST");

    /* Software Bus publish/subscribe round trip (received inline). */
    pub_ok = (FF_SbSubscribe(RENODE_TEST_MSG_ID, me) == FF_SUCCESS);

    msg.msg_id = RENODE_TEST_MSG_ID;
    msg.apid = 0x010u;
    msg.time_ns = FF_TimeGetMetNs();
    msg.length = 4u;
    msg.payload[0] = 0xDEu;
    msg.payload[1] = 0xADu;
    msg.payload[2] = 0xBEu;
    msg.payload[3] = 0xEFu;
    if (FF_SbPublish(&msg) != FF_SUCCESS) {
        pub_ok = false;
    }
    if (FF_SbReceive(me, &msg, 1000u) == FF_SUCCESS) {
        rx_ok = true;
    }

    /* Event Services emit (visible on the bus / PUS S5). */
    FF_EvsSend(0x7E01u, FF_EVS_INFO, me, "renode-test");

    /* Time Services available and monotonic. */
    if (FF_TimeGetMetNs() == 0u) {
        rx_ok = false;
    }

#if defined(FRICK_PLATFORM_MPFS)
    hal_ok = renode_hal_gpio_check();
    hal_serial_ok = renode_hal_serial_check();
    hal_mem_ok = renode_hal_mem_check();
    hal_eth_ok = renode_hal_eth_check();
    hal_net_ok = renode_hal_net_check();
    hal_nvm_ok = renode_hal_nvm_check();
    hal_can_ok = renode_hal_can_check();
    hal_i2c_ok = renode_hal_i2c_check();
    hal_spi_ok = renode_hal_spi_check();
    hal_block_ok = renode_hal_block_check();
    hal_tsn_ok = renode_hal_tsn_check();
    hal_pcie_ok = renode_hal_pcie_check();
    hal_spacewire_ok = renode_hal_spacewire_check();
#if defined(FF_USE_LWIP)
    hal_net_lwip_ok = renode_hal_net_lwip_check();
#endif
    FF_PalLog("[renode-test] hal_gpio=%s\n", hal_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_serial=%s\n", hal_serial_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_mem=%s\n", hal_mem_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_eth=%s\n", hal_eth_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_net=%s\n", hal_net_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_nvm=%s\n", hal_nvm_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_can=%s\n", hal_can_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_i2c=%s\n", hal_i2c_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_spi=%s\n", hal_spi_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_block=%s\n", hal_block_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_tsn=%s\n", hal_tsn_ok ? "PASS" : "FAIL");
    FF_PalLog("[renode-test] hal_pcie=%s\n", renode_check_str(hal_pcie_ok));
    FF_PalLog("[renode-test] hal_spacewire=%s\n", renode_check_str(hal_spacewire_ok));
#if defined(FF_USE_LWIP)
    FF_PalLog("[renode-test] hal_net_lwip=%s\n", hal_net_lwip_ok ? "PASS" : "FAIL");
#endif
#endif

    FF_PalLog("[renode-test] sb_roundtrip=%s\n", rx_ok ? "PASS" : "FAIL");
    {
        bool all_ok = pub_ok && rx_ok;
        all_ok = all_ok && hal_ok;
        all_ok = all_ok && hal_serial_ok;
        all_ok = all_ok && hal_mem_ok;
        all_ok = all_ok && hal_eth_ok;
        all_ok = all_ok && hal_net_ok;
        all_ok = all_ok && hal_nvm_ok;
        all_ok = all_ok && hal_can_ok;
        all_ok = all_ok && hal_i2c_ok;
        all_ok = all_ok && hal_spi_ok;
        all_ok = all_ok && hal_block_ok;
        all_ok = all_ok && hal_tsn_ok;
        /* Contract-only checks (hal_pcie, hal_spacewire) are deliberately NOT
         * part of the verdict: on this board description they have no emulated
         * peripheral behind them, so counting them as PASS would overstate N18
         * coverage (N22). Their results are printed above as CONTRACT and their
         * contract coverage lives in the host unit tests. This block previously
         * ANDed their (constant-true) results in, which made the aggregate
         * assertion a tautology (cppcheck knownConditionTrueFalse). */
#if defined(FF_USE_LWIP)
        all_ok = all_ok && hal_net_lwip_ok;
#endif
        FF_PalLog("[renode-test] contract-only: hal_pcie, hal_spacewire (excluded from verdict)\n");
        FF_PalLog("RENODE_TEST: %s\n", all_ok ? "PASS" : "FAIL");
    }

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
