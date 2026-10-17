# ADR-0019 - N7/ESA PUS backend default

| Status | Accepted by repository owner instruction; implementation subject to project review gates |
| ------ | -------- |
| Date   | 2026-10-16 |

## Context

The repository has conflicting historical decisions: ADR-0016 selected the
N7/ESA ASN.1 backend by default, while ADR-0018 changed the default to the open
CCSDS backend. The project owner clarified that the N7/ESA backend is intended
to be ON by default, as it is the PUS implementation selected for project
testing. The open backend remains useful as an explicitly selected alternative.

The current N7 adapter uses the generated ASN1SCC `BasicTypes` model for
compile-time encoding-width checks and the N7/ESA CRC implementation. CCSDS
packet framing and PUS field encode/decode remain delegated to the local CCSDS
codec. This decision does not claim complete service-specific ASN.1 codec
integration or external N7 SDK/profile compatibility; those remain separately
tracked scope.

## Decision

- `FF_USE_PUS_ASN1` defaults to `ON`; a normal configured build selects
  `FF_PusBackendN7` and requires the pinned N7 source plus generated common
  ASN1SCC files (`make pus-asn1`).
- `-DFF_USE_PUS_ASN1=OFF` explicitly selects the always-available open CCSDS
  backend. Missing generated inputs with the option ON are a configure error,
  not a silent fallback.
- `FF_USE_LWIP` remains `ON`; its fetch/configuration policy is unchanged.
- This ADR supersedes the PUS-default portion of ADR-0018 and reinstates the
  default-selection intent of ADR-0016. ADR-0018's lwIP decision remains in
  force.

## Consequences and limits

- Default host tests exercise the generated N7 common code and N7/ESA CRC
  wrapper through the configured backend. The generated source and pinned
  library are build inputs, not committed files.
- The default must be built and tested with the generated inputs in CI and in
  local verification. Users deliberately choosing the open backend must set
  `FF_USE_PUS_ASN1=OFF`.
- Host or emulation tests do not establish physical-target, ground-segment,
  profile interoperability, qualification, or release approval.

## Traceability

`SRS-REQ-050`, `SSS-REQ-025`, `FF_USE_PUS_ASN1`, `docs/vv/test-spec/ts-1-protocol-core.md`.
