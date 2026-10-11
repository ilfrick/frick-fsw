# Review Report — Product Assurance final review of the reference eNVM NVM backend increment (norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0089                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `7e00e4c` ("NVM backend: close delta review findings (N12) - Renode oracle, mock, records"), branch `main`, pushed to `origin` and `gitea` |
| Items under review                | Closure of FRICK-REVIEW-0084 (Major F1 + F2–F9) and FRICK-REVIEW-0085 (PA-85-01..05), plus the sibling delta findings FRICK-REVIEW-0086 (R1 D-R1-01..05, D-02/D-03), FRICK-REVIEW-0087 (R2 D1–D4) and FRICK-REVIEW-0088 (R4 DEL-1/DEL-2), at HEAD. Products: `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; `tests/unit/test_nvm.c`; `src/apps/app_renode_test.c` (`renode_hal_nvm_check`); `config/platform/platform_mpfs.h`; requirement SRS-REQ-055; records `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md`, `docs/status/handover.md` (1.3), `docs/status/loop-audit.md`, `docs/design/software-design.md`, `CHANGELOG.md`, `README.md`, `docs/reviews/README.md`, and the committed delta reports 0085/0086/0087/0088. |
| Date                              | 2026-10-11                             |
| Scope                             | Whether the SRS-REQ-055 increment's N20 gate is now satisfied at HEAD `7e00e4c`: the closure of the prior Major and Minors, the honesty of the Renode `hal_nvm` evidence, the mock de-aliasing, the record reconciliation, and the commitment/logging of the delta review set. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified. Gates were executed read-only (outputs under the git-ignored `build/`); the Renode probe ran in an ephemeral container. |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the norm-N20 Product Assurance **final** review of the milestone M4/M11
increment that adds the reference eNVM non-volatile-memory backend behind the HAL
contract `FF_Nvm_t` (SRS-REQ-055) and a `hal_nvm` clause to the Renode functional
test, as delivered and **pushed** at HEAD `7e00e4c`. It closes the loop opened by
FRICK-REVIEW-0084 (Major F1, REJECTED) and continued by FRICK-REVIEW-0085, and it
adjudicates the residual delta findings of the sibling R1/R2/R4 delta reviews
(0086/0087/0088) to the extent they bear on the process/evidence obligations PA owns.

**Included:** the disposition of FRICK-REVIEW-0084 F1–F9 and FRICK-REVIEW-0085
PA-85-01..05 at HEAD; the honesty and strength of the Renode `hal_nvm` evidence;
the host-mock de-aliasing (R1-04/D3); the record reconciliation (handover History,
tool-register wording, header claims); the commitment and logging of the delta
review set; a re-run of the project gates; and the N20-gate statement.

**Excluded:** the internal code correctness owned by R1 (FRICK-REVIEW-0081/0086,
re-derived there); the V&V adequacy owned by R2 (0082/0087); the documentation
adjudication owned by R4 (0083/0088); the Lean proofs; and on-target/silicon
behaviour, which is GAP-A5 by the increment's own scope. This review does not
re-perform the sibling reviews; it assesses the process, records and evidence
obligations they converge on.

**Method.** Read the HEAD diff and every in-scope artifact; independently executed
the project gates; measured the real `test_nvm` check count; reconciled recorded
numbers against measured ones; inspected the repository/remote state (committed vs
untracked, local vs remote); and independently probed the Renode emulated eNVM
platform to test the honesty of the recorded justification for the read-presence
check.

**Independence note (log rows).** Per the task instruction this review modifies
nothing but this report file. The `docs/reviews/README.md` review-log row and the
`docs/vv/verification-results.md` cycle-log row for FRICK-REVIEW-0089 are
therefore **not** written here and remain the integrator's obligation (norm
N11/N17).

## 2. References

- Prior reviews under closure: `docs/reviews/2026-10-11-nvm-pa-review.md`
  (FRICK-REVIEW-0084), `docs/reviews/2026-10-11-nvm-pa-delta-review.md`
  (FRICK-REVIEW-0085), and the sibling deltas `…-nvm-r1-delta-review.md`
  (0086), `…-nvm-r2-delta-review.md` (0087), `…-nvm-r4-delta-review.md` (0088).
- Corrective commits: `0eb009d` (delivery), `03962d6` (first close-out),
  `7e00e4c` (HEAD, delta close-out).
- Project norms: `docs/rules.md` (N1–N20); entry point `AGENTS.md`.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`;
  `tools/scripts/trace.py`.
- HAL contract: `src/hal/hal_mem.h` (`FF_Nvm_t`, SRS-REQ-032).
- Renode platform: `tool/docker/renode/frick-renode.resc` (loads
  `@platforms/cpus/polarfire-soc.repl` from the pinned `antmicro/renode` image,
  digest `sha256:7905c329…`).
- Records: `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md`,
  `CHANGELOG.md`, `docs/design/software-design.md` §6, `docs/status/handover.md`
  (1.3), `docs/status/loop-audit.md`, `README.md`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — tool
  qualification, product-assurance records, traceability matrices.

## 3. Verification performed (independent)

| # | Check | Command / artifact | Result |
| - | ----- | ------------------ | ------ |
| 1 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 97 requirements (56 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims (exit 0) — matches the records |
| 2 | Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| 3 | Unit suite (N6) | `ctest --test-dir build/native` | **17/17 PASS** (incl. `nvm`) |
| 4 | NVM check count (N11) | `./build/native/tests/test_nvm`; `grep -c FF_TEST_ASSERT tests/unit/test_nvm.c` | **`checks=36 failures=0`**; source = 36; records quote "36 checks" |
| 5 | Delta reports committed (N11) | `git ls-files docs/reviews/ \| grep nvm`; `git status --short` | **All four deltas tracked** (0085/0086/0087/0088); working tree clean; no untracked review file |
| 6 | Delta log/V&V rows (N11) | `git show 7e00e4c -- docs/reviews/README.md docs/vv/verification-results.md` | **4 review-log rows + 4 V&V rows** added for the deltas |
| 7 | Repository/remote state (N7) | `git branch -avv`, `git status` | HEAD `7e00e4c` **=** `origin/main` **=** `gitea/main` (pushed); clean tree |
| 8 | Mock de-aliasing (R1-04/D3) | `tests/unit/test_nvm.c:18,24-36` | `MockNvm_t.mem` enlarged to `FF_NVM_MAX_IDS*FF_NVM_SLOT_SIZE + 8` (1032 B); modulo removed; direct `off < sizeof(m->mem)` guard on both read and write; scratch offsets 1024/1025 are now in-range and genuinely exercised |
| 9 | Header claim (R2 D1) | `src/drivers/nvm/mpfs/ff_nvm_mpfs.h:47-49` | No longer claims a Renode caller; now "host unit test … available for on-target MMIO round-trip checks; on the emulated SoC the eNVM region is read-only, so the Renode check is a read presence check" |
| 10 | Renode oracle (F6/PA-85-01) | `src/apps/app_renode_test.c:221-238` | Still `setup` + `drv.nvm.read(0u,…)==FF_SUCCESS`; comment documents the read-presence scope and GAP-A5; no write/endurance over-claim |
| 11 | Handover History (R4 DEL-1) | `docs/status/handover.md:7,117,186` | Revision **1.3** == History row **1.3** (counts 97/56/27/14); body M11 5/13 and remaining list no longer contain NVM (`:107`) |
| 12 | Tool-register wording (R2 D2) | `docs/vv/tool-qualification.md:108` | `test_nvm` "**29 → 36** checks" (the "14 → 36" narrative corrected) |
| 13 | SDD/README design trace (F3) | `docs/design/software-design.md:113`; `README.md:54` | §6 row carries `nvm/mpfs` + SRS-REQ-055; README **56/56 (100%)** |
| 14 | Fresh Renode run (PA-85-05) | `stat build/renode/console.log` vs `git log -1 --format=%ci 7e00e4c` | console **18:02:38**, ELF 18:02:23 — contemporaneous with HEAD content (commit 18:05:45); **supersedes** the stale 03962d6-era log (17:28). `hal_nvm=PASS`, `RENODE_TEST: PASS` |
| 15 | Renode eNVM platform probe (F6 justification) | ephemeral `antmicro/renode:latest` (Renode 1.16.1) container, `polarfire-soc.repl` | `0x20200000` (`envmCfg`) read → `0x00`; write `0xAA` → **warning "Unhandled write … Unhandled bits"**, read-back `0x00` (no fault); `0x20220000` (eNVM data array) write `0x5A` → read-back `0x5A` (**writable**); `0x20200400` → "non existing peripheral". Detection limit: local image digest `e278e663…` ≠ the Dockerfile-pinned digest |
| 16 | N14 trigger (systems review) | rules.md §N14; `git show 7e00e4c -- docs/requirements/SSS.md traceability.yaml` | SSS.md and the SSS manifest entries untouched; only a new **SRS** child was added → **N14 not triggered** (confirms FRICK-REVIEW-0085) |

## 4. Findings

### 4.1 Disposition of FRICK-REVIEW-0084 / FRICK-REVIEW-0085 and the sibling deltas

| Prior NC | Severity | Finding (abridged) | Evidence at `7e00e4c` | Disposition |
| -------- | -------- | ------------------ | --------------------- | ----------- |
| 0084 **F1** / PA-84-01 | **Major** (process) | Increment pushed without committed review evidence / complete review set | Review set committed + logged with `03962d6` (checks 5–6); confirmed still present at HEAD | **CLOSED** (remedied; downgraded to Minor per 0084's own criterion) |
| 0084 F2 / PA-84-02 | Minor | `test_nvm` recorded "14 checks" vs measured | Records "36 checks"; source = binary = 36 (check 4) | **CLOSED** |
| 0084 F3 / PA-84-03 | Minor | SDD §6 omitted `nvm/mpfs`/SRS-REQ-055 | SDD §6 + README 56/56 (check 13) | **CLOSED** |
| 0084 F4 / PA-84-04 | Minor | Handover listed NVM as remaining M11 | Removed (check 11) | **CLOSED** |
| 0084 F5 / PA-84-05 | Minor | T17 row omitted `hal_nvm` | T17 row lists `hal_nvm` (tool register `:41`) | **CLOSED** |
| **0084 F6 / PA-85-01** | Minor | Renode `hal_nvm` presence oracle non-discriminating; header claimed a Renode caller | Oracle unchanged but now **explicitly justified** (emulated region read-only / write self-test faults); header claim corrected (checks 9–10) | **CLOSED-BY-JUSTIFICATION**, with residual **PA-89-01** (Minor) |
| 0084 F7 / PA-84-07 | Comment | Stale loop-audit PUS-C row | Reconciled (ADR-0016) | **CLOSED** |
| 0084 F8 / PA-84-08 | Comment | Tool-register header vs History | Header 0.3.10 == History 0.3.10 | **CLOSED** |
| 0084 F9 / PA-85-04 | Comment | Stale `test_gpio`(21)/`test_mem`(20) counts | Still stale (actual 30/23) | **OPEN** (advisory) → PA-89-04 |
| PA-85-01 | Minor | F6 oracle not corrected | Reverted to documented read-presence + header fixed | **CLOSED (justified)** → PA-89-01 |
| PA-85-02 | Minor | Sibling reviews not closed; deltas untracked | Deltas committed + logged (checks 5–6) | **CLOSED** |
| PA-85-03 | Comment | Host mock aliases scratch to slot 0 | Mock enlarged + direct bounds guard (check 8) | **CLOSED** |
| PA-85-04 | Comment | tool register gpio/mem counts | unchanged | **OPEN** (advisory) → PA-89-04 |
| PA-85-05 | Comment | No fresh Renode run for the corrective commit | Fresh run at HEAD (18:02 vs commit 18:05) (check 14) | **CLOSED** |
| 0086 D-R1-04 / D-02 | Minor | Mock masks out-of-range offsets (self-test aliased) | Fixed (check 8): no aliasing; scratch covered | **CLOSED** |
| 0086 D-03 | Comment | `-Wall -Wextra` not on `frick_fsw_core` | Not addressed | **OPEN** (advisory, outside the NVM-increment gate) |
| 0087 D1 | Minor | Header claimed a non-existent Renode caller | Corrected (check 9) | **CLOSED** |
| 0087 D2 | Comment | History narrative "14 → 36" used the wrong pre-fix baseline | Corrected to "29 → 36" (check 12) | **CLOSED** |
| 0087 D4 | Comment | CI provenance parentheticals omit SRS-REQ-055 | unchanged | **OPEN** (advisory) → PA-89-03 |
| 0088 DEL-1 (DOC-6) | Minor | Handover counts changed without a History row/revision bump | Revision 1.3 + History row 1.3 (check 11) | **CLOSED** |
| 0088 DEL-2 (DOC-10) | Comment | NVM still framed as upcoming work | `README.md:408` and `handover.md:125` drop NVM from the "next" list | **CLOSED** |

### 4.2 New / residual findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| **PA-89-01** | **Minor** | `src/apps/app_renode_test.c:221-226`; commit message `7e00e4c`; SRS-REQ-055 (N8/N10/N11/N18) | **The stated justification for the read-presence check is not corroborated in-repo and is imprecise.** The code comment and commit message assert "the emulated eNVM region is read-only" and "a write self-test **faults** on the model — empirically confirmed by a failed Renode run". An independent probe of the locally available Renode image shows: (a) the driver's base `FF_ENVM0_BASE = 0x20200000` is the `envmCfg` **configuration** block — its read returns `0x00`, and a write is **dropped with a warning ("Unhandled write … Unhandled bits"), not a fault**; (b) the eNVM **data array** at `0x20220000` is **writable** (a `0x5A` write round-trips). So the blanket "read-only / faults" wording is inexact, the check exercises configuration registers rather than eNVM storage, and no failed-run log is archived. The *scope* of the check is nonetheless honestly stated as present-only with no write/endurance over-claim (so this is a documentation/evidence-accuracy defect, not a capability over-claim). | `app_renode_test.c:221-226`; `git log -1 7e00e4c`; check 15 (`envmCfg` warning + data-array writability); no archived failed-run artifact; `config/platform/platform_mpfs.h:47` | Correct the wording to the measured behaviour (e.g. "the `envmCfg` block does not retain writes, so a write/read self-test cannot round-trip on the model"), record which region the presence check exercises, and either archive the failed-run console log under the **pinned** image digest or document the image digest used to establish the claim. |
| PA-89-02 | Comment | `docs/vv/renode-environment.md` (N10) | The Renode environment note still mentions eNVM only in the platform list (`:25`); it does **not** document the `hal_nvm` presence-check scope/limitation, which FRICK-REVIEW-0084 F6 recommended. The caveat currently lives only in the application code comment. | `docs/vv/renode-environment.md:25`; `git show 7e00e4c` (file not touched) | Add a short paragraph to `renode-environment.md` stating the `hal_nvm` check is an MMIO read-presence check and its emulator dependency. |
| PA-89-03 | Comment | `.github/workflows/ci.yml:80`, `.gitea/workflows/ci.yml:83` (N6) | The `ci-renode` provenance parentheticals still omit `SRS-REQ-055` (R2 D4); the gate checks only claim-name existence, so this is non-blocking. | `ci.yml:80` | Add `SRS-REQ-055` to both parentheticals when the workflows are next touched. |
| PA-89-04 | Comment | `docs/vv/tool-qualification.md:95,97` (N11/N16) | Pre-existing stale counts: T3 history cites `test_gpio` "21 checks" (actual **30**) and `test_mem` "20 checks" (actual **23**). Non-derogating (0084 graded it an optional Comment). | `./build/native/tests/test_gpio` = 30; `test_mem` = 23; register history rows | Correct when the register is next touched; prefer quoting counts from the harness output. |
| PA-89-05 | Comment | `CHANGELOG.md` (N16) | The HEAD commit modifies behaviour-adjacent artifacts (app, driver header, test) but does not touch `CHANGELOG.md`. The existing `[Unreleased]` NVM entry is accurate but predates the delta close-out. | `git show --stat 7e00e4c` (no CHANGELOG); `CHANGELOG.md:9-15` | Add/append a CHANGELOG bullet for the N12 close-out (read-presence scope, mock de-aliasing, record reconciliation). |
| PA-89-06 | Comment | committed FRICK-REVIEW-0081/0082/0083 (N12) | The parent R1/R2/R4 reports were committed as snapshots with every NC row still "open"/"pending"; the actual closures are recorded in the (now committed) delta reports 0086/0087/0088. This is a bookkeeping artefact, not a live NC. | committed 0081/0082/0083 follow-up tables; 0086/0087/0088 dispositions | Optionally annotate the parent tables ("closed via FRICK-REVIEW-008X") so a reader does not mistake the snapshots for open work. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 5. Process-obligation assessment (asked explicitly)

- **N15 (full cycle incl. N18 Renode).** **Met.** All execution gates pass and are
  recorded: host build, `ctest` 17/17, `lake build` 21/21, `trace`
  97/56/27/14/51, `clang-format` clean, `metrics --check` exit 0, and a **real**
  Renode run contemporaneous with HEAD (`RENODE_TEST: PASS` incl. `hal_nvm=PASS`).
  The review stage (N9/N14/N17/N20) is now committed and logged (checks 5–6).
- **N18 (Renode).** **Met**, with a documented evidence-scope limit. The gate is
  in force and the fresh run passes; the `hal_nvm` clause is folded into the
  `RENODE_TEST: PASS` conjunction, so a regression fails the gate. The check is a
  **read-presence** check (PA-89-01/PA-89-02 record the scope).
- **N4 (trace convergence).** **Met.** `trace.py` PASS 97/56/27/14/51; SRS-REQ-055
  is wired to SSS-REQ-020 with implementation + test evidence; the mirror and
  counts are consistent.
- **N10 (tool register).** **Met.** T3 gains `test_nvm.c`; T17 lists `hal_nvm`;
  the caller/header claim is corrected; the register header is reconciled with its
  History. The `renode-environment.md` scope note (PA-89-02) and the stale
  gpio/mem counts (PA-89-04) are advisory.
- **N11 (records).** **Met.** `verification-results.md` 0.26, `CHANGELOG.md`, the
  SDD, handover, loop-audit, tool register, traceability mirror and README are
  consistent; the check count is reproducible (36 == source == binary); the delta
  review reports and their log/V&V rows are committed. Residuals: PA-89-05
  (CHANGELOG not updated at HEAD) and PA-89-06 (parent-report bookkeeping), both
  advisory.
- **N14 (SSS systems review).** **Not triggered** by this increment — SSS.md and
  the SSS manifest entries are unchanged; only a new SRS child was added (check 16).
- **N17 (documentation review).** **Met.** The R4 (FRICK-REVIEW-0083) review and
  its delta (FRICK-REVIEW-0088) are committed; the delta's one residual Minor
  (DEL-1) is corrected at HEAD (check 11) and its Comment (DEL-2) is addressed
  (README/handover next-work). No outstanding R4 Minor remains.
- **N20 (PA review).** **Met** (see §7): the prior Major is closed and no Major
  remains in PA scope; one new Minor (PA-89-01) and advisory Comments are tracked.

## 6. Honesty assessment (asked explicitly)

- **No capability over-claim.** The Renode check is scoped as an "MMIO
  presence/identification check" in the code, the `CHANGELOG.md`, the
  `verification-results.md` 0.26 row and the handover; write/erase register logic
  is host-tested and on-target flash is GAP-A5. No record claims write, erase,
  endurance or retention validation on target.
- **The read-count correction is now honest and reproducible**: records quote
  "36 checks" == source == binary; the change narrative uses the correct pre-fix
  baseline "29 → 36".
- **One residual precision defect (PA-89-01):** the *justification* wording
  ("read-only", "write self-test faults") is not corroborated and no failed-run
  log is archived. It under-states the very capability it declines to test, so it
  is an accuracy/retention defect rather than an over-claim — but it is a
  substantive justification and should be corrected.
- **No misstatement found** in the corrected records. The remaining items
  (PA-89-02..06) are omissions/bookkeeping, not false statements.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — the prior Major is closed; residual Minor and Comments tracked to closure
- [ ] REJECTED — one or more Major findings

The increment is substantively sound and the process obligation that caused the
prior rejection is discharged. Independently verified at HEAD `7e00e4c`:

- the **Major F1 is closed** — the full review set (R1/R2/R4/PA and their deltas)
  is committed and logged with the change (checks 5–6); HEAD is pushed to both
  remotes (check 7);
- the delta findings **PA-85-02/03/05 and R1-04/D3, R2 D1/D2, R4 DEL-1/DEL-2 are
  corrected and verified** (checks 8–12);
- the host mock no longer aliases offsets (bounded, direct indexing with an
  in-range scratch region) and the header claim about the Renode app is corrected;
- the records reconcile (handover 1.3 == History 1.3; tool-register wording
  "29 → 36"; design trace 56/56);
- all automated gates converge: `trace` 97/56/27/14/51, `metrics --check` exit 0,
  `ctest` 17/17, `test_nvm` 36 checks, format clean, fresh Renode `hal_nvm=PASS`.

Against that, one **new Minor (PA-89-01)** — the imprecise, unarchived
justification for the read-presence check — and five advisory Comments remain,
none of which invalidates SRS-REQ-055 or defeats a safety barrier considered in
scope. The verdict is therefore **APPROVED WITH COMMENTS**.

**N20 gate for the NVM increment (SRS-REQ-055) — SATISFIED.** The gate is no
longer blocked: there is **no remaining Major** in PA scope, the prior Major is
closed, the mandated review set is committed and logged, and the full cycle
converges at HEAD. The residual **Minor PA-89-01** and the advisory Comments are
tracked for closure and do not invalidate the increment; under the project's
precedent (FRICK-REVIEW-0085) a pass-with-tracked-Minors is a pass. The human
release/PA authority retains the decision to accept the residual Minor.

## 8. Confidence

```text
Decision/product and configuration: frick-fsw @ 7e00e4c; final PA review of the
  SRS-REQ-055 reference eNVM NVM backend + Renode hal_nvm clause; ECSS criticality B target.
Claim type: mixed (Fact for gate results, file contents, repository/remote state and the
  Renode probe; Inference for the process verdict and the PA-89-01 severity judgement).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1
  (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025). Customer tailoring matrix not
  available to this reviewer (unchanged from the PA-67/0084/0085 assessments).
Evidence consulted: HEAD diff and message; ff_nvm_mpfs.{c,h}; tests/unit/test_nvm.c;
  src/apps/app_renode_test.c; config/platform/platform_mpfs.h; SDD, handover (1.3),
  loop-audit, tool-qualification, verification-results (0.26), CHANGELOG, traceability.yaml/.md,
  README; docs/reviews/ (committed 0081-0088); live gates (trace.py, metrics.py --check, ctest,
  test_nvm); git refs/remotes/status; build/renode artifacts; an ephemeral Renode probe of
  config/platform/polarfire-soc.repl.
Cross-check and reconciliation: recorded "36 checks" vs measured 36; handover body 97 vs
  History 1.3; tool-register "29 -> 36"; SDD/README 56/56; Renode console 18:02 vs commit 18:05;
  local HEAD vs origin/gitea refs; the code-comment read-only/faults claim vs the probe result.
Confidence: High for the gate results, the closure dispositions and the Renode probe facts;
  Medium for PA-89-01 severity and for the N20 wording (depends on project policy toward an
  APPROVED-WITH-COMMENTS PA review with one tracked Minor). The probe used the locally
  available image (Renode 1.16.1, digest e278e663...) not the Dockerfile-pinned digest
  (7905c329...), so the read-only/faults finding is stated with that detection limit.
Basis and detection limits: full diff and all in-scope records read; gates executed; test count
  measured; the Renode probe was run on a possibly different Renode build. The committed sibling
  reports were treated as snapshots; only repository evidence at HEAD was assessed.
Missing/inaccessible evidence: customer tailoring matrix; an archived failed-run console log
  for the write self-test; the exact pinned Renode image used for the recorded run; the RTEMS
  cross/on-target results (re-used as recorded, not re-run).
Assumptions and expiry/recheck date: assumes the task instruction to modify only this report is
  authoritative. Recheck on the next corrective commit (PA-89-01) or 2027-04-11.
Consequences if wrong: if PA-89-01 is judged a Comment (e.g. the pinned image genuinely faults on
  write), the verdict becomes APPROVED; the closure dispositions and the N20-gate statement are
  unaffected. Authorised approver: project engineering/PA/release authority; this agent cannot
  approve, close NCs, waive findings, or accept residual risk.
```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) | see FRICK-REVIEW-0086 / 0087 / — / 0088 (deltas) | 2026-10-11 | R1/R2/R4 deltas APPROVED WITH COMMENTS (residuals now dispositioned at HEAD); N14 not triggered |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0089** | **2026-10-11** | **APPROVED WITH COMMENTS (1 Minor PA-89-01 + advisory Comments; N20 gate SATISFIED)** |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-84-01 | Major | N15/N9/N17/N20/N11/N7 — review set | Increment pushed without committed review evidence | closed in `03962d6` | review set committed + logged (checks 5–6) | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| PA-84-02..05 | Minor | records/SDD/handover/T17 | 14-checks; SDD matrix; handover list; T17 row | closed | checks 4, 11–13 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| PA-84-06 / F6 | Minor | Renode oracle | non-discriminating presence oracle | closed-by-justification (read-presence + header fix) | checks 9–10; PA-89-01 residual | frick-fsw | **closed (residual PA-89-01)** | FRICK-REVIEW-0089 |
| PA-84-07/08 | Comment | loop-audit PUS-C; tool-register header | stale | closed | tool register; loop-audit | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| PA-84-09 / F9 | Comment | tool register gpio/mem counts | stale 21/20 | not corrected | see PA-89-04 | frick-fsw | **open (advisory)** | FRICK-REVIEW-0089 |
| PA-85-02 | Minor | sibling closure / delta reports | deltas untracked | deltas committed + logged; R4 DEL-1 corrected | checks 5–6, 11 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| PA-85-03 | Comment | host mock aliasing | aliases scratch to slot 0 | mock enlarged + bounds guard | check 8 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| PA-85-05 | Comment | fresh Renode run | none for `03962d6` | fresh run at HEAD | check 14 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| R1-04 / 0086 D-02 | Minor | `test_nvm.c` mock | masks OOB / self-test aliased | fixed | check 8 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| 0087 D1 | Minor | `ff_nvm_mpfs.h` claim | claims a Renode caller | corrected | check 9 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| 0087 D2 | Comment | history narrative | "14 -> 36" wrong baseline | corrected to "29 -> 36" | check 12 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| 0088 DEL-1 | Minor | handover History | counts without History row/bump | Revision 1.3 + History row | check 11 | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| 0088 DEL-2 | Comment | README/handover next-work | NVM framed as upcoming | NVM dropped from "next" | check 11; `README.md:408` | frick-fsw | **closed** | FRICK-REVIEW-0089 |
| **PA-89-01** | **Minor** | `app_renode_test.c`; commit msg; N8/N10/N11/N18 | read-only/faults justification inexact; region is `envmCfg` not storage; no archived failed-run log | correct wording; record region; archive pinned-image evidence | probe check 15; re-review of app/comment | frick-fsw | **open** | pending |
| PA-89-02 | Comment | `docs/vv/renode-environment.md` | presence-check scope not documented | add scope paragraph | doc re-read | frick-fsw | **open (advisory)** | pending |
| PA-89-03 | Comment | CI provenance | `SRS-REQ-055` omitted | add to both workflows | `ci.yml` re-read | frick-fsw | **open (advisory)** | pending |
| PA-89-04 | Comment | tool register counts | gpio/mem stale | correct when next touched | register re-read | frick-fsw | **open (advisory)** | pending |
| PA-89-05 | Comment | `CHANGELOG.md` | not updated at HEAD | add close-out bullet | CHANGELOG re-read | frick-fsw | **open (advisory)** | pending |
| PA-89-06 | Comment | parent R1/R2/R4 reports | NC tables left all-open | annotate closures | parent report re-read | frick-fsw | **open (advisory)** | pending |

---

This report is the norm-N20 final review of the SRS-REQ-055 increment at
`7e00e4c`. It modifies no repository file other than itself. The
`docs/reviews/README.md` review-log row and the `docs/vv/verification-results.md`
cycle-log row for FRICK-REVIEW-0089 are to be added by the integrator, and the
file is to be committed and pushed with the increment (norm N11/N7).