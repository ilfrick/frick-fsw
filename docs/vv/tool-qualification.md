# V&V Tool and Test-Script Qualification Register

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TOOLS-001                     |
| Revision                          | 0.3.16                                 |
| Date                              | 2026-10-12                             |
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
| T3  | CTest / unit tests                      | `tests/unit/*.c` (incl. `test_r2_gap.c`, `test_r3_gap.c`, `test_nvm.c`, `test_mtd.c`, `test_can.c`) | SRS/SSS test evidence | reviewed + CM | internal; independent at release (R2) | QUALIFIED |
| T4  | Native simulator smoke                 | `tools/scripts/run_sim.sh`      | integration/smoke results       | reviewed + CM              | internal                                | QUALIFIED |
| T5  | Lean 4 / lake / formal theorems         | `verification/lean/`            | norm N1 machine-checked proofs  | tool is self-checking; sources reviewed | review of theorem statements (R1/R2) | QUALIFIED |
| T6  | Traceability gate                       | `tools/scripts/trace.py`        | norm N4 convergence             | reviewed + CM              | independent (R2), qualified once, reviewed at change | QUALIFIED |
| T7  | Ground station `ff_gsw.py`              | `tools/gsw/ff_gsw.py`           | TC injection / TM decode logs   | reviewed + CM              | independent (R2) on changes             | QUALIFIED |
| T8  | clang-tidy static analysis              | `.clang-tidy`, CI job            | defect / style evidence         | reviewed + CM              | internal                                | QUALIFIED |
| T9  | Simulator bootstrap                     | `tools/scripts/run_sim.sh`      | boot and TM evidence            | reviewed + CM              | internal                                | Traced to T4, QUALIFIED |
| T10 | RTEMS toolchain bootstrap (open RTEMS project) | `tools/scripts/setup_rtems.sh`  | open-source RTEMS 6.2 cross toolchain + BSP | upstream-tested + CM       | internal                                | QUALIFIED |
| T11 | CI workflows (GH Actions / Gitea)       | `.github/workflows`, `.gitea/workflows` | pipeline evidence `ci:*` | reviewed + CM              | internal                                | QUALIFIED |
| T12 | Coverage collector                      | `tools/scripts/coverage.py`   | code line coverage metrics    | reviewed + CM              | internal (R2 on changes)                | QUALIFIED |
| T13 | Status-panel generator (completion + coverage + semaphores) | `tools/scripts/metrics.py`    | README completion/coverage panel + V&V semaphores | reviewed + CM | internal (R2 on changes); R2 source review FRICK-REVIEW-0070 + PA approval FRICK-REVIEW-0068/0069/0071 | QUALIFIED |
| T14 | MISRA C:2012 scan (cppcheck addon)      | `tools/scripts/misra.sh`      | MISRA findings report         | reviewed + CM              | internal (advisory; certified checker = GAP-A4) | QUALIFIED |
| T15 | Renode (PolarFire SoC emulator)         | `tools/scripts/emulated_run.sh` | emulated boot/driver evidence | reviewed + CM              | internal (advisory; not silicon)        | QUALIFIED |
| T16 | QEMU RISC-V (`riscv/rv64imafdc`)        | `tools/scripts/emulated_run.sh` | emulated RTEMS boot evidence  | reviewed + CM              | internal (advisory; generic virt)       | QUALIFIED |
| T17 | Renode V&V environment (norm N18)       | `tools/scripts/renode_env_selftest.sh`, `renode_vv.sh`, `tools/docker/renode/`, `src/apps/app_renode_test.c` | env self-test PASS; functional run of the open RTEMS image: `RENODE_TEST: PASS` (incl. HAL checks `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm`/`hal_can`); UART console + full Renode process log retained under `build/renode/` | reviewed + CM | independent R2 on change; not silicon | QUALIFIED |
| T18 | Open RTEMS 6.2 toolchain image          | `tools/docker/rtems/Dockerfile`, `setup_rtems.sh` (`make rtems-toolchain`/`rtems-docker`) | open-source RTEMS 6.2 toolchain + `mpfs64imafdc` BSP (flight/Renode) and `rv64imafdc` BSP (QEMU) built; real cross build OK (plain + Renode + generic image) | reviewed + CM              | internal (RTEMS project upstream)       | QUALIFIED (see note) |
| T19 | ASN1SCC PUS-C code generation           | `tools/scripts/gen_pus_asn1.sh`, `tools/docker/asn1scc/` (`make pus-asn1`) | C generated from the N7/ESA `asn1-pusc-lib` ASN.1/ACN model | reviewed + CM | ESA ASN1SCC (GPL+runtime exception) | QUALIFIED |
| T20 | QEMU emulated-target functional check (M10) | `tools/scripts/qemu_vv.sh`, `cmake/toolchains/rtems-riscv-generic.cmake` (`make qemu-image`/`qemu-image-docker`, `make qemu`) | generic `rv64imafdc` image boots under `qemu-system-riscv64 -M virt -bios`: banner + 6 applications registered | reviewed + CM | independent R2 on change; not silicon | QUALIFIED |
| —   | Evidence-claims mapping                 | `ci:renode` -> T17 (Renode V&V environment); `ci:emulated-run` -> T15/T16 (advisory CI step) and T20 (nightly QEMU job `.github/workflows/nightly.yml`) | Renode functional run (`RENODE_TEST: PASS`, HAL checks); QEMU `virt` generic-BSP boot | — | — | — |

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

**T18 note (reproducibility caveat).** The `frick-rtems-toolchain:local` image
that produced the recorded M10 QEMU pass was assembled by applying the updated
`Dockerfile`'s steps individually (the extra `riscv/rv64imafdc` kernel-bset build
and the `qemu-system-misc`/`cmake`/`ninja-build` install), each executed and
verified in this environment; a single clean `docker build` of the updated
`Dockerfile` was not completed locally. The clean, reproducible build is provided
by `make rtems-toolchain`; re-qualification (a clean image build + re-run of
`make qemu`/`make renode`) is tracked as a closure action.

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
| T13              | README completion/coverage panel (CI currency gate: `make coverage` + `metrics.py --check`; the non-coverage panel must match, coverage-row drift is warned) |
| T17              | `ci:renode` (Renode V&V environment; functional run + HAL checks) |
| T20              | `ci:emulated-run` (QEMU `virt` generic-BSP functional check) |
| T19              | `ci:pus-asn1` / PUS-C ASN.1 generation for `ci:renode` (SRS-REQ-050) |

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
| 0.2.0 | 2026-10-10 | frick-fsw | Re-qualification after M4 (increment 4): T3 gains `tests/unit/test_netloop.c` (reference loopback network stack, 29 checks) |
| 0.2.1 | 2026-10-10 | frick-fsw | Re-qualification after M4 (increment 5): T3 gains `tests/unit/test_lwip.c` (lwIP adapter TCP round trip, 19 checks; built with FF_USE_LWIP + fetched lwIP) |
| 0.2.2 | 2026-10-10 | frick-fsw | M9: new qualified tool T14 (`tools/scripts/misra.sh`, cppcheck MISRA scan, advisory) |
| 0.2.3 | 2026-10-10 | frick-fsw | M10: new qualified tools T15 (Renode PolarFire SoC) and T16 (QEMU RISC-V), `tools/scripts/emulated_run.sh` (advisory) |
| 0.2.4 | 2026-10-10 | frick-fsw | M10/N18: new qualified tool T17 (Renode V&V environment, `renode_vv.sh` + `tools/docker/renode/`) |
| 0.2.5 | 2026-10-10 | frick-fsw | Open RTEMS alignment: T10 updated (open-source RTEMS project) and new qualified tool T18 (`tools/docker/rtems/Dockerfile`, containerised open RTEMS toolchain + BSP) |
| 0.2.6 | 2026-10-10 | frick-fsw | T18 built and exercised (real cross build OK); T10/T17 evidence updated (real Renode functional run `RENODE_TEST: PASS`) |
| 0.2.7 | 2026-10-10 | frick-fsw | M4/M11: T3 gains `tests/unit/test_gpio.c` (MSS GPIO, 21 checks, mock MMIO); Renode run also checks `hal_gpio=PASS` |
| 0.2.8 | 2026-10-10 | frick-fsw | M4/M11: T3 gains `tests/unit/test_serial.c` (MSS MMUART, 30 checks, mock MMIO); Renode run also checks `hal_serial=PASS` |
| 0.2.9 | 2026-10-11 | frick-fsw | M4/M11: T3 gains `tests/unit/test_mem.c` (RAM pool + DMA, 20 checks); Renode run also checks `hal_mem=PASS` |
| 0.3.0 | 2026-10-11 | frick-fsw | N19/PUS: new qualified tool T19 (ASN1SCC PUS-C code generation, `gen_pus_asn1.sh` + `tools/docker/asn1scc/`) |
| 0.3.1 | 2026-10-11 | frick-fsw | Re-qualification: T17 now also covers the Renode testware `src/apps/app_renode_test.c` (HAL checks), and the console is captured to a container-local path + `docker cp` (robustness) |
| 0.3.2 | 2026-10-11 | frick-fsw | Evidence-claims mapping adds T17 (`ci:renode`) and T19 (PUS-C ASN.1); header date refreshed (PA-11) |
| 0.3.3 | 2026-10-11 | frick-fsw | Re-qualification after the N12 delta: T3 `test_netloop` grows to 328 checks (boundary/negative paths, exhaustion, full-ring partial send, cross-socket duplicate bind) |
| 0.3.4 | 2026-10-11 | frick-fsw | Re-qualification: T13 `metrics.py` now also computes the completion/metrics panel (sprints, SSS/SRS/design/review completion + coverage) rendered in README; definitions under PA review (N20) |
| 0.3.5 | 2026-10-11 | frick-fsw | T13: single consolidated panel; PA findings F1-F3 addressed (strict verification, Lean coverage restored, review-stream keying); CI currency gate added; PA approval FRICK-REVIEW-0068 |
| 0.3.6 | 2026-10-11 | frick-fsw | T13: address PA delta D1 + R2 source-review Minors (canon M1/M10, derived coverage %, proxy label, sprint source, gate wording); cite FRICK-REVIEW-0070; PA final approval FRICK-REVIEW-0071 |
| 0.3.7 | 2026-10-11 | frick-fsw | T3 gains the R2/R3 test-gap suites `test_r2_gap.c` (487 checks) + `test_r3_gap.c` (198 checks) covering uncovered functions/branches and requirement-level flows (FRICK-REVIEW-0075/0076) |
| 0.3.8 | 2026-10-11 | frick-fsw | T3 re-qualification after the FRICK-REVIEW-0077 fix: TBL regression added (`test_r3_gap` 198 -> 201), `test_r2_gap` made order-independent; source `tbl.c` fix + `FF_PusInit` idempotence |
| 0.3.9 | 2026-10-11 | frick-fsw | T3 gains `tests/unit/test_nvm.c` (reference eNVM NVM backend, SRS-REQ-055); T17 Renode functional check gains `hal_nvm` |
| 0.3.10 | 2026-10-11 | frick-fsw | T17 row lists `hal_nvm`; T3 `test_nvm` 29 -> 36 checks; header/History reconciled (FRICK-REVIEW-0081/0082/0083/0084) |
| 0.3.11 | 2026-10-11 | frick-fsw | T3 gains `tests/unit/test_mtd.c` (NOR/QSPI MTD, SRS-REQ-056) |
| 0.3.12 | 2026-10-11 | frick-fsw | Re-qualification: T17 `frick-renode.resc` and `renode_env.resc` now load the BeagleV-Fire board repl (QSPI NOR + system services); header/History reconciled |
| 0.3.13 | 2026-10-11 | frick-fsw | T3 gains `tests/unit/test_can.c` (reference MSS CAN driver, SRS-REQ-057); T17 Renode functional check gains `hal_can` |
| 0.3.14 | 2026-10-11 | frick-fsw | Re-qualification: T17 `renode_vv.sh` now also retains the full Renode process log (`build/renode/renode-process.log`) next to the UART console (evidence retention, PA-0109-06) |
| 0.3.15 | 2026-10-11 | frick-fsw | M10: new qualified tool T20 (QEMU generic-BSP functional check, `qemu_vv.sh`); T18 builds the `rv64imafdc` BSP (QEMU) alongside `mpfs64imafdc`; `ci:emulated-run` mapped to T15/T16/T20 |
| 0.3.16 | 2026-10-12 | frick-fsw | Review NCs: `ci:emulated-run` mapping clarified (T20 exercised by the nightly QEMU job); T18 reproducibility caveat recorded (clean `make rtems-toolchain` build tracked) |
