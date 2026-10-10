/*
 * frick-fsw - CCSDS SDLS (Space Data Link Security) sub-layer
 *
 * Milestone M7. SDLS (CCSDS 355.0-B) secures TM/TC transfer frames with
 * authentication and/or encryption and provides Security Association (SA)
 * management and key management (key derivation, rotation and zeroisation).
 * Algorithms are mission/HW specific (CCSDS 352.0-B: e.g. AES-GCM/CMAC), so the
 * cryptographic primitive is a **pluggable backend** (FF_CryptoBackend_t): the
 * middleware implements the SA/key lifecycle and the SDLS processing, never a
 * specific cipher. See docs/architecture/security-sdls.md and ADR-0013.
 *
 * This module holds no dynamic memory; all state is bounded (FF_SDLS_MAX_SA).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_SDLS_H
#define FF_SDLS_H

#include "common/ff_types.h"
#include "fsw_config.h"

/* SDLS security services (CCSDS 355.0-B). */
typedef enum {
    FF_SDLS_PLAIN = 0, /* no security (bypass, for testing/plain TM)     */
    FF_SDLS_AUTH,      /* authentication only (MAC)                      */
    FF_SDLS_ENC,       /* encryption only                                */
    FF_SDLS_AUTH_ENC   /* authenticated encryption                       */
} FF_SdlsService_t;

/* Pluggable cryptographic backend. A conforming backend provides symmetric
 * encryption/decryption, a keyed MAC and key derivation. The middleware never
 * implements the cipher (norm: pluggable philosophy; SRS-REQ-034). */
typedef struct FF_CryptoBackend FF_CryptoBackend_t;
struct FF_CryptoBackend
{
    const char *name;
    uint16_t key_len; /* bytes (e.g. 16/32 for AES-128/256) */
    uint16_t mac_len; /* bytes (e.g. 16 for CMAC/GCM tag)   */
    uint16_t iv_len;  /* bytes (e.g. 12 for AES-GCM)        */
    FF_Status_t (*init)(void);
    FF_Status_t (*encrypt)(const uint8_t *key, const uint8_t *iv, const uint8_t *in, uint16_t len,
                           uint8_t *out);
    FF_Status_t (*decrypt)(const uint8_t *key, const uint8_t *iv, const uint8_t *in, uint16_t len,
                           uint8_t *out);
    FF_Status_t (*mac)(const uint8_t *key, const uint8_t *iv, const uint8_t *data, uint16_t len,
                       uint8_t *mac_out);
    FF_Status_t (*key_derive)(const uint8_t *master, uint16_t master_len, const uint8_t *info,
                              uint16_t info_len, uint8_t *out_key);
};

/* Security Association (bounded, SPI-indexed). `master_key` is long-lived;
 * `session_key` is derived from it and rotated. */
typedef struct
{
    uint16_t spi;
    uint8_t active;
    FF_SdlsService_t service;
    uint8_t master_key[FF_SDLS_KEY_MAX];
    uint16_t master_key_len;
    uint8_t session_key[FF_SDLS_KEY_MAX];
    uint16_t session_key_len;
    uint32_t epoch;             /* session-key rotation counter            */
    uint32_t tx_count;          /* frames protected with the current key   */
    uint32_t max_tx;            /* rotation threshold in frames (0 = none) */
    uint64_t activation_ns;     /* when the current session key was set    */
    uint64_t lifetime_ns;       /* rotation threshold in time (0 = none)   */
    uint8_t iv[FF_SDLS_IV_MAX]; /* IV/initialisation base              */
    uint16_t iv_len;
    uint32_t iv_counter; /* per-frame IV counter (uniqueness)          */
} FF_Sa_t;

/* Lifecycle and backend.
 *
 * Concurrency: SA configuration (add/activate/rotate/remove/zeroize) is a
 * control-plane activity executed single-threaded at start-up or from the
 * security-management task; protect/unprotect are executed from the link task
 * that owns the SPIs. The module does not lock (no hidden blocking in the
 * datapath); callers serialise control-plane updates against the datapath. */
FF_Status_t FF_SdlsInit(void);
FF_Status_t FF_SdlsSetBackend(FF_CryptoBackend_t *backend);
FF_CryptoBackend_t *FF_SdlsGetBackend(void);

/* SA management (bounded by FF_SDLS_MAX_SA). Add derives the first session key
 * from the master key; removing an SA zeroises its key material. */
FF_Status_t FF_SdlsSaAdd(const FF_Sa_t *sa);
FF_Status_t FF_SdlsSaGet(uint16_t spi, FF_Sa_t *out);
FF_Status_t FF_SdlsSaActivate(uint16_t spi, uint8_t active);
FF_Status_t FF_SdlsSaRemove(uint16_t spi); /* zeroises keys */
uint16_t FF_SdlsSaCount(void);

/* Key management: rotate the session key of an SA (zeroise old, derive new,
 * increment epoch, reset usage/time), and the policy tick that rotates every SA
 * beyond its frame/time threshold. */
FF_Status_t FF_SdlsRotateKey(uint16_t spi, uint64_t now_ns);
FF_Status_t FF_SdlsTick(uint64_t now_ns);
FF_Status_t FF_SdlsZeroize(void); /* wipe all key material */

/* SDLS processing. On the wire: [SPI:2 BE][IV:iv_len][payload][MAC:mac_len].
 * `payload` is encrypted for ENC/AUTH_ENC and a MAC is appended for
 * AUTH/AUTH_ENC. Protect increments the per-frame IV counter and usage. */
FF_Status_t FF_SdlsProtect(uint16_t spi, const uint8_t *in, uint16_t in_len, uint8_t *out,
                           uint16_t out_cap, uint16_t *out_len, uint64_t now_ns);
FF_Status_t FF_SdlsUnprotect(const uint8_t *in, uint16_t in_len, uint8_t *out, uint16_t out_cap,
                             uint16_t *out_len);

#endif /* FF_SDLS_H */
