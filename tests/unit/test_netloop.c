/*
 * frick-fsw - Reference loopback network stack unit tests (milestone M4)
 *
 * Exercises the pluggable network-stack contract (SRS-REQ-039).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
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

FF_TEST_MAIN(test_loopback_stream, test_loopback_errors)
