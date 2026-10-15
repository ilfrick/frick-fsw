/*
 * frick-fsw - MSS CAN driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Classic CAN 2.0 A/B over the Microchip PolarFire SoC MSS CAN controller,
 * register logic on an injected double-word MMIO bus. Static, no allocation.
 * Single instance (the HAL CAN contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/can/mpfs/ff_can_mpfs.h"

static FF_CanMpfs_t *g_can; /* single bound instance */

static uint32_t can_tx_off(uint32_t buf)
{
    return FF_CAN_TX_BASE + (buf * FF_CAN_TX_STRIDE);
}

static uint32_t can_rx_off(uint32_t buf)
{
    return FF_CAN_RX_BASE + (buf * FF_CAN_RX_STRIDE);
}

/* Pack up to 4 payload bytes into a 32-bit little-endian data register. */
static uint32_t can_pack(const uint8_t *data, uint8_t len)
{
    uint32_t v = 0u;
    uint8_t i;

    for (i = 0u; (i < len) && (i < 4u); i++) {
        v |= ((uint32_t)data[i]) << (8u * (uint32_t)i);
    }
    return v;
}

static FF_Status_t can_init(void)
{
    if (g_can == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Stop, clear interrupts, then run. */
    g_can->bus.write32(g_can->bus.ctx, FF_CAN_REG_COMMAND, 0u);
    g_can->bus.write32(g_can->bus.ctx, FF_CAN_REG_INTERRUPT_ENABLE, 0u);
    g_can->bus.write32(g_can->bus.ctx, FF_CAN_REG_COMMAND, FF_CAN_CMD_RUN);
    return FF_SUCCESS;
}

/* Reference bit timing from FF_CAN_MPFS_CLK_HZ (87.5% sample point). The MSS
 * CAN is classic only; a CAN-FD request is rejected (FD is a fabric device).
 * The driver targets the standard bit rates (125/250/500 kbit/s and 1 Mbit/s,
 * exact at the 80 MHz reference); any other rate is rounded to the nearest
 * prescaler, i.e. the achieved rate is within one prescaler step of the request
 * (the per-rate result is confirmed on target, GAP-A5). */
static FF_Status_t can_configure(uint32_t bitrate, uint8_t fd, uint32_t data_bitrate)
{
    uint32_t tq;
    uint32_t divisor;
    uint32_t presc;
    uint32_t cfg;

    (void)data_bitrate;
    if ((g_can == NULL) || (bitrate == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (fd != 0u) {
        return FF_NOT_IMPLEMENTED;
    }
    tq = 1u + FF_CAN_TSEG1 + FF_CAN_TSEG2; /* total time quanta (16) */
    if (bitrate > (FF_CAN_MPFS_CLK_HZ / tq)) {
        return FF_INVALID_ARGUMENT; /* requested rate exceeds the bus clock */
    }
    divisor = bitrate * tq;
    presc = (FF_CAN_MPFS_CLK_HZ + (divisor / 2u)) / divisor; /* rounded */
    if (presc == 0u) {
        presc = 1u; /* defensive: unreachable, as presc >= 1 whenever bitrate <= clk/tq */
    }
    if (presc > 0x7FFFu) {
        return FF_INVALID_ARGUMENT; /* prescaler field is 15 bits */
    }
    cfg = ((presc & 0x7FFFu) << 16) | (FF_CAN_SJW << 2) | (FF_CAN_TSEG2 << 5) | (FF_CAN_TSEG1 << 8);
    g_can->bus.write32(g_can->bus.ctx, FF_CAN_REG_CONFIG, cfg);
    return FF_SUCCESS;
}

static FF_Status_t can_transmit(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len)
{
    uint32_t base;
    uint32_t hi;
    uint32_t lo;
    uint32_t cmd;

    if (g_can == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (((data == NULL) && (len != 0u)) || (len > FF_CAN_MAX_DLC)) {
        return FF_INVALID_ARGUMENT;
    }
    if (((ext == 0u) && (id > FF_CAN_STD_ID_MAX)) || ((ext != 0u) && (id > FF_CAN_EXT_ID_MAX))) {
        return FF_INVALID_ARGUMENT;
    }
    base = can_tx_off(g_can->next_tx);
    lo = (data != NULL) ? can_pack(data, len) : 0u;
    hi = ((data != NULL) && (len > 4u)) ? can_pack(&data[4], (uint8_t)(len - 4u)) : 0u;
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_TX_DATA_LO_OFF, lo);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_TX_DATA_HI_OFF, hi);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_TX_ID_OFF,
                       (id & FF_CAN_ID_MASK) << FF_CAN_ID_SHIFT);
    cmd = FF_CAN_TX_CMD_TXREQ | (((uint32_t)len) << FF_CAN_TX_CMD_DLC_SHIFT);
    if (ext != 0u) {
        cmd |= FF_CAN_TX_CMD_IDE;
    }
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_TX_CMD_OFF, cmd);
    g_can->next_tx = (g_can->next_tx + 1u) % FF_CAN_TX_BUFFERS;
    return FF_SUCCESS;
}

static void can_set_rx_callback(FF_CanRxCb_t cb, void *ctx)
{
    uint32_t base;

    if (g_can == NULL) {
        return;
    }
    g_can->rx_cb = cb;
    g_can->rx_ctx = ctx;
    /* Accept every identifier into RX buffer 0; interrupts stay disabled (the
     * mission wires the ISR/poll loop via FF_CanMpfsPoll). */
    base = can_rx_off(0u);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_RX_AMR_OFF, FF_CAN_ACCEPT_ALL);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_RX_ACR_OFF, 0u);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_RX_AMR_DATA_OFF, 0xFFFFu);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_RX_ACR_DATA_OFF, 0u);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_RX_ID_OFF, 0u);
    g_can->bus.write32(g_can->bus.ctx, base + FF_CAN_RX_CMD_OFF, FF_CAN_RX_CMD_RXENBL);
}

FF_Status_t FF_CanMpfsPoll(FF_CanMpfs_t *c, uint32_t *got)
{
    uint32_t status;
    uint32_t i;

    if (c == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (got != NULL) {
        *got = 0u;
    }
    status = c->bus.read32(c->bus.ctx, FF_CAN_REG_RX_STATUS);
    for (i = 0u; i < FF_CAN_RX_BUFFERS; i++) {
        uint32_t base;
        uint32_t cmd;
        uint32_t id;
        uint32_t dlc;
        uint32_t lo;
        uint32_t hi;
        uint32_t k;
        uint8_t buf[FF_CAN_MAX_DLC];

        if ((status & (1u << i)) == 0u) {
            continue;
        }
        base = can_rx_off(i);
        cmd = c->bus.read32(c->bus.ctx, base + FF_CAN_RX_CMD_OFF);
        id = c->bus.read32(c->bus.ctx, base + FF_CAN_RX_ID_OFF);
        dlc = (cmd >> FF_CAN_RX_CMD_DLC_SHIFT) & 0xFu;
        if (dlc > FF_CAN_MAX_DLC) {
            dlc = FF_CAN_MAX_DLC;
        }
        lo = c->bus.read32(c->bus.ctx, base + FF_CAN_RX_DATA_LO_OFF);
        hi = c->bus.read32(c->bus.ctx, base + FF_CAN_RX_DATA_HI_OFF);
        for (k = 0u; (k < dlc) && (k < 4u); k++) {
            buf[k] = (uint8_t)((lo >> (8u * k)) & 0xFFu);
        }
        for (k = 4u; k < dlc; k++) {
            buf[k] = (uint8_t)((hi >> (8u * (k - 4u))) & 0xFFu);
        }
        if (c->rx_cb != NULL) {
            c->rx_cb((id >> FF_CAN_ID_SHIFT) & FF_CAN_ID_MASK,
                     ((cmd & FF_CAN_RX_CMD_IDE) != 0u) ? 1u : 0u, buf, (uint8_t)dlc, c->rx_ctx);
        }
        /* Acknowledge: clear MsgAv (write-1-to-clear) preserving the other
         * functional bits (buffer enable, DLC, IDE). Re-read the control word
         * after the callback so a reconfiguration performed inside the callback
         * for this buffer is not clobbered by the acknowledge. */
        cmd = c->bus.read32(c->bus.ctx, base + FF_CAN_RX_CMD_OFF);
        c->bus.write32(c->bus.ctx, base + FF_CAN_RX_CMD_OFF,
                       (cmd & ~FF_CAN_RX_CMD_MSGAV) | FF_CAN_RX_CMD_MSGAV);
        if (got != NULL) {
            *got = *got + 1u;
        }
    }
    return FF_SUCCESS;
}

FF_Status_t FF_CanMpfsSelfTest(FF_CanMpfs_t *c)
{
    uint32_t v;

    if (c == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Probe with interrupts disabled (no IRQ raised; status untouched). */
    c->bus.write32(c->bus.ctx, FF_CAN_REG_INTERRUPT_ENABLE, 0u);

    /* Command run/stop bit round trip. */
    c->bus.write32(c->bus.ctx, FF_CAN_REG_COMMAND, 0u);
    if ((c->bus.read32(c->bus.ctx, FF_CAN_REG_COMMAND) & FF_CAN_CMD_RUN) != 0u) {
        return FF_ERROR;
    }
    c->bus.write32(c->bus.ctx, FF_CAN_REG_COMMAND, FF_CAN_CMD_RUN);
    if ((c->bus.read32(c->bus.ctx, FF_CAN_REG_COMMAND) & FF_CAN_CMD_RUN) == 0u) {
        return FF_ERROR;
    }

    /* Interrupt-enable register round trip (then left disabled). */
    c->bus.write32(c->bus.ctx, FF_CAN_REG_INTERRUPT_ENABLE, FF_CAN_INT_ENBL | FF_CAN_INT_TX_MSG);
    v = c->bus.read32(c->bus.ctx, FF_CAN_REG_INTERRUPT_ENABLE);
    if ((v & (FF_CAN_INT_ENBL | FF_CAN_INT_TX_MSG)) != (FF_CAN_INT_ENBL | FF_CAN_INT_TX_MSG)) {
        return FF_ERROR;
    }
    c->bus.write32(c->bus.ctx, FF_CAN_REG_INTERRUPT_ENABLE, 0u);

    /* TX buffer 0 identifier register round trip. */
    c->bus.write32(c->bus.ctx, FF_CAN_TX_BASE + FF_CAN_TX_ID_OFF, 0x123u << FF_CAN_ID_SHIFT);
    if (c->bus.read32(c->bus.ctx, FF_CAN_TX_BASE + FF_CAN_TX_ID_OFF) !=
        (0x123u << FF_CAN_ID_SHIFT)) {
        return FF_ERROR;
    }

    /* RX acceptance-mask data register round trip. The emulated MPFS_CAN
     * implements only bits 0-15 of this register (and does not implement the
     * AMR RTR/IDE mask bits), so the round trip uses this fully implemented
     * field rather than the AMR. */
    c->bus.write32(c->bus.ctx, FF_CAN_RX_BASE + FF_CAN_RX_AMR_DATA_OFF, 0xFFFFu);
    if (c->bus.read32(c->bus.ctx, FF_CAN_RX_BASE + FF_CAN_RX_AMR_DATA_OFF) != 0xFFFFu) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

FF_Status_t FF_CanMpfsSetup(FF_CanMpfs_t *c, const FF_CanBus_t *bus)
{
    if ((c == NULL) || (bus == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    c->bus = *bus;
    c->rx_cb = NULL;
    c->rx_ctx = NULL;
    c->next_tx = 0u;
    c->can.name = "mpfs-can";
    c->can.supports_fd = 0u;
    c->can.init = can_init;
    c->can.configure = can_configure;
    c->can.transmit = can_transmit;
    c->can.set_rx_callback = can_set_rx_callback;
    g_can = c;
    return FF_SUCCESS;
}
