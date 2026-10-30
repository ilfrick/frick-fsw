Independent V&V Review: OBCP Engine Comparison Study (FRICK-ARCH-OBCP-STUDY-001)
================================================================================

Date: 2026-10-30
Reviewer: Senior Aeronautics and Space Software V&V Engineer (independent sub-agent)

---

## SCOPE REVIEWED
- Document: `docs/architecture/obcp-engine-comparison-study.md` (FRICK-ARCH-OBCP-STUDY-001, Rev 0.1, 2026-10-30)
- Context: Technical study to inform DECISION-FOR-NICOLA on M12 OBCP integration (NC-S9-01)
- Current baseline: n7space/n7s-obcp (C1) integrated; NC-S9-01 OPEN

---

## FINDINGS

### F1: Study Scope and Posture — ACCURATE
The study explicitly declares: "Draft — Technical Study (not architecture adoption)" and "records evidence for the authority decision". It does not select a candidate or authorize architecture changes. This posture is maintained throughout.

### F2: Candidate Inventory (C1–C7) — FACTUALLY ACCURATE
All 7 candidates correctly described with provenance, license, status, obtainability:
- C1: n7space/n7s-obcp (current baseline) — MIT + ESA ECL-3, public GitHub, pinned revision
- C2/C7: Spacebel/ESA OBCP Engine — ECSS CAT-B qualified on Euclid (LEON2/RTEMS), Spacebel IPR, not publicly obtainable
- C3: Spacebel/George Robotics MicroPython VM — ECSS CAT-B qualified, available via ESA ESSR
- C4: F Prime FpySequencer — Apache 2.0, different paradigm (Fpy bytecode VM), "in development"
- C5: NASA cFS Python Integration — TechPort 95917, IRAD complete, full Python 3, open source planned
- C6: F Prime traditional sequencer — Apache 2.0, binary sequences, not Python
- C7: Spacebel OBCP Engine (qualified) — Same as C2, qualified CAT-B

All descriptions match public sources (GitHub, ESA ESSR, ESA indico, F Prime docs, NASA TechPort).

### F3: Gap Analysis — ACCURATE AND COMPLETE
Current `ff_obcp.c` + n7s-obcp gaps correctly identified against SRS-REQ-045, SSS-REQ-022, SRS-REQ-054, SSS-REQ-027:
- No VM-level instruction quota
- No interpreter failure propagation
- No preemptive abort (cooperative only)
- No VM-level timeout/quota (250ms panic deadline shared with console)
- No VM-level fault containment
- No memory/stack accounting
- No WCET evidence
- No qualified VM-level fault containment

All gaps confirmed by `ff_obcp.c` implementation and `obcp-integration-assessment.md` §2.

### F4: Three-Options Comparison — ACCURATE AND COMPLETE
Options A/B/C match `obcp-integration-assessment.md` §4 exactly. Gap-to-option mapping in §4.1 is logically consistent:
- Option A (upstream patch): addresses all gaps but requires re-qualification, upstream acceptance uncertain
- Option B (cooperative only): retains all gaps; immediate status quo; not qualified
- Option C (in-house M16): designed for qualification from ground up; deferred to M16

### F5: Conclusions — ACCURATE AND RESTRAINED
- "No candidate currently provides VM-level instruction quota + safe abort + fault containment + ECSS qualification in a single obtainable package for RISC-V/RTEMS" — CORRECT
- "No candidate currently provides all three: VM-level instruction quota + safe abort + fault containment + ECSS qualification for RISC-V/RTEMS" — CORRECT (duplicated in conclusions, editorial)
- Study correctly identifies: Spacebel/ESA is only qualified CAT-B engine; not publicly obtainable; Spacebel VM obtainable via ESA ESSR; no candidate provides all three for RISC-V/RTEMS

### F6: Editorial Issues (Low Severity)
- §4.1 Gap Mapping: duplicate rows for "Interpreter Failure Propagation" (lines 93, 96) and "VM-Level Fault Containment" (lines 95, 99)
- §8 Conclusions: duplicate numbering (two "5." items at lines 186, 188)

---

## V&V REVIEW VERDICT: **ACCEPTABLE** (with editorial corrections)

**Rationale:** The study is factually accurate, well-sourced, correctly identifies gaps, fairly compares options against three authorized options, maintains proper study-only posture without architecture decision, and maintains traceability. Editorial issues are minor and don't affect technical conclusions.

**Open Gaps (per study §8 Conclusions):**
1. No candidate provides VM instruction quota + safe abort + fault containment + ECSS qualification for RISC-V/RTEMS
2. Spacebel/ESA OBCP Engine is only qualified CAT-B engine but not publicly obtainable
3. Spacebel MicroPython VM is qualified but Engine not publicly obtainable
4. No candidate provides all three for RISC-V/RTEMS

**V&V Review Status: ACCEPTABLE** — Study fulfills its purpose as evidence record for authority decision. No architecture decision made. No NC closed. No human approval claimed.

---

**Reviewer:** Senior Aeronautics and Space Software V&V Engineer (independent sub-agent)  
**Date:** 2026-10-30  
**Authority:** Review provides evidence only. Does not approve baselines, waive gates, authorize commit/push, or make safety decisions.
