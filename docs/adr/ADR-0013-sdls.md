# ADR-0013 - CCSDS SDLS security sub-layer with pluggable cryptography

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Telemetry and telecommand links on spacecraft may require link-layer security
(authentication and/or encryption). CCSDS SDLS (Space Data Link Security,
CCSDS 355.0-B) defines the protocol, the Security Association (SA) concept and
key management; the cryptographic algorithms are defined separately
(CCSDS 352.0-B: AES-GCM/CMAC, ...) and are typically mission- and
hardware-specific (HSM/keystore, radiation-tolerant implementations).

## Decision
- Add a **CCSDS SDLS sub-layer** (milestone M7, `src/security/ff_sdls.{c,h}`)
  to the middleware that implements the parts that belong to flight software:
  the SDLS processing (SDLS header + MAC, configurable security service), the
  **security-association database**, and the **key lifecycle** (session-key
  derivation, rotation on frame/time thresholds, zeroisation).
- Keep the **cryptographic primitive pluggable** through
  `FF_CryptoBackend_t` (encrypt/decrypt/MAC/key-derive). The middleware does
  **not** implement a cipher; the mission supplies a software or HW-accelerated
  backend.
- Ship a **non-secure reference backend** (`ff_crypto_test.c`) used only by the
  unit tests (it is not built into the flight or simulator binaries); it is
  explicitly not for flight.
- Key material can be persisted through the HAL non-volatile-memory contract
  (`FF_Nvm_t`); the SA/key software stays independent of the storage.

## Rationale
- Same design principle as the HAL and the PUS/open backend: a stable contract
  with a swappable implementation, so the certified algorithm/HSM choice is a
  configuration/integration decision, not a rewrite.
- Keeps the middleware free of cryptographic algorithm liability while still
  delivering the security protocol logic and key management that are genuinely
  flight-software concerns.

## Consequences
- The SDLS protocol and key management are testable on the host with the
  reference backend (deterministic); the real algorithm, the hardware keystore,
  the IV-uniqueness analysis and the ground key management are a **class-A gap
  (GAP-A14)**.
- Key zeroisation uses a compiler-unelidable wipe; hardware-protected key
  storage/zeroisation remains the mission's responsibility.

## Traceability
SSS-REQ-019, SRS-REQ-034, SRS-REQ-035; norms N3/N4/N8.
