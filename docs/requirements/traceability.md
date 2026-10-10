# Traceability Matrices

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-TRACE-001                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Single source of truth            | `docs/requirements/traceability.yaml`  |
| Gate                              | `tools/scripts/trace.py` (norm N4)     |

This document is the human-readable presentation of the traceability of
frick-fsw. The authoritative machine-readable manifest is
`traceability.yaml`; this page mirrors its *current* content. Whenever the
manifest changes, `tools/scripts/trace.py` re-verifies convergence and this
document is regenerated/reviewed in the same commit (norms N8/N11).

Counts (current): SRS = 21, SSS = 15, GAP = 6, Lean theorems = 20, CI evidence
claims = 8.

## 1. Top-down matrix (SSS -> SRS)

| SSS | SRS children |
| --- | ------------ |
| SSS-REQ-001 (flight target) | SRS-REQ-017 |
| SSS-REQ-002 (software bus) | SRS-REQ-001, SRS-REQ-002 |
| SSS-REQ-003 (TM/TC chain) | SRS-REQ-003, SRS-REQ-004, SRS-REQ-009 |
| SSS-REQ-004 (request verification) | SRS-REQ-005, SRS-REQ-006, SRS-REQ-007 |
| SSS-REQ-005 (housekeeping) | SRS-REQ-008 |
| SSS-REQ-006 (events) | SRS-REQ-011 |
| SSS-REQ-007 (time) | SRS-REQ-010 |
| SSS-REQ-008 (health & safety) | SRS-REQ-014 |
| SSS-REQ-009 (tables) | SRS-REQ-013, SRS-REQ-019 |
| SSS-REQ-010 (determinism) | SRS-REQ-015 |
| SSS-REQ-011 (multi-target build) | SRS-REQ-016 |
| SSS-REQ-012 (doc set) | FRICK-SRS-REQ-000 |
| SSS-REQ-013 (continuous verification) | SRS-REQ-020 |
| SSS-REQ-014 (link/transport) | SRS-REQ-012 |
| SSS-REQ-015 (PUS backends) | SRS-REQ-018 |

## 2. Requirement verification summary (SRS)

| Requirement | Implementation | Tests / CI | Lean | Note |
| ----------- | -------------- | ---------- | ---- | ---- |
| FRICK-SRS-REQ-000 | CMakeLists, presets, Makefile | ci:host-build | - | baseline |
| SRS-REQ-001 | sb.c/h | test_sb.c | req_srs001_exact_and_wildcard, req_srs001_no_match_no_delivery | |
| SRS-REQ-002 | sb.c, pal_posix.c | test_sb.c | - | FIFO by ring-buffer semantics |
| SRS-REQ-003 | ccsds.c | test_ccsds.c | req_srs003_tm_length, req_srs003_tc_length | |
| SRS-REQ-004 | ccsds.c | test_ccsds.c | req_srs004_crc_appended_zero, req_srs004_crc_initial | |
| SRS-REQ-005 | pus.c | test_pus.c | - | dispatch contract in Frick.Pus |
| SRS-REQ-006 | pus.c | test_pus.c | req_srs006_acceptance_completion, complete_ack_produces_two, ack_acceptance_is_counted | |
| SRS-REQ-007 | pus.c | test_pus.c | req_srs007_counter_increases, req_srs007_counter_wrap_bounded | |
| SRS-REQ-008 | pus_services.c, app_hk.c | test_pus.c | - | GAP-A2 formalisation |
| SRS-REQ-009 | pus_services.c | test_pus.c | - | region bound asserted |
| SRS-REQ-010 | time.c, pus_services.c | test_pus.c, test_ccsds.c | - | CUC round-trip |
| SRS-REQ-011 | evs.c, fsw_main.c, pus.c | test_pus.c, ci:simulator-smoke | - | observed in simulator |
| SRS-REQ-012 | pus_services.c | test_pus.c | - | S17 ping/pong |
| SRS-REQ-013 | tbl.c | test_tbl.c | - | atomic activate |
| SRS-REQ-014 | es.c, app_hs.c | ci:simulator-smoke | - | roadmap HS unit tests |
| SRS-REQ-015 | pal_posix.c, pal_rtems.c | ci:static-analysis | - | no heap after init |
| SRS-REQ-016 | CMake, rtems toolchain, pal | ci:dual-build | - | same sources |
| SRS-REQ-017 | fsw_config.h, ccsds.c | test_ccsds.c | - | derived: tailoring |
| SRS-REQ-018 | pus.h, adapters | test_pus.c | - | derived: ADR-0003 |
| SRS-REQ-019 | fsw_config.h, fsw_tables.c | test_tbl.c | - | derived: cFS heritage |
| SRS-REQ-020 | workflows, tools/docker, AGENTS.md, docs | ci:trace, ci:docs | - | derived: N6/N8 |

Verification method mapping: **Tst** unit/integration tests, **Anl** analysis /
Lean proofs, **Inp** inspection/static analysis, **Dem** demonstration
(simulator / CI), **MIR** mission readiness. Each SRS row above is associated
with the methods exercised by its Tests/CI evidence plus its Lean links (Anl).

## 3. System level (SSS) verification summary

| Requirement | Implementation | Tests / CI | Lean |
| ----------- | -------------- | ---------- | ---- |
| SSS-REQ-001 | board.c, platform_mpfs.h, pal_rtems.c, rtems toolchain | ci:rtems-cross-build | - |
| SSS-REQ-002 | sb.c, fsw_config.h | test_sb.c | req_sss002_queue_bounded, fits_queue_when_bounded |
| SSS-REQ-003 | ccsds.c, pus.c, link.c | test_pus.c, test_ccsds.c | crc16_append_makes_zero |
| SSS-REQ-004 | pus.c | test_pus.c | req_srs006_acceptance_completion |
| SSS-REQ-005 | pus_services.c, app_hk.c | ci:simulator-smoke | - |
| SSS-REQ-006 | evs.c, fsw_main.c, pus.c | ci:simulator-smoke | - |
| SSS-REQ-007 | time.c, pus_services.c | test_ccsds.c | - |
| SSS-REQ-008 | app_hs.c, es.c | ci:simulator-smoke | - |
| SSS-REQ-009 | tbl.c, fsw_tables.c | test_tbl.c | - |
| SSS-REQ-010 | pal, fsw_config.h | ci:static-analysis | - |
| SSS-REQ-011 | CMake, presets, toolchain | ci:dual-build | - |
| SSS-REQ-012 | SRS/SSS/traceability/quality docs | ci:trace, ci:docs | - |
| SSS-REQ-013 | workflows, tools/docker | ci:ci-pipeline | - |
| SSS-REQ-014 | link.h, link_loopback.c, link_uart.c | test_pus.c | - |
| SSS-REQ-015 | adapters | test_pus.c | - |

## 4. Gaps to ECSS class A (norm N5)

| Gap | Meaning | Reference |
| --- | ------- | --------- |
| GAP-A1 | arbitrary-length CRC induction / full formal embedding | class-a-gaps.md, Frick/Ccsds.lean |
| GAP-A2 | full PUS-C service set formalisation & per-service tests | class-a-gaps.md |
| GAP-A3 | independent SES / QA organisation | class-a-gaps.md |
| GAP-A4 | MISRA evidence, WCET, stack analysis | class-a-gaps.md |
| GAP-A5 | on-target RTEMS / HWIL qualification | class-a-gaps.md |
| GAP-A6 | radiation / environmental campaign | class-a-gaps.md |

## 5. Bottom-up checks performed by the gate

`tools/scripts/trace.py` additionally verifies: every id in SRS.md/SSS.md exists
in the manifest and vice versa; every srs has one sss parent and every sss at
least one srs child; every implementation/test path exists; every `ci:*` claim
is declared in a workflow; every linked Lean theorem exists; every GAP id is
referenced in class-a-gaps.md; no `req_*` theorem is left unlinked.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial matrices aligned with traceability.yaml |