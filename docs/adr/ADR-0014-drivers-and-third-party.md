# ADR-0014 - Reference drivers and third-party integration (lwIP, N7/ESA PUS)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Milestone M4 integrates (a) a **reference Ethernet driver** for the PolarFire SoC
GEM and (b) open third-party components for the network stack and the PUS-C
product, behind the existing HAL/`FF_Pus_Backend_t` contracts, without coupling
the middleware to them.

## Decision
- **Ethernet driver**: a reference `FF_EthMac_t` implementation for the Cadence
  GEM (`src/drivers/eth/gem/`), static and allocation-free, with injected MMIO
  and HAL `FF_Dma_t` translation so the register/descriptor logic is host-testable
  (mock MMIO). TSN is available on the **GEM natively** (802.1Q/Qbv/Qav/802.1AS
  per the PolarFire SoC Product Overview §3.5.1); the fabric CoreTSN IP is only
  for multi-port switching/FRER.
- **TCP/IP**: integrate **lwIP** (BSD-3, embedded heritage) as the reference
  `FF_NetStack_t` backend, vendored **on demand** (pinned tag
  `STABLE-2_2_1_RELEASE`) via `tools/scripts/fetch_third_party.sh`; Tuxera stays
  a user-supplied adapter (closed source).
- **PUS-C**: integrate the **ESA/N7 Space `asn1-pusc-lib`** (GPL-3.0) as an
  alternative `FF_Pus_Backend_t` backend, vendored on demand (pinned commit);
  PUSopen stays a user-supplied commercial adapter.
- **Historical licence compatibility note (superseded):** this ADR's prior
  AGPL-3.0 compatibility analysis is not a disposition for the current MPL-2.0
  transition. Preserve BSD/GPL upstream notices, and track exact incorporation,
  generated-code and distribution-profile review under NC-EXT-43. Do not infer
  compatibility or release permission from this historical text. Third-party sources are **not committed**
  (size/CM); they are fetched at a **pinned version** and recorded in
  `third_party/README.md` and `docs/quality/licensing.md`.
- **WP-FV-100** (formal completeness): increase Lean coverage to 100% of
  generated requirements (theorem or explicit `lean_note`) and mirror the pure
  logic modules in Lean; proving the *C as compiled* uses Frama-C/CBMC/CompCert
  alongside Lean (GAP-A1).

## Rationale
The middleware depends only on contracts; drivers and stacks are swappable,
optional and separately reviewable. Pinned fetch keeps configuration management
clean while enabling real integration.

## Consequences
- The Ethernet driver is testable on the host; on-target PHY/link validation is
  GAP-A5.
- Network/PUS third-party code is exercised only when fetched (`FF_USE_LWIP`,
  `FF_USE_N7PUS`); CI without network builds the adapters as stubs.
- Formal completeness is tracked as WP-FV-100 (Lean models) + Frama-C/CBMC for
  C-level proofs (GAP-A1).

## Traceability
SSS-REQ-017/020, SRS-REQ-026/027/036; norms N3/N4/N8/N13.
