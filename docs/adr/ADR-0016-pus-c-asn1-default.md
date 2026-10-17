# ADR-0016 - PUS-C ASN.1 default: build-time generation vs vendoring

| Status | Default selection reinstated by ADR-0019; generation/no-vendoring rationale retained (ADR-0003/ADR-0014; ADR-0015 skipped) |
| ------ | -------- |
| Date   | 2026-10-11 |

## Context
_(Naming note: ADR-0015 is intentionally skipped; no decision was taken under
that number. This ADR **supersedes** the "open backend as default" choice of
ADR-0003/ADR-0014 for the PUS-C data model.)_

The PUS-C layer must use the N7 Space / ESA `asn1-pusc-lib` ASN.1/ACN model
(ASN1SCC-compiled) as the **default** basis (SRS-REQ-050, norm N19
reconciliation). `asn1-pusc-lib` is an ASN.1 model plus small C wrappers that
depend on ASN1SCC-generated headers; the generated code and the ASN1SCC runtime
are license-clean (ASN1SCC is GPL-with-runtime-exception), the wrappers are
GPL-3.0 (the earlier AGPL project analysis does not disposition the current
MPL-2.0 transition; see open NC-EXT-43). Two ways to make the artifacts
available to the build were considered: **vendor** them into the repository
(commit generated + wrappers) or **generate at build time**.

## Decision at the time (updated by ADR-0019)
**Build-time generation** (option B). Nothing generated or third-party is
committed: `tools/scripts/gen_pus_asn1.sh` (`make pus-asn1`) fetches the pinned
`asn1-pusc-lib`, builds the pinned ASN1SCC compiler image
(`tools/docker/asn1scc/Dockerfile`, esa/asn1scc `3124590a…`) and generates the C
into `third_party/asn1-pusc-lib-generated/` (git-ignored). CMake option
`FF_USE_PUS_ASN1` (default ON) compiles the generated code + the N7 wrappers and
selects `FF_PusBackendN7` as the default backend. That original decision was
temporarily superseded for PUS selection by ADR-0018 and is now reinstated by
ADR-0019. Current CMake configuration fails if generated inputs are absent;
`make pus-asn1` generates them. `-DFF_USE_PUS_ASN1=OFF` explicitly selects the
open backend. The earlier warning/fallback behavior is no longer current.

## Consequences
- The repository stays clean (no generated/third-party code committed; consistent
  with the lwIP / PUS-C "fetched, not committed" policy).
- Current host and emulated build entry points explicitly generate or require
  the pinned common ASN1SCC files; CI workflows now include that generation step.
  The in-repository adapter currently uses generated common type widths and the
  N7/ESA CRC wrapper while delegating framing and PUS field coding to the
  reference codec; full service mappings remain M15.
- Tool qualification: ASN1SCC generation is V&V tool **T19**.
- The deep per-service ASN.1 type-codec mapping and the fundamental service
  behaviour remain milestone **M15** (`docs/architecture/pus-c-from-scratch-assessment.md`).

ADR-0018 temporarily superseded this ADR's PUS-default selection. ADR-0019
reinstates it; ADR-0018's lwIP default decision remains in force.

## Traceability
SRS-REQ-050, SSS-REQ-015; T19; `docs/status/loop-audit.md`.
