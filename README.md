# frick-fsw

**Codename "Vela" · v0.1.0 · AGPL-3.0-only**

cFS-inspired flight software framework: PUS-C TM/TC over CCSDS Space Packets
for a small/medium spacecraft OBC built on the **Microchip RT PolarFire SoC
RTPFS160ZT** (RISC-V) under **RTEMS 6.2** (embedded brains ECSS pre-qualified
baseline), target ECSS criticality **B**.

> Agent-oriented project guide: **`AGENTS.md`**. Non-derogable norms (rules
> N1-N16): **`docs/rules.md`**. Approach & tools: **`docs/tooling.md`**.
> Licence: **AGPL-3.0-only** (`LICENSE`, `docs/quality/licensing.md`).

## Highlights

- **PUS-C** (ECSS-E-ST-70-41C) on CCSDS Space Packets (CCSDS 133.0-B-2) with
  CRC-16/CCITT-FALSE trailer; pluggable backend: open codec (default) and
  documented commercial PUSopen adapter (optional, `FRICK_USE_PUSOPEN`).
- **cFS-style core**: PAL, Software Bus, Event Services, Executive Services,
  Table Services, Time Services; six reference applications.
- **Same sources** for the native simulator and the RTEMS 6 target
  (`riscv/mpfs64imafdc`, Microchip RTPFS160ZT).
- **Formal verification with Lean 4** (norm N1): CRC, Software Bus, PUS
  counters, CCSDS length bounds — requirement theorems linked from the
  traceability manifest.
- **Reproducible V&V**: Docker image, GitHub Actions + Gitea Actions via
  `act_runner` (norm N6), traceability gate `tools/scripts/trace.py`.
- **Backwards ECSS engineering data**: SSS, SRS, traceability matrices,
  architecture, ADRs, ICD, quality (class B baseline + class A gaps).

## Quickstart

```bash
# Full local pipeline inside the reproducible V&V container (norm N6):
make ci-local

# Or natively:
make configure build test      # build + unit tests (ctest)
make run                       # boot the native simulator (6 apps)
make verify-lean               # lake build (formal verification, norm N1)
make trace                     # traceability convergence gate (norm N4)

# RTEMS 6.2 cross build (toolchain: tools/scripts/setup_rtems.sh):
make rtems
```

## Requirements set (current)

SRS = 21 · SSS = 15 · GAP (to class A) = 6 · Lean theorems = 20. See
`docs/requirements/`.

## Repository layout

| Path | Content |
| ---- | ------- |
| `src/` | C framework (core, transport, PUS, apps, BSP, main) |
| `config/` | single configuration point + mission tables + platform headers |
| `tests/` | unit tests (ctest) |
| `verification/lean/` | Lean 4 formal verification project |
| `tools/` | scripts, ground station, Docker V&V environment |
| `docs/` | rules, requirements, architecture, ADRs, ICD, quality, V&V, reviews |
| `board/` | device-tree template for the RTPFS160ZT |
| `.github/`, `.gitea/` | CI/CD pipelines |

## Documentation set

- Rules: `docs/rules.md` (N1-N16)
- Requirements: `docs/requirements/SSS.md`, `SRS.md`, `traceability.yaml`, `traceability.md`
- Architecture: `docs/architecture/` · ADRs: `docs/adr/` · ICD: `docs/interfaces/ICD.md`
- Quality: `docs/quality/ecss-criticality-b.md`, `class-a-gaps.md`, `licensing.md`
- V&V: `docs/vv/verification-plan.md`, `verification-results.md`, `tool-qualification.md`
- Reviews: `docs/engineering/independent-review.md`, `docs/reviews/`
- Formal verification: `docs/verification/formal-verification.md`
- Glossary, roadmap, tooling

## Contributing

See `CONTRIBUTING.md` (rules N7/N11/N15/N16 apply: full cycle, dual push,
history, reviews). Security: `SECURITY.md`. License: AGPL-3.0-only.

## Mirrors

- GitHub: https://github.com/ilfrick/frick-fsw
- Gitea (home): https://www.housefz.com/git/ilfrick/frick-fsw

## Acknowledgments

Architectural inspiration from NASA cFS (Draco); RTEMS 6.2 baseline by embedded
brains; Lean 4 by the Lean community. The project is independent and contains
no copied cFS/RTEMS code.