# Re-review candidate evidence - 2026-11-04

Status: gate evidence for the candidate submitted to independent re-review after
FRICK-REVIEW-0452 (V&V), FRICK-REVIEW-0453 (PA), FRICK-REVIEW-0454 (V&V),
FRICK-REVIEW-0455 (PA), FRICK-REVIEW-0456 (V&V), FRICK-REVIEW-0457 (PA),
FRICK-REVIEW-0458 (V&V) and FRICK-REVIEW-0459 (PA).

The authoritative gate directory is **`build/gate11`**. `build/gate10` and all
earlier logs are retained as superseded history (N11) and are superseded wherever
they conflict with the `gate11-*` files.

This package records what was run and its original exit codes. It closes no NC,
closes no sprint, and claims no approval.

## 1. What changed relative to the 2026-11-04 gate-reconciliation candidate

| Finding | Correction |
| --- | --- |
| 0452 F-01 / 0453 PA-01 (T3) | T3 narrowed and hash-bound in `docs/vv/tool-qualification.md` rev 1.3: **NOT FULLY QUALIFIED** for this candidate; qualification asserted only for the exact revisions reviewed by FRICK-REVIEW-0443. |
| 0452 F-02 / 0453 PA-02 | "all seven PA-NC235-01..07 resolved" withdrawn everywhere; PA-NC235-07 recorded as **PARTIALLY RESOLVED**; erratum appended to FRICK-REVIEW-0451. |
| 0452 F-04/F-05/F-06, 0453 PA-06/PA-07 | `test_evs_hs_e2e.c` replaced by `tests/unit/test_evs_hs_health.c` (bounded copy, no `strstr` on raw payload, no local duplicate constant, severity→subservice oracle, out-of-bound cases). Gaps registered as **NC-EXT-236** and **NC-EXT-237**. SRS-REQ-074 link removed. |
| 0452 F-07 | Tautological "due logic" block removed from `test_frozen_utc_seam.c`; replaced by `test_frozen_cuc_advance`, which exercises `FF_TimeGetCuc` at 1000 s, 1010 s and 1010.25 s (`fine == 16384`). |
| 0452 F-08 / 0453 PA-09 | Handover rev 3.59 annotated as superseded with the measured values. |
| 0452 F-09/F-10/F-11, 0453 PA-O-01 | Local constant removed, bounded copy used, seam scope wording corrected in `docs/architecture/time-and-timecode.md` rev 0.5. |
| 0453 PA-03/PA-04 | FRICK-REVIEW-0451 erratum records the non-reproducible PA3-NC235-01 and the N22-TS5-47 misattribution. |
| 0453 PA-05 | SRS-REQ-074 link removed from `traceability.yaml`. |
| 0453 PA-10 | The four pre-existing formatting-only test modifications are **added to the manifest** so that base commit + manifest reconstructs the tested configuration; they remain out of review scope. |
| 0453 PA-11 | `docs/reviews/README.md` indexes the 2026-11 reviews with session IDs where recorded. |
| 0453 PA-12 | `tool-qualification.md` history renumbered 1.1/1.2 and ordered below 1.0; blank separators removed. |
| 0453 PA-13 | `make smoke` and `make gsw-selfcheck` run and retained here. `gsw-selfcheck` initially failed on two authored helper files that had no SPDX header (N13); the headers were added (non-functional) and the check now passes. |
| 0453 PA-14 | Reviewer session IDs recorded in the review index; reviews with no observable ID are marked "not recorded" rather than back-filled. |
| 0453 PA-NC235-07 | Effective native and RTEMS `CMakeCache` values captured in `effective-build-config.txt`. |

## 2. Corrections from the round-two reviews (0454 / 0455)

| Round-two finding | Correction in this package |
| --- | --- |
| **0454 F-0454-01 / 0455 PA-0455-01 (Major)** — `2026-11-03-gate-diagnosis/assets.sha256` failed verification, and three records claimed 23 OK | The package README was **finalised first**, the manifest was regenerated **second** over the frozen tree, and verification was run **third** with its transcript retained. The self-entry is excluded (a file cannot contain its own hash) and the seven `*-final*` logs that the previous manifest silently omitted are now bound. Result: **exit 0, 29 OK**, `assets-sha256-verify.log`. The rejected first attempt is documented in the package README rather than erased. |
| 0454 F-0454-02 / 0455 PA-0455-03 — TS-5 said 60/60 | `ts-5-core-services.md` now records **110/110**, the script sha256, and the run log; the 60-check figure is labelled as the superseded predecessor and 108 as an intermediate revision. |
| 0454 F-0454-03 / 0455 PA-0455-02 — live "T3 QUALIFIED" text | The two former task rows are annotated as superseded in `sprint-plan.md`; `2026-11-04-gate-reconciliation/CANDIDATE.md` given a SUPERSEDED banner and per-row annotations. |
| 0454 F-0454-04 / 0455 PA-0455-06 — RTEMS log lacked commands, `total_symbols` irreproducible | `rtems-symbols.log` was **re-executed** and now contains the literal commands, their exit codes and three explicit counts: full table **1450**, exported (`-g`) **1085**, defined-only **1450**. The reference binutils is stated. The missing `compile_commands.json` is disclosed and replaced by a compile-flags check. |
| 0455 PA-0455-04 — FRICK-REVIEW-0449 not annotated | Erratum appended: SRS-REQ-074 is no longer SUPPORTED by the deleted script, the expiry clause has triggered, 60/60 is historical; a dated scope correction was later appended for NC-EXT-236. |
| 0455 PA-0455-05 — manifest bound 2 of 12 untracked `.py` gate inputs | All **12** untracked `.py` files plus the 6 untracked `.sh`/`.resc` files scanned by `license_check.py` are now bound in the manifest as manifest-only inputs. |
| 0455 PA-0455-07 — NC-EXT-236/237 had no owner and were absent from the residual list | Both rows now name an owner **role**, an acceptance authority (**Nicola**) and no acceptance date; both are listed in `verification-results.md` §Residual. |
| 0455 PA-0455-08 — handover annotation said 4320 | Corrected to **4325**. |
| 0455 PA-O-02 — modified out-of-scope evidence-mapping doc was unbound | Classified **once** in `CANDIDATE.md`. In the committed candidate the file **is excluded from the manifest but IS in the commit as the base version** (`9215342f…d3ba12`); the working-tree modification (`c94bca27…30fadb`) is uncommitted. PA-0457 §6.9 and PA-0459 required review/reversion before any commit that includes it; this commit includes the base version, so the requirement was not met. Disposition open. |
| 0455 PA-O-03 — "14 tests" wording | Corrected: `gsw-selfcheck` runs **35** checks (12 `test_ff_gsw` + 9 `test_metrics` + 14 `test_license_check`). |
| 0454 F-0454-05 (Advisory) — `fillers < 64` is weaker than the slot count | **Strengthened** (F-0456-03): `FF_PAL_MAX_TASKS` stays out of the test because it is a file-local `#define` in `pal_posix.c`, but after filling the table the test now asserts `FF_PalTaskCreate(...) == FF_NO_RESOURCE`. `FF_NO_RESOURCE` is produced only by the table-full branch, so the fixture demonstrates exhaustion instead of merely failing to create tasks. It does not assert *which* reservation filled the table; PAL slots are process-global and a prior sub-test may already hold some. |

### 2.1 Disclosure: formatting failure on the new test script (corrected wording)

The first `format-check` run on this candidate failed (exit 2) because
`tests/unit/test_evs_hs_health.c` had not been passed through clang-format. The
failing transcript was **not retained**: `make format-check` was re-run after
`clang-format -i`, and the log was overwritten. That is a breach of N11 (retain
failed-cycle evidence), not something this package can cure retroactively; it is
recorded here so that no reader believes the first run passed.

Two further corrections to the earlier wording of this section:

- `format-check.log` contains the **command line only**. On success the gate
  prints nothing, so the PASS evidence is the empty output plus the separate
  exit-code file (`gate11-format-check-exit.txt` = 0), not a PASS line.
- `clang-format -i` did modify a source file. The accurate claim is **no
  semantic change**, not "no C source changed". No pre-format hash was retained,
  so token-level identity of the test file before and after formatting is
  **unverified**. What is verified is that the final file is format-clean
  (`clang-format --dry-run --Werror`, 18.1.3, exit 0) and that the clean
  `gate11` build, CTest and test executions below were all performed on the
  final bytes.

### 2.2 Gate-directory history and why `gate11` is authoritative

| Directory | What it is | Status |
| --- | --- | --- |
| `build/gate7` | the candidate build used for the round-two submission | **superseded** — predates the F-0456-03 test change |
| `build/gate8` | first build after the F-0456-03 change | **superseded** — its `build.log` was overwritten by a 1-line incremental rebuild (the very defect F-0456-01 reported); not cited anywhere |
| `build/gate9` | clean rebuild after strengthening the oracle, before the F-0456-02 comment fix | **superseded** — same test file, earlier comment text |
| `build/gate10` | clean configure + full build + CTest + test executions + format + smoke | **superseded** — F-0458-02: the smoke step re-ran `cmake -S/-B` and `cmake --build` on this directory ~30 s **after** the retained CTest, so its CTest log does not describe the final linked binaries |
| `build/gate11` | clean configure + full build + CTest + test executions + format + cppcheck on the final bytes, followed by smoke **in `build/gate11-smoke`** | **authoritative for build/test/format/cppcheck/smoke** |

The F-0458-02 ordering defect is resolved structurally, not by wording: smoke is
now run with `BUILD_DIR=build/gate11-smoke`, a *separate* build directory, so no
step reconfigures or relinks `build/gate11` after its CTest. This is verifiable
rather than asserted:

- every `build/gate11` **product or test** executable is dated
  **09:43:18–09:43:19** (the last ninja
  edge is 09:43:19.861; the CMake compiler-probe executables written during
  configure at 09:42:53 are excluded as they are neither product nor test
  binaries), `gate11-ctest.log` is 09:43:42 and `gate11-new-tests.log`
  09:43:43 — the CTest and test-execution logs post-date the last link;
- `cmake --build build/gate11` after the whole sequence prints
  `ninja: no work to do`;
- `gate11-smoke.log` (09:45:00) records the separate directory's own build. The
  log itself does **not** print `BUILD_DIR`; the evidence that smoke used
  `build/gate11-smoke` is the existence of that tree with its own executables
  (09:44:43/09:44:46) plus the unchanged mtimes and `no work to do` in
  `build/gate11`, not the log text.

Old gate logs (`configure.log`, `build.log`, `ctest.log`, `format-check.log`,
`new-tests-run.log`, `evs-hs-health-run.log`, `smoke.log`, and the whole
`gate7`–`gate10` family) are retained as history. They are kept because N11
requires failed and superseded cycles to remain visible.

### 2.3 Corrections from the round-four confirmation (0458 / 0459)

A further narrow confirmation of this delta returned passing verdicts with
**0 Blocker / 0 Major**: FRICK-REVIEW-0460 (V&V, **PASS WITH OBSERVATIONS**, 0 Minor,
2 Observations) and FRICK-REVIEW-0461 (PA, **PASS WITH COMMENTS**, 1 Minor,
5 Observations). The
0461 Minor (a stale `Date` field in the TS-5 header) and the non-functional
observations are corrected here; the corrections are documentation-only and
are reconciled against the reviewed manifest in FRICK-REVIEW-0462.

| Finding | Correction |
| --- | --- |
| **F-0458-01 (Minor)** — the test header contradicted the corrected NC-EXT-236 scope and falsely stated that the listener does not forward | `tests/unit/test_evs_hs_health.c:12-19` now states that the listener **does** forward (`src/main/fsw_main.c` installs `ff_event_listener`, which calls `FF_PusEmitEvent`), that `test_r3_gap.c:523-575` covers listener → sink, and that the unverified element is the **assembled** chain through the production `FF_AppTmOutputInit`/`ff_to_sink`. Comment-only change; script is now sha256 `23535db3…6035`, **110 checks**, `gate11-new-tests.log`. |
| **F-0458-02 (Minor)** — the smoke step rebuilt the authoritative directory after CTest and discarded the log | Smoke now runs in `build/gate11-smoke` (separate directory); `build/gate11` is never rebuilt after its CTest, and the ordering is evidenced above. `build/gate10` is marked superseded. |
| **PA-0459-01 (Minor)** — the bound candidate identity table cited `tool-qualification.md` rev 1.3 while the manifested file was 1.4 | `CANDIDATE.md` now cites **rev 1.5** (the revision that re-binds the corrected script hash); `verification-results.md` still records rev 1.3 as the revision of the T3 determination and notes the status is unchanged at 1.5. |
| **PA-0459-02 (Minor)** — "cannot affect any gate result" was too strong for `src/main/fsw_main_reqtest.c` | The file is now **bound** in the manifest as a manifest-only gate input: it is uncompiled and unreferenced by the build system, but `license_check.py` enumerates `src/**` and `cppcheck_scan.sh` scans `/workspace/src`, so base commit + manifest now reconstructs those gate inputs too. |
| **PA-0459-O-01** — stale "Rev 1.121" label on a live 52/52 figure | Re-labelled as "52/52 as of Rev 1.121 … still 52/52 at Rev 1.125". |
| **PA-0459-O-02** — rev 1.3 pointers did not mention that the status is unchanged | Both pointers now say "unchanged at rev 1.5". |
| **PA-0459-O-03** — index rows missing for 0458/0459 | `docs/reviews/README.md` now indexes both. |

## 3. Gate results (original exit codes, no pipelines)

Every command wrote its output to a log and its exit code to a `*-exit.txt`
file. No gate result was taken through `tail` or `tee`.

| Gate | Exit | Result | Log |
| --- | --- | --- | --- |
| configure (`build/gate11`, Ninja, Debug, `FF_USE_PUS_ASN1=ON`) | **0** | configured | `gate11-configure.log`, `gate11-configure-exit.txt` |
| build (`build/gate11`, full: 403 log lines) | **0** | **402/402 targets**, 0 compiler warnings, 0 errors | `gate11-build.log`, `gate11-build-exit.txt` |
| CTest (`build/gate11`) | **0** | **52/52 passed**, 0 failed | `gate11-ctest.log`, `gate11-ctest-exit.txt` |
| `test_frozen_utc_seam` (gate11 binary) | **0** | `checks=4325 failures=0` | `gate11-new-tests.log`, `gate11-new-tests-exit.txt` |
| `test_evs_hs_health` (gate11 binary) | **0** | `checks=110 failures=0` | `gate11-new-tests.log`, `gate11-new-tests-exit.txt` |
| `format-check` (clang-format 18.1.3) | **0** | clean (command-only log; PASS is empty output) | `gate11-format-check.log`, `gate11-format-check-exit.txt` |
| `make smoke` (`BUILD_DIR=build/gate11-smoke`, separate directory) | **0** | banner + 7 applications registered | `gate11-smoke.log`, `gate11-smoke-exit.txt` |
| cppcheck 2.13.0 (advisory, `src`+`config`) | **0** | **163** findings, GAP-A4 baseline unchanged | `gate11-cppcheck.log`, `gate11-cppcheck.txt`, `gate11-cppcheck-exit.txt` |
| `verify-lean` | **0** | `Build completed successfully` | `verify-lean.log`, `verify-lean-exit.txt` |
| `make trace` (incl. elaboration receipt) | **0** | PASS 120 / 76 / 30 / 14 / 68 | `trace.log`, `trace-exit.txt` |
| RTEMS cross-build (`build/rtems-gate7`) | **0** | ELF linked, sha256 `2c4934d5…8a87b` | `rtems-build.log`, `rtems-build-exit.txt` |
| RTEMS seam absence | **0** | 0 seam symbols (`nm`, `nm -g`), 0 seam strings, 4 positive controls | `rtems-symbols.log`, `rtems-symbols-exit.txt` |
| `make gsw-selfcheck` | **0** | 35 checks OK (12+9+14), license check OK, frame self-check | `gsw-selfcheck.log`, `gsw-selfcheck-exit.txt` |

**Why some rows are not re-run in `gate11`.** Lean, trace, the RTEMS cross-build,
the RTEMS symbol inventory and `gsw-selfcheck` do not compile `tests/`:

- Lean and trace consume the Lean project, requirements and traced identifiers;
  a comment in a C test changes none of them.
- The RTEMS build does not build tests (`FF_BUILD_TESTS=ON` but the test targets
  are native-only), so its ELF is unaffected. This is verifiable rather than
  asserted: the last production-source edit was `src/core/pal/pal_posix.c` at
  08:44:51 and the retained ELF was linked at 08:54:03, with no `src/` or
  `config/` edit afterwards; `sha256sum build/rtems-gate7/frick_fsw` still equals
  the hash in `rtems-symbols.log`.
- `cppcheck_scan.sh` scans `src` and `config` only; it was nevertheless re-run on
  the final tree (`gate11-cppcheck.txt`, 163 findings, identical count).
- `gsw-selfcheck` checks the ground-segment Python helpers and the N13 licence
  policy; those files are unchanged since the round-two run.

## 4. Evidence-integrity transcripts

- `assets-sha256-verify.log` — `sha256sum -c` of the **2026-11-03** package
  manifest: **exit 0, 29 OK, 0 FAILED**. It lists every other regular file in
  that directory; the manifest itself is excluded because it cannot contain its
  own hash.
- `manifest-verify.log` — `sha256sum -c` of this candidate's manifest
  (`candidate-manifest.sha256`). The manifest hash itself is deliberately not
  written into any manifested file; compute it with `sha256sum`.
- `effective-build-config.txt` — the effective native and RTEMS `CMakeCache`
  values, including fail-closed options that are `OFF`.

`assets.sha256` and `candidate-manifest.sha256` are regenerated only after all
content they bind is final; any later edit invalidates them and requires the
same freeze → generate → verify order again.

## 5. Explicit limits

- N18 functional validation in Renode remains **FAIL** (NC-EXT-198 /
  NC-PAL-01); it is not re-run here and is not claimed as passing.
- cppcheck remains an unqualified advisory checker (GAP-A4).
- T3 is **NOT FULLY QUALIFIED** for this candidate; the execution record here
  proves execution, not qualification.
- No NC and no sprint is closed. No hardware or human review is claimed.
- `NC-EXT-19`, `NC-EXT-236` and `NC-EXT-237` are open. NC-EXT-236 is the absent
  end-to-end oracle through the **production** sink (`FF_AppTmOutputInit`/
  `ff_to_sink`), not an unimplemented forwarding leg.
- These logs were produced by the coordinator, not by an independent reviewer.
  They are inputs to FRICK-REVIEW-0456/0457 and to the final delta confirmation,
  not a verdict.
- A direct host `ctest` can fail with `Permission denied` on
  `Testing/Temporary` because the directory is container/root-owned; every CTest
  result above is the container run and is the one that counts.

## 6. Candidate manifest

`candidate-manifest.sha256` binds the reviewed product files, the review basis,
the bound review records, the manifest-only inputs and this evidence package. It
deliberately excludes itself and `manifest-verify.log`, which are byproducts of
verifying it, so the manifest can be checked with:

```
sha256sum -c docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256
```

The expected result and the actual transcript are in `manifest-verify.log`.

### 6.1 Deliberately excluded paths

`docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md` is **excluded from the
manifest** but **IS in the commit** as the base version
(`9215342f…d3ba12`, identical to `41dcbe2`). The working tree carries a
pre-existing, unreviewed 28-insertion/13-deletion content change (mtime
`2026-11-03 07:32:15`, sha256 `c94bca27…30fadb`) that is unrelated to this
delta and is **uncommitted**. FRICK-REVIEW-0457 §6.9 / FRICK-REVIEW-0459 residual
5 require the file to be reviewed or reverted before any commit that *includes*
it; this commit includes the base version (it was already tracked), so the
requirement was not met by this commit. Disposition remains open with the owner.
The working-tree modification is left untouched; only its two hashes are
recorded (`CANDIDATE.md`, "Single classification"). Adding the working-tree
version to the manifest would make `sha256sum -c` fail against the committed
tree. No gate reads this file (`.md` outside `src/`), so nothing in §3 changes.

Also excluded, by design: the manifest's own hash (circular) and
`manifest-verify.log` (a byproduct of verifying the manifest). Every other
working-tree item not listed in the manifest — `build/`, `build2/`, `build3/`,
`.opencode/`, `opencode.json`, `tests/node/`, the older evidence directories and
the unrelated review drafts — is pre-existing work that is not part of this
candidate and is not committed.
