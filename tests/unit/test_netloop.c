/*
 * frick-fsw - Reference loopback network stack unit tests (milestone M4)
 *
 * Exercises the pluggable network-stack contract (SRS-REQ-039).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/net/ff_net_loopback.h"
#include <string.h>

static void test_loopback_stream(void)
{
    FF_NetStack_t *s = &FF_NetStackLoopback;
    FF_NetSock_t srv;
    FF_NetSock_t cli;
    FF_NetSock_t acc;
    uint8_t addr[1] = {7u};
    uint16_t sent = 0u;
    uint16_t got = 0u;
    uint8_t buf[32];
    static const uint8_t msg[5] = {1u, 2u, 3u, 4u, 5u};
    uint8_t peer[1];
    uint16_t peer_len = 1u;

    FF_TEST_ASSERT(s->init() == FF_SUCCESS);
    srv = s->open(0, 0, 0);
    FF_TEST_ASSERT(srv >= 0);
    FF_TEST_ASSERT(s->bind(srv, addr, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->listen(srv, 1u) == FF_SUCCESS);
    cli = s->open(0, 0, 0);
    FF_TEST_ASSERT(cli >= 0);
    FF_TEST_ASSERT(s->connect(cli, addr, 1u) == FF_SUCCESS);
    acc = s->accept(srv, peer, &peer_len);
    FF_TEST_ASSERT(acc >= 0);

    FF_TEST_ASSERT(s->send(cli, msg, sizeof(msg), &sent) == FF_SUCCESS);
    FF_TEST_ASSERT(sent == sizeof(msg));
    FF_TEST_ASSERT(s->recv(acc, buf, sizeof(buf), &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == sizeof(msg));
    FF_TEST_ASSERT(memcmp(buf, msg, sizeof(msg)) == 0);

    FF_TEST_ASSERT(s->send(acc, msg, sizeof(msg), &sent) == FF_SUCCESS);
    FF_TEST_ASSERT(s->recv(cli, buf, sizeof(buf), &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == sizeof(msg));
    FF_TEST_ASSERT(memcmp(buf, msg, sizeof(msg)) == 0);

    FF_TEST_ASSERT(s->close(acc) == FF_SUCCESS);
    FF_TEST_ASSERT(s->close(cli) == FF_SUCCESS);
    FF_TEST_ASSERT(s->close(srv) == FF_SUCCESS);
}

static void test_loopback_errors(void)
{
    FF_NetStack_t *s = &FF_NetStackLoopback;
    FF_NetSock_t sd;
    uint8_t addr[1] = {9u};
    uint16_t sent = 0u;
    uint16_t got = 99u;
    uint8_t buf[8];

    s->init();
    sd = s->open(0, 0, 0);
    FF_TEST_ASSERT(sd >= 0);
    FF_TEST_ASSERT(s->send(sd, buf, 1u, &sent) == FF_ERROR); /* unconnected */
    FF_TEST_ASSERT(s->connect(sd, addr, 1u) == FF_ERROR);    /* nothing listening */
    FF_TEST_ASSERT(s->recv(sd, buf, sizeof(buf), &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 0u);
    FF_TEST_ASSERT(s->close(sd) == FF_SUCCESS);
    FF_TEST_ASSERT(s->close(sd) == FF_INVALID_ARGUMENT);
    /* out-of-range handles must not alias a valid descriptor */
    FF_TEST_ASSERT(s->close((FF_NetSock_t)65536) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->recv((FF_NetSock_t)65536, buf, sizeof(buf), &got, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->send((FF_NetSock_t)65536, buf, 1u, &sent) == FF_INVALID_ARGUMENT);
}

static void test_loopback_boundaries(void)
{
    FF_NetStack_t *s = &FF_NetStackLoopback;
    FF_NetSock_t a;
    FF_NetSock_t b;
    FF_NetSock_t srv;
    FF_NetSock_t cli;
    FF_NetSock_t acc;
    uint8_t addr7[1] = {7u};
    uint8_t addr8[1] = {8u};
    uint16_t n = 0u;
    uint16_t got = 0u;
    uint8_t buf[300];
    uint8_t peer[1];
    uint16_t peer_len = 1u;
    int i;

    s->init();

    /* argument and binding boundaries */
    a = s->open(0, 0, 0);
    FF_TEST_ASSERT(a >= 0);
    FF_TEST_ASSERT(s->bind(a, NULL, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->bind(a, addr7, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->bind(a, addr7, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->bind(a, addr7, 1u) == FF_ALREADY_EXISTS); /* same socket */
    b = s->open(0, 0, 0);
    FF_TEST_ASSERT(b >= 0);
    FF_TEST_ASSERT(s->bind(b, addr7, 1u) == FF_ALREADY_EXISTS); /* port in use by a */
    FF_TEST_ASSERT(s->bind(b, addr8, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->listen(a, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->accept(a, peer, &peer_len) == -1); /* empty accept queue */
    FF_TEST_ASSERT(s->send(a, buf, 1u, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->recv(a, NULL, 1u, &got, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->recv(a, buf, 1u, NULL, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->close(a) == FF_SUCCESS);
    FF_TEST_ASSERT(s->close(b) == FF_SUCCESS);

    /* listen without bind is rejected */
    a = s->open(0, 0, 0);
    FF_TEST_ASSERT(a >= 0);
    FF_TEST_ASSERT(s->listen(a, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(s->close(a) == FF_SUCCESS);

    /* descriptor exhaustion returns -1 (bounded 4-slot table), and init resets */
    s->init();
    for (i = 0; i < 4; i++) {
        FF_TEST_ASSERT(s->open(0, 0, 0) >= 0);
    }
    FF_TEST_ASSERT(s->open(0, 0, 0) < 0);

    /* partial send when the peer ring (256 bytes) is full; then a wrap-around
     * send/receive still delivers intact */
    s->init();
    srv = s->open(0, 0, 0);
    cli = s->open(0, 0, 0);
    FF_TEST_ASSERT(srv >= 0);
    FF_TEST_ASSERT(cli >= 0);
    FF_TEST_ASSERT(s->bind(srv, addr7, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->listen(srv, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->connect(cli, addr7, 1u) == FF_SUCCESS);
    acc = s->accept(srv, peer, &peer_len);
    FF_TEST_ASSERT(acc >= 0);
    /* advance the ring head by 10 bytes, then fill the full 256-byte ring so the
     * write crosses the 256-byte index boundary (255 -> 0); read it back intact */
    for (i = 0; i < 10; i++) {
        buf[i] = (uint8_t)(0x10u + (unsigned)i);
    }
    FF_TEST_ASSERT(s->send(cli, buf, 10u, &n) == FF_SUCCESS);
    FF_TEST_ASSERT(n == 10u);
    FF_TEST_ASSERT(s->recv(acc, buf, sizeof(buf), &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 10u);
    for (i = 0; i < 256; i++) {
        buf[i] = (uint8_t)i;
    }
    FF_TEST_ASSERT(s->send(cli, buf, 256u, &n) == FF_SUCCESS);
    FF_TEST_ASSERT(n == 256u); /* full ring, wrapped across the boundary */
    memset(buf, 0, sizeof(buf));
    FF_TEST_ASSERT(s->recv(acc, buf, 256u, &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 256u);
    for (i = 0; i < 256; i++) {
        FF_TEST_ASSERT(buf[i] == (uint8_t)i);
    }
    /* refill the full ring, then a further (300-byte) send on it transfers 0 */
    FF_TEST_ASSERT(s->send(cli, buf, 256u, &n) == FF_SUCCESS);
    FF_TEST_ASSERT(n == 256u);
    FF_TEST_ASSERT(s->send(cli, buf, 300u, &n) == FF_SUCCESS);
    FF_TEST_ASSERT(n == 0u); /* ring already full */
    FF_TEST_ASSERT(s->recv(acc, buf, sizeof(buf), &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == 256u);
    (void)s->close(acc);
    (void)s->close(cli);
    (void)s->close(srv);
}

FF_TEST_MAIN(test_loopback_stream, test_loopback_errors, test_loopback_boundaries)
