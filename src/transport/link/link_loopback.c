/*
 * frick-fsw - Loopback link (in-memory, simulator and unit tests)
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "link.h"
#include "core/pal/pal.h"
#include <string.h>

#define FF_LOOP_RX_SIZE 8192u
#define FF_LOOP_TX_SIZE 8192u

static uint8_t      g_rx[FF_LOOP_RX_SIZE];
static uint16_t     g_rx_head;
static uint16_t     g_rx_tail;
static uint8_t      g_tx[FF_LOOP_TX_SIZE];
static uint16_t     g_tx_head;
static uint16_t     g_tx_tail;
static FF_MutexId_t g_lock;

static FF_Status_t loop_init(void)
{
    g_rx_head = g_rx_tail = 0u;
    g_tx_head = g_tx_tail = 0u;
    return FF_PalMutexCreate(&g_lock, "loop");
}

static uint16_t ring_avail(uint16_t head, uint16_t tail, uint16_t size)
{
    return (uint16_t)((head + size - tail) % size);
}

static FF_Status_t loop_send(const uint8_t *data, uint16_t len)
{
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    for (uint16_t i = 0; i < len; i++)
    {
        uint16_t next = (uint16_t)((g_tx_head + 1u) % FF_LOOP_TX_SIZE);
        if (next == g_tx_tail) { FF_PalMutexUnlock(g_lock); return FF_BUFFER_FULL; }
        g_tx[g_tx_head] = data[i];
        g_tx_head = next;
    }
    FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

static FF_Status_t loop_recv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms)
{
    (void)timeout_ms;
    uint16_t n = 0u;
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    while ((g_rx_head != g_rx_tail) && (n < cap))
    {
        data[n++] = g_rx[g_rx_tail];
        g_rx_tail = (uint16_t)((g_rx_tail + 1u) % FF_LOOP_RX_SIZE);
    }
    FF_PalMutexUnlock(g_lock);
    *len = n;
    return (n > 0u) ? FF_SUCCESS : FF_TIMEOUT;
}

const FF_Link_t FF_LinkLoopbackBackend = {
    .name = "loopback",
    .init = loop_init,
    .send = loop_send,
    .recv = loop_recv,
};

FF_Status_t FF_LinkLoopbackPush(const uint8_t *data, uint16_t len)
{
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    for (uint16_t i = 0; i < len; i++)
    {
        uint16_t next = (uint16_t)((g_rx_head + 1u) % FF_LOOP_RX_SIZE);
        if (next == g_rx_tail) { FF_PalMutexUnlock(g_lock); return FF_BUFFER_FULL; }
        g_rx[g_rx_head] = data[i];
        g_rx_head = next;
    }
    FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

FF_Status_t FF_LinkLoopbackPop(uint8_t *data, uint16_t cap, uint16_t *len)
{
    uint16_t n = 0u;
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    while ((g_tx_head != g_tx_tail) && (n < cap))
    {
        data[n++] = g_tx[g_tx_tail];
        g_tx_tail = (uint16_t)((g_tx_tail + 1u) % FF_LOOP_TX_SIZE);
    }
    FF_PalMutexUnlock(g_lock);
    *len = n;
    (void)ring_avail;
    return (n > 0u) ? FF_SUCCESS : FF_TIMEOUT;
}
