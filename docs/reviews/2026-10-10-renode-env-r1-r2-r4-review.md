# Review Report — Renode V&V environment, norm N18, Renode test application

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0036                      |
| Reviewer session                  | independent agent, profile R1/R2/R4 (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | working tree on base commit `3048d87` (branch `main`, uncommitted) |
| Items under review                | `tools/docker/renode/Dockerfile` + `*.resc` + `renode_env_test.c`; `tools/scripts/renode_vv.sh`; `Makefile` (`renode`/`emulated`); `src/apps/app_renode_test.c` (+ `app.h`, `fsw_main.c`, `config/fsw_config.h`, `CMakeLists.txt`); `docs/rules.md` N18; `docs/vv/renode-environment.md`; `docs/requirements/{SRS.md,traceability.yaml,traceability.md}` SRS-REQ-044; `docs/status/status.json`; `docs/vv/tool-qualification.md` T17; CI `# evidence: ci-renode`; `README.md`/`AGENTS.md`; `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | (a) correctness of the Renode test app and its conditional registration; (b) honesty of the Renode-environment and advisory-proxy status; (c) whether N18 is stated and validly gated; (d) documentation completeness/accuracy (N8/N17) and counts vs the trace gate (N4). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N18 (see `docs/rules.md`). |
| Independence declaration          | Reviewer took no part in authoring, executing or fixing any item under review; fresh context; every finding is based solely on the repository state at the stated revision and on the commands run below. |
| Norms applied                     | N1-N18 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the new Renode V&V environment (pinned image + scripts + bare-metal
self-test), the advisory proxy `tools/scripts/renode_vv.sh`, the middleware test
application `app_renode_test.c` and its conditional registration, the new norm
**N18**, the V&V document `docs/vv/renode-environment.md`, requirement
**SRS-REQ-044** and its traceability, the status semaphore
(`renode_functional_validation`), tool-qualification **T17**, the CI evidence
marker `ci-renode`, and the README/AGENTS/CHANGELOG updates.

Excluded: the previously reviewed M10 emulated-target work (SRS-REQ-043, T15/T16,
`emulated-target.md`) except where the new change interacts with it; the Renode
image internals; the RTEMS cross-toolchain itself (not present in this
environment).

Assumption recorded: no RISC-V/RTEMS toolchain is available on the review host,
so the `.elf` path from `.resc` could not be executed end-to-end; the test app
was exercised on the **native** target for behavioural evidence.

### Commands run (evidence)

```
python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  -> PASS (80 reqs, 45 srs, 21 sss, 14 gaps, 51 lean theorems, 12 CI claims)

cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/native
  -> OK (no warnings); ctest -> 11/11 passed
./build/native/frick_fsw -> "frick-fsw: 6 applications registered"   (default = six apps)

cmake -S . -B build/rn -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-DFF_ENABLE_RENODE_TEST=1
cmake --build build/rn -> OK
./build/rn/frick_fsw -> 7 applications; "[renode-test] sb_roundtrip=FAIL"; "RENODE_TEST: FAIL"

bash -n tools/scripts/renode_vv.sh -> OK
bash tools/scripts/renode_vv.sh -> "[renode] SKIP: target ELF absent ..."; exit 0
find src config tests -name '*.[ch]' | xargs -0 -n1 clang-format --dry-run --Werror -> OK (exit 0)

docker run <pinned image> renode --disable-xwt --version -> Renode v1.16.1.16973
minimal .resc LoadPlatformDescription @platforms/cpus/polarfire-soc.repl -> "System bus created" (no error)
```

## 2. References

- Project norms: `docs/rules.md` (N1-N18)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity | Item / Requirement (profile) | Finding | Evidence | Recommendation |
| - | -------- | ---------------------------- | ------- | -------- | -------------- |
| F-01 | **Major** | `app_renode_test.c` SB round trip; SRS-REQ-044 / N18 (R1, R2) | The Software-Bus round-trip receiver is **dead code**: `renode_rx_task` is defined but never spawned, so `g_rx` never increments and the app always emits `RENODE_TEST: FAIL`. The documented `RENODE_TEST: PASS` marker is unreachable, and the N18 gate can never pass once made blocking. | `src/apps/app_renode_test.c:27-36` defines `static void renode_rx_task` (the only writer of `g_rx`, `:33`); `grep -rn renode_rx_task src/` matches only that definition; the registered entry `FF_AppRenodeTestMain` (`src/main/fsw_main.c:104`) subscribes (`:49`) and publishes (`:60`) but never calls `FF_SbReceive`; `g_rx==0` drives `ok=false` (`:64-66`) and prints `RENODE_TEST: FAIL` (`:77`). Reproduced: `./build/rn/frick_fsw` → `[renode-test] sb_roundtrip=FAIL` / `RENODE_TEST: FAIL`. | Spawn the receiver (`FF_PalTaskCreate` with `renode_rx_task`) or receive synchronously in the app before the check; add a host `ctest` covering the marker logic; re-qualify T17 (N10/N12). |
| F-02 | Minor | `CMakeLists.txt` / `app_renode_test.c` (R1) | The test-only application is compiled into `frick_fsw_core` **unconditionally**; only its *registration* is guarded, so the default/flight binary carries unreachable test code and an unused static function. No compiler warning is raised because `-Wall` is not enabled by default and the CI clang-tidy step is non-blocking. | `CMakeLists.txt:76` adds `app_renode_test.c` with no `FF_ENABLE_RENODE_TEST` guard; the guard exists only at `src/main/fsw_main.c:103-106`; `.github/workflows/ci.yml:93-101` runs clang-tidy with `|| true`. | Guard the source inclusion with the same option (or document the exclusion of the test module from flight builds); enable `-Wall -Wextra` on the core so dead code is caught. |
| F-03 | Minor | `frick-renode.resc`; N18 gate (R2) | The `.resc` used by the gate lacks the multi-hart boot handling that the self-test `.resc` already has, so the functional script is not in its final form; the V&V doc itself records this as outstanding. | `tools/docker/renode/frick-renode.resc` performs only `LoadELF` + `start`; `tools/docker/renode/renode_env.resc:18-23` halts cpu0/2/3/4 and sets `cpu1 PC 0x80000000`. `docs/vv/renode-environment.md:63-65` lists "(b) finalising the multi-hart boot in the `.resc`" as a prerequisite. | Apply the same boot-hart handling (or confirm Renode's default) to `frick-renode.resc`; re-run the proxy. |
| F-04 | Minor | `tools/scripts/renode_vv.sh`; `Makefile` `rtems`; `renode-environment.md` (R2, R4) | **No build path produces the ELF the proxy expects.** The documented command and the referenced "uniformity note" are inaccurate. | `tools/scripts/renode_vv.sh:20` expects `build/renode/frick_fsw_renode.elf`; `Makefile:74-77` (`rtems`) builds `build/rtems` and exposes no `CFLAGS_EXTRA` / `FF_ENABLE_RENODE_TEST` option; `docs/vv/renode-environment.md:48-50` shows `RTEMS_HOME=/opt/rtems make rtems CFLAGS_EXTRA=-DFF_ENABLE_RENODE_TEST=1` (marked "concept") and says "see uniformity note below", but `grep -rn uniformity docs/ tools/` has no other match. | Add a CMake option/target (or Make variable) that produces `build/renode/frick_fsw_renode.elf`, document the exact invocation, and delete/replace the dangling reference. |
| F-05 | Minor | `ci:renode`; SRS-REQ-044 evidence (R2) | The `ci:renode` evidence claim is satisfied by a CI step that always SKIPs (exit 0) in the absence of the RTEMS ELF; the traceability `tests:` field reads as executed evidence although nothing is executed on the middleware. A marker-absent run is indistinguishable from a pass at the CI level (advisory exit 0), so the future advisory→blocking switch is not implemented. | `.github/workflows/ci.yml:76-78` (and `.gitea/workflows/ci.yml:79-81`) run `renode_vv.sh`; `renode_vv.sh:26-29` prints SKIP/exit 0 when the ELF is absent and `:34-39` exits 0 even when the marker is missing; `traceability.yaml:422` maps `tests: ["ci:renode"]` for SRS-REQ-044. Advisory status is documented (`renode-environment.md:59-68`, `traceability.yaml:424`). | Mark the evidence as advisory/pending in the manifest until the marker is produced; make the script exit non-zero when the ELF **is** present but the marker is missing; re-run trace. |
| F-06 | Minor | `renode_env.resc`, `renode_env_test.c` (R2, R4) | The environment **self-test is orphaned and not reproducible from the repo**: no script or Make target invokes `renode_env.resc`, and there is no documented build command or linker script to produce `renode_env_test.elf` (the header comment names `clang --target=riscv64-unknown-elf` but no such tool/script exists). The independent checks confirmed the image runs and the platform loads, but the self-test path cannot be reproduced as committed. | `grep -rn renode_env tools/ Makefile` matches only the Dockerfile `COPY` (`Dockerfile:12-13`) and the default paths; `renode_env_test.c:6-7`; independent run: `renode --version` → v1.16.1.16973, `LoadPlatformDescription @platforms/cpus/polarfire-soc.repl` → "System bus created". | Add a build+run script (with the linker script) and document the exact command, so the "verified now" claim in `renode-environment.md §5` is reproducible (N8). |
| F-07 | Minor | `docs/vv/verification-results.md`; T17 (R2, R4) | The V&V records were **not updated for this change**, and T17's stated evidence (the PASS marker) is not demonstrated. | `docs/vv/verification-results.md` history ends at rev 0.13 (M10) with no Renode/N18 entry (`grep -n "renode\|N18"` → one M10 line); `docs/vv/tool-qualification.md:41` lists T17 evidence as "emulated functional `RENODE_TEST: PASS`", which F-01 makes unreachable. | Record the N18 cycle (including the SKIP/pending status) in verification-results and re-qualify T17 after F-01 (N11/N16/N10). |
| F-08 | Minor | Norm-range references (R4) | Adding N18 left several **current** documents enumerating the old norm set, so the documentation is internally inconsistent. | `docs/rules.md:1` title "Non-derogable Norms (Rules N1-N17)" while the doc defines N18 (rev 1.2); `README.md:269` "(N1-N17)" vs `README.md:20` "N1-N18"; `docs/development-plan.md:10,47`; `docs/engineering/independent-review.md:53`; `docs/reviews/review-template.md:13,22`. (Point-in-time historical review reports legitimately retain their old range.) | Update the current documents and the template to N1-N18 (N8/N16/N17). |
| F-09 | Minor | CHANGELOG / `renode-environment.md` accuracy (R4) | The documentation asserts a behaviour that the code does not produce, and the purpose statement overstates current capability. | `CHANGELOG.md:120-126` ("emits `RENODE_TEST: PASS`"), `docs/vv/renode-environment.md:37-40` (expected output block) and `docs/rules.md:293` ("gate passes when... produces the marker") contra F-01. `renode-environment.md:14-18` ("the same RTEMS binary ... is executed") is not qualified, although §5 is honest. | Fix F-01; then reconcile CHANGELOG and qualify §1 wording ("designed to run") to match the observed status. |
| F-10 | Comment | Entry-point completeness (R4) | The N8 entry points are not fully updated for the new tooling. | `AGENTS.md:31` (`tools/` row) does not list `renode_vv.sh` / `docker/renode/`, and the README Quickstart (`README.md:230-244`) does not mention `make renode` / `make emulated` although N18 is non-derogable. | Add the Renode tooling to the AGENTS repository map and the README quickstart. |
| F-11 | Comment | N18 non-derogability vs advisory enforcement (R1, R2) | N18 is declared **non-derogable**, yet currently enforced only as an advisory SKIP/exit-0 proxy; under the rules' own non-derogable clause, every change is literally non-compliant until the gate can actually run and pass. This is a governance tension separate from the code defect. | `docs/rules.md:12-16` ("none ... can be waived ... any change that does not satisfy all applicable norms is not compliant and must not be committed"); N18 at `:284-298`; enforcement summary `:321`. | State explicitly (enforcement summary / N18) that the gate is advisory and not yet in force, or record a governed derogation; note it as a tracked temporary state (N12). |

**Severity definition** — Major: defeats a requirement or a safety barrier. Minor:
must be corrected/justified. Comment: observation.

### Positive findings (verified)

- `trace.py` PASS: 80 reqs / 45 srs / 21 sss / 14 gaps / 51 theorems; counts in
  `README.md:73-77,248`, `docs/requirements/traceability.md:18`, `AGENTS.md` and
  the status semaphore are **consistent** with the gate.
- Default build and smoke unchanged: `frick-fsw: 6 applications registered`;
  `FF_ENABLE_RENODE_TEST=1` builds and registers 7 apps; native 11/11 tests;
  format gate clean; `bash -n` clean.
- `FF_ENABLE_RENODE_TEST` default is safely defined as 0 in `config/fsw_config.h:121-125`.
- The advisory proxy **message** is honest (SKIP wording, exit 0) and the V&V
  document §5 is honest about the RTEMS-toolchain prerequisite; the pinned image
  runs (v1.16.1.16973) and the `polarfire-soc` platform loads.
- N18 is stated as a general non-derogable rule and linked in SRS-REQ-044 /
  traceability / T17 / `ci:renode`; requirement counts match the trace gate.

## 4. Conclusion

Graded per profile (the change touches code, V&V tooling and documentation):

- **R1 — Software development: REJECTED (F-01 Major).** The test application
  cannot produce its marker; the functional-validation path is non-functional.
  Minor F-02 also applies.
- **R2 — Software V&V: REJECTED (F-01 Major).** The N18 / SRS-REQ-044 evidence
  cannot be produced as specified, and the advisory gate cannot transition to
  blocking with the current code; plus Minors F-03/F-04/F-05/F-07.
- **R4 — Software documentation: APPROVED WITH COMMENTS.** No Major in the
  documentation itself; Minors F-07/F-08/F-09 and Comments F-10/F-11 tracked.
  Note: R4 approval is contingent on the documentation corrections being made in
  the same cycle as the F-01 code fix (norm N8/N12/N17).

Overall (change): **REJECTED** — one Major open; commit/push blocked until F-01
is resolved and the deltas are re-reviewed (norms N7/N12).

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] REJECTED — one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent R1   | 2026-10-10 | REJECTED (F-01 Major) |
| R2 (N9)       | independent R2   | 2026-10-10 | REJECTED (F-01 Major) |
| R4 (N9/N17)   | independent R4   | 2026-10-10 | APPROVED WITH COMMENTS (contingent on F-01 cycle) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Major | F-01 — `app_renode_test.c` / N18 / SRS-REQ-044 | SB round-trip receiver never runs → always `RENODE_TEST: FAIL` | Start the receiver task (or receive synchronously); add a host ctest; re-qualify T17 | `./build/rn/frick_fsw` must print `RENODE_TEST: PASS`; ctest green | frick-fsw | **open** | required |
| NC-02 | Minor | F-02 — build wiring | Test app compiled unconditionally into the core | Guard source inclusion / document exclusion; enable `-Wall -Wextra` | Build log / CMake option evidence | frick-fsw | **open** | required |
| NC-03 | Minor | F-03 — `frick-renode.resc` | Gate `.resc` lacks multi-hart boot handling | Fix the `.resc` or justify the default | Updated `.resc` + re-run | frick-fsw | **open** | required |
| NC-04 | Minor | F-04 — build path | No target produces `build/renode/frick_fsw_renode.elf`; doc reference dangling | Add target/option; fix doc | Reproduced build + doc | frick-fsw | **open** | required |
| NC-05 | Minor | F-05 — `ci:renode` | Evidence claim satisfied by always-SKIP step; no blocking switch | Mark advisory/pending; implement non-zero on marker-absent | CI change + trace | frick-fsw | **open** | required |
| NC-06 | Minor | F-06 — self-test orphans | Self-test not reproducible (no script/linker script) | Add build+run script and docs | Reproducible self-test | frick-fsw | **open** | required |
| NC-07 | Minor | F-07 — V&V records / T17 | Records not updated; T17 evidence not demonstrated | Update verification-results; re-qualify T17 | Updated records | frick-fsw | **open** | required |
| NC-08 | Minor | F-08 — norm-range refs | Stale N1-N17 in current docs/template | Update to N1-N18 | Doc diff + R4 re-review | frick-fsw | **open** | required |
| NC-09 | Minor | F-09 — doc accuracy | CHANGELOG/§1 assert PASS behaviour not produced | Fix code; qualify wording | Reconciled docs | frick-fsw | **open** | required |
| NC-10 | Comment | F-10 — entry points | AGENTS/README miss Renode tooling | Add references | Doc diff | frick-fsw | **open** | optional |
| NC-11 | Comment | F-11 — N18 governance | Non-derogable vs advisory tension | State advisory status / derogation | Rules doc diff | frick-fsw | **open** | optional |

---

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer (R1/R2/R4, norm N9) | Initial independent review of the Renode V&V environment, norm N18, and the Renode test application |
