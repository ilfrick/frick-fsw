# Changelog

Detailed revision history of frick-fsw (norm N16). Document-level histories are
maintained in each file's `## History` table; this file records the project
level history per release and per significant change, with the origin of every
change. All dates UTC, project language English (norm N2).

## [Unreleased]

### Changed

- Project codename changed from **"Vela"** to **"Eärendil"** (README banner,
  `config/fsw_config.h` `FF_CODENAME`, simulator boot banner).

### Fixed

- CI `simulator smoke` (GitHub): the banner check grepped for `=` which the
  banner never contained; with `bash -e` the step failed. Now checks for
  `frick-fsw` and reports a clear error.
- CI `static analysis`: the `clang-format --dry-run --Werror` gate failed on
  files that had never been auto-formatted; all C sources are now formatted with
  clang-format 18 (the runner version), so the gate passes.

## [0.1.0] - 2026-10-10 - baseline release

**Origin:** rule set establishment (N1-N16) and initial development; review
closure (norm N12/N15) applied before the first public push.

**Origin note:** the review-closure cycle below re-ran the complete
development/verification cycle (norm N15) and is covered by
`docs/reviews/2026-10-10-baseline-vv-review.md` (R2) and
`docs/reviews/2026-10-10-sss-systems-review.md` (R3).

### Added

- Formal verification (norm N1) now actually builds: `lake build` compiles all
  15 modules and `lake exe frick_verify_checks` passes 3/3 in the Docker V&V
  image; requirement theorems are linked in the traceability manifest.
- Independent reviews under `docs/reviews/`: baseline V&V (R2), SSS systems
  review (R3, norm N14) and the norms N1-N16 conflict analysis.
- Ground support tool `tools/gsw/ff_gsw.py` (qualified as T7).

- Core framework (cFS-inspired, AGPL-3.0):
  - Portable Abstraction Layer (PAL) with POSIX (host) and RTEMS 6 backends.
  - Software Bus (publish/subscribe, wildcard subscriptions, bounded pipes).
  - Event Services, Executive Services, Table Services, Time Services.
  - CCSDS Space Packet codec + CRC-16/CCITT-FALSE trailer.
  - Link layer with loopback (host) and UART (target) transports.
  - PUS-C layer with open backend default and documented PUSopen adapter.
  - Services implemented: 1 (verification), 3 (HK), 5 (events), 6 (memory),
    8 (function), 9 (time), 17 (test), 20 (parameter).
  - Board support for the Microchip RT PolarFire SoC RTPFS160ZT (FCVG784).
  - Reference applications: CMD_INGEST, TM_OUTPUT, SCHEDULER, HK, HS, SAMPLE.
- Tests: `tests/unit/{test_ccsds,test_sb,test_tbl,test_pus}.c` (ctest; 4 suites).
- Formal verification (norm N1): Lean 4 project `verification/lean/` with
  requirement theorems (CRC, Software Bus, PUS counters, CCSDS length bounds)
  and `Main.lean` runtime checks.
- Build & tooling: CMake + Ninja + presets, RTEMS `riscv/mpfs64imafdc`
  toolchain file, `Makefile`, clang-format/tidy, Doxygen, Docker V&V image,
  act_runner deployment for Gitea, ground station `ff_gsw.py`.
- CI/CD: `.github/workflows/ci.yml` + `nightly.yml` (RTEMS), `.gitea/workflows/ci.yml`.
- Requirements & quality data (norms N3/N4/N5/N8):
  - `docs/requirements/{SSS,SRS,traceability}.{md,yaml}` (SRS=21, SSS=15, GAP=6).
  - Traceability gate `tools/scripts/trace.py` (convergence on every change).
  - `docs/rules.md` (N1-N16), `docs/tooling.md`, `docs/architecture/`,
    `docs/adr/` (ADR-0001..0011), `docs/interfaces/ICD.md`, `docs/glossary.md`,
    `docs/roadmap.md`, `docs/verification/formal-verification.md`.
  - `docs/quality/ecss-criticality-b.md`, `class-a-gaps.md` (GAP-A1..A6),
    `licensing.md`.
  - V&V: `docs/vv/{verification-plan,verification-results,tool-qualification}.md`.
  - Reviews: `docs/engineering/independent-review.md` (profiles R1, R2, R3),
    `docs/reviews/` template and log.
- Licensing: AGPL-3.0-only (`LICENSE`), SPDX headers repo-wide (norm N13).

### Changed

- (first release; no prior history)

### Fixed

- Lean proofs: added missing `import Frick.Common` in `Frick.SoftwareBus`,
  replaced the Mathlib-only `norm_num` with core tactics, made `TmCounter` an
  `abbrev` and unfolded `def` constants before `omega`; `lake build` now passes.
- PUS service 6 memory dump: corrected the S6 unit test to write the address and
  size in network (big-endian) order per the ICD, and made `svc6_dump` bound the
  requested range with overflow-safe 32-bit address arithmetic while reading
  from the registered region base (host-safe and target-accurate).
- Traceability gate: `THEOREM_RE` now uses `re.MULTILINE`; the CI evidence
  parser accepts both `ci-<name>` and bare `# evidence:` names and multiple
  claims per line; `ID_RE` accepts the `GAP-A<n>` identifiers.

### Security

- See `SECURITY.md`.

### Norms compliance status

All norms N1-N16 applied; gates PASS: host build, ctest, `lake build`,
`trace.py`, simulator smoke (6 apps), static analysis, RTEMS cross build proxy
(toolchain install via `tools/scripts/setup_rtems.sh`). Class-A gaps (GAP-A1..A6)
documented and tracked.