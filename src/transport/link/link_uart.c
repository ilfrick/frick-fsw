/*
 * frick-fsw - UART link (POSIX termios; valid on Linux and RTEMS 6)
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "link.h"
#include "fsw_config.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

static int g_fd = -1;

static FF_Status_t uart_init(void)
{
    struct termios tio;
    g_fd = open(FF_LINK_UART_DEVICE, O_RDWR | O_NOCTTY);
    if (g_fd < 0) {
        return FF_ERROR;
    }
    if (tcgetattr(g_fd, &tio) == 0) {
        cfmakeraw(&tio);
        cfsetispeed(&tio, B115200);
        cfsetospeed(&tio, B115200);
        (void)tcsetattr(g_fd, TCSANOW, &tio);
    }
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
    (void)timeout_ms;
    if (g_fd < 0) {
        return FF_NOT_READY;
    }
    ssize_t r = read(g_fd, data, cap);
    if (r < 0) {
        *len = 0u;
        return (errno == EAGAIN) ? FF_TIMEOUT : FF_ERROR;
    }
    *len = (uint16_t)r;
    return (r > 0) ? FF_SUCCESS : FF_TIMEOUT;
}

const FF_Link_t FF_LinkUartBackend = {
    .name = "uart",
    .init = uart_init,
    .send = uart_send,
    .recv = uart_recv,
};
