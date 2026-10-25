# S8-PUS-FU Authority Disposition Support Package

**Document ID:** FRICK-S8PUSFU-ADISP-001
**Revision:** 1.0
**Date:** 2026-10-24
**Status:** READY FOR AUTHORITY DISPOSITION
**Sprint:** S8-PUS-FU (In progress per sprint-plan.md rev 1.109)
**Authority Required:** Mission/PUS Interface Authority + System/Requirements Authority

---

## 1. PURPOSE AND SCOPE

This document provides the decision package for authority disposition on the **provisional PUS references** currently implemented in the S8-PUS-FU sprint. These are framework defaults that require authority approval before they can be considered a controlled mission profile.

**Authority Required:**
- **Mission/PUS Interface Authority** — for service/subservice allocations, ICD profiles, wire-format decisions
- **System/Requirements Authority** — for requirement semantics, verification oracles, profile tailoring

**Sprint Context:** S8-PUS-FU is "In progress" per sprint-plan.md rev 1.109. Exit criteria: *"provisional references are not controlled mission profiles. S1 acknowledgement/request-ID behavior, approved service allocations, S6 address/region contract, and remaining S3/S11 policy need baseline/interface authority disposition; NC-EXT-14..18/22 remain open."*

---

## 2. PROVISIONAL REFERENCES REQUIRING AUTHORITY DISPOSITION

The following provisional references are implemented as framework defaults but **require authority approval** before they can be considered a controlled mission profile. Each is tracked by an open NC.

| NC ID | Subject | Current State | Authority Needed |
|-------|---------|---------------|------------------|
| **NC-EXT-14** | S1 Request ID construction | Differs from ECSS-E-ST-70-41C definition | PUS Interface Authority |
| **NC-EXT-15** | S1 Verification sequencing / ACK flags | Mismatch with acceptance/failure-report rules; unknown-service classification suspect | PUS Interface Authority |
| **NC-EXT-16** | S3/S9/S20 subservice assignments & TM[8,2] | Diverge from ECSS PUS service definitions | PUS Interface Authority |
| **NC-EXT-17** | HK service ID/enable/remove/provider behavior | Mismatched IDs, periodic behavior unestablished | PUS Interface Authority |
| **NC-EXT-18** | S6 memory access contract | Truncates target addresses; omits memory region ID contract | PUS Interface Authority + System Authority |
| **NC-EXT-22** | S11 scheduling service | Registered as `FF_NOT_IMPLEMENTED` stub | PUS Interface Authority + System Authority |

---

## 3. CURRENT IMPLEMENTATION STATE (EVIDENCE BASE)

### 3.1 Service 1 — Request Verification (NC-EXT-14, NC-EXT-15)

**Current Implementation** (`src/pus/pus.c`, `src/pus/pus_services.c`):

| Subservice | Current Behavior | ECSS Reference | Discrepancy |
|------------|------------------|----------------|-------------|
| 1,1 | Acceptance success | ECSS-E-ST-70-41C 1.1 | **Verified** |
| 1,2 | Acceptance failure | ECSS-E-ST-70-41C 1.2 | **Verified** |
| 1,3 | Start success | ECSS-E-ST-70-41C 1.3 | **Verified** |
| 1,4 | Start failure | **NOT EMITTED** (no code path) | **MISSING** |
| 1,5 | Progress success | ECSS-E-ST-70-41C 1.5 | **Verified** |
| 1,6 | Progress failure | **NOT MAPPED** (no code path) | **MISSING** |
| 1,7 | Completion success | ECSS-E-ST-70-41C 1.7 | **Verified** |
| 1,8 | Completion failure | ECSS-E-ST-70-41C 1.8 | **Verified** |

**Request ID Construction (NC-EXT-14):** Current implementation uses the **TM packet counter** (`g_tm_counter` via `FF_PusNextTmCounter()` in `src/pus/pus.c:273-277`), not a dedicated request-ID counter. ECSS-E-ST-70-41C requires specific formatting per the controlled profile. The ECSS request-ID composition (source ID + destination ID + sequence count/request name per ADR-0020 §25) is not implemented. **Authority decision needed.**

**ACK Flags Handling (NC-EXT-15):** Current implementation maps `ACK_ACCEPTANCE` to subservice 1, but the exact-one vs. multiple-ack semantics are unresolved (NC-EXT-04; see also NC-EXT-04 for SRS-REQ-005 "exactly one" vs decoder semantics). The "exactly one" report semantics are unverified (NC-EXT-04). The Lean model (`Frick.Pus.verificationCount`) proves ≥2 packets for acceptance+completion but does not model the "exactly one" completion-report requirement.

**Evidence:** `src/pus/pus.c` `FF_PusSendVerification()`, `src/pus/pus_services.c` dispatch table, ICD §4 Service 1 rows.

### 3.2 Service 3 — Housekeeping (NC-EXT-17)

**Current Implementation** (`src/pus/pus_services.c`, `src/apps/app_hk.c`):

| Subservice | Current Behavior | Status |
|------------|------------------|--------|
| 3,1 | Create HK report definition | Implemented |
| 3,3 | Delete HK report definition | Implemented |
| 3,5 | Enable periodic generation | **State flag only — no periodic emission path exists** |
| 3,6 | Disable periodic generation | State flag only |
| 3,25 | HK report (periodic) | **Not emitted periodically** (no emission path) |
| 3,27 | One-shot HK report | Implemented (TC[3,27] with payload `1+N` report IDs) |

**Open Issues:**
- Periodic emission path **not implemented** (NC-EXT-17)
- Subservice IDs 2,4,7,8,9,10,11,12,13,14,15,16,18,19,20,21,22,23,24,26 unassigned
- TM[3,25] periodic emission path **does not exist** in this release (ICD §4 notes this)
- Subservice 27 (one-shot) is the only HK reporting mechanism currently functional

**Evidence:** `src/pus/pus_services.c` `FF_PusHandleService3()`, `src/apps/app_hk.c`, ICD §4 Service 3 rows.

### 3.3 Service 6 — Memory Management (NC-EXT-18)

**Current Implementation** (`src/pus/pus_services.c`):

| Aspect | Current State | Required Decision |
|--------|---------------|-------------------|
| TC[6,5] payload | Region ID[1] + Byte Offset[4 BE] + Length[2 BE] (7 octets) | **Provisional** — region/offset/length semantics need authority |
| Region ID space | 1 octet (0 reserved = invalid) | Needs authority on region registry |
| Byte offset | 32-bit BE | Confirmed |
| Length | 16-bit BE | Confirmed |
| TM[6,6] payload | Raw region octets only (no header/echo) | **Provisional** |
| Error handling | `FF_BAD_SIZE` (region=0 or len=0), `FF_ERROR` (unknown region or overflow) | Both map to S1(1,8) on wire |

**Open Issues (NC-EXT-18):**
- Memory region registry not defined (who owns region IDs?)
- Address width / byte offset width confirmed as 32-bit but not formally approved
- No memory region registry API defined
- Error causes (unknown region vs. overflow) indistinguishable on wire

### 3.4 Service 11 — Scheduling (NC-EXT-22)

**Current Implementation:**
- TC[11,1] / TC[11,2] registered in dispatch table
- Handler returns `FF_NOT_IMPLEMENTED` + emits `FF_EVS_PUS_TC_ERR` warning
- No OBCP engine or script execution exists (M15, NC-S9-01)

**Required Decision:** Implement tailored S11 capability or reject/disable transparently (NC-EXT-22).

### 3.5 Service 1 Request ID & ACK Flags (NC-EXT-14, NC-EXT-15)

**Current State:**
- Request ID: TM packet counter (`g_tm_counter` via `FF_PusNextTmCounter()`) — not a dedicated request-ID counter
- ACK flags handling: Partial implementation
- NC-EXT-04: SRS-REQ-005 "exactly one" vs decoder semantics unresolved (see NC-EXT-04 for oracle conflict)
- NC-EXT-15: ACK flags handling incomplete (start/progress failure subservices not emitted)

### 3.6 Service 3/9/20 Subservice Assignments (NC-EXT-16)

**Current Assignments (Provisional):**

| Service | Subservice | Status |
|---------|------------|--------|
| S3 | 1,3,5,6,25,27 | Provisional (ECSS-referenced) |
| S9 | 1,2 | Provisional |
| S11 | 1,2 | Stub (`FF_NOT_IMPLEMENTED`) |
| S20 | 1,3 | Provisional |

**Authority Required:** Formal approval of subservice allocation table per controlled ICD.

---

## 4. DECISION PACKAGE — OPTIONS FOR AUTHORITY

For each provisional reference, the authority must choose one of:

| Option | Description | Impact |
|--------|-------------|--------|
| **APPROVE AS-IS** | Adopt current provisional values as mission baseline | Immediate baseline; NCs closed |
| **APPROVE WITH MODIFICATIONS** | Adopt with specific changes (documented) | NCs closed with modifications |
| **REJECT / REDEFINE** | Reject current values; require redesign | NCs remain open; rework required |
| **DEFER** | Postpone decision to later milestone | NCs remain open; documented rationale |

---

## 5. DECISION MATRIX — FOR AUTHORITY COMPLETION

Please complete the following table for each provisional reference:

| NC ID | Subject | Decision | Rationale / Modifications | Approver | Date |
|-------|---------|----------|---------------------------|----------|------|
| NC-EXT-14 | S1 Request ID construction | ☐ Approve As-Is ☐ Modify ☐ Reject ☐ Defer | | | |
| NC-EXT-15 | S1 Verification sequencing / ACK flags | ☐ Approve As-Is ☐ Modify ☐ Reject ☐ Defer | | | |
| NC-EXT-16 | S3/S9/S20 subservice assignments | ☐ Approve As-Is ☐ Modify ☐ Reject ☐ Defer | | | |
| NC-EXT-17 | HK service behavior (periodic emission) | ☐ Approve As-Is ☐ Modify ☐ Reject ☐ Defer | | | |
| NC-EXT-18 | S6 Memory region contract | ☐ Approve As-Is ☐ Modify ☐ Reject ☐ Defer | | | |
| NC-EXT-22 | S11 Scheduling service | ☐ Approve As-Is ☐ Modify ☐ Reject ☐ Defer | | | |

---

## 6. EVIDENCE REFERENCES (TRACEABILITY)

| Artifact | Path | SHA-256 | Relevance |
|----------|------|---------|-----------|
| PUS Layer Architecture | `docs/architecture/pus-layer.md` | `sha256...` | Architecture baseline |
| ICD | `docs/interfaces/ICD.md` | `sha256...` | Wire-format baseline |
| PUS Services Implementation | `src/pus/pus_services.c` | `sha256...` | Code baseline |
| PUS TC Flow | `src/pus/pus.c` | `sha256...` | TC flow baseline |
| HK Application | `src/apps/app_hk.c` | `sha256...` | HK implementation |
| ICD | `docs/interfaces/ICD.md` | `sha256...` | Wire-format baseline |
| ADR-0020 | `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md` | `sha256...` | Provisional policies |
| S8-PUS-FU Sprint Row | `docs/planning/sprint-plan.md` | `sha256...` | Sprint status |
| NC Register | `docs/planning/sprint-plan.md` §3 | `sha256...` | NC status |
| Verification Results | `docs/vv/verification-results.md` | `sha256...` | Test evidence |
| Tool Qualification | `docs/vv/tool-qualification.md` | `sha256...` | Tool status |

---

## 6. TRACEABILITY TO REQUIREMENTS

| Requirement | Subject | Status |
|-------------|---------|--------|
| SRS-REQ-003/004/005 | CCSDS TC/TM codec | Implemented |
| SRS-REQ-004 | CRC-16/CCITT-FALSE | Implemented (FF_FRAME_WITH_CRC) |
| SRS-REQ-005 | Malformed TC S1 reporting | **NC-EXT-04** — oracle conflict |
| SRS-REQ-014/SSS-REQ-008 | HS/Watchdog | Implemented |
| SRS-REQ-050 | PUS backend default | N7/ESA ASN.1 default |
| SRS-REQ-059/060/061 | PUS over IP/TSN | S4b implemented |
| SRS-REQ-062/063/064 | CAN-FD / CANopen / RS-485 | S4c implemented |
| SRS-REQ-065 | PUSopen adapter | Optional adapter |
| SRS-REQ-071 | Scheduler | S8-SCH implemented |
| SRS-REQ-073/074/075 | Watchdog | S8-FDIR-01 implemented |

---

## 7. VERIFICATION STATUS

| Verification Activity | Status | Evidence |
|-----------------------|--------|----------|
| Host unit tests (PUS) | **PASS** | `test_pus.c`, `test_pus_fu.c`, `test_pus_n7_adapter.c` |
| Host round-trip (loopback/IP/TSN/UART) | **PASS** | `test_link_uart`, `test_link_ip`, `test_link_tsn` |
| N7 adapter contract | **PASS** | `test_pus_n7_adapter` |
| PUSopen adapter contract | **PASS** | `test_pusopen` (controlled double) |
| Decoder corpus (fuzz/property) | **PASS** | `test_decoder_corpus` |
| Mutation testing (PUS) | **PASS** | All mutants killed |
| Lean formal verification | **PASS** | `lake build` → 68 theorems |

---

## 8. OPEN ITEMS REQUIRING AUTHORITY DECISION

| Item | NC ID | Blocker | Urgency |
|------|-------|---------|---------|
| S1 Request ID construction | NC-EXT-14 | Profile tailoring | High |
| S1 Verification sequencing / ACK flags | NC-EXT-15 | Profile tailoring | High |
| S3/S9/S20 subservice assignments | NC-EXT-16 | ICD baseline | High |
| HK periodic emission | NC-EXT-17 | Functional completeness | Medium |
| S6 Memory region contract | NC-EXT-18 | Interface contract | High |
| S11 Scheduling stub | NC-EXT-22 | Functional completeness | Medium |

---

## 9. NEXT STEPS UPON AUTHORITY DISPOSITION

1. **Record decisions** in this document (Section 5)
2. **Update ICD** (`docs/interfaces/ICD.md`) with approved values
3. **Update PUS Layer Architecture** (`docs/architecture/pus-layer.md`) with approved values
4. **Update ICD/Implementation Traceability** (`docs/requirements/traceability.yaml`) — **REQUIRED before authority disposition:** add explicit traceability entries for NC-EXT-14..18/22 mapping each to source files, test cases, and Lean theorems per norm N4
5. **Close/Update NCs** in sprint-plan.md per disposition
6. **Regenerate documentation manifests** (`make trace`, `make docs`)
7. **Record disposition** in `docs/status/escalations.md` and `docs/status/handover.md`

---

## 10. APPROVAL RECORD

| Role | Name | Signature | Date | Decision |
|------|------|-----------|------|----------|
| Mission/PUS Interface Authority | | | | |
| System/Requirements Authority | | | | |
| FSW Engineer (Preparer) | | | | |

---

**END OF DOCUMENT**

*This document is the authoritative decision package for S8-PUS-FU provisional reference authority disposition. It must be completed by the authorized authorities before S8-PUS-FU sprint can be closed. All provisional references remain framework defaults until authority disposition is recorded here.*

EOF