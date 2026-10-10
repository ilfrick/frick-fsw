/*
 * frick-fsw - HAL contract: pluggable TCP/IP network stack
 *
 * The framework addresses the network only through this stack-independent
 * vtable (SRS-REQ-027), so a user can plug Tuxera TCP/IP, lwIP, RTEMS libbsd
 * or their own stack by implementing an adapter; no core change is required.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_HAL_NET_H
#define FF_HAL_NET_H

#include "common/ff_types.h"

/* Stack-independent socket descriptor (opaque to the framework). */
typedef int FF_NetSock_t;

typedef struct FF_NetStack FF_NetStack_t;

struct FF_NetStack
{
    const char *name;
    FF_Status_t (*init)(void);

    /* Optional: bind a link-layer driver to the stack (netif registration). */
    FF_Status_t (*attach_netif)(void *netif);

    /* Socket-like API (blocking, bounded). `open`/`accept` return a negative
     * FF_NetSock_t on failure (FF_NetSock_t is signed for that reason). */
    FF_NetSock_t (*open)(int domain, int type, int proto);
    FF_Status_t (*connect)(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len);
    FF_Status_t (*bind)(FF_NetSock_t sd, const uint8_t *addr, uint16_t addr_len);
    FF_Status_t (*listen)(FF_NetSock_t sd, uint16_t backlog);
    FF_NetSock_t (*accept)(FF_NetSock_t sd, uint8_t *peer, uint16_t *peer_len);
    FF_Status_t (*send)(FF_NetSock_t sd, const uint8_t *buf, uint16_t len, uint16_t *sent);
    FF_Status_t (*recv)(FF_NetSock_t sd, uint8_t *buf, uint16_t cap, uint16_t *got,
                        uint32_t timeout_ms);
    FF_Status_t (*close)(FF_NetSock_t sd);
};

#endif /* FF_HAL_NET_H */
