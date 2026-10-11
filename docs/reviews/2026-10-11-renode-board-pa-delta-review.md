# Review Report — Product Assurance delta re-review (N12/N20) of the Renode V&V environment board-repl change closure at `b6b2d96`

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0103                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `b6b2d96` ("Renode board repl: close R4/PA review findings (N12)"), branch `main`, **= `origin/main` = `gitea/main`** (already pushed) |
| Items under review                | Closing commit `b6b2d96` of the board-repl change `8764dd4`: `docs/vv/renode-environment.md` (0.10), `docs/status/loop-audit.md` (0.8), `docs/vv/tool-qualification.md` (0.3.12), `README.md`, `docs/rules.md` (N18), `tools/docker/renode/renode_env.resc`, `tools/scripts/renode_env_selftest.sh`, `docs/vv/verification-results.md`, `docs/reviews/README.md`; the review reports `docs/reviews/2026-10-11-renode-board-r4-review.md` (FRICK-REVIEW-0101) and `docs/reviews/2026-10-11-renode-board-pa-review.md` (FRICK-REVIEW-0102) |
| Date                              | 2026-10-11                             |
| Scope                             | Whether the R4 (N17) and PA (N20) **Minor** findings raised at `8764dd4` are closed by `b6b2d96` (norm N12): FRICK-REVIEW-0101 findings 1–4 and FRICK-REVIEW-0102 findings F1–F5 + Comment C1. Whether the board-repl change loop is closed and the N20 gate is satisfied; whether any stale `polarfire-soc` / QSPI over-claim remains. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (`git status` clean before and after). Gates were re-run read-only (outputs under the git-ignored `build/` and ephemeral containers). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the norm-**N12 delta re-review** (with norm-**N20** assurance scope) of the
**closing commit `b6b2d96`** of the small Renode V&V-environment change
`8764dd4`, which switches the functional-test platform from the SoC-only
`@platforms/cpus/polarfire-soc.repl` to the board repl
`@platforms/boards/beaglev-fire.repl` (which includes the SoC and adds the QSPI
NOR flash `SPI.MPFS_QSPI` + `SPI.Micron_MT25Q`, `MPFS_SystemServices`, and a
board button on GPIO0).

**Included:** closure of the six findings (five Minor + one Comment) raised by
FRICK-REVIEW-0101 (R4) and FRICK-REVIEW-0102 (PA): per-document revision/History
bumps (`renode-environment.md`, `loop-audit.md`), stale functional-platform
wording, the `hal_nvm` omission in the marker example / 5-of-13 enumeration, the
T17 re-qualification register entry, the change to the environment self-test
`renode_env.resc`, the review-set commit/logging (N11), and the residual
`polarfire-soc` / "pending board repl" cross-references (N8/N19); plus the
independence of the re-run gates.

**Excluded:** internal design/code correctness of the drivers and of the Renode
test application (R1/R2 scope); the Lean proofs (N1) beyond re-read; the
adjudication of documentation wording owned by R4 (N17) beyond the deltas; and
on-target/silicon NOR behaviour, which stays **GAP-A5**. The QSPI controller
command driver and the write/erase QSPI check remain out of scope (M11
follow-up).

**Method.** Read `git show b6b2d96` (full diff and `--stat`) and the parent
`git show 8764dd4`; inspected every touched artifact and its surrounding
records; inspected the actual `beaglev-fire.repl` inside the Renode image to
test the platform claim; re-ran `trace.py`, `metrics.py --check`, `ctest`,
`make renode-env` and `make renode`; inspected the captured console log; and
checked the repository/remote state.

## 2. References

- Project norms: `docs/rules.md` (N1-N20), especially N4, N8, N9, N10, N11, N12,
  N15, N16, N17, N18, N19, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056,
  `lean_note`); human mirror `docs/requirements/traceability.md`.
- Records: `docs/vv/verification-results.md` (0.28), `docs/vv/renode-environment.md`
  (0.10), `docs/vv/tool-qualification.md` (T17, 0.3.12), `CHANGELOG.md`,
  `docs/status/loop-audit.md` (0.8), `docs/planning/sprint-plan.md`,
  `docs/status/handover.md`, `README.md`.
- Platform source: `tools/docker/renode/frick-renode.resc`,
  `tools/docker/renode/renode_env.resc`, `tools/scripts/renode_vv.sh`,
  `tools/scripts/renode_env_selftest.sh`, `tools/scripts/emulated_run.sh`; pinned
  image `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`.
- Precedent: FRICK-REVIEW-0094/0099 (MTD PA review/closure),
  FRICK-REVIEW-0101 (R4 docs), FRICK-REVIEW-0102 (PA).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

| Gate / item | Command / method | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), **51** Lean theorems, 12 CI claims — matches the task and records |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** (incl. `nvm`, `mtd`) |
| Renode env self-test (N18/T17) | `make renode-env` | **PASS** — `[renode-env] PASS: Renode runs and the PolarFire SoC board platform loads` |
| Renode functional run (N18) | `make renode` | **PASS** — `[renode] PASS (RENODE_TEST: PASS)` |
| Console evidence | `build/renode/console.log` | banner + 7 apps; `hal_gpio`, `hal_serial`, `hal_mem`, `hal_eth`, `hal_net`, `hal_nvm`, `sb_roundtrip` = PASS; `RENODE_TEST: PASS` |
| Platform claim | `cat /opt/renode/platforms/boards/beaglev-fire.repl` in the image | `using "platforms/cpus/polarfire-soc.repl"` + `SPI.MPFS_QSPI @ 0x21000000` + `SPI.Micron_MT25Q` + `MPFS_SystemServices @ 0x37020000` + `syscontroller_qspi` + `UserButton @ gpio0` — **matches the commit message and the docs** |
| `renode-environment.md` (F3/R4-1/R4-3/R4-4) | read 0.10 + History | Revision **0.10** = latest History row; §4 platform = **board platform (`beaglev-fire`)**; §5 env = board platform; marker example includes `[renode-test] hal_nvm=PASS`; §5 peripheral-bound list = 5/13 with **NVM** (SRS-REQ-055) |
| `loop-audit.md` (F4/R4-2) | read 0.8 + History | Revision **0.8** = latest History row (QSPI disposition + environment change recorded) |
| `tool-qualification.md` (F2) | read T17 row + History | Revision **0.3.12**; History row 0.3.12 "Re-qualification: T17 `frick-renode.resc` now loads the BeagleV-Fire board repl…" |
| Self-test platform switch (C1) | `cat tools/docker/renode/renode_env.resc`; `renode_env_selftest.sh` message | `LoadPlatformDescription @platforms/boards/beaglev-fire.repl`; message = "…PolarFire SoC board platform loads" — matches the functional platform |
| Review set (F1/R4-5) | `git show --stat b6b2d96`; `ls docs/reviews`; `grep 8764dd4` | 0101 (R4) + 0102 (PA) committed + logged in `docs/reviews/README.md` + recorded in `verification-results.md`; **no R1/R2 sibling report** exists for this change |
| Stale cross-refs (F5/R4-5) | grep `pending board repl` / `needs the board repl` / `polarfire-soc` | **residual**: `README.md:186`, `docs/requirements/traceability.md:117`, `docs/requirements/traceability.yaml:532`, `docs/planning/sprint-plan.md:42`, `docs/status/handover.md:130`, `CHANGELOG.md:21`; `README.md:191/305` and `docs/rules.md:291` fixed |
| Repository/remote state | `git status`, `git branch -avv` | clean; HEAD `b6b2d96` = `origin/main` = `gitea/main` (pushed) |
| Lean gate (N1) | `make verify-lean` | **not reproduced on this host** (`lake: not found`); the change does not touch `verification/lean/`; recorded limitation |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D1 | **Minor** | N8 / N19 — FRICK-REVIEW-0102 **F5** not fully closed | The PA F5 disposition was to reconcile the platform/QSPI references to the new environment ("board repl enabled; QSPI check pending the **driver**"). Of the references F5 named, `README.md:191`, `README.md:305` and `docs/rules.md:291` were fixed, but **`README.md:186`, `docs/requirements/traceability.md:117` and `docs/requirements/traceability.yaml:532` (`lean_note`) were not**: they still say the on-SoC QSPI check is "pending board repl" although the board repl is now loaded. The same stale wording also remains in `docs/planning/sprint-plan.md:42`, `docs/status/handover.md:130` and `CHANGELOG.md:21`. The QSPI NOR model is now *reachable*, so the pending item is the QSPI **controller command driver**, not the repl (N19 "declared vs actual"). | `README.md:186`; `docs/requirements/traceability.md:117`; `docs/requirements/traceability.yaml:532`; `docs/planning/sprint-plan.md:42`; `docs/status/handover.md:130`; `CHANGELOG.md:21`; `git show b6b2d96` (README only §191/§305; no traceability hunk) | Reword the live records to "board repl enabled; on-SoC QSPI write/erase check pending the QSPI controller command driver (M11)"; keep historical cycle rows (`verification-results.md:39/0.27`) as-is. Re-run `make trace`. |
| D2 | **Minor** | N9 / N10 / N11 / N20 — FRICK-REVIEW-0102 **F1** not fully closed | F1's disposition target was "commit the **complete** review set (R1 + R2 + R4 + PA)". The commit contains **R4 (0101) + PA (0102)** only; no R1 or R2 report exists for `8764dd4`/`b6b2d96`. Per `docs/engineering/independent-review.md` a **test-script / V&V-tool change requires an R2 review** (T17 register row states "independent R2 on change"), and the board switch changed two recorded V&V scripts (`frick-renode.resc`, `renode_env.resc`) — so the **T17 re-qualification is self-declared** (History row 0.3.12) without the independent review the register requires (R1 is arguably N/A: no product C code changed). F1 is therefore only **partially** closed. | `git show --stat b6b2d96`; `ls docs/reviews` (no `*renode-board-r1*`/`*-r2*`); `docs/engineering/independent-review.md:107`; `docs/vv/tool-qualification.md:41` (T17 "independent R2 on change"); `docs/vv/tool-qualification.md:110` (T17 0.3.12) | Either commit the R2 (V&V) re-qualification review of T17, or record a documented justification that the R4 + PA + this delta review constitute the class-B re-qualification (N10 allows source review + inspection + CM at class B) — no silent gap. |
| D3 | **Comment** | N8 — FRICK-REVIEW-0101 Comment 6 / residual naming | The new `verification-results.md` board-repl cycle row still uses the short trace-cell form (`98/57/27/14, 51 theorems, 12 CI claims`) that R4 Comment 6 asked to align with the sibling long form; `docs/tooling.md:70` still describes the self-test as loading the "`polarfire-soc` platform" (now the board repl), and `tools/scripts/emulated_run.sh:36` still loads the SoC-only repl (advisory tool T15, not the N18 gate). No over-claim (these under-state the environment). | `docs/vv/verification-results.md` (board-repl row); `docs/tooling.md:70`; `tools/scripts/emulated_run.sh:36` | Optional cosmetics: align the cycle-row format; note the self-test now loads the board repl; leave `emulated_run.sh` as the documented advisory SoC path. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive closure (no finding).**

- **FRICK-REVIEW-0101 (R4) findings 1–4 — CLOSED.** `renode-environment.md`
  Revision **0.10** equals its latest History row and §4/§5 now name the board
  platform; the marker example adds `[renode-test] hal_nvm=PASS`; the
  peripheral-bound enumeration is **5/13** and includes **NVM (SRS-REQ-055)**;
  `loop-audit.md` Revision **0.8** equals its latest History row.
- **FRICK-REVIEW-0102 (PA) F2/F3/F4 — CLOSED.** T17 re-qualification History row
  **0.3.12** added (revision bumped); `renode-environment.md` brought current;
  `loop-audit.md` History advanced.
- **FRICK-REVIEW-0102 Comment C1 — CLOSED.** `renode_env.resc` now loads
  `@platforms/boards/beaglev-fire.repl` and the self-test message matches; the
  self-test again exercises the platform used by the functional run.
- **Claims honest — no over-claim, no prior NC reopened.** The switch is real
  (verified `beaglev-fire.repl` contents in the image), `make renode-env` and
  `make renode` both reproduce (`RENODE_TEST: PASS`, incl. `hal_nvm`), and no
  record claims the QSPI write/erase check is exercised. The remaining
  `polarfire-soc` mentions are the SoC included by the board repl and are
  accurate (except D3); the residual wording under D1 under-states the
  environment, so it is not a QSPI over-claim.
- **All gates reproduce**: trace 98/57/27/14/51, metrics panel current,
  ctest 18/18, `make renode-env` PASS, `make renode` PASS.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

The substantive content of the board-repl change is **sound and honest**: the
board repl is genuinely loaded, the environment self-test now matches the
functional platform (C1 closed), the documentation set is brought current on the
touched documents (R4 1–4, PA F2/F3/F4 closed), and every gate reproduces. Two
**Minor** non-conformities of the prior PA review remain **open**: **F5** (D1 —
stale "pending board repl" references in `README.md:186`, `traceability.md:117`,
`traceability.yaml:532` and same-class records) and **F1** (D2 — the committed
review set is R4 + PA, without the R2 V&V/tool re-qualification review that the
T17 register requires, or a recorded justification). D3 is a Comment with no
derogation.

**Board-repl change loop (norm N12): NOT fully closed** — two Minor residuals
(D1, D2) remain open; close them and delta-re-review.

**N20 gate: NOT yet satisfied.** The PA review passes only when its Minor
findings are corrected and re-reviewed (N12); with F1 and F5 still open, the N20
gate is not satisfied at `b6b2d96`. Closing D1 and D2 (or recording the
documented justifications) and re-reviewing the deltas should clear it — no
Major finding blocks the change.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0103 | 2026-10-11 | **APPROVED WITH COMMENTS (Minors D1/D2 to close; Comment D3)** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: closing commit b6b2d96 of the Renode board-repl change, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on gates/records/platform content; Inference on the N20-gate status; Recommendation for D1/D2).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025).
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at b6b2d96 and 8764dd4 (files in the header table); pinned Renode image antmicro/renode@sha256:7905c3...; gates re-run 2026-10-11 read-only.
Cross-check and reconciliation: recorded numbers (98/57/27/14/51; 18/18; RENODE_TEST: PASS incl. hal_* checks; metrics current) all independently reproduced; the beaglev-fire.repl contents match the commit/docs exactly; the residual stale strings were read in place at HEAD.
Confidence: High on closed findings F2/F3/F4/C1/R4-1..4 and on D1 (direct, configuration-matched, read at HEAD); Medium-High on D2 (the R1/R2 requirement is inferred from the independence matrix + the T17 register wording; F1's own text names R1+R2+R4+PA, but the class-B N10 minimum is source review + inspection + CM, so a recorded justification could close it).
Basis and detection limits: read-only inspection + gate re-runs; driver/test internal correctness (R1/R2 scope) was not independently re-derived; Lean (N1) was not re-run (no `lake` on this host) and the change does not touch verification/lean/.
Missing/inaccessible evidence (including paywalls): the R1/R2 sibling reports for the board-repl change are absent (D2); no blocking paywall (ECSS clause texts were not needed beyond the invoked norms).
Assumptions and expiry/recheck date: the integrator will decide whether R1/R2 are required or record the justification; recheck at the next closure commit.
Consequences if wrong: if D1/D2 are left unreconciled, the board-repl loop stays open and the N20 gate is not satisfied (N12/N20), and the traceability/README records keep a stale QSPI/board-repl statement (N8/N19).
Required independent human reviewer/authority: the project integrator/owner decides closure; this review is not a certification or approval.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0102-F5 | Minor | N8/N19 | Stale "pending board repl" references not reconciled (`README.md:186`, `traceability.md:117`, `traceability.yaml:532`, + `sprint-plan.md:42`, `handover.md:130`, `CHANGELOG.md:21`) | Reword the live records to "board repl enabled; QSPI check pending the controller driver"; keep historical cycle rows | updated README/traceability/sprint-plan/handover; `make trace` PASS | frick-fsw | **open** | pending (D1) |
| 0102-F1 | Minor | N9/N10/N11/N20 | Review set committed = R4 + PA only; R2 T17 re-qualification review (register requires "independent R2 on change") missing, R1 arguably N/A | Commit the R2 (or a documented class-B justification) and log it | `docs/reviews/*` R2 report + README row, or recorded justification | frick-fsw | **open** | pending (D2) |
| 0101-1 | Minor | N16 | `renode-environment.md` Revision/History not advanced | Bump to 0.10 + History row | `renode-environment.md:7,129` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0101-2 | Minor | N16/N19 | `loop-audit.md` Revision/History not advanced | Bump to 0.8 + History row | `loop-audit.md:7,161` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0101-3 | Minor | N8/N17/N18 | §4/§5 still described the functional run as `polarfire-soc` | Reword to the board platform | `renode-environment.md:74,82` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0101-4 | Minor | N8 | `hal_nvm`/NVM missing from marker example and 5/13 list | Add `hal_nvm=PASS` + NVM (SRS-REQ-055) | `renode-environment.md:45,94-97` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0102-F2 | Minor | N10/N8 | T17 register not updated for the changed `.resc` | Re-qualification History row | `tool-qualification.md:7,110` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0102-F3 | Minor | N8/N16 | `renode-environment.md` Revision/History + §4/§5 | Bump + History + reword | `renode-environment.md:7,74,82,129` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0102-F4 | Minor | N16 | `loop-audit.md` Revision/History | Bump + History row | `loop-audit.md:7,161` | frick-fsw | **closed** | FRICK-REVIEW-0103 |
| 0102-C1 | Comment | N8/N10 | `renode-env` self-test loaded the SoC-only repl | Switch to the board repl | `renode_env.resc:12`; `renode_env_selftest.sh:36`; `make renode-env` PASS | frick-fsw | **closed** | FRICK-REVIEW-0103 |

---

Committed and pushed with the reviewed change (norm N11).