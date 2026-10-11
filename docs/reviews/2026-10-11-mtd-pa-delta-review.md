# Review Report — Product Assurance delta re-review (N12/N20) of the NOR/QSPI MTD backend increment

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0095                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `cb2576d` ("MTD increment: remove vacuous Renode check, commit reviews, correct QSPI claims") on top of the reviewed revision `a3cdd68`; branch `main`, pushed to `origin` and `gitea` |
| Items under review                | Delta (N12) of the findings F1-F5 of FRICK-REVIEW-0094: `docs/reviews/*mtd*` (0091-0094) + log/V&V rows; `src/apps/app_renode_test.c`; `docs/requirements/traceability.yaml` + `.md`; `docs/design/software-design.md` §6; `docs/status/handover.md`; `docs/planning/sprint-plan.md`; `docs/status/loop-audit.md`; `docs/vv/tool-qualification.md`; `docs/vv/verification-results.md`; `CHANGELOG.md`; `README.md`; the on-SoC QSPI tracking |
| Date                              | 2026-10-11                             |
| Scope                             | Whether the fixes at `cb2576d` close the five findings of FRICK-REVIEW-0094 (incl. Major F1), whether the vacuous Renode claim is fully retired across **all** records, whether the on-SoC QSPI gap is tracked as an M11 task, whether the N20 gate is satisfied at this revision, and whether any residual/derivative defect remains. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified. Gates executed read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the **norm-N12 delta re-review** of FRICK-REVIEW-0094 (PA, N20), which
had **REJECTED** the NOR/QSPI MTD backend increment (SRS-REQ-056) at `a3cdd68`
with one Major finding (F1: the mandated review set was neither produced nor
committed with the change) and four Minor findings (F2-F5: record accuracy and
consistency). The integrator applied fixes at **`cb2576d`** and requested this
delta re-review.

**Included:** closure status of each of F1-F5; whether the vacuous `hal_mtd`
Renode check has been fully retired from code, manifest, and **every** record;
whether SDD §6 now allocates SRS-REQ-056; whether the handover and sprint plan
are corrected; whether the on-SoC QSPI NOR gap is tracked as an M11 task; and
whether the N20 gate is satisfied at `cb2576d`.

**Excluded:** re-derivation of the internal driver/test correctness (R1/R2
scope) and the sibling review verdicts (R1/R2/R4) themselves. The product-level
gates are re-run read-only to confirm the fixing commit is green.

**Method.** Read `git show cb2576d` and every touched artifact; verified the
commit contents against each finding; grepped the working tree for residual
claim/flag references; re-ran `trace.py`, `metrics.py --check` and `ctest`;
inspected the Renode console log; and checked the repository/remote state.

## 2. References

- Predecessor: `docs/reviews/2026-10-11-mtd-pa-review.md` (FRICK-REVIEW-0094, REJECTED).
- Sibling reviews committed with the fix: `docs/reviews/2026-10-11-mtd-r1-review.md`
  (FRICK-REVIEW-0091), `...-r2-review.md` (FRICK-REVIEW-0092), `...-r4-review.md`
  (FRICK-REVIEW-0093).
- Project norms: `docs/rules.md` (N1-N20), especially N4, N8, N11, N12, N15, N16, N18, N20.
- Traceability manifest: `docs/requirements/traceability.yaml`; human mirror
  `docs/requirements/traceability.md`.
- Records: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`,
  `docs/status/handover.md`, `docs/status/loop-audit.md`, `CHANGELOG.md`, `README.md`,
  `docs/planning/sprint-plan.md`.
- Renode platform: `tools/docker/renode/frick-renode.resc` (loads the SoC-only
  `@platforms/cpus/polarfire-soc.repl`); board repls map `SPI.MPFS_QSPI @0x21000000`
  + `SPI.Micron_MT25Q` (not loaded by this test).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

### 3.1 Product gates re-run at `cb2576d`

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), 51 Lean theorems |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** (incl. `test_mtd`) |
| Renode evidence (N18) | `build/renode/console.log` (mtime 2026-10-11 19:15) | `RENODE_TEST: PASS`; clauses `hal_gpio/serial/mem/eth/net/nvm=PASS`; **no `hal_mtd`** (clause removed) |
| Commit/remote state (N7) | `git branch -avv`, `git show --stat cb2576d` | HEAD `cb2576d` = `origin/main` = `gitea/main` (pushed); clean working tree |

### 3.2 Delta closure matrix (F1-F5)

| Finding (0094) | Severity | Verification at `cb2576d` | Status |
| --- | --- | --- | --- |
| **F1** — review set not committed with the change | Major | `git show --stat cb2576d` adds `docs/reviews/2026-10-11-mtd-{r1,r2,r4,pa}-review.md` (0091-0094) and the `docs/reviews/README.md` + `docs/vv/verification-results.md` log rows **in the same commit** as the change | **CLOSED** |
| **F2** — manifest `lean_note` claims a write/read Renode check | Minor | `traceability.yaml:530-534`: `tests: ["tests/unit/test_mtd.c"]` ( `ci:renode` removed); note reworded to "register logic host-tested (the on-SoC QSPI check needs the board repl - M11 follow-up)"; the vacuous `renode_hal_mtd_check`/`mpfs_mtd_read8/write8` removed from `app_renode_test.c` | **CLOSED (yaml/code)** — **residual** in `traceability.md` (DF1) |
| **F3** — SDD §6 omits `mtd/mpfs` / SRS-REQ-056 | Minor | `docs/design/software-design.md`: §6 row now `{...,nvm/mpfs,mtd/mpfs,net}` with SRS-REQ-056; rev bumped 0.3→0.4; README design traceability 57/57 (100%) | **CLOSED** |
| **F4** — handover stale "next classes: NOR MTD"; duplicated CAN bullet | Minor | `handover.md:130` bracket corrected, **but** line 126 still reads "NVM done; next classes: NOR MTD, CAN, ..." and CAN still appears twice (§5 bullets at :130 and :132); §4 resume point remains stale (old commit ref, 5/13, "(14/14)") | **NOT CLOSED** (partially fixed) |
| **F5** — sprint-plan exit criterion names `hal_nor=PASS` | Minor | `sprint-plan.md:42` now `hal_nvm=PASS; M10 done; on-SoC NOR check pending board repl (MPFS_QSPI+Micron_MT25Q)` | **CLOSED** |
| On-SoC QSPI gap tracked as M11 task | — | `loop-audit.md` dispositions: "On-SoC QSPI NOR write/erase check (MTD) - needs the board repl `MPFS_QSPI`+`Micron_MT25Q` + flash command path — tracked — frick-fsw — **M11**"; `sprint-plan.md` S1 also records it | **PRESENT** |

### 3.3 Residual claim/flag scan

| Probe | Result |
| ----- | ------ |
| `grep -rn "hal_mtd\|hal_nor\|renode_hal_mtd\|mpfs_mtd_read8\|FF_QSPI0_BASE"` (code+records, excl. `docs/reviews/`) | `docs/vv/verification-results.md:410` (History 0.27) still claims `hal_mtd=PASS` in Renode → **DF2**; `config/platform/platform_mpfs.h:53` `FF_QSPI0_BASE` now unused → **DF3**; `traceability.md:117` still lists `ci:renode` for SRS-REQ-056 → **DF1** |
| `.yaml` vs `.md` mirror | `.yaml` corrected; `.md` **not** (still `ci:renode`) → **DF1** (N4/N8 divergence; CI docs gate only checks file existence, not content sync, so it is not caught) |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F4 | **Minor** | `docs/status/handover.md:126,130,132`; N8/N19 | **Carried from 0094, not closed.** The §5 blockquote at line 126 still lists "**NOR MTD**" among "next classes" although it is the deliverable of this increment; the §5 bullet list still enumerates CAN twice (line 130 and the :132 bullet). §4 "Current state" remains stale (references commit `967f563`/FRICK-REVIEW-0063, "5/13 peripheral-bound", "(14/14)"), contradicting §5 and History 1.4. | `handover.md:126,130,132`; `sed -n '60,115p' handover.md`; commit diff touched only line 130 | Remove "NOR MTD" from line 126; de-duplicate the CAN bullets; refresh §4 (6/13 + DRIVER done / on-SoC check pending, "18/18", current commit). |
| DF1 | **Minor** | `docs/requirements/traceability.md:117`; N4/N8 | **The human-readable traceability mirror was not updated with the manifest fix.** The `.yaml` for SRS-REQ-056 is corrected to `tests: [test_mtd.c]`, but `traceability.md` still lists "`test_mtd.c, ci:renode`" — re-asserting the removed Renode check. This is the same over-claim class as 0094-F2, surviving in a second record. | `traceability.md:117`; `traceability.yaml:530-534`; `.github/workflows/ci.yml:164` (docs gate checks existence, not yaml/md sync) | Update `traceability.md:117` to `test_mtd.c`; regenerate/reconcile the mirror. |
| DF2 | **Minor** | `docs/vv/verification-results.md:410` (History 0.27); N11/N16 | **A second record still overstates the retired Renode evidence.** History row 0.27 states "…`test_mtd` (ctest 18/18), **`hal_mtd=PASS` in Renode**", but the `hal_mtd` clause no longer exists and the Renode run no longer emits it. The cycle row (`:39`) and `CHANGELOG.md` were corrected; the History row was not. | `verification-results.md:410`; `build/renode/console.log` (no `hal_mtd`); commit diff | Correct History 0.27 to host-only evidence; add a History row for the `cb2576d` correction (N16). |
| DF3 | Comment | `config/platform/platform_mpfs.h:53` | `FF_QSPI0_BASE` (0x21000000) is now **unused** after removing the `hal_mtd` clause (grep finds only the definition). Cosmetic dead define; not a defect. | `grep -rn FF_QSPI0_BASE` → definition only | Remove the define or annotate it for the future M11 QSPI path. |
| DF4 | Comment | Sibling NCs (N12) | Several findings of the sibling reviews committed in this same commit remain **open and uncorrected** — R1-MTD-04 (no `off > size` / `size > FF_MTD_MAX_SIZE` cases; `test_mtd.c` untouched), R2 F1 (erase `FF_BAD_SIZE` path untested), R4-93-01 (`tool-qualification.md` header rev `0.3.10` vs History `0.3.11`), R4-93-02 (`loop-audit.md` content changed, rev still `0.6`), R4-93-03 (`development-plan.md` rev/M11 row stale), R4-93-05 (`verification-results.md` Remaining still lists QSPI), R4-93-06 (`README.md` scope table still plans QSPI/NOR; focus sentence "NVM done; NOR/CAN"). Per N12 these must be closed before the increment loop is fully closed. | `tests/unit/test_mtd.c` (no `size+1`/`MAX_SIZE` cases); `tool-qualification.md:7`; `loop-audit.md:7`; `development-plan.md:302`; `verification-results.md:277`; `README.md:137,408` | Correct and delta-verify each sibling NC (N12). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive observations (no finding).** The disqualifying Major is resolved:
the mandated review set (R1/R2/R4 + PA, ids 0091-0094) and their log/V&V rows
are now **committed with the change** (N11/N15 fixed). The Major's root cause —
the vacuous `hal_mtd` read of an unmapped `0x21000000` in the SoC-only platform
— is **removed**, not merely reworded, so the increment no longer over-claims
on-SoC NOR evidence; the requirement is honestly scoped as host-verified with
the on-SoC QSPI write/erase check **tracked as an M11 task** (board repl
`MPFS_QSPI` + `Micron_MT25Q` + flash command path). F2 (manifest), F3 (SDD §6,
design 57/57) and F5 (sprint plan) are correctly fixed. All gates are green at
`cb2576d`: trace 98/57/27/14/51, metrics current, ctest 18/18, Renode PASS.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**FRICK-REVIEW-0094 status: the Major F1 is CLOSED** (the review set is now
committed and logged with the change). **F2, F3 and F5 are CLOSED.** **F4 is
NOT CLOSED** (handover only partially corrected — line 126 and the duplicated
CAN bullet persist, §4 stale). Two derivative record defects survive the same
over-claim class as F2 (DF1: `traceability.md`; DF2: `verification-results.md`
History 0.27), and the sibling R1/R2/R4 NCs committed with this change remain
open (DF4).

No **Major** remains, so the verdict is **APPROVED WITH COMMENTS**: the product
is sound, the gates are green, and the disqualifying process breach is fixed.
**The N20 gate is satisfied at this revision for the review-set requirement**
(the mandated independent set exists, is committed with the change, and the
Major is closed); it is **not yet fully N12-closed**, because F4 and the
residual Minors (DF1, DF2, DF4) must be corrected and delta-verified before the
increment loop is truly closed.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0095 | 2026-10-11 | **APPROVED WITH COMMENTS** (Major F1 closed; F4 + DF1/DF2/DF4 Minors tracked) |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: NOR/QSPI MTD backend increment (SRS-REQ-056), delta at cb2576d over a3cdd68, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on gates/closure; Inference on N20/N12 status; Recommendation for F4/DF1-DF4).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (2025), ECSS-Q-ST-80C Rev.2 (2025).
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at cb2576d (files in the header table); git show cb2576d; gates re-run 2026-10-11 read-only; build/renode/console.log (19:15).
Cross-check and reconciliation: F1/F2/F3/F5 closure confirmed directly; F4 confirmed open by reading handover.md:126/130/132; the yaml/md divergence (DF1) and the History 0.27 over-claim (DF2) found by residual grep; recorded numbers (98/57/27/14/51; 18/18; Renode PASS) independently reproduced.
Confidence: High on F1-F5 closure status and on DF1/DF2 (direct, configuration-matched evidence); High on DF4 (finding tables read in the committed sibling reports).
Basis and detection limits: read-only inspection + gate re-runs; the internal driver/test correctness (R1/R2 scope) was not independently re-derived; the Renode run was inspected via its console log, not fully re-executed here.
Missing/inaccessible evidence (including paywalls): none blocking; ECSS clause texts were not needed beyond the invoked norms.
Assumptions and expiry/recheck date: the sibling NC corrections may land in a subsequent commit; recheck at the next delta.
Consequences if wrong: if F4 and DF1/DF2/DF4 are corrected and delta-verified, the increment loop closes and the verdict becomes APPROVED (Comments only).
Required independent human reviewer/authority: the project integrator/owner decides closure; this review is not a certification or approval.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0094-F1 | Major | N15/N17/N20/N11 | Review set not committed with the change; N20 gate unsatisfied | Closed: R1/R2/R4/PA (0091-0094) + README/V&V rows committed in `cb2576d` | `git show --stat cb2576d`; `docs/reviews/*mtd*`; `docs/reviews/README.md` | frick-fsw | **closed** | 0095 (this) |
| 0094-F2 | Minor | traceability.yaml:532 | `lean_note` said "write/read check" | Closed in `.yaml`/code | `traceability.yaml:530-534`; `app_renode_test.c` | frick-fsw | **closed** (yaml) | 0095 |
| 0094-F3 | Minor | SDD §6 / SRS-REQ-056 | SDD reference-drivers row not updated | Closed: `mtd/mpfs` + SRS-REQ-056 added; rev 0.4; design 57/57 | `software-design.md` §6 + History; `README.md:54` | frick-fsw | **closed** | 0095 |
| 0094-F4 | Minor | handover.md:126/130/132 | Stale "next classes: NOR MTD"; duplicated CAN bullet; stale §4 | Only line 130 bracket fixed; line 126 + CAN duplicate + §4 remain | `handover.md:126,130,132`; `sed -n '60,115p'` | frick-fsw | **open** | 0095 |
| 0094-F5 | Minor | sprint-plan.md:42 | Exit criterion named `hal_nor=PASS` | Closed: now `hal_nvm=PASS` + M11 board-repl note | `sprint-plan.md:42` | frick-fsw | **closed** | 0095 |
| DF1 | Minor | traceability.md:117 | Mirror still lists `ci:renode` for SRS-REQ-056 | Correct the mirror | updated `traceability.md`; yaml/md reconciled | frick-fsw | **open** | pending |
| DF2 | Minor | verification-results.md:410 | History 0.27 still claims `hal_mtd=PASS` in Renode | Correct History row; add N16 row for `cb2576d` | updated `verification-results.md` | frick-fsw | **open** | pending |
| DF3 | Comment | platform_mpfs.h:53 | `FF_QSPI0_BASE` now unused | Remove or annotate for M11 | updated platform header | frick-fsw | **open** | pending |
| DF4 | Comment | R1/R2/R4 NCs (N12) | Sibling NCs committed open: R1-MTD-04, R2-F1, R4-93-01/02/03/05/06 | Correct + delta-verify each | sibling reports' follow-up tables | frick-fsw | **open** | pending |

---

Committed with the reviewed change (norm N11).