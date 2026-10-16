/*
 * frick-fsw - TSN-scheduled PUS-C link backend implementation
 *
 * See link_tsn.h. RX frames arrive via the MAC receive callback and are
 * buffered in a ring buffer; FF_LinkRecv reads from it.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "fsw_config.h"
#include "link_tsn.h"
#include "core/pal/pal.h"
#include <stdint.h>
#include <string.h>

/* Reference: broadcast destination MAC. */
#define FF_LINK_TSN_REF_MAC {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}

typedef struct
{
    uint16_t len;
    uint8_t data[FF_LINK_TSN_RX_FRAMESZ];
} FF_LinkTsnFrameSlot_t;

static FF_LinkTsnConfig_t g_cfg;
static uint8_t g_mac_addr[6];
static FF_MutexId_t g_lock;
static bool g_tsn_eth_inited = false;

static FF_LinkTsnFrameSlot_t g_rx_ring[FF_LINK_TSN_RX_SLOTS];
static uint16_t g_rx_head;
static uint16_t g_rx_tail;
static uint16_t g_rx_count;

static FF_Status_t tsn_eth_init(void);

static void tsn_eth_rx_cb(const uint8_t *frame, uint16_t len, void *ctx)
{
    (void)ctx;
    /* Drop frames that are too short to carry an Ethernet header + payload. */
    if (len <= FF_LINK_TSN_HDR_LEN) {
        return;
    }

    uint16_t payload_len = (uint16_t)(len - FF_LINK_TSN_HDR_LEN);
    /* Check EtherType matches our PUS-C EtherType. */
    uint16_t etype = ((uint16_t)frame[12] << 8) | (uint16_t)frame[13];
    if (etype != FF_LINK_TSN_ETHERTYPE) {
        return; /* not our frame: drop */
    }

    if (payload_len > FF_LINK_TSN_RX_FRAMESZ) {
        return; /* too large: drop */
    }

    /* Use non-blocking try-lock: this callback may fire from ISR context
     * where pthread_mutex_lock is not safe. If the lock is unavailable,
     * drop the frame rather than risk data corruption. */
    FF_Status_t lock_st = FF_PalMutexLock(g_lock, 0u);
    if (lock_st != FF_SUCCESS) {
        return; /* lock unavailable (ISR or contention): drop frame */
    }
    /* Ring-full check under the lock (fixes TOCTOU). */
    if (g_rx_count >= FF_LINK_TSN_RX_SLOTS) {
        (void)FF_PalMutexUnlock(g_lock);
        return; /* ring full: drop */
    }

    g_rx_ring[g_rx_head].len = payload_len;
    (void)memcpy(g_rx_ring[g_rx_head].data, &frame[FF_LINK_TSN_HDR_LEN], payload_len);
    g_rx_head = (uint16_t)((g_rx_head + 1u) % FF_LINK_TSN_RX_SLOTS);
    g_rx_count++;
    (void)FF_PalMutexUnlock(g_lock);
}

static FF_Status_t tsn_eth_init(void)
{
    FF_Status_t st = FF_PalMutexCreate(&g_lock, "tsn_link");
    if (st != FF_SUCCESS) {
        return st;
    }

    g_rx_head = 0u;
    g_rx_tail = 0u;
    g_rx_count = 0u;
    (void)memset(g_rx_ring, 0, sizeof(g_rx_ring));

    if (g_cfg.mac == NULL) {
        return FF_INVALID_ARGUMENT;
    }

    /* Initialise the MAC. */
    if (g_cfg.mac->init != NULL) {
        st = g_cfg.mac->init((void *)g_cfg.mac);
        if (st != FF_SUCCESS) {
            return st;
        }
    }

    /* Read the MAC address for frame construction. */
    st = g_cfg.mac->get_mac_address((void *)g_cfg.mac, g_mac_addr);
    if (st != FF_SUCCESS) {
        return st;
    }

    /* Configure TSN: VLAN/PCP if a TSN backend is present. */
    if (g_cfg.tsn != NULL) {
        if (g_cfg.tsn->set_vlan != NULL) {
            st = g_cfg.tsn->set_vlan((void *)g_cfg.tsn, g_cfg.vid, g_cfg.pcp);
            if (st != FF_SUCCESS) {
                return st;
            }
        }
    }

    /* Register the RX callback. */
    g_cfg.mac->set_rx_callback((void *)g_cfg.mac, tsn_eth_rx_cb, NULL);

    /* Bring the link up. */
    if (g_cfg.mac->set_link_up != NULL) {
        st = g_cfg.mac->set_link_up((void *)g_cfg.mac, 1u);
        if (st != FF_SUCCESS) {
            return st;
        }
    }

    return FF_SUCCESS;
}

FF_LinkTsnConfig_t FF_LinkTsnDefaultConfig(FF_EthMac_t *mac, FF_TsnBackend_t *tsn)
{
    FF_LinkTsnConfig_t cfg;
    cfg.mac = mac;
    cfg.tsn = tsn;
    cfg.vid = FF_LINK_TSN_DEFAULT_VID;
    cfg.pcp = FF_LINK_TSN_DEFAULT_PCP;
    /* Default: broadcast destination MAC. */
    cfg.dest_mac[0] = 0xFFu;
    cfg.dest_mac[1] = 0xFFu;
    cfg.dest_mac[2] = 0xFFu;
    cfg.dest_mac[3] = 0xFFu;
    cfg.dest_mac[4] = 0xFFu;
    cfg.dest_mac[5] = 0xFFu;
    return cfg;
}

FF_Status_t FF_LinkTsnInit(const FF_LinkTsnConfig_t *cfg)
{
    if (cfg == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (cfg->mac == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    (void)memset(&g_cfg, 0, sizeof(g_cfg));
    g_cfg = *cfg;
    g_rx_head = 0u;
    g_rx_tail = 0u;
    g_rx_count = 0u;
    g_tsn_eth_inited = false; /* allow re-init of MAC + callback via tsn_eth_init */
    return FF_SUCCESS;
}

/* ---- FF_Link_t vtable ---- */

static FF_Status_t tsn_link_init(void)
{
    if (g_tsn_eth_inited) {
        return FF_SUCCESS; /* already initialised */
    }
    FF_Status_t st = tsn_eth_init();
    if (st == FF_SUCCESS) {
        g_tsn_eth_inited = true;
    }
    return st;
}

static FF_Status_t tsn_link_send(const uint8_t *data, uint16_t len)
{
    if ((g_cfg.mac == NULL) || (g_cfg.mac->transmit == NULL)) {
        return FF_NOT_READY;
    }
    if ((data == NULL) || (len == 0u) || (len > FF_LINK_TSN_RX_FRAMESZ)) {
        return FF_INVALID_ARGUMENT;
    }

    /* Use a static frame buffer: the worst case (14-byte Eth header + up to
     * FF_LINK_TSN_RX_FRAMESZ payload + 2-byte pad) exceeds typical stack
     * budgets on the target. */
    static uint8_t frame[FF_LINK_TSN_HDR_LEN + FF_LINK_TSN_RX_FRAMESZ];
    uint16_t eth_len = 0u;

    /* Destination MAC. */
    (void)memcpy(&frame[eth_len], g_cfg.dest_mac, 6u);
    eth_len += 6u;

    /* Source MAC (from the MAC driver). */
    (void)memcpy(&frame[eth_len], g_mac_addr, 6u);
    eth_len += 6u;

    /* EtherType (big-endian). */
    frame[eth_len] = (uint8_t)((FF_LINK_TSN_ETHERTYPE >> 8) & 0xFFu);
    frame[eth_len + 1u] = (uint8_t)(FF_LINK_TSN_ETHERTYPE & 0xFFu);
    eth_len += 2u;

    /* Payload (CCSDS frame). */
    (void)memcpy(&frame[eth_len], data, len);
    eth_len += len;

    /* Pad to minimum Ethernet frame size (64 bytes including 4-byte FCS).
     * The MAC appends FCS, so we pad to 60 bytes of header+payload. */
    if (eth_len < 60u) {
        (void)memset(&frame[eth_len], 0, (uint16_t)(60u - eth_len));
        eth_len = 60u;
    }

    return g_cfg.mac->transmit((void *)g_cfg.mac, frame, eth_len);
}

static FF_Status_t tsn_link_recv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms)
{
    (void)timeout_ms; /* RX is callback-driven; ring is polled */
    if ((data == NULL) || (len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (cap < 1u) {
        return FF_INVALID_ARGUMENT;
    }

    FF_Status_t lock_st = FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    if (lock_st != FF_SUCCESS) {
        *len = 0u;
        return FF_ERROR;
    }
    if (g_rx_count == 0u) {
        (void)FF_PalMutexUnlock(g_lock);
        *len = 0u;
        return FF_TIMEOUT;
    }

    uint16_t frame_len = g_rx_ring[g_rx_tail].len;
    if (frame_len > cap) {
        /* Drop the frame: caller buffer too small. */
        g_rx_tail = (uint16_t)((g_rx_tail + 1u) % FF_LINK_TSN_RX_SLOTS);
        g_rx_count--;
        (void)FF_PalMutexUnlock(g_lock);
        return FF_BUFFER_FULL;
    }

    (void)memcpy(data, g_rx_ring[g_rx_tail].data, frame_len);
    *len = frame_len;
    g_rx_tail = (uint16_t)((g_rx_tail + 1u) % FF_LINK_TSN_RX_SLOTS);
    g_rx_count--;
    (void)FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

const FF_Link_t FF_LinkTsnBackend = {
    .name = "tsn_eth",
    .init = tsn_link_init,
    .send = tsn_link_send,
    .recv = tsn_link_recv,
};
