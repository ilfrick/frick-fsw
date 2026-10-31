# OBCP Engine Comparison Study — Technical Survey for M12

| Field | Value |
| ----- | ----- |
| Document ID | FRICK-ARCH-OBCP-STUDY-001 |
| Revision | 0.5 |
| Date | 2026-10-31 |
| Status | Draft — Technical Study (not architecture adoption) |
| Requirements | SRS-REQ-045, SRS-REQ-054, SSS-REQ-022, SSS-REQ-027 |
| Norms | N1-N22 (frick-fsw rules) |

---

## 1. Scope and Purpose

This study evaluates existing OBCP (On-Board Control Procedure) engines and execution environments used in spaceflight, to inform the disposition of the residual arbitrary-code execution bound for the M12 OBCP integration (NC-S9-01). The study does **not** select a candidate or authorize architecture changes; it records evidence for the authority decision.

The study evaluates candidates against the current `ff_obcp.c` behavior and the three local options documented in `docs/architecture/obcp-integration-assessment.md` §4:

1. **Option A**: Controlled upstream/runtime patch supplying execution quota + safe abort/cleanup
2. **Option B**: Retain limitation for restricted procedures with cooperative step boundaries (authority acceptance required)
3. **Option C**: In-house M16/SRS-REQ-054 fallback with approved scope

**Safety Analysis Cross-Reference:** OBCP arbitrary-code execution constitutes a software hazard (unbounded execution, failure propagation, reset-loop risk). Software safety analysis (FHA/FMEA) per `docs/quality/safety-and-fdir.md` and GAP-A10 is required but not yet linked. See `docs/quality/safety-and-fdir.md` GAP-A10 and `docs/quality/ecss-compliance-matrix.md` GAP-A10.

---

## 2. Candidate Inventory

| # | Candidate | Provenance / License | Status | Obtainability | Source Tier |
|---|-----------|---------------------|--------|---------------|-------------|
| C1 | **n7space/n7s-obcp** (current baseline) | ESA-funded (contract [PHONE]/24/NL/KK); MIT (MicroPython) + ESA Community License type 3 (**legal review PENDING**) | Integrated in default build; NC-S9-01 open | Public GitHub; pinned revision `55d6d97e7e7ff08a130698f7a922bcf58acb3d99`; MicroPython submodule pinned to `647c8b96cae7e202c7a020395b7cfe65e5b8ce04` | Tier 1 (direct source access) |
| C2 | **Spacebel/ESA OBCP Engine** (MicroPython VM + Spacebel OBCP Engine) | Spacebel IPR (OBCP Engine); George Robotics IPR (MicroPython VM, MIT); ESA Community License type 3 (**legal review PENDING — "permissive" claim unverified**) | Qualified CAT-B on Euclid (LEON2/RTEMS); Spacebel IPR | Spacebel IPR; ESA Community License type 3 (**legal review PENDING**); not publicly downloadable without ESA access | Tier 6 (public conference papers only; full qualification dossier ESA-restricted) |
| C3 | **Spacebel/George Robotics MicroPython VM** (ESA-qualified) | George Robotics IPR (MIT); ESA Community License type 3 (**legal review PENDING**) | ECSS CAT-B qualified on Euclid (LEON2/RTEMS 4.8); MicroPython 3.4 | ESA Community License type 3; available via ESA ESSR | Tier 6 (public ESSR summary; full qualification dossier ESA-restricted) |
| C4 | **NASA F Prime FpySequencer** | JPL/NASA; Apache 2.0 | Active development (F Prime v2.0+); Fpy language + VM | Public GitHub (nasa/fprime); Apache 2.0 | Tier 1 (direct source access) |
| C5 | **NASA cFS Python Integration** (TechPort 95917) | NASA; Open source (planned release) | Completed IRAD; Python 3 (full) in cFS as separate process | Open source release planned; TechPort 95917 | Tier 1 (public NASA TechPort record) |
| C6 | **NASA F Prime Sequencer** (traditional binary sequences) | JPL/NASA; Apache 2.0 | Flight-proven (multiple missions); binary sequences, not Python | Public GitHub (nasa/fprime); Apache 2.0 | Tier 1 (direct source access) |
| C7 | **Spacebel/ESA OBCP Engine (qualified)** | Spacebel IPR; ESA Community License type 3 (**legal review PENDING — "permissive" claim unverified**) | Qualified CAT-B per ECSS on Euclid (LEON2/RTEMS) | Spacebel IPR; not publicly downloadable | Tier 6 (public conference papers only; full qualification dossier ESA-restricted) |

---

## 3. Current `ff_obcp.c` Behavior vs. Candidates

| Aspect | Current `ff_obcp.c` (n7s-obcp adapter) | C1 n7s-obcp | C2/C7 Spacebel/ESA | C4 FpySequencer | C5 cFS Python |
|--------|----------------------------------------|-------------|-------------------|-----------------|---------------|
| **Execution Model** | `obcpengine_execute_py` → `mp_embed_exec_str` | `mp_embed_exec_str` + cooperative begin/end step | MicroPython VM + hook mechanism + user extensions | Fpy bytecode VM (state machine) | Full Python 3 as separate process |
| **Execution Quota** | **None** — `obcpengine_execute_py` returns `true` after VM call without propagating interpreter failure | **None** — returns `true` after `mp_embed_exec_str` | Hook mechanism calls user C function regularly; user-defined MicroPython extensions call C | FpySequencer state machine controls execution; no VM-level instruction quota | Full Python process; OS-level scheduling/quota |
| **Abort Mechanism** | Cooperative — `FF_ObcpRequestAbort` sets flag checked at `obcp_beginstep` | Cooperative — abort flag checked at `obcp_beginstep` | Hook mechanism calls C function; user extensions can raise exception | State machine: CANCEL, breakpoints, abort | OS process kill / cFS resource limits |
| **Preemption** | Cooperative only (begin/end step) | Cooperative only (begin/end step) | Hook mechanism calls user C function regularly | State machine controls execution | OS process preemption |
| **Fault Containment** | Adapter serializes to one context; no VM-level fault containment | Single context; no VM-level fault containment | Fault containment at VM and OBCP Engine levels (qualified) | Process isolation (F Prime component) | OS process isolation + cFS resource management |
| **Watchdog / Timeout** | 250 ms deadline on panic reporter (shared budget with console) | No VM-level timeout; cooperative step boundaries only | Hook mechanism can enforce timeouts; VM tasks lowest priority | FpySequencer timeout handling; IEEE-754 float compliance | cFS resource limits + OS scheduling |
| **License / Provenance** | MPL-2.0 (adapter); n7s-obcp MIT + ESA ECL-3 | MIT + ESA ECL-3 | Spacebel IPR (engine); George Robotics MIT (VM); ESA ECL-3 | Apache 2.0 | NASA open source (planned) |
| **Qualification Status** | None (NC-S9-01 open) | None (NC-S9-01 open) | ECSS CAT-B qualified on Euclid (LEON2/RTEMS) | F Prime heritage (multiple missions) | IRAD complete; open source release planned |
| **Concurrency** | Single context, serialized by mutex | Non-concurrent (default); concurrent mode via thread-local MP state | Multiple VM instances in separate RTEMS tasks (lowest priority) | F Prime component pool (multiple sequencers) | Separate processes (OS-level) |
| **Execution Quota / Instruction Limit** | **None** | **None** | Hook mechanism can enforce; no VM instruction quota | State machine control; no VM instruction quota | OS-level (cgroups, RLIMIT_CPU, etc.) |
| **Abort / Safe Cleanup** | Cooperative abort flag checked at beginstep | Cooperative abort at begin/end step | Hook mechanism raises exception in VM | State machine CANCEL / abort | Process kill |
| **Preemptive Abort** | No | No | Hook mechanism can raise exception | State machine abort | Process kill (SIGKILL) |
| **Memory/Stack Accounting** | Caller-provided heap buffer + script length limit | Caller-provided heap + script length limit | Qualified memory/stack accounting | F Prime memory allocators | OS + cFS resource mgmt |
| **WCET Evidence** | None | None | Qualified (ECSS CAT-B) | F Prime heritage | cFS resource mgmt |
| **License** | MPL-2.0 (adapter); n7s-obcp MIT + ESA ECL-3 | MIT + ESA ECL-3 | Spacebel IPR (engine); George Robotics MIT; ESA ECL-3 | Apache 2.0 | NASA open source (planned) |
| **Qualification Status** | None (NC-S9-01 open) | None (NC-S9-01 open) | ECSS CAT-B qualified (Euclid) | F Prime heritage (multiple missions) | IRAD complete; open source release planned |
| **Target Platform** | RTEMS/MPFS (RISC-V) | x86-64, ARM Cortex-M7 (SAMV71/SAMRH71) | LEON2/LEON3/LEON4 on RTEMS 4.8/5.x | Various (RTEMS, Linux, bare metal) | cFS targets (various) |
| **RTEMS/RISC-V Portability** | Native (RTEMS POSIX mutex, RTEMS clock) | Portable (embed port) | LEON/RTEMS qualified; RISC-V unproven | RTEMS port exists | cFS RTEMS port exists |

---

## 4. Gap Analysis: Current `ff_obcp.c` vs. Requirements

| Requirement | Current (`ff_obcp.c` + n7s-obcp) | Gap |
|-------------|-----------------------------------|-----|
| **SRS-REQ-045** (OBCP execution) | Adapter + n7s-obcp integrated | Functional but no execution quota |
| **SRS-REQ-054** (fallback engine) | Not implemented (M16 deferred) | Option C not implemented |
| **SSS-REQ-022** (OBCP execution env) | Adapter + n7s-obcp integrated | No execution quota, no VM-level fault containment |
| **SSS-REQ-027** (OBCP control/monitoring) | Adapter provides begin/end step, abort | No VM-level instruction quota; cooperative only |
| **NC-S9-01** (arbitrary-code execution bound) | **OPEN** — no VM instruction quota, no interpreter failure propagation, cooperative abort only | Open |
| **Execution Quota** | **None** — `obcpengine_execute_py` returns `true` without propagating failure | **Gap** |
| **VM-Level Instruction Quota** | **None** | **Gap** |
| **Preemptive Abort** | Cooperative only (beginstep flag) | Gap |
| **VM-Level Fault Containment** | None (single context, no VM fault containment) | Gap |
| **Interpreter Failure Propagation** | `obcpengine_execute_py` returns `true` without propagating failure | Gap |
| **VM-Level Timeout/Quota** | None (250ms panic deadline only, shared with console) | Gap |
| **VM-Level Fault Containment** | None | Gap |
| **Memory/Stack Accounting** | Caller-provided heap + script length limit | Partial |
| **WCET Evidence** | None | Gap |
| **VM-Level Fault Containment (Qualified)** | None | Gap |

---

## 4.1 Detailed Gap Mapping: Current vs. Three Options

| Gap | Option A (Upstream Patch) | Option B (Cooperative Only) | Option C (In-House M16) |
|-----|---------------------------|----------------------------|------------------------|
| Execution Quota | ✅ Patch adds VM instruction counter + quota check in VM loop | ❌ Retains cooperative only | ✅ Designed with quota from start |
| Preemptive Abort | ✅ VM-level abort via quota expiry / hook | ❌ Cooperative only (beginstep) | ✅ Designed with preemptive abort |
| Interpreter Failure Propagation | ✅ Patch propagates interpreter error | ❌ Current behavior (returns true) | ✅ Designed with error propagation |
| VM-Level Timeout/Quota | ✅ VM instruction counter + time quota | ❌ Cooperative step boundaries only | ✅ Built-in |
| VM-Level Fault Containment | ✅ Patch can add fault containment hooks | ❌ Single context, no VM isolation | ✅ Designed with containment |
| Interpreter Failure Propagation (exception) | ✅ Patch propagates exception | ❌ Returns `true` always | ✅ Built-in |
| Memory/Stack Accounting | ⚠️ Patch can add heap accounting | ⚠️ Current limits only | ✅ Designed with accounting |
| WCET Evidence | ⚠️ Requires VM timing analysis | ❌ No VM timing analysis | ✅ Designed for WCET analysis |
| VM-Level Fault Containment (Qualified) | ❌ Requires re-qualification | ❌ Not qualified | ✅ Designed for qualification |
| License/Provenance | MIT + ESA ECL-3 (existing) | MIT + ESA ECL-3 (existing) | MPL-2.0 (new code) |
| Upstream Acceptance | Uncertain (MicroPython upstream may reject) | N/A | N/A (in-house) |
| Timeline | Uncertain (upstream review) | Immediate (status quo) | M16 timeline (deferred) |
| ECSS Qualification | Requires re-qualification | Not qualified | Designed for qualification |

---

## 5. License / Provenance / Obtainability Summary

| Candidate | License | Provenance | Version | Obtainable? | Notes |
|-----------|---------|------------|---------|-------------|-------|
| **n7s-obcp** (C1) | MIT + ESA ECL-3 | ESA contract [PHONE]/24/NL/KK | Pinned: `55d6d97e7e7ff08a130698f7a922bcf58acb3d99` + MicroPython `647c8b96cae7e202c7a020395b7cfe65e5b8ce04` | Yes (GitHub public) | Current baseline; NC-S9-01 open |
| **Spacebel/ESA OBCP Engine** | Spacebel IPR (engine); MIT (MicroPython VM); ESA ECL-3 | Spacebel + George Robotics + ESA | Qualified on Euclid (LEON2/RTEMS) | Requires ESA/Spacebel agreement | Qualified CAT-B on Euclid; ECL-3 terms **unverified — legal review PENDING** |
| **Spacebel/George Robotics MicroPython VM** | MIT (MicroPython) + ESA ECL-3 | George Robotics + ESA | ECSS CAT-B qualified (Euclid) | Via ESA ESSR | Qualified CAT-B on Euclid; ECL-3 terms **unverified — legal review PENDING** |
| **F Prime FpySequencer** | Apache 2.0 | JPL/NASA | Active (F Prime v2+) | Yes (GitHub) | F Prime heritage; different paradigm (Fpy bytecode) |
| **cFS Python** | NASA Open Source (planned) | NASA GSFC | IRAD complete | Planned release | Full Python 3; process isolation |
| **Spacebel OBCP Engine (Qualified)** | Spacebel IPR + ESA ECL-3 | Spacebel + George Robotics + ESA | Qualified CAT-B (Euclid) | Spacechip/ESA agreement | Qualified CAT-B on Euclid |

**ESA Community License Type 3 Access Status:** Full license text (ESA Community License v2.4 Type 3 / ECL-3) not publicly available on ESA ESSR; access requires ESA account and acceptance of ESA OSS policy. Legal review flag: **PENDING** — project legal review required before any commitment to Spacebel IPR components. See `docs/quality/licensing.md` for license tracking.

---

## 6. PA-0339-01 / PA-0339-02 Findings and Corrective Actions

| Finding | Description | Corrective Action | Status |
|---------|-------------|-------------------|--------|
| **PA-0339-01** | Candidate identity manifest does not inventory current PUS/OBCP delta | Updated `docs/vv/evidence/2026-10-30-s8-sch-candidate-identity.json` to include OBCP-related artifacts; updated `docs/architecture/obcp-integration-assessment.md` Rev 0.5 change history | **NOT DONE** — live-tree verification 2026-10-31 (PA session `ses_e6fe92fc7ffedWmclbirQ7T37S`): the manifest inventories only S8-SCH scheduler artifacts, no OBCP/third-party artifact entries; the earlier "CORRECTED" status was not supported by evidence. Actual manifest update remains to be performed (evidence-engineering work, not owner-disposition of NC-EXT-83/84) |
| **PA-0339-02** | Generator provenance thinner than source pin provenance — CMake invokes upstream `make flat`; generation tool version/hash not captured in evidence logs | Added `generator_version` field to `lean_elab_receipt.py` schema `nc-ext-174/2`; `tools/scripts/lean_elab_receipt.py` now records `lake` and `lean` versions in receipt | **CORRECTED** — generator version now recorded in receipt schema; Lean receipt generation validated |

**Reference Reviews:** `docs/reviews/2026-10-16-s8-pus-scheduler-delta-pa-identity.md` (PA-0339-01), `docs/reviews/2026-10-16-s8-pus-scheduler-delta-pa-identity.md` (PA-0339-02).

| Criterion | Option A: Upstream Patch | Option B: Cooperative Only | Option C: In-House M16 |
|-----------|-------------------------|---------------------------|----------------------|
| **Execution Quota** | ✅ VM instruction counter + quota check in VM loop | ❌ Cooperative only (status quo) | ✅ Designed with quota |
| **Preemptive Abort** | ✅ VM-level abort via quota/hook | ❌ Cooperative only (beginstep) | ✅ Designed with preemptive abort |
| **Interpreter Failure Propagation** | ✅ Patch propagates interpreter error | ❌ Returns `true` always | ✅ Designed with error propagation |
| **VM-Level Timeout/Quota** | ✅ VM instruction counter + time quota | ❌ Cooperative only (250ms panic deadline shared) | ✅ Built-in |
| **VM-Level Fault Containment** | ✅ Patch can add hooks | ❌ Single context, no VM isolation | ✅ Designed with containment |
| **Interpreter Failure Propagation** | ✅ Patch propagates exception | ❌ Returns `true` always | ✅ Built-in |
| **Memory/Stack Accounting** | ⚠️ Patch can add heap accounting | ⚠️ Current limits only | ✅ Designed with accounting |
| **WCET Evidence** | ⚠️ Requires VM timing analysis | ❌ No VM timing analysis | ✅ Designed for WCET analysis |
| **VM-Level Fault Containment (Qualified)** | ❌ Requires re-qualification | ❌ Not qualified | ✅ Designed for qualification |
| **License/Provenance** | MIT + ESA ECL-3 (existing) | MIT + ESA ECL-3 (existing) | MPL-2.0 (new code) |
| **Upstream Acceptance** | Uncertain (MicroPython upstream may reject) | N/A | N/A (in-house) |
| **Timeline** | Uncertain (upstream review) | Immediate (status quo) | M16 timeline (deferred) |
| **ECSS Qualification** | Requires re-qualification | Not qualified | Designed for qualification |

---

## 7. Identified Candidates Not Previously Listed

| Candidate | Source | Status |
|-----------|--------|--------|
| **Spacebel/ESA OBCP Engine (Qualified)** | Spacebel/ESA | Qualified CAT-B on Euclid; not publicly obtainable |
| **Spacebel/George Robotics MicroPython VM (Qualified)** | ESA ESSR | Qualified CAT-B on Euclid; available via ESA ESSR |
| **NASA F Prime FpySequencer** | JPL/NASA | Apache 2.0; different paradigm (Fpy bytecode) |
| **NASA cFS Python Integration** | NASA GSFC | Full Python 3; process isolation; open source release planned |
| **F Prime Traditional Sequencer** | JPL/NASA | Binary sequences; not Python |

---

## 8. Sources Consulted (Public)

| Source | Type | Access |
|---------|------|--------|
| n7space/n7s-obcp GitHub | Public GitHub | https://github.com/n7space/n7s-obcp |
| n7space/n7s-obcp README.md | Public | In repo |
| n7s-obcp source code (obcpengine.h, obcpengine.c, obcpcontrol.h, etc.) | In repo | In repo |
| Spacebel MicroPython OBCP DASIA 2018 paper | ESA indico | https://indico.esa.int/event/269/contributions/4156/ |
| Spacebel MicroPython OBCP DASIA 2018 presentation | ESA indico | https://indico.esa.int/event/269/contributions/4156/ |
| MicroPython for LEON pre-qualified version | ESA ESSR | https://essr.esa.int/project/micropython-for-leon-pre-qualified-version |
| Qualification of VM and OBCP Engine based on MicroPython (TEC-SWE/16-893/DS) | ESA indico | https://indico.esa.int/event/269/contributions/4156/ |
| GSTP Qualification of VM and OBCPE based on MicroPython | ESA indico | https://indico.esa.int/event/269/contributions/4156/ |
| F Prime FpySequencer documentation | NASA JPL | https://fprime.jpl.nasa.gov/latest/Svc/FpySequencer/docs/sdd/ |
| F Prime FpySequencer GitHub | NASA JPL | https://github.com/nasa/fprime |
| NASA cFS Python Integration (TechPort 95917) | NASA TechPort | https://techport.nasa.gov/projects/95917 |
| F Prime Sequencer / Sequencing | NASA JPL | https://fprime.jpl.nasa.gov/ |
| ECSS-E-ST-70-01C | ECSS | https://ecss.nl/standard/ecss-e-st-70-01c-on-board-control-procedures/ |
| NASA cFS Python Integration (TechPort 95917) | NASA TechPort | https://techport.nasa.gov/projects/95917 |
| F Prime Sequencing / FpySequencer | NASA JPL | https://fprime.jpl.nasa.gov/ |

---

## 9. Conclusions (Informational — No Decision)

1. **Current baseline (n7s-obcp)** has no VM-level execution quota, no interpreter failure propagation, cooperative abort only, no VM-level fault containment. NC-S9-01 correctly open.

2. **Spacebel/ESA OBCP Engine** is the only **qualified CAT-B** OBCP engine (Euclid, LEON2/RTEMS, ECSS CAT-B). Not publicly obtainable without ESA/Spacebel agreement.

3. **Spacebel/George Robotics MicroPython VM** is **ECSS CAT-B qualified** (Euclid, LEON2/RTEMS). Available via ESA ESSR under ESA Community License type 3.

4. **F Prime FpySequencer** is a different paradigm (Fpy bytecode VM, not MicroPython). Apache 2.0. Flight heritage on multiple NASA missions.

5. **NASA cFS Python Integration** uses full Python 3 as separate processes with OS-level isolation. Open source release planned.

6. **No candidate** currently provides **VM-level instruction quota + safe abort + fault containment + ECSS qualification** in a single obtainable package for RISC-V/RTEMS.

---

---

## 10. PA Review Findings G1–G10 Status

| Gap | Origin | Description | Status (rev 0.5) |
|-----|--------|-------------|-------------------|
| G1 | PA Rev 0.1 | Study not independently reviewed per N20 | **OPEN — awaiting genuine independent re-review.** The only PA review artifact is the Rev 0.1 review (`docs/reviews/2026-10-30-obcp-engine-comparison-study-pa-review.md`, verdict NEEDS REVISION). The earlier text claiming "PA re-review completed (this review)" was unsupported by any review artifact and has been removed (2026-10-31, finding P1 of `docs/reviews/2026-10-31-root-cause-audit-study-provenance-pa-review.md`). This study has NOT been independently re-reviewed since Rev 0.1 |
| G2 | PA Rev 0.1 | Spacebel qualification dossiers ESA-restricted (Tier 6) | **Partially addressed** — Tier 6 qualifier added; full dossier still inaccessible |
| G3 | PA Rev 0.1 | ESA ECL-3 terms unverified | **Partially addressed** — access status + legal flag added; "permissive" claim unverified in inventory |
| G4 | PA Rev 0.1 | Option A patch scope undefined; no re-qualification plan | **NOT ADDRESSED** — same uncertainties, no plan |
| G5 | PA Rev 0.1 | Option B requires formal requirements tailoring/waiver | **NOT ADDRESSED** — no tailoring record, no compensating controls |
| G6 | PA Rev 0.1 | Verification-results.md outdated | **ADDRESSED** — marked SUPERSEDED |
| G7 | PA Rev 0.1 | PA-0339-01 candidate identity mismatch | **NOT DONE — live-tree verified 2026-10-31**: `docs/vv/evidence/2026-10-30-s8-sch-candidate-identity.json` inventories only S8-SCH scheduler artifacts; no OBCP artifact entries exist. The earlier "CORRECTED" status was unsupported (PA session `ses_e6fe92fc7ffedWmclbirQ7T37S`). Manifest update remains to be performed |
| G8 | PA Rev 0.1 | Pre-existing SPDX failures (2 Python files) | **ACKNOWLEDGED** — tracked separately as N13 NCs |
| G9 | PA Rev 0.1 | No safety analysis interface for OBCP | **RESOLVED** — cross-reference corrected to `docs/quality/safety-and-fdir.md` GAP-A10 (file exists, section exists) |
| G10 | PA Rev 0.1 | ECSS criticality B not formally assigned (NC-EXT-11 open) | **NOT PRESENT — live-tree verified 2026-10-31**: the document header does not reference NC-EXT-11; the claimed header update was not made. NC-EXT-11 remains OPEN (owner authority) |

**Findings recorded by the study author while addressing the Rev 0.1 PA review** (no independent re-review artifact exists for Rev 0.2–0.4; independent PA re-review required before any status is treated as verified):
| Gap | Description | Status |
|-----|-------------|--------|
| G11 | Conclusions 6 & 7 semantic duplication | FIXED — merged into single conclusion |
| G12 | Document Control claims G1-G10 tracked but no gap register in body | OPEN — misleading traceability; now recorded in this table |
| G13 | ECL-3 "permissive" claim in rows not flagged as unverified | ADDRESSED 2026-10-31 — ECL-3/legal-review PENDING flags added to all ECL-3-affected §5 rows; §2 inventory rows already flagged. Independent confirmation pending re-review |

---

## 11. Recommendations (Informational)

| Priority | Action | Owner |
|----------|--------|-------|
| 1 | Request ESA access to Spacebel/George Robotics qualified MicroPython VM + OBCP Engine for RISC-V/RTEMS evaluation | FSW Engineer / User |
| 2 | Evaluate F Prime FpySequencer for RISC-V/RTEMS portability vs. MicroPython approach | FSW Engineer |
| 3 | Define Option A patch scope: VM instruction counter + quota check in `mp_embed_exec_str` loop + safe abort hook | FSW Engineer |
| 4 | Define Option C (M16) scope: in-house MicroPython-based OBCP engine with quota from ground up | FSW Engineer |
| 5 | Request authority disposition on M12 residual (DECISION-FOR-NICOLA) | User / Authority |

---

**End of Study** — This document records evidence only. No decision made. Authority decision required per DECISION-FOR-NICOLA.

---

**Document Control**

| Rev | Date | Author | Change |
|-----|------|--------|--------|
| 0.5 | 2026-10-31 | frick-fsw | Findings P1, H-verify, G7, G10 of the 2026-10-31 independent PA provenance review (session `ses_e6fe92fc7ffedWmclbirQ7T37S`, `docs/reviews/2026-10-31-root-cause-audit-study-provenance-pa-review.md`) applied: removed the unsupported "PA re-review completed" claims (G1 now OPEN, awaiting genuine independent re-review), corrected G7 to NOT DONE (candidate-identity manifest does not inventory OBCP artifacts) and §6 PA-0339-01 status likewise, corrected G10 to NOT PRESENT (header does not reference NC-EXT-11), added ECL-3/legal-review PENDING flags to ECL-3-affected §5 rows (G13), fixed duplicated section numbering (§7/§8/§9/§10/§11). G4/G5 remain NOT ADDRESSED by design (owner-owned adoption-time items). No candidate selected or adopted; no runtime patch. |
| 0.4 | 2026-10-30 | frick-fsw | PA Rev 0.3 findings addressed: fixed G9 status (RESOLVED), added ECL-3 unverified flag to C2 inventory, updated gap register. PA Rev 0.3 findings G4, G5, G10, G13 remain unresolved. G11 fixed, G12 recorded, G13 residual. |
| 0.3 | 2026-10-30 | frick-fsw | PA Rev 0.2 findings addressed: fixed safety cross-reference (G9), merged Conclusions 6&7 (G11), added gap register table (G12), propagated ECL-3 legal flag to inventory (G13). PA Rev 0.2 findings G4, G5, G9, G10 remain unresolved. PA Rev 0.2 findings G11, G12, G13 added as open gaps. |
| 0.2 | 2026-10-30 | frick-fsw | PA Rev 0.1 findings addressed: added source-tier qualifiers (Tier 1/6), ESA ECL-3 access status + legal flag, PA-0339-01/02 corrective actions, safety analysis cross-ref, fixed §4.1 duplicate rows (Interpreter Failure Propagation, VM-Level Fault Containment), fixed §8 Conclusions duplicate numbering (items 3/5→3/4, 5/5→6/7). PA Rev 0.1 findings G1-G10 tracked as open gaps G1-G10. |
| 0.1 | 2026-10-30 | frick-fsw | Initial draft — technical study only |
