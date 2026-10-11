# PUS-C From-Scratch Assessment (fundamental service set, flight-worthy)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0015                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-11                             |
| Status                            | engineering assessment (decision support); rev 0.1 - estimates labelled |
| Scope                             | Alternative to SRS-REQ-050: implement PUS-C (ECSS-E-ST-70-41C) from scratch |
| Target                            | ECSS criticality B, MISRA C:2012, AGPL-3.0-only, RTEMS 6.2 / RTPFS160ZT |
| Norms                             | N1, N3, N4, N5, N8, N9, N10, N11, N17  |
| Related                           | `docs/architecture/pus-layer.md`, `ADR-0003`, `ADR-0004`, `docs/status/loop-audit.md` |

## 1. Purpose

frick-fsw is a backend-agnostic PUS-C middleware (`src/pus/pus.h`,
`FF_Pus_Backend_t`). The project has decided (SRS-REQ-050, norm N19
reconciliation) to use the N7 Space / ESA `asn1-pusc-lib` ASN.1/ACN model,
ASN1SCC-compiled, as the **default** PUS-C basis. This document assesses the
**alternative**: implementing PUS-C **from scratch** with the **fundamental
services**, in a "truly flight-worthy" way. It is a decision-support document
for the integrator/PM: scope, effort, assurance, make-vs-buy, risks and a
milestone proposal. Effort figures are **estimates**; uncertainty is labelled.

## 2. What "fundamental" means for a criticality-B OBSW

A small/medium OBC needs a **capability set** that covers the command/data-handling
use cases the spacecraft cannot fly without, independently of mission equipment:
ground-commanded housekeeping, event visibility, telecommand verification, time,
memory/parameter access, on-board autonomy (scheduling, monitoring, event-action)
and test. Everything else (forwarding, large packets, on-board storage, file
management, parameter statistics) is mission tailoring and **out of the
fundamental set**. The table below fixes the scope; the "in scope" subservices
are the tailoring target of M15 (§9).

| Svc | Name | In scope (fundamental) | Out of scope / optional | Today | Verdict |
| --- | ---- | ---------------------- | ----------------------- | ----- | ------- |
| 1 | Request verification | TM acceptance/start/progress/completion incl. failures (1,2,3,4,5,6,7,8) + failure codes | 9,10 | partial (1,2,4,8) | **core** |
| 2 | Device command | TC/TM 1,2,4..12 (raw + wrap, device/composite) | — | missing | optional (reference, M13) |
| 3 | Housekeeping | 1,2,3,4,5,6,9,10,11,25,27,31 (structure admin, periodic generation, HK report) | 7,8,12,26,28..44 (advanced/derived) | partial | **in scope** |
| 4 | Parameter statistics | — | 1..9 | missing | out (mission) |
| 5 | Event reporting | 1..4 (info/low/warning/high), event definitions | 5..8 (custom/forwarding) | partial | **in scope** |
| 6 | Memory management | raw 1,2,3,5,6,9,11,12,13; structured 15,17,18,19,20,21 | 4,7,8,10,14,16,22 | partial (5,6) | **in scope** |
| 8 | Function management | 1 (direct function invocation) | — | partial | **in scope** |
| 9 | Time management | 1,2,3 (set time, report time, UTC correlation) | — | partial (1,2) | **in scope** |
| 11 | Time-based scheduling | 1..11, 15..18, 20..27 (schedule/sub-schedule/activity/group admin + report) | 12..14,19 | stub | **in scope** |
| 12 | On-board monitoring | 1..16, 24..28 (parameter/functional monitoring definitions, checks, transitions) | 17..23 | missing | **in scope** |
| 13 | Large packet | — | 1..3,9..11,16 | missing | out (or M4) |
| 14 | Real-time forwarding | — | all | missing | out |
| 15 | On-board storage & retrieval | — | all | missing | out (M4/M13) |
| 17 | Test | 1,2 (connection test / ping-pong) | 3,4 | partial | **in scope** |
| 18 | OBCP | management + execution surface 1..9,12..22 (PUS side; engine is M12) | — | missing | **in scope** (M12 surface) |
| 19 | Event-action | 1..11 (event-action definitions, enable/disable/report) | — | missing | **in scope** |
| 20 | On-board parameters | 1,2,3,4 (set, report/defaults, delete) | — | partial (1,3) | **in scope** |
| 23 | File management | — | all | missing | out (M4) |

Evidence for the enumeration: the pinned ASN.1 model
(`third_party/asn1-pusc-lib`, `info.json` v1.1.1) mirrors S1..S19 (312 `.asn1` +
311 `.acn`): S1 (10), S3 (32), S6 (22), S11 (27), S12 (28), S17 (4), S18 (20),
S19 (11) subservice definitions. **Note:** the model does **not** define S20
(on-board parameters) or S23 (file management) - those are mission/custom
subsets and must be designed in-house even in the reuse path.

## 3. What "flight-worthy" entails

### 3.1 ECSS-E-ST-70-41C tailoring and capability set
- A documented tailoring record (which services/subservices, which options, which
  P-fields, which TM/TC data fields), reviewed by R3, frozen in the ICD and
  `config/fsw_config.h`. The "capability set" is the agreed subset that ground
  and space segments both implement; deviations are explicit, never silent.
- Per-service **state models** must be specified (e.g. S11 schedule enable state,
  S12 monitor validity, S19 event-action state), not just packet layouts.

### 3.2 Packet / data model (CCSDS 133.0-B-2)
- Space Packet primary header (version 1, type, sec-hdr flag, APID, sequence
  flags/count, 16-bit PDL), PUS-C secondary header (TC 5 octets, TM 13 octets),
  mission CRC-16 trailer (SRS-REQ-004), endianness fixed big-endian.
- Bounded sizes: `FF_PUS_MAX_TM_FRAME`/`FF_PUS_MAX_TC_FRAME` (1024); overflow
  proven (SRS-REQ-003). No dynamic allocation (SSS-REQ-010/SRS-REQ-015).

### 3.3 TC acceptance / verification / execution
- S1 stages exactly once per acknowledged stage (SRS-REQ-006), unknown
  service/subservice and malformed frames rejected without execution
  (SRS-REQ-005), correct failure codes, illegal APID handling.

### 3.4 TM generation
- Service/subservice routing, monotonic 14-bit TM counter wrap (SRS-REQ-007),
  core-stamped CUC time, deterministic payload assembly, per-service report
  identifiers.

### 3.5 Service handover
- Typed dispatch `(service, subservice) -> handler` (today a bounded table in
  `pus.c`), plus a **datapool** that lets any service reference parameters by
  local/global identifier across applications (cFS-like app model). Handover of
  a TC to a mission application must be deterministic and bounded.

### 3.6 Parameter / datapool
- A static, bounded parameter registry: identifier, type, size, validity, access
  (R/W), engineering conversion. Backs S3, S12, S19, S20 and OBCP datapool.
  This is the single largest new in-house component.

### 3.7 Time correlation and PUS-C P-field
- Keep the CUC tailoring already frozen (ADR-0004, ICD §5): P-field `0x2E`,
  4-octet coarse + 2-octet fine, on-wire 6-octet T-field with implicit P-field
  and the explicit 7-octet form (SRS-REQ-024). From-scratch must implement
  S9,3 correlation (time offset + rate) and larger CUC counter resolution
  (48-bit) rather than 32-bit coarse.

### 3.8 On-board monitoring (S12) and event-action (S19)
- Limit checking (parameter monitoring: expected/limit ranges, validity,
  delta), functional monitoring, and transitions to actions; event-action
  triggers on-board actions from events with states and enable/disable. Both
  need a persistent parameter snapshot and a bounded action list.

### 3.9 Error handling
- Per-service error codes plus the S1 failure taxonomy; errors never truncate or
  corrupt packets; malformed/malicious input cannot cause out-of-bounds access
  (GAP-A2 malformed-frame matrix).

### 3.10 Determinism, resources, MISRA
- Static allocation, bounded loops/stack, no recursion, no VLAs; MISRA C:2012
  conformance with documented deviations (SRS-REQ-041); WCET/stack evidence is a
  class-A gap (GAP-A4).

## 4. Effort estimate (estimates - uncertainty around +/-40%)

Scope: the in-scope set of §2 (S1,3,5,6,8,9,11,12,17,18-surface,19,20 + core),
excluding the OBCP engine (M12) and S2/S13/S14/S15/S23. Split assumed
consistent with the project's norms (N1 Lean, N3/N4 requirements, N11 tests,
N9/N17 reviews): roughly **coding 40% / verification 40% / review 20%**, because
every increment re-runs the full cycle (N15). Basis: the current shallow
implementation is ~1.0 kLOC (`src/pus/*.c`); a flight-worthy fundamental set is
estimated at ~8-11 kLOC including the datapool and service state machines.

| Group | Content | Est. PM | Coding | Verification | Review |
| ----- | ------- | ------: | -----: | -----------: | -----: |
| A | PUS core: dispatch, S1, handover, datapool, tailoring, error taxonomy | 5.0 | 2.0 | 2.0 | 1.0 |
| B | S3 housekeeping (full fundamental) | 2.5 | 1.0 | 1.0 | 0.5 |
| C | S5 event reporting | 1.0 | 0.4 | 0.4 | 0.2 |
| D | S6 memory management (raw + structured) | 3.0 | 1.2 | 1.2 | 0.6 |
| E | S8 function management | 0.5 | 0.2 | 0.2 | 0.1 |
| F | S9 time management + correlation | 1.0 | 0.4 | 0.4 | 0.2 |
| G | S11 time-based scheduling | 4.0 | 1.6 | 1.6 | 0.8 |
| H | S12 on-board monitoring | 4.5 | 1.8 | 1.8 | 0.9 |
| I | S17 test | 0.5 | 0.2 | 0.2 | 0.1 |
| J | S18 OBCP PUS surface (engine excluded, M12) | 2.0 | 0.8 | 0.8 | 0.4 |
| K | S19 event-action | 3.0 | 1.2 | 1.2 | 0.6 |
| L | S20 parameter management | 2.5 | 1.0 | 1.0 | 0.5 |
| **Total (fundamental)** | | **29.5** | **11.8** | **11.8** | **5.9** |
| +15% contingency | | **~34** | | | |
| Optional add-ons | S2 device command 2.0; S13/S14/S15 6.0; S23 (M4) 4.0 | +12 | | | |

**Team assumptions (estimate):** 2-3 senior FSW engineers (protocol/state
machines), 1 dedicated V&V engineer with independent context (R2 role, N9), 0.5
systems engineer (R3/ICD tailoring) and PM/QA part time. Calendar at 2.5 FTE:
**~12 months nominal, 14-18 months with the N15 cycle overhead and reviews**.
This is delivery of the framework PUS layer only; mission apps are out of scope.

## 5. Verification and assurance (ECSS-Q-ST-80C)

- **Unit / service / subservice coverage.** Every in-scope subservice gets at
  least one nominal, one malformed and one boundary test; S1 gets a
  stage-exactly-once test; the datapool gets bounds/aliasing tests. Target:
  **100% of in-scope subservices** exercised, **statement >=95%** and
  **branch/decision >=90%** for `src/pus/` (today the whole repo is 65.9% line;
  GAP-A2 explicitly calls for per-service coverage).
- **Malformed-frame matrix.** Wrong length, bad CRC, wrong APID, wrong PUS
  version, unknown service/subservice, truncated secondary header, oversized
  PDL, bad P-field/length; plus a small deterministic fuzz corpus. Rejection
  must be total (no execution) and must emit S1 failure (SRS-REQ-005).
- **MISRA C:2012.** Required/Mandatory rules with a documented deviation process
  (`docs/quality/misra-c.md`); the in-house code is written to MISRA and scanned
  (`tools/scripts/misra.sh`). A certified checker and formal report remain
  **GAP-A4**.
- **Independence (N9/N14).** R1 design/code, R2 V&V, R3 SSS (N14), R4 docs
  (N17); findings closed by re-applying all norms (N12). Organisational
  independence remains **GAP-A3/A11**.
- **Formal (Lean, N1).** Model the pure logic of each service: dispatch and S1
  counter (already `Frick.Pus`), length/field-bound checks, datapool indexing,
  S11 schedule arithmetic, S12 limit-check transitions, S19 action dispatch.
  Requirement-level theorems linked in the manifest. Full C-to-Lean refinement
  (Frama-C/CBMC) is **GAP-A1**; this work **expands** the Lean surface and thus
  narrows GAP-A1 in scope but does not close it.
- **Static analysis / tools (N10).** New scripts/harnesses are V&V tools and must
  be qualified and registered (`docs/vv/tool-qualification.md`).
- **Applicable class-A gaps:** **GAP-A1** (deep formal / C refinement),
  **GAP-A2** (full PUS-C service coverage - this work is the closure path),
  **GAP-A4** (certified MISRA, WCET, stack depth); **GAP-A5** (on-target
  execution) and **GAP-A12** (mission OPD) remain outside this layer.

## 6. Make-vs-buy / reuse

| Dimension | (a) From scratch (in-house) | (b) N7/ESA `asn1-pusc-lib` + ASN1SCC (current decision) | (c) Commercial PUSopen(R) |
| --------- | --------------------------- | ------------------------------------------------------ | ------------------------- |
| Heritage | none (re-invention risk) | ESA-funded, standard-conformant model, ACN encoding | commercial, operational heritage |
| Control | total (wire + logic) | data model/vocabulary fixed by library; logic in-house | low (vendor library) |
| Cost | highest engineering (est. §4) | medium: toolchain integration + in-house logic | licence cost + integration |
| Qualification | all evidence ours | must qualify ASN1SCC + generated code (GAP-A1b) | vendor qualification, if usable |
| MISRA | clean if written so | generated code needs analysis/deviations | opaque, cannot be analysed |
| Maintainability | own code, full traceability | model updates = regenerate; less code to own | vendor releases only |
| Licensing | AGPL, ideal (N13) | GPL-3.0 combinable with AGPL (generated runtime exception); build tool not distributed | proprietary; never distributed; adapter only (ADR-0003) |
| Formal/Lean (N1) | fully modellable | model + generated code; logic ours | defeats the auditable/Lean default |
| Verdict | justified only for **service behaviour** | **best for the data/wire layer** | optional drop-in only |

## 7. Risks and mitigations

| Id | Risk | Likelihood/Impact | Mitigation |
| -- | ---- | ---------------- | ---------- |
| R1 | Under-estimated effort for S11/S12/S19 state machines | H / H | phase per service; reuse the ASN.1 model; contingency in §4 |
| R2 | Datapool scope creep into mission-specific semantics | M / M | fix a minimal typed datapool in M15; mission extends |
| R3 | Lean obligation (N1) grows faster than code | H / M | model only pure logic; `lean_note` with justification elsewhere |
| R4 | MISRA deviations explode in generated/advanced services | M / M | delegate wire types to `asn1-pusc-lib`; keep logic small |
| R5 | Divergence from the ASN.1 default (SRS-REQ-050) | M / H | keep `FF_Pus_Backend_t`; services independent of backend |
| R6 | No heritage defects surface late | M / H | protocol conformance vectors + ground GSW interop tests |
| R7 | On-target timing/WCET unknown (GAP-A4/A5) | M / M | bounded design; Renode (N18); WCET work package in GAP-A4 |

## 8. Recommendation

1. **Keep `asn1-pusc-lib` + ASN1SCC as the default data/wire layer**
   (SRS-REQ-050 stands). Do **not** re-implement the PUS/CCSDS byte codec: it is
   the tedious, conformance-critical part and the model already covers S1..S19.
2. **Build in-house only the service behaviour** - handlers, state machines,
   datapool, scheduling, monitoring, event-action - in MISRA-clean C behind the
   existing `FF_Pus_Backend_t`, so the implementation stays Lean-modellable and
   auditable (N1).
3. **Design S20 and S23 in-house** regardless of path (absent from the ASN.1
   model).
4. **Do not adopt PUSopen** as the default; keep it as the documented optional
   adapter (ADR-0003).
5. Net: "from scratch" is justified for the **fundamental service behaviour**,
   not for the wire/data model. This is the hybrid that gives heritage, control,
   MISRA cleanliness and Lean coverage at the lowest defensible cost.

## 9. Roadmap milestone proposal - M15

**M15 - In-house fundamental PUS-C service set** (planned; place after **M14**,
the current last milestone).

- **Objective (proposed OBJ-23):** implement, in MISRA-clean in-house C and
  behind `FF_Pus_Backend_t`, the **fundamental PUS-C service behaviour** for
  S1/3/5/6/8/9/11/12/17/18(PUS surface)/19/20, on the `asn1-pusc-lib` default
  data model, with a bounded on-board parameter datapool; cover every in-scope
  subservice with verification and requirement-level Lean theorems.
- **Deliverables:**
  - service handlers + state machines and the datapool (`src/pus/`);
  - tailoring/capability-set record (ICD + `config/fsw_config.h`) reviewed by R3;
  - per-subservice unit tests + malformed-frame matrix (targets of §5);
  - Lean models for the pure logic (dispatch, counters, bounds, S11/S12/S19
    arithmetic/transitions);
  - docs: `pus-layer.md`, `service-development.md`, ICD, `licensing.md`
    (any new dependency), CHANGELOG and per-doc History;
  - full cycle green (N15) incl. Renode functional validation (N18) and reviews
    N9/N17.
- **Exit criteria:** all in-scope subservices implemented and tested; statement
  >=95% / branch >=90% for `src/pus/`; MISRA scan green with documented
  deviations; Lean obligations linked with no silent waiver; traceability
  converged (N4); independent review APPROVED; SRS-REQ-052/SSS-REQ-025 verified.
- **Requirements added:** SRS-REQ-052 (fundamental PUS-C service set, method
  Tst | Anl) and SSS-REQ-025 (fundamental service capability set) - to be
  reviewed by R3 (N14).
- **Requirements / gaps closed or advanced:** narrows **GAP-A2** (full PUS-C
  coverage) to a closure candidate; refines SRS-REQ-008/009/010/011/012;
  supports SRS-REQ-050 (ASN.1 default) and SRS-REQ-045 (S18 OBCP surface);
  contributes Lean surface toward **GAP-A1**; depends on **M12** (OBCP engine,
  S18), **M13** (equipment/fault management use S2/S11/S12), **M9**
  (MISRA process) and **GAP-A4** for final assurance.

## 10. References

- ECSS-E-ST-70-41C (PUS-C); CCSDS 133.0-B-2; ECSS-Q-ST-80C; ECSS-E-ST-40C.
- `docs/architecture/pus-layer.md`, `docs/interfaces/ICD.md`, `ADR-0003`,
  `ADR-0004`, `docs/status/loop-audit.md`, `docs/quality/class-a-gaps.md`,
  `docs/requirements/{SSS,SRS,traceability}.md`.
- `src/pus/`, `third_party/asn1-pusc-lib` (v1.1.1), `docs/rules.md` (N1-N19).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | frick-fsw | Initial assessment: fundamental PUS-C service scope, flight-worthiness, effort (person-month), ECSS-Q-ST-80C assurance, make-vs-buy (scratch / N7-ESA ASN.1 / PUSopen), risks, hybrid recommendation and proposed M15 |
