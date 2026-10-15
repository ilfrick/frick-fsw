# ADR-0003 - Pluggable PUS backend (open default, PUSopen optional)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
PUS-C encoding/decoding can be done with an own auditable codec or with the
commercial PUSopen(R) library. Norm N1 requires the auditable path to be the
default so that Lean verification covers the buildable product.

## Decision
- Define an abstract backend: `FF_Pus_Backend_t` vtable
  (encode/decode TC/TM, verification hooks) in `src/pus/pus.h`.
- Default backend `FF_PusBackendOpen` built on the local CCSDS codec
  (`src/pus/adapters/pus_adapter_open.c`).
- Optional backend `FF_PusBackendPusopen` compiled only with
  `FRICK_USE_PUSOPEN=1`
  (`src/pus/adapters/pus_adapter_pusopen.c`); the commercial library is never
  distributed.

## Consequences
Default path is project-authored open source targeted for MPL-2.0 (norm N13, transition
authority/compatibility still open under NC-EXT-43) and Lean-checkable; the
PUSopen adapter is a documented drop-in for customers who own the licence.

## Traceability
SRS-REQ-018, SSS-REQ-015.
