# Review Report — Product Assurance review of the reference eNVM NVM backend increment (norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0084                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `0eb009d` ("Sprint 1.1: reference eNVM NVM backend (SRS-REQ-055) + Renode hal_nvm check"), branch `main`, pushed to `origin` and `gitea` |
| Items under review                | `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; `tests/unit/test_nvm.c`; `tests/CMakeLists.txt`; `CMakeLists.txt`; `src/apps/app_renode_test.c` (`renode_hal_nvm_check`); `config/platform/platform_mpfs.h`; requirement SRS-REQ-055 (SRS.md, traceability.yaml/.md, SSS-REQ-020 link); records `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md` (T3, 0.3.9), `CHANGELOG.md`, `docs/development-plan.md` (M11), `docs/status/loop-audit.md`, `docs/status/handover.md`, `README.md`; the review evidence set under `docs/reviews/` for this increment |
| Date                              | 2026-10-11                             |
| Scope                             | Process and records for the SRS-REQ-055 increment: whether the applicable process obligations are met (N15 full cycle incl. N18 Renode, N4 trace convergence, N10 tool register, N11 records, N9/N14/N17/N20 review set); honesty of the Renode/coverage claims; any record that overstates or misstates; whether the N20 gate is satisfied for this increment. |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified. Gates were executed read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1-N20 (see `docs/rules.md`), focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the norm-N20 Product Assurance review of the milestone M4/M11 increment
that adds the reference eNVM non-volatile-memory backend behind the HAL contract
`FF_Nvm_t` (SRS-REQ-055) and a `hal_nvm` clause to the Renode functional test, as
delivered and **pushed** at `0eb009d`.

**Included:** the process obligations this increment triggers (N15 full cycle,
N18 Renode, N4 trace, N10 tool register, N11 records, and the N9/N14/N17/N20
review set); the factual accuracy of its recorded evidence (test counts, cycle
log, tool register, status/handover statements, README panel); and whether any
record overstates the evidence.

**Excluded:** the internal correctness of the driver and test (owned by the R1
review FRICK-REVIEW-0081 and by R2); the Lean proofs; on-target/silicon
behaviour (GAP-A5). This review does not re-perform the R1/R2/R3/R4 reviews.

**Method.** Read the commit diff and every in-scope artifact; independently
executed the project gates; measured the actual unit-test assertion counts;
compared recorded numbers against measured ones; and inspected the repository
state (committed vs untracked, local vs remote) to determine whether the
mandated review evidence exists at the reviewed revision.

**Independence note (log row).** Per the task instruction this review modifies
nothing but this report file. The `docs/reviews/README.md` review-log row and
the `docs/vv/verification-results.md` cycle-log row for FRICK-REVIEW-0084 are
therefore **not** written here and remain the integrator's obligation
(norm N11/N17). The untracked sibling reports (R1 / FRICK-REVIEW-0081, R2 /
FRICK-REVIEW-0082) are likewise the integrator's to commit; this review is a
snapshot of the repository state and the sibling review set may change after it.

## 2. References

- Project norms: `docs/rules.md` (N1-N20), especially N4, N9, N10, N11, N15, N17, N18, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-055).
- HAL contract: `src/hal/hal_mem.h` (`FF_Nvm_t`, SRS-REQ-032).
- Records: `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md`,
  `CHANGELOG.md`, `docs/development-plan.md`, `docs/status/loop-audit.md`,
  `docs/status/handover.md`, `README.md`, `docs/planning/sprint-plan.md`.
- Renode platform definition: `platforms/cpus/polarfire-soc.repl` (Renode, `MTD.MPFS_eNVM` / mapped eNVM data).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — tool
  qualification, product-assurance records, traceability matrices verified at
  each milestone.

## 3. Verification performed (independent)

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 97 requirements (56 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims (matches the records) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **17/17 PASS** (incl. `nvm`) |
| NVM suite check count | `./build/native/tests/test_nvm` | **`checks=29 failures=0`** — the records say "14 checks" (Finding 2) |
| Sibling check counts | `test_gpio`, `test_serial`, `test_mem`, `test_r3_gap`, `test_r2_gap` | 30 / 30 / 23 / 201 / 487 (records: 21 / 30 / 20 / 201 / 487 — gpio/mem stale, pre-existing; Finding 9) |
| Renode evidence (N18) | `build/renode/console.log` (real run) | `hal_nvm=PASS`, `RENODE_TEST: PASS` (captured 2026-10-11 17:28, consistent with the commit) |
| Repository/remote state | `git branch -avv`, `git show --stat 0eb009d`, `git status` | HEAD `0eb009d` **=** `origin/main` **=** `gitea/main` (pushed); commit contains **no** `docs/reviews/*` file; `docs/reviews/2026-10-11-nvm-r1-review.md` and `docs/reviews/2026-10-11-nvm-r2-review.md` are **untracked** (`??`) |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F1 | **Major** | N15 / N9 / N17 / N20 / N11 / N7 — review set for this increment | The increment was **committed and pushed to both remotes without any committed review evidence and without the complete review set**. At the reviewed revision the R1 (FRICK-REVIEW-0081) and R2 (FRICK-REVIEW-0082, V&V) reports exist only as **untracked** working-tree files; **no R4 (documentation, N17) review exists** (required on *every* change); R3/N14 is arguably triggered (the SSS-REQ-020 child set changed in the manifest) and is unconfirmed; and **no `docs/reviews/README.md` log row** for the increment exists. Norm N15 requires the cycle (including N9/N10/N14/N17/N20 reviews and N12 close-out) to complete before commit/push, and N17/N20 require the reviews **before commit/push**; N20 requires the PA review to be *additional to* the R1/R2/R3/R4 reviews. The N20 gate is therefore **not satisfied** for this increment. | `git branch -avv` (HEAD = origin/main = gitea/main, `0eb009d`); `git show --stat 0eb009d` lists no `docs/reviews/*` and the message cites no review id; `git status` → `?? docs/reviews/2026-10-11-nvm-r1-review.md` and `?? docs/reviews/2026-10-11-nvm-r2-review.md`; `docs/reviews/README.md` last row is the test-gap PA delta (no eNVM row); `grep -rl SRS-REQ-055 docs/reviews/` → only the (untracked) R1/R2 reports | Complete the R4 (N17 documentation) review (and confirm/record the R3/N14 trigger), **commit** the R1/R2 reports and add the review-log rows, close the open R1/R2 Minor NCs, then re-review the deltas (N12). Only then is the N20 gate satisfied. If the reports are merely not yet integrated, this finding downgrades to Minor once they are committed and logged; as of the pushed revision it is a non-derogable-norm breach. |
| F2 | Minor | `CHANGELOG.md` (Unreleased/Added); `docs/vv/verification-results.md:38`; commit message | **The recorded NVM check count is wrong.** The change states "`test_nvm` … (14 checks)"; the harness (`tests/ff_test.h`) counts executed `FF_TEST_ASSERT` macros and the binary prints `checks=29 failures=0` (29 assertions in `test_nvm.c`). The evidence record is inaccurate (N11/N16). | `./build/native/tests/test_nvm` → `checks=29`; `grep -c FF_TEST_ASSERT tests/unit/test_nvm.c` = 29; `CHANGELOG.md`; `verification-results.md:38`; `git show 0eb009d` message | Correct the count to **29** in the CHANGELOG and the verification-results row, and avoid hard-coded counts in commit text. (Also independently raised as FRICK-REVIEW-0081 R1-05 and FRICK-REVIEW-0082 finding 1.) |
| F3 | Minor | `docs/design/software-design.md` §6; SRS-REQ-055 (N8) | **The SDD requirements→design matrix was not updated for the new requirement/design element.** The §6 "Reference drivers" row still reads `src/drivers/{eth/gem,gpio/mpfs,serial/mpfs,mem,net}` and lists SRS-REQ-036/038/040/047/048/049 only; neither the new `nvm/mpfs` path nor SRS-REQ-055 is present. This is visible as the README "Design traceability 55/56 (98.2%)" regression but is not tracked as a gap. | `docs/design/software-design.md:113`; `README.md:54`; `git show --stat 0eb009d` (no SDD file) | Add `nvm/mpfs` + SRS-REQ-055 to the SDD §6 reference-drivers row (or record the deferral explicitly). Norm N8: docs affected by the change are updated in the same commit. |
| F4 | Minor | `docs/status/handover.md:107` (N8/N19) | **Internal inconsistency: the handover still lists NVM among the remaining M11 peripheral classes.** Line 101 and 106 record NVM as done (`hal_nvm=PASS`, `ff_nvm_mpfs`, SRS-REQ-055), but line 107 still reads "Remaining M11 peripheral classes: NET(lwIP-on-GEM)/TSN/CAN/SPI/I2C/PCIE/SPACEWIRE/BLOCK/**NVM**". The development-plan M11 pending list was correctly updated (NVM removed); the handover was not. | `docs/status/handover.md:97-107`; `docs/development-plan.md:302` | Remove `NVM` from the handover remaining-M11 list so the resume point is self-consistent. |
| F5 | Minor | `docs/vv/tool-qualification.md` T17 (N10) | **The T17 register row's evidence description was not updated.** The 0.3.9 history entry says "T17 Renode functional check gains `hal_nvm`", but the T17 "Used for evidence of" cell still enumerates only `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`. Since T17's location includes `src/apps/app_renode_test.c` (which now has the NVM clause), the register is internally inconsistent. | `docs/vv/tool-qualification.md:41,71,107` | Add `hal_nvm` to the T17 row (and the `ci:renode` mapping note) in the same commit. |
| F6 | Minor | `src/apps/app_renode_test.c:225-236`; SRS-REQ-055 (evidence strength) | **The `hal_nvm` "presence check" oracle is non-discriminating.** It calls `drv.nvm.read(0, buf[4], &len)` and passes whenever the emulated byte at `FF_ENVM0_BASE` is a length `<= cap` (including `0`); it does not verify an eNVM-specific identifier, and a benign/zero (or, in Renode, an unmapped-read) byte yields `FF_SUCCESS`. It also reads value-space slot 0 rather than a fixed identification/scratch location, and on the Renode platform `0x20200000` is `MTD.MPFS_eNVM` (controller) while the mapped data array is at `0x20220000` (`platforms/cpus/polarfire-soc.repl`). The evidence therefore supports "an MMIO read responded", not unambiguously "the eNVM storage is present/addressed". The records **honestly** scope it as a presence check (not write/endurance) — see §6 — so this is an evidence-strength gap, not an over-claim. | `app_renode_test.c:225-236`; `ff_nvm_mpfs.c:43-65`; Renode `polarfire-soc.repl` (`envmCfg @0x20200000`, `envmData @0x20220000`); corroborated by FRICK-REVIEW-0081 R1-06 and FRICK-REVIEW-0082 finding 6 | Probe a fixed identification/scratch offset (or accept any `FF_Status_t` as "bus responded"), and document in `docs/vv/renode-environment.md` that the check relies on zero-initialised emulated eNVM / the emulator's eNVM layout. |
| F7 | Comment | `docs/status/loop-audit.md` (N19) | The loop-audit PUS-C ASN.1 row still states the N7/ESA library is "only fetched … NOT wired into build/runtime; default stays the local CCSDS open backend", contradicting `docs/status/handover.md` and ADR-0016 (PUS-C ASN.1 is the default). This loop updated the M11 row and the N3 count but did not reconcile this stale objective row (N19 requires the audit to reflect the actual implementation at the start of every loop). Pre-existing, not introduced by this increment. | `docs/status/loop-audit.md` (PUS-C row); `docs/status/handover.md:93-96`; `git show 0eb009d -- docs/status/loop-audit.md` | Reconcile the PUS-C row (closed, wired as default per ADR-0016) in the next loop-audit update. |
| F8 | Comment | `docs/vv/tool-qualification.md` header (N16) | The register's header revision is **0.3.6** while its History already carries 0.3.7, 0.3.8 and now 0.3.9. The header was not advanced with the history (stale before this change; left stale by it). | `docs/vv/tool-qualification.md:8,105-107` | Advance the header revision to match the latest history row as part of the N16 record discipline. |
| F9 | Comment | `docs/vv/verification-results.md`/tool register history (N11/N16) | Pre-existing stale check counts: T3's history cites `test_gpio` "21 checks" (actual 30) and `test_mem` "20 checks" (actual 23). Not caused by this increment, but the same class of record-accuracy defect as F2. | `./build/native/tests/test_gpio` = 30; `test_mem` = 23; `tool-qualification.md:95,97` | Optionally correct these counts when the records are next touched; adopt a rule that counts are quoted from the harness output, not hand-maintained. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 5. Process-obligation assessment (asked explicitly)

- **N15 (full cycle incl. N18 Renode).** The *execution* gates pass and are
  recorded: host build, `ctest` 17/17, `lake build` 21/21, `trace` 97/56/27/14/51,
  clang-format clean, and a **real** Renode run (`RENODE_TEST: PASS` incl.
  `hal_nvm`, console captured). However the cycle's **review** stage (N9/N14/N17/
  N20) was **not completed** before the change was committed and pushed
  (Finding F1). **Not fully met.**
- **N18 (Renode).** **Met.** The gate is in force and the real run passes with the
  new `hal_nvm` clause folded into the `RENODE_TEST: PASS` conjunction, so a
  regression in the clause fails the gate. Evidence: `build/renode/console.log`.
- **N4 (trace convergence).** **Met.** `trace.py` PASS 97/56/27/14/51; SRS-REQ-055
  is wired to SSS-REQ-020 with implementation + tests evidence and an explicit
  `lean_note` (no silent waiver). The `traceability.md` mirror and counts are
  consistent.
- **N10 (tool register).** **Mostly met, one gap.** T3 correctly gains
  `tests/unit/test_nvm.c`; the Renode testware (`app_renode_test.c`) is under T17
  and the history records the `hal_nvm` addition. But the T17 "used for evidence
  of" cell was not updated (Finding F5), and the qualification review of the
  changed testware/T3 is part of the outstanding review set (F1).
- **N11 (records).** **Partially met.** `verification-results.md` 0.26,
  `CHANGELOG.md` and the traceability mirror were updated in the change. But the
  only review report for the increment (R1/0081) is **uncommitted**, no review
  report was committed with the pushed change, and the records contain a factual
  error (F1/F2).
- **N9/N14/N17/N20 (review set).** **Not met at the reviewed revision** — see F1.
  R1 (R1/0081) and R2 (V&V/0082) reports exist but are **untracked** (no review
  report was committed with the pushed change and none is in the review log); the
  **R4 (N17) documentation review is absent**; R3/N14 is arguably triggered and
  unconfirmed; the PA review is this report.

## 6. Honesty assessment (asked explicitly)

- **The Renode claim is honestly scoped.** `app_renode_test.c:221-224`,
  `ff_nvm_mpfs.h:45-47`, the `CHANGELOG.md` entry, the `verification-results.md`
  0.26 row and `handover.md` all describe the check as an **MMIO presence/
  identification check**, explicitly note that write/erase register logic is
  host-tested and that real flash endurance is **GAP-A5**. No record claims write,
  erase or endurance validation on target. **No over-claim found.**
- **The M11 "5/13 peripheral-bound" claim is honest** and consistent across
  `development-plan.md`, `loop-audit.md` and `handover.md` (the NET contract check
  is explicitly labelled *not* peripheral-bound). The only M11 defect is the
  stale "remaining" list in `handover.md` (F4).
- **The coverage/status panel is current** (`metrics.py --check` exit 0) and the
  "Design traceability 55/56" row honestly reflects the SDD gap rather than
  overstating it (F3).
- **The one factual misstatement is the check count** ("14" vs measured **29**,
  F2), which *understates* the assertion count; it is an accuracy defect, not a
  capability over-claim.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED — one Major finding (F1)**

The technical product is sound: the driver and host test are correct and
bounded (independently corroborated by the R1 review FRICK-REVIEW-0081, which
found no Major), all automated gates pass (build, `ctest` 17/17, Lean, trace
97/56/27/14/51, format clean), and the **real** Renode run passes with
`hal_nvm=PASS` honestly scoped as a presence check. The rejection is on
**process and records**, not on product engineering:

1. the increment was **pushed to both remotes without its mandated review set**
   (no R4/documentation review, R3/N14 unconfirmed; the R1 and R2 reports
   untracked and unlogged) — the **N20 gate is not satisfied** (F1, Major); and
2. several records are inaccurate or stale (F2-F5).

The path to closure is short and mechanical: complete and commit the R4 (and
R3 if triggered) review, **commit** the R1/R2 reports and log them, close the
R1/R2 Minor NCs, correct the check count and the SDD/handover/tool-register
entries, then re-review the deltas (N12). On that basis the N20 gate can be
re-assessed as APPROVED WITH COMMENTS.

## 8. Confidence

```text
Decision/product and configuration: frick-fsw @ 0eb009d; SRS-REQ-055 reference eNVM NVM
  backend + Renode hal_nvm clause; ECSS criticality B target.
Claim type: mixed (Fact for gate results, file contents and repository/remote state;
  Inference for the process verdict).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1
  (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025). Customer tailoring matrix not
  available to this reviewer (unchanged from the PA-67 assessment).
Evidence consulted: commit 0eb009d diff and message; src/drivers, tests, config, Renode
  console log; traceability.yaml/.md; SRS.md; tool-qualification.md; CHANGELOG.md;
  verification-results.md; development-plan.md; loop-audit.md; handover.md; README.md;
  docs/reviews/README.md and the untracked R1 report; live gates (trace.py, metrics.py --check,
  ctest, test binaries); git refs/remotes; Renode platforms/cpus/polarfire-soc.repl.
Cross-check and reconciliation: recorded "14 checks" vs measured 29; recorded check counts
  for gpio/mem vs measured; local HEAD vs origin/gitea refs; Renode platform eNVM layout vs
  driver FF_ENVM0_BASE.
Confidence: High for the findings (direct file, git and gate evidence, reproduced locally);
  Medium on the F1 severity judgement (depends on whether the sibling R2/R3/R4 reviews are
  merely not yet integrated — if so, F1 downgrades to Minor once they are produced and committed).
Basis and detection limits: full diff and all in-scope records read; gates executed; test counts
  measured. The R2/R3/R4 reviews, if any exist outside the repository, were not visible; only
  the repository evidence at HEAD was assessed.
Missing/inaccessible evidence: customer tailoring matrix; any R2/R3/R4 review not committed to
  the repository; the RTEMS cross/on-target results (re-used as recorded, not re-run).
Assumptions and expiry/recheck date: assumes the task instruction to modify only this report is
  authoritative; assumes the untracked R1/R2 reports are the integrator's to commit. Recheck on the
  next baseline change, on completion of the review set, or 2027-04-11.
Consequences if wrong: if F1 is intended to be a process state rather than a finding, the verdict
  becomes APPROVED WITH COMMENTS with F1 as the leading Minor; all other findings (F2-F9) hold.
Required independent human reviewer/authority: project engineering/PA/release authority; this
  agent cannot approve, close NCs, waive findings, or accept residual risk.
```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) | — | | |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0084** | **2026-10-11** | **REJECTED (1 Major, F1)** |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-84-01 | Major | N15/N9/N17/N20/N11/N7 — review set | Increment pushed without any committed review; R4 (N17) absent, R3/N14 unconfirmed; R1/R2 reports untracked/unlogged; N20 gate not satisfied | Complete R4 (and R3 if triggered); **commit** R1/R2 reports + log rows; close R1/R2 NCs | `git log`/review log showing R4 report and tracked R1/R2; N12 delta re-review | frick-fsw / review agents | open | pending |
| PA-84-02 | Minor | `CHANGELOG.md`; `verification-results.md:38` (N11/N16) | Recorded "14 checks" vs measured 29 | Correct to 29 | binary output + record re-read | frick-fsw | open | pending |
| PA-84-03 | Minor | `docs/design/software-design.md` §6 (N8) | SRS-REQ-055 / `nvm/mpfs` missing from the design matrix | Update §6 (or record deferral) | SDD re-read; README design-trace 56/56 | frick-fsw | open | pending |
| PA-84-04 | Minor | `docs/status/handover.md:107` (N8/N19) | NVM listed among remaining M11 classes though done | Remove NVM from the remaining list | handover re-read | frick-fsw | open | pending |
| PA-84-05 | Minor | `docs/vv/tool-qualification.md` T17 (N10) | Evidence-of cell not updated with `hal_nvm` | Add `hal_nvm` to T17 | register re-read | frick-fsw | open | pending |
| PA-84-06 | Minor | `src/apps/app_renode_test.c:225-236` (evidence strength, N10) | Non-discriminating presence oracle (accepts benign/zero read; reads value-slot 0) | Fixed-id/scratch offset or any-status bus-response check; document emulator dependency | platform note + re-run | frick-fsw | open | pending |
| PA-84-07 | Comment | `docs/status/loop-audit.md` (N19) | Stale PUS-C ASN.1 objective row | Reconcile in next loop-audit update | audit re-read | frick-fsw | open | pending |
| PA-84-08 | Comment | `docs/vv/tool-qualification.md:8` (N16) | Header revision 0.3.6 vs history 0.3.9 | Advance header | register re-read | frick-fsw | open | pending |
| PA-84-09 | Comment | `docs/vv/tool-qualification.md:95,97` (N11/N16) | Pre-existing stale `test_gpio`(21)/`test_mem`(20) counts | Correct when next touched | register re-read | frick-fsw | open | pending |

---

Committed and pushed with the reviewed change (norm N11). The `docs/reviews/README.md`
review-log row and the `docs/vv/verification-results.md` cycle-log row for FRICK-REVIEW-0084
are to be added by the integrator (see §1).