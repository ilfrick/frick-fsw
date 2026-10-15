/*
 * frick-fsw - Unit tests for link_tsn backend (SRS-REQ-061)
 *
 * Tests the TSN-scheduled Ethernet link backend with a mock MAC and
 * mock TSN backend. Verifies Ethernet frame construction on send, RX
 * callback buffering on recv, ring-full drop, and EtherType filtering.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "transport/link/link_tsn.h"
#include "transport/ccsds/ccsds.h"
#include "hal/hal_eth.h"
#include "hal/hal_tsn.h"
#include "pus/pus.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include <string.h>

/* ---- Mock MAC ---- */

#define MOCK_MAC_ADDR                                                                              \
    {                                                                                              \
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55                                                         \
    }
#define MOCK_RX_BUF 256u

static uint8_t g_captured_frame[MOCK_RX_BUF];
static uint16_t g_captured_len;
static FF_EthRxCb_t g_rx_cb;
static void *g_rx_ctx;
static bool g_mock_echo; /* when true, transmit loops the frame back to the RX callback */

static FF_Status_t mock_mac_init(void *self)
{
    (void)self;
    g_captured_len = 0u;
    return FF_SUCCESS;
}

static FF_Status_t mock_mac_get_mac(void *self, uint8_t out[6])
{
    (void)self;
    uint8_t mac[] = MOCK_MAC_ADDR;
    (void)memcpy(out, mac, 6u);
    return FF_SUCCESS;
}

static FF_Status_t mock_mac_set_link_up(void *self, uint8_t up)
{
    (void)self;
    (void)up;
    return FF_SUCCESS;
}

static FF_Status_t mock_mac_transmit(void *self, const uint8_t *frame, uint16_t len)
{
    (void)self;
    if (len > MOCK_RX_BUF) {
        return FF_BAD_SIZE;
    }
    (void)memcpy(g_captured_frame, frame, len);
    g_captured_len = len;
    /* Optional loop-back: deliver the frame synchronously to the registered RX
     * callback. Invariant this relies on (see src/transport/link/link_tsn.c):
     * tsn_link_send does NOT hold the link lock while calling transmit, and
     * tsn_eth_rx_cb takes a non-blocking try-lock. If either becomes blocking,
     * this echo would self-deadlock. */
    if (g_mock_echo && (g_rx_cb != NULL)) {
        g_rx_cb(frame, len, g_rx_ctx);
    }
    return FF_SUCCESS;
}

static void mock_mac_set_rx_cb(void *self, FF_EthRxCb_t cb, void *ctx)
{
    (void)self;
    g_rx_cb = cb;
    g_rx_ctx = ctx;
}

static FF_Status_t mock_mac_get_stats(void *self, uint32_t *tx, uint32_t *rx, uint32_t *err)
{
    (void)self;
    if (tx != NULL) {
        *tx = 0u;
    }
    if (rx != NULL) {
        *rx = 0u;
    }
    if (err != NULL) {
        *err = 0u;
    }
    return FF_SUCCESS;
}

static FF_EthMac_t g_mock_mac = {
    .name = "mock_mac",
    .mtu = 1500u,
    .init = mock_mac_init,
    .get_mac_address = mock_mac_get_mac,
    .set_link_up = mock_mac_set_link_up,
    .transmit = mock_mac_transmit,
    .set_rx_callback = mock_mac_set_rx_cb,
    .get_stats = mock_mac_get_stats,
};

/* ---- Mock TSN ---- */

static uint16_t g_mock_vid = 0;
static uint8_t g_mock_pcp = 0;
static FF_Status_t mock_tsn_set_vlan(void *self, uint16_t vid, uint8_t pcp)
{
    (void)self;
    g_mock_vid = vid;
    g_mock_pcp = pcp;
    return FF_SUCCESS;
}

static FF_TsnBackend_t g_mock_tsn = {
    .name = "mock_tsn",
    .init = NULL,
    .set_vlan = mock_tsn_set_vlan,
    .configure_tas = NULL,
    .configure_cbs = NULL,
    .set_stream_filter = NULL,
    .enable_pfc = NULL,
    .get_time_ns = NULL,
};

/* ---- Helpers ---- */

static void reset_mocks(void)
{
    g_captured_len = 0u;
    g_rx_cb = NULL;
    g_rx_ctx = NULL;
    g_mock_vid = 0u;
    g_mock_pcp = 0u;
    g_mock_echo = false;
    memset(g_captured_frame, 0, sizeof(g_captured_frame));
}

static void simulate_rx(const uint8_t *eth_frame, uint16_t eth_len)
{
    /* Simulate the MAC delivering an Ethernet frame to the callback. */
    if ((g_rx_cb != NULL) && (eth_len >= FF_LINK_TSN_HDR_LEN)) {
        g_rx_cb(eth_frame, eth_len, g_rx_ctx);
    }
}

static void build_eth_frame(uint8_t *out, uint16_t *out_len, const uint8_t dest[6],
                            const uint8_t src[6], uint16_t etype, const uint8_t *payload,
                            uint16_t payload_len)
{
    uint16_t pos = 0u;
    memcpy(&out[pos], dest, 6u);
    pos += 6u;
    memcpy(&out[pos], src, 6u);
    pos += 6u;
    out[pos] = (uint8_t)((etype >> 8) & 0xFFu);
    out[pos + 1u] = (uint8_t)(etype & 0xFFu);
    pos += 2u;
    memcpy(&out[pos], payload, payload_len);
    pos += payload_len;
    *out_len = pos;
}

/* ---- Tests ---- */

static void test_tsn_init_success(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    FF_Status_t st = FF_LinkTsnInit(&cfg);
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* Select and init the backend. */
    st = FF_LinkSelect(&FF_LinkTsnBackend);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock_vid == FF_LINK_TSN_DEFAULT_VID);
    FF_TEST_ASSERT(g_mock_pcp == FF_LINK_TSN_DEFAULT_PCP);
    FF_TEST_ASSERT(g_rx_cb != NULL);
}

static void test_tsn_send_eth_frame(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    uint8_t payload[] = {0xAA, 0xBB, 0xCC, 0xDD};
    FF_Status_t st = FF_LinkSend(payload, sizeof(payload));
    FF_TEST_ASSERT(st == FF_SUCCESS);
    /* The frame is padded to minimum Ethernet size (60 bytes). */
    FF_TEST_ASSERT(g_captured_len >= (uint16_t)(FF_LINK_TSN_HDR_LEN + sizeof(payload)));

    /* Verify destination MAC (broadcast). */
    uint8_t expected_dest[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    FF_TEST_ASSERT(memcmp(&g_captured_frame[0], expected_dest, 6u) == 0);
    /* Verify source MAC (from mock). */
    uint8_t expected_src[6] = MOCK_MAC_ADDR;
    FF_TEST_ASSERT(memcmp(&g_captured_frame[6], expected_src, 6u) == 0);
    /* Verify EtherType (big-endian 0x88F5). */
    FF_TEST_ASSERT(g_captured_frame[12] == 0x88u);
    FF_TEST_ASSERT(g_captured_frame[13] == 0xF5u);
    /* Verify payload at offset 14. */
    FF_TEST_ASSERT(memcmp(&g_captured_frame[14], payload, sizeof(payload)) == 0);
    /* Verify padding is zeros (if present). */
    if (g_captured_len > (uint16_t)(FF_LINK_TSN_HDR_LEN + sizeof(payload))) {
        for (uint16_t i = (uint16_t)(FF_LINK_TSN_HDR_LEN + sizeof(payload)); i < g_captured_len;
             i++) {
            FF_TEST_ASSERT(g_captured_frame[i] == 0x00u);
        }
    }
}

static void test_tsn_recv_after_rx_cb(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    /* Simulate RX of an Ethernet frame carrying our EtherType. */
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t eth_frame[64];
    uint16_t eth_len = 0u;
    build_eth_frame(eth_frame, &eth_len, (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                    (uint8_t[]){0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}, FF_LINK_TSN_ETHERTYPE, payload,
                    sizeof(payload));

    simulate_rx(eth_frame, eth_len);

    uint8_t rx_buf[64];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkRecv(rx_buf, sizeof(rx_buf), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(payload));
    FF_TEST_ASSERT(memcmp(rx_buf, payload, sizeof(payload)) == 0);
}

static void test_tsn_recv_wrong_etype_dropped(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    /* Simulate RX of an Ethernet frame with a wrong EtherType. */
    uint8_t payload[] = {0x01, 0x02};
    uint8_t eth_frame[64];
    uint16_t eth_len = 0u;
    build_eth_frame(eth_frame, &eth_len, (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                    (uint8_t[]){0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
                    0x0800u, /* IPv4 — wrong type */
                    payload, sizeof(payload));

    simulate_rx(eth_frame, eth_len);

    uint8_t rx_buf[64];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkRecv(rx_buf, sizeof(rx_buf), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_TIMEOUT); /* nothing in the ring */
}

static void test_tsn_recv_null_args(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    uint8_t buf[8];
    uint16_t len = 0u;
    /* NULL data → INVALID_ARGUMENT (checked before lock). */
    FF_Status_t st = FF_LinkTsnBackend.recv(NULL, sizeof(buf), &len, 0u);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
    /* NULL len → INVALID_ARGUMENT. */
    st = FF_LinkTsnBackend.recv(buf, sizeof(buf), NULL, 0u);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_tsn_init_null_config(void)
{
    FF_Status_t st = FF_LinkTsnInit(NULL);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_tsn_init_null_mac(void)
{
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(NULL, &g_mock_tsn);
    FF_Status_t st = FF_LinkTsnInit(&cfg);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_tsn_send_too_large(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    uint8_t big[FF_LINK_TSN_RX_FRAMESZ + 1u];
    FF_Status_t st = FF_LinkSend(big, sizeof(big));
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_tsn_ring_full_drop(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    /* Fill the ring buffer (16 slots). */
    uint8_t payload = 0x42;
    uint8_t eth_frame[64];
    uint16_t eth_len = 0u;
    build_eth_frame(eth_frame, &eth_len, (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                    (uint8_t[]){0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}, FF_LINK_TSN_ETHERTYPE,
                    &payload, 1u);

    for (unsigned i = 0u; i < FF_LINK_TSN_RX_SLOTS; i++) {
        simulate_rx(eth_frame, eth_len);
    }
    /* One more should be dropped (ring full). */
    simulate_rx(eth_frame, eth_len);

    /* All 16 should be retrievable. */
    uint8_t rx_buf[FF_LINK_TSN_RX_FRAMESZ];
    uint16_t rx_len = 0u;
    for (unsigned i = 0u; i < FF_LINK_TSN_RX_SLOTS; i++) {
        FF_Status_t st = FF_LinkRecv(rx_buf, sizeof(rx_buf), &rx_len, 0u);
        FF_TEST_ASSERT(st == FF_SUCCESS);
        FF_TEST_ASSERT(rx_len == 1u);
        FF_TEST_ASSERT(rx_buf[0] == 0x42);
    }
    /* 17th should timeout (dropped). */
    FF_Status_t st = FF_LinkRecv(rx_buf, sizeof(rx_buf), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_TIMEOUT);
}

static void test_tsn_backend_name(void)
{
    FF_TEST_ASSERT(strcmp(FF_LinkTsnBackend.name, "tsn_eth") == 0);
}

static void test_tsn_custom_dest_mac(void)
{
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    /* Set a custom destination MAC. */
    cfg.dest_mac[0] = 0x02;
    cfg.dest_mac[1] = 0x00;
    cfg.dest_mac[2] = 0x00;
    cfg.dest_mac[3] = 0x00;
    cfg.dest_mac[4] = 0x00;
    cfg.dest_mac[5] = 0x01;
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    uint8_t payload[] = {0x01};
    (void)FF_LinkSend(payload, sizeof(payload));
    FF_TEST_ASSERT(g_captured_len >= (uint16_t)(FF_LINK_TSN_HDR_LEN + 1u));
    FF_TEST_ASSERT(g_captured_frame[0] == 0x02u);
    FF_TEST_ASSERT(g_captured_frame[1] == 0x00u);
    FF_TEST_ASSERT(g_captured_frame[5] == 0x01u);
    /* Verify payload at offset 14. */
    FF_TEST_ASSERT(g_captured_frame[14] == 0x01u);
}

static void test_tsn_short_frame_dropped(void)
{
    /* R2-04: frames shorter than the Ethernet header must be dropped. */
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    /* Simulate RX of a too-short frame (< 14-byte header). */
    uint8_t short_frame[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    simulate_rx(short_frame, 8u);

    /* Nothing should be in the ring. */
    uint8_t rx_buf[64];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkRecv(rx_buf, sizeof(rx_buf), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_TIMEOUT);
}

static void test_tsn_oversized_payload_dropped(void)
{
    /* R2-04: frames with payload exceeding FF_LINK_TSN_RX_FRAMESZ must be dropped. */
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    /* Build a frame with oversized payload (header + FF_LINK_TSN_RX_FRAMESZ + 1). */
    uint16_t payload_len = (uint16_t)(FF_LINK_TSN_RX_FRAMESZ + 1u);
    uint16_t total_len = (uint16_t)(FF_LINK_TSN_HDR_LEN + payload_len);
    uint8_t big_frame[2048];
    /* Fill with a valid EtherType at the right offset. */
    memset(big_frame, 0xAB, sizeof(big_frame));
    big_frame[12] = (uint8_t)((FF_LINK_TSN_ETHERTYPE >> 8) & 0xFFu);
    big_frame[13] = (uint8_t)(FF_LINK_TSN_ETHERTYPE & 0xFFu);

    simulate_rx(big_frame, total_len);

    /* Nothing should be in the ring. */
    uint8_t rx_buf[64];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkRecv(rx_buf, sizeof(rx_buf), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_TIMEOUT);
}

static void test_tsn_init_idempotent(void)
{
    /* R1-12: calling tsn_link_init twice must not re-create the mutex or crash. */
    FF_PalInit();
    reset_mocks();
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    (void)FF_LinkTsnInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkTsnBackend);

    /* First init via vtable. */
    FF_Status_t st = FF_LinkTsnBackend.init();
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* Second init via vtable should be idempotent (no error). */
    st = FF_LinkTsnBackend.init();
    FF_TEST_ASSERT(st == FF_SUCCESS);
}

/* ---- PUS-C over TSN demonstration ("pus_tsn", SRS-REQ-061, method Tst|Dem) ---- */

static void pus_tsn_tm_sink(const uint8_t *packet, uint16_t len)
{
    (void)FF_LinkSend(packet, len);
}

static void test_pus_tsn_roundtrip(void)
{
    /* S4b exit criterion ("pus_tsn"): carry a real PUS-C TM/TC exchange over the
     * TSN FF_Link_t backend end to end (PUS -> link -> MAC -> link -> PUS) with
     * the mock MAC looped back. Demonstration (Dem) evidence for SRS-REQ-061. */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    reset_mocks();
    g_mock_echo = true;
    FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&g_mock_mac, &g_mock_tsn);
    FF_TEST_ASSERT(FF_LinkTsnInit(&cfg) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkSelect(&FF_LinkTsnBackend) == FF_SUCCESS);

    /* TC path: a PUS S17 ping TC arrives as an Ethernet frame (TC over TSN). */
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = FF_PUS_SERVICE_TEST;
    tc.subservice = 1u;
    tc.source_id = 0x0102u;
    const uint8_t tc_payload[] = {0x33, 0x44};
    uint8_t tc_frame[128];
    uint16_t tc_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, tc_payload, sizeof(tc_payload), tc_frame,
                                    sizeof(tc_frame), &tc_len) == FF_SUCCESS);

    uint8_t eth[128];
    uint16_t eth_len = 0u;
    build_eth_frame(eth, &eth_len, (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                    (uint8_t[]){0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}, FF_LINK_TSN_ETHERTYPE,
                    tc_frame, tc_len);
    simulate_rx(eth, eth_len);

    uint8_t link_rx[128];
    uint16_t link_rx_len = 0u;
    FF_TEST_ASSERT(FF_LinkRecv(link_rx, sizeof(link_rx), &link_rx_len, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(link_rx_len == tc_len);
    FF_TEST_ASSERT(memcmp(link_rx, tc_frame, tc_len) == 0);

    /* TM path: the sink transmits the TM, which the looped-back mock MAC returns. */
    FF_PusSetTmSink(pus_tsn_tm_sink);
    FF_TEST_ASSERT(FF_PusIngest(link_rx, link_rx_len) == FF_SUCCESS);

    bool saw_reply = false;
    for (uint16_t guard = 0u; guard < 8u; guard++) {
        uint8_t tm_rx[256];
        uint16_t tm_rx_len = 0u;
        if (FF_LinkRecv(tm_rx, sizeof(tm_rx), &tm_rx_len, 0u) != FF_SUCCESS) {
            break;
        }
        if ((tm_rx_len >= 9u) && (tm_rx[7] == FF_PUS_SERVICE_TEST) && (tm_rx[8] == 2u)) {
            saw_reply = true;
        }
    }
    FF_TEST_ASSERT(saw_reply);
    FF_PusSetTmSink(NULL);
}

FF_TEST_MAIN(test_tsn_init_success, test_tsn_send_eth_frame, test_tsn_recv_after_rx_cb,
             test_tsn_recv_wrong_etype_dropped, test_tsn_recv_null_args, test_tsn_init_null_config,
             test_tsn_init_null_mac, test_tsn_send_too_large, test_tsn_ring_full_drop,
             test_tsn_backend_name, test_tsn_custom_dest_mac, test_tsn_short_frame_dropped,
             test_tsn_oversized_payload_dropped, test_tsn_init_idempotent, test_pus_tsn_roundtrip)
