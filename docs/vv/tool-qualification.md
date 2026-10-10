# V&V Tool and Test-Script Qualification Register

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TOOLS-001                     |
| Revision                          | 0.1.9                                  |
| Date                              | 2026-10-10                             |
| Applicable norms                  | N6, N10, N11                           |

Norm N10 requires every test script / V&V tool to be **qualified** per
ECSS-Q-ST-80C (tool and tool qualification) and ECSS-E-ST-40C (verification
tools derived from standards). For criticality B the minimum qualification of a
script-level tool is: source review + inspection + configuration management; a
change in the tool invalidates previously produced evidence until
re-qualification.

Independence of the qualification review is required when stated beside the
tool, or always when targeting criticality A (norm N9 profiles R2).

## Register

| ID  | Tool / script                           | Location                        | Used for evidence of            | Qualification level        | Review (independence)                    | Status |
| --- | --------------------------------------- | ------------------------------- | ------------------------------- | -------------------------- | ---------------------------------------- | ------ |
| T1  | Docker V&V image                        | `tools/docker/Dockerfile`       | reproducibility of all evidence | reviewed + CM              | internal (R2 on changes)                | QUALIFIED |
| T2  | Unit test harness `ff_test.h`           | `tests/ff_test.h`               | unit test results               | reviewed + CM              | internal                                | QUALIFIED |
| T3  | CTest / unit tests                      | `tests/unit/*.c`                | SRS/SSS test evidence           | reviewed + CM              | internal; independent at release (R2)   | QUALIFIED |
| T4  | Native simulator smoke                 | `tools/scripts/run_sim.sh`      | integration/smoke results       | reviewed + CM              | internal                                | QUALIFIED |
| T5  | Lean 4 / lake / formal theorems         | `verification/lean/`            | norm N1 machine-checked proofs  | tool is self-checking; sources reviewed | review of theorem statements (R1/R2) | QUALIFIED |
| T6  | Traceability gate                       | `tools/scripts/trace.py`        | norm N4 convergence             | reviewed + CM              | independent (R2), qualified once, reviewed at change | QUALIFIED |
| T7  | Ground station `ff_gsw.py`              | `tools/gsw/ff_gsw.py`           | TC injection / TM decode logs   | reviewed + CM              | independent (R2) on changes             | QUALIFIED |
| T8  | clang-tidy static analysis              | `.clang-tidy`, CI job            | defect / style evidence         | reviewed + CM              | internal                                | QUALIFIED |
| T9  | Simulator bootstrap                     | `tools/scripts/run_sim.sh`      | boot and TM evidence            | reviewed + CM              | internal                                | Traced to T4, QUALIFIED |
| T10 | RTEMS toolchain bootstrap               | `tools/scripts/setup_rtems.sh`  | RTEMS cross toolchain           | upstream-tested + CM       | internal                                | QUALIFIED |
| T11 | CI workflows (GH Actions / Gitea)       | `.github/workflows`, `.gitea/workflows` | pipeline evidence `ci:*` | reviewed + CM              | internal                                | QUALIFIED |
| T12 | Coverage collector                      | `tools/scripts/coverage.py`   | code line coverage metrics    | reviewed + CM              | internal (R2 on changes)                | QUALIFIED |
| T13 | Metrics panel generator                 | `tools/scripts/metrics.py`    | README coverage + V&V panel   | reviewed + CM              | internal (R2 on changes)                | QUALIFIED |

## Qualification procedure (per tool)

1. The tool/script is placed under configuration management in this repository.
2. Source review for correctness and for traceability to the evidence claims it
   supports (norm N10).
3. If the risk or the class-A target demands it, the review is independent
   (norm N9, profile R2) and recorded in `docs/reviews/`.
4. The tool is exercised in the CI pipeline; a recorded pass is the
   qualification evidence.
5. Any change to a qualified tool/script invalidates its qualification: the
   review is repeated and the register updated in the same commit (norm N11).

## Evidence claims mapping

| Qualification ID | Evidence claims it supports |
| ---------------- | --------------------------- |
| T1               | `ci:*` reproducibility for all jobs |
| T5               | `lean` links in `traceability.yaml` |
| T6               | norms N3, N4 convergence gate |
| T7               | GSW-related test (`test_pus` S17 ping, TC injection) |
| T3               | `tests/unit/*.c` links |
| T8               | `ci:static-analysis` |
| T10              | `ci:rtems-cross-build` |
| T11              | `ci:ci-pipeline`, `ci:dual-build`, `ci:trace`, `ci:docs`, `ci:host-build`, `ci:simulator-smoke` |

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial register T1-T11 (norm N10) |
| 0.1.1 | 2026-10-10 | frick-fsw | Re-qualification after review-closure changes: T6 (`trace.py`) evidence-marker parser hardened and independently re-checked; T7 (`ff_gsw.py`) exercised via `--ping` self-check; T3 (`test_pus.c` S6 case) corrected and re-run |
| 0.1.2 | 2026-10-10 | frick-fsw | Re-qualification after CI deployment fix: T1 image gains `nodejs`/`npm` (needed by Gitea `actions/checkout`); T11 workflows' smoke banner check corrected and format gate re-satisfied (`clang-format` 18 over all C sources) |
| 0.1.3 | 2026-10-10 | frick-fsw | Re-qualification after CI fix: Gitea workflow clones the mirror explicitly (sub-path `/git`); Doxygen output dir created. New qualified tools T12 (`coverage.py`, gcov) and T13 (`metrics.py`); doc-only changes in README/guides |
| 0.1.4 | 2026-10-10 | frick-fsw | Re-qualification after M2 review (R2 M2-03): T6 `trace.py` `ci_evidence()` scoping corrected so only `ci:`/`ci-` tokens count as claims (now 9 unique); the N6 undeclared-claim check can no longer be false-passed. T3 `test_time.c`/`test_pus.c` harness-entry and determinism defects corrected and re-run (R2 M2-01/02/06) |
| 0.1.5 | 2026-10-10 | frick-fsw | Re-qualification after M3: T3 gains `tests/unit/test_hal.c` (HAL registry + contract presence, 31 checks); T5 Lean gains `Frick.Hal`; T6 gate re-run (63 reqs, 45 theorems, 9 CI claims); docs job gains the document-id uniqueness check and the M3 required documents |
| 0.1.6 | 2026-10-10 | frick-fsw | Re-qualification after M6: T3 gains `tests/unit/test_math.c` (libm facade, 20 checks, tolerance-based) |
| 0.1.7 | 2026-10-10 | frick-fsw | Re-qualification after M7: T3 gains `tests/unit/test_sdls.c` (SDLS SA/key management, 81 checks incl. bounds and removal/reuse; verified under AddressSanitizer) |
| 0.1.8 | 2026-10-10 | frick-fsw | Re-qualification after M4 (increment 1): T3 gains `tests/unit/test_gem.c` (Cadence GEM register/descriptor logic, 23 checks, mock MMIO) |
| 0.1.9 | 2026-10-10 | frick-fsw | Re-qualification after M4 (increment 3): T3 gains `tests/unit/test_gem_tsn.c` (GEM native TSN: VLAN/ENST/TSU, 16 checks, mock MMIO) |