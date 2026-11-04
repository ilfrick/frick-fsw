# Independent PA Review — NC-EXT-235 "corrected candidate" (2026-11-03 gate failure)

|                                   |                                                                                          |
| --------------------------------: | :--------------------------------------------------------------------------------------- |
| Document id                       | **FRICK-REVIEW-0440** (see PA-NC235-17: `0440` was already allocated to `docs/reviews/2026-10-31-obcp-study-audit-pa-0440.md`; the filename suffix disambiguates this report, but the ID collision must be resolved by the review-index owner) |
| Reviewer profile                  | **PA — Senior SW Product Assurance** (N20), independent sub-agent, fresh context          |
| Reviewer session                  | sub-agent session spawned by the coordinator; no author context inherited                  |
| Reference revision                | `HEAD = 41dcbe21435e14b5aaff06c63d9df6f0de04df54` **plus** the working-tree candidate set  |
| Date                              | 2026-11-03                                                                                |
| Items under review                | NC-EXT-235 disposition; corrected-candidate evidence; `pal.h`/`pal_posix.c` test seam; test edits; sprint-plan Rev 1.117–1.120; evidence directory |
| Independence declaration          | I did not author, edit, build, test or commit any item under review. I inspected the live tree, git history and the named logs, and recomputed the published file hashes. I did not execute the test suite. |
| Norms applied                     | N1–N22 (focused: N7, N8, N9, N10, N11, N12, N15, N16, N17, N20, N21)                       |

---

## 1. Scope

**In scope.** The candidate defined as `41dcbe2` plus exactly:

- `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` (frozen-UTC seam under `FF_TEST_HOOKS`)
- `tests/unit/test_pus.c` (HK due tests freeze UTC)
- `tests/unit/test_link_uart.c` (positive PTY receive bounds 20 ms / 500 ms → 2 s)
- `docs/planning/sprint-plan.md` (Rev 1.117/1.118 corrections, Rev 1.119 failure record, Rev 1.120 verification record, NC-EXT-235)
- `docs/vv/evidence/2026-11-03-gate-diagnosis/` (new evidence set)

**Out of scope, and to remain unchanged.** `tests/unit/test_pus_n7_adapter.c`, `test_pusopen.c`, `test_r2_gap.c`, `test_sched_max_entries.c`, `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`, and all other untracked files.

**Not a test-design review.** Test design observations are recorded only where they affect whether the evidence can support the assurance claim.

---

## 2. Artifacts examined (exact paths and commands)

Git and working tree:

- `git rev-parse HEAD`, `git status --porcelain=v1`, `git log --oneline -8`
- `git diff` for all ten modified tracked files; `git show HEAD:docs/planning/sprint-plan.md` (pre-correction text)
- `git log -p --all -S "1.119" -- docs/planning/sprint-plan.md`; `git log --all -S "cach" -- docs/planning/sprint-plan.md`
- `sha256sum` on the five files in the evidence manifest (all five matched the README, §3)
- `stat -c '%y %n'` on the candidate files, the four excluded test files, and every evidence log
- `grep -E "FF_USE_LWIP|FF_USE_N7_OBCP|FF_USE_PUS_ASN1|FF_BUILD_TESTS|FF_PLATFORM" build/{ci,native,xw1-host}/CMakeCache.txt`

Candidate content:

- `src/core/pal/pal.h` (:71–:87), `src/core/pal/pal_posix.c` (:498–:542), `CMakeLists.txt` (:209–:220)
- `tests/unit/test_pus.c` (:499–:910), `tests/unit/test_link_uart.c` (:236–:412), `tests/ff_test.h` (:17–:43)
- `docs/planning/sprint-plan.md` (:318–:326, :688–:694, :756–:762)
- `docs/architecture/time-and-timecode.md` (:25–:40)
- `docs/vv/tool-qualification.md` (:7, :26, History), `docs/vv/verification-results.md` (no 2026-11-03 entry)
- `docs/vv/evidence/2026-11-03-gate-diagnosis/`: `README.md`, `ci-local-final.log`, `ci-local-after-fix.log`, `ctest-native.log`, `ctest-xw1-host.log`, `build-native.log`, `build-xw1-host.log`, `format-check.log`, `cppcheck.log`, `cppcheck.txt`, `stress-100-under-load.log`
- `docs/vv/evidence/2026-11-02-cppcheck-canonical-final/README.md`, `cppcheck_report_20261102_final.txt` (baseline 163)
- Cache evidence cited by the candidate but **outside the repository**: `~/.hermes/profiles/jeeves/cache/fsw/2026-11-03-ci-pipeline-evidence.json`, `2026-11-03-ci-direct-17633.json`, `2026-11-03-ci-direct-17849.json`, `2026-11-03-pus-reruns.json`
- `docs/reviews/2026-10-31-obcp-study-audit-pa-0440.md` (prior allocation of ID 0440)

No files were modified, no build/test was re-executed, no commit/push was performed.

---

## 3. Candidate identity — verified

The five published SHA-256 values in `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md` (:58–:64) **match the live working tree exactly**:

| File | Published SHA-256 | Re-computed |
|---|---|---|
| `src/core/pal/pal.h` | `9a3708f2…2330219` | match |
| `src/core/pal/pal_posix.c` | `44820fd4…5bb939` | match |
| `tests/unit/test_pus.c` | `6f68de14…de2c0d` | match |
| `tests/unit/test_link_uart.c` | `c7307c7d…abe4252` | match |
| `docs/planning/sprint-plan.md` | `e48aa046…92650a` | match |

The container image id `sha256:80cff001…b958e08b` is corroborated by `ci-local-final.log:69` (image build) and matches the canonical cppcheck evidence. Host kernel `7.0.0-38-generic` and 4 CPUs match `uname -r` / `nproc`. **Source identity is therefore established; build configuration and execution time are not** (PA-NC235-07, PA-NC235-01).

---

## 4. Assessment against the applicable norms

| Norm | Result | Basis |
|---|---|---|
| **N11** — failed-cycle reports retained and published | **NOT SATISFIED (Major)** | The pre-correction failure records live only in `~/.hermes/...` (outside CM); the evidence dir cites three JSON filenames that are not present in it; `time_probe.c` and its log, the `3/60`, `200/200` and `20/200` records are not retained; an observed `test_pus.c:869,870` failure is omitted from the published record. See PA-NC235-02/03/08. |
| **N15** — full cycle on every change | **NOT SATISFIED (Major)** | After the last edit to `src/core/pal/pal_posix.c` (mtime 08:00:27), only `make ci-local` was rerun (log 08:02:34). Native CTest and xw1-host build/CTest logs are 07:56–07:57, i.e. pre-edit. No cross/RTEMS build was run at all. See PA-NC235-01/06/07. |
| **N16** — revision history | **PARTIALLY SATISFIED** | Rev 1.119 and 1.120 rows were added and the revision header advanced to 1.120. However, Rev 1.117/1.118 rows were edited in place and the original claim wording removed from those rows; the "earlier provisional note" that Rev 1.119 says it replaces is not identifiable in git history. The original text is still recoverable from `41dcbe2`/`e5fdc5d`, so the failure is **not hidden**, but the controlled record is not point-in-time immutable. See PA-NC235-14. |
| **N12** — NC resolved by re-applying all rules | **NOT SATISFIED / IN PROGRESS** | NC-EXT-235 is correctly **not closed**, but the correction does not yet re-apply all affected rules: N11 (`verification-results.md`), N10 (T3 re-qualification), N8 (`time-and-timecode.md`) and N15 (re-run after final edit / cross build) are incomplete. See PA-NC235-04/05/11. |
| **N21** — functional vs non-functional handling | **SATISFIED for classification and registration** | The two timing defects are non-functional test-evidence defects; they are registered with impact, owner and disposition; no unrelated work was stopped; the functional production logic was not changed. N21 does **not** waive N11/N15/N10. |
| **N7** — dual push / no self-approval | **NOT YET APPLICABLE (no violation observed)** | HEAD is unchanged and the candidate is uncommitted; no commit/push was performed before independent review. Correct behaviour. |
| **N9** — independent review context | **PARTIALLY SATISFIED** | This PA leg is fresh-context and independent. The V&V and R4 legs are still pending; the plan says so. |
| **N17** — documentation review | **PENDING** | No R4 review of Rev 1.119/1.120 or the evidence README is recorded. |
| **N20** — independent PA review | **PARTIALLY SATISFIED** | This report is the PA leg. Because it carries Major findings, a corrected-content re-review is required before commit; the report is not yet indexed in `docs/reviews/README.md` (PA-NC235-18). |
| **N10** — test tool qualification | **NOT SATISFIED (Major)** | T3 covers `tests/unit/*.c` including `test_pus.c` and `test_link_uart.c` (`docs/vv/tool-qualification.md:26`). Both changed; T3's qualification basis ("reviewed + CM") is therefore invalidated and the register (rev 0.3.106) was not updated. See PA-NC235-05. |
| **N8** — documentation current | **NOT SATISFIED (Minor)** | `docs/architecture/time-and-timecode.md:30–32` still states the unconditional formula `FF_PalGetUtcNs = monotonic + g_utc_offset_ns`; it does not mention the `FF_TEST_HOOKS` frozen path. See PA-NC235-11. |

---

## 5. Findings

Severity scale: **Blocker** = cannot proceed at all; **Major** = substantive non-conformity / evidence gap that must be corrected and re-reviewed before commit or NC closure; **Minor** = must be corrected or justified; **Observation** = advisory, no derogation.

### Major

| ID | Finding | Evidence | Required action |
|---|---|---|---|
| **PA-NC235-01** | **Per-gate evidence is stale relative to the final candidate.** The README and Rev 1.120 present native CTest 50/50 and xw1-host build/CTest 50/50 as evidence for the candidate identified by the five hashes. The last edit to `src/core/pal/pal_posix.c` was at **08:00:27**; the xw1-host objects are **07:56:44–07:56:52**, `build-xw1-host.log` is **07:56:53**, `ctest-native.log` is **07:56:55**, `ctest-xw1-host.log` is **07:57:20**. Those gates therefore ran against a different file content. Only `ci-local-final.log` (08:02:34), `cppcheck.*` (08:00:55), `format-check.log` (08:00:44) and the stress log (08:03:20) post-date the edit. The README contains no timestamps, so the mismatch cannot be excluded from the documents alone. | `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md:46–56`; filesystem mtimes of the listed logs and of `build/xw1-host/CMakeFiles/frick_fsw_core.dir/src/core/pal/pal_posix.c.o`; `sprint-plan.md:694` ("xw1-host is evidenced **for this corrected candidate only**") | Re-run native CTest and the xw1-host build + CTest **after** the final edit, archive the new logs, and either replace the claims or explicitly mark the 07:56 logs SUPERSEDED. Do not claim xw1-host evidence for this candidate until this is done. |
| **PA-NC235-02** | **The published failure record is incomplete and part of its reproduction evidence is unretained.** (a) `2026-11-03-pus-reruns.json` seq 17795 records a real `test_pus` failure at `tests/unit/test_pus.c:869,870` (`FF_PusEmitPeriodicHk() == FF_SUCCESS`, `stream_count(...) == 1u`), `checks=225 failures=2` — the forward-again boundary assertion of `test_hk_backward_step_no_emission` at the pre-fix line numbering (`git show HEAD:tests/unit/test_pus.c:869–870`). This failure is absent from the README table and from Rev 1.119, and from the NC-EXT-235 problem statement, which names only `test_hk_coarse_wrap_due`. (b) The cited "3/60" pre-fix reproduction is not in any retained artifact; the cited `2026-11-03-pus-reruns.json` contains a single 2-failure run, not 3/60. (c) "200/200" probe mismatches cite `time_probe.c`, which is not in the repository, evidence directory or cache; no probe log exists. (d) The `link_uart` "20/200" reproduction log is likewise absent. | `~/.hermes/profiles/jeeves/cache/fsw/2026-11-03-pus-reruns.json` (seq 17795); `README.md:9–27`; `sprint-plan.md:322, :693`; `git show HEAD:tests/unit/test_pus.c:866–870`; no `time_probe.c` under `/home/nicola/fsw/frick-fsw` | Publish the complete failure set, including the 17795 run and the third test site, and state the failure mode honestly. Archive the probe source, the probe log, the 3/60 loop output and the 20/200 loop output in `docs/vv/evidence/2026-11-03-gate-diagnosis/`, or withdraw the quantitative claims. Correct NC-EXT-235 item (1) to name every affected test. |
| **PA-NC235-03** | **The evidence set is not self-contained, and the exclusion statement excludes the evidence set itself.** The README's "Evidence" column points to `2026-11-03-ci-pipeline-evidence.json`, `2026-11-03-ci-direct-17633.json`, `2026-11-03-ci-direct-17849.json`; none exists in `docs/vv/evidence/2026-11-03-gate-diagnosis/` — they exist only at `~/.hermes/profiles/jeeves/cache/fsw/`, outside the repository and outside CM. Rev 1.119 cites the same cache paths plus `2026-11-03-pus-reruns.json`. Separately, README:66–69 says the excluded set includes "the untracked files shown by `git status`" — but `docs/vv/evidence/2026-11-03-gate-diagnosis/` is itself untracked, so the sentence literally excludes the evidence directory from the candidate it is describing. There is also no `assets.sha256`/attestation for the evidence files (contrast `docs/vv/evidence/2026-11-02-cppcheck-canonical-final/`). | `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md:9–13, :58–69`; directory listing (no JSON files); `sprint-plan.md:693`; `docs/vv/evidence/2026-11-02-cppcheck-canonical-final/` contents | Copy the four JSON records into the evidence directory (or mark them unavailable and remove the citation), add a hash manifest for every evidence file, and rewrite the exclusion sentence as "all other untracked files, excluding this evidence directory". List the evidence directory explicitly in the candidate definition. |
| **PA-NC235-04** | **`docs/vv/verification-results.md` was not updated for this cycle.** N11 requires verification results to be recorded there and updated in every cycle; N15 repeats it. The file's last section is the 2026-11-02 XW-2 target-contract record; there is no 2026-11-03 gate-diagnosis/correction entry, no candidate identity, no gate results and no link to the new evidence directory. | `docs/rules.md:158–175` (N11), `:254–270` (N15); `docs/vv/verification-results.md` (no 2026-11-03 entry; tail ends at the XW-2 verdict) | Add the 2026-11-03 cycle record: candidate hashes, every gate with original exit, the failure history, the open findings of this review and the fact that N18 remains FAIL. |
| **PA-NC235-05** | **N10 T3 re-qualification was not performed or recorded.** T3 covers `tests/unit/*.c`, explicitly including `test_pus.c` and `test_link_uart.c`, with qualification basis "reviewed + CM". Both files were changed in this candidate. The tool register is still rev 0.3.106 with no History entry for this change. N10 states that any change to a recorded V&V tool invalidates its qualification and forces a re-qualification review before evidence reuse. | `docs/vv/tool-qualification.md:7, :26` (T3 row), History table; `docs/rules.md:145–156` (N10) | Record the T3 change and requalification: V&V review of the modified tests, register History entry, and the review reference. Until then, the 50/50 result is evidence from a changed, not-yet-requalified T3. |
| **PA-NC235-06** | **No cross/RTEMS build after a change to a shared PAL header/source.** N15 requires the complete cycle including the cross proxy. `src/core/pal/pal.h` and `pal_posix.c` changed; the only host builds recorded are native/ci/xw1-host. The claim "absent from flight/RTEMS builds" rests on inspection of `CMakeLists.txt:217–220` only; there is no RTEMS build log and no `nm`/preprocessor proof that the two symbols are absent from an RTEMS object. | `CMakeLists.txt:217–220`; evidence directory (no `build-rtems` log); `README.md:33–36`; `sprint-plan.md:694` | Run the RTEMS cross build (`make rtems` or the recorded proxy) on the corrected candidate and retain the log; add a symbol-absence check (`nm` on `pal_posix`/`pal_rtems` object or preprocessed output) to substantiate the "absent from flight" statement. |
| **PA-NC235-07** | **The effective build configuration of each gate is not recorded and is not reproducible from the documented command.** `make ci-local` configures `build/ci` with only `-DFF_USE_PUS_ASN1=ON`, but the live cache has `FF_USE_LWIP:BOOL=ON`, `FF_USE_N7_OBCP:BOOL=ON`, `FF_USE_PUS_ASN1:BOOL=ON` inherited from earlier configurations. A clean checkout running the documented command would therefore build a different feature set from the one that produced `ci-local-final.log`. The xw1-host configure command is not recorded at all (only `cmake --build build/xw1-host -j`). The logs carry no start/end timestamp, host, or command line. | `Makefile:109–117`; `build/ci/CMakeCache.txt`, `build/native/CMakeCache.txt`, `build/xw1-host/CMakeCache.txt`; `README.md:46–56`; no timestamps in any log (`grep` for `2026-11-03`/`Txx:xx` finds none) | For each gate, record the exact configure command, the resulting `CMakeCache.txt` hash (or the effective `FF_*` values), start/end timestamps, host and image ID in the log or a sidecar. Re-run from a clean unused build directory (or `--fresh`) so the result is reproducible. |

### Minor

| ID | Finding | Evidence | Required action |
|---|---|---|---|
| **PA-NC235-08** | **The stress log does not substantiate the 0/100 claim.** `stress-100-under-load.log` is a single line: `candidate build/ci (post-fix) under 4x CPU load: link_uart=0/100 pus=0/100`. It contains no command, no loop count per binary, no exit code, no timestamps, no burner identity and no per-iteration output. This is an assertion, not a log. | `docs/vv/evidence/2026-11-03-gate-diagnosis/stress-100-under-load.log`; `README.md:56`; `sprint-plan.md:694` | Retain the stress driver script and its full output (or at least per-run PASS/FAIL lines and the final exit), and state that 0/100 is a sample with a confidence limit, not proof of absence of flakiness. |
| **PA-NC235-09** | **`ci-local-after-fix.log` is stale and unlabelled.** It is 07:51:25, i.e. before the `test_link_uart.c` edit (07:53:30) and before the `pal_posix.c` edit (08:00:27). It is not referenced in the README post-correction table and has no `SUPERSEDED` marker. A reader can mistake it for a second final run. | file mtimes; absence from `README.md:46–56` | Mark it `SUPERSEDED — intermediate run 07:51, before the 07:53/08:00 edits; not evidence for the candidate` or move it out of the evidence set. |
| **PA-NC235-10** | **Wrong NC citation in the test comments.** Both freeze comments cite `NC-EXT-211`, which is the unrelated latent concurrency NC (`svc3_create`/`svc3_enable` ordering). The change belongs to NC-EXT-235 (and the boundary lineage is NC-EXT-207). `pal.h:81` correctly cites `NC-EXT-207`, so the two source comments contradict the header. | `tests/unit/test_pus.c:513–517, :701–704`; `sprint-plan.md:334` (NC-EXT-211 definition); `pal.h:80–82` | Cite `NC-EXT-235` for the freeze rationale (optionally `NC-EXT-207` for the boundary requirement) and explicitly note that NC-EXT-211 is unaffected. |
| **PA-NC235-11** | **Architecture documentation is no longer unconditionally true.** `docs/architecture/time-and-timecode.md:30–32` states `FF_PalGetUtcNs = monotonic + g_utc_offset_ns` and that `FF_PalSetUtcNs` sets the offset. Under `FF_TEST_HOOKS`, `FF_PalGetUtcNs` returns the frozen value and `FF_PalSetUtcNs` does not write the offset. This is a test-only path, but N8 requires the affected documentation to stay current. | `docs/architecture/time-and-timecode.md:25–40`; `pal_posix.c:523–542`; `CMakeLists.txt:217–220` | Add one paragraph: host-test-only frozen seam, `FF_TEST_HOOKS` condition, no effect on flight/RTEMS builds, and the fact that `FF_PalSetUtcNs` is a no-op on the offset while frozen. |
| **PA-NC235-12** | **NC-EXT-235 is not yet closeable and its closure criteria are ambiguous.** The row combines a Major evidence-claim NC with a Minor test-harness NC under one "Major / Minor" label, which can be read as "Minor overall". It states "CORRECTED — PENDING INDEPENDENT REVIEW" but does not enumerate what must be true to close the Major half (retained failure evidence, re-run identity, claim corrections) versus the Minor half (test determinism, N10 requalification). The problem statement is also incomplete (PA-NC235-02). | `sprint-plan.md:322` | Split the closure account in the row: (i) Major — failure evidence retained and published, stale gates re-run, claims corrected; (ii) Minor — determinism and N10 requalification. Add the closure evidence list and the independent re-review reference. Consider splitting into NC-EXT-235 (evidence claim) and NC-EXT-236 (test harness) per N12. |
| **PA-NC235-13** | **The cppcheck suppression is acceptable but not transparent in the evidence.** The inline `// cppcheck-suppress unusedFunction` is justified in-comment, is confined to the `FF_TEST_HOOKS` block, and `--inline-suppr` is pinned, so the mechanism is sound. The count 163 is correct and the finding set is identical to the 2026-11-02 canonical-final baseline modulo line shifts — but only because two new findings are suppressed. `cppcheck.txt` does not show that two findings were suppressed, and neither does `README.md:55` beyond the generic statement in Rev 1.120. | `pal_posix.c:511, :517`; `tools/scripts/cppcheck_scan.sh` (pinned `--inline-suppr`); `docs/vv/evidence/2026-11-02-cppcheck-canonical-final/cppcheck_report_20261102_final.txt` (163); diff of the two reports (only line-number shifts) | Record the suppressed inventory (file, line, id, reason) in the cppcheck evidence and in the GAP-A4 record; state "163 visible + 2 suppressed (test-only)" rather than "163 unchanged". Alternatively exclude the test-only symbols via a `--suppress` file so the production source carries no suppression comment. |
| **PA-NC235-14** | **Controlled history rows were rewritten rather than superseded in place.** The Rev 1.117 and 1.118 rows were edited: the original "All gates PASS: `make ci-local`, ctest 50/50 native + 50/50 xw1-host, …" and "All gates PASS: …" text is gone from the current rows and replaced with qualified wording. The correction is explicit ("CORRECTED 2026-11-03", "withdrawn"), so it is **not hidden** — the original is recoverable from `41dcbe2`/`e5fdc5d` and the diff shows the deletions — but the revision rows are no longer point-in-time records. Rev 1.119 also says it "replaces the earlier provisional note"; no earlier 1.119 row exists in any git history, so that reference is unverifiable. | `git diff docs/planning/sprint-plan.md` (removed lines for 1.117/1.118); `git show HEAD:docs/planning/sprint-plan.md:690–691`; `git log -p --all -S "1.119"` (no result) | Preserve the superseded claim verbatim in a quoted block (or cite the exact commit/revision where it was published) and append the correction; identify or remove the "earlier provisional note" reference. |
| **PA-NC235-15** | **The frozen seam is global state with no failure-safe teardown.** `FF_PalTimeTestFreeze` sets two file-scope globals; `FF_TEST_ASSERT` increments a counter and continues (it does not abort). If any assertion fails between freeze and unfreeze, the clock stays frozen for every subsequent test in the same process. A later `FF_TimeSetUtcNs` then only updates the frozen value and never restores the offset, so the UTC epoch after unfreeze is whatever it was before the test. | `tests/ff_test.h:17–43`; `test_pus.c:517, :657, :704, :797, :838, :904`; `pal_posix.c:504–542` | V&V to confirm test isolation/order independence; preferably use a fixture with guaranteed teardown (or re-freeze/re-init per test) rather than relying on assertions not failing. Record the limitation if kept. |

### Observations

| ID | Observation | Evidence | Recommendation |
|---|---|---|---|
| **PA-NC235-16** | `FF_TEST_HOOKS` is `PUBLIC` on `frick_fsw_core` and keyed to `FF_BUILD_TESTS AND NOT FF_PLATFORM MATCHES "^rtems"`. This is sufficient for the stated host/RTEMS split, but a host "flight-shaped" build with tests enabled would also carry the seam. The test calls are not themselves `#ifdef`-guarded, so a hypothetical RTEMS test build would fail to compile. | `CMakeLists.txt:217–220`; `test_pus.c:517` etc.; `pal.h:76–87` | Consider a dedicated `FF_ENABLE_TEST_HOOKS` option defaulting to OFF, or an interface/PRIVATE definition plus a test-only support TU. Not required for this candidate, but reduces the chance of the seam leaking into a future non-RTEMS flight build. |
| **PA-NC235-17** | Review ID `0440` is already allocated to `docs/reviews/2026-10-31-obcp-study-audit-pa-0440.md` (FRICK-REVIEW-0440). This report is being written under the filename mandated by the task; the ID is duplicated. The review index already documents two prior collision precedents. | `docs/reviews/2026-10-31-obcp-study-audit-pa-0440.md:1`; `docs/reviews/README.md` (0378 collision note) | Review-index owner to renumber one of the two (or record the collision and the disambiguating filename) before either is cited by ID. No technical content is affected. |
| **PA-NC235-18** | This report is not yet indexed in `docs/reviews/README.md`, and the evidence directory has no file-hash manifest. | `docs/reviews/README.md`; evidence directory listing | Add the index entry (N11/N20) and an `assets.sha256` covering the evidence files. |
| **PA-NC235-19** | Two claim details are not supported by the retained log text: "clang-format 18" (the log shows the command and `FORMAT_CHECK_EXIT=0`, not the version) and "245 targets" (the xw1 build log shows `[242/245]` plus the exit, not a stated target count). | `format-check.log`; `build-xw1-host.log` | Either add the version/target count to the log sidecar or drop the specific numbers. |
| **PA-NC235-20** | The corrected test still exercises the full boundary (including `now_fine < last_fine` at `test_pus.c:758`), so the freeze does not remove the tie-break coverage — a positive observation. No mutation/discrimination run is recorded for the freeze change itself (e.g. remove the freeze and confirm the old flake returns); the "pre-fix 3/60" claim is the intended discrimination evidence but is unretained (PA-NC235-02). | `test_pus.c:713–790`; `README.md:23–27` | Add the freeze-removal negative control to the V&V requalification, or rely on the retained pre-fix reproduction once archived. |

---

## 6. Direct answers to the assurance questions

### 6.1 NC-EXT-235 disposition quality

**Problem statement:** adequate in outline but incomplete. It names `test_hk_coarse_wrap_due` only; the observed failures include `test_hk_periodic_emission` (`:561..574` class) and `test_hk_backward_step_no_emission` (`:869,870`), both of which the correction also freezes. The `link_uart` item is stated as "tight bounds under load"; the root cause is not analysed beyond that, though for a PTY test under 4× CPU load it is plausible and the positive assertions are now 2 s.

**Root cause:** strong for the `test_pus` fine-quantisation mechanism (the 15.2588 µs LSB arithmetic and the enable-vs-due read ordering are correct and match the code). Not demonstrated for `link_uart` (no retained failure/reproduction log) and not demonstrated by a retained probe for `test_pus` (the probe source/log are absent).

**Correction:** technically sound and minimal: production due-test and framing logic untouched; the frozen seam is compile-time test-only; the boundary test still exercises the `now_fine < last_fine` branch. The 2 s bounds still fail on a hang.

**Evidence reference:** **not complete enough to close.** The evidence chain required by N11 is broken at the failure end (external cache, missing JSONs, missing probe, omitted 17795) and at the cycle-record end (`verification-results.md` absent). The success end is partially sound (`ci-local-final` 50/50, trace counts, cppcheck 163, format exit 0), but the xw1-host and native CTest evidence is stale.

**Severity:** "Major (evidence claim) / Minor (test harness)" is **defensible in substance** — publishing "all gates PASS" without retained logs is a Major evidence/process defect, while the two timing defects are Minor test-harness defects. It is **not well presented**: the dual label can be read as "Minor overall", and it conflates two closure regimes. Recommend a single highest severity (Major) with a split closure account, or two NCs.

**Historical claims:** **correctly withdrawn/qualified, not silently deleted.** Rev 1.117 now says the `ctest 50/50 xw1-host` claim "had no retained log and is withdrawn"; Rev 1.118 says the "all gates passed" claim "is withdrawn" because the invocation was pipeline-masked; Rev 1.120 repeats the retraction. `git show HEAD:docs/planning/sprint-plan.md` confirms the original claims were published at `41dcbe2`, and `git diff` shows the explicit deletions/additions. There is no evidence of history rewriting (no amend/rebase; HEAD is `41dcbe2`; the pre-correction text is intact in git). The only deficiency is that the original wording is not preserved verbatim in the row (PA-NC235-14).

### 6.2 Norm compliance

See the table in §4. Summary: **N11 and N15 fail (Major); N10 fails (Major); N12 is in progress and cannot close; N16 is partial; N8 is partial; N21 is satisfied for classification/registration; N7 has no violation (nothing committed); N9/N17/N20 are partially satisfied with the V&V/R4 legs pending.** N20 is not satisfied by this report alone because it carries Majors and requires a corrected-content re-review.

### 6.3 Evidence integrity

- **Original exit codes outside pipelines:** yes for the post-correction gates. `ci-local-final.log` ends `ORIGINAL_EXIT=0`; `ctest-native.log` ends `CTEST_NATIVE_EXIT=0`; `ctest-xw1-host.log` ends `CTEST_XW1_EXIT=0`; `build-native.log` ends `BUILD_NATIVE_EXIT=0`; `build-xw1-host.log` ends `BUILD_XW1_EXIT=0`; `format-check.log` ends `FORMAT_CHECK_EXIT=0`; `cppcheck.log` ends `CPPCHECK_EXIT=0`. The pre-correction runs are correctly described as pipeline-masked (`tail`), with the pipeline exit distinguished from `make`'s `Errore 8`. **Exception:** the stress log has no exit code or command at all (PA-NC235-08).
- **Candidate identity:** commit `41dcbe2` + five file hashes (all re-verified) + container image id is a good source identity. It is **not** a complete configuration identity: no tree hash, no build-directory state, no effective `FF_*` set, no timestamps, and the evidence directory itself is untracked and ambiguously excluded (PA-NC235-01/03/07).
- **Excluded-work statement:** accurate for the four test files and the n22 mapping. The four test files have mtime 2026-11-02 13:40 and their diffs are clang-format-only (line wrapping, comment alignment); they were not touched on 2026-11-03. The n22 mapping has mtime 07:32 on 2026-11-03, before the candidate work, and is docs-only. The statement is **inaccurate only in the self-referential "untracked files" clause** (PA-NC235-03).
- **Log content supporting claims:** CTest 50/50 is supported (50/50, exit 0 in three logs); trace counts 120/76/30/14/68 are supported (`traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)`); cppcheck 163 is supported (163 lines, identical finding set to the canonical-final baseline); **stress 0/100 is not supported by the log content** (one assertion line); "245 targets" and "clang-format 18" are not textually supported (PA-NC235-19); the pre-fix 3/60 and probe 200/200 are not retained at all (PA-NC235-02).

### 6.4 Test-only code in production source

- **Guard:** the seam is entirely inside `#ifdef FF_TEST_HOOKS` in both the header and the implementation; the macro is defined only when `FF_BUILD_TESTS AND NOT FF_PLATFORM MATCHES "^rtems"` (`CMakeLists.txt:217–220`). With the hook unused, `FF_PalGetUtcNs`/`FF_PalSetUtcNs` execute exactly the previous code path. The claim "no production behaviour changes when the hook is not used" is correct by inspection. The claim "absent from flight/RTEMS builds" is correct **by CMake logic** but has no build/symbol evidence (PA-NC235-06).
- **cppcheck suppression:** the inline `// cppcheck-suppress unusedFunction` is acceptable here: the scanner's pinned path set is `src` + `config` only (tests excluded), so a test-only symbol is a genuine path-set false positive; the suppression is immediately above the definition, carries a reason in the adjacent comment, and the scanner runs with `--inline-suppr`. It is reviewable and not hidden. The weaker point is evidence transparency: two findings are now suppressed, so the "163 unchanged" statement is true in count but not in composition (PA-NC235-13).
- **Baseline effect:** verified against `docs/vv/evidence/2026-11-02-cppcheck-canonical-final/cppcheck_report_20261102_final.txt` (163 findings). The new report is 163 findings; the diff is only line-number shifts in `pal_posix.c` and `pus.c`. No new unsuppressed finding. The two suppressed symbols do not appear in `cppcheck.txt`. So the numerical baseline is unchanged; the suppressed inventory must be recorded (PA-NC235-13).

### 6.5 Configuration management

- **Candidate files:** the five source/plan files and the new evidence directory, as listed in §1. `git status` shows the four excluded test files and the n22 mapping as modified but untouched today (mtimes 2026-11-02 13:40 and 2026-11-03 07:32); the four test diffs are formatting-only, so they cannot have changed the built binaries' semantics. The exclusion is genuine; only the wording of the "untracked files" clause is wrong.
- **Documentation updates missing:** `docs/vv/verification-results.md` (N11/N15, Major); `docs/vv/tool-qualification.md` T3 (N10, Major); `docs/architecture/time-and-timecode.md` (N8, Minor); `docs/reviews/README.md` index (N11/N20, Observation). The sprint-plan itself is updated (1.119/1.120 + ledger), so the change is not undocumented at the plan level.
- **Stale evidence:** `ci-local-after-fix.log` is stale (07:51, before the 07:53 and 08:00 edits) and is **not** clearly marked superseded; the README does not mention it. `build-native.log` and `build-xw1-host.log` are stale in the same sense as PA-NC235-01. `ctest-native.log`/`ctest-xw1-host.log` are stale. Only `ci-local-final.log`, `cppcheck.*`, `format-check.log` and `stress-100-under-load.log` post-date the final source edit.

### 6.6 Residual risk and open items

After this correction the following remain **unproven or unchanged**:

- **Target/RTEMS behaviour:** no RTEMS cross build and no target/emulator run on the corrected candidate. The seam is host-test-only, but its absence from an RTEMS object is not demonstrated (PA-NC235-06). N18 remains **FAIL** (NC-EXT-198); no device/HIL evidence (GAP-A5).
- **xw1-host evidence:** valid only for the 07:56 pre-edit content; not valid for the hashed final candidate (PA-NC235-01). The `ci-local` final run happens to build with `FF_USE_LWIP=ON`/`FF_USE_N7_OBCP=ON` because of inherited cache state, but that configuration is not declared or reproducible (PA-NC235-07).
- **GAP-A4:** cppcheck remains an **unqualified advisory checker**; 163 findings is not MISRA compliance, not a qualification, and not a proof of absence of defects. The two suppressed entries add a small transparency debt.
- **NC-EXT-198 / NC-EXT-199 / NC-PAL-01:** unchanged and OPEN. The correction does not touch the emulated-target fidelity, evidence-integrity or hardware-binding gaps.
- **NC-EXT-207 / 208 / 211:** unchanged and OPEN/authority-owned. The test comment mis-citation must not be read as progress on NC-EXT-211.
- **Other flake risk:** the M2 review already recorded `test_time.c` live-clock sensitivity; the 100× stress covers only `test_pus` and `test_link_uart`, not the whole suite. The 0/100 claim is a sample, not a demonstration of suite-wide determinism.
- **Plan statement "no NC and no sprint is closed":** **correct.** The diff adds NC-EXT-235 without changing any other NC status; the daily ledger says "Sprint closures today: 0"; Rev 1.120 says "No NC closed, no sprint closed." No closure claim is made anywhere in the candidate.

---

## 7. Verdict

> ## **NEEDS REVISION**

The correction itself is technically credible and minimal, and the published success evidence that *is* bound to the final source (`ci-local-final` 50/50, trace 120/76/30/14/68, cppcheck 163, format exit 0) supports the narrow claim that the two test suites now pass deterministically on the host. However, the candidate cannot be accepted as a closeable NC-EXT-235 correction or as "all gates re-verified" because:

1. the xw1-host and native CTest evidence predates the final source edit (PA-NC235-01);
2. the failure record omits a real observed failure and cites reproduction data that is not retained (PA-NC235-02);
3. the failure evidence is outside CM and the evidence set is not self-contained (PA-NC235-03);
4. the N11/N15 verification record, N10 T3 requalification, N8 architecture note and N15 cross build are missing (PA-NC235-04/05/06/11).

Per N12, the Major findings must be corrected and the corrected content re-reviewed before commit/push. This verdict does **not** reject the correction's technical approach; it rejects the completeness and integrity of the evidence package and the claim set built on it.

**Confidence: High** for the findings based on direct file/git/log inspection and recomputed hashes; **Medium** for the precise execution times (filesystem mtimes are the only time evidence because the logs carry no timestamps); **Low/unknown** for anything depending on the unretained pre-fix probe and reproduction runs.

---

## 8. Authority statement

**This review does not approve release, does not close any NC (including NC-EXT-235), and does not confer qualification, certification, airworthiness, flightworthiness or compliance credit.** It is an independent PA assessment of evidence and process only. No source, test, plan or evidence file was modified; no commit or push was performed. The owner/authorized human authority retains every disposition, risk-acceptance, NC-closure and release decision. The reviewer is not a licensed engineer, certification representative or approval authority.

---

## 9. Follow-up and re-review entry criteria

A corrected-content PA re-review can be opened when:

1. the native CTest and xw1-host build/CTest are re-run after the final source edit and the logs replace or explicitly supersede the 07:56 set;
2. the four cache JSONs and the probe/reproduction records are copied into `docs/vv/evidence/2026-11-03-gate-diagnosis/` with a hash manifest, and the 17795 failure is added to the record;
3. `docs/vv/verification-results.md` has the 2026-11-03 cycle entry;
4. `docs/vv/tool-qualification.md` T3 has the change/requalification entry;
5. the RTEMS cross build and a symbol-absence check for the seam are retained;
6. the effective configuration and timestamps of each gate are recorded;
7. the minor corrections (stress log, stale log labelling, NC citation, architecture note, cppcheck suppressed inventory, history-row preservation, closure criteria) are applied;
8. the V&V and R4 (N17) legs are complete and this report is indexed (with the 0440 collision resolved).

Until then, NC-EXT-235 remains **CORRECTED — PENDING INDEPENDENT REVIEW** and must not be closed; no gate claim beyond the narrow `ci-local` host result should be published.
