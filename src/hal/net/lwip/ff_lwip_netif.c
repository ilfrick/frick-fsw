/*
 * frick-fsw - lwIP netif glue implementation
 *
 * See ff_lwip_netif.h. Bounded, single static instance; the only heap-style
 * allocation is the lwIP pbuf pool used to hand received frames to the stack
 * (the framework glue itself allocates nothing). Built only with FF_USE_LWIP=ON.
 *
 * Threading model: `linkoutput` and the lwIP core calls run in the tcpip
 * thread; `FF_LwipNetifAttach` is called from the application task and takes
 * the lwIP core lock (LOCK_TCPIP_CORE) to register the netif. Received frames
 * are handed over with `netif->input` (= tcpip_input), which posts them to the
 * tcpip thread, so the MAC RX callback (possibly an ISR/poll context) never
 * touches the lwIP core directly.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "hal/net/lwip/ff_lwip_netif.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include <string.h>

/* The glue owns exactly one netif (framework singleton per stack). */
typedef struct
{
    struct netif lwip;                           /* lwIP interface               */
    FF_NetIf_t *iface;                           /* framework netif (MAC + addr) */
    uint8_t tx_scratch[FF_LWIP_NETIF_FRAME_MAX]; /* linearised TX frame        */
    uint32_t tx_frames;                          /* frames handed to the MAC     */
    uint32_t rx_dropped;                         /* frames dropped (pool empty)  */
} ff_lwip_netif_t;

static ff_lwip_netif_t g_glue;
static int g_attached;

/* Transmit one frame through the framework MAC. Called from the tcpip thread
 * only, so the single static scratch buffer is not re-entrant. */
static err_t ff_lwip_netif_linkoutput(struct netif *netif, struct pbuf *p)
{
    ff_lwip_netif_t *g = (ff_lwip_netif_t *)netif->state;
    FF_EthMac_t *mac;
    uint16_t len;

    if ((g == NULL) || (g->iface == NULL) || (g->iface->mac == NULL)) {
        return ERR_IF;
    }
    mac = g->iface->mac;
    len = p->tot_len;
    if ((len == 0u) || (len > FF_LWIP_NETIF_FRAME_MAX)) {
        return ERR_MEM;
    }
    if (pbuf_copy_partial(p, g->tx_scratch, len, 0u) != len) {
        return ERR_MEM;
    }
    if (mac->transmit(mac, g->tx_scratch, len) != FF_SUCCESS) {
        return ERR_IF;
    }
    g->tx_frames++;
    return ERR_OK;
}

/* Feed one received frame into the stack. Called by the framework MAC (e.g.
 * FF_GemPollRx) for every completed RX frame; a pbuf is taken from the lwIP
 * pool and posted to the tcpip thread via netif->input (tcpip_input). Bounded:
 * the frame is dropped (and counted) when the pool is exhausted. */
static void ff_lwip_netif_rx(const uint8_t *frame, uint16_t len, void *ctx)
{
    ff_lwip_netif_t *g = (ff_lwip_netif_t *)ctx;
    struct pbuf *p;

    if ((g == NULL) || (frame == NULL) || (len == 0u)) {
        return;
    }
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (p == NULL) {
        g->rx_dropped++;
        return;
    }
    if (pbuf_take(p, frame, len) != ERR_OK) {
        g->rx_dropped++;
        pbuf_free(p);
        return;
    }
    if (g->lwip.input(p, &g->lwip) != ERR_OK) {
        pbuf_free(p);
    }
}

/* lwIP netif initialiser: fill the link-layer parameters from the framework
 * MAC. `output` is the IPv4/ARP path; `linkoutput` is the framework transmit. */
static err_t ff_lwip_netif_init(struct netif *netif)
{
    ff_lwip_netif_t *g = (ff_lwip_netif_t *)netif->state;
    FF_EthMac_t *mac;
    uint8_t hw[6];

    if ((g == NULL) || (g->iface == NULL) || (g->iface->mac == NULL)) {
        return ERR_IF;
    }
    mac = g->iface->mac;
    if (mac->get_mac_address(mac, hw) != FF_SUCCESS) {
        return ERR_IF;
    }
    netif->name[0] = 'g';
    netif->name[1] = '0';
    netif->output = etharp_output;
    netif->linkoutput = ff_lwip_netif_linkoutput;
    /* IP MTU (not the MAC frame size): a frame-size MTU would let lwIP emit
     * frames the stack cannot carry. Standard Ethernet IP MTU = 1500. */
    netif->mtu = 1500u;
    netif->hwaddr_len = 6u;
    memcpy(netif->hwaddr, hw, sizeof(hw));
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    return ERR_OK;
}

FF_Status_t FF_LwipNetifAttach(FF_NetIf_t *nif)
{
    ff_lwip_netif_t *g = &g_glue;
    FF_EthMac_t *mac;
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gw;

    if ((nif == NULL) || (nif->mac == NULL) || (nif->mac->init == NULL) ||
        (nif->mac->get_mac_address == NULL) || (nif->mac->transmit == NULL) ||
        (nif->mac->set_rx_callback == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_attached != 0) {
        return FF_ALREADY_EXISTS;
    }
    mac = nif->mac;

    memset(g, 0, sizeof(*g));
    g->iface = nif;
    IP4_ADDR(&ip, nif->ipv4[0], nif->ipv4[1], nif->ipv4[2], nif->ipv4[3]);
    IP4_ADDR(&netmask, nif->netmask[0], nif->netmask[1], nif->netmask[2], nif->netmask[3]);
    IP4_ADDR(&gw, nif->gateway[0], nif->gateway[1], nif->gateway[2], nif->gateway[3]);

    LOCK_TCPIP_CORE();
    if (netif_add(&g->lwip, &ip, &netmask, &gw, g, ff_lwip_netif_init, tcpip_input) == NULL) {
        UNLOCK_TCPIP_CORE();
        return FF_ERROR;
    }
    netif_set_default(&g->lwip);
    netif_set_up(&g->lwip);
    netif_set_link_up(&g->lwip);
    UNLOCK_TCPIP_CORE();

    mac->set_rx_callback(mac, ff_lwip_netif_rx, g);
    g_attached = 1;
    return FF_SUCCESS;
}

int FF_LwipNetifIsUp(void)
{
    return ((g_attached != 0) && netif_is_up(&g_glue.lwip) && netif_is_link_up(&g_glue.lwip)) ? 1
                                                                                              : 0;
}

FF_Status_t FF_LwipNetifGetMac(uint8_t out[6])
{
    if ((out == NULL) || (g_attached == 0)) {
        return FF_INVALID_ARGUMENT;
    }
    memcpy(out, g_glue.lwip.hwaddr, 6u);
    return FF_SUCCESS;
}
