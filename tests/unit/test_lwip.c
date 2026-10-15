/*
 * frick-fsw - lwIP network-stack adapter unit tests (milestone M4)
 *
 * Exercises the adapter over the lwIP socket API using the built-in loopback
 * interface (127.0.0.1): bind/listen/connect/accept and a TCP send/recv round
 * trip (SRS-REQ-040). Built only with FF_USE_LWIP=ON.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "hal/hal_net.h"
#include "hal/net/lwip/ff_net_lwip.h"
#include <string.h>

/* numeric constants to avoid pulling lwIP headers into the test */
#define T_AF_INET 2
#define T_SOCK_STREAM 1

static void test_lwip_loopback(void)
{
    FF_NetStack_t *s = &FF_NetStackLwip;
    FF_NetSock_t srv;
    FF_NetSock_t cli;
    FF_NetSock_t acc;
    uint8_t srv_addr[6] = {127u, 0u, 0u, 1u, 0x30u, 0x39u}; /* 127.0.0.1:12345 */
    uint8_t peer[6];
    uint16_t peer_len = 6u;
    uint16_t sent = 0u;
    uint16_t got = 0u;
    uint8_t buf[16];
    static const uint8_t msg[4] = {0xDEu, 0xADu, 0xBEu, 0xEFu};

    FF_TEST_ASSERT(s->init() == FF_SUCCESS);
    srv = s->open(T_AF_INET, T_SOCK_STREAM, 0);
    FF_TEST_ASSERT(srv >= 0);
    FF_TEST_ASSERT(s->bind(srv, srv_addr, 6u) == FF_SUCCESS);
    FF_TEST_ASSERT(s->listen(srv, 1u) == FF_SUCCESS);
    cli = s->open(T_AF_INET, T_SOCK_STREAM, 0);
    FF_TEST_ASSERT(cli >= 0);
    FF_TEST_ASSERT(s->connect(cli, srv_addr, 6u) == FF_SUCCESS);
    acc = s->accept(srv, peer, &peer_len);
    FF_TEST_ASSERT(acc >= 0);

    FF_TEST_ASSERT(s->send(cli, msg, sizeof(msg), &sent) == FF_SUCCESS);
    FF_TEST_ASSERT(sent == sizeof(msg));
    FF_TEST_ASSERT(s->recv(acc, buf, sizeof(buf), &got, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(got == sizeof(msg));
    FF_TEST_ASSERT(memcmp(buf, msg, sizeof(msg)) == 0);

    FF_TEST_ASSERT(s->close(acc) == FF_SUCCESS);
    FF_TEST_ASSERT(s->close(cli) == FF_SUCCESS);
    FF_TEST_ASSERT(s->close(srv) == FF_SUCCESS);

    /* invalid address arguments are rejected */
    {
        FF_NetSock_t sd = s->open(T_AF_INET, T_SOCK_STREAM, 0);
        FF_TEST_ASSERT(sd >= 0);
        FF_TEST_ASSERT(s->connect(sd, srv_addr, 3u) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(s->bind(sd, srv_addr, 0u) == FF_INVALID_ARGUMENT);
        FF_TEST_ASSERT(s->close(sd) == FF_SUCCESS);
    }
}

FF_TEST_MAIN(test_lwip_loopback)
