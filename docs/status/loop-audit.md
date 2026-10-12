# Development-Loop Conformance Audit (norm N19)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-AUDIT-001                        |
| Revision                          | 0.21                                   |
| Date                              | 2026-10-12                             |
| Baseline commit                   | 471afb5 + Sprint-6b-2 + S7 (PCIe/SpaceWire) + NET follow-up (audited revision; pre-commit) |
| Norms                             | N19 (this document), N4/N5/N8/N16      |

At the start of every development/verification loop (norm **N19**) this document
compares the **declared objectives** (milestones/OBJs) and **adherence to the
non-derogable rules N1-N20** against the **actual implementation**, and records
each gap with a disposition (closed in the loop, or tracked with owner and
milestone). No gap is left silent.

## 1. Objectives / milestones audit

Declared in `docs/development-plan.md` / `docs/roadmap.md`.

| Objective / milestone | Declared intent | Actual implementation | Gap | Disposition |
| --- | --- | --- | --- | --- |
| M1 baseline | cFS-style core (ES/SB/EVS/TBL/Time/PUS/Link), host sim | delivered | none | closed |
| M2 time base | PPS, CUC, time sources | delivered | none | closed |
| M3 HAL interfaces + ECSS docs | HAL contracts + SDD/CM/PA/… | delivered | none | closed |
| M4 MPFS low-level layers + 1 Gb/s Ethernet | drivers behind HAL (MMUART/CAN/SPI/I2C/QSPI/MMC/GPIO/…) + GEM | **GEM/TSN, GPIO, MMUART, MEM pool, NVM+eNVM done, MSS CAN done (SRS-REQ-057), NOR/QSPI MTD driver host-verified (on-SoC check pending), MSS I2C done (SRS-REQ-058), MSS SPI done (SRS-REQ-069), MSS block-device done (SRS-REQ-070); eMMC/storage/PCIe/SpaceWire pending; restored scope now explicit: PUS-C over IP/TSN, CAN-FD/CANopen/RS-485 DE, PUSopen adapter (SRS-REQ-059..065)** | partial | tracked: M4/M11 (remaining classes) |
| M5 documentation/status program | living docs | in progress | partial | tracked: M5 |
| M6 libm | facade | delivered | none | closed |
| M7 SDLS + key mgmt | SDLS | delivered | none | closed |
| M8 gPTP + fabric CoreTSN | 802.1AS + multi-port/FRER | planned | none yet | tracked: M8 |
| M9 MISRA + coding style | style + MISRA process | declared; **certified checker absent** | GAP-A4 | tracked: GAP-A4 |
| M10 emulated target | Renode/QEMU | **Renode real run PASS (MPFS image); QEMU generic-BSP run PASS (`make qemu`, T20)** | none | closed |
| M11 Renode HAL coverage | all HAL classes exercised | **Peripheral-bound: GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C/SPI/BLOCK/TSN/PCIe done (11/13); NET `FF_NetStack_t` contract check (loopback) done and, on the lwIP-enabled image, an lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS`; packet round-trip + GEM DMA = NC-S6-01/NC-S6-03)**; SpaceWire = NC-S7-01 (no emulated peripheral, contract-level); PCIe done (emulated `MPFS_PCIe` available, `hal_pcie=PASS`); **NET follow-up**: NC-S6-03 co-implemented (GEM 64-bit descriptor addressing); NC-154-01 closed (`test_lwip_netif` host test) | partial | tracked: M11 (GEM DMA full round-trip → M14; SpaceWire → S7 contract-level) |
| M12 OBCP engine (uPython) | n7s-obcp / ECSS engine | planned | none yet | tracked: M12 |
| M13 reference mission services | equipment manager + fault mgmt | planned | none yet | tracked: M13 |
| **M14 multicore/SMP** | SMP on the 4×U54 (after all others) | not started | none yet | tracked: M14/OBJ-22 (after all others) |
| **M15 in-house fundamental PUS-C set (+ CFDP + VFS)** | OBJ-23 (fundamental PUS-C behaviour + CFDP + virtual filestore + S23) | planned | none yet | tracked: M15 (after M14) |
| **M16 in-house OBCP engine development** | OBJ-24 (ECSS-compliant OBCP engine behind `FF_Obcp_t`, after M12) | planned | none yet | tracked: M16 (after M12) |
| OBJ-05 pluggable TCP/IP+TSN | Tuxera/lwIP/libbsd/custom | lwIP integrated; Tuxera = user adapter (documented) | none | closed (documented) |
| **PUS-C ASN.1 (N7/ESA) as default** | integrate N7/ESA PUS-C library as the open/default PUS basis | **reconciled** (build-time generation; `FF_PusBackendN7` default, ADR-0016); see §3 | none | closed (M4/now) |

## 2. Rules audit (N1-N20)

| Rule | Requirement | Actual | Gap | Disposition |
| --- | --- | --- | --- | --- |
| N1 | every generated artifact checked with Lean | pure-logic modules mirrored + 51 theorems; rest `lean_note` | full C refinement absent | GAP-A1 (tracked) |
| N2 | English only | compliant | none | closed |
| N3 | SRS/SSS for all code | compliant (114 reqs) | planned reqs documented-only | tracked: M4-residual/M8/M12-M16 |
| N4 | bidirectional traceability converged | `trace.py` PASS | none | closed |
| N5 | class-B deliverable; class-A gaps documented | class B + GAP-A1..A14 | A1..A14 open | tracked: class-a-gaps |
| N6 | full SW V&V; CI/CD via Docker | CI + V&V image + Renode | RTEMS per-commit proxy; nightly real | documented |
| N7 | push to both remotes | compliant | none | closed |
| N8 | complete/current docs | compliant | some stale-word recurrences found by reviews | closed per review |
| N9 | independent reviews | R1/R2/R3/R4 + PA per change | none | closed |
| N10 | tools qualified | registry T1-T20 | new tools each change | closed |
| N11 | scripts/reviews/results committed | compliant | none | closed |
| N12 | NC close-out loop | compliant | none | closed |
| N13 | AGPL-3.0; licensing reviewed | compliant (+ Eärendil disclaimer) | none | closed |
| N14 | SSS reviewed by R3 | compliant | none | closed |
| N15 | full cycle each change | compliant (native+trace+format+Renode) | Lean/RTEMS proxies | documented |
| N16 | revision history | compliant | none | closed |
| N17 | documentation review (R4) | per change; one N20-change review was initially missed (FRICK-REVIEW-0056 DOC-10) then performed | none | closed |
| N18 | Renode functional validation | **in force** (`make renode` PASS incl. the HAL checks `hal_eth`/`hal_net`/`hal_can`/`hal_i2c`/`hal_spi`/`hal_block`/`hal_tsn=PASS` on the current image) | per-commit SKIP proxy (no image) | documented |
| N19 | this audit each loop | **introduced now** | none | closed (this doc) |
| N20 | every phase product PA-reviewed; findings fixed and reviews repeated until pass | in force; FRICK-REVIEW-0052/0053 findings closed, PA PASS (FRICK-REVIEW-0063) | R1-02 bounded residual (tracked) | closed (residual tracked: loopback R1-02) |

## 3. The PUS-C ASN.1 gap: root cause and reconciliation

**Gap:** the project decision records repeatedly state intent to *integrate the
N7 Space / ESA `asn1-pusc-lib` (ASN.1/ACN PUS-C) as the open/default PUS basis*,
but only `tools/scripts/fetch_third_party.sh` (fetch, pinned) was ever added. The
library is **not referenced** by `src/`, `config/` or `CMakeLists.txt`, and the
default PUS backend is the project's own CCSDS `FF_PusBackendOpen`.

**Root cause (why a gap like this arises):**
1. The decision was recorded in prose (plan/ADR) and in a **fetch** script, but
   never converted into a **testable requirement + milestone deliverable + gate**:
   no SRS-REQ pointed at the integration, and the traceability gate only checks
   that listed implementation paths *exist* — it cannot detect an *absent*
   integration that was never required.
2. `asn1-pusc-lib` is **not a C runtime library**: it is an **ASN.1/ACN model**
   (312 `.asn1` + 311 `.acn`) plus small C wrappers that themselves depend on
   **ASN1SCC-generated** headers (`asn1crt.h`, `BasicTypes.h`). "Wiring it" is
   therefore a **toolchain integration** (ASN1SCC code generation) whose
   enabler was not made explicit; without a requirement/gate, the fetch was
   mistaken for the integration.
3. No **loop audit** existed to compare declared intent with implementation
   (which is exactly what norm **N19** now mandates).

**Reconciliation (this loop):** wire PUS-C ASN.1 (N7/ESA) into build and runtime
and make it the **default** PUS basis:
- Fetch/build **ASN1SCC** (ESA, GPL-with-runtime-exception; generated code and
  runtime are not GPL-restricted) and generate C from `asn1-pusc-lib`
  (`tools/scripts/gen_pus_asn1.sh`, tool T19).
- Add `FF_PusBackendN7` on the generated codecs + wrappers
  (packet error control/CRC, ISO checksum, packet length, UInt16 coder).
- Make it the **default** backend (`FF_PusBackendDefault`), keeping the previous
  open backend and PUSopen as alternatives.
- Requirement SRS-REQ-050; verify (host + Renode). Blocked on the ASN1SCC
  toolchain in the V&V image (see §4).

**Status update (validated this loop):** the ASN1SCC toolchain **builds**
(`tools/docker/asn1scc/Dockerfile`, pinned esa/asn1scc `3124590a…`; T19) and
**generation is proven** — `asn1scc -c -ACN -typePrefix Pusc_` over
`asn1-pusc-lib` produces `BasicTypes.h`, `asn1crt.c/h` and the module codecs.
Remaining: commit/consume the generated code in CMake, implement `FF_PusBackendN7`
and switch the default; then verify (host + Renode) and close SRS-REQ-050.

**Reconciled (build-time, ADR-0016):** `FF_PusBackendN7` is the default
(`FF_USE_PUS_ASN1=ON`, build-time generation via `make pus-asn1`; T19), verified on
host (14/14) and on the emulated SoC (`make renode` PASS). SRS-REQ-050 implemented
for the field/error-control layer; the deep per-service ASN.1 type codecs and the
fundamental service behaviour are **M15**. Falls back to the open backend with a
warning when the generated code is absent.

## 4. Other gaps of the same class (declared-but-not-delivered)

Found by this audit (not previously tracked as such):

1. **PUS-C ASN.1 default** — §3 (being reconciled this loop).
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
| PUS-C ASN.1 not wired/default | **reconciled**: `FF_PusBackendN7` default (build-time; ADR-0016); host `test_pus` + Renode | frick-fsw | M4/now |
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
