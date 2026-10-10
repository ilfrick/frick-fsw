# AGENTS.md - Agent-oriented project guide

> frick-fsw: cFS-inspired flight software framework, PUS-C TM/TC on CCSDS Space
> Packets, target: Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS
> 6.2, target ECSS criticality B.
>
> This file is the agent entry point (norm N8): a fresh agent with no prior
> context must be able to build, test, verify and contribute to this project
> using only the repository contents and the commands below. Human entry point:
> `README.md`.

## 1. Ground rules (non-derogable, norm N1-N17)

- Maintain `docs/rules.md` and the enforcement summary. Key gates before any
  commit/push:
  1. `make build` and `make test` green.
  2. `make verify-lean` green (`lake build` in `verification/lean`).
  3. `make trace` green (bidirectional traceability, norm N4).
- Every change (any origin: fix, review NC, requirement delta) re-runs the full
  cycle (norm N15) and updates the revision history (norm N16).
- All repo content (docs, comments, commit messages) is **English** (norm N2).
- Every change that touches behaviour, requirements, tooling or configuration
  must update the affected docs in the same commit (norm N8).
- Every change is committed and pushed to **both** remotes (norm N7).

## 2. Repository map

| Path | Purpose |
| ---- | ------- |
| `src/` | C framework: `core/pal` (PAL), `core/sb` (Software Bus), `core/evs`, `core/es`, `core/tbl`, `core/time`, `transport/ccsds`, `transport/link`, `pus`, `apps`, `bsp/mpfs`, `main` |
| `config/` | single configuration point (`fsw_config.h`) + mission tables + platform headers |
| `tests/` | unit tests (C, ctest) on `tests/ff_test.h` harness |
| `verification/lean/` | Lean 4 project: formal requirement theorems (norm N1) |
| `tools/` | scripts (`trace.py`, `coverage.py`, `metrics.py`, `setup_rtems.sh`, `run_sim.sh`), ground station (`gsw/ff_gsw.py`), docker V&V material |
| `assets/` | project marks (logo) for codename "Eärendil"; see `assets/README.md` |
| `docs/` | rules, requirements (SSS/SRS/traceability), architecture, ADRs, interfaces (ICD), quality (class B/A), V&V, engineering (independent review + driver/service/user guides), reviews, roadmap, **development plan**, status |
| `NOTICE` | copyright and license notice (Copyright (C) 2026 Ilfrick, norm N13) |
| `.github/workflows/`, `.gitea/workflows/` | CI pipelines |
| `board/` | device-tree sources (`mpfs-rtpfs160zt.dts`) |
| `cmake/toolchains/` | RTEMS 6.2 riscv/mpfs64imafdc toolchain file |

## 3. Quickstart (host simulator)

```bash
# one-command local pipeline from a fresh clone (any POSIX host with docker):
make ci-local            # = docker run the V&V image: format, build, test, lean, trace, docs

# or without docker:
make configure           # cmake -S . -B build/native
make build               # native simulator + core library
make test                # ctest unit tests
make run                 # boot the native simulator (6 apps)
make verify-lean         # lake build in verification/lean
make trace               # python3 tools/scripts/trace.py docs/requirements/traceability.yaml
make coverage            # gcov line/function coverage -> build/coverage/coverage.json
make metrics             # regenerate the README status panel (coverage + semaphores)
```

RTEMS cross build (requires toolchain; `tools/scripts/setup_rtems.sh` installs
the RSB toolchain into `/opt/rtems/6`):

```bash
make rtems              # cross compile for riscv/mpfs64imafdc
```

## 4. Deploying the CI/CD and V&V environments (norm N6)

The project is deployable everywhere through Docker:

```bash
# V&V image with pinned toolchain (cmake, ninja, gcc, python3, node, lean, doxygen, tidy)
make docker-image        # builds frick-vv:local from tools/docker/Dockerfile

# Gitea CI/CD: self-hosted Gitea Actions runner (norm N6)
# Requires GITEA_RUNNER_REGISTRATION_TOKEN; maps `ubuntu-24.04` to frick-vv:local.
docker compose -f tools/docker/act_runner/docker-compose.yml up -d
```

The runner executes `.gitea/workflows/ci.yml` on the Gitea mirror. GitHub
Actions executes the identical `.github/workflows/ci.yml`.

## 5. Contribution checklist (agents and humans)

1. Branch, implement, keep English.
2. Update `docs/requirements/traceability.yaml` + SRS/SSS/traceability.md if
   requirements change (norm N3/N4/N8).
3. Update `docs/vv/tool-qualification.md` if a V&V script/tool changed
   (norm N10).
4. Run the full local pipeline above; leave the evidence in
   `docs/vv/verification-results.md` (norm N11).
5. If the change needs a review that the criticality marks independent
   (norm N9), instantiate the reviewer agent as documented in
   `docs/engineering/independent-review.md`, and store the report in
   `docs/reviews/`.
6. Resolve every finding to compliance (norm N12) before closing the NC.
7. Run the **documentation review loop** (norm N17): instantiate the Senior SW
   documentation reviewer (profile R4, `docs/engineering/independent-review.md`)
   over the documentation affected by the change, correct every finding, and
   re-review until the documentation review is APPROVED or APPROVED WITH
   COMMENTS with all NCs closed.
8. Commit + push to both `origin` (GitHub) and `gitea` remotes (norm N7).

## 6. Standards references used

- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.
- RTEMS 6.2 `riscv/mpfs64imafdc` BSP, Microchip RTPFS160ZT (FCVG784) datasheet.
- cFS (NASA core Flight System, Draco release) as architectural inspiration.

## 7. Norms at a glance

| N | Rule |
| - | ---- |
| N1 | Every generated code artifact must be checked with Lean 4 |
| N2 | All docs and comments in English |
| N3 | Backwards ECSS SRS/SSS for all code, verified in Lean |
| N4 | Bidirectional traceability, converged at every commit |
| N5 | Deliverable for ECSS class B; class-A gaps documented |
| N6 | Full SW V&V; deploy CI/CD via Docker (Gitea has no CI/CD) |
| N7 | Commit and push compliant changes to both repositories |
| N8 | Complete, current, deployable documentation (humans + agents) |
| N9 | Independent reviews by independent-context agents (R1/R2 profiles) |
| N10 | Test scripts / V&V tools reviewed & qualified per ECSS |
| N11 | Test scripts and review results committed/pushed every cycle |
| N12 | Non-conformances resolved by re-applying all rules to full compliance |
| N13 | Open source project under AGPL-3.0-only; license documented/reviewed |
| N14 | SSS requirements reviewed by Senior Systems Engineer (R3); NCs to closure |
| N15 | Every change re-runs the complete development+V&V cycle to success |
| N16 | Detailed revision history maintained (CHANGELOG + per-doc history + commits) |
| N17 | Documentation reviewed by the Senior SW documentation reviewer (R4) on every change; review loop closed before commit/push |