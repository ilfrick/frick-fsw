> **SUPERSEDED — do not review as the current candidate.** This file describes the
> 2026-11-04 gate-reconciliation candidate. It is retained unchanged as history
> (N11). The current candidate is
> `docs/vv/evidence/2026-11-04-re-review/CANDIDATE.md`. FRICK-REVIEW-0454 F-0454-03
> and FRICK-REVIEW-0455 PA-0455-02 found this file's live "T3 QUALIFIED" and
> `test_evs_hs_e2e.c`/60-check statements contradictory when carried inside the
> later bundle; the rows below are therefore annotated rather than left implicit.

# Candidate identity and review scope - 2026-11-04

This file fixes the exact content submitted for independent delta review. A
reviewer must verify these hashes before forming a verdict, and must report any
mismatch instead of reviewing a different tree.

## Candidate base

- Repository: `/home/nicola/fsw/frick-fsw`
- Branch: `main`, base commit `41dcbe21435e14b5aaff06c63d9df6f0de04df54`
- Everything below is uncommitted working-tree content on top of that commit.
- Manifest: `candidate-manifest.sha256` (relative paths, sha256).

## In scope (must be reviewed)

| Area | Path | Nature |
| --- | --- | --- |
| PAL test seam | `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` | test-only frozen-UTC seam under `FF_TEST_HOOKS`; no flight/RTEMS symbols |
| Test determinism | `tests/unit/test_pus.c`, `tests/unit/test_link_uart.c` | freeze UTC in HK due tests; PTY positive receive bounds 20 ms/500 ms -> 2 s |
| N22-TS5-47 test | `tests/unit/test_evs_hs_e2e.c` | direct `FF_PusEmitEvent` -> PUS S5 -> TM sink path, 60 checks. **Superseded:** became `tests/unit/test_evs_hs_health.c` (108 checks) in the re-review candidate. |
| NC-EXT-235 test | `tests/unit/test_frozen_utc_seam.c` | frozen-seam behaviour, 7 sub-tests |
| Test wiring | `tests/CMakeLists.txt` | registers `evs_hs_e2e` (label `n22`) and `frozen_utc_seam`; does **not** register `pus_mutation`. **Superseded:** the target is now `evs_hs_health`. |
| Traceability | `docs/requirements/traceability.yaml` | `test_evs_hs_e2e.c` linked to SRS-REQ-014, SRS-REQ-074, SSS-REQ-008 only. **Superseded:** the SRS-REQ-074 link was removed; the file is now `test_evs_hs_health.c`. |
| Test spec | `docs/vv/test-spec/ts-5-core-services.md` | N22-TS5-47 recorded as partial/gap |
| Tool register | `docs/vv/tool-qualification.md` | rev 0.3.108, T3 QUALIFIED **as recorded then; withdrawn 2026-11-04 — rev 1.3 says NOT FULLY QUALIFIED for the current candidate** |
| Time doc | `docs/architecture/time-and-timecode.md` | rev 0.4, frozen seam described as test-only |
| Gate record | `docs/vv/verification-results.md` | FRICK-VV-GATE-20261103 and FRICK-VV-GATE-20261104 |
| Plan/handover | `docs/planning/sprint-plan.md` (Rev 1.121), `docs/status/handover.md` (rev 3.60) | NC-EXT-235 status, figure corrections |
| Evidence | `docs/vv/evidence/2026-11-03-gate-diagnosis/`, `docs/vv/evidence/2026-11-04-gate-reconciliation/` | retained logs, exit codes, withdrawn test |

## Explicitly out of scope (pre-existing, unrelated, must not be reviewed as part of this delta)

These are known dirty-tree items that predate or are unrelated to this
candidate and are **excluded**:

- `tests/unit/test_pus_n7_adapter.c`, `tests/unit/test_pusopen.c`,
  `tests/unit/test_r2_gap.c`, `tests/unit/test_sched_max_entries.c`
  (pre-existing modifications);
- `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`;
- `third_party/n7s-obcp` (dirty submodule with build artifacts);
- `build/`, `build2/`, `build3/`, `.opencode/`, `opencode.json`, `tests/node/`,
  `tests/python/`, `tools/scripts/*`, `src/main/fsw_main_reqtest.c` and the
  other untracked files not listed above.

## Facts the reviewer must not assume

- No NC is closed by this candidate.
- No sprint is closed by this candidate.
- N18 remains FAIL (NC-EXT-198 / NC-PAL-01).
- NC-EXT-19 remains open; N22-TS5-47 is partial/gap, not "covered".
- Hardware and human reviews are deferred under NCs.
- No commit or push has been performed.

## Reproduction commands

All gates were run with original exit codes captured to files (never through
`tail` or `tee`):

```
# clean-room native build/test
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc "cmake -S . -B build/gate6 -G Ninja -DFF_BUILD_TESTS=ON"
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc "cmake --build build/gate6 -j 4"
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc "ctest --test-dir build/gate6 --output-on-failure"

# format, Lean
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc "make format-check"
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc "make verify-lean"

# host-side
python3 tools/scripts/trace.py docs/requirements/traceability.yaml
bash tools/scripts/cppcheck_scan.sh --out /tmp/cppcheck-review.txt

# RTEMS cross build
docker run --rm -v "$PWD":/workspace -w /workspace frick-rtems-toolchain:local \
  bash -lc 'cmake -S . -B build/rtems \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
    -DFF_PLATFORM=rtems -DFF_USE_PUS_ASN1=ON && \
    cmake --build build/rtems -j$(nproc)'
```
