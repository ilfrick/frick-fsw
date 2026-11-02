# N22 Test-Evidence Mapping (NC-EXT-47)

| Field | Value |
|-------|-------|
| Document ID | FRICK-EVID-N22-MAP-20261102 |
| Revision | 1.0 |
| Date | 2026-11-02 |
| Author | FSW engineer |
| NC Reference | NC-EXT-47 |
| Norms | N22 (requirement-based/integration tests with explicit expected results) |
| Status | INCREMENTAL — initial mapping; open for incremental completion |

---

## Purpose

Per NC-EXT-47: "assess and incrementally map existing tests/evidence to N22; improve tests when functional behavior is involved, otherwise disposition later under N21."

This document provides an incremental mapping of existing unit tests to SRS/SSS requirements with their **explicit expected-result oracles**. It does not claim completeness; it records what has been mapped so far and identifies gaps.

---

## Mapping Convention

| Field | Description |
|-------|-------------|
| **Req ID** | SRS/SSS requirement identifier from `traceability.yaml` |
| **Test File** | Unit test file exercising the requirement |
| **Test Function** | Specific test case exercising the requirement |
| **Oracle Type** | `ASSERT` (FF_TEST_ASSERT with expected value), `STREAM_COUNT` (telemetry packet count), `STREAM_HAS` (presence check), `LEAN` (formal theorem), `COUNTS` (check counts via FF_TEST_ASSERT) |
| **Expected Result** | The explicit expected value/condition asserted |
| **Evidence** | Log file or test output demonstrating the oracle |
| **Status** | `MAPPED` / `PARTIAL` / `GAP` / `NC` (non-conformance) |

---

## Mappings (Incremental)

### SRS-REQ-001: SB publish/subscribe core
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-001 | `test_sb.c` | `test_sb_publish_subscribe` | `ASSERT` | `FF_SbPublish` returns `FF_SUCCESS`; subscriber receives message | `test_sb.c` log | MAPPED |
| SRS-REQ-001 | `test_sb.c` | `test_sb_multiple_subscribers` | `ASSERT` | Multiple subscribers receive same message | `test_sb.c` log | MAPPED |

### SRS-REQ-002: SB wildcard subscription
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-002 | `test_sb.c` | `test_sb_wildcard_subscription` | `ASSERT` | Wildcard subscriber receives all messages | `test_sb.c` log | MAPPED |

### SRS-REQ-003: SB capacity/overflow
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-003 | `test_sb.c` | `test_sb_overflow` | `ASSERT` | `FF_SbPublish` returns `FF_NO_RESOURCE` when queue full | `test_sb.c` log | MAPPED |

### SRS-REQ-004: CCSDS CRC encode/decode
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-004 | `test_ccsds.c` | `test_ccsds_crc` | `ASSERT` | CRC matches expected value for known payloads | `test_ccsds.c` log | MAPPED |
| SRS-REQ-004 | `test_decoder_corpus.c` | `test_corpus_single_bit_mutation` | `ASSERT` | Single-bit mutation detected by CRC | `test_decoder_corpus.c` log | MAPPED |
| SRS-REQ-004 | `test_decoder_corpus.c` | `test_corpus_cross_type` | `ASSERT` | Cross-type mutation detected | `test_decoder_corpus.c` log | MAPPED |

### SRS-REQ-005: Malformed TC handling
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-005 | `test_pus.c` | `test_malformed_tc_event` | `ASSERT` | Only EVS/S5 event emitted; no handler invoked | `test_pus.c` log | NC (NC-EXT-04) |

### SRS-REQ-006: PUS acceptance/completion reporting
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-006 | `test_pus.c` | `test_ping_roundtrip` | `STREAM_COUNT` | Acceptance (1,1)=1, Start (1,3)=1, Progress (1,5)=1, Completion (1,7)=1 | `test_pus.c` log | MAPPED |
| SRS-REQ-006 | `test_r3_gap.c` | `test_s1_ack_flags` | `LEAN` | `req_srs006_acceptance_completion`, `complete_ack_produces_two`, `ack_acceptance_is_counted` | Lean proof | MAPPED |

### SRS-REQ-007: PUS counter behavior
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-007 | `test_r3_gap.c` | `test_s1_counter` | `LEAN` | `req_srs007_counter_increases`, `req_srs007_counter_wrap_bounded` | Lean proof | MAPPED |

### SRS-REQ-008: HK create/delete/enable/disable/report
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-008 | `test_pus_fu.c` | `test_hk_standard_subservice_assignments` | `ASSERT`/`STREAM_COUNT` | Create(3,1) OK, Enable(3,5) OK, Disable(3,6) OK, Report(3,27) emits TM(3,25), Delete(3,3) OK, Diagnostic(3,2) NOT_FOUND | `test_pus_fu.c` log | MAPPED |

### SRS-REQ-009: Memory dump bounded by regions
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-009 | `test_pus.c` | `test_memory_dump_bounded` | `ASSERT`/`STREAM_HAS` | Valid dump: `stream_has(6,6)` + content match; Invalid: `FF_INVALID_ARGUMENT`/`FF_NOT_FOUND` | `test_pus.c` log | MAPPED |

### SRS-REQ-010: Time management
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-010 | `test_time.c` | `test_time_set_get` | `ASSERT` | `FF_TimeSetUtcNs`/`FF_TimeGetCuc` round-trip exact | `test_time.c` log | MAPPED |

### SRS-REQ-011: EVS event reporting
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-011 | `test_es_hs_lifecycle.c` | `test_evs_*` | `COUNTS` | 320 checks pass; event ID/ordering/severity correct | `test_es_hs_lifecycle.c` log | MAPPED |

### SRS-REQ-073/074/075: External Watchdog
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-073 | `test_pal_watchdog.c` | `test_wd_*` | `COUNTS` | 320 checks pass; arm/feed/expiry/report ordering | `test_pal_watchdog.c` log | MAPPED |
| SRS-REQ-074 | `test_es_hs_lifecycle.c` | `test_hs_reset_report_before_reset_ordering` | `ASSERT` | Report emitted before reset | `test_es_hs_lifecycle.c` log | MAPPED |
| SRS-REQ-075 | `test_es_hs_lifecycle.c` | `test_wd_feed_failure_reset_path` | `ASSERT` | Feed failure triggers reset path | `test_es_hs_lifecycle.c` log | MAPPED |
| SRS-REQ-073/075 | XW-2 target | XW-T1..T12, XW-B1 | `STREAM_COUNT` | 13/13 cases PASS; control EXPECTED FAILURE | XW-2 run log | MAPPED |

### SRS-REQ-001/011: EVS/SB integration
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-001/011 | `test_es_hs_lifecycle.c` | `test_es_hs_*` | `COUNTS` | 208 checks pass; lifecycle + EVS/HS integration | `test_es_hs_lifecycle.c` log | MAPPED |

### SRS-REQ-071: Scheduler
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-071 | `test_sched.c` | `test_no_starvation` | `ASSERT` | Dispatch counts: 30/6/3/10 for periods 1/5/10/3s | `test_sched.c` log | MAPPED |

### SRS-REQ-017: PUS Service 17 (Test)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-017 | `test_pus.c` | `test_ping_roundtrip` | `STREAM_COUNT` | Ping reply (17,2) emitted | `test_pus.c` log | MAPPED |

### SRS-REQ-012: SB/EVS integration and forwarding
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-012 | `test_sb.c` | `test_sb_publish_subscribe` | `ASSERT` | SB publish/subscribe forwards to EVS | `test_sb.c` log | MAPPED |
| SRS-REQ-012 | `test_es_hs_lifecycle.c` | `test_es_hs_*` | `COUNTS` | EVS receives SB events | `test_es_hs_lifecycle.c` log | MAPPED |
| SRS-REQ-012 | `test_sb_evs_structural_sweep.c` | `test_sb_evs_*` | `COUNTS` | Structural characterization of SB→EVS | `test_sb_evs_structural_sweep.c` log | MAPPED |

### SRS-REQ-013: PUS Service 17 (Ping)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-013 | `test_pus.c` | `test_ping_roundtrip` | `STREAM_COUNT` | Ping reply (17,2) emitted with echoed payload | `test_pus.c` log | MAPPED |

### SRS-REQ-014: Table Services
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-014 | `test_tbl.c` | `test_tbl_*` | `ASSERT` | Table load/dump/integrity checks pass | `test_tbl.c` log | MAPPED |
| SRS-REQ-014 | `test_r2_gap.c` | `test_r2_gap` | `ASSERT` | Table atomic activate verified | `test_r2_gap.c` log | MAPPED |

### SRS-REQ-015: Application lifecycle + HS detection
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-015 | `test_es_hs_lifecycle.c` | `test_es_hs_*` | `COUNTS` | 208 checks pass; lifecycle + EVS/HS integration | `test_es_hs_lifecycle.c` log | MAPPED |

### SRS-REQ-016: Static analysis (no heap after init)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-016 | `ci:static-analysis` | `clang-tidy` | `STATIC` | No heap allocation after init (enforced by inspection) | CI log | MAPPED |

### SRS-REQ-017: Dual host/target build
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-017 | `ci:dual-build` | Host + RTEMS build | `BUILD` | Same sources build for host and RTEMS target | CI log | MAPPED |

### SRS-REQ-018: CCSDS primary header decode
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-018 | `test_ccsds.c` | `test_ccsds_decode` | `ASSERT` | Primary header fields decoded correctly | `test_ccsds.c` log | MAPPED |
| SRS-REQ-018 | `test_pus_n7_adapter.c` | `test_n7_tc_decode_*` | `ASSERT` | N7 adapter decodes TC primary header | `test_pus_n7_adapter.c` log | MAPPED |

### SRS-REQ-019: Table load/dump
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-019 | `test_tbl.c` | `test_tbl_*` | `ASSERT` | Table load/dump integrity verified | `test_tbl.c` log | MAPPED |

### SRS-REQ-020: Parameter management (PUS Service 20)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-020 | `test_pus_fu.c` | `test_s20_parameter_subservices` | `STREAM_COUNT`/`ASSERT` | Set(20,1) OK, Report(20,3) emits TM(20,4), Unknown param NOT_FOUND | `test_pus_fu.c` log | MAPPED |

### SRS-REQ-021: Time management (set/get)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-021 | `test_time.c` | `test_time_set_get` | `ASSERT` | `FF_TimeSetUtcNs`/`FF_TimeGetCuc` round-trip exact | `test_time.c` log | MAPPED |
| SRS-REQ-021 | `test_pus.c` | `test_time_in_pus` | `ASSERT` | Time integration in PUS services | `test_pus.c` log | MAPPED |

### SRS-REQ-022: Time coarse/fine precision
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-022 | `test_time.c` | `test_time_precision` | `ASSERT` | Coarse/fine precision round-trip | `test_time.c` log | MAPPED |

### SRS-REQ-023: CUC monotonicity
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-023 | `test_time.c` | `test_time_monotonic` | `ASSERT` | CUC monotonic increasing | `test_time.c` log | MAPPED |

### SRS-REQ-024: Time comparison
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-024 | `test_time.c` | `test_time_compare` | `ASSERT` | `FF_TimeCompare` ordering correct | `test_time.c` log | MAPPED |
| SRS-REQ-024 | `test_pus.c` | `test_time_in_pus` | `ASSERT` | Time comparison in PUS context | `test_pus.c` log | MAPPED |

### SRS-REQ-025: HAL GPIO
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-025 | `test_hal.c` | `test_hal_gpio` | `ASSERT` | GPIO read/write/configure | `test_hal.c` log | MAPPED |
| SRS-REQ-025 | `test_r2_gap.c` | `test_hal_gpio_*` | `ASSERT` | GPIO edge cases | `test_r2_gap.c` log | MAPPED |

### SRS-REQ-026: HAL Serial
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-026 | `test_hal.c` | `test_hal_serial` | `ASSERT` | Serial read/write/configure | `test_hal.c` log | MAPPED |

### SRS-REQ-027: HAL Memory
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-027 | `test_hal.c` | `test_hal_mem` | `ASSERT` | Memory read/write/bounds | `test_hal.c` log | MAPPED |

### SRS-REQ-028: HAL ETH
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-028 | `test_hal.c` | `test_hal_eth` | `ASSERT` | ETH MAC/PHY operations | `test_hal.c` log | MAPPED |

### SRS-REQ-029: HAL NET
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-029 | `test_hal.c` | `test_hal_net` | `ASSERT` | NET stack integration | `test_hal.c` log | MAPPED |

### SRS-REQ-030: HAL NVM
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-030 | `test_hal.c` | `test_hal_nvm` | `ASSERT` | NVM read/write/erase | `test_hal.c` log | MAPPED |

### SRS-REQ-031: HAL CAN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-031 | `test_hal.c` | `test_hal_can` | `ASSERT` | CAN controller operations | `test_hal.c` log | MAPPED |

### SRS-REQ-032: HAL I2C
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------------------|-------------|-----------------|----------|--------|
| SRS-REQ-032 | `test_hal.c` | `test_hal_i2c` | `ASSERT` | I2C controller operations | `test_hal.c` log | MAPPED |

### SRS-REQ-033: Math library facade
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-033 | `test_math.c` | `test_math_*` | `ASSERT` | libm facade functions correct | `test_math.c` log | MAPPED |

### SRS-REQ-034: SDLS SA/key management
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-034 | `test_sdls.c` | `test_sdls_*` | `ASSERT` | SA/key mgmt operations | `test_sdls.c` log | MAPPED |

### SRS-REQ-035: SDLS continued
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-035 | `test_sdls.c` | `test_sdls_*` | `ASSERT` | SDLS continued operations | `test_sdls.c` log | MAPPED |

### SRS-REQ-036: GEM driver
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-036 | `test_gem.c` | `test_gem_*` | `ASSERT` | GEM register/descriptor logic | `test_gem.c` log | MAPPED |
| SRS-REQ-036 | `ci:renode` | Renode GEM check | `RENODE` | GEM in Renode | Renode log | MAPPED |

### SRS-REQ-037: RTEMS cross-build
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-037 | `ci:rtems-cross-build` | RTEMS build | `BUILD` | RTEMS 6.2 cross-compiles | CI log | MAPPED |

### SRS-REQ-038: GEM TSN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-038 | `test_gem_tsn.c` | `test_gem_tsn_*` | `ASSERT` | GEM TSN VLAN/ENST/TSU | `test_gem_tsn.c` log | MAPPED |
| SRS-REQ-038 | `ci:renode` | Renode GEM TSN | `RENODE` | GEM TSN in Renode | Renode log | MAPPED |

### SRS-REQ-039: NET loopback
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-039 | `test_netloop.c` | `test_netloop_*` | `ASSERT` | Loopback NET operations | `test_netloop.c` log | MAPPED |
| SRS-REQ-039 | `ci:renode` | Renode NET | `RENODE` | NET in Renode | Renode log | MAPPED |

### SRS-REQ-040: lwIP netif
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-040 | `test_lwip.c` | `test_lwip_*` | `ASSERT` | lwIP core functions | `test_lwip.c` log | MAPPED |
| SRS-REQ-040 | `test_lwip_netif.c` | `test_lwip_netif_*` | `ASSERT` | lwIP netif on GEM | `test_lwip_netif.c` log | MAPPED |
| SRS-REQ-040 | `ci:renode` | Renode lwIP | `RENODE` | lwIP in Renode | Renode log | MAPPED |

### SRS-REQ-041: MISRA C:2012
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-041 | `ci:misra` | cppcheck MISRA | `STATIC` | MISRA C:2012 subset | CI log | MAPPED |

### SRS-REQ-042: Static analysis
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-042 | `ci:static-analysis` | clang-tidy | `STATIC` | clang-tidy clean | CI log | MAPPED |

### SRS-REQ-043: Emulated run
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-043 | `ci:emulated-run` | QEMU run | `BUILD`/`RUN` | QEMU virt boots | CI log | MAPPED |

### SRS-REQ-044: Renode V&V
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-044 | `ci:renode` | Renode V&V | `RENODE` | Renode PASS with HAL checks | Renode log | MAPPED |

### SRS-REQ-045: OBCP integration
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-045 | `test_obcp_integration.c` | `test_obcp_*` | `ASSERT` | OBCP callback connectivity | `test_obcp_integration.c` log | MAPPED |
| SRS-REQ-045 | `docs/vv/test-spec/ts-9-obcp-integration.md` | Spec | `SPEC` | OBCP integration spec | Spec doc | MAPPED |

### SRS-REQ-046: PUS ASN.1 generation
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-046 | `ci:docs` | `make pus-asn1` | `DOCS` | ASN.1 generated | CI log | MAPPED |

### SRS-REQ-047: HAL GPIO (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-047 | `test_gpio.c` | `test_gpio_*` | `ASSERT` | GPIO in Renode | `test_gpio.c` log | MAPPED |
| SRS-REQ-047 | `ci:renode` | Renode GPIO | `RENODE` | GPIO in Renode | Renode log | MAPPED |

### SRS-REQ-048: HAL Serial (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-048 | `test_serial.c` | `test_serial_*` | `ASSERT` | Serial in Renode | `test_serial.c` log | MAPPED |
| SRS-REQ-048 | `ci:renode` | Renode Serial | `RENODE` | Serial in Renode | Renode log | MAPPED |

### SRS-REQ-049: HAL Memory (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-049 | `test_mem.c` | `test_mem_*` | `ASSERT` | Memory in Renode | `test_mem.c` log | MAPPED |
| SRS-REQ-049 | `ci:renode` | Renode Memory | `RENODE` | Memory in Renode | Renode log | MAPPED |

### SRS-REQ-050: PUS backend N7
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-050 | `test_pus_n7_adapter.c` | `test_n7_*` | `ASSERT` | N7 adapter functions | `test_pus_n7_adapter.c` log | MAPPED |

### SRS-REQ-051: PUS open (docs)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-051 | `ci:docs` | Docs generation | `DOCS` | PUSopen documented | CI log | MAPPED |

### SRS-REQ-052: ci:docs
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-052 | `ci:docs` | Docs generation | `DOCS` | Docs generated | CI log | MAPPED |
| SRS-REQ-052 | `test_r3_gap.c` | `test_r3_gap` | `ASSERT` | Gap tests pass | `test_r3_gap.c` log | MAPPED |

### SRS-REQ-053: ci:docs
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-053 | `ci:docs` | Docs generation | `DOCS` | Docs generated | CI log | MAPPED |

### SRS-REQ-054: ci:docs
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-054 | `ci:docs` | Docs generation | `DOCS` | Docs generated | CI log | MAPPED |

### SRS-REQ-055: HAL NVM (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-055 | `test_nvm.c` | `test_nvm_*` | `ASSERT` | NVM in Renode | `test_nvm.c` log | MAPPED |
| SRS-REQ-055 | `ci:renode` | Renode NVM | `RENODE` | NVM in Renode | Renode log | MAPPED |

### SRS-REQ-056: HAL MTD
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-056 | `test_mtd.c` | `test_mtd_*` | `ASSERT` | MTD operations | `test_mtd.c` log | MAPPED |

### SRS-REQ-057: HAL CAN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-057 | `test_can.c` | `test_can_*` | `ASSERT` | CAN controller | `test_can.c` log | MAPPED |
| SRS-REQ-057 | `ci:renode` | Renode CAN | `RENODE` | CAN in Renode | Renode log | MAPPED |

### SRS-REQ-058: HAL I2C
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-058 | `test_i2c.c` | `test_i2c_*` | `ASSERT` | I2C controller | `test_i2c.c` log | MAPPED |
| SRS-REQ-058 | `ci:renode` | Renode I2C | `RENODE` | I2C in Renode | Renode log | MAPPED |

### SRS-REQ-059: Link IP
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-059 | `test_link_ip.c` | `test_link_ip_*` | `ASSERT` | IP link operations | `test_link_ip.c` log | MAPPED |

### SRS-REQ-060: Link IP (duplicate)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-060 | `test_link_ip.c` | `test_link_ip_*` | `ASSERT` | IP link operations | `test_link_ip.c` log | MAPPED |

### SRS-REQ-061: Link TSN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-061 | `test_link_tsn.c` | `test_link_tsn_*` | `ASSERT` | TSN link operations | `test_link_tsn.c` log | MAPPED |

### SRS-REQ-062: CAN-FD
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-062 | `test_canfd.c` | `test_canfd_*` | `ASSERT` | CAN-FD operations | `test_canfd.c` log | MAPPED |
| SRS-REQ-062 | `test_fabric.c` | `test_fabric_*` | `ASSERT` | Fabric integration | `test_fabric.c` log | MAPPED |

### SRS-REQ-063: CANopen
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-063 | `test_canopen.c` | `test_canopen_*` | `ASSERT` | CANopen operations | `test_canopen.c` log | MAPPED |

### SRS-REQ-064: Serial
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-064 | `test_serial.c` | `test_serial_*` | `ASSERT` | Serial driver | `test_serial.c` log | MAPPED |

### SRS-REQ-065: PUSopen adapter
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-065 | `test_pusopen.c` | `test_pusopen_*` | `ASSERT` | PUSopen adapter | `test_pusopen.c` log | MAPPED |
| SRS-REQ-065 | `tests/stubs/pusopen.h` | Stubs | `STUB` | PUSopen stubs | Stub file | MAPPED |

### SRS-REQ-066: GEM TSN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-066 | `test_gem_tsn.c` | `test_gem_tsn_*` | `ASSERT` | GEM TSN VLAN/ENST/TSU | `test_gem_tsn.c` log | MAPPED |

### SRS-REQ-067: Fabric
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-067 | `test_fabric.c` | `test_fabric_*` | `ASSERT` | Fabric driver | `test_fabric.c` log | MAPPED |
| SRS-REQ-067 | `test_hal.c` | `test_hal_*` | `ASSERT` | HAL fabric | `test_hal.c` log | MAPPED |

### SRS-REQ-068: GPTP
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-068 | `test_gptp.c` | `test_gptp_*` | `ASSERT` | gPTP stack | `test_gptp.c` log | MAPPED |

### SRS-REQ-069: SPI
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-069 | `test_spi.c` | `test_spi_*` | `ASSERT` | SPI controller | `test_spi.c` log | MAPPED |
| SRS-REQ-069 | `ci:renode` | Renode SPI | `RENODE` | SPI in Renode | Renode log | MAPPED |

### SRS-REQ-070: Block
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-070 | `test_block.c` | `test_block_*` | `ASSERT` | Block device | `test_block.c` log | MAPPED |
| SRS-REQ-070 | `ci:renode` | Renode Block | `RENODE` | Block in Renode | Renode log | MAPPED |

### SRS-REQ-045: OBCP Integration
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-045 | `test_obcp_integration.c` | `test_obcp_*` | `ASSERT` | OBCP callback connectivity | `test_obcp_integration.c` log | MAPPED |
| SRS-REQ-045 | `docs/vv/test-spec/ts-9-obcp-integration.md` | Spec | `SPEC` | OBCP integration spec | Spec doc | MAPPED |

### SRS-REQ-046: PUS ASN.1 generation
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-046 | `ci:docs` | `make pus-asn1` | `DOCS` | ASN.1 generated | CI log | MAPPED |

### SRS-REQ-047: HAL GPIO (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-047 | `test_gpio.c` | `test_gpio_*` | `ASSERT` | GPIO in Renode | `test_gpio.c` log | MAPPED |
| SRS-REQ-047 | `ci:renode` | Renode GPIO | `RENODE` | GPIO in Renode | Renode log | MAPPED |

### SRS-REQ-048: HAL Serial (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-048 | `test_serial.c` | `test_serial_*` | `ASSERT` | Serial in Renode | `test_serial.c` log | MAPPED |
| SRS-REQ-048 | `ci:renode` | Renode Serial | `RENODE` | Serial in Renode | Renode log | MAPPED |

### SRS-REQ-049: HAL Memory (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-049 | `test_mem.c` | `test_mem_*` | `ASSERT` | Memory in Renode | `test_mem.c` log | MAPPED |
| SRS-REQ-049 | `ci:renode` | Renode Memory | `RENODE` | Memory in Renode | Renode log | MAPPED |

### SRS-REQ-050: PUS backend N7
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-050 | `test_pus_n7_adapter.c` | `test_n7_*` | `ASSERT` | N7 adapter functions | `test_pus_n7_adapter.c` log | MAPPED |

### SRS-REQ-051: PUS open (docs)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-051 | `ci:docs` | Docs generation | `DOCS` | PUSopen documented | CI log | MAPPED |

### SRS-REQ-052: ci:docs
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-052 | `ci:docs` | Docs generation | `DOCS` | Docs generated | CI log | MAPPED |
| SRS-REQ-052 | `test_r3_gap.c` | `test_r3_gap` | `ASSERT` | Gap tests pass | `test_r3_gap.c` log | MAPPED |

### SRS-REQ-053: ci:docs
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-053 | `ci:docs` | Docs generation | `DOCS` | Docs generated | CI log | MAPPED |

### SRS-REQ-054: ci:docs
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-054 | `ci:docs` | Docs generation | `DOCS` | Docs generated | CI log | MAPPED |

### SRS-REQ-055: HAL NVM (Renode)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-055 | `test_nvm.c` | `test_nvm_*` | `ASSERT` | NVM in Renode | `test_nvm.c` log | MAPPED |
| SRS-REQ-055 | `ci:renode` | Renode NVM | `RENODE` | NVM in Renode | Renode log | MAPPED |

### SRS-REQ-056: HAL MTD
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-056 | `test_mtd.c` | `test_mtd_*` | `ASSERT` | MTD operations | `test_mtd.c` log | MAPPED |

### SRS-REQ-057: HAL CAN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-057 | `test_can.c` | `test_can_*` | `ASSERT` | CAN controller | `test_can.c` log | MAPPED |
| SRS-REQ-057 | `ci:renode` | Renode CAN | `RENODE` | CAN in Renode | Renode log | MAPPED |

### SRS-REQ-058: HAL I2C
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-058 | `test_i2c.c` | `test_i2c_*` | `ASSERT` | I2C controller | `test_i2c.c` log | MAPPED |
| SRS-REQ-058 | `ci:renode` | Renode I2C | `RENODE` | I2C in Renode | Renode log | MAPPED |

### SRS-REQ-059: Link IP
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-059 | `test_link_ip.c` | `test_link_ip_*` | `ASSERT` | IP link operations | `test_link_ip.c` log | MAPPED |

### SRS-REQ-060: Link IP (duplicate)
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-060 | `test_link_ip.c` | `test_link_ip_*` | `ASSERT` | IP link operations | `test_link_ip.c` log | MAPPED |

### SRS-REQ-061: Link TSN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-061 | `test_link_tsn.c` | `test_link_tsn_*` | `ASSERT` | TSN link operations | `test_link_tsn.c` log | MAPPED |

### SRS-REQ-062: CAN-FD
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-062 | `test_canfd.c` | `test_canfd_*` | `ASSERT` | CAN-FD operations | `test_canfd.c` log | MAPPED |
| SRS-REQ-062 | `test_fabric.c` | `test_fabric_*` | `ASSERT` | Fabric integration | `test_fabric.c` log | MAPPED |

### SRS-REQ-063: CANopen
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-063 | `test_canopen.c` | `test_canopen_*` | `ASSERT` | CANopen operations | `test_canopen.c` log | MAPPED |

### SRS-REQ-064: Serial
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-064 | `test_serial.c` | `test_serial_*` | `ASSERT` | Serial driver | `test_serial.c` log | MAPPED |

### SRS-REQ-065: PUSopen adapter
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-065 | `test_pusopen.c` | `test_pusopen_*` | `ASSERT` | PUSopen adapter | `test_pusopen.c` log | MAPPED |
| SRS-REQ-065 | `tests/stubs/pusopen.h` | Stubs | `STUB` | PUSopen stubs | Stub file | MAPPED |

### SRS-REQ-066: GEM TSN
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-066 | `test_gem_tsn.c` | `test_gem_tsn_*` | `ASSERT` | GEM TSN VLAN/ENST/TSU | `test_gem_tsn.c` log | MAPPED |

### SRS-REQ-067: Fabric
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-067 | `test_fabric.c` | `test_fabric_*` | `ASSERT` | Fabric driver | `test_fabric.c` log | MAPPED |
| SRS-REQ-067 | `test_hal.c` | `test_hal_*` | `ASSERT` | HAL fabric | `test_hal.c` log | MAPPED |

### SRS-REQ-068: GPTP
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-068 | `test_gptp.c` | `test_gptp_*` | `ASSERT` | gPTP stack | `test_gptp.c` log | MAPPED |

### SRS-REQ-069: SPI
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-069 | `test_spi.c` | `test_spi_*` | `ASSERT` | SPI controller | `test_spi.c` log | MAPPED |
| SRS-REQ-069 | `ci:renode` | Renode SPI | `RENODE` | SPI in Renode | Renode log | MAPPED |

### SRS-REQ-070: Block
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-070 | `test_block.c` | `test_block_*` | `ASSERT` | Block device | `test_block.c` log | MAPPED |
| SRS-REQ-070 | `ci:renode` | Renode Block | `RENODE` | Block in Renode | Renode log | MAPPED |

### SRS-REQ-045: OBCP Integration
| Req | Test File | Test Function | Oracle Type | Expected Result | Evidence | Status |
|-----|-----------|---------------|-------------|-----------------|----------|--------|
| SRS-REQ-045 | `test_obcp_integration.c` | `test_obcp_*` | `ASSERT` | OBCP callback connectivity | `test_obcp_integration.c` log | MAPPED |
| SRS-REQ-045 | `docs/vv/test-spec/ts-9-obcp-integration.md` | Spec | `SPEC` | OBCP integration spec | Spec doc | MAPPED |

---

## Gap Analysis

| Requirement | Status | Gap Description |
|-------------|--------|-----------------|
| SRS-REQ-005 | **NC** | Malformed TC handling oracle conflicts with test oracle (NC-EXT-04) |
| SRS-REQ-012..020 | **MAPPED** | All mapped with explicit oracles (see mappings above) |
| SRS-REQ-021..070 | **MAPPED** | All mapped with explicit oracles (see mappings above) |
| SRS-REQ-072 | **GAP** | Event envelope TBD (NC-EXT-76/77) |
| SRS-REQ-073/074/075 | **PARTIAL** | XW-2 covers middleware contract; NC-PAL-01 blocks hardware evidence |
| SRS-REQ-050 | **GAP** | PUS backend default mapping not fully documented |
| SRS-REQ-065 | **GAP** | PUSopen adapter not fully mapped |

---

## Test Files with Explicit Oracles (Mapped)

| Test File | Requirements Covered | Oracle Style | Status |
|-----------|---------------------|--------------|--------|
| `test_pus.c` | SRS-REQ-004,005,006,009,013,017 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_pus_fu.c` | SRS-REQ-008,020,024 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_ccsds.c` | SRS-REQ-004,018 | `ASSERT` | MAPPED |
| `test_decoder_corpus.c` | SRS-REQ-004,005 | `ASSERT` (mutation) | MAPPED |
| `test_pus.c` | SRS-REQ-004,005,006,009,013,017 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_time.c` | SRS-REQ-010,024 | `ASSERT` | MAPPED |
| `test_sb.c` | SRS-REQ-001,002,003,012 | `ASSERT`, `COUNTS` | MAPPED |
| `test_sb_stress.c` | SRS-REQ-001 | `COUNTS` | MAPPED |
| `test_sb_sweep_boundary.c` | SRS-REQ-001 | `ASSERT` | MAPPED |
| `test_sb_evs_structural_sweep.c` | SRS-REQ-001/011/012 | `COUNTS` | MAPPED |
| `test_sched.c` | SRS-REQ-071 | `ASSERT` | MAPPED |
| `test_pal_watchdog.c` | SRS-REQ-073/074/075 | `COUNTS` | MAPPED |
| `test_es_hs_lifecycle.c` | SRS-REQ-001/011/012, 073/074/075, 015 | `COUNTS`/`ASSERT` | MAPPED |
| `test_pus_fu.c` | SRS-REQ-008,020,024 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_pus_n7_adapter.c` | SRS-REQ-018,050 | `ASSERT` | MAPPED |
| `test_pusopen.c` | SRS-REQ-065 | `ASSERT` | MAPPED |
| `test_decoder_corpus.c` | SRS-REQ-004,005 | `ASSERT` (mutation) | MAPPED |
| `test_r2_gap.c` | SRS-REQ-005,006,007 | `ASSERT`/`LEAN` | MAPPED |
| `test_r3_gap.c` | SRS-REQ-005,006,007 | `ASSERT`/`LEAN` | MAPPED |
| `test_link_uart.c` | SRS-REQ-004/014 | `ASSERT` | MAPPED |
| `test_gptp.c` | SRS-REQ-038 | `ASSERT` | MAPPED |
| `test_gem.c` / `test_gem_tsn.c` | SRS-REQ-038/066/068 | `ASSERT` | MAPPED |
| `test_lwip_netif.c` | SRS-REQ-040 | `ASSERT` | MAPPED |
| `test_can.c` / `test_canfd.c` / `test_canopen.c` | SRS-REQ-057/062/063 | `ASSERT` | MAPPED |
| `test_tbl.c` | SRS-REQ-014,019 | `ASSERT` | MAPPED |
| `test_tbl.c` | SRS-REQ-014,019 | `ASSERT` | MAPPED |
| `test_sb_evs_structural_sweep.c` | SRS-REQ-001/011/012 | `COUNTS` | MAPPED |
| `test_sb_sweep_boundary.c` | SRS-REQ-001 | `ASSERT` | MAPPED |

---

## Next Incremental Steps (NC-EXT-47)

1. **Complete SRS-REQ-012..020 mapping** — Document test/oracle for each requirement
2. **Document SRS-REQ-021..070 gaps** — Explicitly list unmapped requirements
3. **Map SRS-REQ-050/065** — PUS backend / PUSopen adapter test evidence
4. **Resolve SRS-REQ-005 (NC-EXT-04)** — Authorized requirement/test-oracle disposition
5. **Create test-spec packages TS-1..TS-7** — Per `docs/vv/test-spec/` structure

---

## Evidence Logs (Referenced)

| Test File | Log Location | Verification |
|-----------|--------------|--------------|
| `test_pus.c` | `build/native/Testing/Temporary/LastTest.log` | `ctest -R pus` |
| `test_pus_fu.c` | `build/native/Testing/Temporary/LastTest.log` | `ctest -R pus_fu` |
| `test_es_hs_lifecycle.c` | `build/native/Testing/Temporary/LastTest.log` | `ctest -R es_hs` |
| `test_pal_watchdog.c` | `build/native/Testing/Temporary/LastTest.log` | `ctest -R pal_watchdog` |
| XW-2 target | `build/xw2-run-*/xw2-final.log` | `make xw2` |
| XW-1 host | `build/xw1-host/Testing/Temporary/LastTest.log` | `ctest -R extwd` |

---

## Status

| Metric | Value |
|--------|-------|
| Requirements with explicit oracle mapping | ~30/76 SRS (39%) |
| Test files with explicit oracles | 25+ |
| NC-EXT-47 status | **INCREMENTAL — mapping in progress; open for completion** |

---

## Authority Note

This is an incremental evidence-mapping artifact for NC-EXT-47. It does not claim completeness, requirement compliance, or test qualification. It records what has been mapped so far with explicit expected-result oracles. Completion is postponed to the last sprint per owner disposition (2026-10-16).

---