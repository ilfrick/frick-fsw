/*
 * frick-fsw - Reference (NON-SECURE) cryptographic backend for SDLS V&V
 *
 * Deterministic, reversible, allocation-free; NOT cryptographic. See the
 * header warning. Copyright (C) 2026 Ilfrick.
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "security/ff_crypto_test.h"

#define FF_TEST_KEY_LEN 16u
#define FF_TEST_MAC_LEN 16u
#define FF_TEST_IV_LEN 12u

/* 32-bit FNV-1a mix over a byte buffer (non-cryptographic). */
static uint32_t ff_test_hash(const uint8_t *a, uint16_t alen, const uint8_t *b, uint16_t blen,
                             uint32_t salt)
{
    uint32_t h = 2166136261u ^ salt;
    uint16_t i;
    for (i = 0u; i < alen; i++) {
        h = (h ^ a[i]) * 16777619u;
    }
    for (i = 0u; i < blen; i++) {
        h = (h ^ b[i]) * 16777619u;
    }
    return h;
}

static uint8_t ff_test_keystream(const uint8_t *key, const uint8_t *iv, uint16_t pos)
{
    uint32_t h = ff_test_hash(key, FF_TEST_KEY_LEN, iv, FF_TEST_IV_LEN, (uint32_t)pos);
    return (uint8_t)(h ^ (h >> 8) ^ (h >> 16) ^ (h >> 24));
}

static FF_Status_t ff_test_init(void)
{
    return FF_SUCCESS;
}

static FF_Status_t ff_test_crypt(const uint8_t *key, const uint8_t *iv, const uint8_t *in,
                                 uint16_t len, uint8_t *out)
{
    uint16_t i;
    for (i = 0u; i < len; i++) {
        out[i] = (uint8_t)(in[i] ^ ff_test_keystream(key, iv, i));
    }
    return FF_SUCCESS;
}

static FF_Status_t ff_test_mac(const uint8_t *key, const uint8_t *iv, const uint8_t *data,
                               uint16_t len, uint8_t *mac_out)
{
    uint32_t h = ff_test_hash(key, FF_TEST_KEY_LEN, iv, FF_TEST_IV_LEN, 0xC0FFEEu);
    h = ff_test_hash((const uint8_t *)&h, 4u, data, len, 0x1234u);
    for (uint16_t i = 0u; i < FF_TEST_MAC_LEN; i++) {
        mac_out[i] = (uint8_t)(h >> ((i % 4u) * 8u));
        h = h * 16777619u;
    }
    return FF_SUCCESS;
}

static FF_Status_t ff_test_key_derive(const uint8_t *master, uint16_t master_len,
                                      const uint8_t *info, uint16_t info_len, uint8_t *out_key)
{
    uint32_t h = ff_test_hash(master, master_len, info, info_len, 0xA5A5u);
    for (uint16_t i = 0u; i < FF_TEST_KEY_LEN; i++) {
        out_key[i] = (uint8_t)(h >> ((i % 4u) * 8u));
        h = h * 16777619u;
    }
    return FF_SUCCESS;
}

FF_CryptoBackend_t FF_CryptoTestBackend = {"test-nonsecure", FF_TEST_KEY_LEN, FF_TEST_MAC_LEN,
                                           FF_TEST_IV_LEN,   ff_test_init,    ff_test_crypt,
                                           ff_test_crypt,    ff_test_mac,     ff_test_key_derive};
