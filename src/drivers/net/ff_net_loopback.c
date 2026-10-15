/*
 * frick-fsw - Reference loopback network stack implementation
 *
 * See ff_net_loopback.h. Addresses are single-byte ports in addr[0].
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/net/ff_net_loopback.h"

#define FF_NETLOOP_MAX 4u
#define FF_NETLOOP_RING 256u
#define FF_NETLOOP_NONE 0xFFFFu

typedef struct
{
    uint8_t used;
    uint8_t bound;
    uint8_t listening;
    uint8_t port;
    uint16_t peer;    /* paired socket index or NONE */
    uint16_t pending; /* for a listening socket: the accepted peer */
    uint16_t head;
    uint16_t count;
    uint8_t rx[FF_NETLOOP_RING];
} FF_NetLoopSock_t;

static FF_NetLoopSock_t g_sock[FF_NETLOOP_MAX];

static FF_Status_t netloop_init(void)
{
    uint16_t i;
    for (i = 0u; i < FF_NETLOOP_MAX; i++) {
        g_sock[i].used = 0u;
        g_sock[i].listening = 0u;
        g_sock[i].port = 0u;
        g_sock[i].peer = FF_NETLOOP_NONE;
        g_sock[i].pending = FF_NETLOOP_NONE;
        g_sock[i].head = 0u;
        g_sock[i].count = 0u;
    }
    return FF_SUCCESS;
}

static FF_NetLoopSock_t *sock_of(FF_NetSock_t sd)
{
    if ((sd < 0) || (sd >= (FF_NetSock_t)FF_NETLOOP_MAX)) {
        return NULL;
    }
    if (g_sock[(uint16_t)sd].used == 0u) {
        return NULL;
    }
    return &g_sock[(uint16_t)sd];
}

static FF_NetSock_t netloop_open(int domain, int type, int proto)
{
    uint16_t i;
    (void)domain;
    (void)type;
    (void)proto;
    for (i = 0u; i < FF_NETLOOP_MAX; i++) {
        if (g_sock[i].used == 0u) {
            g_sock[i].used = 1u;
            g_sock[i].bound = 0u;
            g_sock[i].listening = 0u;
            g_sock[i].port = 0u;
            g_sock[i].peer = FF_NETLOOP_NONE;
            g_sock[i].pending = FF_NETLOOP_NONE;
            g_sock[i].head = 0u;
            g_sock[i].count = 0u;
            return (FF_NetSock_t)i;
        }
    }
    return -1;
}

static FF_Status_t netloop_bind(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    uint16_t i;
    if ((s == NULL) || (addr == NULL) || (addr_len < 1u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (s->bound != 0u) {
        return FF_ALREADY_EXISTS; /* this socket is already bound */
    }
    /* reject a port already bound by another socket (address already in use) */
    for (i = 0u; i < FF_NETLOOP_MAX; i++) {
        if ((i != (uint16_t)sd) && (g_sock[i].used != 0u) && (g_sock[i].bound != 0u) &&
            (g_sock[i].port == addr[0])) {
            return FF_ALREADY_EXISTS;
        }
    }
    s->bound = 1u;
    s->port = addr[0];
    return FF_SUCCESS;
}

static FF_Status_t netloop_listen(FF_NetSock_t sd, uint16_t backlog)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    (void)backlog;
    if ((s == NULL) || (s->bound == 0u)) {
        return FF_INVALID_ARGUMENT; /* must be bound first */
    }
    s->listening = 1u;
    return FF_SUCCESS;
}

static FF_Status_t netloop_connect(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    FF_NetSock_t srv;
    uint16_t i;
    if ((s == NULL) || (addr == NULL) || (addr_len < 1u)) {
        return FF_INVALID_ARGUMENT;
    }
    /* find a listening socket on the requested port */
    for (i = 0u; i < FF_NETLOOP_MAX; i++) {
        if ((g_sock[i].used != 0u) && (g_sock[i].listening != 0u) && (g_sock[i].port == addr[0])) {
            break;
        }
    }
    if (i >= FF_NETLOOP_MAX) {
        return FF_ERROR; /* nothing listening */
    }
    srv = netloop_open(0, 0, 0);
    if (srv < 0) {
        return FF_ERROR;
    }
    g_sock[srv].port = addr[0];
    g_sock[srv].peer = (uint16_t)sd;
    s->peer = (uint16_t)srv;
    g_sock[i].pending = (uint16_t)srv;
    return FF_SUCCESS;
}

static FF_NetSock_t netloop_accept(FF_NetSock_t sd, uint8_t *peer, uint16_t *peer_len)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    FF_NetSock_t ret;
    if (s == NULL) {
        return -1;
    }
    if (s->pending == FF_NETLOOP_NONE) {
        return -1;
    }
    ret = (FF_NetSock_t)s->pending;
    s->pending = FF_NETLOOP_NONE;
    if ((peer != NULL) && (peer_len != NULL) && (*peer_len >= 1u)) {
        peer[0] = g_sock[ret].port;
    }
    return ret;
}

static FF_Status_t netloop_send(FF_NetSock_t sd, const uint8_t *buf, uint16_t len, uint16_t *sent)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    FF_NetLoopSock_t *p;
    uint16_t i;
    if ((s == NULL) || (buf == NULL) || (sent == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((s->peer == FF_NETLOOP_NONE) || (g_sock[s->peer].used == 0u)) {
        return FF_ERROR;
    }
    p = &g_sock[s->peer];
    for (i = 0u; i < len; i++) {
        if (p->count >= FF_NETLOOP_RING) {
            break; /* peer buffer full */
        }
        p->rx[(p->head + p->count) % FF_NETLOOP_RING] = buf[i];
        p->count++;
    }
    *sent = i;
    return FF_SUCCESS;
}

static FF_Status_t netloop_recv(FF_NetSock_t sd, uint8_t *buf, uint16_t cap, uint16_t *got,
                                uint32_t timeout_ms)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    uint16_t i;
    (void)timeout_ms; /* loopback is non-blocking */
    if ((s == NULL) || (buf == NULL) || (got == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    for (i = 0u; i < cap; i++) {
        if (s->count == 0u) {
            break;
        }
        buf[i] = s->rx[s->head % FF_NETLOOP_RING];
        s->head = (uint16_t)((s->head + 1u) % FF_NETLOOP_RING);
        s->count--;
    }
    *got = i;
    return FF_SUCCESS;
}

static FF_Status_t netloop_close(FF_NetSock_t sd)
{
    FF_NetLoopSock_t *s = sock_of(sd);
    uint16_t k;
    if (s == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* clear any reference to this descriptor (peer pairing / pending accept) */
    for (k = 0u; k < FF_NETLOOP_MAX; k++) {
        if (g_sock[k].used != 0u) {
            if (g_sock[k].peer == (uint16_t)sd) {
                g_sock[k].peer = FF_NETLOOP_NONE;
            }
            if (g_sock[k].pending == (uint16_t)sd) {
                g_sock[k].pending = FF_NETLOOP_NONE;
            }
        }
    }
    s->used = 0u;
    s->bound = 0u;
    s->listening = 0u;
    s->peer = FF_NETLOOP_NONE;
    s->pending = FF_NETLOOP_NONE;
    return FF_SUCCESS;
}

static FF_Status_t netloop_attach_netif(void *netif)
{
    (void)netif;
    return FF_SUCCESS;
}

FF_NetStack_t FF_NetStackLoopback = {"loopback",     netloop_init,    netloop_attach_netif,
                                     netloop_open,   netloop_connect, netloop_bind,
                                     netloop_listen, netloop_accept,  netloop_send,
                                     netloop_recv,   netloop_close};
