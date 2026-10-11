# Review Report — R2 independent re-qualification review (N10) of the changed Renode V&V tool scripts (T17/T15), board-repl switch

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0104                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (SW V&V)                            |
| Reference revision                | HEAD `b6b2d96` ("Renode board repl: close R4/PA review findings (N12)"), branch `main`; the working tree additionally carries the uncommitted **PA delta-D1 wording fix** (`README.md`, `docs/planning/sprint-plan.md`, `docs/requirements/traceability.md`, `docs/requirements/traceability.yaml`) and the untracked report `docs/reviews/2026-10-11-renode-board-pa-delta-review.md` (FRICK-REVIEW-0103). The four tool artifacts under review are **committed and clean** at `b6b2d96`. |
| Items under review                | Tool-qualification scope **T17** — `tools/docker/renode/frick-renode.resc`, `tools/docker/renode/renode_env.resc`, `tools/scripts/renode_vv.sh`, `tools/scripts/renode_env_selftest.sh`, `tools/docker/renode/Dockerfile`; and **T15** — `tools/scripts/emulated_run.sh`; register entry `docs/vv/tool-qualification.md` (T15/T17 rows + History 0.3.12). Board-repl change `8764dd4`, closure `b6b2d96`. |
| Date                              | 2026-10-11                             |
| Scope                             | Independent R2 source review + inspection + configuration-management check (norm N10, class B minimum) of the Renode V&V tool scripts after the functional platform was switched from `@platforms/cpus/polarfire-soc.repl` to `@platforms/boards/beaglev-fire.repl` (which includes the SoC); confirmation that the platform include preserves the SoC-level checks, that the functional run and env self-test reproduce, that the T17 register entry is updated/re-qualified, and that no QSPI-NOR over-claim exists. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer had no part in authoring, changing, executing or previously reviewing the items under review; fresh independent session with no author context. Read-only for the repository: only this report file was created; no tracked file was modified. Gates were re-run read-only (outputs under the git-ignored `build/` tree and ephemeral containers). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N4, N6, N8, N9, N10, N11, N15, N16, N17, N18, N19 |

## 1. Scope

This is the **norm-N10 independent R2 re-qualification review** of the V&V tool
scripts changed by the board-repl switch, and it is the review that the PA delta
review FRICK-REVIEW-0103 finding **D2** required (the register T17 row states
"independent **R2 on change**"; `docs/engineering/independent-review.md` marks
"Test script / V&V tool qualification review — R2 — Required (norm N10)").

**Included:**

- `frick-renode.resc` (functional path, T17) and `renode_env.resc` (self-test
  path, T17) now both load `@platforms/boards/beaglev-fire.repl`.
- `renode_vv.sh` and `renode_env_selftest.sh` driver logic, digest pinning,
  bounded execution, and pass/fail semantics.
- The `beaglev-fire.repl` platform actually shipped in the pinned image: does
  it include the SoC (`using "platforms/cpus/polarfire-soc.repl"`) so the
  SoC-level checks (`mmuart1 @ 0x20100000`, `e51`, `u54_1..4`) still resolve?
- Reproduction of the N18 gates (`make renode`, `make renode-env`), plus the
  companion gates named in the task: trace, `metrics.py --check`, `ctest`.
- The `docs/vv/tool-qualification.md` T15/T17 register entry and History row.

**Excluded (and why):**

- Correctness of `src/apps/app_renode_test.c` HAL-check internals and of the
  drivers (R1 scope; reviewed in FRICK-REVIEW-0081/0082, 0094-0099).
- The Lean proofs (N1); the change touches no `verification/lean/` source.
- On-target/silicon NOR behaviour and the QSPI write/erase check (still
  **GAP-A5**; the QSPI controller command driver is the open M11 item).
- PA finding D1's wording reconciliation and its residual (see F2) and D3
  cosmetics — these are PA documentation-currency matters, not T17/T15
  qualification; noted only for the change loop.

**Assumptions.** The pinned image digest
`sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`
is the controlled Renode environment; the RTEMS image
`build/renode/frick_fsw_renode.elf` (present, mtime 2026-10-11 19:25) is the
one under test.

### 1.1 Verification performed (independent, read-only)

| Gate / item | Method | Result |
| ---- | ------- | ------ |
| Board include (T17) | `cat /opt/renode/platforms/boards/beaglev-fire.repl` in the pinned image | First line `using "platforms/cpus/polarfire-soc.repl"`; then adds `qspi: SPI.MPFS_QSPI @0x21000000`, `pseFlash: SPI.Micron_MT25Q`, `sysServices: Miscellaneous.MPFS_SystemServices @0x37020000`, `syscontroller_qspi`, `UserButton @ gpio0` — **the SoC is included** |
| SoC node resolution | `grep` node names in `polarfire-soc.repl` | `e51:`, `u54_1:..u54_4:`, `mmuart1: UART.NS16550 @0x20100000` all exist → the resc references remain valid |
| Platform files exist | `ls` in image | `platforms/boards/beaglev-fire.repl` + `platforms/cpus/polarfire-soc.repl` present |
| Functional run (N18/T17) | `make renode` | **PASS** — `[renode] PASS (RENODE_TEST: PASS)`; console shows banner, 7 apps, and `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm`/`sb_roundtrip` = PASS; **no `hal_mtd`** marker (QSPI not exercised) |
| Env self-test (N18/T17) | `make renode-env` | **PASS** — `[renode-env] PASS: Renode runs and the PolarFire SoC board platform loads` |
| Trace convergence (N4) | `make trace` | **PASS** — 98 reqs / 57 srs / 27 sss / 14 gaps / 51 Lean theorems (matches task) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** |
| Digest pinning (CM) | `grep 7905c329 tools/` | `Dockerfile:10`, `renode_vv.sh:21`, `renode_env_selftest.sh:14` all pin the same digest — reproducible |
| Shell correctness | `bash -n` on both scripts | syntax OK |
| CM / clean tree | `git status --porcelain tools/ docs/vv/ src/apps/` | empty — in-scope tool artifacts committed at `b6b2d96` |
| CI wiring | `grep renode .github/workflows/ci.yml .gitea/workflows/ci.yml` | both run `renode_env_selftest.sh` + `renode_vv.sh` (claim `ci:renode` → T17) |
| Register (T17) | read `docs/vv/tool-qualification.md` | Revision **0.3.12**; History 0.3.12 records the board repl for T17; no QSPI-exercised claim in row 41 |

## 2. References

- Project norms: `docs/rules.md` (N1-N20), especially N6, N9, N10, N11, N15,
  N16, N17, N18, N19.
- Register: `docs/vv/tool-qualification.md` (T15 row, T17 row, History 0.3.12).
- Environment record: `docs/vv/renode-environment.md` (0.10), §2/§4/§5.
- Evidence: `docs/vv/verification-results.md` row "Renode board repl enabled
  (BeagleV-Fire)" (0.28) and History 0.28.
- Independence: `docs/engineering/independent-review.md` ("Test script / V&V
  tool qualification review — R2 — Required").
- Precedent / delta: FRICK-REVIEW-0100, 0101 (R4), 0102 (PA), **0103 (PA delta,
  findings D1/D2/D3)**.
- Tool artifacts: `tools/docker/renode/{frick-renode.resc,renode_env.resc,Dockerfile}`,
  `tools/scripts/{renode_vv.sh,renode_env_selftest.sh,emulated_run.sh}`.
- Pinned image: `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`.
- Standards: ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | **Minor** | N10 / N16 — `docs/vv/tool-qualification.md` T17 History 0.3.12 | The re-qualification History row names **only `frick-renode.resc`** ("T17 `frick-renode.resc` now loads the BeagleV-Fire board repl…"), but **two** T17 scripts were changed by this switch: `frick-renode.resc` **and** `renode_env.resc` (self-test, closed by PA Comment C1). The T17 row's Location already groups `tools/docker/renode/`, so the qualification is valid, but the History row is an incomplete record of *what* was re-qualified (N16 detail / N10 traceability of the re-qual trigger). | `git show b6b2d96 -- docs/vv/tool-qualification.md` (History 0.3.12 text); `tools/docker/renode/renode_env.resc:12`; `docs/vv/tool-qualification.md:41` | Amend History 0.3.12 to name both `.resc` scripts (functional + self-test) that were switched to the board repl. |
| 2 | Comment | N8 / N19 — stale "board repl" references (PA **D1** residual) | The D1 working-tree fix reconciles `README.md:186`, `docs/requirements/traceability.md`, `traceability.yaml` (`lean_note`) and `docs/planning/sprint-plan.md:42` to "pending the QSPI controller command driver". Two same-class records still say the check needs the **board repl**: `CHANGELOG.md:20` ("needs the QSPI NOR board repl… and the flash command path") and `docs/status/handover.md:130` ("on-SoC QSPI check pending board repl"). These **under-state** the environment (the repl is loaded) — they are **not** a QSPI over-claim — and are outside the T17/T15 tool-qualification scope; flagged so the D1 closure is complete. | `git diff` working tree (D1 files fixed); `CHANGELOG.md:20`; `docs/status/handover.md:130`; FRICK-REVIEW-0103 D1 | In the D1 closure, either reword these two live records to "pending the QSPI controller command driver" or record them as justified historical rows (as for `verification-results.md:39/0.27`). |
| 3 | Comment | N10 / N11 — register-update timing | The tool change landed in `8764dd4`; the T17 register update landed one commit later in `b6b2d96`. The qualification procedure step 5 literally says the register is "updated in the same commit" as the change. The change loop did include both commits and the register is now correct, so this is a process observation, not a defect. | `git log --oneline -- docs/vv/tool-qualification.md` (`b6b2d96` latest; absent in `8764dd4`); `docs/vv/tool-qualification.md:56` | For future tool changes, bump the register in the change commit itself (or state the loop-closure commit in the register row). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor finding 1 tracked to closure (Comments 2/3 no derogation)
- [ ] REJECTED - one or more Major findings

**T17/T15 re-qualification: SATISFIED.** The class-B N10 minimum (source
review + inspection + CM) is met for T17 and no change is triggered for T15:

- **T17 source-review correctness.** Both changed scripts load
  `@platforms/boards/beaglev-fire.repl`, which verifiably **includes the SoC**
  (`using "platforms/cpus/polarfire-soc.repl"`) and only *adds* the QSPI NOR,
  `MPFS_SystemServices` and a GPIO0 button; it redefines nothing the SoC checks
  depend on. The SoC-level nodes the functional script drives (`mmuart1`,
  `e51`, `u54_1..4`) still resolve, and the functional run reproduces with all
  HAL markers PASS.
- **Reproducibility.** The image is pinned by digest identically in the
  Dockerfile and both scripts; the container is run detached and bounded
  (no hang), with the console copied out defensively.
- **Honesty (no QSPI over-claim).** The QSPI NOR is now *reachable* but **not
  exercised**: `app_renode_test.c` emits no `hal_mtd`/`hal_nor`/`hal_qspi`
  marker, the T17 row claims only the six HAL checks actually emitted, and every
  live record states the on-SoC QSPI check still needs the controller command
  driver (M11). No record claims the write/erase check passes.
- **CM / register.** The T17 register is re-qualified (Revision 0.3.12 +
  History row) and the tool artifacts are committed. Finding 1 asks the History
  row to name both changed scripts for a complete record.
- **T15 unaffected.** `tools/scripts/emulated_run.sh` is unchanged and still
  loads the SoC-only `@platforms/cpus/polarfire-soc.repl` as its documented
  advisory path; the board-repl switch does not touch T15, so no T15
  re-qualification is triggered (its QUALIFIED status stands).

**Closing PA delta D2 (FRICK-REVIEW-0103): SATISFIED by this report.** D2 asked
for "the R2 (V&V) re-qualification review of T17, or a documented justification".
This report is that independent R2 review: it confirms the change is correct and
reproducible, the SoC-level checks are unaffected, the gates reproduce
(98/57/27/14/51; metrics current; ctest 18/18; `make renode-env` PASS;
`make renode` PASS), the register is re-qualified, and no QSPI over-claim
remains. The only residual is Minor finding 1 (History-row naming), to be
corrected in the closure commit; it does not reopen the qualification.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) / PA (N20) | R2 (independent agent, FRICK-REVIEW-0104) | 2026-10-11 | **APPROVED WITH COMMENTS** (Minor 1 to close; Comments 2/3) |

## 6. Uncertainty and confidence

```text
Decision/product and configuration: T17/T15 Renode V&V tool scripts at HEAD b6b2d96 (branch main), pinned image antmicro/renode@sha256:7905c329…fb061; working tree carries the uncommitted PA delta-D1 wording fix (README/traceability/sprint-plan) + untracked FRICK-REVIEW-0103.
Claim type: mixed (Facts: platform include, node resolution, gate results, register text; Inference: T15 non-trigger, D2-closure status; Recommendation: History-row naming).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — clause texts not re-read; the invoked norms and the project qualification procedure carry the requirement.
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at b6b2d96 (tool artifacts, register 0.3.12, renode-environment.md 0.10, verification-results.md 0.28); pinned Renode image contents (beaglev-fire.repl, polarfire-soc.repl); gates re-run 2026-10-11 read-only; FRICK-REVIEW-0101/0102/0103.
Cross-check and reconciliation: task-stated numbers (98/57/27/14/51 trace; 18/18 ctest; RENODE_TEST: PASS incl. hal_* checks) all reproduced independently; beaglev-fire.repl contents match the register/doc wording exactly; the D2 history (register requires "independent R2 on change") matches the committed review set (R4 + PA only before this report).
Confidence: High on the platform include, SoC-check preservation, gate reproduction, digest pinning and the absence of a QSPI over-claim (direct, configuration-matched, read/run at HEAD); Medium-High on finding 1's severity (the register Location already groups both scripts, so this is a completeness/History-detail matter); Medium on the D1 residual's disposition (outside T17/T15 scope; understates rather than over-claims).
Basis and detection limits: read-only inspection + gate re-runs on the host; the RTEMS ELF was pre-built (not rebuilt here); on-target/silicon QSPI behaviour and the QSPI controller command driver were not exercised (GAP-A5; M11); Lean (N1) was not re-run (no lake on this host) and is untouched by the change; the board button and MPFS_SystemServices models were inspected, not functionally exercised.
Missing/inaccessible evidence (including paywalls): none blocking. The R1 review of this board-repl change does not exist (arguably N/A: no product C code changed); this R2 report is the independent review the register requires.
Assumptions and expiry/recheck date: the integrator will commit this report, log it in docs/reviews/README.md and apply Minor 1; recheck at the T17 closure commit or the next change to any tools/docker/renode/ script.
Consequences if wrong: if Minor 1 is not applied, the T17 re-qualification record remains incomplete for one of the two changed scripts; the qualification and gates themselves are unaffected.
Required independent human reviewer/authority: the project integrator/owner decides closure; this review is not a certification, approval, qualification authority or flight-readiness decision.
```

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0104-1 | Minor | N10/N16 — T17 History 0.3.12 names only `frick-renode.resc` | `renode_env.resc` (also changed) omitted from the re-qualification row | Name both changed `.resc` scripts in History 0.3.12 | `docs/vv/tool-qualification.md` History row; `make trace` PASS | frick-fsw | **open** | pending |
| 0103-D2 | Minor | N9/N10/N11/N20 — missing R2 T17 re-qualification review | Independent R2 source review + inspection + CM now performed; change correct/reproducible, SoC checks unaffected, no QSPI over-claim, register re-qualified | This report (FRICK-REVIEW-0104) | `docs/reviews/2026-10-11-renode-resc-r2-requal-review.md`; gates reproduced (98/57/27/14/51; 18/18; `make renode-env`/`make renode` PASS; metrics current) | frick-fsw | **closed** | FRICK-REVIEW-0104 |
| 0103-D1 | Minor | N8/N19 — stale "pending board repl" references (PA) | Working tree fixes `README.md:186`, `traceability.md`, `traceability.yaml`, `sprint-plan.md:42`; `CHANGELOG.md:20` and `handover.md:130` remain (Comment 2) | Complete the D1 reconciliation in the closure commit | updated records; `make trace` PASS | frick-fsw | **open** (not T17/T15 scope) | pending |

---

Committed and pushed with the reviewed change (norm N11).