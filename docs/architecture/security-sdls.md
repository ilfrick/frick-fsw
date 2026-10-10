# Security - CCSDS SDLS Sub-layer and Key Management

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0010                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N3, N4, N8                             |
| Requirements                      | SSS-REQ-019, SRS-REQ-034, SRS-REQ-035  |
| Decision                          | `docs/adr/ADR-0013-sdls.md`            |

## 1. Purpose and scope

This note describes the CCSDS Space Data Link Security (SDLS, CCSDS 355.0-B)
sub-layer that frick-fsw provides as **middleware**: the security protocol
processing, the Security Association (SA) database and the key lifecycle. It
also states explicitly what is **pluggable** (the cryptographic algorithms) and
what remains a **class-A gap** (algorithm/HSM qualification, IV-uniqueness
analysis, ground key management).

## 2. What the middleware implements vs what is pluggable

| Concern | Where | Status |
| ------- | ----- | ------ |
| SDLS processing (header + MAC, security service) | `src/security/ff_sdls.c` | implemented, tested |
| Security-association database (bounded, SPI-indexed) | `src/security/ff_sdls.c` | implemented, tested + Lean |
| Key lifecycle: derivation, rotation, zeroisation | `src/security/ff_sdls.c` | implemented, tested + Lean |
| Cryptographic primitive (cipher, MAC, KDF) | `FF_CryptoBackend_t` | **pluggable** (mission/HW) |
| Reference backend for V&V | `src/security/ff_crypto_test.c` | non-secure, test only |
| Key persistence | HAL `FF_Nvm_t` | contract available (M3) |
| Certified algorithm + HSM + ground key management | — | **GAP-A14** |

## 3. Security associations and key management

- An **SA** (`FF_Sa_t`) is indexed by **SPI** and selects the security service:
  `FF_SDLS_PLAIN` (bypass), `FF_SDLS_AUTH` (authentication only), `FF_SDLS_ENC`
  (encryption only), `FF_SDLS_AUTH_ENC` (authenticated encryption).
- A long-lived **master key** is used to derive a **session key** via the
  backend KDF, bound to `(SPI, epoch)`. The SA also carries the IV base, the
  frame usage counter, rotation thresholds (`max_tx` frames, `lifetime_ns`) and
  the activation time.
- **Rotation** (`FF_SdlsRotateKey`, policy `FF_SdlsTick`): the old session key
  is **zeroised**, the epoch is incremented (strictly increasing — no key reuse),
  a new session key is derived, and usage/time counters are reset. Rotation is
  triggered on the configured frame/time thresholds.
- **Zeroisation** (`FF_SdlsSaRemove`, `FF_SdlsZeroize`) wipes key material with
  a compiler-unelidable loop.
- The SA table is **bounded** (`FF_SDLS_MAX_SA`); no dynamic memory is used.

## 4. SDLS processing (on the wire)

A protected unit has the layout `[SPI:2 BE][IV:iv_len][payload][MAC:mac_len]`
(the exact SDLS frame format follows CCSDS 355.0-B and the mission SDLS
Extended Procedures, CCSDS 352.0-B; this module is the flight-software core of
that processing):

- the **IV** is the SA base with a per-frame counter written big-endian, giving
  IV uniqueness within a session key; the backend must provide `iv_len >= 4`
  (enforced by `FF_SdlsSetBackend`) so the counter is embedded;
- the payload is **encrypted** for `ENC`/`AUTH_ENC` and passed through for
  `AUTH`;
- a **MAC** is appended for `AUTH`/`AUTH_ENC`, computed over `SPI || IV ||
  payload`; validation uses a constant-time comparison and rejects tampered
  units.

## 5. Assumptions and limitations

- The middleware assumes the mission supplies a **certified** algorithm and key
  material through the backend; it does not validate cryptographic strength.
- The non-secure reference backend **must not be used in flight**.
- IV uniqueness is per session key; long-lived keys and counter exhaustion are
  handled by rotation. The quantitative IV-uniqueness analysis for the chosen
  algorithm is part of GAP-A14.
- Anti-replay/sequence-number semantics and the mapping of Space Packets to
  secured transfer frames are mission-tailored (the middleware currently
  secures a generic data unit).

## 6. Verification

- `tests/unit/test_sdls.c` (SRS-REQ-034/035): protect/unprotect round trip,
  tamper rejection, SA add/get/activate/remove, capacity bound, key rotation
  (epoch increase, usage reset, key change), policy tick and zeroisation — with
  the reference backend (deterministic).
- Lean `Frick.Sdls` (SRS-REQ-035): `req_srs035_sa_bound`,
  `req_srs035_epoch_monotonic`, `req_srs035_uses_reset`.

## 7. Gaps (class A)

GAP-A14: cryptographic algorithm qualification (CCSDS 352.0-B), hardware
keystore with protected zeroisation, IV/counter uniqueness analysis, and ground
key-management/key-distribution procedures.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial SDLS sub-layer and key-management note (milestone M7) |
