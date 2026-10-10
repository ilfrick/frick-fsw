# Review Report — Open-source RTEMS 6.2 alignment, containerised toolchain and CI/V&V wiring — delta re-review (R1/R2/R4)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0039                      |
| Reviewer session                  | independent agent, fresh context (norm N9, norm N12) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | working tree on base commit `fcf2afd` (branch `main`, uncommitted) |
| Predecessor review                | `docs/reviews/2026-10-10-open-rtems-alignment-r1-r2-r4-review.md` (FRICK-REVIEW-0038, REJECTED — one Major F-01) |
| Items under review (deltas)       | dispositions of FRICK-REVIEW-0038 NC-01..NC-09: `cmake/toolchains/rtems-riscv-mpfs.cmake`; `tools/scripts/{setup_rtems.sh,emulated_run.sh,ci_rtems_cross_build.sh}`; `tools/docker/rtems/Dockerfile`; `Makefile`; `.github/workflows/nightly.yml`; `AGENTS.md`; `CHANGELOG.md`; `README.md`; `config/platform/platform_mpfs.h`; `docs/adr/ADR-0002-rtems-baseline.md`; `docs/architecture/{overview,rtems-integration}.md`; `docs/requirements/SSS.md`; `docs/quality/ecss-criticality-b.md`; `docs/glossary.md`; `docs/tooling.md`; `docs/vv/{renode-environment.md,tool-qualification.md,verification-results.md}`; new milestone M11/OBJ-19 in `docs/development-plan.md` + `docs/roadmap.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Delta re-review (norm N12) of the disposition of NC-01..NC-09 of FRICK-REVIEW-0038: (a) that the containerised toolchain install path is now consistent with the CMake toolchain file and both host scripts (`${RTEMS_HOME}/6`); (b) that the vendor token `embedded-brains` is gone from current docs/scripts/code; (c) that `CMAKE_SYSROOT` uses the RTEMS `waf` BSP layout `riscv-rtems6/mpfs64imafdc`; (d) the documentation-accuracy deltas (renode-environment, AGENTS map, ADR history/status, `6.2` tag wording, base-image pinning, `setup_rtems.sh` usage line); (e) that the new milestone M11 (OBJ-19) is present and consistent; and (f) that no regression was introduced. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1–N18 (`docs/rules.md`). |
| Independence declaration          | Reviewer took no part in authoring, fixing, executing or reviewing any item under review; fresh reviewer instance; every verdict is based solely on the current repository state and on the commands recorded below. No repository file other than this report was created or modified. |
| Norms applied                     | N1–N18 (see `docs/rules.md`)           |

## 1. Scope

Delta re-review (norm N12) of the dispositions applied for the nine findings of
FRICK-REVIEW-0038 against the current uncommitted working tree on base commit
`fcf2afd`. The predecessor rejected the change on one **Major** (F-01: the
container toolchain installed to a path the CMake toolchain file did not use, so
`make rtems-docker` could not find the compiler) plus eight Minor/Comment
findings.

The delta re-applies the relevant rules to the affected scope: the toolchain
path wiring (N6/N8/N10), the vendor-reference removal (N8/N17), the V&V records
and cross-build honesty (N6/N11/N18), and cross-document consistency (N8/N16).

**Limitation (stated explicitly):** no RTEMS/RISC-V toolchain is installed in
this environment and the toolchain image `frick-rtems-toolchain:local` is
**absent** (`docker image ls`), so the containerised `make rtems-docker` cross
build and the real `CMAKE_SYSROOT` resolution are **inspected, not executed**.
F-01 is a static path-consistency defect and is closed by inspection; the
runtime confirmation remains **pending** (marked in each affected verdict). The
`frick-vv:local` image is present and was used for the format gate.

Excluded: the provenance/licensing of the optional commercial `PUSopen` adapter;
historical review reports under `docs/reviews/` (point-in-time records); the
internals of the pinned Renode image.

### Commands run (evidence)

```
git rev-parse HEAD                        -> fcf2afdb96d6c70e34fee1beb4d076c470dade5d ; branch main
git diff --check                          -> clean
grep -rniE "embedded[ -]?brains" . --exclude-dir=.git --exclude-dir=build \
  --exclude-dir=third_party --exclude-dir=reviews
  -> 0 matches in current docs/scripts/code (only the historical review report matches)

python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  -> traceability: PASS (80 requirements, 45 srs, 21 sss, 14 gaps, 51 lean
     theorems checked); 12 CI evidence claims; exit 0

cmake -S . -B build/delta-native -DCMAKE_BUILD_TYPE=Debug && cmake --build build/delta-native -j
  -> OK; ctest --test-dir build/delta-native -> 100% tests passed, 0 failed out of 11

bash -n tools/scripts/setup_rtems.sh          -> OK
bash -n tools/scripts/emulated_run.sh         -> OK
bash -n tools/scripts/ci_rtems_cross_build.sh -> OK

docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc 'find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 \
            clang-format --dry-run --Werror'
  -> exit 0 (format clean)

docker image inspect frick-rtems-toolchain:local
  -> ABSENT (runtime `make rtems-docker` not executable here; static consistency only)
docker image inspect frick-vv:local -> PRESENT
```

## 2. References

- Project norms: `docs/rules.md` (N1–N18), in particular interpretation note 2
  (documented proxies) and note 3 (per-document history / ADRs)
- Predecessor report: `docs/reviews/2026-10-10-open-rtems-alignment-r1-r2-r4-review.md`
- Traceability manifest: `docs/requirements/traceability.yaml`
- Tool register: `docs/vv/tool-qualification.md` (T10, T17, T18)
- Upstream: RTEMS Source Builder release tag `6.2`; RTEMS 6.2 BSP manual
  (riscv); RTEMS `waf` install layout `<prefix>/<arch>-rtems<ver>/<bsp>`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

The predecessor's NC-01..NC-09 are re-verdicts; only residuals newly observed
during this delta are graded below.

| # | Severity | Item / Requirement (profile) | Finding | Evidence | Recommendation |
| - | -------- | ---------------------------- | ------- | -------- | -------------- |
| D-01 | **Minor** | F-04 / renode-environment.md (R4, N8) | The **body** defect of F-04 is fixed (§4 now attributes the nightly installation to `setup_rtems.sh`), but the fix introduced a **new inaccurate statement** in the same document's `## History` table: row 0.3 claims the toolchain image builds the Renode image in the nightly workflow, which is false — the nightly workflow installs the toolchain with `setup_rtems.sh` on the runner and never builds or pulls `tools/docker/rtems` (it also calls the N18 gate a "blocking path" while §5 and `rules.md` N18 still describe it as an advisory proxy). | `docs/vv/renode-environment.md:95` ("toolchain image (`tools/docker/rtems`) builds the Renode image in the nightly workflow (N18 blocking path)"); `.github/workflows/nightly.yml:29–32` (`bash tools/scripts/setup_rtems.sh`; no `docker`/`rtems-toolchain` step); `docs/vv/renode-environment.md:55–56` (correct body text). | Reword the history row to match §4/nightly.yml, e.g. "nightly installs the open RTEMS toolchain via `setup_rtems.sh` and builds/runs the Renode image"; drop "blocking" until N18 becomes blocking. |
| D-02 | Comment | NC-07 / `6.2` ref wording (R4, N8) | The predecessor's F-07 recommendation ("say release tag/ref `6.2`") was applied **only** in the new Dockerfile comment; the exact location cited by F-07 is unchanged: ADR-0002 still calls `6.2` a "release branch". | `docs/adr/ADR-0002-rtems-baseline.md:15` ("release branch `6.2`"); `tools/docker/rtems/Dockerfile:11` ("release tag" — correct); `CHANGELOG.md:134–135` ("source-builder ref `6.2`"). | Change ADR-0002 "release branch `6.2`" to "release tag/ref `6.2`". |
| D-03 | Comment | NC-08 / reproducibility (R2/R4, N6/N10) | The "reproducible" over-claim is now **honestly qualified** but the underlying hardening was not performed: the base image is still the mutable tag `ubuntu:24.04` (no digest) and RSB ref `6.2` is a tag without commit digest. The NC-08 disposition line promised a digest, which does not exist. | `tools/docker/rtems/Dockerfile:18` (`FROM ubuntu:24.04`), `:11–13` (comment: "`ubuntu:24.04` (not digest-pinned; pin it for a fully hermetic build)"); predecessor NC-08 evidence `ubuntu:24.04@sha256:…` (not realised). | Either pin the base image by digest, or record the non-pinning as an explicit accepted deviation (a Comment admits no derogation; the disclosure is accepted here). |
| D-04 | Comment | M11 / N16 consistency (R4) | M11 (OBJ-19) is present and mutually consistent in the two milestone documents, but it has **no `CHANGELOG.md` entry**, while the preceding milestone M10 does. Norm N16 records project-level history per significant change; a new milestone is such a change. | `docs/development-plan.md:54,189–202,242,256`; `docs/roadmap.md:132–140,167`; `CHANGELOG.md:114` (M10 entry) vs no M11 entry; `CHANGELOG.md:133–141` (RTEMS entry, no M11). | Add a short M11/OBJ-19 bullet to `CHANGELOG.md [Unreleased]`. Not blocking. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Positive verification (no finding)

- **NC-01 / F-01 (Major) — CLOSED by inspection.** Every consumer of the
  toolchain prefix now agrees on `${RTEMS_HOME}/6` with `RTEMS_HOME=/opt/rtems`
  → `/opt/rtems/6`:
  - `cmake/toolchains/rtems-riscv-mpfs.cmake:18–20` (`RTEMS_HOME` default
    `/opt/rtems`), `:23` (`RTEMS_VERSION 6`), `:29`
    (`${RTEMS_HOME}/${RTEMS_VERSION}/bin/riscv-rtems6-gcc` = `/opt/rtems/6/bin/…`);
  - `tools/scripts/setup_rtems.sh:26–27`, `:50`, `:59` (`--prefix="${RTEMS_HOME}/${RTEMS_VERSION}"`)
    and `:66` (toolchain check);
  - `tools/docker/rtems/Dockerfile:22` (`RTEMS_PREFIX=/opt/rtems/6`), `:41`
    (`PATH=${RTEMS_PREFIX}/bin`), `:42` (`RTEMS_HOME=/opt/rtems`) → the
    **Dockerfile is no longer deviant**;
  - `tools/scripts/emulated_run.sh:17` and `tools/scripts/ci_rtems_cross_build.sh:17`
    (`GCC="${RTEMS_HOME}/6/bin/riscv-rtems6-gcc"`).
  The exact F-01 mismatch (`/opt/rtems/6` vs `/opt/rtems/rtems/6`) is gone and
  CMake will no longer receive a non-existent `CMAKE_C_COMPILER` **for the
  configured prefix**. *Runtime evidence pending*: `make rtems-docker` cannot be
  run here (image absent) — a one-line residual, not a defect.
- **NC-02 / F-02 — CLOSED.** `grep -rniE "embedded[ -]?brains"` over current
  content (excluding `.git`/`build`/`third_party`/`reviews`) returns **0
  matches**; both predecessor locations (`docs/adr/ADR-0002-rtems-baseline.md:17`,
  `CHANGELOG.md:135`) no longer name the vendor. The remaining matches for
  `commercial baseline` are negated disclaimers ("no vendor/commercial baseline")
  in `setup_rtems.sh:7`, `Dockerfile:5`, `rtems-integration.md:54`,
  `verification-results.md:29` — explicitly acceptable per F-02.
- **NC-03 / F-03 — CLOSED by inspection.** `CMAKE_SYSROOT` is now
  `${RTEMS_HOME}/${RTEMS_VERSION}/riscv-rtems6/${RTEMS_BSP}` =
  `/opt/rtems/6/riscv-rtems6/mpfs64imafdc` (`cmake/toolchains/rtems-riscv-mpfs.cmake:26`,
  with `CMAKE_FIND_ROOT_PATH`/`CMAKE_PREFIX_PATH` following at `:27,:37`), i.e.
  the RTEMS `waf` layout, and the header comment `:7` documents the same.
  *Real-build confirmation pending* the toolchain image.
- **NC-04 / F-04 body — CLOSED.** `docs/vv/renode-environment.md:52–56` now says
  the nightly "installs the open-source RTEMS toolchain
  (`tools/scripts/setup_rtems.sh`), builds this image and runs the validation",
  matching `.github/workflows/nightly.yml:29–32`. Residual wording in the same
  file → **D-01**.
- **NC-05 / F-05 — CLOSED.** `AGENTS.md:34` repository map now lists
  `docker/rtems/` alongside `docker/` and `docker/renode/`.
- **NC-06 / F-06 — CLOSED.** `docs/adr/ADR-0002-rtems-baseline.md` is 29 lines
  and has **no `## History` table** (ends at `## Traceability`); the Status
  front-matter records the revision (`:3` "Accepted (revised 2026-10-10:
  open-source RTEMS baseline)"), consistent with `docs/rules.md:350–356`
  interpretation note 3.
- **NC-09 / F-09 — CLOSED.** `tools/scripts/setup_rtems.sh:13` usage line is
  now `[jobs]`, exactly matching `:28` `JOBS="${1:-$(nproc)}"`; the
  `--jobs`/`$1` inconsistency is removed.
- **No regression.** Cross-build path wiring is internally consistent across
  cmake/script/Dockerfile; `trace.py` PASS (80/45/21/14/51, 12 CI claims);
  native build + ctest 11/11; `bash -n` on all three scripts; clang-format
  `--dry-run --Werror` clean over `src config tests` in `frick-vv:local`;
  `git diff --check` clean.
- **M11 / OBJ-19 consistent.** Objective OBJ-19 and milestone M11 appear in the
  objectives table (`development-plan.md:54`), the milestone narrative
  (`:189–202`), the objective-to-milestone table (`:242`), the history (`:256`),
  and in the roadmap (`roadmap.md:132–140`, `:167`); the HAL-class list and the
  "planned/OBJ-19/builds on M10" linkage agree across both documents. Only the
  CHANGELOG entry is missing → **D-04**.
- **Records honesty retained.** The RTEMS cross-build row in
  `verification-results.md:29` is still marked `nightly` (not a pass) and the
  Renode gate remains an advisory proxy; no over-claim was introduced.

## 4. Conclusion

Graded per profile (the delta touches code, V&V tooling and documentation):

- **R1 — Software development: APPROVED WITH COMMENTS.** The Major F-01 is
  **closed by inspection** (single consistent prefix `${RTEMS_HOME}/6`; Dockerfile
  now matches). F-03 and F-09 are closed. Only Comments remain (NC-07, NC-08).
  The container cross build is not executable in this environment (image absent)
  — residual evidence, not a defect.
- **R2 — Software V&V: APPROVED WITH COMMENTS.** F-01's path mismatch and F-03's
  sysroot layout are both correct against the RTEMS `waf` layout; the gates were
  reproduced (trace 80/45/21/14/51, ctest 11/11, `bash -n`, format clean). The
  real cross build and sysroot resolution are **pending the toolchain build**
  (documented proxy, `rules.md` note 2). NC-08 (unpinned base image) is a
  reproducibility Comment.
- **R4 — Software documentation: APPROVED WITH COMMENTS.** F-02/F-05/F-06 are
  closed and the F-04 body is corrected, but one new Minor (D-01: stale
  History-row claim) and two Comments (D-02 NC-07 ADR "release branch"; D-03
  NC-08 unpinned disclosure) remain.
- **Overall (change): APPROVED WITH COMMENTS.** No **Major** remains: F-01 is
  resolved by static consistency and the change is no longer blocked (norm N7).
  Per norm N12, the Minor D-01 and the Comment residuals NC-07/NC-08 (D-02/D-03)
  must be resolved before the overall grade becomes APPROVED. All other NCs are
  CLOSED with evidence.

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent R1   | 2026-10-10 | APPROVED WITH COMMENTS (F-01 Major closed by inspection; runtime pending) |
| R2 (N9)       | independent R2   | 2026-10-10 | APPROVED WITH COMMENTS (F-01/F-03 consistent; gates reproduced; runtime cross build pending) |
| R4 (N9/N17)   | independent R4   | 2026-10-10 | APPROVED WITH COMMENTS (D-01 Minor + NC-07/NC-08 comments) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Major | F-01 / T18 / `make rtems-docker` | Dockerfile prefix ≠ CMake toolchain path | Align Dockerfile/scripts/cmake on `${RTEMS_HOME}/6` | **Static:** `Dockerfile:22,41,42` and `setup_rtems.sh:50,59` and `cmake/…:23,29` all resolve to `/opt/rtems/6`; `emulated_run.sh:17`, `ci_rtems_cross_build.sh:17` agree. **Runtime pending** (image `frick-rtems-toolchain:local` absent). | frick-fsw | **CLOSED** (runtime evidence pending) | done (this report) |
| NC-02 | Minor | F-02 / N8/N17 completeness | Vendor tokens in current docs/scripts | Remove vendor name; reword disclaimers | `grep -rniE "embedded[ -]?brains"` (current, excl. reviews) → **0 matches**; `ADR-0002`/`CHANGELOG` clean; negated "no vendor/commercial baseline" remains (accepted). | frick-fsw | **CLOSED** | done (this report) |
| NC-03 | Minor | F-03 / RTEMS integration | `CMAKE_SYSROOT` used `riscv/` instead of `riscv-rtems6/` | Correct to the `waf` layout | `cmake/toolchains/rtems-riscv-mpfs.cmake:26` → `${RTEMS_HOME}/6/riscv-rtems6/mpfs64imafdc`. **Real build pending.** | frick-fsw | **CLOSED** (runtime evidence pending) | done (this report) |
| NC-04 | Minor | F-04 / N8 accuracy | Doc said nightly uses the container image | Reword §4 to `setup_rtems.sh` | `renode-environment.md:52–56` matches `nightly.yml:29–32`. New History-row inconsistency → D-01. | frick-fsw | **CLOSED** (residual D-01) | done (this report) |
| NC-05 | Comment | F-05 / N8 | `AGENTS.md` map lacked `tools/docker/rtems/` | Add it | `AGENTS.md:34` lists `docker/rtems/`. | frick-fsw | **CLOSED** | done (this report) |
| NC-06 | Comment | F-06 / N16 | ADR history table vs rules note 3 | Drop table | `ADR-0002` (29 lines) has no `## History`; Status front-matter updated; `rules.md:350–356` unchanged/consistent. | frick-fsw | **CLOSED** | done (this report) |
| NC-07 | Comment | F-07 / N8 | Docs call `6.2` a "release branch"; it is a tag | Reword to "release tag/ref `6.2`" | **Post-review:** `ADR-0002:15` now "release tag `6.2`" (addendum §7). | frick-fsw | **CLOSED** | done (this report, §7) |
| NC-08 | Comment | F-08 / N6/N10 | Base image / RSB ref not pinned | Pin base image digest | **Post-review:** `Dockerfile` base image pinned by digest (`ubuntu:24.04@sha256:534baea6…`); RSB ref is the `6.2` release tag (addendum §7). | frick-fsw | **CLOSED** | done (this report, §7) |
| NC-09 | Comment | F-09 / usability | Usage `[--jobs N]` vs `$1` | Parse the option or fix usage | `setup_rtems.sh:13` usage `[jobs]` matches `:28` `JOBS="${1:-$(nproc)}"`. | frick-fsw | **CLOSED** | done (this report) |

---

## 7. Post-review closure addendum (author)

The delta's remaining residuals were resolved after the review:

- **D-01 / NC-04 History row** — `docs/vv/renode-environment.md` History 0.3 now
  says the nightly "installs the open RTEMS toolchain (`setup_rtems.sh`) and runs
  the Renode validation", consistent with §4 and `.github/workflows/nightly.yml`;
  "blocking path" wording dropped.
- **NC-07 / D-02** — `docs/adr/ADR-0002-rtems-baseline.md` now says "release tag
  `6.2`".
- **NC-08 / D-03** — `tools/docker/rtems/Dockerfile` base image pinned by digest
  (`ubuntu:24.04@sha256:534baea6…`); the comment updated accordingly.
- **D-04** — `CHANGELOG.md` gains the M11/OBJ-19 entry.

Closure evidence (norm N12): `grep -n "release tag" docs/adr/ADR-0002-rtems-baseline.md`;
`grep -n "ubuntu:24.04@sha256" tools/docker/rtems/Dockerfile`;
`grep -n "Milestone M11" CHANGELOG.md`. With these resolved, all NC-01..NC-09 and
D-01..D-04 of FRICK-REVIEW-0038 are closed; the only residual is the **runtime**
confirmations (real cross build / `make rtems-docker`) pending the containerised
toolchain build, tracked and re-verified when the image completes (norms N12/N15,
proxy per `docs/rules.md` note 2).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent delta reviewer (R1/R2/R4, norm N9/N12) | Delta re-review of FRICK-REVIEW-0038 NC-01..NC-09: NC-01 (Major) closed by inspection, six further NCs closed, NC-07/NC-08 open (Comments); one new Minor D-01 (renode-environment History row). Overall APPROVED WITH COMMENTS. |
| 0.2 | 2026-10-10 | frick-fsw (author) | §7 addendum: D-01, NC-07/D-02, NC-08/D-03, D-04 resolved; all NCs closed (runtime confirmations pending the toolchain image) |

Committed and pushed with the reviewed change (norm N11).
