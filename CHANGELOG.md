# Changelog

Detailed revision history of frick-fsw (norm N16). Document-level histories are
maintained in each file's `## History` table; this file records the project
level history per release and per significant change, with the origin of every
change. All dates UTC, project language English (norm N2).

## [Unreleased]

### Added

- **Restored explicit M4 scope (N19 audit).** Scopes that existed only as prose
  "absorbed" notes are now explicit, traceable deliverables: **PUS-C over IP and
  TSN** — bounded segmentation/reassembly (SRS-REQ-059) and reference `FF_Link_t`
  backends for TCP/IP (SRS-REQ-060) and TSN (SRS-REQ-061); **extended fieldbus** —
  the generic fabric-device contract (SRS-REQ-067), CAN-FD via the fabric contract
  (SRS-REQ-062), CANopen (CiA 301/302/402) (SRS-REQ-063) and RS-485 half-duplex
  DE/RE (SRS-REQ-064); and a documented **PUSopen(R) adapter** (SRS-REQ-065,
  library user-supplied). M8 gains the GEM TSN shaping/filtering completion
  (802.1Qav/Qci/Qbb, SRS-REQ-066) and the explicit gPTP + fabric CoreTSN
  requirement (SRS-REQ-068). New SSS-REQ-028/029; new objectives OBJ-25..OBJ-30;
  new sprint items S4b/S4c/S4d; counts 69 srs / 29 sss. The on-board file transfer
  set remains planned in M15.

- **DDJF acronym registration + PUS-C transport-channel documentation
  (FRICK-ENG-0007 0.3, FRICK-ENG-0005 0.5, FRICK-GLOS-001 0.2).** Registered the
  project acronym **DDJF = Design Description and Justification File**: retitled
  `docs/engineering/software-description.md` (FRICK-ENG-0007) as the
  **Design Description and Justification File (DDJF)**, replaced the former
  "no repository acronym defined" naming note with the explicit definition, and
  added the **justification dimension** as §3.10 (the rationale for each major
  design element, cross-referenced to the ADRs ADR-0001…0014/0016; ADR-0015
  unused). Added the acronym to `docs/glossary.md` and named the DDJF in the
  README documentation index and `AGENTS.md` (same path). Added a user-facing
  **§7.1 "PUS-C transport channels"** to `docs/engineering/user-integration-manual.md`:
  PUS-C is transport-agnostic and carried by `FF_Link_t`; only loopback
  (`FF_LinkLoopbackBackend`) and UART (`FF_LinkUartBackend`) reference backends
  ship (default loopback on the host, UART on RTEMS), `FF_LinkSelect()` plugs
  another backend, and TCP/IP/TSN/CAN/SpaceWire PUS-C link backends are **not
  implemented** (planned) — the HAL contracts for those devices are *device*
  contracts, not PUS-C link channels. Documentation only, no capability
  over-claim.
- **User-facing TCP/IP and TSN documentation + expanded framework description
  (FRICK-ENG-0005 0.4, FRICK-ENG-0007 0.2, FRICK-ARCH-0012 0.3).** Added user
  "How to use" subsections to `docs/engineering/user-integration-manual.md`:
  §8.1 (TCP/IP — `FF_NetStack_t` contract; `FF_NetStackLoopback`/`FF_NetStackLwip`
  backends; `FF_USE_LWIP` default OFF; lwIP pinned to `STABLE-2_2_1_RELEASE`
  BSD-3-Clause; Tuxera/RTEMS `libbsd` user-supplied; PUS-C over IP **not**
  implemented end-to-end) and §9.1 (TSN — `FF_TsnBackend_t`/`FF_TsnGemSetup`;
  802.1Q/Qbv implemented; 802.1Qav `FF_NOT_IMPLEMENTED`; 802.1AS time-read seam,
  **not** gPTP; M8 plan). Expanded the comprehensive framework description
  `docs/engineering/software-description.md` (§2.1 "what frick-fsw is not", §3.8
  BSP and start-up sequence, §3.9 interface-contract inventory with
  implemented/interface-only/planned/user-supplied status, §5 platform build/target
  matrix). `docs/architecture/network-stack.md` now states the exact lwIP pin and
  licence and cross-references the manuals (§8.1/§9.1); `AGENTS.md` names the
  software-description path explicitly. Documentation only, no capability
  over-claim.
- **M4/M11 - reference MSS I2C backend (SRS-REQ-058).** `src/drivers/i2c/mpfs/ff_i2c_mpfs.{c,h}`
  implements the HAL I2C contract `FF_I2c_t` over the Microchip PolarFire SoC MSS
  I2C (CoreI2C) per the S3 sprint: byte-injected MMIO bus (`FF_I2cBus_t`),
  canonical-address API (7-bit address passed to `write`/`read`/`write_read`),
  divider-code selection from a reference 100 MHz APB clock (exposed
  `ff_i2c_div_table`; 100 kHz/400 kHz/1 MHz selections tested), polled master
  sequencing driven by the hardware SI-clear discipline (read STATUS -> act ->
  clear SI; START/STA-ADDR/SEND-DATA/begin-read/RECEIVE/STOP), bounded SI wait
  (`FF_I2C_POLL_MAX`), and NACK/timeout/invalid-argument/not-ready error codes.
  Host unit test `test_i2c` (behavioural SI-clear-driven byte mock with a
  register-based slave; 144 checks: write, read incl. cap=1, write-read, fault,
  bus-hook validation and divider cases incl. the 1 MHz code-5 mapping) and a
  Renode MMIO register round-trip check (`hal_i2c=PASS` on
  the emulated SoC). M11 peripheral-bound HAL coverage 6/13 -> 7/13.
  Scope: polled master only (no slave/SMBus/clock-stretch/interrupt path);
  the emulated controller does not model CTRL CR0/CR1/CR2 (asserts ENS1 + IDLE
  STATUS 0xF8 and round-trips ADDR/SLAVE1_ADDR only) — SCL divider is a reference
  derivation, on-target APB/SCL timing = GAP-A5.
- **Engineering documentation set.** `docs/engineering/software-description.md`
  (FRICK-ENG-0007; the "what this framework is and does" entry document, expanded
  to 0.2 and kept current per release), plus status updates in
  `docs/architecture/network-stack.md` (0.3), `docs/engineering/user-integration-manual.md`
  (0.4) and `docs/engineering/driver-development.md` (0.3) recording the
  TCP/IP-over-PUS-C and TSN implementation status (implemented / planned /
  external) with dedicated user-facing sections and no capability over-claim.
- **M10 - QEMU emulated-target run (SRS-REQ-043, OBJ-18).** A **generic**
  `riscv/rv64imafdc` RTEMS image (platform `qemu-virt-rtems`, `FF_PLATFORM=rtems-generic`,
  no PolarFire SoC board module; `config/platform/platform_qemu.h`) is built
  (`cmake/toolchains/rtems-riscv-generic.cmake`, `make qemu-image`/`qemu-image-docker`)
  and run on `qemu-system-riscv64 -M virt` (`make qemu`, V&V tool **T20**
  `tools/scripts/qemu_vv.sh`, which asserts the app count and records the toolchain
  image id): boot banner + **6 applications registered**, console retained at
  `build/rtems-generic/qemu-console.log`. Complements the Renode MPFS run (norm
  N18); the nightly workflow runs the QEMU check. The open-source RTEMS toolchain
  image (`tools/docker/rtems`, T18) now builds the `rv64imafdc` BSP and carries
  QEMU + cmake/ninja. M10 closed.
- **M4/M11 - reference MSS CAN backend (SRS-REQ-057).** `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`
  implements the HAL CAN contract `FF_CanBackend_t` (classic CAN 2.0 A/B) over the
  Microchip PolarFire SoC MSS CAN: run/bit-timing set-up, round-robin TX buffers,
  accept-all RX buffer with poll dispatch, reference bit timing. Host unit test
  `test_can` (mock MMIO, 129 checks) and a Renode MMIO register round-trip check
  (`hal_can=PASS` on the emulated SoC). CAN-FD is rejected (fabric device).
  M11 peripheral-bound HAL coverage 5/13 -> 6/13.
- **Renode V&V environment: board repl (BeagleV-Fire).** The functional test now
  loads `@platforms/boards/beaglev-fire.repl` (which includes the SoC platform)
  instead of the SoC-only repl, adding the **QSPI NOR flash** (`SPI.MPFS_QSPI` +
  `SPI.Micron_MT25Q`) and `MPFS_SystemServices` models. All existing HAL checks
  still pass (`RENODE_TEST: PASS`). A genuine QSPI write/erase check still needs
  the QSPI controller command driver (M11 task).
- **M4/M11 - reference NOR/QSPI MTD backend (SRS-REQ-056).** `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`
  implements the HAL memory contract `FF_Mtd_t` (byte read/write, sector-aligned
  erase) over the MSS QSPI NOR; host unit test `test_mtd` (mock, 4123 checks).
  The on-SoC check needs the QSPI controller command driver (the board repl with
  `MPFS_QSPI` + `Micron_MT25Q` is now loaded) (M11 follow-up).
- **M4/M11 - reference eNVM NVM backend (SRS-REQ-055).** `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`
  implements the HAL memory contract `FF_Nvm_t` (key/value by id) over a bounded
  eNVM slot table (32 ids x 32-byte slots); host unit test `test_nvm` (mock MMIO,
  36 checks) and a Renode eNVM **data-region write/read** check (`hal_nvm=PASS` on the emulated
  SoC; the data region at 0x20220000 is writable).
  M11 peripheral-bound HAL coverage 4/13 -> 5/13.

### Fixed

- **MSS I2C review NCs (FRICK-REVIEW-0129..0136; SRS-REQ-058).** `test_i2c` extended
  from 135 to 144 checks: a real 1 MHz divider-code-5 case (R1-03/R2-01/R3-02/PA-04),
  the minimum-SCL boundary at 104166/104167 Hz (R1-01/R3-04) and a NULL-bus-hook
  rejection case (R1-C2). The header now documents the minimum achievable SCL
  (`FF_I2C_MIN_SCL_HZ`), 7-bit-only addressing, the fire-and-forget STOP, the
  non-re-entrant binding and the self-test side effects (R1-02/C3/C4/C8), and the
  register map is labelled a **reference mapping** (R3-01); the host mock is
  described as behavioural/phase-level, not an independent oracle (R1-C1/R2-03/R3-07);
  comment typos fixed (R1-C7). Records reconciled: `test_i2c` = 144 checks, the
  divider rates corrected, the four document headers bumped to their History rows
  and the loop-audit baseline refreshed (PA-02/PA-03/R4-01/R4-04); `SRS-REQ-058` was
  added to the `ci-renode` evidence markers (R2-02); a reproducible `make misra-docker`
  T14 path was added (PA-05/R1-C9); the ICD board-level scope note and the 802.1AS
  "time-read seam" wording were clarified (R3-06/R4-06); `software-description.md`
  was registered in the README index and `AGENTS.md` (PA-06); the README "Current
  focus" was refreshed (R4-03); the undefined token `RR-058` was expanded (PA-07);
  and the GAP-A5 I2C on-target residual (per-rate SCL accuracy, STOP completion,
  bus recovery) was recorded.
- **M10 QEMU review NCs (FRICK-REVIEW-0120..0123; also closes the predecessor
  M10 review FRICK-REVIEW-0035 NCs).** The QEMU image is now a genuinely generic
  RTEMS build (no PolarFire SoC board module / MPFS banner, per `platform_qemu.h`);
  tool **T20** asserts the `6 applications registered` count and records the
  toolchain image id; the `ci:emulated-run` evidence mapping is corrected and the
  QEMU check is exercised by the **nightly** workflow (`.github/workflows/nightly.yml`);
  the T18 cleanliness caveat is recorded; the emulated-target doc's per-core ISA and
  QSPI/MT25Q attribution are corrected and the driver scope is explicit; status.json,
  loop-audit, traceability mirror, verification-plan, README and trailing newlines fixed.
- **MSS CAN RX accept-all mask (FRICK-REVIEW-0107 R2-01 / 0109 PA-09 Major,
  SRS-REQ-057).** The RX acceptance mask was written to the AMR register in the
  raw identifier domain instead of the register's left-aligned identifier field
  (bits 3-31), so extended identifiers >= 0x04000000 were silently rejected; the
  reference accept-all filter (`FF_CAN_ACCEPT_ALL`) is now left-aligned into the
  identifier field with the RTR/IDE mask bits set. `test_can` gained a regression
  that discriminates the fix (9 failures at the old constant) and boundary /
  negative cases; `FF_CanMpfsPoll` re-reads the RX control word before
  acknowledging so a callback reconfiguration is not clobbered; the raw AMR write
  (including the RTR/IDE mask bits the emulator does not model) is asserted.
- **MSS CAN reference clock (FRICK-REVIEW-0110 R1-D-01 Minor, SRS-REQ-057).** The
  reference bit-timing clock was corrected from an assumed 125 MHz to **80 MHz**
  (the MSS CAN clock is one of 8..80 MHz from the MSS PLL per the PolarFire SoC
  MSS TRM); at 80 MHz the standard rates 125/250/500 kbit/s and 1 Mbit/s are
  exact (prescaler 40/20/10/5). On-target confirmation remains GAP-A5.

- **Table Services validation bypass (FRICK-REVIEW-0077 Major, SRS-REQ-013).**
  `FF_TblLoad` left a previously-staged image valid when a new load failed
  validation, so `FF_TblActivate` could commit a rejected image. It now clears
  `staged` on a validator rejection; a regression test reproduces and guards it.
- **Test bug fixed (ASan):** `test_r2_gap` cast a TC secondary header to a TM
  header pointer in the CCSDS TM test, causing a stack-buffer-overflow READ under
  AddressSanitizer; corrected to use a real `FF_Pus_TmHdr_t`.
- **`FF_PusInit` idempotence.** `FF_PusRegisterCoreServices` now resets the
  provider/function/HK/mem registries so repeated init is clean (fixes the
  order-dependence of the new PUS tests, FRICK-REVIEW-0077 R1-02).


- **Test-coverage gap closure (R2 + R3 task).** Two new host suites close the
  functions/branches that were unexecuted and the requirement-level flows that
  were `ci:docs`-only: `tests/unit/test_r2_gap.c` (487 checks: EVS/ES lifecycle,
  SB wildcard/saturation/drop, TBL revert/dump, Time MET/UTC, HAL invalid-class,
  CCSDS TC/decode errors, PUS core counters + S3/S8/S9/S20/S11, Math NaN/inf, PAL
  lifecycle) and `tests/unit/test_r3_gap.c` (198 checks: TC->dispatch->TM, S1
  verification codes, EVS->S5, TBL activate/revert, S9 set/report, PUS malformed
  TC). ctest 14 -> 16 suites. Reports FRICK-REVIEW-0075 (R2) / 0076 (R3).
- **Completion-metrics panel (README).** `tools/scripts/metrics.py` (V&V tool
  T13) now computes and renders, **above the semaphores**, a completion panel:
  sprint completion (`docs/planning/sprint-plan.md`), SSS/SRS requirements
  delivered, design traceability (SDD), verification coverage, review closure and
  code coverage (statement/function), with a note explaining each metric and the
  ECSS-class-B context. It is refreshed every cycle (norms N8/N11/N19). The metric
  definitions were reviewed for approval by the Senior SW PA (norms N20/N12):
  FRICK-REVIEW-0068/0069/0071 (PA, APPROVED WITH COMMENTS) and the T13 source
  review FRICK-REVIEW-0070 (R2, N10); a CI currency gate (`metrics.py --check`)
  keeps the panel current.
- **Milestone M16 — In-house OBCP engine development.** Added a milestone
  (objective OBJ-24; SSS-REQ-027 / SRS-REQ-054) to develop, within the project, an
  ECSS-compliant OBCP engine behind `FF_Obcp_t` (bounded resources, isolation,
  execution-time budget, abort, reporting, PUS S18 state management) as the
  fallback/hardened engine to the integrated third-party engine (M12). Execution
  order: after M12, before M13. Origin: user directive.
- **N12 PA-review closure loop (norms N12/N20).** Independent Senior reviews
  (R1/R2/R4) of the N20 governance change and of `FRICK-REVIEW-0032` were run,
  their findings fixed and re-reviewed to closure: the reference loopback now
  rejects a duplicate `bind` across sockets; `test_netloop` gains boundary,
  negative, exhaustion and full-ring partial-send cases (29 -> 328 checks); the
  network-stack contract states the timeout/partial-transfer and `bind` semantics;
  SRS-REQ-039 wording corrected; review log, loop-audit, development plan, V&V
  records and the tool register reconciled (FRICK-REVIEW-0054..0062). PA/documentation
  semaphores are driven by the review verdict.
- **Norm N20 — independent Senior SW product-assurance review of every phase
  product.** New non-derogable norm: every phase product is independently
  reviewed also by the **Senior SW Product Assurance engineer (PA)**, and
  findings are corrected and the reviews repeated until they pass
  (`docs/engineering/reviewer-agents/aeronautics-space-sw-pa-engineer.md`).
  The five Senior reviewer personas are now **versioned in this repository**
  under `docs/engineering/reviewer-agents/` (R1 FSW, R2 V&V, R3 Avionics
  Systems, R4 Documentation, PA), superseding the previous inline R1-R4
  profile blocks, and exposed as OpenCode subagents (project `.opencode/agents/`).
  Origin: user directive; ADR-0010 revised; norm N20 (see `docs/rules.md`).
- **Milestone M2 — Time base: PPS tone discipline and CUC framing.** Hardware-
  disciplined time base with time-source selection (`FF_TimeSetSource`, internal
  CPU / external 1 Hz tone / ground correlation), a software PPS backend task on
  the host (`src/core/time/pps.c`, `FF_PpsRegister/Start/Stop`) and an
  RTEMS/MPFS GPIO/PLIC capture hook for the board integrator (GAP-A7). Tone
  discipline (`FF_TimeToneSignal`): strictly increasing epoch, one-second CUC
  jump per valid tone, UTC latched at the edge. Leap seconds scheduled with
  `FF_TimeScheduleLeap` (single ±1 s) and applied/cleared exactly at the next
  tone boundary. CUC framing per CCSDS 301.0-B-4 completed: 7-byte P+T code
  (P-field `0x2E`, 4 coarse + 2 fine octets) with lossless
  encode/decode (`FF_TimeCucEncode/Decode`) and rejection of malformed frames.
  New requirements SSS-REQ-016, SRS-REQ-021..024; Lean model `Frick.Time`
  (11 theorems) + 10 requirement wrappers; unit tests `test_time`
  (ctest `time`, 5/5 suites, 174 checks total: ccsds 15, sb 20, tbl 9, pus 29,
  time 101). Docs: `time-and-timecode.md` 0.3, ICD 0.3, class-A gaps GAP-A7,
  roadmap M2 done. M2 review closure (norm N12): all R1/R2/R3 non-conformances
  resolved and delta-re-reviewed; code line coverage 845/1446 (58.4%),
  functions 114/156; Lean clean-rebuild transcript committed
  (`verification/lean/lake-build.log`).
- **Development plan** `docs/development-plan.md` (FRICK-PLAN-0001): the product
  boundary (frick-fsw is a middleware), objectives OBJ-01..OBJ-12, the
  resource/bus interface catalogue (Ethernet+pluggable TCP/IP/TSN, CAN/
  CANopen/CAN-FD, RS-422/485, SPI, I2C, PCIe, GPIO, SpaceWire ECSS, memory),
  and the milestone plan M1..M5.
- **User & Integration Manual** `docs/engineering/user-integration-manual.md`
  (FRICK-ENG-0005): how to integrate applications, drivers and pluggable stacks
  on the middleware.
- **Milestone M3 — Hardware abstraction interfaces.** A bounded, allocation-free
  HAL registry (`src/hal/hal.{c,h}`) and stack-independent contracts per resource
  (Ethernet MAC + net interface, pluggable TCP/IP stack, TSN, CAN/CANopen/CAN-FD,
  serial RS-422/485, SPI, I2C, PCIe, GPIO, SpaceWire + RMAP, RAM/DMA/MTD/NVM/
  block). Requirements SSS-REQ-017 / SRS-REQ-025..032; Lean model `Frick.Hal`;
  unit test `test_hal`; spec `docs/architecture/hardware-abstraction.md`;
  `FF_BUILD_HAL` partitioning option; layer-focused architecture diagrams.
- **ECSS design and assurance documentation.** Software Design Document
  (`docs/design/software-design.md`), configuration management plan
  (`docs/quality/configuration-management.md`), product/quality assurance plan
  (`docs/quality/product-assurance.md`), validation plan
  (`docs/vv/validation-plan.md`), preliminary safety/FDIR analysis
  (`docs/quality/safety-and-fdir.md`) and the ECSS compliance matrix
  (`docs/quality/ecss-compliance-matrix.md`); gaps to class A extended to
  GAP-A8..A13 (SDP formalisation, SDD depth, FMEA, third-party IVV, OPD,
  maintenance).
- **Milestone M6 — Mathematics library integration (libm).** Portable
  mathematics facade `src/math/ff_math.{c,h}` over the platform `libm`
  (`FF_USE_LIBM`, `FRICK_USE_LIBM=1`; glibc host / newlib RTEMS), linked with
  `-lm`; unit test `test_math` (20 checks); requirements SSS-REQ-018 /
  SRS-REQ-033; decision ADR-0012 and policy `docs/architecture/numerics.md`;
  licence analysis updated.
- **Milestone M7 — CCSDS SDLS (Space Data Link Security).** Middleware-side
  security sub-layer `src/security/ff_sdls.{c,h}`: SDLS processing (SDLS header
  + MAC; plain/auth/enc/auth-enc services), a bounded SPI-indexed
  security-association database, and key management (session-key derivation,
  rotation on frame/time thresholds with strictly increasing epoch, and
  zeroisation). The cryptographic primitive is a **pluggable backend**
  (`FF_CryptoBackend_t`); a non-secure reference backend (`ff_crypto_test.c`,
  test-only) exercises the logic. Requirements SSS-REQ-019 / SRS-REQ-034/-035;
  Lean `Frick.Sdls`; unit test `test_sdls` (69 checks, ASan-clean);
  ADR-0013 and `docs/architecture/security-sdls.md`; algorithm qualification /
  HSM / ground key management is GAP-A14.
- **Milestone M4 — reference drivers and third-party integration (in progress).**
  Reference **Ethernet driver** for the Cadence GEM of the PolarFire SoC MSS
  (`src/drivers/eth/gem/`), implementing the `FF_EthMac_t` HAL contract with
  static, allocation-free descriptor/buffer handling and injected MMIO + HAL DMA
  translation (host-testable; `test_gem`, 23 checks). Requirements SSS-REQ-020 /
  SRS-REQ-036; `FF_BUILD_DRIVERS` partition option; ADR-0014. Third-party
  integration (fetched pinned, not committed): **lwIP** (BSD-3) for
  `FF_NetStack_t` and **N7/ESA `asn1-pusc-lib`** (GPL-3.0) for
  `FF_Pus_Backend_t`; `tools/scripts/fetch_third_party.sh`. TSN uses the GEM's
  native block (802.1Q/Qbv/Qav/802.1AS).
- **M4 — RTEMS application integration.** A bounded RTEMS 6.2 configuration
  table (`config/rtems/rtems_config.c`: console/clock + POSIX objects +
  `CONFIGURE_POSIX_INIT_THREAD_TABLE`) and the `POSIX_Init` entry point
  (`pal_rtems.c`) running the shared bring-up; the RTEMS executable is built
  from the same sources. Requirement SRS-REQ-037; guide
  `docs/architecture/rtems-integration.md`; toolchain flags aligned to the
  `riscv/mpfs64imafdc` BSP.
- **M4 — GEM native TSN backend.** A reference `FF_TsnBackend_t` over the
  Cadence GEM native TSN block (`src/drivers/eth/gem/ff_gem_tsn.{c,h}`): 802.1Q
  VLAN/PCP classification (screener type 2), 802.1Qbv time-aware scheduling
  (ENST gate control lists) and 802.1AS time from the GEM TSU; 802.1Qav CBS is
  documented as not exposed. Requirement SRS-REQ-038; unit test `test_gem_tsn`
  (16 checks).
- **M4 — Reference network stack (loopback).** `src/drivers/net/ff_net_loopback.{c,h}`
  implements the pluggable network-stack contract (`FF_NetStack_t`) as a bounded,
  static, in-process loopback, exercising the contract in the unit tests and the
  emulated-target (Renode) functional check; third-party stacks (lwIP, pinned) plug
  in through the same vtable.
  Requirement SRS-REQ-039; unit test `test_netloop` (328 checks).
- **M4 — lwIP network-stack adapter.** `src/hal/net/lwip/ff_net_lwip.{c,h}`
  implements the `FF_NetStack_t` contract over the lwIP socket API (third-party
  BSD-3 stack, fetched pinned; `contrib` is part of the lwIP tree). Built with
  `FF_USE_LWIP=ON` (CMake target `frick_lwip` + adapter); a host data-path test
  (`test_lwip`, 19 checks) does a real TCP round trip over the lwIP loopback
  interface. Requirement SRS-REQ-040; a CI job fetches lwIP, builds and runs it.
- **Milestone M9 — code quality: MISRA C:2012 and coding style.** Declared the
  **frick-fsw C style** (`.clang-format` normative, cFS-inspired, MISRA-friendly;
  `docs/engineering/coding-style.md`), applied and enforced codebase-wide by the
  format gate; and a **MISRA C:2012** process (`docs/quality/misra-c.md`) with an
  open-source scan (`tools/scripts/misra.sh`, cppcheck addon, advisory → blocking)
  wired into CI (evidence `ci:misra`) and a documented deviation process.
  Requirements SSS-REQ-021 / SRS-REQ-041/042; certified MISRA checker + formal
  report are GAP-A4.
- **Milestone M10 — emulated-target verification.** Run the RTEMS target binary
  on an emulated Microchip PolarFire SoC (**Renode** `polarfire-soc`: 5 CPUs,
  UART, GEM, CAN, SPI, I2C, GPIO, PCIe, SD/eMMC, QSPI flash, eNVM, timers) and/or
  **QEMU** (`riscv/rv64imafdc`), verifying boot/PAL/drivers without hardware
  (partial GAP-A5 mitigation). `docs/vv/emulated-target.md`,
  `tools/scripts/emulated_run.sh`; requirement SRS-REQ-043; tools T15/T16.
- **Norm N18 — Renode functional validation.** A Renode V&V environment
  (pinned Antmicro image, `polarfire-soc` platform) runs the middleware on an
  emulated PolarFire SoC; `src/apps/app_renode_test.c` (CMake option
  `FF_ENABLE_RENODE_TEST=ON`; default build stays at six apps) prints the
  machine-checkable `RENODE_TEST: PASS` marker; `make renode-env` self-tests the
  environment (Renode runs + platform loads), `make renode-image` builds the
  RTEMS image and `make renode` runs the functional validation
  (`tools/scripts/renode_vv.sh`, CI evidence `ci:renode`). Requirement
  SRS-REQ-044; `docs/vv/renode-environment.md`; tool T17. Functional gate is an
  advisory proxy until the RTEMS image is available in the V&V image, then
  blocking (the proxy SKIPs when the image is absent and fails when it is present
  but the marker is missing). Reviews: FRICK-REVIEW-0036 (REJECTED, F-01 Major) +
  delta (all NCs closed).
- **Open-source RTEMS 6.2 baseline.** The RTOS baseline is the **open-source
  RTEMS 6.2** release from the RTEMS project (`gitlab.rtems.org`, source-builder
  ref `6.2`, BSP `riscv/mpfs64imafdc`); no vendor-specific or commercial
  baseline/pre-built toolchain is used. `tools/scripts/setup_rtems.sh` corrected
  (riscv tools bset + `6/rtems-kernel.bset --with-rtems-bsp=riscv/mpfs64imafdc`);
  new reproducible containerised toolchain `tools/docker/rtems/Dockerfile`
  (`make rtems-toolchain` / `make rtems-docker`, tool T18); the nightly workflow
  builds the open RTEMS Renode image and runs `make renode` (norm N18). ADR-0002,
  SSS-REQ-001 and all affected docs aligned.
- **Real open RTEMS toolchain + Renode functional validation (N18 in force).**
  The open-source RTEMS 6.2 toolchain (RSB tag `6.2`, BSP `riscv/mpfs64imafdc`,
  `tools/docker/rtems`) is built and validated; the real cross build succeeds
  (plain and Renode images) and the middleware runs on the Renode `polarfire-soc`
  platform with `make renode` -> `RENODE_TEST: PASS` (boot U54_1 at entry
  `0x1000000000`, console `mmuart1`). Defects found and fixed by the real build:
  `pal_rtems.c` included an unused, non-existent RTEMS header
  (`rtems/posix/time.h`) and other unused includes; `rtems_config.c` lacked the
  `POSIX_Init` declaration before `<rtems/confdefs.h>`; the CMake toolchain file
  now passes the BSP include/lib paths and `-mcmodel=medany` (multilib select).
  Norm N18 is now **in force** (per-commit SKIP proxy, nightly real run).
- **Milestone M12 — OBCP engine (uPython/MicroPython).** Execute On-Board Control
  Procedures written in uPython/MicroPython behind an `FF_Obcp_t` contract (OBCP
  modules bound to SB/EVS/Time/PUS), resource/safety bounded. First integration
  candidate **`n7space/n7s-obcp`** (MicroPython-based OBCP engine, MIT,
  ESA-funded); fallback: an ECSS-compliant engine. Objective OBJ-20; SSS-REQ-022 /
  SRS-REQ-045.
- **Milestone M13 — Reference mission services.** Optional PUS-based reference
  services: an **equipment manager** (device command/monitoring/reconfiguration,
  redundancy) and **fault management** (limit checking, stored command sequences,
  memory/file management). Objective OBJ-21; SSS-REQ-023 / SRS-REQ-046.
- **Component gap analysis.** `docs/architecture/component-gap-analysis.md`
  positions frick-fsw against NASA cFS (missing: LC/SC/MM/MD/CS/FM/DS/CF/SBN/ECI,
  equipment manager, several PUS services) and other FSW (F′, TASTE, N7 MBEP,
  OPS-SAT NMF).
- **Milestone M11 — Renode HAL coverage.** Extend the Renode functional test
  (norm N18) so that **every HAL resource class** is bound to its emulated
  peripheral and exercised on the `polarfire-soc` platform (ETH, NET/TCP-IP, TSN,
  CAN, SERIAL/RS-422/485, SPI, I2C, PCIE, GPIO/IRQ, SPACEWIRE, MEM, BLOCK, NVM),
  each with its backend behind the `hal_*.h` contract (M4-residual MSS
  peripherals), a binding in `frick-renode.resc` and a checkable clause in the
  marker. Objective OBJ-19; `docs/development-plan.md`,
  `docs/roadmap.md`. **Started (GPIO):** MSS GPIO driver
  `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` (SRS-REQ-047) + host unit test
  (`test_gpio`, mock MMIO) + a real MMIO round trip in the Renode run
  (`hal_gpio=PASS`).
- **Reference MSS GPIO driver (M4/M11).** `FF_Gpio_t` backend for the Microchip
  PolarFire SoC MSS GPIO (bus-injected MMIO, allocation-free); host test
  `tests/unit/test_gpio.c` and emulated-MMIO check on the Renode `polarfire-soc`
  platform; requirement SRS-REQ-047.
- **Project codename / third-party name.** Non-affiliation disclaimer for the
  codename "**Eärendil**" (a J. R. R. Tolkien name) added in `NOTICE`, `README.md`
  and `docs/quality/licensing.md §6`: a single name is not copyright-protected
  (copyright vs trademark); the project is not affiliated with the Tolkien Estate
  or Middle-earth Enterprises, includes no Tolkien text, and does not use the name
  as a trademark; licensing contacts and a "not legal advice" note. Product name
  remains **frick-fsw**.
- **M11: reference MSS MMUART serial driver.** `FF_Serial_t` backend for the
  Microchip PolarFire SoC MSS MMUART (NS16550 **wide registers**, 4-byte spaced:
  THR 0x00, LCR 0x0C, LSR 0x14, SCR 0x1C; RS-422/RS-485 via transceiver),
  bus-injected MMIO, allocation-free; host unit test `tests/unit/test_serial.c`
  and an NS16550 scratch-register MMIO check on the emulated MMUART0 in the
  Renode run (`hal_serial=PASS`). The first attempt with compact offsets was
  reviewed (FRICK-REVIEW-0043) and reworked to hardware-correct wide offsets (the
  Renode platform sets `wideRegisters: true`). Requirement SRS-REQ-048.
- **M11: reference static RAM pool + DMA helper.** `FF_MemPool_t` (bounded,
  heap-free bump allocator over a caller-provided region, usually DDR) and a
  reference `FF_Dma_t` (identity address translation; cache-maintenance seam) in
  `src/drivers/mem/ff_mem_static.c`; host unit test `test_mem` and a RAM-pool + DMA
  check over the in-image RAM (emulated DDR) in the Renode run (`hal_mem=PASS`;
  the DMA cache hooks are a seam, real cache ops = GAP-A5). Requirement
  SRS-REQ-049.
- **PUS-C ASN.1 (N7/ESA) default — toolchain enabled.** Reproducible ASN1SCC
  compiler image (`tools/docker/asn1scc/Dockerfile`, pinned esa/asn1scc) and code
  generation from the N7/ESA `asn1-pusc-lib` ASN.1/ACN model
  (`tools/scripts/gen_pus_asn1.sh`, `make pus-asn1`, tool T19); generation
  validated (`BasicTypes.h`, `asn1crt.c/h`, module codecs). Remaining: consume the
  generated code in CMake, implement `FF_PusBackendN7` and switch the default
  (SRS-REQ-050). Tracked by the N19 audit (`docs/status/loop-audit.md`).
  _(Superseded by the "default backend" entry below: the generated code is now
  consumed in CMake and `FF_PusBackendN7` is the default, ADR-0016.)_
- **M15 (new) — In-house fundamental PUS-C service set** (from the PUS-C
  assessment `docs/architecture/pus-c-from-scratch-assessment.md`, FRICK-ARCH-0015):
  implement in MISRA-clean in-house C behind `FF_Pus_Backend_t` the fundamental
  PUS-C service behaviour (S1/3/5/6/8/9/11/12/17/18-surface/19/20) on the
  `asn1-pusc-lib` default data model (+ bounded datapool); est. ~34 PM; hybrid
  recommendation. Objective OBJ-23; SSS-REQ-025 / SRS-REQ-052. Placed after M14.
  **M15 also includes CFDP (CCSDS 727.0-B) and the virtual filestore (VFS) it
  operates on** (pluggable RAM/flash/eMMC/SSD) with PUS-C S23 on the VFS
  (SSS-REQ-026 / SRS-REQ-053).
- **PUS-C ASN.1 (N7/ESA) default backend (SRS-REQ-050, ADR-0016).**
  `FF_PusBackendN7` is the **default** PUS backend: the PUS-C field
  vocabulary/widths come from the ASN1SCC-generated `asn1-pusc-lib` types and the
  packet error control uses the N7/ESA ECSS Annex B.1 CRC; build-time generation
  (`make pus-asn1`, `tools/docker/asn1scc/`, T19; nothing generated/third-party
  committed). Falls back to the open backend (with a warning) when the generated
  code is absent; `-DFF_USE_PUS_ASN1=OFF` forces the open backend. Verified on
  host (14/14) and on the emulated SoC (`make renode` PASS).
- **M11: ETH HAL class exercised in the Renode run.** The functional test now
  identifies the emulated Cadence GEM via its read-only Module ID register
  (`hal_eth=PASS`), in addition to GPIO/SERIAL/MEM (SRS-REQ-036; the full GEM
  DMA/link path stays host-tested + GAP-A5).
- **M11: NET `FF_NetStack_t` contract check in the Renode run.** The functional
  test performs a socket round trip through the pluggable network-stack contract
  using the reference loopback stack (`hal_net=PASS`); it is a contract/behaviour
  check (in-process), **not** a binding to an emulated peripheral (so M11 stays
  4/13 peripheral-bound: GPIO/SERIAL/MEM/ETH). SRS-REQ-039.
- **Norm N19 — development-loop gap audit.** New non-derogable rule: at the start
  of every loop, compare declared objectives + adherence to N1-N19 with the actual
  implementation; gaps closed or tracked. First audit `docs/status/loop-audit.md`.
- **Roadmap: milestone M14 added** — multicore/SMP on the 4×U54 (after all other
  milestones), objective OBJ-22 (SSS-REQ-024 / SRS-REQ-051).
- **Norm N17 — documentation review (profile R4).** New non-derogable norm:
  before every commit/push the documentation set is reviewed for completeness
  and correctness by an independent **Senior SW documentation reviewer** (profile
  R4, `docs/engineering/independent-review.md`), and the review loop
  (review → correction → re-review, norm N12) is closed before commit/push. The
  gate is reported in the README status panel (`documentation_review` in
  `docs/status/status.json`). First run: `docs/reviews/2026-10-10-doc-set-r4-review.md`.
- Gitea CI/CD deployed and operational (norm N6): a self-hosted
  `act_runner` executes `.gitea/workflows/ci.yml`. The runner maps
  `ubuntu-24.04` to the pinned V&V image
  (`ubuntu-24.04:docker://frick-vv:local`) because the minimal
  `gitea/act_runner` Alpine image has no toolchain.
- Project mark for codename "Eärendil": a white stylized ship with a sparkling
  star on a midnight-blue field, in `assets/` (SVG source plus PNG), embedded in
  `README.md`; see `assets/README.md`.
- Mermaid diagrams, compatible with GitHub and Gitea: component architecture
  (`docs/architecture/overview.md`, `README.md`), TC/TM sequence
  (`docs/architecture/pus-layer.md`), and the V-model plus per-change cycle
  (`docs/vv/verification-plan.md`, `README.md`).
- **Copyright header, "Copyright (C) 2026 Ilfrick"** with links to the GitHub
  and Gitea project pages inserted in all 70 authored files (C, headers, Lua/Lean,
  scripts, workflows, Doxyfile, Makefile, CMake and SVGs); consolidated in
  `NOTICE`, `README.md` and `docs/quality/licensing.md` (norm N13).
- **README status panel**: coverage metrics and review/V&V traffic lights
  (red/yellow/green) generated by `tools/scripts/metrics.py` from the
  requirements manifest and `docs/status/status.json`; code line coverage
  collected by `tools/scripts/coverage.py` (`make coverage`, gcov).
- **README project index** (table of contents), documentation index with links,
  PUS-C services catalogue and low-level driver inventory.
- Engineering guides for humans and agents: `docs/engineering/driver-development.md`
  and `docs/engineering/service-development.md`.
- Development roadmap `docs/roadmap.md` (rev 0.3) and development plan
  `docs/development-plan.md`: M1 baseline (done), M2 PPS + CUC (done), M3 HAL
  interfaces + integration manuals, M4 MPFS BSP + 1 Gb/s Ethernet (absorbs
  virtual filestore, PUS service 23, CFDP, and TCP/IP + TSN over PUS-C), M5
  documentation and status program.
- `docs/architecture/time-and-timecode.md`: how MET/UTC/CUC and the PPS tone
  discipline work (PPS delivered in M2).

### Changed

- Project codename changed from **"Vela"** to **"Eärendil"** (README banner,
  `config/fsw_config.h` `FF_CODENAME`, simulator boot banner).
- V&V image (`tools/docker/Dockerfile`) gains `nodejs`/`npm` so Gitea
  `actions/checkout` can run inside the pinned image.

### Fixed

- Documentation consistency (first norm-N17 review loop,
  `docs/reviews/2026-10-10-doc-set-r4-review.md`): README requirement counts
  aligned to the machine-generated panel (25/16/7/41) and README status panel
  regenerated to include the `documentation review` gate; revision headers
  aligned with the history tables (SRS, SSS, traceability.md, roadmap,
  driver-development, licensing, tool-qualification, verification-results,
  tooling, formal-verification); `FRICK-ARCH-0006` id collision fixed
  (`mapping-cfs.md` -> `FRICK-ARCH-0007`); review ids normalised to
  `FRICK-REVIEW-00nn`; `docs/reviews/README.md` log completed (M2 R1 + R4 rows);
  `## History` tables added to `tooling.md` and `glossary.md`;
  `formal-verification.md` M2 scope added; duplicate README `## Repository
  layout` heading removed.
- CI `simulator smoke` (GitHub): the banner check grepped for `=` which the
  banner never contained; with `bash -e` the step failed. Now checks for
  `frick-fsw` and reports a clear error.
- CI `static analysis`: the `clang-format --dry-run --Werror` gate failed on
  files that had never been auto-formatted; all C sources are now formatted with
  clang-format 18 (the runner version), so the gate passes.
- Gitea CI checkout: `act_runner` derives the job server URL without the Gitea
  `/git` sub-path, so `actions/checkout` failed with `repository not found`. The
  Gitea workflow now clones the public mirror explicitly with `git`; the GitHub
  workflow is unchanged.
- CI `documentation set`: the Doxygen step failed inside the pinned V&V image
  (doxygen 1.9.8) because `build/doxygen` did not exist; both workflows now
  create it first.

## [0.1.0] - 2026-10-10 - baseline release

**Origin:** rule set establishment (N1-N16) and initial development; review
closure (norm N12/N15) applied before the first public push.

**Origin note:** the review-closure cycle below re-ran the complete
development/verification cycle (norm N15) and is covered by
`docs/reviews/2026-10-10-baseline-vv-review.md` (R2) and
`docs/reviews/2026-10-10-sss-systems-review.md` (R3).


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
