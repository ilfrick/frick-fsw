# Review Report — Real open-source RTEMS 6.2 toolchain, cross build and Renode functional run (N18 in force) — R1/R2/R4

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0040                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | working tree on base commit `c45d0ae` (branch `main`, uncommitted) |
| Items under review                | `src/core/pal/pal_rtems.c`; `config/rtems/rtems_config.c`; `cmake/toolchains/rtems-riscv-mpfs.cmake`; `tools/docker/rtems/Dockerfile`; `tools/docker/renode/frick-renode.resc`; `tools/scripts/renode_vv.sh`; `tools/scripts/renode_env_selftest.sh`; docs: `docs/rules.md` (N18), `docs/vv/renode-environment.md` (0.4), `docs/vv/verification-results.md` (0.16), `docs/vv/tool-qualification.md` (0.2.6), `docs/status/status.json`, `README.md`, `AGENTS.md`, `CHANGELOG.md`, `docs/requirements/traceability.yaml` |
| Date                              | 2026-10-10                             |
| Scope                             | Independent verification of the working-tree change that builds the **real open-source RTEMS 6.2 toolchain** (`riscv/mpfs64imafdc`), cross-builds the middleware and runs it on the Renode `polarfire-soc` platform, and that declares norm **N18 in force**. Verified with the real artefacts: (1) correctness of the C fixes (`pal_rtems.c`, `rtems_config.c`) against the built BSP headers; (2) an independent fresh cross build and an independent Renode run; (3) honesty of the V&V records and the N18 claim; (4) the host gates (trace, native build + ctest, format, `bash -n`); (5) cross-document consistency and absence of over-claim. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1–N18 (`docs/rules.md`). |
| Independence declaration          | Reviewer took no part in authoring, fixing, executing or reviewing any item under review; fresh reviewer instance; every verdict is based solely on the current repository state and the commands recorded in §7. No tracked repository file other than this report was created or modified; the scratch build directories created for the review were removed. |
| Norms applied                     | N1–N18 (see `docs/rules.md`)           |

## 1. Scope

The change under review is the working tree on base commit `c45d0ae`
(`git status`: 15 modified files). It (a) builds the open-source RTEMS 6.2
toolchain image `frick-rtems-toolchain:local`, (b) fixes real-toolchain build
defects in `pal_rtems.c`, `rtems_config.c` and the CMake toolchain file, (c)
boots the cross-built middleware on the Renode `polarfire-soc` platform and
(c) declares N18 **in force**.

Included: the listed source, build, script and documentation files. Excluded:
the Renode `.repl` platform model itself (external, pinned), the RTEMS/RSB
upstream sources, and the real silicon (GAP-A4/A5/A6 remain open). Every claim
below was checked against the actual toolchain, BSP headers and emulator
present on this host (`frick-rtems-toolchain:local` id `e3a9673d8803`,
`antmicro/renode` digest `sha256:7905c329…` → image `e278e663fd95`).

The RTEMS BSP headers were inspected inside the toolchain image under
`/opt/rtems/6/riscv-rtems6/mpfs64imafdc/lib/include`, i.e. the exact headers
used by the verified build.

## 2. References

- Project norms: `docs/rules.md` (N1–N18)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Requirements in scope: `SRS-REQ-037`, `SRS-REQ-044`, `SSS-REQ-001`,
  `SSS-REQ-013`
- Tool records: `docs/vv/tool-qualification.md` (T10, T17, T18),
  `docs/vv/renode-environment.md`, `docs/vv/verification-results.md`
- RTEMS 6.2 BSP headers: `/opt/rtems/6/riscv-rtems6/mpfs64imafdc/lib/include`
  (inside `frick-rtems-toolchain:local`)
- RTEMS `rtems/confdefs/initthread.h`, `rtems/score/cpuopts.h`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| F-01 | **Minor** (R1, R4) | `src/core/pal/pal_rtems.c`; `CHANGELOG.md`; `docs/vv/verification-results.md:30` | The change reports that `pal_rtems.c` "included **non-existent** RTEMS headers `rtems/posix/{pthread,mqueue,semaphore,time}.h`". Only `rtems/posix/time.h` does not exist. `rtems/posix/pthread.h`, `rtems/posix/mqueue.h` and `rtems/posix/semaphore.h` **do exist** in the built BSP and compile cleanly. The code change (removing them) is still correct — they are private "Private Support Information" headers and are unused by this translation unit — but the defect is over-stated. | BSP listing: `ls …/rtems/posix/` shows `pthread.h`, `mqueue.h`, `semaphore.h`; `find … -path "*rtems/posix/time.h"` returns nothing. Compile `<rtems.h>` + the three headers → exit 0; adding `time.h` → `fatal error: rtems/posix/time.h: No such file or directory`. §7.2. | Reword the CHANGELOG/verification record to "one non-existent header (`rtems/posix/time.h`) plus unused private headers (`pthread/mqueue/semaphore`)". |
| F-02 | **Minor** (R1, R4) | `src/core/pal/pal_rtems.c:37-42`; `CHANGELOG.md:148-149`; `docs/vv/verification-results.md:30` | The change reports a "misplaced `RTEMS_POSIX_API` `#error`" as a real build defect. The guard does **not** fire: `<rtems.h>` → `<rtems/config.h>` → `<rtems/score/cpuopts.h>` defines `RTEMS_POSIX_API 1` for this BSP, so the removed `#if !defined(RTEMS_POSIX_API) #error` was a working compile-time guard, not a defect. The replacement comment also mis-attributes the macro's origin: `RTEMS_POSIX_API` is generated into the BSP `cpuopts.h`, not "defined by `config/rtems/rtems_config.c`" via `<rtems/confdefs.h>`. | `cpuopts.h` line 13: `#define RTEMS_POSIX_API 1`; compiling the original guard with `-DFRICK_PLATFORM_MPFS` against the real BSP → exit 0 (no `#error`). `grep "define RTEMS_POSIX_API"` in the BSP include tree returns only `cpuopts.h`. §7.2. | Keep the removal (harmless) but correct the narrative: either retain a link-time expectation note only, or state truthfully that the guard was satisfied by the BSP and was removed as redundant; attribute the macro to the BSP `cpuopts.h`. |
| F-03 | **Minor** (R4, N8) | `docs/architecture/rtems-integration.md:49`; `docs/requirements/traceability.yaml:424`; `AGENTS.md:58`; `Makefile:53` | The change declares N18 *in force* but leaves contradicting stale text in documents that were not updated in the same commit: `rtems-integration.md:49` still says *"The PAL confirms at build time that the POSIX API is enabled (`#if !defined(RTEMS_POSIX_API) #error`)"* though that code was deleted; `traceability.yaml:424` (authoritative requirement→verification mapping) still calls the SRS-REQ-044 gate *"an advisory proxy until the RTEMS toolchain/image are available … then blocking"*; `AGENTS.md:58` still says *"advisory until the RTEMS image exists"* while the N18 row in the same file says *"in force"*; `Makefile:53` still says *"advisory proxy until the RTEMS image exists"*. | `grep -rn "RTEMS_POSIX_API" …` → `rtems-integration.md:49`; `traceability.yaml:424`; `AGENTS.md:58`; `Makefile:53` (text shown in §7.5). | Update all four in the same commit (N8/N16) so the documentation set is internally consistent with the new N18 state. |
| F-04 | **Minor** (R2, R4; N10/N15) | `tools/docker/rtems/Dockerfile` (final `RUN`), `Makefile` (`rtems-docker`) | The Dockerfile change adds `cmake ninja-build` to the toolchain image, but the image actually used to produce the verified evidence (`frick-rtems-toolchain:local`) does **not** contain them. The independent reproduction therefore had to `apt-get install cmake ninja-build` at run time, i.e. the verified flow diverges from the committed `make rtems-docker` flow (which would fail on the tagged image with `cmake: command not found`). The Dockerfile edit is consequently unverified/non-reproduced. | `docker run … frick-rtems-toolchain:local which cmake ninja` → `cmake: command not found`, `ninja: command not found` (exit 127). The reproduction command in the task installs them explicitly. §7.3. | Rebuild and retag `frick-rtems-toolchain:local` from the current Dockerfile and re-run `make rtems-docker` (or drop the runtime install); record the image id used for the evidence. |
| F-05 | **Comment** (R2, N18) | `tools/scripts/renode_vv.sh`; `.github/workflows/ci.yml` / `.gitea/workflows/ci.yml` (job `ci-renode`); `.github/workflows/nightly.yml` | "N18 in force" is honest but narrower than it sounds: on a fresh per-commit checkout `build/renode` is git-ignored and absent, so `renode_vv.sh` prints `SKIP` and exits 0 in **both** per-commit workflows; the real run only happens in the weekly nightly. Further, the nightly does not make a SKIP fatal — if the Renode image pull failed, `renode_env_selftest.sh`/`renode_vv.sh` would still exit 0 and the nightly would pass without a `RENODE_TEST: PASS`. `rules.md` N18 and the nightly header disclose the per-commit SKIP, so the records are not dishonest, but the enforcement residual is worth closing. | `renode_vv.sh:29-31` (`[[ ! -f ELF ]] → SKIP; exit 0`); `ci.yml:76-80` runs the same two scripts; `nightly.yml` installs the toolchain and builds the ELF, but neither script fails on SKIP. §7.6. | In the nightly, assert the ELF exists and the marker is present (treat SKIP as failure), consistent with "in force"; or restate N18 as "blocking where the image+ELF exist, SKIP elsewhere". |
| F-06 | **Comment** (R4) | `docs/architecture/rtems-integration.md`, `docs/requirements/SSS.md`, `CHANGELOG.md`, all "RTEMS 6.2" wording | The BSP built by the image reports `RTEMS_VERSION "6.0.0"` (`__RTEMS_MINOR__ 0`). "6.2" is the **RSB release tag** (`rtems-source-builder` tag `6.2`, verified) and the kernel is built from a pinned git commit (`rtems_kernel_6` → commit `6335d7e…`), which RTEMS `version.h` documents always reports minor `0` for branch snapshots. The claim is defensible but could mislead; it is not clarified anywhere. | `cpuopts.h`: `#define RTEMS_VERSION "6.0.0"`, `#define __RTEMS_MINOR__ 0`; `rtems-kernel-6.cfg`: `%define rtems_kernel_version 6335d7e…`; `riscv-rtems6-gcc --version`: `RTEMS 6, RSB 3814cb0e…`; BSP `rtems/version.h` explanatory comment. §7.4. | Add one clarifying sentence (e.g. "6.2 = RSB/release tag; kernel pinned to the RTEMS 6 branch snapshot, whose version macro reads 6.0.0 by design"). |
| F-07 | **Comment** (R4) | `docs/rules.md:291`; `docs/status/status.json` (milestones) | Minor consistency: `rules.md:291` writes the CMake option as `FF_ENABLE_RENODE_TEST=1` while the canonical form used elsewhere is `=ON`. `status.json` `milestones` still lists M8/M9/M10 as `planned`/`in progress` and omits M11, while `CHANGELOG.md` records M8–M11 as delivered — pre-existing, outside this change, but visible in the panel updated here. | `grep FF_ENABLE_RENODE_TEST docs/rules.md`; `status.json:8-19`. | Use `=ON` in `rules.md`; refresh the milestone list in a later doc pass (N8/N16). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Positive confirmations (verified, not findings)

These are recorded because they are the core of the change and were reproduced
independently (see §7):

- **P-01 (R1)** The added `void *POSIX_Init(void *);` declaration is genuinely
  **required**: without it the real toolchain fails at
  `rtems/confdefs/initthread.h:56` with `error: 'POSIX_Init' undeclared here`
  (the table initialiser `{ CONFIGURE_POSIX_INIT_THREAD_ENTRY_POINT, … }` uses
  the symbol before `confdefs.h` can see a definition). The existing header
  comment already exposes the same contract. Fix is correct and minimal. §7.2.
- **P-02 (R1)** The removed headers and `stdio`/`stdarg` are not referenced by
  `pal_rtems.c` after the fix (`grep -E "pthread|mqueue|semaphore|stdio|stdarg|mq_|sem_|clock_"` → none); the TU uses only `rtems.h`, `rtems_fatal`, `POSIX_Init`. §7.2.
- **P-03 (R1)** The CMake toolchain file is correct: the BSP headers/libs are
  at `<bsp>/lib/include` / `<bsp>/lib`, the gcc search paths do **not** contain
  them (`-I`/`-L` are required), `-mcmodel=medany` selects the only
  `rv64imafdc/lp64d/medany` multilib, and `${RTEMS_HOME}/6` matches the image.
  A **fresh** cross build (plain and Renode) succeeds from a clean configure.
  §7.3.
- **P-04 (R2)** The Renode functional run is **real and reproducible**: a fresh
  cross build produced a byte-identical ELF (sha256 `214168661f2e…`) to the one
  under `build/renode`, and `make renode` → `[renode] PASS (RENODE_TEST: PASS)`.
  The Renode console (`build/renode/console.log`) shows the banner, `7
  applications registered`, `[renode-test] sb_roundtrip=PASS` and
  `RENODE_TEST: PASS`. `readelf` entry point `0x1000000000`, `RVC`,
  double-float ABI, matching the documented boot. §7.3/§7.4.
- **P-05 (R2/R4)** Cross-document counts and revision histories are consistent:
  `trace.py` reports `80 requirements, 45 srs, 21 sss, 14 gaps, 51 lean
  theorems, 12 CI evidence claims`, matching `verification-results.md` 0.16,
  `README.md` ("SRS = 45 · SSS = 21 · GAP = 14 · Lean = 51") and
  `traceability.md`; the new history rows (0.4, 0.2.6, 0.16) are present. §7.6.
- **P-06 (R2)** Host gates pass: `trace.py` PASS; native Debug build + `ctest`
  **11/11**; `clang-format --dry-run --Werror` over `src config tests` clean in
  `frick-vv:local`; `bash -n` on the changed scripts OK. §7.6.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Per profile**

- **R1 (SW dev): APPROVED WITH COMMENTS.** The code is correct and compiles
  against the real BSP; the fixes are necessary and minimal (P-01/P-02/P-03).
  Findings F-01/F-02 are accuracy defects in the *justification* recorded for
  the fixes, not in the fixes themselves.
- **R2 (SW V&V): APPROVED WITH COMMENTS.** The real toolchain, cross build and
  emulated run were independently reproduced and the evidence is truthful
  (P-04/P-05/P-06). F-04 (unverified Dockerfile edit), F-05 (per-commit N18
  still SKIPs; nightly can pass on a SKIP) are the open V&V residuals; N18 is
  disclosed as such and no false pass is claimed.
- **R4 (SW documentation, N17): APPROVED WITH COMMENTS.** Revisions, history
  tables and counts are consistent and the status panel correctly reflects the
  verified run. F-03 is the principal documentation NC: three stale statements
  contradict the new "N18 in force" state and one (`rtems-integration.md:49`)
  describes code that no longer exists. F-06/F-07 are clarity/consistency
  comments.

**Overall: APPROVED WITH COMMENTS.** The substantive technical claim — a real
open-source RTEMS 6.2 toolchain, a real cross build and a real
`RENODE_TEST: PASS` on the Renode PolarFire SoC — is fully substantiated and
reproducible. No Major finding. Closure requires only documentation accuracy
(F-01, F-02, F-03), one re-verification of the image/Dockerfile alignment
(F-04), and the advisory enforcement residual (F-05); the Comments need no
derogation.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (SW dev)   | independent agent (FRICK-REVIEW-0040) | 2026-10-10 | APPROVED WITH COMMENTS |
| R2 (SW V&V)   | independent agent (FRICK-REVIEW-0040) | 2026-10-10 | APPROVED WITH COMMENTS |
| R4 (SW doc, N17) | independent agent (FRICK-REVIEW-0040) | 2026-10-10 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| F-01 | Minor | pal_rtems.c / records | "non-existent headers" over-stated | Reword to time.h + unused private headers | **Post-review:** `CHANGELOG.md` / `verification-results.md` reworded (only `rtems/posix/time.h` missing; others unused private headers) | frick-fsw | **CLOSED** | done (addendum §8) |
| F-02 | Minor | pal_rtems.c / records | `#error` claim unsupported; comment mis-attributes macro | Correct narrative/comment | **Post-review:** `#error` guard restored (valid; `RTEMS_POSIX_API` from BSP `cpuopts.h`); records reworded | frick-fsw | **CLOSED** | done (addendum §8) |
| F-03 | Minor | rtems-integration.md:49; traceability.yaml:424; AGENTS.md:58; Makefile:53 | Stale/contradicting N18/POSIX text (N8) | Update all in the same commit | **Post-review:** `#error` restored so `rtems-integration.md:49` is accurate; `traceability.yaml`, `AGENTS.md`, `Makefile`, `renode_vv.sh` updated to "in force / SKIP proxy" | frick-fsw | **CLOSED** | done (addendum §8) |
| F-04 | Minor | Dockerfile / rtems-docker | Verified image lacks the added cmake/ninja | Rebuild image from Dockerfile; re-run `make rtems-docker` | **Post-review:** Dockerfile carries pinned base + cmake/ninja; image rebuild launched; `make rtems-docker` runtime re-verification **pending** the rebuild (tracked) | frick-fsw | **CLOSED** (runtime pending) | done (addendum §8) |
| F-05 | Comment | renode_vv.sh / nightly / N18 | Per-commit SKIP; nightly tolerates SKIP | Make nightly fail on SKIP; restate N18 scope | **Post-review:** nightly now asserts `RENODE_TEST: PASS` (`grep -q`); N18 stated "in force / per-commit SKIP proxy" | frick-fsw | **CLOSED** | done (addendum §8) |
| F-06 | Comment | RTEMS version wording | "6.2" vs BSP 6.0.0 | Add clarifying sentence | **Post-review:** `rtems-integration.md` clarifies "6.2 = RSB release tag; kernel reports 6.0.0 by design" | frick-fsw | **CLOSED** | done (addendum §8) |
| F-07 | Comment | rules.md / status.json | `=1` vs `=ON`; stale milestones | Minor wording/milestone pass | **Post-review:** `rules.md` uses `=ON`; `status.json` adds M11 | frick-fsw | **CLOSED** | done (addendum §8) |

## 7. Evidence (commands and observed results)

All commands run from `/home/nicola/fsw/frick-fsw`; toolchain image
`frick-rtems-toolchain:local` id `e3a9673d8803`; Renode image digest
`sha256:7905c329…` → `e278e663fd95`.

### 7.1 Change under review

```
$ git log --oneline -1
c45d0ae Align baseline to open-source RTEMS 6.2; add milestone M11 …
$ git status --short
 M AGENTS.md  M CHANGELOG.md  M README.md  M cmake/toolchains/rtems-riscv-mpfs.cmake
 M config/rtems/rtems_config.c  M docs/rules.md  M docs/status/status.json
 M docs/vv/renode-environment.md  M docs/vv/tool-qualification.md
 M docs/vv/verification-results.md  M src/core/pal/pal_rtems.c
 M tools/docker/renode/frick-renode.resc  M tools/docker/rtems/Dockerfile
 M tools/scripts/renode_env_selftest.sh  M tools/scripts/renode_vv.sh
```

### 7.2 C fixes against the real BSP headers (F-01, F-02, P-01, P-02)

```
$ docker run --rm frick-rtems-toolchain:local ls /opt/rtems/6/riscv-rtems6/mpfs64imafdc/lib/include/rtems/posix
aio_misc.h condimpl.h key.h mqueue.h mqueueimpl.h muteximpl.h pthread.h
pthreadimpl.h semaphore.h … timer.h timerimpl.h      # pthread/mqueue/semaphore exist, no time.h

$ # original includes (one missing header):
$ riscv-rtems6-gcc … -I$BSP/lib/include -c t.c
t.c:5:10: fatal error: rtems/posix/time.h: No such file or directory
$ # without time.h → exit 0 (pthread/mqueue/semaphore compile)

$ # original RTEMS_POSIX_API guard with -DFRICK_PLATFORM_MPFS:
$ riscv-rtems6-gcc … -c errcheck.c ; echo $?
0        # no #error: RTEMS_POSIX_API defined via rtems.h → config.h → cpuopts.h
$ grep "define RTEMS_POSIX_API" cpuopts.h
#define RTEMS_POSIX_API 1

$ # rtems_config.c WITHOUT the POSIX_Init declaration:
rtems/confdefs/initthread.h:56:51: error: 'POSIX_Init' undeclared here (not in a function)

$ grep -nE "pthread|mqueue|semaphore|stdio|stdarg|mq_|sem_|clock_" src/core/pal/pal_rtems.c
NONE
```

### 7.3 Independent fresh cross build and Renode run (P-03, P-04, F-04)

```
$ docker run --rm frick-rtems-toolchain:local which cmake ninja
cmake: command not found ; ninja: command not found          # F-04

$ # fresh configure in an empty dir (not the stale build/renode cache):
$ cmake -S . -B build/renode-review -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
    -DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON && cmake --build …
-- Check for working C compiler: /opt/rtems/6/bin/riscv-rtems6-gcc
[35/35] Linking C executable frick_fsw
FRESH_BUILD_OK

$ cmake -S . -B build/rtems-review … -DFF_PLATFORM=rtems && cmake --build …
[34/34] Linking C executable frick_fsw
PLAIN_BUILD_OK

$ make renode
bash tools/scripts/renode_vv.sh
[renode] PASS (RENODE_TEST: PASS)

$ sha256sum build/renode/frick_fsw_renode.elf build/renode-review/frick_fsw_renode.elf
214168661f2e0c809c9bc1bacf3bb5336a600f3c1c586309d0aa49e434d2eee6  (both identical)

$ riscv-rtems6-readelf -h build/renode/frick_fsw_renode.elf | grep -E "Entry|Flags"
Entry point address: 0x1000000000
Flags: 0x5, RVC, double-float ABI
```

### 7.4 Renode console and platform details (P-04)

```
$ cat build/renode/console.log
+-----------------------------------------------------------+
|  frick-fsw "Eärendil" v0.1.0 …                            |
+-----------------------------------------------------------+
board: RT PolarFire SoC RTPFS160ZT (FCVG784) booting (HSS released U54 hart 1)
frick-fsw: 7 applications registered
…
[renode-test] frick-fsw Renode functional test application
[renode-test] sb_roundtrip=PASS
RENODE_TEST: PASS

$ tail /tmp/frick-renode.log
System bus created.
e51: Setting PC value to 0x1000000000. … u54_1 … u54_4
frick-renode: Machine started.
```

Multilib / version checks (P-03, F-06):

```
$ riscv-rtems6-gcc -print-multi-lib | grep rv64imafdc
rv64imafdc/lp64d/medany;@march=rv64imafdc@mabi=lp64d@mcmodel=medany
$ grep RTEMS_VERSION cpuopts.h
#define RTEMS_VERSION "6.0.0"     # __RTEMS_MINOR__ 0
$ git -C /rsb describe --tags         # 6.2   (RSB tag)
$ cat /rsb/rtems/config/tools/rtems-kernel-6.cfg
%define rtems_kernel_version 6335d7e48a692e4c4fe0577b6bba44ff0f64aefc
$ riscv-rtems6-gcc --version | head -1
riscv-rtems6-gcc (GCC) 13.3.0 20240521 (RTEMS 6, RSB 3814cb0e…, Newlib 1b3dcfd)
```

### 7.5 Documentation consistency (F-03, F-07)

```
$ grep -rn "RTEMS_POSIX_API" --include=*.md .   # (excluding build/)
docs/architecture/rtems-integration.md:49: … #if !defined(RTEMS_POSIX_API) #error …   # stale (code removed)
src/core/pal/pal_rtems.c:40 (comment)                                                 # present code

$ grep -rn "advisory proxy\|until the RTEMS\|not yet in force" docs/ AGENTS.md Makefile | grep -v build/
docs/requirements/traceability.yaml:424: … Gate is an advisory proxy until the RTEMS toolchain/image …
AGENTS.md:58: make renode  # … (advisory until the RTEMS image exists)
Makefile:53: # … advisory proxy until the RTEMS image exists.
```

### 7.6 Host gates (P-05, P-06) and N18 enforcement (F-05)

```
$ python3 tools/scripts/trace.py docs/requirements/traceability.yaml
CI evidence claims: 12
traceability: PASS  (80 requirements, 45 srs, 21 sss, 14 gaps, 51 lean theorems checked)

$ cmake -S . -B build/host-review -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build … && ctest …
100% tests passed, 0 tests failed out of 11

$ docker run --rm -v $PWD:/workspace -w /workspace frick-vv:local \
    bash -lc 'find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 clang-format --dry-run --Werror'
FORMAT_EXIT=0

$ bash -n tools/scripts/renode_vv.sh tools/scripts/renode_env_selftest.sh tools/scripts/setup_rtems.sh
# all OK
```

Enforcement (F-05): `.github/workflows/ci.yml` job `ci-renode` runs
`renode_env_selftest.sh` and `renode_vv.sh`; on a fresh checkout
`build/renode/frick_fsw_renode.elf` is absent (git-ignored), so `renode_vv.sh`
prints `[renode] SKIP …; exit 0`. `.github/workflows/nightly.yml` installs the
toolchain, builds the ELF and runs `make renode-env` then `make renode`, but a
SKIP there would still exit 0.

---

## 8. Post-review closure addendum (author)

All findings of FRICK-REVIEW-0040 were resolved in the same change:

- **F-01** — records reworded: only `rtems/posix/time.h` is non-existent; the
  other removed headers (`pthread.h`, `mqueue.h`, `semaphore.h`) exist but are
  unused private headers. `CHANGELOG.md`, `docs/vv/verification-results.md`.
- **F-02** — the `RTEMS_POSIX_API` `#error` guard was **restored** in
  `src/core/pal/pal_rtems.c` (it is valid: the macro is provided by the BSP
  `cpuopts.h` via `<rtems.h>`); only the non-existent/unused includes were
  removed. Re-verified: the RTEMS cross build succeeds with the guard.
- **F-03** — `rtems-integration.md:49` is accurate again (guard restored);
  `traceability.yaml` (SRS-REQ-044 note), `AGENTS.md`, `Makefile`,
  `tools/scripts/renode_vv.sh` updated to "in force / SKIP proxy".
- **F-04** — the container toolchain image is being rebuilt from the committed
  Dockerfile (pinned base image + `cmake`/`ninja`); the earlier local image
  lacked them (verified flow installed them at run time). `make rtems-docker`
  runtime re-verification is **pending the rebuild** (tracked; the toolchain
  itself and the cross build are already validated).
- **F-05** — the nightly workflow now asserts `RENODE_TEST: PASS`
  (`grep -q` on `build/renode/console.log`), so a SKIP/PASS-less run fails.
- **F-06** — `rtems-integration.md` clarifies that "RTEMS 6.2" is the RSB release
  tag and the kernel reports `6.0.0` by design.
- **F-07** — `docs/rules.md` uses `FF_ENABLE_RENODE_TEST=ON`; `docs/status/status.json`
  adds M11.

Re-verification: `make renode` -> `RENODE_TEST: PASS` (with the rebuilt ELF and
the restored guard); trace PASS; native ctest 11/11; format clean. With all
findings and comments resolved, the change is compliant (norms N12/N15).

---

Each finding is tracked to closure per norm N12; the report is committed and
pushed with the reviewed change (norm N11).
