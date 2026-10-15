/*
 * frick-fsw - CAN-FD fabric backend unit tests (milestone M4/S4c)
 *
 * Exercises SRS-REQ-062 on the host: the CAN-FD backend is bound to the real
 * fabric backend (SRS-REQ-067) over an independent mock of the CAN-FD core's
 * register/FIFO behaviour, so the test covers the two drivers together. Frame
 * encoding, bit timing, transmit register programming, receive draining and
 * every out-of-bound input carry explicit expected results (norm N22).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/can/fabric/ff_canfd_fabric.h"
#include "drivers/fabric/mpfs/ff_fabric_mpfs.h"
#include <string.h>

#define CANFD_BASE 0x50000000u
#define CANFD_SPAN 0x100u
#define CANFD_CLK 100000000u /* 100 MHz core clock */

/* --- mock CAN-FD core ------------------------------------------------------ */

typedef struct
{
    uint32_t hdr;
    uint32_t dlc;
    uint8_t data[FF_CANFD_MAX_PAYLOAD];
} MockFrame_t;

static uint8_t g_reg[CANFD_SPAN];
static MockFrame_t g_rxq[4];
static int g_rxq_n;
static int g_rxq_head;
static int g_tx_ready;
static int g_out_of_window;
static int g_cmd_writes;
static uint32_t g_cmd_last;
static int g_tx_requests;
static int g_pops;
static int g_resets;

static uint32_t mock_status(void)
{
    uint32_t v = 0u;
    if (g_tx_ready != 0) {
        v |= FF_CANFD_STATUS_TX_READY;
    }
    if (g_rxq_n == 0) {
        v |= FF_CANFD_STATUS_RX_EMPTY;
    }
    return v;
}

static int mock_fits(uint32_t off, uint32_t width)
{
    return (off < CANFD_SPAN) && ((CANFD_SPAN - off) >= width);
}

static uint32_t mock_read32(void *ctx, uint32_t addr)
{
    uint32_t off = addr - CANFD_BASE;
    uint32_t v = 0u;
    (void)ctx;
    if (!mock_fits(off, 4u)) {
        g_out_of_window++;
        return 0u;
    }
    if (off == (uint32_t)FF_CANFD_REG_STATUS) {
        return mock_status();
    }
    if (off == (uint32_t)FF_CANFD_REG_RX_ID) {
        return (g_rxq_n > 0) ? g_rxq[g_rxq_head].hdr : 0u;
    }
    if (off == (uint32_t)FF_CANFD_REG_RX_DLC) {
        return (g_rxq_n > 0) ? g_rxq[g_rxq_head].dlc : 0u;
    }
    if (off == (uint32_t)FF_CANFD_REG_RX_AVAIL) {
        return (uint32_t)g_rxq_n;
    }
    if ((off >= (uint32_t)FF_CANFD_REG_RX_DATA) &&
        (off < ((uint32_t)FF_CANFD_REG_RX_DATA + FF_CANFD_MAX_PAYLOAD))) {
        return (g_rxq_n > 0) ? g_rxq[g_rxq_head].data[off - (uint32_t)FF_CANFD_REG_RX_DATA] : 0u;
    }
    memcpy(&v, &g_reg[off], 4u);
    return v;
}

static void mock_write32(void *ctx, uint32_t addr, uint32_t val)
{
    uint32_t off = addr - CANFD_BASE;
    (void)ctx;
    if (!mock_fits(off, 4u)) {
        g_out_of_window++;
        return;
    }
    if (off == (uint32_t)FF_CANFD_REG_STATUS) {
        return; /* read-only */
    }
    if (off == (uint32_t)FF_CANFD_REG_CMD) {
        g_cmd_writes++;
        g_cmd_last = val;
        if ((val & (uint32_t)FF_CANFD_CMD_RESET_FIFOS) != 0u) {
            g_resets++;
            g_rxq_n = 0;
            g_rxq_head = 0;
        }
        if ((val & (uint32_t)FF_CANFD_CMD_TX_REQUEST) != 0u) {
            g_tx_requests++;
        }
        if ((val & (uint32_t)FF_CANFD_CMD_POP_RX) != 0u) {
            g_pops++;
            if (g_rxq_n > 0) {
                g_rxq_n--;
                g_rxq_head++;
            }
        }
        return;
    }
    memcpy(&g_reg[off], &val, 4u);
}

static uint8_t mock_read8(void *ctx, uint32_t addr)
{
    uint32_t off = addr - CANFD_BASE;
    (void)ctx;
    if (!mock_fits(off, 1u)) {
        g_out_of_window++;
        return 0u;
    }
    if ((off >= (uint32_t)FF_CANFD_REG_RX_DATA) &&
        (off < ((uint32_t)FF_CANFD_REG_RX_DATA + FF_CANFD_MAX_PAYLOAD))) {
        return (g_rxq_n > 0) ? g_rxq[g_rxq_head].data[off - (uint32_t)FF_CANFD_REG_RX_DATA] : 0u;
    }
    return g_reg[off];
}

static void mock_write8(void *ctx, uint32_t addr, uint8_t val)
{
    uint32_t off = addr - CANFD_BASE;
    (void)ctx;
    if (!mock_fits(off, 1u)) {
        g_out_of_window++;
        return;
    }
    g_reg[off] = val;
}

/* --- binding --------------------------------------------------------------- */

static const FF_FabricMap_t g_map[] = {
    {FF_FABRIC_DEV_CANFD, CANFD_BASE, CANFD_SPAN},
};
/* Catalogue entry smaller than the core's window: must be refused. */
static const FF_FabricMap_t g_map_small[] = {
    {FF_FABRIC_DEV_CANFD, CANFD_BASE, 0x40u},
};

static FF_FabricMpfs_t g_fabric;
static FF_FabricCanFd_t g_canfd;
static FF_FabricCanFd_t g_canfd2;
static FF_FabricBus_t g_bus = {NULL, mock_read32, mock_write32, mock_read8, mock_write8};

/* Received-frame log (deferred-work callback contract). */
static uint32_t g_rx_id[4];
static uint8_t g_rx_ext[4];
static uint8_t g_rx_len[4];
static uint8_t g_rx_data[4][FF_CANFD_MAX_PAYLOAD];
static int g_rx_cb_calls;

static void rx_cb(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len, void *ctx)
{
    int n = g_rx_cb_calls;
    (void)ctx;
    if (n < 4) {
        g_rx_id[n] = id;
        g_rx_ext[n] = ext;
        g_rx_len[n] = len;
        memcpy(g_rx_data[n], data, len);
    }
    g_rx_cb_calls++;
}

static void mock_reset(void)
{
    memset(g_reg, 0, sizeof(g_reg));
    memset(g_rxq, 0, sizeof(g_rxq));
    g_rxq_n = 0;
    g_rxq_head = 0;
    g_tx_ready = 1;
    g_out_of_window = 0;
    g_cmd_writes = 0;
    g_cmd_last = 0u;
    g_tx_requests = 0;
    g_pops = 0;
    g_resets = 0;
    g_rx_cb_calls = 0;
    memset(g_rx_id, 0, sizeof(g_rx_id));
    memset(g_rx_ext, 0, sizeof(g_rx_ext));
    memset(g_rx_len, 0, sizeof(g_rx_len));
    memset(g_rx_data, 0, sizeof(g_rx_data));
}

static void queue_frame(uint32_t id, uint8_t ext, uint8_t dlc, uint8_t len, uint8_t seed)
{
    MockFrame_t *f;
    uint16_t i;

    if ((g_rxq_head + g_rxq_n) >= (int)(sizeof(g_rxq) / sizeof(g_rxq[0]))) {
        return; /* the mock queue is full: nothing queued */
    }
    f = &g_rxq[g_rxq_head + g_rxq_n];
    f->hdr = (id & 0x1FFFFFFFu) | ((uint32_t)ext << 29) | (1u << 30);
    f->dlc = dlc;
    for (i = 0u; i < len; i++) {
        f->data[i] = (uint8_t)(seed + i);
    }
    g_rxq_n++;
}

/* --- DLC mapping ----------------------------------------------------------- */

static void test_canfd_dlc_mapping(void)
{
    /* Canonical CAN-FD payload lengths (ISO 11898-1:2015). */
    static const uint8_t canonical[16] = {0u, 1u,  2u,  3u,  4u,  5u,  6u,  7u,
                                          8u, 12u, 16u, 20u, 24u, 32u, 48u, 64u};
    uint8_t dlc = 0u;
    uint8_t len = 0u;
    uint8_t round = 0u;
    uint8_t i;

    for (i = 0u; i < 16u; i++) {
        FF_TEST_ASSERT(FF_CanFdDlcToLen(i, &len) == FF_SUCCESS);
        FF_TEST_ASSERT(len == canonical[i]);
        /* Every canonical length encodes back to the same DLC (the mapping is a
         * bijection on its image). */
        FF_TEST_ASSERT(FF_CanFdLenToDlc(len, &round) == FF_SUCCESS);
        FF_TEST_ASSERT(round == i);
    }
    FF_TEST_ASSERT(FF_CanFdDlcToLen(16u, &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanFdDlcToLen(0u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanFdLenToDlc(65u, &dlc) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanFdLenToDlc(0u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanFdLenToDlc(64u, &dlc) == FF_SUCCESS);
    FF_TEST_ASSERT(dlc == 15u);

    /* Lengths with no DLC encoding are refused, not rounded. */
    for (i = 9u; i <= 63u; i++) {
        uint8_t ok = 0u;
        uint8_t j;
        for (j = 0u; j < 16u; j++) {
            if (canonical[j] == i) {
                ok = 1u;
            }
        }
        if (ok == 0u) {
            FF_TEST_ASSERT(FF_CanFdLenToDlc(i, &dlc) == FF_BAD_SIZE);
        }
    }
}

/* --- bit timing ------------------------------------------------------------ */

static void test_canfd_timing(void)
{
    uint32_t presc = 0u;
    uint32_t seg1 = 0u;
    uint32_t seg2 = 0u;
    uint32_t tq;

    /* Nominal cases: the derived timing reconstructs the bit rate exactly. */
    static const uint32_t rate[5] = {1000000u, 500000u, 250000u, 125000u, 100000u};
    uint8_t i;

    for (i = 0u; i < 5u; i++) {
        FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, rate[i], &presc, &seg1, &seg2) ==
                       FF_SUCCESS);
        tq = 1u + seg1 + seg2;
        FF_TEST_ASSERT(tq >= 8u);
        FF_TEST_ASSERT(tq <= 20u);
        FF_TEST_ASSERT((CANFD_CLK / (presc * tq)) == rate[i]);
        FF_TEST_ASSERT((presc * tq * rate[i]) == CANFD_CLK); /* exact: no rounding */
        /* This reference backend only accepts integral 80% sample points. */
        FF_TEST_ASSERT(((1u + seg1) * 5u) == (tq * 4u));
        FF_TEST_ASSERT(seg2 >= 1u);
        FF_TEST_ASSERT(seg1 >= 2u);
        FF_TEST_ASSERT(presc <= 1024u);
    }
    /* 1 Mbit/s on a 100 MHz clock: 20 quanta per bit, prescaler 5. */
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 1000000u, &presc, &seg1, &seg2) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(presc == 5u);
    FF_TEST_ASSERT(seg1 == 15u);
    FF_TEST_ASSERT(seg2 == 4u);

    /* Out of the representable range: refused rather than mis-programmed. */
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 800000u, &presc, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    /* 8 quanta cannot represent an exact 80% sample point with integer segments. */
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(8000000u, 1000000u, &presc, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 1000000000u, &presc, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 12000000u, &presc, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(0u, 1000000u, &presc, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 0u, &presc, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 1000000u, NULL, &seg1, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 1000000u, &presc, NULL, &seg2) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 1000000u, &presc, &seg1, NULL) ==
                   FF_INVALID_ARGUMENT);
}

/* --- setup ----------------------------------------------------------------- */

static void test_canfd_setup(void)
{
    mock_reset();

    /* The fabric must be initialised and hold the catalogue entry. */
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fabric, &g_bus, g_map, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fabric.fabric.init() == FF_SUCCESS);

    FF_TEST_ASSERT(FF_FabricCanFdSetup(NULL, &g_fabric.fabric, FF_FABRIC_DEV_CANFD, CANFD_CLK) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, NULL, FF_FABRIC_DEV_CANFD, CANFD_CLK) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, &g_fabric.fabric, FF_FABRIC_DEV_CANFD, 0u) ==
                   FF_INVALID_ARGUMENT);

    /* A device id the catalogue does not hold is a lookup miss. */
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, &g_fabric.fabric, 0x7Fu, CANFD_CLK) ==
                   FF_NOT_FOUND);
    FF_TEST_ASSERT(g_canfd.fabric == NULL);

    /* An undersized catalogue entry is refused and the window is left closed
     * (no half-open window survives a rejected binding). */
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fabric, &g_bus, g_map_small, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fabric.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd2, &g_fabric.fabric, FF_FABRIC_DEV_CANFD,
                                       CANFD_CLK) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_canfd2.win.open == 0u);
    FF_TEST_ASSERT(g_canfd2.fabric == NULL);

    /* Nominal bind on the full-span catalogue completes the CAN vtable. */
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fabric, &g_bus, g_map, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fabric.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, &g_fabric.fabric, FF_FABRIC_DEV_CANFD,
                                       CANFD_CLK) == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.can.name != NULL);
    FF_TEST_ASSERT(g_canfd.can.supports_fd == 1u); /* this core does CAN-FD */
    FF_TEST_ASSERT(g_canfd.can.init != NULL);
    FF_TEST_ASSERT(g_canfd.can.configure != NULL);
    FF_TEST_ASSERT(g_canfd.can.transmit != NULL);
    FF_TEST_ASSERT(g_canfd.can.set_rx_callback != NULL);
    FF_TEST_ASSERT(g_canfd.win.open == 1u);
    FF_TEST_ASSERT(g_canfd.ready == 0u);
    FF_TEST_ASSERT(g_canfd.bitrate == 0u);

    /* init() resets the FIFOs and reports readiness. */
    FF_TEST_ASSERT(g_canfd.can.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.ready == 1u);
    FF_TEST_ASSERT(g_resets == 1);
    FF_TEST_ASSERT(g_cmd_last == (uint32_t)FF_CANFD_CMD_RESET_FIFOS);
    FF_TEST_ASSERT(g_out_of_window == 0);
}

/* --- configure ------------------------------------------------------------- */

static void test_canfd_configure(void)
{
    uint32_t mode = 0u;
    uint32_t btp = 0u;

    mock_reset();
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fabric, &g_bus, g_map, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fabric.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, &g_fabric.fabric, FF_FABRIC_DEV_CANFD,
                                       CANFD_CLK) == FF_SUCCESS);

    /* Not initialised yet: nothing is programmed. */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 0u, 0u) == FF_NOT_READY);
    FF_TEST_ASSERT(g_canfd.can.init() == FF_SUCCESS);

    /* Out-of-bound requests: nothing is programmed. */
    FF_TEST_ASSERT(g_canfd.can.configure(0u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 2u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 1u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.configure(800000u, 0u, 0u) == FF_INVALID_ARGUMENT);
    memcpy(&mode, &g_reg[FF_CANFD_REG_MODE], 4u);
    memcpy(&btp, &g_reg[FF_CANFD_REG_BTP], 4u);
    FF_TEST_ASSERT(mode == 0u); /* refused before any register write */
    FF_TEST_ASSERT(btp == 0u);

    /* Classic CAN: 500 kbit/s -> 20 quanta per bit, prescaler 10 (seg2 = 4,
     * seg1 = 15). */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.bitrate == 500000u);
    FF_TEST_ASSERT(g_canfd.fd == 0u);
    FF_TEST_ASSERT(g_canfd.data_bitrate == 0u);
    memcpy(&mode, &g_reg[FF_CANFD_REG_MODE], 4u);
    FF_TEST_ASSERT(mode == (uint32_t)FF_CANFD_MODE_ENABLE);
    memcpy(&btp, &g_reg[FF_CANFD_REG_BTP], 4u);
    FF_TEST_ASSERT(btp == ((10u - 1u) | ((15u - 1u) << 10) | ((4u - 1u) << 14)));

    /* CAN-FD: nominal 500 kbit/s, data phase 2 Mbit/s -> 20 quanta per bit,
     * prescaler 5 (seg2 = 2, seg1 = 7); the mode gains FD + bit-rate switch. */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 1u, 2000000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.fd == 1u);
    FF_TEST_ASSERT(g_canfd.data_bitrate == 2000000u);
    memcpy(&mode, &g_reg[FF_CANFD_REG_MODE], 4u);
    FF_TEST_ASSERT(mode == (uint32_t)(FF_CANFD_MODE_ENABLE | FF_CANFD_MODE_FD | FF_CANFD_MODE_BRS));
    memcpy(&btp, &g_reg[FF_CANFD_REG_DBTP], 4u);
    FF_TEST_ASSERT(btp == ((5u - 1u) | ((7u - 1u) << 10) | ((2u - 1u) << 14)));

    /* An FD request whose data phase has no exact timing is refused, and the
     * previously programmed configuration stays in force (no half update). */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 1u, 800000u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.bitrate == 500000u);
    FF_TEST_ASSERT(g_canfd.fd == 1u);
    FF_TEST_ASSERT(g_canfd.data_bitrate == 2000000u);
    FF_TEST_ASSERT(g_out_of_window == 0);
}

/* --- transmit -------------------------------------------------------------- */

static void test_canfd_transmit(void)
{
    uint8_t payload[FF_CANFD_MAX_PAYLOAD];
    uint32_t v = 0u;
    uint16_t i;

    memset(payload, 0, sizeof(payload));
    for (i = 0u; i < FF_CANFD_MAX_PAYLOAD; i++) {
        payload[i] = (uint8_t)(0xA0u + i);
    }

    mock_reset();
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fabric, &g_bus, g_map, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fabric.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, &g_fabric.fabric, FF_FABRIC_DEV_CANFD,
                                       CANFD_CLK) == FF_SUCCESS);

    /* Before init()/configure() nothing can be transmitted. */
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 0u, payload, 8u) == FF_NOT_READY);
    FF_TEST_ASSERT(g_canfd.can.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 0u, payload, 8u) == FF_NOT_READY);
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 1u, 2000000u) == FF_SUCCESS);

    /* Out-of-bound arguments. */
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 2u, payload, 8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 0u, NULL, 8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x800u, 0u, payload, 8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x20000000u, 1u, payload, 8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 1u, payload, 9u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 1u, payload, 65u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_tx_requests == 0);

    /* Classic-mode limit is 8 bytes even on this FD-capable core. */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 0u, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 0u, payload, 9u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 0u, payload, 8u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_tx_requests == 1);

    /* Nominal CAN-FD frame: extended id, 12-byte payload (DLC 9). */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 1u, 2000000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x1234567u, 1u, payload, 12u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_tx_requests == 2);
    memcpy(&v, &g_reg[FF_CANFD_REG_TX_ID], 4u);
    FF_TEST_ASSERT((v & 0x1FFFFFFFu) == 0x1234567u);
    FF_TEST_ASSERT(((v >> 29) & 1u) == 1u); /* extended */
    FF_TEST_ASSERT(((v >> 30) & 1u) == 1u); /* FD */
    memcpy(&v, &g_reg[FF_CANFD_REG_TX_DLC], 4u);
    FF_TEST_ASSERT(v == 9u);
    for (i = 0u; i < 12u; i++) {
        FF_TEST_ASSERT(g_reg[FF_CANFD_REG_TX_DATA + i] == payload[i]);
    }

    /* Zero-length frame and the full 64-byte frame (DLC 15). */
    FF_TEST_ASSERT(g_canfd.can.transmit(0x7FFu, 0u, NULL, 0u) == FF_SUCCESS);
    memcpy(&v, &g_reg[FF_CANFD_REG_TX_DLC], 4u);
    FF_TEST_ASSERT(v == 0u);
    FF_TEST_ASSERT(g_canfd.can.transmit(0x1FFFFFFFu, 1u, payload, 64u) == FF_SUCCESS);
    memcpy(&v, &g_reg[FF_CANFD_REG_TX_DLC], 4u);
    FF_TEST_ASSERT(v == 15u);
    FF_TEST_ASSERT(g_reg[FF_CANFD_REG_TX_DATA + 63u] == payload[63]);
    FF_TEST_ASSERT(g_canfd.tx_frames == 4u);
    FF_TEST_ASSERT(g_canfd.tx_busy == 0u);

    /* No transmit room: reported as busy, nothing queued, counted. */
    g_tx_ready = 0;
    FF_TEST_ASSERT(g_canfd.can.transmit(0x123u, 0u, payload, 8u) == FF_BUSY);
    FF_TEST_ASSERT(g_canfd.tx_busy == 1u);
    FF_TEST_ASSERT(g_tx_requests == 4); /* unchanged: no request was issued */
    FF_TEST_ASSERT(g_canfd.tx_frames == 4u);
    g_tx_ready = 1;
    FF_TEST_ASSERT(g_out_of_window == 0);
}

/* --- receive --------------------------------------------------------------- */

static void test_canfd_receive(void)
{
    uint32_t tq;
    uint32_t presc = 0u;
    uint32_t seg1 = 0u;
    uint32_t seg2 = 0u;

    mock_reset();
    FF_TEST_ASSERT(FF_FabricMpfsSetup(&g_fabric, &g_bus, g_map, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_fabric.fabric.init() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd, &g_fabric.fabric, FF_FABRIC_DEV_CANFD,
                                       CANFD_CLK) == FF_SUCCESS);

    /* Before init() the FIFO is not drained. */
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_NOT_READY);
    FF_TEST_ASSERT(g_canfd.can.init() == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 1u, 2000000u) == FF_SUCCESS);
    g_canfd.can.set_rx_callback(rx_cb, NULL);

    /* Empty FIFO: nothing to do, no bus access beyond the status poll. */
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 0);
    FF_TEST_ASSERT(g_pops == 0);

    /* Two queued frames: one classic standard 8-byte, one FD extended 12-byte. */
    queue_frame(0x123u, 0u, 8u, 8u, 0x10u);
    queue_frame(0x1234567u, 1u, 9u, 12u, 0x20u);
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 2);
    FF_TEST_ASSERT(g_pops == 2);
    FF_TEST_ASSERT(g_canfd.rx_frames == 2u);
    FF_TEST_ASSERT(g_canfd.rx_errors == 0u);
    FF_TEST_ASSERT(g_rx_id[0] == 0x123u);
    FF_TEST_ASSERT(g_rx_ext[0] == 0u);
    FF_TEST_ASSERT(g_rx_len[0] == 8u);
    FF_TEST_ASSERT(g_rx_data[0][0] == 0x10u);
    FF_TEST_ASSERT(g_rx_data[0][7] == 0x17u);
    FF_TEST_ASSERT(g_rx_id[1] == 0x1234567u);
    FF_TEST_ASSERT(g_rx_ext[1] == 1u);
    FF_TEST_ASSERT(g_rx_len[1] == 12u);
    FF_TEST_ASSERT(g_rx_data[1][11] == 0x2Bu);

    /* The FIFO is empty again; a zero budget drains nothing. */
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 2);
    queue_frame(0x1u, 0u, 1u, 1u, 0x30u);
    FF_TEST_ASSERT(FF_FabricCanFdProcess(0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 2);
    FF_TEST_ASSERT(g_canfd.rx_frames == 2u);

    /* A frame the core describes inconsistently (DLC 16) is released and
     * counted, never delivered. */
    g_rxq_n = 0;
    g_rxq_head = 0;
    queue_frame(0x2u, 1u, 16u, 4u, 0x40u);
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 2);
    FF_TEST_ASSERT(g_canfd.rx_errors == 1u);
    FF_TEST_ASSERT(g_pops == 3);

    /* A 12-byte payload while the core is configured for classic CAN is also a
     * device-side inconsistency. */
    FF_TEST_ASSERT(g_canfd.can.configure(500000u, 0u, 0u) == FF_SUCCESS);
    queue_frame(0x3u, 0u, 9u, 12u, 0x50u);
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 2);
    FF_TEST_ASSERT(g_canfd.rx_errors == 2u);

    /* Without a callback the FIFO is still drained (it cannot wedge). */
    g_canfd.can.set_rx_callback(NULL, NULL);
    queue_frame(0x4u, 0u, 8u, 8u, 0x60u);
    FF_TEST_ASSERT(FF_FabricCanFdProcess(4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_rx_cb_calls == 2);
    FF_TEST_ASSERT(g_canfd.rx_frames == 3u);
    FF_TEST_ASSERT(g_pops == 5);

    /* A second instance cannot hijack the bound one: the process entry point
     * uses the bound instance, so re-binding is the only way to switch. */
    FF_TEST_ASSERT(FF_FabricCanFdSetup(&g_canfd2, &g_fabric.fabric, FF_FABRIC_DEV_CANFD,
                                       CANFD_CLK) == FF_SUCCESS);
    FF_TEST_ASSERT(g_canfd2.can.init() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_FabricCanFdProcess(1u) == FF_SUCCESS); /* drained via the new binding */
    FF_TEST_ASSERT(g_out_of_window == 0);

    /* The timing helper remains consistent with the configured core. */
    FF_TEST_ASSERT(FF_FabricCanFdBitTiming(CANFD_CLK, 500000u, &presc, &seg1, &seg2) == FF_SUCCESS);
    tq = 1u + seg1 + seg2;
    FF_TEST_ASSERT(tq == 20u);
}

FF_TEST_MAIN(test_canfd_dlc_mapping, test_canfd_timing, test_canfd_setup, test_canfd_configure,
             test_canfd_transmit, test_canfd_receive)
