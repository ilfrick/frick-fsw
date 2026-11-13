# Verification Results: test_renode_vv_verdict.sh

## Test Execution (HEAD 0ba7459)

- **SHA-256:** 8787631746b00446c842136b140d59c9bf63572e275e542b81ecb865d275e00c ✓
- **Source integrity:** tools/scripts/renode_vv_verdict.sh unchanged vs 0ba7459 ✓

### Container execution (frick-vv:local)
```
17 checks, 0 failures
ok   - C2 the PASS line carries the marker
ok   - D1 current-tree run without the marker fails (rc=1)
ok   - D2 the FAIL reason distinguishes the startup fatal exit
ok   - E1 missing run directory is a usage error (rc=2)
ok   - F1 the runner does not reference the historical fixed log paths
ok   - F2 the runner creates a fresh per-run directory with mktemp -d
ok   - F3 the runner delegates the verdict to the fail-closed unit
renode_vv_verdict: checks=17 failures=0
```

### Host direct execution
```
Identical output: 17 checks, 0 failures
Last 8 lines: same as container path
```

## Mutations (sandbox, /tmp, not in repo)

### (a) Remove chmod 711 "${B2}"
- B2 directory: default 775 permissions (vs expected 711)
- Non-root user: can traverse, cannot read chmod 000 uart-console.log
- Verdict: `"[renode] FAIL: run capture unreadable ... fail closed"`
- B2 discrimination between execution environments affected, but fail-closed behavior preserved

### (b) Force-setprev probe always fail
- setpriv as current user: `setresuid failed: Operation not permitted` (exit code 127)
- Direct verdict run (non-root path): correctly fails closed
- If root with setpriv failing: test SKIPS B2/B2b with explicit message
- "Different B2 outcome": skip vs pass/fail reporting

## Verdict
**PASS WITH COMMENTS** — test passes identically on container and host (17 checks, 0 failures). Mutations confirm fail-closed design and correct B2 discrimination logic. No repo file writes.

---

# Verification Results: XW-2 target contract (2026-11-02)

| Field | Value |
| --- | --- |
| Document ID | FRICK-VV-XW2-20261102 |
| Revision | 1.0 |
| Date | 2026-11-02 |
| Author | FSW engineer |
| Norms | N8 (docs current), N11 (evidence), N15 (cycle), N22 (requirement-based) |
| Base HEAD | `65e4a91` (the committed XW-2 increment) |

Work package: `docs/vv/test-spec/ts-7-xw2-target-contract.md` (TS-7, Rev 1.1).
Requirement links: `SRS-REQ-073`, `SRS-REQ-075`.

> **NOT norm N18.** The three PAL device hooks (`ff_pal_wd_hw_arm/feed/reset`)
> are substituted at link time by the validated double
> `tests/stubs/ff_test_extwd_model.c` through `tests/stubs/ff_test_extwd_pal.c`.
> A PASS here proves the middleware's obligations under the abstract contract
> on the RTEMS build; it does not establish a device, a real reset, reset scope,
> electrical/clock independence, timing/WCET or mission timeout. N18 remains
> **FAIL** (NC-EXT-198, root cause NC-PAL-01).

## XW-1 host counterpart (SIL)

| Target | Result |
| ------ | ------ |
| `test_extwd_model` | `checks=73 failures=0`, exit 0 |
| `test_extwd_middleware` | `checks=49 failures=0`, exit 0 |
| CTest `extwd_model_contract`, `extwd_middleware_contract` | PASS (labels `n22;fdir;watchdog;xw1`) |
| Full host CTest (`build/xw1-host`) | **50/50 PASS**, `ctest` exit 0 |

## XW-2 emulated target (`make xw2` / `tools/scripts/qemu_xw2_vv.sh`)

- **Command:** `bash tools/scripts/qemu_xw2_vv.sh > build/gate-evidence/xw2-run.log 2>&1` (direct full-log redirection; exit captured in `build/gate-evidence/xw2-run.exit`; no `tee`/`tail` in the capture path).
- **Exit status:** `0`.
- **Run directory (evidence of record):** `build/xw2-run-20261102T011328Z` (archived at `docs/vv/evidence/2026-11-02-xw2-postfix/`).
- **Commit at execution:** `ae278ef8734d12e69a20d41ba367c12c16155afe` (XW-2 commit on both remotes).
- **ELF (positive, wrapped):** `a9dc29d4ac147bcc0c07b9d307892797f7ae7d9a7b88e052312758ab0256c920`
- **ELF (no-wrap control):** `7f56b90e1a5716cee0c78b46ccadd26709f2edc6428734364eafb087da954a93`
- **Backend:** `docker` (recorded in run directory `backend.txt`).
- **Toolchain image:** `frick-rtems-toolchain:local` `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` (recorded in `toolchain-id.txt`).
- **PUS ASN.1 backend:** `OFF` (contract-equivalent; recorded in `pus-asn1.txt`).
- **QEMU timeout:** `25` s (recorded in `qemu-timeout.txt`).
- **Positive elapsed (wall-clock):** 25,360,537,941 ns (recorded in `positive-elapsed-ns.txt`).
- **Run nonce:** `ae2cca72f04721807c0ef680df8b5111` (recorded in `run-nonce.txt`).

```
Per requirement:
  SRS-REQ-073: PASS=12
  SRS-REQ-075: PASS=1
COVERAGE: 13/13 required cases
QEMU-PROCESS: exit 124 (accepted)
WHOLE-RUN: PASS (no panic/fatal marker in the console)
RESULT: PASS
[xw2] no-wrap control: EXPECTED FAILURE reproduced: watchdog init panic + component verifier exit 1
```

The no-wrap control is a **separate CMake target** (`frick_fsw_xw2_nowrap`,
`FF_XW2_NOWRAP_CONTROL=ON`), not the same binary unwrapped. Wrapper segregation
(`nm`/`objdump` counts **asserted**: 3 wrapper call sites, 0 real-hook call sites
in the positive image; 0 wrappers and 3 real-hook call sites in the
flight-shaped image) is in `nm-segregation.txt` in the run directory.

### Retained evidence archive

The complete run directory (23 files, all hashes verified) is archived at
`docs/vv/evidence/2026-11-02-xw2-postfix/` with `README.md` and `assets.sha256`.
The pre-correction runs `20261101T170112Z`, `20261101T170759Z`, `20261101T172050Z`
are retained as superseded history in `docs/vv/evidence/2026-11-01-xw2-target-final/`
(whose `INDEX.md` is marked SUPERSEDED HISTORY).

### XW-1 `format` and `LastTest.log` capture (PA-08)

The `checks=73/49` figures above are corroborated by the transient CTest
artifact `build/xw1-host/Testing/Temporary/LastTest.log` (exit 0, 50/50 PASS),
which is now retained in the evidence archive. `format-apply` and `format-check`
exit statuses are captured explicitly as `format-apply.exit` and
`format-check.exit` (both 0) in the archive; the prior archive's assertion of
exit 0 was not backed by a capture file.

## Verdict

**XW-2 target contract: PASS** for the 13 registered cases and the negative
control, on the test-only emulated-target image. This is `PIL-QEMU`
middleware-contract evidence under `SRS-REQ-073`/`SRS-REQ-075`; it is **not**
N18, not HIL and not device evidence. The 8 s table timeout remains an ESC-7
placeholder.
---

# Verification Results: Gate-failure diagnosis and correction (2026-11-03)

| Field | Value |
| --- | --- |
| Document ID | FRICK-VV-GATE-20261103 |
| Revision | 1.0 |
| Date | 2026-11-03 |
| Author | FSW engineer |
| Norms | N8 (docs current), N11 (evidence), N15 (cycle), N16 (history), N21 (sequencing) |
| Base HEAD | `41dcbe21435e14b5aaff06c63d9df6f0de04df54` |

## Scope

Correction of two test-harness timing defects observed on 2026-11-03 (cached sequences 17401, 17633, 17795, 17849) and withdrawal of unsupported gate claims in sprint-plan Rev 1.117/1.118.

- `test_pus` HK due-boundary assertions (`test_hk_coarse_wrap_due` at `test_pus.c:754..761` and `test_hk_periodic_emission` at `:561..574`, `test_hk_backward_step_no_emission` at `:869,870`) failed because the host PAL computes UTC from a free-running monotonic clock; the CUC fine field quantizes to 15.2588 µs per LSB, and at `u == period` the tie-break `now_fine >= last_fine` can flip false between the enable read (which latches `last_emission`) and the later due read.
- `test_link_uart` positive receive assertions failed under 4× CPU load because 20 ms / 500 ms bounds were too tight for PTY scheduling.

Both are test-harness defects; production due-test logic (`src/pus/pus_services.c:406`) and UART framing logic (`src/transport/link/link_uart.c`) were not changed.

## Correction

- `src/core/pal/pal.h`, `pal_posix.c`: test-only frozen-UTC seam under `FF_TEST_HOOKS` (`FF_PalTimeTestFreeze`, `FF_PalTimeTestUnfreeze`), absent from flight/RTEMS builds.
- `tests/unit/test_pus.c`: HK tests freeze UTC for their duration; unfreeze at test end.
- `tests/unit/test_link_uart.c`: positive receive bounds raised from 20 ms / 500 ms to 2 s (bounded; a hang still fails). The 2 ms negative cases unchanged.
- `docs/planning/sprint-plan.md`: Rev 1.119 failure record, Rev 1.120 verification record, NC-EXT-235.

## Evidence (original exits, no pipelines)

| Gate | Command | Exit | Result | Log |
| --- | --- | --- | --- | --- |
| ci-local | `make ci-local > LOG 2>&1` | **0** | 50/50, Lean, elab receipt, trace PASS (120/76/30/14/68) | `docs/vv/evidence/2026-11-03-gate-diagnosis/ci-local-final.log` |
| native CTest | `ctest --test-dir build/native --output-on-failure > LOG 2>&1` | **0** | 50/50 | `docs/vv/evidence/2026-11-03-gate-diagnosis/ctest-native-rerun.log` |
| xw1-host build | `cmake --build build/xw1-host -j` | **0** | 242/245 targets | `docs/vv/evidence/2026-11-03-gate-diagnosis/build-xw1-host-rerun.log` |
| xw1-host CTest | `ctest --test-dir build/xw1-host --output-on-failure > LOG 2>&1` | **0** | 50/50 | `docs/vv/evidence/2026-11-03-gate-diagnosis/ctest-xw1-host-rerun.log` |
| format | `docker run ... frick-vv:local make format-check` | **0** | clang-format 18 (pinned), all files clean | `docs/vv/evidence/2026-11-03-gate-diagnosis/format-check.log` |
| cppcheck | `tools/scripts/cppcheck_scan.sh --out ...` | **0** | 163 visible findings + 2 test-only suppressed (GAP-A4 advisory) | `docs/vv/evidence/2026-11-03-gate-diagnosis/cppcheck.log`, `cppcheck.txt` |
| stress | 100x each under 4 CPU burners | **0** | `test_pus` 0/100, `test_link_uart` 0/100 (sample; not proof of absence) | `docs/vv/evidence/2026-11-03-gate-diagnosis/stress-100-under-load.log` |

## Superseded evidence (retained for audit)

`ctest-native.log` (07:56), `ctest-xw1-host.log` (07:57), `build-xw1-host.log` (07:56), `build-native.log` (07:56), `ci-local-after-fix.log` (07:51) — all predate the final `pal_posix.c` edit at 08:00:27.

## Failure record (cached, now copied into evidence dir)

- `2026-11-03-ci-pipeline-evidence.json` — seq 17401: `tail`-masked pipeline, `24 - link_uart` FAILED
- `2026-11-03-ci-direct-17633.json` — seq 17633: direct run, exit 2, CTest 49/50, `pus` failures at `test_pus.c:754,755,760,761`
- `2026-11-03-pus-reruns.json` — seq 17795: pre-fix binary, `test_hk_backward_step_no_emission` at `test_pus.c:869,870`, `checks=225 failures=2`
- `2026-11-03-ci-direct-17849.json` — seq 17849: direct run, exit 0, 50/50, candidate binding not independently established

## Open findings from independent reviews

- PA review FRICK-REVIEW-0440-PA-NC-EXT-235.md: 7 Major findings (PA-NC235-01..07), primarily evidence-chain completeness and stale gates. **NEEDS REVISION** — not closeable.
- V&V review FRICK-REVIEW-0439-VV-NC-EXT-235.md: pending (free-model limit).
- R4 review FRICK-REVIEW-0441-R4-NC-EXT-235.md: pending (free-model limit).

## Residual items at FRICK-VV-GATE-20261103 (historical, superseded)

> **Superseded 2026-11-04 — PA-0457-04.** Retained as history (N11). The current
> residual list is the "Residual unproven items" section at the end of this
> document. In particular, the RTEMS cross build **has since been run** on the
> corrected candidate (exit 0, `docs/vv/evidence/2026-11-04-re-review/rtems-build-exit.txt`,
> ELF sha256 `2c4934d5…8a87b`); the line below records the 2026-11-03 state only.

- N18 functional validation in Renode emulated PolarFire SoC: **FAIL** (NC-EXT-198, root cause NC-PAL-01 unwired MPFS binding).
- RTEMS cross build had not yet been run on the corrected candidate *at the time of this gate* (superseded — see the note above).
- GAP-A4: cppcheck is an advisory, unqualified checker.
- NC-EXT-198/199, NC-PAL-01, NC-EXT-207/208/211 remain open.
- No NC closed, no sprint completed. S8-SCH/M17 remains **in progress**; 2026-10-30 R3/Section 5 conclusions unchanged.

## Verdict

**CYCLE COMPLETE WITH OPEN FINDINGS** — gates pass for the corrected candidate on the host configurations tested, but the evidence chain requires the Major PA findings to be addressed and the independent V&V/R4 reviews to complete before any commit/push (norm N7/N9/N20).

---

# Verification Results: Contradictory-figure reconciliation and candidate re-gate (2026-11-04)

| Field | Value |
| --- | --- |
| Document ID | FRICK-VV-GATE-20261104 |
| Revision | 1.0 |
| Date | 2026-11-04 |
| Author | FSW engineer |
| Norms | N8, N11, N15, N16, N21 |
| Base HEAD | `41dcbe21435e14b5aaff06c63d9df6f0de04df54` |
| Evidence package | `docs/vv/evidence/2026-11-04-gate-reconciliation/` |

## Scope

1. Re-check the NC-EXT-235 figure "51/55 with exit 0" against retained artifacts.
2. Re-check the later figure "52/52, 100% passed" for `build/gate5`.
3. Restore a green candidate gate after an abandoned, non-building test
   (`tests/unit/test_pus_mutation.c`) had been registered.
4. Produce clean-room gate evidence with original exit codes, outside pipelines.

## Findings

- **"51/55 with exit 0" is not demonstrable.** A repository-wide search for
  `51/55`, `51 / 55`, `55 tests`, `51 of 55` over retained `*.log`, `*.md`,
  `*.txt` and `*.json` returns no match. No retained log records 55 registered
  tests. The claim is withdrawn; no replacement history is invented.
- **`build/gate5` is container-only.** `CMakeCache.txt` records
  `CMAKE_HOME_DIRECTORY=/workspace` and `CMAKE_CACHEFILE_DIR=/workspace/build/gate5`;
  the test commands in `tests/CTestTestfile.cmake` are absolute `/workspace/...`
  paths and the tree is root-owned. A host-side `ctest --test-dir build/gate5`
  cannot run it (observed exit 8, "Cannot create log file").
- **`build/gate5` as found: 53 registered, 52 pass, 1 fail (`pus_mutation`),
  exit 8.** The "52/52, 100%" claim is therefore not reproducible against the
  tree as found either.
- **`tests/unit/test_pus_mutation.c` did not compile** (8 errors under
  `-Wall -Wextra -Werror`: implicit declarations of `hk_test_setup`/`hk_send_tc`,
  duplicate `dummy_hk_provider`, conflicting `static` definitions) and its
  oracles contradicted the product contract (13 failures of 1203 checks on the
  stale binary that was actually built). It was withdrawn and preserved verbatim
  with sha256 in the evidence package. No requirement referenced it.

## Correction

- `tests/CMakeLists.txt`: removed the `test_pus_mutation` target and the
  `pus_mutation` ctest case; moved `test_frozen_utc_seam` out of
  `if(FF_USE_LWIP)` (the test uses only `core/pal` and `core/time`;
  `FF_TEST_HOOKS` is exported `PUBLIC` by `frick_fsw_core`).
- `tests/unit/test_pus_mutation.c`: removed from the tree after archiving.

## Evidence (clean-room `build/gate6`, original exits, no pipelines)

| Gate | Exit | Result | Log |
| --- | --- | --- | --- |
| configure | **0** | `cmake -S . -B build/gate6 -G Ninja -DFF_BUILD_TESTS=ON` in `frick-vv:local` | `candidate-configure.log` |
| build | **0** | 402/402 targets, 0 warnings, 0 errors | `candidate-build.log` |
| CTest | **0** | **52/52 passed**, 0 failed | `candidate-ctest.log` |
| format-check | **0** | clang-format 18 (pinned) | `candidate-format.log` |
| trace | **0** | PASS (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems) | `candidate-trace.log` |
| verify-lean | **0** | `lake build` completed successfully | `candidate-lean.log` |
| cppcheck | **0** | 163 findings (GAP-A4 advisory baseline unchanged) | `candidate-cppcheck.log`, `candidate-cppcheck.txt` |
| RTEMS cross-build | **0** | `frick_fsw` linked; `FF_PalTimeTestFreeze/Unfreeze` absent from ELF | `candidate-rtems-build.log`, `candidate-rtems-symbols.log` |

All exit codes are also summarised in `candidate-exits.txt`.

## Review status (supersedes the "pending" lines in FRICK-VV-GATE-20261103)

- V&V FRICK-REVIEW-0450: **PASS WITH COMMENTS**.
- PA FRICK-REVIEW-0451: **PASS WITH COMMENTS** with 0 Major *at issue time*, but its own finding table records **PA-NC235-07 as PARTIALLY RESOLVED** (effective native/RTEMS `CMakeCache` configuration not captured). Its line 89 summary "all 7 Major findings are resolved" contradicts its line 87 row; the stricter record governs. The "PA-NC235-01..07 resolved" wording used here previously is withdrawn (FRICK-REVIEW-0453 PA-02).
- R4 FRICK-REVIEW-0441 on record. T3 FRICK-REVIEW-0443 covers `tests/unit/test_pus.c` at sha256 `6f68de14…` and `tests/unit/test_link_uart.c` only; the candidate `test_pus.c` is `4ca0e2d0…` and `test_frozen_utc_seam.c` / `test_evs_hs_health.c` have no N10 review, so **T3 is NOT FULLY QUALIFIED** for this candidate (`docs/vv/tool-qualification.md` rev 1.3, the revision of record; unchanged at rev 1.5 except for re-bound script hashes).
- The 2026-11-04 delta reviews FRICK-REVIEW-0452 (V&V) and FRICK-REVIEW-0453 (PA) both returned **NEEDS REVISION**. Their findings were corrected and resubmitted as FRICK-REVIEW-0454/0455.
- FRICK-REVIEW-0454 (V&V) and FRICK-REVIEW-0455 (PA) also returned **NEEDS REVISION**: both Majors were the same
  evidence-integrity defect in `2026-11-03-gate-diagnosis/assets.sha256` (a stale `README.md` hash plus a verify
  transcript taken from a pre-edit run). That defect is corrected in this revision by freezing the README first,
  regenerating the manifest second over all 29 other files, and verifying third (exit 0, 29 OK,
  `docs/vv/evidence/2026-11-04-re-review/assets-sha256-verify.log`). The remaining 0454/0455 Minors are also
  corrected; the corrections are indexed in `docs/vv/evidence/2026-11-04-re-review/README.md` §2.
- A third independent round returned **PASS WITH COMMENTS** for both profiles:
  FRICK-REVIEW-0456 (V&V: 0 Blocker / 0 Major / 3 Minor / 3 Advisory) and
  FRICK-REVIEW-0457 (PA: 0 Blocker / 0 Major / 4 Minor / 4 Observations). Their
  findings (F-0456-01..06, PA-0457-01..08) are corrected; the corrections are
  recorded in `sprint-plan.md` Rev 1.124, `handover.md` rev 3.63,
  `tool-qualification.md` rev 1.4 and the evidence README §2.1–§2.2.
- A fourth, narrow delta confirmation of those corrections returned
  **PASS WITH COMMENTS** for both profiles, with **0 Blocker / 0 Major**:
  FRICK-REVIEW-0458 (V&V: 2 Minor) and FRICK-REVIEW-0459 (PA: 2 Minor,
  3 Observations). All four Minors and the three Observations are corrected in
  this revision (`sprint-plan.md` Rev 1.125, `handover.md` rev 3.64,
  `tool-qualification.md` rev 1.5, `ts-5-core-services.md` Rev 1.14, evidence
  README §2.3). **No finding of any round is open at Blocker or Major severity.**
- FRICK-REVIEW-0460 (V&V) and FRICK-REVIEW-0461 (PA) then confirmed that delta
  with **0 Blocker / 0 Major** (0460: PASS WITH OBSERVATIONS, 2 Observations; 0461: PASS
  WITH COMMENTS, 1 Minor + 5 Observations). The Minor and the non-functional
  observations are documentation-only corrections, applied here and reconciled
  in FRICK-REVIEW-0462.
- Current-cycle evidence is `docs/vv/evidence/2026-11-04-re-review/`: clean
  `build/gate11` configure/build/CTest/new-tests/format/cppcheck plus smoke in a
  **separate** build directory, original exits retained (`gate11-*-exit.txt`).
  `build/gate10` and its logs are retained as superseded history. A final narrow
  confirmation on the regenerated manifest is the remaining independent step;
  **this document claims no verdict for it and no release approval**.

## Residual unproven items (not closed by this cycle)

- N18 functional validation in Renode emulated PolarFire SoC: **FAIL at the
  2026-11-04 gate** (NC-EXT-198; root cause NC-PAL-01 unwired MPFS binding).
  **Superseded 2026-11-09 and 2026-11-10:** the emulated WDOG0 binding and the
  `N22-TS5-66` tree both PASS (`RENODE_TEST: PASS`). What remains open is
  target/HIL evidence, not the emulated functional run (NC-PAL-01, GAP-A5).
- GAP-A4: cppcheck remains an advisory, unqualified checker.
- NC-EXT-19 remains open (scheduler EVS end-to-end path not verified).
- **NC-EXT-236**: **EVIDENCE DELIVERED** — `tests/unit/test_tm_output_chain.c` (47 checks) exercises the assembled chain through the production sink `FF_AppTmOutputInit`/`ff_to_sink`. NC remains open pending human PA authority disposition; functional gap closed. |
  TM sink → link chain through the **production** sink
  (`src/apps/app_tm_output.c:14-25` `ff_to_sink`, installed by
  `FF_AppTmOutputInit`). The forwarding leg is implemented; `test_evs_hs_health.c`
  covers sink → loopback link with an **injected** sink, and `test_r3_gap.c:523-575`
  covers listener → sink. Owner role: FSW engineer; acceptance authority:
  **Nicola**; no acceptance date.
- **NC-EXT-237** (recorded here on 2026-11-04): `FF_PusEmitEvent` ignores
  `event_id` (`src/pus/pus.c`), so event identity is caller-supplied payload
  only. Owner role: FSW engineer; acceptance authority: **Nicola**; no
  acceptance date. **Superseded 2026-11-11 — see the NC-EXT-237 cycle section at
  the end of this record:** the header is now emitter-owned and a discriminating
  oracle exists; the NC itself remains OPEN pending PA acceptance.
- NC-EXT-198/199, NC-PAL-01, NC-EXT-207/208/211 remain open.
- No NC closed, no sprint completed.

## Verdict

**CANDIDATE GATES PASS** on the configurations tested (native build/test, static
format, traceability, Lean, cppcheck baseline, RTEMS cross-build). No NC and no
sprint is closed, no hardware or human review is claimed, and no commit/push is
authorised by this record alone.

## Emulated WDOG0 watchdog binding cycle — 2026-11-09

**Scope:** test-only MPFS WDOG0 binding for the N18 build
(`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`), absolute monotonic feeder
cadence, and the provisional 10 s / 2.5 s mission-table values (NC-EXT-238).
**Not claimed:** flight binding, target/HIL protection, RTPFS160ZT
clock/register/reset-scope fidelity, mission suitability.

### Executed evidence (current tree)

| Gate | Result | Evidence |
| --- | --- | --- |
| Host build + CTest (`build2/native`) | **PASS** | `100% tests passed, 0 tests failed out of 54` |
| Pinned `clang-format` 18.1.3 (`frick-vv:local`) | **PASS** | `find src config tests ... --dry-run --Werror` exit 0 |
| Lean `lake build` | **PASS** | `Build completed successfully.` |
| Lean elaboration receipt (`lean_elab_receipt.py`) | **PASS** | 11 modules; receipt sha256 `3410575860331b36284a199bc91484ce88b94ea3503b46fb8b7b8c460abe1f26` |
| Traceability (`trace.py`) | **PASS** | 120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems |
| `make gsw-selfcheck` | **PASS** | GSW 12 tests OK, metrics 9 tests OK, license 14 tests OK + license checker OK |
| `metrics.py --check` | **PASS** | `metrics panel: current` |
| RTEMS 6.2 cross-build (`frick-rtems-toolchain:local`) | **PASS** | configure/build exit 0, `build/renode/frick_fsw_renode.elf` produced |
| **N18 Renode** (`tools/scripts/renode_vv.sh`) | **PASS** | `RENODE_TEST: PASS`; container exit 0 |

### N18 watchdog evidence (retained console)

```
watchdog: WDOG0 armed (emulated profile: timeout 10000 ms, TIME=1562500, TRIGGER=4095, MSVP=TIME, clk=156250 Hz)
WD armed: timeout 10000 ms, feed 2500 ms
RENODE_TEST: PASS
```

The register sequence was exercised by a separate pinned-image probe before the
binding was written; the model clock was measured at ≈156.25 kHz. The binding
programs `TRIGGER -> MSVP -> TIME -> CONTROL`, reads the three value registers
back before the first keyed refresh, and then feeds with the keyed refresh.

### Residual open items

NC-PAL-01 (flight binding + target/HIL), NC-EXT-186 (250 ms target timing),
NC-EXT-188 (target protection), NC-EXT-238 (mission-authority values). The
emulated run is integration evidence only and does not discharge these.

The documentation identity manifest
`docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-docs.sha256` was regenerated for
the FRICK-ARCH-0024 rev 1.6 text freeze and verifies (22/22 `OK`, exit 0).

## S8-SCH TBL→dispatch coupling cycle (N22-TS5-66) — 2026-11-10

**Scope:** Table Services load/activate → scheduler dispatch coupling in
`src/apps/app_scheduler.c` (activation callback snapshots the activated image;
the scheduler task applies it on its next tick), the `FF_TblActivateCb` contract
clarification in `src/core/tbl/tbl.h`, the production-path host cases in
`tests/unit/test_tbl_sched_coupling.c`, and the deterministic controlled-delay
test seam (`FF_PalTaskDelayMsTestWait`).
**Not claimed:** mission rates/jitter/tolerances, WCET, the worst-case one-tick
deferral budget, target/RTEMS timing, cross-core memory ordering (M14), or any
flight-use authorization.

### Executed evidence (current tree)

| Gate | Result | Evidence |
| --- | --- | --- |
| Host build (all targets, no warnings) | **PASS** | `cmake --build build2/native -j4` exit 0; no `warning:`/`error:` lines |
| Full CTest | **PASS** | `100% tests passed, 0 tests failed out of 55` |
| `tbl_sched_coupling` | **PASS** | `tests/unit/test_tbl_sched_coupling.c checks=61 failures=0` |
| `sched_events` | **PASS** | `tests/unit/test_sched_events.c checks=68 failures=0` |
| `sched_tm_chain` | **PASS** | `tests/unit/test_sched_tm_chain.c checks=50 failures=0` |
| Repeatability (25 consecutive runs of both binaries, 50 executions) | **PASS** | `failures over 25 runs (50 execs): 0` |
| Traceability (`trace.py`) | **PASS** | 120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems |
| Lean `lake build` | **PASS** | `Build completed successfully.` |
| Pinned `clang-format` 18.1.3 (`frick-vv:local`) | **PASS** | `find src config tests ... --dry-run --Werror` exit 0 (host clang-format 23.x is rejected by the Makefile pin) |
| RTEMS 6.2 cross-build, N18 profile (`frick-rtems-toolchain:local`, `build/renode`, `FF_ENABLE_RENODE_TEST=ON`) | **PASS (exit 0, 1 pre-existing warning)** | clean rebuild `EXIT=0`, 164/164 steps; `build/renode-build.log`; ELF sha256 `5557d0bd…1a0e` identical to the copied `frick_fsw_renode.elf` |
| **N18 Renode** (`tools/scripts/renode_vv.sh`, fresh per-run dir) | **PASS** | `[renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)`, run dir `/tmp/frick-renode-vv.02rYaH` |

The single RTEMS warning is **pre-existing and outside this increment**:
`src/core/pal/pal_rtems.c:156: implicit declaration of function 'ff_pal_wd_hw_feed'`
(the prototype exists only in `src/core/pal/pal_watchdog.c`, not in a header).
It is registered as **NC-EXT-239 OPEN** in `docs/planning/sprint-plan.md`; it is
not caused by, and is not corrected in, this cycle.

**Retained evidence:** `docs/vv/evidence/2026-11-10-n22-ts5-66/` (CTest, trace,
format-check, RTEMS/N18 build log, Renode console and process log, candidate
hashes).

### N18 scheduler-startup evidence (retained console excerpt)

```
app SCHEDULER started
SCH app started, 4 entries
watchdog: WDOG0 armed (emulated profile: timeout 10000 ms, ...)
RENODE_TEST: PASS
```

This confirms the changed initialisation path (mutex creation, callback
registration, `FF_TBL_INVALID_HANDLE` reset) starts on the emulated target and
does not break the mission schedule. It does not exercise the activation callback
on target, which remains host-only evidence in this cycle.

### Oracle summary

- **Nominal** (`test_tbl_sched_production_activate`): with the production
  `FF_AppSchedulerMain` running under the controlled-delay seam, the startup
  schedule (4 entries) is active; after `FF_TblLoad` + `FF_TblActivate` on the
  application's registered handle the active count is **unchanged** (the callback
  only snapshots), and after exactly one scheduler tick the count is the new
  table's (2), exactly one `FF_SCHED_EVT_ACTIVATED` event carries
  `"SCH: table activated, 2 entries"`, and the next 1000 ms tick releases exactly
  one message from the new table (`dispatched` +1).
- **Out-of-bound** (`test_tbl_sched_production_invalid`): `FF_TblLoad` with
  `size - 1` returns `FF_BAD_SIZE`, a bad-magic image returns `FF_ERROR`, the
  following `FF_TblActivate` returns `FF_NOT_READY`, and after a tick the
  4-entry schedule is still active with no re-activation event.

### Test-seam correction (not an application behaviour change)

`FF_PalTaskDelayMsTestWait()` now consumes the block notification so each
`FF_PalTaskDelayMsTestStep()` is observed by exactly one scheduler iteration.
Previously two steps could be coalesced, letting injected time be applied to the
wrong iteration. The `N22-TS5-47` integration expectations in
`tests/unit/test_sched_events.c` were corrected to the deterministic sequence
(2 missed-slot events: `3 ... total 3`, then `5 ... total 8`); the application's
report-only-new-skips policy is unchanged.

### Residual open items

NC-EXT-19/90 (mission schedule/timing/policy inputs), `N22-TS5-47`
(EVS-to-HS end-to-end), `N22-TS5-48` (mission rates/tolerances), the one-tick
activation-apply deferral budget, M14 memory ordering, and target/HIL evidence
(GAP-A5). The N18 run above is emulated-target functional validation of this
change only: it establishes that the image boots, the scheduler app starts and
the `RENODE_TEST` marker is produced. It is **not** HIL, timing/WCET, radiation
or mission evidence, and it does not close NC-PAL-01. No NC is closed by this
cycle.

## NC-EXT-237 S5 event-identity cycle — 2026-11-11

**Scope:** the Service 5 event-report identity contract of
`FF_PusEmitEvent(event_id, evs_type, data, len)` in `src/pus/pus.c`, its API
contract in `src/pus/pus.h`, every in-tree caller, and the new discriminating
oracle `tests/unit/test_pus_event_identity.c`.

**Defect (as registered):** the implementation executed `(void)event_id;` and
forwarded the caller buffer verbatim, so the argument had no effect, event
identity was caller-supplied payload only, and the existing
`test_evs_hs_health.c` oracle asserted payload transparency rather than
identity (FRICK-REVIEW-0453 PA-07).

**Change:** `FF_PusEmitEvent` now owns the S5 report header
`event id[2, big-endian] + type[1]` (ICD §4) and encodes it from `event_id` and
`evs_type`; `data`/`len` carry auxiliary data only. Bounds are `len <=
FF_PUS_MAX_TM_FRAME - 3` (`FF_BAD_SIZE` above) and `data == NULL` accepted only
with `len == 0` (`FF_INVALID_ARGUMENT` otherwise). Severity → subservice mapping
(1/2/3, unknown → 3) is unchanged. The encoded octets are identical to what a
caller previously produced by embedding the header itself, so **no wire format
changed**; ICD rev 1.7 records the contract. Callers updated to pass text only:
`src/main/fsw_main.c`, `src/main/fsw_main_reqtest.c`,
`tests/target/fsw_main_xw2.c`, `tests/unit/test_r3_gap.c`,
`tests/unit/test_tm_output_chain.c`, `tests/unit/test_evs_hs_health.c`
(re-bound to 122 checks, sha256 `5310cc3c…277f`).

### Executed evidence (current tree, `BUILD_DIR=build2/native`)

| Gate | Result | Evidence |
| --- | --- | --- |
| Configure + build | exit 0, no warnings | `docs/vv/evidence/2026-11-11-nc-ext-237/gate-build.log` |
| CTest | **56/56 PASS** | `docs/vv/evidence/2026-11-11-nc-ext-237/gate-ctest.log` |
| Lean 4 | `lake build` exit 0 | `docs/vv/evidence/2026-11-11-nc-ext-237/gate-lean.log` |
| Traceability | PASS (120/76/30/14/68) | `docs/vv/evidence/2026-11-11-nc-ext-237/gate-trace.log` |
| Format | pinned clang-format 18, exit 0 | `docs/vv/evidence/2026-11-11-nc-ext-237/gate-format.log` |
| N13 license | `license_check.py` OK | `docs/vv/evidence/2026-11-11-nc-ext-237/gate-license.log` |

### Discriminating oracle and mutation control

`tests/unit/test_pus_event_identity.c` (5 cases, 106 checks; ctest
`pus_event_identity`, `TIMEOUT 60`, labels `n22;pus;nc-ext-237`) asserts: two
emissions that differ only in `event_id` produce different frames each carrying
the requested id; big-endian order; no duplication of the id in the auxiliary
data; `0x0000`/`0xFFFF` encoded rather than treated as sentinels; auxiliary data
that itself looks like an id/type triple passed through unaltered; severity →
subservice with the raw type octet preserved; and the out-of-bound arguments
(`len = FF_PUS_MAX_TM_FRAME - 2` and `UINT16_MAX` → `FF_BAD_SIZE` with no frame,
`NULL` + non-zero length → `FF_INVALID_ARGUMENT`, exact budget
`FF_PUS_MAX_TM_FRAME - 3` accepted).

Mutation control (`docs/vv/evidence/2026-11-11-nc-ext-237/mutation-check.log`):
restoring the pre-fix body yields `checks=106 failures=33`, exit 1; restoring
the fix yields `checks=106 failures=0`, exit 0. The oracle therefore detects the
defect it documents.

### Residual open items

NC-EXT-237 remains **OPEN** pending acceptance by the project/Software PA
(acceptance authority **Nicola**; owner role FSW engineer; no acceptance date).
Event ids are still not a controlled catalogue (NC-EXT-102). T3 remains **NOT
FULLY QUALIFIED** for this candidate: neither the new script nor
`test_sched_events.c` has an N10 review (`tool-qualification.md` rev 1.6). No
NC is closed by this cycle and no release/flight approval is claimed.

## NC-EXT-191 link-backend deadline writers — 2026-11-11

**Scope:** the `FF_Link_t::send_until` contract in `src/transport/link/link.h`,
its IP and TSN implementations (`src/transport/link/link_ip.c`,
`src/transport/link/link_tsn.c`), the `FF_LinkSendUntil` panic-report path
installed by `src/main/fsw_main.c` (SRS-REQ-074), and the new cases in
`tests/unit/test_link_ip.c` / `tests/unit/test_link_tsn.c`.

**Defect:** `FF_LinkSendUntil` returns `FF_NOT_IMPLEMENTED` when the selected
backend leaves `send_until == NULL`, and both the IP and TSN backends did, so the
pre-reset S5 report could not be delivered on two of the four backends (loopback
and UART already implemented it). This is the backend-coverage half of
NC-EXT-191.

**Change:** `ip_send_until` / `tsn_send_until` follow the pattern already used
by `loop_send_until` / `uart_send_until`, with validation factored into
`ip_check_tx()` / `tsn_check_tx()`: validate readiness/arguments → reject an
already-expired deadline **before** transmitting → send → re-check and report
`FF_TIMEOUT` on overrun. The `link.h` struct comment now states the return
contract (entry rejection, post-transfer re-check, `FF_TIMEOUT` on overrun,
`NULL` ⇒ `FF_NOT_IMPLEMENTED`). `ip_check_tx()` also rejects
`data == NULL && len != 0` in the plain `ip_send()`, which previously forwarded
that argument to the stack (defect fix; the other backends already rejected it).

**Documented limitation:** `FF_NetStack_t::send` and `FF_EthMac_t::transmit` are
synchronous submissions, so a stalled driver cannot be preempted. The deadline is
an entry/post-check bound, not a preemption guarantee; the target 250 ms bound
remains GAP-A5 / N22-TS2-78 (HIL scope).

### Executed evidence (current tree, `BUILD_DIR=build2/native`)

| Gate | Result | Evidence |
| --- | --- | --- |
| Configure + build | exit 0 | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-build.log` |
| CTest | **56/56 PASS** | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-ctest.log` |
| Lean 4 | `lake build` exit 0 | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-lean.log` |
| Traceability | PASS (120/76/30/14/68) | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-trace.log` |
| Format | pinned clang-format 18.1.3, exit 0 | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-format.log` |
| GSW / metrics / N13 license | `make gsw-selfcheck` OK | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-license-gsw.log` |
| Smoke (native simulator) | PASS, 7 applications | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-smoke.log` |
| Coverage / metrics | 56/56, 15328/32948 (46.5%) | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-coverage-metrics.log` |
| RTEMS 6.2 cross build | exit 0, `frick_fsw` linked | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-rtems-cross-build.log` |
| Renode N18 | `RENODE_TEST: PASS` | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-renode-n18.log` |

The Renode run used the ELF rebuilt for this change
(sha256 `55188f5a460bf7c2a97f4540bd8a12f571da56d276394c68a1be25b6a9251f84`), so
it does not rest on a stale image.

### Discriminating oracle and mutation control

New cases (registered in existing binaries, so the CTest count stays 56):

- `tests/unit/test_link_tsn.c::test_tsn_send_until_deadline` (149 checks total):
  `send_until != NULL`; in-budget delivery (60-octet padded frame reaches the
  mock MAC); expired-on-entry `FF_TIMEOUT` with **no** transmission;
  post-transfer overrun `FF_TIMEOUT` with the frame transmitted; `NULL` data /
  `len == 0` / `len > FF_LINK_TSN_RX_FRAMESZ` → `FF_INVALID_ARGUMENT`.
- `tests/unit/test_link_ip.c::test_ip_send_until_deadline` (93 checks total):
  same matrix for the UDP datagram path, plus the `NULL` + non-zero-length
  rejection in both `send_until` and `send`.

Determinism: the tests freeze the monotonic clock with the host-only
`FF_PalMonotonicTestSet()` seam, and the mocks advance it during the transfer
(`g_mock_transmit_advance_ns` / `g_mock_net_send_advance_ns`); no wall-clock
timing is involved.

Mutation control
(`docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/mutation-check.log`):
removing the entry check produces 1 failure per binary; removing the
post-transfer re-check produces 1 failure per binary; restored sources produce
0 failures. The cases therefore discriminate the two behaviours they claim.

### Residual open items

NC-EXT-191 remains **OPEN**: only its backend-coverage half is addressed. The
UART shared-descriptor concurrency and target readiness-semantics parts are
unchanged and are not claimed. The target 250 ms panic-report bound, on-target
link behavior and timing remain GAP-A5 / N22-TS2-78 (HIL). T3 remains **NOT
FULLY QUALIFIED** for this candidate: `test_link_ip.c` and `test_link_tsn.c` have
no N10 review at these hashes (`tool-qualification.md` rev 1.7). No NC is closed
by this cycle and no release/flight approval is claimed.

## S8-PUS-FU S1 ingestion failure paths — 2026-11-12

**Scope:** `tests/unit/test_pus.c` only — `test_verification_failure_via_ingest`,
`stream_has_verification`, a bounds guard on the pre-existing `stream_has`, and
new assertions for no completion failure after acceptance failure plus a buffer
length precondition before direct Request-ID reads. No production source
(`src/`) changed; `src/pus/pus.c` is byte-identical to HEAD (sha256
`1eea40c0…5401`).

**Characterization evidence added; requirement gap remains:** the illegal-TC-APID
rejection/acceptance-failure policy in `N22-TS1-23` is not established by the
cited SRS/ICD text; ICD §1 leaves primary-header acceptance policy open under
NC-EXT-13. Existing `test_pus_s11_residual.c` (declared but unregistered,
NC-EXT-240) and `test_decoder_corpus.c` already exercised non-TC-APID rejection
at packet level. The new test adds byte-level characterization for APID `0x011`
with ack = ACCEPTANCE: current `FF_ERROR`, exactly one TM (1,2), no TM (1,8),
the echoed service/subservice and Request-ID octets (source `0x0102`,
provisional destination `0x0001`), no TM (1,1), and non-dispatch of the ping
handler. TS-1 rev 1.12 classifies N22-TS1-23 as `gap`, not covered. The test also
re-asserts the already-covered `N22-TS1-20` handler-not-found branch at
payload-octet level (TM (1,8) codes 8 and 9).

**Oracle authority and open interface gap:** the 9-octet S1 payload order
(Request ID [6] + service + subservice + code) is derived from
`src/pus/pus.c::FF_PusSendVerification`; ICD §4 describes S1 field content but
does not control that layout. The destination identifier is provisional. This
ICD/implementation divergence is registered as **NC-EXT-241**, owned by the
interface authority and unscheduled pending its decision. Concrete failure-code
allocations remain provisional under NC-EXT-18. The `FF_ERROR` / `FF_NOT_FOUND`
return-code assertions characterize the implementation; no API return contract
is documented in `pus.h`.

**Ack = 0 variants are characterization checks, not requirement-derived
oracles.** Sub-cases 2 and 5 (illegal APID `0x020` with ack = 0; TC (17,9) with
ack = 0) pin the observed `FF_ERROR` / `FF_NOT_FOUND` with no TM. SRS-REQ-005
states an unconditional completion failure while the baseline reports one only
when the corresponding ack flag is set; the conflict is registered as
`N22-TS1-104` (`gap`) and cross-referenced to `N22-TS1-22` (NC-EXT-04, NC-EXT-47
§7.3). Resolving NC-EXT-04 may invalidate these characterization checks and
requires re-disposition of the test and case row. TS-1 rev 1.12 is 104 cases =
76 `covered` + 28 `gap`; N22-TS1-23 was reclassified as a gap after R1/R3
review, not counted as covered.

### Executed evidence (ordered fresh run, `BUILD_DIR=build/evidence-20261112-pus-s1-r4`)

Configure, build, CTest and the focused test were run in that order in the same
fresh build directory. The retained full build log has no compiler warning
lines; project C targets use `-Wall -Wextra -Werror`, while configure logs
ASN1SCC generator diagnostics separately (endianness properties with no effect;
not C compiler warnings). The retained log sequence identifies source and build
configuration; exact test/CTest binaries are copied under `r4/artifacts/`. This
table records execution evidence, not the package checksum outcome. The exact
manifest digest, path/exclusion inventory check, and `sha256sum -c` result are
recorded only by the three pre-indexed post-manifest attestation reports; any
missing report, failure, or subsequent covered-file change means the package
must not be treated as currently integrity-verified.

| Gate | Result | Evidence |
| --- | --- | --- |
| Candidate identity | Baseline HEAD and exact eleven-path tracked working-tree delta are retained with path hashes; unrelated untracked paths excluded | `.../r4/candidate-identity.txt`, `.../r4/candidate-working-tree.patch` |
| Configure (`FF_USE_PUS_ASN1=ON`) | exit 0 | `.../r4/gate-configure.log` |
| Build (full output) | exit 0; no compiler warnings observed; project C targets enforce `-Werror` | `.../r4/gate-build.log` |
| Required `make build` / `make test` targets | Both exit 0 with isolated `BUILD_DIR=build/evidence-20261112-pus-s1-r4`; default `build/native` contains an older Docker `/workspace` cache, so its path validation fails and is not overwritten | `.../r4/correction-make-build.log`, `.../r4/correction-make-test.log`, `.../r4/gate-make-cache-preservation-attempt.log` |
| CTest | **57/57 PASS** (registered tests only; NC-EXT-240 target excluded); invocation inventory and 55 distinct compiled executables retained; submanifest verification 55/55 | `.../r4/gate-ctest.log`, `.../r4/artifacts/ctest-list.json`, `.../r4/artifacts/ctest-binaries.sha256`, `.../r4/artifacts/ctest-binaries-verify.log` |
| `test_pus` binary | `checks=322 failures=0`; exact executable retained at `.../r4/artifacts/test_pus` | `.../r4/gate-test-pus.log`, `.../r4/artifacts/identities.txt` |
| Container CI subset | `make ci-local` exit 0; 57/57 CTest, Lean receipt and trace pass | `.../r4/correction-ci-local.log` |
| Lean 4 | `lake build` exit 0 | `.../r4/gate-lean.log` |
| Lean elaboration receipt | 11 modules elaborated, receipt validation PASS | `.../r4/gate-lean-receipt.log` |
| Traceability | PASS (120/76/30/14/68) | `.../r4/correction-trace.log` |
| Format (host clang-format 23.1.3, unpinned) | exit 0 for the project tree | `.../r4/gate-format.log` |
| Format (pinned clang-format 18, full repository) | **FAIL**, unchanged initializers at `src/transport/link/link_tsn.c:18`, `src/transport/link/link_ip.c:19`, `tests/unit/test_link_tsn.c:24`; registered as NC-EXT-244, deferred under N21 | `.../r4/gate-format-pinned.log` |
| Format (pinned clang-format 18, changed `test_pus.c`) | exit 0; a later host clang-format 23 check is retained but is not used as pinned-tool evidence | `.../r4/correction-format-pinned.log`, `.../r4/correction-format-changed-file.log` |
| N13 license / GSW self-check | `make gsw-selfcheck` exit 0; includes authored-file license policy and Python GSW/metrics/license tests | `.../r4/correction-gsw-selfcheck.log` |
| Native simulator smoke | PASS (banner + 7 apps) with isolated `BUILD_DIR`. Default-path attempt failed CMake validation against retained `/workspace` cache and is not a software smoke failure. | `.../r4/correction-smoke.log`, `.../r4/attempt-smoke-default-cache.log` |
| Doxygen docs generation | `make docs-container` exit 0 | `.../r4/correction-docs-container.log` |
| N18 environment self-test | Renode starts and PolarFire SoC platform loads; exit 0 | `.../r4/gate-renode-env.log` |
| N18 fresh RTEMS image + Renode | Fresh ELF SHA-256 `e36aee30…4843a41`, mtime 2026-11-13 00:42:40 +0100; exact ELF retained at `.../r4/artifacts/frick_fsw_renode.elf`; `RENODE_TEST: PASS`, container exit 0 | `.../r4/gate-renode-image-docker.log`, `.../r4/gate-renode-fresh.log`, `.../r4/renode-run/`, `.../r4/artifacts/identities.txt` |

The initial r3 N18 invocation used a stale ELF (SHA-256 `55188f5a…`) and is
retained only as invalid historical evidence in `gate-renode.log`. The r4
candidate image was subsequently rebuilt using the controlled Docker toolchain
image and a fresh `make renode` passed against ELF SHA-256
`e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`; raw
process/UART/status files are retained and hash-bound under `r4/renode-run/`.
This remains emulated integration evidence only: no physical target, RTEMS
runtime on hardware, HIL or hardware-timing claim. The changed C test passes pinned
clang-format 18; the full pinned-format gate's unrelated existing initializer-format failures are
tracked as NC-EXT-244 and remains open.

### Discriminating oracle and mutation control

`.../r4/mutation-check.log` records the source baseline hash, test hash, exact
mutations, mutated hashes and restoration proof:

- `FF_PUS_FAIL_ILLEGAL_APID` → `FF_PUS_FAIL_BAD_DATA` in `src/pus/pus.c:424`
  (sha256 `1eea40c0…` → `1df235d2…`) fails 1 check (`test_pus.c:735`).
- Removing the `tc.service == 0u` code selection at `src/pus/pus.c:439`
  (→ `cd2b01fa…`) fails 2 checks (`test_pus.c:788-789`).
- Restored source (`1eea40c0…`) fails 0 checks.

### Pre-existing finding registered, not fixed here

`tests/unit/test_pus_s11_residual.c` is declared in `FF_TEST_TARGETS` but not
registered with CTest; direct invocation reports `checks=1164 failures=10`
because S3/S6 expectations use service subservice numbers not implemented by
the ADR-0020 provisional mapping. It is outside this test-only delta and is
recorded as **NC-EXT-240** (owner FSW, M17); no pass or closure is claimed.

### Review evidence and residuals

The first PA review found major record defects; its correction re-review
addressed those findings in substance. The earlier R2/V&V review approved a
prior candidate with comments. Fresh V&V/R2 review
`VV-R2-20261113-s8-pus-fu-s1-ingestion-failures.md` found a Major
manifest-integrity/currentness gap on the then-current snapshot: four hashes
were stale and successful isolated Make-target logs were absent. The exact
focused-test and Renode binaries and a 55-executable CTest inventory have since
been retained under `r4/artifacts`; the manifest must still be regenerated and
independently verified. V&V/R2 delta review confirmed the artifact inventory
and corrected status wording, but the Major remains open until package-level
manifest verification succeeds. The fresh R4 correction re-review `2026-11-13-r4-pus-s1-ingest-correction-rereview.md`
found DOC-R4-RR-01 Major (manifest and review-status currentness) and
DOC-R4-RR-02 Minor (N12/N14 audit inconsistency). The R4 delta resolved
DOC-R4-RR-02 and left DOC-R4-RR-01 partially resolved/open for the package
manifest correction. PA independently confirmed the retained CTest inventory
(57 cases, 55 binaries) and 55/55 hash/copy identity, resolving that persistence
residual in substance; final package-manifest and PA delta confirmation remain
pending.
Avionics R1/R3 correction re-review SYS-R1R3-20261113-PUS-S1-INGEST-RR
passed with comments and resolved SYS-R1R3-01 in substance; its two Minor sync
observations were addressed and independently confirmed with PASS in
`SYS-R1R3-20261113-PUS-S1-INGEST-SYNC-CONFIRMATION.md`. PA-R3 independently
verified the initial 45-entry r4 manifest and resolved PA-N20-R3-01 in
substance for those inspected bytes. The report is itself manifest-bound and
has since changed, and the R1/R3 correction/synchronization reports have now
been added; final manifest rebind and PA delta confirmation remain pending.
Reports are retained and indexed in `docs/reviews/README.md`; this record does
not claim any review gate closed until the correction re-reviews pass.

NC-EXT-15 remains open. NC-EXT-04, NC-EXT-13, NC-EXT-14..18 and NC-EXT-47
authority/profile gaps remain open. NC-EXT-241 records the S1 payload interface divergence;
NC-EXT-242 tracks TS-1 case ordering; NC-EXT-243 tracks the unsynchronized
Request-ID sequence counter; NC-EXT-244 tracks the unrelated pinned-format
discrepancy. T3 remains **NOT FULLY QUALIFIED** for this
candidate (`tool-qualification.md` rev 1.9). No requirement or ICD clause is
added; no NC is closed; no release/flight approval is claimed.

### Staging scope and manifest history

This section retains the S8-PUS-FU S1 review sequence and earlier manifest
snapshots. The substantive S1 reports include `PA-REVIEW-PUS-S1-INGEST-VERIFICATION-FAILURES-20261112.md`,
`PA-R2-20261112-pus-s1-ingest-verification-failures.md`,
`VV-R2-20261112-s8-pus-fu-s1-verification-failures.md`,
`SYS-R1R3-20261113-PUS-S1-INGEST-CORRECTION-REREVIEW.md`,
`SYS-R1R3-20261113-PUS-S1-INGEST-SYNC-CONFIRMATION.md`,
`PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`,
`VV-R2-20261113-s8-pus-fu-s1-evidence-delta.md`,
`VV-R2-20261113-s8-pus-s1-manifest-scope-method-review.md`,
`PA-N20-20261113-s8-pus-s1-final-inventory-delta.md`,
`VV-R2-20261113-s8-pus-s1-final-inventory-delta.md`,
`2026-11-13-r4-pus-s1-final-inventory-delta.md`,
`VV-R2-20261113-s8-pus-s1-candidate-identity-delta.md`, and the substantive R4
report `2026-11-13-r4-pus-s1-final-substantive-documentation-review.md`. These
are retained as findings/results for their identified snapshots; the prior
45-entry manifest and 157-entry preview are historical and do not bind the
UART-inclusive candidate.

**Historical path correction:** the original S1-pre-registered report paths
`PA-N20-20261113-s8-pus-s1-final-manifest-attestation.md`,
`VV-R2-20261113-s8-pus-s1-final-manifest-attestation.md`, and
`R4-N17-20261113-s8-pus-s1-final-manifest-attestation.md` are not the combined
candidate's exclusions. Those files are included as historical review evidence.
The current combined-candidate manifest excludes only itself and these exact
pre-registered reports:

- `PA-N20-20261113-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`
- `VV-R2-20261113-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`
- `R4-N17-20261113-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`

The current manifest scope, path inventory, and checksum/attestation method are
specified in `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/README.md`
and pre-registered in `docs/reviews/README.md`. The complete candidate inventory
includes all substantive reports and evidence, including this correction review;
only the manifest itself and the three exact reports above are excluded. The
three independent reports must identify one digest, inspect the exact path set
and exclusions, and record successful `sha256sum -c` results. They are inventory
attestations, not substantive N17/N20 approval, N10 qualification, or NC closure.
Any covered-file change invalidates the manifest and requires a new freeze and
applicable reviews. The final Git tree is intended to bind the manifest and
three detached reports; no commit identity or release approval is claimed.

Other pre-existing untracked review reports from separate increments
remain untouched and outside this evidence scope:
`2026-11-11-r4-doc-review-nc-ext-237-nc-ext-191.md`,
`2026-11-11-vv-r2-nc-ext-239-independent-vv-review.md`,
`2026-11-12-r4-doc-review-icd-s1-subservice-4-6-annotation.md`,
`2026-11-12-r4-doc-review-pus-services-ordering-nc-ext-211.md`,
`FRICK-REVIEW-0479-R4-DOC-NC-EXT-239.md`,
`FRICK-REVIEW-0480-VV-N22-TS5-47-SCHED-TM-CHAIN.md`,
`PA-REVIEW-ICD-PUS-S1-4-6-20261112.md`,
`PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md`,
`PA-REVIEW-PUS-PAYLOAD-INIT-20261112.md`,
`PA-REVIEW-UART-SEND-UNTIL-RESTORE-RESULT-20261112.md`,
`SYS-REVIEW-R1-R3-20261111-avionics-systems-review.md`,
`SYS-REVIEW-R1-R3-20261112-icd-s1-subservice-4-6-annotation-avionics-systems-review.md`,
`SYS-REVIEW-R1-R3-20261112-lean-watchdog-formal-fix-avionics-systems-review.md`,
`SYS-REVIEW-R1-R3-20261112-nc-ext-178-avionics-systems-review.md`,
`SYS-REVIEW-R1-R3-20261112-pus-c-payload-zero-init-avionics-systems-review.md`,
`SYS-REVIEW-R1-R3-20261112-pus-services-ordering-nc-ext-211-avionics-systems-review.md`,
`SYS-REVIEW-R1-R3-20261112-sched-tm-chain-test-increment.md`,
`SYS-REVIEW-R1-R3-20261112-uart-send-until-restore-result-avionics-systems-review.md`,
`VV-R2-20261111-nc-ext-237-independent-vv-review.md`,
`VV-R2-20261112-icd-pus-s1-4-6-not-emitted.md`,
`VV-R2-20261112-nc-ext-178-register-discipline.md`,
`VV-R2-20261112-pus-svc3-init-ordering.md`, and
`VV-R2-20261112-uart-send-until-concurrency-review.md`.
The separate worktree items `.opencode/plugins/`, `opencode.json`, and
`tests/unit/test_sched_events.c.bak` are also untouched and excluded. None of
these pre-existing paths is staged or included in this candidate.


## S8-FU UART `send_until()` restore-failure correction — 2026-11-13

**Status: bounded code/test change implemented; local gates and substantive
independent reviews are recorded; package-wide evidence binding is pending.** This change follows
PA finding PA-001 in `PA-REVIEW-UART-SEND-UNTIL-RESTORE-RESULT-20261112.md`.
`uart_send_until()` now checks the temporary `fcntl(F_SETFL, original_flags)`
restore result after every attempted write, returns `FF_ERROR` without retry on
failure, and snapshots `errno` immediately after `write()` so a successful
restore cannot change error classification. It does not claim that the OS
restored flags after a failed `fcntl`; the shared descriptor may remain
nonblocking.

### Executed evidence

Evidence logs, raw Renode captures, ELF, test executable, and input identities
are retained under `docs/vv/evidence/2026-11-13-uart-restore-error/`; the README
there records each command/result and limits. The candidate source/test
identity at the time of the fresh run is:

- `src/transport/link/link_uart.c` sha256
  `c39c3f271680d0d0c838d10fc72039e5ea65ee0f2fda8df7c56080200737cc99`;
- `src/transport/link/link.h` sha256
  `45e3f58337546d7485efd0e477d042a788e6ca0fd7b88e4e6e14d54d122b1961`;
- `tests/unit/test_link_uart.c` sha256
  `11d18353f7e006a13cf7897055e478e31619adb9ecbd33ecc83de115303f5006`;
- `tests/CMakeLists.txt` sha256
  `68346402e3fa1ad0cc665669cc410d67165753ddf6d03544f29d4b96e1e7d038`;
- `build/uart-full/tests/test_link_uart` sha256
  `726d1052bde18396860a0fd9d1ed354a534e29ee7d0aadc4bc8ac7d47b041b3d`.

Fresh full native build and CTest passed (**57/57**); focused `test_link_uart`
reported **104 checks, 0 failures**, and registered CTest `link_uart` passed 1/1.
N22-TS2-84 has two explicit host PTY subcases: (a) `EAGAIN` + restore failure,
asserting `FF_ERROR`, exactly one write attempt, no transmitted bytes, and the
still-nonblocking descriptor before fixture cleanup; (b) `EAGAIN` + successful
restore that clobbers `errno`, then successful retry and exact payload receipt,
asserting two write attempts. N22-TS2-85 covers final-write deadline overrun:
the delayed accepted transfer returns `FF_TIMEOUT` and exact transmitted bytes
are asserted. N22-TS2-86 checks an expired zero-length request returns
`FF_TIMEOUT` with no write attempt. A final positive write is post-checked against the monotonic deadline. N22-TS2-85
asserts `FF_TIMEOUT` plus exact emitted bytes when the accepted final write
completes after deadline (timeout does not mean no bytes were transmitted). The
expired zero-length API edge is N22-TS2-86. These API subcases are not
SRS-REQ-074 compliance or full panic-path evidence.

`make verify-lean`, `make trace` (120/76/30/14/68), changed-file pinned
clang-format 18, `make gsw-selfcheck` including N13 license checks, isolated
`BUILD_DIR=build/uart-full make smoke`, `make docs-container`, `make rtems-docker`,
and `make renode` all passed. Renode reported `RENODE_TEST: PASS` (container exit
0) on the newly rebuilt RTEMS image; it is emulator-only evidence, not physical
target, HIL, timing/WCET, qualification or flight evidence. Logs capture exact
commands/results.

### Limitations and incomplete gates

Full pinned clang-format 18 `make format-check` still fails only on the three
unchanged initializers in `src/transport/link/link_tsn.c:18`,
`src/transport/link/link_ip.c:19`, and `tests/unit/test_link_tsn.c:24`
(NC-EXT-244, deferred; changed C files pass). `make metrics` completed and
refreshed the README status panel. The first default `make build` and `make smoke`
attempts hit the pre-existing `build/native` CMake cache bound to `/workspace`;
no cache was removed. Both were rerun successfully with the isolated
`build/uart-full` directory. Metrics completed with current outputs. T3 remains
**NOT FULLY QUALIFIED**; successful execution is not qualification.

Independent R1 re-review found no functional defect in the restore, errno,
post-write deadline or expired zero-length cases; R3 found no new defect in the
deadline/API edges. The evidence/docs delta reviews confirm the UART-local
run-assets list, 104-check result, traceability mirror, tool-register hash and
status/date corrections. The R2/R4 reports explicitly do not claim candidate-wide
integrity because the old r4 manifest remains stale. PA/N20 substantive review
found no new product defect in the bounded UART scope and is recorded in
`PA-N20-20261113-UART-RESTORE-DEADLINE-SUBSTANTIVE-REVIEW.md`. Candidate-wide
integrity is controlled by the active r4 inventory/manifest and three
independent post-manifest reports; the local UART run-assets checksum is not a
substitute. Verify the exact digest and report results in that package.
NC-EXT-191 remains OPEN for shared-descriptor effects (especially after actual
restore failure), RTEMS target-readiness semantics and target 250 ms timing.
No NC, sprint, qualification, release, or flight decision is closed or approved.

## ICD §4 S3/S6 dispatch-contract correction — 2026-11-13

**Scope:** `docs/interfaces/ICD.md` rev 1.8 §4 (Service 3 and Service 6 rows),
the S3/S6 residual dispatch test `tests/unit/test_pus_s11_residual.c` (now
registered in CTest), and the citing text that repeated the superseded
allocation (`src/pus/pus_services.c` comments, `docs/architecture/pus-layer.md`
rev 0.1.9, `README.md`, `docs/adr/ADR-0020` rev 0.4, TS-1 rev 1.13,
`docs/vv/target-io-coverage.md`, `docs/vv/xw3-tc-oracle-matrix.md`, the XW-2/XW-3
gap manifests, `src/apps/app_req_test.c`).

**Defect (documented-vs-implemented divergence):** ICD §4 still documented the
superseded Service 3 allocation (1=create, 3=delete, 5=enable, 6=disable,
27=one-shot) and the superseded Service 6 dump subservice (6,5), while
`FF_PusRegisterCoreServices()` registers 1=enable, 2=disable, 3=periodic alias of
enable, 4=one-shot request, 25=report TM, 128=create, 129=delete, and the S6 dump
handler is subservice 2. A ground segment built to the old row would have had a
create executed as an enable.

**Change:** ICD rev 1.8 records the implemented, owner-approved (2026-11-10)
provisional contract, the 6-octet create payload
(`report id + provider id + period_s[4 BE]`), the periodic-emission caveat
(`FF_PusEmitPeriodicHk()` has no production caller, ESC-HK-02) and the
authority-pending create width (ESC-HK-01, NC-EXT-210). **No production wire
behaviour changed by this revision.** NC-EXT-16/17/18 remain CLOSED for
implementation only; formal mission ICD entry remains M15 scope.

### Executed evidence (current tree)

| Gate | Result | Evidence |
| --- | --- | --- |
| Configure + build (`BUILD_DIR=build/uart-full`) | exit 0 | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/build.log` |
| CTest | **58/58 PASS** | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/ctest.log` |
| Focused `pus_s11_residual` | 1176 checks, 0 failures | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/pus-s11-residual.log` |
| Changed-file clang-format (23.1.3, unpinned) | exit 0 | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/format-changed-files.log` |
| Lean 4 | `lake build` exit 0 | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/lean.log` |
| Traceability | PASS (120/76/30/14/68) | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/trace.log` |
| GSW / metrics / N13 license | `make gsw-selfcheck` OK | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/gsw-selfcheck.log` |
| Smoke (native simulator) | PASS, 7 applications | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/smoke.log` |
| Coverage / metrics | 58/58; 15924/33832 (47.1%) lines, 1796/2640 (68.0%) functions | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/metrics.log` |
| Docs (Doxygen) | exit 0, `build/doxygen/html/index.html` non-empty | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/docs-container.log` |
| RTEMS 6.2 cross build (`make rtems-docker`) | exit 0, `frick_fsw` linked | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/rtems-docker.log` |
| Renode N18 (`make renode`) | `RENODE_TEST: PASS`, container exit 0 | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/renode.log`, `renode-run/` |
| Python verdict fixtures | 17 tests OK | `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/python-fixtures.log` |

The Renode run used the RTEMS image rebuilt from the current tree in this cycle
(ELF sha256 `41acb18583ac25a6deb671274e1400fe4db8b84f6c105958f22959db0660cb20`,
`renode-elf.sha256`), so it does not rest on the earlier S1-increment image. It
is emulator-only evidence, not physical target, HIL, timing/WCET, qualification
or flight evidence.

### Discriminating oracle

`tests/unit/test_pus_s11_residual.c` (registered CTest `pus_s11_residual`,
labels `n22;pus;nc-ext-14;nc-ext-15;nc-ext-16;nc-ext-18;nc-ext-22`) drives the
implemented dispatch directly: S3 128=create, 1=enable, 2=disable, 3=periodic
alias, 4=one-shot request, 129=delete; S6 2=dump; plus S9/S17/S20 dispatch and
the S1 verification chain. It asserts that the superseded `{5,6,27}` subservices
are **not** the registered handlers (`FF_NOT_FOUND`), so a regression back to the
old allocation fails the test rather than passing silently. 1176 checks, 0
failures.

The new test artifact is also added to the `tests` lists of SRS-REQ-005,
SRS-REQ-008, SRS-REQ-009, SRS-REQ-010 and SRS-REQ-012 in
`docs/requirements/traceability.yaml` and the human-readable mirror
`docs/requirements/traceability.md` (norm N3/N4); `make trace` still reports
PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 Lean theorems). No
requirement text, status token or Lean theorem changed.

### Limitations and incomplete gates

Full pinned clang-format 18 `make format-check` remains blocked on the host
(only clang-format 23.1.3 installed) and on the three unchanged initializer files
(NC-EXT-244, deferred); the changed C files pass `clang-format --dry-run
--Werror` under 23.1.3. This is not equivalent to the pinned-18 gate and is
recorded as a limitation. `make metrics` refreshed the README status panel
(coverage and review-stream figures changed). T3 remains **NOT FULLY QUALIFIED**;
successful execution is not qualification. No NC is closed, no authority decision
is taken, and no sprint, release or flight decision is approved.
