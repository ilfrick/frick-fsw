# Review Report — PA closure review (N12/N20) of the FRICK-REVIEW-0103 residual D1/D2 (Renode board-repl change, BeagleV-Fire)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0105                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `5ff7198` ("Renode board repl: close PA delta D1/D2 (wording + R2 T17 re-qualification)"), branch `main`, **= `origin/main` = `gitea/main`** (already pushed) |
| Items under review                | Closure of the two open Minor NCs of FRICK-REVIEW-0103 — **D1** (stale "pending board repl" wording) and **D2** (missing R2 T17/T15 re-qualification review) — at HEAD; the touched live records `README.md`, `docs/requirements/traceability.md`, `docs/requirements/traceability.yaml`, `docs/planning/sprint-plan.md`, `docs/status/handover.md`, `CHANGELOG.md`; the register `docs/vv/tool-qualification.md` (T17, History 0.3.12); the new independent review `docs/reviews/2026-10-11-renode-resc-r2-requal-review.md` (FRICK-REVIEW-0104); the review reports committed with this change (`2026-10-11-renode-board-pa-delta-review.md` = FRICK-REVIEW-0103, plus 0104) and their log rows |
| Date                              | 2026-10-11                             |
| Scope                             | Whether `5ff7198` closes D1 (all named live records reworded to "pending the QSPI controller command driver"; historical cycle rows preserved) and D2 (an independent R2 re-qualification review of the changed T17 scripts exists and is committed; the T17 register entry is re-qualified and its History names **both** changed `.resc` files), so that the board-repl change loop can **close** under N12 and the **N20 gate remains satisfied**. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or previously reviewing the items; fresh independent session with no author context. Read-only for the repository: only this report file was created; no tracked file was modified (`git status` clean before and after). Gates were re-run read-only (outputs under the git-ignored `build/` and ephemeral containers). |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the **norm-N12 delta re-review / norm-N20 closure review** of the two
remaining open Minor non-conformities of the prior PA delta review
**FRICK-REVIEW-0103**: **D1** (stale "pending board repl" wording in live
records) and **D2** (review set missing the R2 T17 re-qualification that the
tool-qualification register requires). The corrective commit is **`5ff7198`**,
which also commits the FRICK-REVIEW-0103 and FRICK-REVIEW-0104 reports and adds
their log/V&V rows.

**Included:**

- Whether every live record D1 named (`README.md`, `traceability.md`,
  `traceability.yaml` `lean_note`, `sprint-plan.md`, `handover.md`,
  `CHANGELOG.md`) now states the pending item is the **QSPI controller command
  driver** — not the board repl — and whether the deliberately-kept historical
  cycle rows are correctly left unchanged.
- Whether an **independent R2** re-qualification review of the changed T17
  scripts exists and is committed, and whether the T17 register is re-qualified
  with a History row naming **both** changed `.resc` files
  (`frick-renode.resc` + `renode_env.resc`).
- The product gates (trace, metrics, ctest, `make renode-env`), plus the
  repository/remote state and the review-set logging (N11/N17/N20).
- That no QSPI over-claim and no prior NC reopening was (re)introduced.

**Excluded:** internal design/code correctness of the drivers and of the Renode
test application (R1/R2 scope, already reviewed); the Lean proofs (N1; the change
touches no `verification/lean/` source); documentation/wording adjudication
owned by R4 (N17) beyond the D1 deltas; and on-target/silicon NOR behaviour and
the QSPI write/erase check, which remain **GAP-A5** (M11 controller-driver
follow-up). The advisory Comment D3 of FRICK-REVIEW-0103 is noted, not in scope.

**Method.** Read `git show 5ff7198` (full diff and `--stat`) and the parent
`b6b2d96`; read the full text of the two committed reports (FRICK-REVIEW-0103,
FRICK-REVIEW-0104); inspected every touched record and the register at HEAD;
grepped the repository for the retired wording; inspected both `.resc` scripts
and the pinned `beaglev-fire.repl` claim; re-ran `trace.py`, `metrics.py
--check`, `ctest` and `make renode-env`; and checked the repository/remote state.

## 2. References

- Project norms: `docs/rules.md` (N1–N20), especially N4, N8, N9, N10, N11, N12,
  N15, N16, N17, N18, N19, N20.
- Predecessors: **FRICK-REVIEW-0103** (`docs/reviews/2026-10-11-renode-board-pa-delta-review.md`,
  raised D1/D2), **FRICK-REVIEW-0104** (`docs/reviews/2026-10-11-renode-resc-r2-requal-review.md`,
  the R2 re-qualification), FRICK-REVIEW-0101/0102 (R4/PA at `8764dd4`); closure
  precedent FRICK-REVIEW-0100 (`docs/reviews/2026-10-11-mtd-pa-99-closure.md`).
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056
  `lean_note`); human mirror `docs/requirements/traceability.md`.
- Register/records: `docs/vv/tool-qualification.md` (T15/T17 row, History
  0.3.12), `docs/vv/verification-results.md`, `docs/status/handover.md`,
  `docs/planning/sprint-plan.md`, `CHANGELOG.md`, `docs/reviews/README.md`.
- Platform source: `tools/docker/renode/frick-renode.resc`,
  `tools/docker/renode/renode_env.resc`; pinned image
  `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`.
- Standards: ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

### 3.1 Product gates re-run at `5ff7198`

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — `98 requirements, 57 srs, 27 sss, 14 gaps, 51 lean theorems checked` (matches the task and the records) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** (incl. `nvm`, `mtd`) |
| Renode env self-test (N18/T17) | `make renode-env` (pinned image) | **PASS** — `[renode-env] PASS: Renode runs and the PolarFire SoC board platform loads` |
| Commit/remote state (N7) | `git branch -avv`, `git status --porcelain` | HEAD `5ff7198` **=** `origin/main` **=** `gitea/main`; tracked tree clean |
| Corrective diff | `git show --stat 5ff7198` | 11 files: 6 record fixes + 2 review reports (0103/0104) + `docs/reviews/README.md` + `docs/vv/tool-qualification.md` + `docs/vv/verification-results.md` |

### 3.2 Closure of FRICK-REVIEW-0103 D1 (stale "pending board repl" wording)

Grep of the repository at HEAD for the retired wording (the literal pattern
`pending board repl` / `needs the board repl`) returns hits **only** in
historical review reports (`docs/reviews/*`) and in the two deliberately kept
historical V&V rows (`docs/vv/verification-results.md:39` cycle row, `:420`
History 0.27). Every live record D1 named now reads correctly:

| Live record (D1 target) | At HEAD | Status |
| -- | -- | -- |
| `README.md:186` | "Renode check pending the QSPI controller command driver" | **CLOSED** |
| `docs/requirements/traceability.md:117` | "on-SoC QSPI check pending the QSPI controller command driver" | **CLOSED** |
| `docs/requirements/traceability.yaml:532` (`lean_note`) | "the on-SoC QSPI check needs the QSPI controller command driver; the board repl is now loaded - M11 follow-up" | **CLOSED** |
| `docs/planning/sprint-plan.md:42` | "on-SoC NOR check pending the QSPI controller command driver" | **CLOSED** |
| `docs/status/handover.md:130` | "board repl loaded; on-SoC QSPI check pending the controller command driver" | **CLOSED** |
| `CHANGELOG.md:20-21` | "needs the QSPI controller command driver (the board repl with `MPFS_QSPI` + `Micron_MT25Q` is now loaded) (M11 follow-up)" | **CLOSED** |
| `docs/vv/verification-results.md:39/:420` (historical) | "pending ... the board repl" | **PRESERVED as historical** (per the D1 disposition; the 0.27/0.28 cycle rows record the state at that cycle) |

The two `CHANGELOG.md`/`handover.md` records that the R2 review (FRICK-REVIEW-0104
Comment 2) flagged as still stale in its working tree were fixed by `5ff7198`, so
that Comment is addressed too. `make trace` re-converges after the manifest
reword (98/57/27/14/51).

### 3.3 Closure of FRICK-REVIEW-0103 D2 (R2 T17/T15 re-qualification review)

| Check | Evidence at `5ff7198` | Status |
| -- | -- | -- |
| Independent R2 re-qualification review exists and is committed | `docs/reviews/2026-10-11-renode-resc-r2-requal-review.md` = **FRICK-REVIEW-0104**, present in the `5ff7198` stat; conclusion **APPROVED WITH COMMENTS**; D2 recorded **closed** in its follow-up table | **CLOSED** |
| The review covers the changed T17 scripts | 0104 scope names `frick-renode.resc` and `renode_env.resc` (T17), `renode_vv.sh`, `renode_env_selftest.sh`, `Dockerfile`; confirms both load `@platforms/boards/beaglev-fire.repl` and that the SoC is included (`using platforms/cpus/polarfire-soc.repl`); gates reproduce | **CLOSED** |
| T15 disposition | 0104 records T15 (`emulated_run.sh`) unchanged → no re-qualification triggered; its QUALIFIED status stands (advisory SoC path) | **CLOSED** |
| T17 register re-qualified | `docs/vv/tool-qualification.md:110` History **0.3.12**: "Re-qualification: T17 `frick-renode.resc` **and** `renode_env.resc` now load the BeagleV-Fire board repl …" | **CLOSED** |
| T17 History names **both** changed `.resc` files | Same row names `frick-renode.resc` **and** `renode_env.resc` (the FRICK-REVIEW-0104 Minor finding 1 was corrected by `5ff7198`) | **CLOSED** |
| Review set logged (N11/N17/N20) | `docs/reviews/README.md` gains rows for FRICK-REVIEW-0103 and FRICK-REVIEW-0104; `docs/vv/verification-results.md` gains matching review-record rows | **CLOSED** |

### 3.4 Regression checks (no over-claim, no reopened NC)

- Both T17 `.resc` scripts load `@platforms/boards/beaglev-fire.repl` (board
  repl including the SoC); `make renode-env` reproduces PASS at HEAD.
- No QSPI over-claim: `grep` for `hal_mtd`/`hal_nor`/`hal_qspi`/`FF_QSPI0_BASE`
  across `src/`, `config/`, `tests/` returns **nothing**; the surviving
  "exercised on the emulated SoC" comments are the true GPIO/SERIAL/NVM ones.
  No record claims the QSPI write/erase check is exercised.
- Review-set completeness for this change: R4 (0101) + PA (0102/0103) + R2
  (0104) committed and logged; R1 is **arguably N/A** (no product C code
  changed by the board-repl switch).

## 4. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| C1 | Comment | N8/N19 — residual doc naming (FRICK-REVIEW-0103 D3, advisory) | `docs/tooling.md:70` still describes the self-test as "Renode runs + `polarfire-soc` platform loads" although `make renode-env` now loads the `beaglev-fire` **board** repl. It **under-states** the environment (the board repl includes the SoC) and is not a QSPI over-claim. No derogation. | `docs/tooling.md:70`; `tools/docker/renode/renode_env.resc:12`; `make renode-env` PASS | Optional cosmetics: reword the tooling row to name the `beaglev-fire` board repl (or the SoC-inclusive board platform). |
| C2 | Comment | N12/N16 — review-table status currency | The follow-up tables inside the newly committed reports still show their own D-items as **open** (FRICK-REVIEW-0103: `0102-F5`/`0102-F1`; FRICK-REVIEW-0104: `0104-1`), because the fixes landed in the same commit that introduced the reports. This is expected for the delta report (0103) whose residuals are closed by *this* closure review, but FRICK-REVIEW-0104's Minor `0104-1` was corrected in its own commit and has no separate R2 delta acknowledgment. The direct fix (T17 History naming both `.resc`) is verified here and `0104-1` is **closed by this review**. | `docs/reviews/2026-10-11-renode-board-pa-delta-review.md` §8; `docs/reviews/2026-10-11-renode-resc-r2-requal-review.md` §7; `docs/vv/tool-qualification.md:110` | Housekeeping only: annotate the 0104 table row as closed (or rely on this closure review's record). No product action required. |
| C3 | Comment | N16 — record-row housekeeping | `docs/vv/verification-results.md` gained two review-set rows under Revision **0.28** without a new `## History` row (the same convention as `b6b2d96`). The document is a running log whose History narrates cycles; the rows belong to the current 0.28 cycle. Consistent with precedent, not a defect. | `docs/vv/verification-results.md:386-387,421`; `git show b6b2d96 -- docs/vv/verification-results.md` | Optional: if desired, narrate the closure in a future cycle History row. No action required. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No Major or Minor finding was raised. C1–C3 are advisory Comments concerning
residual documentation naming and record housekeeping; none concerns the
correctness of the closure.

## 5. Conclusion

- [x] **APPROVED** — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**FRICK-REVIEW-0103 D1 — CLOSED.** All six live records named by D1 now state
the pending item is the **QSPI controller command driver** (board repl loaded);
the historical cycle rows are correctly preserved; `make trace` re-converges
(98/57/27/14/51). No over-claim and no prior-NC reopening.

**FRICK-REVIEW-0103 D2 — CLOSED.** The independent **R2** re-qualification
review **FRICK-REVIEW-0104** exists and is committed with `5ff7198`; the T17
register is re-qualified (History **0.3.12**) and now names **both** changed
`.resc` files (`frick-renode.resc` + `renode_env.resc`); T15 is correctly
recorded as unaffected; the change is confirmed correct/reproducible with no
QSPI over-claim.

**Board-repl change loop (norm N12) — NOW CLOSED.** With D1 and D2 corrected and
delta re-verified here, and with the prior chain closed (FRICK-REVIEW-0101
findings 1–4, 0102 F1–F5 + C1, 0103 D1/D2), **no open Major or Minor remains** in
PA scope for the board-repl change. N12 ("all rules re-applied, corrected,
re-verified and delta-reviewed to full compliance") is satisfied.

**N20 gate — SATISFIED.** The mandated independent review set (R4 + PA + R2,
R1 arguably N/A, plus the N12 deltas) is committed and logged, all gates
converge at HEAD, and no Major/Minor is open; per project precedent
(FRICK-REVIEW-0100 among others), a pass with no open Major/Minor is a pass. The
human PA/release authority retains the decision to accept the closed loop. The
advisory Comments C1–C3 carry no derogation.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0105 | 2026-10-11 | **APPROVED (D1/D2 CLOSED; N20 gate SATISFIED; N12 loop CLOSED)** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: closure of FRICK-REVIEW-0103 D1/D2 for the Renode board-repl change (BeagleV-Fire) at HEAD 5ff7198, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on the D1 rewording, the T17 register, the committed R2 report and the gates; Inference on the N12-loop / N20-gate status; Recommendations limited to advisory Comments C1-C3).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — clause texts not re-read; the invoked norms and the project qualification procedure carry the requirement. Customer tailoring matrix not available to this reviewer.
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at 5ff7198 and b6b2d96 (records in the header table); committed reports FRICK-REVIEW-0103/0104; pinned Renode image antmicro/renode@sha256:7905c329…fb061; gates re-run 2026-10-11 read-only.
Cross-check and reconciliation: task-stated numbers (trace 98/57/27/14/51; ctest 18/18; make renode-env PASS; metrics current) all reproduced independently; the D1 target list matches FRICK-REVIEW-0103 D1 exactly and each record reads correctly at HEAD; the T17 History 0.3.12 names both changed .resc files (FRICK-REVIEW-0104 Minor 1 corrected).
Confidence: High on D1 closed, D2 closed and the gate results (direct, configuration-matched evidence, read/run at HEAD); Medium-High on the N12/N20 wording (depends on project policy toward accepting a PA closure review as the delta acknowledgment for an R2-raised Minor — Comment C2).
Basis and detection limits: read-only inspection + gate re-runs on the host; the RTEMS ELF was pre-built (not rebuilt here); on-target/silicon QSPI behaviour and the QSPI controller command driver were not exercised (GAP-A5; M11); Lean (N1) was not re-run (no lake on this host) and is untouched by the change; `make renode` (full functional run) was not re-run by this reviewer (only the env self-test); the committed FRICK-REVIEW-0104 reproduces it (RENODE_TEST: PASS).
Missing/inaccessible evidence (including paywalls): none blocking; the customer tailoring matrix and ECSS clause texts beyond the invoked norms were not needed.
Assumptions and expiry/recheck date: the sibling reports are snapshots of their reviewed revisions; recheck on the next change to the Renode V&V scope or the T17 register.
Consequences if wrong: if a further change is made to tools/docker/renode/, tools/scripts/renode_*, the register, or the QSPI records, this closure applies only to the configuration reviewed (5ff7198) and must be re-verified (N10/N12).
Required independent human reviewer/authority: the project integrator/PA/release authority decides closure and any residual-risk acceptance; this review is not a certification, qualification, approval or flight-readiness decision.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0103-D1 | Minor | N8/N19 — stale "pending board repl" references | Live records not reconciled to the loaded board repl | Reworded the six live records to "pending the QSPI controller command driver"; historical cycle rows kept | `README.md:186`; `traceability.md:117`; `traceability.yaml:532`; `sprint-plan.md:42`; `handover.md:130`; `CHANGELOG.md:20-21`; `make trace` PASS | frick-fsw | **closed** | 0105 (this) |
| 0103-D2 | Minor | N9/N10/N11/N20 — missing R2 T17/T15 re-qualification review | Review set lacked the R2 review the register requires | Committed independent R2 review FRICK-REVIEW-0104; T17 register re-qualified | `docs/reviews/2026-10-11-renode-resc-r2-requal-review.md`; `docs/vv/tool-qualification.md:41,110`; gates reproduced | frick-fsw | **closed** | 0105 (this) |
| 0102-F5 | Minor | N8/N19 — stale platform/QSPI references | Carried by 0103 | Closed via D1 above | see 0103-D1 row | frick-fsw | **closed** | 0105 (this) |
| 0102-F1 | Minor | N11/N15/N17/N20 — review set not committed | Carried by 0103 (D2) | Closed via D2 above (R4 + PA + R2 committed and logged) | `docs/reviews/README.md` rows; `docs/vv/verification-results.md` rows | frick-fsw | **closed** | 0105 (this) |
| 0104-1 | Minor | N10/N16 — T17 History named only `frick-renode.resc` | R2 Minor; corrected in the same commit | History 0.3.12 amended to name both `.resc` scripts | `docs/vv/tool-qualification.md:110` | frick-fsw | **closed** | 0105 (this) |
| 0103-D3 | Comment | N8 — `tooling.md` self-test platform naming | Self-test described as `polarfire-soc` though it now loads the board repl (under-states; no over-claim) | Carried as advisory Comment C1 | `docs/tooling.md:70`; `renode_env.resc:12` | frick-fsw | open (no derogation) | n/a |

---

Prepared as the norm-N12 closure re-review (FRICK-REVIEW-0105). Per the task
instruction only this report file was created; the review-log row
(`docs/reviews/README.md`) and any `docs/vv/verification-results.md` review-record
row for FRICK-REVIEW-0105 remain the integrator's obligation (norms N11/N17/N20).