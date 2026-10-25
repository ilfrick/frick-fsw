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
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int g_master_fd = -1;
static char g_uart_temp_dir[] = "/tmp/frick-fsw-uart-XXXXXX";
static char g_uart_test_device[128];

#define UART_TX_TEST_SIZE 60000u
static uint8_t g_uart_tx_received[UART_TX_TEST_SIZE];

/* External link-time write() harness. It only injects a short write on the
 * UART slave descriptor so that the production partial-progress/retry loop in
 * `uart_send_until` is exercised deterministically; no production source is
 * modified and no flight behaviour changes. The harness is compiled only when
 * the linker supports --wrap (see tests/CMakeLists.txt). */
#ifdef FF_TEST_HAVE_WRITE_WRAP
extern ssize_t __real_write(int fd, const void *buf, size_t count);
#endif
static int g_uart_slave_fd = -1;
#ifdef FF_TEST_HAVE_WRITE_WRAP
static uint32_t g_uart_forced_short_writes;

ssize_t __wrap_write(int fd, const void *buf, size_t count)
{
    if ((fd == g_uart_slave_fd) && (g_uart_forced_short_writes > 0u) && (count > 1u)) {
        g_uart_forced_short_writes--;
        return __real_write(fd, buf, 1u); /* deliberate short write */
    }
    return __real_write(fd, buf, count);
}
#endif

/* Resolve the open descriptor that points at the PTY slave device. */
static int find_uart_slave_fd(const char *slave_name)
{
    DIR *dir = opendir("/proc/self/fd");
    int found = -1;
    if (dir == NULL) {
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char path[64];
        char link[256];
        ssize_t len;
        if (entry->d_name[0] == '.') {
            continue;
        }
        int n = snprintf(path, sizeof(path), "/proc/self/fd/%s", entry->d_name);
        if ((n <= 0) || ((size_t)n >= sizeof(path))) {
            continue;
        }
        len = readlink(path, link, sizeof(link) - 1u);
        if (len <= 0) {
            continue;
        }
        link[len] = '\0';
        if (strcmp(link, slave_name) == 0) {
            found = atoi(entry->d_name);
            break;
        }
    }
    (void)closedir(dir);
    return found;
}

typedef struct
{
    size_t target;
    size_t received;
    /* Written by the test thread and read by the drain thread. The project
     * requires C99 (CMakeLists.txt: CMAKE_C_STANDARD 99 with
     * C_STANDARD_REQUIRED ON), and C11 atomics (<stdatomic.h>, _Atomic) are
     * not part of that language level, so the flag is protected by a POSIX
     * pthread mutex instead: `volatile` is a type qualifier and provides no
     * inter-thread synchronization, so the previous access was a C11 data race
     * (NC-EXT-197). The toolchain happens to accept the C11 atomic builtins as
     * an extension, but the project must not depend on it. The mutex lives in
     * the same object as the flag, is initialized before the thread is created
     * and destroyed after the join, so the stack-object lifetime is unchanged. */
    pthread_mutex_t stop_lock;
    bool stop;
} FF_UartDrain_t;

/* Read the stop flag under the mutex. */
static bool uart_drain_stopped(FF_UartDrain_t *drain)
{
    bool stopped;
    (void)pthread_mutex_lock(&drain->stop_lock);
    stopped = drain->stop;
    (void)pthread_mutex_unlock(&drain->stop_lock);
    return stopped;
}

/* Request the drain thread to stop, under the mutex. */
static void uart_drain_request_stop(FF_UartDrain_t *drain)
{
    (void)pthread_mutex_lock(&drain->stop_lock);
    drain->stop = true;
    (void)pthread_mutex_unlock(&drain->stop_lock);
}

static void *uart_delayed_drain(void *arg)
{
    FF_UartDrain_t *drain = (FF_UartDrain_t *)arg;
    const struct timespec pause = {.tv_sec = 0, .tv_nsec = 15000000L};
    (void)nanosleep(&pause, NULL);
    while ((drain->received < drain->target) && !uart_drain_stopped(drain)) {
        ssize_t n = read(g_master_fd, &g_uart_tx_received[drain->received],
                         drain->target - drain->received);
        if (n > 0) {
            drain->received += (size_t)n;
        }
        else {
            const struct timespec retry = {.tv_sec = 0, .tv_nsec = 1000000L};
            (void)nanosleep(&retry, NULL);
        }
    }
    return NULL;
}

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
#ifdef FF_TEST_HAVE_WRITE_WRAP
    g_uart_slave_fd = find_uart_slave_fd(slave_name);
    FF_TEST_ASSERT(g_uart_slave_fd >= 0);
#endif
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
    {
        const uint8_t tx[] = {0xA5u, 0x5Au, 0x01u};
        uint8_t observed[sizeof(tx)];
        struct timespec now;
        FF_TEST_ASSERT(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
        uint64_t deadline =
            (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec + 50000000ULL;
        FF_TEST_ASSERT(FF_LinkUartBackend.send_until(tx, sizeof(tx), deadline) == FF_SUCCESS);
        FF_TEST_ASSERT(read(g_master_fd, observed, sizeof(observed)) == (ssize_t)sizeof(observed));
        FF_TEST_ASSERT(memcmp(tx, observed, sizeof(tx)) == 0);
        FF_TEST_ASSERT(FF_LinkUartBackend.send_until(tx, sizeof(tx), 1u) == FF_TIMEOUT);
    }
    {
        static uint8_t tx[UART_TX_TEST_SIZE];
        FF_UartDrain_t drain = {.target = sizeof(tx), .received = 0u, .stop = false};
        pthread_t drain_thread;
        struct timespec now;
        for (size_t i = 0u; i < sizeof(tx); i++) {
            tx[i] = (uint8_t)(i * 37u + 11u);
        }
        FF_TEST_ASSERT(pthread_mutex_init(&drain.stop_lock, NULL) == 0);
#ifdef FF_TEST_HAVE_WRITE_WRAP
        /* Force deterministic short writes on the UART slave descriptor so the
         * production partial-progress loop is exercised, not just assumed. */
        g_uart_forced_short_writes = 3u;
#endif
        FF_TEST_ASSERT(pthread_create(&drain_thread, NULL, uart_delayed_drain, &drain) == 0);
        FF_TEST_ASSERT(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
        uint64_t deadline =
            (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec + 1000000000ULL;
        FF_Status_t send_status = FF_LinkUartBackend.send_until(tx, sizeof(tx), deadline);
        /* On a failed send the reader must be released before joining, so the
         * case reports a located failure instead of hanging on bytes that
         * never arrive. */
        if (send_status != FF_SUCCESS) {
            uart_drain_request_stop(&drain);
        }
        FF_TEST_ASSERT(pthread_join(drain_thread, NULL) == 0);
        FF_TEST_ASSERT(pthread_mutex_destroy(&drain.stop_lock) == 0);
        FF_TEST_ASSERT(send_status == FF_SUCCESS);
#ifdef FF_TEST_HAVE_WRITE_WRAP
        /* All injected short writes were consumed by the retry loop. */
        FF_TEST_ASSERT(g_uart_forced_short_writes == 0u);
#endif
        FF_TEST_ASSERT(drain.received == sizeof(tx));
        FF_TEST_ASSERT(memcmp(tx, g_uart_tx_received, sizeof(tx)) == 0);
    }
    {
        static uint8_t stalled[UART_TX_TEST_SIZE];
        struct timespec now;
        FF_TEST_ASSERT(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
        uint64_t deadline =
            (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec + 20000000ULL;
        FF_TEST_ASSERT(FF_LinkUartBackend.send_until(stalled, sizeof(stalled), deadline) ==
                       FF_TIMEOUT);
        /* The bounded send may have pushed a partial frame onto the wire
         * before the deadline expired. The partial emission is observed (not
         * hidden) so the behaviour is characterized rather than assumed. */
        {
            uint8_t partial[UART_TX_TEST_SIZE];
            size_t total = 0u;
            for (;;) {
                ssize_t n = read(g_master_fd, &partial[total], sizeof(partial) - total);
                if (n <= 0) {
                    break;
                }
                total += (size_t)n;
                if (total >= sizeof(partial)) {
                    break;
                }
            }
            /* A partial emission is observed (not merely "fewer bytes"): the
             * bounded writer pushed part of the frame before the deadline. */
            FF_TEST_ASSERT(total > 0u);
            FF_TEST_ASSERT(total < sizeof(stalled));
        }
    }
    /* The bounded send must restore the descriptor mode it changed, so the
     * normal send path keeps its blocking semantics. */
#ifdef FF_TEST_HAVE_WRITE_WRAP
    {
        int flags = fcntl(g_uart_slave_fd, F_GETFL, 0);
        FF_TEST_ASSERT(flags >= 0);
        FF_TEST_ASSERT((flags & O_NONBLOCK) == 0);
    }
#endif
    (void)close(g_master_fd);
    g_master_fd = -1;
    /* Closing the PTY master simulates a disconnected UART. A zero-byte read
     * before VTIME expires must fail promptly, not busy-spin to the caller's
     * longer timeout. */
    {
        const uint8_t tx = 0xA5u;
        struct timespec now;
        FF_TEST_ASSERT(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
        uint64_t deadline =
            (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec + 50000000ULL;
        FF_TEST_ASSERT(FF_LinkUartBackend.send_until(&tx, 1u, deadline) == FF_ERROR);
    }
    FF_TEST_ASSERT(FF_LinkUartBackend.recv(rx, sizeof(rx), &len, 500u) == FF_ERROR);
    FF_TEST_ASSERT(len == 0u);
    (void)unlink(g_uart_test_device);
    (void)rmdir(g_uart_temp_dir);
}

FF_TEST_MAIN(test_uart_framing_split_coalesced_timeout_and_bounds)
