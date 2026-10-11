# ADR-0016 - PUS-C ASN.1 default: build-time generation vs vendoring

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-11 |

## Context
The PUS-C layer must use the N7 Space / ESA `asn1-pusc-lib` ASN.1/ACN model
(ASN1SCC-compiled) as the **default** basis (SRS-REQ-050, norm N19
reconciliation). `asn1-pusc-lib` is an ASN.1 model plus small C wrappers that
depend on ASN1SCC-generated headers; the generated code and the ASN1SCC runtime
are license-clean (ASN1SCC is GPL-with-runtime-exception), the wrappers are
GPL-3.0 (combinable with this AGPL-3.0 project). Two ways to make the artifacts
available to the build were considered: **vendor** them into the repository
(commit generated + wrappers) or **generate at build time**.

## Decision
**Build-time generation** (option B). Nothing generated or third-party is
committed: `tools/scripts/gen_pus_asn1.sh` (`make pus-asn1`) fetches the pinned
`asn1-pusc-lib`, builds the pinned ASN1SCC compiler image
(`tools/docker/asn1scc/Dockerfile`, esa/asn1scc `3124590a…`) and generates the C
into `third_party/asn1-pusc-lib-generated/` (git-ignored). CMake option
`FF_USE_PUS_ASN1` (default ON) compiles the generated code + the N7 wrappers and
selects `FF_PusBackendN7` as the default backend. If the generated code is
absent, the configure step emits a **warning** and falls back to the open CCSDS
backend (so a fresh checkout / CI without Docker still builds), and `-DFF_USE_PUS_ASN1=OFF`
forces the open backend.

## Consequences
- The repository stays clean (no generated/third-party code committed; consistent
  with the lwIP / PUS-C "fetched, not committed" policy).
- The default PUS-C ASN.1 backend is active where the toolchain is available
  (local/nightly); a checkout without `make pus-asn1` uses the open backend with a
  visible warning.
- CI/per-commit runners without Docker-in-Docker do not build the ASN1SCC image;
  they run with the fallback. The N7 path is exercised where the generated code
  is present: the host unit test `tests/unit/test_pus.c` asserts the N7 backend's
  TM output is byte-identical to the reference (same ECSS Annex B.1 CRC), and the
  Renode functional run uses the N7 backend as the default. A generation step in
  the controlled/nightly flow is tracked (norm N19).
- Tool qualification: ASN1SCC generation is V&V tool **T19**.
- The deep per-service ASN.1 type-codec mapping and the fundamental service
  behaviour remain milestone **M15** (`docs/architecture/pus-c-from-scratch-assessment.md`).

## Traceability
SRS-REQ-050, SSS-REQ-015; T19; `docs/status/loop-audit.md`.
