# Evidence archive — XW-2 target contract, final cycle (2026-11-01)

| Field | Value |
| --- | --- |
| Document id | FRICK-EVID-XW2-20261101 |
| Revision | 1.0 |
| Date archived | 2026-11-02 |
| Base HEAD at execution | `65e4a91` |
| Scope | XW-2 target contract (SRS-REQ-073/075), test-only RTEMS/QEMU image |
| Norms | N10/N11/N22 (retention and reproducibility) |

This directory is a **byte-identical archive** of the retained gate logs and the
per-run artifacts of the XW-2 final cycle. It exists so the evidence survives
rebuilds of `build/` and so the retained identities can be re-verified without
re-running anything. `assets.sha256` covers every archived file and was
generated after the copy; the copy step was verified with `cmp` against the live
originals (all 38 files identical at archive time).

## Contents

| File | Role | Original exit status |
| --- | --- | --- |
| `host-build.log` | host build re-check (no work) | 0 |
| `host-ctest.log` | full host CTest, **50/50 PASS**, incl. `extwd_model_contract`, `extwd_middleware_contract` | 0 |
| `lean-host.log` | `lake build` in `verification/lean` | 0 |
| `trace-host.log` | `trace.py` + Lean elaboration receipt (120 req / 76 SRS / 30 SSS / 14 gaps / 68 theorems) | 0 |
| `format.log` | pinned-formatter guard rejecting host clang-format 23 | 1 (expected; guard works) |
| `format-apply.log` | `make format` inside `frick-vv:local` (pinned clang-format 18) | 0 |
| `format-check2.log` | `clang-format --dry-run --Werror` over `src config tests` inside the pinned image | 0 |
| `xw2-final.log` | **authoritative** XW-2 gate run (`make xw2`), run dir `…T172050Z` | 0 (`xw2-final.exit`) |
| `xw2-run.log` / `.exit` | earlier same-day attempt, run dir `…T170112Z` (superseded) | 0 |
| `xw2-run-2.log` / `.exit` | earlier same-day attempt, run dir `…T170759Z` (superseded) | 0 |
| `ci-local.log` | `make ci-local` inside `frick-vv:local` failing at `pus-asn1` because that image has no `docker` CLI | 127 (environment limitation, not a gate result) |
| `core-gates.log` | stale in-container attempt against a host-absolute `CMakeCache.txt`; **not gate evidence** | (failed; retained only to document why the container path was abandoned) |
| `run-20261101T172050Z/` | full run directory: console, verifier/verdict JSON, ELF hashes, `nm-segregation.txt`, toolchain id, backend, run nonce, per-build logs | see file contents |

## Retained identities

- Positive (`frick_fsw_xw2`) ELF sha256:
  `0972d96a0e4f549ae3c956af2edb4c98c94ea03c657ef0a2c5a60fe3c710f251`
- No-wrap control (`frick_fsw_xw2_nowrap`) ELF sha256:
  `b695b1ad4c92647f28017c1e765f456a9dcae0b6bfec1d6cad7e3223d6011926`
- Toolchain image: `frick-rtems-toolchain:local`,
  id `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896`
- Compiler: `riscv-rtems6-gcc (GCC) 13.3.0 20240521 (RTEMS 6, RSB 3814cb0e…, Newlib 1b3dcfd)`
- Run nonce: `f126bd39b793426fac9534fdcf288a60`
- Gate outcome: `SRS-REQ-073 PASS=12`, `SRS-REQ-075 PASS=1`,
  `COVERAGE 13/13`, `WHOLE-RUN PASS`, `RESULT PASS`; no-wrap control
  **EXPECTED FAILURE** (watchdog-init panic + component verifier exit 1).

## What this archive does and does not establish

Establishes: the middleware's obligations under the **abstract** external
watchdog contract, on a test-only RTEMS 6.2 image executed under QEMU `virt`,
with the three PAL device hooks substituted at link time by a validated test
double, plus the discriminating no-wrap control.

Does **not** establish, and must never be cited as: norm N18, a device or HIL
result, real reset or reset scope, electrical/clock independence, timing or
WCET, mission timeout (8 s remains an ESC-7 placeholder), tool qualification
(T25 is NOT QUALIFIED), or any safety/release approval.

## Reproduction

`make xw2` (→ `tools/scripts/qemu_xw2_vv.sh`). The runner records its backend
(`docker` or `host`) in `backend.txt` and exits 2 — never SKIP — when neither
backend prerequisite exists. The archive is the evidence of record; re-running
creates a new `build/xw2-run-*` directory and must not overwrite this one.
