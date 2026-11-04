# FRICK-REVIEW-0452 — Independent V&V Delta Review of the 2026-11-04 Gate-Reconciliation Candidate

**Verdict: NEEDS REVISION** — the product delta is technically sound and every gate reproduces, but two evidence/status claims are not supported by the artifacts they cite (F-01, F-02), including the T3 qualification and the "all seven PA findings resolved" statement.

| Field | Value |
| --- | --- |
| Document ID | `FRICK-REVIEW-0452-VV-20261104-DELTA` |
| Date | 2026-11-04 |
| Reviewer role | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer (independent-context agent, norm N9); AI analysis, not a human approval authority |
| Scope | Delta only: withdrawal of `tests/unit/test_pus_mutation.c`; `tests/CMakeLists.txt` re-wiring; `tests/unit/test_frozen_utc_seam.c` and `tests/unit/test_evs_hs_e2e.c`; frozen-UTC seam in `src/core/pal/pal.h`/`pal_posix.c`; N22-TS5-47 test-spec/traceability links; 2026-11-03/04 gate and status records |
| Candidate base | `41dcbe21435e14b5aaff06c63d9df6f0de04df54`, branch `main`, all reviewed content uncommitted |
| Candidate identity | `docs/vv/evidence/2026-11-04-gate-reconciliation/candidate-manifest.sha256` — 62 entries, `sha256sum -c` exit 0 (62/62 OK) |
| Independence | I did not author, edit, build inside the repository, or commit any reviewed item. I read the tree and rebuilt a copy under `/tmp/opencode/vv0452/src`. No repository file other than this report was created or modified. |

## 0. Bottom line

- The **functional delta is acceptable**: the withdrawn test was genuinely non-compiling and requirement-unsound; the CMake fix is correct; the frozen-UTC seam is test-guarded and absent from the RTEMS/flight ELF; all gates reproduce (**52/52**, exit 0).
- The **evidence chain is not acceptable as written**: the T3 “QUALIFIED” status and two independent reviews assert that `tests/unit/test_pus.c` hash `6f68de14…` matches the live tree, while the live/candidate file is `4ca0e2d0…` and was last written at **08:59:30**, after the 08:54–08:55 gate runs. The 11-04 gate executes the candidate, but **no cited independent review covers the candidate revision**.
- A second status claim (“all seven PA-NC235-01..07 resolved”) is contradicted by the review it cites (PA-NC235-07 is **PARTIALLY RESOLVED**).
- Neither defect invalidates the product behavior; both must be corrected before the NC-EXT-235 “reviews complete / T3 re-qualified” wording can stand.

## 1. Method and constraints

1. Verified the candidate manifest with `sha256sum -c` **before** reviewing any content (all 62 entries OK, `/tmp/opencode/vv0452/manifest-check.log`).
2. Read `CANDIDATE.md`, `README.md`, `candidate-manifest.sha256` in that order.
3. Rebuilt and re-ran the native configuration in a writable copy (`/tmp/opencode/vv0452/src`, mounted as `/workspace`) rather than touching the repository tree or the read-only `build/gate5` history.
4. Re-ran the RTEMS cross-build and inspected `nm`/`strings` directly instead of trusting the one-line `NM_GREP_EXIT` sidecar.
5. Re-ran Lean, the pinned clang-format check (`frick-vv:local`, clang-format 18.1.3) and the cppcheck inventory.
6. Attempted to falsify each author claim against primary artifacts; where a primary artifact no longer exists I say so rather than inferring.

## 2. Findings

| ID | Severity | Finding |
| --- | --- | --- |
| F-01 | **Major** | T3 QUALIFIED / candidate identity is bound to `test_pus.c` revision `6f68de14…`, not the candidate `4ca0e2d0…`; two reviews claim a live-tree hash match that is false. |
| F-02 | **Major** | “all seven PA-NC235-01..07 resolved” contradicts the cited review, which records PA-NC235-07 **PARTIALLY RESOLVED**. |
| F-03 | Minor | `2026-11-03-gate-diagnosis/assets.sha256` cannot verify: it contains its own hash; `sha256sum -c` exit 1 (23 OK, 1 FAILED). |
| F-04 | Minor | `test_evs_hs_e2e.c` does not verify the EVS-listener→PUS-S5→TM-sink chain it is named for; Part 3 bypasses the listener. Label binding to `N22-TS5-47` is unrelated. |
| F-05 | Minor | The unverified listener→TM gap has no controlled NC identifier (`NC-EXT-XXX` placeholder, “tracked separately” without an ID). |
| F-06 | Minor | `test_evs_hs_e2e.c` is added as SRS-REQ-074 evidence although it exercises no reset ordering or 250 ms panic deadline. |
| F-07 | Minor | `test_frozen_utc_seam.c:102-105` is tautological (asserts arithmetic on values it just set; calls no production due logic). |
| F-08 | Minor | `handover.md` rev 3.59 still states “2327 checks” / “51/51 CTest” and is not reconciled by rev 3.60. |
| F-09 | Advisory | Local `#define FF_EVS_APP_ERROR 0x0103u` duplicates `config/fsw_config.h:50`. |
| F-10 | Advisory | `strstr` on a non-NUL-terminated decoded CCSDS payload in the test oracle. |
| F-11 | Advisory | The “test-only” seam is linked into the default host simulator binary as well as test targets. |

### F-01 (Major) — T3 qualification is bound to a non-candidate `test_pus.c`

**Claim under review.** `docs/vv/tool-qualification.md:26` records T3 as **QUALIFIED**, citing `FRICK-REVIEW-0443-R2-T3-NC-EXT-235.md`; `:254` records “T3 re-qualification completed” and “both R2 delta review and hosted execution record criteria satisfied”. `sprint-plan.md:322` states “Minor closure: … T3 re-qualification completed”.

**Evidence that the reviewed artifact is not the candidate.**

```
$ sha256sum tests/unit/test_pus.c
4ca0e2d04c202e33493b1457fa3503a35a1edef9da84711092266d74b64e865b  tests/unit/test_pus.c

$ grep -n test_pus.c docs/vv/evidence/2026-11-04-gate-reconciliation/candidate-manifest.sha256
4:4ca0e2d04c202e33493b1457fa3503a35a1edef9da84711092266d74b64e865b  tests/unit/test_pus.c
```

The cited review and evidence record a different revision:

- `docs/reviews/FRICK-REVIEW-0443-R2-T3-NC-EXT-235.md:32` — `6f68de14635039a35381cb5f6c51accadc3afb2f4d47938778b0b40df4de2c0d`
- `docs/reviews/FRICK-REVIEW-0451-PA-NC-EXT-235-REVIEW.md:60-67` — “The five published SHA-256 values … **match the live working tree exactly**”, with `tests/unit/test_pus.c 6f68de14… match`
- `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md:75` — `6f68de14…`

Timeline (`stat`):

- gate evidence logs: `ctest-native-rerun.log` 08:54:34, `build-xw1-host-rerun.log` 08:55:18, `ctest-xw1-host-rerun.log` 08:55:54 (2026-11-03);
- `tests/unit/test_pus.c` mtime: **2026-11-03 08:59:30**;
- the evidence README was written 09:15 and `FRICK-REVIEW-0443` 10:20; `FRICK-REVIEW-0451` 18:23. Because the mtime is 08:59:30 and the current hash is `4ca0e2d0…`, the file was already the candidate revision when the README and both reviews recorded `6f68de14…`. The “match the live working tree exactly” statement in 0451 is therefore false for `test_pus.c`.

**Consequence.** The 2026-11-04 gate does execute `4ca0e2d0…` (my reproduction: 52/52, exit 0), so the *execution* half is satisfied for the candidate. The *independent review* half is not: the only R2 delta review on record covers a different revision. T3 cannot be recorded as QUALIFIED for this candidate, and the NC-EXT-235 “Minor closure / T3 re-qualification completed” wording is unsupported.

**Limit.** I could not retrieve `6f68de14…`; I therefore cannot say whether the difference is comment-only or functional. That is precisely why the hash binding matters.

### F-02 (Major) — “all seven PA-NC235-01..07 resolved” contradicts the cited review

The candidate asserts, identically in three places, that all seven PA findings are resolved:

- `docs/planning/sprint-plan.md:322` — “PA FRICK-REVIEW-0451 (PASS WITH COMMENTS, 0 Major, all seven PA-NC235-01..07 resolved)”
- `docs/status/handover.md:1597` — “PA FRICK-REVIEW-0451 PASS WITH COMMENTS (0 Major, all 7 original resolved)”
- `docs/vv/verification-results.md:272` — “PA FRICK-REVIEW-0451: PASS WITH COMMENTS (0 Major; PA-NC235-01..07 resolved)”

The cited review says otherwise:

```
docs/reviews/FRICK-REVIEW-0451-PA-NC-EXT-235-REVIEW.md:87
| **PA-NC235-07** | ... | **PARTIALLY RESOLVED** |
```

and §9.1 of the same review states the Minor half is “tracked as open (architecture doc gap PA3-NC235-01)”. The candidate has repeated the exact failure mode that created NC-EXT-235: a status claim that its own evidence does not support. `PA-NC235-07` remains open.

### F-03 (Minor) — the 2026-11-03 integrity manifest fails verification

`docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256:5` contains a hash of `assets.sha256` itself.

```
$ sha256sum -c docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256
docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256: FAILED
sha256sum: WARNING: 1 computed checksum did NOT match
[exit 1; 23 of 24 entries OK]
```

`FRICK-REVIEW-0443` and `FRICK-REVIEW-0451` cite this manifest as the integrity record. A self-referential entry makes the manifest permanently unverifiable. The other 23 entries verify; this is an evidence-construction defect, not a data-loss finding.

### F-04 (Minor) — `test_evs_hs_e2e.c` does not verify the chain in its title

- Test 1 (`:172`) verifies HS detection of a FAILED app through the EVS listener — **verified**.
- Test 2 (`:213`) verifies direct `FF_PusEmitEvent` → PUS S5 → TM sink → loopback link — **verified**.
- Test 3 (`:278`) detects HS failure, then **manually constructs the payload** and calls `FF_PusEmitEvent` (`:323-341`), not the listener. It explicitly comments “Direct PUS emission of the failure event (bypassing listener TM sink issue)”.

Therefore the `EVS listener → PUS S5 → TM sink` link is **not** exercised. The file header still calls the test the “EVS-to-HS end-to-end health path oracle (N22-TS5-47)” (`:2`), and `tests/CMakeLists.txt:232` labels it `n22`. `N22-TS5-47` in `docs/vv/test-spec/ts-5-core-services.md:177` is the scheduler heartbeat/late-missed-slot row (`0x0601`–`0x0605`), a different health path. The test-spec **text is honest** (partial/gap); the test's own identity and label are not.

### F-05 (Minor) — the framework gap has no controlled NC identifier

`tests/unit/test_evs_hs_e2e.c:6` and `:74` say `NC-EXT-XXX`. `ts-5-core-services.md:177` says the EVS-listener→TM-sink→link gap is “tracked separately” without naming an NC. A gap that cannot be located in the NC register is not auditable. Assign an NC id (or cite an existing one) in both places.

### F-06 (Minor) — SRS-REQ-074 over-attribution

This delta adds `test_evs_hs_e2e.c` to SRS-REQ-074 evidence (`docs/requirements/traceability.yaml:694`; `git diff` confirms the addition). SRS-REQ-074 (`docs/requirements/SRS.md:434`) requires “before the reset is enforced … exactly one PUS-C service 5 event report … within one absolute monotonic deadline of 250 ms from panic entry”. The test exercises neither the reset path nor the panic deadline nor the exactly-one obligation; it emits one event through the ordinary PUS path. The requirement's own note already says ordering is established by inspection/Lean, not by test. Either remove the test from the SRS-REQ-074 list or annotate the partial coverage explicitly.

### F-07 (Minor) — tautological due-logic assertion

`tests/unit/test_frozen_utc_seam.c:102-105`:

```c
/* Verify due logic: u == period (10), now_fine >= last_fine */
uint32_t u = coarse - 1000;
FF_TEST_ASSERT(u == 10);
FF_TEST_ASSERT(fine >= last_fine); /* Should be true with frozen time */
```

`coarse == 1010` was asserted two lines earlier, so `u == 10` is arithmetic on a value the test just fixed; `fine >= last_fine` is trivially true while frozen. No production due function is called anywhere in `test_hk_due_simulation`. The sub-test name implies verification it does not perform.

### F-08 (Minor) — unreconciled adjacent handover rows

`docs/status/handover.md:1596` (rev 3.59) states `test_frozen_utc_seam.c` has “7 sub-tests, 2327 checks” and “All gates PASS: 51/51 CTest”. The candidate binary reports `checks=4320 failures=0` (reproduced), and the clean gate reports 52/52. Rev 3.60 (`:1597`) corrects the 51/55 and gate5 figures but neither retracts nor annotates 3.59's 2327/51-51 figures, leaving two adjacent contradictory rows.

### F-09 (Advisory) — duplicated event-id definition

`tests/unit/test_evs_hs_e2e.c:136` defines `FF_EVS_APP_ERROR 0x0103u` locally while `config/fsw_config.h:50` defines the same symbol. Include the configuration header instead; a future ID change would silently diverge.

### F-10 (Advisory) — `strstr` on a non-NUL-terminated payload

`tests/unit/test_evs_hs_e2e.c:165` calls `strstr((const char *)payload, needle)` where `payload` is the decoded CCSDS payload and `payload_len` is known but not used to bound the search. The static capture buffer happens to be zero-initialised, but reuse can leave non-zero tail bytes; the oracle can read past `payload_len`. Bound the search with `memmem`/length or copy to a NUL-terminated buffer. (ASan/UBSan run of the test binary is advisable.)

### F-11 (Advisory) — “test-only” is test-guarded, not test-executable-only

`CMakeLists.txt:15` defaults `FF_BUILD_TESTS` to `ON`, and `:218-219` exports `FF_TEST_HOOKS` as `PUBLIC` on `frick_fsw_core` whenever tests are enabled and the platform is not RTEMS. Result:

```
build/ci/frick_fsw     2   (PalTimeTest symbols)
build/gate6/frick_fsw  2
build/rtems/frick_fsw  0
```

The “absent from flight/RTEMS” claim is true; the default host simulator binary also carries the dormant seam, so “test-only” should be read as “guarded by the test build option”.

## 3. Answers to the seven required questions

### Q1 — Withdrawn `test_pus_mutation.c`: absence, references, archived identity, justification

- **Absent from active build.** No file under `tests/unit/`; no `test_pus_mutation` target and no `add_test(NAME pus_mutation …)` in `tests/CMakeLists.txt`. I verified `build/gate5/tests/CTestTestfile.cmake` still registers it (53 tests) while `build/gate6` does not (52).
- **References remaining.** Only disposition/history references: `docs/vv/verification-results.md:222,236,239,248,252`, the evidence package, and `build/gate5` artifacts. No requirement in `traceability.yaml` and no test-spec row references it.
- **Archived identity.** `docs/vv/evidence/2026-11-04-gate-reconciliation/withdrawn/test_pus_mutation.c` exists, 223 lines, sha256 `09a7a62c…31cd9`, matches manifest. **Caveat:** the archived file is internally duplicated — two `FF_TEST_MAIN` blocks (`:196` and `:221`) and duplicate `dummy_hk_provider` (`:36`, `:185`) and helper definitions (`:200`, `:214`). It therefore cannot be the source of any successfully linked binary, consistent with “never compiled”, but it also cannot be a single coherent “verbatim revision”. The README's eight-error summary lists seven causes and omits the duplicate-`main` error visible in `withdrawn/syntax-check.log`.
- **Justification.** Sound:
  1. it does not compile (8 errors, `syntax-check.log` exit 1);
  2. its oracles are requirement-unsound: it expects `FF_PusRegisterHkProvider(0u/255u, …)` to fail (`:180-181`), but `reg_provider` (`src/pus/pus_services.c:109-131`) accepts the full `uint8_t` id range and no SRS/SSS constrains it;
  3. its decoder mutation sweep duplicates the registered, passing `test_decoder_corpus`;
  4. it was the only failure in the gate5 set (13 failures / 1203 checks on the stale binary).
- **Hidden defect?** None found. The 13 failures are test-side expectations (ingest fixture precondition, `EmitPeriodicHk` success/error alternation, provider IDs 0/255); no product defect is demonstrated.
- **Lost coverage?** None claimed and none identifiable: zero requirement links, and the duplicated sweep remains in `test_decoder_corpus`.

### Q2 — Do the gate logs support the claimed figures, and do they reproduce?

| Claim | Author artifact | My check | Result |
| --- | --- | --- | --- |
| configure 0 | `candidate-exits.txt` | independent configure in copy | 0 |
| build 0, 0 warnings | `candidate-build.log` | independent build | 0; `grep -c 'warning:'` = 0 |
| CTest 0, 52/52 | `candidate-ctest.log` | independent ctest | 0; `100% tests passed, 0 tests failed out of 52` |
| format 0 | `candidate-format.log` | `frick-vv:local` (clang-format 18.1.3) | 0 |
| trace 0, 120/76/30/14/68 | `candidate-trace.log` | log content read | PASS with the stated counts |
| Lean 0 | `candidate-lean.log` | `make verify-lean` | 0, “Build completed successfully” |
| cppcheck 0, 163 | `candidate-cppcheck.log` | `cppcheck_scan.sh` | 0, 163 findings |
| RTEMS 0, seam absent | `candidate-rtems-*.log` | independent cross-build + `nm`/`strings` | 0; 0 `PalTimeTest` in ELF and object |

- **Pipeline masking.** All 2026-11-04 evidence logs are direct redirects; no `| tail`, `| tee` or `| head` occurs in them, and `candidate-exits.txt` records each original exit. The 2026-11-03 package still retains the pipeline-masked seq 17401 as a *failure record*, correctly labelled — that is the right way to preserve it.
- **gate5 nuance.** As found, gate5 is **53 registered / 52 pass / 1 fail (`pus_mutation`), exit 8**. “52/52, 100%” is correctly withdrawn for gate5; the real 52/52 belongs to gate6 and reproduces.
- **Log nuance.** The build log contains the filename `warning.c.o`; a naive `grep -i warning` matches it, but there is no compiler warning. My independent build had zero `warning:` lines.

### Q3 — Is the “51/55” retraction correct?

**Yes.** A repository-wide search (including `build/`, not only `docs/`) finds no artifact containing a 55-test run, no “out of 55”, and no `ctest` run with 55 registered tests. `build/ci` has 51 registered tests, which is the plausible origin of the figure: `handover.md:1596` (rev 3.59) states “51/51 CTest”. The retraction is explicit, repeated in four records, and invents no replacement history — correct disposition. Minor residual: rev 3.59's 51/51 is not reconciled (F-08).

### Q4 — Does `test_evs_hs_e2e.c` verify the claimed chain, and is the spec honest?

**No to the full chain; yes to spec honesty.**

- Verified: HS detection through the EVS listener (Test 1); direct `FF_PusEmitEvent` → S5 → TM sink → link loopback (Test 2).
- Not verified: EVS listener → PUS S5 → TM sink forwarding. Test 3 constructs the payload itself and calls `FF_PusEmitEvent` directly (“bypassing listener TM sink issue”).
- `ts-5-core-services.md:177` correctly records `N22-TS5-47` as **partial**, states that the scheduler `0x0601`–`0x0605` EVS emission to the HS listener is **not** verified, states that only the direct `FF_PusEmitEvent` path is verified, and records the listener→TM-sink→link leg as a framework gap. `NC-EXT-19` is still **OPEN** (`sprint-plan.md:154`, `:101`). I found **no overclaim in the test-spec text**.
- The overclaim is in the test's own identity: the “end-to-end” title and the `n22`/`N22-TS5-47` binding (F-04), plus the missing NC id (F-05).

### Q5 — Does the traceability delta hold?

**Yes, structurally.** `git diff` shows exactly three additions of `tests/unit/test_evs_hs_e2e.c`:

- `traceability.yaml:159` — SRS-REQ-014
- `traceability.yaml:694` — SRS-REQ-074
- `traceability.yaml:765` — SSS-REQ-008

No SRS-REQ-075 link exists in HEAD or the worktree. The SRS-REQ-014 and SSS-REQ-008 links are defensible (the test does detect a failed application). The SRS-REQ-074 link is not supported by the test behavior (F-06).

### Q6 — Is the frozen-UTC seam absent from flight/RTEMS, and is production timing unchanged?

**Absent from RTEMS/flight: confirmed.**

```
RTEMS cross-build (frick-rtems-toolchain:local, -DFF_PLATFORM=rtems): exit 0
nm  build/rtems-vv0452/frick_fsw | grep -c PalTimeTest        -> 0 (exit 1)
strings build/rtems-vv0452/frick_fsw | grep -c PalTimeTest    -> 0 (exit 1)
nm  …/pal_posix.c.obj | grep -c PalTimeTest                   -> 0 (exit 1)
grep -rl FF_TEST_HOOKS build/rtems-vv0452                     -> no match
```

**Production timing unchanged: confirmed.** Only `src/core/pal/pal.h` and `pal_posix.c` changed in `src/`; every addition is inside `#ifdef FF_TEST_HOOKS`, and the unfrozen `FF_PalGetUtcNs` expression is the original `(int64_t)ff_now_ns() + g_utc_offset_ns`. No `src/core/time` or PUS/link change is present.

**Precision caveat.** The host simulator binary built with the default `FF_BUILD_TESTS=ON` does contain the dormant seam (2 symbols in `build/ci/frick_fsw` and `build/gate6/frick_fsw`); “test-only” is a compile-time guard, not an exclusion from all non-RTEMS binaries (F-11).

### Q7 — Do any documents still assert unsupported figures?

| Figure | Status in the candidate |
| --- | --- |
| “51/55 with exit 0” | Only ever appears as a **withdrawn** claim, explicitly labelled unsupported. No document asserts it as true. Correct. |
| “52/52 as found” | Correctly withdrawn for `build/gate5` (which is 53 registered/52 pass/1 fail). gate6 52/52 is true and reproduces. Correct. |
| “N22-TS5-47 covered” | The test-spec says **partial/gap**; the S8-SCH plan row lists the EVS-to-HS gap as pending; `NC-EXT-19` remains open. No “covered” claim. But the test label/title and the SRS-REQ-074 link overstate (F-04/F-06). |
| “NC-EXT-235 closed” | Not asserted (“No NC closed, no sprint completed”). However, “INDEPENDENT REVIEWS COMPLETE”, “T3 re-qualification completed” and “all seven resolved” overstate the evidence (F-01/F-02). |
| “2327 checks / 51/51 CTest” | Still asserted in `handover.md:1596` and not reconciled (F-08). |

## 4. Command log (original exit codes, no pipelines)

All logs are under `/tmp/opencode/vv0452/`. Commands 2–8, 11, 14 were run against the writable copy at `/tmp/opencode/vv0452/src` (mounted as `/workspace`) so the repository and `build/gate5` stayed untouched.

| # | Command | Exit | Log |
| --- | --- | --- | --- |
| 1 | `sha256sum -c docs/vv/evidence/2026-11-04-gate-reconciliation/candidate-manifest.sha256` | 0 (62/62 OK) | `manifest-check.log` |
| 2 | `cmake -S . -B build/reviewer -G Ninja` in `frick-vv:local` | 0 | `configure2.log` |
| 3 | `cmake --build build/reviewer -j` | 0 (0 warnings) | `build2.log` |
| 4 | `ctest --test-dir build/reviewer --output-on-failure` | 0 (52/52) | `ctest2.log` |
| 5 | `make verify-lean` | 0 | `lean.log` |
| 6 | `FF_CLANG_FORMAT_ALLOW_UNPINNED=1 make format-check` (host clang-format 23.1.3) | 2 | `format.log` — **not candidate evidence**: the repo pins clang-format 18.x |
| 7 | `docker run … frick-vv:local make format-check` (clang-format 18.1.3) | 0 | `format-docker.log` |
| 8 | `bash tools/scripts/cppcheck_scan.sh --out /tmp/opencode/vv0452/cppcheck-report.txt` | 0 (163 findings) | `cppcheck.log`, `cppcheck-report.txt` |
| 9 | `docker run … frick-rtems-toolchain:local cmake … -DFF_PLATFORM=rtems -DFF_USE_PUS_ASN1=ON && cmake --build …` | 0 | `rtems2.log` |
| 10 | `nm build/rtems-vv0452/frick_fsw \| grep -c PalTimeTest` | 1 (0 matches) | `rtems2-symbols.log` |
| 11 | `strings build/rtems-vv0452/frick_fsw \| grep -c PalTimeTest` | 1 (0 matches) | `rtems2-symbols.log` |
| 12 | `nm …/pal_posix.c.obj \| grep -c PalTimeTest` | 1 (0 matches) | `rtems2-symbols.log` |
| 13 | `sha256sum -c docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256` (repo root) | 1 (23 OK, 1 FAILED) | `assets-11-03-check3.log` |
| 14 | `./build/reviewer/tests/test_evs_hs_e2e` | 0 (`checks=60 failures=0`) | `evs-direct.log` |
| 15 | `./build/reviewer/tests/test_frozen_utc_seam` | 0 (`checks=4320 failures=0`) | `seam-direct.log` |
| 16 | `nm <host simulator ELF> \| grep -c PalTimeTest` for `build/ci`, `build/gate6`, `build/rtems` | 0 (2, 2, 0 matches) | `host-symbols.log` |
| 17 | `git status --porcelain`, `git diff` (scope/provenance) | 0 | inline in this report |

Discarded attempts (retained for transparency, not used as evidence): an initial `sha256sum -c assets.sha256` run from inside the evidence directory resolved paths against the wrong cwd and produced 24 unreadable files (exit 1); an initial direct execution used a stale relative path (`build2/tests/…`, exit 127). Both were re-run correctly as rows 13 and 14.

## 5. What I could not verify, and why

1. **Byte-identity of the archived `test_pus_mutation.c` to the removed file.** The removed file no longer exists; only the archive is available, so the README's “identical to the file removed from `tests/unit/`” cannot be independently established. The archive's internal duplication (two `FF_TEST_MAIN` blocks) makes a single-revision interpretation doubtful.
2. **Revision `6f68de14…` of `test_pus.c`.** Not present in the worktree, the git object store, or the evidence package. I cannot determine whether the delta from the candidate `4ca0e2d0…` is comment-only or functional.
3. **The 2026-11-03 pre-fix reproductions** (3/60, 200/200 probe, 20/200 link_uart). The README itself declares these logs unretained; I reproduced only the corrected state.
4. **Target/HIL behavior.** No target hardware, no Renode N18 (declared FAIL), no WCET/timing evidence. The RTEMS evidence here is a cross-build and symbol check, not execution.
5. **Container image digests.** I used the locally available `frick-vv:local` and `frick-rtems-toolchain:local` images; I did not resolve them to registry digests.
6. **The precise content of `FRICK-REVIEW-0441`'s “hash match” table.** It verifies README↔`assets.sha256` consistency, which does hold; it does not establish README↔live-tree consistency for `test_pus.c`.

## 6. Residual risk and confidence

- **Product risk of the reviewed delta: low.** The withdrawn test was never valid evidence; the CMake change removes a broken target; the seam is compile-time guarded and absent from the RTEMS ELF; no production logic changed.
- **Evidence/process risk: medium.** The candidate repeats a status-claim pattern in which “reviews complete / all resolved / qualified” is not supported by the cited artifacts. The two Major findings are document/evidence defects, not runtime defects, but they block the closure wording and the T3 qualification.
- **Untested state space:** the EVS-listener→TM-sink chain remains unverified; the scheduler `0x0601`–`0x0605` path remains unverified; NC-EXT-19 remains open; GAP-A4 (unqualified cppcheck) and N18 (FAIL) are unchanged.
- **Confidence:** **High** for F-01, F-02, F-03, F-06, F-07 and for all gate reproductions (direct file/hash/log inspection and independent rebuild). **Medium** for F-04/F-05 (the intent is documented; only the identity/label is defective). **Low** for anything depending on the unretained 2026-11-03 probe logs.

## 7. Required corrections before this candidate can be re-assessed

1. **F-01:** Re-bind T3 to the candidate revision. Either (a) obtain an R2 delta review of `tests/unit/test_pus.c` at `4ca0e2d0…` (or the exact final revision), or (b) correct `FRICK-REVIEW-0443`/`0451`, the 11-03 README and `tool-qualification.md` to state which revision was actually reviewed and which was gated, and set T3 back to NOT FULLY QUALIFIED for the candidate until the review is repeated.
2. **F-02:** Replace “all seven … resolved” with the cited review's actual status (PA-NC235-07 PARTIALLY RESOLVED) in `sprint-plan.md:322`, `handover.md:1597`, `verification-results.md:272`.
3. **F-03:** Rebuild `assets.sha256` without its own entry (or exclude the manifest from its own list) and re-verify; record the exit code.
4. **F-04/F-05/F-06:** Rename/re-label the test (drop “end-to-end”/`N22-TS5-47`), assign a real NC id to the listener→TM-sink gap, and annotate or remove the SRS-REQ-074 link.
5. **F-07:** Delete the tautological due assertion or replace it with a call to the production due predicate under frozen time.
6. **F-08:** Annotate `handover.md:1596` as superseded (2327 → 4320 checks; 51/51 → 52/52) or correct the row.
7. **Advisory items** may be dispositioned as NCs under N21, but F-09/F-10 are cheap to fix in the same edit.

**Approval boundary.** This report is an independent V&V assessment only. It does not close any NC, does not qualify T3, does not approve commit/push, release or flight readiness, and does not confer certification or compliance credit. The responsible human authority retains every disposition and risk-acceptance decision.

---

## Provenance addendum — reviewer session identity (2026-11-04)

FRICK-REVIEW-0455 PA-0455-O-01 and FRICK-REVIEW-0453 PA-14 correctly noted that
session identity was asserted but not recorded. The coordinator's session log
records that this V&V review ran in sub-agent session **`ses_e5a8d44f3ffeAznKT2rZBez5dp`**. This
addendum records that identifier; it changes no finding, no severity and no
verdict. The coordinator cannot retroactively prove the absence of shared
context, so the independence claim remains an assertion backed by a recorded
session boundary, not a formal N9 certificate.
