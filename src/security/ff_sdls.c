/*
 * frick-fsw - CCSDS SDLS (Space Data Link Security) implementation
 *
 * Security Association database, key lifecycle/rotation and SDLS processing.
 * The cryptographic primitive is pluggable (FF_CryptoBackend_t); this module
 * holds no dynamic memory and never implements a cipher.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "security/ff_sdls.h"
#include "fsw_config.h"

static FF_Sa_t g_sa[FF_SDLS_MAX_SA];
static uint16_t g_sa_count;
static FF_CryptoBackend_t *g_crypto;

/* Wipe sensitive memory in a way the compiler must not elide. */
static void ff_sdls_wipe(void *p, uint16_t n)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (n > 0u) {
        *v++ = 0u;
        n--;
    }
}

static FF_Sa_t *ff_sdls_find(uint16_t spi)
{
    uint16_t i;
    for (i = 0u; i < g_sa_count; i++) {
        if (g_sa[i].spi == spi) {
            return &g_sa[i];
        }
    }
    return NULL;
}

/* Build the per-frame IV: SA base with the counter written big-endian in the
 * last four bytes (uniqueness within a session key). */
static void ff_sdls_build_iv(const FF_Sa_t *sa, uint8_t *iv)
{
    uint16_t i;
    for (i = 0u; i < sa->iv_len; i++) {
        iv[i] = sa->iv[i];
    }
    if (sa->iv_len >= 4u) {
        iv[sa->iv_len - 4u] = (uint8_t)(sa->iv_counter >> 24);
        iv[sa->iv_len - 3u] = (uint8_t)(sa->iv_counter >> 16);
        iv[sa->iv_len - 2u] = (uint8_t)(sa->iv_counter >> 8);
        iv[sa->iv_len - 1u] = (uint8_t)(sa->iv_counter);
    }
}

static FF_Status_t ff_sdls_derive(FF_Sa_t *sa)
{
    uint8_t info[6];
    info[0] = (uint8_t)(sa->spi >> 8);
    info[1] = (uint8_t)(sa->spi);
    info[2] = (uint8_t)(sa->epoch >> 24);
    info[3] = (uint8_t)(sa->epoch >> 16);
    info[4] = (uint8_t)(sa->epoch >> 8);
    info[5] = (uint8_t)(sa->epoch);
    if (g_crypto->key_derive(sa->master_key, sa->master_key_len, info, sizeof(info),
                             sa->session_key) != FF_SUCCESS) {
        return FF_ERROR;
    }
    sa->session_key_len = g_crypto->key_len;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsInit(void)
{
    ff_sdls_wipe(g_sa, (uint16_t)sizeof(g_sa));
    g_sa_count = 0u;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsSetBackend(FF_CryptoBackend_t *backend)
{
    if ((backend == NULL) || (backend->key_len == 0u) || (backend->key_len > FF_SDLS_KEY_MAX) ||
        (backend->mac_len == 0u) || (backend->mac_len > FF_SDLS_MAC_MAX) ||
        (backend->iv_len < 4u) || (backend->iv_len > FF_SDLS_IV_MAX) || (backend->init == NULL) ||
        (backend->encrypt == NULL) || (backend->decrypt == NULL) || (backend->mac == NULL) ||
        (backend->key_derive == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (backend->init() != FF_SUCCESS) {
        return FF_ERROR;
    }
    g_crypto = backend;
    return FF_SUCCESS;
}

FF_CryptoBackend_t *FF_SdlsGetBackend(void)
{
    return g_crypto;
}

FF_Status_t FF_SdlsSaAdd(const FF_Sa_t *sa)
{
    FF_Sa_t *slot;

    if (sa == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if ((sa->service != FF_SDLS_PLAIN) && (g_crypto == NULL)) {
        return FF_NOT_READY;
    }
    if ((sa->iv_len > FF_SDLS_IV_MAX) || (sa->master_key_len > FF_SDLS_KEY_MAX)) {
        return FF_INVALID_ARGUMENT;
    }
    if (sa->service != FF_SDLS_PLAIN) {
        if ((sa->master_key_len != g_crypto->key_len) || (sa->iv_len != g_crypto->iv_len)) {
            return FF_INVALID_ARGUMENT;
        }
    }
    if (ff_sdls_find(sa->spi) != NULL) {
        return FF_ALREADY_EXISTS;
    }
    if (g_sa_count >= FF_SDLS_MAX_SA) {
        return FF_ERROR;
    }
    slot = &g_sa[g_sa_count];
    *slot = *sa;
    slot->active = 0u;
    slot->epoch = 0u;
    slot->tx_count = 0u;
    slot->iv_counter = 0u;
    slot->activation_ns = 0u;
    slot->session_key_len = 0u;
    ff_sdls_wipe(slot->session_key, FF_SDLS_KEY_MAX);
    if (sa->service != FF_SDLS_PLAIN) {
        if (ff_sdls_derive(slot) != FF_SUCCESS) {
            *slot = *sa;
            ff_sdls_wipe(slot->session_key, FF_SDLS_KEY_MAX);
            return FF_ERROR;
        }
    }
    g_sa_count++;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsSaGet(uint16_t spi, FF_Sa_t *out)
{
    FF_Sa_t *sa;
    if (out == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    sa = ff_sdls_find(spi);
    if (sa == NULL) {
        return FF_NOT_FOUND;
    }
    *out = *sa;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsSaActivate(uint16_t spi, uint8_t active)
{
    FF_Sa_t *sa = ff_sdls_find(spi);
    if (sa == NULL) {
        return FF_NOT_FOUND;
    }
    sa->active = (active != 0u) ? 1u : 0u;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsSaRemove(uint16_t spi)
{
    uint16_t i;
    for (i = 0u; i < g_sa_count; i++) {
        if (g_sa[i].spi == spi) {
            ff_sdls_wipe(&g_sa[i], (uint16_t)sizeof(FF_Sa_t));
            if (i != (g_sa_count - 1u)) {
                g_sa[i] = g_sa[g_sa_count - 1u];
            }
            /* wipe the vacated tail slot (may hold a duplicate after the swap) */
            ff_sdls_wipe(&g_sa[g_sa_count - 1u], (uint16_t)sizeof(FF_Sa_t));
            g_sa_count--;
            return FF_SUCCESS;
        }
    }
    return FF_NOT_FOUND;
}

uint16_t FF_SdlsSaCount(void)
{
    return g_sa_count;
}

FF_Status_t FF_SdlsRotateKey(uint16_t spi, uint64_t now_ns)
{
    FF_Sa_t *sa = ff_sdls_find(spi);
    uint8_t info[6];
    uint8_t newkey[FF_SDLS_KEY_MAX];
    uint32_t new_epoch;
    uint16_t i;

    if (sa == NULL) {
        return FF_NOT_FOUND;
    }
    if (sa->service == FF_SDLS_PLAIN) {
        return FF_INVALID_ARGUMENT;
    }
    new_epoch = sa->epoch + 1u;
    info[0] = (uint8_t)(sa->spi >> 8);
    info[1] = (uint8_t)(sa->spi);
    info[2] = (uint8_t)(new_epoch >> 24);
    info[3] = (uint8_t)(new_epoch >> 16);
    info[4] = (uint8_t)(new_epoch >> 8);
    info[5] = (uint8_t)(new_epoch);
    /* Derive into a temporary first: a derivation failure must leave the
     * current (still active) session key intact (no all-zero key). */
    if (g_crypto->key_derive(sa->master_key, sa->master_key_len, info, sizeof(info), newkey) !=
        FF_SUCCESS) {
        ff_sdls_wipe(newkey, FF_SDLS_KEY_MAX);
        return FF_ERROR;
    }
    ff_sdls_wipe(sa->session_key, FF_SDLS_KEY_MAX);
    for (i = 0u; i < g_crypto->key_len; i++) {
        sa->session_key[i] = newkey[i];
    }
    ff_sdls_wipe(newkey, FF_SDLS_KEY_MAX);
    sa->session_key_len = g_crypto->key_len;
    sa->epoch = new_epoch;
    sa->tx_count = 0u;
    sa->iv_counter = 0u;
    sa->activation_ns = now_ns;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsTick(uint64_t now_ns)
{
    uint16_t i;
    FF_Status_t st = FF_SUCCESS;
    for (i = 0u; i < g_sa_count; i++) {
        FF_Sa_t *sa = &g_sa[i];
        if ((sa->active == 0u) || (sa->service == FF_SDLS_PLAIN)) {
            continue;
        }
        if (((sa->max_tx != 0u) && (sa->tx_count >= sa->max_tx)) ||
            ((sa->lifetime_ns != 0u) && ((now_ns - sa->activation_ns) >= sa->lifetime_ns))) {
            if (FF_SdlsRotateKey(sa->spi, now_ns) != FF_SUCCESS) {
                st = FF_ERROR; /* rotation failure must not be silent */
            }
        }
    }
    return st;
}

FF_Status_t FF_SdlsZeroize(void)
{
    ff_sdls_wipe(g_sa, (uint16_t)sizeof(g_sa));
    g_sa_count = 0u;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsProtect(uint16_t spi, const uint8_t *in, uint16_t in_len, uint8_t *out,
                           uint16_t out_cap, uint16_t *out_len, uint64_t now_ns)
{
    FF_Sa_t *sa = ff_sdls_find(spi);
    uint8_t iv[FF_SDLS_IV_MAX];
    uint16_t pos;
    uint16_t mac_len = 0u;
    uint16_t total;

    if ((in == NULL) || (out == NULL) || (out_len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (sa == NULL) {
        return FF_NOT_FOUND;
    }
    if (sa->active == 0u) {
        return FF_NOT_READY;
    }
    (void)now_ns;

    if (sa->service == FF_SDLS_PLAIN) {
        uint16_t k;
        if (((uint32_t)in_len + 2u) > (uint32_t)out_cap) {
            return FF_BAD_SIZE;
        }
        out[0] = (uint8_t)(spi >> 8);
        out[1] = (uint8_t)(spi);
        for (k = 0u; k < in_len; k++) {
            out[2u + k] = in[k];
        }
        *out_len = (uint16_t)(in_len + 2u);
        sa->tx_count++;
        return FF_SUCCESS;
    }
    if (g_crypto == NULL) {
        return FF_NOT_READY;
    }
    if (sa->service != FF_SDLS_ENC) {
        mac_len = g_crypto->mac_len;
    }
    {
        uint32_t total32 = 2u + (uint32_t)sa->iv_len + (uint32_t)in_len + (uint32_t)mac_len;
        if ((total32 > (uint32_t)out_cap) || (total32 > 0xFFFFu)) {
            return FF_BAD_SIZE;
        }
        total = (uint16_t)total32;
    }

    ff_sdls_build_iv(sa, iv);
    out[0] = (uint8_t)(spi >> 8);
    out[1] = (uint8_t)(spi);
    for (pos = 0u; pos < sa->iv_len; pos++) {
        out[2u + pos] = iv[pos];
    }
    pos = (uint16_t)(2u + sa->iv_len);
    if ((sa->service == FF_SDLS_ENC) || (sa->service == FF_SDLS_AUTH_ENC)) {
        if (g_crypto->encrypt(sa->session_key, iv, in, in_len, &out[pos]) != FF_SUCCESS) {
            return FF_ERROR;
        }
    }
    else {
        uint16_t k;
        for (k = 0u; k < in_len; k++) {
            out[pos + k] = in[k];
        }
    }
    if (mac_len != 0u) {
        if (g_crypto->mac(sa->session_key, iv, out, (uint16_t)(2u + sa->iv_len + in_len),
                          &out[2u + sa->iv_len + in_len]) != FF_SUCCESS) {
            return FF_ERROR;
        }
    }
    sa->iv_counter++;
    sa->tx_count++;
    *out_len = total;
    return FF_SUCCESS;
}

FF_Status_t FF_SdlsUnprotect(const uint8_t *in, uint16_t in_len, uint8_t *out, uint16_t out_cap,
                             uint16_t *out_len)
{
    uint16_t spi;
    FF_Sa_t *sa;
    uint8_t iv[FF_SDLS_IV_MAX];
    uint8_t mac[FF_SDLS_MAC_MAX];
    uint16_t mac_len = 0u;
    uint16_t payload_len;
    uint16_t pos;

    if ((in == NULL) || (out == NULL) || (out_len == NULL) || (in_len < 2u)) {
        return FF_INVALID_ARGUMENT;
    }
    spi = (uint16_t)(((uint16_t)in[0] << 8) | (uint16_t)in[1]);
    sa = ff_sdls_find(spi);
    if (sa == NULL) {
        return FF_NOT_FOUND;
    }
    if (sa->service == FF_SDLS_PLAIN) {
        uint16_t k;
        if (in_len < 2u) {
            return FF_BAD_SIZE;
        }
        payload_len = (uint16_t)(in_len - 2u);
        if (payload_len > out_cap) {
            return FF_BAD_SIZE;
        }
        for (k = 0u; k < payload_len; k++) {
            out[k] = in[2u + k];
        }
        *out_len = payload_len;
        return FF_SUCCESS;
    }
    if (g_crypto == NULL) {
        return FF_NOT_READY;
    }
    if (sa->service != FF_SDLS_ENC) {
        mac_len = g_crypto->mac_len;
    }
    if (in_len < (uint16_t)(2u + sa->iv_len + mac_len)) {
        return FF_BAD_SIZE;
    }
    payload_len = (uint16_t)(in_len - 2u - sa->iv_len - mac_len);
    if (payload_len > out_cap) {
        return FF_BAD_SIZE;
    }
    for (pos = 0u; pos < sa->iv_len; pos++) {
        iv[pos] = in[2u + pos];
    }
    if (mac_len != 0u) {
        if (g_crypto->mac(sa->session_key, iv, in, (uint16_t)(2u + sa->iv_len + payload_len),
                          mac) != FF_SUCCESS) {
            return FF_ERROR;
        }
        /* constant-time compare */
        {
            uint16_t diff = 0u;
            uint16_t k;
            for (k = 0u; k < mac_len; k++) {
                diff |= (uint16_t)(mac[k] ^ in[2u + sa->iv_len + payload_len + k]);
            }
            if (diff != 0u) {
                return FF_PARSE_ERROR; /* authentication failure */
            }
        }
    }
    pos = (uint16_t)(2u + sa->iv_len);
    if ((sa->service == FF_SDLS_ENC) || (sa->service == FF_SDLS_AUTH_ENC)) {
        if (g_crypto->decrypt(sa->session_key, iv, &in[pos], payload_len, out) != FF_SUCCESS) {
            return FF_ERROR;
        }
    }
    else {
        uint16_t k;
        for (k = 0u; k < payload_len; k++) {
            out[k] = in[pos + k];
        }
    }
    *out_len = payload_len;
    return FF_SUCCESS;
}
