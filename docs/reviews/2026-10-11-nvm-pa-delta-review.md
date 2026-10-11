# Review Report — Product Assurance delta re-review of the reference eNVM NVM backend increment (norm N12/N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0085                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `03962d6` ("NVM backend: close R1/R2/R4/PA review findings (N12)"), branch `main`, pushed to `origin` and `gitea` |
| Items under review                | Delta re-review of FRICK-REVIEW-0084 (findings F1-F9, dispositions PA-84-01..PA-84-09) after the corrective commit `03962d6`: `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`, `tests/unit/test_nvm.c`, `src/apps/app_renode_test.c`; `docs/design/software-design.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`, `docs/requirements/traceability.{yaml,md}`, `CHANGELOG.md`, `README.md`; `docs/reviews/` review set for the increment |
| Date                              | 2026-10-11                             |
| Scope                             | Whether each FRICK-REVIEW-0084 finding (incl. Major F1) is closed at `03962d6`; whether the review set is committed/logged; whether the recorded evidence matches measurement; whether the N20 gate for the increment is now satisfied. ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2. |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session, no prior PA author context. Read-only for the repository: only this report file was created; no tracked file was modified. Gates were executed read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

Delta re-review (norm N12) of the Product Assurance findings raised in
FRICK-REVIEW-0084 against the SRS-REQ-055 reference eNVM NVM backend, after the
corrective commit `03962d6` (`0eb009d` -> `03962d6`).

**Included:** the disposition of F1 (Major process) and F2-F9 at HEAD; the
physical presence and factual correctness of the corrections; the commitment and
logging of the review set; a re-run of the project gates; and whether the N20
gate is now satisfied.

**Excluded:** the internal code correctness re-derived by R1/R2 (their own delta
re-reviews); the Lean proofs; on-target/silicon behaviour (GAP-A5); the
documentation-adjudication detail owned by R4. This review does not re-perform
the sibling reviews; it assesses whether the *process and records* obligations
that F1-F9 concerned are met.

**Method.** Read the `03962d6` diff and every in-scope artifact at HEAD;
independently executed the gates; measured the real `test_nvm` check count;
compared recorded numbers against measured ones; and inspected the repository
state (committed vs untracked, local vs remote).

## 2. References

- Prior PA review under closure: `docs/reviews/2026-10-11-nvm-pa-review.md` (FRICK-REVIEW-0084).
- Corrective commit: `03962d6` (`git show 03962d6`).
- Project norms: `docs/rules.md` (N1-N20), especially N4, N9-N12, N15-N20.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`; `tools/scripts/trace.py`.
- Records: `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md`,
  `CHANGELOG.md`, `docs/design/software-design.md`, `docs/status/handover.md`,
  `docs/status/loop-audit.md`, `README.md`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

| # | Check | Command / artifact | Result |
| - | ----- | ------------------ | ------ |
| 1 | Commit content | `git show --stat 03962d6` | 16 files: the 4 review reports + log/V&V rows + SDD/handover/loop-audit/tool-register/CHANGELOG/traceability.md/README + 3 code files |
| 2 | Review set tracked? (F1) | `git ls-files docs/reviews/ \| grep nvm`; `git status --short` | **All four reports tracked** (`…0081`, `…0082`, `…0083`, `…0084`); none untracked at HEAD |
| 3 | Review-log rows (F1) | `git show 03962d6 -- docs/reviews/README.md` | 4 rows added (R1/R2/R4/PA), latest PA row "REJECTED (Major F1 process) -> fixed" |
| 4 | V&V cycle rows (F1) | `git show 03962d6 -- docs/vv/verification-results.md` | 4 rows added to the review table; row 0.26 `test_nvm` 14 -> 36 |
| 5 | NVM check count (F2) | `grep -c FF_TEST_ASSERT tests/unit/test_nvm.c`; `./build/native/tests/test_nvm` | **36** and `checks=36 failures=0`; records quote "36 checks" (`CHANGELOG.md:15`, `verification-results.md:38`) |
| 6 | SDD design matrix (F3) | `git show 03962d6 -- docs/design/software-design.md`; `README.md:54` | §6 row adds `nvm/mpfs` + `SRS-REQ-055`; Revision 0.2 -> 0.3 + History; README **56/56 (100%)** |
| 7 | Handover remaining list (F4) | `git show 03962d6 -- docs/status/handover.md` | `NVM` removed from the remaining-M11 list (`:107`) |
| 8 | T17 register row (F5) | `git show 03962d6 -- docs/vv/tool-qualification.md` | T17 cell adds `hal_nvm`; header 0.3.6 -> 0.3.10 == History 0.3.10 |
| 9 | Self-test scratch offset (F6) | `git show 03962d6 -- src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}` | `FF_NvmMpfsSelfTest` now uses `FF_NVM_SCRATCH_OFF = FF_NVM_MAX_IDS*FF_NVM_SLOT_SIZE` (=1024), outside the id table; `nvm_init` removed |
| 10 | Renode oracle (F6, as written) | `src/apps/app_renode_test.c:225-236`; `git show --stat 03962d6` | **Unchanged** — app not in the commit; still `drv.nvm.read(0u, buf, sizeof(buf), &len) == FF_SUCCESS`; `docs/vv/renode-environment.md` not updated |
| 11 | Loop-audit PUS-C / History (F7) | `git show 03962d6 -- docs/status/loop-audit.md` | PUS-C row "reconciled … ADR-0016"; Revision 0.5 -> 0.6 + History |
| 12 | Tool-register header (F8) | `git show 03962d6 -- docs/vv/tool-qualification.md` | header 0.3.6 -> 0.3.10 (reconciled) |
| 13 | Stale gpio/mem counts (F9) | `grep -n "21 checks\|20 checks"` tool register | unchanged (Comment, pre-existing) |
| 14 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 97 requirements (56 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims (exit 0) |
| 15 | Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| 16 | Unit suite (N6) | `ctest --test-dir build/native` | **17/17 PASS** (incl. `nvm`) |
| 17 | Format (N8) | `clang-format --dry-run --Werror` on the 3 changed C files | clean (exit 0) |
| 18 | Remotes (N7) | `git branch -avv` | HEAD `03962d6` **=** `origin/main` **=** `gitea/main` (pushed) |
| 19 | Sibling-review closure state | `git status`; the committed R1/R2/R4 reports' follow-up tables | Original R1/R2/R4 reports committed as snapshots with **all NC rows still "open"/"pending"**; concurrent delta reports `FRICK-REVIEW-0086`/`0087`/`0088` exist **untracked** |
| 20 | Renode evidence recency (N18) | `stat build/renode/*`; commit time | `console.log` 17:28, elf 17:26 — **predates** `03962d6` (17:42); no fresh Renode run for the corrective commit |

## 4. Findings

### 4.1 Disposition of the FRICK-REVIEW-0084 findings

| # (0084) | Severity | Finding (abridged) | Delta evidence at `03962d6` | Disposition |
| -------- | -------- | ------------------ | --------------------------- | ----------- |
| **F1** | **Major** (process) | Increment pushed without committed review evidence / complete review set; R4 absent; N20 gate unsatisfied | All four reports committed **with** the change (check 2); review-log + V&V rows added (checks 3-4); R4 review exists (FRICK-REVIEW-0083). Per 0084's own criterion ("downgrades to Minor once they are committed and logged") | **Major REMEDIED → downgraded to Minor**; residual close-out in PA-85-02 |
| F2 | Minor | Recorded `test_nvm` "14 checks" vs measured | Records now "36 checks"; measured 36 (check 5) | **CLOSED** |
| F3 | Minor | SDD §6 matrix omitted `nvm/mpfs`/SRS-REQ-055 | §6 row + Revision/History; README 56/56 (check 6) | **CLOSED** |
| F4 | Minor | Handover still listed NVM as remaining M11 | NVM removed (check 7) | **CLOSED** |
| F5 | Minor | T17 row omitted `hal_nvm` | T17 row lists `hal_nvm`; header reconciled (checks 8,12) | **CLOSED** |
| F6 | Minor | Renode `hal_nvm` presence oracle non-discriminating; reads value-slot 0; recommends fixed scratch/identification probe + document emulator dependency | The **driver self-test** now uses an out-of-table scratch offset (check 9) — but the **Renode oracle itself is unchanged** (check 10); `renode-environment.md` not updated | **OPEN** — see PA-85-01 |
| F7 | Comment | Stale loop-audit PUS-C ASN.1 row | Reconciled in the loop-audit (check 11) | **CLOSED** |
| F8 | Comment | Tool-register header 0.3.6 vs History 0.3.9 | Header 0.3.10 == History 0.3.10 (check 12) | **CLOSED** |
| F9 | Comment | Pre-existing stale `test_gpio`(21)/`test_mem`(20) counts | Unchanged (check 13) | **OPEN** (advisory, as 0084 itself allowed) |

### 4.2 Residual and new findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| PA-85-01 | **Minor** | `src/apps/app_renode_test.c:225-236`; `docs/vv/renode-environment.md`; 0084 F6 (N10/N18) | **PA F6 as written is not corrected.** 0084 F6 concerned the *Renode `hal_nvm` oracle*, which still performs `drv.nvm.read(0u, buf, sizeof(buf), &len) == FF_SUCCESS` — non-discriminating (passes on a zero/aliased read; cannot distinguish "eNVM mapped" from "address reads zeros"). The corrective commit changed the *driver self-test* (`FF_NvmMpfsSelfTest`, a different function the Renode app never calls) to a safe out-of-table scratch offset; it did not touch the app, did not probe a fixed identification/scratch offset, and did not add the zero-initialised-emulator caveat to `renode-environment.md`. The header doc-comment on `FF_NvmMpfsSelfTest` still claims it is "used by the Renode functional test", which it is not. | `app_renode_test.c:235`; `git show --stat 03962d6` (no app file, no `renode-environment.md`); `ff_nvm_mpfs.h:47-49`; corroborated by FRICK-REVIEW-0086 (D-02) and FRICK-REVIEW-0087 (D1) | Apply the F6 recommendation: either wire the (now safe) self-test into the Renode check, or probe a fixed identification/scratch offset / accept any `FF_Status_t` as "bus responded"; fix the header claim; document the emulator dependency. |
| PA-85-02 | **Minor** | N12/N11/N15/N20 — closure of the review set | **The F1 close-out is incomplete: the sibling reviews are not closed and their delta reports are not committed.** The committed R1/R2/R4 reports (FRICK-REVIEW-0081/0082/0083) are snapshots whose follow-up tables still list **every** NC as "open"/"pending" (R1: 5, R2: 4, R4: 10). The delta re-reviews that would close them exist only as **untracked** files (FRICK-REVIEW-0086 R1, 0087 R2, 0088 R4), and each finds its parent review **not fully closed** (R1-04 open; R2 D1/D2-D4 open; R4 DOC-6/DOC-10 open). N12 closes an NC only when the disposition, the re-verification evidence and an APPROVED delta conclusion are recorded; N11 requires review results committed in the cycle. 0084 F1's stated closure path ("close the open R1/R2 Minor NCs, then re-review the deltas") is therefore not yet complete. | Committed R1/R2/R4 reports (follow-up tables all "open"/"pending"); `git status` → `?? …nvm-r1-delta-review.md`, `?? …nvm-r2-delta-review.md`, `?? …nvm-r4-delta-review.md`; their conclusions: R1-04 open, R2 "NOT fully closed", R4 "FRICK-REVIEW-0083 is NOT closed" | Correct or explicitly justify each residual Minor (R1-04 mock OOB masking; R2 header doc claim; R4 handover History row), **commit** the delta reports and their log rows, and re-review until each parent review is APPROVED/closed. |
| PA-85-03 | Comment | `tests/unit/test_nvm.c:24-34` (N6) | **Host-mock aliasing weakens the scratch-offset fix on the host.** The mock indexes with `off % sizeof(m->mem)`, and `sizeof(m->mem) = 32*32 = 1024 = FF_NVM_SCRATCH_OFF`; the self-test's out-of-table scratch accesses (1024, 1025) therefore wrap to slot-0 bytes `mem[0]`/`mem[1]` on the host, i.e. the id table is still touched by the host self-test. No current assertion is affected (id 0 is unused), and the target MMIO path is genuinely out-of-table, but the host oracle does not model the intended "outside the table" property (pre-existing R1-04/R2-7 class). | `test_nvm.c:27,33`; `ff_nvm_mpfs.h:31-35`; corroborated by FRICK-REVIEW-0086 (NC-0086-01) | Enlarge the mock to cover the scratch region and add a bounds tripwire (`assert off < sizeof(mem)`), so the host self-test exercises the true out-of-table behaviour. |
| PA-85-04 | Comment | `docs/vv/tool-qualification.md:95,97` (N11/N16) | F9's pre-existing stale `test_gpio` (21, actual 30) / `test_mem` (20, actual 23) counts are still uncorrected. Non-derogating (0084 graded it an optional Comment); listed for completeness. | tool register T3 history rows | Correct when the register is next touched. |
| PA-85-05 | Comment | N18 — Renode functional run | **No fresh Renode run for `03962d6`.** `build/renode/console.log` (17:28) and the Renode image (17:26) predate the corrective commit (17:42). The changed code (`FF_NvmMpfsSelfTest`) is not on the Renode app's execution path (`app_renode_test.c` calls `drv.nvm.read` only), so the prior `hal_nvm=PASS` remains behaviourally representative; but norm N18 as in force requires functional validation of *every* change. | `stat build/renode/*`; commit time | Re-run `make renode` on the corrective revision and record the console log, to keep the N18 evidence contemporaneous with HEAD. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity that
disqualifies the item for the applicable criticality, or a defect that can defeat a
requirement or a safety barrier; **Minor** = non-conformity that does not invalidate
the requirement but must be corrected or justified; **Comment** = observation/
recommendation, no derogation.

## 5. Process-obligation assessment (asked explicitly)

- **F1 / N15 / N9 / N17 / N11 / N20.** The Major's substance — the increment was
  pushed **without any committed review evidence** — is **remedied**: R1
  (FRICK-REVIEW-0081), R2 (0082), R4 (0083, N17) and the PA review (0084) are all
  committed with `03962d6`, and the `docs/reviews/README.md` review-log and
  `docs/vv/verification-results.md` cycle rows exist. **The Major is closed and
  downgraded to Minor**, exactly as 0084's F1 allowed. However, the F1 close-out
  path also required closing the R1/R2 NCs and re-reviewing the deltas, which is
  **not complete** (PA-85-02). Residual status: **Minor.**
- **N4 (trace convergence).** **Met.** `trace.py` PASS 97/56/27/14/51; SRS-REQ-055
  wired to SSS-REQ-020 with implementation + tests evidence.
- **N10 (tool register).** **Met for F5.** T17 gains `hal_nvm`; T3 count corrected;
  the register header is reconciled with its History.
- **N11 (records).** **Partially met.** The change's records (CHANGELOG,
  verification-results, SDD, handover, loop-audit, traceability, README) are
  consistent and the F2-F5 inaccuracies are corrected; but the review-set NC
  closure records are not complete (PA-85-02).
- **N14 (SSS systems review).** **Not triggered by this increment.** N14 fires on
  creation/modification of `docs/requirements/SSS.md` **or of the SSS entries in the
  traceability manifest**. `03962d6`/`0eb009d` did neither: the `SSS-REQ-020` SSS entry
  is byte-identical and only a new **SRS** child (`SRS-REQ-055`, `parent_sss:
  SSS-REQ-020`) was added. 0084's "arguably triggered" qualifier is therefore
  resolved: no R3/N14 review is required for this increment. (This determination
  should be recorded; see PA-85-02 note.)
- **N17 (documentation review).** **Partially met.** The R4 review exists and is
  committed, but its own delta review (FRICK-REVIEW-0088) finds it **not closed**
  (DOC-6/DEL-1: `handover.md` body 97 vs History 1.2 = 96/55/27/14). Under N17 the
  loop must close before commit/push; it has not.
- **N20 (PA review).** The PA review's Major is cleared; the review is re-assessed
  here. See §7 for the gate statement.

## 6. Honesty assessment

- **No over-claim introduced by the correction.** The records still scope the
  Renode check as an "MMIO presence check"; the SDD/handover/tool-register
  statements now match the implemented artifact (56/56 design trace; NVM not in
  the remaining list; T17 lists `hal_nvm`). The one factual fix (F2) is now
  reproducible: records quote 36 == source == binary.
- **The commit message is honest about its scope**, explicitly naming the
  R1/R2/R4/PA findings it closes and listing the gates it re-ran; it does **not**
  claim a fresh Renode run (consistent with PA-85-05).
- **No misstatement found** in the corrected records; the residual issues
  (PA-85-01/02/03/05) are gaps/omissions, not false statements.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — the Major is remedied; residual Minors and Comments tracked to closure
- [ ] REJECTED — one or more Major findings

The corrective commit is substantively effective. The **Major F1 is closed**: the
review set (R1/R2/R4/PA) is committed and logged with the change, so the specific
defect that caused the rejection — pushing without committed review evidence — is
gone. F2-F5, F7 and F8 are physically corrected and independently verified; the
machine gates converge at HEAD (`trace` 97/56/27/14/51, `metrics --check` 0,
`ctest` 17/17, format clean); and HEAD is pushed to both remotes. No Major remains
in the PA scope.

**FRICK-REVIEW-0084 disposition:**
- **Major F1 — CLOSED** (remedied; downgraded to Minor per 0084's own criterion).
- F2, F3, F4, F5, F7, F8 — **CLOSED**.
- **F6 — OPEN** (Minor): the Renode oracle targeted by F6 is unchanged
  (PA-85-01). What was fixed is the separate driver self-test clobber
  (R1-01/R2-2).
- **F9 — OPEN** (Comment, advisory).
- **FRICK-REVIEW-0084 is NOT yet fully closed**, pending PA-85-01 (F6) and the
  F1 close-out completion in PA-85-02 (which is outside PA's authority to close —
  the originating reviews must do so).

**N20 gate.** The gate has **moved from REJECTED (Major) to a conditional pass**:
the PA review for the increment has **no remaining Major**, so it is no longer
blocked; but it is **not yet fully satisfied** in the strict N12/N20 sense,
because one PA Minor (F6) is open and the review set's NC close-outs (R1/R2/R4)
are not complete and their delta reports are uncommitted (PA-85-02). The gate
becomes fully satisfied when F6 is corrected-or-justified and every sibling NC is
closed with committed delta reviews.

## 8. Confidence

```text
Decision/product and configuration: frick-fsw @ 03962d6; corrective delta of the
  SRS-REQ-055 reference eNVM NVM backend; ECSS criticality B target.
Claim type: mixed (Fact for gate results, file contents and repository/remote state;
  Inference for the process verdict).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1
  (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025). Customer tailoring matrix not
  available to this reviewer (unchanged from the PA-67/0084 assessments).
Evidence consulted: commit 03962d6 diff and message; src/drivers/nvm, tests/unit/test_nvm.c,
  src/apps/app_renode_test.c; SDD, handover, loop-audit, tool-qualification, verification-results,
  CHANGELOG, traceability.yaml/.md, README; docs/reviews/ (committed R1/R2/R4/PA reports + the
  untracked delta reports 0086/0087/0088); live gates (trace.py, metrics.py --check, ctest,
  test binary, clang-format); git refs/remotes; Renode build artifacts.
Cross-check and reconciliation: recorded "36 checks" vs measured 36; SDD §6 and README 56/56;
  handover body 97 vs History 1.2 = 96 (R4/DEL-1); Renode console timestamp vs commit time;
  local HEAD vs origin/gitea refs.
Confidence: High for the finding dispositions (direct file, git and gate evidence, reproduced
  locally); Medium on the N20-gate wording (whether the project treats an APPROVED-WITH-COMMENTS
  PA delta with open Minors as a "pass", and whether the untracked sibling deltas are the
  integrator's to commit). Independently corroborated by the concurrent R1/R2/R4 delta reviews.
Basis and detection limits: full diff and all in-scope records read; gates executed; test count
  measured. The sibling delta reports were untracked and may change before integration; only
  repository evidence at HEAD was assessed.
Missing/inaccessible evidence: customer tailoring matrix; any fresh Renode run for 03962d6
  (none exists); the RTEMS cross/on-target results (re-used as recorded, not re-run).
Assumptions and expiry/recheck date: assumes the task instruction to modify only this report is
  authoritative; assumes the untracked sibling delta reports are the integrator's to commit.
  Recheck on the next corrective commit (F6 + sibling NC closure) or 2027-04-11.
Consequences if wrong: if F6 is read as the driver self-test (not the Renode oracle), PA-85-01
  downgrades to a Comment and FRICK-REVIEW-0084 becomes closeable modulo PA-85-02; the sibling
  closure gap (PA-85-02) is unaffected either way.
Required independent human reviewer/authority: project engineering/PA/release authority; this
  agent cannot approve, close NCs, waive findings, or accept residual risk.
```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) | see FRICK-REVIEW-0086 / 0087 / — / 0088 (deltas) | 2026-10-11 | R1/R2/R4 deltas APPROVED WITH COMMENTS (residual Minors open); N14 not triggered |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0085** | **2026-10-11** | **APPROVED WITH COMMENTS (Major F1 closed; F6 + closure residuals tracked)** |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-84-01 | Major → Minor | N15/N9/N17/N20/N11/N7 — review set | Increment pushed without committed review evidence | corrected in `03962d6` (reports committed + logged); residual sibling closure in PA-85-02 | `git ls-files docs/reviews/` (4 reports); review-log + V&V rows | frick-fsw | **Major closed; Minor residual (PA-85-02)** | FRICK-REVIEW-0085 |
| PA-84-02 | Minor | `CHANGELOG.md`; `verification-results.md:38` | `test_nvm` "14 checks" | corrected to 36 | source/binary = 36; records read 36 | frick-fsw | **closed** | FRICK-REVIEW-0085 |
| PA-84-03 | Minor | `docs/design/software-design.md` §6 | SRS-REQ-055 missing from design matrix | corrected | SDD §6 + README 56/56 | frick-fsw | **closed** | FRICK-REVIEW-0085 |
| PA-84-04 | Minor | `docs/status/handover.md:107` | NVM in remaining-M11 list | corrected | `handover.md:107` | frick-fsw | **closed** | FRICK-REVIEW-0085 |
| PA-84-05 | Minor | `docs/vv/tool-qualification.md` T17 | `hal_nvm` missing | corrected | T17 row lists `hal_nvm` | frick-fsw | **closed** | FRICK-REVIEW-0085 |
| PA-84-06 | Minor | `src/apps/app_renode_test.c` (F6) | Non-discriminating Renode oracle | not corrected (self-test fixed instead) | `app_renode_test.c:235` unchanged | frick-fsw | **open** → PA-85-01 | FRICK-REVIEW-0085 |
| PA-84-07 | Comment | `docs/status/loop-audit.md` | Stale PUS-C row | corrected | loop-audit PUS-C row | frick-fsw | **closed** | FRICK-REVIEW-0085 |
| PA-84-08 | Comment | `docs/vv/tool-qualification.md:8` | Header rev stale | corrected (0.3.10) | header == History | frick-fsw | **closed** | FRICK-REVIEW-0085 |
| PA-84-09 | Comment | tool register `test_gpio`/`test_mem` counts | Stale counts | unchanged | see PA-85-04 | frick-fsw | **open (advisory)** | FRICK-REVIEW-0085 |
| PA-85-01 | Minor | `app_renode_test.c:225-236`; `renode-environment.md` | F6 Renode oracle unchanged | correct per F6 or justify | re-review of the app + doc | frick-fsw | open | pending |
| PA-85-02 | Minor | N12/N11 — sibling review closure | R1/R2/R4 reviews not closed; delta reports untracked | fix/justify residual Minors; commit delta reports + log rows | `git log` + closed NC tables | frick-fsw / review agents | open | pending |
| PA-85-03 | Comment | `tests/unit/test_nvm.c` mock | Scratch offset aliases to slot 0 on host | add bounds tripwire / enlarge mock | mock re-read + self-test probe | frick-fsw | open (advisory) | pending |
| PA-85-04 | Comment | tool register counts | gpio/mem stale counts | correct when next touched | register re-read | frick-fsw | open (advisory) | pending |
| PA-85-05 | Comment | N18 — Renode run | No fresh Renode run for `03962d6` | re-run `make renode` at HEAD | console log date == HEAD | frick-fsw | open (advisory) | pending |

---

Committed and pushed with the reviewed change (norm N11). The `docs/reviews/README.md`
review-log row and the `docs/vv/verification-results.md` cycle-log row for FRICK-REVIEW-0085
are to be added by the integrator (this review was instructed to modify no file other than
this report).