# Review Report — M16 delta (SSS/SRS closure) and SDD requirements->design matrix (Annex F <6>)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                       | FRICK-REVIEW-0072                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R3 (Senior Systems Engineer, space/aero; N9/N14) |
| Reference revision                | HEAD `039e47f` (branch `main`) + working-tree changes in `docs/design/software-design.md`, `README.md`, `tools/scripts/metrics.py` (uncommitted), 2026-10-11 |
| Items under review                | (A) delta re-review of `FRICK-REVIEW-0064` closures: `docs/requirements/SSS.md` (SSS-REQ-027), `docs/requirements/SRS.md` (SRS-REQ-054), `docs/requirements/traceability.md`/`.yaml`, `docs/roadmap.md`, `docs/development-plan.md`, `docs/architecture/component-gap-analysis.md`, `docs/architecture/pus-c-from-scratch-assessment.md`, `docs/status/{handover.md,loop-audit.md,status.json}`; (B) the new SDD §6 requirements->design traceability matrix in `docs/design/software-design.md` (ECSS-E-ST-40C Annex F <6>), added to close NC `PA-67-01` (from `FRICK-REVIEW-0067`), and its consistency with the SRS and `traceability.yaml` |
| Date                              | 2026-10-11                             |
| Scope                             | (A) whether each of the 7 Minor NCs of FRICK-REVIEW-0064 (NC-64-1..7) is closed, partially closed or open, with file:line evidence, after the M16 closure commit `3fe5bf0`; (B) ECSS SDD DRD Annex F <6> correctness of the new matrix: is every SRS requirement allocated to a credible design element, are planned modules honestly marked, is the mapping consistent with the SRS and `traceability.yaml`, and does it discharge NC PA-67-01. Excluded: implementation/code (no OBCP code), toolchain/emulator execution, the R4/N17 and PA/N20 review scopes except where a systems-visible consistency defect overlaps. |
| Independence declaration          | Reviewer took no part in authoring, implementing or reviewing any item under review; fresh independent session with no prior context. Read-only review: only this report file was created; no other repository file was modified. |
| Norms applied                     | N3, N4, N5, N8, N9, N11, N12, N14, N15, N16, N19, N20 (see `docs/rules.md`) |

## 1. Scope

This is a two-part systems-engineering review at HEAD `039e47f`.

**Part A** re-reviews the seven Minor non-conformances raised against the M16
change by `FRICK-REVIEW-0064` (NC-64-1..7) plus its three Comments, after the
closure commit `3fe5bf0` (Sprint plan + M16 closures + ECSS traceability NC).
The delta items announced by the project are: SSS-REQ-027 made
singular/bounded; SRS-REQ-054 method aligned to `Tst | Dem`; `traceability.md`
regenerated (55/27/14 + the new rows + SSS summary); the M12/M16 split
propagated (roadmap / OBJ-24 / component-gap-analysis); the gap linkage updated;
and the roadmap M16 section restored to numeric order. Each prior Minor is
verified closed (evidence file:line) or its residual is stated.

**Part B** reviews the new `docs/design/software-design.md` §6
"Design traceability matrix (SRS -> design component)", added **uncommitted** in
the working tree to discharge NC **PA-67-01** (opened by `FRICK-REVIEW-0067`:
the SDD does not present the SDD-DRD Annex F <6>a requirements->design
traceability matrices). The check is: (i) every SRS requirement is allocated to a
credible design element; (ii) planned modules are honestly marked; (iii) the
mapping is consistent with the SRS and with `traceability.yaml`; (iv) whether
PA-67-01 is closed from the systems side.

**Assumptions.** HEAD is a planned-only baseline for M16 (no OBCP source:
`grep -rn "obcp" src/` is empty). The authoritative traceability source is
`traceability.yaml`, validated by `tools/scripts/trace.py`. The SDD §6 and the
`README.md`/`metrics.py` design-traceability change are working-tree edits, not
committed at `039e47f`.

## 2. References

- Project norms: `docs/rules.md` (N1-N20), in particular N4 (bidirectional
  traceability), N8 (docs current at each change), N11 (records committed each
  cycle), N12 (NC closure), N14 (this review), N16 (revision history), N17/N20
  (sibling reviews).
- Prior reviews under closure: `docs/reviews/2026-10-11-m16-sss-r3-review.md`
  (FRICK-REVIEW-0064), `docs/reviews/2026-10-11-m16-docs-r4-review.md`
  (FRICK-REVIEW-0065), `docs/reviews/2026-10-11-m16-pa-review.md`
  (FRICK-REVIEW-0066), `docs/reviews/2026-10-11-ecss-traceability-pa-review.md`
  (FRICK-REVIEW-0067, defines NC PA-67-01).
- Requirements/design: `docs/requirements/SSS.md`, `docs/requirements/SRS.md`,
  `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`,
  `docs/design/software-design.md`.
- Plan/status: `docs/roadmap.md`, `docs/development-plan.md`,
  `docs/architecture/component-gap-analysis.md`,
  `docs/architecture/pus-c-from-scratch-assessment.md`,
  `docs/status/{handover.md,loop-audit.md,status.json}`,
  `docs/planning/sprint-plan.md`.
- Quality: `docs/quality/ecss-compliance-matrix.md`, `docs/quality/class-a-gaps.md`.
- Standards context: ECSS-E-ST-40C (Rev.1, Annex F <6>, 5.5.2.x, 5.8.3.x, Annex R
  Table R-1), ECSS-Q-ST-80C (6.2.6.12a), ECSS-E-ST-70-41C (PUS-C service 18),
  CCSDS 133.0-B-2. Full standard texts were not re-acquired; the clause reading
  in `FRICK-REVIEW-0067` is taken as the applicable basis.

## 3. Findings

### Part A — FRICK-REVIEW-0064 closure status

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| A-1 | **Minor** (NC-64-2, remains open) | `SSS-REQ-027` / `SRS-REQ-054` (boundedness/testability) | The reworded requirements now **name** the bound categories ("bounded engine memory and one execution context per OBCP", "bounded per-OBCP execution-time budget", "bounded abort latency") but still carry **no quantified limit, acceptance criterion or `TBD/TBC` record** with owner/gate; and SRS-REQ-054 still says "ECSS-compliant fallback" without naming the normative reference. The requirement is clearer but still not objectively testable at this level. | `SSS.md:118-124`; `SRS.md:272-278` (`ECSS-compliant` at `:276`). Compare `SSS.md:121-123`. | State numeric bounds (or explicit `TBC` with owner + decision gate) and cite the exact baseline (PUS-C S18 + ECSS-E-ST-40C/Q-ST-80C). |
| A-2 | **Minor** (NC-64-4, remains open) | development-plan / component-gap-analysis (M12/M16 split) | The split is propagated to the **roadmap** (`roadmap.md:153-162` M12 = integrate; `:192-201` M16 = develop) and to **OBJ-24** (`development-plan.md:59`), but `OBJ-20` and development-plan **§4 M12** still frame the in-house engine as **M12's own fallback**, and component-gap-analysis **§5 "Fallback"** still does so under the M12 context. The **M12 fallback vs M16 relationship remains undefined** (conditional vs definite; who delivers it and when). | `development-plan.md:55` ("fallback: an ECSS-compliant engine"), `:217-218` ("If not integrable, develop an ECSS-compliant engine ..."); `component-gap-analysis.md:104-106`; reframed only at `roadmap.md:160-162`, `development-plan.md:59`. | Reframe OBJ-20 and §4 M12 like the roadmap ("M12 integrates an engine; the in-house engine is M16") and state the M12 exit/fallback rule explicitly. |
| A-3 | **Minor** (NC-64-7, remains open) | M16 execution order vs PUS S18 surface | M16 is still scheduled **after M12, before M13** (`roadmap.md:200`), while the PUS **S18** surface that M16's scope needs is claimed by **SRS-REQ-052 (M15)**, and M15 is scheduled **last** (`roadmap.md:181-190`); `pus-c-from-scratch-assessment.md:53` still attributes the S18 surface to "(M12 surface)". No milestone is defined as the definitive owner of the S18 state/management surface, so a milestone ordered before M14/M15 still depends on a later deliverable. | `roadmap.md:181-201`; `SRS.md:260-265`; `pus-c-from-scratch-assessment.md:53`; `development-plan.md:58`. | State unambiguously which milestone delivers the PUS S18 surface/state model and reconcile the M16 execution order (carve the S18 seam into M12/M16, or reorder M16 after the S18-capable increment). |
| A-4 | **Minor** (N16, residual of NC-64-6) | `docs/requirements/traceability.md` | The mirror was regenerated (rows `SSS-REQ-027`/`SRS-REQ-054`, counts 55/27/14, SSS summary completed) but the **revision history is defective**: the header is `0.23` (`:7`) while the latest History row is `0.22` and row `0.21` appears **twice** (M16 row inserted before the M11 rows without renumbering). | `traceability.md:7` vs `:204-206` (duplicate `0.21`, no `0.23` row). | Renumber the inserted M16 history row to `0.23` and re-order the M11/M16 rows chronologically. |
| A-5 | **Minor** (N4/N8, residual of NC-64-6) | `docs/requirements/traceability.md` (planned note) | The planned-requirements note now reads "**Four** of the requirements (SRS-REQ-045/046/054, SSS-REQ-022/023/027) are **planned**" — it lists **six** ids and still **omits** the other planned (documented-only) requirements `SRS-REQ-051/052/053` and `SSS-REQ-024/025/026`, all of which are marked planned in the manifest and the tables below. | `traceability.md:20-22` vs `:111-113` (SRS-REQ-051/052/053 planned) and `:296-306` rows; `traceability.yaml:480-514`, `:692-722`. | Correct the count and list every planned requirement (or cite the manifest instead of enumerating). |
| A-6 | **Minor** (N16, residual of NC-64-4/NC-64-6) | `docs/development-plan.md` | The plan was edited (OBJ order fixed, OBJ-24 added, header bumped to `0.15`) but the **History is defective**: header `0.15` (`:7`) with the last History row `0.14` and row `0.13` duplicated (the M16 row inserted without renumbering). | `development-plan.md:7` vs `:331-333` (duplicate `0.13`, no `0.15` row). | Renumber the inserted history row to `0.15`. |
| A-7 | **Minor** (N16/N8, cross-ref PA-66-4) | `docs/status/handover.md` | M16 was added to §4/§5, but the header `Revision` is still `1.1` while the latest History row is `1.2`, and the authoritative resume point still says "as of commit `967f563`" (HEAD is `039e47f`). | `handover.md:7` (`1.1`) vs history tail (`1.2`); `:87` ("as of commit 967f563"). | Set the header to the latest History revision and refresh the "as of" pointer. |
| A-8 | Comment | `docs/architecture/component-gap-analysis.md` | §1 purpose still says "the decisions taken in the development plan (milestones M12/M13)"; M16 is now a plan decision but is not named. | `component-gap-analysis.md:16`; §6 updated at `:115-121`. | Extend to "M12/M13/M16". |
| A-9 | Comment (NC-64-3 residual) | `SRS-REQ-054` | The level-to-level method mismatch is **fixed** (`SRS.md:278` now `Tst | Dem` = `SSS.md:123`), but the "MISRA-oriented" attribute still has no `Inp | Anl` method (MISRA is verified by inspection/analysis at `SRS-REQ-041`). | `SRS.md:272-278`; `SRS.md:204-206`. | Add `Inp | Anl` for the MISRA aspect or justify the reduction in the manifest. |
| A-10 | Comment (NC-64-8/9/10a residuals) | SSS/SRS wording; roadmap | Outstanding advisories: milestone tag + stray blank line still in SSS (`SSS.md:117`, `:124`); `SRS-REQ-054` still grouped under the "Code quality (milestone M9)" subsection (`SRS.md:202`, `:272`); "ECSS-E-ST-70 series" still cited loosely (`component-gap-analysis.md:106`, `development-plan.md:218`). | as cited. | Advisory (no derogation); keep on the backlog. |
| A-11 | Comment (cross-ref PA-66-7) | `docs/status/status.json` | The review semaphores `documentation_review`, `product_assurance_review` and `reviews` remain `green` while the M16 reviews are open; M16 is correctly `planned` everywhere. | `status.json:24,35,36,39`; `deferred: []` at `:48`. | Until the loop closes, mark the open-review state explicitly; flip to green only at the closure commit. |

Closure summary of the prior 7 Minors: **closed** NC-64-1 (rationale removed;
the delta is now a distinct engine requirement that references SSS-REQ-022
rather than duplicating it — `SSS.md:118-124`), NC-64-3 (method aligned,
`SRS.md:278`), NC-64-5 (gap linkage now names M16/SSS-REQ-027/SRS-REQ-054 —
`component-gap-analysis.md:10,115-121`). **Partially closed** NC-64-6
(regenerated, but History/numbering and planned-note defects: A-4/A-5).
**Open** NC-64-2 (A-1), NC-64-4 (A-2/A-6), NC-64-7 (A-3). Comments 8/9/10a not
closed; 10b closed (`roadmap.md:192` M16 now after M15).

### Part B — SDD §6 requirements->design matrix

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| B-1 | **Minor** | SDD §6 matrix vs `traceability.yaml` (`SRS-REQ-039`/`SRS-REQ-040`) | `SRS-REQ-039` (reference network **stack**) and `SRS-REQ-040` (lwIP network-stack **adapter**) are allocated to the design element "**Link layer** `src/transport/link/link.c`". The link layer is the byte transport, not the network stack; the manifest implements 039 in `src/drivers/net/ff_net_loopback.c` and 040 in `src/hal/net/lwip/ff_net_lwip.c`, and `SRS-REQ-027` (pluggable stack contract) is already allocated to the HAL row. `SRS-REQ-040` is also **listed twice** (Link layer row and Reference drivers row), and its actual `src/hal/net/lwip/` path is not in the Reference drivers path set. The allocation is therefore not credible and is inconsistent with the manifest. | `software-design.md:109` vs `traceability.yaml:371-387`; `software-design.md:110` (`SRS-REQ-027` in HAL); `:113` (Reference drivers: `SRS-REQ-040`, paths `src/drivers/{...,net}`). | Add a network-stack design element (contract `src/hal/hal_net.h`; backends `src/drivers/net/ff_net_loopback.c`, `src/hal/net/lwip/ff_net_lwip.c`) and allocate 039/040 there once, removing the duplicate. |
| B-2 | **Minor** | `docs/quality/ecss-compliance-matrix.md` (PA-67-01 corrective action #4) | The compliance matrix was **not corrected**: it still states the SDD is "present (added for this matrix)" and that "requirements, design, interfaces … are all present and traced", which overstates the design traceability the NC identified; `GAP-A9` is still filed as class-A although `5.5.2.x`/`5.8.3.4a` apply at criticality B. This is an explicit item of the NC's corrective action and was left untouched. | `ecss-compliance-matrix.md:26`, `:94-99`, `:85`; NC PA-67-01 corrective action in `2026-10-11-ecss-traceability-pa-review.md:273`; sprint-plan S0 lists it (`sprint-plan.md`, S0 row). | Correct `ecss-compliance-matrix.md` to state design traceability truthfully and either re-scope `GAP-A9` onto the B baseline or justify its class-A scoping. |
| B-3 | **Minor** | SDD §6 (PA-67-01 corrective actions #2/#3) | The matrix provides a component→file mapping but there is still **no detailed design** and no explicit statement/justification that "design component = software unit" at this scale, so the code→detailed-design trace (Annex M <4.4>a.1) is still not demonstrable; and there is **no record of the milestone verification of the matrices** required by ECSS-Q-ST-80C 6.2.6.12a (PDR/CDR/QR). | `software-design.md:92-127`, `:129-134`; `traceability.yaml:789-795` (`GAP-A9`); NC PA-67-01 items 2/3 (`2026-10-11-ecss-traceability-pa-review.md:273`). | Add the detailed-design/unit dimension (or the explicit simplification + justification) and record the milestone verification of the matrices per Q-ST-80C 6.2.6.12a. |
| B-4 | **Minor** (N8/N20 truthfulness) | `README.md` / `tools/scripts/metrics.py` | The README now reports "Design traceability | 55/55 (100%)" and states flatly "the SDD carries the requirements->design matrices (SDD section 6)", and the open-NC `PA-67-01` marker was removed from the metrics note, while PA-67-01 is **not closed** (B-2, B-3) and the metric is a **string-presence proxy** (`design_ok = sum(1 for i in srs if i["id"] in sdd)`), not a verified per-requirement allocation. This can be read as the NC being discharged. | `README.md:54`, `:92`; `metrics.py:199-202`, `:251`; NC still open at `2026-10-11-ecss-traceability-pa-review.md:366`, `loop-audit.md:146`. | Keep the open-NC marker until PA-67-01 is verified closed, and describe the metric as a proxy (or strengthen it to check allocation within the §6 matrix). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Verification performed (evidence)

| Check | Command / method | Result |
| ----- | ---------------- | ------ |
| Traceability gate (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — `96 requirements, 55 srs, 27 sss, 14 gaps, 51 lean theorems checked` (exit 0); `SSS-REQ-027 <- 1 srs child` |
| Metrics currency (N8/N10) | `python3 tools/scripts/metrics.py --check` | **PASS** — `metrics panel: current` (exit 0) |
| SRS allocation completeness | parsed SRS.md ids and the SDD §6 matrix ids | All 55 SRS ids (`FRICK-SRS-REQ-000`, `SRS-REQ-001..054`) appear in §6; **no missing allocation**; **one duplicate** (`SRS-REQ-040`) |
| Allocation vs manifest | compared §6 design-element paths with `traceability.yaml` `implementation` | Mismatch for `SRS-REQ-039`/`SRS-REQ-040` (B-1); others consistent |
| Method alignment | `SSS-REQ-027` vs `SRS-REQ-054` | `Tst | Dem` both (`SSS.md:123`, `SRS.md:278`) — NC-64-3 fixed |
| M16 requirement presence | `grep -rn "obcp" src/` | empty — consistent with a planned milestone (no code) |
| Working tree | `git status --porcelain` | `M README.md`, `M docs/design/software-design.md`, `M tools/scripts/metrics.py` (SDD §6 uncommitted; only this report added by the reviewer) |
| Planned marking honesty | SDD §6 rows `:118-123` | M12-M16 rows marked *planned* with `TBD`; no planned requirement shown as implemented |
| Milestone status truthfulness | `status.json`, `roadmap.md`, `development-plan.md` §6, `loop-audit.md` | M16 = `planned` in all; no record claims M16 delivered |

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Graded conclusion**

- **R3 (Systems Engineering, N14): APPROVED WITH COMMENTS** (0 Major, 11 Minor,
  5 Comments).

**Part A.** The delta genuinely closes three of the seven prior Minors and
materially improves the rest: `SSS-REQ-027` no longer embeds its rationale and is
now distinguishable from `SSS-REQ-022`; the verification methods are aligned;
and the gap linkage, the human traceability mirror and the roadmap order are
corrected. However, **FRICK-REVIEW-0064 is not closed**: NC-64-2 (no quantified
bounds/`TBC`), NC-64-4 (M12 fallback vs M16 relationship still undefined in
OBJ-20 and development-plan §4 M12), and NC-64-7 (M16 execution order vs the PUS
S18 surface owned by M15) remain open, and NC-64-6 is only partially closed (the
regenerated `traceability.md` carries a broken revision history and an incorrect
planned-requirements note; `development-plan.md` and `handover.md` have
header/History mismatches). None of these is Major: the requirements are uniquely
identified, correctly and machine-verifiably linked
(`trace.py` PASS 96/55/27/14/51), attributed to M16, and no requirement or safety
barrier is defeated.

**Part B.** The §6 matrix is a credible first materialisation of the Annex F <6>
requirements->design trace: **every** SRS requirement is allocated, planned
modules are honestly marked, and a design-component→file mapping is present.
It does **not** fully discharge **NC PA-67-01** from the systems side: two
requirements are misallocated to a non-credible design element and one is
duplicated (B-1), the compliance-matrix correction and the Q-ST-80C 6.2.6.12a
milestone-verification record (NC corrective actions #3/#4) are missing (B-2,
B-3), and the README/metrics change removes the open-NC marker and claims 100%
design traceability while the NC is still open (B-4). PA-67-01 therefore
**remains open** pending those items.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | Independent systems-engineering review agent (fresh context) | 2026-10-11 | **APPROVED WITH COMMENTS** (0 Major, 11 Minor, 5 Comments) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-64-1 | Minor | SSS-REQ-027 | Not singular; rationale embedded; duplicated SSS-REQ-022 | **closed**: rationale removed, delta expressed distinctly, parent referenced | SSS.md:118-124 | frick-fsw | closed | 2026-10-11 (this review) |
| NC-64-2 | Minor | SSS-REQ-027 / SRS-REQ-054 | Not bounded/testable (no quantified bounds/TBC; unnamed baseline) | open — see NC-72-1 | SSS.md:118-124; SRS.md:272-278 | frick-fsw | **open** | 2026-10-11 (this review) |
| NC-64-3 | Minor | SRS-REQ-054 | Method `Tst` vs parent `Tst|Dem` | **closed**: method aligned (`Tst | Dem`); MISRA `Inp|Anl` aspect advised (A-9) | SSS.md:123; SRS.md:278 | frick-fsw | closed | 2026-10-11 (this review) |
| NC-64-4 | Minor | development-plan / component-gap-analysis §5 | M12 fallback still framed as the in-house engine; M12/M16 relationship undefined | open — see NC-72-2 | development-plan.md:55,:217-218; component-gap-analysis.md:104-106 | frick-fsw | **open** | 2026-10-11 (this review) |
| NC-64-5 | Minor | component-gap-analysis §6 | Gap linkage omitted M16 / SSS-REQ-027 / SRS-REQ-054 | **closed**: §6 now names M16 and the ids | component-gap-analysis.md:10,115-121 | frick-fsw | closed | 2026-10-11 (this review) |
| NC-64-6 | Minor | docs/requirements/traceability.md | Human matrix not regenerated | **partially closed**: regenerated, but History/numbering and planned-note defective — see NC-72-4/NC-72-5 | traceability.md:7,20-22,54,114,148-151,204-206 | frick-fsw | **partially open** | 2026-10-11 (this review) |
| NC-64-7 | Minor | roadmap / M16 order | M16 "before M13" conflicts with the PUS S18 surface owned by M15 | open — see NC-72-3 | roadmap.md:181-201; SRS.md:260-265; pus-c-from-scratch-assessment.md:53 | frick-fsw | **open** | 2026-10-11 (this review) |
| NC-64-8 | Comment | SSS-REQ-027 | Milestone tag in SSS; stray blank line | advisory — remains (A-10) | SSS.md:117,:124 | frick-fsw | open | n/a |
| NC-64-9 | Comment | SRS.md structure | SRS-REQ-054 grouped under the M9 subsection | advisory — remains (A-10) | SRS.md:202,:272 | frick-fsw | open | n/a |
| NC-64-10 | Comment | wording / roadmap | (a) "ECSS-E-ST-70 series" loose; (b) M16 section order | (b) **closed** (roadmap.md:192 after M15); (a) remains (A-10) | roadmap.md:192; component-gap-analysis.md:106 | frick-fsw | (b) closed / (a) open | n/a |
| NC-72-1 | Minor | SSS-REQ-027 / SRS-REQ-054 | No quantified bounds / acceptance criterion / TBC for the M16 engine | pending | — | frick-fsw | open | pending |
| NC-72-2 | Minor | development-plan OBJ-20 / §4 M12; component-gap-analysis §5 | M12 in-house-engine "fallback" framing not reconciled; M12/M16 relationship undefined | pending | — | frick-fsw | open | pending |
| NC-72-3 | Minor | M16 execution order / PUS S18 surface | S18 surface ownership vs M16-before-M13/M15 order not reconciled | pending | — | frick-fsw | open | pending |
| NC-72-4 | Minor | traceability.md History | Header `0.23` vs latest History `0.22`; duplicate `0.21`; no M16 renumbering | pending | — | frick-fsw | open | pending |
| NC-72-5 | Minor | traceability.md planned note | "Four" vs six ids listed; M14/M15 planned requirements omitted | pending | — | frick-fsw | open | pending |
| NC-72-6 | Minor | development-plan.md History | Header `0.15` vs latest History `0.14`; duplicate `0.13` | pending | — | frick-fsw | open | pending |
| NC-72-7 | Minor | handover.md | Header `1.1` vs History `1.2`; stale "as of commit 967f563" | pending | — | frick-fsw | open | pending |
| NC-72-8 | Minor | SDD §6 (`SRS-REQ-039/040`) | Misallocated to "Link layer"; `SRS-REQ-040` duplicated; inconsistent with the manifest | pending | — | frick-fsw | open | pending |
| NC-72-9 | Minor | ecss-compliance-matrix.md (PA-67-01 #4) | Design traceability still overstated; GAP-A9 not re-scoped to B | pending | — | frick-fsw | open | pending |
| NC-72-10 | Minor | SDD §6 / GAP-A9 (PA-67-01 #2/#3) | No detailed design / component-unit statement; no Q-ST-80C 6.2.6.12a verification record | pending | — | frick-fsw | open | pending |
| NC-72-11 | Minor | README.md / metrics.py (PA-67-01 truthfulness) | 100% design-traceability claim + removal of the open-NC marker while PA-67-01 is open; metric is a string-presence proxy | pending | — | frick-fsw | open | pending |

**Systems-side status of sibling NCs.** FRICK-REVIEW-0064 is **not closed**
(NC-64-2, NC-64-4, NC-64-7 open; NC-64-6 partial). NC **PA-67-01**
(FRICK-REVIEW-0067) is **not closed** from the systems side: the §6 matrix is a
credible start but corrective actions #3/#4 and full design traceability remain
(NC-72-8..11). This report is a delta review of the R3/N14 scope only; the R4
(N17) and PA (N20) closure verdicts remain with those reviewers, and per norm N12
the combined Minor set must be closed and re-reviewed before the M16 loop and
Sprint 0 are closed.

---

This review report was created read-only (no other repository file modified),
following `docs/reviews/review-template.md`. The `docs/reviews/README.md` index
row for `FRICK-REVIEW-0072` is to be appended at the closure commit (norm N11).