# Review Report — Final Product Assurance review (N20/N12) of the reference NOR/QSPI MTD backend increment (SRS-REQ-056)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0098                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `d4c3685` ("MTD: close delta findings (N12) - mirror/records/comments + extra tests"), branch `main`, pushed to `origin` and `gitea` |
| Items under review                | Closure of FRICK-REVIEW-0094 (Major F1 + F2–F5) and FRICK-REVIEW-0095 (F4, DF1, DF2, DF4) at HEAD; the sibling delta findings FRICK-REVIEW-0096 (Findings 1–3 + Comment 4) and FRICK-REVIEW-0097 (R4-97-01..11); products `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`, `tests/unit/test_mtd.c`, `src/apps/app_renode_test.c`, `config/platform/platform_mpfs.h`, requirement SRS-REQ-056; records `docs/requirements/traceability.yaml` + `.md`, `docs/vv/verification-results.md` (0.27), `docs/vv/tool-qualification.md` (0.3.11), `docs/vv/renode-environment.md`, `docs/status/{handover.md,loop-audit.md}`, `docs/development-plan.md` (0.16), `CHANGELOG.md`, `README.md`, `docs/reviews/README.md`, `docs/planning/sprint-plan.md`; the on-SoC QSPI M11 tracking |
| Date                              | 2026-10-11                             |
| Scope                             | Whether the SRS-REQ-056 increment's **N20 gate is satisfied** at HEAD `d4c3685` and whether the increment **loop can close under N12**: closure of the prior Major and Minors, honest scoping of the QSPI evidence (no vacuous Renode check, host-only evidence), mirror/manifest convergence, record reconciliation (Revision+History) and M11 tracking of the on-SoC write/erase gap. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean before and after). Gates were executed read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the **final norm-N20 Product Assurance review** of the milestone M4/M11
increment that adds the reference NOR/QSPI memory-technology-device backend
behind the HAL contract `FF_Mtd_t` (SRS-REQ-056), as delivered and **pushed** at
`d4c3685`. It closes the chain opened by FRICK-REVIEW-0094 (PA, **REJECTED** with
Major F1 + Minors F2–F5) → FRICK-REVIEW-0095 (PA delta, APPROVED WITH COMMENTS) →
FRICK-REVIEW-0096 (R1 delta) → FRICK-REVIEW-0097 (R4 delta).

**Included:** closure status of the prior Major/Minor NCs; whether the vacuous
`hal_mtd` Renode check is fully retired from code, manifest **and every record**;
whether the QSPI/NOR evidence is honestly scoped as host-only; whether the
`.yaml`↔`.md` traceability mirror is consistent; whether the
handover/README/development-plan/loop-audit/tool-qualification
`Revision`/`History` are reconciled; whether the on-SoC QSPI write/erase gap is
tracked as an M11 task; the product gates; and whether the N20 gate is satisfied
and the increment loop can close under N12.

**Excluded:** re-derivation of the internal driver/test correctness (R1/R2
scope); the Lean proofs; documentation adjudication owned by R4; and
on-target/silicon NOR behaviour, which is **GAP-A5** (`docs/quality/class-a-gaps.md
§GAP-A5`, blocked on hardware) by the increment's own scope. This review does not
re-perform the R1/R2/R3/R4 reviews.

**Method.** Read `git show d4c3685` and every touched artifact; diffed each
claimed fix against the actual content; grepped the tree for residual
`hal_mtd`/`hal_nor`/`QSPI`/`FF_QSPI0_BASE` tokens **and prose phrases** (per
FRICK-REVIEW-0096 Finding 1); re-ran `trace.py`, `metrics.py --check` and
`ctest`; measured the `test_mtd` assertion count; inspected the Renode console
log; and checked the repository/remote state.

## 2. References

- Predecessors: FRICK-REVIEW-0094 (`docs/reviews/2026-10-11-mtd-pa-review.md`),
  FRICK-REVIEW-0095 (`...-mtd-pa-delta-review.md`), FRICK-REVIEW-0096
  (`...-mtd-r1-delta-review.md`), FRICK-REVIEW-0097 (`...-mtd-r4-delta-review.md`).
- Project norms: `docs/rules.md` (N1–N20), especially N4, N8, N11, N12, N15, N16,
  N17, N18, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056, line
  525–532); human mirror `docs/requirements/traceability.md`.
- HAL contract: `src/hal/hal_mem.h` (`FF_Mtd_t`, SRS-REQ-032).
- Renode: `tools/docker/renode/frick-renode.resc` (loads the SoC-only
  `@platforms/cpus/polarfire-soc.repl`); board repl maps `SPI.MPFS_QSPI
  @0x21000000` + `SPI.Micron_MT25Q` (not loaded by this test).
- Records: `docs/vv/verification-results.md` (0.27), `docs/vv/tool-qualification.md`,
  `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/development-plan.md`,
  `CHANGELOG.md`, `README.md`, `docs/reviews/README.md`, `docs/planning/sprint-plan.md`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

### 3.1 Product gates re-run at `d4c3685`

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** (incl. `mtd`) |
| MTD suite check count | `./build/native/tests/test_mtd` | **`checks=4128 failures=0`** — matches the commit's 4123→4128 claim |
| Renode evidence (N18) | `build/renode/console.log` (mtime 2026-10-11 19:25) | `RENODE_TEST: PASS`; clauses `hal_gpio/serial/mem/eth/net/nvm=PASS`; **no `hal_mtd`** (clause removed) |
| Commit/remote state (N7) | `git branch -avv`, `git status` | HEAD `d4c3685` **=** `origin/main` **=** `gitea/main`; tracked tree clean |

### 3.2 Confirmations (no finding)

- **Major F1 (0094) CLOSED.** The mandated review set (R1/R2/R4/PA, ids 0091–0094)
  and its log/V&V rows were committed with the change in `cb2576d`; the N12 delta
  reviews 0095/0096/0097 are committed in `d4c3685`. Reviewed with the change
  (N11/N15).
- **Vacuous Renode check fully retired (code + primary records).**
  `app_renode_test.c` contains no `hal_mtd`/QSPI/NOR clause (only the generic
  file-header word "Renode"); the console log emits no `hal_mtd`; the marker is
  `hal_gpio/serial/mem/eth/net/nvm=PASS`. The `traceability.yaml` `lean_note` (532)
  and the `.md` mirror (117) no longer claim `ci:renode`; `verification-results.md`
  Remaining (M4) (277) drops QSPI and History 0.27 (410) no longer says
  `hal_mtd=PASS`. `ff_mtd_mpfs.h` and `test_mtd.c` header comments now say
  host-tested + M11 pending. **Exception: `config/platform/platform_mpfs.h`
  (Finding 1).**
- **Revision/History reconciled** across tool-qualification (0.3.11/0.3.11),
  loop-audit (0.7/0.7), development-plan (0.16/0.16), handover (1.4/1.4),
  traceability.md (0.26/0.26) and verification-results (0.27/0.27); README design
  traceability 57/57 (100%); renode-environment 5/13; CHANGELOG single `### Added`.
- **On-SoC QSPI gap tracked as M11.** `loop-audit.md:147` dispositions row and
  `sprint-plan.md:42` S1 exit criterion both track "on-SoC NOR check pending board
  repl (`MPFS_QSPI`+`Micron_MT25Q`)"; `development-plan.md` 0.16 records the task.
- **New boundary tests** present in `test_mtd.c` (`write(0,val,0)`,
  `read(MOCK_MTD_SIZE+1,…)`, `erase(MOCK_MTD_SIZE,4096)`, `Setup(…,MAX+4096,…)`)
  — closes R2-F1 (erase `FF_BAD_SIZE` path) and largely closes R1-MTD-04.

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | **Minor** | `config/platform/platform_mpfs.h:50-53`; SRS-REQ-056; N8/N11/N16 | **The stale over-claim survives in the last remaining MTD artifact, and the commit message asserts a change that was not made.** `platform_mpfs.h:50-52` still reads "reference MTD driver; **exercised on the emulated SoC in the Renode functional test (M11)**" — the exact retired claim (FRICK-REVIEW-0096 Finding 1, third of three artifacts). The file is still an `implementation` artifact of SRS-REQ-056 in the authoritative manifest (`traceability.yaml:529`). Moreover the `d4c3685` commit message states "platform_mpfs.h: **drop the now-unused `FF_QSPI0_BASE`**", but the file was **not modified** in that commit and `FF_QSPI0_BASE` remains defined (line 53) — a false statement in the revision record. | `config/platform/platform_mpfs.h:50-53`; `git show --stat d4c3685` (file absent); `traceability.yaml:529`; grep `exercised on the emulated` → only this file + true GPIO/SERIAL/NVM comments | Reword `platform_mpfs.h:50-52` to "reference MTD driver; host-tested (mock MMIO); on-SoC QSPI check pending board repl — M11"; remove or annotate `FF_QSPI0_BASE` for the M11 path; correct the record with a follow-up commit (N16). |
| 2 | **Minor** | `docs/requirements/traceability.md:117` vs `traceability.yaml:529`; N4/N8 | **The human mirror no longer mirrors the manifest for SRS-REQ-056.** The `.md` edit dropped `config/platform/platform_mpfs.h` from the SRS-REQ-056 implementation list, but the authoritative `.yaml` **keeps it**. The document states it "mirrors its *current* content"; `trace.py` (and the CI docs gate) check only the `.yaml`, so the divergence passes uncaught. This is a new inconsistency introduced by the corrective commit, in the same reflection class the delta reviews targeted. | `traceability.md:116-117`; `traceability.yaml:529`; `trace.py` (single-source-of-truth = yaml) | Regenerate/reconcile the mirror so SRS-REQ-056 lists the same implementation files as the `.yaml` (either restore `platform_mpfs.h` or drop it from both, per the M11 decision). |
| 3 | **Minor** | `docs/reviews/README.md`; N11/N17 | **The N12 delta reviews are not logged.** The commit `d4c3685` adds the delta reports `2026-10-11-mtd-{pa,r1,r4}-delta-review.md` (ids 0095/0096/0097) but does **not** append rows to the review log, whose last row is the 0094 review (`:95`). The project convention logs delta reviews (see the NVM rows 85–91) and the log's own instructions require "append a row to the log … together with the change". | `docs/reviews/README.md:95`; `git show --stat d4c3685` (no `README.md`); NVM precedent rows 85–91 | Append review-log rows for 0095/0096/0097 and for this final review 0098. |
| 4 | **Minor** | `docs/status/handover.md:130,132`; N8 | **Carried from 0094-F4 / 0095-F4, not closed: the §5 bullet list still enumerates CAN twice.** Line 130 ("**CAN** …, then I2C/SPI/BLOCK/PCIe/SpaceWire; [NVM + NOR/QSPI MTD driver done…]") and line 132 ("**CAN** …, **SPI**, **I2C** …") both start with CAN and give inconsistent follow-on ordering. The line-126 "next classes: NOR MTD" and the §4 "(14/14)"/5/13 items are now corrected (18/18, 5/13 defensible), but the duplicated bullet persists, so the handover is not self-consistent. | `handover.md:130,132`; `git show d4c3685 -- docs/status/handover.md` | De-duplicate the §5 bullets (one ordered enumeration) so the resume point is self-consistent. |
| C1 | Comment | `docs/requirements/traceability.md:20-23`; N8 | **R4-97-09 not corrected.** The prose still says "Six of the requirements (SRS-REQ-045/046/051/052/053/054 …) are … documented-only", but SRS-REQ-052 carries `tests/unit/test_r3_gap.c` and a non-doc implementation (`src/pus/pus.h`), so it is not documented-only. | `traceability.md:20-23`; `traceability.yaml:489-495` | Reword to "five" / exclude SRS-REQ-052. |
| C2 | Comment | `docs/vv/verification-results.md:7,410`; N16 | The History **0.27 row was edited in place** (correctly reworded to host-only) but the document `Revision` stayed `0.27` and **no new History row** records the `d4c3685` correction, as FRICK-REVIEW-0095 DF2 recommended. Acceptable as an in-cycle correction; a History row would make the N16 trail explicit. | `verification-results.md:7,410`; `git show d4c3685 -- docs/vv/verification-results.md` | Optionally add a History row for the correction. |
| C3 | Comment | `tests/unit/test_mtd.c:66-76,86-89`; R1-MTD-04 | Boundary coverage is substantially improved and the erase `FF_BAD_SIZE` path is now exercised, but the strict `off > size` disjunct for **write/erase** (`size+1`) remains unexercised, and the "second sector untouched" comment (`:86`) is still not substantiated (the sector-1 write occurs *after* the sector-0 erase). | `test_mtd.c:66-76,86-89` | Add `write/erase(size+1,…)` cases and a pre-erase sentinel (or reword the comment). |
| C4 | Comment | N14 trigger | The increment changed only the **SRS** child set (SRS-REQ-056 added under SSS-REQ-020); `docs/requirements/SSS.md` is unchanged, so the R3/N14 SSS review is **not triggered** — consistent with the NVM precedent (FRICK-REVIEW-0089, check 16). Recorded here for completeness. | `git diff --stat a91e51f d4c3685 -- docs/requirements/SSS.md` (empty) | Document the N14 determination alongside the increment. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive observations (no finding).** The **disqualifying Major is closed**: the
review set is committed with the change; the vacuous on-SoC check is **removed**
from the code and the Renode marker (not reworded); and the requirement is
honestly scoped as **host-verified** with the on-SoC QSPI write/erase check
**tracked as an M11 task**. All gates converge at `d4c3685`: trace
98/57/27/14/51, metrics current, ctest 18/18, `test_mtd` 4128 checks, Renode
PASS, tree clean and pushed to both remotes. F2/F3/F5 (0094), DF1/DF2 (0095),
0096 Findings 2–3, and R4-97-01/-02/-03/-05/-06/-08/-10 are corrected.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**FRICK-REVIEW-0094 status: the Major F1 remains CLOSED**; F2/F3/F5 CLOSED; **F4
still NOT fully closed** (the duplicate CAN bullet persists — Finding 4).
**FRICK-REVIEW-0095:** DF1 (mirror `ci:renode`) and DF2 (History 0.27) CLOSED;
**DF3** (`FF_QSPI0_BASE` / stale `platform_mpfs.h` comment) **NOT closed**
(Finding 1); **DF4** (sibling NCs) mostly closed, with Finding 1 (0096) and the
R4-delta residuals in C1/C3 outstanding. **FRICK-REVIEW-0096:** Findings 2–3
closed; Finding 1 **partly open** (Finding 1); **FRICK-REVIEW-0097:**
R4-97-01/-02/-03/-05/-06/-08/-10 closed, R4-97-09 open (C1).

**N20 gate for the MTD increment (SRS-REQ-056) — SATISFIED.** The gate is no
longer blocked: there is **no remaining Major** in PA scope, the prior Major is
closed, the mandated independent review set (R1/R2/R4 + PA, plus the N12 deltas)
is committed with the change, and the full cycle converges at HEAD. Per the
project precedent (FRICK-REVIEW-0085/0089), a pass-with-tracked-Minors is a pass.
Four **Minor** NCs (F1–F4) and four advisory Comments are tracked for closure and
do not invalidate the requirement.

**Increment loop closure under N12 — NOT YET SATISFIED.** N12 requires every NC
to be corrected, re-verified and delta-reviewed to full compliance. The open
Minors **F1** (stale `platform_mpfs.h` over-claim + `FF_QSPI0_BASE` + the
inaccurate commit record), **F2** (`.yaml`↔`.md` divergence), **F3** (missing
delta review-log rows) and **F4** (handover duplicate CAN) must be corrected and
delta-verified before the increment loop can close; C1–C4 are advisory. The human
PA/release authority retains the decision to accept the residual Minors and to
close the loop.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0098 | 2026-10-11 | **APPROVED WITH COMMENTS (4 Minor + 4 Comments; N20 gate SATISFIED; N12 loop not yet closable)** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: reference NOR/QSPI MTD backend increment (SRS-REQ-056) at d4c3685, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on gates/records/closure; Inference on the N20-gate and N12-loop status; Recommendation for F1-F4/C1-C4).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (2025), ECSS-Q-ST-80C Rev.2 (2025). Customer tailoring matrix not available to this reviewer.
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at d4c3685 (all files in the header table); git show/log/branch; gates re-run 2026-10-11 read-only; build/renode/console.log (19:25).
Cross-check and reconciliation: recorded numbers (98/57/27/14/51; 18/18; 4128 checks; Renode PASS) independently reproduced; the yaml/md mirror and the platform_mpfs.h stale claim found by token+prose grep; the commit-message platform_mpfs.h claim contradicted by the empty file diff.
Confidence: High on F1-F4 and C1-C4 (direct, configuration-matched evidence); High on the gate results; Medium on the N20/N12 wording (depends on project policy toward a pass-with-tracked-Minors review).
Basis and detection limits: read-only inspection + gate re-runs; internal driver/test correctness (R1/R2 scope) not independently re-derived; the Renode run was inspected via its console log, not fully re-executed; the CI workflow claim-name checks were not run.
Missing/inaccessible evidence (including paywalls): customer tailoring matrix; ECSS clause texts not needed beyond the invoked norms; the exact pinned Renode image digest was not re-verified.
Assumptions and expiry/recheck date: sibling reviews treated as snapshots; recheck on the next corrective commit or 2027-04-11.
Consequences if wrong: if F1-F4 are corrected and delta-verified, the verdict becomes APPROVED and the N12 loop closes; the gate facts are unaffected.
Required independent human reviewer/authority: the project integrator/PA/release authority decides closure and any residual-risk acceptance; this review is not a certification or approval.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0094-F1 | Major | N15/N17/N20/N11 | Review set not committed with the change | Closed: R1/R2/R4/PA (0091-0094) + deltas committed | `git show --stat cb2576d`/`d4c3685`; `docs/reviews/*mtd*` | frick-fsw | **closed** | 0095/0098 |
| 0094-F2 / F3 / F5; 0095-DF1 / DF2 | Minor | manifest/SDD/sprint-plan/mirror/History | QSPI over-claim in records | Closed | `traceability.yaml:530-532`; `traceability.md:117`; `verification-results.md:277,410` | frick-fsw | **closed** | 0095/0098 |
| 0094-F4 / 0095-F4 | Minor | `handover.md:126,130,132` | stale "next classes"; duplicated CAN bullet | Line 126 + §4 corrected; **duplicate CAN remains** | `handover.md:130,132` | frick-fsw | **open** (F4) | 0098 |
| 0095-DF3 / 0096-01 | Minor | `config/platform/platform_mpfs.h:50-53` | stale "exercised on the emulated SoC" claim; `FF_QSPI0_BASE` retained; commit-message overstates the change | Correct + record | updated header/commit | frick-fsw | **open** (F1) | 0098 |
| 0098-F2 | Minor | `traceability.md:117` vs `traceability.yaml:529` | mirror dropped `platform_mpfs.h` for SRS-REQ-056 | Reconcile yaml/md | reconciled mirror; yaml==md | frick-fsw | **open** | pending |
| 0098-F3 | Minor | `docs/reviews/README.md` | delta reviews 0095/0096/0097 (and 0098) not logged | Append rows | updated review log | frick-fsw | **open** | pending |
| 0098-C1..C4 | Comment | traceability prose / History row / test bounds / N14 | advisory | Correct as convenient | — | frick-fsw | **open (advisory)** | pending |

---

Prepared as the final PA review (FRICK-REVIEW-0098). Per the task instruction
only this report file was created; the review-log row (`docs/reviews/README.md`)
and any `docs/vv/verification-results.md` review-record row for FRICK-REVIEW-0098
remain the integrator's obligation (norms N11/N17/N20).