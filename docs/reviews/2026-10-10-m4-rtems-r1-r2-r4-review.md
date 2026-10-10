# Review Report - M4 increment 2 (RTEMS application integration)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0028                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | base commit `ebe1bf2` + uncommitted working tree (M4 increment 2, 2026-10-10) |
| Items under review                | `config/rtems/rtems_config.c`, `src/core/pal/pal_rtems.c`, `src/main/fsw_main.c`, `src/main/main_native.c`, `CMakeLists.txt`, `cmake/toolchains/rtems-riscv-mpfs.cmake`, `docs/architecture/rtems-integration.md`, `docs/requirements/SRS.md` (SRS-REQ-037) + `traceability.{yaml,md}`, `docs/architecture/platform.md`, `README.md`, `AGENTS.md`, `CHANGELOG.md`, both CI workflows, `docs/vv/verification-results.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness/completeness of the RTEMS 6.2 configuration and entry point, sufficiency for the PAL POSIX surface, CMake wiring, honesty of verification claims, requirement quality/traceability, documentation completeness (N8/N17). ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C, CCSDS 133.0-B-2; norms N1-N17. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh context, independent session. |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the milestone-M4 increment-2 delta (RTEMS application integration)
against the base commit `ebe1bf2` and the uncommitted working tree, as three
independent profiles:

- **R1 (design/code)**: the RTEMS configuration table
  (`config/rtems/rtems_config.c`), the RTEMS glue and `POSIX_Init` entry point
  (`src/core/pal/pal_rtems.c`), the shared bring-up / native entry split
  (`src/main/fsw_main.c`, `src/main/main_native.c`), the CMake wiring
  (`CMakeLists.txt`) and the cross toolchain
  (`cmake/toolchains/rtems-riscv-mpfs.cmake`). Sufficiency of the configuration
  for the actual PAL surface (`src/core/pal/pal_posix.c`, `pal.h`) and
  consistency with `pal_rtems.c`'s stated prerequisites was checked by
  inspection and against the RTEMS 6 configuration documentation and
  `<rtems/confdefs.h>` sources.
- **R2 (V&V)**: honesty and validity of the verification claims for
  SRS-REQ-037 / SSS-REQ-001, the per-commit cross-build proxy
  (`tools/scripts/ci_rtems_cross_build.sh`), the nightly workflow
  (`.github/workflows/nightly.yml`), the verification record
  (`docs/vv/verification-results.md`) and the traceability gate.
- **R4 (documentation, N17)**: completeness/correctness/consistency of the new
  and touched documentation, counts, revision histories, the README metrics
  panel and CI required-document lists.

Executed in this session:

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
  -> **PASS** (72 requirements: 38 SRS / 20 SSS / 14 GAP; 51 Lean theorems;
  9 CI claims).
- `cmake -S . -B build/rv -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/rv && ctest --test-dir build/rv --output-on-failure` -> **9/9 PASS**.
- document-id uniqueness scan -> **64 unique ids, no duplicates**.
- markdown relative-link scan (all `docs/**/*.md` and root `*.md`) -> **0
  broken links**.
- `test_gem` executed directly -> `checks=23 failures=0`.

Excluded / not executable in this environment: the real RTEMS 6.2 cross build
(toolchain absent under `/opt/rtems`), and any on-target runtime (documented
GAP-A5). RTEMS 6 behaviour was therefore verified against the official RTEMS 6
documentation and the upstream `<rtems/confdefs.h>` sources (references in
section 2).

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- Traceability manifest: `docs/requirements/traceability.yaml`
- RTEMS 6 Classic API Guide, *Obsolete Configuration Options* (list of options
  obsolete since RTEMS 5.1) and *POSIX API Configuration*.
- RTEMS upstream `<rtems/confdefs/obsolete.h>` (RTEMS `main`): `#warning` for
  `CONFIGURE_MAXIMUM_POSIX_MUTEXES`,
  `CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES`,
  `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUE_DESCRIPTORS`.
- RTEMS 5-to-6 migration guide: only the `-specs bsp_specs` option was removed;
  `-qrtems` remains the BSP selection flag in RTEMS 6 (`rtems-config --cflags`,
  BSP pkg-config `Cflags` include `-qrtems`).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | `config/rtems/rtems_config.c`; SRS-REQ-037 | The table defines three configuration options that are **obsolete since RTEMS 5.1** and unknown to the RTEMS 6.2 config schema: `CONFIGURE_MAXIMUM_POSIX_MUTEXES` (line 30), `CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES` (line 31), `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUE_DESCRIPTORS` (line 35). Upstream `<rtems/confdefs/obsolete.h>` emits `#warning` for each, so the build carries three config warnings and the documented sizing ("mutexes (32), condition variables (64)") is dead: POSIX mutexes/condvars are self-contained (storage in the `pthread_mutex_t`/`pthread_cond_t` object) and are not sizably configured in RTEMS 6. The artifact is therefore not a valid/current RTEMS 6.2 configuration. | RTEMS 6 *Obsolete Configuration Options* lists all three as obsolete since 5.1; upstream `cpukit/include/rtems/confdefs/obsolete.h` emits `#warning "… is obsolete since RTEMS 5.1"` for the three macros. `rtems-integration.md` §3 repeats the same sizing. | Remove the three obsolete macros (RTEMS 6 needs no mutex/condvar/mqueue-descriptor allocation) and update `rtems-integration.md` §3 and the `pal_rtems.c` prerequisite list. Re-validate against RTEMS 6.2 `confdefs.h`. |
| R1-02 | **Major** | `cmake/toolchains/rtems-riscv-mpfs.cmake`; `CMakeLists.txt`; SRS-REQ-037 | The toolchain sets only `-march=rv64imafdc -mabi=lp64d -mcmodel=medany -O2` and, in this delta, **removes the only link flag** (`CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-Ttext,0x80200000"`) without adding the RTEMS/BSP link configuration. `-qrtems` (and the BSP `-B`/library path) is still required in RTEMS 6 to pull the BSP start files and linker script. Without it the CMake link step either fails or does not produce a bootable RTEMS image, defeating the core deliverable of SRS-REQ-037. Removing the hard-coded `-Ttext` was correct (the BSP owns the memory map), but it had to be replaced by the RTEMS link flags, not dropped. | Toolchain lines 29-32; `git diff` shows `-Wl,-Ttext,0x80200000` deleted and nothing added. RTEMS 5-to-6 migration removes only `-specs bsp_specs`; RTEMS 6 BSP pkg-config `Cflags`/`rtems-config --cflags` include `-qrtems`, and upstream riscv RTEMS 6 flag examples are `-qrtems -march=rv64imafdc -mabi=lp64d -mcmodel=medany`. | Add `-qrtems` to `CMAKE_C_FLAGS_INIT` (and/or `CMAKE_EXE_LINKER_FLAGS_INIT`) together with the BSP library path; confirm with a real cross build before claiming SRS-REQ-037. |
| R1-03 | Minor | `config/rtems/rtems_config.c`, `src/core/pal/pal_rtems.c`, `pal.h`; SRS-REQ-037 | The configuration, the `pal_rtems.c` header comment and `pal.h` describe the PAL as requiring POSIX **message queues/keys/timers/semaphores**, but the actual PAL (`pal_posix.c`) uses pthread mutexes, condition variables, `nanosleep` and `clock_gettime` only — it never calls `mq_open`, POSIX timers, `sem_*` or `pthread_key_*`. Conversely, the explicit prerequisite list in `pal_rtems.c` (lines 31-36) omits the message queues that its own header (line 20 includes `<rtems/posix/mqueue.h>`; line 7) claims are needed. The declared VS used surface is inconsistent and over-stated. | `pal_posix.c` (no mqueue/timer/sem/key calls); `pal_rtems.c:6-7,20-37`; `rtems-integration.md:26,41-42`; `pal.h:11`. | Make the config and comments match the real PAL surface (or explicitly mark the extra object classes as reserved capacity with a justification). Align `pal_rtems.c` prerequisites with the header. |
| R1-04 | Comment | `config/rtems/rtems_config.c` | `CONFIGURE_MAXIMUM_SEMAPHORES 16`, `CONFIGURE_MAXIMUM_POSIX_TIMERS 8`, `CONFIGURE_MAXIMUM_POSIX_KEYS 8` and `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES 8` are not consumed by the PAL. They are valid RTEMS 6 options and harmless, but they consume workspace and are not justified. If POSIX mqueues are ever used, `CONFIGURE_MESSAGE_BUFFER_MEMORY` must also be provided (RTEMS docs). | `pal_posix.c`; RTEMS 6 *POSIX API Configuration* note on `CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES`. | Trim to the objects actually used, or add a one-line justification per class. |
| R1-05 | Comment | `CMakeLists.txt` | `pal_rtems.c` is always compiled into `frick_fsw_core` (guarded by `#ifdef FRICK_PLATFORM_MPFS`); correct, but `FF_ENABLE_WERROR` is never applied to the RTEMS `frick_fsw` executable, so the R1-01 config warnings would be silently tolerated in a stricter build. Enabling `-Werror` for the flight target (or for the config translation unit) would catch config drift. | `CMakeLists.txt:115-117,122-129`. | Consider `-Werror` for the flight build or at least treat confdefs warnings as failures in the cross build. |
| R2-01 | **Major** | `tools/scripts/ci_rtems_cross_build.sh`; SRS-REQ-037 / SSS-REQ-001 | The script that is the sole manifest evidence for `ci:rtems-cross-build` **cannot report a failure**: it uses `set -uo pipefail` (no `-e`) and ends with an unconditional `exit 0`. In the toolchain-present branch it runs `cmake --build` and then prints `cross build: PASS` even if the build failed. The evidence claim can therefore pass while the real cross build is broken. | Script lines 14, 20-32 (`exit 0`; no error propagation). Manifest links SRS-REQ-037 and SSS-REQ-001 to `ci:rtems-cross-build`. | Propagate the build status (`set -euo pipefail` or explicit `if ! cmake --build …; then exit 1; fi`); keep the toolchain-absent SKIP branch separate and never print PASS for an unattempted/failed build. |
| R2-02 | Minor | SRS-REQ-037 (`docs/requirements/SRS.md`) | The requirement states the integration produces a "bootable RTEMS image" and is tagged `method: Dem | Inp`, but there is no boot/runtime demonstration — on-target execution is explicitly GAP-A5. `Dem` (demonstration) is unsupported by the available evidence (build proxy + inspection only). | `SRS.md` SRS-REQ-037; `traceability.yaml` SRS-REQ-037 `tests: ["ci:rtems-cross-build"]`, `lean_note` says "Runtime on target is GAP-A5"; `rtems-integration.md` §7/§8. | Reword to a linkable/compilable image (or state boot explicitly as GAP-A5), and correct the verification method to `Inp` (+ proxy build) until a boot demonstration exists. |
| R2-03 | Minor | `docs/vv/verification-results.md` | The already-recorded "M4 Ethernet GEM driver (increment 1)" cycle-log row was **edited in place** to carry increment-2 counts (72/38) and to add "RTEMS application integration (SRS-REQ-037)", while the History rev 0.7 still records increment 1 as "71 reqs / 37 srs". There is no separate increment-2 cycle row. This retroactively mutates a prior verification record and conflates two increments. | `git diff docs/vv/verification-results.md`: changed line 27 and added rev 0.8; rev 0.7 line retained at 71/37. | Restore the increment-1 row to 71/37 and add a distinct increment-2 row with its own evidence; keep counts aligned with `trace.py`. |
| R2-04 | Comment | `docs/vv/verification-results.md` | The cycle row counts `gem 23` (total 329) but the M4 notes say `test_gem (18 checks)`. The direct run confirms **23** checks, so the notes line is the stale value. Pre-existing (increment 1), but the touched row could have reconciled it. | `./build/rv/tests/test_gem` -> `checks=23 failures=0`; `verification-results.md` line ~253. | Correct the M4 notes to 23 checks. |
| R2-05 | Comment | CI evidence marker | `# evidence: ci-rtems-cross-build (SSS-REQ-001, SRS-REQ-016)` in both workflows does not reference SRS-REQ-037, although the manifest links SRS-REQ-037 to `ci:rtems-cross-build`. The trace gate only requires the claim name, so this is annotation-only. | `.github/workflows/ci.yml:203`, `.gitea/workflows/ci.yml:244`. | Add SRS-REQ-037 to the marker comment for auditability. |
| R4-01 | Minor | `README.md` (N8/N17) | The README status panel (`<!-- METRICS-BEGIN -->`) still reports "Requirements in manifest | 71 (srs=37, sss=20, gap=14)", contradicting the same file's "SRS = 38" and the manifest/gate (72/38/20/14). The README explicitly calls this generated panel authoritative, and N17 lists the README metrics panel in scope. It was not regenerated in this delta. | `README.md` status panel lines ~67-71; `trace.py` output 72/38/20/14/51. | Regenerate with `make metrics`/`tools/scripts/metrics.py` (or update the panel) in the same change. |
| R4-02 | Minor | `docs/architecture/rtems-integration.md` (N8) | §7 reads "the per-commit CI job is a documented proxy that runs the real build in the nightly workflow", which is misleading (the per-commit job does not run the nightly build), and the "nightly" workflow is in fact **weekly** (`cron: "0 2 * * 1"`). §8 likewise says "the nightly already builds for real", an unsubstantiated claim (no nightly result is recorded in the repository). | `rtems-integration.md` §7-§8; `.github/workflows/nightly.yml:15-16`. | Reword to state precisely: per-commit = SKIP proxy unless the toolchain is present; the scheduled (weekly) workflow performs the real build; record its result in `verification-results.md`. |
| R4-03 | Minor | `src/core/pal/pal.h` (N8) | The PAL header describes `pal_rtems.c` as "RTEMS specific bootstrap (POSIX keys, clock, panic)" and `pal_posix.c` as "pthread/mqueue based". Neither matches the code: `pal_rtems.c` contains no key/clock/panic code (only the POSIX-API check and `POSIX_Init`), and `pal_posix.c` uses pthread mutex/condvar ring buffers, not message queues. | `pal.h:9-12`; `pal_rtems.c`; `pal_posix.c`. | Update the header comment to describe the code as implemented. |
| R4-04 | Comment | `docs/architecture/platform.md`, `docs/requirements/SRS.md` | Both files end without a trailing newline (carried over from the base); cosmetic, but it showed up as "\ No newline at end of file" in the delta diff. | `git diff`; `tail -c` on both files. | Add a trailing newline. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Positive verification evidence (reproduced in this session)

- `trace.py`: `traceability: PASS (72 requirements, 38 srs, 20 sss, 14 gaps, 51
  lean theorems checked)`; the new SRS-REQ-037 -> SSS-REQ-001 parent link and
  the `ci:rtems-cross-build` claim resolve. Counts agree with the scope baseline.
- Host build + `ctest`: 9/9 suites PASS.
- Document-id scan: 64 unique ids, no duplicates (`FRICK-ARCH-0011` is unique
  for the new guide).
- Markdown link scan: 0 broken relative links; the new guide is reachable from
  `README.md`, `docs/architecture/platform.md`, and is present in the required
  document lists of **both** CI workflows.
- `AGENTS.md`, `CHANGELOG.md`, `SRS.md`, `traceability.{yaml,md}` and
  `platform.md` were updated together with the code (N8), and the entry-point
  split (`POSIX_Init` for RTEMS, `main()` for host via `FF_HAS_RTEMS`) is
  consistent between `pal_rtems.c`, `main_native.c` and `CMakeLists.txt`.
- The verification narrative in `rtems-integration.md` §7/§8 and the
  `lean_note` for SRS-REQ-037 are, in substance, honest about the proxy/real/
  GAP-A5 distinction (wording issues in R4-02, R2-02 notwithstanding).

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **REJECTED** | Two Major findings: R1-01 (obsolete RTEMS 6 options, invalid configuration) and R1-02 (missing `-qrtems`/BSP link flags, no bootable image). |
| R2 (V&V) | **REJECTED** | One Major finding: R2-01 (the sole cross-build evidence script cannot report failure). |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | Minor findings R4-01..R4-03 and Comment R4-04 only; all to be tracked to closure. |

Overall: **REJECTED**. Per norm N12 the change must not be committed/pushed
until R1-01, R1-02 and R2-01 are resolved and the deltas re-reviewed. The
Minor/Comment findings are to be dispositioned and tracked to closure.

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | REJECTED (R1-01, R1-02 Major) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | REJECTED (R2-01 Major) |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `config/rtems/rtems_config.c`, SRS-REQ-037 | Obsolete RTEMS <5.1 config options defined | Remove obsolete macros; update docs | (pending) | frick-fsw | open | (pending) |
| R1-02 | Major | `cmake/toolchains/rtems-riscv-mpfs.cmake`, SRS-REQ-037 | Missing `-qrtems`/BSP link flags; no bootable image | Add RTEMS link flags; real cross build | (pending) | frick-fsw | open | (pending) |
| R2-01 | Major | `tools/scripts/ci_rtems_cross_build.sh` | Cross-build evidence script cannot fail | Propagate build status; keep SKIP distinct | (pending) | frick-fsw | open | (pending) |
| R1-03 | Minor | config / PAL docs | Declared POSIX surface over-stated / inconsistent | Align config, comments and `pal_rtems.c` prerequisites | (pending) | frick-fsw | open | (pending) |
| R2-02 | Minor | SRS-REQ-037 | "bootable" claim / `Dem` method unsupported | Reword; fix verification method | (pending) | frick-fsw | open | (pending) |
| R2-03 | Minor | `docs/vv/verification-results.md` | Increment-1 row mutated to carry increment-2 data | Restore row; add increment-2 row | (pending) | frick-fsw | open | (pending) |
| R4-01 | Minor | `README.md` | Stale metrics panel (71/37) | Regenerate `make metrics` | (pending) | frick-fsw | open | (pending) |
| R4-02 | Minor | `docs/architecture/rtems-integration.md` | Proxy/nightly wording; weekly not nightly | Reword; record scheduled result | (pending) | frick-fsw | open | (pending) |
| R4-03 | Minor | `src/core/pal/pal.h` | PAL header description stale | Update header comment | (pending) | frick-fsw | open | (pending) |
| R1-04 | Comment | config | Unused object classes not justified | Trim or justify | (pending) | frick-fsw | open | (pending) |
| R1-05 | Comment | `CMakeLists.txt` | No `-Werror` on flight target | Consider `-Werror` | (pending) | frick-fsw | open | (pending) |
| R2-04 | Comment | verification record | `test_gem` 18 vs 23 checks | Correct notes to 23 | (pending) | frick-fsw | open | (pending) |
| R2-05 | Comment | CI marker | SRS-REQ-037 missing from marker | Add SRS-REQ-037 | (pending) | frick-fsw | open | (pending) |
| R4-04 | Comment | docs | Missing trailing newlines | Cosmetic fix | (pending) | frick-fsw | open | (pending) |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Initial milestone-M4 increment-2 review (RTEMS application integration) |
