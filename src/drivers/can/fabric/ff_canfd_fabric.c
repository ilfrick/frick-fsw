/*
 * frick-fsw - CAN-FD fabric backend (PolarFire SoC), HAL backend (M4/S4c)
 *
 * CAN-FD frame/timing logic over a fabric window (SRS-REQ-062). Static, bounded,
 * no allocation at any time (SRS-REQ-015 / norm N10).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/can/fabric/ff_canfd_fabric.h"

static FF_FabricCanFd_t *g_canfd; /* single bound instance */

/* Bounded wait for transmit room in the controller FIFO; a full FIFO is reported
 * as FF_BUSY rather than blocking an unbounded time. */
#define FF_CANFD_TX_SPIN_LIMIT 100000u

/* CAM/standard identifier field widths (ISO 11898-1). */
#define FF_CANFD_ID_STD_MAX 0x7FFu
#define FF_CANFD_ID_EXT_MAX 0x1FFFFFFFu

/* DLC -> payload length (ISO 11898-1:2015): 0..8 map to themselves, then 12, 16,
 * 20, 24, 32, 48, 64. This table is the whole CAN-FD length encoding. */
static const uint8_t k_dlc_len[16] = {0u, 1u,  2u,  3u,  4u,  5u,  6u,  7u,
                                      8u, 12u, 16u, 20u, 24u, 32u, 48u, 64u};

FF_Status_t FF_CanFdDlcToLen(uint8_t dlc, uint8_t *len)
{
    if ((len == NULL) || (dlc > 15u)) {
        return FF_INVALID_ARGUMENT;
    }
    *len = k_dlc_len[dlc];
    return FF_SUCCESS;
}

FF_Status_t FF_CanFdLenToDlc(uint8_t len, uint8_t *dlc)
{
    uint8_t i;

    if ((dlc == NULL) || (len > (uint8_t)FF_CANFD_MAX_PAYLOAD)) {
        return FF_INVALID_ARGUMENT;
    }
    for (i = 0u; i < 16u; i++) {
        if (k_dlc_len[i] == len) {
            *dlc = i;
            return FF_SUCCESS;
        }
    }
    return FF_BAD_SIZE; /* a length CAN-FD cannot encode (e.g. 9, 13, 25) */
}

FF_Status_t FF_FabricCanFdBitTiming(uint32_t clk_hz, uint32_t bitrate, uint32_t *prescaler,
                                    uint32_t *seg1, uint32_t *seg2)
{
    uint32_t presc;
    uint32_t tq;

    if ((prescaler == NULL) || (seg1 == NULL) || (seg2 == NULL) || (clk_hz == 0u) ||
        (bitrate == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (bitrate > (0xFFFFFFFFu / 1024u)) {
        return FF_INVALID_ARGUMENT; /* outside anything the prescaler range could express */
    }
    for (presc = 1u; presc <= 1024u; presc++) {
        uint32_t div = bitrate * presc; /* cannot wrap: checked above */

        if ((clk_hz % div) != 0u) {
            continue; /* the bit time would not be exact: refuse rather than mis-program */
        }
        tq = clk_hz / div;
        if ((tq < 8u) || (tq > 20u) || ((tq % 5u) != 0u)) {
            continue; /* outside one sync + seg1 + seg2 with a 80% sample point */
        }
        *seg2 = tq / 5u;
        *seg1 = ((tq * 4u) / 5u) - 1u;
        if ((*seg2 == 0u) || (*seg1 < 2u)) {
            continue;
        }
        *prescaler = presc;
        return FF_SUCCESS;
    }
    return FF_INVALID_ARGUMENT;
}

/* Pack a bit timing into the core's BTP/DBTP word (reference layout): prescaler
 * (10 bits, stored minus one) | seg1-1 (4 bits) | seg2-1 (3 bits) | sjw-1
 * (2 bits, always 0: one quantum). */
static uint32_t canfd_pack_timing(uint32_t prescaler, uint32_t seg1, uint32_t seg2)
{
    return (((prescaler - 1u) & 0x3FFu) | (((seg1 - 1u) & 0x0Fu) << 10) |
            (((seg2 - 1u) & 0x07u) << 14));
}

static FF_Status_t canfd_w32(uint32_t off, uint32_t val)
{
    return g_canfd->fabric->write32(&g_canfd->win, off, val);
}

static FF_Status_t canfd_r32(uint32_t off, uint32_t *val)
{
    return g_canfd->fabric->read32(&g_canfd->win, off, val);
}

static FF_Status_t canfd_w8(uint32_t off, uint8_t val)
{
    return g_canfd->fabric->write8(&g_canfd->win, off, val);
}

static FF_Status_t canfd_r8(uint32_t off, uint8_t *val)
{
    return g_canfd->fabric->read8(&g_canfd->win, off, val);
}

static FF_Status_t canfd_init(void)
{
    if (g_canfd == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_canfd->win.open == 0u) {
        return FF_NOT_READY;
    }
    if (canfd_w32(FF_CANFD_REG_CMD, FF_CANFD_CMD_RESET_FIFOS) != FF_SUCCESS) {
        return FF_ERROR;
    }
    g_canfd->tx_frames = 0u;
    g_canfd->tx_busy = 0u;
    g_canfd->rx_frames = 0u;
    g_canfd->rx_errors = 0u;
    g_canfd->ready = 1u;
    return FF_SUCCESS;
}

static FF_Status_t canfd_configure(uint32_t bitrate, uint8_t fd, uint32_t data_bitrate)
{
    uint32_t presc = 0u;
    uint32_t seg1 = 0u;
    uint32_t seg2 = 0u;
    uint32_t dpresc = 0u;
    uint32_t dseg1 = 0u;
    uint32_t dseg2 = 0u;
    uint32_t mode;

    if (g_canfd == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_canfd->ready == 0u) {
        return FF_NOT_READY;
    }
    if ((bitrate == 0u) || (fd > 1u)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((fd != 0u) && (data_bitrate == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Derive every timing before touching the core, so a refused configuration
     * leaves the device exactly as it found it. */
    if (FF_FabricCanFdBitTiming(g_canfd->clk_hz, bitrate, &presc, &seg1, &seg2) != FF_SUCCESS) {
        return FF_INVALID_ARGUMENT;
    }
    if (fd != 0u) {
        if (FF_FabricCanFdBitTiming(g_canfd->clk_hz, data_bitrate, &dpresc, &dseg1, &dseg2) !=
            FF_SUCCESS) {
            return FF_INVALID_ARGUMENT;
        }
    }
    if (canfd_w32(FF_CANFD_REG_BTP, canfd_pack_timing(presc, seg1, seg2)) != FF_SUCCESS) {
        return FF_ERROR;
    }
    mode = FF_CANFD_MODE_ENABLE;
    if (fd != 0u) {
        if (canfd_w32(FF_CANFD_REG_DBTP, canfd_pack_timing(dpresc, dseg1, dseg2)) != FF_SUCCESS) {
            return FF_ERROR;
        }
        mode |= (FF_CANFD_MODE_FD | FF_CANFD_MODE_BRS);
    }
    if (canfd_w32(FF_CANFD_REG_MODE, mode) != FF_SUCCESS) {
        return FF_ERROR;
    }
    g_canfd->bitrate = bitrate;
    g_canfd->data_bitrate = (fd != 0u) ? data_bitrate : 0u;
    g_canfd->fd = fd;
    return FF_SUCCESS;
}

static FF_Status_t canfd_transmit(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len)
{
    uint8_t dlc = 0u;
    uint8_t max_len;
    uint32_t status = 0u;
    uint32_t spin = 0u;
    uint32_t flags;
    uint16_t i;

    if (g_canfd == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if ((g_canfd->ready == 0u) || (g_canfd->bitrate == 0u)) {
        return FF_NOT_READY; /* not initialised or not configured yet */
    }
    if (ext > 1u) {
        return FF_INVALID_ARGUMENT;
    }
    if ((data == NULL) && (len != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (ext != 0u) {
        if (id > FF_CANFD_ID_EXT_MAX) {
            return FF_INVALID_ARGUMENT;
        }
    }
    else if (id > FF_CANFD_ID_STD_MAX) {
        return FF_INVALID_ARGUMENT; /* 11-bit identifiers cannot carry this id */
    }
    max_len = (g_canfd->fd != 0u) ? (uint8_t)FF_CANFD_MAX_PAYLOAD : 8u;
    if (len > max_len) {
        return FF_BAD_SIZE; /* classic frames carry at most 8 bytes */
    }
    if (FF_CanFdLenToDlc(len, &dlc) != FF_SUCCESS) {
        return FF_BAD_SIZE;
    }
    for (;;) {
        if (canfd_r32(FF_CANFD_REG_STATUS, &status) != FF_SUCCESS) {
            return FF_ERROR;
        }
        if ((status & FF_CANFD_STATUS_TX_READY) != 0u) {
            break;
        }
        spin++;
        if (spin >= FF_CANFD_TX_SPIN_LIMIT) {
            g_canfd->tx_busy++;
            return FF_BUSY; /* no room: reported, nothing queued */
        }
    }
    flags = (id & FF_CANFD_ID_EXT_MAX) | ((uint32_t)ext << 29) | ((uint32_t)g_canfd->fd << 30);
    if (canfd_w32(FF_CANFD_REG_TX_ID, flags) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (canfd_w32(FF_CANFD_REG_TX_DLC, (uint32_t)dlc) != FF_SUCCESS) {
        return FF_ERROR;
    }
    for (i = 0u; i < (uint16_t)len; i++) {
        if (canfd_w8((uint32_t)FF_CANFD_REG_TX_DATA + (uint32_t)i, data[i]) != FF_SUCCESS) {
            return FF_ERROR;
        }
    }
    if (canfd_w32(FF_CANFD_REG_CMD, FF_CANFD_CMD_TX_REQUEST) != FF_SUCCESS) {
        return FF_ERROR;
    }
    g_canfd->tx_frames++;
    return FF_SUCCESS;
}

static void canfd_set_rx_callback(FF_CanRxCb_t cb, void *ctx)
{
    if (g_canfd == NULL) {
        return;
    }
    g_canfd->rx_cb = cb;
    g_canfd->rx_ctx = ctx;
}

FF_Status_t FF_FabricCanFdProcess(uint16_t budget)
{
    uint32_t status = 0u;
    uint32_t hdr = 0u;
    uint32_t raw_dlc = 0u;
    uint8_t data[FF_CANFD_MAX_PAYLOAD];
    uint8_t len = 0u;
    uint8_t ext;
    uint16_t n;
    uint16_t i;

    if (g_canfd == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if ((g_canfd->ready == 0u) || (g_canfd->win.open == 0u)) {
        return FF_NOT_READY;
    }
    for (n = 0u; n < budget; n++) {
        if (canfd_r32(FF_CANFD_REG_STATUS, &status) != FF_SUCCESS) {
            return FF_ERROR;
        }
        if ((status & FF_CANFD_STATUS_RX_EMPTY) != 0u) {
            break;
        }
        if ((canfd_r32(FF_CANFD_REG_RX_ID, &hdr) != FF_SUCCESS) ||
            (canfd_r32(FF_CANFD_REG_RX_DLC, &raw_dlc) != FF_SUCCESS)) {
            return FF_ERROR;
        }
        /* A frame the core describes inconsistently (DLC out of range, or a
         * payload longer than the configured mode can carry) is dropped: it is
         * released from the FIFO so the queue cannot wedge, and it is counted
         * rather than delivered. */
        if ((raw_dlc > 15u) || (FF_CanFdDlcToLen((uint8_t)raw_dlc, &len) != FF_SUCCESS) ||
            ((g_canfd->fd == 0u) && (len > 8u))) {
            g_canfd->rx_errors++;
            (void)canfd_w32(FF_CANFD_REG_CMD, FF_CANFD_CMD_POP_RX);
            continue;
        }
        for (i = 0u; i < (uint16_t)len; i++) {
            if (canfd_r8((uint32_t)FF_CANFD_REG_RX_DATA + (uint32_t)i, &data[i]) != FF_SUCCESS) {
                return FF_ERROR;
            }
        }
        if (canfd_w32(FF_CANFD_REG_CMD, FF_CANFD_CMD_POP_RX) != FF_SUCCESS) {
            return FF_ERROR;
        }
        g_canfd->rx_frames++;
        ext = (uint8_t)((hdr >> 29) & 1u);
        if (g_canfd->rx_cb != NULL) {
            g_canfd->rx_cb(hdr & (ext != 0u ? FF_CANFD_ID_EXT_MAX : FF_CANFD_ID_STD_MAX), ext, data,
                           len, g_canfd->rx_ctx);
        }
    }
    return FF_SUCCESS;
}

FF_Status_t FF_FabricCanFdSetup(FF_FabricCanFd_t *c, FF_Fabric_t *fabric, uint32_t dev_id,
                                uint32_t clk_hz)
{
    FF_Status_t rc;

    if ((c == NULL) || (fabric == NULL) || (clk_hz == 0u) || (fabric->open == NULL) ||
        (fabric->close == NULL) || (fabric->read32 == NULL) || (fabric->write32 == NULL) ||
        (fabric->read8 == NULL) || (fabric->write8 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    c->fabric = fabric;
    rc = fabric->open(dev_id, &c->win);
    if (rc != FF_SUCCESS) {
        c->fabric = NULL;
        return rc;
    }
    if (c->win.size < (uint32_t)FF_CANFD_WIN_SIZE) {
        (void)fabric->close(&c->win); /* never leave the window open on a refusal */
        c->fabric = NULL;
        return FF_BAD_SIZE;
    }
    c->rx_cb = NULL;
    c->rx_ctx = NULL;
    c->clk_hz = clk_hz;
    c->bitrate = 0u;
    c->data_bitrate = 0u;
    c->fd = 0u;
    c->ready = 0u;
    c->tx_frames = 0u;
    c->tx_busy = 0u;
    c->rx_frames = 0u;
    c->rx_errors = 0u;
    c->can.name = "fabric-canfd";
    c->can.supports_fd = 1u;
    c->can.init = canfd_init;
    c->can.configure = canfd_configure;
    c->can.transmit = canfd_transmit;
    c->can.set_rx_callback = canfd_set_rx_callback;
    g_canfd = c;
    return FF_SUCCESS;
}
