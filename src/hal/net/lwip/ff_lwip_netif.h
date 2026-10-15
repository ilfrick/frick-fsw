/*
 * frick-fsw - lwIP netif glue (framework FF_NetIf_t <-> lwIP struct netif)
 *
 * Binds a framework network interface (FF_NetIf_t: an FF_EthMac_t MAC plus the
 * IPv4 addressing) to the lwIP stack (SRS-REQ-040, S6b-2). It registers one
 * lwIP `struct netif` whose `linkoutput` transmits through the framework MAC
 * and whose received frames are fed back from the MAC RX callback into the
 * tcpip thread. This is the implementation behind
 * `FF_NetStack_t::attach_netif` (which the adapter passes to this glue).
 *
 * The glue is bounded and allocation-free (static single instance; only the
 * lwIP objects that lwIP itself requires are allocated). It is built only with
 * FF_USE_LWIP=ON; on both the host (lwIP unix port) and the RTEMS target (the
 * in-repo sys_arch port). See docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_LWIP_NETIF_H
#define FF_LWIP_NETIF_H

#include "hal/hal_eth.h"

/* Largest Ethernet frame (header + payload; the hardware appends the FCS) the
 * glue will linearise and hand to the MAC. Matches the GEM driver MTU. */
#define FF_LWIP_NETIF_FRAME_MAX 1536u

/* Attach the framework network interface to lwIP: add a netif whose MAC is
 * `nif->mac`, copy the IPv4 address/netmask/gateway from `nif`, register the
 * MAC receive callback and bring the netif up (netif_set_up + link up). The
 * MAC must already be initialised by the caller (FF_EthMac_t::init).
 *
 * Returns FF_SUCCESS when the netif is registered and up,
 * FF_INVALID_ARGUMENT on a NULL/incomplete interface, FF_ALREADY_EXISTS if a
 * netif is already attached, FF_ERROR if lwIP rejects the netif. */
FF_Status_t FF_LwipNetifAttach(FF_NetIf_t *nif);

/* Non-zero when a netif is attached and reports up and link-up
 * (netif_is_up() && netif_is_link_up()). Read-only status query. */
int FF_LwipNetifIsUp(void);

/* Copy the attached netif hardware address (6 octets). Returns FF_SUCCESS, or
 * FF_INVALID_ARGUMENT when `out` is NULL or no netif is attached. */
FF_Status_t FF_LwipNetifGetMac(uint8_t out[6]);

#endif /* FF_LWIP_NETIF_H */
