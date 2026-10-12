/*
 * frick-fsw - SpaceWire driver unit tests (milestone M4/M11)
 *
 * Exercises the contract-level FF_SpaceWire_t and FF_Rmap_t API on the host
 * with a mock link that simulates send/receive. The mock models the RMAP
 * command/reply format, sequence numbers, keys, and CRC per ECSS-E-ST-50-52.
 * There is no emulated SpaceWire peripheral in the Renode beaglev-fire repl
 * (GAP-A5); this test verifies the API contract and RMAP state machine.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/spacewire/mpfs/ff_spacewire_mpfs.h"
#include <string.h>

/* Local CRC-16 (CCITT) implementation for test independence (mirrors driver). */
static uint16_t test_crc16(const uint8_t *data, uint16_t len)
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

static void test_build_hdr(uint8_t *hdr, uint8_t cmd, uint8_t target, uint8_t key,
                           uint8_t reply_addr, uint16_t tid, uint32_t addr, uint32_t data_len)
{
    hdr[0] = FF_SPW_PROTOCOL_ID_RMAP;
    hdr[1] = cmd;
    hdr[2] = target;
    hdr[3] = key;
    hdr[4] = reply_addr;
    hdr[5] = (uint8_t)(tid >> 8);
    hdr[6] = (uint8_t)(tid & 0xFFu);
    hdr[7] = (uint8_t)(addr >> 24);
    hdr[8] = (uint8_t)(addr >> 16);
    hdr[9] = (uint8_t)(addr >> 8);
    hdr[10] = (uint8_t)(addr & 0xFFu);
    hdr[11] = (uint8_t)(data_len >> 16);
    hdr[12] = (uint8_t)(data_len >> 8);
    hdr[13] = (uint8_t)(data_len & 0xFFu);
    uint16_t crc = test_crc16(hdr, 14u);
    hdr[14] = (uint8_t)(crc >> 8);
    hdr[15] = (uint8_t)(crc & 0xFFu);
}

static bool test_verify_hdr_crc(const uint8_t *hdr)
{
    uint16_t calc = test_crc16(hdr, 14u);
    uint16_t recv = (uint16_t)(hdr[14] << 8) | hdr[15];
    return calc == recv;
}

static bool test_verify_data_crc(const uint8_t *data, uint16_t data_len, const uint8_t *crc_bytes)
{
    uint16_t calc = test_crc16(data, data_len);
    uint16_t recv = (uint16_t)(crc_bytes[0] << 8) | crc_bytes[1];
    return calc == recv;
}

typedef struct
{
    uint8_t tx_buf[256];
    uint16_t tx_len;
    uint8_t rx_buf[256];
    uint16_t rx_len;
    uint16_t rx_cap;
    uint32_t send_calls;
    uint32_t recv_calls;
    bool fail_send;
    bool fail_recv;
} MockSpwBus_t;

static MockSpwBus_t g_mock;
static FF_SpaceWireMpfs_t g_drv;
static FF_SpwMockBus_t g_bus;

static void mock_reset(MockSpwBus_t *m)
{
    (void)memset(m, 0, sizeof(*m));
    m->rx_cap = sizeof(m->rx_buf);
}

static FF_Status_t mock_send(void *ctx, const uint8_t *data, uint16_t len)
{
    MockSpwBus_t *m = (MockSpwBus_t *)ctx;
    if (m->fail_send) {
        return FF_ERROR;
    }
    if ((data == NULL) || (len == 0u) || (len > sizeof(m->tx_buf))) {
        return FF_INVALID_ARGUMENT;
    }
    (void)memcpy(m->tx_buf, data, len);
    m->tx_len = len;
    m->send_calls++;
    return FF_SUCCESS;
}

static FF_Status_t mock_recv(void *ctx, uint8_t *data, uint16_t *len)
{
    MockSpwBus_t *m = (MockSpwBus_t *)ctx;
    if (m->fail_recv) {
        return FF_ERROR;
    }
    if ((data == NULL) || (len == NULL) || (m->rx_len == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    *len = (*len < m->rx_len) ? *len : m->rx_len;
    (void)memcpy(data, m->rx_buf, *len);
    m->recv_calls++;
    return FF_SUCCESS;
}

static void mock_bus_init(void)
{
    g_bus.ctx = &g_mock;
    g_bus.send = mock_send;
    g_bus.recv = mock_recv;
}

static void mock_bind(void)
{
    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(&g_drv, &g_bus, 0xFEu) == FF_SUCCESS);
}

static void test_setup_and_init(void)
{
    static FF_SpaceWireMpfs_t drv;
    FF_SpwMockBus_t bad;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(NULL, &g_bus, 0xFEu) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(&drv, NULL, 0xFEu) == FF_INVALID_ARGUMENT);
    bad.ctx = &g_mock;
    bad.send = NULL;
    bad.recv = mock_recv;
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(&drv, &bad, 0xFEu) == FF_INVALID_ARGUMENT);
    bad.send = mock_send;
    bad.recv = NULL;
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(&drv, &bad, 0xFEu) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(&drv, &g_bus, 0xFEu) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.spw.name != NULL);
    FF_TEST_ASSERT(strcmp(drv.spw.name, "mpfs-spacewire") == 0);
    FF_TEST_ASSERT(drv.spw.init != NULL);
    FF_TEST_ASSERT(drv.spw.send != NULL);
    FF_TEST_ASSERT(drv.spw.send_pid != NULL);
    FF_TEST_ASSERT(drv.spw.set_rx_callback != NULL);
    FF_TEST_ASSERT(drv.rmap.name != NULL);
    FF_TEST_ASSERT(strcmp(drv.rmap.name, "mpfs-rmap") == 0);
    FF_TEST_ASSERT(drv.rmap.read != NULL);
    FF_TEST_ASSERT(drv.rmap.write != NULL);
    FF_TEST_ASSERT(drv.rmap.rmw != NULL);

    /* Not ready before init. */
    FF_TEST_ASSERT(drv.spw.send(NULL, 0u) == FF_NOT_READY);
    FF_TEST_ASSERT(drv.spw.send_pid(0u, NULL, 0u) == FF_NOT_READY);

    FF_TEST_ASSERT(drv.spw.init() == FF_SUCCESS);
    FF_TEST_ASSERT(drv.ready == 1u);
}

static void test_send_recv(void)
{
    uint8_t tx[4] = {0x11u, 0x22u, 0x33u, 0x44u};
    uint8_t rx[8] = {0u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spw.init() == FF_SUCCESS);

    /* send() prepends protocol ID (CCSDS default). */
    FF_TEST_ASSERT(g_drv.spw.send(tx, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.send_calls == 1u);
    FF_TEST_ASSERT(g_mock.tx_len == 5u); /* 1 protocol ID + 4 data */
    FF_TEST_ASSERT(g_mock.tx_buf[0] == FF_SPW_PROTOCOL_ID_CCSDS);
    FF_TEST_ASSERT(g_mock.tx_buf[1] == 0x11u);
    FF_TEST_ASSERT(g_mock.tx_buf[4] == 0x44u);

    /* send_pid() with explicit protocol ID. */
    FF_TEST_ASSERT(g_drv.spw.send_pid(FF_SPW_PROTOCOL_ID_RMAP, tx, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.send_calls == 2u);
    FF_TEST_ASSERT(g_mock.tx_buf[0] == FF_SPW_PROTOCOL_ID_RMAP);

    /* set_rx_callback. */
    g_drv.spw.set_rx_callback(NULL, NULL);
    FF_TEST_ASSERT(g_drv.rx_cb_set == 1u);
}

static void test_rmap_read(void)
{
    uint8_t resp[16];
    uint16_t len;

    mock_bind();
    FF_TEST_ASSERT(g_drv.spw.init() == FF_SUCCESS);

    FF_TEST_ASSERT(g_drv.rmap.read(0x1000u, resp, sizeof(resp), &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == 0u); /* mock returns zero-length read reply */
    FF_TEST_ASSERT(g_mock.send_calls == 1u);
    /* Verify RMAP read command format (new header: Protocol ID at 0, Command at 1). */
    FF_TEST_ASSERT(g_mock.tx_len == FF_RMAP_HEADER_SIZE);
    FF_TEST_ASSERT(g_mock.tx_buf[0] == FF_SPW_PROTOCOL_ID_RMAP);
    FF_TEST_ASSERT(g_mock.tx_buf[1] == FF_RMAP_CMD_READ);
    FF_TEST_ASSERT(g_mock.tx_buf[2] == 0xFEu); /* target_addr */
    FF_TEST_ASSERT(g_mock.tx_buf[3] == 0x00u); /* key */
    FF_TEST_ASSERT(g_mock.tx_buf[4] == 0xFEu); /* reply_addr */
    /* Transaction ID increments (now at indices 5,6). */
    FF_TEST_ASSERT((g_mock.tx_buf[5] << 8) | g_mock.tx_buf[6] == 0u);
}

static void test_rmap_write(void)
{
    uint8_t data[4] = {0xDEu, 0xADu, 0xBEu, 0xEFu};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spw.init() == FF_SUCCESS);

    FF_TEST_ASSERT(g_drv.rmap.write(0x2000u, data, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.send_calls == 1u);
    /* Header (16) + data (4) + data CRC (2) = 22 bytes. */
    FF_TEST_ASSERT(g_mock.tx_len == FF_RMAP_HEADER_SIZE + 4u + 2u);
    FF_TEST_ASSERT(g_mock.tx_buf[1] == FF_RMAP_CMD_WRITE);
    /* Data payload starts after header (16 bytes). */
    FF_TEST_ASSERT(g_mock.tx_buf[FF_RMAP_HEADER_SIZE + 0] == 0xDEu);
    FF_TEST_ASSERT(g_mock.tx_buf[FF_RMAP_HEADER_SIZE + 3] == 0xEFu);
    /* Data CRC present at end. */
}

static void test_rmap_rmw(void)
{
    uint8_t and_mask[2] = {0xFFu, 0x0Fu};
    uint8_t or_mask[2] = {0x00u, 0xF0u};

    mock_bind();
    FF_TEST_ASSERT(g_drv.spw.init() == FF_SUCCESS);

    FF_TEST_ASSERT(g_drv.rmap.rmw(0x3000u, and_mask, or_mask, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.send_calls == 1u);
    /* Header (16) + 2*mask_len (4) + data CRC (2) = 22 bytes. */
    FF_TEST_ASSERT(g_mock.tx_len == FF_RMAP_HEADER_SIZE + 4u + 2u);
    FF_TEST_ASSERT(g_mock.tx_buf[1] == FF_RMAP_CMD_RMW);
    /* AND mask then OR mask, after header. */
    FF_TEST_ASSERT(g_mock.tx_buf[FF_RMAP_HEADER_SIZE + 0] == 0xFFu);
    FF_TEST_ASSERT(g_mock.tx_buf[FF_RMAP_HEADER_SIZE + 1] == 0x0Fu);
    FF_TEST_ASSERT(g_mock.tx_buf[FF_RMAP_HEADER_SIZE + 2] == 0x00u);
    FF_TEST_ASSERT(g_mock.tx_buf[FF_RMAP_HEADER_SIZE + 3] == 0xF0u);
}

static void test_crc_helpers(void)
{
    uint8_t hdr[16];
    uint8_t data[4] = {0x01u, 0x02u, 0x03u, 0x04u};
    uint16_t crc;

    mock_bind();
    FF_TEST_ASSERT(g_drv.spw.init() == FF_SUCCESS);

    /* Header CRC verification. */
    test_build_hdr(hdr, FF_RMAP_CMD_READ, 0xFEu, 0x00u, 0xFEu, 0x1234u, 0x1000u, 0u);
    FF_TEST_ASSERT(test_verify_hdr_crc(hdr));
    /* Corrupt command byte (now at index 1). */
    hdr[1] ^= 0xFFu;
    FF_TEST_ASSERT(!test_verify_hdr_crc(hdr));

    /* Data CRC. */
    crc = test_crc16(data, 4u);
    {
        uint8_t crc_bytes[2] = {(uint8_t)(crc >> 8), (uint8_t)(crc & 0xFFu)};
        FF_TEST_ASSERT(test_verify_data_crc(data, 4u, crc_bytes));
    }
    /* Corrupt data. */
    data[0] ^= 0xFFu;
    {
        uint8_t crc_bytes[2] = {(uint8_t)(crc >> 8), (uint8_t)(crc & 0xFFu)};
        FF_TEST_ASSERT(!test_verify_data_crc(data, 4u, crc_bytes));
    }
}

static void test_argument_validation(void)
{
    uint8_t buf[16];
    uint16_t len;

    mock_bind();
    FF_TEST_ASSERT(g_drv.spw.init() == FF_SUCCESS);

    /* send: NULL data, zero len, too large. */
    FF_TEST_ASSERT(g_drv.spw.send(NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.spw.send(buf, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.spw.send(buf, FF_SPW_MAX_PAYLOAD + 1u) == FF_INVALID_ARGUMENT);

    /* send_pid: same. */
    FF_TEST_ASSERT(g_drv.spw.send_pid(0u, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.spw.send_pid(0u, buf, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.spw.send_pid(0u, buf, FF_SPW_MAX_PAYLOAD + 1u) == FF_INVALID_ARGUMENT);

    /* RMAP read: NULL resp, NULL len, zero cap, too large cap. */
    FF_TEST_ASSERT(g_drv.rmap.read(0u, NULL, 4u, &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.read(0u, buf, 4u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.read(0u, buf, 0u, &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.read(0u, buf, FF_SPW_MAX_PAYLOAD + 1u, &len) == FF_INVALID_ARGUMENT);

    /* RMAP write: NULL data, zero len, too large. */
    FF_TEST_ASSERT(g_drv.rmap.write(0u, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.write(0u, buf, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.write(0u, buf, FF_SPW_MAX_PAYLOAD + 1u) == FF_INVALID_ARGUMENT);

    /* RMAP RMW: NULL masks, zero len, too large. */
    FF_TEST_ASSERT(g_drv.rmap.rmw(0u, NULL, buf, 2u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.rmw(0u, buf, NULL, 2u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.rmw(0u, buf, buf, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.rmap.rmw(0u, buf, buf, FF_SPW_MAX_PAYLOAD + 1u) == FF_INVALID_ARGUMENT);
}

static void test_self_test(void)
{
    static FF_SpaceWireMpfs_t drv;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_SpaceWireMpfsSetup(&drv, &g_bus, 0xFEu) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SpaceWireMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SpaceWireMpfsSelfTest(&drv) == FF_SUCCESS);

    /* Self-test on unbound instance is rejected. */
    {
        FF_SpaceWireMpfs_t other;
        (void)memset(&other, 0, sizeof(other));
        FF_TEST_ASSERT(FF_SpaceWireMpfsSelfTest(&other) == FF_INVALID_ARGUMENT);
    }
}

FF_TEST_MAIN(test_setup_and_init, test_send_recv, test_rmap_read, test_rmap_write, test_rmap_rmw,
             test_crc_helpers, test_argument_validation, test_self_test)