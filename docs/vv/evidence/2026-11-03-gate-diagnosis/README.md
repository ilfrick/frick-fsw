# Gate-failure diagnosis and correction — 2026-11-03

Candidate: git `41dcbe21435e14b5aaff06c63d9df6f0de04df54` plus the working-tree files listed below.
Container image: `frick-vv:local` = `sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b`.
RTEMS toolchain image: `frick-rtems-toolchain:local` = `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896`.
Host: Linux 7.0.0-38-generic, 4 CPUs. Timezone Europe/Rome.

## What was observed (before the correction)

| Run | Command (verbatim) | Original exit | Result | Evidence |
| --- | --- | --- | --- | --- |
| seq 17401 | `make ci-local 2>&1 \| tail -10` | pipeline exit 0 (tail), make reported `Errore 8` | `24 - link_uart` FAILED; full log not retained | `2026-11-03-ci-pipeline-evidence.json` |
| seq 17633 | `make ci-local 2>&1` | **2** | CTest 49/50; `pus` failed at `test_pus.c:754,755,760,761`; `checks=225 failures=4` | `2026-11-03-ci-direct-17633.json` |
| seq 17795 | `docker run ... ./tests/test_pus` (pre-fix binary) | **0** (ctest PASS, but assertions failed inside) | `pus` `test_hk_backward_step_no_emission` at `test_pus.c:869,870`; `checks=225 failures=2` | `2026-11-03-pus-reruns.json` (seq 17795) |
| seq 17849 | `make ci-local 2>&1` | **0** | 50/50; candidate binding "not independently established" in the record | `2026-11-03-ci-direct-17849.json` |

## Root cause (established, not assumed)

- The host PAL computes UTC as `monotonic + offset` (`src/core/pal/pal_posix.c`).
  A time written by `FF_PalSetUtcNs` therefore advances immediately; the CUC
  fine field quantizes to `1e9/65536` ns = 15.2588 us per LSB.
- The HK due test compares `u = now_coarse - last_coarse`; at `u == period`
  it applies `now_fine >= last_fine`. If the enable read crossed a fine LSB
  and the later due read did not, the comparison flips false.
- Probe `time_probe.c` (200 iterations, 20 us between the two reads):
  **200/200 mismatches** (`last_fine=1 now_fine=0`). Pre-fix binary: **3/60**
  failing runs at `test_pus.c:561..574` and `:754..761` (observed; **probe source and loop output not retained in this evidence set**).
- Additionally, a pre-fix run at `test_pus.c:869,870` (`test_hk_backward_step_no_emission`) with `checks=225 failures=2` is recorded in `2026-11-03-pus-reruns.json` seq 17795.
- `link_uart`: reproduced under 4x CPU load, **20/200** runs, assertions
  `test_link_uart.c:240,289,407` with 20 ms/500 ms receive bounds (**reproduction log not retained in this evidence set**).
- Conclusion: test-harness timing defects. The production due test and the
  UART framing logic were not changed.

## Correction

- `src/core/pal/pal.h`, `pal_posix.c`: test-only frozen-UTC seam under
  `FF_TEST_HOOKS` (absent from flight/RTEMS builds); no production behaviour
  changes when the hook is not used. Inline cppcheck suppressions keep the
  advisory inventory at 163 visible findings.
- `tests/unit/test_pus.c`: the HK tests freeze UTC for their duration; unfreeze at test end.
- `tests/unit/test_link_uart.c`: positive receive bounds raised from 20 ms /
  500 ms to 2 s (bounded; a hang still fails). The 2 ms negative cases are
  unchanged.
- `docs/planning/sprint-plan.md`: unsupported claims withdrawn (Rev 1.117,
  1.118), accurate failure record (Rev 1.119), verification record (Rev
  1.120), NC-EXT-235.

## Post-correction evidence (original exits, no pipelines)

**Note:** The native CTest and xw1-host build/CTest logs `ctest-native.log`, `ctest-xw1-host.log`, `build-xw1-host.log` in this directory are from **07:56–07:57** and predate the final `pal_posix.c` edit (08:00:27). They are **SUPERSEDED** by the reruns below. The superseded logs are retained for audit.

| Gate | Command | Exit | Result | Log |
| --- | --- | --- | --- | --- |
| ci-local | `make ci-local > LOG 2>&1` | **0** | 50/50, Lean, elab receipt, trace PASS (120/76/30/14/68) | `ci-local-final.log` (08:02) |
| native CTest (rerun) | `ctest --test-dir build/native --output-on-failure > LOG 2>&1` | **0** | 50/50 | `ctest-native-rerun.log` (08:55) |
| xw1-host build (rerun) | `cmake --build build/xw1-host -j` | **0** | 242/245 targets built | `build-xw1-host-rerun.log` (08:55) |
| xw1-host CTest (rerun) | `ctest --test-dir build/xw1-host --output-on-failure > LOG 2>&1` | **0** | 50/50 | `ctest-xw1-host-rerun.log` (08:55) |
| RTEMS cross-build | `docker run ... frick-rtems-toolchain:local cmake ... && cmake --build build/rtems` | **0** | ELF linked; `FF_PalTimeTestFreeze/Unfreeze` **absent** from final ELF | `build-rtems-docker.log` |
| format | `docker run ... frick-vv:local make format-check` | **0** | clang-format 18 (pinned), all files clean | `format-check.log` (08:00) |
| cppcheck | `tools/scripts/cppcheck_scan.sh --out ...` | **0** | 163 visible findings + 2 test-only suppressed (GAP-A4 advisory) | `cppcheck.log`, `cppcheck.txt` (08:00) |
| stress | 100x each under 4 CPU burners (driver script logged) | **0** | `test_pus` 0/100, `test_link_uart` 0/100 (sample; not proof of absence) | `stress-100-under-load-detailed.log` |

## Superseded evidence (retained for audit)

- `ctest-native.log` (07:56) — native CTest before final `pal_posix.c` edit
- `ctest-xw1-host.log` (07:57) — xw1-host CTest before final edit
- `build-xw1-host.log` (07:56) — xw1-host build before final edit
- `build-native.log` (07:56) — native build before final edit
- `ci-local-after-fix.log` (07:51) — intermediate ci-local before `test_link_uart.c` and `pal_posix.c` edits

## File identities (sha256)

- `src/core/pal/pal.h` `9a3708f2c4091ed8cc702186da5edd318866ddb9070ac5e8433c25daa2330219`
- `src/core/pal/pal_posix.c` `44820fd4fb8b1cfe17afa4542612b20b97141e2c9bab5c3f3d9a10c47c5bb939`
- `tests/unit/test_pus.c` `6f68de14635039a35381cb5f6c51accadc3afb2f4d47938778b0b40df4de2c0d`
- `tests/unit/test_link_uart.c` `c7307c7d13167c8d20acf59b0d5fa68f440108d196de3dc9ef8de82faabe4252`
- `docs/planning/sprint-plan.md` `e48aa046949b29584670cf9f62cdbb8181ad07f993dd17d25c7d1c5fba92650a`

## Evidence manifest (every other file in this directory)

`assets.sha256` lists the SHA-256 of **every other regular file in this directory**
(29 entries). It does not list itself: a file cannot contain its own hash.

Verify from the repository root:

```
sha256sum -c docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256
```

Expected result: **exit 0, 29 OK, 0 FAILED**. The retained transcript of that
command is `docs/vv/evidence/2026-11-04-re-review/assets-sha256-verify.log`.

### Correction history (2026-11-04)

Two defects were found in the original manifest by FRICK-REVIEW-0452 F-03 and
FRICK-REVIEW-0454/0455:

1. it contained a self-referential entry for `assets.sha256`, so verification
   could never succeed;
2. it had been generated before the seven `*-final*` logs existed, so those
   files were not bound at all while this README claimed "every file".

A first correction removed only the self-entry. That correction was itself
defective: this README was edited *after* the new manifest was generated, so the
recorded README hash no longer matched the file. FRICK-REVIEW-0454 F-0454-01 and
FRICK-REVIEW-0455 PA-0455-01 correctly rejected it as a false closure.

The 2026-11-04 regeneration below was performed in the only order that can hold:
the README text was finalised first, the manifest was generated over the frozen
tree second, and verification was run third with its output retained. Any later
change to a file in this directory requires repeating all three steps.

This package is the **2026-11-03** gate state; the file hashes quoted in the body
above are not the current candidate.


Pre-existing working-tree changes excluded from this candidate and preserved:
`tests/unit/test_pus_n7_adapter.c`, `test_pusopen.c`, `test_r2_gap.c`,
`test_sched_max_entries.c`, `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`,
and all other untracked files shown by `git status` **except this evidence directory** (`docs/vv/evidence/2026-11-03-gate-diagnosis/`).
