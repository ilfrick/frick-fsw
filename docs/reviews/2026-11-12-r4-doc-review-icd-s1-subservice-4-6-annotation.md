# DOC-R4-20261112 — Software Documentation Review (N17, R4 profile)

**Repository:** /home/nicola/fsw/frick-fsw  
**Branch:** main  
**HEAD:** b6bf060be425db99325c2d4685cd0a8b0ef104a6  
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2  
**Criticality target:** ECSS class B  
**Reviewer profile:** Senior Aeronautics and Space Software Documentation Engineer (R4) — independent documentation review  
**Review date:** 2026-11-12  
**Change under review:** `docs/interfaces/ICD.md` line 106 — interface annotation only  
- Before: `| 1  | 4 / 6 | start / progress failure | TM | errcode |`  
- After:  `| 1  | 4 / 6 | start / progress failure (reserved, not emitted) | TM | errcode |`  
**Gates reported as passing:** `make build` exit 0 (0 warnings), CTest 56/56, Lean `lake build`, `make trace` PASS (120/76/30/14/68), format-check PASS, license OK.

---

## 1. Change characterisation (R4 assessment)

The delta is a single-cell annotation in the PUS Service 1 subservice table of the Interface Control Document (ICD, document id `FRICK-ICD-001`, revision 1.7). It adds the parenthetical qualifier `(reserved, not emitted)` to the name column for Service 1 / subservice 4 / 6 ("start / progress failure").

Verified scope of the delta:
- No source-code change (`src/` untouched); no behaviour change.
- No hardware / target / device-tree / BSP / HAL change.
- No timing, scheduling, bus, network, clock, FDIR, watchdog, or event behaviour change.
- No requirements, traceability matrix, SSS/SRS, or verification artefact change; no traceability requirement change.
- `ICD.md` is itself the controlled interface document for this table; the annotation clarifies existing interface semantics rather than creating or removing a wire behaviour.
- Norm N2 (English): the added text `(reserved, not emitted)` is English — **compliant**.
- Norm N8 (complete, current, deployable documentation): no separate document needs updating; the ICD row itself is the interface contract — **compliant for this change**.

---

## 2. Accuracy of the annotation against the implementation (review item 4)

**Fact, High confidence — directly observed in source.**

`FF_PusSendVerification` (`src/pus/pus.c:298`) implements the stage→subservice map:

| stage | success=1 | success=0 |
|---|---|---|
| ACCEPTANCE | subservice 1 | subservice 2 |
| START | subservice 3 | subservice 4 |
| PROGRESS | subservice 5 | subservice 6 |
| COMPLETION | subservice 7 | subservice 8 |

(`src/pus/pus.c:310-324`; switch comment at `:307-308` documents the same map.)

Call sites (`src/pus/pus.c`):
- `:423` ACCEPTANCE `success=0` → TM[1,2] **emitted**
- `:432` ACCEPTANCE `success=1` → TM[1,1] **emitted**
- `:442` COMPLETION `success=0` → TM[1,8] **emitted**
- `:451` START `success=1` → TM[1,3] emitted; **TM[1,4] has no emission path** (`success` is hard-coded `1u`)
- `:456` PROGRESS `success=1` → TM[1,5] emitted; **TM[1,6] has no emission path** (`success` is hard-coded `1u`)
- `:464` COMPLETION conditional `(rc == FF_SUCCESS) ? 1u : 0u` → TM[1,7] or TM[1,8]

Conclusion: subservice 4 (start failure) and subservice 6 (progress failure) exist in the standard's subservice map and in the function's `switch`, but no call site passes `success=0` for STAGE_START or STAGE_PROGRESS, so neither TM[1,4] nor TM[1,6] is emitted by this build. The annotation **"(reserved, not emitted)" is accurate** for the ICD context ("Rows below are the services/subservices this build answers on the wire", `ICD.md:96`).

This matches the established emitted set `{1,2,3,5,7,8}` re-derived from `src/pus/pus.c` by all three independent profiles (R1/R2/R3/R4), recorded in NC-EXT-94 (`docs/planning/sprint-plan.md:231`) and reflected in:
- `README.md:184` — Service 1 row already states start failure (4) is not emitted and progress failure (6) is not mapped, cross-referencing NC-EXT-94.
- `docs/architecture/pus-c-from-scratch-assessment.md:39` — already "partial (1,2,3,5,7,8)".
- `docs/vv/s8-pus-fu-authority-disposition-20261024.md:50-52` — already records "1,4 Start failure NOT EMITTED (no code path)" and "1,6 Progress failure NOT MAPPED (no code path)".
- `docs/status/handover.md:878` — already records "subservice 4 is mapped but unreachable and 6 is unmapped — NC-EXT-94 widened".

The ICD row annotation (`:106`) is therefore the final, interface-authority-owned piece of the NC-EXT-94 disposition ("annotate the row as reserved-but-not-generated; do not widen scope"; `sprint-plan.md:231`, `docs/reviews/2026-10-18-docs-implementation-alignment.md:52`).

---

## 3. Whether any other documentation needs updating (review item 1)

**Finding: No.** The ICD is the interface contract for this table. Every other document that addresses the S1(1,4/6) wire behaviour already carries the corrected emitted set `{1,2,3,5,7,8}` and/or references NC-EXT-94:

| Document | Line | State |
|---|---|---|
| `README.md` | 184 | Corrected: lists implemented subservices {1,2,3,5,7,8}; states 4/6 not emitted; references NC-EXT-94 |
| `docs/architecture/pus-c-from-scratch-assessment.md` | 39 | Corrected: "partial (1,2,3,5,7,8)" |
| `docs/vv/s8-pus-fu-authority-disposition-20261024.md` | 50-52 | Already records 1,4/1,6 as not emitted / not mapped |
| `docs/status/handover.md` | 878 | Already records subservice 4 unreachable, 6 unmapped |
| `docs/planning/sprint-plan.md` | 231 | NC-EXT-94 register entry; disposition = annotate ICD row (this change) |

No SRS/SSS, `traceability.yaml`, requirements, other-discipline ICD, or verification artefact requires a corresponding change (NC-EXT-94 is non-functional under N21 and does not alter requirement semantics/IDs/links). **No documentation finding blocks commit/push on this account.**

---

## 4. Norm N2 and Norm N8 (review items 2 and 3)

- **Norm N2 (English):** The added text `(reserved, not emitted)` is English. **Compliant — no finding.**
- **Norm N8 (complete, current, deployable documentation):** The ICD row itself is the controlled interface contract; no separate document needs updating for this annotation (see §3). The change is self-contained and deployable as-is. **Compliant — no finding.**

---

## 5. Findings

| Finding ID | Classification | File:line | Blocks commit/push? | Finding |
|---|---|---|---|---|
| DOC-R4-001 | Minor | `docs/interfaces/ICD.md:106` | No | Annotation "(reserved, not emitted)" added to S1 subservice 4/6. Documentation/interface annotation only. No wire behaviour, code, hardware, timing, or FDIR is created, removed, or altered. No system-level risk introduced. |
| DOC-R4-002 | Minor | `docs/interfaces/ICD.md:106`; `src/pus/pus.c:298-324,451,456` | No | Confirmed accurate against implementation: `FF_PusSendVerification` maps STAGE_START→{3,4} and STAGE_PROGRESS→{5,6}, but the only call sites (`pus.c:451`, `:456`) pass `success=1`, so TM[1,4] and TM[1,6] have no emission path. "Reserved, not emitted" is consistent with the implemented behaviour. |
| DOC-R4-003 | Minor | `README.md:184`; `docs/architecture/pus-c-from-scratch-assessment.md:39`; `docs/interfaces/ICD.md:106` | No | Cross-document consistency confirmed: all documents addressing S1(1,4/6) already carry emitted set {1,2,3,5,7,8} and/or reference NC-EXT-94; no other documentation needs updating for this ICD annotation. |
| DOC-R4-004 | Minor | `docs/interfaces/ICD.md:106` | No | Norm N2 (English): added text is English — compliant. Norm N8: no doc changes beyond the ICD row itself — compliant. |
| DOC-R4-005 | Minor | `docs/interfaces/ICD.md:213-233` (History table); N16 | No | **Administrative observation, does not block.** The ICD content changed but the `History` table (latest entry rev 1.7, 2026-11-11, NC-EXT-237) does not yet record this annotation. Per norm N16 (detailed per-doc revision history) and the ICD's own convention (every content change rev 1.3–1.7 has a History entry), a revision entry (e.g. 1.8) should be added either in this commit or the next ICD delta. This is an administrative revision record, not a technical-content defect. |

**No Major findings. No finding blocks commit/push.**

---

## 6. Relationship to open NCs

- **NC-EXT-94** (`docs/planning/sprint-plan.md:231`) remains OPEN. It is classified Minor interface/spec, non-functional under N21, and is **not a blocker for commit/push**. This ICD annotation is precisely the agreed disposition action ("annotate the row as reserved-but-not-generated; do not widen scope"). Closure of NC-EXT-94 is an interface/requirements-authority disposition decision, separate from this documentation change, and is not implied or closed by this review.
- Untouched NCs (per context): NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01. None are affected by this annotation.

---

## 7. Independent reviews already on record

- Avionics/systems review (R1/R3 profile): `docs/reviews/SYS-REVIEW-R1-R3-20261112-icd-s1-subservice-4-6-annotation-avionics-systems-review.md` — findings SYS-R1/R3-001 … -005, all Minor, all "No" (do not block); recommendation: commit/push permitted from the avionics/systems perspective, subject to remaining applicable gates.
- This R4 documentation review is consistent with and complementary to that systems review.

---

## 8. Recommendation (R4 documentation perspective)

**Commit/push: permitted** from the documentation (N17/R4) standpoint, provided:
- the project's remaining applicable gates are completed by the designated authorities (N7 sequencing; PA/N20 review where required; N1/N3/N4/N10/N11/N15/N18/N20/N21/N22 as applicable),
- the `History` administrative entry (DOC-R4-005) is added in this commit or the next ICD delta,
- NC-EXT-94 remains open pending interface/requirements-authority disposition (it does not block commit/push under N21).

This review does **not** waive any separate safety, security, regulatory, contractual, or release gate, does **not** certify compliance, does **not** close any NC or approve any risk, and does **not** substitute for engineering evidence. Technical content and safety-critical decisions require review and approval by qualified authorized humans.

---

## 9. Review revision history

| Version | Date | Author/role | Description |
|---|---|---|---|
| 1.0 | 2026-11-12 | Senior SW Documentation Engineer (R4) | Independent N17 documentation review of ICD.md:106 "(reserved, not emitted)" annotation |
