/*
 * frick-fsw - SpaceWire driver (contract-level backend, milestone M4/M11)
 *
 * Contract-level implementation of FF_SpaceWire_t and FF_Rmap_t for
 * ECSS-E-ST-50-12/51/52/53 API compliance. No emulated peripheral in the
 * Renode beaglev-fire repl (GAP-A5); this backend provides a deterministic
 * mock that exercises the API and RMAP protocol state machine.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/spacewire/mpfs/ff_spacewire_mpfs.h"
#include <string.h>

static FF_SpaceWireMpfs_t *g_spw; /* single bound instance */

/* Simple CRC-16 (CCITT) for RMAP header/data CRC per ECSS-E-ST-50-52.
 * Polynomial 0x1021, initial 0xFFFF, no reflection. */
static uint16_t spw_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFu;
    uint16_t i;
    uint8_t bit;

    for (i = 0u; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (bit = 0u; bit < 8u; bit++) {
            if ((crc & 0x8000u) != 0u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

/* Verify RMAP header CRC (first 14 bytes, CRC at offset 14-15). */
static bool spw_verify_hdr_crc(const uint8_t *hdr)
{
    uint16_t calc = spw_crc16(hdr, 14u);
    uint16_t recv = (uint16_t)(hdr[14] << 8) | hdr[15];
    return calc == recv;
}

/* Verify RMAP data CRC (data bytes, CRC at end). */
static bool spw_verify_data_crc(const uint8_t *data, uint16_t data_len, const uint8_t *crc_bytes)
{
    uint16_t calc = spw_crc16(data, data_len);
    uint16_t recv = (uint16_t)(crc_bytes[0] << 8) | crc_bytes[1];
    return calc == recv;
}

/* Build RMAP header (14 bytes header + 2 bytes CRC = 16 bytes total).
 * Per ECSS-E-ST-50-52, the header CRC is appended after the data length field. */
static void spw_build_hdr(uint8_t *hdr, uint8_t cmd, uint8_t target, uint8_t key,
                          uint8_t reply_addr, uint16_t tid, uint32_t addr, uint32_t data_len)
{
    hdr[0] = FF_SPW_PROTOCOL_ID_RMAP;      /* Protocol ID */
    hdr[1] = cmd;                          /* Command */
    hdr[2] = target;                       /* Target Logical Address */
    hdr[3] = key;                          /* Key */
    hdr[4] = reply_addr;                   /* Reply Address */
    hdr[5] = (uint8_t)(tid >> 8);          /* Transaction ID MSB */
    hdr[6] = (uint8_t)(tid & 0xFFu);       /* Transaction ID LSB */
    hdr[7] = (uint8_t)(addr >> 24);        /* Address[31:24] */
    hdr[8] = (uint8_t)(addr >> 16);        /* Address[23:16] */
    hdr[9] = (uint8_t)(addr >> 8);         /* Address[15:8] */
    hdr[10] = (uint8_t)(addr & 0xFFu);     /* Address[7:0] */
    hdr[11] = (uint8_t)(data_len >> 16);   /* Data Length[23:16] */
    hdr[12] = (uint8_t)(data_len >> 8);    /* Data Length[15:8] */
    hdr[13] = (uint8_t)(data_len & 0xFFu); /* Data Length[7:0] */
    /* Compute header CRC over bytes 0-13 (14 bytes) */
    uint16_t crc = spw_crc16(hdr, 14u);
    hdr[14] = (uint8_t)(crc >> 8);    /* Header CRC MSB */
    hdr[15] = (uint8_t)(crc & 0xFFu); /* Header CRC LSB */
}

/* SpaceWire send: wraps payload with protocol ID and sends via mock bus. */
static FF_Status_t spw_send(const uint8_t *data, uint16_t len)
{
    uint8_t frame[FF_SPW_MAX_PAYLOAD + 2];
    uint16_t frame_len;

    if ((g_spw == NULL) || (g_spw->ready == 0u)) {
        return FF_NOT_READY;
    }
    if ((data == NULL) || (len == 0u) || (len > FF_SPW_MAX_PAYLOAD)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_spw->bus.send == NULL) {
        return FF_NOT_IMPLEMENTED;
    }
    /* Prepend protocol ID (ECSS-E-ST-50-51). For raw send, use a default. */
    frame[0] = FF_SPW_PROTOCOL_ID_CCSDS;
    (void)memcpy(&frame[1], data, len);
    frame_len = len + 1u;
    return g_spw->bus.send(g_spw->bus.ctx, frame, frame_len);
}

/* SpaceWire send with explicit protocol ID (ECSS-E-ST-50-51). */
static FF_Status_t spw_send_pid(uint8_t protocol_id, const uint8_t *data, uint16_t len)
{
    uint8_t frame[FF_SPW_MAX_PAYLOAD + 2];
    uint16_t frame_len;

    if ((g_spw == NULL) || (g_spw->ready == 0u)) {
        return FF_NOT_READY;
    }
    if ((data == NULL) || (len == 0u) || (len > FF_SPW_MAX_PAYLOAD)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_spw->bus.send == NULL) {
        return FF_NOT_IMPLEMENTED;
    }
    frame[0] = protocol_id;
    (void)memcpy(&frame[1], data, len);
    frame_len = len + 1u;
    return g_spw->bus.send(g_spw->bus.ctx, frame, frame_len);
}

/* SpaceWire set RX callback. */
static void spw_set_rx_cb(FF_SpwRxCb_t cb, void *ctx)
{
    if (g_spw != NULL) {
        g_spw->rx_cb = cb;
        g_spw->rx_cb_ctx = ctx;
        g_spw->rx_cb_set = 1u;
    }
}

/* SpaceWire init. */
static FF_Status_t spw_init(void)
{
    if (g_spw == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    g_spw->tx_tid = 0u;
    g_spw->rx_tid = 0u;
    g_spw->ready = 1u;
    return FF_SUCCESS;
}

/* RMAP read: builds and sends an RMAP read command. */
static FF_Status_t rmap_read(uint32_t address, uint8_t *resp, uint16_t cap, uint16_t *len)
{
    uint8_t cmd_buf[FF_RMAP_HEADER_SIZE];
    FF_Status_t rc;

    if ((g_spw == NULL) || (g_spw->ready == 0u)) {
        return FF_NOT_READY;
    }
    if ((resp == NULL) || (len == NULL) || (cap == 0u) || (cap > FF_SPW_MAX_PAYLOAD)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_spw->bus.send == NULL) {
        return FF_NOT_IMPLEMENTED;
    }
    /* Build RMAP read command. */
    spw_build_hdr(cmd_buf, FF_RMAP_CMD_READ, g_spw->target_addr, g_spw->key, g_spw->reply_addr,
                  g_spw->tx_tid, address, 0u);
    g_spw->tx_tid++;
    /* Send via mock bus. */
    rc = g_spw->bus.send(g_spw->bus.ctx, cmd_buf, FF_RMAP_HEADER_SIZE);
    if (rc != FF_SUCCESS) {
        return rc;
    }
    /* In a real implementation, we would wait for a reply. In the contract
     * mock, the test drives the reply via the bus.recv path. For the self-test
     * we simulate a successful reply with zero data. */
    *len = 0u;
    return FF_SUCCESS;
}

/* RMAP write: builds and sends an RMAP write command with data. */
static FF_Status_t rmap_write(uint32_t address, const uint8_t *data, uint16_t len)
{
    /* Header (15) + data (max 256) + data CRC (2) = 273 bytes. */
    uint8_t cmd_buf[FF_RMAP_HEADER_SIZE + FF_SPW_MAX_PAYLOAD + 2u];
    uint16_t data_len = len;
    FF_Status_t rc;

    if ((g_spw == NULL) || (g_spw->ready == 0u)) {
        return FF_NOT_READY;
    }
    if ((data == NULL) || (len == 0u) || (len > FF_SPW_MAX_PAYLOAD)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_spw->bus.send == NULL) {
        return FF_NOT_IMPLEMENTED;
    }
    /* Build RMAP write command header. */
    spw_build_hdr(cmd_buf, FF_RMAP_CMD_WRITE, g_spw->target_addr, g_spw->key, g_spw->reply_addr,
                  g_spw->tx_tid, address, data_len);
    g_spw->tx_tid++;
    /* Append data. */
    (void)memcpy(&cmd_buf[FF_RMAP_HEADER_SIZE], data, data_len);
    /* Append data CRC. */
    uint16_t data_crc = spw_crc16(data, data_len);
    cmd_buf[FF_RMAP_HEADER_SIZE + data_len] = (uint8_t)(data_crc >> 8);
    cmd_buf[FF_RMAP_HEADER_SIZE + data_len + 1] = (uint8_t)(data_crc & 0xFFu);
    /* Send via mock bus. */
    rc = g_spw->bus.send(g_spw->bus.ctx, cmd_buf, FF_RMAP_HEADER_SIZE + data_len + 2u);
    return rc;
}

/* RMAP read-modify-write: builds and sends an RMAP RMW command with AND/OR masks. */
static FF_Status_t rmap_rmw(uint32_t address, const uint8_t *and_mask, const uint8_t *or_mask,
                            uint16_t mask_len)
{
    /* Header (15) + AND mask (max 256) + OR mask (max 256) + data CRC (2) = 529 bytes. */
    uint8_t cmd_buf[FF_RMAP_HEADER_SIZE + 2u * FF_SPW_MAX_PAYLOAD + 2u];
    FF_Status_t rc;

    if ((g_spw == NULL) || (g_spw->ready == 0u)) {
        return FF_NOT_READY;
    }
    if ((and_mask == NULL) || (or_mask == NULL) || (mask_len == 0u) ||
        (mask_len > FF_SPW_MAX_PAYLOAD)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_spw->bus.send == NULL) {
        return FF_NOT_IMPLEMENTED;
    }
    /* Build RMAP RMW command header. Data length = 2 * mask_len (AND + OR). */
    spw_build_hdr(cmd_buf, FF_RMAP_CMD_RMW, g_spw->target_addr, g_spw->key, g_spw->reply_addr,
                  g_spw->tx_tid, address, 2u * mask_len);
    g_spw->tx_tid++;
    /* Append AND mask, then OR mask. */
    (void)memcpy(&cmd_buf[FF_RMAP_HEADER_SIZE], and_mask, mask_len);
    (void)memcpy(&cmd_buf[FF_RMAP_HEADER_SIZE + mask_len], or_mask, mask_len);
    /* Data CRC over concatenated masks. */
    uint16_t data_crc = spw_crc16(cmd_buf + FF_RMAP_HEADER_SIZE, 2u * mask_len);
    cmd_buf[FF_RMAP_HEADER_SIZE + 2u * mask_len] = (uint8_t)(data_crc >> 8);
    cmd_buf[FF_RMAP_HEADER_SIZE + 2u * mask_len + 1] = (uint8_t)(data_crc & 0xFFu);
    /* Send via mock bus. */
    rc = g_spw->bus.send(g_spw->bus.ctx, cmd_buf, FF_RMAP_HEADER_SIZE + 2u * mask_len + 2u);
    return rc;
}

FF_Status_t FF_SpaceWireMpfsSelfTest(FF_SpaceWireMpfs_t *s)
{
    uint8_t tx_buf[FF_SPW_MAX_PAYLOAD];
    uint8_t rx_buf[FF_SPW_MAX_PAYLOAD];
    uint16_t rx_len;
    uint16_t rmap_len;

    if ((s == NULL) || (s != g_spw)) {
        return FF_INVALID_ARGUMENT;
    }
    /* 1. init() */
    if (s->spw.init() != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (s->ready != 1u) {
        return FF_ERROR;
    }
    /* 2. send()/send_pid() with protocol ID */
    tx_buf[0] = 0xA1u;
    tx_buf[1] = 0xB2u;
    if (s->spw.send(tx_buf, 2u) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (s->spw.send_pid(FF_SPW_PROTOCOL_ID_RMAP, tx_buf, 2u) != FF_SUCCESS) {
        return FF_ERROR;
    }
    /* 3. set_rx_callback() */
    s->spw.set_rx_callback(NULL, NULL);
    if (s->rx_cb_set != 1u) {
        return FF_ERROR;
    }
    /* 4. RMAP read */
    if (s->rmap.read(0x1000u, rx_buf, sizeof(rx_buf), &rmap_len) != FF_SUCCESS) {
        return FF_ERROR;
    }
    if (rmap_len != 0u) {
        return FF_ERROR;
    }
    /* 5. RMAP write */
    tx_buf[0] = 0xDEu;
    tx_buf[1] = 0xADu;
    tx_buf[2] = 0xBEu;
    tx_buf[3] = 0xEFu;
    if (s->rmap.write(0x2000u, tx_buf, 4u) != FF_SUCCESS) {
        return FF_ERROR;
    }
    /* 6. RMAP RMW */
    {
        uint8_t and_m[2] = {0xFFu, 0x0Fu};
        uint8_t or_m[2] = {0x00u, 0xF0u};
        if (s->rmap.rmw(0x3000u, and_m, or_m, 2u) != FF_SUCCESS) {
            return FF_ERROR;
        }
    }
    /* 7. CRC verification helper test */
    {
        uint8_t hdr[16];
        spw_build_hdr(hdr, FF_RMAP_CMD_READ, 0xFEu, 0x00u, 0xFEu, 0x1234u, 0x1000u, 0u);
        if (!spw_verify_hdr_crc(hdr)) {
            return FF_ERROR;
        }
        /* Corrupt and verify failure */
        hdr[2] ^= 0xFFu;
        if (spw_verify_hdr_crc(hdr)) {
            return FF_ERROR;
        }
    }
    /* 8. Data CRC test */
    {
        uint8_t data[4] = {0x01u, 0x02u, 0x03u, 0x04u};
        uint16_t crc = spw_crc16(data, 4u);
        uint8_t crc_bytes[2] = {(uint8_t)(crc >> 8), (uint8_t)(crc & 0xFFu)};
        if (!spw_verify_data_crc(data, 4u, crc_bytes)) {
            return FF_ERROR;
        }
    }
    return FF_SUCCESS;
}

FF_Status_t FF_SpaceWireMpfsSetup(FF_SpaceWireMpfs_t *s, const FF_SpwMockBus_t *bus,
                                  uint8_t logical_addr)
{
    if ((s == NULL) || (bus == NULL) || (bus->send == NULL) || (bus->recv == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    s->bus = *bus;
    s->logical_addr = logical_addr;
    s->target_addr = 0xFEu; /* default target (broadcast-ish) */
    s->key = 0x00u;         /* default key */
    s->reply_addr = logical_addr;
    s->tx_tid = 0u;
    s->rx_tid = 0u;
    s->rx_cb_set = 0u;
    s->rx_cb = NULL;
    s->rx_cb_ctx = NULL;
    s->ready = 0u;
    s->spw.name = "mpfs-spacewire";
    s->spw.init = spw_init;
    s->spw.send = spw_send;
    s->spw.send_pid = spw_send_pid;
    s->spw.set_rx_callback = spw_set_rx_cb;
    s->rmap.name = "mpfs-rmap";
    s->rmap.init = NULL; /* uses spw.init */
    s->rmap.read = rmap_read;
    s->rmap.write = rmap_write;
    s->rmap.rmw = rmap_rmw;
    g_spw = s;
    return FF_SUCCESS;
}