# AGENTS.md - Agent-oriented project guide

> frick-fsw: cFS-inspired flight software framework, PUS-C TM/TC on CCSDS Space
> Packets, target: Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS
> 6.2, target ECSS criticality B.
>
> This file is the agent entry point (norm N8): a fresh agent with no prior
> context must be able to build, test, verify and contribute to this project
> using only the repository contents and the commands below. Human entry point:
> `README.md`.

## 1. Ground rules (non-derogable, norm N1-N22)

- Maintain `docs/rules.md` and the enforcement summary. Key gates before any
  commit/push:
  1. `make build` and `make test` green.
  2. `make verify-lean` green (`lake build` in `verification/lean`).
  3. `make trace` green (bidirectional traceability, norm N4).
- Every change (any origin: fix, review NC, requirement delta) re-runs the full
  applicable cycle (norm N15) and updates the revision history (norm N16).
- **N21 controls work sequencing:** immediately correct functional bugs (bugs
  affecting correct or intended software functionality) and implement missing
  features committed for the current or previous sprints. Record non-functional
  findings as NCs for later review/disposition; they do not stop unrelated
  development. Do not expand a functional task into a broad wording/perfection
  review loop. This does not itself waive separate commit, release, safety,
  security, regulatory, or contractual gates, or close an NC.
- **N22 controls test design:** requirement-based and integration tests include
  applicable nominal and out-of-bound cases, each compared against explicit
  expected results. Renode is an emulator that can host integration tests; it
  is not itself a test type or evidence of requirement coverage merely because
  it boots or emits an aggregate PASS marker. See `docs/rules.md` for definitions
  and applicability/fidelity limits.
- All repo content (docs, comments, commit messages) is **English** (norm N2).
- Every change that touches behaviour, requirements, tooling or configuration
  must update the affected docs in the same commit (norm N8).
- Only the `aeronautics-space-fsw-engineer` agent may commit or push, and only
  after verifying all applicable gates/norms and completion of the full sprint
  scope (N7). No other agent/subagent may commit or push unless the user
  explicitly delegates that authority for a defined scope. The designated agent
  may not waive gates or approve risk/release. Record intermediate review/V&V
  evidence in the working tree immediately; retain failed-cycle reports and
  publish them with the complete compliant change, never by bypassing a failed
  gate (N11/N15).

## 2. Repository map

| Path | Purpose |
| ---- | ------- |
| `src/` | C framework: `core/pal` (PAL), `core/sb` (Software Bus), `core/evs`, `core/es`, `core/tbl`, `core/time`, `transport/ccsds`, `transport/link`, `pus`, `apps`, `hal` (HAL contracts), `drivers` (reference drivers), `math`, `security`, `bsp/mpfs`, `main` |
| `config/` | single configuration point (`fsw_config.h`) + mission tables + platform headers + `rtems/` RTEMS configuration table |
| `tests/` | unit tests (C, ctest) on `tests/ff_test.h` harness |
| `verification/lean/` | Lean 4 project: formal requirement theorems (norm N1) |
| `tools/` | scripts (`trace.py`, `coverage.py`, `metrics.py`, `setup_rtems.sh`, `run_sim.sh`, `renode_vv.sh`, `renode_env_selftest.sh`, `qemu_vv.sh`, `emulated_run.sh`), ground station (`gsw/ff_gsw.py`), docker V&V material (`docker/`, `docker/renode/`, `docker/rtems/`) |
| `assets/` | project marks (logo) for codename "Eärendil"; see `assets/README.md` |
| `docs/` | rules, requirements (SSS/SRS/traceability), design (SDD), architecture, ADRs, interfaces (ICD), quality (class B/A + compliance/CM/PA/safety), V&V, engineering (DDJF `docs/engineering/software-description.md` + independent review + driver/service/user guides), reviews, roadmap, **development plan**, status (**`handover.md` = resume point**) |
| `NOTICE` | copyright and license notice (Copyright (C) 2026 Ilfrick, norm N13) |
| `.github/workflows/`, `.gitea/workflows/` | CI pipelines |
| `board/` | device-tree sources (`mpfs-rtpfs160zt.dts`) |
| `cmake/toolchains/` | RTEMS 6.2 riscv/mpfs64imafdc toolchain file |

## 3. Quickstart (host simulator)

```bash
# containerized configure/build/test/Lean/trace cycle from a fresh clone:
make ci-local            # configure/build/test/Lean/trace only; no format, smoke, GSW, RTEMS, or docs
make smoke               # native simulator smoke test (separate)
make docs                # Doxygen generation (separate)
make gsw-selfcheck        # GSW, metrics, and N13 license-policy Python checks

# or without docker:
make configure           # cmake -S . -B build/native
make build               # native simulator + core library
make test                # ctest unit tests
make run                 # boot the native simulator (6 apps)
make verify-lean         # lake build in verification/lean
make trace               # python3 tools/scripts/trace.py docs/requirements/traceability.yaml
make coverage            # gcov line/function coverage -> build/coverage/coverage.json
make metrics             # regenerate the README status panel (coverage + semaphores)
make renode-env          # Renode V&V environment self-test (norm N18; needs docker)
make renode              # Renode functional validation (norm N18; SKIP proxy if the RTEMS image is absent)
make qemu-image          # generic riscv/rv64imafdc RTEMS image for QEMU (milestone M10)
make qemu                # QEMU `virt` emulated-target check (M10; SKIP proxy if the image is absent)
make pus-asn1            # generate the PUS-C ASN.1 (N7/ESA) default backend C (build-time; ASN1SCC)
                         # (default FF_USE_PUS_ASN1=ON; if the generated code is absent the
                         #  build warns and falls back to the open backend; -DFF_USE_PUS_ASN1=OFF forces open)
```

RTEMS cross build (open-source RTEMS 6.2; `tools/scripts/setup_rtems.sh` builds
the RSB toolchain into `/opt/rtems/6`, or use the Docker-based toolchain image.
Its mutable base/apt/RSB inputs do not establish bit-for-bit reproducibility
(NC-EXT-40):

```bash
make rtems-toolchain    # build the open RTEMS 6.2 toolchain image (T18)
make rtems-docker       # cross compile inside the image (no host toolchain)
make rtems              # cross compile for riscv/mpfs64imafdc (host /opt/rtems)
```

## 4. Deploying the CI/CD and V&V environments (norm N6)

The project is deployable everywhere through Docker:

```bash
# Common controlled V&V image (cmake, ninja, gcc, python3, node, lean, doxygen, graphviz, tidy); mutable inputs do not establish reproducibility (NC-EXT-35)
make docker-image        # builds frick-vv:local from tools/docker/Dockerfile

# Gitea CI/CD: self-hosted Gitea Actions runner (norm N6)
# Requires GITEA_RUNNER_REGISTRATION_TOKEN; maps `ubuntu-24.04` to frick-vv:local.
docker compose -f tools/docker/act_runner/docker-compose.yml up -d
```

The runner executes `.gitea/workflows/ci.yml` on the Gitea mirror. GitHub
Actions separately executes `.github/workflows/ci.yml`. The workflows contain
platform-specific differences; both currently invoke the N13 license checker,
but neither run is evidenced here as hosted qualification for T21.

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
7. For a required **documentation review** (N17), scope it to the changed
   product. Correct functional defects; record non-functional findings as NCs
   for later disposition under N21. Do not repeat broad review cycles to
   eliminate non-functional wording findings as a condition of continuing
   unrelated development. Commit/release approval remains subject to its
   separately applicable gates.
8. For required **product assurance** (N20), record findings and their scope.
   Correct functional bugs; non-functional findings are tracked as open NCs for
   later review/disposition under N21. Escalate any immediate safety/security
   concern to the authorized human authority. An NC is not closed or passed by
   deferral.
9. **Do not commit or push unless you are the `aeronautics-space-fsw-engineer`
   agent or have explicit user delegation.** The designated FSW engineer commits
   and pushes to both `origin` (GitHub) and `gitea` only after all applicable
   gates/norms pass and the full sprint scope is complete (N7).

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
| N7 | Only `aeronautics-space-fsw-engineer` commits/pushes after all gates/norms and full sprint scope pass; explicit user delegation required for exceptions |
| N8 | Complete, current, deployable documentation (humans + agents) |
| N9 | Independent reviews by independent-context agents (R1/R2/R3/R4 + PA (N20) profiles) |
| N10 | Test scripts / V&V tools reviewed & qualified per ECSS |
| N11 | Test scripts and review results retained immediately; committed/pushed with each completed compliant cycle |
| N12 | Non-conformances resolved by re-applying all rules to full compliance |
| N13 | Owner-authorized MPL-2.0 for original project content; third-party/generated boundaries documented and reviewed |
| N14 | SSS requirements reviewed by Senior Systems Engineer (R3); NCs to closure |
| N15 | Every change re-runs the complete development+V&V cycle to success |
| N16 | Detailed revision history maintained (CHANGELOG + per-doc history + commits) |
| N17 | Documentation reviewed by the Senior SW documentation reviewer (R4) on every change; review loop closed before commit/push |
| N18 | Functional validation of every change passes in the Renode emulated PolarFire SoC environment (**in force**; real open-RTEMS image verified) |
| N19 | At the start of every development loop, `docs/status/loop-audit.md` audits conformance of the declared objectives (milestones) and all norms N1-N22 against the actual implementation; every gap is closed or tracked with owner/milestone |
| N20 | Every phase product is independently reviewed also by the Senior SW Product Assurance engineer (`aeronautics-space-sw-pa-engineer`); findings are corrected and the reviews repeated until they pass |
| N21 | Correct functional bugs and implement missing sprint-committed features; log other findings as NCs for later review/disposition; do not stop unrelated development; preserve separate release/safety/security gates |
| N22 | Requirement-based and integration tests cover nominal and applicable out-of-bound operation against explicit expected results; Renode is an emulator, not a test type |
