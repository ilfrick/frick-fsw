/*
 * frick-fsw - CCSDS SDLS unit tests (milestone M7)
 *
 * Verifies SDLS processing, SA/key lifecycle and rotation (SRS-REQ-034/035)
 * using the non-secure test backend (V&V only).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "security/ff_sdls.h"
#include "security/ff_crypto_test.h"
#include <string.h>

static FF_Sa_t make_sa(uint16_t spi, FF_SdlsService_t svc)
{
    FF_Sa_t sa;
    uint16_t i;
    memset(&sa, 0, sizeof(sa));
    sa.spi = spi;
    sa.service = svc;
    sa.master_key_len = 16u;
    for (i = 0u; i < 16u; i++) {
        sa.master_key[i] = (uint8_t)(0x10u + i + (uint8_t)spi);
    }
    sa.iv_len = 12u;
    for (i = 0u; i < 12u; i++) {
        sa.iv[i] = (uint8_t)(i);
    }
    return sa;
}

static void test_backend_and_registry(void)
{
    FF_Sa_t sa, got;

    FF_TEST_ASSERT(FF_SdlsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSetBackend(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_SdlsSetBackend(&FF_CryptoTestBackend) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsGetBackend() == &FF_CryptoTestBackend);
    FF_TEST_ASSERT(FF_SdlsSaCount() == 0u);

    sa = make_sa(0x0001u, FF_SDLS_AUTH_ENC);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(FF_SdlsSaCount() == 1u);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x0001u, &got) == FF_SUCCESS);
    FF_TEST_ASSERT(got.spi == 0x0001u && got.active == 0u);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x9999u, &got) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_SdlsSaActivate(0x0001u, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x0001u, &got) == FF_SUCCESS);
    FF_TEST_ASSERT(got.active == 1u);
    FF_TEST_ASSERT(FF_SdlsSaRemove(0x0001u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaCount() == 0u);
}

static void test_protect_roundtrip(void)
{
    FF_Sa_t sa = make_sa(0x0010u, FF_SDLS_AUTH_ENC);
    const uint8_t plain[20] = "ping-through-sdls!!";
    uint8_t wire[128];
    uint8_t out[64];
    uint16_t wlen = 0u, olen = 0u;

    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsProtect(0x0010u, plain, sizeof(plain), wire, sizeof(wire), &wlen, 0u) ==
                   FF_NOT_READY); /* inactive */
    FF_SdlsSaActivate(0x0010u, 1u);
    FF_TEST_ASSERT(FF_SdlsProtect(0x0010u, plain, sizeof(plain), wire, sizeof(wire), &wlen, 0u) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(wlen == (uint16_t)(2u + 12u + sizeof(plain) + 16u));
    FF_TEST_ASSERT(FF_SdlsUnprotect(wire, wlen, out, sizeof(out), &olen) == FF_SUCCESS);
    FF_TEST_ASSERT(olen == sizeof(plain));
    FF_TEST_ASSERT(memcmp(out, plain, sizeof(plain)) == 0);

    /* tamper detection (authentication failure) */
    wire[6] ^= 0xFFu;
    FF_TEST_ASSERT(FF_SdlsUnprotect(wire, wlen, out, sizeof(out), &olen) == FF_PARSE_ERROR);
    wire[6] ^= 0xFFu;
    FF_TEST_ASSERT(FF_SdlsUnprotect(wire, wlen, out, sizeof(out), &olen) == FF_SUCCESS);
}

static void test_key_rotation(void)
{
    FF_Sa_t sa = make_sa(0x0020u, FF_SDLS_AUTH_ENC);
    FF_Sa_t got;
    uint8_t k0[FF_SDLS_KEY_MAX];

    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    sa.max_tx = 2u; /* rotate after 2 frames */
    FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_SUCCESS);
    FF_SdlsSaActivate(0x0020u, 1u);
    FF_SdlsSaGet(0x0020u, &got);
    FF_TEST_ASSERT(got.epoch == 0u);
    memcpy(k0, got.session_key, FF_SDLS_KEY_MAX);

    FF_TEST_ASSERT(FF_SdlsRotateKey(0x0020u, 1000u) == FF_SUCCESS);
    FF_SdlsSaGet(0x0020u, &got);
    FF_TEST_ASSERT(got.epoch == 1u);    /* epoch strictly increases */
    FF_TEST_ASSERT(got.tx_count == 0u); /* usage reset              */
    FF_TEST_ASSERT(got.activation_ns == 1000u);
    FF_TEST_ASSERT(memcmp(k0, got.session_key, FF_SDLS_KEY_MAX) != 0); /* new key */

    /* policy tick rotates when max_tx is reached */
    {
        uint8_t wire[64];
        uint16_t wlen;
        got.max_tx = 1u;
        FF_SdlsSaRemove(0x0020u);
        FF_SdlsSaAdd(&got);
        FF_SdlsSaActivate(0x0020u, 1u);
        FF_SdlsProtect(0x0020u, (const uint8_t *)"x", 1u, wire, sizeof(wire), &wlen, 0u);
        FF_TEST_ASSERT(FF_SdlsTick(2000u) == FF_SUCCESS);
        FF_SdlsSaGet(0x0020u, &got);
        FF_TEST_ASSERT(got.epoch == 1u);
        FF_TEST_ASSERT(got.tx_count == 0u);
    }
}

static void test_capacity_and_plain(void)
{
    FF_Sa_t sa;
    uint16_t i;
    uint8_t out[8];
    uint16_t olen;
    const uint8_t plain[3] = {1u, 2u, 3u};

    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    for (i = 0u; i < (uint16_t)FF_SDLS_MAX_SA; i++) {
        sa = make_sa((uint16_t)(0x0100u + i), FF_SDLS_AUTH);
        FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_SUCCESS);
    }
    sa = make_sa(0x0200u, FF_SDLS_AUTH);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_ERROR); /* full */
    FF_TEST_ASSERT(FF_SdlsSaCount() == (uint16_t)FF_SDLS_MAX_SA);

    /* PLAIN bypass */
    FF_SdlsInit();
    sa = make_sa(0x00AAu, FF_SDLS_PLAIN);
    sa.master_key_len = 0u;
    sa.iv_len = 0u;
    FF_TEST_ASSERT(FF_SdlsSaAdd(&sa) == FF_SUCCESS);
    FF_SdlsSaActivate(0x00AAu, 1u);
    FF_TEST_ASSERT(FF_SdlsProtect(0x00AAu, plain, sizeof(plain), out, sizeof(out), &olen, 0u) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(olen == (uint16_t)(sizeof(plain) + 2u));
    FF_TEST_ASSERT(out[0] == 0x00u && out[1] == 0xAAu);
    FF_TEST_ASSERT(memcmp(&out[2], plain, sizeof(plain)) == 0);
    {
        uint8_t rt[8];
        uint16_t rlen = 0u;
        FF_TEST_ASSERT(FF_SdlsUnprotect(out, olen, rt, sizeof(rt), &rlen) == FF_SUCCESS);
        FF_TEST_ASSERT(rlen == sizeof(plain));
        FF_TEST_ASSERT(memcmp(rt, plain, sizeof(plain)) == 0);
    }
}

static void test_bounds(void)
{
    FF_Sa_t sa = make_sa(0x0040u, FF_SDLS_AUTH_ENC);
    const uint8_t small[4] = {0u, 0u, 0u, 0u};
    uint8_t out[64];
    uint16_t olen = 0u;

    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    FF_SdlsSaAdd(&sa);
    FF_SdlsSaActivate(0x0040u, 1u);
    /* a length that would overflow 16-bit arithmetic must be rejected, not written */
    FF_TEST_ASSERT(FF_SdlsProtect(0x0040u, small, 0xFFF0u, out, sizeof(out), &olen, 0u) ==
                   FF_BAD_SIZE);
    /* too-small output buffer */
    FF_TEST_ASSERT(FF_SdlsProtect(0x0040u, small, 8u, out, 4u, &olen, 0u) == FF_BAD_SIZE);
}

static void test_remove_reuse(void)
{
    FF_Sa_t a = make_sa(0x0050u, FF_SDLS_AUTH);
    FF_Sa_t b = make_sa(0x0051u, FF_SDLS_AUTH);
    FF_Sa_t c = make_sa(0x0052u, FF_SDLS_AUTH);
    FF_Sa_t got;

    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&a) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&b) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&c) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaRemove(0x0051u) == FF_SUCCESS); /* middle of the table */
    FF_TEST_ASSERT(FF_SdlsSaCount() == 2u);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x0051u, &got) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x0050u, &got) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x0052u, &got) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&b) == FF_SUCCESS); /* slot reused */
    FF_TEST_ASSERT(FF_SdlsSaCount() == 3u);
}

static void test_zeroize(void)
{
    FF_Sa_t sa = make_sa(0x0030u, FF_SDLS_AUTH_ENC);
    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    FF_SdlsSaAdd(&sa);
    FF_TEST_ASSERT(FF_SdlsSaCount() == 1u);
    FF_TEST_ASSERT(FF_SdlsZeroize() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaCount() == 0u);
    FF_TEST_ASSERT(FF_SdlsSaGet(0x0030u, &sa) == FF_NOT_FOUND);
}

static void test_services(void)
{
    uint8_t wire[128];
    uint8_t out[64];
    uint16_t wlen = 0u;
    uint16_t olen = 0u;
    const uint8_t plain[8] = {9u, 8u, 7u, 6u, 5u, 4u, 3u, 2u};
    FF_Sa_t enc = make_sa(0x0060u, FF_SDLS_ENC);
    FF_Sa_t auth = make_sa(0x0061u, FF_SDLS_AUTH);

    FF_SdlsInit();
    FF_SdlsSetBackend(&FF_CryptoTestBackend);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&enc) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SdlsSaAdd(&auth) == FF_SUCCESS);
    FF_SdlsSaActivate(0x0060u, 1u);
    FF_SdlsSaActivate(0x0061u, 1u);

    /* ENC-only: no MAC on the wire, payload encrypted */
    FF_TEST_ASSERT(FF_SdlsProtect(0x0060u, plain, sizeof(plain), wire, sizeof(wire), &wlen, 0u) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(wlen == (uint16_t)(2u + 12u + sizeof(plain)));
    FF_TEST_ASSERT(memcmp(&wire[2u + 12u], plain, sizeof(plain)) != 0);
    FF_TEST_ASSERT(FF_SdlsUnprotect(wire, wlen, out, sizeof(out), &olen) == FF_SUCCESS);
    FF_TEST_ASSERT(olen == sizeof(plain) && memcmp(out, plain, sizeof(plain)) == 0);

    /* AUTH-only: MAC present, payload in clear */
    FF_TEST_ASSERT(FF_SdlsProtect(0x0061u, plain, sizeof(plain), wire, sizeof(wire), &wlen, 0u) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(wlen == (uint16_t)(2u + 12u + sizeof(plain) + 16u));
    FF_TEST_ASSERT(memcmp(&wire[2u + 12u], plain, sizeof(plain)) == 0);
    FF_TEST_ASSERT(FF_SdlsUnprotect(wire, wlen, out, sizeof(out), &olen) == FF_SUCCESS);
    FF_TEST_ASSERT(olen == sizeof(plain) && memcmp(out, plain, sizeof(plain)) == 0);
}

FF_TEST_MAIN(test_backend_and_registry, test_protect_roundtrip, test_services, test_key_rotation,
             test_capacity_and_plain, test_bounds, test_remove_reuse, test_zeroize)
