/*
 * frick-fsw - UART packet-framing integration test (host pseudo-terminal)
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#define _XOPEN_SOURCE 700
#include "../ff_test.h"
#include "transport/link/link.h"
#include "transport/ccsds/ccsds.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int g_master_fd = -1;
static char g_uart_temp_dir[] = "/tmp/frick-fsw-uart-XXXXXX";
static char g_uart_test_device[128];

typedef struct
{
    const uint8_t *data;
    size_t len;
    ssize_t written;
} FF_UartWriter_t;

static void *uart_delayed_writer(void *arg)
{
    FF_UartWriter_t *writer = (FF_UartWriter_t *)arg;
    const struct timespec delay = {.tv_sec = 0, .tv_nsec = 10000000L};
    (void)nanosleep(&delay, NULL);
    writer->written = write(g_master_fd, writer->data, writer->len);
    return NULL;
}

static void make_frame(uint8_t frame[13], uint8_t marker)
{
    static const uint8_t hdr[6] = {0x18u, 0x10u, 0xC0u, 0x00u, 0x00u, 0x06u};
    memcpy(frame, hdr, sizeof(hdr));
    frame[6] = 0x20u;
    frame[7] = marker;
    frame[8] = 0x01u;
    frame[9] = 0x02u;
    frame[10] = 0x03u;
    frame[11] = 0xA5u;
    frame[12] = 0x5Au;
}

static void make_max_frame(uint8_t frame[FF_CCSDS_MAX_TC_PACKET])
{
    memset(frame, 0xA5, FF_CCSDS_MAX_TC_PACKET);
    frame[0] = 0x18u;
    frame[1] = 0x10u;
    frame[2] = 0xC0u;
    frame[3] = 0x00u;
    frame[4] = (uint8_t)((FF_CCSDS_MAX_TC_PACKET - 7u) >> 8);
    frame[5] = (uint8_t)(FF_CCSDS_MAX_TC_PACKET - 7u);
}

static void write_all(const uint8_t *data, size_t len)
{
    size_t pos = 0u;
    while (pos < len) {
        ssize_t written = write(g_master_fd, &data[pos], len - pos);
        FF_TEST_ASSERT(written > 0);
        pos += (size_t)written;
    }
}

static void setup_uart_pty(void)
{
    char *slave_name;
    char *temp_dir = mkdtemp(g_uart_temp_dir);
    FF_TEST_ASSERT(temp_dir != NULL);
    int path_len = snprintf(g_uart_test_device, sizeof(g_uart_test_device), "%s/pty", temp_dir);
    FF_TEST_ASSERT((path_len > 0) && ((size_t)path_len < sizeof(g_uart_test_device)));
    FF_TEST_ASSERT(setenv("FRICK_TEST_UART_DEVICE", g_uart_test_device, 1) == 0);
    g_master_fd = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
    FF_TEST_ASSERT(g_master_fd >= 0);
    FF_TEST_ASSERT(grantpt(g_master_fd) == 0);
    FF_TEST_ASSERT(unlockpt(g_master_fd) == 0);
    slave_name = ptsname(g_master_fd);
    FF_TEST_ASSERT(slave_name != NULL);
    FF_TEST_ASSERT(symlink(slave_name, g_uart_test_device) == 0);
    FF_TEST_ASSERT(FF_LinkUartBackend.init() == FF_SUCCESS);
}

static void test_uart_framing_split_coalesced_timeout_and_bounds(void)
{
    uint8_t frame_a[13];
    uint8_t frame_b[13];
    uint8_t frame_c[13];
    uint8_t plausible_packet[FF_CCSDS_MAX_TC_PACKET];
    FF_UartWriter_t delayed;
    pthread_t writer_thread;
    uint8_t plausible_max[6] = {0x18u, 0x10u, 0xC0u, 0x00u, 0x04u, 0x06u};
    uint8_t max_frame[FF_CCSDS_MAX_TC_PACKET];
    uint8_t rx[FF_CCSDS_MAX_TC_PACKET];
    uint8_t malformed[6] = {0x18u, 0x10u, 0xC0u, 0x00u, 0xFFu, 0xFFu};
    uint16_t len = 0xA55Au;

    setup_uart_pty();
    make_frame(frame_a, 0x11u);
    make_frame(frame_b, 0x22u);
    make_frame(frame_c, 0x33u);

    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 2u) == FF_TIMEOUT);
    FF_TEST_ASSERT(len == 0u);

    /* Partial packet survives the receive timeout and joins with later bytes. */
    write_all(frame_a, 4u);
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 2u) == FF_TIMEOUT);
    FF_TEST_ASSERT(len == 0u);
    write_all(&frame_a[4], sizeof(frame_a) - 4u);
    write_all(frame_b, sizeof(frame_b));

    /* A too-small caller buffer does not consume the complete packet. */
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, 12u, &len, 20u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(len == 0u);
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 20u) == FF_SUCCESS);
    FF_TEST_ASSERT(len == sizeof(frame_a));
    FF_TEST_ASSERT(memcmp(rx, frame_a, sizeof(frame_a)) == 0);

    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 20u) == FF_SUCCESS);
    FF_TEST_ASSERT(len == sizeof(frame_b));
    FF_TEST_ASSERT(memcmp(rx, frame_b, sizeof(frame_b)) == 0);

    /* Over-limit PDL candidate is discarded bytewise; the following complete
     * packet is recovered without writing beyond the bounded receive buffer. */
    write_all(malformed, sizeof(malformed));
    write_all(frame_c, sizeof(frame_c));
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 20u) == FF_SUCCESS);
    FF_TEST_ASSERT(len == sizeof(frame_c));
    FF_TEST_ASSERT(memcmp(rx, frame_c, sizeof(frame_c)) == 0);

    make_max_frame(max_frame);
    write_all(max_frame, sizeof(max_frame));
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 20u) == FF_SUCCESS);
    FF_TEST_ASSERT(len == FF_CCSDS_MAX_TC_PACKET);
    FF_TEST_ASSERT(memcmp(rx, max_frame, sizeof(max_frame)) == 0);

    /* Delayed input with a very large caller timeout confirms the bounded
     * VTIME read path waits for data; it is not a 25.5-second expiry test. */
    delayed.data = frame_a;
    delayed.len = sizeof(frame_a);
    delayed.written = -1;
    FF_TEST_ASSERT(pthread_create(&writer_thread, NULL, uart_delayed_writer, &delayed) == 0);
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, UINT32_MAX) == FF_SUCCESS);
    FF_TEST_ASSERT(pthread_join(writer_thread, NULL) == 0);
    FF_TEST_ASSERT(delayed.written == (ssize_t)sizeof(frame_a));
    FF_TEST_ASSERT(len == sizeof(frame_a));
    FF_TEST_ASSERT(memcmp(rx, frame_a, sizeof(frame_a)) == 0);

    /* A valid-looking maximum-length candidate is ambiguous without an
     * independent sync marker. Do not guess that the following packet begins
     * inside its declared payload; return timeout and retain the candidate. */
    write_all(plausible_max, sizeof(plausible_max));
    write_all(frame_c, sizeof(frame_c));
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 2u) == FF_TIMEOUT);
    FF_TEST_ASSERT(len == 0u);
    memcpy(plausible_packet, plausible_max, sizeof(plausible_max));
    memcpy(&plausible_packet[sizeof(plausible_max)], frame_c, sizeof(frame_c));
    memset(&plausible_packet[sizeof(plausible_max) + sizeof(frame_c)], 0x6Bu,
           sizeof(plausible_packet) - sizeof(plausible_max) - sizeof(frame_c));
    write_all(&plausible_packet[sizeof(plausible_max) + sizeof(frame_c)],
              sizeof(plausible_packet) - sizeof(plausible_max) - sizeof(frame_c));
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 20u) == FF_SUCCESS);
    FF_TEST_ASSERT(len == FF_CCSDS_MAX_TC_PACKET);
    FF_TEST_ASSERT(memcmp(rx, plausible_packet, sizeof(plausible_packet)) == 0);

    FF_TEST_ASSERT(FF_LinkUartBackend.recv(NULL, sizeof(rx), &len, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), NULL, 0u) == FF_INVALID_ARGUMENT);
    (void)close(g_master_fd);
    g_master_fd = -1;
    /* Closing the PTY master simulates a disconnected UART. A zero-byte read
     * before VTIME expires must fail promptly, not busy-spin to the caller's
     * longer timeout. */
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 500u) == FF_ERROR);
    FF_TEST_ASSERT(len == 0u);
    (void)unlink(g_uart_test_device);
    (void)rmdir(g_uart_temp_dir);
}

FF_TEST_MAIN(test_uart_framing_split_coalesced_timeout_and_bounds)
