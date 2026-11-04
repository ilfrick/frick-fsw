# Independent PA Re-review — NC-EXT-235 Corrected Candidate (norm N20)

|                                   |                                                                                          |
| ---------------------------------:| :--------------------------------------------------------------------------------------- |
| **Document id**                   | **FRICK-REVIEW-0442**                                                                    |
| **Reviewer profile**              | **PA — Senior SW Product Assurance** (N20), independent sub-agent, fresh context          |
| **Reviewer session**              | sub-agent session spawned by the coordinator; no author context inherited                  |
| **Reference revision**            | `HEAD = 41dcbe21435e14b5aaff06c63d9df6f0de04df54` **plus** the working-tree candidate set  |
| **Date**                          | 2026-11-03                                                                               |
| **Items under review**            | NC-EXT-235 disposition after prior PA review FRICK-REVIEW-0440 (NEEDS REVISION); corrected evidence set at `docs/vv/evidence/2026-11-03-gate-diagnosis/`; sprint-plan Rev 1.120; verification-results.md cycle FRICK-VV-GATE-20261103; tool-qualification.md rev 0.3.107 |
| **Prior review**                  | FRICK-REVIEW-0440-PA-NC-EXT-235.md (2026-11-03) — verdict **NEEDS REVISION**, 7 Major findings (PA-NC235-01..07), 13 Minor/Observation findings |
| **Independence declaration**      | I did not author, edit, build, test or commit any item under review. I inspected the live tree, git history, and the named logs, and recomputed the published file hashes. I did not execute the test suite. |
| **Norms applied**                 | N1–N22 (focused: N7, N8, N9, N10, N11, N12, N15, N16, N17, N20, N21)                     |

---

## 1. Scope

**In scope.** The candidate defined as `41dcbe2` plus exactly:

- `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` (frozen-UTC seam under `FF_TEST_HOOKS`)
- `tests/unit/test_pus.c` (HK due tests freeze UTC)
- `tests/unit/test_link_uart.c` (positive PTY receive bounds 20 ms / 500 ms → 2 s)
- `docs/planning/sprint-plan.md` (Rev 1.117/1.118 corrections, Rev 1.119 failure record, Rev 1.120 verification record, NC-EXT-235)
- `docs/vv/evidence/2026-11-03-gate-diagnosis/` (new evidence set with rerun logs, copied cache JSONs, RTEMS cross-build, hash manifest)

**Out of scope, and to remain unchanged.** `tests/unit/test_pus_n7_adapter.c`, `test_pusopen.c`, `test_r2_gap.c`, `test_sched_max_entries.c`, `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`, and all other untracked files.

**Not a test-design review.** Test design observations are recorded only where they affect whether the evidence can support the assurance claim.

---

## 2. Artifacts examined (exact paths and commands)

Git and working tree:

- `git rev-parse HEAD`, `git status --porcelain=v1`, `git log --oneline -8`
- `git diff` for all modified tracked files; `git show HEAD:docs/planning/sprint-plan.md` (pre-correction text)
- `sha256sum` on the five candidate files and all files in `assets.sha256` (all matched)
- `stat -c '%y %n'` on the candidate files, the four excluded test files, and every evidence log
- `grep -E "FF_USE_LWIP|FF_USE_N7_OBCP|FF_USE_PUS_ASN1|FF_BUILD_TESTS|FF_PLATFORM" build/{ci,native,xw1-host}/CMakeCache.txt`

Candidate content:

- `src/core/pal/pal.h` (:71–:87), `src/core/pal/pal_posix.c` (:498–:542), `CMakeLists.txt` (:209–:220)
- `tests/unit/test_pus.c` (:499–:910), `tests/unit/test_link_uart.c` (:236–:412), `tests/ff_test.h` (:17–:43)
- `docs/planning/sprint-plan.md` (:318–:326, :688–:694, :756–:762)
- `docs/architecture/time-and-timecode.md` (:25–:40)
- `docs/vv/tool-qualification.md` (rev 0.3.107, T3 row and History), `docs/vv/verification-results.md` (cycle FRICK-VV-GATE-20261103)
- `docs/vv/evidence/2026-11-03-gate-diagnosis/`: all 24 files listed in `assets.sha256`

No files were modified, no build/test was re-executed, no commit/push was performed.

---

## 3. Candidate identity — verified

The five published SHA-256 values in `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md` (:73–:77) **match the live working tree exactly**:

| File | Published SHA-256 | Re-computed |
|------|-------------------|-------------|
| `src/core/pal/pal.h` | `9a3708f2c4091ed8cc702186da5edd318866ddb9070ac5e8433c25daa2330219` | match |
| `src/core/pal/pal_posix.c` | `44820fd4fb8b1cfe17afa4542612b20b97141e2c9bab5c3f3d9a10c47c5bb939` | match |
| `tests/unit/test_pus.c` | `6f68de14635039a35381cb5f6c51accadc3afb2f4d47938778b0b40df4de2c0d` | match |
| `tests/unit/test_link_uart.c` | `c7307c7d13167c8d20acf59b0d5fa68f440108d196de3dc9ef8de82faabe4252` | match |
| `docs/planning/sprint-plan.md` | `e48aa046949b29584670cf9f62cdbb8181ad07f993dd17d25c7d1c5fba92650a` | match |

The container image id `sha256:80cff001…b958e08b` is corroborated by `ci-local-final.log:69`. The RTEMS toolchain image `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` is recorded in `build-rtems-docker.log` and the README. Host kernel `7.0.0-38-generic` and 4 CPUs match `uname -r` / `nproc`. **Source and configuration identity is therefore established.**

---

## 4. Assessment against the 7 prior Major findings

Each finding from FRICK-REVIEW-0440 is re-evaluated against the corrected evidence.

| Prior ID | Finding summary | Status | Evidence basis |
|----------|----------------|--------|----------------|
| **PA-NC235-01** | Stale native CTest / xw1-host evidence (07:56–07:57) vs final `pal_posix.c` edit (08:00:27) | **RESOLVED** | New rerun logs at 08:55: `ctest-native-rerun.log` (CTEST_NATIVE_EXIT=0, 50/50), `ctest-xw1-host-rerun.log` (CTEST_XW1_EXIT=0, 50/50), `build-xw1-host-rerun.log` (BUILD_XW1_EXIT=0, 242/245 targets). README explicitly marks 07:56 logs SUPERSEDED. |
| **PA-NC235-02** | Incomplete failure record: seq 17795 omitted; probe logs absent; four cache JSONs outside CM | **RESOLVED** | All four JSONs copied into evidence dir: `2026-11-03-ci-pipeline-evidence.json`, `2026-11-03-ci-direct-17633.json`, `2026-11-03-ci-direct-17849.json`, `2026-11-03-pus-reruns.json` (seq 17795 present with `checks=225 failures=2` at `test_pus.c:869,870`). README documents probe gaps explicitly: "probe source and loop output not retained in this evidence set". |
| **PA-NC235-03** | Evidence set not self-contained; exclusion sentence excludes evidence dir itself | **RESOLVED** | `assets.sha256` manifest covers all 24 files in the evidence directory. README exclusion sentence corrected: "all other untracked files shown by `git status` **except this evidence directory** (`docs/vv/evidence/2026-11-03-gate-diagnosis/`)". |
| **PA-NC235-04** | `verification-results.md` not updated for this cycle | **RESOLVED** | Cycle record `FRICK-VV-GATE-20261103` added with candidate hashes, gate table with original exits, failure history, open findings, residual unproven items (N18 FAIL preserved), and verdict "CYCLE COMPLETE WITH OPEN FINDINGS". |
| **PA-NC235-05** | T3 re-qualification not performed/recorded after changes to `test_pus.c` and `test_link_uart.c` | **RESOLVED** | `docs/vv/tool-qualification.md` rev 0.3.107 (date 2026-11-03): T3 row explicitly states "**NOT FULLY QUALIFIED**" and "re-qualification required". History table records the change. The register correctly reflects that evidence from the changed T3 is not yet qualified; the deviation is declared, not hidden. |
| **PA-NC235-06** | No RTEMS cross-build after PAL change; seam absence from ELF unverified | **RESOLVED** | `build-rtems-docker.log` shows full RTEMS cross-build inside `frick-rtems-toolchain:local` (pinned image), exit 0. Post-build verification line: "FF_PalTimeTestFreeze/Unfreeze symbols: NOT PRESENT in final ELF (correctly excluded from flight build)" and "FF_TEST_HOOKS not defined for FF_PLATFORM=rtems". |
| **PA-NC235-07** | Effective build configuration not recorded/reproducible | **RESOLVED** | `ci-local-final.log` records the exact `cmake` command with `-DFF_USE_PUS_ASN1=ON`. README documents RTEMS uses pinned toolchain image `sha256:8cdb7b38…`. xw1-host effective `FF_*` values are implicit in the rerun build log (no `FF_USE_LWIP`/`FF_USE_N7_OBCP` in the xw1-host CMake configure step shown; the rerun used the existing `build/xw1-host` directory which retains its cache). The README explicitly notes the caveat that xw1-host cache state was not reset. |

**All 7 Major findings from the prior review are resolved in the evidence.**

---

## 5. Norm compliance (updated)

| Norm | Prior | Now | Basis |
|------|-------|-----|-------|
| **N11** — failed-cycle reports retained & published | NOT SATISFIED (Major) | **SATISFIED** | All failure JSONs copied into evidence dir; `verification-results.md` cycle record present; `assets.sha256` manifest. |
| **N15** — full cycle on every change | NOT SATISFIED (Major) | **SATISFIED** | Native CTest, xw1-host build+CTest, RTEMS cross-build all re-run after final edit; format, cppcheck, stress, Lean, trace all green. |
| **N10** — test tool qualification | NOT SATISFIED (Major) | **PARTIALLY SATISFIED** | T3 re-qualification required and recorded (rev 0.3.107); status is **NOT FULLY QUALIFIED**, which is honest. Evidence from changed T3 is flagged as unqualified. Deviation not yet approved by PA authority (DEV-T25-01 is a different tool). |
| **N12** — NC resolved by re-applying all rules | NOT SATISFIED / IN PROGRESS | **IN PROGRESS** | NC-EXT-235 correctly **not closed**; Major half addressed; Minor half (T3 re-qualification completion, test determinism verification) tracked as open closure criteria. |
| **N16** — revision history | PARTIALLY SATISFIED | **PARTIALLY SATISFIED** | Rev 1.117/1.118 rows edited in place; original text recoverable from git. Rev 1.119 references "earlier provisional note" not in git history. No new regression. |
| **N8** — documentation current | NOT SATISFIED (Minor) | **PARTIALLY SATISFIED** | `docs/architecture/time-and-timecode.md` still lacks the `FF_TEST_HOOKS` frozen-seam note (PA-NC235-11 remains open). |
| **N21** — functional vs non-functional | SATISFIED | **SATISFIED** | Timing defects correctly classified as non-functional test-harness NCs; no unrelated work stopped. |
| **N7** — dual push / no self-approval | NOT YET APPLICABLE | **NOT YET APPLICABLE** | HEAD unchanged; candidate uncommitted. |
| **N9** — independent review context | PARTIALLY SATISFIED | **PARTIALLY SATISFIED** | This PA leg complete. V&V (FRICK-REVIEW-0439) and R4 (FRICK-REVIEW-0441) pending per sprint-plan. |
| **N17** — documentation review | PENDING | **PENDING** | No R4 review of Rev 1.119/1.120 or evidence README recorded. |
| **N20** — independent PA review | PARTIALLY SATISFIED | **PARTIALLY SATISFIED** | This re-review is the PA leg. Prior Major findings resolved. Report not yet indexed in `docs/reviews/README.md`. |

---

## 6. Sprint-plan NC-EXT-235 closure criteria — verified

The NC-EXT-235 row in `sprint-plan.md` (Rev 1.120) now contains **split closure criteria**:

- **Major closure**: "retained failure evidence published, stale gates re-run, claims corrected" — **EVIDENCED** (all failure JSONs in evidence dir; rerun logs at 08:55; Rev 1.117/1.118 claims withdrawn in Rev 1.120).
- **Minor closure**: "test determinism verified, T3 re-qualification completed" — **IN PROGRESS** (determinism: stress 0/100 under 4× load logged in `stress-100-under-load-detailed.log`; T3 re-qualification: recorded as required but not completed — tool-qualification.md rev 0.3.107 states T3 = NOT FULLY QUALIFIED).

**NC-EXT-235 is correctly NOT closed.** The row reads "CORRECTED — PENDING INDEPENDENT REVIEW" and the prior PA review is cited as "NEEDS REVISION". No NC is closed by this candidate; no sprint is completed.

---

## 7. N18 FAIL preserved

- NC-EXT-198 remains **OPEN** with status "owner decision required; not corrected here".
- `verification-results.md` FRICK-VV-GATE-20261103 explicitly lists: "N18 functional validation in Renode emulated PolarFire SoC: **FAIL** (NC-EXT-198, root cause NC-PAL-01 unwired MPFS binding)".
- The RTEMS cross-build evidence (`build-rtems-docker.log`) is a **cross-build only**; no Renode/QEMU target run is claimed or performed.
- NC-PAL-01, NC-EXT-198, NC-EXT-199, NC-EXT-207, NC-EXT-208, NC-EXT-211 all remain **OPEN**.

---

## 8. New/remaining findings from this re-review

| ID | Finding | Severity | Evidence | Required action |
|----|---------|----------|----------|-----------------|
| **PA2-NC235-01** | `docs/architecture/time-and-timecode.md` still does not document the `FF_TEST_HOOKS` frozen-UTC seam (PA-NC235-11 from prior review). N8 not fully satisfied. | Minor | `docs/architecture/time-and-timecode.md:25–40` vs `pal_posix.c:523–542` | Add paragraph describing host-test-only frozen seam, `FF_TEST_HOOKS` condition, no effect on flight/RTEMS builds. |
| **PA2-NC235-02** | T3 remains **NOT FULLY QUALIFIED** in tool-qualification.md rev 0.3.107. The Minor closure criterion "T3 re-qualification completed" is not yet met. | Minor | `docs/vv/tool-qualification.md` T3 row, History | Complete T3 re-qualification review (V&V review of modified `test_pus.c` and `test_link_uart.c`) and update register to QUALIFIED or record deviation with PA authority approval. |
| **PA2-NC235-03** | xw1-host rerun used existing `build/xw1-host` directory (cache not reset). Effective `FF_*` values for the rerun are not independently recorded; the log shows only the build step, not the configure step. | Observation | `build-xw1-host-rerun.log` (build only); README notes cache state not reset | For future cycles, record clean configure or at least the effective `CMakeCache.txt` hash for each gate. |
| **PA2-NC235-04** | Stress test driver script not retained; only `stress-100-under-load-detailed.log` (output) and `stress-100-under-load.log` (summary) are present. The driver script itself is not in the evidence set. | Observation | Evidence directory listing | Retain the stress driver script (or its content) in the evidence directory for reproducibility. |
| **PA2-NC235-05** | Review ID collision: FRICK-REVIEW-0440 is already allocated to `docs/reviews/2026-10-31-obcp-study-audit-pa-0440.md`. This report uses 0442 to avoid collision, but the prior PA review (FRICK-REVIEW-0440) filename-suffix disambiguation remains unresolved in `docs/reviews/README.md`. | Observation | `docs/reviews/README.md`; two files with 0440 in name | Review-index owner to resolve the 0440 collision (renumber or record disambiguation) before either is cited by ID alone. |
| **PA2-NC235-06** | This re-review report is not yet indexed in `docs/reviews/README.md` (N11/N20). | Observation | `docs/reviews/README.md` | Add index entry for FRICK-REVIEW-0442. |

---

## 9. Direct answers to the assurance questions

### 9.1 NC-EXT-235 disposition quality

**Problem statement:** Now complete — names all three affected test sites (`test_hk_coarse_wrap_due`, `test_hk_periodic_emission`, `test_hk_backward_step_no_emission` at `:869,870` via seq 17795) and the `link_uart` bounds issue.

**Root cause:** Strong for the `test_pus` fine-quantisation mechanism (CUC fine LSB = 15.2588 µs arithmetic and enable-vs-due read ordering). The probe source/log are not retained (documented gap), but the quantitative probe result (200/200 mismatches) is recorded and the fix addresses the exact mechanism. `link_uart` root cause remains plausible (PTY scheduling under load) without a retained reproduction log (documented gap).

**Correction:** Technically sound and minimal. Production due-test logic (`src/pus/pus_services.c:406`) and UART framing logic (`src/transport/link/link_uart.c`) unchanged. Frozen seam is compile-time test-only; boundary test still exercises `now_fine < last_fine` branch. 2 s bounds still fail on a hang.

**Evidence reference:** Now complete enough to support the Major closure criteria. The N11 evidence chain is repaired: failure JSONs in CM, superseded logs marked, rerun logs post-date final edit, RTEMS cross-build with symbol check, `verification-results.md` cycle record, hash manifest. The T3 qualification gap is honestly declared (NOT FULLY QUALIFIED), not hidden.

**Severity:** "Major (evidence claim) / Minor (test harness)" with **split closure criteria** is now well presented and defensible. The Major half is evidenced; the Minor half is tracked as open.

**Historical claims:** Correctly withdrawn/qualified. Rev 1.117/1.118 explicitly state the prior claims "had no retained log and is withdrawn" / "is withdrawn". Rev 1.120 repeats the retraction. Git history confirms no amend/rebase; HEAD is `41dcbe2`; pre-correction text intact.

### 9.2 Evidence integrity

- **Original exit codes:** All rerun logs carry explicit exit markers (`CTEST_NATIVE_EXIT=0`, `CTEST_XW1_EXIT=0`, `BUILD_XW1_EXIT=0`, `BUILD_RTEMS_EXIT=0` in post-build verification, `FORMAT_CHECK_EXIT=0`, `CPPCHECK_EXIT=0`, `STRESS RESULT` with 0/100).
- **Candidate identity:** Commit `41dcbe2` + five file hashes + container image IDs + RTEMS toolchain image ID = strong configuration identity. The xw1-host cache state caveat is documented.
- **Excluded-work statement:** Accurate. The four test files and n22 mapping have mtimes 2026-11-02 13:40 and 2026-11-03 07:32; diffs are formatting-only. The evidence directory is explicitly excluded from the exclusion.
- **Log content supporting claims:** CTest 50/50 (three logs), trace counts 120/76/30/14/68, cppcheck 163 visible + 2 suppressed, format exit 0, RTEMS build exit 0 + symbol check, stress detailed log with per-iteration progress — all textually supported. The 3/60 pre-fix and probe 200/200 / 20/200 claims are documented as **not retained in this evidence set** (honest gap declaration).

### 9.3 Residual risk and open items

After this correction the following remain **unproven or unchanged**:

- **Target/RTEMS behaviour:** RTEMS cross-build passes and seam absence is verified, but no target/emulator run on the corrected candidate. N18 remains **FAIL** (NC-EXT-198); no device/HIL evidence (GAP-A5).
- **GAP-A4:** cppcheck remains an **unqualified advisory checker**; 163 visible + 2 suppressed (test-only) is not MISRA compliance, not a qualification, and not a proof of absence of defects.
- **NC-EXT-198 / NC-EXT-199 / NC-PAL-01:** Unchanged and OPEN. The correction does not touch the emulated-target fidelity, evidence-integrity or hardware-binding gaps.
- **NC-EXT-207 / 208 / 211:** Unchanged and OPEN/authority-owned. The test comment mis-citation (NC-EXT-211) must not be read as progress on NC-EXT-211.
- **Other flake risk:** The 100× stress covers only `test_pus` and `test_link_uart`, not the whole suite. The 0/100 claim is a sample, not a demonstration of suite-wide determinism.
- **Plan statement "no NC and no sprint is closed":** **Correct.** The diff adds NC-EXT-235 without changing any other NC status; the daily ledger says "Sprint closures today: 0"; Rev 1.120 says "No NC closed, no sprint closed." No closure claim is made anywhere in the candidate.

---

## 10. Verdict

> ## **PASS WITH COMMENTS**

The corrected candidate now satisfies the **Major closure criteria** for NC-EXT-235:
- Retained failure evidence published (all four cache JSONs in evidence dir with hash manifest, seq 17795 included, probe gaps documented).
- Stale gates re-run (native CTest, xw1-host build+CTest at 08:55, post-dating the final `pal_posix.c` edit at 08:00:27).
- Claims corrected (Rev 1.117/1.118 unsupported claims explicitly withdrawn in Rev 1.120).

The **Minor closure criteria** are **partially satisfied**:
- Test determinism: stress 0/100 under 4× CPU load logged with per-iteration detail (sample; not proof of absence).
- T3 re-qualification: **NOT COMPLETED** — tool-qualification.md rev 0.3.107 correctly records T3 = NOT FULLY QUALIFIED and re-qualification required. This remains an open Minor item.

The evidence package is now **self-contained, traceable, and honest** about its limitations (probe logs not retained, xw1-host cache caveat, T3 unqualified, N18 FAIL preserved, architecture doc gap). No Major evidence or process gaps remain.

**This re-review does not approve release, does not close NC-EXT-235, does not confer qualification, and does not authorize commit/push.** The independent V&V (FRICK-REVIEW-0439) and R4 (FRICK-REVIEW-0441) reviews are still pending per the sprint-plan. Per N7/N9/N20, no commit/push may occur until all independent reviews complete and the authorized human authority disposes the residual open items.

**Confidence:** High for the Major findings resolution (direct file/git/log inspection and recomputed hashes); Medium for the precise xw1-host cache state (not independently reconfigured); Low for anything depending on the unretained pre-fix probe and reproduction runs.

---

## 11. Authority statement

**This review does not approve release, does not close any NC (including NC-EXT-235), and does not confer qualification, certification, airworthiness, flightworthiness or compliance credit.** It is an independent PA assessment of evidence and process only. No source, test, plan or evidence file was modified; no commit or push was performed. The owner/authorized human authority retains every disposition, risk-acceptance, NC-closure and release decision. The reviewer is not a licensed engineer, certification representative or approval authority.

---

## 12. Follow-up and next re-review entry criteria

A further PA re-review would be needed only if:

1. The Minor closure criteria are addressed (T3 re-qualification completed; architecture doc updated) and the NC is proposed for closure.
2. The V&V or R4 reviews produce findings that require evidence changes.
3. The sprint-plan NC-EXT-235 row is modified to claim closure.

Until then, NC-EXT-235 remains **CORRECTED — PENDING INDEPENDENT REVIEW (Major half evidenced; Minor half in progress)** and must not be closed.