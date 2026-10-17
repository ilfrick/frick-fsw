# ADR-0018 - Open PUS backend and lwIP default configuration

| Status | Superseded for PUS default by ADR-0019; lwIP decision remains in force |
| ------ | -------- |
| Date   | 2026-10-15 |

## Context

At the time of this decision, the owner directed that the default PUS backend be
**OPEN** and lwIP be **ON**. The later PUS-default correction is recorded in
ADR-0019; this ADR remains the controlling record for the lwIP decision.

## Decision

- The former open-PUS default is superseded by ADR-0019. See ADR-0019 for the
  current N7/ESA default and explicit open-backend selection. The PUSopen
  adapter remains optional and requires a user-supplied SDK.
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
- Deep per-service ASN.1 type-codec mapping remains M15; see ADR-0019 for the
  current default and integration limits.
- The PUS-default portion of this ADR supersedes ADR-0016 temporarily; ADR-0019
  reinstates its default-selection intent. The build-time-generation and
  no-vendoring rationale of ADR-0016 remains applicable.

## Traceability

`SRS-REQ-050`, `SSS-REQ-015`, `FF_USE_PUS_ASN1`, `FF_USE_LWIP`,
`docs/status/loop-audit.md`.
