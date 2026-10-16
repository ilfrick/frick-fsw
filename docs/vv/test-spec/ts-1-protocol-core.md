# Test Specification TS-1 — Protocol core (CCSDS Space Packet, PUS-C services, time/CUC)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-TS1                     |
| Revision                         | 0.8                                    |
| Date                             | 2026-10-15                             |
| Norms                            | N10, N11, N22                          |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |
| Work-package index               | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-1 |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |
| Scope                            | CCSDS Space Packet (CCSDS 133.0-B-2), PUS-C services 1/3/5/6/8/9/17/20 (ECSS-E-ST-70-41C), time / CUC (CCSDS 301.0-B-4) |
| Baseline                         | Current candidate source HEAD `13ebf92` plus uncommitted deltas in `src/apps/app.h`, `src/apps/app_cmd_ingest.c`, and `tests/unit/test_r2_gap.c`; host configuration `FF_PUS_MAX_TC_FRAME=1024`. Earlier cases retain their recorded historical baselines. |
| Modules under test               | `src/transport/ccsds/ccsds.c`, `src/pus/pus.c`, `src/pus/pus_services.c`, `src/core/time/time.c`, `src/core/time/pps.c` |
| Interfaces cited                 | `src/transport/ccsds/ccsds.h`, `src/pus/pus.h`, `src/pus/pus_types.h`, `src/core/time/time.h`, `docs/interfaces/ICD.md` (FRICK-ICD-001) |

> **Authority statement.** This work package defines verification *cases and
> oracles* for the current baseline. It does not assign criticality, accept
> residual risk, qualify tools, close an NC, or approve release/flight
> readiness. Requirement/interface gaps are recorded (never resolved by
> inventing an oracle); disposition and closure require the authorized
> authorities.

## 1. Scope and method

### 1.1 In scope

Cases derived from the following requirements, which are the ones in this
work package's area and present in `docs/requirements/traceability.yaml`:

`SRS-REQ-003` (CCSDS PDL overflow safety), `SRS-REQ-004` (CRC-16 integrity),
`SRS-REQ-005` (TC acceptance / verification failure), `SRS-REQ-006` (service 1
acknowledged stages), `SRS-REQ-007` (TM message-type counter), `SRS-REQ-008`
(service 3 housekeeping), `SRS-REQ-009` (service 6 memory supervision),
`SRS-REQ-010` (service 9 time), `SRS-REQ-011` (service 5 events),
`SRS-REQ-012` (service 17 test), `SRS-REQ-017` (CCSDS APIDs / mission id),
`SRS-REQ-018` (pluggable PUS backends), `SRS-REQ-024` (CUC framing),
`SRS-REQ-021` (time source), `SRS-REQ-022` (tone discipline), `SRS-REQ-023`
(scheduled leap), `SRS-REQ-050` (optional PUS-C ASN.1 backend), `SRS-REQ-052`
(fundamental PUS-C service set; implemented subset only), `SRS-REQ-044`
(Renode functional-validation environment, cited only by the PIL gap case
`N22-TS1-92`).

Out of scope (owned by other work packages): link layer and SDLS (TS-2), I/O
drivers (TS-3/TS-4), core services SB/TBL/EVS/ES/scheduler (TS-5), software-only
robustness programme (TS-6). Service 11 is included only as the delivered stub
boundary of `SRS-REQ-052`; `SRS-REQ-059`/`060`/`061` (segmentation, IP/TSN
links) belong to TS-2.

### 1.2 Class, scope, environment and status conventions

- **Class**: `nominal` = a legal input inside the specified operating envelope
  (this includes legal minimum/maximum boundary values); `OOB` = a
  range-violating, malformed, out-of-envelope, resource-exhausted or
  prohibited-ordering input.
- **Scope**: `individual` = one protocol layer / one module surface is under
  test (the other modules are only fixtures); `combined` = the case
  deliberately asserts coupling across two or more modules/layers in one
  scenario (e.g. CCSDS + PUS + time, EVS + PUS + CCSDS).
- **Env**: `SIL` = native host unit/integration suite (`make test`, ctest);
  `PIL` = Renode emulated PolarFire SoC; `HIL` = target silicon
  (RTPFS160ZT). Per N22 and FRICK-VV-TSPEC-001 §1.3, an emulator establishes
  only what its model represents.
- **Status**: `covered` = the case is specified with an oracle grounded in a
  controlled requirement or interface clause and is executable in the named
  environment (the test may still have to be added to the suite; execution
  status is tracked by the umbrella and `docs/vv/verification-results.md`).
  `gap` = no controlled oracle exists (the requirement/interface is silent,
  contradictory, or the feature is not delivered), so the case is recorded
  against **NC-EXT-47** and **not** executed to a fabricated oracle.
- **Gap discipline (N22)**: every `OOB` row whose expected behaviour is not
  defined by a requirement/interface clause is `gap`; the oracle cell names
  the open NC/TBD and, where applicable, an uncontrolled *observation*
  (explicitly not an oracle). The Gaps list (§3) maps every gap row to an
  NC-EXT-47 sub-item (FRICK-VV-TSPEC-001 §7).

### 1.3 Interface clauses used as oracle ground

| Clause | Content used as oracle ground |
| --- | --- |
| FRICK-ICD-001 §2 | PUS version 2, mission id 0x01, TC APID 0x010, TM APID 0x020, CRC-16/CCITT-FALSE trailer enabled, max TC/TM frame 1024 |
| FRICK-ICD-001 §3 | primary-header layout (version/type/sec-hdr flag/APID, seq flags = standalone and seq count emitted as zero, PDL = packet data length − 1, PDL includes CRC), TC 5-octet and TM 13-octet secondary headers, CRC computed over bytes before the trailer and compared with the transmitted value (not a zero-residue check) |
| FRICK-ICD-001 §4 | implemented services/subservices, S1 verification failure code set `FF_PUS_FAIL_*`, S3 report (report id + data), S5 event content, S6 addr[4]+size[2] big-endian bounded to registered regions, S9 6-octet CUC T-field, S17 echo |
| FRICK-ICD-001 §5 | two CUC exchange forms: 6-octet on-wire T-field (P-field implicit, accepted only at exactly 6 octets, wrong length → `FF_BAD_SIZE`) and 7-octet self-describing code (P-field 0x2E, coarse 4 BE, fine 2 BE; wrong length → `FF_BAD_SIZE`, unknown P-field → `FF_PARSE_ERROR`) |
| `pus_types.h`, `ccsds.h`, `time.h`, `ff_types.h` | declared API contract: status codes (`FF_INVALID_ARGUMENT`, `FF_BAD_SIZE`, `FF_NOT_READY`, `FF_NOT_FOUND`, `FF_NO_RESOURCE`, `FF_NOT_IMPLEMENTED`, `FF_PARSE_ERROR`, `FF_CRC_ERROR`) and documented bounds (`FF_PUS_MAX_SERVICES`, `FF_TIME_CUC_LEN`, `FF_TIME_CUC_PFIELD`, `FF_TIME_CUC_MAX_LEAP_S`) |

## 2. Test cases

Case IDs `N22-TS1-1` … `N22-TS1-96`; groups: `1–16` CCSDS packet, `17–36` PUS
core (S1, counter, dispatch, backends), `37–61` PUS services (3/5/6/8/9/17/20,
S11 stub), `62–83` time base and CUC, `84–96` combined scenarios and
environment-bound gaps.

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| N22-TS1-1 | SRS-REQ-017 | `ccsds.c` primary header (EncodeTc / EncodeTm): version, type, sec-hdr flag, APID, seq flags, PDL | nominal | apid = 0x010 (TC) and 0x020 (TM); data_len = 4; out_cap ≥ FF_CCSDS_MAX_TC_PACKET / FF_CCSDS_MAX_TM_PACKET | TC bytes [0..1] = 0x18 0x10 (version 000, type bit = 1, sec-hdr flag = 1, APID 0x010); TM bytes [0..1] = 0x08 0x20 (type bit = 0, APID 0x020); bytes [2..3] = 0xC000 (standalone, seq count 0); bytes [4..5] = total_len − 7 (ICD §2/§3) | SIL | individual | covered |
| N22-TS1-2 | SRS-REQ-003 | `ccsds.c` TC encode/decode round-trip | nominal | hdr {version 2, ack 0x0B, service 17, subservice 1, source 0x0102}; data = {01,02,03,04}; apid 0x010 | encode = `FF_SUCCESS`; out_len = 17 (6+5+4+2); PDL field = 0x000A; decode = `FF_SUCCESS` with apid 0x010, version 2, ack 0x0B, service 17, subservice 1, source 0x0102, data_len 4, data byte-identical (ICD §3) | SIL | individual | covered |
| N22-TS1-3 | SRS-REQ-003 | `ccsds.c` TM encode/decode round-trip | nominal | hdr {version 2, service 3, subservice 25, counter 7, destination_id 0x0102, coarse 1234, fine 0x8000}; data 3 octets; apid 0x020 | encode = `FF_SUCCESS`; out_len = 24 (6+13+3+2); decode = `FF_SUCCESS`; all header fields recovered; absolute octets 13..16 = coarse 0x000004D2, 17..18 = fine 0x8000; data_len 3, data identical (ICD §3/§5) | SIL | individual | covered |
| N22-TS1-4 | SRS-REQ-003 | `ccsds.c` PDL minimum (zero-length user data) | nominal | data_len = 0, data = NULL; TC out_cap = 13, TM out_cap = 21 | encode = `FF_SUCCESS`; TC out_len = 13 with PDL 0x0006 (declared data 7 = 5 sec-hdr + 2 CRC); TM out_len = 21 with PDL 0x000E (declared 15 = 13 + 2); decode = `FF_SUCCESS` with data_len 0 (SRS-REQ-003 minimum) | SIL | individual | covered |
| N22-TS1-5 | SRS-REQ-003 | `ccsds.c` PDL maximum (FF_PUS_MAX_TC_FRAME / FF_PUS_MAX_TM_FRAME) | nominal | data_len = 1024; TC out_cap = 1037; TM out_cap = 1045; payload = 1024-octet ramp | encode = `FF_SUCCESS`; TC out_len 1037 and PDL 0x0406 (1030); TM out_len 1045 and PDL 0x040E (1038); PDL+1 fits 16 bits (no wraparound); decode = `FF_SUCCESS`, data_len 1024, payload identical | SIL | individual | covered |
| N22-TS1-6 | SRS-REQ-003 | `ccsds.c` encode over-max payload (max+1) and failure atomicity | OOB | data_len = 1025; out_cap = 1037 (FF_CCSDS_MAX_TC_PACKET) | **gap** — SRS-REQ-003 is bounded to frames "up to FF_PUS_MAX_*_FRAME bytes"; no requirement/ICD clause defines encoder behaviour or buffer/out_len atomicity above the bound. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_BAD_SIZE`, no octets written. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-7 | SRS-REQ-003 | `ccsds.c` decode short buffer (len < minimum) | OOB | len = 0, 5, 10 for DecodeTc (min 11 = 6+5); len = 18 for DecodeTm (min 19 = 6+13) | `FF_PARSE_ERROR` for every short length; no partial decode (SRS-REQ-003 reject clause) | SIL | individual | covered |
| N22-TS1-8 | SRS-REQ-003 | `ccsds.c` decode declared data field below minimum (min−1) | OOB | TC PDL = 4 (declared 5 < 5+2 = 7); TM PDL = 12 (declared 13 < 13+2 = 15) | `FF_PARSE_ERROR`; APID, secondary header, data pointer, and data length remain unchanged for both decoders (SRS-REQ-003: field cannot contain configured secondary header + CRC) | SIL | individual | covered |
| N22-TS1-9 | SRS-REQ-003 | `ccsds.c` decode PDL overflow / 16-bit wrap | OOB | TC and TM PDL = 0xFFFA (declared 0xFFFB; expected packet length 65537 > 65535); TC and TM PDL = 0xFFFF (declared 65536; expected packet length 65542) in allocated buffers containing the minimum valid packet | `FF_PARSE_ERROR` for all four; APID, secondary header, data pointer, and data length remain unchanged; widened arithmetic must not wrap (SRS-REQ-003, Lean `req_srs003_*` covers encode bound only) | SIL | individual | covered (FRICK-REVIEW-0316; host only) |
| N22-TS1-10 | SRS-REQ-003 | `ccsds.c` decode exact packet boundary | OOB | valid TC and TM frame plus 1 trailing octet; each valid frame truncated by 3 octets; malformed PDL while retaining the encoded minimum packet length | `FF_PARSE_ERROR` in all cases (SRS-REQ-003: primary-header length describes exactly the supplied boundary; one decoder call is one packet, not packet plus suffix) | SIL | individual | covered |
| N22-TS1-11 | SRS-REQ-003, SRS-REQ-017 | `ccsds.c` decode wrong packet type (type-bit mismatch) | OOB | TM frame into `FF_CcsdsDecodeTc` and TC frame into `FF_CcsdsDecodeTm` | `FF_PARSE_ERROR` for both (ICD §3 type bit + APID profile: TC 0x010 / TM 0x020) | SIL | individual | covered |
| N22-TS1-12 | SRS-REQ-003 | `ccsds.c` decode NULL raw pointer | OOB | `FF_CcsdsDecodeTc(NULL, 0, …)`, `FF_CcsdsDecodeTm(NULL, 0, …)` | **gap** — no SRS/ICD clause defines the return code for a NULL buffer. Open TBD (NC-EXT-47 §7.2/§7.3). Uncontrolled observation only: `FF_PARSE_ERROR`. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-13 | SRS-REQ-004 | `ccsds.c` CRC-16/CCITT-FALSE determinism and residue | nominal | data {01,02,03,04}; then the same 4 octets followed by their CRC | `FF_CcsdsCrc16` deterministic and non-zero; CRC over (data ‖ CRC) = 0 (checksum property, Lean `crc16_append_makes_zero`, `req_srs004_*`; ICD §3) | SIL | individual | covered |
| N22-TS1-14 | SRS-REQ-004 | `ccsds.c` CRC mismatch detection (body and trailer) | OOB | flip 1 byte of the packet body (e.g. TM octet 10) and, separately, 1 trailer octet; TC and TM | decode = `FF_CRC_ERROR` in all four variants; SRS-REQ-004 (decoder compares the computed value with the transmitted trailer, ICD §3) | SIL | individual | covered |
| N22-TS1-15 | SRS-REQ-017 | `ccsds.c` APID outside the 11-bit field | OOB | apid = 0x0800 (only bit 11 set) and apid = 0xFFFF | **gap** — the ICD fixes no behaviour for an APID wider than the 11-bit field; primary-header acceptance policy is open (NC-EXT-13 / NC-EXT-47 §7.3). Uncontrolled observation only: masking to 0x000 and 0x7FF, round-trip not lossless. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-16 | SRS-REQ-017 | `ccsds.c` received primary-header sequence-count / seq-flag policy | OOB | TC with seq flags 0b01 / 0b10 / 0b11 and sequence count 0x3FFF, 1, 0 | **gap** — ICD §3 states sequence-count fields are "currently encoded as zero … pending the controlled profile decision (NC-EXT-06)"; no received-frame acceptance rule exists (NC-EXT-06 / NC-EXT-13; NC-EXT-47 §7.3). Uncontrolled observation only: the decoder ignores seq flags/count and dispatches. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-17 | SRS-REQ-006 | `pus.c` `FF_PusIngest` acknowledged-stage set (acceptance + start + completion) | nominal | registered handler; TC ack = ACCEPTANCE\|START\|COMPLETION; service 17 subservice 1 | exactly one (1,1), exactly one (1,3), exactly one (1,7); zero (1,2)/(1,4)/(1,8); handler executed once (SRS-REQ-006) | SIL | individual | covered |
| N22-TS1-18 | SRS-REQ-006 | `pus.c` acceptance + completion produces at least two S1 packets | nominal | registered handler; TC ack = ACCEPTANCE\|COMPLETION | emitted stream contains exactly one (1,1) and one (1,7) = 2 S1 packets, and no other S1 subservice (SRS-REQ-006 explicit "at least two packets") | SIL | individual | covered |
| N22-TS1-19 | SRS-REQ-006 | `pus.c` ack-flag matrix (start-only, progress-only, no ack) | nominal | TC ack = START (0x02) only; then PROGRESS (0x04) only; then 0x00; registered handler | START-only → exactly one (1,3), no (1,1)/(1,7); PROGRESS-only → exactly one (1,5), no (1,1)/(1,7); ack = 0 → zero S1 packets while the handler still executes (SRS-REQ-006 constrains acknowledged stages only) | SIL | individual | covered |
| N22-TS1-20 | SRS-REQ-005 | `pus.c` unknown service (0x00) and unknown subservice → S1 completion failure | OOB | TC (service 0x00, sub 0x00) and TC (service 17, sub 0x7F), both ack = ACCEPTANCE\|COMPLETION | `FF_NOT_FOUND`; exactly one (1,8) with payload[4] = `FF_PUS_FAIL_UNKNOWN_SERVICE` (8) for service 0x00 and `FF_PUS_FAIL_UNKNOWN_SUBSERVICE` (9) for the unknown subservice; no service TM (SRS-REQ-005 + ICD §4) | SIL | individual | covered |
| N22-TS1-21 | SRS-REQ-005 | `pus.c` unknown *non-zero* service id → failure code selection | OOB | TC (service 0x7F, sub 0x01), ack = ACCEPTANCE\|COMPLETION | **gap** — SRS-REQ-005 requires "the appropriate error code" but the ICD does not map codes to an unknown non-zero service; a code-8 vs code-9 choice is undefined. Open NC-EXT-04 (NC-EXT-47 §7.3). Uncontrolled observation only: (1,8) carrying code 9 (`FF_PUS_FAIL_UNKNOWN_SUBSERVICE`). No oracle is asserted here | SIL | individual | gap |
| N22-TS1-22 | SRS-REQ-005 | `pus.c` unknown service / subservice with no ack flags → failure reporting | OOB | TC (service 0x00, sub 0x00) and TC (service 17, sub 0x7F), ack = 0 | **gap** — SRS-REQ-005 states a telecommand with an unknown service/subservice "shall produce a PUS service 1 completion failure"; the implementation reports a failure only when the corresponding ack flag is set, so the requirement and PUS-C ack semantics conflict. Open NC-EXT-04 (NC-EXT-47 §7.3). Uncontrolled observation only: no S1 packet. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-23 | SRS-REQ-005, SRS-REQ-017 | `pus.c` illegal TC APID → acceptance failure, no dispatch | OOB | TC apid = 0x011 (≠ FF_APID_TC 0x010), service 17 sub 1, ack = ACCEPTANCE\|COMPLETION; registered handler | `FF_PusIngest` returns `FF_ERROR`; exactly one (1,2) with payload[4] = `FF_PUS_FAIL_ILLEGAL_APID` (1); handler not executed; no (17,2) (SRS-REQ-017 TC APID + ICD §4) | SIL | individual | covered |
| N22-TS1-24 | SRS-REQ-005 | `pus.c` malformed TC is not executed | OOB | CRC-corrupted S17 ping frame (flip a body octet); PDL-shortened frame; handler registered | `FF_PARSE_ERROR`; handler not executed; no (17,2); no (1,1); SRS-REQ-005 "shall not be executed" | SIL | individual | covered |
| N22-TS1-25 | SRS-REQ-005 | `pus.c` malformed-TC failure *reporting* (S1 vs EVS/S5) | OOB | CRC-corrupted S17 ping frame with EVS listener wired to PUS S5 as in `fsw_main.c` | **gap** — SRS-REQ-005 requires a PUS service 1 completion failure, while the baseline emits only an EVS event forwarded as (5,3); the two expectations conflict and no clause resolves them. Open NC-EXT-04 (NC-EXT-47 §7.3). Uncontrolled observation only: (5,3) with type `FF_EVS_ERROR` and zero (1,1)/(1,8). No oracle is asserted here | SIL | individual | gap |
| N22-TS1-26 | SRS-REQ-007 | `pus.c` TM message-type counter strictly increases | nominal | 100 consecutive `FF_PusSendTm` / `FF_PusNextTmCounter` calls | returned values strictly increase by 1 between consecutive calls; no value repeats before wrap; `FF_PusPeekTmCounter` equals the next value to be issued (SRS-REQ-007) | SIL | individual | covered |
| N22-TS1-27 | SRS-REQ-007 | `pus.c` TM counter wrap modulo 16384 | OOB | > 16384 consecutive increments (full wrap) | every returned value ∈ [0, 16383]; the transition is 16383 → 0; no value ≥ 16384 ever issued (mask 0x3FFF, SRS-REQ-007 wrap clause, Lean `req_srs007_counter_wrap_bounded`) | SIL | individual | covered |
| N22-TS1-28 | SRS-REQ-007 | `pus.c` counter value placed in the emitted TM secondary header | nominal | emit 3 TMs; decode each with `FF_CcsdsDecodeTm` | each TM absolute octets 9..10 (big-endian) equal the counter issued for that packet and increase by 1 modulo 16384 between packets (ICD §3 counter field + SRS-REQ-007) | SIL | individual | covered |
| N22-TS1-29 | SRS-REQ-003 | `pus.c` `FF_PusSendTm` at the maximum payload | nominal | len = FF_PUS_MAX_TM_FRAME (1024); backend set; TM sink registered | `FF_SUCCESS`; sink receives one frame of 1045 octets with PDL 0x040E and service/subservice as requested (SRS-REQ-003 bound + ICD §2) | SIL | individual | covered |
| N22-TS1-30 | SRS-REQ-003 | `pus.c` `FF_PusSendTm` above the maximum payload (max+1) | OOB | len = 1025 (> FF_PUS_MAX_TM_FRAME) | **gap** — no clause defines the over-max behaviour of the emitter. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_BAD_SIZE`, sink not called. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-31 | SRS-REQ-005 | `pus.c` `FF_PusSendTm` before a backend is selected | OOB | `FF_PusInit()` then `FF_PusSendTm(...)` with no `FF_PusSetBackend` | **gap** — no clause defines the pre-backend emitter state. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NOT_READY`. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-32 | SRS-REQ-018 | `pus.c` invalid backend / invalid emitter arguments | OOB | `FF_PusSetBackend(NULL)`; backend with NULL `encode_tm`; `FF_PusSendTm(..., data=NULL, len=1)` | **gap** — SRS-REQ-018 requires pluggability but defines no rejection contract for a malformed backend or NULL payload. Open TBD (NC-EXT-47 §7.2/§7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-33 | SRS-REQ-018, SRS-REQ-050 | `pus.c` backend selection and optional N7 backend | nominal | default build uses `FF_PusBackendOpen`; when optional generated N7 sources are available, set N7 and encode a matched test header | open is the configured default (ADR-0018); N7 is only built/tested when `FF_USE_PUS_ASN1=ON` and generated sources exist; parity assertion is conditional and does not imply PUS compliance | SIL | individual | covered (configured build) |
| N22-TS1-95 | SRS-REQ-065 | PUSopen adapter seam against controlled API double | nominal + OOB | `test_pusopen` exercises init, TM encode mapping/output bounds/error, TC decode type/APID/payload checks, null/short/malformed inputs and SDK return failures | expected `FF_Status_t`, APID, header fields, payload pointer/length and emitted bytes match assertions; test uses `tests/stubs/pusopen.h` and does not establish vendor SDK compatibility | SIL | individual | covered (API test double only) |
| N22-TS1-96 | SRS-REQ-003 | Maximum-size TC through the production command-ingest receive boundary | nominal | `test_r2_gap` encodes a 1024-octet payload to a 1037-octet packet; a controlled `FF_Link_t` supplies it to `FF_AppCmdIngestReceiveOne`, the exact helper used by `FF_AppCmdIngestMain` | receiver supplies `cap == FF_CCSDS_MAX_TC_PACKET`; ingest returns `FF_SUCCESS`; service-8/function-7 handler receives all 1023 argument octets byte-identically after the function-ID octet; test asserts encoded packet length equals `FF_CCSDS_MAX_TC_PACKET` | SIL | combined | host covered (FRICK-REVIEW-0316); no real backend/target claim |
| N22-TS1-34 | SRS-REQ-052 | `pus.c` / `pus_services.c` bounded registry saturation | OOB | register FF_PUS_MAX_SERVICES+1 (33rd) service; 17th HK report; 17th data provider; 17th parameter setter; 17th function; 9th memory region | **gap** — no clause states the saturation contract (return code and state) for the PUS registries. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NO_RESOURCE`, prior registrations unchanged. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-35 | SRS-REQ-006 | `pus.c` `FF_PusSendVerification` with an invalid stage value | OOB | stage = (FF_Pus_Stage_t)0 and (FF_Pus_Stage_t)3 | **gap** — no clause defines the invalid-stage contract. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT`, no TM emitted. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-36 | SRS-REQ-005 | `pus_services.c` undersized service payload → verification error code | OOB | TC 3,1 len 1; TC 6,5 len 5; TC 8,1 len 0; TC 20,1 len 0; all ack = ACCEPTANCE\|COMPLETION | **gap** — SRS-REQ-005 requires "the appropriate error code" for a malformed telecommand, but no clause maps an undersized PUS payload to a code; the baseline reports a single execution-failure code although `FF_PUS_FAIL_ILLEGAL_LENGTH` (2) exists. Open NC-EXT-04 (NC-EXT-47 §7.3). Uncontrolled observation only: (1,8) code 20. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-37 | SRS-REQ-012 | `pus_services.c` S17 ping echo (nominal) | nominal | TC (17,1) payload {01,02,03,04}, ack = ACCEPTANCE\|COMPLETION | one TM (17,2) whose payload is byte-identical to the TC user data (data_len 4) (SRS-REQ-012 + ICD §4) | SIL | individual | covered |
| N22-TS1-38 | SRS-REQ-012 | `pus_services.c` S17 ping payload boundaries (zero and max) | nominal | TC (17,1) with 0-octet payload; then with 1024-octet payload | TM (17,2) echoed with data_len 0 and 1024 respectively; the 1024-octet case yields a 1045-octet frame (≤ FF_PUS_MAX_TM_FRAME path) (SRS-REQ-012 "answered with the same data") | SIL | individual | covered |
| N22-TS1-39 | SRS-REQ-052 | `pus_services.c` S8 function invocation (nominal) | nominal | registered function id 5; TC (8,1) payload {05, AA, BB}, ack = ACCEPTANCE\|COMPLETION | handler invoked exactly once with args {AA, BB} and len 2; one TM (8,2) with payload[0] = 0x05 and payload[1] = 0 (success) (SRS-REQ-052 service surface) | SIL | individual | covered |
| N22-TS1-40 | SRS-REQ-005, SRS-REQ-052 | `pus_services.c` S8 unknown function id | OOB | TC (8,1) with function id 0x7F (unregistered), ack = ACCEPTANCE\|COMPLETION | `FF_NOT_FOUND`; no handler invoked; exactly one (1,8) code 20; no (8,2) (SRS-REQ-005 non-execution + SRS-REQ-052) | SIL | individual | covered |
| N22-TS1-41 | SRS-REQ-052 | `pus_services.c` S8 function-execution report on handler failure | OOB | registered handler returning `FF_ERROR`; TC (8,1) id 5 args empty, ack = COMPLETION | **gap** — ICD §4 lists S8 as "TC (dispatch) … function id" only; it does not define the (8,2) reply or its status octet, and SRS-REQ-052 does not fix the failure encoding. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: (8,2) with status byte 1 plus (1,8) code 20. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-42 | SRS-REQ-008 | `pus_services.c` S3 housekeeping lifecycle (create/report/enable/disable/delete) | nominal | providers registered for report id 1; TC 3,1 {01,02}; 3,27; 3,4; 3,3; 3,2; each with ack = ACCEPTANCE\|COMPLETION | 3,1 = `FF_SUCCESS`; report (3,27) while enabled emits one TM (3,25); after 3,4 (disable) the report emits none; after 3,3 (enable) it emits again; 3,2 deletes so a later report emits none (SRS-REQ-008) | SIL | individual | covered |
| N22-TS1-43 | SRS-REQ-008 | `pus_services.c` S3 report content (report id + provider data) | nominal | report id 1 with provider writing 3 octets {p0,p1,p2}; TC 3,27 {01} | one TM (3,25) with 4-octet payload = {0x01, p0, p1, p2} (report id followed by the provider data), total frame 25 octets (ICD §4 + SRS-REQ-008) | SIL | individual | covered |
| N22-TS1-44 | SRS-REQ-008 | `pus_services.c` S3 error semantics (duplicate/unknown/full/disabled) | OOB | duplicate TC 3,1 with the same report id; 3,2/3,3/3,4 with an unknown id; a 17th report creation; 3,27 for an unknown report id | **gap** — SRS-REQ-008 requires create/enable/disable/remove/report but defines no outcome for duplicate, unknown or capacity-exhausted report ids. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_ALREADY_EXISTS`, `FF_NOT_FOUND`, `FF_NO_RESOURCE`; a report request for an unknown id returns success with no TM. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-45 | SRS-REQ-009 | `pus_services.c` S6 memory dump inside a registered region (nominal) | nominal | region registered over a 64-octet buffer; TC (6,5) payload addr = base, size = 4, ack = ACCEPTANCE\|COMPLETION | `FF_SUCCESS`; one TM (6,6) whose payload is the 4 octets at the requested address (SRS-REQ-009 + ICD §4 addr[4]+size[2] big-endian) | SIL | individual | covered |
| N22-TS1-46 | SRS-REQ-009 | `pus_services.c` S6 in-region boundaries (first byte, last byte, whole region) | nominal | (addr = base, size = 64); (addr = base+63, size = 1); (addr = base, size = 64) | each request returns `FF_SUCCESS` and one TM (6,6) whose payload equals the corresponding in-region octets (SRS-REQ-009 "only inside registered regions"; values at base-1 and base+64 are the excluded neighbours) | SIL | individual | covered |
| N22-TS1-47 | SRS-REQ-009 | `pus_services.c` S6 zero-length dump at the region end boundary | OOB | addr = base + region_size, size = 0 | **gap** — SRS-REQ-009 bounds access to "inside registered memory regions"; a zero-length access exactly at the excluded end address is neither inside nor outside by the clause text. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: accepted with a zero-length (6,6) TM. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-48 | SRS-REQ-009 | `pus_services.c` S6 out-of-region request | OOB | (addr = base−1, size 1); (addr = base+64, size 1); (addr = 0, size 1); (addr = base, size = 65); ack = COMPLETION | `FF_ERROR`; no (6,6) TM; exactly one (1,8) code 20 (SRS-REQ-009 region bound; SRS-REQ-005 failure reporting) | SIL | individual | covered |
| N22-TS1-49 | SRS-REQ-009 | `pus_services.c` S6 over-max size and invalid region registration | OOB | size = FF_PUS_MAX_TM_FRAME+1 (1025); `FF_PusRegisterMemoryRegion(NULL, 1)`, `(base, 0)`, `(base, 0xFFFFFFFF)` | **gap** — no clause defines the over-max dump-size outcome or the contract for a NULL/zero/overflowing region registration. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_BAD_SIZE`; `FF_INVALID_ARGUMENT` for NULL/0. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-50 | SRS-REQ-010 | `pus_services.c` S9,1 set time (6-octet on-wire T-field) | nominal | TC (9,1) payload coarse = 5, fine = 0 (6 octets), ack = ACCEPTANCE\|COMPLETION | `FF_SUCCESS`; exactly one (1,1) and one (1,7); `FF_TimeGetCuc` coarse = 5; source implicitly becomes `FF_TIME_SRC_GROUND` (SRS-REQ-010 + ICD §5) | SIL | individual | covered |
| N22-TS1-51 | SRS-REQ-024, SRS-REQ-010 | `pus_services.c` S9,1 wrong T-field length (5 and 7 octets) | OOB | TC (9,1) with 5-octet payload; then 7-octet payload beginning with P-field 0x2E (self-describing form) | `FF_BAD_SIZE` for both; time unchanged (coarse still 5); (1,8) emitted; the leading 0x2E is never mis-parsed as the coarse MSB (SRS-REQ-024(b) + ICD §5 explicit "accept only in its exact 6-octet length") | SIL | individual | covered |
| N22-TS1-52 | SRS-REQ-010 | `pus_services.c` S9,2 report time | nominal | TC (9,2) with empty payload, ack = ACCEPTANCE\|COMPLETION | one TM (9,2) with a 6-octet payload = current CUC T-field (coarse 4 octets BE at payload[0..3], fine 2 octets BE at payload[4..5]) equal to `FF_TimeGetCuc` at the same instant (ICD §5 + SRS-REQ-010) | SIL | individual | covered |
| N22-TS1-53 | SRS-REQ-010, SRS-REQ-024 | `pus_services.c` S9,1 maximum coarse/fine boundary | nominal | TC (9,1) payload coarse = 0xFFFFFFFF, fine = 0xFFFF | `FF_SUCCESS`; exported `FF_TimeGetCuc` coarse = 0xFFFFFFFF and fine = the truncation of the 2^-16 fraction (≤ 0xFFFF, no wraparound); 4-octet coarse / 2-octet fine field widths per ICD §5 | SIL | individual | covered |
| N22-TS1-54 | SRS-REQ-024 | time base CUC coarse wrap with a non-zero leap offset | OOB | `FF_TimeSetUtcNs(0xFFFFFFFF × 10^9)` with leap_seconds = 1; then `FF_TimeGetCuc` / `FF_TimeCucEncode` | **gap** — SRS-REQ-024 defines coarse as `utc_ns/10^9 + leap_seconds` in a 4-octet field but defines no saturation/overflow behaviour at the field end. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the exported coarse wraps modulo 2^32. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-55 | SRS-REQ-010 | `pus_services.c` S9,2 with an unexpected TC payload | OOB | TC (9,2) carrying a non-empty payload (ignored by the handler) | **gap** — ICD §5 defines only the reply content of 9,2; no clause fixes the treatment of a 9,2 TC payload. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: payload ignored, normal (9,2) reply. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-56 | SRS-REQ-052 | `pus_services.c` S20 parameter set/report (nominal) | nominal | setter and provider registered for parameter id 7; TC (20,1) {07, A1, A2}; TC (20,3) {07}; ack = ACCEPTANCE\|COMPLETION | 20,1 invokes the setter once with (7, {A1,A2}); 20,3 emits one TM (20,4) with payload = {0x07, provider octets} (SRS-REQ-052 service surface + ICD §4 on-board parameters) | SIL | individual | covered |
| N22-TS1-57 | SRS-REQ-005, SRS-REQ-052 | `pus_services.c` S20 unknown parameter id | OOB | TC (20,1) {0x7F, A1}; TC (20,3) {0x7F}; ack = ACCEPTANCE\|COMPLETION | `FF_NOT_FOUND`; setter/provider not invoked; no (20,4) TM; (1,8) code 20 per the SRS-REQ-005 non-execution principle | SIL | individual | covered |
| N22-TS1-58 | SRS-REQ-011 | `pus.c` `FF_PusEmitEvent` S5 forwarding (all severities) | nominal | `FF_PusEmitEvent` for `FF_EVS_INFO`, `FF_EVS_WARNING`, `FF_EVS_ERROR`, `FF_EVS_CRITICAL` with an event record (id 2 octets + type + text) | one TM per event, service 5, with the event payload (id, type, text) present in the TM user data; no event dropped (SRS-REQ-011 + ICD §4) | SIL | individual | covered |
| N22-TS1-59 | SRS-REQ-011 | `pus.c` S5 severity → subservice mapping and oversized event payload | OOB | evs_type = 1/2/3/4 and out-of-range (0, 99); payload len = FF_PUS_MAX_TM_FRAME−2 (1022) | **gap** — no requirement/ICD clause fixes the severity → S5 subservice mapping, the handling of an out-of-range severity, or the over-limit event payload outcome. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: subservices 1/2/3 with any other value defaulting to 3; `FF_BAD_SIZE` above 1021. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-60 | SRS-REQ-052 | `pus_services.c` S11 scheduling service behaviour | OOB | TC (11,1) and TC (11,2), ack = ACCEPTANCE\|COMPLETION | **gap** — SRS-REQ-052 places S11 in the fundamental service set, but no scheduling behaviour or subservice semantics are delivered (stub only) and ICD §4 does not define S11. Open TBD (NC-EXT-47 §7.3, delivery tracked by M15). Uncontrolled observation only: `FF_NOT_IMPLEMENTED` and (1,8) code 20. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-61 | SRS-REQ-052 | `pus_services.c` S12 / S18 / S19 service surface absent | OOB | TC to S12 (monitoring/limit checking), S18 (OBCP state) and S19 (event-action), various subservices with acks | **gap** — SRS-REQ-052 lists S12/S18/S19 in the fundamental set, but no implementation or ICD clause defines their subservices or failure semantics (planned M15; S18 also tied to SRS-REQ-045/054). Open TBD (NC-EXT-47 §7.3). No oracle is asserted here | SIL | individual | gap |
| N22-TS1-62 | SRS-REQ-021 | `time.c` `FF_TimeInit` initial state | nominal | fresh `FF_TimeInit()` | source = `FF_TIME_SRC_INTERNAL`; leap seconds 0; pending leap 0; tone count 0; `FF_TimeGetCuc` coarse 0 at a zero UTC correlation (SRS-REQ-021) | SIL | individual | covered |
| N22-TS1-63 | SRS-REQ-021 | `time.c` time-source selection and reporting | nominal | `FF_TimeSetSource` = TONE, GROUND, INTERNAL in turn | every legal value returns `FF_SUCCESS` and is returned by `FF_TimeGetSource` (source selectable at run time and reportable, SRS-REQ-021) | SIL | individual | covered |
| N22-TS1-64 | SRS-REQ-021 | `time.c` exported CUC continuity at a source switch | nominal | GROUND correlation set; then `FF_TimeSetSource(TONE)` at a fresh tone edge | \|CUC_after − CUC_before\| ≤ 1 s; the exported CUC is continuous with the newly selected source (SRS-REQ-021 switch clause) | SIL | individual | covered |
| N22-TS1-65 | SRS-REQ-021 | `time.c` invalid source value | OOB | `FF_TimeSetSource((FF_TimeSource_t)99)` | `FF_INVALID_ARGUMENT`; source unchanged (only the three defined sources are selectable, SRS-REQ-021) | SIL | individual | covered |
| N22-TS1-66 | SRS-REQ-021, SRS-REQ-010 | `time.c` ground correlation implicitly selects the GROUND source | nominal | `FF_TimeSetUtcNs(10^9)` after init | `FF_SUCCESS`; `FF_TimeGetSource` = `FF_TIME_SRC_GROUND`; CUC coarse = 1 (SRS-REQ-021 + SRS-REQ-010 correlation settable from ground) | SIL | individual | covered |
| N22-TS1-67 | SRS-REQ-022 | `time.c` first tone latches the correlation and establishes the first second boundary | nominal | `FF_TimeToneSignal(t0)` on a fresh monotonic t0 with the correlation at C | `FF_SUCCESS`; tone count 1; with source TONE the exported CUC coarse = C + 1 s (latched correlation + 1 s, SRS-REQ-022) | SIL | individual | covered |
| N22-TS1-68 | SRS-REQ-022 | `time.c` subsequent tones advance exactly one tone second | nominal | N = 3 tones at strictly increasing monotonic times | tone count = 3 and strictly increasing; under TONE the exported coarse = tone second count + accumulated leap offset (SRS-REQ-022) | SIL | individual | covered |
| N22-TS1-69 | SRS-REQ-022 | `time.c` non-monotonic and duplicate tones rejected | OOB | tone at the same monotonic value as the previous edge; tone 1 ns earlier | `FF_INVALID_ARGUMENT` for both; tone count and exported CUC unchanged (SRS-REQ-022 "non-monotonic tones shall be rejected") | SIL | individual | covered |
| N22-TS1-70 | SRS-REQ-022 | `time.c` tone strictly one nanosecond after the previous edge | nominal | `FF_TimeToneSignal(t_prev + 1)` | `FF_SUCCESS` (the acceptance rule is strict increase; no minimum spacing is required by the clause) (SRS-REQ-022) | SIL | individual | covered |
| N22-TS1-71 | SRS-REQ-022 | `time.c` tone received while the active source is not TONE | OOB | source = INTERNAL (or GROUND); valid tone at a fresh monotonic time | **gap** — SRS-REQ-022 governs tone discipline but does not state whether tones arriving under a non-TONE source must advance or be ignored. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: tone state advances regardless of the active source. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-72 | SRS-REQ-023 | `time.c` scheduled +1 leap applied at the first valid tone | nominal | `FF_TimeScheduleLeap(1)`, then `FF_TimeToneSignal` | pending apparent immediately (pending = 1); before the tone leap_seconds and exported CUC unchanged (inert); at the tone leap_seconds = 1, pending = 0, and the exported coarse includes the leap (SRS-REQ-023) | SIL | individual | covered |
| N22-TS1-73 | SRS-REQ-023 | `time.c` scheduled −1 leap clears at the tone (skipped second) | nominal | `FF_TimeScheduleLeap(-1)`, then a tone under TONE source | at the tone leap_seconds = −1 and the exported coarse does not advance (the second is skipped) (SRS-REQ-023 single-second change) | SIL | individual | covered |
| N22-TS1-74 | SRS-REQ-023 | `time.c` leap magnitude beyond one second | OOB | `FF_TimeScheduleLeap(+2)` and `FF_TimeScheduleLeap(-2)` | `FF_INVALID_ARGUMENT`; previous pending value unchanged (SRS-REQ-023 "single second at a time", `FF_TIME_CUC_MAX_LEAP_S` = 1) | SIL | individual | covered |
| N22-TS1-75 | SRS-REQ-023 | `time.c` pending leap with no tone ever arriving | OOB | `FF_TimeScheduleLeap(1)` then never signal a tone; observe over time | leap_seconds stays 0; exported CUC stays unchanged; pending stays 1 (SRS-REQ-023: "if no valid tone ever arrives the pending leap shall remain pending and shall never be applied") | SIL | individual | covered |
| N22-TS1-76 | SRS-REQ-023 | `time.c` leap re-scheduling before a tone and the delta = 0 case | OOB | `FF_TimeScheduleLeap(1)` then `FF_TimeScheduleLeap(-1)` before any tone; separately `FF_TimeScheduleLeap(0)` after a pending +1 | **gap** — SRS-REQ-023 fixes only single-second magnitude, application-at-tone and never-applied-without-tone; overwrite semantics and the meaning of delta 0 (cancel vs no-op) are undefined. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the last scheduled value replaces the pending one; 0 clears it. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-77 | SRS-REQ-021 | `time.c` exported CUC seconds do not regress across a tone boundary carrying a non-negative leap | nominal | tone under TONE source; then `FF_TimeScheduleLeap(1)`; then the next tone | exported coarse after the boundary ≥ coarse before it (no regression); the leap is cleared at the boundary (SRS-REQ-021 non-regression clause; Lean `req_srs021_utc_non_regression`) | SIL | individual | covered |
| N22-TS1-78 | SRS-REQ-024 | `time.c` CUC 7-octet self-describing encode/decode (lossless) | nominal | `FF_TimeCucEncode(buf,&len)` at a fixed UTC anchor; then `FF_TimeCucDecode` | len = `FF_TIME_CUC_LEN` = 7; buf[0] = `FF_TIME_CUC_PFIELD` 0x2E; coarse 4 octets BE then fine 2 octets BE; decode at the same instant recovers the anchored coarse and the fine value with no loss (SRS-REQ-024(a)) | SIL | individual | covered |
| N22-TS1-79 | SRS-REQ-024 | `time.c` CUC decode wrong length | OOB | len = 0, 6, 8 against a valid 7-octet buffer | `FF_BAD_SIZE` for every wrong length (SRS-REQ-024(a) "shall reject wrong-length inputs") | SIL | individual | covered |
| N22-TS1-80 | SRS-REQ-024 | `time.c` CUC decode wrong P-field | OOB | buf[0] = 0x00, 0x2D, 0xFF | `FF_PARSE_ERROR` for every wrong P-field (SRS-REQ-024(a) "unknown P-field with FF_PARSE_ERROR") | SIL | individual | covered |
| N22-TS1-81 | SRS-REQ-024 | `time.c` CUC codec NULL arguments | OOB | `FF_TimeCucEncode(NULL,&len)`, `(buf,NULL)`; `FF_TimeCucDecode(NULL,7,…)`, `(buf,7,NULL,…)`, `(buf,7,&c,NULL)` | **gap** — no SRS/ICD clause defines the NULL-argument contract of the CUC codec. Open TBD (NC-EXT-47 §7.2/§7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS1-82 | SRS-REQ-024 | `time.c` CUC fine fraction bound (maximum representable fraction) | nominal | encode at a UTC whose fraction is 999 999 999 ns | computed fine = 65535 (floor), never 65536 or wrapped; the 2-octet field is not exceeded (SRS-REQ-024, Lean `req_srs024_fine_frac_bounded`) | SIL | individual | covered |
| N22-TS1-83 | SRS-REQ-024 | `time.c` CUC coarse byte decomposition (maximum value) | nominal | decode {0x2E, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF} | coarse = 0xFFFFFFFF, fine = 0xFFFF; big-endian byte order per ICD §5 (Lean `req_srs024_coarse_byte_decompose`) | SIL | individual | covered |
| N22-TS1-84 | SRS-REQ-003, SRS-REQ-006 | CCSDS ↔ PUS end-to-end TC→TM round trip with the full S1 lifecycle | nominal | TC (17,1) built by `FF_CcsdsEncodeTc` with ack = ACCEPTANCE\|START\|COMPLETION; TM sink captures the stream; every TM decoded by `FF_CcsdsDecodeTm` | exactly 4 TM packets in order (1,1), (1,3), (1,7), (17,2); each TM decodes with the requested service/subservice at absolute octets 7/8, a well-formed 13-octet secondary header, counters strictly increasing by 1 mod 16384, and CUC fields present (SRS-REQ-003/006/007 + ICD §3) | SIL | combined | covered |
| N22-TS1-85 | SRS-REQ-010, SRS-REQ-024 | TM CUC ≡ S9,2 payload ≡ time-base export consistency | nominal | at one instant: emit a TM via any service, issue TC (9,2), and call `FF_TimeGetCuc` | the 6-octet T-field at TM absolute octets 13..18, the (9,2) reply payload, and the `FF_TimeGetCuc` coarse/fine agree (coarse 4 octets BE, fine 2 octets BE); the 6-octet on-wire form and the 7-octet codec form describe the same instant (ICD §5, SRS-REQ-024) | SIL | combined | covered |
| N22-TS1-86 | SRS-REQ-011 | EVS event → PUS S5 → CCSDS TM decode | nominal | EVS listener wired to `FF_PusEmitEvent` as in `fsw_main.c`; raise an `FF_EVS_ERROR` event | one TM decoded by `FF_CcsdsDecodeTm` with service 5 and a payload whose type octet = `FF_EVS_ERROR`; the event id and text are preserved (SRS-REQ-011 + ICD §4) | SIL | combined | covered |
| N22-TS1-87 | SRS-REQ-008 | S3 housekeeping report end-to-end to a decoded TM | nominal | create report id 1 with a provider writing 3 octets; command 3,27; decode the emitted TM | one TM (3,25) decoding to payload {report id, p0, p1, p2}; SRS-REQ-008 + ICD §4 (report id + data) | SIL | combined | covered |
| N22-TS1-88 | SRS-REQ-009 | S6 memory dump end-to-end to a decoded TM | nominal | registered region with a known pattern; dump addr = base, size = 8; decode the reply | one TM (6,6) whose decoded payload equals the 8 region octets (SRS-REQ-009 + ICD §4) | SIL | combined | covered |
| N22-TS1-89 | SRS-REQ-010, SRS-REQ-021 | S9,1 set time then the next emitted TM reflects it | nominal | TC (9,1) coarse = 100, fine = 0; then TC (17,1) to force a TM; decode that TM | source = GROUND; the decoded TM CUC coarse = 100 (TM secondary-header T-field tracks the correlation, ICD §5) | SIL | combined | covered |
| N22-TS1-90 | SRS-REQ-007 | Mixed-service burst: counter monotonicity and frame integrity across services | nominal | one run issuing TC 17,1; 9,2; 3,27; 20,3 (all ack = ACCEPTANCE\|COMPLETION); decode all captured TMs | every emitted TM decodes, has a well-formed primary header and 13-octet secondary header, and the counter field is strictly increasing by 1 mod 16384 across the whole burst with no duplicates or gaps (SRS-REQ-007 + ICD §3) | SIL | combined | covered |
| N22-TS1-91 | SRS-REQ-003, SRS-REQ-005 | Malformed-PDL telecommand: decoder rejection with no execution and no service TM | OOB | valid S17,1 frame with the PDL field rewritten to declare fewer octets than supplied (coalesced/trailing octets); CRC left consistent with the declared length | `FF_PARSE_ERROR`; no (17,2); no (1,1); handler not executed (SRS-REQ-003 exact-boundary rejection + SRS-REQ-005 non-execution) | SIL | combined | covered |
| N22-TS1-92 | SRS-REQ-006, SRS-REQ-044 | CCSDS + PUS protocol flow on the emulated PolarFire SoC (Renode) | nominal | Renode `polarfire-soc` run of `app_renode_test` exercising the protocol core (encode/decode, S1 lifecycle, CUC) | **gap** — the Renode test application exercises SB/EVS/Time-monotonic only; it does not drive CCSDS/PUS/CUC, and the `RENODE_TEST: PASS` aggregate marker is not a per-requirement oracle. Open NC-EXT-47 §7.6 (target evidence §7.5 / GAP-A5 for the protocol path). No oracle is asserted here | PIL | combined | gap |
| N22-TS1-93 | SRS-REQ-005, SRS-REQ-006 | On-target link-level PUS-C TM/TC (HIL) | nominal | target RTPFS160ZT, RTEMS 6.2, PUS-C TC in / TM out over the flight link (UART and mission link profile) | **gap** — on-silicon PUS-C link behaviour, timing and electrical integrity require flight hardware; GAP-A5 / NC-EXT-31 (NC-EXT-47 §7.5). No oracle is asserted here | HIL | combined | gap |
| N22-TS1-94 | SRS-REQ-021, SRS-REQ-022 | On-target 1 Hz/PPS tone capture and CUC epoch correlation (HIL) | nominal | target PPS/GPIO capture ISR deferring to a task feeding `FF_TimeToneSignal`; mission epoch correlation | **gap** — GAP-A7: on-target PPS capture and a CCSDS 1958/TAI epoch correlation with a leap-second table require flight hardware and an on-board campaign (NC-EXT-47 §7.5). No oracle is asserted here | HIL | individual | gap |

## 3. Gaps (mapping to NC-EXT-47 sub-items)

All gaps are recorded, not resolved (N21). Each row below maps a `gap` case to
the NC-EXT-47 sub-item in FRICK-VV-TSPEC-001 §7 that it belongs to, and names
the specific open NC where one exists. Gap rows assert **no** oracle.

**NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause
(unresolved/absent contract).**

- `N22-TS1-6` — encoder over-max payload and failure atomicity: no clause
  (TBD; NC-EXT-47 §7.3).
- `N22-TS1-15` — APID outside the 11-bit field: primary-header acceptance
  policy open (NC-EXT-13; NC-EXT-47 §7.3).
- `N22-TS1-16` — received sequence-count/seq-flag policy: emitted as zero
  pending profile decision (NC-EXT-06, NC-EXT-13; NC-EXT-47 §7.3).
- `N22-TS1-21` — failure-code selection for an unknown non-zero service
  (NC-EXT-04; NC-EXT-47 §7.3).
- `N22-TS1-22` — unknown service/subservice with no ack flags vs the
  unconditional completion-failure wording of SRS-REQ-005 (NC-EXT-04;
  NC-EXT-47 §7.3).
- `N22-TS1-25` — malformed-TC reporting conflict: S1 completion failure
  (SRS-REQ-005) vs EVS/S5 only (baseline/`test_pus_malformed_tc_event`)
  (NC-EXT-04; NC-EXT-47 §7.3).
- `N22-TS1-30` — PUS emitter over-max payload (TBD; NC-EXT-47 §7.3).
- `N22-TS1-31` — PUS emitter before a backend is selected (TBD;
  NC-EXT-47 §7.3).
- `N22-TS1-34` — PUS registry saturation contract (TBD; NC-EXT-47 §7.3).
- `N22-TS1-35` — `FF_PusSendVerification` invalid stage (TBD;
  NC-EXT-47 §7.3).
- `N22-TS1-36` — undersized service payload → "appropriate error code"
  (NC-EXT-04; NC-EXT-47 §7.3).
- `N22-TS1-41` — S8 execution-report TM and failure encoding not in ICD §4
  (TBD; NC-EXT-47 §7.3).
- `N22-TS1-44` — S3 duplicate/unknown/disabled/capacity semantics (TBD;
  NC-EXT-47 §7.3).
- `N22-TS1-47` — S6 zero-length dump exactly at the region end (TBD;
  NC-EXT-47 §7.3).
- `N22-TS1-49` — S6 over-max dump size and invalid region registration (TBD;
  NC-EXT-47 §7.3).
- `N22-TS1-54` — CUC coarse wrap at the 4-octet field end with a non-zero
  leap offset (TBD; NC-EXT-47 §7.3).
- `N22-TS1-55` — S9,2 with an unexpected TC payload (TBD; NC-EXT-47 §7.3).
- `N22-TS1-59` — S5 severity→subservice mapping, out-of-range severity and
  over-limit event payload (TBD; NC-EXT-47 §7.3).
- `N22-TS1-60` — S11 scheduling behaviour in the SRS-REQ-052 set not
  delivered (stub only) (TBD; NC-EXT-47 §7.3, M15).
- `N22-TS1-61` — S12/S18/S19 service surfaces in the SRS-REQ-052 set not
  delivered (TBD; NC-EXT-47 §7.3, M15).
- `N22-TS1-71` — tone arriving under a non-TONE source (TBD;
  NC-EXT-47 §7.3).
- `N22-TS1-76` — leap re-scheduling/overwrite and delta-0 semantics (TBD;
  NC-EXT-47 §7.3).

**NC-EXT-47 §7.2 — software-only robustness oracles not specified as a
controlled, requirement-mapped set.**

- `N22-TS1-12` — `FF_CcsdsDecode*` NULL buffer (TBD; NC-EXT-47 §7.2/§7.3).
- `N22-TS1-32` — `FF_PusSetBackend`/`FF_PusSendTm` invalid arguments (TBD;
  NC-EXT-47 §7.2/§7.3).
- `N22-TS1-81` — `FF_TimeCucEncode/Decode` NULL arguments (TBD;
  NC-EXT-47 §7.2/§7.3).

**NC-EXT-47 §7.6 — emulated-target aggregate marker is not a per-requirement
oracle.**

- `N22-TS1-92` — CCSDS+PUS protocol flow on Renode is not exercised by the
  current test application; `RENODE_TEST: PASS` does not evidence
  SRS-REQ-003/006/024 (TBD; NC-EXT-47 §7.6).

**NC-EXT-47 §7.5 — target (HIL) evidence remains GAP-A5 / NC-EXT-31.**

- `N22-TS1-93` — on-target link-level PUS-C TM/TC (GAP-A5 / NC-EXT-31).
- `N22-TS1-94` — on-target PPS capture and CUC epoch correlation (GAP-A7).

### 3.1 Coverage summary

| Metric | Count |
| --- | --- |
| Total cases | 94 |
| Nominal | 48 |
| Out-of-bound | 46 |
| Individual | 84 |
| Combined | 10 |
| `covered` (oracle grounded in a requirement/interface clause) | 66 |
| `gap` (no controlled oracle; recorded under NC-EXT-47) | 28 |
| SIL / PIL / HIL | 91 / 1 / 2 |

Requirement coverage in this scope: `SRS-REQ-003/004/005/006/007/008/009/010/
011/012/017/018/021/022/023/024/044/050/052` all have at least one case; every
implemented service, subservice, ingress path, egress path and the CUC codec
have nominal and (where a contract exists) out-of-bound cases.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial TS-1 work package per N22 and the FRICK-VV-TSPEC-002 chunk plan: nominal + out-of-bound case table with explicit oracles for the CCSDS Space Packet codec, the PUS-C core and implemented services (1/3/5/6/8/9/17/20), and the time/CUC base; gaps mapped to NC-EXT-47 sub-items with the open NC/TBD named |
| 0.2 | 2026-10-15 | frick-fsw | Align SRS-REQ-050 with open-default/optional-N7 decision and add the controlled PUSopen adapter seam test case; vendor compatibility remains unverified |
| 0.3 | 2026-10-15 | frick-fsw | Resolve test-case ID collision by assigning PUSopen case N22-TS1-95 and update the documented case range; uniqueness scan passes |
| 0.4 | 2026-10-15 | frick-fsw | Add maximum-size CCSDS TC ingest-to-service-dispatch case N22-TS1-96 for NC-EXT-02; retain independent review as open |
| 0.5 | 2026-10-15 | frick-fsw | Record independent V&V acceptance of the initial N22-TS1-96 codec-to-dispatch scope; a later change extends it through the production receive helper |
| 0.6 | 2026-10-15 | frick-fsw | Align TC/TM PDL boundary descriptions with symmetric overflow, truncation, exact-boundary, and output-preservation assertions; update candidate effectivity for current source deltas |
| 0.7 | 2026-10-15 | frick-fsw | Record independent V&V 0316 of symmetric TC/TM tests and production ingest-helper maximum-boundary test; retain target limits |
| 0.8 | 2026-10-15 | frick-fsw | Update candidate effectivity after the command-ingest helper and symmetric decoder-boundary deltas; keep earlier case baselines point-in-time |
