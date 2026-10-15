# Development-Loop Conformance Audit (norm N19)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-AUDIT-001                        |
| Revision                          | 4.9                                    |
| Date                              | 2026-10-15                             |
| Baseline commit                   | `6ce6bd7cacd9e04968d40809009a23fd5410261f` (HEAD, pushed to both remotes); current S4c/S4d/defaults work is uncommitted and not yet covered by a frozen evidence baseline |
| Norms                             | N19 (this document), N4/N5/N8/N16      |

At the start of every development/verification loop (norm **N19**) this document
compares the **declared objectives** (milestones/OBJs) and **adherence to the
non-derogable rules N1-N22** against the **actual implementation**, and records
each gap with a disposition (closed in the loop, or tracked with owner and
milestone). No gap is left silent.

## 1. Objectives / milestones audit

Declared in `docs/development-plan.md` / `docs/roadmap.md`.

| Objective / milestone | Declared intent | Actual implementation | Gap | Disposition |
| --- | --- | --- | --- | --- |
| Project license transition (user-directed priority) | Apply owner-authorized MPL-2.0 to original project-authored content; preserve external terms | Owner authorization recorded in NOTICE/ADR/N13; original source headers and license metadata/check updated; remaining third-party/generated combined-distribution scope is NC-EXT-43 | External/generated compatibility, T21 qualification and full applicable gates remain open; owner authorization itself is no longer pending | **NC-EXT-43 remains OPEN for external scope; NC-EXT-44 T21 and NC-EXT-45 DDJF remain OPEN in S8-FU.** |
| M1 baseline | cFS-style core (ES/SB/EVS/TBL/Time/PUS/Link), host sim | delivered | none | closed |
| M2 time base | PPS, CUC, time sources | delivered | none | closed |
| M3 HAL interfaces + ECSS docs | HAL contracts + SDD/CM/PA/… | delivered | none | closed |
| M4 MPFS low-level layers + 1 Gb/s Ethernet | drivers behind HAL (MMUART/CAN/SPI/I2C/QSPI/MMC/GPIO/…) + GEM | GEM/TSN, GPIO, MMUART, MEM, NVM/MTD, MSS CAN/I2C/SPI/BLOCK, S4b, PCIe, SpaceWire contract, and S4c/S4d code+host tests implemented. Reviews 0288..0307 and bounded deltas have been conducted; NC-S4c-02 is closed only for enumerated source-level defects. CAN-FD target path has no Renode model; PUSopen test is an API double, not vendor SDK validation. On-SoC storage/target evidence remains open | partial | M4 residuals + NC-S4c-01/GAP-A5 |
| M5 documentation/status program | living docs | in progress | partial | tracked: M5 |
| M6 libm | facade | delivered | none | closed |
| M7 SDLS + key mgmt | SDLS | delivered | none | closed |
| M8 gPTP + fabric CoreTSN | 802.1AS + multi-port/FRER | implemented gPTP (BMCA, Sync/Follow_Up, PDelay, servo, `ff_gptp.{c,h}`, 58 host checks; integrates GEM TSU + EthMac); fabric CoreTSN (multi-port/FRER) deferred to M14 (requires SRS-REQ-067 fabric-device contract, NC-S8-02) | partial | tracked: gPTP delivered (M8); fabric CoreTSN M14 |
| M9 MISRA + coding style | style + MISRA process | declared; **certified checker absent** | GAP-A4 | tracked: GAP-A4 |
| M10 emulated target | Renode/QEMU | **Renode real run PASS (MPFS image); QEMU generic-BSP run PASS (`make qemu`, T20)** | none | closed |
| M11 Renode HAL coverage | all HAL classes exercised | **Peripheral-bound: GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C/SPI/BLOCK/TSN/PCIe done (11/13); NET `FF_NetStack_t` contract check (loopback) done and, on the lwIP-enabled image, an lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS`; packet round-trip + GEM DMA = NC-S6-01/NC-S6-03)**; SpaceWire = NC-S7-01 (no emulated peripheral, contract-level); PCIe done (emulated `MPFS_PCIe` available, `hal_pcie=PASS`); **NET follow-up**: NC-S6-03 co-implemented (GEM 64-bit descriptor addressing); NC-154-01 closed (`test_lwip_netif` host test) | partial | tracked: M11 (GEM DMA full round-trip → M14; SpaceWire → S7 contract-level) |
| M12 OBCP engine (uPython) | n7s-obcp / ECSS engine | planned | none yet | tracked: M12 |
| M13 reference mission services | equipment manager + fault mgmt | planned | none yet | tracked: M13 |
| **M14 multicore/SMP** | SMP on the 4×U54 (after all others) | not started | none yet | tracked: M14/OBJ-22 (after all others) |
| **M15 in-house fundamental PUS-C set (+ CFDP + VFS)** | OBJ-23 (fundamental PUS-C behaviour + CFDP + virtual filestore + S23) | planned | none yet | tracked: M15 (after M14) |
| **M16 in-house OBCP engine development** | OBJ-24 (ECSS-compliant OBCP engine behind `FF_Obcp_t`, after M12) | planned | none yet | tracked: M16 (after M12) |
| **M17 RTEMS-backed application scheduler** | OBJ-31; bounded SB schedule dispatcher, RTEMS retains task preemption | current `app_scheduler.c` has shared-timestamp starvation defect | NC-EXT-19 | tracked: S8-SCH after S8-PUS-FU; not implemented |
| OBJ-05 pluggable TCP/IP+TSN | Tuxera/lwIP/libbsd/custom | lwIP integrated; Tuxera = user adapter (documented) | none | closed (documented) |
| **PUS backend defaults** | PUS backend selection and optional N7/ESA backend | Open CCSDS backend is default; `FF_USE_PUS_ASN1=OFF`; optional N7/ESA backend remains behind explicit opt-in. lwIP is default-on and fetched before CMake configuration (ADR-0018) | direct CMake users must fetch lwIP first; hosted CI and broader T23 qualification remain open | implementation received bounded independent reviews; full phase/hosted gates remain open |
| **Bounded SB publish / EVS overflow notification** | User-approved correction following FRICK-REVIEW-0264: SB lock contention is atomic `FF_TIMEOUT`; matched pipe queue-lock contention is non-waiting, drops that copy, continues bounded fan-out, requests a busy-specific deferred EVS error, and returns `FF_TIMEOUT`; full queue remains distinct | PAL queue try-put and deterministic multiple-pipe contention case implemented; SRS-REQ-001 updated only for this approved behavior | SRS-REQ-011 event-envelope values/policy and SRS-REQ-072 end-to-end loss counter/reporting/profile remain external inputs/unimplemented | complete applicable verification and review; keep NC-EXT-75/76 open and preserve SSS/SRS authority TBDs |
| **SB/EVS configured structural characterization** | User directs framework-only characterization without invented mission values | 9,984 host fan-out/depth/burst profiles and dispatch bounds 1–64; retained CSV and source identities; full runner now registered in CTest; latest `make ci-local` 30/30, including sweep in 18.70 s; Lean/trace, docs, smoke/Renode and GSW pass | Not a mission profile, WCET, target timing, link/ground receipt, or requirements-compliance result. R3 found SRS-REQ-072 Lmax scope and SSS-REQ-006 machine trace stale; corrected in this bounded pass. Local CI exercise is evidenced; hosted CI and formal T22 qualification remain pending | Keep mission inputs/TBDs and NC-EXT-75/76 open; no mission-compliance or formal tool-qualification claim; preserve the existing external authority gates |
| **Working-tree scope boundary** | Separate the current S4c/S4d candidate from earlier committed scope | HEAD is `6ce6bd7`; the current candidate is uncommitted and includes source, tests, requirements, planning, CI, and evidence updates (see `git status` and the current evidence note) | Candidate has no commit hash or hosted CI results; bounded independent reviews are recorded, but final R4 and broader release gates remain open | Keep changes uncommitted until applicable gates/reviews pass; no commit/push authorization implied |

## 2. Rules audit (N1-N22)

| Rule | Requirement | Actual | Gap | Disposition |
| --- | --- | --- | --- | --- |
| N1 | every generated artifact checked with Lean | Lean 4.10.0 `lake build` passes; trace detects 62 linked theorems, including the new bounded S4c contracts; rest are explicitly `lean_note` or lack a theorem | full C refinement remains absent; Lean models do not prove C, MMIO or target behavior | GAP-A1 remains open |
| N2 | English only | compliant | none | closed |
| N3 | SRS/SSS for all code | SRS/SSS allocations cover the S4c/S4d changes; trace links include requirements 062..068 | Authority and target gaps remain explicitly open; trace convergence is not proof of completeness | local allocation/model links recorded; R3 review 0290 and corrective delta 0295 completed, mission/board authority gaps remain open |
| N4 | bidirectional traceability converged | `trace.py` PASS: 117 requirements (73 SRS/30 SSS/14 GAP), 62 Lean links | None for current manifest validation; this does not imply requirement compliance | current local trace gate PASS |
| N5 | ECSS Category B target; class-A gaps documented | Category B is project target; no controlled mission assignment evidenced; GAP-A1..A14 | authority input and A1..A14 gaps open | tracked: class-a-gaps; NC-EXT-11 |
| N6 | full SW V&V; CI/CD via Docker | CI + V&V image + Renode | RTEMS per-commit proxy; nightly real | documented |
| N7 | designated-agent commit/push after all gates/norms and full sprint scope; dual remotes | HEAD `6ce6bd7` is the prior baseline; current S4c/S4d/default candidate remains uncommitted and has not been pushed; bounded changed-scope R4 reviews are recorded through 0306 | Overall N17/document-set closure, remaining NC/authority dispositions and hosted GitHub/Gitea results remain outstanding | No commit/push until applicable gates and reviews are complete; no release/flight authorization implied |
| N8 | complete/current docs | SRS/SSS, trace matrices, plan/roadmap, ADR, user-facing descriptions and V&V evidence updated for the current scope; corrections received bounded R4 review | Candidate remains mutable; specific review findings and broader legacy documentation gaps remain | Record and disposition findings; no general document-set approval inferred |
| N9 | independent reviews | Initial FSW/V&V/R3/R4/PA reviews 0288..0292 and bounded deltas 0293..0307 are recorded | Provenance, hosted CI, target/vendor, integration, authority and overall phase/release gates remain open | Correct source-level findings as bounded; keep residual NCs open; no overall phase closure inferred |
| N10 | tools qualified | T23 fetch helper checks pinned HEAD plus tracked/untracked/ignored checkout state; R2/PA reports 0298..0300 reviewed the bounded wrong-HEAD and dirty-state behavior | T23 is not qualified; hosted workflow execution and complete provenance remain absent; mutable images/dependency inputs remain | Do not claim T23 qualification from local builds; retain the independent qualification/hosted gates |
| N11 | scripts/reviews/results committed | Final local-cycle records, dependency-state probes, final Renode/QEMU logs, and reviews 0288..0307 are present in the working tree | Candidate/evidence remain uncommitted; raw logs for some earlier failed attempts are unavailable; worktree is mutable | Preserve all current records; no commit until applicable gates pass |
| N12 | NC close-out loop | open | NC-S4c-01/03/04/05 and other applicable NCs remain open; NC-EXT-01..81 and GAP-R3-1 remain open; NC-EXT-82 is closed only for enumerated reused-checkout integrity behavior by FRICK-REVIEW-0298..0300 (tracked probe added in 0299/0300). NC-EXT-43 tracks external/generated boundaries; NC-EXT-44 T21; NC-EXT-45 DDJF range; NC-EXT-47 N22 evidence mapping; NC-EXT-48..81 bounded review follow-ups | open; scoped NC closures are not aggregate phase closure |
| N13 | owner-authorized MPL-2.0 for original project-authored content | Owner decision recorded; scope excludes externally controlled material | NC-EXT-43 remains open for third-party/generated component boundaries and combined-distribution compatibility | open |
| N14 | SSS reviewed by R3 | R3 review FRICK-REVIEW-0290 and corrective delta 0295 recorded; S4c requirement/interface wording reviewed | Mission SSS/SRS baseline still lacks approved Bmax/Rmax/Dmin/Lmax, link profile, ground boundary and loss counter/report semantics | **OPEN** for mission baseline; bounded review does not close NC-EXT-75/76 or establish system authority approval |
| N15 | full cycle each change | Latest `make ci-local` PASS (36/36, Lean, trace); Debug/Release/ASan CTest each 36/36; format, docs, smoke, GSW, fresh RTEMS MPFS/generic cross-build, Renode and QEMU pass after fetch-script change | Hosted CI not run for this uncommitted candidate; local checks do not close remaining NCs or establish qualification | Local verification cycle recorded; hosted and authority gates remain prerequisites to any commit/push |
| N16 | revision history | compliant | none | closed |
| N17 | documentation review (R4) | S4c/S4d/default docs received initial review FRICK-REVIEW-0291, bounded corrective review 0296 and status/evidence deltas 0301/0302/0304/0306 | Bounded corrections are acceptable; broad N17/document-set approval and remaining NC dispositions are not established | Preserve bounded verdict; do not claim overall N17 or phase closure |
| N18 | Renode functional validation | Current MPFS RTEMS image was cross-built in `frick-rtems-toolchain:local`; `make renode` PASS (`RENODE_TEST: PASS`) | CAN-FD fabric peripheral has no emulator model, so no CAN-FD path is exercised (NC-S4c-01/GAP-A5) | Current Renode cycle passes for exercised scope; explicitly exclude CAN-FD peripheral validation |
| N19 | this audit each loop | Audit updated against HEAD `6ce6bd7` plus current candidate; review reports/deltas, final 171-check cycle, dependency probes, evidence artifacts and residual authority/target gaps reconciled | Worktree remains uncommitted; hosted CI, T23 qualification and several NCs remain open | Audit reflects the current candidate; refresh after new material changes or gate results |
| N21 | correct functional bugs and implement missing features committed for current/previous sprints; log other findings without stopping unrelated development | SB-lock busy returns `FF_TIMEOUT` before mutation; queue-lock busy drops one copy, continues remaining fan-out, increments drop accounting, requests deferred busy event, and returns `FF_TIMEOUT`; full queue follows distinct overflow path | EVS notification can still be lost under EVS-lock contention or saturation; SRS-REQ-072 loss reporting/profile and approved values remain open under NC-EXT-75/76; no full SRS-REQ-011 compliance claimed | implementation and deterministic unit test added; full cycle/reviews pending, NCs remain open |
| N22 | requirement-based/integration tests use nominal and applicable out-of-bound cases with expected-result comparison | SB/EVS CTest boundary oracle plus full 9,984-profile runner now exercised by `make ci-local`, with explicit expected accounting and dispatch oracles | This remains configured host structural characterization, not requirement-based mission test evidence; NC-EXT-47 broader test-evidence mapping remains open | bounded host evidence recorded; no mission coverage claim |
| N20 | every phase product PA-reviewed; findings fixed and reviews repeated until pass | PA reviews FRICK-REVIEW-0292/0297/0300/0303/0305/0307 were conducted; local evidence and tracked-state records were corrected/verified in bounded scope | PA-S4-01..05 and target/vendor/authority findings remain open; review verdict is not phase/release approval | Retain open findings and overall N20 status; no risk/release acceptance inferred |

## 3. PUS and lwIP default configuration reconciliation

The earlier configuration made the N7/ESA ASN.1 backend the default when its
generated files were present and made lwIP opt-in. The current owner direction
reverses those defaults: the open CCSDS PUS codec is default and lwIP is enabled
by default. ADR-0018 records this decision and supersedes ADR-0016 only on PUS
default selection; ADR-0016's optional-backend generation/no-vendoring rationale
remains applicable.

**Current implementation state (this loop):** `CMakeLists.txt` sets
`FF_USE_PUS_ASN1=OFF` and `FF_USE_LWIP=ON`; `fsw_main.c` selects the open backend
in the default configuration. `make configure` and `make ci-local` fetch pinned
lwIP before configure, and both forge workflows do so in each job that performs
a default CMake configure. Direct CMake users must fetch first. The optional N7
backend still requires generated sources and explicit `FF_USE_PUS_ASN1=ON`.
Fresh default host configure/build/test passed with lwIP enabled and PUS ASN.1
disabled (36/36 CTest as of this uncommitted run); this is local evidence only.

**Residuals:** fetched-source provenance/reproducibility remains tracked by
NC-EXT-35/40; external/generated licensing boundaries remain NC-EXT-43. Default
lwIP enablement does not establish target timing, network interoperability,
security, or mission suitability. Bounded independent reviews are recorded;
hosted CI, T23 qualification and final document/release gates remain open. No
release/qualification conclusion is drawn.

## 4. Other gaps of the same class (declared-but-not-delivered)

Found by this audit (not previously tracked as such):

1. **PUS-C ASN.1 default** — reconciled by ADR-0018 and verified in the current
   default build: open CCSDS is selected; N7/ESA ASN.1 is opt-in.
2. **Multicore/SMP** — never planned despite the 4×U54 target; now **M14/OBJ-22**
   (user directive; after all other milestones).
3. **QEMU** emulated target (M10) — **closed**: generic `riscv/rv64imafdc` RTEMS
   image run on `qemu-system-riscv64 -M virt` (`make qemu`, tool T20).
4. **Certified MISRA checker** (M9/GAP-A4) — process only.
5. **Full PUS-C service coverage** — partial (GAP-A2).
6. **Full formal C refinement** — GAP-A1 (Lean model only).
These are now explicitly tracked (M14/GAP-A1/A2/A4/M11/M13); **QEMU (M10) is closed**.

## 5. Dispositions summary

The ordered execution view of these dispositions is `docs/planning/sprint-plan.md`
(FRICK-PLAN-0002).

| Gap | Disposition | Owner | Milestone |
| --- | --- | --- | --- |
| Default PUS backend / lwIP integration | **reconciled by ADR-0018**: open PUS default; optional N7; lwIP ON with fetch-before-configure order; local default build/test PASS, remaining hosted/target review open | frick-fsw | S4c/S4d current cycle |
| Multicore/SMP | tracked | frick-fsw | **M14** |
| In-house fundamental PUS-C service set (+ CFDP + virtual filestore) | tracked (assessment FRICK-ARCH-0015, ~34 PM; hybrid) | frick-fsw | **M15** |
| QEMU target | **closed** (`make qemu`, T20; generic `rv64imafdc` BSP on QEMU `virt`) | frick-fsw | M10 |
| T18 image reproducibility caveat (recorded pass used a hand-assembled image) | tracked: complete one clean `make rtems-toolchain` build + re-run `make qemu`/`make renode` | frick-fsw | M10 closure / next CI run |
| Certified MISRA checker | tracked | frick-fsw | GAP-A4 |
| PUS-C service coverage | tracked | frick-fsw | GAP-A2 / M13 |
| Formal C refinement | tracked | frick-fsw | GAP-A1 / M8 |
| Loopback relay/`pending` orphan (FRICK-REVIEW-0032 R1-02; `backlog` ignored, second `connect` overwrites `pending`) | tracked (bounded 4-slot reference backend; `init()` resets) | frick-fsw | M4/M11 |
| PA-review dispositions (FRICK-REVIEW-0052/0053) | tracked | frick-fsw | now |
| ECSS traceability - SDD requirements->design matrices (FRICK-REVIEW-0067 PA-67-01) | tracked | frick-fsw | **S0/S1** (`docs/planning/sprint-plan.md`) |
| On-SoC QSPI NOR write/erase check (MTD) - board repl `MPFS_QSPI`+`Micron_MT25Q` now loaded; needs the QSPI controller command driver | tracked | frick-fsw | **M11** (environment enabled; driver pending) |
| M16 review NCs (FRICK-REVIEW-0064/0065/0066) | tracked | frick-fsw | **S0** |

## 6. Second-pass audit of the S4b follow-up increment (2026-10-13)

Re-running the N19 check over the increment's own products (the corrected
documents, the records and the follow-up review reports) produced four candidate
deviations. Each was re-verified against the tree before disposition — a
sub-agent's finding is a claim, not a fact:

| # | Candidate deviation | Disposition |
| - | ------------------- | ----------- |
| D1 | The three follow-up review reports (`FRICK-REVIEW-0167`/`0168`/`0169`) record the pre-fix test counts (77 and 101 checks) | **No NC**: a review report is an immutable point-in-time record of what the reviewer measured at review time; the corrected counts are carried by the delta re-review set of this increment. A "point-in-time" note is added to each report so the figures are not mistaken for current state. |
| D2 | `README.md` "Low-level drivers" table + closing paragraph stale (omitted the delivered IP/TSN link backends, PCIe and SpaceWire; classified them as M3/M4 deliverables; `PPS` has no `hal_*.h` contract) and the architecture-diagram node `T2` still read "Link layer: loopback / UART" | **NC-S4b-06** (Minor) — correction implemented; OPEN pending independent delta review (`docs/planning/sprint-plan.md` §3) |
| D3 | The gPTP / fabric-CoreTSN milestone status in `network-stack.md` §6, `software-description.md` §2.1/§3.7/§3.9, `development-plan.md` §M8/§6 and `handover.md` ("milestone M8, planned") contradicted SRS-REQ-068, which records the 802.1AS gPTP stack as implemented (M8) and the fabric CoreTSN path as deferred to M14 | **NC-S4b-07** (Minor) — correction implemented; OPEN pending independent delta review |
| D4 | `handover.md` S4b summary retained defects already corrected elsewhere: "Reference loopback models both UDP and TCP" (the wording corrected under R2-F03), an understated "`-Wall -Wextra -Werror` clean on authored files", and an M4-residual driver list omitting the delivered MSS SPI and MSS MMC/SD (BLOCK) backends | **NC-S4b-08** (Minor) — correction implemented; OPEN pending independent delta review |

## 7. Independent project assessment (2026-10-13) — verified disposition

The user-supplied assessment was based on `57974d2`, while the working tree
contains subsequent increments. The claims below were checked against the
current tree; one independent V&V agent also read the CCSDS decoder and UART
receive path (read-only; contract reminder and N1-N20 norms supplied).

| Finding | Current-tree disposition |
| ------- | ------------------------ |
| C-01 decoder PDL wrap/underflow and unsafe CRC bounds | **Confirmed** in TC and TM decoders. NC-EXT-01 opened; widened PDL validation, minimum secondary-header/CRC checks, exact packet boundary, non-mutating rejection and boundary tests implemented. OPEN pending independent delta review. |
| C-03 CCSDS primary-header validation | **NC-EXT-13 OPEN**: decoders do not validate primary version, secondary-header flag or sequence flags; controlled profile/acceptance policy is missing. Obtain authority-controlled profile before selecting rejection rules. |
| C-13 maximum TC packet exceeds ingest buffer | **Confirmed** (`1024`-byte buffer vs `FF_CCSDS_MAX_TC_PACKET`); NC-EXT-02 opened and buffer changed to the packet-size macro. OPEN pending delta review. |
| T7 ground-tool wire-layout mismatch | **Confirmed** by comparison to the flight codec. NC-EXT-03 opened; TC/TM layouts corrected and fixed golden-vector tests added to local/CI gates. Tool-scope closure pending delta review; independent ground-segment E2E remains an open evidence item. |
| D-04 malformed-TC requirement/test-oracle conflict | **Confirmed**: SRS-REQ-005 demands S1 failure on malformed TC, while `test_pus_malformed_tc_event` expects only an EVS/S5 event and no S1. NC-EXT-04 remains OPEN for authorized systems/requirements disposition; do not alter the requirement or oracle without that decision. |
| C-08 PUS concurrency | **Credible, not closed**: shared global state lacks explicit serialization; NC-EXT-05 open for caller-context/concurrency analysis and disposition. |
| Constant CCSDS sequence count | **Confirmed** (`ff_write_primary` receives zero); NC-EXT-06 open for approved-profile decision and, if required, implementation/test. |
| UART framing | **Confirmed limitation**: one `read()` is returned as one frame, with no accumulation/carry for split or coalesced packets; NC-EXT-07 open, planned in M4/M11 UART-link hardening. |
| Overall GREEN semantics | NC-EXT-08 open pending R4 review of the new README disclaimer. |
| `run_sim.sh` generator conflict | NC-S4b-09 open pending delta review; smoke passes using both the cached Makefiles generator and a fresh build directory. |
| ASan-found mock-register overflow | NC-EXT-09 opened after ASan detected `test_gem_tsn` writing the ENST control offset beyond its 1024-byte mock register array; range increased to 4096 bytes and the full ASan/UBSan suite passes 27/27. OPEN pending delta review. |
| Metrics-panel yellow legend | NC-EXT-10: generator wording corrected and README regenerated; independent R2/R4/PA requalification pending. |
| Category B target vs assigned classification | NC-EXT-11: README/sprint/PA-plan wording now identifies Category B as project target only; authorized mission assignment/tailoring remains open. |
| GSW incomplete-header finite-buffer recovery | NC-EXT-12 remains open overall; FRICK-REVIEW-0173 verified the bounded PDL=0xFFFF candidate-followed-by-valid-packet test, but T7 qualification/PA closure remains pending. |
| CCSDS primary-header policy (C-03) | NC-EXT-13 OPEN; acceptance policy and criterion are authority-controlled and absent. Severity is provisional pending NC-EXT-31; do not characterize as a confirmed violation before disposition. |
| C-04 PUS S1 Request ID | NC-EXT-14 OPEN; correct against controlled ECSS/mission PUS profile; R1/R2/R3 verification planned S8-PUS-FU. |
| C-05 PUS S1 verification behavior | NC-EXT-15 OPEN; failure/acceptance semantics and unknown-service classification need code/test correction; SRS-REQ-005 oracle conflict remains authority-owned in NC-EXT-04. |
| C-06 PUS subtype allocation | NC-EXT-16 OPEN; reconcile S3/S9/S20 and TM[8,2] with controlled PUS definitions/profile, then add service-vector tests. |
| C-07 housekeeping behavior | NC-EXT-17 OPEN; correct behavior that is already specified; periodic-report/reserved-ID policy is not to be invented. |
| C-09 PUS S6 address/region | NC-EXT-18 OPEN; width and memory-ID contract require HAL/system authority input before RV64-safe implementation can be closed. |
| C-10 reference scheduler | NC-EXT-19 OPEN; bug confirmed; plan S8-SCH/M17, distinct from PUS S11. |
| C-11 N7 adapter CRC | NC-EXT-20 OPEN; N7 TC decoder does not validate the CRC claimed by its comment; verify the approved trailer policy, implement check and tests. |
| C-12 PUSopen adapter | NC-EXT-21 OPEN; APID contract and compile/test coverage need a controlled adapter test, using a stub if the licensed library is unavailable. |
| C-14 PUS S11 stub | NC-EXT-22 OPEN; M17 application scheduler does not implement PUS S11; disposition/implementation remains in M15 PUS service work. |
| C-15 duplicate PUS registrations | NC-EXT-23 OPEN; reject duplicates rather than silently shadowing; plan S8-PUS-FU tests. |
| C-16 PUS provider lengths | NC-EXT-24 OPEN; add bounded result validation and regression tests in S8-PUS-FU. |
| T-01/T-05 decoder fuzz/oracle gap | NC-EXT-25 OPEN; deterministic bounded corpus + sanitizer harness in S8-VV-FU; no claim of exhaustive verification. |
| T-08 application tests | Existing GAP-R3-1 remains OPEN; S8-VV-FU plans ES/HS lifecycle and heartbeat integration tests with independent oracle. |
| D-01 TC/TM secondary-header documentation | NC-EXT-26 OPEN; author corrected byte diagram, TC/TM layouts, CRC-included PDL arithmetic and explicit absolute/relative offsets; sequence count is identified as implemented zero pending profile decision NC-EXT-06; independently verify against codec and wire vectors. |
| D-02 PUS backend API documentation | NC-EXT-27 OPEN; `pus-layer.md` vtable corrected to match `src/pus/pus.h`; independent API-documentation re-review pending. |
| D-03 review-evidence publication instructions | NC-EXT-32 OPEN; AGENTS, N11/N14 rules, CONTRIBUTING, review README, independent-review closure criteria and PA plan distinguish working-tree retention from gated commit/push; independent consistency review pending. |
| D-04 sprint precedence | NC-EXT-30 OPEN; plan, README and handover now mark corrective order as proposed and require human owner decision against “drivers first”; no precedence inferred. |
| D-05 N14 review coverage | NC-EXT-33 OPEN; recent SSS-REQ-030 change lacks a recorded independent R3 review; do not represent N14 closed. |
| D-06 V&V history integrity | NC-EXT-34 OPEN; duplicate revision and split table consolidated; source corrected to FRICK-REVIEW-0185; R4 0188 and PA 0189 were scoped. Loop-audit now distinguishes author correction from pending independent disposition; no NC closure claimed. |
| D-07 audit revision integrity | NC-EXT-37 OPEN; FRICK-REVIEW-0191 found header/history mismatch and stale review pointer; author corrected; fresh review pending. |
| R4/PA status and evidence findings | NC-EXT-28..31 OPEN; correct status summaries but retain pending independent review; immutable evidence identity, owner-approved sprint precedence, and severity rationale remain explicit closure dependencies. |

The report's broader assurance gaps already tracked by the project are not
duplicated as NCs: full PUS coverage/datapool and third-party verification are
GAP-A2/GAP-A11; organizationally independent SES/QA is GAP-A3; MISRA/WCET/stack,
fault injection and tool/runtime evidence are GAP-A4; on-target/environmental
qualification is GAP-A5/A6; FMEA/FMECA is GAP-A10; maintenance is GAP-A13. Keep
these GAPs open and schedule their work through the existing class-A/assurance
plan. SAVOIR/OSRA and mission-specific licensing adoption are
benchmark/applicability or program/legal decisions, not project nonconformances
unless adopted as controlled requirements; do not claim conformance or
compatibility without authority/legal disposition. Project-license transition
NC-EXT-43 remains open.

## 8. SB overflow behavior correction (2026-10-14)

The initial correction re-verified the FRICK-ARCH-0002 §2 contract against
`src/core/sb/sb.c`: on a failed non-blocking queue put, the baseline incremented
`dropped` but did not emit EVS. That implementation logged the EVS event after
unlocking SB, but the independent R1 follow-up FRICK-REVIEW-0263 correctly found
that `FF_EvsSend` synchronously called its listener and could block through
TM/UART or recursively re-enter SB. The initial change was functionally
incomplete and is superseded by the deferred-delivery correction recorded below.
No NC closure is claimed.

**Review-reference cross-check:** the checked-in FRICK-REVIEW-0261 finding #1
addresses N21/N22 governance wording, not SB overflow. This task's SB behavior
claim is independently confirmed by the architecture/code comparison above;
the report is not represented as having identified or closed this code defect.

## 9. Scoped deferred EVS delivery correction (2026-10-14)

The R1 follow-up identified that the previous correction had changed the
semantics of all `FF_EvsSend` calls. The current working tree restores ordinary
synchronous `FF_EvsSend` listener delivery and uses `FF_EvsSendDeferred` only
for SB overflow. Deferred notifications occupy a static bounded queue;
`FF_AppTmOutputMain` dispatches at most eight entries from the queue snapshot per
iteration. Callbacks run outside the EVS lock. Recursive dispatcher entry
returns zero; callback-triggered SB overflow enqueues work for a later dispatch.
On queue saturation, the EVS event remains in the circular log, the notification
is dropped, `FF_EvsNotifyStats_t.dropped` increments, and the deferred API
returns `FF_NO_RESOURCE`. `FF_SbPublish` does not propagate that secondary
status, but the log and statistics expose the notification loss. Existing
startup initializes EVS before SB and configures the listener before apps.

The callback-thread blocking concern is mitigated: SB publisher no longer runs
the potentially blocking TM/listener callback. Strictly wait-free/nonblocking
publish is not demonstrated because SB and EVS mutex acquisitions use
`FF_PAL_WAIT_FOREVER`; the architecture requirement is unchanged. Clarify whether
the requirement forbids all mutex contention and provide target timing evidence
through the authorized requirement/assurance owner before claiming that stronger
property. This gap remains open in NC-EXT-75.

The full-pipe regression asserts no callback before explicit dispatch and
delivery later. `test_r2_gap` asserts ordinary `FF_EvsSend` is synchronous;
`test_sb` covers SB re-entry, queue-snapshot behavior, recursive-dispatch
rejection and notification-queue saturation. The direct SB event logs even when
queue saturation drops forwarding; EVS statistics expose the loss. Full cycle
and independent delta review status is recorded in the verification log;
NC-EXT-75 remains open and NC-EXT-47 is not closed.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | frick-fsw | First N19 loop audit: objective/rule conformance; PUS-C ASN.1 gap root cause + reconciliation; multicore M14 |
| 0.2 | 2026-10-11 | frick-fsw | Re-baselined at e1def89: counts 94/54/26/14/51, registry T1-T19, N9 row + PA, N20 in progress, R1-02 residual tracked (PA-review loop) |
| 0.3 | 2026-10-11 | frick-fsw | N19 reconciliation: add the M15 objective row, refresh M14 wording, correct the M11 pending-class count (9) to match development-plan/roadmap |
| 0.4 | 2026-10-11 | frick-fsw | Add the M16 objective row (in-house OBCP engine development) |
| 0.5 | 2026-10-11 | frick-fsw | Link the sprint plan (FRICK-PLAN-0002); track the ECSS traceability NC PA-67-01 and the M16 review NCs in the dispositions summary |
| 0.6 | 2026-10-11 | frick-fsw | M11 5/13 + NVM done (SRS-REQ-055); counts 97/56/27/14; PUS-C ASN.1 row marked reconciled |
| 0.7 | 2026-10-11 | frick-fsw | NOR/QSPI MTD driver (SRS-REQ-056) host-verified; counts 98/57/27/14; on-SoC QSPI task opened |
| 0.8 | 2026-10-11 | frick-fsw | Renode V&V environment switched to the BeagleV-Fire board repl (QSPI NOR + system services reachable) |
| 0.9 | 2026-10-11 | frick-fsw | M4/M11: MSS CAN done (SRS-REQ-057); M11 6/13; counts 99/58/27/14; re-baselined to the CAN increment |
| 0.10 | 2026-10-11 | frick-fsw | M10 closed: QEMU generic-BSP emulated-target run (`make qemu`, T20); M11 6/13; counts 99/58/27/14 |
| 0.11 | 2026-10-12 | frick-fsw | M10 review NCs (FRICK-REVIEW-0120..0123): N10 registry T1-T20; QEMU gap rows closed; QEMU run recorded |
| 0.12 | 2026-10-12 | frick-fsw | Sprint 3: reference MSS I2C backend (SRS-REQ-058) delivered; M11 7/13 peripheral-bound (`hal_i2c=PASS`); counts 100/59/27/14/51; `make renode` PASS on the Sprint-3 image |
| 0.13 | 2026-10-12 | frick-fsw | Sprint-3 review set closed (FRICK-REVIEW-0129..0136: R1/R2/R3/R4/PA initial + R1/R2 delta + PA delta; PA REJECTED -> APPROVED WITH COMMENTS); user-facing TCP/IP + TSN sections and the expanded framework description (FRICK-ENG-0007) added; T14 reproducible via `make misra-docker` |
| 0.14 | 2026-10-12 | frick-fsw | N19 audit: restored the M4 scopes left as prose "absorbed" notes as explicit deliverables (PUS-C over IP/TSN, CAN-FD/CANopen/RS-485 DE, PUSopen adapter; fabric contract; M8 TSN Qav/Qci/Qbb + gPTP/CoreTSN); counts 112/69/29/14/51 — FRICK-REVIEW-0137 |
| 0.15 | 2026-10-12 | frick-fsw | Sprint 4: reference MSS SPI backend (SRS-REQ-069) delivered; M11 8/13 peripheral-bound (`hal_spi=PASS`); counts 113/70/29/14/51 |
| 0.16 | 2026-10-12 | frick-fsw | Sprint 5: reference MSS block-device backend (SRS-REQ-070) delivered; M11 9/13 peripheral-bound (`hal_block=PASS`); counts 114/71/29/14/51 |
| 0.17 | 2026-10-12 | frick-fsw | Sprint 6a: GEM native TSN backend exercised peripheral-bound (`hal_tsn=PASS`, M11 10/13; SRS-REQ-038); NC-S6-01 opened (NET lwIP-on-GEM peripheral-bound deferred to sprint S6b) |
| 0.18 | 2026-10-12 | frick-fsw | Sprint 6b-1: in-repo lwIP RTEMS sys_arch port (SRS-REQ-040); host 23/23 with FF_USE_LWIP=ON, RTEMS build produces the lwIP archive (forced-extraction link resolves the stack); NC-S6-02 (RTEMS object sizing) + NC-S7-01 (SpaceWire no model) recorded |
| 0.19 | 2026-10-12 | frick-fsw | Sprint 6b-2: RTEMS object sizing wired (NC-S6-02 closed), lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS`); NC-151-02/05 closed; NC-S6-03 opened (latent GEM 64-bit descriptor DMA) |
| 0.20 | 2026-10-12 | frick-fsw | Sprint 7: PCIe peripheral-bound (`hal_pcie=PASS`, SRS-REQ-031/032) and SpaceWire contract-level (`hal_spacewire=CONTRACT`, SRS-REQ-032); M11 11/13 peripheral-bound; NC-S7-01 recorded (SpaceWire no emulated peripheral) |
| 0.21 | 2026-10-12 | frick-fsw | NET follow-up: NC-S6-03 co-implemented (GEM 64-bit `gem_phys`/`gem_wr_qbqp`/`TBQP_HI`/`RBQP_HI`/`DMACFG.ADDR64`; per-descriptor 32-bit truncation explicit; full 3-word descriptor format deferred to M14); NC-154-01 closed (`test_lwip_netif` 26/26 host); `test_gem` extended for 64-bit register writes; Renode register-only GEM confirmed (NC-S6-01: mock-MAC loopback host test) |
| 0.22 | 2026-10-13 | frick-fsw | **S8/M8 delivered:** gPTP stack (`src/drivers/eth/gptp/ff_gptp.{c,h}`, BMCA/Sync/Follow_Up/PDelay/PI servo, 58 host checks) + 802.1Qci via GEM screener type 1 (`set_stream_filter`, 22 test_gem_tsn checks); 802.1Qav CBS + 802.1Qbb PFC = `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02); fabric CoreTSN deferred to M14; integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160) corrected; traceability + SRS updated for SRS-REQ-066/068 |
| 0.23 | 2026-10-13 | frick-fsw | **S4b delivered:** PUS-C over IP/TSN (SRS-REQ-059/060/061) — seg/reassembly layer (`link_seg.{c,h}`, 2-byte BE length-prefix, allocation-free), IP backend (`link_ip.{c,h}`, UDP + TCP) over `FF_NetStack_t`, TSN backend (`link_tsn.{c,h}`, EtherType 0x88F5) over `FF_EthMac_t + FF_TsnBackend_t`; ctest 27/27 + trace + Lean + smoke + GSW PASS; Renode SKIP (host-only code). M4 row: restored scope PUS-C over IP/TSN row updated from "pending" to "delivered (S4b)"; on-target IP/TSN link-level = GAP-A5 (Renode GEM DMA register-only) |
| 0.24 | 2026-10-13 | frick-fsw | **S4b delta re-review (N12 closure):** all findings from FRICK-REVIEW-0165 (R1+R2 REJECTED) and FRICK-REVIEW-0166 (PA REJECTED) resolved — 2 Major (R1-01 seg-layer reassembly desync, R1-02 TSN ISR mutex), 10 Minor (R1-03–12), 6 V&V gaps (R2-01–06), 4 PA Minor (PA-MN-01–04), 3 Comments; 3 new findings raised and closed (NF-1 unused variable, NF-2 Major buffer overflow in `rx_partial`, NF-3 doc omission). Delta re-reviews: R1+R2 (FRICK-REVIEW-0165-D1) APPROVED; PA (FRICK-REVIEW-0166-D1) APPROVED. All gates green: build + ctest 27/27 + trace + Lean + smoke + GSW + `-Wall -Wextra -Werror` + clang-format clean |
| 0.25 | 2026-10-13 | frick-fsw | **S4b follow-up (N19 conformance audit):** five S4b deviations opened and closed — NC-S4b-01 (GAP-A5 omitted the IP/TSN on-target residual), NC-S4b-02 (**Major**: `pus_ip`/`pus_tsn` exit criterion unevidenced; `Dem` method of SRS-REQ-060/061 unmet), NC-S4b-03 (stale suite/check counts + `Verfied` typo + stale headers), NC-S4b-04 (`traceability.md` SRS-REQ-059 mirror), NC-S4b-05 (stale M4 pending/remaining lists here and in `verification-results.md`). Added the host PUS-C TM/TC round-trip demonstrations (`test_pus_ip_roundtrip`, `test_pus_tsn_roundtrip`); ctest 27/27; counts unchanged 114/71/29/14/51 |
| 0.26 | 2026-10-13 | frick-fsw | S4b follow-up author corrections were applied to the initial R1+R2/R4/PA findings; the reports FRICK-REVIEW-0167/0168/0169 remain APPROVED WITH COMMENTS and their follow-up tables say delta re-review pending. The initial records do not establish N12/N17/N20 closure. |
| 0.27 | 2026-10-13 | frick-fsw | Second-pass N19 audit identified NC-S4b-06..08 and applied documentation corrections. These NCs remain OPEN pending the independent delta review; the immutable point-in-time counts in review reports are not a deviation. |
| 0.28 | 2026-10-13 | frick-fsw | Independent assessment cross-check in new §7: verify high-risk claims against the current tree; open NC-EXT-01..08 and NC-S4b-09 with sprint/owner dispositions; implement decoder bounds, max TC ingest buffer, GSW wire-format vectors, UART smoke generator and status-interpretation corrections; list unresolved requirement/profile/concurrency/framing matters without inventing project policy. |
| 0.29 | 2026-10-13 | frick-fsw | Follow-up audit reconciles status with actual review evidence: N12 remains open; no FRICK-REVIEW-0167-D1/0168-D1/0169-D1 records exist yet; NC-S4b-06..09 and NC-EXT-01..13 remain open pending delta review/authority disposition. Assessment follow-up code changes, GSW offset fix, test additions, sanitizer fixture correction, current verification record and handover updated; local Renode image now built through RTEMS Docker toolchain and functional run PASS. |
| 0.30 | 2026-10-13 | frick-fsw | Delta-review cross-check: confirm S4b follow-up findings are still open until reports exist; correct N12/semaphore status, add S8-FU sprint mapping and disclose current GSW stream-offset fix and symmetric decoder-test evidence. |
| 0.31 | 2026-10-13 | frick-fsw | Incorporate final R1/R2, R4 and PA review results: NC-EXT-11/12 added; N5 target-vs-assignment clarified; N7/N8/N9/N10/N11/N15/N17/N20 reflect open/uncommitted review state; correct stale sprint, handover, V&V, GAP-A5, README and attribution claims. |
| 0.32 | 2026-10-13 | frick-fsw | Add C-03/NC-EXT-13 after independent R1/R2 found unsupported primary-header fields accepted without controlled profile; record FRICK-REVIEW-0173 limited GSW parser re-review and update current NC status. |
| 0.33 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0173 limited parser re-review, NC-EXT-13 primary-header policy finding, and corresponding status/evidence updates. |
| 0.34 | 2026-10-13 | frick-fsw | Convert current-tree assessment findings C-04..C-16 into NC-EXT-14..24; map existing assurance gaps to GAP-A1..A14 without duplicate NCs; schedule autonomous PUS follow-up and distinguish PUS S11 from M17 scheduler |
| 0.35 | 2026-10-13 | frick-fsw | Add S8-VV-FU for GAP-R3-1 and NC-EXT-25 bounded test/fuzz work; align M17 execution order and retain all findings open pending evidence/review |
| 0.36 | 2026-10-13 | frick-fsw | Record D-01/D-02 documentation mismatches as NC-EXT-26/27; confirm TC header length against source constants and current ICD text |
| 0.37 | 2026-10-13 | frick-fsw | Incorporate latest R4/PA review verdicts; track status, evidence provenance, sprint-order and severity rationale gaps as NC-EXT-28..31 |
| 0.38 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0174/0175 and qualify NC-EXT-13 as an authority-owned policy issue pending approved acceptance criteria and severity rationale |
| 0.39 | 2026-10-13 | frick-fsw | Clarify N20 status from the two current PA FAIL results; retain review gate open |
| 0.40 | 2026-10-13 | frick-fsw | Update N17 to identify FRICK-REVIEW-0174 as the latest rejected R4 review |
| 0.41 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0176/0177 verdicts; update N17/N20 to their latest failed reviews and retain all applicable gates open |
| 0.42 | 2026-10-13 | frick-fsw | Open NC-EXT-32 for review-record publication wording; conservatively set verification semaphores yellow pending complete immutable evidence identity |
| 0.43 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0178/0179 verdicts and update N17/N20 to latest rejected/failed review states |
| 0.44 | 2026-10-13 | frick-fsw | Align N15 with passing pinned gates and yellow status pending NC-EXT-29 evidence identity |
| 0.45 | 2026-10-13 | frick-fsw | Record author corrections to ICD/PUS API documentation; NC-EXT-26/27 remain open pending independent review |
| 0.46 | 2026-10-13 | frick-fsw | Record latest R4 verdict and pending independent verification of NC-EXT-26/27 corrections |
| 0.47 | 2026-10-13 | frick-fsw | Expand NC-EXT-26/27/32 corrective scope to include offsets, source-comment path and publication policy; keep findings open |
| 0.48 | 2026-10-13 | frick-fsw | Record cross-repository N11 clarification in AGENTS/rules and keep NC-EXT-32 open pending re-review |
| 0.49 | 2026-10-13 | frick-fsw | Capture R4 diagram/sequence-count, stale plan pointer, sprint owner-order and review-gate wording corrections; retain affected NCs open |
| 0.50 | 2026-10-13 | frick-fsw | Align live summaries with proposed-not-approved sprint order and N14 review publication gates; NC-EXT-30/32 remain open |
| 0.51 | 2026-10-13 | frick-fsw | Open NC-EXT-33 for missing independent R3 review of SSS-REQ-030; correct N14 status and N12 closure criteria |
| 0.52 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0180/0181 and latest R4/PA findings; keep all relevant gates and NCs open |
| 0.53 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0182/0183 findings; retain ICD, evidence identity and review gates open |
| 0.54 | 2026-10-13 | frick-fsw | Record scoped R4 approval/comments, PA FAIL, and NC-EXT-34 V&V history correction; gates remain open |
| 0.55 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0186/0187; scoped PA PASS does not close phase gates or NC-EXT-34 |
| 0.56 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0188/0189; scoped results do not close NC-EXT-34 or overall N17/N20 |
| 0.57 | 2026-10-13 | frick-fsw | Record T1 requalification gaps NC-EXT-35/36 and add them to N10/N12 audit |
| 0.58 | 2026-10-13 | frick-fsw | Reconcile loop-audit wording per PA-0189 while keeping NC-EXT-34 open pending fresh independent disposition |
| 0.59 | 2026-10-13 | frick-fsw | Reconcile audit header with History and clarify NC-EXT-34 re-review scope after R4 finding |
| 0.60 | 2026-10-13 | frick-fsw | Incorporate R4/PA/R2 reports 0191..0193; retain NC-EXT-35..37 and T1 status gates open |
| 0.61 | 2026-10-13 | frick-fsw | Correct audit header/history revision and explicitly distinguish R2 NC-EXT-35/36 attribution |
| 0.62 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0194 as latest R4 result; NC-EXT-36 remains open pending hosted verification |
| 0.63 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0195..0197 scoped outcomes; N17/N20 and NC-EXT-36 remain open |
| 0.64 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0198/0199 and latest active T1 wording corrections; NC-EXT-35 remains open |
| 0.65 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0200/0201; active wording corrections await independent re-review |
| 0.66 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0202/0203 and NC-EXT-38; latest docs corrections remain pending independent review |
| 0.67 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0204/0205 and latest open NC-EXT-38/39/40 status |
| 0.68 | 2026-10-13 | frick-fsw | Correct handover plan pointer to rev 0.56 and align audit NC range to 40 |
| 0.69 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0206/0207; retain all NCs and current R4/PA gates open |
| 0.70 | 2026-10-13 | frick-fsw | Record corrected CONTRIBUTING/driver/ADR scope wording after 0208/0209; retain NC-EXT-38 open |
| 0.71 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0210/0211 and NC-EXT-41; retain N17/N20 open |
| 0.72 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0212/0213; reconcile active NC range and guide revision metadata |
| 0.73 | 2026-10-13 | frick-fsw | Reconcile handover pointer to sprint-plan rev 0.60; keep review findings open |
| 0.74 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0214/0215 and current corrections pending independent review |
| 0.75 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0216/0217; retain V&V ledger and NC-EXT-41 review gates open |
| 0.76 | 2026-10-13 | frick-fsw | Update handover pointer to sprint-plan rev 0.64; preserve pending review status |
| 0.77 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0218/0219; distinguish scoped PA pass from open N20 gate |
| 0.78 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0221..0223, add NC-EXT-42, and retain R4/PA/N10 gates open |
| 0.79 | 2026-10-13 | frick-fsw | Reconcile current review and NC-EXT-42 status summary after T13 classifier/panel corrections |
| 0.80 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0224..0226 and preserve open N17/N20 status |
| 0.81 | 2026-10-13 | frick-fsw | Update status pointer to sprint-plan rev 0.71; retain open NC-EXT-41/42 |
| 0.82 | 2026-10-13 | frick-fsw | Reconcile latest T13 report IDs and handover/sprint pointers after 0224..0226 corrections |
| 0.83 | 2026-10-13 | frick-fsw | Record latest R2 finding 0227 and make final NC summary/panel language corrections pending review |
| 0.84 | 2026-10-13 | frick-fsw | Update handover and plan pointers after 0227; retain N10/N17/N20 open |
| 0.85 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0228 and current test-count/review-summary correction |
| 0.86 | 2026-10-13 | frick-fsw | Update N10 latest T13 review pointer from 0227 to 0228; no requalification or NC closure implied |
| 0.87 | 2026-10-13 | frick-fsw | Record scoped 0229..0231 outcomes and retain overall N10/N17/N20 gates open |
| 0.88 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0232 scoped handover pass; overall N17 remains open |
| 0.89 | 2026-10-13 | frick-fsw | Start user-prioritized MPL-2.0 transition; audit actual candidate against N1-N20, identify NC-EXT-43, keep N13/N19/N20 open |
| 0.90 | 2026-10-13 | frick-fsw | Record R2/R4/PA findings 0235..0237, add NC-EXT-44, and retain N10/N12/N13/N17/N19/N20 open |
| 0.91 | 2026-10-13 | frick-fsw | Record T21 parser review sequence 0238..0242; 0242 bounded no-new-finding only; retain NC-EXT-44 and T21 qualification open |
| 0.92 | 2026-10-13 | frick-fsw | Record T21 SVG finding/correction and 0245 bounded result; refresh T21/NC-EXT-44 open status |
| 0.93 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0246 DDJF range discrepancy and NC-EXT-45; retain all applicable gates open |
| 0.94 | 2026-10-13 | frick-fsw | Record author correction for DDJF T1..T21 and pending independent R4/PA review; retain NC-EXT-45 open |
| 0.95 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0247 scoped DDJF pass; overall N17/N20 and NC closure remain open |
| 0.96 | 2026-10-14 | frick-fsw | Record owner authorization for original project content; re-scope NC-EXT-43; add PA 0248 unreadable-file finding under NC-EXT-44 |
| 0.97 | 2026-10-14 | frick-fsw | Record bounded R2 0249 pass and R4/PA 0250/0251 scope findings; reconcile active scope wording without closing NCs |
| 0.98 | 2026-10-14 | frick-fsw | Record R4 0252 and PA 0253 status findings, corrected NC-EXT-43 scope and active metadata; retain all gates open |
| 0.99 | 2026-10-14 | frick-fsw | Record R4/PA follow-up findings 0254/0255; distinguish reported metadata discrepancy from confirmed evidence; all gates remain open |
| 1.0 | 2026-10-14 | frick-fsw | Record 0254/0255 author dispositions and current pending re-reviews; preserve all NCs open |
| 1.1 | 2026-10-14 | frick-fsw | Record 0256/0257 status follow-ups and author corrections; no NC closure or release approval |
| 1.2 | 2026-10-14 | frick-fsw | Record bounded R4/PA no-new-finding re-reviews 0258/0259; keep all applicable gates open |
| 1.3 | 2026-10-14 | frick-fsw | Align current audit revision to the latest bounded R4/PA review cycle; no overall approval or NC closure |
| 1.4 | 2026-10-14 | frick-fsw | Record non-derogable N21 and track non-functional current-status findings as NC-EXT-46 without stopping development |
| 1.5 | 2026-10-14 | frick-fsw | Record N22 and track existing test-oracle/coverage assessment as NC-EXT-47 without stopping unrelated development |
| 1.6 | 2026-10-14 | frick-fsw | Update N7/N21/N22 audit state, record FRICK-REVIEW-0260/0261 and NC-EXT-48..68, and preserve review/verification gates as open |
| 1.7 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0262 and NC-EXT-69..74; reflect scoped R4 result without claiming overall N17/N20 closure |
| 1.8 | 2026-10-14 | frick-fsw | Audit SB overflow behavior against FRICK-ARCH-0002, record the safe EVS correction/regression and exact cycle gate status; preserve NC-EXT-47 and review gates as open |
| 1.9 | 2026-10-14 | frick-fsw | Add FRICK-REVIEW-0263 and NC-EXT-75; update NC range and record that functional correction awaits independent delta review |
| 2.0 | 2026-10-14 | frick-fsw | Correct the R1-identified synchronous EVS callback defect with bounded deferred TM-output dispatch; add re-entry and queue-overflow evidence; keep NC-EXT-75 open pending independent delta review |
| 2.1 | 2026-10-14 | frick-fsw | Scope deferred delivery to explicit `FF_EvsSendDeferred` for SB overflow; restore synchronous ordinary sends; retain strict mutex/nonblocking ambiguity as open NC-EXT-75 evidence |
| 2.2 | 2026-10-14 | frick-fsw | Update current-cycle evidence for scoped API behavior; document RTEMS cache limitation and leave nonblocking-claim disposition/NC-EXT-75 open |
| 2.3 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0264 and NC-EXT-76; update current NC range and preserve the unresolved no-block/SRS-REQ-011 conflict as functional work |
| 2.4 | 2026-10-14 | frick-fsw | User-approved bounded-failure correction: add PAL try-lock, pre-mutation SB timeout, EVS deferred lock timeout, synchronized contention tests, and update exact cycle evidence; preserve SRS/SSS authority boundary and NC-EXT-75/76 open |
| 2.5 | 2026-10-14 | frick-fsw | Draft SSS-REQ-006/SRS-REQ-072 event-arrival and dispatch envelope with authority-owned TBD values; add trace/test gaps; update active trace count to 117 and retain N14/NC-EXT-75/76 gates |
| 2.6 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0265/0266 R3 results, correct traceability counts, register NC-EXT-77, and preserve N14/N15/N17/N20 gates on the uncommitted final-tree delta |
| 2.7 | 2026-10-14 | frick-fsw | Implement approved non-waiting queue-mutex behavior and partial fan-out; update SRS-REQ-001, TS-5/TS-6 and audit results while retaining NC-EXT-75/76 and SRS-REQ-072 profile/loss reporting open |
| 2.8 | 2026-10-14 | frick-fsw | Record final host/container Lean/trace, smoke, Renode, format, GSW and advisory scan results; retain RTEMS-cache/Doxygen permission limitations and independent review gates |
| 2.9 | 2026-10-14 | frick-fsw | Re-audit exact dirty-tree scope (234 tracked + 120 untracked), correct R3 SRS/trace findings, clarify N10's actual CI-exercise gap for T22, and explicitly freeze further review expansion pending a new scope snapshot |
| 3.0 | 2026-10-14 | frick-fsw | Register the full T22 sweep runner in CTest and record 30/30 local CI pass; Doxygen container recovery is recorded; worktree scope and independent/hosted gates remain open |
| 3.1 | 2026-10-14 | frick-fsw | Reconcile T22 audit status with local full-matrix CTest and scoped R2/R4/PA reviews; preserve hosted/immutable evidence and aggregate worktree-scope gates |
| 3.2 | 2026-10-15 | frick-fsw | New loop: fix the CI `docs` job failure (`make docs` on a fresh checkout; Doxygen 1.9.8 creates `OUTPUT_DIRECTORY` with a single-level `mkdir`, so `build/doxygen` cannot be created when `build/` is absent) by running `mkdir -p build/doxygen` in the `docs` target; verify on a pristine tree and record; hosted CI, T21, NC-EXT-43 and release gates remain open |
| 3.3 | 2026-10-15 | frick-fsw | Add the `make docs` output-existence guard in the `docs` target and the `docs`-job assertion + `doxygen --version` echo (PA FRICK-REVIEW-0284 Comment 5); bring the baseline-commit and §1 scope-boundary snapshot and the N7/N11/N15/N19 statements into line with the committed/pushed state (`03773f7`, `620fff8`; GitHub Actions run #83 green, Gitea run unevidenced), extend N12 to NC-EXT-81, and record NC-EXT-79 as the open `ci:docs` register mapping; scoped R4 FRICK-REVIEW-0285 and PA FRICK-REVIEW-0286 reviewed the delta APPROVED WITH COMMENTS; hosted-CI qualification evidence, T21, NC-EXT-43 and release gates remain open |
| 3.4 | 2026-10-15 | frick-fsw | Apply the scoped R4 FRICK-REVIEW-0285 and PA FRICK-REVIEW-0286 findings (complete the loop-audit currency and hosted-CI evidence anchor, extend N12 to NC-EXT-81, correct handover currency/NC ranges, split the guard into its own `CHANGELOG.md` section with the sync obligation) and record the bounded R4 delta re-review FRICK-REVIEW-0287 closing NC-01/NC-02; hosted-CI qualification evidence, T21, NC-EXT-36/43 and release gates remain open |
| 3.5 | 2026-10-15 | frick-fsw | Record the committed guard delta (`77cd548`) and its green hosted GitHub Actions run #84 (all 8 jobs, `documentation set` included) in the baseline/N15/N19 rows; the Gitea run log remains unevidenced (NC-EXT-36), and hosted-CI qualification evidence, T21, NC-EXT-43 and release gates remain open |
| 3.6 | 2026-10-15 | frick-fsw | Record hosted run #85 for the record-only commit `03366f1` (all 8 jobs) and adopt the record-only convention in N15 — record-only/evidence-only commits are verified by their own hosted run but not recursively re-recorded, terminating the record → run → record regress; the Gitea run log remains unevidenced (NC-EXT-36) and hosted-CI qualification evidence, T21, NC-EXT-43 and release gates remain open |
| 3.7 | 2026-10-15 | frick-fsw | N19 re-audit at baseline 6ce6bd7 plus uncommitted scope: reconcile M4 S4c/S4d and NC-S4c-01, record ADR-0018 open-PUS/lwIP-on defaults and current-cycle gate status; local default host suite 36/36, remaining gates pending |
| 3.8 | 2026-10-15 | frick-fsw | Record final local verification cycle (36/36 Debug/Release/ASan, Lean, trace, format, docs, smoke, GSW, RTEMS cross-build, Renode and QEMU), exact artifact/image hashes and failed attempts; correct N1/N3/N4/N7/N8/N9/N10/N11/N15/N17/N18/N19/N20 dispositions; independent reviews and hosted CI remain pending |
| 3.9 | 2026-10-15 | frick-fsw | Reconcile audit with completed reviews 0288..0297, final 171-check/full local cycle and retained emulator logs; preserve open provenance, dependency, hosted, target/vendor and authority NCs |
| 4.0 | 2026-10-15 | frick-fsw | Record FRICK-REVIEW-0298 and scoped closure of NC-S4c-02/NC-EXT-82; update N9/N10/N11/N15/N19 without claiming tool qualification, phase closure or release approval |
| 4.1 | 2026-10-15 | frick-fsw | Record R2 tracked-state follow-up 0299, preserve the 0298 point-in-time boundary, and retain PA/R4/hosted/T23 qualification gates |
| 4.2 | 2026-10-15 | frick-fsw | Record PA FRICK-REVIEW-0300 and update the remaining R4 scope and N9/N10/N11/N20 dispositions; preserve all nonlocal and authority gates |
| 4.3 | 2026-10-15 | frick-fsw | Correct stale review-status wording after R4 review 0301; bounded reviews are complete while final R4/hosted/qualification/authority gates remain open; cite NC-EXT-82 deltas 0298..0300 |
| 4.4 | 2026-10-15 | frick-fsw | Record R4 FRICK-REVIEW-0301 and update N9/N17 review scope; preserve overall N17, hosted, qualification, target/vendor and authority gates |
| 4.5 | 2026-10-15 | frick-fsw | Record R4/PA final evidence reviews 0302/0303 and update N9/N11/N17/N20 pointers; preserve all broader open gates |
| 4.6 | 2026-10-15 | frick-fsw | Extend M4 review pointer through 0303 after final R4/PA evidence deltas; keep NC closures and residual gates bounded |
| 4.7 | 2026-10-15 | frick-fsw | Record evidence chronology follow-ups 0304/0305 and update N9/N11/N17/N20 pointers without extending any technical or release claim |
| 4.8 | 2026-10-15 | frick-fsw | Record R4/PA timing chronology reviews 0306/0307 and update review pointers; retain all broader open gates |
| 4.9 | 2026-10-15 | frick-fsw | Clarify N7: bounded current documentation reviews are complete through 0306, while overall N17, NC/authority, hosted and full-scope gates remain open |
