# V&V Independent Review Report — R2 Profile (Norm N10)

**Repository:** frick-fsw (`/home/nicola/fsw/frick-fsw`)
**Branch / HEAD:** `main` / `b6bf060`
**Change under review:** `docs/interfaces/ICD.md` line 106
**Review type:** Documentation / interface-annotation V&V evidence review (independent)
**Reviewer profile:** Senior Aeronautics and Space Software V&V Engineer — norm N9 R2 (independent verification), norm N10 scope (test scripts / V&V tools)
**Review date:** 2026-11-12
**Change nature:** Documentation / interface annotation only — no `src/` change, no new or modified test scripts, no T3 impact, no N10 re-qualification trigger.
**Reviewer authority boundary:** Independent V&V evidence only. This report does not waive any safety/security/regulatory/contractual gate, does not close any NC, does not constitute authority acceptance or flight readiness, and does not authorize commit/push.

---

## 1. Change text

`docs/interfaces/ICD.md:106` (working-tree modification, `git status:  M docs/interfaces/ICD.md`, unstaged, not yet committed):

```
-| 1  | 4 / 6 | start / progress failure | TM | errcode |
+| 1  | 4 / 6 | start / progress failure (reserved, not emitted) | TM | errcode |
```

Scope: one ICD table row annotation (Service 1, subservices 4/6 "start / progress failure"). No requirement ID, semantics, or traceability link changed. Service 1 row is part of FRICK-ICD-001 §4.

---

## 2. Governing norms cited

| Norm | Subject | Reference |
|------|---------|-----------|
| N4 | Bidirectional traceability, converged at every commit | `docs/rules.md:55` |
| N9 | Independent review agents | `docs/rules.md:122` |
| N10 | Review of test scripts and V&V tools; re-qualification on any script change | `docs/rules.md:138`; `docs/vv/tool-qualification.md` |
| N11 | Scripts + review results retained immediately; committed with each compliant cycle | `docs/rules.md:158` |
| N18 | Renode functional-validation gate (in force) | `docs/rules.md:315` |
| N20 | Independent Senior SW Product Assurance review of every phase product | `docs/rules.md:352` |
| N22 | Requirement-based/integration tests, nominal + applicable out-of-bound cases; Renode is an emulator, not a test type | `docs/rules.md:405` |

---

## 3. Evidence reviewed

| # | Evidence | File:line(s) | What it shows |
|---|----------|--------------|---------------|
| E1 | ICD §4 Service 1 table | `docs/interfaces/ICD.md:101-108` | Row `:106` lists S1 subservices 4/6 "start / progress failure"; `:103-105,107-108` list the emitted set neighbors {1,2,3,5,7,8}. Lead-in `:96` states "Rows below are the services/subservices this build answers on the wire." |
| E2 | `FF_PusSendVerification` definition + subservice mapping | `src/pus/pus.c:298-354` | Switch maps `STAGE_START`: `success ? 3 : 4` (`:313-314`); `STAGE_PROGRESS`: `success ? 5 : 6` (`:316-318`); emission via `FF_PusSendTm` at `:353`. |
| E3 | Production call sites in `FF_PusIngest` | `src/pus/pus.c:430-468` | `:423` ACCEPTANCE/0 → subservice 2; `:432` ACCEPTANCE/1 → subservice 1; `:442` COMPLETION/0 → subservice 8; `:451` START/1 → subservice 3; `:456` PROGRESS/1 → subservice 5; `:464` COMPLETION/(rc?1:0) → subservice 7/8. START and PROGRESS are **only** called at `:451`/`:456`, both with `success=1`. |
| E4 | Unit test exercising the mapping | `tests/unit/test_pus.c:409-480` | `test_verification_missing_subservices` calls `FF_PusSendVerification` directly with START/0 (`:422-428`) and PROGRESS/0 (`:430-436`) but asserts only `== FF_SUCCESS`; no `stream_has(1u,4u)` / `stream_has(1u,6u)` wire-emission assertion. |
| E5 | NC-EXT-94 finding + disposition | `docs/planning/sprint-plan.md:232`; `docs/reviews/2026-10-18-docs-implementation-alignment.md:32-54` | ICD overstates S1 wire behaviour; disposition: "annotate the row as reserved-but-not-generated; no wire behaviour changed and no PUS-C compliance claim is made." Re-derived emitted set {1,2,3,5,7,8}; TM[1,4] has no emission path; subservice 6 not emitted. |
| E6 | Tool-qualification register | `docs/vv/tool-qualification.md` | T3 = `tests/unit/*.c` (incl. `test_pus.c`); N10 rule: "a change in the tool invalidates previously produced evidence until re-qualification" (`:13-15`). |
| E7 | Reported gate evidence (corroborated in `docs/vv/tool-qualification.md:T17`) | — | Build exit 0 (0 warnings); CTest 56/56; Lean `lake build`; trace PASS (120/76/30/14/68); format-check PASS; license OK. |

---

## 4. Findings

### VV-R2-001 — ICD.md:106 annotation accurately reflects wire behavior (Confirming; Minor)

**Classification:** Minor — confirming evidence; no defect.  
**Blocks commit/push:** No.

**Evidence (E1–E4, E5):**
- `FF_PusSendVerification` (`pus.c:298-354`) retains the mappings `STAGE_START`/0 → subservice 4 (`:313-314`) and `STAGE_PROGRESS`/0 → subservice 6 (`:316-318`), so the function *can select* 4/6 when invoked directly with `success=0`.
- However, the production TC-ingestion path calls `FF_PusSendVerification` at exactly six sites (`pus.c:423, 432, 442, 451, 456, 464`). `STAGE_START` appears only at `:451` with `success=1` (→ subservice 3); `STAGE_PROGRESS` appears only at `:456` with `success=1` (→ subservice 5). No production path passes `START`/0 or `PROGRESS`/0.
- `success=0` *is* passed elsewhere (`:423` ACCEPTANCE/0 → subservice 2; `:442`/`:464` COMPLETION/0 → subservice 8), so the precise limitation is "`success=0` is never passed with `STAGE_START` or `STAGE_PROGRESS`," not the broader "call sites never pass success=0."
- Consequently the TC-ingestion path never emits TM[1,4] or TM[1,6] on the wire. Production emission set = {1,2,3,5,7,8}, matching the NC-EXT-94 re-derivation (E5).
- The unit test (E4) exercises START/0 and PROGRESS/0 through `FF_PusSendVerification` directly but asserts only `FF_SUCCESS`; it does not assert wire emission of subservice 4/6 — consistent with "not emitted," and no positive wire-emission claim for 4/6 is made or needed.

**Assessment:** The annotation "(reserved, not emitted)" correctly describes Service 1 subservices 4/6 wire behavior in this build. It implements the NC-EXT-94 disposition (annotate row reserved-but-not-generated; no wire behaviour changed). Accurate.

### VV-R2-002 — Subservice 4/6 mapping retained in `FF_PusSendVerification`, reachable by direct call (Minor)

**Classification:** Minor — clarification / traceability.  
**Blocks commit/push:** No.

**Evidence:** `pus.c:313-314` and `pus.c:316-318` retain the mappings; a direct caller (including the unit test at `test_pus.c:428, :436`) can cause the function to select subservice 4 or 6. No production TC path reaches these patterns (see VV-R2-001).

**Assessment:** "Not emitted" is accurate at the wire/TC-ingestion level; it is not a claim that the function is incapable of selecting subservice 4/6 under *any* call. The ICD's "TM" column plus the "(reserved, not emitted)" qualifier correctly scope the statement to wire behavior. Optional (not required for correctness): add a clarifying note in ICD §4 lead-in (`:96`) or on the row that the mapping is retained in `FF_PusSendVerification` but no production path reaches it. Does not block.

### VV-R2-003 — ICD §4 lead-in vs row annotation consistency (Minor)

**Classification:** Minor — documentation consistency.  
**Blocks commit/push:** No.

**Evidence:** `ICD.md:96` lead-in states "Rows below are the services/subservices this build answers on the wire"; `ICD.md:106` now annotates 4/6 as "reserved, not emitted."

**Assessment:** The row-level annotation is the controlling qualifier for subservice 4/6 and resolves the general lead-in for that row. No contradiction requiring a code change. Optional: align the lead-in wording ("with the row-level reservations noted"). Does not block.

### VV-R2-004 — No additional tests needed; documentation-only change (Confirming; Minor)

**Classification:** Minor — scope confirmation.  
**Blocks commit/push:** No.

**Assessment:** No `src/` change, no new or modified test scripts, no testware/procedure/oracle change. Existing requirement-based and integration test evidence (CTest 56/56, including `test_pus.c`) remains valid; N22 test scope is unchanged. The direct unit exercise of `FF_PusSendVerification` (`test_pus.c:413-480`) already covers START/0 and PROGRESS/0 at return-status level. No additional test is required for an ICD annotation.

### VV-R2-005 — No effect on existing test behavior (Confirming; Minor)

**Classification:** Minor — scope confirmation.  
**Blocks commit/push:** No.

**Assessment:** `ICD.md` is interface documentation; no test oracle, expected result, or execution path references `ICD.md:106` as executable evidence. CTest behavior unchanged (56/56, 0 warnings). Regression selection unaffected. Does not block.

### VV-R2-006 — Tool qualification: no new scripts, no T3 impact, no N10 re-qualification trigger (Confirming; Minor)

**Classification:** Minor — scope confirmation.  
**Blocks commit/push:** No.

**Evidence (E6):** `docs/vv/tool-qualification.md` register T1–T25 untouched; T3 (`tests/unit/*.c`, incl. `test_pus.c`) unchanged by this ICD edit. N10 rule: "a change in the tool invalidates previously produced evidence until re-qualification" (`:13-15`). No tool or script changed, so no evidence is invalidated and no re-qualification is triggered. The ICD annotation does not feed any qualified tool (T1 Docker image, T2 harness, T3 CTest/tests, T6 trace.py, T17 Renode, etc.).

---

## 5. Norm N18 (Renode) applicability

The change is a pure ICD annotation with no executable or behavioral delta. N18 Renode functional validation targets functional change; no new functional behavior requires emulated-target re-run. Existing tree-level Renode evidence remains valid. Not V&V-blocking.

---

## 6. Blocking findings

**None.** No Major findings. No finding classifies as blocking commit/push.

---

## 7. Conclusion and recommendation

The `ICD.md:106` annotation is V&V-accurate: Service 1 subservices 4 (start failure) and 6 (progress failure) are not emitted on the wire by the production TC-ingestion path (`STAGE_START` and `STAGE_PROGRESS` are always acknowledged with `success=1` at `pus.c:451` and `pus.c:456`), so "(reserved, not emitted)" correctly reflects implementation behavior and implements the NC-EXT-94 disposition. The change is documentation-only, affects no test behavior, and triggers no N10 re-qualification.

**V&V verdict:** No V&V findings block commit/push. The change may proceed to commit/push subject to the separate, non-V&V gates: N7 dual push (`aeronautics-space-fsw-engineer` authority), N9/N14 independent reviews, N17 documentation review, N20 PA review, and the N15 full-cycle re-run as applicable. This independent R2 reviewer records "do not block" and does not authorize release.

**Approval boundary:** This report is independent V&V evidence only. It does not waive any safety/security/regulatory/contractual gate, does not close any NC (NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 are not touched), and does not constitute authority acceptance or flight readiness.

---
*End of report. Finding IDs: VV-R2-001 … VV-R2-006. All classified Minor (confirming or clarification); none blocks commit/push.*
