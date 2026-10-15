/*
 * frick-fsw - IP-based PUS-C link backend implementation
 *
 * See link_ip.h. UDP datagrams carry one CCSDS frame each; TCP uses the
 * segmentation layer (2-byte BE length prefix).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "fsw_config.h"
#include "link_ip.h"
#include "link_seg.h"
#include "drivers/net/ff_net_loopback.h"
#include <string.h>

/* Reference peer address (127.0.0.1) for host tests / emulator. */
#define FF_LINK_IP_REF_PEER                                                                        \
    {                                                                                              \
        127u, 0u, 0u, 1u                                                                           \
    }

static FF_LinkIpConfig_t g_cfg;
static FF_NetSock_t g_sock = -1;
static FF_LinkSeg_t g_seg; /* TCP stream framing context            */

/* ---- Byte-stream bridge for the segmentation layer (TCP mode) ---- */

static FF_Status_t ip_write_bytes(void *ctx, const uint8_t *data, uint16_t len)
{
    FF_NetStack_t *stack = (FF_NetStack_t *)ctx;
    uint16_t sent = 0u;
    FF_Status_t st = stack->send(g_sock, data, len, &sent);
    if (st != FF_SUCCESS) {
        return st;
    }
    if (sent < len) {
        return FF_BUFFER_FULL; /* partial write: treat as full */
    }
    return FF_SUCCESS;
}

static FF_Status_t ip_read_bytes(void *ctx, uint8_t *data, uint16_t cap, uint16_t *len,
                                 uint32_t timeout_ms)
{
    FF_NetStack_t *stack = (FF_NetStack_t *)ctx;
    uint16_t got = 0u;
    FF_Status_t st = stack->recv(g_sock, data, cap, &got, timeout_ms);
    if (st != FF_SUCCESS) {
        return st;
    }
    *len = got;
    return FF_SUCCESS;
}

static FF_LinkByteStream_t g_byte_stream = {
    .write_bytes = ip_write_bytes,
    .read_bytes = ip_read_bytes,
    .ctx = NULL, /* set in ip_init_tcp */
};

FF_LinkIpConfig_t FF_LinkIpDefaultConfig(FF_NetStack_t *stack)
{
    FF_LinkIpConfig_t cfg;
    cfg.stack = stack;
    cfg.mode = FF_LINK_IPMODE_UDP;
    cfg.port = FF_LINK_IP_DEFAULT_PORT;
    cfg.peer_addr[0] = 127u;
    cfg.peer_addr[1] = 0u;
    cfg.peer_addr[2] = 0u;
    cfg.peer_addr[3] = 1u;
    return cfg;
}

static FF_Status_t ip_init_udp(FF_NetStack_t *stack)
{
    uint8_t addr[1]; /* reference loopback: single-byte port */
    addr[0] = (uint8_t)(g_cfg.port & 0xFFu);
    FF_Status_t st = stack->init();
    if (st != FF_SUCCESS) {
        return st;
    }
    g_sock = stack->open(0, 0, 0);
    if (g_sock < 0) {
        return FF_NO_RESOURCE;
    }
    st = stack->bind(g_sock, addr, 1u);
    if (st != FF_SUCCESS) {
        (void)stack->close(g_sock);
        g_sock = -1;
        return st;
    }
    /* For UDP: bind and optionally connect to a peer for send. On a real IP
     * stack, connect sets the default destination for send(); on reference
     * stacks that model UDP as TCP-like (bind+listen needed), connect may
     * fail — that is non-fatal: the backend still binds. */
    uint8_t peer[1];
    peer[0] = (uint8_t)(g_cfg.peer_addr[3] & 0xFFu); /* reference: low byte of addr */
    (void)stack->connect(g_sock, peer, 1u);
    return FF_SUCCESS;
}

static FF_Status_t ip_init_tcp(FF_NetStack_t *stack)
{
    uint8_t addr[1];
    addr[0] = (uint8_t)(g_cfg.port & 0xFFu);
    FF_Status_t st = stack->init();
    if (st != FF_SUCCESS) {
        return st;
    }
    g_sock = stack->open(0, 0, 0);
    if (g_sock < 0) {
        return FF_NO_RESOURCE;
    }
    st = stack->bind(g_sock, addr, 1u);
    if (st != FF_SUCCESS) {
        (void)stack->close(g_sock);
        g_sock = -1;
        return st;
    }
    st = stack->listen(g_sock, 1u);
    if (st != FF_SUCCESS) {
        (void)stack->close(g_sock);
        g_sock = -1;
        return st;
    }
    /* Accept blocks until a peer connects. This is acceptable for the reference
     * backend init phase: TCP link establishment is expected during setup, not
     * in the real-time data path. A production integrator may override
     * accept() with a timeout via their FF_NetStack_t implementation. */
    int peer_sd = stack->accept(g_sock, NULL, NULL);
    if (peer_sd < 0) {
        (void)stack->close(g_sock);
        g_sock = -1;
        return FF_ERROR;
    }
    /* Switch the sock handle to the accepted peer for send/recv. */
    g_sock = (FF_NetSock_t)peer_sd;

    /* Configure the byte-stream bridge for the seg layer. */
    g_byte_stream.ctx = (void *)stack;
    FF_LinkSegInit(&g_seg, g_byte_stream);
    return FF_SUCCESS;
}

FF_Status_t FF_LinkIpInit(const FF_LinkIpConfig_t *cfg)
{
    if (cfg == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (cfg->stack == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    (void)memset(&g_cfg, 0, sizeof(g_cfg));
    g_cfg = *cfg;
    g_sock = -1;

    if (g_cfg.mode == FF_LINK_IPMODE_TCP) {
        return ip_init_tcp(g_cfg.stack);
    }
    return ip_init_udp(g_cfg.stack);
}

/* ---- FF_Link_t vtable ---- */

static FF_Status_t ip_init(void)
{
    /* Reference-only fallback: if no stack was configured via FF_LinkIpInit(),
     * use the in-memory loopback for host tests/emulator. In production,
     * integrators must call FF_LinkIpInit() with a real FF_NetStack_t. */
    if (g_cfg.stack == NULL) {
        /* Use the reference loopback stack if none provided. */
        g_cfg.stack = &FF_NetStackLoopback;
        g_cfg.mode = FF_LINK_IPMODE_UDP;
        g_cfg.port = FF_LINK_IP_DEFAULT_PORT;
        g_cfg.peer_addr[0] = 127u;
        g_cfg.peer_addr[1] = 0u;
        g_cfg.peer_addr[2] = 0u;
        g_cfg.peer_addr[3] = 1u;
    }
    if (g_sock >= 0) {
        return FF_SUCCESS; /* already initialised */
    }
    if (g_cfg.mode == FF_LINK_IPMODE_TCP) {
        return ip_init_tcp(g_cfg.stack);
    }
    return ip_init_udp(g_cfg.stack);
}

static FF_Status_t ip_send(const uint8_t *data, uint16_t len)
{
    if (g_sock < 0) {
        return FF_NOT_READY;
    }
    if (g_cfg.stack == NULL) {
        return FF_NOT_READY;
    }
    FF_NetStack_t *stack = g_cfg.stack;

    if (g_cfg.mode == FF_LINK_IPMODE_TCP) {
        return FF_LinkSegSendFrame(&g_seg, data, len);
    }

    /* UDP: one datagram = one frame. */
    uint16_t sent = 0u;
    FF_Status_t st = stack->send(g_sock, data, len, &sent);
    if (st != FF_SUCCESS) {
        return st;
    }
    /* Consistent error code for partial write (matches ip_write_bytes). */
    return (sent == len) ? FF_SUCCESS : FF_BUFFER_FULL;
}

static FF_Status_t ip_recv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms)
{
    if (g_sock < 0) {
        return FF_NOT_READY;
    }
    if (g_cfg.stack == NULL) {
        return FF_NOT_READY;
    }
    FF_NetStack_t *stack = g_cfg.stack;

    if (g_cfg.mode == FF_LINK_IPMODE_TCP) {
        return FF_LinkSegRecvFrame(&g_seg, data, cap, len, timeout_ms);
    }

    /* UDP: one datagram = one frame. */
    uint16_t got = 0u;
    FF_Status_t st = stack->recv(g_sock, data, cap, &got, timeout_ms);
    if (st != FF_SUCCESS) {
        return st;
    }
    if (got == 0u) {
        return FF_TIMEOUT;
    }
    *len = got;
    return FF_SUCCESS;
}

const FF_Link_t FF_LinkIpBackend = {
    .name = "ip_link", /* mode-agnostic: UDP or TCP at init time */
    .init = ip_init,
    .send = ip_send,
    .recv = ip_recv,
};
