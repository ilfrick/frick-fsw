# Review Report — PA delta re-review of the Sprint-3 (M4/M11 MSS I2C) increment after the initial PA findings (norm N20/N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | **FRICK-REVIEW-0136**                  |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | **PA** (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | base commit `db8261f` + **uncommitted** Sprint-3 working tree (**fixed revision**, 2026-10-12); HEAD `db8261f` |
| Items under review                | The closure of PA-01…PA-07 (`docs/reviews/2026-10-12-m4-m11-i2c-pa-review.md`, FRICK-REVIEW-0133) and the records/tool-qualification/documentation deltas of the Sprint-3 MSS I2C increment: `docs/reviews/` (the full review set + `README.md` log), `docs/status/status.json`, `docs/status/loop-audit.md`, `docs/status/handover.md`, `docs/planning/sprint-plan.md`, `docs/requirements/{SRS.md,traceability.md,traceability.yaml}`, `docs/design/software-design.md`, `docs/quality/class-a-gaps.md` (GAP-A5), `docs/interfaces/ICD.md`, `docs/engineering/{software-description.md,user-integration-manual.md,driver-development.md}`, `docs/architecture/network-stack.md`, `docs/vv/{verification-results.md,tool-qualification.md,renode-environment.md}`, `CHANGELOG.md`, `README.md`, `AGENTS.md`, both CI workflows, `Makefile`, `tools/scripts/misra.sh`, `tests/unit/test_i2c.c` |
| Date                              | 2026-10-12                             |
| Scope                             | Delta re-review (norm N12) of the fixed increment after the initial PA review FRICK-REVIEW-0133: verify closure of the Major PA-01 (review-set closure/log gate + semaphores) and the Minors PA-02…PA-06 and Comment PA-07, independently re-run the gates, and check records/tool-qualification/claim-discipline/residual-gap/CM-baseline integrity; against ECSS-E-ST-40C Rev.1 and ECSS-Q-ST-80C Rev.2 and the norms N1–N20 (`docs/rules.md`). |
| Independence declaration          | I authored no part of this increment, ran no gate as its author, hold no decision authority over it, and did not review work in which I participated. This review ran in a fresh context; every result below was re-executed/re-derived first-hand from the repository. **Read-only for the repository: I modified no tracked file; the only file I wrote is this report** (`build/` is gitignored, so the gate re-runs regenerated only ignored build artifacts). Nothing was committed or pushed. |
| Norms applied                     | N1–N20 (`docs/rules.md` 1.4); focus N4, N5, N6, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the **PA delta re-review (norm N20/N12)** of the Sprint-3 (M4/M11 MSS I2C,
SRS-REQ-058) increment after the initial PA review `FRICK-REVIEW-0133` raised the
Major **PA-01** (the review-set closure/log gate and premature review semaphores)
plus the Minors PA-02…PA-06 and the Comment PA-07.

**In scope (verified first-hand at the fixed revision):**

- The **completeness and logging** of the increment's independent review set
  (R1/R2/R3/R4 + deltas + PA) and the `docs/reviews/README.md` log (PA-01).
- The **truthfulness of `docs/status/status.json`** (gates, note, milestones,
  sprints) at the frozen revision (PA-01).
- **Revision-history integrity** (header `Revision`/`Date` vs the last `## History`
  row) for every touched engineering document (PA-02).
- The `docs/status/loop-audit.md` **baseline** (PA-03).
- The **144-check figure** and the **1 MHz** divider case (PA-04).
- The **T14/MISRA reproducibility** path (`make misra-docker`, `tools/scripts/misra.sh`)
  and the T14 tool-register row (PA-05).
- The **documentation-set registration** of `software-description.md` (PA-06).
- The **`RR-058` token** (PA-07).
- Independent **re-execution** of the applicable gates (native/ASan ctest, `trace.py`,
  `metrics.py --check`, `make misra-docker`, `lake build`, `make smoke`, `make renode`,
  `make qemu`, `clang-format`).
- **New PA findings** (records, qualification, claim discipline, residual-gap
  register, CM/baseline integrity) and the residual closure of the R3/R4/R2-delta
  new findings (R3-09…R3-14, R4-N1…N8, N-R1-D1…D5, N-R2-D1…D5) that bear on the
  pre-commit state.

**Excluded (other profiles' remits):** the internal correctness adjudication of the
driver (R1) and of the V&V method/oracle/coverage (R2) except where a PA-level
records/qualification defect is in scope; the systems/requirements adjudication (R3);
the documentation adjudication (R4); the Lean model (N1 — no Lean artifact touched;
SRS-REQ-058 carries an explicit `lean_note`); on-target silicon behaviour (GAP-A5).

**Assumption recorded:** the increment is an **uncommitted** working tree on
`db8261f`, so no commit hash identifies the reviewed state; evidence is tied to
file content at the time of review (norm N9 requires a frozen revision — see
N-R2-D5 and §5 PA-12).

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract
  `docs/engineering/independent-review.md` (rev 0.7); template
  `docs/reviews/review-template.md`.
- Prior PA review: `docs/reviews/2026-10-12-m4-m11-i2c-pa-review.md`
  (FRICK-REVIEW-0133). Precedents: FRICK-REVIEW-0109 (PA-01 review-set gate),
  FRICK-REVIEW-0123 (M10 PA-01).
- Review set under closure: R2 `FRICK-REVIEW-0129`, R3 `FRICK-REVIEW-0130`,
  R1 `FRICK-REVIEW-0131`, R4 `FRICK-REVIEW-0132`, PA `FRICK-REVIEW-0133`,
  R1-δ `FRICK-REVIEW-0134`, R2-δ `FRICK-REVIEW-0135`; this report `0136`.
- Requirements/records: `docs/requirements/SRS.md` (SRS-REQ-058, rev 1.17),
  `docs/requirements/traceability.yaml` (`:543-550`), `docs/requirements/traceability.md`
  (0.29), `docs/quality/class-a-gaps.md` (GAP-A5, 0.7), `docs/vv/{verification-results.md
  (0.34),tool-qualification.md (0.3.18),renode-environment.md (0.14)}`, `CHANGELOG.md`,
  `docs/status/{status.json,loop-audit.md (0.12),handover.md (1.8)}`,
  `docs/planning/sprint-plan.md` (0.5).
- ECSS-E-ST-40C Rev.1; ECSS-Q-ST-80C Rev.2.

## 3. Verification performed (independent), with results

| # | Gate / check (norm) | Exact command or artifact | Result (independently reproduced 2026-10-12) |
| - | ------------------- | ------------------------- | --------------------------------- |
| V1 | Review set present (N9/N14/N17/N20) | `ls docs/reviews/ \| grep m4-m11-i2c` | **7 reports** for the increment: R2 0129, R3 0130, R1 0131, R4 0132, PA 0133, R1-δ 0134, R2-δ 0135 (+ this PA-δ 0136) |
| V2 | Review set logged (N11) | `docs/reviews/README.md` rows 129–136 | **all 7 (+pending 0136) rows present**, incl. R2/R3/R1/R4/PA + R1-δ/R2-δ |
| V3 | Review conclusions (N12) | report headers/conclusions | R1-δ/R2-δ/R3/R4 = **APPROVED WITH COMMENTS** with prior findings closed; PA 0133 = REJECTED (Major PA-01) |
| V4 | Semaphores/note (N8/N19) | `docs/status/status.json` | gates all **green**; `meta.note` says the review set is **"in progress"** (residual PA-12) |
| V5 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — `100 requirements, 59 srs, 27 sss, 14 gaps, 51 lean theorems checked` |
| V6 | Metrics panel currency (N8/T13) | `python3 tools/scripts/metrics.py --check` | **FAIL — exit 1**: `README metrics panel is stale` (see PA-08) |
| V7 | Unit suite, native (N6) | `ctest --test-dir build/native` | **20/20 PASS** |
| V8 | Unit suite, ASan/UBSan (N6) | `ctest --test-dir build/asan` | **20/20 PASS** |
| V9 | `test_i2c` checks (N11) | `./build/native/tests/test_i2c` | **`checks=144 failures=0`** |
| V10 | Lean model (N1) | `docker run … frick-vv:local lake build` | **Build completed successfully** |
| V11 | Formatting (N6) | `clang-format --dry-run --Werror` (changed files + full `src config tests`) | **exit 0** (clean) |
| V12 | Simulator smoke (N6) | `make smoke` | **PASS** (banner + 6 applications registered) |
| V13 | Renode functional run (N18) | `make renode` | **REAL PASS** — `RENODE_TEST: PASS`, console `hal_i2c=PASS` (7 apps) |
| V14 | QEMU emulated target (M10) | `make qemu` | **PASS** (banner + 6 apps; toolchain image `90f81d080611`) |
| V15 | MISRA/T14 reproducibility (N10) | `make misra-docker` | **13 advisory findings, 0 in the I2C files**; report regenerated |
| V16 | Header ↔ History (N16) | header `Revision`/`Date` vs last `## History` row, 16 docs | **all 16 match** (PA-02 closed) |
| V17 | Loop-audit baseline (N19) | `docs/status/loop-audit.md:9` | `db8261f + Sprint-3 MSS I2C increment (audited revision)` (PA-03 closed) |
| V18 | Doc-set registration (N8) | `grep software-description` | README index `:371`, `AGENTS.md:36`, `.github/workflows/ci.yml:171`, `.gitea/workflows/ci.yml:212` (PA-06 closed) |
| V19 | `RR-058` token (N8) | `grep -rn "RR-0" docs/` | live S3 row now reads "SRS-REQ-058 review record closed …" (`sprint-plan.md:44`); no live `RR-` token remains (PA-07 closed) |
| V20 | 1 MHz divider case (N11) | `grep -noE "init\([0-9]+u?\)" tests/unit/test_i2c.c` | `init(1000000u)` present → `div_code 5` (`test_i2c.c:280-281`); `init(104167u)`/`init(104166u)` boundary present (PA-04 closed) |
| V21 | Cross-record count (N8/N16) | `grep "135 checks\|144 checks"` | live records (status.json/handover/sprint-plan/CHANGELOG/verification-results/tool-qualification) all say **144**; the only "135" remainders are historical History snapshots (0.33) and the "135 → 144" transition notes (correct) |
| V22 | Review-id range (N8/N16) | `grep "FRICK-REVIEW-0129"` | CHANGELOG:91, verification-results:435, tool-qualification:127 all cite **`0129..0136`** (consistent) |
| V23 | SRS-REQ-058 completeness (N3/N4) | `SRS.md:304-321`; `traceability.yaml:543-550` | single bound instance, error mapping, minimum SCL, non-re-entrancy, 7-bit-only, `lean_note` — all present |
| V24 | Residual-gap register (N5) | `class-a-gaps.md` GAP-A5 | I2C on-target residual recorded; History 0.7 cites **0131 (R1-02) / 0130 (R3-08)** correctly (R3-14 fixed) |
| V25 | R3/R4 new findings | R3-09…R3-14; R4-N1…N8 | R3-09 (`traceability.md` SRS=59), R4-N1 (SDD `Date`), R4-N2 (README I2C row), R4-N3 (range+log), R4-N4 (CHANGELOG versions), R4-N5 (loop-audit "100 reqs"), R4-N6 (802.1AS), R4-N7 (ADR range), R3-13 (PLIC qualifier), R3-14 (cross-ref) — **all corrected**; residual R4-N8 = PA-12 |
| V26 | CM / baseline integrity | `git status --porcelain` | 29 modified tracked + untracked (`software-description.md`, 8 review reports, `src/drivers/i2c/`, `tests/unit/test_i2c.c`) — a single coherent increment |

**Summary:** every declared gate re-executes green **except** `metrics.py --check`
(V6), which fails because the README metrics panel was not regenerated after the
Sprint-3 review-log rows were appended (new finding **PA-08**). Both CI workflows
run `metrics.py --check` (`.github/workflows/ci.yml:61`, `.gitea/workflows/ci.yml:64`),
so the frozen increment **would fail the CI `docs` job** as it stands.

## 4. Prior-findings closure (PA-01…PA-07)

| NC | Severity | Item / Requirement | Disposition verified first-hand | Status |
| -- | -------- | ------------------ | ------------------------------- | ------ |
| **PA-01** | **Major** | Review set (N15/N9/N17/N20/N11) | **The complete review set now exists and is logged.** `docs/reviews/` holds R2 0129, R3 0130 (consolidated), R1 0131, R4 0132 (consolidated), PA 0133, R1-δ 0134, R2-δ 0135; the `README.md` log rows 129–136 are present; R1/R2/R3/R4/deltas conclude APPROVED WITH COMMENTS with their findings closed. The core of PA-01 (produce + log the set before commit) is **satisfied**. **Residuals:** the README metrics panel was not regenerated after the log rows were added (**PA-08**, Minor) and the `status.json` note still says "in progress" while the review gates are green (**PA-12**, Comment) — the lead's stated just-before-commit reconciliation; both must close before commit/push. | **CLOSED** (residuals PA-08/PA-12) |
| **PA-02** | Minor | Revision histories (N16) | All **16** touched engineering docs have header `Revision`/`Date` equal to their last `## History` row: SRS 1.17, traceability 0.29, software-design 0.6, loop-audit 0.12, sprint-plan 0.5, class-a-gaps 0.7, ICD 0.4, driver-development 0.3, user-integration-manual 0.4, software-description 0.2, network-stack 0.3, handover 1.8, tool-qualification 0.3.18, verification-results 0.34, renode-environment 0.14, development-plan 0.19 (V16). `software-design.md` `Date` corrected to 2026-10-12 (R4-N1). | **CLOSED** |
| **PA-03** | Minor | `loop-audit.md` baseline (N19/N16) | `loop-audit.md:9` now reads `Baseline commit = db8261f + Sprint-3 MSS I2C increment (audited revision)` (V17). | **CLOSED** |
| **PA-04** | Minor | Divider-coverage claim (N8/N11) | A genuine `init(1000000u)` → `div_code 5` case exists (`test_i2c.c:280-281`); `test_i2c` executes **144 checks / 0 failures** (V9/V20); CHANGELOG/verification-results/tool-qualification/status/handover/sprint-plan reconciled to 144 (V21). | **CLOSED** |
| **PA-05** | Minor | T14 MISRA evidence (N10/N15/N6/N8) | A reproducible `make misra-docker` target (`Makefile:53-55`, pinned `ubuntu:24.04` + apt `cppcheck`, runs `tools/scripts/misra.sh`) reproduces the scan: **13 advisory findings, 0 in the I2C files** (V15); `misra.sh` is now **fail-closed** (`exit 2` when the checker is absent, `git diff` confirms); T14 row (`tool-qualification.md:38`) points to `make misra-docker`. **Residual:** checker revision unpinned / image stale / script-comment inaccurate (**PA-09/PA-10**, Comments). | **CLOSED** (residuals PA-09/PA-10) |
| **PA-06** | Minor | Documentation-set registration (N8/N16) | `software-description.md` registered in the README "Engineering guides" index (`:371`), in `AGENTS.md` (`:36`), and in **both** CI `docs` required-files lists (`.github:171`, `.gitea:212`); a CHANGELOG "Engineering documentation set" bullet exists (V18). | **CLOSED** |
| **PA-07** | Comment | `sprint-plan.md` terminology (N8) | The live S3 exit-criteria cell now reads "SRS-REQ-058 review record closed on the eNVM/MTD/CAN precedent" (`sprint-plan.md:44`); no live `RR-` token remains (V19). | **CLOSED** |

**Result:** **PA-01 (Major) and PA-02…PA-06 (Minor) and PA-07 (Comment) are all CLOSED**
on first-hand evidence. PA-01 and PA-05 leave residuals (PA-08/PA-12 and PA-09/PA-10)
that must be closed at commit.

## 5. New findings (this round)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| **PA-08** | **Minor** | `README.md` metrics panel / `tools/scripts/metrics.py` (T13) — N8/N11/N15 | **The README metrics panel is stale at the frozen revision and the gate fails.** `metrics.py --check` exits **1** (`README metrics panel is stale - run 'make metrics'`). The single differing row is `Review streams closed`: the README panel says **28/32 (87.5%)** while the current review log generates **29/34 (85.3%)** — because the Sprint-3 review-log rows were appended to `docs/reviews/README.md` **without regenerating the panel**. Both CI workflows run `metrics.py --check`, so the increment as frozen **would fail the CI `docs` job**. This sub-item was explicitly part of PA-01's disposition. | `python3 tools/scripts/metrics.py --check` → exit 1; `diff` of the generated vs README panel (only the Review-streams row differs); `.github/workflows/ci.yml:61`; `.gitea/workflows/ci.yml:64`; `docs/reviews/README.md` rows 129–136 | Regenerate the panel with `make metrics` (updates `README.md` between the METRICS markers) in the same commit, then re-run `metrics.py --check` to exit 0. **Pre-commit blocker.** |
| **PA-09** | Comment | `tools/scripts/misra.sh` header comment — N2/N10 | The script header still says "Exit code: number of reported findings (0 = clean), capped for CI", but the (now fail-closed) script exits **2** when `cppcheck` is absent and **0** otherwise regardless of findings. The comment is inaccurate (pre-existing, made visible by this change). | `tools/scripts/misra.sh:9` (comment) vs `:17-20` (`exit 2`) and `:31` (`exit 0`); `git diff -- tools/scripts/misra.sh` | Reword the header to state the actual contract (exit 2 = checker missing, else 0; the finding count is printed). |
| **PA-10** | Comment | T14 reproducibility (N10) — residual of PA-05 | The `make misra-docker` path is **tag-pinned** (`ubuntu:24.04`, not digest-pinned) and fetches an **unpinned** `cppcheck` via apt (2.13.0 today); the declared reproducibility anchor `frick-vv:local` still has **no `cppcheck`** although `tools/docker/Dockerfile:15` lists it, so the T14 row's "QUALIFIED … reproducible via `make misra-docker`" rests on an unpinned path. | `Makefile:53-55`; `docker run --rm frick-vv:local bash -lc 'which cppcheck'` → ABSENT; `tools/docker/Dockerfile:15`; `tool-qualification.md:38` | Pin the checker revision (image digest / version) and rebuild the T1 image so it matches its Dockerfile, or record an explicit owner-assigned T14 limitation. Carried from R1-C9 / N-R1-D3 / N-R2-D2. |
| **PA-11** | Comment | Third-party pin (N8/N10) | lwIP is pinned by the **release tag** `STABLE-2_2_1_RELEASE` (documented in `network-stack.md`, `user-integration-manual.md`, `software-description.md`), which is a **moving** reference in the upstream repo; the exact commit hash is not recorded. | `docs/architecture/network-stack.md` (0.3); `docs/engineering/user-integration-manual.md` §8.1; `docs/engineering/software-description.md` §3.7/§3.9 | Record the exact lwIP commit hash alongside the tag (or pin by hash) so a fetched dependency is reproducible (N10). |
| **PA-12** | Comment | `docs/status/status.json` note vs gates (N8/N11) — residual of PA-01/R4-N8 | `meta.note` says the Sprint-3 review set is **"in progress"**, yet `gates.reviews`, `gates.documentation_review` and `gates.product_assurance_review` are **green**. If the semaphores describe the current cycle they are premature; if they describe the last closed cycle the note wording is ambiguous. The lead reconciles the semaphores just before commit; at the frozen revision the note/gate pair is internally inconsistent. | `docs/status/status.json` `meta.note` vs `gates` block; `docs/reviews/README.md` row 136 `(pending)` | Reconcile at commit: keep green (last closed cycle) with an unambiguous note, or reflect the in-progress state until the loop closes; state the semaphore semantics. |
| **PA-13** | Comment | Cross-review accuracy (N11/N12) — carried | The R1-delta finding **N-R1-D4** ("duplicate `init(100000u)` at `test_i2c.c:277-278`") is **not reproducible** at the fixed revision: line 277 is `init(625000u)` → `div_code 3`; there is only one `init(100000u)` in `test_divider_selection`. The R1-delta follow-up record should be reconciled (close as addressed). | `tests/unit/test_i2c.c:270-271,277`; `docs/reviews/2026-10-12-m4-m11-i2c-r1-delta-review.md:99,155`; R2-δ `N-R2-D4` | Reconcile the N-R1-D4 follow-up record so the review set is internally consistent. |
| **PA-14** | Comment | Test hygiene (N9/N10) — carried | `test_setup_and_init` and `test_self_test` bind a **stack-local** `FF_I2cMpfs_t` as the module-global `g_i2c`, which dangles after return (safe only because every later test re-binds via `mock_bind()`). Latent; not currently exercised. | `tests/unit/test_i2c.c:227,240,466,471`; `ff_i2c_mpfs.c:22,357`; R1-δ `N-R1-D5`, R2-δ `N-R2-D3` | Prefer the file-scope instance for binding, or null `g_i2c` on scope exit. Owner: R2/authoring agent. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier. *(None identified.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified. *(PA-08.)*
- **Comment**: observation/recommendation, no derogation. *(PA-09…PA-14.)*

### 5.1 Positive observations (no finding)

- **The review set is now real and complete** (V1–V3): R1/R2/R3/R4 + deltas + PA,
  each concluding APPROVED WITH COMMENTS with its findings closed — the same
  non-derogable closure gate the previous three increments breached is now met
  *before* commit.
- **The technical increment remains sound and honestly scoped:** `trace.py` PASS
  (100/59/27/14/51); native **20/20** and ASan/UBSan **20/20**; `test_i2c` **144/0**;
  `lake build` success; `clang-format` clean; `make smoke` PASS; **real** Renode run
  (`hal_i2c=PASS`, `RENODE_TEST: PASS`); QEMU PASS; T14 MISRA reproducible (13
  findings, 0 in I2C).
- **The initial PA findings are genuinely fixed**, not merely annotated: the 1 MHz
  case is now executed; the four header revisions and the loop-audit baseline are
  reconciled; the MISRA gate is fail-closed and reproducible; the new document is
  registered; the undefined token is expanded.
- **The R3/R4 new findings are closed** (V24/V25): the traceability mirror count
  (SRS = 59), the SDD `Date`, the README I2C inventory row, the CHANGELOG versions,
  the loop-audit "100 reqs" row, the 802.1AS "time-read seam" wording, the ADR range,
  the PLIC reference-value qualifier and the GAP-A5 cross-reference are all corrected.
- **Limitations are disclosed, not hidden** (GAP-A5 records the I2C on-target residual:
  STOP completion, bus recovery, per-rate SCL accuracy); no documentation over-claim
  on TCP/IP/TSN; no new third-party/generated code; SPDX `AGPL-3.0-only` on the new
  files (N13).

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — no open Major; the initial Major/Minor findings
  PA-01…PA-07 are **CLOSED**; one new **Minor** (PA-08, README metrics panel stale /
  `metrics.py --check` fails) and six **Comments** (PA-09…PA-14) are tracked to closure.
- [ ] REJECTED — one or more Major findings

**Rationale.** The Major **PA-01** review-set closure/log gate is **satisfied**: the
complete independent review set (R1/R2/R3/R4 + deltas + PA) now exists and is logged,
and every prior finding of the increment's reviews is closed. The product-level
evidence is **honest and independently reproducible from the artifacts on file**;
there is **no product defect and no open Major**. The remaining items are a
**records-currency** Minor (PA-08) plus advisory Comments. Because **PA-08 makes the
CI `docs` gate fail** (`metrics.py --check`), it — together with the semaphore/note
reconciliation (PA-12) — **must be closed before commit/push** (N7/N15); the fix is a
single `make metrics` regeneration and a `status.json` note/gate reconciliation.

### 6.1 What a qualified human must still accept (this assistant cannot)

1. **Emulation-only evidence for SRS-REQ-058.** The I2C backend is verified by a host
   unit test plus an **emulated** SoC register round-trip. There is no real-silicon SCL
   timing, bus-electrical or clock-stretch/slave-mode evidence: the emulated result is
   a **partial mitigation** of **GAP-A5**.
2. **Divider accuracy.** The serial-clock divider is a **reference derivation** from an
   assumed 100 MHz APB clock (a 100 kHz request resolves to `PCLK/960` ≈ 104.2 kHz).
   A human must accept the on-target SCL-accuracy residual under GAP-A5.
3. **Tool-qualification evidence.** A human must accept the T14 MISRA evidence basis —
   reproducible via `make misra-docker`, but on an **unpinned** checker (PA-10) — or the
   tracked limitation.
4. **Residual class-A gaps.** GAP-A5 (and GAP-A4, GAP-A1/A2) remain open.
5. **Commit/release authority.** This PA delta review is an assurance input only; the
   decision to commit, push to both remotes (N7) and continue the loop remains with the
   project's authorized authority after PA-08 and PA-12 close. **This assistant cannot
   authorize commit, push, release or flight use.**

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20)      | independent reviewer agent (**FRICK-REVIEW-0136**) | 2026-10-12 | **APPROVED WITH COMMENTS** — PA-01…PA-07 CLOSED (Major PA-01 review-set gate satisfied); new Minor PA-08 (README metrics panel stale / `metrics.py --check` fails) + Comments PA-09…PA-14 tracked to closure; PA-08 and PA-12 must close before commit/push |

## 8. Follow-up records

(norm N12: NCs are resolved by re-applying all rules to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-01 | Major | Review set (N15/N9/N17/N20/N11) | No R1/R2/R4 reports; no log rows; premature semaphores | Produce/log the complete set; reconcile semaphores | 7 reports present; log rows 129–136; R1/R2/R3/R4/deltas APPROVED WITH COMMENTS | frick-fsw | **closed** (residuals PA-08/PA-12) | this report |
| PA-02 | Minor | Revision histories (N16) | 4 docs: header not bumped to the new History row | Bump headers | 16/16 header ↔ History match | frick-fsw | **closed** | this report |
| PA-03 | Minor | `loop-audit.md` baseline (N19/N16) | Baseline-commit stale | Refresh the baseline line | `loop-audit.md:9` | frick-fsw | **closed** | this report |
| PA-04 | Minor | Divider-coverage claim (N8/N11) | "1 MHz tested" not exercised | Add the 1 MHz case; reconcile records | `test_i2c.c:280-281`; `checks=144`; records 144 | frick-fsw | **closed** | this report |
| PA-05 | Minor | T14 MISRA evidence (N10/N15/N6/N8) | `cppcheck` absent; report not reproducible | Reproducible `make misra-docker`; fail-closed script | `make misra-docker` → 13/0 I2C; `misra.sh` exit 2 | frick-fsw | **closed** (residuals PA-09/PA-10) | this report |
| PA-06 | Minor | Documentation-set registration (N8/N16) | New doc not indexed/CHANGELOG'd | Add to README index + AGENTS + CI lists | `README.md:371`; `AGENTS.md:36`; both CI lists | frick-fsw | **closed** | this report |
| PA-07 | Comment | `sprint-plan.md` terminology (N8) | `RR-058` undefined token | Expand the token | `sprint-plan.md:44` | frick-fsw | **closed** | this report |
| PA-08 | **Minor** | `README.md` metrics panel (T13) — N8/N11/N15 | Panel stale; `metrics.py --check` exit 1; CI `docs` job would fail | Regenerate with `make metrics`; re-run `--check` | `metrics.py --check`; panel diff (Review streams 28/32 vs 29/34) | frick-fsw | **open** | pending |
| PA-09 | Comment | `tools/scripts/misra.sh` header comment (N2/N10) | Exit-code comment inaccurate | Reword the header | `misra.sh:9` vs `:17-20,31` | frick-fsw | open (advisory) | — |
| PA-10 | Comment | T14 reproducibility (N10) | Checker unpinned; T1 image stale | Pin the checker / rebuild the image or record a limitation | `Makefile:53-55`; `frick-vv:local` no cppcheck; `Dockerfile:15` | frick-fsw / R2 | open (advisory) | — |
| PA-11 | Comment | lwIP pin (N8/N10) | Moving release tag, no commit hash | Record the exact commit hash | `network-stack.md`; `user-integration-manual.md` §8.1 | frick-fsw | open (advisory) | — |
| PA-12 | Comment | `status.json` note vs gates (N8/N11) | "in progress" note vs green review gates | Reconcile at commit | `status.json` `meta.note` vs `gates` | frick-fsw (lead) | open (advisory) | — |
| PA-13 | Comment | Cross-review accuracy (N11/N12) | N-R1-D4 not reproducible | Reconcile the R1-δ record | `test_i2c.c:270-271,277` | frick-fsw / R1 | open (advisory) | — |
| PA-14 | Comment | Test hygiene (N9/N10) | Stack-local instance bound as `g_i2c` | Bind the file-scope instance | `test_i2c.c:227,240,466,471` | frick-fsw / R2 | open (advisory) | — |

## 9. Norm-by-norm audit (N1–N20) for this increment (delta)

| Norm | Assessment | How established |
| ---- | ---------- | --------------- |
| N1 (Lean) | **N/A / satisfied** | No Lean artifact touched; SRS-REQ-058 carries an explicit `lean_note`; `lake build` success (V10). |
| N2 (English) | **Pass** | All new/touched content English; one inaccurate script comment (PA-09, advisory). |
| N3 (SRS/SSS backwards) | **Pass** | SRS-REQ-058 present; parent SSS-REQ-020 linked (V23). |
| N4 (bidirectional traceability) | **Pass** | `trace.py` PASS (100/59/27/14/51) (V5); mirror count SRS=59 corrected (R3-09). |
| N5 (class gaps) | **Pass** | GAP-A5 records the I2C on-target residual; cross-references correct (V24). |
| N6 (V&V/deploy) | **Pass** | Native + ASan suites, smoke, real Renode, QEMU → PASS (V7/V8/V12/V13/V14); T14 reproducible with an unpinned-checker residual (PA-10). |
| N7 (dual push) | **Pending** | Increment uncommitted; and PA-08/PA-12 must close before push. |
| N8 (docs current) | **Pass with findings** | Increment docs updated and reconciled; **PA-08** (stale panel), PA-11, PA-12. |
| N9 (independent review) | **Pass** | Complete independent review set (R1/R2/R3/R4 + deltas) present and logged (V1/V2). |
| N10 (tool qualification) | **Pass with Comment** | T3 gains `test_i2c.c`; T17 gains `hal_i2c`; T14 reproducible via `make misra-docker`; residual PA-10; PA-09 comment. |
| N11 (scripts/records in repo) | **Pass with Minor** | Review reports logged; records reconciled to 144; **PA-08** (panel), PA-13. |
| N12 (NC resolution) | **Closing** | All initial Majors/Minors closed; PA-08 (Minor) tracked to closure — conversion to APPROVED WITH COMMENTS. |
| N13 (licensing) | **Pass** | SPDX on all new files; no new third-party. |
| N14 (SSS systems review) | **Pass** | SSS-REQ-020 text unchanged; R3 review (0130) confirms coherent parenting. |
| N15 (full cycle) | **Pass with Minor** | All applicable gates green **except** `metrics.py --check` (**PA-08**). |
| N16 (revision history) | **Pass** | Headers/History reconciled (V16); review-id range consistent (V22); CHANGELOG updated. |
| N17 (documentation review) | **Pass** | R4 report 0132 (consolidated) APPROVED WITH COMMENTS, R4-01…06 closed. |
| N18 (Renode) | **Pass** | Real Renode run (`hal_i2c=PASS`, `RENODE_TEST: PASS`, 7 apps) (V13). |
| N19 (loop audit) | **Pass** | `loop-audit.md` baseline refreshed (V17); N3 row "100 reqs". |
| N20 (PA review) | **This report** | PA delta re-review; PA-01…PA-07 closed; PA-08 + Comments to closure. |

## 10. Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

```text
Facts (independently reproduced this session):
  - Review set complete: docs/reviews/ holds R2 0129, R3 0130, R1 0131, R4 0132, PA 0133,
    R1-delta 0134, R2-delta 0135; README log rows 129-136 present (row 136 = 0136 pending).
  - trace.py PASS (100/59/27/14, 51 theorems); metrics.py --check exit 1 (stale panel).
  - ctest 20/20 (native) and 20/20 (asan); test_i2c checks=144 failures=0; lake build success;
    clang-format clean; make smoke PASS; make renode PASS (hal_i2c=PASS); make qemu PASS;
    make misra-docker 13 findings / 0 in I2C.
  - 16/16 touched docs: header Revision+Date == last History row.
  - loop-audit baseline = "db8261f + Sprint-3 MSS I2C increment (audited revision)".
  - software-description.md registered in README:371, AGENTS:36, both CI docs lists; RR-058 expanded.
  - traceability.md SRS=59; loop-audit N3 row "100 reqs"; review-id range 0129..0136 in all three records.
  - metrics.py --check fails because the README panel "Review streams closed" row = 28/32 while the
    current log generates 29/34; both CI workflows run metrics.py --check.

Assumptions:
  - The uncommitted working tree is the intended commit content atop db8261f (frozen for this review).
  - build/ is gitignored, so the gate re-runs modified no tracked file.

Inferences:
  - The panel staleness (PA-08) was introduced by appending the Sprint-3 review-log rows without
    regenerating the panel (the R2-delta review saw "metrics panel: current" before the rows landed).
  - The semaphore/note tension (PA-12) is the lead's just-before-commit reconciliation, not a hidden defect.
  - N-R1-D4 is a stale citation in the R1-delta record (the cited duplicate does not exist).

Recommendations:
  1. Close PA-08 before commit: run `make metrics`, then `metrics.py --check` to exit 0 (owner: authoring agent).
  2. Reconcile the status.json semaphores/note (PA-12) and append this report's log row (owner: lead).
  3. Address PA-09/PA-10 (misra.sh comment; pin the checker / rebuild the T1 image) under N10.
  4. Record the lwIP commit hash (PA-11); reconcile the R1-delta N-R1-D4 record (PA-13); fix the
     test-hygiene latent dangling binding (PA-14).

Confidence: High on PA-01...PA-07 closure, PA-08 (direct, reproducible gate failure), and every gate
  reproduction; High on PA-12/PA-13 (direct text comparisons); Medium on PA-10/PA-11 (tooling/dependency
  reproducibility judgements).

Missing/inaccessible evidence:
  - The exact image that produced the historical build/misra-report.txt is not available (superseded by
    the reproducible make misra-docker path).
  - No real-silicon SCL/timing evidence (GAP-A5).
  - The uncommitted revision is a moving target (N-R2-D5); freeze/commit before the final closure pass.

Required independent human reviewer/authority: the project's authorized human/release authority
  (acceptance of the review-set closure, the PA-08 pre-commit gate closure, and the GAP-A5 on-target residual).
```

## 11. Evidence consulted

- Norms/instruments: `docs/rules.md` (N1–N20), `docs/reviews/review-template.md`,
  `docs/engineering/independent-review.md`; precedents FRICK-REVIEW-0109/0123/0133.
- Review set: `docs/reviews/2026-10-12-m4-m11-i2c-{r1,r2,r3,r4,pa}-review.md`,
  `…-r1-delta-review.md`, `…-r2-delta-review.md`, `docs/reviews/README.md`.
- Increment delta: `git status --porcelain`, `git diff --stat`, `git diff -- tools/scripts/misra.sh`,
  `tests/unit/test_i2c.c`, `src/drivers/i2c/mpfs/ff_i2c_mpfs.{c,h}`, `src/apps/app_renode_test.c`,
  `config/platform/platform_mpfs.h`.
- Requirements/records: `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`,
  `docs/quality/class-a-gaps.md` (GAP-A5), `docs/vv/{verification-results.md,tool-qualification.md,renode-environment.md}`,
  `CHANGELOG.md`, `README.md`, `AGENTS.md`, `docs/status/{status.json,loop-audit.md,handover.md}`,
  `docs/planning/sprint-plan.md`, `docs/design/software-design.md`, `docs/interfaces/ICD.md`,
  `docs/engineering/{software-description.md,user-integration-manual.md,driver-development.md}`,
  `docs/architecture/network-stack.md`, `docs/development-plan.md`.
- Gates/artifacts: `tools/scripts/{trace.py,metrics.py,misra.sh}`, `Makefile`,
  `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `build/renode/console.log`,
  `build/misra-report.txt`.
- ECSS-E-ST-40C Rev.1; ECSS-Q-ST-80C Rev.2.

---

*This report is the PA delta member of the Sprint-3 review set and must be committed
and logged together with the increment (norm N11). The review was read-only: I
modified no tracked file, created no file other than this report, and did not commit
or push. This is product-assurance analysis, not human sign-off, and does not
authorise qualification, acceptance, release or flight use; PA-08 and PA-12 must close
before commit/push (N7/N15) and a qualified human must accept the residual items.*
