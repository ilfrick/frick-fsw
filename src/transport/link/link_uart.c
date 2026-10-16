/*
 * frick-fsw - UART link (POSIX termios; valid on Linux and RTEMS 6)
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "link.h"
#include "fsw_config.h"
#include "transport/ccsds/ccsds.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#ifdef FF_LINK_UART_DEVICE_ENV
#include <stdlib.h>
#endif

static int g_fd = -1;
static uint8_t g_rx_buf[FF_CCSDS_MAX_TC_PACKET];
static uint16_t g_rx_used;

static const char *uart_device_path(void)
{
#ifdef FF_LINK_UART_DEVICE_ENV
    const char *path = getenv(FF_LINK_UART_DEVICE_ENV);
    if ((path != NULL) && (path[0] != '\0')) {
        return path;
    }
#endif
    return FF_LINK_UART_DEVICE;
}

/* The backend has one receive owner (the command-ingest application); callers
 * must serialize recv calls. Per-call VTIME is POSIX/RTEMS decisecond-granular,
 * so short timeout requests can return up to one decisecond late, plus system
 * scheduling latency. This is not a target timing guarantee. */

static bool uart_monotonic_ms(uint64_t *ms)
{
    struct timespec now;
    if ((ms == NULL) || (clock_gettime(CLOCK_MONOTONIC, &now) != 0)) {
        return false;
    }
    *ms = ((uint64_t)now.tv_sec * 1000u) + ((uint64_t)now.tv_nsec / 1000000u);
    return true;
}

/* Return one complete PUS TC packet from the retained stream prefix. Invalid
 * or over-limit candidates advance by one octet; profile-specific primary
 * header rejection remains the CCSDS codec's responsibility. */
static FF_Status_t uart_extract_packet(uint8_t *data, uint16_t cap, uint16_t *len)
{
    const uint32_t min_packet =
        FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN + (FF_FRAME_WITH_CRC ? 2u : 0u);

    while (g_rx_used >= FF_CCSDS_HDR_LEN) {
        uint32_t packet_len =
            FF_CCSDS_HDR_LEN + ((uint32_t)(((uint16_t)g_rx_buf[4] << 8) | g_rx_buf[5]) + 1u);
        if ((packet_len < min_packet) || (packet_len > sizeof(g_rx_buf))) {
            memmove(g_rx_buf, &g_rx_buf[1], --g_rx_used);
            continue;
        }
        if (g_rx_used < packet_len) {
            return FF_NOT_READY;
        }
        if (cap < packet_len) {
            return FF_BAD_SIZE;
        }
        memcpy(data, g_rx_buf, packet_len);
        g_rx_used = (uint16_t)(g_rx_used - packet_len);
        if (g_rx_used > 0u) {
            memmove(g_rx_buf, &g_rx_buf[packet_len], g_rx_used);
        }
        *len = (uint16_t)packet_len;
        return FF_SUCCESS;
    }
    return FF_NOT_READY;
}

static FF_Status_t uart_init(void)
{
    struct termios tio;
    g_fd = open(uart_device_path(), O_RDWR | O_NOCTTY);
    if (g_fd < 0) {
        return FF_ERROR;
    }
    if ((tcgetattr(g_fd, &tio) != 0) || (cfsetispeed(&tio, B115200) != 0) ||
        (cfsetospeed(&tio, B115200) != 0)) {
        (void)close(g_fd);
        g_fd = -1;
        return FF_ERROR;
    }
    cfmakeraw(&tio);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;
    if (tcsetattr(g_fd, TCSANOW, &tio) != 0) {
        (void)close(g_fd);
        g_fd = -1;
        return FF_ERROR;
    }
    g_rx_used = 0u;
    return FF_SUCCESS;
}

static FF_Status_t uart_send(const uint8_t *data, uint16_t len)
{
    if (g_fd < 0) {
        return FF_NOT_READY;
    }
    ssize_t w = write(g_fd, data, len);
    return (w == (ssize_t)len) ? FF_SUCCESS : FF_ERROR;
}

static FF_Status_t uart_recv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms)
{
    uint64_t now;
    uint64_t deadline;
    if ((data == NULL) || (len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    *len = 0u;
    if (g_fd < 0) {
        return FF_NOT_READY;
    }
    if (!uart_monotonic_ms(&now)) {
        return FF_ERROR;
    }
    deadline = now + timeout_ms;
    for (;;) {
        FF_Status_t extracted = uart_extract_packet(data, cap, len);
        if (extracted != FF_NOT_READY) {
            return extracted;
        }

        if (!uart_monotonic_ms(&now)) {
            return FF_ERROR;
        }
        if ((timeout_ms == 0u) || (now >= deadline)) {
            return FF_TIMEOUT;
        }
        uint64_t remain = deadline - now;
        uint64_t wait_ds = (remain + 99u) / 100u;
        if (wait_ds > 255u) {
            wait_ds = 255u;
        }
        if (wait_ds == 0u) {
            wait_ds = 1u;
        }

        struct termios tio;
        if ((tcgetattr(g_fd, &tio) != 0)) {
            return FF_ERROR;
        }
        tio.c_cc[VMIN] = 0;
        tio.c_cc[VTIME] = (cc_t)wait_ds;
        if (tcsetattr(g_fd, TCSANOW, &tio) != 0) {
            return FF_ERROR;
        }

        size_t free_bytes = sizeof(g_rx_buf) - g_rx_used;
        uint64_t read_started_ms;
        if (!uart_monotonic_ms(&read_started_ms)) {
            return FF_ERROR;
        }
        ssize_t r = read(g_fd, &g_rx_buf[g_rx_used], free_bytes);
        if (r > 0) {
            g_rx_used = (uint16_t)(g_rx_used + (uint16_t)r);
            if (!uart_monotonic_ms(&now)) {
                return FF_ERROR;
            }
            if (now >= deadline) {
                /* Preserve data returned at/after expiry for a later call; do
                 * not report a packet after this caller's deadline. */
                return FF_TIMEOUT;
            }
            continue;
        }
        if (r == 0) {
            if (!uart_monotonic_ms(&now)) {
                return FF_ERROR;
            }
            /* With VMIN=0, a real VTIME expiry takes approximately the
             * configured decisecond interval. A zero-byte result much sooner
             * can mean terminal hangup (not timeout); return an error rather
             * than spinning until the caller's longer deadline. Allow clock
             * quantization and small scheduling/driver variation. */
            uint64_t elapsed_ms = (now >= read_started_ms) ? (now - read_started_ms) : 0u;
            uint64_t expected_wait_ms = wait_ds * 100u;
            if ((elapsed_ms + 10u) < expected_wait_ms) {
                return FF_ERROR;
            }
            if (now >= deadline) {
                return FF_TIMEOUT;
            }
            continue; /* VTIME chunk expired; continue toward long deadline. */
        }
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
            continue;
        }
        return FF_ERROR;
    }
}

const FF_Link_t FF_LinkUartBackend = {
    .name = "uart",
    .init = uart_init,
    .send = uart_send,
    .recv = uart_recv,
};
