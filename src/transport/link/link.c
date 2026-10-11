/*
 * frick-fsw - Link selection and API
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "link.h"
#include "fsw_config.h"

static const FF_Link_t *g_link;

FF_Status_t FF_LinkInit(void)
{
    /* Default link: loopback on the host, UART on the RTEMS platforms. */
#if defined(FRICK_PLATFORM_MPFS) || defined(FRICK_PLATFORM_QEMU)
    g_link = &FF_LinkUartBackend;
#else
    g_link = &FF_LinkLoopbackBackend;
#endif
    if (g_link->init != NULL) {
        return g_link->init();
    }
    return FF_SUCCESS;
}

FF_Status_t FF_LinkSelect(const FF_Link_t *link)
{
    if (link == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    g_link = link;
    return (link->init != NULL) ? link->init() : FF_SUCCESS;
}

const FF_Link_t *FF_LinkGet(void)
{
    return g_link;
}

FF_Status_t FF_LinkSend(const uint8_t *data, uint16_t len)
{
    if ((g_link == NULL) || (g_link->send == NULL)) {
        return FF_NOT_READY;
    }
    return g_link->send(data, len);
}

FF_Status_t FF_LinkRecv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms)
{
    if ((g_link == NULL) || (g_link->recv == NULL)) {
        return FF_NOT_READY;
    }
    return g_link->recv(data, cap, len, timeout_ms);
}
