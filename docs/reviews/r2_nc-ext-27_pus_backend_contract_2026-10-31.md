# R2 REVIEW REPORT — NC-EXT-27: PUS Backend Contract Tests (Final State)

**Review date:** 2026-10-31  
**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (R2 — requirements/test coverage)  
**Artifact versions:** `frick-fsw` HEAD `6135768`  
**Review type:** Fresh review of FINAL state (prior signature-mismatch findings resolved)  
**Read-only:** No modifications made

---

## 1. SCOPE

| Requirement | Description | Backend under test | Contract test file |
|-------------|-------------|-------------------|-------------------|
| **SRS-REQ-050** | N7/ESA default PUS-C backend (FF_USE_PUS_ASN1=ON) | `FF_PusBackendN7` (vtable: init, encode_tm, decode_tc) | `tests/unit/test_pus_n7_adapter.c` (2 test functions, 23 checks) |
| **SRS-REQ-065** | PUSopen(R) commercial adapter (optional, FRICK_USE_PUSOPEN=1) | `FF_PusBackendPusopen` (vtable: init, encode_tm, decode_tc) | `tests/unit/test_pusopen.c` (3 test functions, 33 checks) |

**Traceability source:** `docs/requirements/traceability.yaml` lines 474–481 (SRS-REQ-050), 609–616 (SRS-REQ-065).

**Governance:** N22 (test design: nominal + out-of-bound with explicit expected results), ECSS-E-ST-40C Rev.1 (verification), ECSS-Q-ST-80C Rev.2 (product assurance).

---

## 2. REQUIREMENTS COVERAGE MATRIX

### 2.1 SRS-REQ-050 → `test_pus_n7_adapter.c` + `test_pus.c` cross-check

| SRS-REQ-050 Claim | Test Case | Oracle (Explicit Expected Result) | Independence |
|---|---|---|---|
| Backend selectable via `FF_PusSetBackend` & `FF_PusGetBackend` | `test_n7_tc_decode_with_generated_crc` (lines 42–44) | `FF_PusSetBackend(&FF_PusBackendN7) == FF_SUCCESS`; `FF_PusGetBackend() == &FF_PusBackendN7` | Black-box vtable; no implementation knowledge used |
| `init()` returns success | Same test, line 42 | `FF_PusInit() == FF_SUCCESS` | |
| `decode_tc` accepts valid TC with N7/ESA CRC | Same test, lines 50–54 | `decode_tc == FF_SUCCESS`; `apid == FF_APID_TC`; `hdr.service == FF_PUS_SERVICE_TEST`; `hdr.subservice == 1u`; `data_len == 2u && data[0]==0x31 && data[1]==0x42` | Valid TC built by reference `FF_CcsdsEncodeTc`; decoded payload compared bytewise |
| `decode_tc` rejects single-bit CRC corruption | Same test, lines 56–60 | `decode_tc == FF_CRC_ERROR` | Mutated trailer bit; exact expected status asserted |
| `decode_tc` rejects structural/boundary errors (length overflow, version field, NULL hdr, short packet, zero primary header) | Same test, lines 62–83 | `decode_tc == FF_PARSE_ERROR` for each of 5 distinct malformed inputs | Each error case is an independent call with controlled corruption; expected status is `FF_PARSE_ERROR` |
| `encode_tm` produces frame decodable by reference codec with matching CRC | `test_n7_tm_uses_generated_crc`, lines 113–135 | `encode_tm == FF_SUCCESS`; `frame_len >= 2u`; CRC trailer matches `FF_CcsdsCrc16`; `FF_CcsdsDecodeTm == FF_SUCCESS`; `decoded_apid == FF_APID_TM`; service/subservice match; payload round-trip `memcmp == 0` | Reference codec is the oracle; N7 CRC cross-checked against same polynomial (common-mode acknowledged in comment) |
| Backend framing parity with Open backend (when FF_USE_PUS_ASN1=ON) | `test_pus.c::test_ping_roundtrip`, lines 122–149 | `Open.encode_tm` vs `N7.encode_tm` produce identical length (`la == lb`) and bytewise-equal frames (`memcmp == 0`); backend switch verified (`FF_PusSetBackend(&FF_PusBackendN7) == FF_SUCCESS`) | Independent oracle: Open backend output is the reference; N7 output compared to it |

**SRS-REQ-050 Coverage:** **Complete** — all vtable functions exercised; nominal + 6 distinct out-of-bound decode cases; encode cross-checked against independent reference.

---

### 2.2 SRS-REQ-065 → `test_pusopen.c`

| SRS-REQ-065 Claim | Test Case | Oracle (Explicit Expected Result) | Independence |
|---|---|---|---|
| `init()` success path | `test_pusopen_init`, line 90 | `init() == FF_SUCCESS` | Controlled stub returns `PUSOPEN_OK` |
| `init()` failure propagation | `test_pusopen_init`, lines 91–92 | Stub returns `PUSOPEN_ERROR` → `init() == FF_ERROR` | Stub directly controls SDK return code |
| `encode_tm` nominal field mapping & payload pass-through | `test_pusopen_encode`, lines 103–112 | `encode_tm == FF_SUCCESS`; `out_len == 3u`; payload bytes match; `g_encode_packet` fields = `hdr` fields (service, subservice, counter, time_s, time_f, data_len) | Stub captures `PUSopen_Packet_t` input; test asserts exact field equality |
| `encode_tm` NULL-pointer guards (hdr, payload, out, out_len) | `test_pusopen_encode`, lines 114–121 | Each NULL → `FF_INVALID_ARGUMENT` (4 cases) | Adapter's own validation; stub not invoked |
| `encode_tm` PUSopen SDK error propagation | `test_pusopen_encode`, lines 123–125 | `g_encode_status = PUSOPEN_ERROR` → `FF_ERROR` | Stub returns controlled error |
| `encode_tm` buffer-capacity overflow (`written > out_cap`) | `test_pusopen_encode`, lines 127–129 | `g_encode_written = out_cap+1` → `FF_BAD_SIZE` | Adapter bounds check exercised |
| `encode_tm` size overflow (`written > UINT16_MAX`) | `test_pusopen_encode`, lines 130–133 | `g_encode_written = UINT16_MAX+1` → `FF_BAD_SIZE` | Adapter bounds check exercised |
| `decode_tc` nominal decode with field mapping | `test_pusopen_decode`, lines 154–161 | `decode_tc == FF_SUCCESS`; `apid == FF_APID_TC`; `hdr.version == FF_PUS_VERSION`; `ack_flags`, `service`, `subservice`, `source_id`, `data`, `data_len` all match stub-provided packet | Stub provides controlled `PUSopen_Packet_t`; test asserts exact field values |
| `decode_tc` NULL raw pointer | `test_pusopen_decode`, line 163 | `FF_PARSE_ERROR` | Adapter guard before SDK |
| `decode_tc` length < primary header | `test_pusopen_decode`, line 165–166 | `len=5u` → `FF_PARSE_ERROR` | Adapter guard before SDK |
| `decode_tc` APID mismatch (reject before SDK call) | `test_pusopen_decode`, lines 172–178 | `APID = FF_APID_TC+1` → `FF_PARSE_ERROR`; `g_decode_calls` unchanged (SDK not called) | **Critical contract guard**: adapter enforces local APID policy before delegating |
| `decode_tc` non-TC packet type (TM) | `test_pusopen_decode`, lines 179–181 | `packet_type = TM` → `FF_PARSE_ERROR` | Adapter primary-header check |
| `decode_tc` PUSopen SDK error propagation | `test_pusopen_decode`, lines 183–186 | `g_decode_status = PUSOPEN_ERROR` → `FF_PARSE_ERROR` | Stub error mapped |
| `decode_tc` NULL payload in decoded packet | `test_pusopen_decode`, lines 188–191 | `g_decode_packet.data = NULL, data_len=1` → `FF_BAD_SIZE` | Adapter bounds check on SDK output |
| `decode_tc` oversized payload (`data_len > UINT16_MAX`) | `test_pusopen_decode`, lines 192–195 | `data_len = UINT16_MAX+1` → `FF_BAD_SIZE` | Adapter bounds check on SDK output |

**SRS-REQ-065 Coverage:** **Complete** — all vtable functions exercised; nominal + 11 distinct out-of-bound/error cases with explicit status codes; guard ordering (APID/packet-type before SDK) verified by call-count oracle.

---

## 3. N22 COMPLIANCE ASSESSMENT

| N22 Criterion | SRS-REQ-050 (N7) | SRS-REQ-065 (PUSopen) | Verdict |
|---|---|---|---|
| Requirement-based test design (not code-derived) | ✅ Tests derive from vtable contract & SRS claims; black-box calls to `FF_PusBackendN7` | ✅ Tests derive from adapter contract (SRS-REQ-065, `pus-layer.md` §§6–7); black-box vtable calls | **PASS** |
| Nominal cases with explicit expected results | ✅ Valid TC decode, valid TM encode, backend selection, cross-backend framing parity | ✅ Init success, encode nominal, decode nominal with full field mapping | **PASS** |
| Applicable out-of-bound / adversarial cases with explicit expected results | ✅ 6 decode error cases: CRC corruption, length overflow, version corruption, NULL hdr, short packet, zero primary header — all assert `FF_CRC_ERROR` or `FF_PARSE_ERROR` | ✅ 11 error cases across init/encode/decode: NULL pointers (4), SDK error, buffer overflow, size overflow, APID mismatch, wrong packet type, NULL payload, oversized payload — each asserts exact `FF_Status_t` | **PASS** |
| Oracles are explicit assertions (not "should work") | ✅ Every `FF_TEST_ASSERT` compares against a specific `FF_Status_t` or field value | ✅ Every `FF_TEST_ASSERT` compares against a specific `FF_Status_t` or captured stub field | **PASS** |
| Test design independent of implementation (black-box vtable contract) | ✅ Tests call `FF_PusBackendN7.decode_tc/encode_tm/init` directly; no internal function calls | ✅ Tests call `FF_PusBackendPusopen.init/encode_tm/decode_tc` directly; stub replaces vendor SDK | **PASS** |
| Generated N7 sources are not the test oracle | ✅ Oracle is reference `FF_CcsdsEncodeTc/DecodeTm` and `FF_CcsdsCrc16`; N7 output compared to reference | N/A (no generated sources; adapter tested against controlled stub) | **PASS** |

---

## 4. FINDINGS

| ID | Severity | Description | Location |
|---|---|---|---|
| **R2-NC27-01** | **Observation** | **SRS-REQ-050 cross-backend framing parity test** (`test_pus.c` lines 122–149) only executes when `FF_USE_PUS_ASN1=ON`. This is correct (the N7 backend only exists in that configuration), but the traceability.yaml lists `test_pus.c` as evidence for SRS-REQ-050 unconditionally. Recommendation: guard the traceability entry or document the configuration dependency. | `docs/requirements/traceability.yaml` line 479; `tests/unit/test_pus.c` lines 122–149 |
| **R2-NC27-02** | **Observation** | **N7 CRC cross-check** (`test_pus_n7_adapter.c` lines 122–128) compares N7 CRC against `FF_CcsdsCrc16` (reference codec). Comment acknowledges this is a *compatibility* check, not an independent CRC implementation (both follow same polynomial). This is a known common-mode limitation, explicitly documented. No action required; evidence boundary is honest. | `tests/unit/test_pus_n7_adapter.c` lines 115–128 |
| **R2-NC27-03** | **Observation** | **PUSopen adapter test uses a stub** (`tests/unit/test_pusopen.c` lines 26–63) that mimics the vendor SDK API. The test file header (lines 3–8) and traceability lean_note (line 616) correctly state: "The test double is not vendor SDK compatibility or PUSopen qualification." This is appropriate for vtable-contract verification. No action required. | `tests/unit/test_pusopen.c` lines 3–8; `traceability.yaml` line 616 |
| **R2-NC27-04** | **Minor** | **Missing test for `encode_tm` with `data == NULL && data_len == 0`** — the adapter's `encode_tm` (line 36–38 in `pus_adapter_pusopen.c`) permits this combination, but no test exercises it. The N7 backend test also lacks this case (`n7_encode_tm` at line 49 checks `*out_len < 2u` but not the NULL-payload path separately). Low risk (zero-length payload is a valid edge case). | `src/pus/adapters/pus_adapter_pusopen.c` lines 36–38; `src/pus/adapters/pus_adapter_n7.c` lines 44–51 |
| **R2-NC27-05** | **Minor** | **N7 `decode_tc` with CRC disabled** (`FF_FRAME_WITH_CRC=0`) exercises only the success path (line 88–90). No out-of-bound cases are tested in the CRC-disabled configuration. Since `FF_FRAME_WITH_CRC` defaults ON and the project's SRS-REQ-004 requires CRC, this is an acceptable tailoring boundary. | `tests/unit/test_pus_n7_adapter.c` lines 84–90 |

---

## 5. VERDICT

| Requirement | Verdict | Rationale |
|---|---|---|
| **SRS-REQ-050** (N7 backend contract) | **PASS WITH COMMENTS** | All vtable functions covered; nominal + 6 out-of-bound decode cases with explicit oracles; cross-backend framing parity verified against independent reference; CRC cross-check boundary documented. Observation R2-NC27-01 (traceability configuration guard) and R2-NC27-04 (zero-length payload) are non-blocking. |
| **SRS-REQ-065** (PUSopen adapter contract) | **PASS WITH COMMENTS** | All vtable functions covered; nominal + 11 out-of-bound/error cases with explicit `FF_Status_t` oracles; critical guard ordering (APID/packet-type before SDK) verified by call-count oracle; stub correctly characterized as contract test double, not SDK qualification. Observation R2-NC27-04 (zero-length payload) is non-blocking. |
| **Overall NC-EXT-27** | **PASS WITH COMMENTS** | Contract tests satisfy N22: requirement-based, nominal + adversarial cases with explicit expected results, independent oracles, black-box vtable design. Prior signature-mismatch findings are resolved. Observations are documentation/tailoring boundary items, not evidence gaps. |

---

## 6. RECOMMENDATIONS

1. **Traceability hardening** (R2-NC27-01): Update `traceability.yaml` to reflect that `test_pus.c` N7 cross-check only applies under `FF_USE_PUS_ASN1=ON`, or split the trace entry.
2. **Zero-length payload coverage** (R2-NC27-04): Add one nominal test case per backend for `encode_tm` with `data == NULL, data_len == 0` to exercise the valid empty-payload path.
3. **No further action** on R2-NC27-02, R2-NC27-03, R2-NC27-05 — these are correctly documented evidence boundaries.

---

## 7. EVIDENCE RETENTION

This review report shall be stored at `docs/reviews/r2_nc-ext-27_pus_backend_contract_2026-10-31.md` per N11 (test scripts and review results retained immediately; committed with compliant change).

---

**End of R2 Review Report**
