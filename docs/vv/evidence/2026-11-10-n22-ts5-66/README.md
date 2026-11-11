# Evidence: `N22-TS5-66` TBL load/activate → scheduler dispatch (2026-11-10)

Scope: verification evidence for the scheduler Table Services coupling increment
described in `CHANGELOG.md` (2026-11-10) and `docs/status/handover.md` rev 3.67.
It records the host gates plus the RTEMS/N18 build and the Renode run executed on
the same working tree.

**Not claimed:** mission rates, tolerances, jitter or WCET; the worst-case
one-tick activation-apply deferral budget; target/HIL, radiation or
RTPFS160ZT fidelity; flight-use authorization. The activation callback itself is
exercised on the host, not on the emulated target; N18 establishes startup and
the scheduler app's initial table only.

## Artifacts

| File | Content |
| --- | --- |
| `ctest.log` | `ctest --test-dir build2/native -j4` — 100% tests passed, 0 failed out of 55 |
| `trace.log` | `tools/scripts/trace.py docs/requirements/traceability.yaml` — PASS (120/76/30/14/68) |
| `format-check.log` | pinned clang-format 18.1.3 (`frick-vv:local`) `make format-check` — exit 0 |
| `rtems-n18-build.log` | clean RTEMS 6.2 cross-build of `build/renode` (`FF_ENABLE_RENODE_TEST=ON`), 164/164 steps, exit 0; contains 1 pre-existing warning (NC-EXT-239) |
| `renode-console.log` | UART console of the N18 run — `SCH app started, 4 entries`, `RENODE_TEST: PASS` |
| `renode-process.log` | Renode process log of the same run |

## Commands and results

```text
docker run --rm -v "$PWD":/workspace -w /workspace frick-rtems-toolchain:local \
  bash -lc 'cmake --build build/renode --clean-first -j$(nproc)'      # exit 0
bash tools/scripts/renode_vv.sh
  -> [renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)
ctest --test-dir build2/native -j4                                     # 55/55 PASS
python3 tools/scripts/trace.py docs/requirements/traceability.yaml     # PASS
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local make format-check  # exit 0
```

## Identity of the candidate

| Item | SHA-256 |
| --- | --- |
| `build/renode/frick_fsw_renode.elf` | `5557d0bd2c951af1e6a3d1afee7a5fb36858f74738dca48ac6cacc02155e1a0e` |
| `src/apps/app_scheduler.c` | `3b5b8d9f75401a536ef60162d5bcac2ec4912b467bb55b25aafb9b7a5a2c6355` |
| `src/apps/sched_evt.h` | `ffa686956b6012d73a58c7fd69f289f57028215652d955b72d3f0f7b31852ed4` |
| `src/core/tbl/tbl.h` | `1013b2515bc2ed9ec703c26770cb1ce977c2f9e54565cd7f52181383fa4d78e5` |
| `src/core/tbl/tbl.c` | `587b6becb0cbaacb768aeeb63b2609489929d39e4b642bd049b427d6963f3b15` |
| `src/core/pal/pal_posix.c` | `c49a265838e39553394a2e4998f991749255d01c19af2a4c5b6fe77d7a05d017` |
| `src/core/pal/pal.h` | `f24e71d4307246a8a15f7e87ab3c665fb30cdf4e4df1ad6d66bf32a628284755` |
| `tests/unit/test_tbl_sched_coupling.c` | `df46cd09784944545c7494f3f073fca1d3f8f90d3dd6528b434455ec1c5d1ceb` |
| `tests/unit/test_sched_events.c` | `2a31472c4602bd587928ab1301ca2231e7d8e561039db0ef74bfe8f4b155ed5c` |

`build/renode/frick_fsw` (rebuilt after `--clean-first`) is byte-identical to the
copied `.elf` (same SHA-256), so the emulated run and the retained binary are the
same artifact for this configuration.

## Delta manifest for independent review

`delta-manifest.sha256` (sha256 `9575c3a27b64a68e628cb17abb8dd3c905e0adc6aea98b1e59d16a16e2c747d5`)
binds the 17 files carrying the `N22-TS5-66` delta (sources, tests and records)
as reviewed by FRICK-REVIEW-0479..0482. Those files also carry unrelated
uncommitted work from earlier increments (watchdog/PAL test seams, scheduler
event-format extraction); the manifest binds the whole file, while the review
scope is the `N22-TS5-66` symbols and hunks named in the review reports.

## Open items

- **NC-EXT-239 OPEN** (non-functional, N21): pre-existing
  `src/core/pal/pal_rtems.c:156: implicit declaration of function
  'ff_pal_wd_hw_feed'` in the RTEMS build; prototype exists only in
  `src/core/pal/pal_watchdog.c`. Outside this increment.
- Independent R2/R3/R4/PA review of the increment is not yet performed.
- Target/HIL (GAP-A5), NC-PAL-01, NC-EXT-19/90, `N22-TS5-47`/`N22-TS5-48`, M14
  memory ordering and the one-tick deferral budget remain open.
- No NC is closed and no commit/push is authorized by this evidence.
