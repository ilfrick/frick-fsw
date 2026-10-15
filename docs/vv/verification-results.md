# Verification and Validation Results

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RES-001                       |
| Revision                          | 2.11                                   |
| Date                              | 2026-10-15                             |
| Baseline cycle                    | v0.1.0 initial release                 |
| Norms                             | N6, N11 (records retained each cycle; publication after gates pass) |

This log is the running verification record. Every development/verification
cycle appends a row with the evidence produced and identifies the configuration
and artifact state; the current configured SB/EVS structural stress row describes
an uncommitted working tree.
Emulator console captures (Renode `build/renode/console.log`, QEMU
`build/rtems-generic/qemu-console.log`) are regenerable build outputs (not
tracked), reproduced by `make renode` / `make qemu` from the committed sources,
toolchain and scripts.

## Cycle log

| Cycle / date | Build (host) | ctest | Lean | trace | Smoke | Static | RTEMS cross | Comment |
| ------------ | ------------ | ----- | ---- | ----- | ----- | ------ | ----------- | ------- |
| `make docs` output-existence guard, 2026-10-15 | NOT RUN (Makefile/workflow-only change) | NOT RUN (no code change) | NOT RUN | NOT RUN (traceability unchanged) | NOT RUN | PASS (guard validation in `frick-vv:local`, Doxygen 1.9.8: pristine tree `make docs` exit 0 with `build/doxygen/html/index.html` 57,394 B; a tree with `GENERATE_HTML = NO` reproduced the silent no-op — Doxygen exited 0 with `warning: No output formats selected!` and produced no HTML — and the guard then failed `make docs` with exit 2 and the explicit message; removing `index.html` from a successful tree also fired the guard) | NOT RUN | Adds the output-existence assertion raised as Comment 5 of the scoped PA delta review FRICK-REVIEW-0284. The guard is in the `docs` target (covering host + both forge jobs) and the two workflow `docs` jobs also assert `build/doxygen/html/index.html` and echo `doxygen --version`, so a silent generation regression fails the gate rather than only the step exit status. This addresses a silent-failure class and closes no release, qualification, hosted-evidence or NC gate; the loop-audit currency statements updated in the same delta are documentation-only. Scoped R4 FRICK-REVIEW-0285 / PA FRICK-REVIEW-0286 reviewed this delta APPROVED WITH COMMENTS, and R4 FRICK-REVIEW-0287 verified the R4 NC-01/NC-02 currency actions closed. Hosted evidence: GitHub Actions run #84 on `77cd548` (this guard delta, `https://github.com/ilfrick/frick-fsw/actions/runs/38695319390`) succeeded, all 8 jobs including `documentation set` (whose `Doxygen` step carries the new assertion) and the pipeline summary, as did run #85 on the record-only follow-up `03366f1` (all 8 jobs, `https://github.com/ilfrick/frick-fsw/actions/runs/38695861328`); no Gitea run log is retained (NC-EXT-36). Per the record-only convention in the loop audit (N15), record-only/evidence-only commits are verified by their own hosted run but not recursively re-recorded, so the run for this very record update is not separately logged. |
| CI docs-job fix (Doxygen output dir), 2026-10-15 | NOT RUN (docs/Makefile-only change) | NOT RUN (no code change) | NOT RUN | NOT RUN (traceability unchanged) | NOT RUN | PASS (pristine-tree CI-docs replication in `frick-vv:local`, Doxygen 1.9.8: unrecovered failure reproduced first (`Output directory 'build/doxygen' ... cannot be created`, Doxygen exit 1 / `make` exit 2), then `make docs` exit 0 and `build/doxygen/html/index.html` present after the fix; documentation-set id uniqueness 248; reviewer-persona drift in sync; N13 license check OK; `make gsw-selfcheck` 5 GSW + 9 metrics + 14 license Python checks) | NOT RUN | Root cause: Doxygen 1.9.8 creates `OUTPUT_DIRECTORY` with a single-level `mkdir(2)`, so `mkdir("build/doxygen")` fails `ENOENT` when a fresh clone has no `build/`; the `docs` target now runs `mkdir -p build/doxygen` before `doxygen Doxyfile`. Verified against a pristine copy with no `build/`, replicating the GitHub `docs` job. Pre-existing Doxygen warnings are unchanged and non-fatal. Hosted CI run #82 (commit 03773f7) executed: the `docs` job passes; the `build` job's *Metrics panel currency* step flagged the README panel after the two review reports added a review stream, so the panel was regenerated (`tools/scripts/metrics.py --readme`, review streams 32/78 -> 32/79; `--check` current). This corrects a functional build defect (N21) and closes no release, qualification, hosted-evidence or NC-EXT-43 gate. |
| Full SB/EVS runner wired into CTest, 2026-10-14 | PASS (`make ci-local`; mutable local V&V image) | PASS (30/30; `sb_evs_structural_sweep` ran 9,984 profiles plus dispatch caps 1–64 in 18.70 s) | PASS (`lake build` in `make ci-local`) | PASS (117 requirements; 73 SRS / 30 SSS / 14 GAP; 51 Lean links) | NOT RUN in this focused cycle | NOT RUN in this focused cycle | NOT RUN in this focused cycle | Added full matrix runner to CTest; output CSV is generated under `build/ci/` and is not mission evidence. The earlier retained characterization CSV and source hashes remain the artifact for the manual characterized run; this CI run demonstrates the configured runner exercised under the local container pipeline. Scoped R2/R4/PA reviews were run; see review log. Hosted CI was not run. No claim of WCET, mission envelope, ground receipt, or end-to-end loss semantics. |
| Doxygen root-owned-output recovery, 2026-10-14 | NOT RUN | NOT RUN | NOT RUN | NOT RUN | NOT RUN | PASS (`docker run --rm -v "$PWD:/workspace" -w /workspace frick-vv:local make docs`, exit 0; existing Doxygen warnings) | NOT RUN | Host `make docs` failed because it runs `rm -rf build/doxygen` as the host user against root-owned output left by an earlier container run. Running the same `make docs` target inside `frick-vv:local` successfully removed the output and regenerated documentation. The Makefile target itself is host-executed; the container invocation is the recovery procedure. Generated docs are ignored build output. No source changes; this resolves the Doxygen execution gate for this cycle, not the separate worktree-scope/review/tool-qualification gates. |
| Sweep self-check failed attempt, 2026-10-14 | PASS (`make docs` in V&V image; existing Doxygen warnings) | Not reached | Not run | Not run | Not run | FAIL (`make gsw-selfcheck`: 5 GSW + 9 metrics tests passed; license test failed because new sweep Python script lacked required SPDX header) | NOT RUN | Failure retained per N11. Added the required MPL-2.0 SPDX header to `tools/scripts/sb_evs_structural_sweep.py`; because the runner source changed, rerun the sweep and update its identity before accepting the evidence. No commit/push. |
| SB/EVS stress evidence reconciliation, 2026-10-14 | PASS (fresh Debug configure/build in `build/stress-clean`; plus `make ci-local`) | PASS (28/28 in both clean and CI-local CTest; `test_sb_stress` 454 checks, 0 failures) | PASS (`lake build` in CI-local) | PASS (117 requirements / 51 Lean links) | PASS (`make smoke`, 6 applications; `make renode`, `RENODE_TEST: PASS`) | PASS (clang-format, diff check, containerized `make docs`; existing warnings) | NOT RUN | Supersedes the immediately following earlier stress row, adding clean-build/artifact evidence and scoped review outcomes. All 32 matched queue locks held: `FF_TIMEOUT`, 32 accounted drops/reports; then 32 × depth 32 accepts 1024 copies; three distinct wildcard-routed overflow bursts create 96 more drops/events. EVS queues and dispatches 96 accepted notifications cumulatively, counts 32 notification losses, and keeps the newest 64 of 128 log entries; assertions distinguish retained bursts 2/3 and app-ID order. See `docs/vv/evidence/2026-10-14-sb-evs-structural-stress.md` for profile, selected SHA-256 identities and limits. Docker image upstream inputs remain mutable; identifiers are not a complete immutable manifest. Scoped R4 FRICK-REVIEW-0268 and PA FRICK-REVIEW-0269 are APPROVED WITH COMMENTS only. No WCET/event-rate/target/ground-receipt/SRS-REQ-072 claim; mission profile remains TBD. Initial in-suite fixture failure was due PAL queue-slot exhaustion across service reinitializations; isolated process corrected it; raw initial output was not retained. RTEMS and hosted CI were not run. NC-EXT-75/76 and broader gates remain open. |
| SB/EVS structural parameter sweep, 2026-10-14 | PASS (fresh Debug host configure/build in `build/sb-sweep-final`, GCC 13.3.0 in observed V&V image; final `make ci-local` after runner SPDX header) | PASS (29/29; `sb_sweep_boundary`; 9,984 parameter profiles + dispatch limits 1–64; 1,697,329 checks, zero failures) | PASS (`make ci-local`, Lean build success) | PASS (`make ci-local`, 117 requirements / 73 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`, 6 applications; `make renode`, `RENODE_TEST: PASS`) | PASS (containerized Doxygen completed with existing warnings; `make gsw-selfcheck` Python tests 28/28 + CLI self-check; clang-format; local `git diff --check`) | PASS (fresh `build/rtems-sweep-final` cross-build; GCC 13.3.0 RTEMS 6 toolchain; ELF64 RISC-V) | Retained CSV `docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.csv`, 9,985 data rows and 9,986 lines, SHA-256 `83eafc861ec1fd029043b34f244504bd858ffc0a85fa35431eb6de27773e623a`; test/runner/executable hashes and method in adjacent evidence note. Initial `make gsw-selfcheck` failure from missing SPDX header is retained above; after adding the header, `make ci-local`, full sweep, Python/license self-check, format and diff checks all passed. Hosted CI not run. Fan-out/depth 1–32 crossed with overflow publish count zero through first EVS notification saturation; dispatch cap tested with excess pending for limits 1–63 and at capacity 64. Scoped R3/R2/R4/PA reports FRICK-REVIEW-0270..0274 are recorded; formal T22 N10 acceptance pending. No mission values, WCET, target timing, link or ground-receipt claim. Initial malformed CSV was overwritten and is not retained separately. N14 mission baseline, NC-EXT-75/76 and broader release gates remain open. |
| Configured SB/EVS structural stress, 2026-10-14 | PASS (`make ci-local`; Docker V&V image `frick-vv:local`, mutable inputs) | PASS (28/28; isolated `test_sb_stress` passes) | PASS (`lake build` in `make ci-local`) | PASS (117 requirements / 73 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`, 6 applications) | PASS (`clang-format --dry-run --Werror`; `git diff --check`) | NOT RUN | Stress hypothesis reaches structural maxima: all 32 matched queue locks held yields `FF_TIMEOUT`, 32 drop accounts and 32 deferred reports; after release, 32 × depth 32 accepts 1024 copies; three additional wildcard-routed full-queue publishes with distinct IDs produce 96 SB drops/events. EVS accepts/dispatched 96 notifications cumulatively, counts 32 notification losses at capacity and retains the newest 64 of 128 log records; the oracle checks ordered app IDs and that retained message IDs correspond to overflow bursts 2 and 3. `make smoke`, `make renode` (`RENODE_TEST: PASS`), and containerized `make docs` passed; docs emitted pre-existing warnings. This is host structural saturation only, not WCET, event rate, ground receipt, or SRS-REQ-072 compliance; mission values remain TBD. Initial in-suite placement failed because prior tests consume fixed PAL queue slots across service reinitializations; separate-process isolation resolved this test-fixture collision. Independent delta reviews are pending; RTEMS and hosted CI were not run. NC-EXT-75/76 remain open. No qualification or release approval claimed. |
| SB queue-lock contention behavior, 2026-10-14 | PASS (`make build`; `make ci-local`) | PASS (27/27; direct `build/native/tests/test_sb` = 341 checks, 0 failures) | PASS (`lake build` in `make ci-local`; host `make verify-lean` unavailable: `lake` not found) | PASS (`make trace`, 117 requirements / 73 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`; `make renode`, `RENODE_TEST: PASS`) | PASS (clang-format dry-run; `make misra-docker` advisory scan: 37 findings) | BLOCKED (`make rtems`: pre-existing `build/rtems` cache points at `/workspace`; left untouched) | `make gsw-selfcheck` PASS (5 GSW, 9 metrics, 14 license checks and CLI self-check). Deterministic test holds the first matching pipe mutex through `FF_SbPublish` return, verifies exact `FF_TIMEOUT`, continued copy to second pipe, no copy to held pipe, exact published/delivered/dropped deltas, deferred busy-specific EVS log/notification, and callback only after explicit dispatch. Direct PAL case distinguishes `FF_TIMEOUT` busy from `FF_BUFFER_FULL`; existing full-pipe SB case expects `FF_SUCCESS` and `pipe full`. `make docs` failed because existing root-owned `build/doxygen/html` files could not be removed. Independent R1/R2/R4/PA reviews are pending; NC-EXT-75/76 and SRS-REQ-072 remain open. No NC closure or release approval claimed. |
| SB overflow correction, 2026-10-14 | PASS (`make build`) | PASS (`make test`, 27/27; `test_sb` 48 checks) | BLOCKED (`make verify-lean`: `lake` unavailable) | PASS (`make trace`, 116 reqs / 72 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`, 6 apps) | PASS (`clang-format --dry-run --Werror` on changed C files) | not run | `make renode` PASS (`RENODE_TEST: PASS`); `make docs` failed because existing root-owned `build/doxygen/html` files could not be removed. EVS-log oracle checks `FF_EVS_SB_ERR`/`FF_EVS_ERROR`, app id, overflow message, retained first message, and exact dropped/delivered statistics. Working tree uncommitted; NC-EXT-47 and other NCs remain open; no qualification/NC closure claimed. |
| SB overflow R1 follow-up: deferred EVS callbacks, 2026-10-14 | PASS (`make build`; `make ci-local`) | PASS (27/27; `test_sb` 152 checks, `test_r2_gap` 581, `test_r3_gap` 205) | PASS (`lake build` in `make ci-local`; host `lake` unavailable) | PASS (`make trace`, 116 requirements / 72 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`, 6 applications; `make renode`, `RENODE_TEST: PASS`) | PASS (`clang-format --dry-run --Werror`); `make misra` in Docker reports 36 advisory findings total, with no findings in modified EVS/TM/test files (one `sb.c` unknown-type configuration warning) | BLOCKED (RTEMS compiler `/opt/rtems/6/bin/riscv-rtems6-gcc` absent; existing `build/rtems` cache also belongs to `/workspace`) | `make ci-local` fully PASS (configure/build/27 CTest/Lean/trace). `make gsw-selfcheck` PASS (5 GSW, 9 metrics, 14 license checks + CLI self-check). Host `make docs` failed on permissions for pre-existing root-owned output; `docker run ... make docs` PASS with pre-existing Doxygen warnings. Tests demonstrate full-pipe publish returns before callback, explicit later dispatch delivers, callback re-entry depth remains 1, and queue saturation returns `FF_NO_RESOURCE`, retains the EVS log record, increments dropped-notification count and never calls listener inline. Independent delta review/NC closure and release approval are not claimed. |
| SB overflow scoped EVS R1 follow-up, 2026-10-14 | PASS (`make ci-local`; native `cmake --build build/coverage -j2`) | PASS (27/27; `test_sb` 236 checks, `test_r2_gap` 581, `test_r3_gap` 201) | PASS (`lake build` in `make ci-local`) | PASS (116 requirements / 72 SRS / 30 SSS / 14 GAP; 51 Lean links in `make ci-local`) | PASS (`make smoke`, 6 apps; `make renode`, `RENODE_TEST: PASS`) | PASS (`clang-format --dry-run --Werror` on modified C files) | BLOCKED (`make rtems` cannot use existing `build/rtems` cache: it was configured from `/workspace`, source is this checkout; cache left untouched) | Regression proves ordinary `FF_EvsSend` is synchronous; only `FF_EvsSendDeferred` enqueues and is used by `FF_SbPublish`. SB event delivery is delayed; deferred queue is bounded, snapshot-limited, and recursive dispatcher entry returns zero. Integrated saturation verifies the SB error remains in the EVS log and dropped-notification stats increment when its callback cannot enqueue; `FF_SbPublish` does not propagate this secondary EVS status. Publisher-thread listener blocking is removed, but mutex calls still use `FF_PAL_WAIT_FOREVER`, so strict wait-free/nonblocking publish is unproven and remains open in NC-EXT-75. Intermediate failures retained: an initial `test_r3_gap` assertion expected deferred delivery and was updated for synchronous ordinary `FF_EvsSend`; a host `cmake --build build/ci -j2` attempt could not use the Docker-owned `/workspace` cache. Final checks used containerized `make ci-local` successfully. Independent delta review/NC closure and release approval are not claimed. |
| SB/EVS bounded lock failure, 2026-10-14 | PASS (`make ci-local`; fresh `cmake -S . -B build/agent-bounded -G Ninja -DFF_BUILD_TESTS=ON` + build) | PASS (27/27 in both CTest runs; `test_sb` 298 checks) | PASS (`lake build` in `make ci-local`) | PASS (116 requirements / 72 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`, six apps; `make renode`, `RENODE_TEST: PASS`) | PASS (`clang-format --dry-run --Werror`); `make misra-docker` produced 36 advisory findings (including `pal_posix.c` `ETIMEDOUT` config warnings and SB `FF_Msg_t` unknown-type warning) | NOT RUN | Forced-lock tests use explicit test hooks, not sleeps: SB lock busy returns exact `FF_TIMEOUT` with unchanged published/delivered/dropped counters, EVS log and notification stats, and no queued copy; direct deferred EVS lock contention returns `FF_TIMEOUT` with unchanged log/queued/dispatched/dropped statistics. An integrated overflow test verifies SB accounting can succeed while EVS lock contention loses the event, which remains an explicit SRS-REQ-011/NC-EXT-76 gap rather than an accepted forwarding outcome. Host `make docs` could not remove pre-existing root-owned output; containerized `make docs` passed with existing warnings. RTEMS cross-build and GSW not run. No SRS/SSS edits. Existing `build/native` generator mismatch was not modified; a fresh `build/agent-bounded` tree was used. Independent reviews and NC-EXT-75/76 remain open. |
| Event-envelope requirement delta, 2026-10-14 | PASS (`make ci-local`; configuration/build) | PASS (27/27 CTest) | PASS (`lake build` in `make ci-local`) | PASS (117 requirements / 73 SRS / 30 SSS / 14 GAP; 51 Lean links) | PASS (`make smoke`; `make renode`, `RENODE_TEST: PASS`; aggregate marker is not event-envelope coverage) | PASS (clang-format dry-run on changed C/H; GSW self-check Python suites 5/5 + 9/9 + 14/14; containerized `make docs` completed with existing warnings) | NOT RUN | R3 delta FRICK-REVIEW-0266 approves continued development with comments only, not baseline/N14 closure. SSS-REQ-006/SRS-REQ-011/-072 now define arrival/acceptance/dispatch/ground boundaries and loss-accounting obligations; all event profile values remain TBD and queue=64/batch=8/nominal 1000 ms are candidates only. TS-5-74 and TS-6-52 remain gap/TBD; local SB/EVS tests are not ground-receipt evidence. R4/PA review of this final requirements/status delta remains pending; N14, NC-EXT-75/76 and loss-counter implementation remain open. Direct host `ctest` logging attempt failed because Docker-created `build/ci/Testing/Temporary` is root-owned; the full `make ci-local` CTest run passed in-container. RTEMS cross-build and hosted CI were not run. No NC closure, qualification, or release approval is claimed. |

| v0.1.0 initial, 2026-10-10 | PASS | PASS (4/4) | PASS | PASS | PASS (6 apps) | clang-tidy clean | PASS (toolchain artifacts) | Baseline release; formal verification details below |
| v0.1.0 review closure, 2026-10-10 | PASS | PASS (4/4) | PASS (`lake build` 15/15 + 3/3 checks) | PASS (42 reqs, 20 theorems) | PASS (6 apps) | clean | proxy | Norm N15 re-run after review NCs (Lean proofs, S6 test, trace gate); see `docs/reviews/2026-10-10-*.md` |
| M2 time base, 2026-10-10 | PASS (Debug + Release) | PASS (5/5) | PASS (`lake build` 17/17 + 3/3 checks) | PASS (48 reqs, 41 theorems) | PASS (6 apps) | clean | proxy | Milestone M2: PPS tone discipline, time-source selection, leap-at-boundary, CUC P-field framing; see M2 notes below |
| M2 review closure, 2026-10-10 | PASS (Debug + Release) | PASS (5/5; 174 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101) | PASS (`lake build` 17/17, transcript `verification/lean/lake-build.log`; 3/3 checks) | PASS (48 reqs / 25 srs / 16 sss / 7 gap, 41 theorems, 9 CI claims) | PASS (6 apps) | clang-format + clang-tidy clean | proxy | All M2 NCs (R1/R2/R3) resolved and delta-re-reviewed; coverage lines 845/1446 (58.4%), functions 114/156; see review evidence below |
| M3 HAL interfaces + ECSS docs, 2026-10-10 | PASS (Debug) | PASS (6/6; 205 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101, hal 31) | PASS (`lake build` 19/19, transcript `verification/lean/lake-build.log`; 3/3 checks) | PASS (63 reqs / 33 srs / 17 sss / 13 gap, 45 theorems, 9 CI claims) | PASS (6 apps) | clang-format clean | proxy | HAL registry + stack-independent contracts (SSS-REQ-017, SRS-REQ-025..032); ECSS SDD/CM/PA/validation/safety/compliance-matrix docs; gaps GAP-A8..A13; coverage 893/1515 (58.9%), functions 124/166 |
| M6 libm integration, 2026-10-10 | PASS (Debug) | PASS (7/7; 225 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101, hal 31, math 20) | PASS (`lake build` 19/19) | PASS (65 reqs / 34 srs / 18 sss / 13 gap, 45 theorems, 9 CI claims) | PASS (6 apps) | clang-format clean | proxy | Mathematics facade over platform libm (SSS-REQ-018, SRS-REQ-033); ADR-0012, numerics policy, licence analysis; coverage 932/1575 (59.2%), functions 141/184 |
| M7 CCSDS SDLS, 2026-10-10 | PASS (Debug) | PASS (8/8; 306 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101, hal 31, math 20, sdls 81) | PASS (`lake build` 21/21) | PASS (69 reqs / 36 srs / 19 sss / 14 gap, 51 theorems, 9 CI claims) | PASS (6 apps) | clang-format + ASan clean | proxy | SDLS processing + SA/key management/rotation, pluggable crypto backend (SSS-REQ-019, SRS-REQ-034/035); GAP-A14; coverage 1252/2005 (62.4%), functions 175/217 |
| M4 drivers + RTEMS + TSN + netstack + lwIP, 2026-10-10 | PASS (Debug) | PASS (11/11; 374 checks default; lwIP adapter built/tested separately with FF_USE_LWIP, `test_lwip` 19 checks) | PASS (`lake build` 21/21) | PASS (78 reqs / 43 srs / 21 sss / 14 gap, 51 theorems, 10 CI claims) | PASS (6 apps) | clang-format clean | proxy | Cadence GEM driver (SRS-REQ-036), RTEMS integration (SRS-REQ-037), GEM TSN (SRS-REQ-038), loopback netstack (SRS-REQ-039), lwIP adapter (SRS-REQ-040); coverage 1656/2521 (65.7%), functions 220/264 |
| M10 Renode V&V env + N18, 2026-10-10 | PASS (Debug) | PASS (11/11; 374 checks; Renode test app: `FF_ENABLE_RENODE_TEST=1` builds 7 apps and prints `RENODE_TEST: PASS`; default build stays 6 apps) | PASS (`lake build` 21/21) | PASS (80 reqs / 45 srs / 21 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; `make renode-env` PASS (Renode runs + `polarfire-soc` platform loads) | proxy | Renode-on-Docker V&V environment + norm N18 + middleware test application (SRS-REQ-044, tool T17); functional gate is an advisory proxy until the RTEMS image is available (then blocking); review FRICK-REVIEW-0036 + delta |
| Open RTEMS baseline + toolchain, 2026-10-10 | PASS (Debug) | PASS (11/11) | PASS (`lake build` 21/21) | PASS (80 reqs / 45 srs / 21 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; `make renode-env` PASS | nightly (open RTEMS RSB, containerised T18) | Aligned to the **open-source RTEMS project** (no vendor/commercial baseline): `setup_rtems.sh` corrected (tools bset + `6/rtems-kernel.bset --with-rtems-bsp=riscv/mpfs64imafdc`) and vendor wording removed across docs; new `tools/docker/rtems/Dockerfile` (`make rtems-toolchain`/`rtems-docker`, tool T18); nightly builds the open RTEMS Renode image and runs `make renode` (N18) |
| Open RTEMS toolchain + Renode functional run, 2026-10-10 | PASS (Debug) | PASS (11/11) | PASS (`lake build` 21/21) | PASS (80 reqs / 45 srs / 21 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (open RTEMS 6.2 built from RSB tag 6.2; `make rtems-docker`) | **open-source RTEMS 6.2 toolchain built and validated** (T18); real cross build OK (plain + Renode image); **functional run of the RTEMS middleware on the Renode `polarfire-soc` platform: `make renode` -> `RENODE_TEST: PASS`** (boot U54_1 at entry 0x1000000000, console mmuart1). Real defects found and fixed by the real build: `pal_rtems.c` included an unused, non-existent RTEMS header (`rtems/posix/time.h`) and other unused includes; `rtems_config.c` lacked the `POSIX_Init` declaration before `<rtems/confdefs.h>`; toolchain file now passes the BSP include/lib paths and `-mcmodel=medany` (multilib). N18 gate is now **in force** (blocking when the image is present; per-commit SKIP proxy; nightly real). |
| MSS GPIO driver + Renode MMIO (M4/M11), 2026-10-10 | PASS (Debug) | PASS (12/12; `test_gpio` 27 checks, mock MMIO) | PASS (`lake build` 21/21) | PASS (85 reqs / 48 srs / 23 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio=PASS`) | Reference MSS GPIO driver (`FF_Gpio_t`, bus-injected MMIO, allocation-free) behind the HAL; host unit test with an independent mock MMIO register bank (per-pin CFG, IN=0x84, OUT=0x88); real MMIO round trip on the emulated MSS GPIO in the Renode run (milestone M11 first class). Requirement SRS-REQ-047. |
| MSS MMUART serial driver + Renode MMIO (M4/M11), 2026-10-10 | PASS (Debug) | PASS (13/13; `test_serial` 30 checks, mock MMIO) | PASS (`lake build` 21/21) | PASS (86 reqs / 49 srs / 23 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio=PASS`, `hal_serial=PASS`) | Reference MSS MMUART (NS16550, 4-byte-spaced registers) driver (`FF_Serial_t`, bus-injected MMIO, allocation-free) behind the HAL; host unit test with an independent mock; NS16550 scratch-register MMIO round trip on the emulated MSS MMUART0 (milestone M11). Requirement SRS-REQ-048. |
| Static RAM pool + DMA + Renode DDR (M4/M11), 2026-10-11 | PASS (Debug) | PASS (14/14; `test_mem` 20 checks) | PASS (`lake build` 21/21) | PASS (87 reqs / 50 srs / 23 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio`/`hal_serial`/`hal_mem=PASS`) | Bounded, heap-free `FF_MemPool_t` (bump allocator) + reference `FF_Dma_t` (identity translation; cache hooks are a seam = GAP-A5); host unit test; the pool is exercised over the in-image RAM (emulated DDR) in the Renode run (milestone M11). Requirement SRS-REQ-049. |
| PUS-C ASN.1 (N7/ESA) default backend (SRS-REQ-050), 2026-10-11 | PASS (Debug) | PASS (14/14) | PASS (`lake build` 21/21) | PASS (94 reqs / 54 srs / 26 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`) | `FF_PusBackendN7` is the **default** PUS backend (ADR-0016): ASN1SCC-generated `asn1-pusc-lib` types + N7/ESA ECSS Annex B.1 CRC; build-time generation (`make pus-asn1`, T19); host unit test `test_pus` asserts the N7 backend is byte-identical to the reference (36 checks); fallback to open backend with a warning when the generated code is absent. Deep per-service codecs = M15. |
| M11 ETH HAL class in Renode (SRS-REQ-036), 2026-10-11 | PASS (Debug) | PASS (14/14) | PASS (`lake build` 21/21) | PASS (94 reqs / 54 srs / 26 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_eth=PASS`) | The Renode functional test identifies the emulated Cadence GEM via its read-only Module ID register (`hal_eth=PASS`), in addition to GPIO/SERIAL/MEM; the full GEM DMA/link path remains host-tested (`test_gem`) + GAP-A5 (M11). |
| M11 NET HAL class in Renode (SRS-REQ-039), 2026-10-11 | PASS (Debug) | PASS (14/14) | PASS (`lake build` 21/21) | PASS (94 reqs / 54 srs / 26 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_net=PASS`) | The Renode functional test performs a socket round trip through the pluggable `FF_NetStack_t` contract using the reference loopback stack (`hal_net=PASS`), in addition to GPIO/SERIAL/MEM/ETH (M11). |
| R2/R3 test-gap task + TBL fix, 2026-10-11 | PASS (Debug) | PASS (16/16; `test_r2_gap` 487 checks, `test_r3_gap` 201 checks) | PASS (`lake build` 21/21) | PASS (96 reqs / 55 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; cppcheck MISRA 0 findings on the new suites | proxy | Two new host suites close previously unexecuted functions/branches and requirement-level flows (FRICK-REVIEW-0075/0076); **real defect fixed** in `src/core/tbl/tbl.c` (rejected table image was committed by activate - FRICK-REVIEW-0077 Major) and `FF_PusInit` made idempotent (registries reset); coverage 1917/2920 -> 2831/3961 lines (71.5%), functions 257/302 -> 371/376 (98.7%). |
| M4/M11 eNVM NVM backend, 2026-10-11 | PASS (Debug) | PASS (17/17; `test_nvm` 36 checks) | PASS (`lake build` 21/21) | PASS (97 reqs / 56 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_nvm=PASS`) | Reference `FF_Nvm_t` eNVM backend (SRS-REQ-055): bounded slot table (32 ids x 32 B), host test (mock MMIO) + Renode eNVM data write/read check; M11 5/13 peripheral-bound. |
| M4 QSPI NOR MTD backend, 2026-10-11 | PASS (Debug) | PASS (18/18; `test_mtd` 4123 checks) | PASS (`lake build` 21/21) | PASS (98 reqs / 57 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | proxy | Reference `FF_Mtd_t` NOR/QSPI backend (SRS-REQ-056): byte read/write, sector-aligned erase; host test (mock MMIO). On-SoC QSPI check pending (needs the board repl `MPFS_QSPI` + `Micron_MT25Q` and the flash command path) - M11 follow-up. |
| Renode board repl enabled (BeagleV-Fire), 2026-10-11 | PASS (Debug) | PASS (18/18) | PASS (`lake build` 21/21) | PASS (98/57/27/14, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm=PASS`) | V&V environment switched from the SoC-only platform to the `beaglev-fire` board repl (includes the SoC). New peripherals available for drivers: QSPI controller `SPI.MPFS_QSPI`@0x21000000 + `SPI.Micron_MT25Q` NOR flash, `MPFS_SystemServices`@0x37020000 (+ its QSPI/flash), board button. The QSPI NOR still needs the controller command path (M11 task). |
| M4/M11 MSS CAN backend, 2026-10-11 | PASS (Debug) | PASS (19/19; `test_can` 129 checks) | PASS (`lake build` 21/21) | PASS (99 reqs / 58 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_can=PASS`) | Reference `FF_CanBackend_t` MSS CAN backend (SRS-REQ-057): classic CAN 2.0 A/B, round-robin TX buffers (32), accept-all RX buffer with poll dispatch, reference bit timing; host test (mock MMIO, 129 checks) + Renode MSS CAN MMIO register round trips; M11 6/13 peripheral-bound. CAN-FD rejected (fabric device). Review NC R2/PA-09 fixed: the accept-all RX mask is left-aligned into the AMR identifier field (the host test discriminates the fix: 9 failures at the old constant). Limitations: no bus-off/error handling, polling-only RX. Reference bit timing from the MSS CAN clock (16 tq, 87.5% SP): the MSS CAN clock is one of 8..80 MHz from the MSS PLL (R1-D-01 corrected the earlier 125 MHz assumption); at the 80 MHz reference the standard rates 125/250/500 kbit/s and 1 Mbit/s are exact (prescaler 40/20/10/5). On-target confirmation of the configured clock = GAP-A5. |
| M10 QEMU emulated target, 2026-10-12 | PASS (Debug) | PASS (19/19) | PASS (`lake build` 21/21) | PASS (99 reqs / 58 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **QEMU PASS** (`make qemu`: `platform : qemu-virt-rtems`, banner + 6 apps registered) | M10 closed: the generic `riscv/rv64imafdc` RTEMS image (platform `qemu-virt-rtems`, `FF_PLATFORM=rtems-generic`, no MPFS board module) runs on `qemu-system-riscv64 -M virt` (`-bios`, entry `0x80000000`); tool T20 (`tools/scripts/qemu_vv.sh`) asserts the app count and records the toolchain image id; the nightly workflow runs it. Complements Renode (MPFS image, norm N18). |
| M4/M11 MSS I2C backend (S3), 2026-10-12 | PASS (Debug) | PASS (20/20; `test_i2c` 144 checks; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (100 reqs / 59 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; cppcheck (T14, advisory; reproducible via `make misra-docker`) 0 findings in the changed files | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_i2c=PASS`); QEMU cross-build PASS (T20) | Reference `FF_I2c_t` MSS I2C (CoreI2C) backend (SRS-REQ-058): polled master over a byte-injected MMIO bus; full START/address/data/repeated-START/receive/STOP sequence host-tested against a behavioural SI-clear-driven mock (write, read incl. cap=1, write-read, NACK/timeout/arg faults, bus-hook validation, divider-code selection for 100 kHz/400 kHz/1 MHz and the minimum-SCL boundary at the 100 MHz APB reference); self-test round-trips the ADDR/SLAVE1_ADDR registers on the emulated `I2C.MPFS_I2C` (`hal_i2c=PASS`, M11 7/13 peripheral-bound). Scope limits: emulated controller does not model CTRL CR0/CR1/CR2 (asserts ENS1 only, no divider readback) and advances on register writes (full transaction verified on host); polled master only (no slave/SMBus/clock-stretch/STOP-completion/interrupt path); SCL divider is a reference derivation from FF_I2C_MPFS_CLK_HZ=100 MHz (the minimum achievable SCL is PCLK/960 = 104166 Hz, so a 100 kHz request resolves to ≈104.17 kHz and the caller must request >= that minimum) — on-target APB/SCL timing = GAP-A5. |
| M4-restored scope + DDJF (planning), 2026-10-12 | PASS (Debug) | PASS (20/20; `test_i2c` 144 checks; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (112 reqs / 69 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; documentation/requirements-only increment (no C change); trace + metrics gates green | **REAL** (`make renode` -> `RENODE_TEST: PASS`; no target-code change, Sprint-3 image result stands) | N19 scope audit: restored the M4 scopes that existed only as prose "absorbed" notes as explicit, traceable deliverables — PUS-C over IP/TSN (SRS-REQ-059/060/061), generic fabric contract (SRS-REQ-067), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064), PUSopen(R) adapter (SRS-REQ-065), M8 GEM TSN 802.1Qav/Qci/Qbb (SRS-REQ-066) + gPTP/fabric CoreTSN (SRS-REQ-068); new SSS-REQ-028/029; new objectives OBJ-25..OBJ-30; sprint items S4b/S4c/S4d; DDJF acronym registered (FRICK-ENG-0007 0.3). Reviews FRICK-REVIEW-0137/0138/0139. |
| M4/M11 MSS SPI backend (S4), 2026-10-12 | PASS (Debug) | PASS (21/21; `test_spi` 126 checks; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (113 reqs / 70 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; cppcheck (T14, advisory; reproducible via `make misra-docker`) 0 findings in the changed files | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_spi=PASS`); QEMU cross-build PASS (T20) | Reference `FF_Spi_t` MSS SPI (CoreSPI) backend (SRS-REQ-069): polled master over a 32-bit injected MMIO bus; full TXDATA/RXDATARCVD/RXDATA transfer sequence host-tested against a behavioural 32-bit mock (transfer, cs_assert/cs_release, arg faults, status-timeout, clock-divider selection); self-test round-trips CONTROL/CLKRATE/FRAMESIZE/STATUS on the emulated `SPI.MPFS_SPI` (`hal_spi=PASS`, M11 8/13 peripheral-bound). Scope limits: the emulated model has no registered slave (register round-trips only; the transfer path is host-tested); polled master only (8-bit frames, no DMA/interrupt/enhanced-framing); the CLKRATE divider is a reference derivation from FF_SPI_MPFS_CLK_HZ=100 MHz — on-target APB/SCL timing = GAP-A5. |
| M4/M11 MSS block-device backend (S5), 2026-10-12 | PASS (Debug) | PASS (22/22; `test_block` 85 checks; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; cppcheck (T14, advisory; reproducible via `make misra-docker`) 0 findings in the changed files | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_block=PASS`); QEMU cross-build PASS (T20) | Reference `FF_BlockDev_t` MSS MMC/SD (SDHCI-style) backend (SRS-REQ-070): polled single/multi-block buffer-mode transfers (CMD17/18 read, CMD24/25 write) over a 32-bit injected MMIO bus; host-tested against a behavioural 32-bit mock (read/write, arg faults, status-timeout/error, block-size/count programming); self-test round-trips the block-size/argument/capability registers on the emulated `SD.MPFS_SDController` (`hal_block=PASS`, M11 9/13 peripheral-bound). Scope limits: the emulated board has no SD card (register round-trips only; the transfer path is host-tested); polled buffer mode only (no DMA/ADMA/CQ/SDIO/PHY); on-target SD clock/bus-width and card init = GAP-A5. |
| M11/S6a GEM TSN peripheral-bound, 2026-10-12 | PASS (Debug) | PASS (22/22; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; change limited to `app_renode_test.c` + docs | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_tsn=PASS`) | HAL TSN check on the emulated Cadence GEM (SRS-REQ-038, M11 10/13): the GEM native TSN backend (`FF_TsnGem_t`) is bound to the emulated MAC and exercised peripheral-bound (init, VLAN/PCP, 802.1AS TSU time); 802.1Qav returns `FF_NOT_IMPLEMENTED` (documented) and full gPTP/CoreTSN are M8. **NC-S6-01**: the NET lwIP-on-GEM peripheral-bound check is deferred to sprint S6b (it needs the lwIP RTEMS port built into the emulated image plus a Renode network path); the lwIP adapter remains host-tested. |
| M11/S6b-2 lwIP-on-GEM netif binding, 2026-10-12 | PASS (Debug) | PASS (22/22 default + 22/22 ASan/UBSan, both lwIP-off; **23/23** host with `FF_USE_LWIP=ON`) | PASS (`lake build` 21/21) | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; `-Wall -Wextra -Werror` clean on the authored files (host + RTEMS) | **REAL** (`make renode` -> `RENODE_TEST: PASS`: default image `hal_net=PASS` + lwIP image `hal_net_lwip=PASS`) | Sprint 6b-2: RTEMS runtime object sizing wired (`CONFIGURE_MAXIMUM_TASKS 24`/`SEMAPHORES 32`/`MESSAGE_QUEUES 24`, NC-S6-02 closed); new netif glue `src/hal/net/lwip/ff_lwip_netif.{c,h}` binds an lwIP `netif` to the emulated GEM (`FF_EthMac_t`) and the check asserts netif-up + GEM MODID + hwaddr on the **lwIP-enabled** Renode image; CI lwIP leg + `nm` link check (NC-151-02) and the `sys_thread_new` failure diagnostic (NC-151-05) closed. **Not delivered in S6b-2 (NC-S6-01 residual)**: no full TX/RX packet round-trip and the emulated GEM DMA was not armed because the driver's descriptor translation (`gem_phys`, then `uint32_t`) could not express an MPFS DDR address ≥ `0x1000000000` (latent 64-bit addressing defect, **NC-S6-03**, co-implemented in the NET follow-up; on-target GEM DMA/PHY = GAP-A5). |
| M11/S6b-1 lwIP RTEMS sys_arch port, 2026-10-12 | PASS (Debug) | PASS (22/22 default; **23/23** host with `FF_USE_LWIP=ON`; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; new port files `-Wall -Wextra` clean on the RTEMS target (third-party lwIP stays non-`-Werror`, unchanged policy) | **REAL** (`make renode` unaffected; `FF_USE_LWIP` OFF by default) | In-repo lwIP RTEMS `sys_arch` port (SRS-REQ-040, S6b-1): semaphores/mutexes/mailboxes/threads/time over the RTEMS classic API (`src/hal/net/lwip/rtems/`); the host build keeps the lwIP unix port; the RTEMS build produces the lwIP archive with `FF_USE_LWIP=ON` (85 objects incl. `sys_arch.c`) and a **forced-extraction link resolves** `tcpip_init`, `lwip_socket` and all `sys_*` symbols; the default image does **not** reference the adapter yet (S6b-2), so it is unchanged. **NC-S6-02**: the RTEMS runtime object sizing (`CONFIGURE_MAXIMUM_MESSAGE_QUEUES`) is still to be wired in S6b-2. |
| M11/S8 NET follow-up: GEM 64-bit + netif glue test, 2026-10-12 | PASS (Debug) | PASS (24/24 native; **26/26** host with `FF_USE_LWIP=ON`; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; `-Wall -Wextra -Werror` clean on authored files (host + RTEMS) | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_pcie=PASS`, `hal_spacewire=CONTRACT`) | **NC-S6-03 (co-implemented)**: `gem_phys` returns `uint64_t` (was `uint32_t`); new `FF_GEM_TBQP_HI`/`FF_GEM_RBQP_HI` registers (0x00C8/0x00CC) + `gem_wr_qbqp` helper write the 64-bit descriptor-ring base via low+high word; `DMACFG.ADDR64` (bit 30) asserted; per-descriptor addr is now explicitly truncated `(uint32_t)gem_phys(...)` with the full 3-word 64-bit descriptor format deferred to M14. **NC-154-01 (closed)**: new `test_lwip_netif` (3 tests, 19 checks) exercises the netif glue (`FF_LwipNetifAttach`/`IsUp`/`GetMac` + loopback TX/RX) against a mock `FF_EthMac_t`; `test_gem` updated to verify TBQP_HI/RBQP_HI + ADDR64 writes. **NC-S6-01 (documented)**: the Renode `Network.CadenceGEM` model is register-only (TSTART is an unhandled bit, no DMA descriptor engine — GAP-A5); a full packet round-trip cannot be performed in Renode; the TX/RX data path is host-tested with the mock-MAC loopback in `test_lwip_netif`. |
| S4b PUS-C over IP/TSN, 2026-10-13 | PASS | PASS (27/27; `test_link_ip` 19 test funcs / 58 checks, `test_link_tsn` 14 test funcs / 122 checks) | PASS (`lake build` 21/21 + 3/3 runtime checks) | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format + `-Wall -Wextra -Werror` clean (host) | SKIP (host-only code; no RTEMS image change — norm N18 proxy) | S4b delivered (SRS-REQ-059/060/061): seg/reassembly layer (`link_seg.{c,h}`, 2-byte BE length-prefix, allocation-free, bounded `rx_partial` buffer of `FF_LINK_SEG_PREFIX_LEN + FF_LINK_SEG_MAX_FRAME`), IP link backend (`link_ip.{c,h}`, UDP default + TCP) over `FF_NetStack_t` (host unit tests use the mock `FF_NetStack_t`; the reference `FF_NetStackLoopback` is a socket pair, so it models the peer across two sockets, not a single-socket self-loop), TSN link backend (`link_tsn.{c,h}`, GEM MAC RX callback + ring buffer, Ethertype 0x88F5) over `FF_EthMac_t + FF_TsnBackend_t`; `link.h` declares all four backends. Integration manual rev 0.8 (FRICK-REVIEW-0164 APPROVED WITH COMMENTS, NC-INT-MAN-01 closed, N17). On-target link-level = GAP-A5. R1/R2 review (FRICK-REVIEW-0165): **APPROVED** (2 Major + 10 Minor + 6 V&V gaps fixed in delta re-review; +3 new findings NF-1/NF-2/NF-3 raised + closed; see delta report). PA review (FRICK-REVIEW-0166): **APPROVED** (2 Major + 10 Minor + 6 V&V gaps + 4 PA Minor fixed; see delta report). Commit/push unblocked. |
| S4b delta re-review (NF-1/NF-2/NF-3 closure), 2026-10-13 | PASS | PASS (27/27; `test_link_ip` 19 test funcs incl. `test_seg_max_size_fragmented_partial`, `test_link_tsn` 14 test funcs) | PASS | PASS | PASS (6 apps) | clang-format clean; `-Wall -Wextra -Werror` clean on the authored files | SKIP (host-only code; no RTEMS image change — norm N18 proxy; the S4b baseline carried a REAL Renode PASS) | **NF-2 (Major):** OOB write in `link_seg.c` copy-back fixed — `rx_partial` buffer resized from `FF_LINK_SEG_MAX_FRAME` (1024) to `FF_LINK_SEG_PREFIX_LEN + FF_LINK_SEG_MAX_FRAME` (1026) in `link_seg.h:63`; new `test_seg_max_size_fragmented_partial` test exercises max-size frame + partial delivery. **NF-1 (Minor):** unused variable `need` removed from `link_seg.c:101` (`-Wall -Wextra -Werror` now clean). **NF-3 (Minor):** `verification-results.md` + review reports updated. R1/R2 delta re-review: APPROVED. |
| S4b follow-up (NC-S4b-02: PUS-C over IP/TSN host demonstration), 2026-10-13 | PASS (Debug) | PASS (27/27; `test_link_ip` 20 test funcs / 79 checks, `test_link_tsn` 15 test funcs / 135 checks) | Not run locally in that cycle; Lean source unchanged and existing verification retained as historical baseline | PASS (114 reqs / 71 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; strict `-Wall -Wextra -Werror` gate applied to test targets | SKIP (host-only code) | Host PUS-C demonstrations added (`test_pus_ip_roundtrip` uses a mock `FF_NetStack_t`; TSN test uses mock MAC). Corrections for NC-S4b-01..05 were applied; independent delta closure remained pending. On-target Ethernet/PTP link-level remains GAP-A5. |
| S8-FU assessment follow-up, 2026-10-13 | PASS (Docker-image `make ci-local`; native Debug; RTEMS cross-build) | PASS (27/27; TC/TM adversarial boundaries; ASan+UBSan 27/27) | PASS (`lake build` in observed `frick-vv:local`; host `make verify-lean` unavailable) | PASS (114 reqs / 71 SRS / 29 SSS / 14 GAP; 51 theorems linked) | PASS (native smoke, 6 applications) | PASS (clang-format; ASan+UBSan CTest 27/27) | PASS (`make rtems-docker`); REAL Renode PASS (`RENODE_TEST: PASS`) | Docker-image cycle completed after latest C/Python test changes; `make gsw-selfcheck` 5/5 plus CLI self-check PASS; `metrics.py --check` and trace PASS. The image inputs are not fully immutable (NC-EXT-35); configuration was the mutable working tree based on `57974d2`; target evidence under `build/renode/`; forge CI not run. FRICK-REVIEW-0173 re-verified bounded finite-buffer parser recovery; NC-EXT-13 opened for primary-header profile/validation policy. No overall NC closure claimed. |
| S8-FU follow-up record reconciliation, 2026-10-13 | PASS (`make ci-local`, observed Docker image; image inputs not fully immutable) | PASS (27/27) | PASS (`lake build`, observed Docker image) | PASS (116 reqs / 72 SRS / 30 SSS / 14 GAP; 51 Lean theorems) | PASS (`make smoke`, GSW Python tests 5/5, CLI self-check) | PASS (`clang-format --dry-run --Werror`; metrics current) | REAL Renode PASS (`RENODE_TEST: PASS`; selected executable-source digest) | Evidence was associated with HEAD `57974d297ab8a1e295258d1f3054bbbe2dd0f323` and SHA-256 of selected source diff `29ddcd1603e89cf9a6f92e0904341ac72c35bee2b3c7a871eb1e6dd6a9b10c03`; this is not a complete configuration/artifact identity (NC-EXT-29). `make ci-local` passed in the observed local image, but Docker base/package inputs are mutable (NC-EXT-35); forge CI unrun. FRICK-REVIEW-0174/0176/0178 R4 REJECTED; FRICK-REVIEW-0175/0177/0179 PA FAIL at that time. All NCs remain OPEN; verification semaphores remain yellow pending complete immutable configuration and output/log identity. |
| S8-FU T13/status follow-up, 2026-10-13 | PASS (`make ci-local`; observed mutable `frick-vv:local`) | PASS (27/27 CTest; Python 14/14) | PASS (`lake build` inside `make ci-local`; host `make check` stopped after CTest because host `lake` is unavailable) | PASS (116 reqs; 51 theorems; trace in `make ci-local`) | PASS (`make smoke`, 6 apps) | NOT RUN (static analysis/clang-format not separately rerun) | Not run this cycle | `make ci-local` configured/built/tested, Lean-built and traced successfully; 27/27 CTest. `make gsw-selfcheck` passed (5 GSW + 9 metrics tests; CLI self-check PASS). `make docs` failed on host cleanup of root-owned prior Doxygen output; rerun in `frick-vv:local` as UID 0 completed with existing Doxygen warnings. `python3 tools/scripts/metrics.py --readme` regenerated panel (31/54); `--check` passed. No target Renode/QEMU run or hosted CI. Tree is mutable/uncommitted; no complete source/tool/configuration/artifact/log identity; T13 not fully requalified, NC-EXT-41/42 and all other NCs remain open; semaphores yellow. |

## v0.1.0 baseline notes

### Formal verification (norm N1)

Lean 4 project under `verification/lean/` proves the requirements referenced by
the traceability manifest, including:

- CRC-16/CCITT-FALSE transmission check (frames extended by their CRC yield a
  zero CRC) - `Frick.Ccsds`.
- Software Bus delivery counting and queue-depth bound - `Frick.SoftwareBus`.
- PUS verification-count and TM counter monotonicity (mod 16384) -
  `Frick.Pus`, `Frick.Requirements`.
- CCSDS length arithmetic - `Frick.Requirements`.

verification/results: `make verify-lean` (lake build) is the gate; theorem-by-
theorem discharge is mirrored in the traceability manifest `lean:` links. In the
v0.1.0 review-closure cycle the project was built in the pinned Docker V&V
image: `lake build` compiled 15/15 modules and `lake exe frick_verify_checks`
reported 3/3 runtime checks PASS.

### Unit test results (methods Tst)

| Suite | Result | Coverage note |
| ----- | ------ | ------------- |
| test_ccsds | PASS | packet headers, length fields, CRC detect/append |
| test_sb    | PASS | exact and wildcard subscriptions, FIFO, depth |
| test_tbl   | PASS | load/validate/activate, atomic activation |
| test_pus   | PASS | dispatch, S1 verification packets, S17 ping, S6 region bound |

### System demonstration (method Dem)

Native simulator boots with 6 applications (scheduler, command ingest, housekeeping,
health & safety, sample telemetry, TM output); EVS events flow to the PUS
service 5 telemetry stream on the loopback link. Ground tool `ff_gsw.py` can
inject a ping and decode the returned service 17 telemetry.

### Traceability (norms N3/N4)

`make trace` exits 0: 21 SRS, 15 SSS, 6 GAP items, every id present in the
documents, every srs -> sss parent resolved, every Lean link resolved, every
`ci:*` evidence claim declared in a workflow.

### Open non-conformances (norm N12)

None open at the v0.1.0 baseline. Class-A gaps (GAP-A1..A6) are tracked in
`docs/quality/class-a-gaps.md` and are not NCs for the class-B baseline.

## M2 time base notes (2026-10-10)

### Formal verification (norm N1)

The M2 model `Frick.Time` adds tones and leap arithmetic on top of the
existing modules:

- Time-source enum is a proper, disjoint set; CUC seconds never regress across
  a tone boundary under a non-negative pending leap (SRS-REQ-021).
- Each valid tone advances the second counter by exactly one second and the
  tone counter strictly increases (SRS-REQ-022).
- The pending leap is applied and cleared exactly at the tone boundary
  (SRS-REQ-023).
- CUC field round trip is lossless, the fine fraction of any sub-second count
  fits 16 bits, and the coarse 4-octet decomposition recombines (SRS-REQ-024).

Gate: `lake build` compiled 17/17 modules and `lake exe frick_verify_checks`
reported 3/3 runtime checks PASS. The clean-rebuild transcript is committed at
`verification/lean/lake-build.log` so the module count is re-derivable from the
repository (R2 M2-05, norm N11). Lean 4 core v4.10.0 only (no Mathlib); the
proofs use `omega` and structural induction (`verification/lean/lake-manifest.json`
has no dependencies).

### Unit test results (methods Tst)

| Suite | Result | Coverage note |
| ----- | ------ | ------------- |
| test_ccsds | PASS (15 checks) | packet headers, length fields, CRC detect/append |
| test_sb    | PASS (20 checks) | exact and wildcard subscriptions, FIFO, depth |
| test_tbl   | PASS (9 checks) | load/validate/activate, atomic activation |
| test_pus   | PASS (29 checks) | dispatch, S1 verification packets, S17 ping, S6 region bound, S9 set-time 6-octet enforcement |
| test_time  | PASS (99 checks) | source selection, tone discipline (reject/jump/epoch), leap at boundary, no-tone pending, leap non-regression/skipped second, CUC encode/decode + fixed reference frame `{0x2E,0,0,0,5,0x40,0x00}` (two tautological `fine < 65536` range asserts removed: the fine fraction is a `uint16_t`, so the assert was always true and `-Wtype-limits` rejects it; its lossless recovery is covered by the CUC byte round-trip) |

### Code coverage (method Anl, gcov)

`make coverage` (gcov, all suites executing): lines **845/1446 (58.4%)**,
functions **114/156**. Recorded in `build/coverage/coverage.json` and the README
metrics panel. The line count rose from the previously recorded 579/1310
(44.2%) because the M2 `time` suite previously did not execute (R2 M2-01) and
now does.

### Traceability (norms N3/N4)

`make trace` exits 0: 25 SRS, 16 SSS, 7 GAP items (incl. new GAP-A7), every id
present in the documents, every srs -> sss parent resolved, every Lean link
resolved, every `ci:*` evidence claim declared in a workflow. New links:
SSS-REQ-016, SRS-REQ-021..024.

### Non-conformance closure (norm N12)

The M2 delta was reviewed independently by three profiles; all non-conformances
were resolved to compliance and the deltas re-reviewed (norm N12):

| Review | Profile | Findings | Disposition |
| ------ | ------- | -------- | ----------- |
| `2026-10-10-m2-time-base-r1-review.md` | R1 | 6 Minor + 2 Comments | Resolved (test determinism, TONE first-tone semantics + ARCH, tone-state mutex, PPS start/stop state, CUC macro `_Static_assert`s, tone-spacing dispositioned to GAP-A7, priority/stack note, extra tests) |
| `2026-10-10-m2-vv-r2-review.md` | R2 | 2 Major + 4 Minor | Resolved (dead `main` fixed, tone test made clock-independent, `trace.py` `ci_evidence` scoping, Gitea `ci-lean` marker, Lean transcript committed, coverage/metrics regenerated) |
| `2026-10-10-m2-sss-r3-review.md` | R3 | 1 Major + 4 Minor + 3 Comments | Resolved (CUC two-form wire truth in ICD/ARCH/SRS, source-switch semantics, tone-second-counter wording, no-tone leap clause, `ecss-criticality-b.md` GAP-A1..A7) |

Open non-conformances: **none**. On-target PPS capture and the TAI/CCSDS epoch
remain a documented class-A gap (GAP-A7, `docs/quality/class-a-gaps.md`), not an
NC for the class-B baseline.

## M3 hardware abstraction and ECSS documentation notes (2026-10-10)

### Delivered

The HAL registry and the stack-independent device/bus/memory contracts
(SSS-REQ-017, SRS-REQ-025..032), plus the ECSS engineering set: Software Design
Document, configuration management plan, product/quality assurance plan,
validation plan, preliminary safety/FDIR analysis and the ECSS compliance
matrix; gaps to class A extended to GAP-A8..A13.

### Formal verification (norm N1)

`Frick.Hal` proves the HAL registry bound and duplicate invariance
(`hal_registry_count_bound`, `hal_dup_registration_unchanged`, wrapped as
`req_srs025_registry_bound` / `req_srs025_dup_unchanged`). Gate: `lake build`
19/19, `frick_verify_checks` 3/3; transcript committed
(`verification/lean/lake-build.log`).

### Unit tests (methods Tst)

6/6 suites: ccsds 15, sb 20, tbl 9, pus 29, time 101, **hal 31** checks
(205 total).

### Inspection evidence (methods Inp)

SRS-REQ-026..032 are **interface declarations** (non-executable). Their
verification is inspection of the contract headers plus the contract-presence
test (`tests/unit/test_hal.c::test_contract_presence`, which compiles and
instantiates every contract type). This is the recorded `Inp` evidence for
these requirements (manifest `lean_note`).

### Coverage (method Anl, gcov)

`make coverage`: lines **893/1515 (58.9%)**, functions **124/166**.

### Traceability (norms N3/N4)

`make trace` exits 0: 63 requirements (33 SRS / 17 SSS / 13 GAP), 45 Lean
theorems, 9 CI evidence claims.

### Non-conformance closure

The M3 delta was reviewed by R1 (design/code), R2 (V&V), R3 (SSS, N14) and R4
(documentation, N17); every non-conformance was dispositioned and the deltas
re-reviewed until closure (norm N12). See the review evidence table below.

## M6 mathematics library notes (2026-10-10)

### Delivered

Portable mathematics facade over the platform `libm` (`src/math/ff_math.{c,h}`,
`FF_USE_LIBM`, `-lm`); requirements SSS-REQ-018 / SRS-REQ-033; ADR-0012 and
policy `docs/architecture/numerics.md`.

### Unit tests (Tst)

7/7 suites; `math` 20 checks (elementary functions within tolerance, helpers,
domain/finiteness). Total 225 checks.

### Inspection (Inp)

The facade is a thin, allocation-free wrapper; the framework core remains free
of floating point. A `lean_note` is recorded for SRS-REQ-033 (third-party libm,
not Lean-checkable).

### Coverage

`make coverage`: lines **932/1575 (59.2%)**, functions **141/184**.

## M7 SDLS security notes (2026-10-10)

### Delivered

The CCSDS SDLS sub-layer (`src/security/ff_sdls.{c,h}`): SDLS processing,
bounded SPI-indexed security-association database, and key lifecycle
(derive/rotate/zeroise) with a **pluggable** cryptographic backend
(`FF_CryptoBackend_t`). Requirements SSS-REQ-019, SRS-REQ-034/-035; ADR-0013;
`docs/architecture/security-sdls.md`.

### Formal verification (Anl, norm N1)

`Frick.Sdls`: `req_srs035_sa_bound`, `req_srs035_epoch_monotonic`,
`req_srs035_uses_reset`. Gate: `lake build` 21/21, 3/3 runtime checks.

### Unit tests (Tst)

8/8 suites; `sdls` **81 checks** (round trip, AUTH-only/ENC-only, tamper
rejection, SA lifecycle, capacity bound, length bounds, removal/reuse,
rotation/epoch with a failed-derivation-safe path, policy tick, zeroisation)
with the non-secure reference backend; **AddressSanitizer clean and wired into
CI** (build job).

### Coverage (Anl, gcov)

`make coverage`: lines **1252/2005 (62.4%)**, functions **175/217**.

### Non-conformance closure

Reviewed by R1/R2 (code/V&V), R3 (SSS, N14) and R4 (documentation, N17); NCs
dispositioned and deltas re-reviewed (norm N12). Cryptographic algorithm/HSM/
ground key management is class-A gap GAP-A14.

## M4 Ethernet driver notes (2026-10-10)

### Delivered (increment 1)

The Cadence GEM Ethernet driver (`src/drivers/eth/gem/`) implementing the
`FF_EthMac_t` HAL contract: static, allocation-free, with injected MMIO
(`FF_GemBus_t`) and HAL `FF_Dma_t` translation so the register/descriptor logic
is host-testable. Requirement SSS-REQ-020 / SRS-REQ-036; ADR-0014. Third-party
integration scaffolding (fetched, pinned, not committed): lwIP and N7/ESA
`asn1-pusc-lib` (`tools/scripts/fetch_third_party.sh`).

### Verification (Tst / Inp)

`test_gem` (23 checks) with a mock MMIO bus: init register programming, MAC
address registers, transmit descriptor + completion, RX dispatch, error
counting. Real PHY/link behaviour needs the board (GAP-A5); register map source:
PolarFire SoC MSS TRM DS60001702.

### Coverage

`make coverage`: lines **1656/2521 (65.7%)**, functions **220/264**. `test_netloop` (328 checks) exercises the loopback `FF_NetStack_t` (bind/listen/connect/accept, bidirectional send/recv, boundary/negative arguments, descriptor exhaustion, full-ring partial send, cross-socket duplicate bind, out-of-range handle rejection).

### Remaining (M4)

CAN-FD/CANopen/RS-485 DE backends and the generic fabric-device contract
(SRS-REQ-062/063/064/067; restored M4 scope), the PUSopen(R) adapter
(SRS-REQ-065), the on-SoC QSPI NOR write/erase check and the GEM DMA packet
round-trip (M14); N7/ESA `FF_Pus_Backend_t` per-service mapping (M15),
filestore/CFDP/service 23 (M15). Delivered since the original list: SPI/I2C,
MMC/eMMC block device, PCIe, SpaceWire (contract-level), lwIP adapter, GEM TSN,
MSS MMUART/GPIO, the eNVM NVM backend, the NOR/QSPI MTD backend, the MSS CAN
backend and the PUS-C over IP/TSN link backends (kept here only as history).

## Documentation review (norm N17)

Every commit is preceded by a documentation review loop by the independent
**Senior SW documentation reviewer** (profile R4). The documentation set
(README/AGENTS/`docs/**`/status) is reviewed for completeness and correctness
against the code, requirements, traceability and CI; findings are resolved and
re-reviewed (norm N12) before commit/push. Reports are stored in `docs/reviews/`.

## Review evidence (norm N9/N11)

| Review | Profile | Date | Conclusion | Report |
| ------ | ------- | ---- | ---------- | ------ |
| Baseline V&V result assessment | R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-baseline-vv-review.md |
| SSS requirements review (norm N14) | R3 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-sss-systems-review.md |
| Norms N1-N16 conflict analysis | R1+R3 | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-rules-conflict-review.md |
| M2 code review (time base) | R1 (independent) | 2026-10-10 | APPROVED WITH COMMENTS, NCs closed | docs/reviews/2026-10-10-m2-time-base-r1-review.md |
| M2 code review delta | R1 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m2-time-base-r1-delta-review.md |
| M2 V&V review | R2 (independent) | 2026-10-10 | REJECTED, NCs closed after fix | docs/reviews/2026-10-10-m2-vv-r2-review.md |
| M2 V&V review delta | R2 (independent) | 2026-10-10 | APPROVED | docs/reviews/2026-10-10-m2-vv-r2-delta-review.md |
| M2 SSS/SRS/CUC review (N14) | R3 (independent) | 2026-10-10 | REJECTED, NCs closed after fix | docs/reviews/2026-10-10-m2-sss-r3-review.md |
| M2 SSS/SRS review delta (N14) | R3 (independent) | 2026-10-10 | APPROVED | docs/reviews/2026-10-10-m2-sss-r3-delta-review.md |
| Documentation set review (N17) | R4 (independent) | 2026-10-10 | REJECTED, findings corrected (delta re-review in cycle) | docs/reviews/2026-10-10-doc-set-r4-review.md |
| Documentation set review delta (N17) | R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (findings corrected) | docs/reviews/2026-10-10-doc-set-r4-delta-review.md |
| Documentation set review final (N17) | R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (all NCs closed) | docs/reviews/2026-10-10-doc-set-r4-final-review.md |
| M3 HAL code/design review | R1 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m3-hal-r1-review.md |
| M3 V&V review | R2 (independent) | 2026-10-10 | REJECTED, NCs closed after fix | docs/reviews/2026-10-10-m3-vv-r2-review.md |
| M3 SSS/SRS review (N14) | R3 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m3-sss-r3-review.md |
| M3 documentation review (N17) | R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m3-doc-set-r4-review.md |
| M3 R1+R2 delta | R1+R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (both R2 Majors closed) | docs/reviews/2026-10-10-m3-r1-r2-delta-review.md |
| M3 R3+R4 delta | R3+R4 (independent) | 2026-10-10 | R3 APPROVED / R4 APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m3-r3-r4-delta-review.md |
| M6 mathematics review | R1+R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m6-math-r1-r2-review.md |
| M6 mathematics review | R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m6-math-r3-r4-review.md |
| M6 mathematics delta | R1+R2+R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m6-math-delta-review.md |
| M7 SDLS code/V&V review | R1+R2 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m7-sdls-r1-r2-review.md |
| M7 SDLS SSS/docs review | R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m7-sdls-r3-r4-review.md |
| M7 SDLS delta | R1+R2+R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m7-sdls-delta-review.md |
| M4 GEM driver review | R1+R2 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m4-gem-r1-r2-review.md |
| M4 GEM SSS/docs review | R3+R4 (independent) | 2026-10-10 | REJECTED (R3), APPROVED WITH COMMENTS (R4) | docs/reviews/2026-10-10-m4-gem-r3-r4-review.md |
| M4 GEM delta | R1+R2+R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m4-gem-delta-review.md |
| M4 RTEMS integration review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m4-rtems-r1-r2-r4-review.md |
| M4 RTEMS integration delta | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m4-rtems-delta-review.md |
| M4 GEM TSN backend review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m4-gem-tsn-r1-r2-r4-review.md |
| M4 GEM TSN backend delta | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m4-gem-tsn-delta-review.md |
| M4 netstack + M8 review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED -> CLOSED (independent deltas/N12 0057/0058/0059/0060/0061/0062) | docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md |
| M4 lwIP + M9 code quality review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m9-codequality-r1-r2-r4-review.md |
| M10 emulated-target review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md |
| Renode env + N18 review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED (F-01 Major) -> resolved + delta re-review | docs/reviews/2026-10-10-renode-env-r1-r2-r4-review.md |
| M4/M11 static RAM pool + DMA review | R1+R2+R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-11-m4-m11-mem-r1-r2-r4-review.md |
| PUS-C ASN.1 (N7/ESA) default backend review | R1+R2+R4 (independent) | 2026-10-11 | REJECTED -> closed (CRC guard + test) | docs/reviews/2026-10-11-pus-c-asn1-default-r1-r2-r4-review.md |
| README scope + integration/service docs (N17) | R4 (independent) | 2026-10-11 | REJECTED -> closed | docs/reviews/2026-10-11-readme-scope-r4-review.md |
| M11 ETH HAL class (Cadence GEM) review | R1+R2+R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS -> closed | docs/reviews/2026-10-11-m11-eth-r1-r2-r4-review.md |
| M11 NET contract check (loopback) — FSW | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m11-net-r1-review.md |
| M11 NET contract check (loopback) — V&V | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m11-net-r2-review.md |
| M11 NET contract check (loopback) — docs | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m11-net-r4-review.md |
| M11 NET delta | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS -> closed | docs/reviews/2026-10-11-m11-net-delta-review.md |
| PA review of all phase products (N20) | PA (independent) | 2026-10-11 | REJECTED (3 Major) | docs/reviews/2026-10-11-pa-review-c5f51bc.md |
| PA delta re-review of dispositions (N12/N20) | PA (independent) | 2026-10-11 | REJECTED -> dispositions in progress | docs/reviews/2026-10-11-pa-delta-review-077d38b.md |
| M4 netstack delta re-review (N12) — R1 | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (R1-01 closed; R1-02 tracking held) | docs/reviews/2026-10-11-m4-netstack-delta-r1-review.md |
| M4 netstack delta re-review (N12) — R2 | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m4-netstack-delta-r2-review.md |
| N20 governance documentation review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-n20-governance-r4-review.md |
| M4 lwIP + M9 code-quality review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m4-lwip-r1-r2-r4-review.md |
| M4 netstack delta closure (N12) — R1 | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (all R1 closed) | docs/reviews/2026-10-11-m4-netstack-delta-r1-closure.md |
| M4 netstack delta closure (N12) — R2 | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m4-netstack-delta-r2-closure.md |
| N20 governance documentation closure (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (DOC-01..12 closed) | docs/reviews/2026-10-11-n20-governance-r4-closure.md |
| M4 netstack final confirmation — R1 | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (D-CL closed) | docs/reviews/2026-10-11-m4-netstack-final-r1.md |
| M4 netstack final confirmation — R2 | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (no V&V barrier) | docs/reviews/2026-10-11-m4-netstack-final-r2.md |
| N20 governance final confirmation (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (DOC-13..17 closed) | docs/reviews/2026-10-11-n20-governance-final-r4.md |
| PA final review of all phase products (N20) | PA (independent) | 2026-10-11 | PASS | docs/reviews/2026-10-11-pa-final-review-967f563.md |
| M16 SSS/system requirements review (N14) | R3 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (S0) | docs/reviews/2026-10-11-m16-sss-r3-review.md |
| M16 documentation review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (S0) | docs/reviews/2026-10-11-m16-docs-r4-review.md |
| M16 PA review (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (S0) | docs/reviews/2026-10-11-m16-pa-review.md |
| ECSS traceability (SDD design matrices) | PA (independent) | 2026-10-11 | NC PA-67-01 (Minor) | docs/reviews/2026-10-11-ecss-traceability-pa-review.md |
| Completion-metrics panel — PA approval (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (F1-F5) | docs/reviews/2026-10-11-metrics-panel-pa-review.md |
| Completion-metrics panel — PA delta approval (N12/N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (F1-F5 closed) | docs/reviews/2026-10-11-metrics-panel-pa-delta-review.md |
| Completion-metrics tool T13 source review (N10) | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-metrics-tool-r2-review.md |
| Completion-metrics panel — PA final approval (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (owner approval satisfied) | docs/reviews/2026-10-11-metrics-panel-pa-approval.md |
| Test-coverage gap analysis + tests (V&V) | R2 (independent) | 2026-10-11 | test_r2_gap.c, 487 checks | docs/reviews/2026-10-11-test-coverage-r2-report.md |
| Requirement-level test-gap analysis + tests (systems) | R3 (independent) | 2026-10-11 | test_r3_gap.c, 198 checks | docs/reviews/2026-10-11-requirements-test-gap-r3-report.md |
| New test suites — R1 code review | R1 (independent) | 2026-10-11 | REJECTED (Major TBL bypass) -> fixed | docs/reviews/2026-10-11-test-gap-r1-review.md |
| Test-gap increment — PA review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-test-gap-pa-review.md |
| TBL fix — R1 delta re-review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (Major closed) | docs/reviews/2026-10-11-test-gap-r1-delta-review.md |
| Test-gap increment — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (all closed) | docs/reviews/2026-10-11-test-gap-pa-delta-review.md |
| NVM backend — R1 code review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r1-review.md |
| NVM backend — R2 V&V review | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r2-review.md |
| NVM backend — R4 docs review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r4-review.md |
| NVM backend — PA review (N20) | PA (independent) | 2026-10-11 | REJECTED (Major F1 process) -> fixed | docs/reviews/2026-10-11-nvm-pa-review.md |
| NVM backend — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (Major closed) | docs/reviews/2026-10-11-nvm-pa-delta-review.md |
| NVM backend — R1 delta review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r1-delta-review.md |
| NVM backend — R2 delta review | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r2-delta-review.md |
| NVM backend — R4 delta review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r4-delta-review.md |
| NVM backend — PA final review (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (N20 gate satisfied) | docs/reviews/2026-10-11-nvm-pa-final-review.md |
| NVM backend — PA PA-89-01 closure | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-pa-89-closure.md |
| NOR/QSPI MTD backend — R1 code review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r1-review.md |
| NOR/QSPI MTD backend — R2 V&V review | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r2-review.md |
| NOR/QSPI MTD backend — R4 docs review | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r4-review.md |
| NOR/QSPI MTD backend — PA review (N20) | PA (independent) | 2026-10-11 | REJECTED (Major F1 process) -> fixed | docs/reviews/2026-10-11-mtd-pa-review.md |
| MTD — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-pa-delta-review.md |
| MTD (QSPI) — R1 delta review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r1-delta-review.md |
| MTD — R4 delta review | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r4-delta-review.md |
| MTD — PA final review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-pa-final-review.md |
| MTD — PA closure (N12) | PA (independent) | 2026-10-11 | APPROVED (MTD loop closed) | docs/reviews/2026-10-11-mtd-pa-99-closure.md |
| Renode board repl — R4 docs review | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-board-r4-review.md |
| Renode board repl — PA review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-board-pa-review.md |
| Renode board repl — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-board-pa-delta-review.md |
| Renode V&V scripts — R2 T17/T15 re-qualification | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-resc-r2-requal-review.md |
| Renode board repl — PA D1/D2 closure (N12) | PA (independent) | 2026-10-11 | APPROVED (loop closed) | docs/reviews/2026-10-11-renode-board-pa-d1d2-closure.md |
| S4b integration manual R4 documentation review (N17) | R4 (independent) | 2026-10-13 | APPROVED WITH COMMENTS (DOC-R4-020/021/022; NC-INT-MAN-01 closed) | docs/reviews/2026-10-13-s4b-integration-manual-r4-review.md |
| S4b code/V&V review (R1+R2) | R1+R2 (independent) | 2026-10-13 | REJECTED (2 Major: R1-01 seg-layer TCP reassembly desync, R1-02 TSN ISR-unsafe blocking mutex; 10 Minor R1-03–12; 6 V&V gaps R2-01–06) | docs/reviews/2026-10-13-s4b-r1-r2-review.md |
| S4b product assurance review (N20) | PA (independent) | 2026-10-13 | REJECTED (2 Major R1-01/R1-02 open + 10 Minor + 6 V&V-gaps + 4 PA Minor: PA-MN-01/02/03/04) | docs/reviews/2026-10-13-s4b-pa-review.md |
| S4b follow-up code/V&V review (R1+R2) | R1+R2 (independent) | 2026-10-13 | APPROVED WITH COMMENTS (delta review pending) | docs/reviews/2026-10-13-s4b-followup-r1-r2-review.md |
| S4b follow-up documentation review (R4) | R4 (independent) | 2026-10-13 | APPROVED WITH COMMENTS (delta review pending) | docs/reviews/2026-10-13-s4b-followup-r4-review.md |
| S4b follow-up product assurance review (PA) | PA (independent) | 2026-10-13 | APPROVED WITH COMMENTS (delta review pending) | docs/reviews/2026-10-13-s4b-followup-pa-review.md |
| S8-FU engineering/V&V delta review (R1+R2) | R1+R2 (independent) | 2026-10-13 | PARTIAL PASS (NC-EXT-12 bounded case re-reviewed; NC-EXT-13 opened) | docs/reviews/2026-10-13-s8-fu-r1-r2-delta-review.md |
| S8-FU follow-up documentation review (R4) | R4 (independent) | 2026-10-13 | REJECTED (R4-01..05; current gate remains open) | docs/reviews/2026-10-13-s8-fu-followup-r4-review.md (FRICK-REVIEW-0174) |
| S8-FU follow-up PA review (N20) | PA (independent) | 2026-10-13 | FAIL (PA-01..05; current gate remains open) | docs/reviews/2026-10-13-s8-fu-followup-pa-review.md (FRICK-REVIEW-0175) |
| S8-FU final documentation delta review (R4) | R4 (independent) | 2026-10-13 | REJECTED (R4-D1..D5; current gate remains open) | docs/reviews/2026-10-13-s8-fu-final-r4-delta.md (FRICK-REVIEW-0176) |
| S8-FU final PA delta review (N20) | PA (independent) | 2026-10-13 | FAIL (PA-D1..D4; current gate remains open) | docs/reviews/2026-10-13-s8-fu-final-pa-delta.md (FRICK-REVIEW-0177) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | REJECTED (0178-01..05) | docs/reviews/2026-10-13-s8-fu-r4-delta-0178.md (FRICK-REVIEW-0178) |
| S8-FU PA delta review | PA (independent) | 2026-10-13 | FAIL (0179-01..05) | docs/reviews/2026-10-13-s8-fu-pa-delta-0179.md (FRICK-REVIEW-0179) |
| S8-FU R4 documentation/policy delta review | R4 (independent) | 2026-10-13 | REJECTED (0180-01..04) | docs/reviews/2026-10-13-s8-fu-r4-delta-0180.md (FRICK-REVIEW-0180) |
| S8-FU PA evidence/status delta review | PA (independent) | 2026-10-13 | FAIL (0181-01..04) | docs/reviews/2026-10-13-s8-fu-pa-delta-0181.md (FRICK-REVIEW-0181) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | REJECTED (0182-01..02) | docs/reviews/2026-10-13-s8-fu-r4-delta-0182.md (FRICK-REVIEW-0182) |
| S8-FU PA delta review | PA (independent) | 2026-10-13 | FAIL (0183-01..04) | docs/reviews/2026-10-13-s8-fu-pa-delta-0183.md (FRICK-REVIEW-0183) |
| S8-FU R4 targeted delta re-review | R4 (independent) | 2026-10-13 | APPROVED WITH COMMENTS (0182 correction scope only; no overall closure) | docs/reviews/2026-10-13-s8-fu-r4-delta-0184.md (FRICK-REVIEW-0184) |
| S8-FU PA delta review | PA (independent) | 2026-10-13 | FAIL (0185-01; NC-EXT-34 opened) | docs/reviews/2026-10-13-s8-fu-pa-delta-0185.md (FRICK-REVIEW-0185) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | REJECTED (0186-01/02) | docs/reviews/2026-10-13-s8-fu-r4-delta-0186.md (FRICK-REVIEW-0186) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | PASS (history/index/status checks only; no overall closure) | docs/reviews/2026-10-13-s8-fu-pa-delta-0187.md (FRICK-REVIEW-0187) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | PASS (0186 correction scope only; no NC closure) | docs/reviews/2026-10-13-s8-fu-r4-delta-0188.md (FRICK-REVIEW-0188) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | PASS WITH COMMENT (NC-EXT-34 records only; no overall closure) | docs/reviews/2026-10-13-s8-fu-pa-delta-0189.md (FRICK-REVIEW-0189) |
| T1 V&V image requalification delta review | R2 (independent) | 2026-10-13 | NOT YET REQUALIFIED (0190-01/02) | docs/reviews/2026-10-13-t1-r2-delta-0190.md (FRICK-REVIEW-0190) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0191-01/02) | docs/reviews/2026-10-13-s8-fu-r4-delta-0191.md (FRICK-REVIEW-0191) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | PASS WITH COMMENT (record/status scope only; no overall closure) | docs/reviews/2026-10-13-s8-fu-pa-delta-0192.md (FRICK-REVIEW-0192) |
| T1 V&V image requalification follow-up | R2 (independent) | 2026-10-13 | NOT FULLY REQUALIFIED (NC-EXT-35/36 remain open) | docs/reviews/2026-10-13-t1-r2-delta-0193.md (FRICK-REVIEW-0193) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0194-01/02) | docs/reviews/2026-10-13-s8-fu-r4-delta-0194.md (FRICK-REVIEW-0194) |
| T1 R2 delta review | R2 (independent) | 2026-10-13 | PASS (0194 correction scope only; no full T1 requalification) | docs/reviews/2026-10-13-t1-r2-delta-0195.md (FRICK-REVIEW-0195) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | APPROVED WITH COMMENTS (0194 correction scope only) | docs/reviews/2026-10-13-s8-fu-r4-delta-0196.md (FRICK-REVIEW-0196) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | PASS WITH COMMENT (record/status scope only; no overall closure) | docs/reviews/2026-10-13-s8-fu-pa-delta-0197.md (FRICK-REVIEW-0197) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0198-01/02) | docs/reviews/2026-10-13-s8-fu-r4-delta-0198.md (FRICK-REVIEW-0198) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NC-EXT-35 residual claim noted; no positive qualification conclusion | docs/reviews/2026-10-13-s8-fu-pa-delta-0199.md (FRICK-REVIEW-0199) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0200-01) | docs/reviews/2026-10-13-s8-fu-r4-delta-0200.md (FRICK-REVIEW-0200) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NC-EXT-35 correction gap at review time; no overall closure | docs/reviews/2026-10-13-s8-fu-pa-delta-0201.md (FRICK-REVIEW-0201) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0202-01/02) | docs/reviews/2026-10-13-s8-fu-r4-delta-0202.md (FRICK-REVIEW-0202) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED for correction scope (PA-01/02) | docs/reviews/2026-10-13-s8-fu-pa-delta-0203.md (FRICK-REVIEW-0203) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0204 findings) | docs/reviews/2026-10-13-s8-fu-r4-delta-0204.md (FRICK-REVIEW-0204) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED for correction scope (NC-EXT-38; V&V qualification wording) | docs/reviews/2026-10-13-s8-fu-pa-delta-0205.md (FRICK-REVIEW-0205) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (0206 findings) | docs/reviews/2026-10-13-s8-fu-r4-delta-0206.md (FRICK-REVIEW-0206) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-35/36/38/39/40) | docs/reviews/2026-10-13-s8-fu-pa-delta-0207.md (FRICK-REVIEW-0207) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-38; CONTRIBUTING/driver checklist scope mismatch) | docs/reviews/2026-10-13-s8-fu-r4-delta-0208.md (FRICK-REVIEW-0208) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-38; CONTRIBUTING/driver checklist/ADR scope mismatch) | docs/reviews/2026-10-13-s8-fu-pa-delta-0209.md (FRICK-REVIEW-0209) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (N16 CONTRIBUTING history gap) | docs/reviews/2026-10-13-s8-fu-r4-delta-0210.md (FRICK-REVIEW-0210) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-38; static analysis omitted from driver/service gate lists) | docs/reviews/2026-10-13-s8-fu-pa-delta-0211.md (FRICK-REVIEW-0211) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (active NC range omits 41) | docs/reviews/2026-10-13-s8-fu-r4-delta-0212.md (FRICK-REVIEW-0212) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-41 range; N16 guide headers) | docs/reviews/2026-10-13-s8-fu-pa-delta-0213.md (FRICK-REVIEW-0213) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-38 separate-target scope list incomplete) | docs/reviews/2026-10-13-s8-fu-r4-delta-0214.md (FRICK-REVIEW-0214) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-41 V&V ledger/history and README status wording) | docs/reviews/2026-10-13-s8-fu-pa-delta-0215.md (FRICK-REVIEW-0215) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-41 duplicate ledger rows) | docs/reviews/2026-10-13-s8-fu-r4-delta-0216.md (FRICK-REVIEW-0216) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-41 duplicate ledger rows) | docs/reviews/2026-10-13-s8-fu-pa-delta-0217.md (FRICK-REVIEW-0217) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (status-registry vs product-scope deferral wording) | docs/reviews/2026-10-13-s8-fu-r4-delta-0218.md (FRICK-REVIEW-0218) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | PASS for narrow ledger/history/status correction only; no overall N20 approval or NC closure | docs/reviews/2026-10-13-s8-fu-pa-delta-0219.md (FRICK-REVIEW-0219) |
| T13 R2 delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (classifier false positives; boundary tests insufficient) | docs/reviews/2026-10-13-t13-r2-delta-0220.md (FRICK-REVIEW-0220) |
| T13 R2 delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (stale panel; scope-only boundary; test gaps) | docs/reviews/2026-10-13-t13-r2-delta-0221.md (FRICK-REVIEW-0221) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (metrics currency, closure semantics, design-traceability wording, stale pointer) | docs/reviews/2026-10-13-s8-fu-r4-delta-0222.md (FRICK-REVIEW-0222) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (stale panel; pending/open verdict forms) | docs/reviews/2026-10-13-s8-fu-pa-delta-0223.md (FRICK-REVIEW-0223) |
| T13 R2 delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (contradictory negative clause; boundary tests) | docs/reviews/2026-10-13-t13-r2-delta-0224.md (FRICK-REVIEW-0224) |
| S8-FU R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (NC-EXT-42 omitted from plan range) | docs/reviews/2026-10-13-s8-fu-r4-delta-0225.md (FRICK-REVIEW-0225) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (heuristic metric limits; range/history) | docs/reviews/2026-10-13-s8-fu-pa-delta-0226.md (FRICK-REVIEW-0226) |
| T13 R2 delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (contradictory negative clauses still counted closed) | docs/reviews/2026-10-13-t13-r2-delta-0227.md (FRICK-REVIEW-0227) |
| T13 R2 delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T13 requalification (summary/test-count wording) | docs/reviews/2026-10-13-t13-r2-delta-0228.md (FRICK-REVIEW-0228) |
| T13 R2 scoped delta review | R2 (independent) | 2026-10-13 | No blocking finding in bounded correction; not qualification or NC closure | docs/reviews/2026-10-13-t13-r2-delta-0229.md (FRICK-REVIEW-0229) |
| S8-FU R4 scoped delta review | R4 (independent) | 2026-10-13 | APPROVED for scoped review only; no NC closure or overall N17 approval | docs/reviews/2026-10-13-s8-fu-r4-delta-0230.md (FRICK-REVIEW-0230) |
| S8-FU PA scoped delta review | PA (independent) | 2026-10-13 | Scoped corrections consistent; no T13 qualification, NC closure or phase approval | docs/reviews/2026-10-13-s8-fu-pa-delta-0231.md (FRICK-REVIEW-0231) |
| S8-FU R4 handover delta review | R4 (independent) | 2026-10-13 | PASS for handover-summary scope only; no NC closure or overall N17 approval | docs/reviews/2026-10-13-s8-fu-r4-delta-0232.md (FRICK-REVIEW-0232) |
| MPL transition PA audit | PA (independent) | 2026-10-13 | OPEN findings; inventory only; no license approval or NC closure | docs/reviews/2026-10-13-mpl-transition-pa-audit-0233.md (FRICK-REVIEW-0233) |
| MPL transition R4 inventory | R4 (independent) | 2026-10-13 | OPEN findings; inventory only; no transition approval or NC closure | docs/reviews/2026-10-13-mpl-transition-r4-inventory-0234.md (FRICK-REVIEW-0234) |
| T21 R2 delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T21 qualification (compound/malformed SPDX expressions pass) | docs/reviews/2026-10-13-mpl-t21-r2-delta-0235.md (FRICK-REVIEW-0235) |
| MPL transition R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (candidate wording, N13 traceability, ADR-0011 history) | docs/reviews/2026-10-13-mpl-transition-r4-delta-0236.md (FRICK-REVIEW-0236) |
| MPL transition PA delta review | PA (independent) | 2026-10-13 | OPEN / NOT APPROVED for release (authority, configuration identity, hosted CI/T21) | docs/reviews/2026-10-13-mpl-transition-pa-delta-0237.md (FRICK-REVIEW-0237) |
| T21 R2 header-position delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T21 qualification (prose and post-code SPDX declarations; mismatched wrappers) | docs/reviews/2026-10-13-mpl-t21-r2-header-delta-0238.md (FRICK-REVIEW-0238) |
| T21 R2 C-preprocessor delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T21 qualification (C preprocessor directive treated as comment/header) | docs/reviews/2026-10-13-mpl-t21-r2-preprocessor-delta-0239.md (FRICK-REVIEW-0239) |
| T21 R2 file-format delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T21 qualification (shebang accepted in non-script formats; suffix matrix coverage) | docs/reviews/2026-10-13-mpl-t21-r2-format-delta-0240.md (FRICK-REVIEW-0240) |
| T21 R2 C block-comment-state delta review | R2 (independent) | 2026-10-13 | NOT APPROVED for T21 qualification (starred declaration accepted without opening block comment) | docs/reviews/2026-10-13-mpl-t21-r2-block-state-delta-0241.md (FRICK-REVIEW-0241) |
| T21 R2 C block-comment-state re-review | R2 (independent) | 2026-10-13 | No new finding in bounded correction; not T21 qualification or NC closure | docs/reviews/2026-10-13-mpl-t21-r2-block-state-delta-0242.md (FRICK-REVIEW-0242) |
| MPL transition R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (explicit no-grant wording; workflow files not identical) | docs/reviews/2026-10-13-mpl-transition-r4-delta-0243.md (FRICK-REVIEW-0243) |
| MPL transition R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (SVG checker false negative; README release-label ambiguity; stale status JSON) | docs/reviews/2026-10-13-mpl-transition-r4-delta-0244.md (FRICK-REVIEW-0244) |
| T21 R2 SVG prolog delta re-review | R2 (independent) | 2026-10-13 | SVG correction confirmed for bounded scope; not T21 qualification or NC closure | docs/reviews/2026-10-13-mpl-t21-r2-svg-delta-0245.md (FRICK-REVIEW-0245) |
| DDJF T21 tool-register reference R4 delta review | R4 (independent) | 2026-10-13 | NOT APPROVED (DDJF says T1..T20 while register contains T1..T21) | docs/reviews/2026-10-13-ddjf-t21-register-r4-delta-0246.md (FRICK-REVIEW-0246) |
| DDJF T21 tool-register reference R4 re-review | R4 (independent) | 2026-10-13 | Bounded correction verified; no new finding; not overall N17/PA approval or NC closure | docs/reviews/2026-10-13-ddjf-t21-register-r4-delta-0247.md (FRICK-REVIEW-0247) |
| T21 PA unreadable-managed-file review | PA (independent) | 2026-10-14 | NOT APPROVED for T21 qualification (unreadable source-like files skipped) | docs/reviews/2026-10-14-mpl-t21-pa-unreadable-file-0248.md (FRICK-REVIEW-0248) |
| T21 fail-closed correction R2 delta review | R2 (independent) | 2026-10-14 | Bounded pass; no new finding; no T21 qualification/NC closure | docs/reviews/2026-10-14-t21-fail-closed-r2-delta-0249.md (FRICK-REVIEW-0249) |
| MPL authorization scope R4 delta review | R4 (independent) | 2026-10-14 | NOT APPROVED; current records inconsistently narrowed authorization scope and mishandled generated asset scope | docs/reviews/2026-10-14-mpl-scope-r4-delta-0250.md (FRICK-REVIEW-0250) |
| MPL authorization scope PA delta review | PA (independent) | 2026-10-14 | NOT APPROVED for scope/status consistency; NC-EXT-43/44/45 remain open | docs/reviews/2026-10-14-mpl-scope-pa-delta-0251.md (FRICK-REVIEW-0251) |
| T21/MPL active status R4 delta re-review | R4 (independent) | 2026-10-14 | NOT APPROVED; NC-EXT-43/45 scope/current-status wording findings | docs/reviews/2026-10-14-current-status-r4-delta-0252.md (FRICK-REVIEW-0252) |
| T21/MPL active status PA delta re-review | PA (independent) | 2026-10-14 | NOT APPROVED; README freshness and NC-EXT-45 PA gate wording; follow-up response relabeled as 0253 to avoid report-ID collision | docs/reviews/2026-10-14-current-status-pa-delta-0253.md (FRICK-REVIEW-0253) |
| T21/MPL current-status R4 follow-up | R4 (independent) | 2026-10-14 | NOT APPROVED; reported missing 0.3.62 history entry was fact-checked as present; stale T21 row status corrected | docs/reviews/2026-10-14-current-status-r4-followup-0254.md (FRICK-REVIEW-0254) |
| T21/MPL current-status PA follow-up | PA (independent) | 2026-10-14 | NOT APPROVED; handover gate wording and T21 row/history alignment | docs/reviews/2026-10-14-current-status-pa-followup-0255.md (FRICK-REVIEW-0255) |
| Handover current-review pointer R4 delta | R4 (independent) | 2026-10-14 | NOT APPROVED; current pointer ambiguity corrected | docs/reviews/2026-10-14-handover-current-review-r4-0256.md (FRICK-REVIEW-0256) |
| Current status PA follow-up | PA (independent) | 2026-10-14 | NOT APPROVED; stale status.json summary rewritten | docs/reviews/2026-10-14-current-status-pa-followup-0257.md (FRICK-REVIEW-0257) |
| Current-status R4 re-review | R4 (independent) | 2026-10-14 | Bounded no-new-finding in status/license-history scope; no NC closure | docs/reviews/2026-10-14-current-status-r4-rereview-0258.md (FRICK-REVIEW-0258) |
| Current-status PA re-review | PA (independent) | 2026-10-14 | Bounded no-new-finding in status assertions; no broader approval | docs/reviews/2026-10-14-current-status-pa-rereview-0259.md (FRICK-REVIEW-0259) |
| SB/EVS full CTest runner integration | R2 (independent) | 2026-10-14 | PASS WITH COMMENT for local characterization runner execution; not N10 formal qualification | docs/reviews/2026-10-14-sb-evs-ctest-r2-review-0275.md (FRICK-REVIEW-0275) |
| SB/EVS CTest records | R4 (independent) | 2026-10-14 | Initial NOT APPROVED; R4-01..03 closed by bounded delta re-review | docs/reviews/2026-10-14-sb-evs-ctest-r4-review-0276.md; docs/reviews/2026-10-14-sb-evs-ctest-r4-delta-0278.md (FRICK-REVIEW-0276/0278) |
| SB/EVS CTest evidence | PA (independent) | 2026-10-14 | APPROVED WITH COMMENTS; duration discrepancy closed by bounded delta; no N10/mission/release acceptance | docs/reviews/2026-10-14-sb-evs-ctest-pa-review-0277.md; docs/reviews/2026-10-14-sb-evs-ctest-pa-delta-0279.md (FRICK-REVIEW-0277/0279) |
## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | v0.1.0 initial V&V record |
| 0.1.1 | 2026-10-10 | frick-fsw | Review-closure cycle: Lean build fixed and verified, S6 test corrected, trace gate hardened |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2 cycle: time base, PPS tone discipline, CUC framing, leap-at-boundary |
| 0.3 | 2026-10-10 | frick-fsw | M2 review closure: NCs (R1/R2/R3) resolved, deterministic tests, coverage 845/1446, Lean transcript committed, norm N17 documentation review introduced |
| 0.4 | 2026-10-10 | frick-fsw | M3: HAL interfaces + ECSS design/assurance docs; 63 reqs / 45 theorems, coverage 893/1515; M3 reviews R1/R2/R3/R4 and deltas |
| 0.5 | 2026-10-10 | frick-fsw | M6: libm mathematics facade; 65 reqs / 34 srs / 18 sss, coverage 932/1575 (59.2%) |
| 0.6 | 2026-10-10 | frick-fsw | M7: CCSDS SDLS security + key management; 69 reqs / 36 srs / 19 sss / 14 gap, coverage 1252/2005 (62.4%) |
| 0.7 | 2026-10-10 | frick-fsw | M4 (increment 1): Cadence GEM Ethernet driver; 71 reqs / 37 srs / 20 sss, coverage 1440/2245 (64.1%) |
| 0.8 | 2026-10-10 | frick-fsw | M4 (increment 2): RTEMS application integration; 72 reqs / 38 srs / 20 sss |
| 0.9 | 2026-10-10 | frick-fsw | M4 (increment 3): GEM native TSN backend; 73 reqs / 39 srs / 20 sss |
| 0.10 | 2026-10-10 | frick-fsw | M4 (increment 4): reference loopback network stack; 74 reqs / 40 srs / 20 sss; milestone M8 added |
| 0.11 | 2026-10-10 | frick-fsw | M4 (increment 5): lwIP network-stack adapter; 75 reqs / 41 srs / 20 sss |
| 0.12 | 2026-10-10 | frick-fsw | M9: MISRA C:2012 process + declared coding style; 78 reqs / 43 srs / 21 sss |
| 0.13 | 2026-10-10 | frick-fsw | M10: emulated-target verification (Renode/QEMU); 79 reqs / 44 srs / 21 sss |
| 0.14 | 2026-10-10 | frick-fsw | N18/M10: Renode-on-Docker V&V environment + middleware test application (SRS-REQ-044, tool T17); 80 reqs / 45 srs / 21 sss; review FRICK-REVIEW-0036 + delta |
| 0.15 | 2026-10-10 | frick-fsw | Open-source RTEMS 6.2 alignment + reproducible toolchain image (tool T18); `setup_rtems.sh` corrected; nightly runs the open RTEMS Renode image (N18) |
| 0.16 | 2026-10-10 | frick-fsw | Real open RTEMS toolchain built + real cross build OK; Renode functional run of the RTEMS image PASS (`RENODE_TEST: PASS`); real-build defects fixed (pal_rtems headers/err, POSIX_Init decl, toolchain include/lib paths); N18 in force |
| 0.17 | 2026-10-10 | frick-fsw | M4/M11: reference MSS GPIO driver + host unit test (12/12) + Renode MMIO check (`hal_gpio=PASS`); SRS-REQ-047 |
| 0.18 | 2026-10-10 | frick-fsw | M4/M11: reference MSS MMUART serial driver + host unit test (13/13) + Renode MMIO check (`hal_serial=PASS`); SRS-REQ-048 |
| 0.19 | 2026-10-11 | frick-fsw | M4/M11: static RAM pool + DMA helper + host unit test (14/14) + Renode DDR check (`hal_mem=PASS`); SRS-REQ-049 |
| 0.20 | 2026-10-11 | frick-fsw | PUS-C ASN.1 (N7/ESA) default backend (SRS-REQ-050, ADR-0016, build-time generation T19); host 14/14 + Renode PASS |
| 0.21 | 2026-10-11 | frick-fsw | M11: ETH HAL class exercised in Renode (`hal_eth=PASS`, SRS-REQ-036) |
| 0.22 | 2026-10-11 | frick-fsw | M11: NET HAL class (pluggable TCP/IP, loopback stack) exercised in Renode (`hal_net=PASS`, SRS-REQ-039) |
| 0.23 | 2026-10-11 | frick-fsw | Record all 2026-10-11 reviews (MEM, PUS-C ASN.1, README, ETH, NET R1/R2/R4+delta, PA N20 + delta); header date refreshed (PA-10) |
| 0.24 | 2026-10-11 | frick-fsw | Record the N12 delta re-reviews (netstack R1/R2) + the N17 N20-governance review; m4-lwip row; test_netloop 328 checks |
| 0.25 | 2026-10-11 | frick-fsw | R2/R3 test-gap task: new suites `test_r2_gap` (487 checks) + `test_r3_gap` (201 checks); ctest 14 -> 16; trace manifest tests evidence extended; TBL defect fix + `FF_PusInit` idempotence; SDD section 6 requirements->design matrix added |
| 0.26 | 2026-10-11 | frick-fsw | M4/M11: reference eNVM NVM backend (SRS-REQ-055) - `ff_nvm_mpfs`, `test_nvm` (ctest 17/17), `hal_nvm=PASS` in Renode; M11 5/13 |
| 0.27 | 2026-10-11 | frick-fsw | M4: reference NOR/QSPI MTD backend (SRS-REQ-056) - `ff_mtd_mpfs`, `test_mtd` (ctest 18/18); on-SoC QSPI check pending board repl (M11) |
| 0.28 | 2026-10-11 | frick-fsw | Renode V&V environment switched to the BeagleV-Fire board repl (QSPI NOR + system services available); all HAL checks still PASS |
| 0.29 | 2026-10-11 | frick-fsw | M4/M11: reference MSS CAN backend (SRS-REQ-057) - `ff_can_mpfs`, `test_can` (ctest 19/19, 60 checks), `hal_can=PASS` in Renode; M11 6/13 |
| 0.30 | 2026-10-11 | frick-fsw | CAN review NCs: RX accept-all mask fixed (identifier domain, R2-01/PA-09), `test_can` 60 -> 129 checks (regression discriminates the fix; boundary/negative cases; TX wraparound; raw AMR write assert), poll ack re-read, reference clock 125 -> 80 MHz (R1-D-01, MSS PLL range); M11 6/13 |
| 0.31 | 2026-10-11 | frick-fsw | M10 closed: QEMU generic-BSP (`riscv/rv64imafdc`) run (`make qemu`, T20, `qemu-system-riscv64 -M virt`); toolchain image gains the rv64imafdc BSP + QEMU |
| 0.32 | 2026-10-12 | frick-fsw | M10 review NCs (FRICK-REVIEW-0120..0123): generic `qemu-virt-rtems` platform (no MPFS board module), T20 asserts the app count + records the image id, nightly QEMU job |
| 0.33 | 2026-10-12 | frick-fsw | M4/M11: reference MSS I2C backend (SRS-REQ-058, Sprint 3) - `ff_i2c_mpfs.{c,h}`, `test_i2c` (ctest 20/20, 135 checks), `hal_i2c=PASS` in Renode (M11 7/13); divider reference / GAP-A5 noted |
| 0.34 | 2026-10-12 | frick-fsw | I2C review NCs (FRICK-REVIEW-0129..0136): `test_i2c` 135 -> 144 checks (1 MHz code-5 mapping, minimum-SCL boundary, bus-hook validation); mock wording qualified (behavioural, not an independent oracle); minimum achievable SCL documented (R1-01/R3-04); STOP-completion/bus-recovery residual tracked; T14 MISRA reproducible via `make misra-docker` |
| 0.35 | 2026-10-12 | frick-fsw | Restored explicit M4 scope + DDJF (planning increment): PUS-C over IP/TSN, fabric contract, CAN-FD/CANopen/RS-485 DE, PUSopen adapter, M8 GEM TSN Qav/Qci/Qbb + gPTP/CoreTSN; counts 112/69/29/14/51; reviews FRICK-REVIEW-0137/0138/0139 |
| 0.36 | 2026-10-12 | frick-fsw | M4/M11: reference MSS SPI backend (SRS-REQ-069, Sprint 4) - `ff_spi_mpfs.{c,h}`, `test_spi` (ctest 21/21, 126 checks), `hal_spi=PASS` in Renode (M11 8/13); clock-divider reference / GAP-A5 noted |
| 0.37 | 2026-10-12 | frick-fsw | M4/M11: reference MSS block-device backend (SRS-REQ-070, Sprint 5) - `ff_block_mpfs.{c,h}`, `test_block` (ctest 22/22, 85 checks), `hal_block=PASS` in Renode (M11 9/13); buffer-mode/GAP-A5 noted |
| 0.38 | 2026-10-12 | frick-fsw | M11/S6a: GEM native TSN backend exercised peripheral-bound (`hal_tsn=PASS`, M11 10/13; SRS-REQ-038); NC-S6-01 opened (NET lwIP-on-GEM peripheral-bound deferred to sprint S6b) |
| 0.39 | 2026-10-12 | frick-fsw | M11/S6b-1: in-repo lwIP RTEMS `sys_arch` port (SRS-REQ-040) - host 23/23 with `FF_USE_LWIP=ON`, RTEMS build produces the lwIP archive (forced-extraction link resolves the stack; the image does not reference the adapter yet); NC-S6-02 opened (RTEMS object sizing, S6b-2) |
| 0.40 | 2026-10-12 | frick-fsw | M11/S7: PCIe peripheral-bound (`hal_pcie=PASS`, SRS-REQ-031/032) and SpaceWire contract-level (`hal_spacewire=CONTRACT`, SRS-REQ-032); M11 11/13 peripheral-bound; NC-S7-01 recorded. |
| 0.41 | 2026-10-12 | frick-fsw | M11/S8 NET follow-up: GEM 64-bit co-implementation (NC-S6-03: `gem_phys` returns `uint64_t`, `gem_wr_qbqp` + `TBQP_HI`/`RBQP_HI` + `DMACFG.ADDR64` asserted; per-descriptor truncation explicit; full 3-word descriptor format deferred to M14); host netif glue test (NC-154-01 closed: `test_lwip_netif` 3 tests/19 checks, 26/26 host with `FF_USE_LWIP=ON`); `test_gem` extended for 64-bit register writes; NET follow-up cycle row added; NC-S6-01 documented (Renode CadenceGEM register-only = GAP-A5; mock-MAC loopback host test). |
| 0.42 | 2026-10-12 | frick-fsw | Confirmation review (FRICK-REVIEW-0162) findings corrected: test-count corrected to "3 tests, 19 checks" across all docs; S6b-2 cycle row updated to note NC-S6-03 was co-implemented in the NET follow-up (gem_phys was then uint32_t at S6b-2 time); traceability.yaml SRS-REQ-040 `tests` field now lists `test_lwip_netif.c`; traceability.md History 0.36 + mirror row updated; renode-environment.md header aligned to 0.21; tool-qualification.md header aligned to 0.3.24. |
| 0.43 | 2026-10-13 | frick-fsw | **S4b delivered** (SRS-REQ-059/060/061): seg/reassembly, IP link backend, TSN link backend; 27/27 ctest + trace + Lean + smoke + GSW all PASS; Renode SKIP; integration manual rev 0.8 + R4 review (FRICK-REVIEW-0164, APPROVED WITH COMMENTS, NC-INT-MAN-01 closed); traceability + SRS updated. |
| 0.44 | 2026-10-13 | frick-fsw | S4b follow-up authoring evidence: host round-trip tests and documentation corrections were added; the requirement method evidence is bounded to host mocks, and NC-S4b-01..05 closure remained pending independent delta review. |
| 0.45 | 2026-10-13 | frick-fsw | Follow-up review findings received APPROVED WITH COMMENTS (FRICK-REVIEW-0167/0168/0169); corrections were applied but N12/N17/N20 closure remained pending delta re-review. |
| 0.46 | 2026-10-13 | frick-fsw | S4b second-pass documentation changes were applied; NC-S4b-06..08 are OPEN pending the required independent delta review. The earlier verification results and point-in-time review counts remain historical evidence; no current-loop review closure was established by this record. |
| 0.47 | 2026-10-13 | frick-fsw | S8-FU follow-up: current-tree test and target evidence recorded; no review NC closure claimed; pinned-container CI/Lean, native/sanitizer, GSW, trace, smoke, format, RTEMS Docker and Renode evidence also recorded on the mutable tree. |
| 0.48 | 2026-10-13 | frick-fsw | FRICK-REVIEW-0173 added: bounded GSW finite-buffer recovery independently re-verified; NC-EXT-13 opened for CCSDS primary-header policy; current verification and NC status synchronized. |
| 0.49 | 2026-10-13 | frick-fsw | Record current pinned-container build/test/Lean/trace, smoke, GSW, format, metrics and Renode evidence with executable-source diff digest; capture R4 REJECTED/PA FAIL and NC-EXT-28..31 |
| 0.50 | 2026-10-13 | frick-fsw | Add FRICK-REVIEW-0176/0177 verdicts to the V&V log; retain all current review and NC states as open |
| 0.51 | 2026-10-13 | frick-fsw | Add FRICK-REVIEW-0178/0179 verdicts; retain all NCs and verification semaphores open pending complete configuration identity |
| 0.52 | 2026-10-13 | frick-fsw | Update current-cycle review verdicts and evidence semaphore disposition; add NC-EXT-32 for publication wording and keep all gates open |
| 0.53 | 2026-10-13 | frick-fsw | Align N11 header with working-tree retention and gated publication policy (NC-EXT-32) |
| 0.54 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0180..0183 in the review-results ledger; all findings and gates remain open |
| 0.55 | 2026-10-13 | frick-fsw | Consolidate duplicate History tables, add NC-EXT-34, and record FRICK-REVIEW-0184/0185; keep gates open |
| 0.56 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0186/0187 scoped results; NC-EXT-34 remains open pending R4 review |
| 0.57 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0188/0189 scoped results and retain NC-EXT-34 open pending disposition |
| 0.58 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0190 T1 findings; incorporate clean/serialized docs and mandatory CI fixes pending requalification |
| 0.59 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0191..0193 and retain current review/T1 gates open |
| 0.60 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0194; retain NC-EXT-36 and T1 requalification gates open |
| 0.61 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0195..0197 scoped results; hosted/T1 and overall assurance gates remain open |
| 0.62 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0198/0199 residual NC-EXT-35 wording findings; T1 remains open |
| 0.63 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0200/0201 and correct active T1 reproducibility claims pending re-review |
| 0.64 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0202/0203 and track ci-local scope mismatch as NC-EXT-38 |
| 0.65 | 2026-10-13 | frick-fsw | Reconcile review/verification records after 0202/0203; retain NC-EXT-35/36/38 open pending independent review |
| 0.66 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0204/0205 and retain NC-EXT-35/38/39/40 open pending re-review |
| 0.67 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0206/0207; T1/T18 and local gate qualification claims remain open pending fresh review |
| 0.68 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0208/0209 scope findings; retain NC-EXT-38 open |
| 0.69 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0210/0211; add open history/scope corrections pending review |
| 0.70 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0212/0213; retain NC-EXT-41 and revision metadata finding open |
| 0.71 | 2026-10-13 | frick-fsw | Reconcile review/status pointers and record sprint-plan rev 0.60; no NC closed |
| 0.72 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0214/0215; retain NC-EXT-38/41 open pending correction and re-review |
| 0.73 | 2026-10-13 | frick-fsw | Remove duplicated FRICK-REVIEW-0212/0213 rows; NC-EXT-41 remains open pending review |
| 0.74 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0216/0217; duplicate-row correction awaits fresh verification |
| 0.75 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0218/0219 and update metrics-panel semantics; NC-EXT-41 remains open |
| 0.76 | 2026-10-13 | frick-fsw | Record R2 FRICK-REVIEW-0220 finding and subsequent T13 classifier tests; panel regenerated and currency check passed |
| 0.77 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0221..0223 and subsequent 9-test classifier/panel correction; requalification pending |
| 0.78 | 2026-10-13 | frick-fsw | Record NC-EXT-42 design-metric wording correction; current T13 review/qualification remains open |
| 0.79 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0224..0226 and preserve their NOT APPROVED status |
| 0.80 | 2026-10-13 | frick-fsw | Reconcile current T13 follow-up and NC-EXT-42 status after recorded reports |
| 0.81 | 2026-10-13 | frick-fsw | Record post-0224..0226 corrections to range, handover, and review-count heuristic labeling |
| 0.82 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0228 and retain current R2 disposition |
| 0.83 | 2026-10-13 | frick-fsw | Record scoped FRICK-REVIEW-0229..0231; no NC or T13 qualification closure |
| 0.84 | 2026-10-13 | frick-fsw | Reconcile T13 qualification history with bounded/scoped follow-up review outcomes |
| 0.85 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0232 scoped handover pass; no NC closure or overall approval |
| 0.86 | 2026-10-13 | frick-fsw | Record current Docker CI-local, host-check limitation, smoke/GSW, metrics and containerized docs outcomes; no NC closure |
| 0.87 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0238 rejection and current T21 corrective actions; latest parser change awaits independent re-review |
| 0.88 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0239 C-preprocessor rejection and NC-EXT-44 correction; T21 remains unqualified pending independent review |
| 0.89 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0240 format/shebang rejection and NC-EXT-44 correction; T21 remains unqualified pending independent review |
| 0.90 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0241 C block-state rejection and NC-EXT-44 correction; T21 remains unqualified pending independent review |
| 0.91 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0242 bounded no-new-finding result; T21 remains unqualified and NC-EXT-44 open |
| 0.92 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0243 R4 findings; keep NC-EXT-43 open and document author corrections for delta re-review |
| 0.93 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0244 SVG/header and active-status findings; author corrections and independent re-review pending |
| 0.94 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0245 bounded SVG correction result; T21/NC-EXT-44 remain open |
| 0.95 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0246 DDJF tool-range finding and author correction; NC-EXT-45 remains open pending R4/PA review |
| 0.96 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0247 bounded DDJF correction result; NC-EXT-45 remains open pending PA review |
| 0.97 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0248 unreadable-file finding and author correction; independent review remains pending |
| 0.98 | 2026-10-14 | frick-fsw | Record R2 0249 bounded pass and R4/PA 0250/0251 scope findings; record author reconciliation and pending re-review |
| 0.99 | 2026-10-14 | frick-fsw | Record R4/PA status follow-up findings 0252/0253 and author corrections; re-review remains pending |
| 1.0 | 2026-10-14 | frick-fsw | Reconcile active verification/review status after 0252/0253; record new correction state pending independent review |
| 1.1 | 2026-10-14 | frick-fsw | Record current-status follow-up reviews 0254/0255, findings and corrections pending re-review |
| 1.2 | 2026-10-14 | frick-fsw | Record author dispositions for 0254/0255 and active-status corrections; independent re-review pending |
| 1.3 | 2026-10-14 | frick-fsw | Record 0256/0257 and current status-note corrections; independent re-review pending |
| 1.4 | 2026-10-14 | frick-fsw | Record bounded no-new-finding status re-reviews 0258/0259; preserve broader open gates |
| 1.5 | 2026-10-14 | frick-fsw | Record SB overflow correction verification, passing host/Renode gates, blocked Lean gate and Doxygen permission failure; no NC closure claimed |
| 1.6 | 2026-10-14 | frick-fsw | Record R1 follow-up EVS non-recursive deferred-dispatch tests and exact current-cycle results/limitations; NC-EXT-75 and independent review gates remain open |
| 1.7 | 2026-10-14 | frick-fsw | Restore synchronous ordinary `FF_EvsSend`, document scoped `FF_EvsSendDeferred`, and record exact passing build/unit/Lean/trace/smoke/Renode/format evidence plus unresolved strict mutex nonblocking limit |
| 1.8 | 2026-10-14 | frick-fsw | Record bounded SB/EVS lock-contention implementation and exact local/CI/trace/smoke/Renode evidence; document Doxygen permission failure and open NC-EXT-75/76; no SRS/SSS change or NC closure |
| 1.9 | 2026-10-14 | frick-fsw | Record final event-envelope requirement cycle at 117 requirements and FRICK-REVIEW-0265/0266 scope; retain N14/N17/N20, NC-EXT-75/76, target and hosted-evidence gaps |
| 2.0 | 2026-10-14 | frick-fsw | Record final host/container verification for queue-mutex contention partial fan-out, exact commands and limitations; independent reviews, NC-EXT-75/76 and SRS-REQ-072 profile/loss reporting remain open |
| 2.1 | 2026-10-14 | frick-fsw | Record configured structural SB/EVS stress boundary, initial test-harness resource failure and isolated-process correction; retain mission-envelope, target-timing, independent-review and NC-EXT-75/76 gates |
| 2.2 | 2026-10-14 | frick-fsw | Reconcile stress-cycle evidence with a fresh clean build, selected artifact identities, scoped R4/PA results, test-case summaries and open commit/release gates |
| 2.5 | 2026-10-14 | frick-fsw | Record host Doxygen permission failure and successful containerized `make docs` recovery; other scope and assurance gates remain open |
| 2.6 | 2026-10-14 | frick-fsw | Record full SB/EVS parameter runner as a passing local CTest/CI-local case (30/30); hosted CI and independent integration delta review remain pending |
| 2.7 | 2026-10-14 | frick-fsw | Record R2/R4/PA scoped T22 CTest reviews and closures (FRICK-REVIEW-0275..0279); correct runtime to 18.70 s and retain hosted/mission gates open |
| 2.8 | 2026-10-15 | frick-fsw | Record the CI docs-job fix (Doxygen 1.9.8 single-level mkdir; `mkdir -p build/doxygen` in the `docs` target) with pristine-tree replication evidence; hosted CI, T21, NC-EXT-43 and release gates remain open |
| 2.9 | 2026-10-15 | frick-fsw | Add the `make docs` output-existence guard (`test -s build/doxygen/html/index.html`) raised as PA FRICK-REVIEW-0284 Comment 5, plus the workflow `docs`-job assertion and `doxygen --version` echo, with pristine/negative validation evidence; scoped R4 FRICK-REVIEW-0285 / PA FRICK-REVIEW-0286 APPROVED WITH COMMENTS and R4 NC-01/NC-02 closed by FRICK-REVIEW-0287; release, qualification, hosted and NC gates remain open |
| 2.10 | 2026-10-15 | frick-fsw | Record the committed guard delta (`77cd548`) and its green hosted GitHub Actions run #84 (all 8 jobs) as hosted evidence; Gitea run log unevidenced (NC-EXT-36); release, qualification and NC gates remain open |
| 2.11 | 2026-10-15 | frick-fsw | Record hosted run #85 for the record-only commit `03366f1` (all 8 jobs) and the loop-audit N15 record-only convention (record-only/evidence-only commits are verified by their own hosted run but not recursively re-recorded, terminating the record → run → record regress); Gitea run log unevidenced (NC-EXT-36); release, qualification and NC gates remain open |
