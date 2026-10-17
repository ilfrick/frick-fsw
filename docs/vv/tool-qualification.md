# V&V Tool and Test-Script Qualification Register

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TOOLS-001                     |
| Revision                          | 0.3.86                                 |
| Date                              | 2026-10-16                             |
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
| T1  | Docker V&V image                        | `tools/docker/Dockerfile`       | common container environment for specified jobs | reviewed + CM | internal (R2 on changes) | Graphviz added; local clean serialized Doxygen run passed with 107 warnings per FRICK-REVIEW-0193. Mutable base/package/tool sources remain (NC-EXT-35); hosted GitHub/Gitea execution/evidence remains open (NC-EXT-36); not fully requalified. |
| T2  | Unit test harness `ff_test.h`           | `tests/ff_test.h`               | unit test results               | reviewed + CM              | internal                                | QUALIFIED |
| T3  | CTest / unit tests                      | `tests/unit/*.c` (incl. `test_r2_gap.c`, `test_r3_gap.c`, `test_link_uart.c`, `test_nvm.c`, `test_mtd.c`, `test_can.c`, `test_i2c.c`, `test_spi.c`, `test_block.c`, `test_lwip_netif.c`, `test_sb_stress.c`, `test_sb_sweep.c`) | SRS/SSS test evidence | reviewed + CM | internal; independent at release (R2) | Host-PTY UART framing test exercises production `link_uart.c`; latest focused test reports 51 checks / 0 failures, including disconnected-terminal handling and unique temporary-path setup. Full candidate `make ci-local` passes 37/37 CTest. Independent V&V 0319 and R1 0326 accept only enumerated source/host scopes; malformed plausible-PDL recovery remains authority-blocked. RTEMS cross-build passed, but target runtime/timing remain open; formal tool-owner N10 acceptance and release-specific independent evidence remain pending; no mission-compliance credit. |
| T4  | Native simulator smoke                 | `tools/scripts/run_sim.sh`      | integration/smoke results       | reviewed + CM              | internal                                | Author-retested cached/fresh CMake-generator paths; independent delta qualification review pending (S8-FU) |
| T5  | Lean 4 / lake / formal theorems         | `verification/lean/`            | norm N1 machine-checked proofs  | tool is self-checking; sources reviewed | review of theorem statements (R1/R2) | QUALIFIED |
| T6  | Traceability gate                       | `tools/scripts/trace.py`        | norm N4 convergence             | reviewed + CM              | independent (R2), qualified once, reviewed at change | QUALIFIED |
| T7  | Ground station `ff_gsw.py`              | `tools/gsw/ff_gsw.py`; `tests/python/test_ff_gsw.py` | TC injection / TM decode logs | reviewed + CM + fixed golden vectors | R2 FRICK-REVIEW-0317 (bounded host delta only) | R2 accepted the bounded incremental serial parser and deadline-limited exact S17.2 echo reader; 12/12 GSW tests. Plausible incomplete candidates can cause bounded head-of-line delay; response freshness/correlation and independent ground-segment E2E are not established. T7 remains NOT FULLY QUALIFIED. |
| T8  | clang-tidy static analysis              | `.clang-tidy`, CI job            | defect / style evidence         | reviewed + CM              | internal                                | QUALIFIED |
| T9  | Simulator bootstrap                     | `tools/scripts/run_sim.sh`      | boot and TM evidence            | reviewed + CM              | internal                                | Traced to T4; author-retested cached/fresh generator paths; independent delta review pending |
| T10 | RTEMS toolchain bootstrap (open RTEMS project) | `tools/scripts/setup_rtems.sh`  | open-source RTEMS 6.2 cross toolchain + BSP | upstream-tested + CM       | internal                                | QUALIFIED |
| T11 | CI workflows (GH Actions / Gitea)       | `.github/workflows`, `.gitea/workflows` | pipeline evidence `ci:*` | reviewed + CM              | internal                                | Existing workflow baseline qualified; this loop's fetch-before-configure changes are pending hosted execution and independent delta review. |
| T12 | Coverage collector                      | `tools/scripts/coverage.py`   | code line coverage metrics    | reviewed + CM              | internal (R2 on changes)                | QUALIFIED |
| T13 | Status-panel generator (completion + coverage + semaphores) | `tools/scripts/metrics.py`; `tests/python/test_metrics.py` | README completion/coverage panel + V&V semaphores | reviewed + CM | internal (R2 on changes); R2 source review FRICK-REVIEW-0070 + PA approval FRICK-REVIEW-0068/0069/0071 | Registry/product-scope deferral distinction; verdict classifier rejects known scoped/open/pending/contradictory outcomes; 9 T13-focused tests pass (21 total Python tests include 12 GSW tests) and panel is current. FRICK-REVIEW-0221/0223/0224/0226/0227/0228 NOT APPROVED; 0229 is bounded only and 0230/0231 are scoped; none requalifies T13. Independent T13 qualification and NC-EXT-41 closure remain pending. |
| T14 | MISRA C:2012 scan (cppcheck addon)      | `tools/scripts/misra.sh`; `make misra-docker` (Ubuntu package install) | Advisory MISRA findings report; checker/package versions are not pinned by this target | reviewed + CM              | internal (advisory; certified checker = GAP-A4) | QUALIFIED |
| T15 | Renode (PolarFire SoC emulator)         | `tools/scripts/emulated_run.sh` | emulated boot/driver evidence | reviewed + CM              | internal (advisory; not silicon)        | QUALIFIED |
| T16 | QEMU RISC-V (`riscv/rv64imafdc`)        | `tools/scripts/emulated_run.sh` | emulated RTEMS boot evidence  | reviewed + CM              | internal (advisory; generic virt)       | QUALIFIED |
| T17 | Renode V&V environment (norm N18)       | `tools/scripts/renode_env_selftest.sh`, `renode_vv.sh`, `tools/docker/renode/`, `src/apps/app_renode_test.c` | env self-test PASS; functional run of the open RTEMS image: `RENODE_TEST: PASS` (incl. HAL checks `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm`/`hal_can`/`hal_i2c`/`hal_spi`/`hal_block`/`hal_tsn`/`hal_pcie`; on the lwIP-enabled image also `hal_net_lwip`); UART console + full Renode process log retained under `build/renode/` | reviewed + CM | independent R2 on change; not silicon | QUALIFIED |
| T18 | Open RTEMS 6.2 toolchain image          | `tools/docker/rtems/Dockerfile`, `setup_rtems.sh` (`make rtems-toolchain`/`rtems-docker`) | open-source RTEMS 6.2 toolchain + `mpfs64imafdc` BSP (flight/Renode) and `rv64imafdc` BSP (QEMU) built; real cross build OK (plain + Renode + generic image) | reviewed + CM | internal (RTEMS project upstream) | **NOT FULLY REQUALIFIED**: prior QEMU evidence used a stepwise-assembled image; clean Docker build and repeat target checks pending (NC-EXT-40). |
| T19 | ASN1SCC PUS-C code generation           | `tools/scripts/gen_pus_asn1.sh`, `tools/docker/asn1scc/` (`make pus-asn1`) | Common ASN1SCC C runtime/types generated from pinned N7/ESA inputs and built/tested in default configuration | reviewed + CM | ESA ASN1SCC (GPL+runtime exception) | **NOT FULLY REQUALIFIED** — generation scope/script changed in S8-PUS-FU; retain new logs and obtain applicable independent delta review before qualification claim |
| T20 | QEMU emulated-target functional check (M10) | `tools/scripts/qemu_vv.sh`, `cmake/toolchains/rtems-riscv-generic.cmake` (`make qemu-image`/`qemu-image-docker`, `make qemu`) | generic `rv64imafdc` image boots under `qemu-system-riscv64 -M virt -bios`: banner + 6 applications registered | reviewed + CM | independent R2 on change; not silicon | QUALIFIED |
| T21 | N13 canonical license / SPDX policy checker | `tools/scripts/license_check.py`; `tests/python/test_license_check.py`; both CI workflow checker steps and `make gsw-selfcheck` test step | exact MPL license-text digest, exact project SPDX expressions and initial comment headers for managed code/config/build/test/tool roots, with format syntax, block-comment state, XML prolog, shebang and fail-closed unreadable-managed-file handling | reviewed + CM | FRICK-REVIEW-0248 NOT APPROVED; FRICK-REVIEW-0249 bounded R2 pass for fail-closed correction; FRICK-REVIEW-0250..0257 NOT APPROVED for documentation/status findings; 0258/0259 bounded no-new-finding status checks only; hosted/configuration evidence and immutable identity pending | **NOT FULLY QUALIFIED** — NC-EXT-43/44 |
| T22 | SB/EVS structural sweep runner | `tools/scripts/sb_evs_structural_sweep.py`; `tests/unit/test_sb_sweep.c`; CTest `sb_evs_structural_sweep`; `make sb-evs-sweep` | CSV of fanout/depth/overflow-burst profiles and dispatch-cap checks; expressly exploratory structural characterization | Source review + inspection + CM; independent R2 FRICK-REVIEW-0271/0274/0275, R4 0276/0278 and PA 0277/0279; full runner registered in CTest and exercised by `make ci-local` | internal; no requirements/mission-compliance credit | 9,984-profile matrix and dispatch bounds 1–64 passed manually and through CTest in the local CI container; exact source/configuration/executable/CSV/log identities are in the evidence note. Local CI exercise and scoped review are complete; hosted CI and immutable build inputs remain open. This is not formal mission-compliance evidence. Initial CSV formatter defect/correction recorded (malformed CSV overwritten, not retained separately). NC-EXT-75/76 remain open |
| T23 | Pinned third-party fetch script | `tools/scripts/fetch_third_party.sh`; `make fetch-third-party` | Fetch lwIP and PUS ASN.1 inputs; new `obcp-only` mode fetches pinned n7s-obcp + MicroPython submodule without FreeRTOS test submodule | Source review + dependency-state probes + CM | Internal; bounded behavior deltas reviewed (FRICK-REVIEW-0298/0299/0300); this dependency/mode delta has not been independently reviewed; hosted workflow evidence remains pending | Existing probes cover lwIP wrong-HEAD/tracked/untracked/ignored cases. S9-2026-10-16 records a successful OBCP parent/submodule fetch and immutable identities. Not evidence of engine build, RTEMS/RISC-V portability, or OBCP operation. T23 remains not qualified. |
| T24 | clang-format wrapper (`make format` / `format-check`) | `Makefile`, pinned clang-format 18 in `frick-vv:local` | Apply/check repository C formatting | reviewed + CM; bounded independent FSW functional review; formal N10 qualification open | Internal; pinned-container test | The previous `make format` command passed directory arguments directly to clang-format and failed; corrected to `find ... -print0 | xargs -0 clang-format -i`. Final pinned transcript demonstrates `make format` then `make format-check` pass. Host clang-format 23 is not an equivalent pinned run. Empty-match behavior is not evidenced; no formal qualification claim. |
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

**T18 note (build-provenance limitation).** The `frick-rtems-toolchain:local` image
that produced the recorded M10 QEMU pass was assembled by applying the updated
`Dockerfile`'s steps individually (the extra `riscv/rv64imafdc` kernel-bset build
and the `qemu-system-misc`/`cmake`/`ninja-build` install), each executed and
verified in this environment; a single clean `docker build` of the updated
`Dockerfile` was not completed locally. `make rtems-toolchain` provides the clean
image-build procedure, but the base image, apt package resolution, and RSB
reference are mutable; this does not establish bit-for-bit reproducibility.
The recorded image was assembled stepwise, so T18 requalification remains open
pending a clean image build and repeat of `make qemu`/`make renode` under NC-EXT-40.

## Evidence claims mapping

| Qualification ID | Evidence claims it supports |
| ---------------- | --------------------------- |
| T1               | Common Docker environment for jobs that use T1; does not establish cross-build reproducibility while NC-EXT-35 remains open |
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
| 0.3.17 | 2026-10-12 | frick-fsw | T3 gains `tests/unit/test_i2c.c` (reference MSS I2C driver, SRS-REQ-058); T17 Renode functional check gains `hal_i2c` |
| 0.3.18 | 2026-10-12 | frick-fsw | T3 `test_i2c` 135 -> 144 checks (1 MHz code-5 mapping, minimum-SCL boundary, bus-hook validation); T14 reproducibility: `make misra-docker` runs `misra.sh` in a pinned image (regenerates `build/misra-report.txt`; 13 advisory findings, 0 in I2C); header/History reconciled (FRICK-REVIEW-0129..0136) |
| 0.3.19 | 2026-10-12 | frick-fsw | T3 gains `tests/unit/test_spi.c` (reference MSS SPI driver, SRS-REQ-069); T17 Renode functional check gains `hal_spi` |
| 0.3.20 | 2026-10-12 | frick-fsw | T3 gains `tests/unit/test_block.c` (reference MSS block-device driver, SRS-REQ-070); T17 Renode functional check gains `hal_block` |
| 0.3.21 | 2026-10-12 | frick-fsw | T17 Renode functional check gains `hal_tsn` (GEM native TSN peripheral-bound, SRS-REQ-038, M11/S6a) |
| 0.3.22 | 2026-10-12 | frick-fsw | T10 `ci:rtems-cross-build` re-qualified: the script now also cross-builds with `FF_USE_LWIP=ON` (in-repo RTEMS `sys_arch` port, S6b-1) when the pinned lwIP tree is present (FRICK-REVIEW-0150 R2-02 / 0151 NC-151-02) |
| 0.3.23 | 2026-10-12 | frick-fsw | T17 gains `hal_net_lwip` (lwIP netif bound to the emulated GEM on the lwIP-enabled image, S6b-2); T10 lwIP leg gains the Renode test app + `nm` link check |
| 0.3.24 | 2026-10-12 | frick-fsw | NET follow-up: T3 list gains `test_lwip_netif.c` (host netif glue test against a mock `FF_EthMac_t`, 26/26 with `FF_USE_LWIP=ON`, ASan/UBSan clean; closes NC-154-01); T17 covers the host netif loopback as the TX/RX data-path evidence for the register-only GEM (NC-S6-01/NC-S6-03); T3 `test_gem` extended to assert 64-bit register writes (TBQP_HI/RBQP_HI + ADDR64) |
| 0.3.25 | 2026-10-13 | frick-fsw | S4b follow-up: **T3 re-qualified** (two test scripts under T3 changed) — `test_link_ip` gains `test_pus_ip_roundtrip` (PUS-C TM/TC round-trip over the IP link backend; 20 funcs / 77 checks) and `test_link_tsn` gains `test_pus_tsn_roundtrip` (over the TSN backend with the mock MAC looped back; 15 funcs / 135 checks), the `Dem` evidence for SRS-REQ-060/061 (NC-S4b-02). No *tool* under `tools/` changed, and no T3 criterion/oracle changed beyond the two scripts. |
| 0.3.26 | 2026-10-13 | frick-fsw | S4b follow-up review loop (FRICK-REVIEW-0167/0168/0169): **T3 re-qualified again** (authoring corrections to the same test scripts + the strict-warning gate). The test targets now genuinely compile under `-Wall -Wextra -Werror` (`tests/CMakeLists.txt`), which required four pre-existing test-script defects to be corrected (`test_link_tsn.c` sign-compare, `test_spacewire.c` unused variable + operator precedence, `test_time.c` tautological range asserts, `test_gptp.c` unused static helper). `test_link_ip` 20 funcs / **79** checks (was 77 — IP send-leg assertion added), `test_time` 99 checks (was 101 — tautological asserts removed). No tool under `tools/` changed. |
| 0.3.27 | 2026-10-13 | frick-fsw | Independent assessment follow-up: T4/T9 (`run_sim.sh`) re-qualified after fixing the forced-Ninja generator conflict; verified with both cached Unix Makefiles and a fresh build directory. T7 (`ff_gsw.py`) re-qualified for TC construction/TM parsing against fixed golden PUS-C vectors; aligned CCSDS/PUS headers and offsets. Added `tests/python/test_ff_gsw.py`, `make gsw-selfcheck` and CI execution. External segment-to-ground interoperability evidence remains open under NC-EXT-03. |
| 0.3.28 | 2026-10-13 | frick-fsw | T3 ASan/UBSan re-qualification after the sanitizer found an undersized mock register array in `test_gem_tsn.c`; mock range expanded to 4096 bytes (GEM ENST register block offset `0x0880`). Full sanitizer ctest now passes 27/27; test-fixture NC-EXT-09 recorded pending independent delta review. |
| 0.3.29 | 2026-10-13 | frick-fsw | T7 re-qualification delta: `decode_tm()` packet-relative payload slicing fixed after independent review found absolute stream-offset use; `tests/python/test_ff_gsw.py` now covers noise prefix and concatenated packets in addition to fixed golden vectors, truncation and CRC error. SPDX header added. Independent re-review remains pending; external ground-segment E2E remains out of scope/open. |
| 0.3.30 | 2026-10-13 | frick-fsw | T13 change corrects the legend to distinguish open/pending gates from explicitly deferred issues; README panel regenerated. Independent R2/PA requalification review pending under NC-EXT-10. |
| 0.3.31 | 2026-10-13 | frick-fsw | T1 V&V image adds graphviz after Doxygen reported missing graph-map files; independent R2 requalification pending. |
| 0.3.32 | 2026-10-13 | frick-fsw | Serialize clean Doxygen generation, include docs/Markdown inputs, and require Doxygen/Graphviz in both hosted CI workflows; record T1 reproducibility/CI findings NC-EXT-35/36 pending R2/PA. |
| 0.3.33 | 2026-10-13 | frick-fsw | Update T1 row from FRICK-REVIEW-0193 local-run evidence; remove `sudo apt-get` from Gitea Doxygen step because the job uses the root-based V&V container; hosted workflow evidence remains pending. |
| 0.3.34 | 2026-10-13 | frick-fsw | Correct Dockerfile reproducibility overclaim per FRICK-REVIEW-0197; NC-EXT-35 remains open pending immutable input control or approved claim scope. |
| 0.3.35 | 2026-10-13 | frick-fsw | Remove remaining active T1 reproducibility overclaims from OCI label, Makefile, and README per FRICK-REVIEW-0198/0199; NC-EXT-35 remains open. |
| 0.3.36 | 2026-10-13 | frick-fsw | Align T1 evidence-claim mapping and T14 package-input wording with actual mutable environment/checker inputs following FRICK-REVIEW-0200/0201. |
| 0.3.37 | 2026-10-13 | frick-fsw | Clarify T18 build-provenance/reproducibility limitation and open NC-EXT-40 requalification status. |
| 0.3.38 | 2026-10-13 | frick-fsw | Align T7 and T18 status text with pending independent review and clean-image requalification; track NC-EXT-39/40. |
| 0.3.39 | 2026-10-13 | frick-fsw | Update T13/README status-panel deferral wording to distinguish registry entries from product-scope deferrals; independent delta review pending (NC-EXT-41). |
| 0.3.40 | 2026-10-13 | frick-fsw | Prevent scoped/partial PA PASS from counting as overall closed review stream; add three focused verdict tests and regenerate panel; independent T13 delta review pending (NC-EXT-41). |
| 0.3.41 | 2026-10-13 | frick-fsw | Restrict verdict recognition to affirmative prefixes, reject negative-substring false positives, add stream/negative/partial regression cases (7 total), and regenerate panel; T13 remains not fully requalified. |
| 0.3.42 | 2026-10-13 | frick-fsw | Add pending/open/scope-only verdict boundaries, test approved-with-comments and stream behavior, and qualify T13 status after FRICK-REVIEW-0221..0223; requalification remains pending. |
| 0.3.43 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0221/0223 NOT APPROVED and maintain T13 not-fully-requalified status pending fresh review. |
| 0.3.44 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0224..0226 NOT APPROVED; later classifier/status corrections await fresh independent review; T13 remains not fully requalified. |
| 0.3.45 | 2026-10-13 | frick-fsw | Record expanded negative-clause handling and latest R2/R4/PA findings; T13 remains not fully requalified pending fresh independent review. |
| 0.3.46 | 2026-10-13 | frick-fsw | Add contradictory-negative regression cases and record FRICK-REVIEW-0227 NOT APPROVED; T13 remains not fully requalified. |
| 0.3.47 | 2026-10-13 | frick-fsw | Correct T13 focused-test count to distinguish 9 metrics tests from 14 total Python tests and update latest review summary to 0227. |
| 0.3.48 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0228 NOT APPROVED and retain T13 not-fully-requalified status pending fresh review. |
| 0.3.49 | 2026-10-13 | frick-fsw | Clarify generated metrics are project-selected indicators and do not imply approved mission criticality assignment; independent review pending. |
| 0.3.50 | 2026-10-13 | frick-fsw | Record bounded FRICK-REVIEW-0229/0230/0231 outcomes without claiming T13 qualification or closure. |
| 0.3.51 | 2026-10-13 | frick-fsw | Add T21 candidate license checker and test references; independent qualification pending under NC-EXT-43. |
| 0.3.52 | 2026-10-13 | frick-fsw | Correct exact SPDX expression parsing after FRICK-REVIEW-0235; retain T21 unqualified and NC-EXT-44 open pending independent re-review. |
| 0.3.53 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0238 header-position finding; parser now limits qualification to initial comment region and tests cover late/mismatched declarations; T21 remains unqualified pending R2 delta. |
| 0.3.54 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0239 C-preprocessor false-acceptance finding; distinguish C-family hash directives from comment lines and add regression test; T21 remains unqualified. |
| 0.3.55 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0240 format-specific comment and shebang finding; restrict comment syntax by file format and add managed-format matrix tests; T21 remains unqualified. |
| 0.3.56 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0241 C-family block-comment-state finding; require opening block delimiter for starred declaration lines and add C/H/DTS tests; T21 remains unqualified. |
| 0.3.57 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0242 bounded no-new-finding re-review; T21 remains NOT FULLY QUALIFIED pending full applicable evidence and other reviews. |
| 0.3.58 | 2026-10-13 | frick-fsw | Clarify T21 CI test execution through `make gsw-selfcheck`, include assets in declared header scope, and retain NOT FULLY QUALIFIED status |
| 0.3.59 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0244 SVG XML-prolog false negative and author parser correction; T21 remains unqualified pending rerun and independent review |
| 0.3.60 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0245 bounded SVG parser correction result; T21 remains NOT FULLY QUALIFIED pending complete evidence and authority review |
| 0.3.61 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0248 unreadable-file finding and author fail-closed correction; T21 remains unqualified pending independent re-review |
| 0.3.62 | 2026-10-14 | frick-fsw | Record bounded R2 pass FRICK-REVIEW-0249 and active-document scope findings FRICK-REVIEW-0250/0251; maintain T21 NOT FULLY QUALIFIED |
| 0.3.63 | 2026-10-14 | frick-fsw | Record follow-up status reviews 0252..0255, correct active summaries and align T21 status without claiming qualification |
| 0.3.64 | 2026-10-14 | frick-fsw | Record status follow-ups 0256/0257; T21 remains not fully qualified and independent review remains open |
| 0.3.65 | 2026-10-14 | frick-fsw | Record bounded no-new-finding status reviews 0258/0259; T21 remains NOT FULLY QUALIFIED |
| 0.3.67 | 2026-10-14 | frick-fsw | T3 delta: add deterministic synchronized SB and EVS mutex-contention oracles, including the open SRS-REQ-011 forwarding-loss observation; `test_sb` passes 298 checks, but independent R2 qualification remains pending |
| 0.3.68 | 2026-10-14 | frick-fsw | Register T22 candidate SB/EVS structural sweep tool; record its exploratory-only scope, fresh sweep run, initial output defect/correction, and pending independent N10 review |
| 0.3.69 | 2026-10-14 | frick-fsw | Correct T22 N10 gate statement: source review/inspection and independent R2 delta are complete, but local procedure's CI-execution criterion is not met because only the CTest boundary case—not the matrix runner—is currently wired into CI |
| 0.3.70 | 2026-10-14 | frick-fsw | Register the full SB/EVS sweep runner as a CTest so `make ci-local` exercises it; retain T22 as pending independent requalification/hosted evidence, not mission compliance |
| 0.3.71 | 2026-10-14 | frick-fsw | Record local CTest run identities and scoped R2/R4/PA review dispositions; hosted/immutable evidence and formal mission-compliance credit remain open |
| 0.3.72 | 2026-10-15 | frick-fsw | Record the `make docs` target fix (`mkdir -p build/doxygen` before `doxygen Doxyfile`) so fresh-checkout host/CI generation works; scoped R4 0283 / PA 0284 delta reviews record the change and its tracked comments; no N10 qualification or release credit |
| 0.3.73 | 2026-10-15 | frick-fsw | Record the `make docs` output-existence guard (`test -s build/doxygen/html/index.html`) in the `docs` target plus the `docs`-job assertion and `doxygen --version` echo (PA FRICK-REVIEW-0284 Comment 5); Doxygen version/build identities for the T1-based jobs remain open, so no N10 qualification or release credit |
| 0.3.74 | 2026-10-15 | frick-fsw | Record T23 fetch-script mode scope and mark the CI workflow dependency-order delta pending hosted execution and independent review; do not infer tool qualification from local host build |
| 0.3.75 | 2026-10-15 | frick-fsw | Record ignored-content rejection and wrong-revision/ignored-file probes for T23; retain tool qualification and hosted-workflow gates open |
| 0.3.76 | 2026-10-15 | frick-fsw | Record bounded independent T23 behavior delta review FRICK-REVIEW-0298; retain tool qualification, hosted workflow and complete provenance gates |
| 0.3.77 | 2026-10-15 | frick-fsw | Add retained tracked-dirty lwIP probe alongside wrong-HEAD, ignored, untracked and clean-retry outputs; keep independent follow-up and T23 qualification pending |
| 0.3.78 | 2026-10-15 | frick-fsw | Record independent R2/PA follow-up of tracked-dirty probe; T23 qualification and hosted-workflow evidence remain open |
| 0.3.79 | 2026-10-15 | frick-fsw | Record candidate T7 incremental serial decoder and focused tests; independent R2 qualification delta and external E2E remain open |
| 0.3.80 | 2026-10-15 | frick-fsw | Record deadline-limited exact-ping reader, bounded telemetry capture, and 12-test T7 delta; R2 and external E2E remain pending |
| 0.3.81 | 2026-10-15 | frick-fsw | Record R2 0317 bounded T7 delta acceptance and retain external E2E, freshness/correlation and qualification limits |
| 0.3.82 | 2026-10-15 | frick-fsw | Add production UART stream-framing test to T3; record host PTY evidence and retain timeout-authority/target limits |
| 0.3.83 | 2026-10-16 | frick-fsw | Update T3 for 51-check UART disconnect/path correction and bounded R1/R2 reviews; no formal N10 qualification claimed |
| 0.3.84 | 2026-10-16 | frick-fsw | Add pinned OBCP/MicroPython dependency fetch path to T23; record acquisition evidence only and retain T23 NOT QUALIFIED status pending independent delta review |
| 0.3.85 | 2026-10-16 | frick-fsw | Correct `make format` recursive file selection after pinned-container discovery; register T24 and preserve host/pinned-tool evidence boundary pending independent review |
| 0.3.86 | 2026-10-16 | frick-fsw | Record bounded independent FSW review and successful pinned `make format` + `format-check`; leave formal N10 qualification/hosted execution open |
