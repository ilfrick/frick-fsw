# Candidate identity and review scope - 2026-11-04 (re-review)

This file fixes the exact content submitted for independent re-review after
FRICK-REVIEW-0452 (V&V), FRICK-REVIEW-0453 (PA), FRICK-REVIEW-0454 (V&V) and
FRICK-REVIEW-0455 (PA) returned **NEEDS REVISION**.
A reviewer must verify these hashes before forming a verdict, and must report a
mismatch instead of reviewing a different tree.

## Candidate base

- Repository: `/home/nicola/fsw/frick-fsw`
- Branch: `main`, base commit `41dcbe21435e14b5aaff06c63d9df6f0de04df54`
- Everything below is uncommitted working-tree content on top of that commit.
- Manifest: `candidate-manifest.sha256` (relative paths, sha256, no self-entry).

## In scope (must be reviewed)

| Area | Path | Nature |
| --- | --- | --- |
| PAL test seam | `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` | compile-time test-guarded frozen-UTC seam under `FF_TEST_HOOKS`; absent from RTEMS/flight |
| Test determinism | `tests/unit/test_pus.c` (sha256 `4ca0e2d0…`), `tests/unit/test_link_uart.c` (`c7307c7d…`) | frozen UTC in HK due tests; PTY positive receive bounds 2 s |
| N22-TS5-47 test | `tests/unit/test_evs_hs_health.c` (`23535db3…6035`, 110 checks) | direct `FF_PusEmitEvent` → PUS S5 → injected TM sink → loopback-link path, severity→subservice oracle, out-of-bound case; **does not** exercise the production sink `FF_AppTmOutputInit`/`ff_to_sink` end to end (NC-EXT-236) nor `event_id` identity (NC-EXT-237). Listener → sink is covered by `test_r3_gap.c:523-575`. |
| NC-EXT-235 test | `tests/unit/test_frozen_utc_seam.c` (`8660eb77…`) | frozen-seam behaviour, 7 sub-tests, 4325 checks |
| Test wiring | `tests/CMakeLists.txt` | registers `evs_hs_health` (label `n22`) and `frozen_utc_seam`; does **not** register `pus_mutation` |
| Traceability | `docs/requirements/traceability.yaml` | `test_evs_hs_health.c` linked to SRS-REQ-014 and SSS-REQ-008 only |
| Test spec | `docs/vv/test-spec/ts-5-core-services.md` | N22-TS5-47 recorded as partial with NC-EXT-236/237, 110/110 check count and script hash |
| Tool register | `docs/vv/tool-qualification.md` | rev 1.5, **T3 NOT FULLY QUALIFIED** for this candidate (status unchanged from rev 1.3; rev 1.4/1.5 only re-bind the script hashes) |
| Time doc | `docs/architecture/time-and-timecode.md` | rev 0.5, compile-time-guarded seam wording |
| Gate record | `docs/vv/verification-results.md` | FRICK-VV-GATE-20261104 plus this re-review cycle; authoritative gate directory is `build/gate11` |
| Plan/handover | `docs/planning/sprint-plan.md` (Rev 1.125), `docs/status/handover.md` (rev 3.64) | NC-EXT-235/236/237 status, figure corrections |
| Review index/corrections | `docs/reviews/README.md`; errata in `docs/reviews/FRICK-REVIEW-0451-PA-NC-EXT-235-REVIEW.md` and `docs/reviews/FRICK-REVIEW-0449-PA-N22-TS5-47-FINAL.md` | index rows 0439–0459; 0451 erratum §14; 0449 erratum + scope correction (SRS-REQ-074) |
| Licence headers | `tools/scripts/fsw_opencode_event_bridge.py`, `tests/python/test_fsw_opencode_event_bridge.py` | SPDX header added only; no functional change (N13 gate) |
| Evidence | `docs/vv/evidence/2026-11-04-re-review/`, `docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256` + `README.md` | retained logs, exit codes, RTEMS symbol evidence, `sha256sum -c` repair |

## Manifest-only (in the manifest, out of review scope)

The files below are bound by the manifest so that base commit + manifest
reconstructs the tested configuration (FRICK-REVIEW-0453 PA-10, FRICK-REVIEW-0455
PA-0455-05 / PA-O-02). None is part of the delta and none must be reviewed as if
it were.

Pre-existing formatting-only test modifications (pure clang-format reflows,
verified hunk by hunk, first recorded by FRICK-REVIEW-0451):

- `tests/unit/test_pus_n7_adapter.c`
- `tests/unit/test_pusopen.c`
- `tests/unit/test_r2_gap.c`
- `tests/unit/test_sched_max_entries.c`

Untracked gate inputs scanned by `make gsw-selfcheck` / `license_check.py`
(all 12 untracked `.py` files, plus the 6 untracked `.sh`/`.resc` files):

- `tests/python/test_atomic_publish.py`, `test_fsw_opencode_event_bridge.py`,
  `test_qemu_runner_failclosed.py`, `test_qemu_uart_runner_failclosed.py`,
  `test_renode_vv_verdict_failclosed.py`, `test_target_req_verify.py`,
  `test_target_run_verdict.py`, `test_uart_tm_verify.py`
- `tools/scripts/uart_tm_verify.py`, `xw3_negative_controls.py`, `xw3_verdict.py`
- `tools/scripts/qemu_req_vv.sh`, `qemu_uart_tc_vv.sh`, `qemu_xw3_vv.sh`,
  `renode_xw2_vv.sh`
- `tools/docker/renode/frick-xw2-nowrap-renode.resc`,
  `tools/docker/renode/frick-xw2-renode.resc`

Other bound but out-of-scope items:

- `tools/scripts/fsw_opencode_event_bridge_plugin_core.mjs` (untracked helper,
  not scanned by the licence gate — `.mjs` is outside its managed suffixes);
- review records bound as superseded basis, not as product content:
  `docs/reviews/FRICK-REVIEW-0439-VV-NC-EXT-235.md`,
  `FRICK-REVIEW-0440-PA-NC-EXT-235.md`,
  `FRICK-REVIEW-0442-PA-NC-EXT-235-REVIEW.md`,
  `FRICK-REVIEW-0444-VV-N22-TS5-47.md`,
  `FRICK-REVIEW-0445-VV-N22-TS5-47-REVISION.md`,
  `FRICK-REVIEW-0446-PA-N22-TS5-47.md`,
  `FRICK-REVIEW-0447-PA-N22-TS5-47-REVIEW.md`,
  `FRICK-REVIEW-0448-PA-N22-TS5-47-FINAL.md`,
  `FRICK-REVIEW-0449-PA-N22-TS5-47-FINAL.md`,
  `FRICK-REVIEW-0454-VV-20261104-REREVIEW.md`,
  `FRICK-REVIEW-0455-PA-20261104-REREVIEW.md`.

### Single classification of the evidence-mapping document

`docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md` is **excluded from the
manifest** (`candidate-manifest.sha256`) but **IS present in the commit** as the
base-commit version (sha256 `9215342f…d3ba12`), unchanged from `41dcbe2`. The
working tree carries a pre-existing 28-insertion/13-deletion content modification
(mtime `2026-11-03 07:32:15 +0100`, sha256 `c94bca27…30fadb`) that pre-dates
this delta, is unrelated to it (NC-EXT-47 mapping work), and is **uncommitted**.
The working-tree modification is neither formatting-only nor reviewed by any
report in this bundle. FRICK-REVIEW-0457 §6.9 and FRICK-REVIEW-0459 residual
item 5 require the file to be **reviewed or reverted before any commit that
includes it**. This commit *includes the base version* (it was already tracked);
the requirement was not met by this commit. The disposition (review, or owner-
authorised reversion of the working-tree change) remains open.

Because the manifest must describe the committed candidate, the file is excluded
from the reconstruction set. Its two states are recorded for auditability:

| State | sha256 |
| --- | --- |
| committed (base `41dcbe2`, tracked, unmodified) | `9215342f…d3ba12` |
| working tree (pre-existing unreviewed modification, uncommitted) | `c94bca27…30fadb` |

No gate consumes this file — it is a `.md` document, not compiled, not under
`src/` (so neither `license_check.py` nor `cppcheck_scan.sh` reads it), and no
requirement, test or telemetry claim depends on it. It affects no gate result
and no product claim. This paragraph is the single classification of that file.

## Explicitly out of scope (pre-existing, unrelated)

- `src/main/fsw_main_reqtest.c` — **bound in the manifest** (manifest-only input,
  not product content) but **not compiled**: it contains a hard `#error` unless
  `FF_ENABLE_TARGET_REQ_TEST=1`, that macro appears nowhere in the build system,
  and `CMakeLists.txt`/`*.cmake` do not reference the file. It nevertheless
  participates in two gate input sets: `tools/scripts/license_check.py`
  enumerates `src/**` managed suffixes (it currently passes: SPDX tag at line
  27) and `tools/scripts/cppcheck_scan.sh` scans `/workspace/src`. It is bound
  precisely so that base commit + manifest reconstructs those gate inputs; it
  cannot change a build, CTest or execution result because it is never linked;
- `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md` — see the single
  classification above (excluded from the manifest; committed as base version
  `9215342f…d3ba12`; working-tree modification `c94bca27…30fadb` preserved
  unreviewed and uncommitted);
- `third_party/n7s-obcp` (dirty submodule);
- `build/`, `build2/`, `build3/`, `.opencode/`, `opencode.json`, `tests/node/`,
  other untracked helper scripts and the other untracked files not listed above.

## Facts the reviewer must not assume

- No NC is closed by this candidate.
- No sprint is closed by this candidate.
- The hash of the **current** manifest is deliberately not written into any manifested
  file (that would be circular); the reviewer must compute it from
  `candidate-manifest.sha256`. Hashes of *earlier* candidate manifests do appear in
  this bundle, where they identify the state a review or a superseded package
  referred to; they are historical identifiers, not a self-reference.
- N18 remains FAIL (NC-EXT-198 / NC-PAL-01).
- NC-EXT-19/236/237 remain open; N22-TS5-47 is partial, not "covered".
- NC-EXT-236 is the absence of an end-to-end oracle through the **production** sink
  (`FF_AppTmOutputInit`/`ff_to_sink`), not an unimplemented forwarding leg.
- T3 is **NOT FULLY QUALIFIED** for this candidate.
- Hardware and human reviews are deferred under NCs.
- No commit or push has been performed.

## Reproduction commands

All gates were run with original exit codes captured to files (never through
`tail` or `tee`):

```
# clean-room native build/test inside the controlled image
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc \
  'cmake -S . -B /workspace/build/gate11 -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON'
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc \
  'cmake --build /workspace/build/gate11'
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc \
  'ctest --test-dir /workspace/build/gate11 --output-on-failure'
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc 'make format-check'
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc \
  'cd /workspace/verification/lean && lake build && cd /workspace && python3 tools/scripts/lean_elab_receipt.py && make trace'
bash tools/scripts/cppcheck_scan.sh --out docs/vv/evidence/2026-11-04-re-review/cppcheck.txt
docker run --rm -v "$PWD":/workspace -w /workspace frick-rtems-toolchain:local bash -lc \
  'cmake -S . -B build/rtems-gate7 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_USE_PUS_ASN1=ON && cmake --build build/rtems-gate7 -j$(nproc)'
docker run --rm -e BUILD_DIR=/workspace/build/gate11-smoke -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc 'bash tools/scripts/run_sim.sh'
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local bash -lc 'make gsw-selfcheck'
```

Expected results and original exit codes are in
`docs/vv/evidence/2026-11-04-re-review/README.md` and `candidate-exits.txt`.
