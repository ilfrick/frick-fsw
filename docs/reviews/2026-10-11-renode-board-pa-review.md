# Review Report — Product Assurance review (norm N20) of the Renode V&V environment board-repl change (BeagleV-Fire) at `8764dd4`

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0102                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `8764dd4` ("Renode V&V environment: enable the BeagleV-Fire board repl (QSPI NOR + sys services)"), branch `main`, **= `origin/main` = `gitea/main`** (already pushed) |
| Items under review                | `tools/docker/renode/frick-renode.resc`; records `docs/vv/verification-results.md` (rev 0.28), `docs/vv/renode-environment.md`, `CHANGELOG.md`, `docs/status/loop-audit.md`; scope M11 `SRS-REQ-056` (QSPI NOR reachability); the review evidence set under `docs/reviews/` for this change |
| Date                              | 2026-10-11                             |
| Scope                             | Whether the claims of the environment change are honest (all existing checks still pass; the QSPI NOR model is now **reachable but not exercised** — no over-claim); whether the change is appropriately recorded (N8/N16/N10); whether the N18 functional validation remains valid at the new platform; whether the change reopens a prior NC; and whether the review set is committed with the change (N11/N15/N17/N20). ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean before and after). Gates were re-run read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the norm-**N20** Product Assurance review of the small **Renode V&V
environment** change delivered at `8764dd4`, which switches the functional-test
platform from the SoC-only `@platforms/cpus/polarfire-soc.repl` to the board
platform `@platforms/boards/beaglev-fire.repl` (which includes the SoC and adds
`SPI.MPFS_QSPI@0x21000000` + `SPI.Micron_MT25Q` NOR flash, `MPFS_SystemServices`,
and a board button on GPIO0).

**Included:** the honesty of the recorded claims (existing HAL checks still pass;
the QSPI NOR model is reachable but **not** exercised); whether the change is
recorded in the affected V&V/engineering records (N8/N16) and whether it
invalidated the qualification of the changed V&V script (N10); whether the N18
functional gate is still valid on the new platform; whether the change reopens
any prior NC; and the state of the review set (N11/N15/N17/N20).

**Excluded:** internal design/code correctness of the drivers and of the Renode
test application (R1/R2 scope); the Lean proofs (N1) beyond re-read; the
adjudication of documentation wording owned by R4 (N17); on-target/silicon NOR
behaviour, which stays **GAP-A5**. The `QSPI` controller command driver and the
write/erase QSPI check are explicitly out of scope (M11 follow-up).

**Method.** Read `git show 8764dd4` (full diff and `--stat`); inspected every
touched artifact and the surrounding records; inspected the actual
`beaglev-fire.repl` inside the pinned Renode image to test the commit's
platform claim; re-ran `trace.py`, `metrics.py --check`, `ctest` and
`make renode`; inspected the captured console log; and checked the repository/
remote state.

## 2. References

- Project norms: `docs/rules.md` (N1-N20), especially N4, N8, N9, N10, N11, N12,
  N15, N16, N17, N18, N19, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056,
  `lean_note`); human mirror `docs/requirements/traceability.md`.
- Records: `docs/vv/verification-results.md` (0.28), `docs/vv/renode-environment.md`
  (0.9), `docs/vv/tool-qualification.md` (T17, 0.3.11), `CHANGELOG.md`,
  `docs/status/loop-audit.md` (0.7), `README.md`.
- Platform source: `tools/docker/renode/frick-renode.resc`,
  `tools/docker/renode/renode_env.resc`, `tools/scripts/renode_vv.sh`,
  `tools/scripts/renode_env_selftest.sh`; pinned image
  `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`.
- Precedent: `docs/reviews/2026-10-11-mtd-pa-review.md` (FRICK-REVIEW-0094, F1),
  `docs/reviews/2026-10-11-nvm-pa-review.md` (FRICK-REVIEW-0084).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

| Gate / item | Command / method | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), **51** Lean theorems — matches the 0.28 record |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest` in `build/native` | **18/18 PASS** (incl. `mtd`, `nvm`) |
| Renode functional run (N18) | `make renode` (pinned image, real RTEMS ELF present) | **PASS** — `[renode] PASS (RENODE_TEST: PASS)` |
| Console evidence | `build/renode/console.log` | banner + 7 apps; `hal_gpio=PASS`, `hal_serial=PASS`, `hal_mem=PASS`, `hal_eth=PASS`, `hal_net=PASS`, `hal_nvm=PASS`, `sb_roundtrip=PASS`, `RENODE_TEST: PASS` |
| Platform claim | `cat /opt/renode/platforms/boards/beaglev-fire.repl` in the pinned image | `.repl` `using "platforms/cpus/polarfire-soc.repl"` + `SPI.MPFS_QSPI @ 0x21000000` + `SPI.Micron_MT25Q` + `Miscellaneous.MPFS_SystemServices @ 0x37020000` + `UserButton @ gpio0` — **matches the commit message and the docs** |
| Record completeness | grep/read of the touched records | verification-results 0.28 + CHANGELOG + renode-environment §1/§2 + loop-audit row updated; **gaps** in tool-register (F2), renode-environment §4/§5 + History (F3), loop-audit History (F4), stale cross-refs (F5) |
| Lean gate (N1) | `make verify-lean` | **not reproducible on this host** (`lake: not found`); the change does not touch `verification/lean/`; the 0.28 `lake build 21/21` claim was **not** independently re-run (recorded limitation) |
| Repository/remote state | `git show --stat 8764dd4`, `git branch -avv`, `git status` | HEAD `8764dd4` **=** `origin/main` **=** `gitea/main` (pushed); commit contains **no** `docs/reviews/*` file; `docs/reviews/README.md` has no row for this change; `git status` clean (no untracked sibling reports) |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F1 | **Minor** | N11 / N15 / N17 / N20 — review set | The change was **committed and pushed to both remotes with no review report in the commit** and no `docs/reviews/README.md` row. Per the task/integrator the sibling **R1/R2/R4 reviews of this change are produced in the same batch** and are not present at this snapshot. This is the repo's established batch model; the precedent (FRICK-REVIEW-0094 F1) is a Major **only while unproduced** and **downgrades to Minor once the reports are run, committed and logged**. Tracked here as Minor under the stated batch scope. | `git show --stat 8764dd4` (only 5 files, no `docs/reviews/*`); `git branch -avv` → HEAD = origin/main = gitea/main; `ls docs/reviews` shows no report for this change; README log lacks a row | Commit the complete review set (**R1 + R2 + R4 + PA**) and add the `docs/reviews/README.md` rows in the same batch (N11); condition for the N20 gate. |
| F2 | **Minor** | N10 / N8 — tool qualification register | The V&V script `tools/docker/renode/frick-renode.resc` changed (platform path), which **invalidates the T17 qualification** until re-qualified; `docs/vv/tool-qualification.md` was **not updated** in the same commit (T17 row unchanged, Revision still 0.3.11, no History row). Norm N10 requires the register to be maintained in the same commit and the tool re-qualified before its evidence is reused. | `git show 8764dd4 -- tools/docker/renode/frick-renode.resc`; `docs/vv/tool-qualification.md:41` (T17), `:107-108` (last T17 history rows) | Add a T17 re-qualification entry + History row (and any testware scope note) in this batch; the sibling R2 review provides the re-qualification review (N9/N10). |
| F3 | **Minor** | N8 / N16 — `docs/vv/renode-environment.md` | The document is **not brought current**. The header Revision is still **0.9** (line 7) and the `## History` table has **no row** for this change; and §4 (line 73) and §5 (line 80) still state that the functional run uses the **`polarfire-soc`** platform, contradicting §1/§2 which now say `beaglev-fire`. N16 requires the per-doc History updated in the same commit; N8 requires the doc to stay current/consistent. | `docs/vv/renode-environment.md:7,26,73,80,112-124`; `git show 8764dd4` (only §1/§2 touched) | Bump Revision (→ 1.0), add a History row, and correct §4/§5 to the board repl (`beaglev-fire`, includes the SoC). |
| F4 | **Minor** | N16 — `docs/status/loop-audit.md` | The N19 dispositions row for the on-SoC QSPI check was edited, but the document Revision stays **0.7** (line 7) with **no History row**; the loop-audit carries its own `## History` table and N16 applies. | `git show 8764dd4 -- docs/status/loop-audit.md` (only the table row changed); `docs/status/loop-audit.md:7,160` | Add a loop-audit History row (rev 0.8) describing the environment change and the reworded disposition. |
| F5 | **Minor** | N8 / N4 — stale cross-references | Records that cite the platform or the QSPI status were **not updated** after the switch: `README.md:186` ("Renode check pending (board repl `MPFS_QSPI`+`Micron_MT25Q`)"), `README.md:191` and `:305` (`polarfire-soc` platform); `docs/rules.md:291` (N18 environment text: `polarfire-soc` platform); `docs/requirements/traceability.md:117` and `traceability.yaml` SRS-REQ-056 `lean_note` ("on-SoC QSPI check **needs the board repl**"). The board repl is now loaded; the pending item is the **controller command driver**, not the repl. | grep of the repo for `polarfire-soc`/board-repl wording; `docs/requirements/traceability.yaml:532`; `docs/requirements/traceability.md:117` | Reconcile these references to the new environment (board repl enabled; QSPI check pending the controller driver). |
| C1 | **Comment** | N8 / N10 — self-test scope | The environment **self-test** `tools/docker/renode/renode_env.resc` (and `tools/scripts/emulated_run.sh`) still loads the SoC-only `polarfire-soc.repl`, so `make renode-env` no longer exercises the platform actually used by the functional run. | `tools/docker/renode/renode_env.resc:12`; `tools/scripts/renode_env_selftest.sh:36`; `tools/scripts/emulated_run.sh:36` | Switch the self-test to `@platforms/boards/beaglev-fire.repl`, or document why the self-test deliberately validates only the SoC platform. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive observations (no finding).**

- **The claims are honest — no over-claim.** The switch is real: the pinned image
  contains the claimed `beaglev-fire.repl` contents, and `make renode` was
  independently **reproduced** (`RENODE_TEST: PASS`, all HAL checks including
  `hal_nvm`). The records state the QSPI NOR model is only *reachable* and that a
  genuine write/erase check **still needs the controller command driver (M11)** —
  in the commit message, `CHANGELOG.md`, the 0.28 row and `renode-environment.md`
  §2. No record claims the QSPI check is exercised (the earlier over-claim was
  retired in `fec12b2`/`cb2576d`; grep of `src/`/`config/`/`docs/vv/` finds no
  `hal_mtd`/`FF_QSPI0_BASE` residue).
- **N18 remains valid.** The board repl includes the SoC platform, the real RTEMS
  ELF boots (U54_1 @ `0x1000000000`, console `mmuart1`) and the gate passes on the
  new platform; the per-commit SKIP proxy / nightly real-run policy is unchanged.
- **No prior NC is reopened.** The change records the QSPI model as reachable
  but unexercised, so the retired QSPI over-claim NC (chain 0094→0099) is not
  reintroduced.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

The **product** of the change is sound and its claims are **honest**: the board
repl is genuinely loaded, `make renode` still passes, and the QSPI NOR model is
correctly described as *reachable but not yet exercised* (no over-claim; no prior
NC reopened). The **N18** functional validation remains valid on the new
platform. The findings are **record/process** defects (F1–F5) that do not
invalidate the change but must be corrected in the same cycle (N12). The N20 gate
**passes once F1–F5 are closed and delta-re-reviewed** and the complete review
set (R1+R2+R4+PA) is committed and logged; F1 is tracked as Minor under the
stated same-batch scope.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0102 | 2026-10-11 | **APPROVED WITH COMMENTS (Minors F1-F5 to close; Comment C1)** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: Renode V&V environment board-repl change at 8764dd4, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on gates/records/platform content; Inference on the N20-gate status; Recommendation for F1-F5).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025).
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at 8764dd4 (files in the header table); pinned Renode image antmicro/renode@sha256:7905c3...; gates re-run 2026-10-11 read-only.
Cross-check and reconciliation: recorded numbers (98/57/27/14/51; 18/18; RENODE_TEST: PASS incl. hal_* checks; metrics current) all independently reproduced; the beaglev-fire.repl contents match the commit/docs exactly.
Confidence: High on the honesty of the claims, the N18 validity and F1-F4 (direct, configuration-matched evidence); High on F5; the F1 severity is conditional by construction (same-batch review set).
Basis and detection limits: read-only inspection + gate re-runs; driver/test internal correctness (R1/R2 scope) was not independently re-derived; Lean (N1) was not re-run (no `lake` on this host) and the change does not touch verification/lean/.
Missing/inaccessible evidence (including paywalls): none blocking; sibling R1/R2/R4 reports are not yet present at the reviewed revision (F1); ECSS clause texts were not needed beyond the invoked norms.
Assumptions and expiry/recheck date: sibling R1/R2/R4 reviews are produced/committed in the same batch (as stated); recheck at the batch/closure commit.
Consequences if wrong: if the sibling reviews are not committed with the closure (N11/N12), F1 reverts toward the Major classification of the FRICK-REVIEW-0094 precedent and the N20 gate is not satisfied.
Required independent human reviewer/authority: the project integrator/owner decides closure; this review is not a certification or approval.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| F1 | Minor | N11/N15/N17/N20 | Review set not committed with the change; N20 gate not yet satisfied | Commit R1+R2+R4+PA reports and add `docs/reviews/README.md` rows in this batch | `docs/reviews/*` for this change committed; README log rows present | frick-fsw | open | pending |
| F2 | Minor | N10/N8 | T17 qualification register not updated for the changed `frick-renode.resc` | Add T17 re-qualification entry + History row (R2 review = re-qualification) | updated `docs/vv/tool-qualification.md` | frick-fsw | open | pending |
| F3 | Minor | N8/N16 | `renode-environment.md` Revision/History not bumped; §4/§5 stale platform | Bump revision, add History row, fix §4/§5 to `beaglev-fire` | updated `docs/vv/renode-environment.md` | frick-fsw | open | pending |
| F4 | Minor | N16 | `loop-audit.md` disposition changed without Revision/History bump | Add loop-audit History row (rev 0.8) | updated `docs/status/loop-audit.md` | frick-fsw | open | pending |
| F5 | Minor | N8/N4 | Stale platform/QSPI references (README, rules.md N18, traceability.md/yaml) | Reconcile to the new environment (board repl enabled; QSPI check pending the driver) | updated README/rules/traceability; `trace.py` PASS | frick-fsw | open | pending |
| C1 | Comment | N8/N10 | `renode-env` self-test still loads the SoC-only repl | Switch to the board repl or document the intent | updated `renode_env.resc` / selftest | frick-fsw | open | n/a |

---

Committed and pushed with the reviewed change (norm N11).