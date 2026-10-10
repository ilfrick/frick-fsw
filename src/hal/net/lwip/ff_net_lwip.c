/*
 * frick-fsw - lwIP network-stack adapter implementation
 *
 * See ff_net_lwip.h. Built only when FF_USE_LWIP=ON.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "hal/net/lwip/ff_net_lwip.h"

#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include <string.h>

static int g_inited;

static void ff_lwip_fill_addr(struct sockaddr_in *sa, const uint8_t *addr, uint16_t addr_len)
{
    memset(sa, 0, sizeof(*sa));
    sa->sin_family = AF_INET;
    if ((addr != NULL) && (addr_len >= 6u)) {
        memcpy(&sa->sin_addr.s_addr, addr, 4u);
        sa->sin_port = htons((uint16_t)(((uint16_t)addr[4] << 8) | (uint16_t)addr[5]));
    }
}

static FF_Status_t ff_lwip_init(void)
{
    if (g_inited == 0) {
        /* NO_SYS=0 + netconn/socket: tcpip_init() runs lwip_init() and starts
         * the tcpip thread, initialising the core-locking mutex. */
        tcpip_init(NULL, NULL);
        g_inited = 1;
    }
    return FF_SUCCESS;
}

static FF_Status_t ff_lwip_attach_netif(void *netif)
{
    (void)netif;
    return FF_SUCCESS;
}

static FF_NetSock_t ff_lwip_open(int domain, int type, int proto)
{
    int sd;
    (void)ff_lwip_init();
    sd = lwip_socket(domain, type, proto);
    return (sd < 0) ? (FF_NetSock_t)-1 : (FF_NetSock_t)sd;
}

static FF_Status_t ff_lwip_connect(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len)
{
    struct sockaddr_in sa;
    if ((addr == NULL) || (addr_len < 6u)) {
        return FF_INVALID_ARGUMENT;
    }
    ff_lwip_fill_addr(&sa, addr, addr_len);
    return (lwip_connect(sd, (struct sockaddr *)&sa, sizeof(sa)) == 0) ? FF_SUCCESS : FF_ERROR;
}

static FF_Status_t ff_lwip_bind(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len)
{
    struct sockaddr_in sa;
    if ((addr == NULL) || (addr_len < 6u)) {
        return FF_INVALID_ARGUMENT;
    }
    ff_lwip_fill_addr(&sa, addr, addr_len);
    return (lwip_bind(sd, (struct sockaddr *)&sa, sizeof(sa)) == 0) ? FF_SUCCESS : FF_ERROR;
}

static FF_Status_t ff_lwip_listen(FF_NetSock_t sd, uint16_t backlog)
{
    return (lwip_listen(sd, (int)backlog) == 0) ? FF_SUCCESS : FF_ERROR;
}

static FF_NetSock_t ff_lwip_accept(FF_NetSock_t sd, uint8_t *peer, uint16_t *peer_len)
{
    struct sockaddr_in sa;
    socklen_t slen = sizeof(sa);
    int ret = lwip_accept(sd, (struct sockaddr *)&sa, &slen);
    if (ret < 0) {
        return (FF_NetSock_t)-1;
    }
    if ((peer != NULL) && (peer_len != NULL) && (*peer_len >= 6u)) {
        memcpy(peer, &sa.sin_addr.s_addr, 4u);
        peer[4] = (uint8_t)(ntohs(sa.sin_port) >> 8);
        peer[5] = (uint8_t)(ntohs(sa.sin_port));
    }
    return (FF_NetSock_t)ret;
}

static FF_Status_t ff_lwip_send(FF_NetSock_t sd, const uint8_t *buf, uint16_t len, uint16_t *sent)
{
    int n;
    if (sent == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    n = lwip_send(sd, buf, len, 0);
    if (n < 0) {
        return FF_ERROR;
    }
    *sent = (uint16_t)n;
    return FF_SUCCESS;
}

static FF_Status_t ff_lwip_recv(FF_NetSock_t sd, uint8_t *buf, uint16_t cap, uint16_t *got,
                                uint32_t timeout_ms)
{
    int n;
    if (got == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (timeout_ms > 0u) {
        struct timeval tv;
        tv.tv_sec = (time_t)(timeout_ms / 1000u);
        tv.tv_usec = (suseconds_t)((timeout_ms % 1000u) * 1000u);
        (void)lwip_setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
    n = lwip_recv(sd, buf, cap, 0);
    if (n < 0) {
        return FF_ERROR; /* includes the timeout/again case (see lwipopts) */
    }
    *got = (uint16_t)n;
    return FF_SUCCESS;
}

static FF_Status_t ff_lwip_close(FF_NetSock_t sd)
{
    return (lwip_close(sd) == 0) ? FF_SUCCESS : FF_ERROR;
}

FF_NetStack_t FF_NetStackLwip = {"lwip",         ff_lwip_init,    ff_lwip_attach_netif,
                                 ff_lwip_open,   ff_lwip_connect, ff_lwip_bind,
                                 ff_lwip_listen, ff_lwip_accept,  ff_lwip_send,
                                 ff_lwip_recv,   ff_lwip_close};
