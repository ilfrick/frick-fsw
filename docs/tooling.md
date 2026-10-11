# Approach and Tooling

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-TOOLING-001                      |
| Revision                          | 0.5                                    |
| Date                              | 2026-10-10                             |

This document describes the engineering approach and every tool used for
development and for Software V&V (norm N8: approach and tools are documented and
kept current; norm N10: V&V tools are qualified in `docs/vv/tool-qualification.md`).

## 1. Engineering approach

The project is a cFS-inspired, PUS-C and CCSDS Space-Packet capable flight
software framework aimed at ECSS criticality B on the Microchip RT PolarFire
SoC (RTPFS160ZT, RISC-V) under RTEMS 6.2 (open-source RTEMS project). The approach is:

1. **Architecture first, verified backwards** (norm N3): the SRS/SSS are
   generated backwards from the architecture and code and traced in
   `docs/requirements/traceability.yaml`.
2. **Single configuration point**: all tunables in `config/fsw_config.h` and
   mission tables (cFS heritage).
3. **Portable abstraction layer (PAL)** so the same sources run on the host
   simulator (POSIX threads) and on RTEMS 6 / RISC-V.
4. **Pluggable PUS backend**: the N7 Space / ESA ASN.1/ACN PUS-C backend is the
   default (build-time ASN1SCC generation via `make pus-asn1`, `FF_USE_PUS_ASN1`);
   the open CCSDS backend is the fallback and a documented commercial PUSopen
   adapter is provided.
5. **Formal verification with Lean 4** of the requirement-level theorems
   (norm N1).
6. **Continuous, reproducible V&V** (norm N6): every change goes through the
   same pipeline in CI (GitHub Actions) and locally (Docker image); Gitea CI is
   provided through `act_runner` deployed with Docker.
7. **Independent reviews** for anything the criticality marks as independent
   (norm N9), with results committed per norm N11.

Decisions are recorded as ADRs in `docs/adr/`.

## 2. Development tools

| Tool | Purpose | Version pinning |
| ---- | ------- | --------------- |
| CMake + Ninja | Build system and generator | `CMakeLists.txt`, `CMakePresets.json` |
| GCC (host) | Native simulator and unit tests (C99, `-Wall -Wextra`) | CI image |
| RTEMS Source Builder (RSB) | Open-source RTEMS 6.2 toolchain (`riscv/mpfs64imafdc`) | `tools/scripts/setup_rtems.sh`, `tools/docker/rtems/` |
| clang-format / clang-tidy | Formatting and static analysis | `.clang-format`, `.clang-tidy` |
| Doxygen | API documentation generation | `Doxyfile`, `make docs` |
| Git | Version control | dual remotes (GitHub + Gitea) |
| Docker / Docker Compose | Reproducible V&V environment and Gitea CI runner | `tools/docker/` |
| Platform `libm` | Portable mathematics facade for applications (M6; `FF_USE_LIBM`) | `src/math/` |

## 3. Verification & validation tools

| Tool | Role | Evidence produced | Qualification ref |
| ---- | ---- | ----------------- | ----------------- |
| CTest | Unit test execution | `tests/unit/*.c` results | `docs/vv/tool-qualification.md` T3 |
| ff_test.h harness | Minimal assertion harness for unit tests | PASS/FAIL summary | T2 |
| Native simulator (`frick_fsw`) | Integration/smoke: 6 apps, SB topics, EVS -> PUS S5 TM on loopback | `run_sim.sh` output | T4 |
| Lean 4 + lake | Formal verification of requirement theorems (norm N1) | `verification/lean/**`, `lake build` | T5 |
| `tools/scripts/trace.py` | Traceability convergence gate (norm N4) | `make trace` report | T6 |
| `tools/gsw/ff_gsw.py` | Ground station: TC injection + TM decode over UART/loopback | operator/automation logs | T7 |
| clang-tidy | Static analysis (memory, style, defects) | analysis report | T8 |
| Docker V&V image | Reproducible pipeline for humans and agents (norm N6/N8) | containerised `make ci-local` | T1 |
| `tools/scripts/run_sim.sh` | Simulator bootstrap for humans/agents/CI | FT logs | T9 |
| `tools/scripts/setup_rtems.sh` | Open-source RTEMS 6.2 toolchain bootstrap (RSB, `riscv/mpfs64imafdc`) | cross build | T10 |
| `tools/scripts/coverage.py` | gcov line/function coverage collector (`make coverage`) | `build/coverage/coverage.json` | T12 |
| `tools/scripts/metrics.py` | README status panel generator (coverage + V&V semaphores) | README metrics panel | T13 |
| `tools/scripts/renode_env_selftest.sh` | Renode V&V environment self-test (Renode runs + PolarFire SoC board platform loads) | `make renode-env` PASS | T17 |
| `tools/scripts/renode_vv.sh` | Renode functional validation (`RENODE_TEST: PASS`, norm N18) | emulated run log; `make renode` | T17 |
| `tools/docker/rtems/Dockerfile` | Reproducible open-source RTEMS 6.2 toolchain + BSP image | `make rtems-toolchain` / `make rtems-docker` | T18 |
| `tools/scripts/gen_pus_asn1.sh` | Generate C from the N7/ESA `asn1-pusc-lib` ASN.1/ACN PUS-C model with ASN1SCC | `make pus-asn1` -> `third_party/asn1-pusc-lib-generated/` | T19 |

## 4. CI/CD tools

| Component | Role | Where |
| --------- | ---- | ----- |
| GitHub Actions workflows | Pipeline on the github mirror | `.github/workflows/ci.yml` |
| Gitea Actions workflows | Pipeline on the gitea mirror | `.gitea/workflows/ci.yml` |
| Gitea act_runner (Docker) | Self-hosted runner so Gitea has CI/CD in Docker (norm N6); maps `ubuntu-24.04` to `docker://frick-vv:local` | `tools/docker/act_runner/` |
| Dockerfile (V&V image) | Pinned toolchain: cmake/ninja/gcc/python3/node/lean/doxygen/tidy | `tools/docker/Dockerfile` |
| Dockerfile (RTEMS toolchain image) | Open-source RTEMS 6.2 toolchain + BSP (T18) | `tools/docker/rtems/Dockerfile` |
| Dockerfile (Renode V&V image) | Pinned Renode, norm N18 (T17) | `tools/docker/renode/Dockerfile` |

The Gitea instance is served under the `/git` sub-path, but `act_runner`
derives the job `GITHUB_SERVER_URL` without it, so `actions/checkout` builds an
unreachable clone URL (`https://<host>/<owner>/<repo>` instead of
`https://<host>/git/<owner>/<repo>`). As the mirror is public, the Gitea
workflow therefore clones the repository explicitly with `git` instead of
`actions/checkout`; the GitHub workflow keeps `actions/checkout`. The evidence
markers are identical in both files (norm N4/N6).

The pipeline stages (identical for both forges and `make ci-local`):

1. `format` / `lint` (clang-format, clang-tidy)
2. `build` host (Debug + Release) and unit tests (`ctest`)
3. `simulator-smoke` (boot `frick_fsw`, check apps registered + TM produced)
4. `verify-lean` (`lake build`)
5. `trace` (traceability convergence, norms N3/N4/N10/N11)
6. `rtems-cross-build` (RTEMS 6.2 / riscv-mpfs64imafdc cross compilation)
7. `docs` (Doxygen + required-document list + document-id uniqueness + doc-set consistency)

### CI evidence markers (norm N4/N6)

Each CI job carries a comment that `tools/scripts/trace.py` consumes to prove
that a `ci:<name>` evidence claim in the manifest is produced by a real job:

```yaml
# evidence: ci-host-build, ci-dual-build (SRS-REQ-016, SSS-REQ-011)
```

Rules: a marker line may list several claim names (comma or space separated);
each name may be written as `ci-<name>` or `<name>`; the trailing parenthetical
reference list is documentation only. `trace.py` fails the gate if a manifest
`ci:<name>` claim has no matching marker in either workflow.

## 5. Keeping this document current

Any change to the approach, to a tool, to its version or to its role MUST
update this document and the qualification register in the same commit
(norm N8/N10). The `docs` and `trace` CI jobs verify the referenced files
exist.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial approach and tooling document (norm N8/N10) |
| 0.2 | 2026-10-10 | frick-fsw | Add qualified V&V tools T12 (`coverage.py`), T13 (`metrics.py`) to the inventory |
| 0.3 | 2026-10-10 | frick-fsw | M3: `docs` CI job now checks the required-document list and document-id uniqueness; HAL added to the approach |
| 0.4 | 2026-10-10 | frick-fsw | M6: platform `libm` mathematics facade added to the tools |
| 0.5 | 2026-10-10 | frick-fsw | Open-source RTEMS 6.2 alignment + reproducible toolchain image (T18) and Renode V&V tools (T17) added to the inventory |
