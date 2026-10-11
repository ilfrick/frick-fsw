# Review Report — Product Assurance review of the reference NOR/QSPI MTD backend increment (norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0094                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `a3cdd68` ("Sprint 1.2: reference NOR/QSPI MTD backend (SRS-REQ-056)"), branch `main`, pushed to `origin` and `gitea` |
| Items under review                | `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`; `tests/unit/test_mtd.c`; `tests/CMakeLists.txt`; `CMakeLists.txt`; `src/apps/app_renode_test.c` (`renode_hal_mtd_check`); `config/platform/platform_mpfs.h`; requirement SRS-REQ-056 (SRS.md, traceability.yaml/.md, SSS-REQ-020 link); records `docs/vv/verification-results.md` (0.27), `docs/vv/tool-qualification.md` (T3/T17, 0.3.11), `CHANGELOG.md`, `docs/development-plan.md` (M4), `docs/status/loop-audit.md`, `docs/status/handover.md` (1.4), `README.md`; the review evidence set under `docs/reviews/` for this increment |
| Date                              | 2026-10-11                             |
| Scope                             | Process and records for the SRS-REQ-056 increment: whether the applicable process obligations are met (N15 full cycle incl. N18 Renode, N4 trace convergence, N10 tool register, N11 records, N9/N14/N17/N20 review set); honesty of the Renode/coverage claims (the `hal_mtd` check is a **read-presence** check, not write/erase; on-target NOR behaviour = GAP-A5); any record that overstates or misstates; whether the N20 gate is satisfied for this increment. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean before and after). Gates were executed read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the norm-N20 Product Assurance review of the milestone M4/M11 increment
that adds the reference NOR/QSPI memory-technology-device backend behind the HAL
contract `FF_Mtd_t` (SRS-REQ-056) and a `hal_mtd` clause to the Renode functional
test, as delivered and **pushed** at `a3cdd68`.

**Included:** the process obligations this increment triggers (N15 full cycle,
N18 Renode, N4 trace, N10 tool register, N11 records, and the N9/N14/N17/N20
review set); the factual accuracy of its recorded evidence (test counts, cycle
log, tool register, status/handover statements, README panel); and whether any
record overstates the evidence — in particular the traceability `lean_note` for
SRS-REQ-056 versus the actual (read-only) Renode check.

**Excluded:** the internal correctness of the driver and test (owned by the R1
review and by R2); the Lean proofs; documentation adjudication owned by R4; and
on-target/silicon NOR behaviour, which is GAP-A5 by the increment's own scope.
This review does not re-perform the R1/R2/R3/R4 reviews.

**Method.** Read the commit diff and every in-scope artifact; independently
executed the project gates; measured the actual unit-test assertion counts and
compared them against the recorded ones; inspected the recorded Renode run
timestamp/content against the commit; and inspected the repository/remote state
(committed vs untracked, local vs remote) to determine whether the mandated
review evidence exists at the reviewed revision.

**Independence note (log row).** Per the task instruction this review modifies
nothing but this report file. The `docs/reviews/README.md` review-log row and any
`docs/vv/verification-results.md` cycle-log rows for FRICK-REVIEW-0094 remain the
integrator's obligation (norms N11/N17/N20). The sibling R1/R2/R4 reports for this
increment **do not yet exist** in the working tree or the repository; this review
is a snapshot of the repository state and the sibling review set may change after
it. Review ids `0091`-`0093` are left available for the sibling R1/R2/R4 reviews
of this increment.

## 2. References

- Project norms: `docs/rules.md` (N1-N20), especially N4, N8, N9, N10, N11, N12,
  N15, N16, N17, N18, N19, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056).
- HAL contract: `src/hal/hal_mem.h` (`FF_Mtd_t`, SRS-REQ-032).
- Records: `docs/vv/verification-results.md` (0.27), `docs/vv/tool-qualification.md`,
  `CHANGELOG.md`, `docs/development-plan.md`, `docs/status/loop-audit.md`,
  `docs/status/handover.md`, `README.md`, `docs/planning/sprint-plan.md`.
- Renode platform: `tools/docker/renode/frick-renode.resc` (loads
  `@platforms/cpus/polarfire-soc.repl` from the pinned `antmicro/renode` image);
  `tools/scripts/renode_vv.sh` (captures the console to `build/renode/console.log`).
- Precedent for the sequencing finding: `docs/reviews/2026-10-11-nvm-pa-review.md`
  (FRICK-REVIEW-0084, Major F1, same norm question at the prior increment).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — tool
  qualification, product-assurance records, traceability matrices verified at
  each milestone.

## 3. Verification performed (independent)

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), 51 Lean theorems (matches the records) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** (incl. `mtd`) |
| MTD suite check count | `./build/native/tests/test_mtd` | **`checks=4123 failures=0`** — matches the records |
| Format gate | `clang-format --dry-run --Werror` on the three new files | **exit 0** — clean |
| Renode evidence (N18) | `build/renode/console.log` (real run) | `hal_mtd=PASS`, `RENODE_TEST: PASS` (log mtime 2026-10-11 18:58:57, consistent with commit 18:59:30) |
| Renode oracle nature | `app_renode_test.c:254-264`; `ff_mtd_mpfs.h:47-51` | The check calls `drv.mtd.read(0, buf, 4)` only — a **read-presence** check; driver source, commit message, CHANGELOG, README, verification-results all say so (Finding 2 concerns the one record that does not) |
| Repository/remote state | `git branch -avv`, `git show --stat a3cdd68`, `git status` | HEAD `a3cdd68` **=** `origin/main` **=** `gitea/main` (pushed); commit contains **no** `docs/reviews/*` file; `git status` clean → **no** untracked sibling (R1/R2/R4) reports exist; `ls docs/reviews/*mtd* docs/reviews/*nor*` → none (Finding 1) |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F1 | **Major** | N15 / N9 / N17 / N20 / N11 / N7 — review set for this increment | The increment was **committed and pushed to both remotes without any committed review evidence and without the complete review set**. The commit message itself defers them ("Reviews (R1/R2/R4 + PA) to follow"). At the reviewed revision **no** R1 (FSW), R2 (V&V), R4 (documentation, N17) or PA review for this increment exists anywhere — neither committed nor untracked — and there is **no `docs/reviews/README.md` log row**. Norm N15 requires the full cycle (including the N9/N10/N14 reviews and N12 close-out) to complete before commit/push; N17 and N20 require the reviews **before commit/push**; N20 requires the PA review to be *additional to* the R1/R2/R3/R4 reviews. The N20 gate is therefore **not satisfied** for this increment. This repeats the pattern found at the prior eNVM increment (FRICK-REVIEW-0084 Major F1). | `git show --stat a3cdd68` lists no `docs/reviews/*`; `git branch -avv` → HEAD = origin/main = gitea/main (`a3cdd68`); `git status` clean (no untracked reports); `ls docs/reviews/*mtd* docs/reviews/*nor*` → none; `docs/reviews/README.md` last row is the NVM closure (0090); commit message line "Reviews (R1/R2/R4 + PA) to follow" | Run and commit the complete review set for this increment (R1 + R2 + R4, plus R3/N14 if the SSS-REQ-020 child set change triggers it — it did change in the manifest), add the `docs/reviews/README.md` rows, record this PA review, close every Minor finding (N12), and re-review the deltas. As of the pushed revision this is a non-derogable-norm breach; it **downgrades to Minor once the reports are committed and logged**. |
| F2 | Minor | `docs/requirements/traceability.yaml:532` (SRS-REQ-056 `lean_note`); N11/N16 | **The SRS-REQ-056 `lean_note` overstates the Renode evidence.** It claims a "**write/read check on the emulated SoC (hal_mtd)**", but the check is a **read-only, bounded XIP read-presence** check (`drv.mtd.read(0, buf, 4)`), because the QSPI XIP window is read-only in the platform model. Every other record (driver header `ff_mtd_mpfs.h:47-51`, the app comment, the commit message, `CHANGELOG.md`, `README.md`, `verification-results.md:39`) correctly scopes it as a read-presence check; only the manifest note is inaccurate. This is the same class of wording defect that was expressly corrected for SRS-REQ-055 in `a91e51f`. | `docs/requirements/traceability.yaml:532`; `src/apps/app_renode_test.c:254-264`; `src/drivers/mtd/mpfs/ff_mtd_mpfs.h:47-51`; `docs/vv/verification-results.md:39`; `a91e51f` ("NVM: correct SRS-REQ-055 lean_note") | Change the note to "**read-presence check on the emulated SoC (hal_mtd)**" (write/erase/retention on target remains GAP-A5). |
| F3 | Minor | `docs/design/software-design.md:113` §6; SRS-REQ-056 (N8) | **The SDD requirements→design matrix was not updated for the new requirement/design element.** The §6 "Reference drivers" row reads `src/drivers/{eth/gem,gpio/mpfs,serial/mpfs,mem,nvm/mpfs,net}` and lists SRS-REQ-036/038/040/047/048/049/055 only — neither `mtd/mpfs` nor SRS-REQ-056 is present. The regression is visible as the README "Design traceability 56/57 (98.2%)" indicator but is not tracked as a gap. Same class as the eNVM F3 finding (FRICK-REVIEW-0084). | `docs/design/software-design.md:113`; `README.md:54`; `git show --stat a3cdd68` (no SDD file) | Add `mtd/mpfs` + SRS-REQ-056 to the SDD §6 reference-drivers row (or record the deferral explicitly). Norm N8: docs affected by the change are updated in the same commit. |
| F4 | Minor | `docs/status/handover.md` §5/line 126 (N8/N19) | **Internal inconsistency in the resume point.** Line 126 still reads "NVM done; next classes: **NOR MTD**, CAN, ..." although NOR/QSPI MTD is the deliverable of this increment. The edit to the §5 bullet list also reordered the first bullet to "CAN ..., then I2C/SPI/BLOCK/PCIe/SpaceWire; [NVM + NOR/QSPI MTD done]" while the third bullet still enumerates "CAN ..., SPI, I2C, BLOCK, PCIe, SpaceWire", so **CAN now appears twice** and the ordering no longer matches the section's stated "ease of emulated validation" rationale. | `docs/status/handover.md:126,130,132-134`; `docs/development-plan.md` (M4 correctly updated); `git show a3cdd68 -- docs/status/handover.md` | Remove the stale "NOR MTD" from the line‑126 "next classes" list and de-duplicate the §5 bullet ordering so the handover is self-consistent. |
| F5 | Minor | `docs/planning/sprint-plan.md:42` (S1 exit criterion); N8 | **Sprint-plan evidence name does not match the implemented check.** The S1 exit criterion states "`hal_nvm`/**`hal_nor`=PASS**", but the implemented and recorded Renode marker is **`hal_mtd`=PASS** (`app_renode_test.c:326`; `verification-results.md:39`; T17 row). An auditor following the plan would look for a marker the test never emits. | `docs/planning/sprint-plan.md:42`; `src/apps/app_renode_test.c:326`; `docs/vv/tool-qualification.md:41` | Reconcile the plan to the implemented marker `hal_mtd` (or rename the check if `hal_nor` is the intended public name). |
| C1 | Comment | `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`, `tests/unit/test_mtd.c` | The three new files lack a terminating newline. The project `.clang-format` does not set `InsertNewlineAtEOF`, so the "format clean" claim is **true** (`clang-format --dry-run --Werror` exits 0); this is a cosmetic nit only. | tail bytes of the three files; `.clang-format`; `clang-format --dry-run --Werror` exit 0 | Add a trailing newline for consistency with the rest of the tree. |
| C2 | Comment | Renode evidence strength (N18) | The `hal_mtd` oracle is non-discriminating by design: it returns PASS whenever a byte read at `FF_QSPI0_BASE+0` succeeds, irrespective of value, and it does not exercise write/erase/retention (correctly deferred to GAP-A5). The records **honestly** scope it as a read-presence check (except F2), so this is an evidence-strength observation, not an over-claim. | `app_renode_test.c:254-264`; `board`/`config/platform/platform_mpfs.h:50-52`; `verification-results.md:39` | Keep the read-presence scoping explicit; consider a fixed identification/scratch offset if a discriminating read oracle becomes available. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive observations (no finding).** All product-level obligations of the
increment are met and honestly recorded: the requirement (SRS-REQ-056) and the
SSS-REQ-020 link are present and converged (`trace.py` PASS 98/57/27/14/51); the
unit suite is 18/18 with `test_mtd` at 4123 checks (matches the records); the
Renode functional run is **real** (`RENODE_TEST: PASS`, `hal_mtd=PASS`, console
log timestamp consistent with the commit); the tool register (T3 gains
`test_mtd.c`; T17 gains `hal_mtd`; history 0.3.11), the cycle log (0.27), the
CHANGELOG, the development-plan M4 line, and the loop-audit M4/N3 rows are all
updated in the same commit (N8/N16); clang-format is clean; and the metrics panel
is current. The single disqualifying issue is the **process sequencing** (F1),
not the product.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** — one Major finding (F1: the review set for this increment was not
  produced/committed with the change as N15/N17/N20/N11 require, and the commit
  was pushed to both remotes; the N20 gate is not satisfied).

The product of the increment is sound and its evidence is largely honest, but the
**N20 gate is not satisfied at the reviewed revision** because the increment was
pushed with no committed review evidence and none of the mandated sibling reviews
(R1/R2/R4) exist. F1 is a non-derogable-norm breach at `a3cdd68` and **downgrades
to Minor once the review set is run, committed and logged**. F2-F5 are Minor
record-accuracy/consistency defects to be corrected in the same cycle (N12).

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0094 | 2026-10-11 | **REJECTED (1 Major F1; Minors F2-F5)** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: NOR/QSPI MTD backend increment (SRS-REQ-056) at a3cdd68, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on gates/records; Inference on the N20-gate status; Recommendation for F2-F5).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (2025), ECSS-Q-ST-80C Rev.2 (2025).
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at a3cdd68 (all files listed in the header table); gates re-run 2026-10-11 read-only.
Cross-check and reconciliation: recorded numbers (98/57/27/14/51; 18/18; 4123 checks; hal_mtd=PASS; format clean; metrics current) all independently reproduced; the only mismatch is the traceability lean_note (F2).
Confidence: High on F1 and F2 (direct, configuration-matched evidence); High on F3-F5.
Basis and detection limits: read-only inspection + gate re-runs; the internal driver/test correctness (R1/R2 scope) was not independently re-derived.
Missing/inaccessible evidence (including paywalls): none blocking; ECSS clause texts were not needed beyond the invoked norms.
Assumptions and expiry/recheck date: sibling R1/R2/R4 reports may appear after this snapshot; recheck on the closure commit.
Consequences if wrong: if the sibling reviews are produced and committed with the closure (N12), F1 downgrades to Minor and the verdict becomes APPROVED WITH COMMENTS with F2-F5 tracked.
Required independent human reviewer/authority: the project integrator/owner decides closure; this review is not a certification or approval.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| F1 | Major | N15/N17/N20/N11 | Review set not committed with the change; N20 gate unsatisfied | Run + commit R1/R2/R4 (+ R3 if triggered), add README rows, record this PA review; downgrades to Minor once committed | `docs/reviews/*` committed; `docs/reviews/README.md` rows | frick-fsw | open | pending |
| F2 | Minor | traceability.yaml:532 | `lean_note` says "write/read check"; actual is read-presence | Reword to "read-presence check"; keep GAP-A5 for on-target | updated manifest; `trace.py` PASS | frick-fsw | open | pending |
| F3 | Minor | SDD §6 / SRS-REQ-056 | SDD reference-drivers row not updated | Add `mtd/mpfs` + SRS-REQ-056 | `software-design.md:113`; README design trace back to 57/57 | frick-fsw | open | pending |
| F4 | Minor | handover.md §5/126 | Stale "next classes: NOR MTD"; duplicated CAN bullet | Correct the resume point | updated `handover.md` | frick-fsw | open | pending |
| F5 | Minor | sprint-plan.md:42 | Exit criterion names `hal_nor=PASS` vs actual `hal_mtd=PASS` | Reconcile the marker name | updated sprint plan | frick-fsw | open | pending |

---

Committed and pushed with the reviewed change (norm N11).