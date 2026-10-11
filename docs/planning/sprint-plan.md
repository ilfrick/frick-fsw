# Sprint plan - completing the roadmap

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-PLAN-0002                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-11                             |
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
| **S1** | M4/M11 class 1-2 (memory-backed) + M10 close | `FF_Nvm_t` **eNVM** backend (SRS-REQ-055) + `FF_Mtd_t` **NOR/QSPI** backend (SRS-REQ-056); host tests; `hal_nvm` Renode check; **QEMU** run closes M10 | `hal_nvm=PASS`; M10 done; on-SoC NOR check pending the QSPI controller command driver |
| **S2** | M4/M11 class 3 | **CAN** `FF_CanBackend_t` MSS backend (SRS-REQ-029) + host test + Renode check | `hal_can=PASS` |
| **S3** | M4/M11 class 4 | **I2C** `FF_I2c_t` MSS backend (SRS-REQ-031) + host test + Renode check | `hal_i2c=PASS` |
| **S4** | M4/M11 class 5 | **SPI** `FF_Spi_t` MSS backend (SRS-REQ-031) + host test + Renode check | `hal_spi=PASS` |
| **S5** | M4/M11 class 6 | **BLOCK** `FF_BlockDev_t` eMMC/SD backend (SRS-REQ-032) + host test + Renode check | `hal_block=PASS` |
| **S6** | M4/M11 class 7 | **NET** lwIP-on-GEM peripheral-bound + **TSN** peripheral-bound (SRS-REQ-040/038) under Renode | `hal_net` / `hal_tsn` peripheral-bound PASS |
| **S7** | M4/M11 classes 8-9 | **PCIe** (SRS-REQ-031/032) + **SpaceWire** (SRS-REQ-032) generic fabric backends + Renode checks | M11 13/13 classes exercised; M4 residual drivers done |
| **S8** | **M12** OBCP integration | Integrate `n7space/n7s-obcp` behind `FF_Obcp_t` (SRS-REQ-045) with SB/EVS/Time/PUS bindings | M12 done; OBCP round-trip verified |
| **S9-S10** | **M16** in-house OBCP engine | Develop the ECSS-compliant engine behind `FF_Obcp_t` (SRS-REQ-054) with resource/safety bounds + PUS S18 | M16 done; fallback engine verified |
| **S11-S12** | **M13** mission services | Equipment manager + fault management (SRS-REQ-046) layered on PUS | M13 done |
| **S13-S14** | **M8** deterministic Ethernet | 802.1AS gPTP (BMCA/Sync/PDelay/servo) + fabric CoreTSN / 802.1CB (SRS-REQ-037/038) | M8 done |
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