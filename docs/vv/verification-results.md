# Verification and Validation Results

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RES-001                       |
| Revision                          | 0.35                                   |
| Date                              | 2026-10-12                             |
| Baseline cycle                    | v0.1.0 initial release                 |
| Norms                             | N6, N11 (records committed every cycle)|

This log is the running verification record. Every development/verification
cycle appends a row with the evidence produced; rows reference committed
artifacts. Emulator console captures (Renode `build/renode/console.log`, QEMU
`build/rtems-generic/qemu-console.log`) are regenerable build outputs (not
tracked), reproduced by `make renode` / `make qemu` from the committed sources,
toolchain and scripts.

## Cycle log

| Cycle / date | Build (host) | ctest | Lean | trace | Smoke | Static | RTEMS cross | Comment |
| ------------ | ------------ | ----- | ---- | ----- | ----- | ------ | ----------- | ------- |
| v0.1.0 initial, 2026-10-10 | PASS | PASS (4/4) | PASS | PASS | PASS (6 apps) | clang-tidy clean | PASS (toolchain artifacts) | Baseline release; formal verification details below |
| v0.1.0 review closure, 2026-10-10 | PASS | PASS (4/4) | PASS (`lake build` 15/15 + 3/3 checks) | PASS (42 reqs, 20 theorems) | PASS (6 apps) | clean | proxy | Norm N15 re-run after review NCs (Lean proofs, S6 test, trace gate); see `docs/reviews/2026-10-10-*.md` |
| M2 time base, 2026-10-10 | PASS (Debug + Release) | PASS (5/5) | PASS (`lake build` 17/17 + 3/3 checks) | PASS (48 reqs, 41 theorems) | PASS (6 apps) | clean | proxy | Milestone M2: PPS tone discipline, time-source selection, leap-at-boundary, CUC P-field framing; see M2 notes below |
| M2 review closure, 2026-10-10 | PASS (Debug + Release) | PASS (5/5; 174 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101) | PASS (`lake build` 17/17, transcript `verification/lean/lake-build.log`; 3/3 checks) | PASS (48 reqs / 25 srs / 16 sss / 7 gap, 41 theorems, 9 CI claims) | PASS (6 apps) | clang-format + clang-tidy clean | proxy | All M2 NCs (R1/R2/R3) resolved and delta-re-reviewed; coverage lines 845/1446 (58.4%), functions 114/156; see review evidence below |
| M3 HAL interfaces + ECSS docs, 2026-10-10 | PASS (Debug) | PASS (6/6; 205 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101, hal 31) | PASS (`lake build` 19/19, transcript `verification/lean/lake-build.log`; 3/3 checks) | PASS (63 reqs / 33 srs / 17 sss / 13 gap, 45 theorems, 9 CI claims) | PASS (6 apps) | clang-format clean | proxy | HAL registry + stack-independent contracts (SSS-REQ-017, SRS-REQ-025..032); ECSS SDD/CM/PA/validation/safety/compliance-matrix docs; gaps GAP-A8..A13; coverage 893/1515 (58.9%), functions 124/166 |
| M6 libm integration, 2026-10-10 | PASS (Debug) | PASS (7/7; 225 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101, hal 31, math 20) | PASS (`lake build` 19/19) | PASS (65 reqs / 34 srs / 18 sss / 13 gap, 45 theorems, 9 CI claims) | PASS (6 apps) | clang-format clean | proxy | Mathematics facade over platform libm (SSS-REQ-018, SRS-REQ-033); ADR-0012, numerics policy, licence analysis; coverage 932/1575 (59.2%), functions 141/184 |
| M7 CCSDS SDLS, 2026-10-10 | PASS (Debug) | PASS (8/8; 306 checks: ccsds 15, sb 20, tbl 9, pus 29, time 101, hal 31, math 20, sdls 81) | PASS (`lake build` 21/21) | PASS (69 reqs / 36 srs / 19 sss / 14 gap, 51 theorems, 9 CI claims) | PASS (6 apps) | clang-format + ASan clean | proxy | SDLS processing + SA/key management/rotation, pluggable crypto backend (SSS-REQ-019, SRS-REQ-034/035); GAP-A14; coverage 1252/2005 (62.4%), functions 175/217 |
| M4 drivers + RTEMS + TSN + netstack + lwIP, 2026-10-10 | PASS (Debug) | PASS (11/11; 374 checks default; lwIP adapter built/tested separately with FF_USE_LWIP, `test_lwip` 19 checks) | PASS (`lake build` 21/21) | PASS (78 reqs / 43 srs / 21 sss / 14 gap, 51 theorems, 10 CI claims) | PASS (6 apps) | clang-format clean | proxy | Cadence GEM driver (SRS-REQ-036), RTEMS integration (SRS-REQ-037), GEM TSN (SRS-REQ-038), loopback netstack (SRS-REQ-039), lwIP adapter (SRS-REQ-040); coverage 1656/2521 (65.7%), functions 220/264 |
| M10 Renode V&V env + N18, 2026-10-10 | PASS (Debug) | PASS (11/11; 374 checks; Renode test app: `FF_ENABLE_RENODE_TEST=1` builds 7 apps and prints `RENODE_TEST: PASS`; default build stays 6 apps) | PASS (`lake build` 21/21) | PASS (80 reqs / 45 srs / 21 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; `make renode-env` PASS (Renode runs + `polarfire-soc` platform loads) | proxy | Renode-on-Docker V&V environment + norm N18 + middleware test application (SRS-REQ-044, tool T17); functional gate is an advisory proxy until the RTEMS image is available (then blocking); review FRICK-REVIEW-0036 + delta |
| Open RTEMS baseline + toolchain, 2026-10-10 | PASS (Debug) | PASS (11/11) | PASS (`lake build` 21/21) | PASS (80 reqs / 45 srs / 21 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; `make renode-env` PASS | nightly (open RTEMS RSB, containerised T18) | Aligned to the **open-source RTEMS project** (no vendor/commercial baseline): `setup_rtems.sh` corrected (tools bset + `6/rtems-kernel.bset --with-rtems-bsp=riscv/mpfs64imafdc`) and vendor wording removed across docs; new `tools/docker/rtems/Dockerfile` (`make rtems-toolchain`/`rtems-docker`, tool T18); nightly builds the open RTEMS Renode image and runs `make renode` (N18) |
| Open RTEMS toolchain + Renode functional run, 2026-10-10 | PASS (Debug) | PASS (11/11) | PASS (`lake build` 21/21) | PASS (80 reqs / 45 srs / 21 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (open RTEMS 6.2 built from RSB tag 6.2; `make rtems-docker`) | **open-source RTEMS 6.2 toolchain built and validated** (T18); real cross build OK (plain + Renode image); **functional run of the RTEMS middleware on the Renode `polarfire-soc` platform: `make renode` -> `RENODE_TEST: PASS`** (boot U54_1 at entry 0x1000000000, console mmuart1). Real defects found and fixed by the real build: `pal_rtems.c` included an unused, non-existent RTEMS header (`rtems/posix/time.h`) and other unused includes; `rtems_config.c` lacked the `POSIX_Init` declaration before `<rtems/confdefs.h>`; toolchain file now passes the BSP include/lib paths and `-mcmodel=medany` (multilib). N18 gate is now **in force** (blocking when the image is present; per-commit SKIP proxy; nightly real). |
| MSS GPIO driver + Renode MMIO (M4/M11), 2026-10-10 | PASS (Debug) | PASS (12/12; `test_gpio` 27 checks, mock MMIO) | PASS (`lake build` 21/21) | PASS (85 reqs / 48 srs / 23 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio=PASS`) | Reference MSS GPIO driver (`FF_Gpio_t`, bus-injected MMIO, allocation-free) behind the HAL; host unit test with an independent mock MMIO register bank (per-pin CFG, IN=0x84, OUT=0x88); real MMIO round trip on the emulated MSS GPIO in the Renode run (milestone M11 first class). Requirement SRS-REQ-047. |
| MSS MMUART serial driver + Renode MMIO (M4/M11), 2026-10-10 | PASS (Debug) | PASS (13/13; `test_serial` 30 checks, mock MMIO) | PASS (`lake build` 21/21) | PASS (86 reqs / 49 srs / 23 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio=PASS`, `hal_serial=PASS`) | Reference MSS MMUART (NS16550, 4-byte-spaced registers) driver (`FF_Serial_t`, bus-injected MMIO, allocation-free) behind the HAL; host unit test with an independent mock; NS16550 scratch-register MMIO round trip on the emulated MSS MMUART0 (milestone M11). Requirement SRS-REQ-048. |
| Static RAM pool + DMA + Renode DDR (M4/M11), 2026-10-11 | PASS (Debug) | PASS (14/14; `test_mem` 20 checks) | PASS (`lake build` 21/21) | PASS (87 reqs / 50 srs / 23 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio`/`hal_serial`/`hal_mem=PASS`) | Bounded, heap-free `FF_MemPool_t` (bump allocator) + reference `FF_Dma_t` (identity translation; cache hooks are a seam = GAP-A5); host unit test; the pool is exercised over the in-image RAM (emulated DDR) in the Renode run (milestone M11). Requirement SRS-REQ-049. |
| PUS-C ASN.1 (N7/ESA) default backend (SRS-REQ-050), 2026-10-11 | PASS (Debug) | PASS (14/14) | PASS (`lake build` 21/21) | PASS (94 reqs / 54 srs / 26 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`) | `FF_PusBackendN7` is the **default** PUS backend (ADR-0016): ASN1SCC-generated `asn1-pusc-lib` types + N7/ESA ECSS Annex B.1 CRC; build-time generation (`make pus-asn1`, T19); host unit test `test_pus` asserts the N7 backend is byte-identical to the reference (36 checks); fallback to open backend with a warning when the generated code is absent. Deep per-service codecs = M15. |
| M11 ETH HAL class in Renode (SRS-REQ-036), 2026-10-11 | PASS (Debug) | PASS (14/14) | PASS (`lake build` 21/21) | PASS (94 reqs / 54 srs / 26 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_eth=PASS`) | The Renode functional test identifies the emulated Cadence GEM via its read-only Module ID register (`hal_eth=PASS`), in addition to GPIO/SERIAL/MEM; the full GEM DMA/link path remains host-tested (`test_gem`) + GAP-A5 (M11). |
| M11 NET HAL class in Renode (SRS-REQ-039), 2026-10-11 | PASS (Debug) | PASS (14/14) | PASS (`lake build` 21/21) | PASS (94 reqs / 54 srs / 26 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_net=PASS`) | The Renode functional test performs a socket round trip through the pluggable `FF_NetStack_t` contract using the reference loopback stack (`hal_net=PASS`), in addition to GPIO/SERIAL/MEM/ETH (M11). |
| R2/R3 test-gap task + TBL fix, 2026-10-11 | PASS (Debug) | PASS (16/16; `test_r2_gap` 487 checks, `test_r3_gap` 201 checks) | PASS (`lake build` 21/21) | PASS (96 reqs / 55 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; cppcheck MISRA 0 findings on the new suites | proxy | Two new host suites close previously unexecuted functions/branches and requirement-level flows (FRICK-REVIEW-0075/0076); **real defect fixed** in `src/core/tbl/tbl.c` (rejected table image was committed by activate - FRICK-REVIEW-0077 Major) and `FF_PusInit` made idempotent (registries reset); coverage 1917/2920 -> 2831/3961 lines (71.5%), functions 257/302 -> 371/376 (98.7%). |
| M4/M11 eNVM NVM backend, 2026-10-11 | PASS (Debug) | PASS (17/17; `test_nvm` 36 checks) | PASS (`lake build` 21/21) | PASS (97 reqs / 56 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_nvm=PASS`) | Reference `FF_Nvm_t` eNVM backend (SRS-REQ-055): bounded slot table (32 ids x 32 B), host test (mock MMIO) + Renode eNVM data write/read check; M11 5/13 peripheral-bound. |
| M4 QSPI NOR MTD backend, 2026-10-11 | PASS (Debug) | PASS (18/18; `test_mtd` 4123 checks) | PASS (`lake build` 21/21) | PASS (98 reqs / 57 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | proxy | Reference `FF_Mtd_t` NOR/QSPI backend (SRS-REQ-056): byte read/write, sector-aligned erase; host test (mock MMIO). On-SoC QSPI check pending (needs the board repl `MPFS_QSPI` + `Micron_MT25Q` and the flash command path) - M11 follow-up. |
| Renode board repl enabled (BeagleV-Fire), 2026-10-11 | PASS (Debug) | PASS (18/18) | PASS (`lake build` 21/21) | PASS (98/57/27/14, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm=PASS`) | V&V environment switched from the SoC-only platform to the `beaglev-fire` board repl (includes the SoC). New peripherals available for drivers: QSPI controller `SPI.MPFS_QSPI`@0x21000000 + `SPI.Micron_MT25Q` NOR flash, `MPFS_SystemServices`@0x37020000 (+ its QSPI/flash), board button. The QSPI NOR still needs the controller command path (M11 task). |
| M4/M11 MSS CAN backend, 2026-10-11 | PASS (Debug) | PASS (19/19; `test_can` 129 checks) | PASS (`lake build` 21/21) | PASS (99 reqs / 58 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_can=PASS`) | Reference `FF_CanBackend_t` MSS CAN backend (SRS-REQ-057): classic CAN 2.0 A/B, round-robin TX buffers (32), accept-all RX buffer with poll dispatch, reference bit timing; host test (mock MMIO, 129 checks) + Renode MSS CAN MMIO register round trips; M11 6/13 peripheral-bound. CAN-FD rejected (fabric device). Review NC R2/PA-09 fixed: the accept-all RX mask is left-aligned into the AMR identifier field (the host test discriminates the fix: 9 failures at the old constant). Limitations: no bus-off/error handling, polling-only RX. Reference bit timing from the MSS CAN clock (16 tq, 87.5% SP): the MSS CAN clock is one of 8..80 MHz from the MSS PLL (R1-D-01 corrected the earlier 125 MHz assumption); at the 80 MHz reference the standard rates 125/250/500 kbit/s and 1 Mbit/s are exact (prescaler 40/20/10/5). On-target confirmation of the configured clock = GAP-A5. |
| M10 QEMU emulated target, 2026-10-12 | PASS (Debug) | PASS (19/19) | PASS (`lake build` 21/21) | PASS (99 reqs / 58 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean | **QEMU PASS** (`make qemu`: `platform : qemu-virt-rtems`, banner + 6 apps registered) | M10 closed: the generic `riscv/rv64imafdc` RTEMS image (platform `qemu-virt-rtems`, `FF_PLATFORM=rtems-generic`, no MPFS board module) runs on `qemu-system-riscv64 -M virt` (`-bios`, entry `0x80000000`); tool T20 (`tools/scripts/qemu_vv.sh`) asserts the app count and records the toolchain image id; the nightly workflow runs it. Complements Renode (MPFS image, norm N18). |
| M4/M11 MSS I2C backend (S3), 2026-10-12 | PASS (Debug) | PASS (20/20; `test_i2c` 144 checks; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (100 reqs / 59 srs / 27 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; cppcheck (T14, advisory; reproducible via `make misra-docker`) 0 findings in the changed files | **REAL** (`make renode` -> `RENODE_TEST: PASS`, incl. `hal_i2c=PASS`); QEMU cross-build PASS (T20) | Reference `FF_I2c_t` MSS I2C (CoreI2C) backend (SRS-REQ-058): polled master over a byte-injected MMIO bus; full START/address/data/repeated-START/receive/STOP sequence host-tested against a behavioural SI-clear-driven mock (write, read incl. cap=1, write-read, NACK/timeout/arg faults, bus-hook validation, divider-code selection for 100 kHz/400 kHz/1 MHz and the minimum-SCL boundary at the 100 MHz APB reference); self-test round-trips the ADDR/SLAVE1_ADDR registers on the emulated `I2C.MPFS_I2C` (`hal_i2c=PASS`, M11 7/13 peripheral-bound). Scope limits: emulated controller does not model CTRL CR0/CR1/CR2 (asserts ENS1 only, no divider readback) and advances on register writes (full transaction verified on host); polled master only (no slave/SMBus/clock-stretch/STOP-completion/interrupt path); SCL divider is a reference derivation from FF_I2C_MPFS_CLK_HZ=100 MHz (the minimum achievable SCL is PCLK/960 = 104166 Hz, so a 100 kHz request resolves to ≈104.17 kHz and the caller must request >= that minimum) — on-target APB/SCL timing = GAP-A5. |
| M4-restored scope + DDJF (planning), 2026-10-12 | PASS (Debug) | PASS (20/20; `test_i2c` 144 checks; ASan/UBSan clean) | PASS (`lake build` 21/21) | PASS (112 reqs / 69 srs / 29 sss / 14 gap, 51 theorems, 12 CI claims) | PASS (6 apps) | clang-format clean; documentation/requirements-only increment (no C change); trace + metrics gates green | **REAL** (`make renode` -> `RENODE_TEST: PASS`; no target-code change, Sprint-3 image result stands) | N19 scope audit: restored the M4 scopes that existed only as prose "absorbed" notes as explicit, traceable deliverables — PUS-C over IP/TSN (SRS-REQ-059/060/061), generic fabric contract (SRS-REQ-067), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064), PUSopen(R) adapter (SRS-REQ-065), M8 GEM TSN 802.1Qav/Qci/Qbb (SRS-REQ-066) + gPTP/fabric CoreTSN (SRS-REQ-068); new SSS-REQ-028/029; new objectives OBJ-25..OBJ-30; sprint items S4b/S4c/S4d; DDJF acronym registered (FRICK-ENG-0007 0.3). Reviews FRICK-REVIEW-0137/0138/0139. |

## v0.1.0 baseline notes

### Formal verification (norm N1)

Lean 4 project under `verification/lean/` proves the requirements referenced by
the traceability manifest, including:

- CRC-16/CCITT-FALSE transmission check (frames extended by their CRC yield a
  zero CRC) - `Frick.Ccsds`.
- Software Bus delivery counting and queue-depth bound - `Frick.SoftwareBus`.
- PUS verification-count and TM counter monotonicity (mod 16384) -
  `Frick.Pus`, `Frick.Requirements`.
- CCSDS length arithmetic - `Frick.Requirements`.

verification/results: `make verify-lean` (lake build) is the gate; theorem-by-
theorem discharge is mirrored in the traceability manifest `lean:` links. In the
v0.1.0 review-closure cycle the project was built in the pinned Docker V&V
image: `lake build` compiled 15/15 modules and `lake exe frick_verify_checks`
reported 3/3 runtime checks PASS.

### Unit test results (methods Tst)

| Suite | Result | Coverage note |
| ----- | ------ | ------------- |
| test_ccsds | PASS | packet headers, length fields, CRC detect/append |
| test_sb    | PASS | exact and wildcard subscriptions, FIFO, depth |
| test_tbl   | PASS | load/validate/activate, atomic activation |
| test_pus   | PASS | dispatch, S1 verification packets, S17 ping, S6 region bound |

### System demonstration (method Dem)

Native simulator boots with 6 applications (scheduler, command ingest, housekeeping,
health & safety, sample telemetry, TM output); EVS events flow to the PUS
service 5 telemetry stream on the loopback link. Ground tool `ff_gsw.py` can
inject a ping and decode the returned service 17 telemetry.

### Traceability (norms N3/N4)

`make trace` exits 0: 21 SRS, 15 SSS, 6 GAP items, every id present in the
documents, every srs -> sss parent resolved, every Lean link resolved, every
`ci:*` evidence claim declared in a workflow.

### Open non-conformances (norm N12)

None open at the v0.1.0 baseline. Class-A gaps (GAP-A1..A6) are tracked in
`docs/quality/class-a-gaps.md` and are not NCs for the class-B baseline.

## M2 time base notes (2026-10-10)

### Formal verification (norm N1)

The M2 model `Frick.Time` adds tones and leap arithmetic on top of the
existing modules:

- Time-source enum is a proper, disjoint set; CUC seconds never regress across
  a tone boundary under a non-negative pending leap (SRS-REQ-021).
- Each valid tone advances the second counter by exactly one second and the
  tone counter strictly increases (SRS-REQ-022).
- The pending leap is applied and cleared exactly at the tone boundary
  (SRS-REQ-023).
- CUC field round trip is lossless, the fine fraction of any sub-second count
  fits 16 bits, and the coarse 4-octet decomposition recombines (SRS-REQ-024).

Gate: `lake build` compiled 17/17 modules and `lake exe frick_verify_checks`
reported 3/3 runtime checks PASS. The clean-rebuild transcript is committed at
`verification/lean/lake-build.log` so the module count is re-derivable from the
repository (R2 M2-05, norm N11). Lean 4 core v4.10.0 only (no Mathlib); the
proofs use `omega` and structural induction (`verification/lean/lake-manifest.json`
has no dependencies).

### Unit test results (methods Tst)

| Suite | Result | Coverage note |
| ----- | ------ | ------------- |
| test_ccsds | PASS (15 checks) | packet headers, length fields, CRC detect/append |
| test_sb    | PASS (20 checks) | exact and wildcard subscriptions, FIFO, depth |
| test_tbl   | PASS (9 checks) | load/validate/activate, atomic activation |
| test_pus   | PASS (29 checks) | dispatch, S1 verification packets, S17 ping, S6 region bound, S9 set-time 6-octet enforcement |
| test_time  | PASS (101 checks) | source selection, tone discipline (reject/jump/epoch), leap at boundary, no-tone pending, leap non-regression/skipped second, CUC encode/decode + fixed reference frame `{0x2E,0,0,0,5,0x40,0x00}` |

### Code coverage (method Anl, gcov)

`make coverage` (gcov, all suites executing): lines **845/1446 (58.4%)**,
functions **114/156**. Recorded in `build/coverage/coverage.json` and the README
metrics panel. The line count rose from the previously recorded 579/1310
(44.2%) because the M2 `time` suite previously did not execute (R2 M2-01) and
now does.

### Traceability (norms N3/N4)

`make trace` exits 0: 25 SRS, 16 SSS, 7 GAP items (incl. new GAP-A7), every id
present in the documents, every srs -> sss parent resolved, every Lean link
resolved, every `ci:*` evidence claim declared in a workflow. New links:
SSS-REQ-016, SRS-REQ-021..024.

### Non-conformance closure (norm N12)

The M2 delta was reviewed independently by three profiles; all non-conformances
were resolved to compliance and the deltas re-reviewed (norm N12):

| Review | Profile | Findings | Disposition |
| ------ | ------- | -------- | ----------- |
| `2026-10-10-m2-time-base-r1-review.md` | R1 | 6 Minor + 2 Comments | Resolved (test determinism, TONE first-tone semantics + ARCH, tone-state mutex, PPS start/stop state, CUC macro `_Static_assert`s, tone-spacing dispositioned to GAP-A7, priority/stack note, extra tests) |
| `2026-10-10-m2-vv-r2-review.md` | R2 | 2 Major + 4 Minor | Resolved (dead `main` fixed, tone test made clock-independent, `trace.py` `ci_evidence` scoping, Gitea `ci-lean` marker, Lean transcript committed, coverage/metrics regenerated) |
| `2026-10-10-m2-sss-r3-review.md` | R3 | 1 Major + 4 Minor + 3 Comments | Resolved (CUC two-form wire truth in ICD/ARCH/SRS, source-switch semantics, tone-second-counter wording, no-tone leap clause, `ecss-criticality-b.md` GAP-A1..A7) |

Open non-conformances: **none**. On-target PPS capture and the TAI/CCSDS epoch
remain a documented class-A gap (GAP-A7, `docs/quality/class-a-gaps.md`), not an
NC for the class-B baseline.

## M3 hardware abstraction and ECSS documentation notes (2026-10-10)

### Delivered

The HAL registry and the stack-independent device/bus/memory contracts
(SSS-REQ-017, SRS-REQ-025..032), plus the ECSS engineering set: Software Design
Document, configuration management plan, product/quality assurance plan,
validation plan, preliminary safety/FDIR analysis and the ECSS compliance
matrix; gaps to class A extended to GAP-A8..A13.

### Formal verification (norm N1)

`Frick.Hal` proves the HAL registry bound and duplicate invariance
(`hal_registry_count_bound`, `hal_dup_registration_unchanged`, wrapped as
`req_srs025_registry_bound` / `req_srs025_dup_unchanged`). Gate: `lake build`
19/19, `frick_verify_checks` 3/3; transcript committed
(`verification/lean/lake-build.log`).

### Unit tests (methods Tst)

6/6 suites: ccsds 15, sb 20, tbl 9, pus 29, time 101, **hal 31** checks
(205 total).

### Inspection evidence (methods Inp)

SRS-REQ-026..032 are **interface declarations** (non-executable). Their
verification is inspection of the contract headers plus the contract-presence
test (`tests/unit/test_hal.c::test_contract_presence`, which compiles and
instantiates every contract type). This is the recorded `Inp` evidence for
these requirements (manifest `lean_note`).

### Coverage (method Anl, gcov)

`make coverage`: lines **893/1515 (58.9%)**, functions **124/166**.

### Traceability (norms N3/N4)

`make trace` exits 0: 63 requirements (33 SRS / 17 SSS / 13 GAP), 45 Lean
theorems, 9 CI evidence claims.

### Non-conformance closure

The M3 delta was reviewed by R1 (design/code), R2 (V&V), R3 (SSS, N14) and R4
(documentation, N17); every non-conformance was dispositioned and the deltas
re-reviewed until closure (norm N12). See the review evidence table below.

## M6 mathematics library notes (2026-10-10)

### Delivered

Portable mathematics facade over the platform `libm` (`src/math/ff_math.{c,h}`,
`FF_USE_LIBM`, `-lm`); requirements SSS-REQ-018 / SRS-REQ-033; ADR-0012 and
policy `docs/architecture/numerics.md`.

### Unit tests (Tst)

7/7 suites; `math` 20 checks (elementary functions within tolerance, helpers,
domain/finiteness). Total 225 checks.

### Inspection (Inp)

The facade is a thin, allocation-free wrapper; the framework core remains free
of floating point. A `lean_note` is recorded for SRS-REQ-033 (third-party libm,
not Lean-checkable).

### Coverage

`make coverage`: lines **932/1575 (59.2%)**, functions **141/184**.

## M7 SDLS security notes (2026-10-10)

### Delivered

The CCSDS SDLS sub-layer (`src/security/ff_sdls.{c,h}`): SDLS processing,
bounded SPI-indexed security-association database, and key lifecycle
(derive/rotate/zeroise) with a **pluggable** cryptographic backend
(`FF_CryptoBackend_t`). Requirements SSS-REQ-019, SRS-REQ-034/-035; ADR-0013;
`docs/architecture/security-sdls.md`.

### Formal verification (Anl, norm N1)

`Frick.Sdls`: `req_srs035_sa_bound`, `req_srs035_epoch_monotonic`,
`req_srs035_uses_reset`. Gate: `lake build` 21/21, 3/3 runtime checks.

### Unit tests (Tst)

8/8 suites; `sdls` **81 checks** (round trip, AUTH-only/ENC-only, tamper
rejection, SA lifecycle, capacity bound, length bounds, removal/reuse,
rotation/epoch with a failed-derivation-safe path, policy tick, zeroisation)
with the non-secure reference backend; **AddressSanitizer clean and wired into
CI** (build job).

### Coverage (Anl, gcov)

`make coverage`: lines **1252/2005 (62.4%)**, functions **175/217**.

### Non-conformance closure

Reviewed by R1/R2 (code/V&V), R3 (SSS, N14) and R4 (documentation, N17); NCs
dispositioned and deltas re-reviewed (norm N12). Cryptographic algorithm/HSM/
ground key management is class-A gap GAP-A14.

## M4 Ethernet driver notes (2026-10-10)

### Delivered (increment 1)

The Cadence GEM Ethernet driver (`src/drivers/eth/gem/`) implementing the
`FF_EthMac_t` HAL contract: static, allocation-free, with injected MMIO
(`FF_GemBus_t`) and HAL `FF_Dma_t` translation so the register/descriptor logic
is host-testable. Requirement SSS-REQ-020 / SRS-REQ-036; ADR-0014. Third-party
integration scaffolding (fetched, pinned, not committed): lwIP and N7/ESA
`asn1-pusc-lib` (`tools/scripts/fetch_third_party.sh`).

### Verification (Tst / Inp)

`test_gem` (23 checks) with a mock MMIO bus: init register programming, MAC
address registers, transmit descriptor + completion, RX dispatch, error
counting. Real PHY/link behaviour needs the board (GAP-A5); register map source:
PolarFire SoC MSS TRM DS60001702.

### Coverage

`make coverage`: lines **1656/2521 (65.7%)**, functions **220/264**. `test_netloop` (328 checks) exercises the loopback `FF_NetStack_t` (bind/listen/connect/accept, bidirectional send/recv, boundary/negative arguments, descriptor exhaustion, full-ring partial send, cross-socket duplicate bind, out-of-range handle rejection).

### Remaining (M4)

SPI/I2C/MMC/eMMC/storage/PCIe/SpaceWire MSS drivers (M4 residual),
N7/ESA `FF_Pus_Backend_t` per-service mapping (M15), filestore/CFDP/service 23
(M15). Delivered since the original list: lwIP adapter, GEM TSN, MSS MMUART,
MSS GPIO, the eNVM NVM backend, the NOR/QSPI MTD backend and the MSS CAN
backend (kept here only as history).

## Documentation review (norm N17)

Every commit is preceded by a documentation review loop by the independent
**Senior SW documentation reviewer** (profile R4). The documentation set
(README/AGENTS/`docs/**`/status) is reviewed for completeness and correctness
against the code, requirements, traceability and CI; findings are resolved and
re-reviewed (norm N12) before commit/push. Reports are stored in `docs/reviews/`.

## Review evidence (norm N9/N11)

| Review | Profile | Date | Conclusion | Report |
| ------ | ------- | ---- | ---------- | ------ |
| Baseline V&V result assessment | R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-baseline-vv-review.md |
| SSS requirements review (norm N14) | R3 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-sss-systems-review.md |
| Norms N1-N16 conflict analysis | R1+R3 | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-rules-conflict-review.md |
| M2 code review (time base) | R1 (independent) | 2026-10-10 | APPROVED WITH COMMENTS, NCs closed | docs/reviews/2026-10-10-m2-time-base-r1-review.md |
| M2 code review delta | R1 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m2-time-base-r1-delta-review.md |
| M2 V&V review | R2 (independent) | 2026-10-10 | REJECTED, NCs closed after fix | docs/reviews/2026-10-10-m2-vv-r2-review.md |
| M2 V&V review delta | R2 (independent) | 2026-10-10 | APPROVED | docs/reviews/2026-10-10-m2-vv-r2-delta-review.md |
| M2 SSS/SRS/CUC review (N14) | R3 (independent) | 2026-10-10 | REJECTED, NCs closed after fix | docs/reviews/2026-10-10-m2-sss-r3-review.md |
| M2 SSS/SRS review delta (N14) | R3 (independent) | 2026-10-10 | APPROVED | docs/reviews/2026-10-10-m2-sss-r3-delta-review.md |
| Documentation set review (N17) | R4 (independent) | 2026-10-10 | REJECTED, findings corrected (delta re-review in cycle) | docs/reviews/2026-10-10-doc-set-r4-review.md |
| Documentation set review delta (N17) | R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (findings corrected) | docs/reviews/2026-10-10-doc-set-r4-delta-review.md |
| Documentation set review final (N17) | R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (all NCs closed) | docs/reviews/2026-10-10-doc-set-r4-final-review.md |
| M3 HAL code/design review | R1 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m3-hal-r1-review.md |
| M3 V&V review | R2 (independent) | 2026-10-10 | REJECTED, NCs closed after fix | docs/reviews/2026-10-10-m3-vv-r2-review.md |
| M3 SSS/SRS review (N14) | R3 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m3-sss-r3-review.md |
| M3 documentation review (N17) | R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m3-doc-set-r4-review.md |
| M3 R1+R2 delta | R1+R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (both R2 Majors closed) | docs/reviews/2026-10-10-m3-r1-r2-delta-review.md |
| M3 R3+R4 delta | R3+R4 (independent) | 2026-10-10 | R3 APPROVED / R4 APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m3-r3-r4-delta-review.md |
| M6 mathematics review | R1+R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m6-math-r1-r2-review.md |
| M6 mathematics review | R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m6-math-r3-r4-review.md |
| M6 mathematics delta | R1+R2+R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-10-m6-math-delta-review.md |
| M7 SDLS code/V&V review | R1+R2 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m7-sdls-r1-r2-review.md |
| M7 SDLS SSS/docs review | R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m7-sdls-r3-r4-review.md |
| M7 SDLS delta | R1+R2+R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m7-sdls-delta-review.md |
| M4 GEM driver review | R1+R2 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m4-gem-r1-r2-review.md |
| M4 GEM SSS/docs review | R3+R4 (independent) | 2026-10-10 | REJECTED (R3), APPROVED WITH COMMENTS (R4) | docs/reviews/2026-10-10-m4-gem-r3-r4-review.md |
| M4 GEM delta | R1+R2+R3+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m4-gem-delta-review.md |
| M4 RTEMS integration review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m4-rtems-r1-r2-r4-review.md |
| M4 RTEMS integration delta | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m4-rtems-delta-review.md |
| M4 GEM TSN backend review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED, Majors fixed | docs/reviews/2026-10-10-m4-gem-tsn-r1-r2-r4-review.md |
| M4 GEM TSN backend delta | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS (Majors closed) | docs/reviews/2026-10-10-m4-gem-tsn-delta-review.md |
| M4 netstack + M8 review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED -> CLOSED (independent deltas/N12 0057/0058/0059/0060/0061/0062) | docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md |
| M4 lwIP + M9 code quality review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m9-codequality-r1-r2-r4-review.md |
| M10 emulated-target review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md |
| Renode env + N18 review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED (F-01 Major) -> resolved + delta re-review | docs/reviews/2026-10-10-renode-env-r1-r2-r4-review.md |
| M4/M11 static RAM pool + DMA review | R1+R2+R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (NCs closed) | docs/reviews/2026-10-11-m4-m11-mem-r1-r2-r4-review.md |
| PUS-C ASN.1 (N7/ESA) default backend review | R1+R2+R4 (independent) | 2026-10-11 | REJECTED -> closed (CRC guard + test) | docs/reviews/2026-10-11-pus-c-asn1-default-r1-r2-r4-review.md |
| README scope + integration/service docs (N17) | R4 (independent) | 2026-10-11 | REJECTED -> closed | docs/reviews/2026-10-11-readme-scope-r4-review.md |
| M11 ETH HAL class (Cadence GEM) review | R1+R2+R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS -> closed | docs/reviews/2026-10-11-m11-eth-r1-r2-r4-review.md |
| M11 NET contract check (loopback) — FSW | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m11-net-r1-review.md |
| M11 NET contract check (loopback) — V&V | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m11-net-r2-review.md |
| M11 NET contract check (loopback) — docs | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m11-net-r4-review.md |
| M11 NET delta | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS -> closed | docs/reviews/2026-10-11-m11-net-delta-review.md |
| PA review of all phase products (N20) | PA (independent) | 2026-10-11 | REJECTED (3 Major) | docs/reviews/2026-10-11-pa-review-c5f51bc.md |
| PA delta re-review of dispositions (N12/N20) | PA (independent) | 2026-10-11 | REJECTED -> dispositions in progress | docs/reviews/2026-10-11-pa-delta-review-077d38b.md |
| M4 netstack delta re-review (N12) — R1 | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (R1-01 closed; R1-02 tracking held) | docs/reviews/2026-10-11-m4-netstack-delta-r1-review.md |
| M4 netstack delta re-review (N12) — R2 | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m4-netstack-delta-r2-review.md |
| N20 governance documentation review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-n20-governance-r4-review.md |
| M4 lwIP + M9 code-quality review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m4-lwip-r1-r2-r4-review.md |
| M4 netstack delta closure (N12) — R1 | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (all R1 closed) | docs/reviews/2026-10-11-m4-netstack-delta-r1-closure.md |
| M4 netstack delta closure (N12) — R2 | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-m4-netstack-delta-r2-closure.md |
| N20 governance documentation closure (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (DOC-01..12 closed) | docs/reviews/2026-10-11-n20-governance-r4-closure.md |
| M4 netstack final confirmation — R1 | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (D-CL closed) | docs/reviews/2026-10-11-m4-netstack-final-r1.md |
| M4 netstack final confirmation — R2 | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (no V&V barrier) | docs/reviews/2026-10-11-m4-netstack-final-r2.md |
| N20 governance final confirmation (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (DOC-13..17 closed) | docs/reviews/2026-10-11-n20-governance-final-r4.md |
| PA final review of all phase products (N20) | PA (independent) | 2026-10-11 | PASS | docs/reviews/2026-10-11-pa-final-review-967f563.md |
| M16 SSS/system requirements review (N14) | R3 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (S0) | docs/reviews/2026-10-11-m16-sss-r3-review.md |
| M16 documentation review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (S0) | docs/reviews/2026-10-11-m16-docs-r4-review.md |
| M16 PA review (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (S0) | docs/reviews/2026-10-11-m16-pa-review.md |
| ECSS traceability (SDD design matrices) | PA (independent) | 2026-10-11 | NC PA-67-01 (Minor) | docs/reviews/2026-10-11-ecss-traceability-pa-review.md |
| Completion-metrics panel — PA approval (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (F1-F5) | docs/reviews/2026-10-11-metrics-panel-pa-review.md |
| Completion-metrics panel — PA delta approval (N12/N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (F1-F5 closed) | docs/reviews/2026-10-11-metrics-panel-pa-delta-review.md |
| Completion-metrics tool T13 source review (N10) | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-metrics-tool-r2-review.md |
| Completion-metrics panel — PA final approval (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (owner approval satisfied) | docs/reviews/2026-10-11-metrics-panel-pa-approval.md |
| Test-coverage gap analysis + tests (V&V) | R2 (independent) | 2026-10-11 | test_r2_gap.c, 487 checks | docs/reviews/2026-10-11-test-coverage-r2-report.md |
| Requirement-level test-gap analysis + tests (systems) | R3 (independent) | 2026-10-11 | test_r3_gap.c, 198 checks | docs/reviews/2026-10-11-requirements-test-gap-r3-report.md |
| New test suites — R1 code review | R1 (independent) | 2026-10-11 | REJECTED (Major TBL bypass) -> fixed | docs/reviews/2026-10-11-test-gap-r1-review.md |
| Test-gap increment — PA review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-test-gap-pa-review.md |
| TBL fix — R1 delta re-review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS (Major closed) | docs/reviews/2026-10-11-test-gap-r1-delta-review.md |
| Test-gap increment — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (all closed) | docs/reviews/2026-10-11-test-gap-pa-delta-review.md |
| NVM backend — R1 code review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r1-review.md |
| NVM backend — R2 V&V review | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r2-review.md |
| NVM backend — R4 docs review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r4-review.md |
| NVM backend — PA review (N20) | PA (independent) | 2026-10-11 | REJECTED (Major F1 process) -> fixed | docs/reviews/2026-10-11-nvm-pa-review.md |
| NVM backend — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (Major closed) | docs/reviews/2026-10-11-nvm-pa-delta-review.md |
| NVM backend — R1 delta review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r1-delta-review.md |
| NVM backend — R2 delta review | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r2-delta-review.md |
| NVM backend — R4 delta review (N17) | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-r4-delta-review.md |
| NVM backend — PA final review (N20) | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS (N20 gate satisfied) | docs/reviews/2026-10-11-nvm-pa-final-review.md |
| NVM backend — PA PA-89-01 closure | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-nvm-pa-89-closure.md |
| NOR/QSPI MTD backend — R1 code review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r1-review.md |
| NOR/QSPI MTD backend — R2 V&V review | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r2-review.md |
| NOR/QSPI MTD backend — R4 docs review | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r4-review.md |
| NOR/QSPI MTD backend — PA review (N20) | PA (independent) | 2026-10-11 | REJECTED (Major F1 process) -> fixed | docs/reviews/2026-10-11-mtd-pa-review.md |
| MTD — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-pa-delta-review.md |
| MTD (QSPI) — R1 delta review | R1 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r1-delta-review.md |
| MTD — R4 delta review | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-r4-delta-review.md |
| MTD — PA final review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-mtd-pa-final-review.md |
| MTD — PA closure (N12) | PA (independent) | 2026-10-11 | APPROVED (MTD loop closed) | docs/reviews/2026-10-11-mtd-pa-99-closure.md |
| Renode board repl — R4 docs review | R4 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-board-r4-review.md |
| Renode board repl — PA review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-board-pa-review.md |
| Renode board repl — PA delta review | PA (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-board-pa-delta-review.md |
| Renode V&V scripts — R2 T17/T15 re-qualification | R2 (independent) | 2026-10-11 | APPROVED WITH COMMENTS | docs/reviews/2026-10-11-renode-resc-r2-requal-review.md |
| Renode board repl — PA D1/D2 closure (N12) | PA (independent) | 2026-10-11 | APPROVED (loop closed) | docs/reviews/2026-10-11-renode-board-pa-d1d2-closure.md |

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | v0.1.0 initial V&V record |
| 0.1.1 | 2026-10-10 | frick-fsw | Review-closure cycle: Lean build fixed and verified, S6 test corrected, trace gate hardened |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2 cycle: time base, PPS tone discipline, CUC framing, leap-at-boundary |
| 0.3 | 2026-10-10 | frick-fsw | M2 review closure: NCs (R1/R2/R3) resolved, deterministic tests, coverage 845/1446, Lean transcript committed, norm N17 documentation review introduced |
| 0.4 | 2026-10-10 | frick-fsw | M3: HAL interfaces + ECSS design/assurance docs; 63 reqs / 45 theorems, coverage 893/1515; M3 reviews R1/R2/R3/R4 and deltas |
| 0.5 | 2026-10-10 | frick-fsw | M6: libm mathematics facade; 65 reqs / 34 srs / 18 sss, coverage 932/1575 (59.2%) |
| 0.6 | 2026-10-10 | frick-fsw | M7: CCSDS SDLS security + key management; 69 reqs / 36 srs / 19 sss / 14 gap, coverage 1252/2005 (62.4%) |
| 0.7 | 2026-10-10 | frick-fsw | M4 (increment 1): Cadence GEM Ethernet driver; 71 reqs / 37 srs / 20 sss, coverage 1440/2245 (64.1%) |
| 0.8 | 2026-10-10 | frick-fsw | M4 (increment 2): RTEMS application integration; 72 reqs / 38 srs / 20 sss |
| 0.9 | 2026-10-10 | frick-fsw | M4 (increment 3): GEM native TSN backend; 73 reqs / 39 srs / 20 sss |
| 0.10 | 2026-10-10 | frick-fsw | M4 (increment 4): reference loopback network stack; 74 reqs / 40 srs / 20 sss; milestone M8 added |
| 0.11 | 2026-10-10 | frick-fsw | M4 (increment 5): lwIP network-stack adapter; 75 reqs / 41 srs / 20 sss |
| 0.12 | 2026-10-10 | frick-fsw | M9: MISRA C:2012 process + declared coding style; 78 reqs / 43 srs / 21 sss |
| 0.13 | 2026-10-10 | frick-fsw | M10: emulated-target verification (Renode/QEMU); 79 reqs / 44 srs / 21 sss |
| 0.14 | 2026-10-10 | frick-fsw | N18/M10: Renode-on-Docker V&V environment + middleware test application (SRS-REQ-044, tool T17); 80 reqs / 45 srs / 21 sss; review FRICK-REVIEW-0036 + delta |
| 0.15 | 2026-10-10 | frick-fsw | Open-source RTEMS 6.2 alignment + reproducible toolchain image (tool T18); `setup_rtems.sh` corrected; nightly runs the open RTEMS Renode image (N18) |
| 0.16 | 2026-10-10 | frick-fsw | Real open RTEMS toolchain built + real cross build OK; Renode functional run of the RTEMS image PASS (`RENODE_TEST: PASS`); real-build defects fixed (pal_rtems headers/err, POSIX_Init decl, toolchain include/lib paths); N18 in force |
| 0.17 | 2026-10-10 | frick-fsw | M4/M11: reference MSS GPIO driver + host unit test (12/12) + Renode MMIO check (`hal_gpio=PASS`); SRS-REQ-047 |
| 0.18 | 2026-10-10 | frick-fsw | M4/M11: reference MSS MMUART serial driver + host unit test (13/13) + Renode MMIO check (`hal_serial=PASS`); SRS-REQ-048 |
| 0.19 | 2026-10-11 | frick-fsw | M4/M11: static RAM pool + DMA helper + host unit test (14/14) + Renode DDR check (`hal_mem=PASS`); SRS-REQ-049 |
| 0.20 | 2026-10-11 | frick-fsw | PUS-C ASN.1 (N7/ESA) default backend (SRS-REQ-050, ADR-0016, build-time generation T19); host 14/14 + Renode PASS |
| 0.21 | 2026-10-11 | frick-fsw | M11: ETH HAL class exercised in Renode (`hal_eth=PASS`, SRS-REQ-036) |
| 0.22 | 2026-10-11 | frick-fsw | M11: NET HAL class (pluggable TCP/IP, loopback stack) exercised in Renode (`hal_net=PASS`, SRS-REQ-039) |
| 0.23 | 2026-10-11 | frick-fsw | Record all 2026-10-11 reviews (MEM, PUS-C ASN.1, README, ETH, NET R1/R2/R4+delta, PA N20 + delta); header date refreshed (PA-10) |
| 0.24 | 2026-10-11 | frick-fsw | Record the N12 delta re-reviews (netstack R1/R2) + the N17 N20-governance review; m4-lwip row; test_netloop 328 checks |
| 0.25 | 2026-10-11 | frick-fsw | R2/R3 test-gap task: new suites `test_r2_gap` (487 checks) + `test_r3_gap` (201 checks); ctest 14 -> 16; trace manifest tests evidence extended; TBL defect fix + `FF_PusInit` idempotence; SDD section 6 requirements->design matrix added |
| 0.26 | 2026-10-11 | frick-fsw | M4/M11: reference eNVM NVM backend (SRS-REQ-055) - `ff_nvm_mpfs`, `test_nvm` (ctest 17/17), `hal_nvm=PASS` in Renode; M11 5/13 |
| 0.27 | 2026-10-11 | frick-fsw | M4: reference NOR/QSPI MTD backend (SRS-REQ-056) - `ff_mtd_mpfs`, `test_mtd` (ctest 18/18); on-SoC QSPI check pending board repl (M11) |
| 0.28 | 2026-10-11 | frick-fsw | Renode V&V environment switched to the BeagleV-Fire board repl (QSPI NOR + system services available); all HAL checks still PASS |
| 0.29 | 2026-10-11 | frick-fsw | M4/M11: reference MSS CAN backend (SRS-REQ-057) - `ff_can_mpfs`, `test_can` (ctest 19/19, 60 checks), `hal_can=PASS` in Renode; M11 6/13 |
| 0.30 | 2026-10-11 | frick-fsw | CAN review NCs: RX accept-all mask fixed (identifier domain, R2-01/PA-09), `test_can` 60 -> 129 checks (regression discriminates the fix; boundary/negative cases; TX wraparound; raw AMR write assert), poll ack re-read, reference clock 125 -> 80 MHz (R1-D-01, MSS PLL range); M11 6/13 |
| 0.31 | 2026-10-11 | frick-fsw | M10 closed: QEMU generic-BSP (`riscv/rv64imafdc`) run (`make qemu`, T20, `qemu-system-riscv64 -M virt`); toolchain image gains the rv64imafdc BSP + QEMU |
| 0.32 | 2026-10-12 | frick-fsw | M10 review NCs (FRICK-REVIEW-0120..0123): generic `qemu-virt-rtems` platform (no MPFS board module), T20 asserts the app count + records the image id, nightly QEMU job |
| 0.33 | 2026-10-12 | frick-fsw | M4/M11: reference MSS I2C backend (SRS-REQ-058, Sprint 3) - `ff_i2c_mpfs.{c,h}`, `test_i2c` (ctest 20/20, 135 checks), `hal_i2c=PASS` in Renode (M11 7/13); divider reference / GAP-A5 noted |
| 0.34 | 2026-10-12 | frick-fsw | I2C review NCs (FRICK-REVIEW-0129..0136): `test_i2c` 135 -> 144 checks (1 MHz code-5 mapping, minimum-SCL boundary, bus-hook validation); mock wording qualified (behavioural, not an independent oracle); minimum achievable SCL documented (R1-01/R3-04); STOP-completion/bus-recovery residual tracked; T14 MISRA reproducible via `make misra-docker` |
| 0.35 | 2026-10-12 | frick-fsw | Restored explicit M4 scope + DDJF (planning increment): PUS-C over IP/TSN, fabric contract, CAN-FD/CANopen/RS-485 DE, PUSopen adapter, M8 GEM TSN Qav/Qci/Qbb + gPTP/CoreTSN; counts 112/69/29/14/51; reviews FRICK-REVIEW-0137/0138/0139 |
