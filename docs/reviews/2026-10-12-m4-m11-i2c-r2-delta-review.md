# Independent Review Report — R2 delta re-review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | **FRICK-REVIEW-0135**                  |
| Reviewer session                  | independent agent (fresh context, norm N9) |
| Reviewer profile                  | **R2** (Senior SW V&V engineer)        |
| Reference revision                | `db8261f` + **uncommitted** Sprint-3 working tree (fixed revision; content hashes recorded in §9) |
| Items under review                | `src/drivers/i2c/mpfs/ff_i2c_mpfs.{c,h}` (new); `tests/unit/test_i2c.c` (new); I2C check in `src/apps/app_renode_test.c`; `config/platform/platform_mpfs.h`; `CMakeLists.txt`, `tests/CMakeLists.txt`, `Makefile`; records `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`, `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`, `CHANGELOG.md`, `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml` |
| Date                              | 2026-10-12                             |
| Scope                             | Delta re-review (norm N12) of the Sprint-3 MSS I2C increment after the initial R2 review `FRICK-REVIEW-0129`; verify closure of **R2-01…R2-04**, look for regressions and new findings, and re-execute every declared V&V gate — against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA), the norms N1–N20 (`docs/rules.md`), and the HAL contract `FF_I2c_t` (`src/hal/hal_spi.h`). |
| Independence declaration          | I authored no part of this increment, ran no gate as its author, hold no decision authority over it, and did not review work in which I participated. This review ran in a fresh context; every result below was re-executed/re-derived by me first-hand from the repository. |
| Norms applied                     | N1–N20 (`docs/rules.md` 1.4)           |

## 1. Scope

**In scope (verified first-hand on 2026-10-12):**

- The fixed driver `src/drivers/i2c/mpfs/ff_i2c_mpfs.c` / `.h` — specifically the
  changes made in response to `FRICK-REVIEW-0129`: the header limitation notes,
  the `FF_I2cMpfsSetup` NULL-bus-hook rejection, and the mock/oracle disclaimer.
- The extended unit test `tests/unit/test_i2c.c` (mock bus + register-based slave
  model; **144 checks**): the added 1 MHz divider-code-5 case, the 104166/104167 Hz
  minimum-SCL boundary cases, the 625000 Hz code-3 case, and the NULL-bus-hook
  rejection cases.
- The I2C functional check in `src/apps/app_renode_test.c`
  (`renode_hal_i2c_check`, `hal_i2c=PASS`) and its inclusion in the `RENODE_TEST`
  verdict.
- The record deltas: SRS-REQ-058 text, `traceability.yaml`/`traceability.md`,
  `verification-results.md`, `renode-environment.md`, `tool-qualification.md`
  (T3/T14/T17), `CHANGELOG.md`, the `ci-renode` evidence markers in both CI
  workflows, and the new `Makefile` `misra-docker` target.
- Independent re-execution of the local gate set (see §9).

**Out of scope / excluded (bounded):**

- On-target silicon behaviour (real APB clock, real SCL accuracy, real-slave
  timing, clock stretching, bus recovery, arbitration) — tracked to GAP-A5 and
  explicitly not verifiable here.
- The R1/R3/R4/PA reviews of the same increment and their (non-R2) findings,
  except where a fix also bears on an R2 finding.
- Unrelated doc-only deltas present in the same working tree
  (`docs/architecture/network-stack.md`, `docs/engineering/*`, `README.md`,
  `AGENTS.md`, `docs/design/*`, `docs/interfaces/ICD.md`, `docs/status/*`,
  `docs/planning/sprint-plan.md`, `docs/quality/class-a-gaps.md`), reviewed only
  where they carry R2-relevant evidence (GAP-A5, the review-set records).

**Assumption recorded — moving revision.** The item under review is an
**uncommitted** working tree, so no commit hash identifies it. During the review
window the tree was observed to change (`CHANGELOG.md` mtime `08:16:06`, ~53 s
before my clock read `08:16:59`; the cited `FRICK-REVIEW-0129..0136` line moved).
My verdict is therefore tied to the **content hashes recorded in §9**, not to a
commit id; any later edit invalidates the closure evidence for the touched items
(N-R2-D5).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract:
  `docs/engineering/independent-review.md` (rev 0.7).
- Report format: `docs/reviews/review-template.md`.
- Prior review: `docs/reviews/2026-10-12-m4-m11-i2c-r2-review.md`
  (`FRICK-REVIEW-0129`).
- Contract: `src/hal/hal_spi.h` (`FF_I2c_t`).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-058),
  `docs/requirements/SSS.md` (SSS-REQ-020),
  `docs/requirements/traceability.yaml` (`:543-550`).
- Gaps: `docs/quality/class-a-gaps.md` (GAP-A5, rev 0.7).
- Evidence: `docs/vv/verification-results.md` (0.34), `docs/vv/tool-qualification.md`
  (0.3.18), `docs/vv/renode-environment.md` (0.14), `CHANGELOG.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Prior findings closure (R2-01…R2-04)

| Finding | Disposition (as declared) | First-hand evidence seen (this session) | Status |
| ------- | ------------------------- | --------------------------------------- | ------ |
| **R2-01** (Minor) — "1 MHz tested" unsupported | Genuine 1 MHz case added; records reconciled to 144 checks | `test_i2c.c:280-281` `init(1000000u)` → `div_code 5` (PCLK/120 = 833333 ≤ 1000000); executed `./build/native/tests/test_i2c` → `checks=144 failures=0`; `CHANGELOG.md:22,76,83` and `verification-results.md:46` now state 144 checks and the rates 100 kHz/400 kHz/1 MHz; `tool-qualification.md:127` (0.3.18) records 135 → 144 | **CLOSED** |
| **R2-02** (Minor) — `ci-renode` marker omits SRS-REQ-058 | `SRS-REQ-058` added to both markers | `.github/workflows/ci.yml:80` and `.gitea/workflows/ci.yml:83` both list `… SRS-REQ-057, SRS-REQ-058, SSS-REQ-013, SSS-REQ-020`; `python3 tools/scripts/trace.py …` → **PASS** (100/59/27/14, 51 theorems) | **CLOSED** |
| **R2-03** (Comment) — shared-constant / SI write-gate oracle limits | Mock/emulator discrimination limit documented | `test_i2c.c:10-18` states the mock is "a convenient behavioural/phase-level fake, not an independent oracle", cannot discriminate a wrong CTRL/SI bit assignment, and "acts on STA/STO before applying the SI write-gate that real CoreI2C imposes", pointing to `docs/vv/renode-environment.md`; `renode-environment.md:111-118` documents the emulator scope limit (no CTRL CR0/CR1/CR2; advances on register writes); T14 row/History updated (0.3.18) | **CLOSED** |
| **R2-04** (Comment) — "/DMA" wording for I2C | Reworded to "full driver path" | `renode-environment.md:105-106` now reads "the I2C checks are MMIO identification/round-trip checks; the full driver path is host-tested (the reference DMA seam applies to MEM/ETH only, not to I2C)"; History 0.14 records the correction (FRICK-REVIEW-0129 R2-04) | **CLOSED** |

**Regression check on the fixes:** no regression observed. The added test cases
(1 MHz, 625000 Hz, minimum-SCL boundary, NULL-bus-hook) execute green; native
`ctest` 20/20; ASan/UBSan `ctest` 20/20; `clang-format` clean; `make renode`
→ `RENODE_TEST: PASS` with `hal_i2c=PASS`; `make qemu` → PASS; `trace.py` PASS.
The only behavioural code change from the initial round (`FF_I2cMpfsSetup`
NULL-hook rejection, `ff_i2c_mpfs.c:346-348`) affects only misconfigured callers;
the Renode app and every valid caller pass a fully-populated bus.

## 4. New findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| **N-R2-D1** | **Minor** | Review-set records (N8/N11/N16) | **The CHANGELOG cites a review set and findings that are not present in the repository.** `CHANGELOG.md:91` says the NCs are "`FRICK-REVIEW-0129..0136`" and its `Fixed` entry attributes fixes to findings `R3-01/R3-02/R3-04/R3-06/R3-07/R3-08` and `R4-01/R4-03/R4-04/R4-06`; but `docs/reviews/` contains only R2=0129, R1=0131, PA=0133 and R1-delta=0134 for this increment (no R3 or R4 I2C report), and `docs/reviews/README.md` has **no** Sprint-3 rows. The cited id range also contradicts the increment's own records: `verification-results.md:435` (0.34) and `tool-qualification.md:127` (0.3.18) both say "`FRICK-REVIEW-0129..0133`". The cited R3/R4 fixes therefore cannot be independently verified at this revision (N11: review results in the repo; N16: history accuracy). | `CHANGELOG.md:91`; `verification-results.md:435`; `tool-qualification.md:127`; `docs/quality/class-a-gaps.md` GAP-A5 cites "R3-08"; `ls docs/reviews/` (no R3/R4 I2C report); `docs/reviews/README.md` (no Sprint-3 rows) | Land the R3/R4 reports (and their `README.md` log rows) with the change, or correct the CHANGELOG citations and id range so every referenced finding is verifiable. Cross-reference PA-01 (review-set closure gate). |
| **N-R2-D2** | Comment | Tool qualification T14 (N10) | **The advisory MISRA/T14 gate is fail-open and its declared reproducibility anchor is stale.** `tools/scripts/misra.sh:17-19` prints "cppcheck not installed" and `exit 0` when the checker is absent, so a green T14 can be a silent no-op; `frick-vv:local` (the image whose `tools/docker/Dockerfile:15` lists `cppcheck`) has **no** `cppcheck` (probe → ABSENT), and `make misra-docker` fetches an unpinned `cppcheck` at run time. The T14 row (`tool-qualification.md:38`) now claims "reproducible via `make misra-docker`"; I reproduced 13 advisory findings, 0 in any I2C file, but the qualification rests on an unpinned/fail-open path. | `tools/scripts/misra.sh:17-19,31`; `docker run --rm frick-vv:local bash -lc 'which cppcheck'` → ABSENT; `tools/docker/Dockerfile:15`; `Makefile` `misra-docker`; `tool-qualification.md:38,127`; `make misra-docker` → 13 findings, 0 in `ff_i2c_mpfs`/`test_i2c`/`app_renode_test` | Make `misra.sh` fail closed (non-zero) when the checker is missing, pin the checker revision (image digest/version), and refresh the T1 image — or record an explicit owner-assigned T14 limitation. Carried from R1-C9/N-R1-D3 and PA-05; routed to R2/PA. |
| **N-R2-D3** | Comment | Test robustness (N9/N10) | **A stack-local instance is bound as the module-global `g_i2c`.** `test_setup_and_init` (`test_i2c.c:227,240`) and `test_self_test` (`:466,471`) bind a function-local `FF_I2cMpfs_t` as `g_i2c`, which dangles after return; this is safe only because every later test re-binds first via `mock_bind()`. A future test that used the driver without rebinding would exercise freed stack. Carried from R1's N-R1-D5 (routed to R2). | `test_i2c.c:227,240,466,471`; `ff_i2c_mpfs.c:22,357` | Prefer the file-scope instance (`g_drv`) for binding, or null `g_i2c` on scope exit, to remove the latent dangling pointer. |
| **N-R2-D4** | Comment | Cross-review accuracy | **The R1-delta finding N-R1-D4 is not reproducible at the reviewed revision.** It cites a duplicate `init(100000u)` at `test_i2c.c:270-271` and `:277-278`, but line 277 is `init(625000u)` → `div_code 3` (PCLK/160); there is only one `init(100000u)` in `test_divider_selection`. Either the item was already addressed or the citation is stale; the R1-delta follow-up record should be reconciled. | `test_i2c.c:270-271,277-278`; `docs/reviews/2026-10-12-m4-m11-i2c-r1-delta-review.md:99,155` | Reconcile the N-R1-D4 follow-up record (close as addressed) in the R1 delta report. |
| **N-R2-D5** | Comment | Review integrity (N11/N12) | **The reviewed revision is a moving target.** The uncommitted working tree changed during the review window (`CHANGELOG.md` mtime `08:16:06`, ~53 s before my clock read `08:16:59`), so closure evidence is bound to content hashes (§9), not a commit id. Independent verification (N9) requires a frozen revision. | `stat -c '%y %n' CHANGELOG.md` → `2026-10-12 08:16:06`; `date` → `08:16:59`; §9 hash snapshot | Freeze (or commit) the revision before the final closure pass; re-verify any item whose hash changes. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(None identified.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified. *(N-R2-D1.)*
- **Comment**: observation/recommendation, no derogation. *(N-R2-D2…N-R2-D5.)*

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — all four prior findings (R2-01/R2-02 Minor,
  R2-03/R2-04 Comment) are **CLOSED**, with no regression; one new **Minor**
  (N-R2-D1, records/review-set) is tracked to closure per norm N12 and four
  Comments (N-R2-D2…N-R2-D5) are advisory.
- [ ] REJECTED — one or more Major findings

Rationale: every declared gate re-executes green (§9); the "144 checks" figure is
now accurate (verified by execution); the `ci-renode` marker names SRS-REQ-058 in
**both** workflows; and the mock/oracle wording is honest and specific. No
Major finding; no product defect; no safety/security-critical behaviour observed
as incorrect. The one Minor (N-R2-D1) and the highest-priority Comment
(N-R2-D2, fail-open T14) are records/tool-qualification issues that do not
invalidate SRS-REQ-058. The report must not be read as a qualification,
acceptance, or release decision (see §7 and the closing note).

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9)       | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — R2-01…R2-04 CLOSED; N-R2-D1 (Minor) + N-R2-D2…N-R2-D5 (Comments) tracked to closure per N12 |

## 7. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R2-01 | Minor | Evidence (N11) — SRS-REQ-058 | "1 MHz tested" unsupported | 1 MHz (code 5) case added; records reconciled to 144 checks | `test_i2c.c:280-281`; runtime `checks=144`; `CHANGELOG.md:22`; `verification-results.md:46` | frick-fsw | **closed** | this report |
| R2-02 | Minor | `ci-renode` marker, both workflows | Marker omits SRS-REQ-058 | `SRS-REQ-058` added to both markers | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83`; `trace.py` PASS | frick-fsw | **closed** | this report |
| R2-03 | Comment | `tests/unit/test_i2c.c` oracle / Renode model | Shared constants; SI write-gate not modelled | Documented in the test header + `renode-environment.md`; T14 row/History updated | `test_i2c.c:10-18`; `renode-environment.md:111-118`; `tool-qualification.md:38,127` | frick-fsw | **closed** | this report |
| R2-04 | Comment | `renode-environment.md:104-105` | "/DMA" wording inaccurate for I2C | Reworded to "full driver path" (DMA seam = MEM/ETH only) | `renode-environment.md:105-106`; History 0.14 | frick-fsw | **closed** | this report |
| N-R2-D1 | Minor | Review-set records (N8/N11/N16) | CHANGELOG cites `0129..0136` and R3/R4 findings with no R3/R4 report in the repo; id range contradicts other records | (to be corrected: land R3/R4 reports + log rows, or fix the citations) | `CHANGELOG.md:91`; `verification-results.md:435`; `tool-qualification.md:127`; `ls docs/reviews/` | frick-fsw | **open** | pending |
| N-R2-D2 | Comment | Tool qualification T14 (N10) | `misra.sh` fail-open; `frick-vv:local` lacks `cppcheck`; checker unpinned | (to be corrected or tracked) | `tools/scripts/misra.sh:17-19`; `docker … frick-vv:local which cppcheck` → ABSENT; `tool-qualification.md:38` | frick-fsw / R2 / PA | **open** (advisory) | — |
| N-R2-D3 | Comment | Test robustness (N9/N10) | Stack-local instance bound as module-global `g_i2c` | (advisory) | `test_i2c.c:227,240,466,471`; `ff_i2c_mpfs.c:22,357` | frick-fsw / R2 | **open** (advisory) | — |
| N-R2-D4 | Comment | Cross-review accuracy | N-R1-D4 (duplicate `init(100000u)`) not reproducible | (reconcile the R1-delta record) | `test_i2c.c:270-271,277-278` | frick-fsw / R1 | **open** (advisory) | — |
| N-R2-D5 | Comment | Review integrity (N11/N12) | Uncommitted revision changed during review | (freeze/commit before final closure) | `stat CHANGELOG.md` mtime `08:16:06` vs `date` `08:16:59`; §9 hashes | frick-fsw | **open** (advisory) | — |

## 8. Norm-by-norm verification (I2C increment, delta)

| Norm | Status | Basis |
| ---- | ------ | ----- |
| N1 (Lean) | **Pass (pattern)** | No Lean theorem for SRS-REQ-058; explicit `lean_note` present (`traceability.yaml:550`); `lake build` success (§9). |
| N2 (English) | **Pass** | New sources/docs reviewed in English. |
| N3 (backwards SSS/SRS) | **Pass** | SRS-REQ-058 (SRS.md rev 1.17) / SSS-REQ-020 exist. |
| N4 (traceability) | **Pass with Minor** | `trace.py` PASS (100/59/27/14, 51 theorems); evidence-marker annotation now correct (R2-02 closed); review-id citation gap = **N-R2-D1**. |
| N5 (criticality B) | **Pass** | GAP-A5 gains the I2C on-target residual (rev 0.7). |
| N6 (V&V + environments) | **Pass** | Host + ASan + Renode + QEMU evidence re-executed (§9). |
| N7 (dual push) | **N/A here** | Uncommitted; owner action. |
| N8 (docs current) | **Pass with findings** | Increment docs updated; **N-R2-D1**, **N-R2-D5**. |
| N9 (independent review) | **This report** | Independent R2, fresh context. |
| N10 (tool qualification) | **Pass with Comment** | T3/T17 updated; T14 reproducibility claim reproduced, but fail-open/unpinned = **N-R2-D2**. |
| N11 (records in repo) | **Pass with Minor** | Results present; R3/R4 reports + `README.md` rows absent = **N-R2-D1**. |
| N12 (NC resolution) | **In progress (tracked)** | R2-01…R2-04 closed; N-R2-D1 to closure. |
| N13 (licensing) | **Pass** | SPDX `AGPL-3.0-only` on new source/header/test. |
| N14 (SSS review) | **N/A** | SSS text unchanged in this increment. |
| N15 (full cycle) | **Pass** | Applicable gates re-run green (§9). |
| N16 (revision history) | **Pass with Minor** | Headers/History reconciled (SRS 1.17, traceability 0.29, SDD 0.6, loop-audit 0.12); CHANGELOG citation accuracy = **N-R2-D1**. |
| N17 (doc review) | **Owner action** | No R4 I2C report in the repo yet (part of **N-R2-D1**). |
| N18 (Renode) | **Pass** | `RENODE_TEST: PASS`, `hal_i2c=PASS` (§9). |
| N19 (loop audit) | **Pass (owner)** | `loop-audit.md` baseline refreshed to `db8261f + Sprint-3 MSS I2C increment`. |
| N20 (PA review) | **Owner action** | PA report 0133 exists (REJECTED at that revision; PA-01 review-set gate); R3/R4 still pending (**N-R2-D1**). |

## 9. Evidence consulted (re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native build | `cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build` | BUILD OK |
| Unit tests | `ctest --test-dir build/native` | **20/20 pass** (incl. test 20 `i2c`) |
| `test_i2c` checks | `./build/native/tests/test_i2c` | **`checks=144 failures=0`** |
| Sanitizers | `-fsanitize=address,undefined` + `ctest --test-dir build/asan` | **20/20 pass** |
| Formatting | `clang-format --dry-run --Werror` on the 4 changed C/H files | clean |
| Traceability | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (100 reqs / 59 srs / 27 sss / 14 gaps, 51 theorems) |
| Metrics panel | `python3 tools/scripts/metrics.py --check` | `metrics panel: current` |
| Lean | `lake build` (host lacks `lake`; run in `frick-vv:local`) | Build completed successfully |
| Renode functional run | `make renode` | **PASS** (`RENODE_TEST: PASS`; console `hal_i2c=PASS`, `hal_can=PASS`, `hal_nvm=PASS`) |
| QEMU | `make qemu` | **PASS** (banner + 6 apps; toolchain image `90f81d080611`) |
| Smoke | `./build/native/frick_fsw` | banner + "6 applications registered" |
| MISRA/T14 | `make misra-docker` (docker + network) | 13 advisory findings, **0 in any I2C file** |
| `cppcheck` availability | host + `frick-vv:local` probe | host ABSENT; `frick-vv:local` **ABSENT** (Dockerfile lists it) |
| `ci-renode` marker | `grep "ci-renode"` both workflows | SRS-REQ-058 present in `.github:80` and `.gitea:83` |
| Header-revision reconciliation | header `Revision` vs last History row (SRS/traceability/SDD/loop-audit) | 1.17 / 0.29 / 0.6 / 0.12 — all match |
| Review-file inventory | `ls docs/reviews/` + `README.md` | R2=0129, R1=0131, PA=0133, R1-delta=0134; **no R3/R4 I2C report**; no Sprint-3 log rows (N-R2-D1) |
| Revision stability | `stat -c '%y' CHANGELOG.md` vs `date` | mtime `08:16:06` ≈ clock `08:16:59` (N-R2-D5) |

**Reviewed-revision content hashes (md5) — verdict bound to these:**

```
2510454d17080906f57753f1613a2b04  src/drivers/i2c/mpfs/ff_i2c_mpfs.c
4622ced260c25c520d92c6fca50e53da  src/drivers/i2c/mpfs/ff_i2c_mpfs.h
caaab4fa39f2b0ce44860e912244ab8b  tests/unit/test_i2c.c
f2d270c1351ff2703a84dfe680b29fd7  src/apps/app_renode_test.c
940989b777d0f7d7abd8390e50de3754  config/platform/platform_mpfs.h
a6f6f61d701bb443678309f1288c0f90  CMakeLists.txt
18edc6bbea153ad5766da5b6a6b7789d  tests/CMakeLists.txt
5f12cf256161d09c8c6041df31744be9  Makefile
69c65b099296b943796cb1ef6a2dd157  docs/requirements/SRS.md
157cb4ad088e32b3f238508ebb5fbc30  docs/requirements/traceability.yaml
1290edd75ae0d741d1044b796f4a7d9c  docs/requirements/traceability.md
5f3547affebbc49b1ff84f067dc283fb  docs/vv/verification-results.md
d1de2401f4dc5d5b8dbcdadd2b05f8e7  docs/vv/renode-environment.md
51ad81a7b30fe1fc30900a8e907df6ec  docs/vv/tool-qualification.md
f0c10f9c74c8b7ca89c73a20288507d6  CHANGELOG.md
87206d92ca31fe0b2a312b230ca6fd61  .github/workflows/ci.yml
352eaba9f49fd9f8c3e3c460bbac0ba6  .gitea/workflows/ci.yml
```

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- **F1** — Native `ctest` 20/20; ASan/UBSan `ctest` 20/20; `test_i2c` = `checks=144 failures=0`.
- **F2** — `test_i2c.c:280-281` adds a genuine `init(1000000u)` → `div_code 5` (PCLK/120) case; `:272-278` adds the 104166/104167/625000 Hz boundary cases.
- **F3** — `CHANGELOG.md:22,76,83` and `verification-results.md:46` state 144 checks and the rates 100 kHz/400 kHz/1 MHz; the 144 figure matches execution.
- **F4** — The `ci-renode` marker names SRS-REQ-058 in **both** `.github/workflows/ci.yml:80` and `.gitea/workflows/ci.yml:83`; `trace.py` PASS.
- **F5** — `test_i2c.c:10-18` qualifies the mock as a behavioural/phase-level fake, not an independent oracle, and discloses the shared constants and the missing SI write-gate; `renode-environment.md:111-118` documents the emulator scope limit.
- **F6** — `renode-environment.md:105-106` (rev 0.14) rewors the I2C clause and scopes the DMA seam to MEM/ETH only.
- **F7** — `FF_I2cMpfsSetup` rejects NULL `read8`/`write8` (`ff_i2c_mpfs.c:346-348`); the test asserts both NULL-hook cases (`test_i2c.c:230,235-239`).
- **F8** — Gates re-run green: `trace.py` PASS, `lake build` success, `make renode` PASS (`hal_i2c=PASS`), `make qemu` PASS, `make misra-docker` 13 findings / 0 in I2C, `metrics.py --check` current, `clang-format` clean.
- **F9** — `tools/scripts/misra.sh:17-19` exits 0 when `cppcheck` is absent; `frick-vv:local` has no `cppcheck` though `tools/docker/Dockerfile:15` lists it.
- **F10** — `docs/reviews/` contains only R2=0129, R1=0131, PA=0133, R1-delta=0134 for this increment; `CHANGELOG.md:91` cites `0129..0136` and R3/R4 findings; `verification-results.md:435` / `tool-qualification.md:127` cite `0129..0133`.
- **F11** — The uncommitted tree changed during the review window (`CHANGELOG.md` mtime `08:16:06` vs clock `08:16:59`).
- **F12** — The R1-delta's N-R1-D4 citation (`test_i2c.c:277` as a duplicate `init(100000u)`) is actually `init(625000u)`.

**Assumptions** (owner, impact, validation)

- *The reviewed content is the hashed uncommitted working tree over `db8261f`* — owner: authoring agent; impact: review identity; validation: §9 hashes; re-verify if any hash changes (N-R2-D5).
- *`FF_I2C_MPFS_CLK_HZ = 100 MHz` is the reference APB clock* — owner: hardware/mission (GAP-A5); validation: confirm the configured APB clock against the RTPFS160ZT datasheet and re-derive the divider.
- *Renode/QEMU/Lean results are environment-local functional proxies, not silicon evidence* (GAP-A5).

**Inferences**

- **I1** — Because the executed count is 144 and the reconciled records (CHANGELOG/verification-results/tool-qualification/status/handover/sprint-plan) all now say 144, the "144 checks" figure is accurate → R2-01 **closed**. Confidence **High**.
- **I2** — Because `trace.py` still matches only the `ci-renode` token (ignoring the requirement-id parenthetical), the marker correction (R2-02) is not gate-enforced; it is correct but relies on reviewer attention. Confidence **High**.
- **I3** — Because `misra.sh` returns 0 on a missing checker and the declared T1 image lacks `cppcheck`, a green T14 could in principle be a no-op → N-R2-D2. Confidence **High**.
- **I4** — The R3/R4 I2C findings cited by the CHANGELOG are not verifiable from the repository at this revision → N-R2-D1. Confidence **High** (file-level).

**Recommendations** (owner)

1. Close **N-R2-D1**: land the R3/R4 review reports and `docs/reviews/README.md` rows with the change (or correct the CHANGELOG id range and citations); reconcile the `0129..0136` vs `0129..0133` discrepancy.
2. Close **N-R2-D2** (owner R2/PA, N10): make `misra.sh` fail closed when the checker is missing, pin the `cppcheck` revision, and refresh the T1 image so the T14 "reproducible" claim is meaningful.
3. Address **N-R2-D3** (test hygiene: bind the file-scope instance or null `g_i2c`).
4. Reconcile **N-R2-D4** in the R1-delta follow-up record (the cited duplicate does not exist).
5. Freeze or commit the revision (**N-R2-D5**) before the final closure pass, then re-run the delta.

**Confidence:** **High** for the closure of R2-01…R2-04 and for the gate results
(F1–F8), which were all re-executed first-hand and are internally consistent.
**High** for N-R2-D1/D2/D4 (direct, reproducible text/tool comparisons).
**Medium-High** for N-R2-D3 (latent, not currently exercised) and N-R2-D5
(observed via a single mtime/clock comparison).

**Open gaps**

- **N-R2-D1** — review-set records (Minor), to be closed.
- **N-R2-D2** — T14/MISRA fail-open + unpinned checker + stale T1 image (Comment).
- **On-target evidence absent** — GAP-A5 (per-rate SCL accuracy, STOP completion,
  bus recovery).
- **No independent oracle** for the transaction path — disclosed (R2-03) and
  carried for V&V.
- **Revision not frozen** — the uncommitted tree changed during review (N-R2-D5).

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only: I modified no repository file and created
no file other than this report, and I did not commit or push. This is V&V
analysis, not human sign-off, and does not authorise qualification, acceptance,
release, or flight use; a qualified human must close the open items.*
