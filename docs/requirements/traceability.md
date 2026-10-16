# Traceability Matrices

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-TRACE-001                        |
| Revision                          | 0.47                                   |
| Date                              | 2026-10-16                             |
| Single source of truth            | `docs/requirements/traceability.yaml`  |
| Gate                              | `tools/scripts/trace.py` (norm N4)     |

This document is the human-readable presentation of the traceability of
frick-fsw. The authoritative machine-readable manifest is
`traceability.yaml`; this page mirrors its *current* content. Whenever the
manifest changes, `tools/scripts/trace.py` re-verifies convergence and this
document is regenerated/reviewed in the same commit (norms N8/N11).

Counts (current): SRS = 73, SSS = 30, GAP = 14, Lean theorems = 62, CI evidence
claims = 12 unique workflow claim names (11 referenced by the manifest; `ci-lean`
is reported in CI but not claimed by any requirement, norm N1). The planned
requirements remain documented-only with `ci:docs`: M12..M16 (SRS-REQ-045/046/
051/053/054 with SSS parents SSS-REQ-022/023/024/026/027). S4b requirements
059/060/061, fieldbus/PUSopen requirements 062..065, GEM TSN 066, fabric 067,
and gPTP 068 have implementation and host-test links to their delivered or
partial scope; target/emulator and vendor-SDK limits remain explicit. Residual
fabric CoreTSN work is deferred under SRS-REQ-068. SRS-REQ-052 is assigned to
the M15 fundamental PUS-C service set. Event
profile parameters in SSS-REQ-006/SRS-REQ-072 remain authority-owned TBDs;
passing trace convergence does not establish event-rate, latency or loss-report
compliance (NC-EXT-75/76).

## 1. Top-down matrix (SSS -> SRS)

| SSS | SRS children |
| --- | ------------ |
| SSS-REQ-001 (flight target) | SRS-REQ-017, SRS-REQ-037 |
| SSS-REQ-002 (software bus) | SRS-REQ-001, SRS-REQ-002 |
| SSS-REQ-003 (TM/TC chain) | SRS-REQ-003, SRS-REQ-004, SRS-REQ-009 |
| SSS-REQ-004 (request verification) | SRS-REQ-005, SRS-REQ-006, SRS-REQ-007 |
| SSS-REQ-005 (housekeeping) | SRS-REQ-008 |
| SSS-REQ-006 (bounded event reporting and overload observability) | SRS-REQ-011, SRS-REQ-072 |
| SSS-REQ-007 (time) | SRS-REQ-010 |
| SSS-REQ-008 (health & safety) | SRS-REQ-014 |
| SSS-REQ-009 (tables) | SRS-REQ-013, SRS-REQ-019 |
| SSS-REQ-010 (determinism) | SRS-REQ-015 |
| SSS-REQ-011 (multi-target build) | SRS-REQ-016 |
| SSS-REQ-012 (doc set) | FRICK-SRS-REQ-000 |
| SSS-REQ-013 (continuous verification) | SRS-REQ-020, SRS-REQ-043, SRS-REQ-044 |
| SSS-REQ-014 (link/transport) | SRS-REQ-012 |
| SSS-REQ-015 (PUS backends) | SRS-REQ-018, SRS-REQ-050 |
| SSS-REQ-016 (time base) | SRS-REQ-021, SRS-REQ-022, SRS-REQ-023, SRS-REQ-024 |
| SSS-REQ-017 (hardware abstraction) | SRS-REQ-025, SRS-REQ-026, SRS-REQ-027, SRS-REQ-028, SRS-REQ-029, SRS-REQ-030, SRS-REQ-031, SRS-REQ-032, SRS-REQ-067 |
| SSS-REQ-018 (numerical support) | SRS-REQ-033 |
| SSS-REQ-019 (data-link security) | SRS-REQ-034, SRS-REQ-035 |
| SSS-REQ-020 (reference drivers) | SRS-REQ-036, SRS-REQ-038, SRS-REQ-039, SRS-REQ-040, SRS-REQ-047, SRS-REQ-048, SRS-REQ-049, SRS-REQ-055, SRS-REQ-056, SRS-REQ-057, SRS-REQ-058, SRS-REQ-066, SRS-REQ-068, SRS-REQ-069, SRS-REQ-070 |
| SSS-REQ-021 (code quality) | SRS-REQ-041, SRS-REQ-042 |
| SSS-REQ-022 (on-board autonomy / OBCP) | SRS-REQ-045 |
| SSS-REQ-023 (reference mission services) | SRS-REQ-046 |
| SSS-REQ-024 (multicore / SMP) | SRS-REQ-051 |
| SSS-REQ-025 (fundamental PUS-C service set) | SRS-REQ-052 |
| SSS-REQ-026 (CFDP + virtual filestore) | SRS-REQ-053 |
| SSS-REQ-027 (in-house OBCP engine) | SRS-REQ-054 |
| SSS-REQ-028 (PUS-C over IP/TSN) | SRS-REQ-059, SRS-REQ-060, SRS-REQ-061 |
| SSS-REQ-029 (extended fieldbus + PUSopen) | SRS-REQ-062, SRS-REQ-063, SRS-REQ-064, SRS-REQ-065 |
| SSS-REQ-030 (application scheduling) | SRS-REQ-071 |

## 2. Requirement verification summary (SRS)

| Requirement | Implementation | Tests / CI | Lean | Note |
| ----------- | -------------- | ---------- | ---- | ---- |
| FRICK-SRS-REQ-000 | CMakeLists, presets, Makefile | ci:host-build | - | baseline |
| SRS-REQ-001 | sb.c/h, pal.h, pal_posix.c | test_sb.c | req_srs001_exact_and_wildcard, req_srs001_no_match_no_delivery | SB-lock busy asserts FF_TIMEOUT with no mutation; queue-lock busy asserts FF_TIMEOUT after partial fan-out and dropped accounting; full pipe remains a distinct overflow path |
| SRS-REQ-002 | sb.c, pal_posix.c | test_sb.c | - | FIFO by ring-buffer semantics |
| SRS-REQ-003 | ccsds.c | test_ccsds.c, test_r2_gap.c, test_decoder_corpus.c | req_srs003_tm_length, req_srs003_tc_length | Lean models encoder bounds only; decoder PDL checks (incl. length-boundary corpus) C-tested, not C-refined |
| SRS-REQ-004 | ccsds.c | test_ccsds.c, test_decoder_corpus.c | req_srs004_crc_appended_zero, req_srs004_crc_initial | CRC-match/reject on decode exercised by S8-VV-FU single-bit mutation corpus (host SIL), not C-refined |
| SRS-REQ-005 | pus.c | test_pus.c, test_r2_gap.c, test_r3_gap.c, test_decoder_corpus.c | - | Malformed-frame S1 requirement conflicts with test_pus_malformed_tc_event; NC-EXT-04 open. S8-VV-FU corpus asserts 'shall not be executed' (no handler for malformed/unknown-service) on host |
| SRS-REQ-006 | pus.c | test_pus.c | req_srs006_acceptance_completion, complete_ack_produces_two, ack_acceptance_is_counted | |
| SRS-REQ-007 | pus.c | test_pus.c | req_srs007_counter_increases, req_srs007_counter_wrap_bounded | |
| SRS-REQ-008 | pus_services.c, app_hk.c | test_pus.c | - | GAP-A2 formalisation |
| SRS-REQ-009 | pus_services.c | test_pus.c | - | region bound asserted |
| SRS-REQ-010 | time.c, pps.c, pus_services.c | test_pus.c, test_ccsds.c, test_time.c | req_srs024_cuc_field_roundtrip | CUC round-trip |
| SRS-REQ-011 | evs.c, sb.c, app_tm_output.c, fsw_main.c, pus.c | test_pus.c, test_sb.c, test_r2_gap.c, test_r3_gap.c, ci:simulator-smoke | - | Forwarding-rate/latency guarantee applies within approved SSS-REQ-006 envelope; loss accounting is independent of that envelope. Mission profile and post-acceptance loss-detection/counter semantics remain TBD (NC-EXT-76); see host structural characterization, not compliance evidence |
| SRS-REQ-012 | pus_services.c | test_pus.c | - | S17 ping/pong |
| SRS-REQ-013 | tbl.c | test_tbl.c | - | atomic activate |
| SRS-REQ-014 | es.c, app_hs.c | ci:simulator-smoke, test_r2_gap.c, test_es_hs_lifecycle.c | - | HS/ES host integration harness with EVS-listener oracle added in S8-VV-FU (detection-latency and external-watchdog cadence stay TBD) |
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
| SRS-REQ-038 | drivers/eth/gem/ff_gem_tsn.c/h | test_gem_tsn.c, ci:renode | - | GEM native TSN backend (M4/M11; peripheral-bound `hal_tsn` check) |
| SRS-REQ-039 | drivers/net/ff_net_loopback.c/h | test_netloop.c, ci:renode | - | Reference loopback network stack; `FF_NetStack_t` contract check in Renode (M4/M11) |
| SRS-REQ-040 | hal/net/lwip/ff_net_lwip.c/h + ff_lwip_netif.c/h + rtems/ (RTEMS sys_arch port) | test_lwip.c + test_lwip_netif.c | - | lwIP network-stack adapter + netif glue (M4; in-repo RTEMS port, S6b-1/S6b-2; host netif glue test, NET follow-up NC-154-01) |
| SRS-REQ-041 | docs/quality/misra-c.md, tools/scripts/misra.sh | ci:misra | - | MISRA C:2012 compliance (M9) |
| SRS-REQ-042 | .clang-format, docs/engineering/coding-style.md | ci:static-analysis | - | C coding style (M9) |
| SRS-REQ-043 | tools/scripts/emulated_run.sh, tools/scripts/qemu_vv.sh, cmake/toolchains/rtems-riscv-generic.cmake, docs/vv/emulated-target.md | ci:emulated-run | - | Emulated-target verification (M10, done: Renode + QEMU generic-BSP) |
| SRS-REQ-044 | tools/docker/renode/, tools/scripts/renode_vv.sh, src/apps/app_renode_test.c | ci:renode | - | Renode functional validation, N18 in force (M10) |
| SRS-REQ-045 | docs/development-plan.md, docs/roadmap.md, docs/architecture/component-gap-analysis.md | ci:docs | - | OBCP engine uPython/MicroPython (planned, M12) |
| SRS-REQ-046 | docs/development-plan.md, docs/roadmap.md, docs/architecture/component-gap-analysis.md | ci:docs | - | Reference mission services (planned, M13) |
| SRS-REQ-047 | drivers/gpio/mpfs/ff_gpio_mpfs.c/h, config/platform/platform_mpfs.h | test_gpio.c | - | MSS GPIO driver + Renode MMIO check (M4/M11) |
| SRS-REQ-048 | drivers/serial/mpfs/ff_serial_mpfs.c/h, config/platform/platform_mpfs.h | test_serial.c | - | MSS MMUART serial driver + Renode MMIO check (M4/M11) |
| SRS-REQ-049 | drivers/mem/ff_mem_static.c/h | test_mem.c | - | Static RAM pool + DMA helper + Renode DDR check (M4/M11) |
| SRS-REQ-050 | src/pus/adapters/pus_adapter_n7.c, src/pus/adapters/pus_adapter_open.c, tools/scripts/gen_pus_asn1.sh, tools/docker/asn1scc/Dockerfile | test_pus.c | req_srs050_open_backend_is_default, req_srs050_n7_is_opt_in | Open CCSDS backend default; optional N7/ESA ASN.1 backend (ADR-0018) |
| SRS-REQ-051 | docs/development-plan.md, docs/roadmap.md | ci:docs | - | Multicore / SMP (planned, M14) |
| SRS-REQ-052 | docs/architecture/pus-c-from-scratch-assessment.md, src/pus/pus.h | ci:docs | - | Fundamental PUS-C service set (planned, M15) |
| SRS-REQ-053 | docs/development-plan.md, docs/roadmap.md | ci:docs | - | CFDP + virtual filestore (planned, M15) |
| SRS-REQ-054 | docs/development-plan.md, docs/roadmap.md, docs/status/loop-audit.md | ci:docs | - | In-house OBCP engine (planned, M16) |
| SRS-REQ-055 | drivers/nvm/mpfs/ff_nvm_mpfs.c/h, config/platform/platform_mpfs.h | test_nvm.c, ci:renode | - | Reference eNVM NVM backend (M4/M11) |
| SRS-REQ-056 | drivers/mtd/mpfs/ff_mtd_mpfs.c/h | test_mtd.c | - | Reference NOR/QSPI MTD backend (M4/M11; on-SoC QSPI check pending the QSPI controller command driver) |
| SRS-REQ-057 | drivers/can/mpfs/ff_can_mpfs.c/h, config/platform/platform_mpfs.h | test_can.c, ci:renode | - | Reference MSS CAN backend (M4/M11; classic 2.0 A/B, CAN-FD rejected) |
| SRS-REQ-058 | drivers/i2c/mpfs/ff_i2c_mpfs.c/h, config/platform/platform_mpfs.h | test_i2c.c, ci:renode | - | Reference MSS I2C backend (M4/M11; polled CoreI2C master, serial-clock divider reference-derived) |
| SRS-REQ-059 | src/transport/link/link_seg.{c,h}, tests/unit/test_link_ip.c, docs/engineering/user-integration-manual.md, docs/requirements/SRS.md | ci:build, ci:test | - | Seg/reassembly: 2-byte BE length-prefix for TCP, allocation-free, fixed-point buffers (SRS-REQ-015); implemented in S4b |
| SRS-REQ-060 | src/transport/link/link_ip.{c,h}, tests/unit/test_link_ip.c, docs/engineering/user-integration-manual.md, docs/requirements/SRS.md | ci:build, ci:test | - | IP backend: UDP datagrams + TCP segmentation over `FF_NetStack_t`; PUS-C round-trip uses a mock `FF_NetStack_t`, while `FF_NetStackLoopback` is a separate socket-pair reference implementation; `test_pus_ip_roundtrip` is host demonstration only |
| SRS-REQ-061 | src/transport/link/link_tsn.{c,h}, tests/unit/test_link_tsn.c, docs/engineering/user-integration-manual.md, docs/requirements/SRS.md | ci:build, ci:test | - | TSN link backend: GEM MAC RX callback + ring buffer, Ethertype 0x88F5, over `FF_EthMac_t + FF_TsnBackend_t`; implemented in S4b; PUS-C TM/TC round-trip `test_pus_tsn_roundtrip` (Dem) |
| SRS-REQ-062 | src/drivers/can/fabric/ff_canfd_fabric.c, src/drivers/fabric/mpfs/ff_fabric_mpfs.c, src/hal/hal_fabric.h | tests/unit/test_canfd.c, tests/unit/test_fabric.c | req_srs062_dlc_max_payload | CAN-FD fabric-window backend host-verified; target peripheral absent from emulator (NC-S4c-01/GAP-A5) |
| SRS-REQ-063 | src/drivers/can/canopen/ff_canopen.c/h, src/hal/hal_can.h | tests/unit/test_canopen.c | req_srs063_operational_command, req_srs063_stopped_command, req_srs063_preop_command, req_srs063_unknown_command_rejected, req_srs063_heartbeat_disabled, req_srs063_heartbeat_due_at_period | Bounded allocation-free CANopen subset; reset NMT commands rejected; API/callback calls require a serialized owner context; not full CiA 301/302/402 conformance |
| SRS-REQ-064 | src/drivers/serial/mpfs/ff_serial_mpfs.c/h, src/hal/hal_serial.h | tests/unit/test_serial.c | req_srs064_drain_poll_count_bounded | RS-485 DE/RE bounded drain, post-drain hold, and visible `FF_TIMEOUT`; board/electrical policy and behavior GAP-A5 |
| SRS-REQ-065 | src/pus/adapters/pus_adapter_pusopen.c, docs/architecture/pus-layer.md | tests/unit/test_pusopen.c, tests/stubs/pusopen.h | - | Adapter seam uses controlled API test double; vendor SDK compatibility not established |
| SRS-REQ-066 | src/drivers/eth/gem/ff_gem_tsn.c, tests/unit/test_gem_tsn.c | test_gem_tsn.c | - | GEM TSN 802.1Qci implemented; Qav/Qbb return `FF_NOT_IMPLEMENTED`, residuals NC-S8-01/02 |
| SRS-REQ-067 | src/hal/hal_fabric.h, src/drivers/fabric/mpfs/ff_fabric_mpfs.c | tests/unit/test_fabric.c, tests/unit/test_hal.c | req_srs067_fabric_range_rejects_overrun | Generic fabric-window contract host-verified; no FPGA peripheral included/emulated |
| SRS-REQ-068 | src/drivers/eth/gptp/ff_gptp.c/h, src/hal/hal_tsn.h | tests/unit/test_gptp.c | - | gPTP stack implemented/host-tested; fabric CoreTSN/802.1CB remains deferred to M14 |
| SRS-REQ-069 | drivers/spi/mpfs/ff_spi_mpfs.c/h, config/platform/platform_mpfs.h | test_spi.c, ci:renode | - | Reference MSS SPI backend (M4/M11; polled CoreSPI master, clock divider reference-derived) |
| SRS-REQ-070 | drivers/block/mpfs/ff_block_mpfs.c/h, config/platform/platform_mpfs.h | test_block.c, ci:renode | - | Reference MSS block-device backend (M4/M11; polled SDHCI-style MMC/SD, buffer mode) |
| SRS-REQ-071 | core/sched/ff_sched.{c,h}, apps/app_scheduler.c, core/tbl/tbl.c | test_sched.c, ci:host-build | - | Bounded, allocation-free table-driven dispatch (S8-SCH first increment): image validation, atomic double-buffer activation, per-entry absolute release targets (no shared-timestamp starvation, NC-EXT-19), bounded per-poll work with missed-slot/heartbeat/dispatch-failure evidence; host-verified with an injected clock. Mission rates, jitter limits, late-slot policy and recovery authority remain TBD, so no schedulability, jitter, WCET or target-evidence claim is made |
| SRS-REQ-072 | evs.c, sb.c, app_tm_output.c, fsw_main.c, `docs/architecture/sb-evs-characterization.md` | test_sb.c, test_sb_stress.c, test_sb_sweep.c, `tools/scripts/sb_evs_structural_sweep.py`, test_r2_gap.c, ci:simulator-smoke | - | Dispatch-rate/latency guarantee applies only within approved arrival/link performance envelopes; exactly-once loss accounting applies independently. Candidate tests and the 9,984-profile host sweep characterize configured local structure only; mission envelope, ground boundary, post-acceptance loss detection/counter/reset semantics and overload-loss reporting remain TBD (NC-EXT-75/76) |

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
| SSS-REQ-006 | evs.c, sb.c, app_tm_output.c, fsw_main.c, pus.c, `docs/architecture/sb-evs-characterization.md` | test_sb.c, test_pus.c, test_sb_stress.c, test_sb_sweep.c, `tools/scripts/sb_evs_structural_sweep.py`, test_r2_gap.c, test_r3_gap.c, ci:simulator-smoke | - | Envelope performance values, ground-receipt boundary, and post-acceptance loss-detection/counter/reporting semantics are authority-owned TBDs; host structural sweep is not requirement compliance evidence |
| SSS-REQ-007 | time.c, pps.c, pus_services.c | test_ccsds.c, test_time.c | req_srs024_cuc_field_roundtrip |
| SSS-REQ-008 | app_hs.c, es.c | ci:simulator-smoke, test_es_hs_lifecycle.c | - | HS host integration harness with EVS-listener oracle added in S8-VV-FU (watchdog window stays TBD) |
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
| SSS-REQ-028 | docs/development-plan.md, docs/roadmap.md, docs/planning/sprint-plan.md | ci:docs | - |
| SSS-REQ-029 | src/drivers/can/fabric/ff_canfd_fabric.c, src/drivers/can/canopen/ff_canopen.c, src/drivers/serial/mpfs/ff_serial_mpfs.c, src/pus/adapters/pus_adapter_pusopen.c, docs/planning/sprint-plan.md | tests/unit/test_canfd.c, tests/unit/test_canopen.c, tests/unit/test_serial.c, tests/unit/test_pusopen.c | - |
| SSS-REQ-030 | src/core/sched/ff_sched.c, src/core/sched/ff_sched.h, src/apps/app_scheduler.c, docs/architecture/scheduling.md | tests/unit/test_sched.c, ci:host-build | - |

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
| 0.28 | 2026-10-12 | frick-fsw | M10: SRS-REQ-043 mirror row adds `qemu_vv.sh` + the generic toolchain file (done) |
| 0.29 | 2026-10-12 | frick-fsw | M4/M11: SRS-REQ-058 reference MSS I2C driver (+Renode MMIO check); counts 59/27/14 |
| 0.30 | 2026-10-12 | frick-fsw | Restore explicit M4 scope: SSS-REQ-028/029, SRS-REQ-059..068 (PUS-C over IP/TSN, CAN-FD, CANopen, RS-485 DE, PUSopen adapter, GEM TSN Qav/Qci/Qbb, fabric contract, gPTP/CoreTSN); counts 69/29/14 — FRICK-REVIEW-0137 |
| 0.31 | 2026-10-13 | frick-fsw | **S4b delivered**: SRS-REQ-059/060/061 implemented (seg/reassembly, IP link backend, TSN link backend); traceability table rows updated from plan/ci:docs to source+test files with ci:build+ci:test evidence; 114/71/29/14/51; trace PASS |
| 0.31 | 2026-10-12 | frick-fsw | M4/M11: SRS-REQ-069 reference MSS SPI driver (+Renode MMIO check); counts 70/29/14 |
| 0.32 | 2026-10-12 | frick-fsw | M4/M11: SRS-REQ-070 reference MSS block-device driver (+Renode MMIO check); counts 71/29/14 |
| 0.33 | 2026-10-12 | frick-fsw | M11/S6a: SRS-REQ-038 GEM TSN mirror row gains `ci:renode` (peripheral-bound `hal_tsn` check); counts unchanged 71/29/14 |
| 0.34 | 2026-10-12 | frick-fsw | M11/S6b-1: SRS-REQ-040 mirror row records the in-repo lwIP RTEMS `sys_arch` port (FF_USE_LWIP builds for the target); counts unchanged 71/29/14 |
| 0.35 | 2026-10-12 | frick-fsw | M11/S6b-2: SRS-REQ-040 mirror row also records the netif glue (`ff_lwip_netif.c/h`); counts unchanged 71/29/14 |
| 0.36 | 2026-10-12 | frick-fsw | NET follow-up: SRS-REQ-040 mirror row records `test_lwip_netif.c` (NC-154-01 closed); counts unchanged 71/29/14 |
| 0.37 | 2026-10-13 | frick-fsw | S4b delta re-review (N12/N20 closure): all findings from initial R1+R2 (FRICK-REVIEW-0165) and PA (FRICK-REVIEW-0166) reviews resolved; 3 new findings (NF-1/NF-2/NF-3) raised and closed; delta re-reviews APPROVED; `-Wall -Wextra -Werror` clean on authored files; max-size regression test added; test counts updated (test_link_ip 19 funcs, test_link_tsn 14 funcs) |
| 0.38 | 2026-10-13 | frick-fsw | S4b follow-up (NC-S4b-02/03/04): host PUS-C TM/TC round-trip demonstration added for the IP backend (`test_pus_ip_roundtrip`) and the TSN backend (`test_pus_tsn_roundtrip`) — satisfies the `pus_ip`/`pus_tsn` S4b exit criterion and the `Dem` method of SRS-REQ-060/061; SRS-REQ-059 mirror row corrected (removed `test_link_tsn.c`, which tests SRS-REQ-061); counts unchanged 114/71/29/14/51 |
| 0.39 | 2026-10-13 | frick-fsw | S4b follow-up 2nd pass: update the SRS-REQ-003 mirror to include decoder PDL-boundary checks in `test_r2_gap.c` and explicitly delimit the existing Lean proof to encoder bounds; counts unchanged 114/71/29/14/51 |
| 0.40 | 2026-10-13 | frick-fsw | Assessment follow-up: disclose SRS-REQ-005/test-oracle conflict under NC-EXT-04; correct SRS-REQ-060 mirror to distinguish mock `FF_NetStack_t` round-trip from the socket-pair reference loopback; expand SRS-REQ-003 tests for TM malformed bounds and GSW stream offsets; counts unchanged 114/71/29/14/51 |
| 0.41 | 2026-10-14 | frick-fsw | Add SRS-REQ-072 under SSS-REQ-006; record bounded SB mutex return and authority-owned event-rate/dispatch/latency inputs; trace counts pending convergence |
| 0.42 | 2026-10-14 | frick-fsw | Reconcile current human-readable trace counts to 117 requirements (73 SRS, 30 SSS, 14 GAP) and state that event-envelope parameters remain TBD |
| 0.43 | 2026-10-14 | frick-fsw | Update SRS-REQ-001 implementation/test summary for non-waiting PAL queue lock, partial fan-out and distinct full-queue status; trace gate remains 117/73/30/14/51 |
| 0.44 | 2026-10-14 | frick-fsw | Trace the exploratory SB/EVS structural characterization and distinguish it from requirement compliance; clarify mission loss-accounting semantics remain TBD |
| 0.45 | 2026-10-14 | frick-fsw | Resolve bounded R3 findings by conditioning SRS-REQ-072 rate/latency guarantees on the approved envelopes and reconciling SSS-REQ-006/SRS-REQ-011/072 manifest and readable trace paths/tests; mission inputs remain TBD |
| 0.46 | 2026-10-15 | frick-fsw | Update trace matrix for S4c/S4d source/test/Lean evidence and ADR-0018 PUS/lwIP defaults; retain explicit target-peripheral and vendor-SDK limits |
| 0.47 | 2026-10-16 | frick-fsw | Sync the readable mirror to the manifest: add the SRS-REQ-071 / SSS-REQ-030 delivered rows (core/sched/ff_sched + test_sched) and restore the missing SSS-REQ-030 rows in sections 1 and 3 (29 -> 30 SSS rows, mirroring the yaml); mission rate/jitter/recovery inputs remain TBD |
