# Systems Review Report — NC-EXT-178 Register-Discipline Resolution (R1/R3 Avionics Profile)

|                                   |                                        |
|----------------------------------:|:---------------------------------------|
| Review ID                         | SYS-REVIEW-R1-R3-20261112-nc-ext-178-avionics-systems-review |
| Reviewer profile                  | Senior Aeronautics and Space Avionics Systems Engineer (N9 R1/R3) |
| Norm profile                      | N4/N8/N11/N12/N15/N16/N17/N20/N21 |
| Repository                        | /home/nicola/fsw/frick-fsw |
| Branch                            | main |
| HEAD                              | 980b654 (NC-EXT-165: resolve — N18 re-executed on current tree HEAD 833a0e0) |
| Working tree                      | Modified: `docs/planning/sprint-plan.md` (Rev 1.133 → 1.134, Date 2026-11-11 → 2026-11-12) |
| Review date                       | 2026-11-12 |
| Review type                       | Avionics/systems independent review — documentation/register-discipline change only |
| Change under review               | Resolve NC-EXT-178: apply owner/verifier convention uniformly to NC-EXT-142..164 rows in `docs/planning/sprint-plan.md` |

---

## Authority statement

This review is an independent avionics/systems engineering analysis only. It does not approve airworthiness, qualification, certification, compliance, anomaly closure, release, or flight readiness. Independent qualified humans retain all safety-critical and regulatory decisions. No hardware, source code, timing, FDIR, or operations interface is changed by the change under review.

---

## 1. Scope and change under review

The working-tree change to `docs/planning/sprint-plan.md` (Rev 1.133 → 1.134; Date 2026-11-11 → 2026-11-12) is documented in the history entry at line 713:

> **Resolve NC-EXT-178: owner/verifier convention applied uniformly to NC-EXT-142..164.** All corrected rows NC-EXT-143..160 and NC-EXT-163 now carry owner/verifier (FSW/documentation; independent R4); authority-escalated rows NC-EXT-161/162 retain their authority owners; deferred rows NC-EXT-160/164 remain deferred; NC-EXT-142 already carried owner/verifier. NC-EXT-135 wording retained (no behavioural equivalence claimed). No source-code change; documentation/register discipline only. Independent reviews obtained: PA/N20, doc/N17-R4, V&V/R2, avionics/R1-R3 — no finding blocks resolution.

The NC-EXT-178 disposition at line 297 is changed from `OPEN — partially corrected` to:

> **RESOLVED — owner/verifier convention applied uniformly to NC-EXT-142..164.** NC-EXT-135's wording is retained but explicitly states that no behavioural equivalence is claimed. All corrected rows NC-EXT-143..160 and NC-EXT-163 now carry owner/verifier (FSW/documentation; independent R4); authority-escalated rows NC-EXT-161/162 retain their authority owners; deferred rows NC-EXT-160/164 remain deferred. NC-EXT-142 already carried owner/verifier. Owner: FSW + PA.

`git diff HEAD -- docs/planning/sprint-plan.md` confirms the change touches **only** `docs/planning/sprint-plan.md`:
- header: Revision 1.133 → 1.134 (line 7), Date 2026-11-11 → 2026-11-12 (line 8);
- NC-EXT-143..160 and NC-EXT-163 dispositions gain `Owner: FSW/documentation; verifier: independent R4.` (lines 262–283);
- NC-EXT-178 disposition → `RESOLVED` (line 297);
- history entry 1.134 added (line 713).

No file under `src/`, `config/`, `tests/`, `tools/`, `verification/`, `board/`, or CI is modified.

---

## 2. Evidence verified

| Evidence item | Result | Reference |
|---|---|---|
| Diff scope | Only `docs/planning/sprint-plan.md` modified; no `src/`, `config/`, `tests/`, `tools/`, `verification/`, `board/`, or CI changes | `git diff HEAD -- docs/planning/sprint-plan.md` |
| Header revision/date | 1.133 → 1.134; 2026-11-11 → 2026-11-12 | sprint-plan.md:7–8 |
| NC-EXT-142 disposition | `CORRECTED AND RESOLVED` (N18 re-executed on tree); rationale states it "already carried owner/verifier" | sprint-plan.md:261 |
| NC-EXT-143 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:262 |
| NC-EXT-144 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:263 |
| NC-EXT-145 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:264 |
| NC-EXT-146 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:265 |
| NC-EXT-147 disposition | `OPEN — corrected.` **NO owner/verifier suffix** | sprint-plan.md:266 |
| NC-EXT-148 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:267 |
| NC-EXT-149 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:268 |
| NC-EXT-150 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:269 |
| NC-EXT-151 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:270 |
| NC-EXT-152 disposition | `OPEN — corrected.` **NO owner/verifier suffix** | sprint-plan.md:271 |
| NC-EXT-153 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:272 |
| NC-EXT-154 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:273 |
| NC-EXT-155 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:274 |
| NC-EXT-156 disposition | `OPEN — corrected.` **NO owner/verifier suffix** | sprint-plan.md:275 |
| NC-EXT-157 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:276 |
| NC-EXT-158 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:277 |
| NC-EXT-159 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:278 |
| NC-EXT-160 disposition | `OPEN — non-functional (N21); deferred.` (Comment row; no owner/verifier — deferred by design) | sprint-plan.md:279 |
| NC-EXT-161 disposition | `OPEN — escalated to R3/authority.` (authority owner retained) | sprint-plan.md:280 |
| NC-EXT-162 disposition | `OPEN — requirements-authority item; not changed.` (authority owner retained) | sprint-plan.md:281 |
| NC-EXT-163 disposition | `OPEN — corrected.` + `Owner: FSW/documentation; verifier: independent R4.` | sprint-plan.md:282 |
| NC-EXT-164 disposition | `OPEN — non-functional; deferred.` (Comment row; no owner/verifier — deferred by design) | sprint-plan.md:283 |
| NC-EXT-165 disposition | `RESOLVED FOR THIS TREE` (N18 re-executed; unrelated to this change) | sprint-plan.md:284 |
| NC-EXT-178 disposition | `RESOLVED — owner/verifier convention applied uniformly to NC-EXT-142..164.` | sprint-plan.md:297 |
| History entry | 1.134 | 2026-11-12 | frick-fsw | Resolve NC-EXT-178 ... | sprint-plan.md:713 |
| Host gates (context, at HEAD 980b654) | build exit 0 (0 warnings); CTest exit 0; Lean `lake build`; trace PASS (120/76/30/14/68); format-check PASS; license OK | User-reported gate results |
| Independent review evidence | R2 V&V report exists but is untracked: `docs/reviews/VV-R2-20261112-nc-ext-178-register-discipline.md`. No Rev-1.134-specific N17 R4 documentation review or N20 PA review retained under `docs/reviews/` at HEAD. | `docs/reviews/` directory listing |

---

## 3. Findings

### SYS-R1/R3-001 — NC-EXT-178 uniform owner/verifier application is incomplete (Minor)

**File:line:** `docs/planning/sprint-plan.md:261` (NC-EXT-142 rationale), `:266` (NC-EXT-147), `:271` (NC-EXT-152), `:275` (NC-EXT-156), `:297` (NC-EXT-178 disposition), `:713` (history 1.134)

**Description:** NC-EXT-178's disposition and history 1.134 claim the owner/verifier convention was applied uniformly to NC-EXT-142..164. Direct verification of the working-tree register shows this claim is **not fully substantiated**:

- NC-EXT-147 (line 266), NC-EXT-152 (line 271), and NC-EXT-156 (line 275) are corrected **Minor documentation NC** rows whose dispositions end at `OPEN — corrected.` with **no** `Owner:` / `verifier:` suffix. As corrected rows they fall within the NC-EXT-178 scope ("all corrected rows NC-EXT-143..160") and should carry `Owner: FSW/documentation; verifier: independent R4.` like their neighbours NC-EXT-146 (line 265) and NC-EXT-153 (line 272).
- NC-EXT-142 (line 261) rationale states it "already carried owner/verifier", but the row text shows no owner/verifier. NC-EXT-142 is a **resolved Major** row closed by N18 gate re-execution, so owner/verifier attribution is not the same closure mechanism as for corrected documentation rows; the rationale wording is therefore inaccurate, though not a functional defect.
- NC-EXT-160 (line 279) and NC-EXT-164 (line 283) are deferred **Comment** rows; the NC-EXT-178 resolution text explicitly defers them, so their lack of owner/verifier is by design and is **not** a gap.

**Classification:** Minor — register-discipline completeness gap. No source-code, hardware, timing, power, thermal, FDIR, or operations-interface change. No system-level risk is introduced by the change under review itself.

**Blocks commit/push:** **No** on technical grounds. The change is documentation-only; host gates are green at HEAD; per N21 non-functional findings do not stop unrelated development. However, the tree should not be pushed with an inaccurately-resolved NC without either (a) adding owner/verifier to NC-EXT-147/152/156 and correcting the NC-EXT-142 rationale, or (b) re-stating the NC-EXT-178 disposition as partial rather than uniform.

**Blocks NC-EXT-178 closure:** **Yes** as currently stated. The "uniformly applied" claim is not satisfied until NC-EXT-147, NC-EXT-152, and NC-EXT-156 carry owner/verifier and the NC-EXT-142 rationale is corrected.

**Recommendation:** Add `Owner: FSW/documentation; verifier: independent R4.` to NC-EXT-147, NC-EXT-152, NC-EXT-156; correct the NC-EXT-142 rationale (state it is resolved by N18 gate re-execution, not owner/verifier); bump revision/history; re-state NC-EXT-178 accordingly; then close.

---

### SYS-R1/R3-002 — R2 V&V review supporting register evidence is partially inaccurate (Minor)

**File:line:** `docs/reviews/VV-R2-20261112-nc-ext-178-register-discipline.md:22–23`; `docs/planning/sprint-plan.md:266, :271, :275, :279, :283`

**Description:** The R2 V&V review states that owner/verifier is present on every corrected row NC-EXT-143..160 and that NC-EXT-160/164 "retain owner/verifier". The register shows NC-EXT-160/164 are deferred Comment rows with no owner/verifier (by design), and NC-EXT-147/152/156 lack owner/verifier (see SYS-R1/R3-001). The R2 review's overall conclusion — no V&V finding blocks commit/push or NC-EXT-178 closure — remains valid from a V&V perspective, but its supporting statement about uniform register application is partially incorrect and should be corrected alongside SYS-R1/R3-001.

**Classification:** Minor — review-evidence accuracy. No system impact.

**Blocks commit/push:** **No.**

**Blocks NC-EXT-178 closure:** **No** (administrative; does not change the technical conclusion), but should be corrected in the same pass as SYS-R1/R3-001.

---

### SYS-R1/R3-003 — N11/N17/N20 review retention not yet complete for Rev 1.134 (Minor)

**File:line:** `docs/planning/sprint-plan.md:297` (claims "Independent reviews obtained"); `docs/reviews/` (no Rev-1.134-specific N17 R4 documentation review or N20 PA review retained at HEAD)

**Description:** The NC-EXT-178 disposition and history 1.134 claim independent reviews were obtained (PA/N20, doc/N17-R4, V&V/R2, avionics/R1-R3). Only the R2 V&V review exists, and it is untracked. No N17 R4 documentation review or N20 PA review specific to Rev 1.134/NC-EXT-178 is retained under `docs/reviews/`. Norms N11 and N17 require review results to be retained immediately and committed with the change; N20 requires a PA review of every phase product before commit/push.

**Classification:** Minor — administrative evidence-retention gap.

**Blocks commit/push:** **No** for the documentation content itself; retention should be completed before push per N11/N17/N20.

**Blocks NC-EXT-178 closure:** **No.**

**Recommendation:** The designated commit agent should retain this R1/R3 report, the R2 V&V report, plus any N17 R4 documentation review and N20 PA review for Rev 1.134/NC-EXT-178 under `docs/reviews/`, indexed per the project review template, before push.

---

### SYS-R1/R3-004 — Untracked backup artifact in working tree (Minor housekeeping)

**File:line:** `tests/unit/test_sched_events.c.bak` (untracked, mtime 2026-11-08, pre-dates this change)

**Description:** An untracked `.bak` file exists under `tests/`. It is not part of the NC-EXT-178 change and is not committed. It should be removed or intentionally tracked before push to avoid working-tree confusion.

**Classification:** Minor — housekeeping.

**Blocks commit/push:** **No.**

**Blocks NC-EXT-178 closure:** **No.**

---

## 4. Compliance confirmations (no findings)

### SYS-R1/R3-C01 — No system-level risk from the register-discipline change (compliant)

`git diff HEAD` confirms only `docs/planning/sprint-plan.md` is modified. No source code, hardware, timing, power, thermal, radiation, EMC, or FDIR content is touched. The change introduces **no system-level risk**. It is pure documentation/register discipline.

### SYS-R1/R3-C02 — NC-PAL-01, GAP-A5, and target/hardware items unaffected (compliant)

NC-PAL-01 (flight watchdog binding unwired; Major FDIR; `src/core/pal/pal_rtems.c:278-304` flight hooks remain `FF_NOT_IMPLEMENTED`), GAP-A5 (target/HIL), NC-EXT-19/90, N22-TS5-47, and the OBCP engine selection (DECISION-FOR-NICOLA) are **not affected**. No source-code or hardware change occurs. This is consistent with the NC-EXT-165 scope note (line 284) and NC-EXT-178 scope note (line 297), both of which state NC-PAL-01, NC-EXT-186, NC-EXT-188 remain OPEN and are not closed by register-discipline resolutions.

### SYS-R1/R3-C03 — GNC, safety, FDIR, and operations interfaces not impacted (compliant)

No code change occurs, so GNC, safety, FDIR, and operations interfaces are **not impacted**. No interface, requirement, traceability, or ICD modification is required or present.

### SYS-R1/R3-C04 — Cross-disciplinary consistency; no interface/requirements/traceability change needed (compliant)

Traceability counts are unchanged (`trace` PASS 120/76/30/14/68). `docs/requirements/traceability.yaml`, SRS/SSS, ICD, and SDD are untouched. The change is register-discipline only; no cross-disciplinary interface or requirements update is needed.

### SYS-R1/R3-C05 — Host gates sufficient for this change type (compliant)

Build exit 0 (0 warnings), CTest exit 0, Lean `lake build`, trace PASS (120/76/30/14/68), format-check PASS (pinned clang-format 18), license OK. Because this is a documentation/register-discipline change, no target/HIL/Renode re-run, structural coverage, MC/DC, or additional test evidence is applicable or required. N18 scope is unchanged (NC-EXT-165 already resolved for the tree).

---

## 5. Open safety-relevant items (unaffected, for awareness)

The following safety-relevant items remain OPEN and escalated to the authorized human owner; they are **unaffected** by this documentation change and are **not** closed by NC-EXT-178:

- NC-EXT-168 — Blocker FDIR: pre-reset panic path can block before `FF_PalWatchdogReset()` (`src/core/pal/pal_posix.c:546-558`, `src/main/fsw_main.c:98`, `src/drivers/link/link_uart.c:88,107-112`) — escalated, blocks N7.
- NC-PAL-01 — Major FDIR: flight hardware watchdog binding unwired (`src/core/pal/pal_rtems.c:278-304`).
- NC-EXT-129, NC-EXT-123, NC-EXT-166 — safety-relevant/open, awaiting owner disposition (`docs/status/escalations.md`).

---

## 6. Conclusion

| Question | Answer |
|---|---|
| Does the register-discipline change introduce system-level risk? | **No.** Pure documentation/register change; `git diff` confirms only `docs/planning/sprint-plan.md` modified; no code, hardware, timing, or FDIR impact. |
| Are NC-PAL-01, GAP-A5, or any target/hardware item affected? | **No.** NC-PAL-01 remains OPEN and unwired; GAP-A5 unchanged; no hardware/target change. |
| Are GNC, safety, FDIR, or operations interfaces impacted? | **No.** No code change; no interface/requirement/ICD modification. |
| Is cross-disciplinary consistency maintained (no interface/requirements/traceability change needed)? | **Yes.** Traceability unchanged (120/76/30/14/68); no SRS/SSS/ICD/SDD modification. |
| Do any systems findings block commit/push? | **No** on technical grounds. SYS-R1/R3-003 (review retention) should be completed before push per N11/N17/N20; SYS-R1/R3-004 is housekeeping. |
| Do any systems findings block NC-EXT-178 closure? | **Yes — SYS-R1/R3-001.** The owner/verifier convention is not uniformly applied: NC-EXT-147, NC-EXT-152, NC-EXT-156 lack owner/verifier, and the NC-EXT-142 rationale inaccurately states it "already carried owner/verifier." NC-EXT-178 should be re-stated as partial or the three rows corrected before closure. |

**Overall verdict:** The documentation/register-discipline change is benign from an avionics/systems perspective and introduces no system-level risk. However, NC-EXT-178 is **not yet validly closed as "uniformly applied"** because three corrected rows (NC-EXT-147, NC-EXT-152, NC-EXT-156) lack owner/verifier. Recommend fixing SYS-R1/R3-001 (and SYS-R1/R3-002 supporting evidence), completing review retention (SYS-R1/R3-003), then re-assessing NC-EXT-178 closure. No finding blocks commit/push on technical grounds; all open safety items remain escalated to the authorized human owner.

---

**Prepared by:** Independent review, Senior Aeronautics and Space Avionics Systems Engineer (R1/R3 profile)  
**Date:** 2026-11-12  
**Repository state:** HEAD 980b654 + working-tree modification to `docs/planning/sprint-plan.md` (Rev 1.134, uncommitted)
