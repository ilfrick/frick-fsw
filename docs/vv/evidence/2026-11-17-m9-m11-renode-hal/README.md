# Evidence set — NC-EXT-251 correction (second pass), M9/M11 increment and emulated N18 (2026-11-17/18)

Retained under norm N11 for the uncommitted increment on top of pushed baseline
`c166cea36483c3262a64edd63dc4368fcbc119df` (branch `main`).

The `-v2` files are the **current** evidence for the corrected tree, produced
after the 2026-11-17 R1/R2/R3 reviews and the second correction of
`src/pus/pus.c`. The un-suffixed files are the first-pass evidence; they are
retained because reviews reference them and because the first-pass numbers
(65 tests, 77 theorems, `hal_pcie=PASS`) are exactly what the reviews examined.
**Use the `-v2` set for the state of the tree.** See "Known stale artifacts" at
the end.

## How each recorded claim maps to a file

| Claim | File |
| --- | --- |
| `make ci-local`: 65/65 tests passed, `elab-receipt: PASS`, `lean theorems found: 81`, traceability PASS (120/76/30/14/81) | `ci-local-v2.log` |
| `BUILD_DIR=build/smoke-fresh make smoke`: banner + 7 applications registered | `smoke-v2.log` |
| Lean build: `Build completed successfully.` | `lean-v2.log` |
| `make trace`: 120/76/30/14/81, exit 0 | `trace-v2.log` |
| RTEMS image rebuilt from current sources; ELF sha256 `d17b124d4af1e9e1f736de2d2b6e8947a0bb4c1c4855f0c7f04ea59243e4b43d` | `rtems-rebuild.log` (first pass, ELF superseded), `elf-v2.sha256` |
| N18 Renode run: `RENODE_TEST: PASS`, container exit code 0, HAL matrix with `hal_pcie=CONTRACT` and `hal_spacewire=CONTRACT` excluded from the verdict | `renode-run-v2.log` (UART console), `renode-process-v2.log`, `assets.sha256` |
| NC-EXT-251 TDD RED on the original pre-fix source (2 failures, exit 1) | `tdd-nc-ext-251/green-on-head-code-fails.log` |
| NC-EXT-251 TDD GREEN on the first correction (`checks=147 failures=0`, exit 0) | `tdd-nc-ext-251/green-on-fixed-code-passes.log` |
| NC-EXT-251 behavioural RED against the first correction: `checks=7 failures=4`, exit 1, with the pre-fix source hash, command line and compiler recorded in the log | `tdd-nc-ext-251/red-v2/red-on-first-correction.log` |
| NC-EXT-251 behavioural GREEN on the second correction: `checks=7 failures=0`, exit 0, with source hash, library hash, command line and compiler | `tdd-nc-ext-251/red-v2/green-on-second-correction.log` |
| The RED/GREEN probe source (public API only, so it links against the pre-fix code) | `tdd-nc-ext-251/red-v2/probe.c` |
| Reviewed candidate sources for the PUS module (first and second correction) | `tdd-nc-ext-251/pus.c.candidate` (sha256 `f5a3722b…`), `tdd-nc-ext-251/pus.c.candidate-v2` (sha256 `a1b1a49a…`) |

Also in this set: `status.env` (run metadata), `docker-logs.stderr`,
`elf.sha256` (first-pass ELF `6905773d…`, superseded), `ci-local.log`,
`smoke.log`, `lean.log`, `trace.log`, `renode-run.log`, `renode-process.log`,
`uart-console.log` (first pass, retained for review provenance),
`tdd-nc-ext-251/red-v2/probe.c`.

## Scope and limits

- **Emulated only.** These are Renode runs. There is no physical target or HIL
  evidence, and nothing here supports a target, qualification or flight claim.
- **Contract-only HAL checks.** `hal_pcie` and `hal_spacewire` report `CONTRACT`,
  not `PASS`. The board description (`beaglev-fire.repl`, loaded by
  `tools/docker/renode/frick-renode.resc`) instantiates no PCIe root complex and
  no SpaceWire controller, so neither can be exercised peripheral-bound. Their
  contract coverage is the host unit tests (`tests/unit/test_pcie.c` and the
  SpaceWire mock-link self-test). Both are **excluded** from the aggregate
  verdict, so `RENODE_TEST: PASS` no longer depends on a check that never touched
  a register (NC-EXT-253).
- **MTD/QSPI is not covered.** Renode models the Micron MT25Q flash as a child of
  the `SPI.MPFS_QSPI` controller rather than memory mapped at `0x21000000`, so the
  reference MTD driver cannot be exercised by direct MMIO in this environment.
  On-target QSPI remains GAP-A5.
- **NC-EXT-246 remains OPEN.** The containerised runner was used without an
  approved preflight/cleanup procedure. The runs are recorded as performed, not
  as cleared by an approved procedure.
- `build/native` is unusable (its CMake cache points to `/workspace`), which is
  why the smoke run uses `BUILD_DIR=build/smoke-fresh`.
- `build/renode/` is root-owned, so `tools/scripts/renode_vv.sh` cannot retain
  `console.log` there; the per-run `/tmp/frick-renode-vv.*` capture is the source
  of the Renode files reproduced here.
- Several source files and the Lean build output are written by root inside the
  controlled Docker images. The elaboration receipt is mode `0600` root-owned
  after a Docker run, which makes `tools/scripts/trace.py` report
  `E: NC-EXT-174: receipt_unreadable`; regenerate it on the host with
  `python3 tools/scripts/lean_elab_receipt.py` before running `trace.py`.
- These files are logs and source snapshots, not an attestation. The manifest
  `assets.sha256` is a digest of this directory only and does not bind the whole
  candidate tree.

## Known stale artifacts (kept deliberately)

The un-suffixed first-pass logs describe a tree that no longer exists: 65
registered tests (65 is still current), 77 Lean theorems (now 81),
`hal_pcie=PASS` (now `CONTRACT`), and ELF `6905773d…` (now `d17b124d…`). They
are retained rather than overwritten because the 2026-11-17 R1/R2/R3 findings
cite those numbers, and an evidence set that silently rewrote its own past would
be untraceable. Anything read from the un-suffixed files must be read as "the
state the reviews examined", not as the current state.
