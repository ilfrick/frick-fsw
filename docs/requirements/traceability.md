# Traceability Matrices

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-TRACE-001                        |
| Revision                          | 0.27                                   |
| Date                              | 2026-10-11                             |
| Single source of truth            | `docs/requirements/traceability.yaml`  |
| Gate                              | `tools/scripts/trace.py` (norm N4)     |

This document is the human-readable presentation of the traceability of
frick-fsw. The authoritative machine-readable manifest is
`traceability.yaml`; this page mirrors its *current* content. Whenever the
manifest changes, `tools/scripts/trace.py` re-verifies convergence and this
document is regenerated/reviewed in the same commit (norms N8/N11).

Counts (current): SRS = 58, SSS = 27, GAP = 14, Lean theorems = 51, CI evidence
claims = 12 unique workflow claim names (11 referenced by the manifest; `ci-lean`
is reported in CI but not claimed by any requirement, norm N1). Five of the
requirements (SRS-REQ-045/046/051/053/054 with their SSS parents
SSS-REQ-022/023/024/026/027) are **planned** (M12..M16): they
are documented-only, with `ci:docs` as evidence until their implementation lands
(SRS-REQ-052 is delivered by the fundamental PUS-C service set, M15).

## 1. Top-down matrix (SSS -> SRS)

| SSS | SRS children |
| --- | ------------ |
| SSS-REQ-001 (flight target) | SRS-REQ-017, SRS-REQ-037 |
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
| SSS-REQ-013 (continuous verification) | SRS-REQ-020, SRS-REQ-043, SRS-REQ-044 |
| SSS-REQ-014 (link/transport) | SRS-REQ-012 |
| SSS-REQ-015 (PUS backends) | SRS-REQ-018 |
| SSS-REQ-016 (time base) | SRS-REQ-021, SRS-REQ-022, SRS-REQ-023, SRS-REQ-024 |
| SSS-REQ-017 (hardware abstraction) | SRS-REQ-025, SRS-REQ-026, SRS-REQ-027, SRS-REQ-028, SRS-REQ-029, SRS-REQ-030, SRS-REQ-031, SRS-REQ-032 |
| SSS-REQ-018 (numerical support) | SRS-REQ-033 |
| SSS-REQ-019 (data-link security) | SRS-REQ-034, SRS-REQ-035 |
| SSS-REQ-020 (reference drivers) | SRS-REQ-036, SRS-REQ-038, SRS-REQ-039, SRS-REQ-040, SRS-REQ-047, SRS-REQ-048, SRS-REQ-049, SRS-REQ-055, SRS-REQ-056, SRS-REQ-057 |
| SSS-REQ-021 (code quality) | SRS-REQ-041, SRS-REQ-042 |
| SSS-REQ-022 (on-board autonomy / OBCP) | SRS-REQ-045 |
| SSS-REQ-023 (reference mission services) | SRS-REQ-046 |
| SSS-REQ-024 (multicore / SMP) | SRS-REQ-051 |
| SSS-REQ-025 (fundamental PUS-C service set) | SRS-REQ-052 |
| SSS-REQ-026 (CFDP + virtual filestore) | SRS-REQ-053 |
| SSS-REQ-027 (in-house OBCP engine) | SRS-REQ-054 |

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
| SRS-REQ-010 | time.c, pps.c, pus_services.c | test_pus.c, test_ccsds.c, test_time.c | req_srs024_cuc_field_roundtrip | CUC round-trip |
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
| SRS-REQ-021 | time.c, pps.c | test_time.c | req_srs021_sources_distinct, req_srs021_source_set_disjoint, req_srs021_utc_non_regression | source selection |
| SRS-REQ-022 | time.c | test_time.c | req_srs022_tone_second_advance, req_srs022_tone_count_strict | tone discipline |
| SRS-REQ-023 | time.c | test_time.c | req_srs023_leap_cleared_at_tone, req_srs023_leap_accumulated_at_tone | leap at boundary |
| SRS-REQ-024 | time.c | test_time.c | req_srs024_cuc_field_roundtrip, req_srs024_fine_frac_bounded, req_srs024_coarse_byte_decompose | CUC framing |
| SRS-REQ-025 | hal.c/h, fsw_config.h | test_hal.c | req_srs025_registry_bound, req_srs025_dup_unchanged | HAL registry |
| SRS-REQ-026 | hal_eth.h | test_hal.c | - | MAC + net interface contract |
| SRS-REQ-027 | hal_net.h | test_hal.c | - | pluggable stack (Tuxera/lwIP/libbsd) |
| SRS-REQ-028 | hal_tsn.h | test_hal.c | - | TSN backend contract |
| SRS-REQ-029 | hal_can.h | test_hal.c | - | CAN / CAN-FD / CANopen |
| SRS-REQ-030 | hal_serial.h | test_hal.c | - | RS-422/485 contract |
| SRS-REQ-031 | hal_spi.h, hal_pcie.h, hal_gpio.h | test_hal.c | - | SPI/I2C/PCIe/GPIO contracts |
| SRS-REQ-032 | hal_spacewire.h, hal_mem.h | test_hal.c | - | SpaceWire/RMAP + memory/storage |
| SRS-REQ-033 | math/ff_math.c/h, CMake | test_math.c | - | libm facade (M6) |
| SRS-REQ-034 | security/ff_sdls.c/h | test_sdls.c | - | SDLS processing (M7) |
| SRS-REQ-035 | security/ff_sdls.c | test_sdls.c | req_srs035_sa_bound, req_srs035_epoch_monotonic, req_srs035_uses_reset | SA + key rotation |
| SRS-REQ-036 | drivers/eth/gem/ff_gem.c/h | test_gem.c, ci:renode | - | Cadence GEM Ethernet driver + Renode GEM identification (M4/M11) |
| SRS-REQ-037 | config/rtems/rtems_config.c, pal_rtems.c | ci:rtems-cross-build | - | RTEMS application integration (M4) |
| SRS-REQ-038 | drivers/eth/gem/ff_gem_tsn.c/h | test_gem_tsn.c | - | GEM native TSN backend (M4) |
| SRS-REQ-039 | drivers/net/ff_net_loopback.c/h | test_netloop.c, ci:renode | - | Reference loopback network stack; `FF_NetStack_t` contract check in Renode (M4/M11) |
| SRS-REQ-040 | hal/net/lwip/ff_net_lwip.c/h | test_lwip.c | - | lwIP network-stack adapter (M4) |
| SRS-REQ-041 | docs/quality/misra-c.md, tools/scripts/misra.sh | ci:misra | - | MISRA C:2012 compliance (M9) |
| SRS-REQ-042 | .clang-format, docs/engineering/coding-style.md | ci:static-analysis | - | C coding style (M9) |
| SRS-REQ-043 | tools/scripts/emulated_run.sh, docs/vv/emulated-target.md | ci:emulated-run | - | Emulated-target verification (M10) |
| SRS-REQ-044 | tools/docker/renode/, tools/scripts/renode_vv.sh, src/apps/app_renode_test.c | ci:renode | - | Renode functional validation, N18 in force (M10) |
| SRS-REQ-045 | docs/development-plan.md, docs/roadmap.md, docs/architecture/component-gap-analysis.md | ci:docs | - | OBCP engine uPython/MicroPython (planned, M12) |
| SRS-REQ-046 | docs/development-plan.md, docs/roadmap.md, docs/architecture/component-gap-analysis.md | ci:docs | - | Reference mission services (planned, M13) |
| SRS-REQ-047 | drivers/gpio/mpfs/ff_gpio_mpfs.c/h, config/platform/platform_mpfs.h | test_gpio.c | - | MSS GPIO driver + Renode MMIO check (M4/M11) |
| SRS-REQ-048 | drivers/serial/mpfs/ff_serial_mpfs.c/h, config/platform/platform_mpfs.h | test_serial.c | - | MSS MMUART serial driver + Renode MMIO check (M4/M11) |
| SRS-REQ-049 | drivers/mem/ff_mem_static.c/h | test_mem.c | - | Static RAM pool + DMA helper + Renode DDR check (M4/M11) |
| SRS-REQ-050 | src/pus/adapters/pus_adapter_n7.c, tools/scripts/gen_pus_asn1.sh, tools/docker/asn1scc/Dockerfile | test_pus.c, ci:renode | - | PUS-C ASN.1 (N7/ESA) default backend (implemented; ADR-0016) |
| SRS-REQ-051 | docs/development-plan.md, docs/roadmap.md | ci:docs | - | Multicore / SMP (planned, M14) |
| SRS-REQ-052 | docs/architecture/pus-c-from-scratch-assessment.md, src/pus/pus.h | ci:docs | - | Fundamental PUS-C service set (planned, M15) |
| SRS-REQ-053 | docs/development-plan.md, docs/roadmap.md | ci:docs | - | CFDP + virtual filestore (planned, M15) |
| SRS-REQ-054 | docs/development-plan.md, docs/roadmap.md, docs/status/loop-audit.md | ci:docs | - | In-house OBCP engine (planned, M16) |
| SRS-REQ-055 | drivers/nvm/mpfs/ff_nvm_mpfs.c/h, config/platform/platform_mpfs.h | test_nvm.c, ci:renode | - | Reference eNVM NVM backend (M4/M11) |
| SRS-REQ-056 | drivers/mtd/mpfs/ff_mtd_mpfs.c/h | test_mtd.c | - | Reference NOR/QSPI MTD backend (M4/M11; on-SoC QSPI check pending the QSPI controller command driver) |
| SRS-REQ-057 | drivers/can/mpfs/ff_can_mpfs.c/h, config/platform/platform_mpfs.h | test_can.c, ci:renode | - | Reference MSS CAN backend (M4/M11; classic 2.0 A/B, CAN-FD rejected) |

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
| SSS-REQ-007 | time.c, pps.c, pus_services.c | test_ccsds.c, test_time.c | req_srs024_cuc_field_roundtrip |
| SSS-REQ-008 | app_hs.c, es.c | ci:simulator-smoke | - |
| SSS-REQ-009 | tbl.c, fsw_tables.c | test_tbl.c | - |
| SSS-REQ-010 | pal, fsw_config.h | ci:static-analysis | - |
| SSS-REQ-011 | CMake, presets, toolchain | ci:dual-build | - |
| SSS-REQ-012 | SRS/SSS/traceability/quality docs | ci:trace, ci:docs | - |
| SSS-REQ-013 | workflows, tools/docker | ci:ci-pipeline | - |
| SSS-REQ-014 | link.h, link_loopback.c, link_uart.c | test_pus.c | - |
| SSS-REQ-015 | adapters | test_pus.c | - |
| SSS-REQ-016 | time.c, pps.c, fsw_config.h | test_time.c | req_srs022_tone_second_advance, req_srs021_utc_non_regression |
| SSS-REQ-017 | hal.c/h + hal_*.h, fsw_config.h | test_hal.c | req_srs025_registry_bound |
| SSS-REQ-018 | math/ff_math.c/h | test_math.c | - |
| SSS-REQ-019 | security/ff_sdls.c/h | test_sdls.c | req_srs035_epoch_monotonic |
| SSS-REQ-020 | drivers/eth/gem/ff_gem.c, CMakeLists.txt | test_gem.c | - |
| SSS-REQ-021 | .clang-format, docs/engineering/coding-style.md, docs/quality/misra-c.md | ci:static-analysis | - |
| SSS-REQ-022 | docs/development-plan.md, docs/roadmap.md, docs/architecture/component-gap-analysis.md | ci:docs | - |
| SSS-REQ-023 | docs/development-plan.md, docs/roadmap.md, docs/architecture/component-gap-analysis.md | ci:docs | - |
| SSS-REQ-024 | docs/development-plan.md, docs/roadmap.md | ci:docs | - |
| SSS-REQ-025 | docs/development-plan.md, docs/roadmap.md | ci:docs | - |
| SSS-REQ-026 | docs/development-plan.md, docs/roadmap.md | ci:docs | - |
| SSS-REQ-027 | docs/development-plan.md, docs/roadmap.md, docs/status/loop-audit.md | ci:docs | - |

## 4. Gaps to ECSS class A (norm N5)

| Gap | Meaning | Reference |
| --- | ------- | --------- |
| GAP-A1 | arbitrary-length CRC induction / full formal embedding | class-a-gaps.md, Frick/Ccsds.lean |
| GAP-A2 | full PUS-C service set formalisation & per-service tests | class-a-gaps.md |
| GAP-A3 | independent SES / QA organisation | class-a-gaps.md |
| GAP-A4 | MISRA evidence, WCET, stack analysis | class-a-gaps.md |
| GAP-A5 | on-target RTEMS / HWIL qualification | class-a-gaps.md |
| GAP-A6 | radiation / environmental campaign | class-a-gaps.md |
| GAP-A7 | on-target PPS capture and CCSDS/TAI epoch | class-a-gaps.md |
| GAP-A8 | Software Development Plan formalisation/tailoring | class-a-gaps.md |
| GAP-A9 | SDD per-function detailed design completeness | class-a-gaps.md |
| GAP-A10 | full software FMEA/FMECA (needs system hazards) | class-a-gaps.md |
| GAP-A11 | third-party independent verification of unit tests | class-a-gaps.md |
| GAP-A12 | operations manual / TM-TC database (OPD) | class-a-gaps.md |
| GAP-A13 | maintenance and obsolescence plan | class-a-gaps.md |
| GAP-A14 | SDLS cryptographic algorithm qualification and key store | class-a-gaps.md |

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
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: SSS-REQ-016, SRS-REQ-021..024, GAP-A7; counts updated |
| 0.3 | 2026-10-10 | frick-fsw | M3: SSS-REQ-017, SRS-REQ-025..032, GAP-A8..A13; counts 33/17/13, 45 theorems |
| 0.4 | 2026-10-10 | frick-fsw | M6: SSS-REQ-018, SRS-REQ-033; counts 34/18/13 |
| 0.5 | 2026-10-10 | frick-fsw | M7: SSS-REQ-019, SRS-REQ-034/-035, GAP-A14; counts 36/19/14, 51 theorems |
| 0.6 | 2026-10-10 | frick-fsw | M4: SSS-REQ-020, SRS-REQ-036; counts 37/20/14 |
| 0.7 | 2026-10-10 | frick-fsw | M4: SRS-REQ-037 RTEMS integration; counts 38/20/14 |
| 0.8 | 2026-10-10 | frick-fsw | M4: SRS-REQ-038 GEM native TSN backend; counts 39/20/14 |
| 0.9 | 2026-10-10 | frick-fsw | M4: SRS-REQ-039 reference network stack; counts 40/20/14 |
| 0.10 | 2026-10-10 | frick-fsw | M4: SRS-REQ-040 lwIP adapter; counts 41/20/14 |
| 0.11 | 2026-10-10 | frick-fsw | M9: SRS-REQ-041/042 code quality; counts 43/21/14 |
| 0.12 | 2026-10-10 | frick-fsw | M10: SRS-REQ-043 emulated-target verification; counts 44/21/14 |
| 0.13 | 2026-10-10 | frick-fsw | N18/M10: SRS-REQ-044 Renode environment; counts 45/21/14 |
| 0.14 | 2026-10-10 | frick-fsw | M12/M13: SSS-REQ-022/023, SRS-REQ-045/046 (OBCP engine, reference mission services); counts 47/23/14 |
| 0.15 | 2026-10-10 | frick-fsw | M4/M11: SRS-REQ-047 reference MSS GPIO driver (+Renode MMIO check); counts 48/23/14 |
| 0.16 | 2026-10-10 | frick-fsw | M4/M11: SRS-REQ-048 reference MSS MMUART serial driver (+Renode MMIO check); counts 49/23/14 |
| 0.17 | 2026-10-11 | frick-fsw | M4/M11: SRS-REQ-049 static RAM pool + DMA helper (+Renode DDR check); counts 50/23/14 |
| 0.18 | 2026-10-11 | frick-fsw | N19/M14: SSS-REQ-024, SRS-REQ-050 (PUS-C ASN.1 default), SRS-REQ-051 (multicore); counts 52/24/14 |
| 0.19 | 2026-10-11 | frick-fsw | M15: SSS-REQ-025, SRS-REQ-052 (fundamental PUS-C service set); counts 53/25/14 |
| 0.20 | 2026-10-11 | frick-fsw | M15: SSS-REQ-026, SRS-REQ-053 (CFDP + virtual filestore); counts 54/26/14 |
| 0.21 | 2026-10-11 | frick-fsw | M11: SRS-REQ-036 Renode GEM identification (ci:renode); SRS-REQ-047/048/049 add ci:renode |
| 0.22 | 2026-10-11 | frick-fsw | M11: SRS-REQ-039 FF_NetStack_t contract check (ci:renode; not peripheral-bound) |
| 0.23 | 2026-10-11 | frick-fsw | M16: SSS-REQ-027, SRS-REQ-054 (in-house OBCP engine); counts 55/27/14; complete the SSS summary (024-027) |
| 0.24 | 2026-10-11 | frick-fsw | M4/M11: SRS-REQ-055 reference eNVM NVM backend; counts 56/27/14 |
| 0.25 | 2026-10-11 | frick-fsw | M4/M11: SRS-REQ-056 reference NOR/QSPI MTD backend; counts 57/27/14 |
| 0.26 | 2026-10-11 | frick-fsw | SRS-REQ-056 evidence corrected to host-only (on-SoC QSPI check tracked M11) |
| 0.27 | 2026-10-11 | frick-fsw | M4/M11: SRS-REQ-057 reference MSS CAN driver; counts 58/27/14 |
