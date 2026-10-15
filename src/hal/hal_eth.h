/*
 * frick-fsw - HAL contract: Ethernet MAC and network interface
 *
 * The MAC driver moves Ethernet frames; the net interface binds a MAC to a
 * pluggable TCP/IP stack (hal_net.h). The middleware depends only on these
 * contracts, never on a specific PHY/MAC/stack (SRS-REQ-026).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_ETH_H
#define FF_HAL_ETH_H

#include "common/ff_types.h"
#include "hal/hal_net.h"

/* Ethernet media access controller. Frames are complete Ethernet frames
 * (header + payload, CRC appended by the hardware). */
typedef struct FF_EthMac FF_EthMac_t;
typedef void (*FF_EthRxCb_t)(const uint8_t *frame, uint16_t len, void *ctx);

struct FF_EthMac
{
    const char *name;
    uint32_t mtu; /* maximum frame payload, e.g. 1500 */
    FF_Status_t (*init)(void *self);
    FF_Status_t (*get_mac_address)(void *self, uint8_t out[6]);
    FF_Status_t (*set_link_up)(void *self, uint8_t up);
    FF_Status_t (*transmit)(void *self, const uint8_t *frame, uint16_t len);
    void (*set_rx_callback)(void *self, FF_EthRxCb_t cb, void *ctx);
    FF_Status_t (*get_stats)(void *self, uint32_t *tx, uint32_t *rx, uint32_t *err);
};

/* Network interface: a MAC paired with a network stack. */
typedef struct FF_NetIf FF_NetIf_t;
struct FF_NetIf
{
    const char *name;
    FF_EthMac_t *mac;
    FF_NetStack_t *stack;
    uint8_t ipv4[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
    FF_Status_t (*init)(FF_NetIf_t *self);
    FF_Status_t (*max_frame)(FF_NetIf_t *self, uint16_t *out_mtu);
};

#endif /* FF_HAL_ETH_H */
