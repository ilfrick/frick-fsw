/*
 * frick-fsw - Unit tests for link_seg (SRS-REQ-059) + link_ip backend (SRS-REQ-060)
 *
 * Tests the segmentation/reassembly layer with a mock byte stream, and the
 * IP link backend with a mock network stack and the reference loopback stack.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "transport/link/link_seg.h"
#include "transport/link/link_ip.h"
#include "transport/ccsds/ccsds.h"
#include "drivers/net/ff_net_loopback.h"
#include "pus/pus.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include <string.h>

/* ---- Mock byte stream for seg layer testing ---- */

#define MOCK_STREAM_CAP 256u

typedef struct
{
    uint8_t buf[MOCK_STREAM_CAP];
    uint16_t head;
    uint16_t count;
} FF_MockStream_t;

static FF_MockStream_t g_mock;

static FF_Status_t mock_write(void *ctx, const uint8_t *data, uint16_t len)
{
    FF_MockStream_t *s = (FF_MockStream_t *)ctx;
    for (uint16_t i = 0u; i < len; i++) {
        if (s->count >= MOCK_STREAM_CAP) {
            return FF_BUFFER_FULL;
        }
        s->buf[(s->head + s->count) % MOCK_STREAM_CAP] = data[i];
        s->count++;
    }
    return FF_SUCCESS;
}

static FF_Status_t mock_read(void *ctx, uint8_t *data, uint16_t cap, uint16_t *len,
                             uint32_t timeout_ms)
{
    (void)timeout_ms;
    FF_MockStream_t *s = (FF_MockStream_t *)ctx;
    uint16_t n = 0u;
    while ((s->count > 0u) && (n < cap)) {
        data[n++] = s->buf[s->head];
        s->head = (uint16_t)((s->head + 1u) % MOCK_STREAM_CAP);
        s->count--;
    }
    *len = n;
    return FF_SUCCESS;
}

static void mock_reset(void)
{
    memset(&g_mock, 0, sizeof(g_mock));
}

/* ---- Chunked mock stream (returns at most CHUNK bytes per read) ---- */

#define MOCK_CHUNK_SIZE 3u

static FF_MockStream_t g_chunked;

static FF_Status_t mock_read_chunked(void *ctx, uint8_t *data, uint16_t cap, uint16_t *len,
                                     uint32_t timeout_ms)
{
    (void)timeout_ms;
    FF_MockStream_t *s = (FF_MockStream_t *)ctx;
    uint16_t n = 0u;
    uint16_t max = (cap < MOCK_CHUNK_SIZE) ? cap : MOCK_CHUNK_SIZE;
    while ((s->count > 0u) && (n < max)) {
        data[n++] = s->buf[s->head];
        s->head = (uint16_t)((s->head + 1u) % MOCK_STREAM_CAP);
        s->count--;
    }
    *len = n;
    return FF_SUCCESS;
}

/* ---- Error mock stream (read returns an error after N bytes) ---- */

static FF_MockStream_t g_err_stream;
static uint16_t g_err_stream_max_bytes;
static uint16_t g_err_stream_read_count;

static FF_Status_t mock_read_with_error(void *ctx, uint8_t *data, uint16_t cap, uint16_t *len,
                                        uint32_t timeout_ms)
{
    (void)timeout_ms;
    (void)ctx;
    FF_MockStream_t *s = (FF_MockStream_t *)ctx;
    /* After delivering g_err_stream_max_bytes, return an error. */
    if (g_err_stream_read_count >= g_err_stream_max_bytes) {
        *len = 0u;
        return FF_ERROR;
    }
    uint16_t n = 0u;
    uint16_t want = (uint16_t)(g_err_stream_max_bytes - g_err_stream_read_count);
    if (want > cap) {
        want = cap;
    }
    while ((s->count > 0u) && (n < want)) {
        data[n++] = s->buf[s->head];
        s->head = (uint16_t)((s->head + 1u) % MOCK_STREAM_CAP);
        s->count--;
    }
    g_err_stream_read_count = (uint16_t)(g_err_stream_read_count + n);
    *len = n;
    return FF_SUCCESS;
}

static void mock_chunked_reset(void)
{
    memset(&g_chunked, 0, sizeof(g_chunked));
}

static void mock_err_reset(void)
{
    memset(&g_err_stream, 0, sizeof(g_err_stream));
    g_err_stream_max_bytes = 0u;
    g_err_stream_read_count = 0u;
}

/* ---- Mock net stack for IP backend tests ---- */

static uint8_t g_mock_net_rx[256];
static uint16_t g_mock_net_rx_len;
static uint8_t g_mock_net_tx[256];
static uint16_t g_mock_net_tx_len;
static FF_NetSock_t g_mock_net_sock = -1;
static FF_NetSock_t g_mock_net_peer = -1;
static uint16_t g_mock_net_send_limit = 0u; /* when >0, limits bytes per mock_net_send call */

static FF_Status_t mock_net_init(void)
{
    g_mock_net_rx_len = 0u;
    g_mock_net_tx_len = 0u;
    g_mock_net_send_limit = 0u;
    g_mock_net_sock = 0;
    g_mock_net_peer = 1;
    return FF_SUCCESS;
}

static FF_Status_t mock_net_attach(void *netif)
{
    (void)netif;
    return FF_SUCCESS;
}

static FF_NetSock_t mock_net_open(int domain, int type, int proto)
{
    (void)domain;
    (void)type;
    (void)proto;
    g_mock_net_sock = 0;
    g_mock_net_peer = 1;
    return g_mock_net_sock;
}

static FF_Status_t mock_net_connect(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len)
{
    (void)addr;
    (void)addr_len;
    if (sd != g_mock_net_sock) {
        return FF_INVALID_ARGUMENT;
    }
    return FF_SUCCESS; /* connected to the mock peer */
}

static FF_Status_t mock_net_bind(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len)
{
    (void)sd;
    (void)addr;
    (void)addr_len;
    return FF_SUCCESS;
}

static FF_Status_t mock_net_listen(FF_NetSock_t sd, uint16_t backlog)
{
    (void)sd;
    (void)backlog;
    return FF_SUCCESS;
}

static FF_NetSock_t mock_net_accept(FF_NetSock_t sd, uint8_t *peer, uint16_t *peer_len)
{
    (void)sd;
    if (peer != NULL && peer_len != NULL && *peer_len >= 1u) {
        peer[0] = 80u;
    }
    return g_mock_net_peer;
}

static FF_Status_t mock_net_send(FF_NetSock_t sd, const uint8_t *buf, uint16_t len, uint16_t *sent)
{
    (void)sd;
    /* Partial-send mode: deliver at most g_mock_net_send_limit bytes per call. */
    if (g_mock_net_send_limit > 0u) {
        uint16_t partial = (len < g_mock_net_send_limit) ? len : g_mock_net_send_limit;
        for (uint16_t i = 0u; i < partial; i++) {
            g_mock_net_tx[g_mock_net_tx_len + i] = buf[i];
        }
        g_mock_net_tx_len = (uint16_t)(g_mock_net_tx_len + partial);
        *sent = partial;
        return FF_SUCCESS;
    }
    if ((g_mock_net_tx_len + len) > sizeof(g_mock_net_tx)) {
        *sent = 0u;
        return FF_BUFFER_FULL;
    }
    for (uint16_t i = 0u; i < len; i++) {
        g_mock_net_tx[g_mock_net_tx_len + i] = buf[i];
    }
    g_mock_net_tx_len = (uint16_t)(g_mock_net_tx_len + len);
    *sent = len;
    return FF_SUCCESS;
}

static FF_Status_t mock_net_recv(FF_NetSock_t sd, uint8_t *buf, uint16_t cap, uint16_t *got,
                                 uint32_t timeout_ms)
{
    (void)sd;
    (void)timeout_ms;
    /* Copy from front, shift remaining. */
    uint16_t i = 0u;
    uint16_t available = g_mock_net_rx_len;
    uint16_t copy_len = (available < cap) ? available : cap;
    for (i = 0u; i < copy_len; i++) {
        buf[i] = g_mock_net_rx[i];
    }
    /* Shift remaining. */
    uint16_t remaining = (uint16_t)(available - copy_len);
    for (i = 0u; i < remaining; i++) {
        g_mock_net_rx[i] = g_mock_net_rx[copy_len + i];
    }
    g_mock_net_rx_len = remaining;
    *got = copy_len;
    return FF_SUCCESS;
}

static FF_Status_t mock_net_close(FF_NetSock_t sd)
{
    (void)sd;
    return FF_SUCCESS;
}

static FF_NetStack_t g_mock_net = {
    .name = "mock_net",
    .init = mock_net_init,
    .attach_netif = mock_net_attach,
    .open = mock_net_open,
    .connect = mock_net_connect,
    .bind = mock_net_bind,
    .listen = mock_net_listen,
    .accept = mock_net_accept,
    .send = mock_net_send,
    .recv = mock_net_recv,
    .close = mock_net_close,
};

/* ---- Seg layer tests ---- */

static void test_seg_basic_roundtrip(void)
{
    mock_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read,
        .ctx = &g_mock,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    uint8_t frame[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    FF_Status_t st = FF_LinkSegSendFrame(&seg, frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_SUCCESS);

    uint8_t rx[32];
    uint16_t rx_len = 0u;
    st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(frame));
    FF_TEST_ASSERT(memcmp(rx, frame, sizeof(frame)) == 0);
}

/* ---- Large-buffer mock stream for max-size fragment tests ---- */

#define MOCK_LARGE_CAP (FF_LINK_SEG_PREFIX_LEN + FF_LINK_SEG_MAX_FRAME)

static uint8_t g_large_buf[MOCK_LARGE_CAP];
static uint16_t g_large_len;
static uint16_t g_large_pos;

static FF_Status_t large_read(void *ctx, uint8_t *data, uint16_t cap, uint16_t *len,
                              uint32_t timeout_ms)
{
    (void)ctx;
    (void)timeout_ms;
    uint16_t remaining = (uint16_t)(g_large_len - g_large_pos);
    uint16_t n = (cap < remaining) ? cap : remaining;
    for (uint16_t i = 0u; i < n; i++) {
        data[i] = g_large_buf[g_large_pos + i];
    }
    g_large_pos = (uint16_t)(g_large_pos + n);
    *len = n;
    return FF_SUCCESS;
}

static void large_reset(void)
{
    memset(g_large_buf, 0, sizeof(g_large_buf));
    g_large_len = 0u;
    g_large_pos = 0u;
}

static void test_seg_max_size_fragmented_partial(void)
{
    /* NF-2 regression: a max-size frame delivered in small chunks must not
     * overflow rx_partial when the stream returns 0 bytes mid-frame. */
    large_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = large_read,
        .ctx = NULL,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    /* Build a max-size frame: 2-byte prefix (0x04 0x00 = 1024) + 1024 payload. */
    g_large_buf[0] = (uint8_t)(FF_LINK_SEG_MAX_FRAME >> 8);
    g_large_buf[1] = (uint8_t)(FF_LINK_SEG_MAX_FRAME & 0xFFu);
    /* Fill payload with a recognizable pattern. */
    for (uint16_t i = 0u; i < FF_LINK_SEG_MAX_FRAME; i++) {
        g_large_buf[FF_LINK_SEG_PREFIX_LEN + i] = (uint8_t)(i & 0xFFu);
    }
    g_large_len = (uint16_t)(FF_LINK_SEG_PREFIX_LEN + FF_LINK_SEG_MAX_FRAME);
    g_large_pos = 0u;

    /* Read in 3-byte chunks; the stream will exhaust after g_large_len bytes.
     * The recv loop reads in chunks (cap=3), so it will iterate many times.
     * If the stream still has data, it returns data; if not, got==0 → returns
     * early with FF_TIMEOUT and preserves rx_partial. We deliver prefix +
     * (MAX_FRAME-1) bytes, then truncate the stream so the next read returns 0. */
    g_large_len = (uint16_t)(FF_LINK_SEG_PREFIX_LEN + (uint16_t)(FF_LINK_SEG_MAX_FRAME - 1u));
    g_large_pos = 0u;

    uint8_t rx[FF_LINK_SEG_MAX_FRAME];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    /* Stream exhausts before full frame arrives → partial timeout. */
    FF_TEST_ASSERT(st == FF_TIMEOUT);
    /* rx_partial must hold prefix + (MAX_FRAME-1) bytes without overflow. */
    FF_TEST_ASSERT(seg.rx_len == (uint16_t)(FF_LINK_SEG_PREFIX_LEN + (FF_LINK_SEG_MAX_FRAME - 1u)));
}

static void test_seg_max_frame_rejected(void)
{
    mock_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read,
        .ctx = &g_mock,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    uint8_t big[FF_LINK_SEG_MAX_FRAME + 1u];
    FF_Status_t st = FF_LinkSegSendFrame(&seg, big, sizeof(big));
    FF_TEST_ASSERT(st == FF_BAD_SIZE);
}

static void test_seg_buffer_too_small(void)
{
    mock_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read,
        .ctx = &g_mock,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    uint8_t frame[] = {0xAA, 0xBB, 0xCC};
    FF_Status_t st = FF_LinkSegSendFrame(&seg, frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_SUCCESS);

    uint8_t small[2];
    uint16_t rx_len = 0u;
    st = FF_LinkSegRecvFrame(&seg, small, sizeof(small), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_BUFFER_FULL);
}

static void test_seg_null_args(void)
{
    mock_reset();
    FF_LinkSeg_t seg;
    FF_LinkByteStream_t bs = {0};
    FF_LinkSegInit(&seg, bs);

    uint8_t buf[8];
    uint16_t len = 0u;
    FF_Status_t st = FF_LinkSegSendFrame(NULL, buf, sizeof(buf));
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
    st = FF_LinkSegRecvFrame(NULL, buf, sizeof(buf), &len, 0u);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
    st = FF_LinkSegRecvFrame(&seg, NULL, sizeof(buf), &len, 0u);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
    st = FF_LinkSegRecvFrame(&seg, buf, sizeof(buf), NULL, 0u);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_seg_length_prefix_encoding(void)
{
    mock_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read,
        .ctx = &g_mock,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    uint8_t frame[] = {0xDE, 0xAD, 0xBE, 0xEF};
    FF_Status_t st = FF_LinkSegSendFrame(&seg, frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_SUCCESS);

    FF_TEST_ASSERT(g_mock.buf[0] == 0x00u);
    FF_TEST_ASSERT(g_mock.buf[1] == 0x04u);
    FF_TEST_ASSERT(g_mock.buf[2] == 0xDEu);
    FF_TEST_ASSERT(g_mock.buf[3] == 0xADu);
    FF_TEST_ASSERT(g_mock.count == (uint16_t)(2u + sizeof(frame)));
}

static void test_seg_roundtrip_two_frames(void)
{
    mock_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read,
        .ctx = &g_mock,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    uint8_t f1[] = {0x01, 0x02};
    uint8_t f2[] = {0x03, 0x04, 0x05};

    (void)FF_LinkSegSendFrame(&seg, f1, sizeof(f1));
    (void)FF_LinkSegSendFrame(&seg, f2, sizeof(f2));

    uint8_t rx[32];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(f1));
    FF_TEST_ASSERT(memcmp(rx, f1, sizeof(f1)) == 0);

    st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(f2));
    FF_TEST_ASSERT(memcmp(rx, f2, sizeof(f2)) == 0);
}

/* ---- IP backend tests ---- */

static void test_ip_init_udp_default(void)
{
    FF_PalInit();
    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&g_mock_net);
    FF_Status_t st = FF_LinkIpInit(&cfg);
    FF_TEST_ASSERT(st == FF_SUCCESS);
}

static void test_ip_null_config(void)
{
    FF_Status_t st = FF_LinkIpInit(NULL);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_ip_null_stack(void)
{
    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(NULL);
    FF_Status_t st = FF_LinkIpInit(&cfg);
    FF_TEST_ASSERT(st == FF_INVALID_ARGUMENT);
}

static void test_ip_backend_name(void)
{
    FF_TEST_ASSERT(strcmp(FF_LinkIpBackend.name, "ip_link") == 0);
}

static void test_ip_send_no_backend(void)
{
    /* No backend selected → FF_LinkSend returns FF_NOT_READY (deterministic). */
    (void)FF_LinkSelect(NULL);
    uint8_t data[] = {1, 2, 3, 4};
    FF_Status_t st = FF_LinkSend(data, sizeof(data));
    FF_TEST_ASSERT(st == FF_NOT_READY);
}

static void test_ip_udp_send_recv_roundtrip(void)
{
    FF_PalInit();

    /* Reset mock net state. */
    g_mock_net_rx_len = 0u;
    g_mock_net_tx_len = 0u;

    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&g_mock_net);
    cfg.mode = FF_LINK_IPMODE_UDP;
    cfg.port = 8080u;
    FF_Status_t st = FF_LinkIpInit(&cfg);
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* Select the backend. */
    st = FF_LinkSelect(&FF_LinkIpBackend);
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* Send a CCSDS frame: in UDP mode it goes straight to the net stack. */
    uint8_t frame[] = {0x50, 0x51, 0x52, 0x53};
    st = FF_LinkSend(frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* The mock net stack's TX buffer should contain the frame. */
    FF_TEST_ASSERT(g_mock_net_tx_len == sizeof(frame));
    FF_TEST_ASSERT(memcmp(g_mock_net_tx, frame, sizeof(frame)) == 0);

    /* Simulate a received datagram: put data in the net stack's RX buffer. */
    uint8_t rx_frame[] = {0x60, 0x61, 0x62};
    for (uint16_t i = 0u; i < sizeof(rx_frame); i++) {
        g_mock_net_rx[i] = rx_frame[i];
    }
    g_mock_net_rx_len = (uint16_t)sizeof(rx_frame);

    /* Recv from the link backend (UDP mode: direct recv). */
    uint8_t rx[32];
    uint16_t rx_len = 0u;
    st = FF_LinkRecv(rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(rx_frame));
    FF_TEST_ASSERT(memcmp(rx, rx_frame, sizeof(rx_frame)) == 0);
}

static void test_ip_tcp_send_segmented(void)
{
    FF_PalInit();

    /* Reset mock net state. */
    g_mock_net_rx_len = 0u;
    g_mock_net_tx_len = 0u;

    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&g_mock_net);
    cfg.mode = FF_LINK_IPMODE_TCP;
    cfg.port = 9090u;
    FF_Status_t st = FF_LinkIpInit(&cfg);
    /* The mock net stack's accept returns a fake peer, so TCP init should succeed. */
    FF_TEST_ASSERT(st == FF_SUCCESS);

    st = FF_LinkSelect(&FF_LinkIpBackend);
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* Send a CCSDS frame: in TCP mode it should be length-prefixed. */
    uint8_t frame[] = {0x01, 0x02, 0x03};
    st = FF_LinkSend(frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* The mock net TX buffer should contain [2-byte len][frame]. */
    uint16_t expected_total = (uint16_t)(2u + sizeof(frame));
    FF_TEST_ASSERT(g_mock_net_tx_len == expected_total);
    /* Length prefix: big-endian 3 */
    FF_TEST_ASSERT(g_mock_net_tx[0] == 0x00u);
    FF_TEST_ASSERT(g_mock_net_tx[1] == 0x03u);
    /* Frame data after prefix. */
    FF_TEST_ASSERT(memcmp(&g_mock_net_tx[2], frame, sizeof(frame)) == 0);
}

static void test_ip_tcp_recv_reassembles(void)
{
    FF_PalInit();

    g_mock_net_rx_len = 0u;
    g_mock_net_tx_len = 0u;

    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&g_mock_net);
    cfg.mode = FF_LINK_IPMODE_TCP;
    cfg.port = 9091u;
    (void)FF_LinkIpInit(&cfg);
    (void)FF_LinkSelect(&FF_LinkIpBackend);

    /* Put a length-prefixed frame in the net RX buffer: [0x00, 0x03, 0xAA, 0xBB, 0xCC] */
    uint8_t incoming[] = {0x00, 0x03, 0xAA, 0xBB, 0xCC};
    for (uint16_t i = 0u; i < sizeof(incoming); i++) {
        g_mock_net_rx[i] = incoming[i];
    }
    g_mock_net_rx_len = (uint16_t)sizeof(incoming);

    uint8_t rx[32];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkRecv(rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == 3u);
    FF_TEST_ASSERT(rx[0] == 0xAAu);
    FF_TEST_ASSERT(rx[1] == 0xBBu);
    FF_TEST_ASSERT(rx[2] == 0xCCu);
}

/* ---- Seg layer: fragmentation, error propagation, over-max prefix ---- */

static void test_seg_fragmented_stream(void)
{
    /* R2-01: verify that a frame arriving in multiple small chunks is reassembled. */
    mock_chunked_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read_chunked,
        .ctx = &g_chunked,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    /* Send a frame into the mock stream, then read it back in ~3-byte chunks. */
    uint8_t frame[] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};
    FF_Status_t st = FF_LinkSegSendFrame(&seg, frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_SUCCESS);

    uint8_t rx[32];
    uint16_t rx_len = 0u;
    st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(frame));
    FF_TEST_ASSERT(memcmp(rx, frame, sizeof(frame)) == 0);
}

static void test_seg_read_error_propagation(void)
{
    /* R2-02: when read_bytes returns an error mid-payload, it must propagate. */
    mock_err_reset();
    /* Pre-fill the chunked mock buffer with a prefixed frame: [0x00 0x05] + 5 payload bytes */
    uint8_t prefixed[] = {0x00, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05};
    for (uint16_t i = 0u; i < sizeof(prefixed); i++) {
        g_err_stream.buf[(g_err_stream.head + g_err_stream.count) % MOCK_STREAM_CAP] = prefixed[i];
        g_err_stream.count++;
    }
    /* Deliver only 4 bytes (prefix + 2 payload) before erroring. */
    g_err_stream_max_bytes = 4u;
    g_err_stream_read_count = 0u;

    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read_with_error,
        .ctx = &g_err_stream,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    uint8_t rx[32];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_ERROR);
}

static void test_seg_over_max_length_prefix(void)
{
    /* R2-05: a length prefix exceeding FF_LINK_SEG_MAX_FRAME must be rejected. */
    mock_reset();
    FF_LinkByteStream_t bs = {
        .write_bytes = mock_write,
        .read_bytes = mock_read,
        .ctx = &g_mock,
    };
    FF_LinkSeg_t seg;
    FF_LinkSegInit(&seg, bs);

    /* Push a prefix that claims a frame larger than the max. */
    uint8_t bad_prefix[] = {(uint8_t)(0xFF), (uint8_t)(0xFF)};
    for (uint16_t i = 0u; i < sizeof(bad_prefix); i++) {
        g_mock.buf[(g_mock.head + g_mock.count) % MOCK_STREAM_CAP] = bad_prefix[i];
        g_mock.count++;
    }

    uint8_t rx[32];
    uint16_t rx_len = 0u;
    FF_Status_t st = FF_LinkSegRecvFrame(&seg, rx, sizeof(rx), &rx_len, 0u);
    FF_TEST_ASSERT(st == FF_BAD_SIZE);
}

/* ---- IP backend: partial TCP write test (R2-06) ---- */
static void test_ip_tcp_partial_write(void)
{
    /* R2-06: a partial send in TCP mode must propagate FF_BUFFER_FULL. */
    FF_PalInit();
    g_mock_net_rx_len = 0u;
    g_mock_net_tx_len = 0u;

    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&g_mock_net);
    cfg.mode = FF_LINK_IPMODE_TCP;
    cfg.port = 9092u;
    FF_Status_t st = FF_LinkIpInit(&cfg);
    FF_TEST_ASSERT(st == FF_SUCCESS);

    /* Set the partial-send limit AFTER init (init resets it via mock_net_init). */
    g_mock_net_send_limit = 1u; /* only 1 byte per send call */

    (void)FF_LinkSelect(&FF_LinkIpBackend);

    /* Send a frame larger than 1 byte — partial write → FF_BUFFER_FULL. */
    uint8_t frame[] = {0x01, 0x02, 0x03};
    st = FF_LinkSend(frame, sizeof(frame));
    FF_TEST_ASSERT(st == FF_BUFFER_FULL);

    g_mock_net_send_limit = 0u;
}

/* ---- PUS-C over IP demonstration ("pus_ip", SRS-REQ-060, method Tst|Dem) ---- */

static uint8_t g_pus_reply[256];
static uint16_t g_pus_reply_len;
static uint16_t g_pus_reply_off; /* offset of the reply inside the mock net-stack TX */
static uint16_t g_pus_tx_count;

/* PUS TM sink: pushes every encoded TM packet onto the selected link. */
static void pus_ip_tm_sink(const uint8_t *packet, uint16_t len)
{
    uint16_t off = g_mock_net_tx_len; /* where FF_LinkSend will place this packet */
    if (FF_LinkSend(packet, len) == FF_SUCCESS) {
        g_pus_tx_count++;
    }
    /* Keep the S17 ping reply (TM secondary header service/subservice at 7/8)
     * together with the offset at which the send leg handed it to the stack. */
    if ((len >= 9u) && (packet[7] == FF_PUS_SERVICE_TEST) && (packet[8] == 2u) &&
        (len <= sizeof(g_pus_reply))) {
        (void)memcpy(g_pus_reply, packet, len);
        g_pus_reply_len = len;
        g_pus_reply_off = off;
    }
}

static void test_pus_ip_roundtrip(void)
{
    /* S4b exit criterion ("pus_ip"): carry a real PUS-C TM/TC exchange over the
     * IP FF_Link_t backend end to end (PUS -> link -> mock net stack -> link ->
     * PUS) against the mock FF_NetStack_t (g_mock_net). The reference loopback
     * stack (FF_NetStackLoopback) is a socket pair, so it cannot self-loop a
     * single link instance; the mock models the peer. This is the demonstration
     * (Dem) evidence for SRS-REQ-060 alongside the unit tests. */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    g_mock_net_rx_len = 0u;
    g_mock_net_tx_len = 0u;
    g_pus_reply_len = 0u;
    g_pus_reply_off = 0u;
    g_pus_tx_count = 0u;

    FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&g_mock_net);
    cfg.mode = FF_LINK_IPMODE_UDP;
    cfg.port = 8081u;
    FF_TEST_ASSERT(FF_LinkIpInit(&cfg) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkSelect(&FF_LinkIpBackend) == FF_SUCCESS);

    /* TC path: a PUS S17 ping TC arriving over the link is handed to PUS. */
    FF_Pus_TcHdr_t tc;
    memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = FF_PUS_SERVICE_TEST;
    tc.subservice = 1u;
    tc.source_id = 0x0102u;
    const uint8_t tc_payload[] = {0x11, 0x22};
    uint8_t tc_frame[128];
    uint16_t tc_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, tc_payload, sizeof(tc_payload), tc_frame,
                                    sizeof(tc_frame), &tc_len) == FF_SUCCESS);

    for (uint16_t i = 0u; i < tc_len; i++) {
        g_mock_net_rx[i] = tc_frame[i];
    }
    g_mock_net_rx_len = tc_len;

    uint8_t link_rx[128];
    uint16_t link_rx_len = 0u;
    FF_TEST_ASSERT(FF_LinkRecv(link_rx, sizeof(link_rx), &link_rx_len, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(link_rx_len == tc_len);
    FF_TEST_ASSERT(memcmp(link_rx, tc_frame, tc_len) == 0);

    /* TM path: install the sink, ingest the TC, TM is pushed over the link. */
    FF_PusSetTmSink(pus_ip_tm_sink);
    FF_TEST_ASSERT(FF_PusIngest(link_rx, link_rx_len) == FF_SUCCESS);
    FF_TEST_ASSERT(g_pus_tx_count >= 2u);  /* acceptance + completion + ping reply */
    FF_TEST_ASSERT(g_pus_reply_len >= 9u); /* S17 ping reply captured */

    /* Send-leg closure (R1-F01): the exact reply bytes were handed by
     * FF_LinkSend to the net stack TX buffer (g_mock_net_tx), so the send path
     * -- not merely the sink invocation -- carried the TM. */
    FF_TEST_ASSERT(((uint32_t)g_pus_reply_off + (uint32_t)g_pus_reply_len) <=
                   (uint32_t)g_mock_net_tx_len);
    FF_TEST_ASSERT(memcmp(&g_mock_net_tx[g_pus_reply_off], g_pus_reply, g_pus_reply_len) == 0);

    /* Loop the reply back through the net stack and receive it via the link. */
    for (uint16_t i = 0u; i < g_pus_reply_len; i++) {
        g_mock_net_rx[i] = g_pus_reply[i];
    }
    g_mock_net_rx_len = g_pus_reply_len;

    uint8_t tm_rx[256];
    uint16_t tm_rx_len = 0u;
    FF_TEST_ASSERT(FF_LinkRecv(tm_rx, sizeof(tm_rx), &tm_rx_len, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(tm_rx_len == g_pus_reply_len);
    FF_TEST_ASSERT(memcmp(tm_rx, g_pus_reply, g_pus_reply_len) == 0);
    FF_TEST_ASSERT(tm_rx[7] == FF_PUS_SERVICE_TEST); /* service 17 */
    FF_TEST_ASSERT(tm_rx[8] == 2u);                  /* subservice 2 = ping reply */

    FF_PusSetTmSink(NULL); /* R1-F02: do not leak the global sink to later tests */
}

FF_TEST_MAIN(test_seg_basic_roundtrip, test_seg_max_frame_rejected, test_seg_buffer_too_small,
             test_seg_null_args, test_seg_length_prefix_encoding, test_seg_roundtrip_two_frames,
             test_seg_fragmented_stream, test_seg_max_size_fragmented_partial,
             test_seg_read_error_propagation, test_seg_over_max_length_prefix,
             test_ip_init_udp_default, test_ip_null_config, test_ip_null_stack,
             test_ip_backend_name, test_ip_send_no_backend, test_ip_udp_send_recv_roundtrip,
             test_ip_tcp_send_segmented, test_ip_tcp_recv_reassembles, test_ip_tcp_partial_write,
             test_pus_ip_roundtrip)
