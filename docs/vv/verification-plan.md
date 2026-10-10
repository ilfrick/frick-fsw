# Software Verification and Validation Plan

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-PLAN-001                      |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Target criticality                | ECSS B (norm N5); class-A gaps tracked |
| Applicable norms                  | N1, N3, N4, N6, N8, N9, N10, N11, N12  |

## 1. Scope

This plan covers the software V&V of the frick-fsw framework from unit level to
system demonstration, including formal verification (norm N1) and the
qualification of V&V tools and test scripts (norm N10). It is executed in every
development and verification cycle, and the results are recorded in
`docs/vv/verification-results.md` and pushed with the cycle (norm N11).

## 2. V&V context per ECSS

- Verification against requirements: ECSS-E-ST-40C (software verification
  process) applied at levels **unit**, **integration**, **system** and
  **formal qualification**.
- Verification methods: Tst, Anl, Inp, Dem, MIR (see `ecss-criticality-b.md`).
- Product assurance: ECSS-Q-ST-80C (independence, tool qualification,
  non-conformance management).

## 3. V&V environment

Deployable by humans and agents, identical everywhere (norm N6/N8):

| Environment | Description |
| ----------- | ----------- |
| Host simulator | `frick_fsw` native build (POSIX threads) with loopback link |
| RTEMS cross build | RTEMS 6.2 `riscv/mpfs64imafdc` cross compilation (`make rtems`) |
| V&V container | `tools/docker/Dockerfile` - pinned toolchain |
| Gitea CI/CD | `act_runner` in Docker (`tools/docker/act_runner/`) |
| GitHub CI | `.github/workflows/ci.yml` |

All test scripts are versioned in the repository (norm N11) and qualified
(norm N10): see `docs/vv/tool-qualification.md`.

## 4. Verification activities and methods

### Unit level (Tst / Anl)
- `tests/unit/test_ccsds.c` - CCSDS packet build/parse, CRC, length bounds.
- `tests/unit/test_sb.c` - Software Bus publish/subscribe, FIFO, depth.
- `tests/unit/test_tbl.c` - table load/validate/activate lifecycle.
- `tests/unit/test_pus.c` - PUS dispatch, service 1 verification, S17 ping,
  memory dump region bound, unknown service handling.
- Execution: `ctest` in the build directory.

### Formal verification (Anl, norm N1)
- Lean 4 project `verification/lean/`; `lake build` must pass.
- Theorems are linked to requirements in the traceability manifest.

### Integration / system level (Dem / Inp)
- `tools/scripts/run_sim.sh` boots the native simulator; it must report 6
  registered applications and produce EVS->PUS service 5 telemetry on the
  loopback link (simulator smoke).
- `tools/gsw/ff_gsw.py` may be used to inject telecommands and decode telemetry
  for end-to-end checks.

### Static analysis (Inp)
- `clang-format` (format), `clang-tidy` (defects/style), code inspection for
  deterministic memory (no heap after init) - evidence `ci:static-analysis`.

### Traceability (Inp / Dem, norm N4)
- `tools/scripts/trace.py` gate run by `make trace` and by CI job `trace`.

### Milestone verification (MIR)
- Pre-release review with the independent review agents (norm N9) and NC
  closure loop (norm N12).

## 5. Verification criteria

| Criteria | Criterion satisfied when |
| -------- | ------------------------ |
| Build | host Debug+Release and RTEMS cross build succeed with no errors |
| Unit | every `tests/unit/*.c` test passes in `ctest` |
| Formal | `lake build` succeeds; requirement-theorem links resolve |
| Smoke | simulator boots, 6 apps registered, TM produced |
| Static | clang-tidy clean for the configured checks; no heap-after-init |
| Traceability | `make trace` exits 0 (no gaps, no dangling ids) |
| Qualification | every V&V tool used has a QUALIFIED entry in the register |
| Reviews | independent review reports stored and NCs closed (norm N9/N12) |

## 6. Independence

Any activity that the criticality class marks as independent (norm N9) is
performed by an independent-context agent with profile R1 or R2
(`docs/engineering/independent-review.md`). At class B, the V&V result
assessment and tool qualification changes are reviewed independently by R2.

## 7. Result recording

Each cycle updates `docs/vv/verification-results.md` with the executed
activities, environment (docker tag / commit hash), results and any NC opened;
the update is committed and pushed with the cycle (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial V&V plan (norm N6/N12) |