# Sprint plan - completing the roadmap

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-PLAN-0002                        |
| Revision                          | 0.12                                   |
| Date                              | 2026-10-12                             |
| Norms                             | N3/N4/N8/N15/N19                       |
| Companion                         | `docs/development-plan.md` (FRICK-PLAN-0001), `docs/roadmap.md` |

This document turns the milestone roadmap into an **ordered sprint plan** that
finishes every development in the roadmap. It is the execution view of
`docs/roadmap.md` / `docs/development-plan.md`; it is kept current per norm **N8**
and re-baselined at every loop start per norm **N19**. Sprint = 2 weeks
(10 working days); effort figures are indicative (the project is executed by an
AI agent under human approval, so calendar time is dominated by review loops,
not by wall-clock).

## 1. Priorities (owner directive)

1. **Drivers first** — finish the M4/M11 HAL reference classes before anything else.
2. **Then M12** (integrate an OBCP engine), **then M16** (in-house OBCP engine
   development), **then M13** (reference mission services).
3. **Class B confirmed**, with a **track toward class A**: the class-A gaps stay
   documented and their closure plans per norm **N5** are maintained (no class-A
   claim is made).
4. **Tools**: open-source or free, preferably with space/aerospace heritage.
5. **M14** after all other milestones; **M15** after **M14**.

Everything is delivered through the same non-derogable cycle (norm N15): full
build/test/Lean/trace/format gates → functional validation in **Renode** (norm
N18) → independent reviews (R1/R2/R3/R4 as applicable) → **Senior SW PA** review
(norm N20) → commit + push to both remotes (norm N7). Each increment closes its
NCs (norm N12) before the loop closes.

## 2. Sprint sequence

| Sprint | Focus | Deliverables | Exit criteria |
| --- | --- | --- | --- |
| **S0** | **Closure & baseline** | Close the open reviews of the M16 change (`FRICK-REVIEW-0064/0065/0066`) and the ECSS traceability NC `PA-67-01` (SDD design matrices); regenerate `traceability.md`; reconcile counts/histories; publish this plan + README section | all open reviews APPROVED (no open Minor); trace 96/55/27/14; docs/semaphores green |
| **S1** | M4/M11 class 1-2 (memory-backed) + M10 close | `FF_Nvm_t` **eNVM** backend (SRS-REQ-055) + `FF_Mtd_t` **NOR/QSPI** backend (SRS-REQ-056); host tests; `hal_nvm` Renode check; **QEMU** run closes M10 — **delivered** | `hal_nvm=PASS` (met); **M10 done** (met: `make qemu`, T20); on-SoC NOR check pending the QSPI controller command driver |
| **S2** | M4/M11 class 3 | **CAN** `FF_CanBackend_t` MSS backend (SRS-REQ-057; the contract is SRS-REQ-029) + host test + Renode check — **delivered** | `hal_can=PASS` (met) |
| **S3** | M4/M11 class 4 | **I2C** `FF_I2c_t` MSS backend (contract SRS-REQ-031; own SRS-REQ-058) + host test + Renode check — **delivered** | `hal_i2c=PASS` (met); SRS-REQ-058 review record closed on the eNVM/MTD/CAN precedent |
| **S4** | M4/M11 class 5 | **SPI** `FF_Spi_t` MSS backend (contract SRS-REQ-031; own SRS-REQ-069) + host test + Renode check — **delivered** | `hal_spi=PASS` (met); M11 8/13 |
| **S4b** | **M4-restored: PUS-C over IP/TSN** | Segmentation/reassembly (SRS-REQ-059) + TCP/IP `FF_Link_t` backend (SRS-REQ-060) + TSN `FF_Link_t` backend (SRS-REQ-061) + host tests + Renode check + reviews | `pus_ip` / `pus_tsn` link round-trip PASS |
| **S4c** | **M4-restored: extended fieldbus** | Generic fabric-device contract (SRS-REQ-067) + CAN-FD fabric backend (SRS-REQ-062) + CANopen (CiA 301/302/402) stack (SRS-REQ-063) + RS-485 half-duplex DE/RE (SRS-REQ-064) + host tests + Renode checks | `hal_canfd` / `canopen` / `rs485` PASS |
| **S4d** | **M4-restored: PUSopen(R) adapter** | Documented adapter over the PUS product contract (SRS-REQ-065); the commercial library is user-supplied | adapter contract test PASS |
| **S5** | M4/M11 class 6 | **BLOCK** `FF_BlockDev_t` eMMC/SD backend (SRS-REQ-032) + host test + Renode check — **delivered** | `hal_block=PASS` (met); M11 9/13 |
| **S6a** | M4/M11 class 7 (part 1) | **TSN** peripheral-bound: bind the GEM native TSN backend to the emulated Cadence GEM and exercise the `FF_TsnBackend_t` seam (SRS-REQ-038) — **delivered** | `hal_tsn=PASS` (met); M11 10/13 |
| **S6b** | M4/M11 class 7 (part 2) | **NET** lwIP-on-GEM peripheral-bound (SRS-REQ-040): **S6b-1** in-repo lwIP RTEMS `sys_arch` port — **delivered**; **S6b-2** RTEMS object sizing + lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS`) — **delivered**; the full TX/RX packet round-trip + GEM DMA remain open (NC-S6-01/NC-S6-03) | `hal_net_lwip=PASS` (netif bound); packet round-trip = follow-up |
| **S7** | M4/M11 classes 8-9 | **PCIe** peripheral-bound (SRS-REQ-031/032; emulated `PCI.MPFS_PCIe @ 0x53004000`) + **SpaceWire** contract-level check (SRS-REQ-032; **no emulated peripheral** → NC-S7-01) | PCIe `hal_pcie=PASS`; SpaceWire contract check PASS |
| **S8** | **M12** OBCP integration | Integrate `n7space/n7s-obcp` behind `FF_Obcp_t` (SRS-REQ-045) with SB/EVS/Time/PUS bindings | M12 done; OBCP round-trip verified |
| **S9-S10** | **M16** in-house OBCP engine | Develop the ECSS-compliant engine behind `FF_Obcp_t` (SRS-REQ-054) with resource/safety bounds + PUS S18 | M16 done; fallback engine verified |
| **S11-S12** | **M13** mission services | Equipment manager + fault management (SRS-REQ-046) layered on PUS | M13 done |
| **S13-S14** | **M8** deterministic Ethernet | 802.1AS gPTP (BMCA/Sync/PDelay/servo) + fabric CoreTSN / 802.1CB (SRS-REQ-038, SRS-REQ-068) + GEM TSN 802.1Qav/Qci/Qbb (SRS-REQ-066) | M8 done |
| **S15+** | **M9 / GAP-A4** and class-A track | Open-source MISRA/static (Frama-C + cppcheck), WCET (Heptane), C refinement (CBMC/Frama-C) for GAP-A1; keep class-A gaps current (N5) | GAP-A4 toolchain qualified; class-A gaps tracked/toward closure |
| **S16-S17** | **M14** multicore/SMP | RTEMS SMP on 4×U54: tasking, per-core IRQ, cache coherency, shared-memory SB (SRS-REQ-051) | M14 done (after all others) |
| **S18-S25** | **M15** fundamental PUS-C + CFDP + VFS | In-house PUS-C service set on `asn1-pusc-lib` + CFDP (727.0-B) + virtual filestore + S23 (SRS-REQ-052/053) | M15 done (after M14); ~34 PM per FRICK-ARCH-0015 |

**Parallel/continuous:** M5 (documentation & status program), the N19
loop-audit at every loop start, and the review/PA gates on every increment.

## 3. Sprint 0 - closure backlog (open non-conformances)

| NC | Source | Severity | Item | Disposition |
| --- | --- | --- | --- | --- |
| PA-67-01 | `FRICK-REVIEW-0067` (ECSS traceability) | Minor | SDD lacks the requirements→design matrices (E-ST-40C Annex F <6>a); no detailed-design ↔ code traces (Annex M <4.4>a.1); compliance matrix overstates design traceability | Add the design matrices to `software-design.md`, add a design-component↔file/unit mapping, make `ecss-compliance-matrix.md` truthful; resolve in S0/S1 (N12) |
| M16-R3-* | `FRICK-REVIEW-0064` | Minor ×7 | SSS-REQ-027 not singular/bounded, method mismatch, M12/M16 split propagation, gap linkage, `traceability.md` not regenerated, execution-order/PUS-S18 dependency | Correct in S0 and re-review (N12) |
| M16-R4-* | `FRICK-REVIEW-0065` | Minor ×7 | stale counts, `traceability.md` mirror, header/History mismatches, roadmap M16 order | Correct in S0 and re-review (N12) |
| M16-PA-* | `FRICK-REVIEW-0066` | Minor ×7 | currency/history obligations for `development-plan`/`handover`/`component-gap-analysis`; loop-audit baseline; semaphores | Correct in S0 and re-review (N12) |
| NC-S6-01 | `FRICK-REVIEW-0148` (scope conformance) | Minor | **NET lwIP-on-GEM peripheral-bound**: partially delivered in S6b-2 (lwIP netif bound to the emulated GEM + MAC configured; `hal_net_lwip=PASS`); the full TX/RX **packet round-trip** and the GEM DMA path remain open (see NC-S6-03) | **Root cause** (resolved part): the pinned lwIP had no RTEMS port (S6b-1) and no netif glue (S6b-2, now `src/hal/net/lwip/ff_lwip_netif.{c,h}`). **Owner**: frick-fsw authoring agent; **sprint**: NET follow-up (after S7). |
| NC-S7-01 | Sprint-6 recon (scope conformance) | Minor | **SpaceWire peripheral-bound** check (SRS-REQ-032/OBJ-19) not deliverable | **Root cause**: the Renode PolarFire SoC model has **no SpaceWire peripheral** (fabric device, "no FPGA IP"), so OBJ-19's "every class peripheral-bound" cannot be met for SpaceWire. **Owner**: frick-fsw authoring agent; resolve in **S7** by providing a **contract-level** `hal_spacewire` check (like the NET loopback contract check) and recording the emulator limitation (GAP-A5). |
| NC-S6-02 | Sprint 6b-1 implementation report | Minor | RTEMS runtime object sizing not wired (no `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`) → runtime `tcpip_init` would fail | **CLOSED in S6b-2**: `config/rtems/rtems_config.c` now sets `CONFIGURE_MAXIMUM_TASKS 24` / `CONFIGURE_MAXIMUM_SEMAPHORES 32` / `CONFIGURE_MAXIMUM_MESSAGE_QUEUES 24` (bounded, justified), and the lwIP image runs `tcpip_init` on the emulated target. |
| NC-151-05 | `FRICK-REVIEW-0151` | Minor | `sys_thread_new` returns an invalid handle silently on task-create failure while `tcpip_init` ignores it → a dead stack with no diagnostic | **CLOSED in S6b-2**: the port now records failures (`frick_lwip_port_thread_failures()` / last status) + `LWIP_PLATFORM_DIAG` under `LWIP_DEBUG`. |
| NC-151-02 | `FRICK-REVIEW-0151` | Minor | The `FF_USE_LWIP=ON` RTEMS cross-build is not reached by any CI job and no link check is added | **CLOSED in S6b-2**: `ci_rtems_cross_build.sh` builds the lwIP Renode image + `nm` link check; `nightly.yml` fetches lwIP, cross-builds with `FF_USE_LWIP=ON` and runs the Renode lwIP check. |
| NC-S6-03 | Sprint 6b-2 implementation report | Minor | **Latent GEM driver defect**: the descriptor address translation (`gem_phys`) returns `uint32_t`, so it cannot express an MPFS DDR address ≥ `0x1000000000`; arming the emulated GEM DMA (NCR RE/TE) makes the descriptor walk fault/loop (observed in Renode). The S6b-2 netif check therefore configures the MAC **without** RE/TE, so the on-emulator GEM DMA path is not exercised | **Owner**: GEM driver author; **sprint**: driver follow-up (before M14); fix the 64-bit descriptor addressing and add a Renode GEM DMA check (related: GAP-A5). |
| NC-154-01 | `FRICK-REVIEW-0154` | Minor | The lwIP netif glue (`ff_lwip_netif.c/h`) is exercised by no test (its `linkoutput`/RX functions need only a mock `FF_EthMac_t`) | **Owner**: frick-fsw authoring agent; **sprint**: NET follow-up (with NC-S6-01). |

The status of every NC remains in its review report's follow-up table (norms
N11/N12); this table is the sprint view only.

## 4. Tool decisions (open/free, space/aerospace heritage)

| Need | Tool | Why (open/free, heritage) |
| --- | --- | --- |
| MISRA C:2012 + static analysis | **Frama-C** (CEA List, France) + **cppcheck** (advisory) | Open source; Frama-C has extensive space/avionics heritage (ESA / French industry) |
| WCET | **Heptane** (INRIA / IRT Saint-Exupéry) | Open source; aeronautics heritage; alternative OTAWA (`aiT` = commercial reference) |
| C→Lean refinement (GAP-A1) | **CBMC** + **Frama-C** (ACSL/WP) | Open source; used in safety-critical/avionics contexts |

No tool is "MISRA-certified": compliance requires a checker with documented rule
coverage and its **qualification evidence** per ECSS-Q-ST-80C / norm N10; the
choice and acceptable coverage require project-authority approval (not asserted
here). WCET on the actual target also needs target/timing evidence (**GAP-A5**).

## 5. Maintenance rule

- This plan is re-baselined at the **start of every loop** together with the N19
  audit (`docs/status/loop-audit.md`) and the milestone status
  (`docs/status/status.json`): completed sprints are ticked, the current sprint is
  re-scoped, and any new NC is added to §3.
- Any change to priorities (owner directive) or to a milestone is reflected here
  in the same commit (norm N8), with a History row (norm N16).
- The README "Roadmap & sprint plan" section and `docs/status/handover.md` point
  to this document as the execution view.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | frick-fsw | Initial sprint plan (priorities: drivers → M12 → M16 → M13 → M8/M9 → M14 → M15; class-A track; open/free tooling); Sprint 0 closure backlog (M16 reviews + ECSS PA-67-01) |
| 0.2 | 2026-10-11 | frick-fsw | S2 (MSS CAN) cites SRS-REQ-057 and marked delivered (`hal_can=PASS`); S1 NVM/MTD done |
| 0.3 | 2026-10-11 | frick-fsw | S1 delivered: M10 closed (QEMU generic-BSP run, T20); next per plan S3 (I2C) |
| 0.4 | 2026-10-12 | frick-fsw | S3 delivered: reference MSS I2C backend (SRS-REQ-058); `hal_i2c=PASS`; M11 7/13; ctest 20/20 (test_i2c 144 checks); counts 100/59/27/14/51; next per plan S4 (SPI) |
| 0.5 | 2026-10-12 | frick-fsw | Expand the undefined token `RR-058` to "SRS-REQ-058 review record" (FRICK-REVIEW-0133 PA-07) |
| 0.6 | 2026-10-12 | frick-fsw | Restore explicit M4 scope: add S4b (PUS-C over IP/TSN), S4c (extended fieldbus: fabric contract/CAN-FD/CANopen/RS-485 DE) and S4d (PUSopen adapter); extend S13-S14 (M8) with GEM TSN 802.1Qav/Qci/Qbb — FRICK-REVIEW-0137 R3-09 |
| 0.7 | 2026-10-12 | frick-fsw | S4 delivered: reference MSS SPI backend (SRS-REQ-069); `hal_spi=PASS`; M11 8/13; ctest 21/21 (test_spi 126 checks); counts 113/70/29/14/51; next per plan S5 (BLOCK eMMC/SD) |
| 0.8 | 2026-10-12 | frick-fsw | S5 delivered: reference MSS block-device backend (SRS-REQ-070); `hal_block=PASS`; M11 9/13; ctest 22/22 (test_block 85 checks); counts 114/71/29/14/51; next per plan S6 (NET/TSN peripheral-bound) |
| 0.9 | 2026-10-12 | frick-fsw | S6 split: **S6a** (TSN peripheral-bound, `hal_tsn=PASS`, M11 10/13) **delivered**; **S6b** (NET lwIP-on-GEM peripheral-bound) deferred with **NC-S6-01** (needs the lwIP RTEMS port in the emulated image + a Renode network path), planned for sprint S6b |
| 0.10 | 2026-10-12 | frick-fsw | Re-plan after recon: NC-S6-01 root-caused (pinned lwIP has no RTEMS port → S6b-1 sys_arch port + S6b-2 netif binding); **NC-S7-01** opened (SpaceWire has no emulated peripheral → contract-level check in S7, GAP-A5); S7 scope adjusted |
| 0.11 | 2026-10-12 | frick-fsw | S6b-1 **delivered**: in-repo lwIP RTEMS `sys_arch` port (host 23/23 + RTEMS build produces the lwIP archive, forced-extraction link resolves the stack); **NC-S6-02** opened (RTEMS runtime object sizing not wired: no `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`), to close in S6b-2 |
| 0.12 | 2026-10-12 | frick-fsw | S6b-2 **delivered**: RTEMS object sizing wired (NC-S6-02 closed), lwIP netif glue bound to the emulated GEM (`hal_net_lwip=PASS`), CI lwIP leg (NC-151-02 closed), thread-failure diagnostic (NC-151-05 closed); **NC-S6-03** opened (latent GEM 64-bit descriptor DMA addressing); NC-S6-01 narrowed (packet round-trip residual) |
