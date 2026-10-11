# Verification and Validation Results

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RES-001                       |
| Revision                          | 0.21                                   |
| Date                              | 2026-10-10                             |
| Baseline cycle                    | v0.1.0 initial release                 |
| Norms                             | N6, N11 (records committed every cycle)|

This log is the running verification record. Every development/verification
cycle appends a row with the evidence produced; rows reference the committed
artifacts (no external state).

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

`make coverage`: lines **1656/2521 (65.7%)**, functions **220/264**. `test_netloop` (29 checks) exercises the loopback `FF_NetStack_t` (bind/listen/connect/accept, bidirectional send/recv, close unpairing, error paths, out-of-range handle rejection).

### Remaining (M4)

lwIP `FF_NetStack_t` adapter + GEM TSN (`FF_TsnBackend_t`), N7/ESA
`FF_Pus_Backend_t` adapter, MMUART/CAN/SPI/I2C/QSPI/MMC/GPIO drivers, filestore/
CFDP/service 23.

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
| M4 netstack + M8 review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED, Major fixed (netstack bounds) | docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md |
| M4 lwIP + M9 code quality review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m9-codequality-r1-r2-r4-review.md |
| M10 emulated-target review | R1+R2+R4 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md |
| Renode env + N18 review | R1+R2+R4 (independent) | 2026-10-10 | REJECTED (F-01 Major) -> resolved + delta re-review | docs/reviews/2026-10-10-renode-env-r1-r2-r4-review.md |

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