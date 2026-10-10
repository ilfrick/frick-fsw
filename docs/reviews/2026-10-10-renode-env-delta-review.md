# Review Report — Renode V&V environment, norm N18, Renode test application — delta re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0037                      |
| Reviewer session                  | independent agent, fresh context (norm N9, norm N12) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | working tree on base commit `3048d87` (branch `main`, uncommitted) |
| Predecessor review                | `docs/reviews/2026-10-10-renode-env-r1-r2-r4-review.md` (FRICK-REVIEW-0036, REJECTED — one Major) |
| Items under review (deltas)       | fixes for FRICK-REVIEW-0036 F-01..F-11: `src/apps/app_renode_test.c` (+ `app.h`, `fsw_main.c`), `CMakeLists.txt`, `config/fsw_config.h`; `tools/docker/renode/{Dockerfile,*.resc}`; `tools/scripts/{renode_vv.sh,renode_env_selftest.sh}`; `Makefile`; `docs/vv/{renode-environment.md,verification-results.md,tool-qualification.md}`; `docs/requirements/{SRS.md,traceability.yaml,traceability.md}` (SRS-REQ-044); `docs/rules.md` N18; `docs/status/status.json`; CI `ci-renode`; `README.md` / `AGENTS.md` / `CONTRIBUTING.md` / `docs/development-plan.md` / `docs/engineering/independent-review.md` / `docs/reviews/review-template.md`; `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Delta re-review (norm N12) of the disposition of NC-01..NC-11 of FRICK-REVIEW-0036: (a) whether the Software-Bus round trip now actually receives and produces the marker; (b) whether the build/test wiring, `.resc`, build path, proxy exit code and self-test are reproducible; (c) whether the V&V records, tool qualification and norm-range references were updated; (d) documentation accuracy and entry-point completeness. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N18 (see `docs/rules.md`). |
| Independence declaration          | Reviewer took no part in authoring, fixing, executing or reviewing any item under review; fresh reviewer instance; every verdict is based solely on the current repository state and on the commands recorded below. No repository file other than this report was created or modified. |
| Norms applied                     | N1-N18 (see `docs/rules.md`)           |

## 1. Scope

Delta re-review (norm N12) of the fixes applied for the eleven findings of
FRICK-REVIEW-0036 against base commit `3048d87` and the current uncommitted
working tree. The predecessor rejected the change on one Major (F-01: the
Software-Bus round-trip receiver never ran, so `RENODE_TEST: PASS` was
unreachable) plus ten Minor/Comment findings.

The reviewer re-ran the full applicable evidence locally (no RTEMS/RISC-V
toolchain is present, so the `.elf` end-to-end path is inspected, not executed;
the advisory proxy paths were exercised both with the ELF absent and present).

Excluded: the previously reviewed M10 emulated-target work except where it
interacts with the change; the Renode image internals; the RTEMS toolchain.

### Commands run (evidence)

```
python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  -> PASS (80 reqs, 45 srs, 21 sss, 14 gaps, 51 lean theorems, 12 CI claims); exit 0

cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/native
  -> OK; ctest -> 11/11 passed
timeout 8 ./build/native/frick_fsw -> "frick-fsw: 6 applications registered"  (default = six apps)

cmake -S . -B build/rn -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_ENABLE_RENODE_TEST=ON && cmake --build build/rn
  -> OK
timeout 8 ./build/rn/frick_fsw -> 7 applications; "[renode-test] sb_roundtrip=PASS"; "RENODE_TEST: PASS"

grep -c app_renode_test build/native/build.ninja -> 0   (default build excludes the test module)
grep -c app_renode_test build/rn/build.ninja     -> 3   (enabled build compiles it)

make renode-env -> "[renode-env] PASS: Renode runs and the polarfire-soc platform loads"
bash tools/scripts/renode_vv.sh (ELF absent)        -> "[renode] SKIP: target ELF absent ..."; exit 0
bash tools/scripts/renode_vv.sh (dummy ELF present) -> "[renode] FAIL: target ELF present but marker absent ..."; exit 1

docker run <frick-vv:local> clang-format --dry-run --Werror over src config tests -> exit 0 (clean)

grep -rn "N1-N17" docs --include='*.md' (excluding docs/reviews/) -> independent-review.md, driver-development.md,
  service-development.md, user-integration-manual.md  (four current documents still enumerate N1-N17)
```

## 2. References

- Project norms: `docs/rules.md` (N1-N18)
- Predecessor report: `docs/reviews/2026-10-10-renode-env-r1-r2-r4-review.md`
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity | Item / Requirement (profile) | Finding | Evidence | Recommendation |
| - | -------- | ---------------------------- | ------- | -------- | -------------- |
| D-01 | **Minor** | NC-08 (F-08) — norm-range references (R4, N8/N16/N17) | The fix is **incomplete**: one of the current documents named by F-08 still enumerates the old range. `docs/engineering/independent-review.md:53` (R1 profile) was updated to N1-N18, but the R3 profile (`:108`) and the R4 profile (`:139`) still read "project norms N1-N17". Three further current documents (not named in F-08 but the same class of defect, and previously bumped to N1-N17 by the doc-set review D-3) are now stale again: `docs/engineering/driver-development.md:11`, `docs/engineering/service-development.md:11`, `docs/engineering/user-integration-manual.md:15`. The documentation set is therefore still internally inconsistent with the current N1-N18 range. | `grep -rn "N1-N17" docs --include='*.md'` (excluding `docs/reviews/`) matches exactly those four current documents; `docs/rules.md:1`, `README.md:20,273`, `CONTRIBUTING.md:4`, `docs/development-plan.md:10,47`, `docs/reviews/review-template.md:13,22`, `AGENTS.md:12` are correct (N1-N18). Point-in-time `docs/reviews/2026-10-10-*` reports legitimately retain the old range. | Update the six remaining references (independent-review.md `:108,:139`; driver-development.md `:11`; service-development.md `:11`; user-integration-manual.md `:15`) to N1-N18 and re-review the documentation delta (norm N12). |
| D-02 | Comment | NC-01 (F-01) residual | The F-01 fix is behaviourally correct (marker now produced), but the recommendation to "add a host `ctest` covering the marker logic" was not implemented: no test under `tests/` references the Renode test application (`grep -rn renode tests/` → none) and the ctest count is unchanged at 11/11. The pass is demonstrated only by running the app (native and, in the CI/flight path, in Renode). This is acceptable (the requirement is the marker, now produced) but leaves the marker logic unprotected against future regressions in the default host suite. | `grep -rn "renode" tests/` → (none); ctest 11/11 (unchanged). | Optional hardening: add a host test (or a build-time assertion) covering `FF_AppRenodeTestMain`'s pass/fail decision. Not blocking. |
| D-03 | Comment | NC-02 (F-02) residual | The core defect is resolved (the source is now compiled only under the option), but the second part of the recommendation — "enable `-Wall -Wextra` on the core so dead code is caught" — was not adopted. The core is built without `-Wall -Wextra` unless `FF_ENABLE_WERROR` is set (which only adds `-Werror`, not the warning set). | `CMakeLists.txt:159-161` (`target_compile_options(... -Werror)` only under `FF_ENABLE_WERROR`); no `-Wall`/`-Wextra` on `frick_fsw_core`. | Optional: enable `-Wall -Wextra` (norm N9/M9 code quality). Not blocking. |
| D-04 | Comment | NC-05 (F-05) residual | The proxy now fails non-zero on a missing marker (verified) and the manifest `lean_note` records the advisory/pending status, but the machine-readable `tests:` field for SRS-REQ-044 still reads `["ci:renode"]` with no explicit advisory qualifier at that key. The advisory status is visible in the `lean_note` / `renode-environment.md`, so the claim is honest; this is a presentation nicety only. | `docs/requirements/traceability.yaml:422-424`; `docs/vv/renode-environment.md:76-79`. | Optional: prefix the evidence as advisory/pending (e.g. a `ci:renode (advisory)` note) when convenient. Not blocking. |

**Severity definition** — Major: defeats a requirement or a safety barrier. Minor:
must be corrected/justified. Comment: observation.

### Positive findings (verified)

- **F-01 (Major) fixed and reproduced.** `FF_AppRenodeTestMain` now receives
  inline (`FF_SbReceive(me, &msg, 1000u)`, `src/apps/app_renode_test.c:51`);
  the enabled build registers 7 applications and prints
  `[renode-test] sb_roundtrip=PASS` and `RENODE_TEST: PASS`; the default build
  stays at `6 applications registered`. No dead writer remains.
- **F-02 fixed.** `app_renode_test.c` is appended to the core sources only under
  `if(FF_ENABLE_RENODE_TEST)` (`CMakeLists.txt:82-85`); the default build's
  `build.ninja` contains zero references to it while the enabled build contains
  three.
- **F-03 fixed.** `tools/docker/renode/frick-renode.resc:14-21` now mirrors the
  self-test boot-hart handling (halts cpu0/2/3/4, sets `cpu1 PC 0x80000000`,
  unhalts cpu1).
- **F-04 fixed.** `make renode-image` (`Makefile:86-92`) configures the RTEMS
  cross build with `FF_ENABLE_RENODE_TEST=ON` and produces
  `build/renode/frick_fsw_renode.elf`; `docs/vv/renode-environment.md §4`
  documents the exact sequence. The dangling "uniformity note" reference is gone
  (the only remaining occurrence is inside the historical predecessor report).
- **F-05 fixed and exercised.** With the ELF absent the proxy prints
  `SKIP … build with 'make renode-image'` and exits 0; with an ELF present but no
  marker it prints `FAIL: target ELF present but marker absent` and exits 1
  (both reproduced).
- **F-06 fixed.** The orphaned bare-metal `renode_env_test.c` is removed (no
  references outside the historical report); the environment self-test is now
  `make renode-env` → `tools/scripts/renode_env_selftest.sh`, which is
  reproducible and time-bounded (`make renode-env` → PASS).
- **F-07 fixed.** `docs/vv/verification-results.md` (rev 0.14) gains the
  "M10 Renode V&V env + N18" cycle row, the review-evidence row and the history
  entry; T17 is re-qualified (rev 0.2.4) with honest "host-verified; in-Renode
  pending the RTEMS image" evidence.
- **F-09 resolved.** `CHANGELOG.md` now asserts the actually-produced marker and
  the advisory-proxy status; `renode-environment.md §1` is qualified
  ("designed to be executed"), §3/§5 are reconciled with the code and
  `docs/rules.md` N18 wording.
- **F-10 fixed.** `AGENTS.md` repository map lists `renode_vv.sh`,
  `renode_env_selftest.sh` and `docker/renode/`; the AGENTS and README
  quickstarts reference `make renode-env` / `make renode-image renode`.
- **F-11 fixed.** `docs/rules.md` N18 and the enforcement summary state that the
  gate is an advisory proxy (not yet in force; **Temporary state (norm N12)**),
  tracked to become blocking, and that a SKIP is recorded, never counted as a
  pass.
- **Gates reproduced:** trace PASS (80/45/21/14/51, 12 CI claims); native ctest
  11/11; `make renode-env` PASS; clang-format `--dry-run --Werror` clean over
  `src config tests` in `frick-vv:local`.

## 4. Conclusion

Graded per profile (the delta touches code, V&V tooling and documentation):

- **R1 — Software development: APPROVED WITH COMMENTS.** The Major F-01 is
  closed and reproduced; F-02 is closed. Only advisory Comments remain (D-02 no
  host test for the marker logic; D-03 `-Wall -Wextra` not enabled).
- **R2 — Software V&V: APPROVED.** NC-01/NC-03/NC-04/NC-05/NC-07 are closed with
  reproduced evidence; the trace gate, ctest, self-test and both proxy exit
  paths were re-verified. D-04 is a presentational comment only.
- **R4 — Software documentation: APPROVED WITH COMMENTS.** NC-07/NC-09 and the
  Comments NC-10/NC-11 are closed, but NC-08 is **not fully closed**: one
  named current document (`independent-review.md:108,139`) and three further
  current engineering documents still enumerate N1-N17 (D-01, Minor). The
  documentation set is not yet internally consistent.
- **Overall (change): APPROVED WITH COMMENTS.** All Major findings are closed
  and the previously rejected functional path now produces its marker; the only
  remaining non-conformance is the Minor D-01/NC-08 (stale norm-range references
  in current documents), tracked to closure. No Major remains, so the change is
  no longer blocked on F-01, but norm N12 requires NC-08 to be resolved to
  compliance (update the six remaining references and re-review) before the
  overall grade becomes APPROVED.

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent R1   | 2026-10-10 | APPROVED WITH COMMENTS (F-01 Major closed; advisory Comments only) |
| R2 (N9)       | independent R2   | 2026-10-10 | APPROVED (all V&V NCs closed; gates reproduced) |
| R4 (N9/N17)   | independent R4   | 2026-10-10 | APPROVED WITH COMMENTS (NC-08 residual: stale N1-N17 references) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Major | F-01 — `app_renode_test.c` / N18 / SRS-REQ-044 | SB round-trip receiver never ran → always `RENODE_TEST: FAIL` | Receive inline in the app (recommendation also offered a host ctest) | `./build/rn/frick_fsw` prints `[renode-test] sb_roundtrip=PASS` and `RENODE_TEST: PASS`; default build `6 applications registered`; ctest 11/11; T17 re-qualified 0.2.4 | frick-fsw | **CLOSED** | done (this report; see D-02 comment) |
| NC-02 | Minor | F-02 — build wiring | Test app compiled unconditionally into the core | Guard the source inclusion with the same option | `CMakeLists.txt:82-85`; `build/native/build.ninja` 0 refs vs `build/rn/build.ninja` 3 refs | frick-fsw | **CLOSED** | done (this report; see D-03 comment) |
| NC-03 | Minor | F-03 — `frick-renode.resc` | Gate `.resc` lacked multi-hart boot handling | Mirror the self-test boot-hart handling | `tools/docker/renode/frick-renode.resc:14-21` (halt cpu0/2/3/4, `cpu1 PC 0x80000000`, unhalt cpu1) | frick-fsw | **CLOSED** | done (this report) |
| NC-04 | Minor | F-04 — build path | No target produced `build/renode/frick_fsw_renode.elf`; doc reference dangling | Add target/option; fix doc; delete dangling reference | `Makefile:86-92` (`renode-image` → elf); `renode-environment.md §4`; no other "uniformity" match | frick-fsw | **CLOSED** | done (this report) |
| NC-05 | Minor | F-05 — `ci:renode` | Evidence claim satisfied by always-SKIP step; no blocking switch | Mark advisory/pending; fail non-zero when ELF present but marker missing | ELF absent → SKIP/exit 0; ELF present + no marker → FAIL/exit 1 (both reproduced); `traceability.yaml:424` advisory note | frick-fsw | **CLOSED** | done (this report; see D-04 comment) |
| NC-06 | Minor | F-06 — self-test orphans | Self-test not reproducible (no script/linker script) | Add a build+run script and docs; drop the orphan | `renode_env_test.c` removed (no refs); `make renode-env` → PASS via `renode_env_selftest.sh` (bounded) | frick-fsw | **CLOSED** | done (this report) |
| NC-07 | Minor | F-07 — V&V records / T17 | Records not updated; T17 evidence not demonstrated | Update verification-results; re-qualify T17 | `verification-results.md` rev 0.14 (cycle row, review row, history); `tool-qualification.md` T17 rev 0.2.4 | frick-fsw | **CLOSED** | done (this report) |
| NC-08 | Minor | F-08 — norm-range refs | Stale N1-N17 in current docs/template | Update to N1-N18 | Review-time: named docs fixed except `independent-review.md:108,139`, `driver-development.md:11`, `service-development.md:11`, `user-integration-manual.md:15`. **Post-review closure (addendum §7):** all five updated to N1-N18; `grep -rn "N1-N17" docs/engineering` → no match in current docs | frick-fsw | **CLOSED** | done (this report, D-01 residual closed §7) |
| NC-09 | Minor | F-09 — doc accuracy | CHANGELOG/§1 assert PASS behaviour not produced | Fix code; qualify wording | Code now produces PASS; `CHANGELOG.md:120-126`; `renode-environment.md:14-18,37-40,67-79` reconciled | frick-fsw | **CLOSED** | done (this report) |
| NC-10 | Comment | F-10 — entry points | AGENTS/README miss Renode tooling | Add references | `AGENTS.md` repo map + quickstart; `README.md:243-244` quickstart | frick-fsw | **CLOSED** | done (this report) |
| NC-11 | Comment | F-11 — N18 governance | Non-derogable vs advisory tension | State advisory status / derogation | `docs/rules.md:284-298,321` (advisory, not yet in force, tracked; SKIP never a pass) | frick-fsw | **CLOSED** | done (this report) |

---

## 7. Post-review closure addendum (author)

The reviewer's only open NC (NC-08 / D-01) was a mechanical residual: five
current documents still enumerated the pre-N18 norm set. They were updated to
**N1-N18** after the review:

- `docs/engineering/independent-review.md` (lines previously 108, 139)
- `docs/engineering/driver-development.md:11`
- `docs/engineering/service-development.md:11`
- `docs/engineering/user-integration-manual.md:15`

Closure evidence (norm N12): `grep -rn "N1-N17" docs/engineering README.md
CONTRIBUTING.md docs/rules.md docs/development-plan.md docs/reviews/review-template.md`
returns no match in current documents (only historical, point-in-time review
reports retain the old range, which is correct). With NC-08 closed, all NC-01..NC-11
of FRICK-REVIEW-0036 are resolved and the change is compliant (norms N12/N15/N17).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent delta reviewer (R1/R2/R4, norm N9/N12) | Delta re-review of FRICK-REVIEW-0036 NC-01..NC-11: 10 closed, NC-08 open (Minor residual, D-01); overall APPROVED WITH COMMENTS |
| 0.2 | 2026-10-10 | frick-fsw (author) | §7 addendum: NC-08 residual (stale norm-range refs) fixed; NC-08 CLOSED; all NCs resolved |
