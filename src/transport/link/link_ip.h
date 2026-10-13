/*
 * frick-fsw - IP-based PUS-C link backend (M4-restored, SRS-REQ-060)
 *
 * Reference FF_Link_t backend that carries PUS-C TM/TC frames over an
 * IP-based link using the network-stack contract (FF_NetStack_t, SRS-REQ-027).
 *
 * Two transport modes:
 *   - UDP (reference): each datagram carries one CCSDS frame; the datagram
 *     boundary delimits the frame (no segmentation/reassembly).
 *   - TCP: frames are length-prefixed ([uint16 BE len][frame]) using the
 *     segmentation layer (link_seg.c, SRS-REQ-059).
 *
 * Selectable via FF_LinkSelect. The backend is initialised by calling
 * FF_LinkIpInit() with a config before selection.
 *
 * Reference defaults (mission ICD may override):
 *   mode  = UDP
 *   port  = 8080
 *   peer  = 127.0.0.1   (loopback for host tests / emulator)
 *
 * No dynamic allocation (SRS-REQ-015). All buffers are static.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_LINK_IP_H
#define FF_LINK_IP_H

#include "link.h"
#include "hal/hal_net.h"
#include "link_seg.h"

/* Reference defaults (mission ICD / fsw_config.h may override). */
#ifndef FF_LINK_IP_DEFAULT_PORT
#define FF_LINK_IP_DEFAULT_PORT 8080u
#endif

/* IP transport mode. */
typedef enum {
    FF_LINK_IPMODE_UDP,
    FF_LINK_IPMODE_TCP,
} FF_LinkIpMode_t;

/* Configuration: set via FF_LinkIpInit() before FF_LinkSelect. */
typedef struct
{
    FF_NetStack_t *stack; /* pluggable network stack (SRS-REQ-027) */
    FF_LinkIpMode_t mode; /* UDP datagram or TCP stream            */
    uint16_t port;        /* local UDP/TCP port                    */
    uint8_t peer_addr[4]; /* remote IPv4 (reference: 127.0.0.1)    */
} FF_LinkIpConfig_t;

/* Default config initialiser (reference profile). */
FF_LinkIpConfig_t FF_LinkIpDefaultConfig(FF_NetStack_t *stack);

/* Initialise and bind the IP backend. Must be called before FF_LinkSelect. */
FF_Status_t FF_LinkIpInit(const FF_LinkIpConfig_t *cfg);

/* Reference IP link backend (selectable via FF_LinkSelect). */
extern const FF_Link_t FF_LinkIpBackend;

#endif /* FF_LINK_IP_H */
