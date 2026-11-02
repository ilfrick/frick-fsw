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

---

## Gap Analysis

| Requirement | Status | Gap Description |
|-------------|--------|-----------------|
| SRS-REQ-005 | **NC** | Malformed TC handling oracle conflicts with test oracle (NC-EXT-04) |
| SRS-REQ-012..020 | **GAP** | No explicit test-oracle mapping documented |
| SRS-REQ-021..070 | **GAP** | No explicit test-oracle mapping documented |
| SRS-REQ-072 | **GAP** | Event envelope TBD (NC-EXT-76/77) |
| SRS-REQ-073/074/075 | **PARTIAL** | XW-2 covers middleware contract; NC-PAL-01 blocks hardware evidence |
| SRS-REQ-050 | **GAP** | PUS backend default mapping not fully documented |
| SRS-REQ-065 | **GAP** | PUSopen adapter not fully mapped |

---

## Test Files with Explicit Oracles (Mapped)

| Test File | Requirements Covered | Oracle Style | Status |
|-----------|---------------------|--------------|--------|
| `test_pus.c` | SRS-REQ-004,005,006,009,017 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_pus_fu.c` | SRS-REQ-008,020,024 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_ccsds.c` | SRS-REQ-004 | `ASSERT` | MAPPED |
| `test_decoder_corpus.c` | SRS-REQ-004,005 | `ASSERT` (mutation) | MAPPED |
| `test_pus.c` | SRS-REQ-004,005,006,009,017 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_time.c` | SRS-REQ-010,024 | `ASSERT` | MAPPED |
| `test_sb.c` | SRS-REQ-001,002,003 | `ASSERT` | MAPPED |
| `test_sb_stress.c` | SRS-REQ-001 | `COUNTS` | MAPPED |
| `test_sb_sweep_boundary.c` | SRS-REQ-001 | `ASSERT` | MAPPED |
| `test_sb_evs_structural_sweep.c` | SRS-REQ-001/011 | `COUNTS` | MAPPED |
| `test_sched.c` | SRS-REQ-071 | `ASSERT` | MAPPED |
| `test_pal_watchdog.c` | SRS-REQ-073/074/075 | `COUNTS` | MAPPED |
| `test_es_hs_lifecycle.c` | SRS-REQ-073/074/075, 001/011 | `COUNTS`/`ASSERT` | MAPPED |
| `test_pus_fu.c` | SRS-REQ-008,020,024 | `STREAM_COUNT`, `ASSERT` | MAPPED |
| `test_pus_n7_adapter.c` | SRS-REQ-050 | `ASSERT` | MAPPED |
| `test_pusopen.c` | SRS-REQ-065 | `ASSERT` | MAPPED |
| `test_decoder_corpus.c` | SRS-REQ-004,005 | `ASSERT` (mutation) | MAPPED |
| `test_r2_gap.c` | SRS-REQ-005,006,007 | `ASSERT`/`LEAN` | MAPPED |
| `test_r3_gap.c` | SRS-REQ-005,006,007 | `ASSERT`/`LEAN` | MAPPED |
| `test_link_uart.c` | SRS-REQ-004/014 | `ASSERT` | MAPPED |
| `test_gptp.c` | SRS-REQ-038 | `ASSERT` | MAPPED |
| `test_gem.c` / `test_gem_tsn.c` | SRS-REQ-038/066/068 | `ASSERT` | MAPPED |
| `test_lwip_netif.c` | SRS-REQ-040 | `ASSERT` | MAPPED |
| `test_can.c` / `test_canfd.c` / `test_canopen.c` | SRS-REQ-057/062/063 | `ASSERT` | MAPPED |

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
| Requirements with explicit oracle mapping | ~20/76 SRS |
| Test files with explicit oracles | 20+ |
| NC-EXT-47 status | **INCREMENTAL — mapping initiated; open for completion** |

---

## Authority Note

This is an incremental evidence-mapping artifact for NC-EXT-47. It does not claim completeness, requirement compliance, or test qualification. It records what has been mapped so far with explicit expected-result oracles. Completion is postponed to the last sprint per owner disposition (2026-10-16).

---