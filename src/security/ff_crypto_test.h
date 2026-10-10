/*
 * frick-fsw - Reference (NON-SECURE) cryptographic backend for SDLS V&V
 *
 * This backend is a deterministic, reversible transform used only to exercise
 * the SDLS SA/key-management logic in the unit tests (it is not built into the
 * flight or simulator binaries). It is **NOT cryptographic and MUST NOT be used
 * in flight**. The real algorithms (CCSDS 352.0-B: AES-GCM/CMAC, ...) are
 * mission/HW specific and are a class-A qualification gap (GAP-A14). See
 * docs/architecture/security-sdls.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_CRYPTO_TEST_H
#define FF_CRYPTO_TEST_H

#include "security/ff_sdls.h"

extern FF_CryptoBackend_t FF_CryptoTestBackend;

#endif /* FF_CRYPTO_TEST_H */
