/*
 * frick-fsw - gPTP (802.1AS) stack unit tests (milestone M8, SRS-REQ-068)
 *
 * Exercises the gPTP state machine, BMCA, Sync/Follow_Up time-offset,
 * PDelay link-delay and the PI servo with a mock MAC and mock TSU timer.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/eth/gptp/ff_gptp.h"
#include <string.h>

/* ----------------------------------------------------------------------- */
/* Mock TSN backend: returns a programmable monotonic time.               */
/* ----------------------------------------------------------------------- */
static uint64_t g_mock_time_ns = 0;

static FF_Status_t mock_get_time(void *self, uint64_t *out_ns)
{
    (void)self;
    *out_ns = g_mock_time_ns;
    return FF_SUCCESS;
}

static FF_TsnBackend_t g_mock_tsn = {
    .name = "mock-tsn",
    .init = NULL,
    .set_vlan = NULL,
    .configure_tas = NULL,
    .configure_cbs = NULL,
    .set_stream_filter = NULL,
    .enable_pfc = NULL,
    .get_time_ns = mock_get_time,
};

/* ----------------------------------------------------------------------- */
/* Mock MAC: stores transmitted frames and can inject received frames.     */
/* ----------------------------------------------------------------------- */
#define MOCK_MAX_TX 8u
#define MOCK_MAX_FRAME 256u

typedef struct
{
    FF_EthMac_t eth; /* must be first: callbacks receive this */
    uint8_t frames[MOCK_MAX_TX][MOCK_MAX_FRAME];
    uint16_t len[MOCK_MAX_TX];
    uint8_t count;
    FF_EthRxCb_t rx_cb;
    void *rx_ctx;
    uint8_t mac_addr[6];
} GptpMockMac_t;

static GptpMockMac_t g_mac;

static FF_Status_t mock_get_mac(void *self, uint8_t out[6])
{
    GptpMockMac_t *m = (GptpMockMac_t *)self;
    (void)memcpy(out, m->mac_addr, 6);
    return FF_SUCCESS;
}

static FF_Status_t mock_transmit(void *self, const uint8_t *frame, uint16_t len)
{
    GptpMockMac_t *m = (GptpMockMac_t *)self;
    if (m->count >= MOCK_MAX_TX) {
        return FF_BUFFER_FULL;
    }
    if (len > MOCK_MAX_FRAME) {
        return FF_BAD_SIZE;
    }
    memcpy(m->frames[m->count], frame, len);
    m->len[m->count] = len;
    m->count++;
    return FF_SUCCESS;
}

static void mock_set_rx_cb(void *self, FF_EthRxCb_t cb, void *ctx)
{
    GptpMockMac_t *m = (GptpMockMac_t *)self;
    m->rx_cb = cb;
    m->rx_ctx = ctx;
}

static FF_Status_t mock_get_stats(void *self, uint32_t *tx, uint32_t *rx, uint32_t *err)
{
    (void)self;
    *tx = 0;
    *rx = 0;
    *err = 0;
    return FF_SUCCESS;
}

static void mock_mac_setup(GptpMockMac_t *m)
{
    memset(m, 0, sizeof(*m));
    m->eth.name = "mock-mac";
    m->eth.mtu = 1500;
    m->eth.init = NULL;
    m->eth.get_mac_address = mock_get_mac;
    m->eth.set_link_up = NULL;
    m->eth.transmit = mock_transmit;
    m->eth.set_rx_callback = mock_set_rx_cb;
    m->eth.get_stats = mock_get_stats;
    m->mac_addr[0] = 0x02;
    m->mac_addr[1] = 0x00;
    m->mac_addr[2] = 0x00;
    m->mac_addr[3] = 0x00;
    m->mac_addr[4] = 0x00;
    m->mac_addr[5] = 0x01;
}

/* Helper: inject a received PTP frame into the gPTP port via the MAC rx callback. */
static void mock_inject_frame(GptpMockMac_t *m, const uint8_t *frame, uint16_t len)
{
    if ((m->rx_cb != NULL) && (len <= MOCK_MAX_FRAME)) {
        m->rx_cb(frame, len, m->rx_ctx);
    }
}

/* Helper: build a PTP frame. Writes the time payload for Follow_Up and
 * PDelay_Resp/Fup messages that carry a 64-bit timestamp (8B sec + 8B nsec
 * in 32.32 fixed-point). */
static void build_ptp_frame(uint8_t *buf, uint16_t bufsize, uint8_t msg_type, uint16_t seq,
                            const uint8_t src_clock[8], uint64_t time_ns, uint8_t domain)
{
    uint8_t i;
    size_t total = 14u + FF_GPTP_HDR_SIZE + 20u;
    (void)bufsize;

    /* Ethernet header: dst = PTP multicast, src = zero (MAC fills in HW). */
    buf[0] = FF_GPTP_DST_MAC_0;
    buf[1] = FF_GPTP_DST_MAC_1;
    buf[2] = FF_GPTP_DST_MAC_2;
    buf[3] = FF_GPTP_DST_MAC_3;
    buf[4] = FF_GPTP_DST_MAC_4;
    buf[5] = FF_GPTP_DST_MAC_5;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;
    buf[10] = 0;
    buf[11] = 0;
    buf[12u] = (FF_GPTP_ETHERTYPE >> 8) & 0xFFu;
    buf[13u] = FF_GPTP_ETHERTYPE & 0xFFu;

    /* PTP header (34 bytes starting at buf[14]). */
    memset(&buf[14u], 0, FF_GPTP_HDR_SIZE);
    buf[14u + FF_GPTP_HDR_MSG_TYPE] = msg_type;
    buf[14u + FF_GPTP_HDR_MSG_LEN] = (uint8_t)((total >> 8) & 0xFFu);
    buf[14u + FF_GPTP_HDR_MSG_LEN + 1u] = (uint8_t)(total & 0xFFu);
    buf[14u + FF_GPTP_HDR_DOMAIN] = domain;
    buf[14u + FF_GPTP_HDR_FLAGS] = 0u;
    buf[14u + FF_GPTP_HDR_FLAGS + 1u] = (uint8_t)(FF_GPTP_FLAG_TIMESCALE | FF_GPTP_FLAG_PTP_TYPE);
    memcpy(&buf[14u + FF_GPTP_HDR_SRC_CLOCK_ID], src_clock, 8);
    buf[14u + FF_GPTP_HDR_SRC_PORT_ID + 8u] = 0u;
    buf[14u + FF_GPTP_HDR_SRC_PORT_ID + 9u] = 1u;
    buf[14u + FF_GPTP_HDR_SEQ_ID] = (uint8_t)((seq >> 8) & 0xFFu);
    buf[14u + FF_GPTP_HDR_SEQ_ID + 1u] = (uint8_t)(seq & 0xFFu);
    buf[14u + FF_GPTP_HDR_CONTROL] = (msg_type == FF_GPTP_MSG_FOLLOW_UP) ? 0x6u : 0x1u;
    buf[14u + FF_GPTP_HDR_LOG_INTVAL] = 0x7Fu;

    /* For Follow_Up, PDelay_Resp and PDelay_Follow_Up, write a 64-bit
     * timestamp at offset FF_GPTP_HDR_SIZE + 10 (8B seconds + 8B nanoseconds
     * in 32.32 fixed-point). Other message types get a zero payload. */
    if (msg_type == FF_GPTP_MSG_FOLLOW_UP || msg_type == FF_GPTP_MSG_PDELAY_RESP ||
        msg_type == FF_GPTP_MSG_PDELAY_FUP) {
        uint64_t sec = time_ns / 1000000000ULL;
        uint64_t nsec_rem = time_ns % 1000000000ULL;
        uint64_t nsec_fixed = nsec_rem << 32;
        for (i = 0; i < 8; i++) {
            buf[14u + FF_GPTP_HDR_SIZE + 10u + i] = (uint8_t)((sec >> (56u - i * 8u)) & 0xFFu);
        }
        for (i = 0; i < 8; i++) {
            buf[14u + FF_GPTP_HDR_SIZE + 18u + i] =
                (uint8_t)((nsec_fixed >> (56u - i * 8u)) & 0xFFu);
        }
    }
}

/* ----------------------------------------------------------------------- */
/* Tests                                                                    */
/* ----------------------------------------------------------------------- */

static void test_gptp_init(void)
{
    FF_GptpPort_t port;
    uint8_t identity[8];

    mock_mac_setup(&g_mac);
    g_mock_time_ns = 5000000000ULL; /* 5 s */

    /* NULL checks. */
    FF_TEST_ASSERT(FF_GptpInit(NULL, &g_mac.eth, &g_mock_tsn) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_GptpInit(&port, NULL, &g_mock_tsn) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, NULL) == FF_INVALID_ARGUMENT);

    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, &g_mock_tsn) == FF_SUCCESS);
    FF_TEST_ASSERT(port.state == FF_GPTP_SM_UNCALIBRATED);
    FF_TEST_ASSERT(port.role == FF_GPTP_ROLE_UNCERTAIN);

    /* The MAC address should have been converted to EUI-64 clock identity. */
    FF_TEST_ASSERT(FF_GptpSetClockIdentity(
                       &port, (const uint8_t[8]){0x02, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x01}) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(FF_GptpSetClockIdentity(&port, NULL) == FF_INVALID_ARGUMENT);

    /* NULL checks on accessors. */
    FF_TEST_ASSERT(FF_GptpGetRole(NULL) == FF_GPTP_ROLE_DISABLED);
    FF_TEST_ASSERT(FF_GptpGetState(NULL) == FF_GPTP_SM_DISABLED);
    uint64_t gm_null = 0;
    FF_TEST_ASSERT(FF_GptpGetGrandmasterTime(&port, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_GptpGetGrandmasterTime(NULL, &gm_null) == FF_INVALID_ARGUMENT);

    /* Grandmaster time = local TSU time (no offset yet). */
    uint64_t gm_ns = 0;
    FF_TEST_ASSERT(FF_GptpGetGrandmasterTime(&port, &gm_ns) == FF_SUCCESS);
    FF_TEST_ASSERT(gm_ns == 5000000000ULL);

    /* Disable the port. */
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(port.state == FF_GPTP_SM_DISABLED);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, (const uint8_t[20]){0}, 20u) == FF_INVALID_ARGUMENT);
    (void)identity;
}

static void test_gptp_sync_followup(void)
{
    FF_GptpPort_t port;
    uint8_t master_clock[8] = {0x02, 0x10, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x0A};
    uint8_t frame[256];
    uint64_t gm_ns;

    mock_mac_setup(&g_mac);
    g_mock_time_ns = 1000000000ULL; /* 1 s */

    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, &g_mock_tsn) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(port.state == FF_GPTP_SM_UNCALIBRATED);

    /* Step 1: receive a Sync from the master. */
    g_mock_time_ns = 1000000000ULL + 500000000ULL; /* 1.5 s */
    build_ptp_frame(frame, sizeof(frame), FF_GPTP_MSG_SYNC, 1u, master_clock, 0, 0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_SUCCESS);
    FF_TEST_ASSERT(port.state == FF_GPTP_SM_SLAVE);
    FF_TEST_ASSERT(port.role == FF_GPTP_ROLE_SLAVE);
    FF_TEST_ASSERT(port.last_sync_ns == 1500000000ULL);

    /* Update mock time to 1.6s. */
    g_mock_time_ns = 1600000000ULL;

    /* Step 2: receive a Follow_Up with the master's precise egress time.
     * Master egressed the Sync at 1.7 s. */
    build_ptp_frame(frame, sizeof(frame), FF_GPTP_MSG_FOLLOW_UP, 1u, master_clock, 1700000000ULL,
                    0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_SUCCESS);

    /* The servo should have computed an offset:
     * offset = master_time - local_arrival = 1.7s - 1.5s = +200ms. */
    FF_TEST_ASSERT(port.servo.offset_ns == 200000000LL);
    FF_TEST_ASSERT(port.servo.stable == 0u);

    /* Grandmaster time = local time + offset = 1.6s + 200ms = 1.8s. */
    FF_TEST_ASSERT(FF_GptpGetGrandmasterTime(&port, &gm_ns) == FF_SUCCESS);
    FF_TEST_ASSERT(gm_ns == 1800000000ULL);
}

static void test_gptp_bmca_and_master(void)
{
    FF_GptpPort_t port;
    uint8_t master_clock[8] = {0x02, 0x10, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x0A};
    uint8_t frame[256];

    mock_mac_setup(&g_mac);
    g_mock_time_ns = 0;

    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, &g_mock_tsn) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 1u) == FF_SUCCESS);

    /* With no GM announced, the port becomes master on the first tick. */
    FF_TEST_ASSERT(FF_GptpTick(&port, 0) == FF_SUCCESS);
    FF_TEST_ASSERT(port.role == FF_GPTP_ROLE_MASTER);
    FF_TEST_ASSERT(port.state == FF_GPTP_SM_MASTER);

    /* A Sync from an external master does not override our master role
     * (we are already the GM; BMCA only overrides on a better master). */
    g_mock_time_ns = 2000000000ULL;
    build_ptp_frame(frame, sizeof(frame), FF_GPTP_MSG_SYNC, 10u, master_clock, 0, 0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_SUCCESS);
    FF_TEST_ASSERT(port.role == FF_GPTP_ROLE_MASTER);
}

static void test_gptp_pdelay_req_tx(void)
{
    FF_GptpPort_t port;

    mock_mac_setup(&g_mac);
    g_mock_time_ns = 5000000000ULL;

    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, &g_mock_tsn) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 1u) == FF_SUCCESS);

    /* NULL check. */
    FF_TEST_ASSERT(FF_GptpSendPDelayReq(NULL, 0) == FF_INVALID_ARGUMENT);

    /* Send a PDelay_Req. */
    FF_TEST_ASSERT(FF_GptpSendPDelayReq(&port, 5000000000ULL) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mac.count == 1u);
    FF_TEST_ASSERT(g_mac.len[0] > 0u);

    /* Verify the PTP ethertype and message type in the transmitted frame. */
    FF_TEST_ASSERT(((uint16_t)(g_mac.frames[0][12u] << 8) | g_mac.frames[0][13u]) ==
                   FF_GPTP_ETHERTYPE);
    FF_TEST_ASSERT((g_mac.frames[0][14u + FF_GPTP_HDR_MSG_TYPE] & 0x0Fu) == FF_GPTP_MSG_PDELAY_REQ);
}

static void test_gptp_state_disable(void)
{
    FF_GptpPort_t port;
    uint8_t frame[256];
    const uint8_t master_clock[8] = {0x02, 0x10, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x0A};

    mock_mac_setup(&g_mac);
    g_mock_time_ns = 1000000000ULL;

    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, &g_mock_tsn) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 1u) == FF_SUCCESS);

    /* Disable and verify no processing occurs. */
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(port.state == FF_GPTP_SM_DISABLED);

    /* A frame sent to a disabled port is rejected. */
    build_ptp_frame(frame, sizeof(frame), FF_GPTP_MSG_SYNC, 1u, master_clock, 0, 0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_INVALID_ARGUMENT);

    /* Tick on disabled port is a no-op. */
    FF_TEST_ASSERT(FF_GptpTick(&port, 1000000000ULL) == FF_SUCCESS);
}

static void test_gptp_frame_parsing(void)
{
    FF_GptpPort_t port;
    uint8_t frame[256];
    const uint8_t master_clock[8] = {0x02, 0x10, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x0A};

    mock_mac_setup(&g_mac);
    g_mock_time_ns = 2000000000ULL;

    FF_TEST_ASSERT(FF_GptpInit(&port, &g_mac.eth, &g_mock_tsn) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_GptpSetEnabled(&port, 1u) == FF_SUCCESS);

    /* Too-short frame. */
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, (const uint8_t[10]){0}, 10u) == FF_BAD_SIZE);

    /* Wrong ethertype (not PTP). */
    memset(frame, 0, sizeof(frame));
    frame[12u] = 0x08;
    frame[13u] = 0x00; /* IPv4 */
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_PARSE_ERROR);

    /* Unknown message type. */
    build_ptp_frame(frame, sizeof(frame), 0x0Fu, 1u, master_clock, 0, 0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_PARSE_ERROR);

    /* Valid PDelay_Resp. */
    build_ptp_frame(frame, sizeof(frame), FF_GPTP_MSG_PDELAY_RESP, 2u, master_clock, 2000000000ULL,
                    0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_SUCCESS);
    FF_TEST_ASSERT(port.servo.delay_ns == 2000000000LL);

    /* Valid PDelay_Follow_Up. */
    build_ptp_frame(frame, sizeof(frame), FF_GPTP_MSG_PDELAY_FUP, 2u, master_clock, 2100000000ULL,
                    0);
    FF_TEST_ASSERT(FF_GptpProcessFrame(&port, frame, 74u) == FF_SUCCESS);
}

FF_TEST_MAIN(test_gptp_init, test_gptp_sync_followup, test_gptp_bmca_and_master,
             test_gptp_pdelay_req_tx, test_gptp_state_disable, test_gptp_frame_parsing)
