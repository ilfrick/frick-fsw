# Review Report — Open-source RTEMS 6.2 alignment, containerised toolchain and CI/V&V wiring — R1/R2/R4

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0038                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | working tree on base commit `fcf2afd` (branch `main`, uncommitted) |
| Items under review                | `tools/scripts/setup_rtems.sh`; `tools/docker/rtems/Dockerfile`; `Makefile` (`rtems-toolchain`, `rtems-docker`, `rtems`); `.github/workflows/nightly.yml`; docs: `docs/adr/ADR-0002-rtems-baseline.md`, `docs/requirements/SSS.md`, `docs/quality/ecss-criticality-b.md`, `docs/glossary.md`, `docs/architecture/overview.md`, `docs/architecture/rtems-integration.md`, `docs/tooling.md`, `docs/vv/tool-qualification.md` (T10/T18), `docs/vv/verification-results.md` (rev 0.15), `docs/vv/renode-environment.md`, `config/platform/platform_mpfs.h`, `README.md`, `AGENTS.md`, `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Alignment to the open-source RTEMS 6.2 release (RTEMS project) and removal of vendor/commercial/"ECSS pre-qualified" wording; a reproducible containerised open-RTEMS toolchain and its CI/V&V wiring. Verified: (1) completeness of the vendor-reference removal; (2) correctness of the RSB bset names, the `--with-rtems-bsp` option, the `6.2` ref and the `riscv/mpfs64imafdc` BSP against upstream RTEMS; (3) honesty of the not-yet-executed cross/Renode validation records; (4) cross-document consistency (counts, revisions, norm/tool references, links); (5) the required gates. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1–N18 (`docs/rules.md`). |
| Independence declaration          | Reviewer took no part in authoring, fixing, executing or reviewing any item under review; fresh reviewer instance; every verdict is based solely on the current repository state and the commands recorded below. No repository file other than this report was created or modified. |
| Norms applied                     | N1–N18 (see `docs/rules.md`)           |

## 1. Scope

Full review of the working-tree change on base commit `fcf2afd` (17 modified
files + new `tools/docker/rtems/`). The change rewrites the RTEMS toolchain
bootstrap to the **open-source RTEMS project** release 6.2, adds a
containerised toolchain image and its Make targets, extends the nightly
workflow to build the Renode functional image and run the functional
validation, and removes vendor/commercial baseline wording from the current
documentation.

Excluded from scope: the provenance/licensing of the optional commercial
`PUSopen` PUS adapter (a separate, pre-existing boundary, out of this change's
subject); historical review reports under `docs/reviews/` (point-in-time
records); the internals of the pinned Renode image; and the previously reviewed
M10 emulated-target work except where it interacts with this change.

The RTEMS cross toolchain and the Renode functional end-to-end run are **not
executed** in this environment (no RTEMS image is built here): the toolchain
configuration is reviewed by inspection against upstream RTEMS/RSB sources, and
the proxy/gate behaviour is exercised locally. This limitation is stated
explicitly in each verdict that depends on it.

### Commands run (evidence)

```
grep -rniE "embedded[ -]?brains|pre-qualified|prequalified|esa/v6|ECSS pre|\bOAR\b|commercial baseline|vendor baseline" . \
  --exclude-dir=.git --exclude-dir=build --exclude-dir=third_party
  -> 7 matches in current docs/scripts (2 name "embedded-brains"); no match for pre-qualified/esa/v6/OAR

python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  -> traceability: PASS (80 requirements, 45 srs, 21 sss, 14 gaps, 51 lean theorems checked); 12 CI claims; exit 0

cmake -S . -B build/review-native -DCMAKE_BUILD_TYPE=Debug && cmake --build build/review-native -j$(nproc)
  -> OK; ctest --test-dir build/review-native -> 100% tests passed, 0 failed out of 11

bash -n tools/scripts/setup_rtems.sh            -> syntax OK

docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc 'find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 clang-format --dry-run --Werror'
  -> exit 0 (format clean)

make renode-env   -> [renode-env] PASS: Renode runs and the polarfire-soc platform loads
make renode       -> [renode] SKIP: target ELF absent (...); exit 0 (documented advisory proxy)
```

Upstream verification (RTEMS project, primary `gitlab.rtems.org`, project id 7):
`rtems/tools/rtems-source-builder` — tag **`6.2`** ("RTEMS 6.2 Release",
commit target `b3426b07…`) contains `rtems/config/6/rtems-riscv.bset` and
`rtems/config/6/rtems-kernel.bset`; the option `--with-rtems-bsp` is consumed
by `rtems/config/tools/rtems-kernel-common.cfg`. `rtems/rtems` (`bsps/riscv`)
and the RTEMS 6.2 BSP manual (`docs.rtems.org/docs/6.2/user/bsps/bsps-riscv.html`)
confirm `mpfs64imafdc` as the Microchip PolarFire SoC variant of the `riscv`
BSP (`[riscv/mpfs64imafdc]`).

## 2. References

- Project norms: `docs/rules.md` (N1–N18)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Tool register: `docs/vv/tool-qualification.md`
- Upstream: RTEMS Source Builder tag `6.2`; RTEMS 6.2 BSP manual (riscv);
  RTEMS `waf` install layout `<prefix>/<arch>-rtems<ver>/<bsp>`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| F-01 | **Major** | T18 / `tools/docker/rtems/Dockerfile` + `make rtems-docker` | The containerised toolchain installs to a path the CMake toolchain file does not use, so `make rtems-docker` cannot find the compiler. The Dockerfile builds with `--prefix=/opt/rtems/6` and sets `RTEMS_HOME=/opt/rtems`, while `cmake/toolchains/rtems-riscv-mpfs.cmake` resolves the compiler as `${RTEMS_HOME}/rtems/6/bin/riscv-rtems6-gcc` = `/opt/rtems/rtems/6/bin/…`. `tools/scripts/setup_rtems.sh` (and hence the nightly) installs to `${RTEMS_HOME}/rtems/6`, confirming the Dockerfile is the deviant. CMake fails when `CMAKE_C_COMPILER` is a non-existent absolute path, defeating the new containerised cross-build deliverable. | `tools/docker/rtems/Dockerfile:18` (`ARG RTEMS_PREFIX=/opt/rtems/6`), `:31–35` (`--prefix="${RTEMS_PREFIX}"`), `:37–38` (`ENV PATH=…${RTEMS_PREFIX}/bin`, `ENV RTEMS_HOME=/opt/rtems`); `cmake/toolchains/rtems-riscv-mpfs.cmake:18,26,29`; `tools/scripts/setup_rtems.sh:25,49,58`; `Makefile:89–91` (`rtems-docker` uses this toolchain file). | Set `ARG RTEMS_PREFIX=/opt/rtems/rtems/6` (and `PATH` accordingly) so the image matches `setup_rtems.sh` and the toolchain file; or, if the `/opt/rtems/6` layout is intended, change `RTEMS_HOME`/the toolchain file consistently. Re-run `make rtems-docker` to evidence the fix. |
| F-02 | **Minor** | N8 / N17 completeness | The required completeness grep still matches in **current** docs/scripts/code. Two current files name the vendor explicitly ("embedded-brains"); five further matches are negated disclaimers ("no vendor/commercial baseline"). The literal check "no match in current docs" therefore fails. | `docs/adr/ADR-0002-rtems-baseline.md:17`; `CHANGELOG.md:135`; `tools/scripts/setup_rtems.sh:7`; `tools/docker/rtems/Dockerfile:5`; `docs/adr/ADR-0002-rtems-baseline.md:37`; `docs/architecture/rtems-integration.md:54`; `docs/vv/verification-results.md:29`. No matches for `pre-qualified`/`prequalified`/`esa/v6`/`ECSS pre`/`\bOAR\b`. | Remove the vendor name (`No vendor (embedded-brains) baseline` → `No vendor or commercial baseline`) and, if a strictly clean grep is required, reword the remaining disclaimers so the banned tokens do not appear even in negations. |
| F-03 | **Minor** | N6/N10 / RTEMS integration (verify) | The CMake sysroot path is suspicious and compounds F-01: it uses `${RTEMS_HOME}/rtems/6/riscv/mpfs64imafdc`, but the RTEMS `waf` install layout is `<prefix>/<arch>-rtems<ver>/<bsp>` (e.g. `sparc-rtems7/erc32`), i.e. `…/6/riscv-rtems6/mpfs64imafdc`. The file is unchanged by this change (pre-existing), but it is now on the critical path of the new toolchain wiring and no real cross build has exercised it. | `cmake/toolchains/rtems-riscv-mpfs.cmake:26,37`; RTEMS 6.2 quick-start install output (`<prefix>/sparc-rtems7/erc32/…`). | Verify against a real toolchain install; if confirmed, use `riscv-rtems6/mpfs64imafdc` for `CMAKE_SYSROOT`/`CMAKE_FIND_ROOT_PATH` (and evidence it in `verification-results.md`). |
| F-04 | **Minor** | N8 (documentation accuracy) | `docs/vv/renode-environment.md` states the nightly builds the Renode image "from the reproducible open RTEMS image (`tools/docker/rtems`)", but the nightly installs the toolchain with `setup_rtems.sh` on the runner (host install), not from the container image. | `docs/vv/renode-environment.md:54–55`; `.github/workflows/nightly.yml:29–32` (`bash tools/scripts/setup_rtems.sh`). | Reword to "the nightly workflow installs the open RTEMS 6.2 toolchain via `setup_rtems.sh`". |
| F-05 | **Comment** | N8 (repo map) | `AGENTS.md` repository map was not updated to include the new `tools/docker/rtems/` directory it now documents elsewhere. | `AGENTS.md:34` (lists `docker/`, `docker/renode/` only); new dir `tools/docker/rtems/`. | Add `docker/rtems/` to the `tools/` map row. |
| F-06 | **Comment** | N16 consistency | ADR-0002 gains a `## History` table, but `docs/rules.md` interpretation note 3 explicitly says ADRs record evolution through `Status`/`Date` and that a separate `## History` table is **not** duplicated there. | `docs/rules.md:350–356`; `docs/adr/ADR-0002-rtems-baseline.md:32–37`. | Either drop the ADR history table or justify/update note 3 as a deliberate convention change. |
| F-07 | **Comment** | N8 accuracy | `6.2` is a **tag** (`RTEMS 6.2 Release`), not a branch, on the primary GitLab repo; the ADR calls it a "release branch". `git clone --branch 6.2` still works (git accepts a tag), so this is wording only. | GitLab API: branches = `4.9,4.10,4.11,5,6,6-update-dtc-hash,main`; tags search `6.2` → release tag. `docs/adr/ADR-0002-rtems-baseline.md:15`. | Say "release tag/ref `6.2`". |
| F-08 | **Comment** | N6/N10 reproducibility | The "reproducible" containerised toolchain is not pinned: base image `ubuntu:24.04` is a mutable tag and RSB ref `6.2` is a tag without a commit digest (contrast the Renode image, pinned by `sha256`). | `tools/docker/rtems/Dockerfile:14,16`. | Pin the base image by digest (`ubuntu:24.04@sha256:…`) and optionally the RSB commit for bit-reproducibility. |
| F-09 | **Comment** | tool usability | The usage line documents `[--jobs N]`, but the script reads the jobs value from `$1`, so `./tools/scripts/setup_rtems.sh --jobs 8` would pass `--jobs=--jobs`. Pre-existing, but inside the rewritten script and part of its "consistency". | `tools/scripts/setup_rtems.sh:13` (usage) and `:27` (`JOBS="${1:-$(nproc)}"`). | Parse `--jobs N`, or fix the usage line to a positional argument. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Positive verification (no finding)

- **Completeness of the RTEMS wording change:** `pre-qualified`, `prequalified`,
  `esa/v6`, `ECSS pre` and `\bOAR\b` no longer occur anywhere; the vendor name
  survives only in the two negations of F-02.
- **Correctness of bsets/option/BSP:** `6/rtems-riscv.bset` and
  `6/rtems-kernel.bset` exist at ref `6.2`; `--with-rtems-bsp=<bsp>` is the
  correct RSB option (consumed by `rtems-kernel-common.cfg`); `riscv/mpfs64imafdc`
  is the correct RTEMS 6.2 Microchip PolarFire SoC BSP variant. The changed
  step 2 (kernel bset instead of repeating the tools bset) is a genuine fix.
  The task brief's `riscv/mpfs64imafdec` is a typo; the repository correctly
  uses `riscv/mpfs64imafdc`.
- **Honesty of the records:** the new `verification-results.md` row marks the
  RTEMS cross build as **nightly** (not a pass) and does not claim a Renode
  functional pass; `docs/vv/renode-environment.md` §5 and
  `docs/status/status.json` (`renode_functional_validation: yellow`) keep the
  gate advisory. Locally reproduced: `make renode-env` PASS (environment only,
  no toolchain) and `make renode` SKIP/exit 0 (image absent) — both as
  documented, no overclaim.
- **Consistency:** counts (80/45/21/14/51, 12 CI claims) match `trace.py`;
  revisions/history rows in `docs/tooling.md` (0.5), `tool-qualification.md`
  (0.2.5) and `verification-results.md` (0.15) are self-consistent; the
  `renode-environment.md` header revision (0.3) now matches its history (0.1,
  0.2, 0.3, fixing a pre-existing header/history skew); tool register T10
  updated and T18 added correctly; all internal links in the changed docs
  resolve.
- **Gates:** `trace.py` PASS; native build + ctest 11/11; `bash -n` OK;
  clang-format gate clean in `frick-vv:local`; `git diff --check` clean.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

**Per profile**

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (SW dev) | **REJECTED** | F-01 (Major): the new containerised toolchain wiring cannot produce a working cross build (`make rtems-docker`). Open Minor/Comment: F-03, F-08, F-09. |
| R2 (SW V&V) | **REJECTED** | F-01 (Major) breaks the T18 tool/evidence path; F-03 (sysroot) and F-08 (pinning) must be verified before the cross-build gate can be trusted. Open Minor/Comment: F-03, F-08. |
| R4 (SW documentation) | **APPROVED WITH COMMENTS** | No Major documentation finding. Open Minor/Comment: F-02, F-04, F-05, F-06, F-07. |

**Overall:** **REJECTED** — one Major finding (F-01) remains open; per norm N12
a REJECTED review blocks commit/push (norm N7) until the Major is fixed and the
deltas are re-reviewed. The documentation set is otherwise close
(APPROVED WITH COMMENTS) and the alignment itself is correct.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R4 (N9) | independent reviewer agent (FRICK-REVIEW-0038) | 2026-10-10 | R1 REJECTED · R2 REJECTED · R4 APPROVED WITH COMMENTS · overall REJECTED |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Major | F-01 / T18 / `make rtems-docker` | Dockerfile prefix ≠ CMake toolchain path (`/opt/rtems/6` vs `${RTEMS_HOME}/rtems/6`) | Make the Dockerfile install to `/opt/rtems/rtems/6` (or align `RTEMS_HOME`/toolchain file); re-run `make rtems-docker` | `docker build … && make rtems-docker` PASS; image `riscv-rtems6-gcc` found by CMake | frick-fsw | open | required |
| NC-02 | Minor | F-02 / N8/N17 completeness | Vendor tokens remain in current docs/scripts (2 name "embedded-brains") | Remove the vendor name; reword disclaimers; re-run the required grep to zero | grep returns no match in current docs/scripts/code | frick-fsw | open | required |
| NC-03 | Minor | F-03 / RTEMS integration | `CMAKE_SYSROOT` uses `riscv/` instead of `riscv-rtems6/` | Verify against a real install; correct if confirmed | Real cross build finds BSP headers/libs | frick-fsw | open | required |
| NC-04 | Minor | F-04 / N8 accuracy | Doc says nightly uses the containerised image; it uses `setup_rtems.sh` | Reword `renode-environment.md` §4 | Doc matches `nightly.yml` | frick-fsw | open | required |
| NC-05 | Comment | F-05 / N8 | `AGENTS.md` map lacks `tools/docker/rtems/` | Add it | Map current | frick-fsw | open | optional |
| NC-06 | Comment | F-06 / N16 | ADR-0002 history table vs rules note 3 | Drop table or justify note 3 | Consistent convention | frick-fsw | open | optional |
| NC-07 | Comment | F-07 / N8 | Docs call `6.2` a "release branch"; it is a tag | Reword to "release tag/ref `6.2`" | Wording accurate | frick-fsw | open | optional |
| NC-08 | Comment | F-08 / N6/N10 | Base image/RSB ref not pinned | Pin base image digest | `ubuntu:24.04@sha256:…` | frick-fsw | open | optional |
| NC-09 | Comment | F-09 / usability | Usage `[--jobs N]` vs `$1` | Parse the option or fix usage | `--jobs N` works | frick-fsw | open | optional |

---

Committed and pushed with the reviewed change (norm N11).
