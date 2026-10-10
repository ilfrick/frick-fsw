# ADR-0008 - Single configuration point + mission tables

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
cFS heritage: a single, reviewable configuration point prevents
misconfiguration and eases mission tailoring.

## Decision
- All framework tunables in `config/fsw_config.h` (SB/EVS/ES/TBL/TIME/PUS
  limits, mission identifiers, APIDs, link profile).
- Mission data in `config/tables/fsw_tables.c` and the `config/platform/`
  platform headers.
- Tables are managed through Table Services (load/validate/activate).

## Consequences
Porting to a new mission is a reviewable config change; the traceability
manifest links requirements to the configuration.

## Traceability
SRS-REQ-013, SRS-REQ-017, SRS-REQ-019, SSS-REQ-009.