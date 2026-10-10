# Review Report - M4 increment 2 (RTEMS application integration) - delta re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0029                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | base commit `ebe1bf2` + uncommitted working tree (M4 increment 2 fixes, 2026-10-10) |
| Items under review                | fixes for `FRICK-REVIEW-0028`: `config/rtems/rtems_config.c`, `cmake/toolchains/rtems-riscv-mpfs.cmake`, `tools/scripts/ci_rtems_cross_build.sh`, `src/core/pal/pal_rtems.c`, `src/core/pal/pal.h`, `docs/requirements/SRS.md` (SRS-REQ-037) + `traceability.{yaml,md}`, `docs/architecture/rtems-integration.md`, `docs/architecture/platform.md`, `README.md`, `docs/vv/verification-results.md`, both CI workflows |
| Predecessor review                | `docs/reviews/2026-10-10-m4-rtems-r1-r2-r4-review.md` (FRICK-REVIEW-0028, REJECTED) |
| Date                              | 2026-10-10                             |
| Scope                             | Delta re-review of the disposition of the predecessor NCs (R1-01, R1-02, R2-01 Major; R1-03, R2-02, R2-03, R4-01..R4-03 Minor; R1-04/R1-05, R2-04/R2-05, R4-04 Comments). ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C, CCSDS 133.0-B-2; RTEMS 6.2 documentation; norms N1-N17. |
| Independence declaration          | Reviewer took no part in the fixes; fresh context, independent session (N9). |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Delta re-review (norm N12) of the fixes applied to the M4 increment-2
(RTEMS application integration) non-conformances raised by FRICK-REVIEW-0028.
The three Major NCs (R1-01 obsolete RTEMS configuration options, R1-02 missing
`-qrtems`, R2-01 cross-build script unable to fail) and the Minor items named
in the fix request (R1-03, R2-02, R4-01, R4-02) were verified by inspection and
by re-running the gates. The remaining predecessor NCs were checked for
collateral change.

Executed in this session:

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (72 requirements: 38 SRS / 20 SSS / 14 GAP; 51 Lean theorems;
  9 CI evidence claims).
- `cmake -S . -B build/drt -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/drt && ctest --test-dir build/drt --output-on-failure` -> **9/9 PASS**
  (ccsds, sb, tbl, pus, time, hal, math, sdls, gem).
- `tools/scripts/ci_rtems_cross_build.sh` control-flow simulation in `/tmp`
  (fake toolchain present/absent; failing/succeeding dummy build):
  absent -> exit **0** (SKIP); failing build -> exit **2**; succeeding build ->
  exit **0** with `cross build: PASS`.
- Independent confirmation against the official RTEMS 6 documentation that the
  three removed macros are obsolete since RTEMS 5.1 and that the retained
  POSIX options are current.
- Read-only manifest/doc checks: 72 requirements / 72 implementation / 58
  tested / 20 Lean; 44 unique document-id headers (no duplicates); 87 markdown
  files scanned, 0 broken relative links; `test_gem` -> `checks=23 failures=0`.

Excluded / not executable in this environment: the real RTEMS 6.2 cross build
(toolchain absent under `/opt/rtems`) and any on-target runtime (documented
GAP-A5). R1-02 was therefore verified **statically**; RTEMS 6 semantics were
verified against the official documentation.

## 2. References

- Project norms: `docs/rules.md` (N1-N17); template `docs/reviews/review-template.md`.
- Predecessor: `docs/reviews/2026-10-10-m4-rtems-r1-r2-r4-review.md` (FRICK-REVIEW-0028).
- RTEMS 6 Classic API Guide, *Obsolete Configuration Options*
  (<https://docs.rtems.org/docs/main/c-user/config/obsolete.html>): lists
  `CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES`,
  `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUE_DESCRIPTORS` and
  `CONFIGURE_MAXIMUM_POSIX_MUTEXES` as obsolete since RTEMS 5.1.
- RTEMS 6 Classic API Guide, *POSIX API Configuration*
  (<https://docs.rtems.org/docs/main/c-user/config/posix-api.html>): current
  options `CONFIGURE_MAXIMUM_POSIX_{KEYS,MESSAGE_QUEUES,THREADS,TIMERS}`;
  `CONFIGURE_MESSAGE_BUFFER_MEMORY` sizing note for message queues.
- RTEMS 5.1 release notes (self-contained synchronization objects, e.g. POSIX
  mutexes).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70C-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings (remaining after the fixes)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-01 | Minor | `docs/architecture/rtems-integration.md` §3 | §3 still states the "POSIX objects required by the PAL: threads (32), **mutexes (32), condition variables (64)**, timers (8), keys (8), message queues (8), file descriptors (16)". Mutex/condvar options are the macros that were just removed (self-contained in RTEMS 6); the PAL uses none of the mqueue/timer/key classes. The guide contradicts the configuration it documents and repeats the dead sizing called out in R1-01. | `rtems-integration.md:41-42`; `config/rtems/rtems_config.c:24-40`; `pal_posix.c` (pthread mutex/condvar + `nanosleep`/`clock_gettime` only). |
| D-02 | Minor | `config/rtems/rtems_config.c` banner; `src/core/pal/pal_rtems.c` header; `src/core/pal/pal.h:9` | The declared PAL surface remains over-stated: the config banner says the PAL requires "pthreads, mutexes, condition variables, **message queues, keys, timers**"; `pal_rtems.c` says the TU contains "the RTEMS clock and the panic/reboot path" though it contains neither (only `FF_PalPlatformInit` + `POSIX_Init`); `pal.h` still describes `pal_posix.c` as "pthread/mqueue based" (no mqueue calls). | `config/rtems/rtems_config.c:7-9`; `pal_rtems.c:4-7,19-23`; `pal.h:9-13`; `pal_posix.c`. |
| D-03 | Minor | `docs/architecture/rtems-integration.md` §8; `docs/requirements/traceability.yaml` SRS-REQ-037 | §8 still reads "the nightly **already builds for real**" (unsubstantiated: the workflow is weekly and no scheduled result is recorded) and the manifest `lean_note` says "per-commit proxy plus **nightly**", while §7 and the CI/script wording now say "scheduled/unattended". Residual of R4-02. | `rtems-integration.md:97-99`; `traceability.yaml` SRS-REQ-037 `lean_note`; `.github/workflows/nightly.yml:15-16`. |
| D-04 | Comment | `docs/vv/verification-results.md` | The cycle log carries a single combined row "M4 drivers + RTEMS integration" (72/38); the increment-1 row was relabelled rather than restored-and-separated, the M4 notes still describe only "increment 1", and the notes still say `test_gem (18 checks)` although the direct run reports **23**. Increment-2 V&V evidence is not distinctly logged. | `verification-results.md:27,241-257`; `./build/drt/tests/test_gem` -> `checks=23`; `git diff`. |
| D-05 | Comment | CI evidence markers | The `# evidence: ci-rtems-cross-build` marker in both workflows lists `SSS-REQ-001, SRS-REQ-016` but not `SRS-REQ-037`, which the manifest links to the claim. Annotation-only (the trace gate keys on the claim name). | `.github/workflows/ci.yml:203`; `.gitea/workflows/ci.yml:244`. |
| D-06 | Comment | `CMakeLists.txt` | `FF_ENABLE_WERROR` is still applied only to `frick_fsw_core`, not to the `frick_fsw` flight executable, so config/confdefs drift would not fail a strict build. | `CMakeLists.txt:115-117,122-130`. |
| D-07 | Comment | docs trailing newlines | `docs/architecture/platform.md`, `docs/requirements/SRS.md`, `docs/requirements/traceability.md` and `docs/vv/verification-results.md` still end without a trailing newline. | `tail -c1` on each file. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No Major finding remains. D-01/D-02 carry the residual of predecessor R1-03
(and R4-03); D-03 carries R4-02; D-04 carries R2-03/R2-04.

## 4. Positive verification evidence (reproduced in this session)

- **R1-01 closed.** `config/rtems/rtems_config.c` no longer defines
  `CONFIGURE_MAXIMUM_POSIX_MUTEXES`, `CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES`
  or `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUE_DESCRIPTORS` (only an explanatory
  comment naming them, line 28-31). Independently confirmed against the RTEMS 6
  *Obsolete Configuration Options* page (all three obsolete since 5.1) and the
  *POSIX API Configuration* page (the retained `CONFIGURE_MAXIMUM_POSIX_THREADS
  / TIMERS / KEYS / MESSAGE_QUEUES` and `CONFIGURE_MAXIMUM_FILE_DESCRIPTORS` are
  current, bounded options). The set covers the PAL surface plus the
  `POSIX_Init` thread.
- **R1-02 closed (static).** `cmake/toolchains/rtems-riscv-mpfs.cmake:33-34`
  now sets `CMAKE_C_FLAGS_INIT "-march=rv64imafdc -mabi=lp64d -mcmodel=medany
  -O2"` and `CMAKE_EXE_LINKER_FLAGS_INIT "-qrtems"`; the `-Wl,-Ttext,0x80200000`
  override is gone. The BSP library/start-file path is provided via
  `CMAKE_SYSROOT` (`.../riscv/mpfs64imafdc`). The real cross link cannot be run
  here (toolchain absent; GAP-A5).
- **R2-01 closed.** `tools/scripts/ci_rtems_cross_build.sh` uses
  `set -euo pipefail`; the toolchain-absent branch is a documented SKIP
  (`exit 0`, line 24); a toolchain-present failing build aborts non-zero. A
  control-flow simulation (stub CMake project whose default target fails)
  returned exit **2**; the success path returned **0** and printed
  `cross build: PASS`.
- **R2-02 closed.** SRS-REQ-037 now reads "so that the same sources build for
  the RTEMS target. `method: Inp` (on-target demonstration is GAP-A5)"
  (`SRS.md:181-186`): "bootable" and the unsupported `Dem` method are gone.
- **R4-01 closed.** The README metrics panel (`README.md:72-76`) now reports
  `72 (srs=38, sss=20, gap=14)`, `72/72` implementation, `58/72 (80.6%)`
  tested, `20/72 (27.8%)` Lean; read-only counts of the manifest reproduce
  exactly these values and agree with `trace.py`.
- **Gates green:** `trace.py` PASS; host build + `ctest` 9/9; document-id
  header scan 44 unique / no duplicates; 87 markdown files, 0 broken relative
  links; `test_gem` 23/0.
- **Wiring:** `config/rtems/rtems_config.c` is compiled into the `frick_fsw`
  executable for `FF_PLATFORM=rtems` (`CMakeLists.txt:125-129`), while
  `pal_rtems.c` is compiled into `frick_fsw_core` under `FRICK_PLATFORM_MPFS`
  (`CMakeLists.txt:33-34,53`); `main_native.c` is excluded on RTEMS.

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **APPROVED WITH COMMENTS** | Both Majors closed (R1-01, R1-02). R1-03 only partially closed: the `pal_rtems.c` prerequisite list now matches the config, but the declared-vs-implemented PAL surface remains over-stated (D-01, D-02). Comments R1-04/R1-05 open. |
| R2 (V&V) | **APPROVED WITH COMMENTS** | Major R2-01 closed (failure now propagates); Minor R2-02 closed. R2-03 only partially closed (no distinct increment-2 cycle row; D-04). Comments R2-04/R2-05 open. |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | R4-01 closed. R4-02 and R4-03 only partially closed (D-03, D-02); R4-04 open (D-07). New documentation Minor findings D-01/D-03. |

Overall: **APPROVED WITH COMMENTS**. No Major finding remains; the delta may
be committed/pushed with the Minor findings tracked to closure (norm N12,
template "APPROVED WITH COMMENTS"). The previously disqualifying defects
(invalid RTEMS 6 configuration, missing `-qrtems`, non-failing cross-build
evidence) are resolved.

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (Majors R1-01/R1-02 closed; R1-03 partial) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (Major R2-01 closed; R2-03 partial) |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R4-01 closed; R4-02/R4-03 partial) |

## 7. Follow-up records

(NC-by-NC disposition after the fixes; norm N12: NCs are resolved by
re-applying ALL rules to the affected scope and re-reviewing the deltas until
full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `config/rtems/rtems_config.c`, SRS-REQ-037 | Obsolete RTEMS <5.1 config options defined | Remove obsolete macros | Three obsolete macros absent; remaining options valid; confirmed vs RTEMS 6 docs | frick-fsw | **closed** | Closed; residual doc wording split to D-01 |
| R1-02 | Major | `cmake/toolchains/rtems-riscv-mpfs.cmake`, SRS-REQ-037 | Missing `-qrtems`/BSP link flags | Add `-qrtems`; drop `-Ttext`; align march/abi | `-qrtems` in `CMAKE_EXE_LINKER_FLAGS_INIT`; `rv64imafdc`/`lp64d`; no `-Ttext`; BSP path via `CMAKE_SYSROOT` (static only) | frick-fsw | **closed** | Closed (real cross link remains an environmental gap) |
| R2-01 | Major | `tools/scripts/ci_rtems_cross_build.sh` | Cross-build evidence script cannot fail | `set -euo pipefail`; SKIP distinct | Simulated failing build -> exit 2; SKIP -> exit 0; success -> exit 0 | frick-fsw | **closed** | Closed |
| R1-03 | Minor | config / PAL docs | Declared POSIX surface over-stated / inconsistent | Align config, comments and `pal_rtems.c` prerequisites | `pal_rtems.c` list now equals the config; over-statement in config banner / `pal.h` / guide §3 remains | frick-fsw | **open** | Partially closed; residual D-01/D-02 |
| R2-02 | Minor | SRS-REQ-037 | "bootable" claim / `Dem` method unsupported | Reword; fix verification method | "same sources build for the RTEMS target"; `method: Inp`; GAP-A5 | frick-fsw | **closed** | Closed |
| R2-03 | Minor | `docs/vv/verification-results.md` | Increment-1 row mutated to carry increment-2 data | Restore row; add increment-2 row | Single relabelled combined row; M4 notes still "increment 1"; no distinct increment-2 evidence row | frick-fsw | **open** | Not closed; carried as D-04 |
| R4-01 | Minor | `README.md` | Stale metrics panel (71/37) | Regenerate `make metrics` | Panel 72/38, 72/72, 58/72, 20/72; matches manifest counts | frick-fsw | **closed** | Closed |
| R4-02 | Minor | `docs/architecture/rtems-integration.md` | Proxy/nightly wording; weekly not nightly | Reword; record scheduled result | §7 reworded to scheduled/unattended; §8 still "nightly already builds for real"; no result recorded | frick-fsw | **open** | Partially closed; residual D-03 |
| R4-03 | Minor | `src/core/pal/pal.h` | PAL header description stale | Update header comment | `pal.h` now mentions `POSIX_Init` but still "clock, panic" and "pthread/mqueue based"; `pal_rtems.c` header still claims clock/panic | frick-fsw | **open** | Partially closed; residual D-02 |
| R1-04 | Comment | config | Unused object classes not justified | Trim or justify | `SEMAPHORES 16`, `POSIX_TIMERS 8`, `POSIX_KEYS 8`, `MESSAGE_QUEUES 8` still unjustified; mqueue buffer-memory note applies | frick-fsw | open | Open (Comment) |
| R1-05 | Comment | `CMakeLists.txt` | No `-Werror` on flight target | Consider `-Werror` | `FF_ENABLE_WERROR` still core-only | frick-fsw | open | Open (Comment; D-06) |
| R2-04 | Comment | verification record | `test_gem` 18 vs 23 checks | Correct notes to 23 | `verification-results.md:254` still "18 checks"; direct run = 23 | frick-fsw | open | Open (Comment; D-04) |
| R2-05 | Comment | CI marker | SRS-REQ-037 missing from marker | Add SRS-REQ-037 | Marker still `(SSS-REQ-001, SRS-REQ-016)` | frick-fsw | open | Open (Comment; D-05) |
| R4-04 | Comment | docs | Missing trailing newlines | Cosmetic fix | Newline still absent in 4 files | frick-fsw | open | Open (Comment; D-07) |
| D-01 | Minor | `docs/architecture/rtems-integration.md` §3 | Guide lists obsolete mutex/condvar sizing | Align §3 with config and real PAL surface | `rtems-integration.md:41-42` vs `rtems_config.c:24-40` | frick-fsw | open | New delta Minor |
| D-02 | Minor | config banner / `pal_rtems.c` header / `pal.h` | Declared PAL surface over-stated | Align comments with implemented surface | See D-02 evidence | frick-fsw | open | New delta Minor |
| D-03 | Minor | guide §8 / manifest `lean_note` | "nightly already builds for real"; nightly vs scheduled | Reword; record scheduled result | See D-03 evidence | frick-fsw | open | New delta Minor |
| D-04 | Comment | `docs/vv/verification-results.md` | Combined cycle row; 18 vs 23 checks | Separate increment-2 row; correct 23 | See D-04 evidence | frick-fsw | open | New delta Comment |
| D-05 | Comment | CI markers | SRS-REQ-037 absent | Add id to marker | See D-05 evidence | frick-fsw | open | New delta Comment |
| D-06 | Comment | `CMakeLists.txt` | `-Werror` not on flight target | Consider `-Werror` | See D-06 evidence | frick-fsw | open | New delta Comment |
| D-07 | Comment | docs | Missing trailing newlines | Cosmetic fix | See D-07 evidence | frick-fsw | open | New delta Comment |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Delta re-review of FRICK-REVIEW-0028 fixes (RTEMS integration): Majors closed, APPROVED WITH COMMENTS |
