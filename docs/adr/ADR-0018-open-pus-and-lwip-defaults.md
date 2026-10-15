# ADR-0018 - Open PUS backend and lwIP default configuration

| Status | Accepted by the repository owner instruction; implementation subject to project review gates |
| ------ | -------- |
| Date   | 2026-10-15 |

## Context

The repository default had selected the N7/ESA ASN.1 backend only when generated
sources were available, otherwise changing to the open CCSDS backend after a
configure warning. The lwIP adapter was opt-in despite the project having a
pinned, fetched lwIP dependency and tested adapter. The owner directed that the
default PUS backend be **OPEN** and lwIP be **ON**.

## Decision

- The local open CCSDS PUS backend is the default. `FF_USE_PUS_ASN1` defaults to
  `OFF`; setting it to `ON` explicitly enables the optional N7/ESA backend and
  requires its generated files (`make pus-asn1`). The PUSopen adapter remains
  optional and requires a user-supplied SDK.
- `FF_USE_LWIP` defaults to `ON`. The pinned lwIP tree must be fetched before
  CMake configure (`make configure` does this; direct CMake/CI flows must run
  `tools/scripts/fetch_third_party.sh` first). Users may explicitly disable
  lwIP with `-DFF_USE_LWIP=OFF`.
- This default does not assert that lwIP is the only supported TCP/IP
  implementation or that its network behavior is mission-qualified.

## Consequences

- Default host and RTEMS builds compile/link the lwIP adapter and pinned stack;
  resource, configuration, security, timing, target-interoperability, and
  licensing/distribution impacts remain subject to project-specific evidence.
- Clean CI jobs fetch pinned third-party inputs before any default CMake
  configuration. Fetch success does not establish immutable build provenance.
- The N7 backend remains available as an opt-in alternative; deep per-service
  ASN.1 type-codec mapping remains M15. `SRS-REQ-050` and current documentation
  specify the new default and optional backend status.
- This decision supersedes ADR-0016's default-selection decision. Its
  build-time-generation and no-vendoring rationale remains applicable to the
  optional N7 backend.

## Traceability

`SRS-REQ-050`, `SSS-REQ-015`, `FF_USE_PUS_ASN1`, `FF_USE_LWIP`,
`docs/status/loop-audit.md`.
