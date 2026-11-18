# Development-Loop Conformance Audit (norm N19)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-AUDIT-001                        |
| Revision                         | 11.21                                  |
| Date                              | 2026-11-18                             |
| Baseline commit                   | `ff52055860c506cb7d124c3507bd2358be134b15` (branch `main`, HEAD at this loop start; the previous loop started at `c166cea36483c3262a64edd63dc4368fcbc119df`). The pre-existing worktree was already dirty; no clean-tree claim. Existing project content was preserved. SHA-256 snapshots recorded before this loop's edits: handover `94fe4975019615298aa4157227acc4418d43507a56394fe9aa862a550ac7c012`, loop audit `c044c6476e38669a130911709c88b9975a514c3e3e7efe3b0cc9e2c340f3bebd`, sprint plan `531a2951d1ba50db50e1218379a00743b7d5d5fa20664a567767fdf917016658`, OBCP study `5593ffab546acd1f5edd21a93ea807a70c62fe1a4d4ab4a70bcf5a6131c6a144`, licensing `18d866102d7b3151a035784a6e65d5e7c4a164ff0a0cb149c5966beff0894048`, ICD `25ab29324bf8863a35b7173f82d9f77a71147c4abdb07564c8e5ee22518853a5`. These are pre-edit identities, not post-edit integrity attestations. |
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
| M4 MPFS low-level layers + 1 Gb/s Ethernet | drivers behind HAL (MMUART/CAN/SPI/I2C/QSPI/MMC/GPIO/…) + GEM | GEM/TSN, GPIO, MMUART, MEM, NVM/MTD, MSS CAN/I2C/SPI/BLOCK, S4b, PCIe, SpaceWire contract, and S4c/S4d code+host tests implemented. Reviews 0288..0307 and bounded deltas have been conducted; NC-S4c-02 is closed only for enumerated source-level defects. A post-0307 CANopen serialization-guard change is host-tested but not independently reviewed (NC-S4c-04 remains open). CAN-FD has no Renode model; PUSopen test is an API double, not vendor SDK validation. On-SoC storage/target evidence remains open | partial | M4 residuals + NC-S4c-01/GAP-A5 |
| M5 documentation/status program | living docs | in progress | partial | tracked: M5 |
| M6 libm | facade | delivered | none | closed |
| M7 SDLS + key mgmt | SDLS | delivered | none | closed |
| M8 gPTP + fabric CoreTSN | 802.1AS + multi-port/FRER | implemented gPTP (BMCA, Sync/Follow_Up, PDelay, servo, `ff_gptp.{c,h}`, 58 host checks; integrates GEM TSU + EthMac); fabric CoreTSN (multi-port/FRER) deferred to M14 (requires SRS-REQ-067 fabric-device contract, NC-S8-02) | partial | tracked: gPTP delivered (M8); fabric CoreTSN M14 |
| M9 MISRA + coding style | style + MISRA process | declared; **certified checker absent** | GAP-A4 | tracked: GAP-A4 |
| M10 emulated target | Renode/QEMU | **Renode real run PASS (MPFS image); QEMU generic-BSP run PASS (`make qemu`, T20)** | none | closed |
| M11 Renode HAL coverage | all HAL classes exercised | **Peripheral-bound: GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C/SPI/BLOCK/TSN/PCIe done (11/13); NET `FF_NetStack_t` contract check (loopback) done and, on the lwIP-enabled image, an lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS`; packet round-trip + GEM DMA = NC-S6-01/NC-S6-03)**; SpaceWire = NC-S7-01 (no emulated peripheral, contract-level); PCIe done (emulated `MPFS_PCIe` available, `hal_pcie=PASS`); **NET follow-up**: NC-S6-03 co-implemented (GEM 64-bit descriptor addressing); NC-154-01 closed (`test_lwip_netif` host test) | partial | tracked: M11 (GEM DMA full round-trip → M14; SpaceWire → S7 contract-level) |
| M12 OBCP engine (uPython) | n7s-obcp / ECSS engine | pinned N7 engine built/linked by default behind `FF_Obcp_t`; host connectivity test passes with test doubles; **VM-instruction-budget slice delivered at host level** (nonzero per-execution budget enforced in VM dispatch path, distinct `BUDGET_EXHAUSTED` result, context reusable after exhaustion, zero-budget fails closed) under project-owned patches applied before `make flat`; `FF_USE_PUS_S18_PROVISIONAL` default-OFF option gates provisional PUS S18 handlers and `test_pus_s18` CTest entry (NOT registered/run by default; mapping NOT an approved ICD, not mission-enabled); M16 placeholder skeleton `src/obcp/ff_obcp_inhouse.{c,h}` added behind `FF_USE_INHOUSE_OBCP` default OFF | NC-S9-01 deferred/open: production service/PUS S18 wire authority, RTEMS/RISC-V, resource bounds, requirement closure, engine selection, target/HIL, independent review, mission acceptance remain open; host adapter evidence only | **IN PROGRESS; do not claim M12/SRS-REQ-045 complete or enable mission scripts.** VM-instruction-budget slice committed and pushed (commit `b9681e8`). M16 placeholder scope-bounded committed and pushed. N18 finalization reconciliation complete: (1) vm.c computed-goto `DISPATCH()` macro now increments `mp_vm_budget_exhaustion_count` (latent — `MICROPY_OPT_COMPUTED_GOTO (0)`); (2) clone `obcpengine.c` reconciled with project patch (`mp_embed_deinit()` + thread-local cleanup on exception branch, boolean flag vs counter); (3) `apply_n7s_obcp_patch.sh` `patch_already_in_place()` requires all markers (no partial-patch acceptance); (4) new test `test_context_reuse_after_interpreter_error` added to `test_obcp_budget.c`. Gates on `build/commit-all` (`FF_USE_INHOUSE_OBCP=ON`): build exit 0, `ctest` 60/60 PASS; `make verify-lean` PASS; `make trace` PASS (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems). Norm N7 full sprint scope and applicable gates remain open; commit/push authorized by owner (norm N7 delegated) after gate verification. Preserve limits in `docs/architecture/obcp-integration-assessment.md`. |
| M13 reference mission services | equipment manager + fault mgmt | planned | none yet | tracked: M13 |
| **M14 multicore/SMP** | SMP on the 4×U54 (after all others) | not started | none yet | tracked: M14/OBJ-22 (after all others) |
| **M15 in-house fundamental PUS-C set (+ CFDP + VFS)** | OBJ-23 (fundamental PUS-C behaviour + CFDP + virtual filestore + S23) | planned | none yet | tracked: M15 (after M14) |
| **M16 in-house OBCP engine development** | OBJ-24 (ECSS-compliant OBCP engine behind `FF_Obcp_t`, after M12) | planned | none yet | tracked: M16 (after M12) |
| **M17 RTEMS-backed application scheduler** | OBJ-31; bounded SB schedule dispatcher, RTEMS retains task preemption | `ff_sched` table-driven dispatcher delivered and committed (`src/core/sched/ff_sched.{c,h}`, `src/apps/app_scheduler.c`) with validation, atomic single-writer double-buffer activation, absolute per-entry release targets, bounded per-poll work and missed/heartbeat/publish-error stats (`FF_SchedGetStats`); host tests in `tests/unit/test_sched.c` (38/38); NC-EXT-19 functionally corrected (old algorithm 30/0/0 vs new 30/6/3) but OPEN pending independent R4/PA review. **TBL load/activate → dispatch coupling (`N22-TS5-66`) delivered 2026-11-10:** the application's `FF_TblActivateCb` re-validates and snapshots the activated image and the scheduler task applies it (`FF_SchedActivate` + `FF_SchedInit`) on its next tick, host-tested with nominal and out-of-bound cases (`test_tbl_sched_coupling.c`, 61 checks); the one-tick deferral budget, mission rates (`N22-TS5-48`) and M14 memory ordering remain open | NC-EXT-19 | tracked: S8-SCH delivered (committed f0e1083); `N22-TS5-66` closed as a test gap; NC-EXT-19/90 open pending independent review and authority inputs; emulated N18 PASS 2026-11-10 (`SCH app started, 4 entries`), target/HIL and timing evidence still GAP-A5 |
| OBJ-05 pluggable TCP/IP+TSN | Tuxera/lwIP/libbsd/custom | lwIP integrated; Tuxera = user adapter (documented) | none | closed (documented) |
| **PUS backend defaults** | PUS backend selection and generated N7/ESA integration | `FF_USE_PUS_ASN1=ON` is default per ADR-0019; pinned common ASN1SCC sources and N7/ESA CRC wrapper are built and tested. Open CCSDS selected explicitly with `OFF`; lwIP remains default-on per ADR-0018 | generated service-specific payload mapping remains M15; current N7 adapter delegates packet framing and PUS fields to reference codec; Gitea and target/runtime evidence remain open | Provisional S3/S6 reference choices and S3/S6 regression updates are in candidate. Latest `make ci-local` passed 43/43 + Lean/trace; no mission-profile, full SDK, or target claim |
| **Bounded SB publish / EVS overflow notification** | User-approved correction following FRICK-REVIEW-0264: SB lock contention is atomic `FF_TIMEOUT`; matched pipe queue-lock contention is non-waiting, drops that copy, continues bounded fan-out, requests a busy-specific deferred EVS error, and returns `FF_TIMEOUT`; full queue remains distinct | PAL queue try-put and deterministic multiple-pipe contention case implemented; SRS-REQ-001 updated only for this approved behavior | SRS-REQ-011 event-envelope values/policy and SRS-REQ-072 end-to-end loss counter/reporting/profile remain external inputs/unimplemented | complete applicable verification and review; keep NC-EXT-75/76 open and preserve SSS/SRS authority TBDs |
| **SB/EVS configured structural characterization** | User directs framework-only characterization without invented mission values | 9,984 host fan-out/depth/burst profiles and dispatch bounds 1–64; retained CSV and source identities; full runner registered in CTest; local `make ci-local` 30/30, including sweep in 18.70 s; GitHub run `38793643786` also passed the committed 36-test suite | Not a mission profile, WCET, target timing, link/ground receipt, or requirements-compliance result. Formal T22 qualification and broader NC-EXT-47 test-evidence mapping remain open | Keep mission inputs/TBDs and NC-EXT-75/76 open; no mission-compliance or formal tool-qualification claim; preserve external authority gates |
| **Working-tree scope boundary** | Keep delivered S8-SCH/S8-VV-FU distinct from the S8-PUS-FU and S9 intake they were committed with | Latest pushed baseline `318f49b` (`1fdbb83` = S8-SCH/S8-VV-FU/S8-PUS-FU + selected N7 OBCP integration batch; `2111968` docs alignment; `5be0e8c` metrics; `e7e48ee` bounded PA manifest review; `318f49b` easy-fixes batch); that batch carries the completed S8-VV-FU independent R2/R3 delta review (`FRICK-REVIEW-0379/0380`) and the R4 `NC-EXT-96` confirmation (0378), the corrected `N22-TS6-57` minimum-PDL sub-case with an enforced CTest time budget, and the NC-EXT-97/98 registrations. CI-local passes 43/43 + Lean/trace; stale-vector failure and corrected pass are retained. | PA closed NC-EXT-91 narrowly for the supplemental 19-file PUS/Scheduler delta identity; broader S8/OBCP identity and NC-EXT-84 provenance remain open. Controlled PUS profile/authority, physical target, hosted CI and sprint closure remain open | Retain pass/fail evidence separately; no commit, sprint completion, qualification or release approval inferred |

| **S8-FDIR-01 Panic/reset-recovery + owner-directed remediation + emulated WDOG0 binding** | Owner responses Telegram 1317/1318: 250 ms single-monotonic local report attempt; actual inter-feed ≤T/4 and absolute-monotonic feed intent. 2026-11-09 user direction: suggest representative literature values for the mission watchdog parameters. | Host-only external harnesses (link-time `nanosleep` and `write` wrappers) for EINTR/EIO, deterministic UART short writes, failed-feed expiry preservation and reset-hook error; saturated-stderr whole-path oracle (notifier observes the expired deadline and the pre-reset reset counter); production reporter wiring case over the loopback deadline-aware writer; exact 96/97/256-character panic-reason counterexample; CUC holdover/UTC-step host tests. **2026-11-09:** test-only emulated WDOG0 binding implemented for the N18 build (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`) against the verified register map (`0x20001000`; `TRIGGER -> MSVP -> TIME -> CONTROL`; read-back before the first keyed refresh), using the pinned model's emulated 156250 Hz clock, `MSVP = TIME`, `TRIGGER = 4095`; feeder moved to absolute monotonic deadlines (`FF_PalTaskDelayUntilNs`); provisional 10 s / 2.5 s mission values (NC-EXT-238). | R1/R3 overlength/exactly-one policy and the unbounded formatting work remain unresolved (R1 0414 rejected for closure of 0409). Flight-target parameters/clock/window remain unauthorized (NC-PAL-01); the emulated profile is not a flight configuration. Whole-path report evidence is host-only; no end-to-end target 250 ms/WCET. Flight builds keep `FF_NOT_IMPLEMENTED`; the emulated binding is integration evidence, not RTPFS160ZT fidelity. | Host totals: watchdog 320, ES/HS 180, UART 72, time 113; **full native CTest 54/54 PASS** on `build2/native`; pinned clang-format 18.1.3 PASS; Lean `lake build` PASS; trace PASS; RTEMS 6.2 cross-build PASS; **N18 Renode PASS 2026-11-09** (`RENODE_TEST: PASS`; WDOG0 armed/fed). Emulated increment pending independent R2/R3/R4/PA review; no gate or NC closed. |

## 2. Historical rules-audit snapshot (N1-N22; 2026-11-09 WDOG0 increment)

> This matrix is retained as the rule audit for its dated historical increment;
> its gate counts and review statements are not current for the 2026-11-13
> UART-inclusive candidate. Current start-of-loop N1-N22 status is §12.2 and the
> current outcome is §12.3. Do not use this historical table as latest gate or
> review status.

| Rule | Requirement | Actual | Gap | Disposition |
| --- | --- | --- | --- | --- |
| N1 | every generated artifact checked with Lean | **PASS on the current tree (2026-11-09):** `lake build` in `verification/lean` reports `Build completed successfully.` and `tools/scripts/lean_elab_receipt.py` elaborates all 11 modules and writes the receipt (sha256 `3410575860331b36284a199bc91484ce88b94ea3503b46fb8b7b8c460abe1f26`). | None for the Lean build itself; the emulated watchdog binding is C code and adds no generated Lean artifact. The Lean model still does not model the hardware binding. | Current-tree Lean PASS; hardware-binding proof remains out of scope (NC-PAL-01) |
| N2 | English only | compliant | none | closed |
| N3 | SRS/SSS for all code | SRS/SSS allocations cover the S4c/S4d changes; trace links include requirements 062..068 | Authority and target gaps remain explicitly open; trace convergence is not proof of completeness | local allocation/model links recorded; R3 review 0290 and corrective delta 0295 completed, mission/board authority gaps remain open |
| N4 | bidirectional traceability converged | `trace.py` PASS: 120 requirements (76 SRS/30 SSS/14 GAP), 68 Lean theorems | None for current manifest validation; this does not imply requirement compliance | current local trace gate PASS |
| N5 | ECSS Category B target; class-A gaps documented | Category B is project target; no controlled mission assignment evidenced; GAP-A1..A14 | authority input and A1..A14 gaps open | tracked: class-a-gaps; NC-EXT-11 |
| N6 | full SW V&V; CI/CD via Docker | CI + V&V image + Renode | RTEMS per-commit proxy; nightly real | documented |
| N7 | designated-agent commit/push after all gates/norms and full sprint scope; dual remotes | Historical fact: `13ebf92`, `e7e48ee` and `318f49b` are present on `origin` and `gitea`; hosted GitHub runs `38793643786` (`13ebf92`), `39134615633` (`e7e48ee`), `39218060325` (`318f49b`, all eight jobs) and `39219583424` (`b6e1f61`, all eight jobs) passed. | Repository evidence reviewed here does not establish that every N7 pre-push gate/authorization was satisfied for those historical pushes; Gitea Actions is unverified. For `318f49b` the applicable gates were green (ci-local build/43-of-43 ctest/Lean/trace, pinned format, smoke, GSW/metrics/license, docker Doxygen, host ASan 43/43) and hosted CI passed all eight jobs, but the batch-scoped N17/N20 reviews were not obtained (NC-EXT-99, explicit owner-dispositioned deferral to the next cycle). | Obtain the deferred batch-scoped reviews recorded as NC-EXT-99; do not infer historical authorization |
| N8 | complete/current docs | Candidate summaries include UART framing, R1/R2 reports, NC-EXT-07 packet-loss/recovery limit, local checks, payload/packet sizing, current candidate manifest and retained logs; verification-results 2.83 records current evidence | Bounded R4/PA reviews 0320..0324 and R2 0327..0329 accepted only enumerated corrections; handover §5 marks stale items historical and points to current item 16. Broader legacy docs remain outside bounded scope | Retain historical/candidate identities and review boundaries; no general document-set approval |
| N9 | independent reviews | Bounded PUS R1/R2 re-reviews 0355/0356 and PA assessments 0354/0361/0364 recorded; scheduler R1/R2 0351/0352, R2 re-review 0353 and PA 0360 recorded; R4 0357–0369 records scoped documentation corrections; reconciliation reviews 0370 R2 (V&V delta, approved with comments), 0371 R3 (requirement grounding, changes required), 0372 R4 (documentation, changes required at the reviewed revision) and 0373 PA (no N20 approval) processed on 2026-10-17 with bounded corrections applied | NC-EXT-88/89 are closed narrowly on the R1 0355/R2 0356 re-reviews; NC-EXT-90, authority/target gates and overall phase/release assurance remain open despite completed focused reviews | Preserve review scopes and authority/target constraints; no overall phase closure inferred |
| N10 | T3 `test_pus.c` remains NOT FULLY QUALIFIED; the changed files pass host clang-format 23.1.3 (version logged); pinned-18 conformance is not evidenced (NC-EXT-101/244). The N13 checker changed (AD-002): the new compound-expression rule and the header-check fix are covered by `tests/python/test_license_check.py` 17/17 PASS, but independent review/qualification of the tool change remains pending (NC-EXT-44, T21). |
| N11 | scripts/reviews/results committed | The current worktree retains the reviews through FRICK-REVIEW-0419: 0405 (R2 V&V), 0406 (PA), 0407 (R2 N12 re-review), 0408 (R4), 0409/0410/0411/0412 (fresh R1/R2/R3/R4 of the owner-directed delta), 0413 (fresh N20 PA, not approved), 0414 (R1 delta, rejected for closure of 0409), 0415 (R2 delta) and 0416/0417/0418 (final R1/R2/PA delta) and 0419 (PA re-review of the correction), plus the earlier R2 0390, the R4/PA 0391/0392 set, 0393-0404 and the retained evidence files. The findings of 0409-0416 are registered as NC-EXT-184..190 and all remain OPEN. NC-EXT-226/FRICK-REVIEW-0436 (2026-10-29): R2 SW V&V review of XW-3 delta against v022 snapshots. Verdict: APPROVED WITH COMMENTS. Two Comment-severity findings: (1) `atomic_publish` same-filesystem limitation (no parent-directory fsync, cleanup best-effort on replace failure). (2) `summary.case.json` not hash-bound to run-index, explicitly unresolved/related only to NC-EXT-230 (verdict-only hash binding). NC-EXT-226 stays OPEN/non-blocking; T35 stays NOT QUALIFIED; human qualification stays deferred per owner msg2046 (2026-10-26); NC-EXT-230 stays OPEN; atomic_publish same-filesystem limitation is a comment, not a closure. No target/hardware execution; no commit/push. | NC close-out is not performed here; no review report is treated as an approval and no gate is claimed passed | Keep the reports and the NC register in the same tree; closure requires the authorized dispositions |
| N12 | **NC-EXT-94 and NC-EXT-95 CLOSED (2026-11-14) by owner disposition AD-001** (`docs/quality/product-assurance.md` §6); NC-EXT-240..251 excluding 94/95 remain OPEN. Closure of 94/95 is not an approval of this change; NC-EXT-249/250 stay OPEN. |
| N13 | `make gsw-selfcheck` **PASS** (license 17/17, GSW 12, metrics 9) after the AD-002 correction: overlay `tools/patches/n7s-obcp/src/modifications/py/vm.c` declares `MIT AND MPL-2.0` (upstream MIT notice retained) and the checker accepts compound expressions only under `tools/patches/` with `MPL-2.0` present and identifiers restricted to `{MIT}`. NC-EXT-43 (notices/provenance/combined distribution) and NC-EXT-44 (T21 qualification) remain OPEN; no legal or compatibility conclusion is made. |
| N14 | SSS reviewed by R3 | R3 review FRICK-REVIEW-0290 and corrective delta 0295 recorded; S4c requirement/interface wording reviewed | Mission SSS/SRS baseline still lacks approved Bmax/Rmax/Dmin/Lmax, link profile, ground boundary and loss counter/report semantics | **OPEN** for mission baseline; bounded review does not close NC-EXT-75/76 or establish system authority approval |
| N15 | full cycle each change | On the current tree (2026-11-09 emulated watchdog increment) the native build and full **CTest 54/54** pass (`build2/native`), Lean `lake build` passes, the Lean elaboration receipt regenerates (11 modules), `make trace` passes, `make gsw-selfcheck` and `metrics.py --check` pass, the pinned clang-format 18.1.3 check passes, the RTEMS 6.2 cross-build passes (`build/renode/frick_fsw_renode.elf`), and **N18 Renode passes** (`RENODE_TEST: PASS`). `make smoke` was not rerun in this increment. | No target execution or HIL leg; the emulated N18 run is integration evidence, not RTPFS160ZT fidelity. `make smoke` not rerun. | Keep every leg's status distinct; no gate or NC closure |
| N16 | revision history | compliant | none | closed |
| N17 | documentation review (R4) | R4 0357–0359 corrected/re-reviewed TS1 range/history/status; 0362/0363 current evidence/status/audit/manifest; 0365/0366 final CI hash; 0367/0368 handover boundary; 0369 audit/index; 0372 reconciliation; 0387 host-gate docs; 0389 N18 pointers; 0391–0401 reviewed successive currentness corrections. PA 0402 found a historical attribution issue, corrected in handover rev 3.32 / sprint-plan rev 1.90; R4/PA 0403/0404 confirmed. NC-EXT-119 remains OPEN pending authorized owner disposition | No aggregate document-set/N17 or phase approval. NC-EXT-116/117/118/119 remain OPEN; earlier R4 findings remain open per their records. | Preserve bounded scope and no-closure boundary; broader S8-FDIR-01 R4 0386 remains REJECTED and its NCs stay open. |
| N18 | Renode emulated-target functional validation | **PASS on the current tree (2026-11-10)** for the `N22-TS5-66` increment (and 2026-11-09 for the emulated WDOG0 binding): `tools/scripts/renode_vv.sh` reports `RENODE_TEST: PASS` with container exit 0; the retained console shows `SCH app started, 4 entries`, `watchdog: WDOG0 armed (emulated profile: timeout 10000 ms, TIME=1562500, TRIGGER=4095, MSVP=TIME, clk=156250 Hz)` and `WD armed: timeout 10000 ms, feed 2500 ms`. The pinned `beaglev-fire.repl` imports `polarfire-soc.repl` with five MPFS watchdog model instances. | The model does not replace target/HIL evidence, does not establish RTPFS160ZT register/clock/reset-scope fidelity, and does not close the flight binding (NC-PAL-01/GAP-A5). | Emulated integration evidence only; real-HW validation still required |
| N19 | this audit each loop | This loop began by inspecting governing docs, worktree and actual implementation. Owner inputs are now recorded in escalations/ADR/SRS. Current implementation/test delta and unverified dependencies are recorded in this audit. | Existing open NCs and all N1-N22 gates remain; this update does not claim N19 closure beyond the bounded delta. | Preserve owner attribution and current evidence; no claim of safety/release acceptance |
| N21 | Functional bug corrected (progress-failure path) and the licence-labelling defect corrected under owner delegation (AD-002); other non-functional findings recorded as NCs, not silently closed. NC-EXT-94/95 closed only on the recorded owner disposition, not by the implementer. |
| N22 | requirement-based/integration tests use nominal and applicable out-of-bound cases with expected-result comparison | Full CTest 54/54 on the current tree; post-review targeted tests: watchdog 320, ES/HS 180, UART 72, time 113. S8-SCH scheduler tests: 104/104 checks pass (4 new N22 out-of-bound cases added: zero entries, activate-zero-count, poll-before-activation, max-out-zero; all with explicit expected results per norm N22). PUS-C boundary tests: 168/168 checks pass (3 new N22 boundary provider/subservice/period tests document implementation scope and rejection behavior per norm N22). Added: saturated-console shared-deadline/reset host oracle in which the notifier observes the expired deadline and the pre-reset reset counter; the production reporter wiring case with an exactly-one-frame assertion; the link deadline-writer semantics; UART 60,000-byte partial-progress/short-write and stalled timeout; external nanosleep EINTR/EIO; failed-feed expiry preservation and reset hook error; the 96/97/256-character panic-report suppression characterization. The mission-table default watchdog case now expects 10 s / 2.5 s with the quarter-period assertion. | TS-5-80 requirement interpretation and 83/84/85/86/87 remain gaps as labeled; host timing does not imply 250 ms target WCET. The over-length test characterizes a known mismatch, not a pass. Hardware reset remains TS-5-82 gap. The emulated WDOG0 arm/feed is integration evidence; the flight parameters remain provisional (NC-EXT-238). | Keep requirement semantics, open target evidence and characterization distinct; no NC/gate closure. S8-SCH N22 test additions verified on native host; no target/HIL claim. PUS-C N22 boundary tests document implementation scope per authority-owned scope limits. |
| N20 | every phase product PA-reviewed; findings fixed and reviews repeated until pass | The bounded N18 PA review 0388 identified NC-EXT-114 OPEN; PA 0392 identified NC-EXT-116/117; R4 0393 identified NC-EXT-118; the R4/PA 0394-0404 set identified NC-EXT-116/117/119. For the earlier FDIR increment the PA legs are 0406, 0413 (**not approved**: 1 Blocker, 7 Major, 1 Minor), the 0418 PA delta (**not approved**: 1 Blocker — the RTEMS `poll()` link break, now corrected — plus 5 Major and 5 Minor) and the 0419 PA re-review of the correction (**not approved**: the product corrections verified, with record-currency findings). The 0406/0413/0416 findings are registered as NC-EXT-184..190. **The 2026-11-09 emulated WDOG0 binding increment has not yet received its independent R2/R3/R4/PA reviews.** | The N20 gate for this increment is **not passed**; the emulated increment is pending independent review and the authority items are unresolved. | No phase/release approval; the registered NCs carry the open items; obtain the pending reviews before any commit |

## 3. PUS and lwIP default configuration reconciliation

> **2026-10-19 review-integration note (post-NC-EXT-99 remediation).** The batch-scoped
> reviews that NC-EXT-99 records as deferred by owner disposition were performed
> **after** publication at HEAD `7e93325`: `FRICK-REVIEW-0381` (R4, N17),
> `FRICK-REVIEW-0382` (R2 core + R3 annex, N22/N10) and `FRICK-REVIEW-0383`
> (PA, N20), all APPROVED WITH COMMENTS with **0 Major**. The bounded corrections they
> implied (ASan duration 166.36 s to 94.02 s per the retained log; the "uncommitted"
> label re-pointed to "published as 318f49b"; the TS-6 3.1 lwIP/ASan evidence bullet
> (F-0382-02); the TS-5 HS oracle caveat plus traceability.yaml `lean_note` caveats
> (F-0382-01); the manifest addendum digest-binding commit named as 318f49b (F-0383-04);
> the VP 9 S7/S8 per-delta rationale (F-0383-03); the reviews/README "Current status"
> block dated historical (F-0381-05)) were applied in this cycle and each is marked
> "delta re-review pending" (N12). Authority-owned findings were **not** corrected but
> disposition-registered: the ICD-vs-README divergence was appended to NC-EXT-94 and the
> SRS-REQ-018/050 contradiction remains tracked by NC-EXT-97. New process NC-EXT-100
> (evidence manifests lack revision/history per N16) and NC-EXT-101 (format/docs logs
> attribute PASS from absence of output) were registered. **NC-EXT-99 remains OPEN**:
> closure requires the independent profiles to confirm the corrections (delta re-review,
> N12). No NC, gate, phase, qualification, PUS-C compliance or release approval is
> established by this note; the rows above continue to record that those reviews were
> not obtained before the push.

ADR-0018 temporarily selected the open PUS backend while enabling lwIP by
default. The owner has now clarified that N7/ESA is the intended PUS default;
ADR-0019 supersedes ADR-0018 only for PUS selection. ADR-0018 continues to govern
lwIP. ADR-0016's generated-source/no-vendoring rationale remains applicable.

**Current candidate implementation state (uncommitted):** `CMakeLists.txt`
defaults `FF_USE_PUS_ASN1=ON`; `fsw_main.c` selects `FF_PusBackendN7`. The
generated common ASN1SCC files and pinned N7/ESA CRC wrapper are linked into the
default build. The adapter uses generated type-width assertions and CRC, but
delegates packet framing and PUS field coding to the reference codec. The N7
adapter host test now links the actual generated common files and CRC wrapper,
not the prior controlled stub. `FF_USE_PUS_ASN1=OFF` selects the open backend.
`FF_USE_LWIP=ON` and fetch-before-configure handling remain unchanged.

**Residuals:** fetched-source provenance/reproducibility remains tracked by
NC-EXT-35/40; external/generated licensing boundaries remain NC-EXT-43. Default
lwIP enablement does not establish target timing, network interoperability,
security, or mission suitability. Bounded independent reviews and the green
GitHub run are recorded; Gitea status, T23 qualification and final
document/release gates remain open. No release/qualification conclusion is
drawn.

## 4. Other gaps of the same class (declared-but-not-delivered)

Found by this audit (not previously tracked as such):

1. **PUS-C ASN.1 default** — ADR-0019 restores N7/ESA as the project default.
   Current candidate build and tests exercise generated common ASN1SCC sources
   and N7/ESA CRC; full service-specific codec integration remains M15.
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
| Default PUS backend / lwIP integration | ADR-0019 restores N7/ESA default; ADR-0018 remains for lwIP ON. Current fresh-cache host cycle exercises generated common types/CRC; full N7 service mapping remains M15 | frick-fsw | S8-PUS-FU |
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
| C-01 decoder PDL wrap/underflow and unsafe CRC bounds | **Confirmed** in TC and TM decoders. Widened PDL validation, minimum secondary-header/CRC checks, exact packet boundary, non-mutating rejection and symmetric boundary tests implemented. V&V 0316 accepts the bounded host codec evidence; NC-EXT-01 remains OPEN pending authorized NC-owner disposition. Profile policy and target qualification remain separate. |
| C-03 CCSDS primary-header validation | **NC-EXT-13 OPEN**: decoders do not validate primary version, secondary-header flag or sequence flags; controlled profile/acceptance policy is missing. Obtain authority-controlled profile before selecting rejection rules. |
| C-13 maximum TC packet exceeds ingest buffer | Production receive is factored into `FF_AppCmdIngestReceiveOne`, used by main; N22-TS1-96 drives 1037 bytes through a controlled link, verifies `FF_CCSDS_MAX_TC_PACKET` capacity and byte-identical PUS dispatch. V&V 0316 accepts this bounded host evidence; NC-EXT-02 remains OPEN pending authorized NC-owner disposition. Real backend/target limits remain. |
| T7 ground-tool wire-layout mismatch | **Confirmed** by comparison to the flight codec. TC/TM vectors, bounded incremental parser and deadline-limited exact-ping reader have 12 GSW tests; R2 0317 accepted the bounded host scope. Plausible incomplete packets can hold later bytes until timeout; exact tuple does not prove reply freshness. NC-EXT-03 remains open for external ground-segment E2E and approved correlation/profile disposition. |
| D-04 malformed-TC requirement/test-oracle conflict | **Confirmed**: SRS-REQ-005 demands S1 failure on malformed TC, while `test_pus_malformed_tc_event` expects only an EVS/S5 event and no S1. NC-EXT-04 remains OPEN for authorized systems/requirements disposition; do not alter the requirement or oracle without that decision. |
| C-08 PUS concurrency | **Credible, not closed**: shared global state lacks explicit serialization; NC-EXT-05 open for caller-context/concurrency analysis and disposition. |
| Constant CCSDS sequence count | **Confirmed** (`ff_write_primary` receives zero); NC-EXT-06 open for approved-profile decision and, if required, implementation/test. |
| UART framing | **Corrected in candidate:** bounded PDL accumulator now handles tested split/coalesced/max-size and over-limit recovery cases; host PTY evidence is recorded in TS2-81. A plausible incomplete PDL retains later bytes; timeout is VTIME-decisecond-granular (<100 ms quantization plus scheduler/driver latency); caller must serialize receive. R2/V&V accepts bounded host behavior only; NC-EXT-07 remains OPEN for approved timeout/recovery policy and RTEMS runtime/timing/hardware evidence. Prior poll/select target-build attempts failed to link; final VTIME implementation cross-builds successfully. |
| Overall GREEN semantics | NC-EXT-08 open pending R4 review of the new README disclaimer. |
| `run_sim.sh` generator conflict | NC-S4b-09 closed: `run_sim.sh` no longer forces `-G Ninja` against an existing Unix Makefiles cache; smoke verified PASS (cached generator: banner + 7 applications registered, exit 0); ctest 56/56 green. Fresh-directory smoke blocked by pre-existing OBCP source-generation dependency (unrelated to the Ninja fix, tracked separately). Independent reviews obtained (PA/N20, doc/N17-R4, V&V/R2, avionics/R1-R3); no finding blocks closure. T4/T9 independent delta qualification review pending separately. |
| ASan-found mock-register overflow | NC-EXT-09: `test_gem_tsn` mock is 4096 bytes; highest exercised queue-3 ENST access `0x0880..0x0883` is in range. Fresh focused ASan CTest passed 1/1 (22 checks); independent V&V 0318 accepts only exercised host offsets. No whole-suite sanitizer or target claim. |
| Metrics-panel yellow legend | NC-EXT-10: generator wording corrected and README regenerated; independent R2/R4/PA requalification pending. |
| Category B target vs assigned classification | NC-EXT-11: README/sprint/PA-plan wording now identifies Category B as project target only; authorized mission assignment/tailoring remains open. |
| GSW incomplete-header finite-buffer recovery | NC-EXT-12 remains open overall; FRICK-REVIEW-0173 verified the bounded PDL=0xFFFF candidate-followed-by-valid-packet test, but T7 qualification/PA closure remains pending. |
| CCSDS primary-header policy (C-03) | NC-EXT-13 OPEN; acceptance policy and criterion are authority-controlled and absent. Severity is provisional pending NC-EXT-31; do not characterize as a confirmed violation before disposition. |
| C-04 PUS S1 Request ID | NC-EXT-14 OPEN; correct against controlled ECSS/mission PUS profile; R1/R2/R3 verification planned S8-PUS-FU. |
| C-05 PUS S1 verification behavior | NC-EXT-15 OPEN; failure/acceptance semantics and unknown-service classification need code/test correction; SRS-REQ-005 oracle conflict remains authority-owned in NC-EXT-04. |
| C-06 PUS subtype allocation | NC-EXT-16 OPEN; reconcile S3/S9/S20 and TM[8,2] with controlled PUS definitions/profile, then add service-vector tests. |
| C-07 housekeeping behavior | NC-EXT-17 OPEN; correct behavior that is already specified; periodic-report/reserved-ID policy is not to be invented. |
| C-09 PUS S6 address/region | NC-EXT-18 OPEN; width and memory-ID contract require HAL/system authority input before RV64-safe implementation can be closed. |
| C-10 reference scheduler | NC-EXT-19 OPEN; bug confirmed and functionally corrected by the delivered `ff_sched` dispatcher (S8-SCH, committed f0e1083); pending independent R4/PA review. |
| C-11 N7 adapter CRC | N7 TC adapter now performs PDL/boundary validation and invokes the N7 CRC routine when `FF_FRAME_WITH_CRC=1`; host controlled-double test covers valid, corrupt, and malformed frames. NC-EXT-20 remains OPEN pending independent V&V and generated-SDK/profile confirmation; no vendor compatibility claim. |
| C-12 PUSopen APID | Adapter checks received TC type/APID before calling the SDK and returns the primary-header APID; controlled stub test verifies rejection before SDK call. NC-EXT-21 remains OPEN pending independent review; stub does not establish commercial SDK compatibility. |
| C-07 HK provider error | S3 report path returns provider failure and emits no report TM rather than false-success; TS1-101 verifies SRS-REQ-008. The bounded report-all first-failure correction is confirmed by R1 re-review 0355 and closed under NC-EXT-88; NC-EXT-17 remains OPEN for broader ID/enable/remove/profile reconciliation and authority disposition. |
| C-12 PUSopen adapter | NC-EXT-21 OPEN; received APID/type guard and controlled-stub test are present; independent review and commercial SDK compatibility remain open. |
| C-14 PUS S11 stub | NC-EXT-22 OPEN; M17 application scheduler does not implement PUS S11; disposition/implementation remains in M15 PUS service work. |
| C-15 duplicate PUS registrations | NC-EXT-23 CLOSED narrowly on R2 re-review 0356: duplicate service/provider/setter/function registrations return `FF_ALREADY_EXISTS` rather than silently shadowing, and retained service/HK-provider callbacks remain active; retained setter/function callback execution is not claimed (TS1-97). |
| C-16 PUS provider lengths | NC-EXT-24 CLOSED narrowly on R2 re-review 0356: HK/parameter provider-reported lengths above the supplied write capacity are rejected (`FF_BAD_SIZE`, no report TM) with exact zero/exact-capacity lengths asserted (TS1-98). |
| C-17 review-log ledger | NC-EXT-93 OPEN (non-functional): review IDs `0338`/`0339` are each carried by two report series (S8-FU NC-disposition committed at `1398a8e`; S8-PUS/OBCP added by the current batch). `docs/reviews/README.md` now states the collision, disambiguates by filename and defines the narrow-closure vs PA-acceptance taxonomy; the bounded rename/relabel cleanup is a later disposition. |
| T-01/T-05 decoder fuzz/oracle gap | NC-EXT-25 OPEN; deterministic bounded corpus + sanitizer harness delivered in S8-VV-FU (`test_decoder_corpus.c`); materially closed, remains OPEN pending independent V&V delta review. |
| T-08 application tests | Existing GAP-R3-1 remains OPEN; S8-VV-FU delivered the bounded ES/HS lifecycle host integration harness (`test_es_hs_lifecycle.c`) with an EVS-listener oracle; materially closed and independently reviewed (`FRICK-REVIEW-0379/0380`, APPROVED WITH COMMENTS), but **still OPEN** — the review's `F-0379-03` (reset-path case unexecuted) leaves a residual carried by NC-EXT-98; closure is an owner disposition. |
| D-01 TC/TM secondary-header documentation | NC-EXT-26 OPEN; author corrected byte diagram, TC/TM layouts, CRC-included PDL arithmetic and explicit absolute/relative offsets; sequence count is identified as implemented zero pending profile decision NC-EXT-06; independently verify against codec and wire vectors. |
| D-02 PUS backend API documentation | NC-EXT-27 OPEN; `pus-layer.md` vtable corrected to match `src/pus/pus.h`; independent API-documentation re-review pending. |
| D-03 review-evidence publication instructions | NC-EXT-32 OPEN; AGENTS, N11/N14 rules, CONTRIBUTING, review README, independent-review closure criteria and PA plan distinguish working-tree retention from gated commit/push; independent consistency review pending. |
| D-04 sprint precedence | Owner instructed resumption with the next sprint as planned; plan interprets that as S8-FU first in the proposed corrective order. R4 0311 closed NC-EXT-30/65 for this bounded interpretation only. |
| D-05 N14 review coverage | NC-EXT-33 OPEN; recent SSS-REQ-030 change lacks a recorded independent R3 review; do not represent N14 closed. |
| D-06 V&V history integrity | NC-EXT-34 OPEN; duplicate revision and split table consolidated; source corrected to FRICK-REVIEW-0185; R4 0188 and PA 0189 were scoped. Loop-audit now distinguishes author correction from pending independent disposition; no NC closure claimed. |
| D-07 audit revision integrity | NC-EXT-37 OPEN; FRICK-REVIEW-0191 found header/history mismatch and stale review pointer; author corrected; fresh review pending. |
| R4/PA status and evidence findings | NC-EXT-28/29/31 remain OPEN; NC-EXT-30/65 closed only for bounded sprint-order wording by R4 0311. NC-EXT-28/29/31 retain their applicable independent review, immutable evidence identity, and severity-rationale dependencies; no broad closure is inferred. |

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

## 10. NC-EXT-191 backend-coverage increment (2026-11-11)

Scope: the IP and TSN `FF_Link_t::send_until` writers, the documented contract in
`link.h`, and the new discriminating cases in `test_link_ip.c` /
`test_link_tsn.c`. Audited against the norms before the commit:

- **N1 (Lean).** No generated code and no new pure algorithm: the change is a
  deadline wrapper around a synchronous submit call, and the Lean models
  (`req_srs074_report_precedes_reset`) still cover the report-before-reset
  ordering. `lake build` PASS. No new theorem is required or claimed.
- **N3/N4 (backwards requirements, bidirectional traceability).** SRS-REQ-060
  and SRS-REQ-061 `lean_note` and SRS-REQ-074 `lean_note` + `tests` list now
  record the implemented writer and its tests; `trace.py` PASS
  (120/76/30/14/68).
- **N10/N11 (tool review, evidence retention).** The changed test scripts have no
  N10 review at these hashes, so `tool-qualification.md` rev 1.7 records a T3
  delta and leaves T3 **NOT FULLY QUALIFIED**; execution logs are retained in
  `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/`.
- **N15 (full cycle per change).** Build, CTest 56/56, Lean, trace, pinned
  clang-format 18.1.3, GSW/license, smoke, metrics, RTEMS 6.2 cross build and
  N18 Renode PASS all re-run on the changed tree; no gate was skipped or waived.
- **N18 (emulated PolarFire SoC).** `make renode` PASS on the ELF rebuilt for
  this change (sha256 `55188f5a…`). This is emulated-target functional evidence
  only, not HIL/timing.
- **N20 (product assurance).** PA review is requested in the same cycle; findings
  are recorded and non-functional ones tracked as NCs (N21), functional ones
  corrected.
- **N21 (sequencing).** The change is a functional defect correction (the panic
  reporter was inert on two of four backends) committed for the current sprint,
  so it is in scope; unrelated non-functional findings are not expanded into it.
- **N22 (test design).** Both new cases are nominal + out-of-bound with explicit
  expected results (in-budget success, expired-on-entry rejection without
  transmission, post-transfer overrun reported late, argument rejection), and
  the mutation control shows each assertion discriminates.
- **N19 disposition.** Remaining gaps are unchanged and tracked: NC-EXT-191
  (UART shared-descriptor concurrency, target readiness semantics), GAP-A5 /
  `N22-TS2-78` (target 250 ms), T3 qualification, NC-PAL-01, NC-EXT-239.

## 12. Start-of-loop N19 audit — UART deadline-writer restore-error correction (2026-11-13)

### 12.1 Objective and starting implementation

The ordered sprint plan carries S8-PUS-FU and the S8-FDIR-01/SRS-REQ-074
deadline-aware panic/report writer. In `src/transport/link/link_uart.c::uart_send_until`,
the current implementation sets `O_NONBLOCK` for one `write()` and immediately
attempts to restore the original descriptor flags, but checks the restore result
only when the write returns positive progress. On `EINTR`, `EAGAIN`/`EWOULDBLOCK`,
zero, or other write failure it retries/exits without checking restore failure.
The prior independent PA report
`PA-REVIEW-UART-SEND-UNTIL-RESTORE-RESULT-20261112.md` identifies the retryable
`EAGAIN` case: a failed restore can leave the shared descriptor nonblocking and
unreported. The bounded functional correction is to check restoration after
every write and stop with `FF_ERROR` if it fails, with a deterministic host test
of an `EAGAIN` write plus failed restoration. This does not require authority
input and does not close NC-EXT-191's remaining target/readiness concerns, or
touch OBCP selection, NC-EXT-19/90, N22-TS5-47, GAP-A5, or NC-PAL-01.

Baseline HEAD is `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`. The starting tree
contains the uncommitted S8-PUS-FU S1 test/document/review candidate and unrelated
pre-existing untracked paths; none may be cleaned/reset/stashed. The r4 S1
manifest had digest `c09c96f867744ade53e2802f6c5979157443e65bd9affe1131fb4602519d9e96`
and PA/R2/R4 attested 175 entries, 175/175. This audit edit is covered by that
manifest and therefore makes those attestations historical for the prior
snapshot. A final combined candidate must be frozen/rebound and re-attested.

### 12.2 N1-N22 audit at loop start

| Norm | Starting state / disposition |
|---|---|
| N1 | C-only change planned; no generated code. Re-run Lean build/receipt; no new theorem indicated for syscall-result propagation. |
| N2 | All repository prose/comments remain English. |
| N3 | SRS-REQ-074 already allocates the UART deadline writer and its test. No requirement change planned; preserve gap/characterization distinctions. |
| N4 | Existing traceability is 120/76/30/14/68; re-run trace after candidate docs/identity changes. |
| N5 | Category B is a project target only; mission classification and class-A gaps remain open. |
| N6 | Containerized tools available. Full pinned clang-format 18 has NC-EXT-244 failures on three unchanged files; use pinned changed-file evidence and do not modify unrelated files. Hosted CI is not yet evidenced for the combined candidate. |
| N7 | No commit/push at start. This is the designated FSW commit/push agent; publish only the exact scoped, gated, reviewed candidate and retain unrelated untracked paths. Do not claim sprint completion. |
| N8 | Handover, sprint plan, results, review index and evidence package require current candidate status and histories. Final manifest must be regenerated after covered edits. |
| N9 | Prior PA report found this specific functional residual but is not approval of the correction. Obtain fresh configured independent reviews on the exact corrected candidate. |
| N10 | Test/tool execution is not qualification; preserve T3 NOT FULLY QUALIFIED and review any new test seam. |
| N11 | Retain exact commands/logs, source/identity, tests and review reports; bind them in the refreshed package manifest. |
| N12 | Do not close NC-EXT-191 or other NCs without authorized disposition; record findings narrowly. |
| N13 | No new dependency planned; run applicable license checks and preserve third-party boundaries. |
| N14 | No SSS change planned. R3 review applies if requirement allocation/interpretation changes. |
| N15 | Re-run applicable build, all tests, Lean, trace, pinned format, license, smoke, and N18 after changes and corrections. |
| N16 | Update CHANGELOG and affected document histories in this candidate. |
| N17 | Obtain R4 review of changed product/status documentation; checksum attestation is not substantive N17 review. |
| N18 | Rebuild/run Renode for functional validation with emulator limits explicit; no physical-target or WCET inference. RTEMS compile coverage is also required for this shared source. |
| N19 | This start-of-loop audit records the sprint objective, actual defect and all N1-N22 dispositions before the behavior change; update it with outcomes before publication. |
| N20 | Obtain fresh PA/N20 review of exact candidate; manifest integrity alone is not product assurance. |
| N21 | The incomplete restore-error handling is a functional defect already identified by PA and in sprint scope. Fix it without expanding to unrelated NCs. |
| N22 | Assert explicit expected `FF_ERROR` for an injected retryable write plus restore failure; also prove the injected path executed and prevented silent retry. |

**Start-of-loop disposition:** proceed only with the UART restore-failure
reporting subcase. Keep NC-EXT-191 open for its other residuals. Do not solicit
or infer any authority-owned decision listed above. The sprint/session remain
in progress.

### 12.3 Current implementation and gate outcome (2026-11-13)

The bounded change is implemented in `uart_send_until()`: save `errno` directly
after `write()`, check the restore `fcntl` result before branching on the write
outcome, return `FF_ERROR`/break on restore failure, and post-check the
monotonic deadline after positive writes (including the final write). `test_link_uart.c`
uses linker wrappers to inject EAGAIN and restore EIO, asserts exactly one write
attempt and no transmitted bytes, observes the failed descriptor state, and
cleans up with real `fcntl`; a complementary case proves saved errno survives a
successful restore that clobbers errno. Focused test = 104 checks, 0 failures;
CTest `link_uart` 1/1; full native CTest 57/57. N22-TS2-85 verifies a positive final UART write
that crosses its deadline (timeout with emitted bytes); N22-TS2-86 verifies an
expired zero-length request returns `FF_TIMEOUT` without a write. Evidence is recorded at
`docs/vv/evidence/2026-11-13-uart-restore-error/` with source/test/binary hashes.
Build, Lean, trace, changed-file pinned format, GSW/license, smoke, docs,
RTEMS cross-build and Renode PASS. Full pinned format continues to fail on
nine files / 39 violations, including `src/pus/pus_services.c:753` (NC-EXT-244);
T3 remains NOT FULLY QUALIFIED.
R1/R3 source/API re-reviews found no functional defect in the bounded cases. The
R2/R4 reports on earlier manifest snapshots and the 48f digest are historical
for the current candidate after this §12.3 correction; do not treat their
checksum passes as current package attestation. PA/N20 substantive review found
no new bounded product defect, but that does not establish package integrity.
The f07c2ca4… snapshot reports recorded matching 297/297 checks, but subsequent
covered status-document edits invalidate that binding. Do not infer the current
package result from those reports; compare the active digest with each current
post-manifest report and the complete inventory/check. Preserve the N19 start-state
above as the before-change observation.

**Residuals:** a failed `fcntl` may leave the shared descriptor nonblocking, so
the correction reports failure but does not guarantee safe behavior for other
writers. Target readiness and 250 ms timing remain open under NC-EXT-191 / GAP-A5.
Do not close NC-EXT-191 or claim target/HIL qualification.

## 13. N19 assessment/outcome — current S1 progress-failure increment (2026-11-14)

### 13.1 Objective and starting implementation

Objective: implement S1 progress-failure emission (TM[1,6]) in `FF_PusIngest`
per ICD §4 rev 1.9 / NC-EXT-94, with focused regression tests. Starting
implementation: `fc68b9d` is committed HEAD (ICD documentation only for
NC-EXT-94/95); S1 source/test edits are uncommitted working-tree state.

### 13.2 N19 timing deviation (recorded, not passed)

N19 requires the audit at the **start** of every development/verification loop,
before behavior change. This audit's header and N1-N22 rows were refreshed
at 2026-11-14 **after** the first source edit (`src/pus/pus.c` progress-failure
move) had already been applied. The pre-edit baseline state was not
audited first. Tracked as NC-EXT-245 (open to system, non-blocking under N21).

### 13.3 N1-N22 outcomes (current cycle)

| Norm | Outcome |
|------|---------|
| N1 | **GAP, not N/A.** `make verify-lean` PASS, but `verification/lean/Frick/Pus.lean` `AckFlags`/`verificationCount` omit the progress stage, so the 'exactly one of TM[1,5]/TM[1,6]' invariant is tested, not proved. Tracked as **NC-EXT-248**. |
| N2 | All docs/comments in English |
| N3/N4 | `tests/unit/test_pus_s11_residual.c` added to the SRS-REQ-006 `tests` list in `docs/requirements/traceability.yaml` (the new N22-TS1-105 oracle had no trace link); `make trace` PASS (120/76/30/14/68) |
| N5 | Class B deliverable; class-A gaps documented |
| N6 | CI/CD via Docker not re-run for uncommitted candidate |
| N7 | **No commit/push performed**; all gates required before commit |
| N8 | TS-1 rev 1.14 (N22-TS1-105 added), verification-results.md, sprint-plan, handover, loop-audit updated |
| N9 | Reviews obtained from sub-agents R1 FSW (NEEDS REVISION), R2 V&V (CORRECTIONS REQUIRED), R3 systems (CONDITIONAL), R4 documentation (NOT PASSED), PA/N20 (REJECTED). **Contextual independence only, not organizational**; corrected tree requires re-review. |
| N10 | T3 `test_pus.c` remains NOT FULLY QUALIFIED; the changed files pass host clang-format 23.1.3 (version logged); pinned-18 conformance is not evidenced (NC-EXT-101/244) |
| N11 | Raw logs and exit codes retained under `docs/vv/evidence/2026-11-14-s1-progress-failure/` |
| N12 | NC-EXT-240..251 remain OPEN; no NC closure claimed |
| N13 | Authored-file license checker not re-run on changed files; the `make gsw-selfcheck` failure (`tools/patches/n7s-obcp/src/modifications/py/vm.c` missing MPL-2.0 SPDX label) is a pre-existing, already-tracked finding under **NC-EXT-43** (external/generated provenance-and-notice scope; dated observation appended) and **NC-EXT-44** (T21 checker semantics) — not a new NC. NC-EXT-246 is the N18 Docker runner preflight/cleanup gap, not a licensing NC. No source/license edit. |
| N14 | No SSS change |
| N15 | Fresh host configure/build on `build/s1-review`: configure exit 0, build exit 0 (0 warnings), CTest 60/60 PASS, focused set 7/7 PASS (`test_pus_s11_residual` checks=1206/0), verify-lean PASS, trace PASS (120/76/30/14/68), host clang-format 23.1.3 PASS on the two changed files. **smoke FAILED on default `build/native` cache** (exit 2); `BUILD_DIR=build/smoke-safe` PASSED. `make gsw-selfcheck` PASS after the AD-002 correction (license 17/17, GSW 12, metrics 9). Full format-check FAILED on nine unrelated files (NC-EXT-244). **N18 not evidenced for this delta (NC-EXT-247).** |
| N16 | CHANGELOG and document histories updated |
| N17 | R4 review obtained (sub-agent, contextual independence only): **NOT PASSED**, 6 Major + 12 Minor + 8 observations; corrections applied, re-review required |
| N18 | **NOT satisfied for this delta (NC-EXT-247).** The retained Renode run (11:26) predates `src/pus/pus.c` (12:21:53) and carries no ELF/image identity for this candidate; no RTEMS rebuild exists. The Docker runner was also invoked without the required preflight (cleanup/mount safety unaudited) — **NC-EXT-246**. No N18 PASS is claimed. |
| N19 | **Sequencing deviation recorded as NC-EXT-245**; not claimed passed |
| N20 | PA review obtained (sub-agent, contextual independence only): **REJECTED — commit/push withheld**, 18 findings; corrections applied, re-review required. N20 is not discharged by a sub-agent. |
| N21 | Functional bug corrected (progress-failure path); nonfunctional findings recorded as NCs, not silently closed |
| N22 | N22-TS1-105 added to TS-1 rev 1.14 (105 cases = 77 covered + 28 gap; 6 mixed; 94 individual; SIL 102); nominal and out-of-bound cases with explicit expected results |

## 14. N19 start-of-loop audit — NC-EXT-251 dispatch-lock correction (2026-11-14)

### 14.1 Objective and actual implementation

Objective: correct functional NC-EXT-251 using owner-approved option 2 and
fail-fast lock semantics (user decisions m00324, m00476, m00588): never hold the
PUS state mutex across handlers, callbacks, or TM emission; serialize complete
TC dispatch separately; return `FF_BUSY` on lock collision before state/counter
mutation; permit handler-originated TM sends; reject sink-recursive sends; and
prevent overlapping TM sink callbacks. Preserve the S1 exact-one progress
contract and ordering. The panic/reset `FF_PusSendTmDirect` path must remain
lock-free and use the shared atomic modulo-16384 TM counter. V&V supplied a
read-only 11-case design; it has not yet been integrated or executed. No
NC-EXT-251 behavior code has changed yet. The S1/license candidate remains
uncommitted on `fc68b9d121bce2194c1beaaa9a7dc4cdab0552c`.

### 14.2 N19 timing deviation (recorded, not passed)

This resumed loop began with read-only source and design inspection before the
current N19 record was refreshed. No NC-EXT-251 behavior code was changed
before this audit update, but the strict N19 start-of-loop sequence was still
missed. This does not retroactively establish N19 compliance. Record this
additional occurrence under existing **NC-EXT-245**, which remains OPEN,
owner FSW engineer, disposition open to system/non-blocking under N21. The
current audit is now updated before any NC-EXT-251 behavior edit; no process
NC is closed or waived.

### 14.3 N1-N22 status for this objective

| Norm | Current status for this loop |
|------|------------------------------|
| N1 | **GAP:** NC-EXT-248; Lean omits the progress stage. No new formal artifact yet. |
| N2 | English-only project content maintained. |
| N3/N4 | Prior S1 trace gate passed; NC-EXT-249/250 authority issues remain. NC-EXT-251 behavior/test trace links will be added with its implementation. |
| N5 | Class B target; class-A and mission/target evidence gaps remain open. |
| N6 | No CI/Docker run for this loop; NC-EXT-246 preflight blocks further Docker/Renode execution. |
| N7 | No commit/push. |
| N8 | Existing S1 candidate docs are updated; PUS API, architecture, ICD, TS-1, sprint, evidence, and handover updates for NC-EXT-251 remain pending. |
| N9 | Corrected-tree R1/R2/R3/R4 re-reviews are unavailable; prior reports did not pass. No current approval claimed. |
| N10 | T3 and T21 qualification remain open (NC-EXT-44 and existing tool NCs). |
| N11 | Prior S1 evidence retained; V&V read-only report exists outside the repository and must be incorporated into controlled TS-1/evidence; no NC-EXT-251 execution evidence exists yet. |
| N12 | NC-EXT-43/44 and NC-EXT-244..251 remain open; other active findings remain open. No NC closure claimed. |
| N13 | Prior candidate `make gsw-selfcheck` PASS is retained; NC-EXT-43/44 remain open. |
| N14 | Mission SSS authority inputs remain open; this change does not dispose them. |
| N15 | NC-EXT-251 implementation/tests are pending. Owner requires the full project test suite every sprint; run it with all applicable gates after the change. N18 remains not evidenced (NC-EXT-247). |
| N16 | Prior S1 histories are updated; NC-EXT-251 change histories/evidence entries are pending implementation. |
| N17 | Corrected-tree R4 review is pending; previous R4 verdict was NOT PASSED. |
| N18 | **NOT satisfied for the current candidate** (NC-EXT-247); NC-EXT-246 blocks Docker/Renode until the approved preflight exists. |
| N19 | Current audit is recorded, but both the earlier and this loop's late audit timing are deviations; NC-EXT-245 remains OPEN and N19 is not claimed passed. |
| N20 | Corrected-tree PA review is pending; previous PA verdict was REJECTED. |
| N21 | NC-EXT-251 is a functional defect and must be corrected; owner-approved option-2 and panic-path atomic-counter design are recorded above, but code is not yet changed. Preserve S1 behavior. |
| N22 | Existing S1 tests cover progress outcomes; V&V supplied an 11-case nominal/OOB/concurrency/re-entry matrix. Add controlled oracles, trace links, and executable tests; do not claim coverage until run. |

### 14.4 Owner dispositions and remaining implementation constraints

Owner selected fail-fast behavior: state, dispatch, and emission locks are
try-only; PUS maps lock contention to `FF_BUSY`, and all other PAL errors remain
distinct. Any lock collision must precede mutation; accepted TM sink calls are
mutually exclusive. A handler may send TM; nested ingest and sink-recursive TM
send return `FF_BUSY`. The PUS state lock is released before every external
callback. Complete TC dispatch remains serialized. Update public PUS APIs that
currently cannot report lock failure, and define the exact status/atomicity
clauses in controlled interfaces before freezing the TS-1 oracle.

Owner also confirmed that `FF_PusSendTmDirect` must remain bounded and
lock-free, sharing a lock-free atomic modulo-16384 counter with ordinary TM
emission. Verify compiler lowering on host and RISC-V target; a helper-backed
atomic is not acceptable. Keep backend/writer snapshots safe for this path and
preserve the panic deadline contract.

V&V's read-only assessment found the design testable and supplied 11 cases,
but the report is not yet controlled evidence. The synchronous `FF_Pus_TmSink_t`
callback is `void`; PUS cannot observe sink failure or bound a callback that
never returns. The lock correction prevents PUS mutex deadlocks, not arbitrary
callback nontermination. Document this boundary, test return/serialization and
failure paths with deterministic barriers, and retain the open callback-timeout
risk unless the transport provides a verifiable deadline.

## 15. N19 start-of-loop audit — end-to-end I/O coverage gaps (2026-11-15)

### 15.1 Objective and baseline

User request m00806: close the end-to-end coverage gaps identified by the
read-only V&V assessment, execute the tests, then resume the normal development
flow. Current committed HEAD remains
`fc68b9d121bce2194c1beaaa9a7dc4cdab0552c`; the working tree is already dirty
with the uncommitted S1 progress-failure/license delta, the prior audit updates,
review reports, and evidence. This loop adds no changes to those baseline
claims. The earlier S1 manifest check failed only for the subsequently edited
`docs/status/loop-audit.md`; regenerate the manifest after this loop's audit and
implementation updates. Subsequent OOB E2E implementation and execution are recorded in §§15.3–15.4; this audit remains a late N19 entry and does not retroactively pass N19.

V&V identified existing SIL integration cases N22-TS2-70..72 for mock/loopback
IP UDP, TSN, and TCP segmentation, but no automated GSW↔backend round trip
(N22-TS2-75), no Renode SDLS or `FF_Link_t` round trip (76/77), and no target
HIL UART/IP/TSN, UART 115200, or real HSM/ground key evidence (78..80). The
SDLS+PUS+link path (73) is not wired and has no controlled oracle. The
comprehensive test specification also leaves target CAN/I2C command paths
open. These are distinct scopes: this loop may add and execute SIL tests for
existing software paths; it may not claim HIL evidence without target hardware,
may not run Docker/Renode while NC-EXT-246 preflight is unapproved, and may not
invent SDLS/key-management or mission interface behavior where the oracle is
open. Track blocked scope under existing NC-EXT-47/GAP-A5/GAP-A14 and
NC-EXT-246/247 with owner/milestone; do not silently mark it covered.

N19 sequencing is late for this fresh objective: source-of-truth docs and
baseline were inspected before this section was recorded. Keep NC-EXT-245 OPEN;
this audit does not retroactively pass N19.

### 15.2 N1-N22 conformance snapshot

| Norm | Current status for this E2E-coverage loop |
| --- | --- |
| N1 | No project source artifact was generated by this test increment; no Lean model/requirement change. `make verify-lean` passes. |
| N2 | English-only repository content maintained. |
| N3/N4 | N22-TS2-75/87 map to FRICK-ICD-001 §§2/4 and SRS-REQ-006/007/012; `traceability.yaml`/`.md` now attribute the integration tests to those SRS items; `make trace` passes 120 requirements (76 SRS, 30 SSS, 14 gaps, 68 Lean theorems). No requirement text changed. |
| N5 | Class-B target remains intended; target/HIL and mission-data gaps remain open. |
| N6 | No CI/Docker run; NC-EXT-246 preflight blocks Docker/Renode. |
| N7 | No commit/push; prohibited until all applicable gates and full sprint scope pass. |
| N8 | TS-2 rev0.15, GSW docs, T3/T7 qualification rev2.3, results, traceability, handovers, changelog and histories updated with test changes. |
| N9 | Corrected-tree R1/R2/R3/R4 and PA reviews remain pending; no current approval. |
| N10 | T3/T7 inventories and exact hashes updated in `docs/vv/tool-qualification.md` rev2.3; Python 14/14 and nominal+OOB E2E CTest 2/2 are execution evidence only. T3/T7 remain NOT FULLY QUALIFIED pending independent exact-hash review. |
| N11 | Test source, commands, focused/full host logs, initial-failure reports, post-mapping trace and `logs.sha256` are retained in `docs/vv/evidence/2026-11-15-gsw-uart-e2e/`; checksum verification passes. |
| N12 | NC-EXT-43/44 and NC-EXT-244..251 remain open; NC-EXT-47, GAP-A5/A14 and target evidence gaps remain open. No NC closure claimed. |
| N13 | `make gsw-selfcheck` passes after E2E/tooling edits (GSW 14/14, metrics 9/9, license 17/17, license-policy check OK). |
| N14 | No mission-authority disposition for SDLS, HSM/ground keys or target interface tailoring. |
| N15 | `BUILD_DIR=build/s1-review make build` and `make test` pass (62/62, 19.71 s); Lean, trace, GSW self-check and native smoke pass. N18 is separately blocked. |
| N16 | TS-2, comprehensive test specification, T3/T7 qualification register, verification results, handover, changelog and document histories updated in this cycle. |
| N17 | Corrected-tree documentation review remains pending and is a commit gate. |
| N18 | Not evidenced for this candidate; NC-EXT-247 open; do not run Docker/Renode until NC-EXT-246 preflight approved/applied. |
| N19 | This objective's audit was recorded after source-doc inspection; late sequence remains NC-EXT-245 OPEN, not passed. |
| N20 | Corrected-tree PA review remains pending and is a commit gate. |
| N21 | User authorized closing the testable E2E gaps; implement coverage for existing supported software paths, while tracking untestable hardware/authority gaps rather than fabricating evidence. |
| N22 | N22-TS2-75 nominal GSW CLI→production UART/PUS exchange and N22-TS2-87 OOB missing-ACK fail-closed case pass with explicit results; PTY/shim is not physical UART/HIL. Cases 73, 76–80 and other target-only paths remain gaps. |

### 15.3 Scope and gates

Implemented and executed nominal `N22-TS2-75` and OOB `N22-TS2-87` host SIL cases: the actual `tools/gsw/ff_gsw.py` CLI sends S17.1 with requested S1 accept/start/completion acknowledgments through a test-only serial adapter and bridged POSIX PTYs to a C server using the production UART backend and PUS. The nominal test checks exact TC bytes, ordered TM services `[1,1],[1,3],[17,2],[1,7]`, S17 echo, ACK payloads, CRC and PUS secondary-header counter wrap. The OOB bridge drops only the acceptance TM[1,1] on the GSW leg; server captures all four emitted TMs, while CLI receives three, exits 1 within `--timeout 0.25`, and prints the exact missing/duplicate/unexpected/failed-response error. Both are host SIL only, not pyserial, external ground, physical UART or HIL. N22-TS2-75/87 are host-covered; T3/T7 remain open pending independent exact-hash reviews.

Renode cases 76/77 remain blocked by NC-EXT-246 preflight; HIL 78..80 and target
CAN/I2C paths require target hardware. SDLS case 73 requires an implemented,
controlled secure-link path and mission/security oracle; no security feature or
key policy was inferred. The first default `make build` and `make smoke` attempts
failed before product execution because ignored `build/native/CMakeCache.txt`
was generated under `/workspace`; this was recorded, not deleted. Re-running
with `BUILD_DIR=build/s1-review` passed. Failure and success evidence is retained
under `docs/vv/evidence/2026-11-15-gsw-uart-e2e/`.

A separate unresolved owner question was sent through `notify_nicola`: the
option-2 PUS emission try-lock can reject an S1 ACK while SRS-REQ-006 requires
exactly one packet per acknowledged stage. This E2E work does not alter that
NC-EXT-251 contract; do not change ACK behavior without owner disposition.

### 15.4 Executed host-SIL evidence and remaining gates

- `BUILD_DIR=build/s1-review make build`: PASS.
- `BUILD_DIR=build/s1-review make test`: 62/62 PASS, 19.71 s, including nominal and OOB GSW UART E2E (2/2).
- `make verify-lean`: PASS; `make trace`: PASS (120 requirements, 76 SRS,
  30 SSS, 14 gaps, 68 Lean theorems).
- `make gsw-selfcheck`: PASS (GSW 14/14, metrics 9/9, license 17/17, policy
  check OK); `BUILD_DIR=build/s1-review make smoke`: PASS (banner + 7 apps).
- N18 is still not evidenced; NC-EXT-247 remains open. NC-EXT-246 prohibits
  Docker/Renode until safe preflight. N9/N17/N20 corrected-tree reviews remain
  pending. NC-EXT-245 sequencing deviation remains OPEN, not retroactively passed.
- No commit/push, release, qualification, flight, safety or security approval.

## History


| Rev | Date       | Author    | Reason |
| 11.14 | 2026-11-15 | frick-fsw | Extend GSW UART/PUS coverage with N22-TS2-87 OOB dropped-ACK fail-closed oracle and `--ack none` support; update traceability/T3/T7/TS-2, record 62/62 host suite plus Lean/trace/self-check/smoke evidence; preserve host-SIL-only limits, stale `/workspace` cache failure, N18/review gates, NC-EXT-245/246/247 and unresolved NC-251 ACK contract. No commit/push. |
| 11.13 | 2026-11-15 | frick-fsw | Record implementation and 61/61 host suite, GSW 14/14, TS-2 75 host-SIL E2E, Lean/trace/self-check/smoke evidence and exact N11 artifacts; preserve stale `/workspace` cache failure, no HIL/N18 claim, N9/N17/N20 reviews pending, NC-EXT-245/246/247 and ACK contract unresolved. No commit/push. |
| 11.12 | 2026-11-15 | frick-fsw | Begin user-authorized E2E coverage-gap loop; record existing TS-2 70..72 host integration evidence, GSW/renode/HIL/SDLS/CAN-I2C gaps, NC-EXT-246 execution barrier, N19 late-sequence NC-EXT-245, and bounded host-SIL scope before code/test changes. No E2E implementation or tests yet; no commit/push. |
| 11.11 | 2026-11-14 | frick-fsw | Record owner-approved fail-fast option-2 contract, V&V's 11-case read-only design, and the selected lock-free panic path with shared atomic TM counter before implementation. Keep the late N19 sequence gap open as NC-EXT-245; identify API status/trace updates, target atomic-lowering evidence, callback nontermination boundary, full-sprint test suite, N18 and corrected-tree review gates. No behavior code or execution evidence yet; no commit/push. |
| Rev | Date       | Author    | Reason |
| 11.11 | 2026-11-14 | frick-fsw | Record owner-approved fail-fast option-2 contract, V&V's 11-case read-only design, and the selected lock-free panic path with shared atomic TM counter before implementation. Keep the late N19 sequence deviation open as NC-EXT-245; identify API status/trace updates, target atomic-lowering evidence, callback nontermination boundary, full-sprint test suite, N18 and corrected-tree review gates. No behavior code or execution evidence yet; no commit/push. |
| 11.10 | 2026-11-14 | frick-fsw | Start the NC-EXT-251 follow-up after owner selected option 2 (release the PUS state lock during callbacks/emissions and add separate dispatch serialization). Refresh N19 before behavior edits, explicitly record this loop's late audit under still-open NC-EXT-245 (not retroactively passed), and audit N1-N22. Identify unresolved callback re-entrancy/concurrent emission semantics; no NC-EXT-251 behavior code or verification evidence yet; no commit/push. |
| 11.09 | 2026-11-14 | frick-fsw | **Owner dispositions recorded and applied (AD-001/AD-002).** NC-EXT-94/95 **CLOSED** on the owner's explicit confirmation that ICD rev 1.9 / `fc68b9d` is the authoritative disposition, with closure conditions recorded (implementation must be committed with ICD rev 1.9; NC-EXT-249/250 remain separate and OPEN; no compliance/approval/qualification implied). AD-002: the third-party-derived overlay `tools/patches/n7s-obcp/src/modifications/py/vm.c` now declares `MIT AND MPL-2.0`; `tools/scripts/license_check.py` gained a narrow compound-expression rule (overlay roots only, exact expression `MIT AND MPL-2.0`) and an identifier-level header check that fixes a latent defect (exact list membership of the `MPL-2.0` string); `tests/python/test_license_check.py` 14 -> 17 tests, 17/17 PASS; `make gsw-selfcheck` PASS (was exit 2). NC-EXT-43/44 remain OPEN; T21 NOT FULLY QUALIFIED. Norm rows N10/N12/N13/N21 updated; N18 remains NOT satisfied for the delta (NC-EXT-247). No gate claimed passed beyond recorded evidence; no commit/push. |
| 11.08 | 2026-11-14 | frick-fsw | **Independent review integration and objective-artifact corrections.** Five sub-agent reviews obtained (R1 FSW NEEDS REVISION, R2 V&V CORRECTIONS REQUIRED, R3 systems CONDITIONAL, R4 documentation NOT PASSED, PA/N20 REJECTED — contextual independence only). Corrected the verified defects: stale `checks=1176` replaced by the fresh `checks=1206 failures=0` on `build/s1-review` (new `build-fresh.log`, `ctest-fresh.log`, `focused-pus-fresh.log`, `pus-s11-residual-fresh.log`, `format-changed-host.log`, `assets.sha256`); false SRS-REQ-006 trace claim removed and the real link added to `docs/requirements/traceability.yaml`; TS-1 §3.1 corrected to 105 = 77 covered + 28 gap (6 mixed, 94 individual, SIL 102) and §2 group list advanced to 105; NC-EXT-95 historical text restored verbatim (append-only) and NC-EXT-94 append marked as describing the pre-change state; baseline hash corrected to `fc68b9d121…`; full-format limitation restated as nine files / 39 violations; ICD header advanced to rev 1.9 / 2026-11-14 and assessment header to rev 0.4 / 2026-11-14; evidence directory marked untracked. New open NCs registered: NC-EXT-247 (N18 not evidenced for the delta), NC-EXT-248 (Lean model omits the progress stage), NC-EXT-249 (S1 handler-failure ordering and 1,6+1,8 co-occurrence need interface-authority disposition), NC-EXT-250 (SRS-REQ-006 no-handler path emits no start/progress packet), NC-EXT-251 (pre-existing PUS mutex defect). No NC closed; no gate claimed passed; no release/qualification/flight approval. |
| 11.07 | 2026-11-14 | frick-fsw | **Tracking correction (docs only).** NC allocation fixed: NC-EXT-246 = N18 Docker runner preflight/cleanup deviation (functional `RENODE_TEST: PASS` retained; cleanup/writable-mount safety unaudited; `mkdir -p` evidence-dir creation not proven absent beforehand; no host damage or overwritten content claimed; no Docker rerun). The `tools/patches/n7s-obcp/src/modifications/py/vm.c` missing-SPDX failure is a pre-existing, already-tracked finding appended as a dated observation to the existing **NC-EXT-43** (external/generated provenance-and-notice scope; see also `docs/quality/licensing.md` §2 and sprint history 1.109/1.110/1.114) and cross-referenced to **NC-EXT-44** (T21 checker semantics) — no new licensing NC. N13/N18 rows and sprint-plan 1.153/1.154 aligned to this mapping; N9/R1-R4/N17/N20 remain UNVERIFIED; NC-EXT-94/95/245/246 remain OPEN. |
| 11.06 | 2026-11-14 | frick-fsw | Final documentation-integrity correction pass: remove duplicate format row in verification-results.md; correct GSW/N13 ownership description (project-owned patch/overlay, upstream-derived code, missing SPDX, git diff HEAD empty); register NC-EXT-246 for GSW/N13 license-classification defect; advance revision 11.05→11.06; remove unverified "clean baseline" adjective from header, mark start-state cleanliness UNVERIFIED; ensure N9/R1-R4/N17/N20 marked UNVERIFIED; NC-EXT-94/95 OPEN; NC-EXT-245 open non-blocking; NC-EXT-246 open blocker; add repo evidence directory path to results record. **Superseded by 11.07:** NC-EXT-246 is reallocated to the N18 Docker runner preflight/cleanup gap; the licensing finding is tracked under the existing NC-EXT-43/44. |
| 11.05 | 2026-11-14 | frick-fsw | Correct baseline header to reflect uncommitted S1 delta (not committed); record N19 sequencing deviation as NC-EXT-245; correct N15 gate status (smoke FAILED exit 2, GSW self-check FAILED exit 2, N18 artifacts preserved); add §13 N19 assessment/outcome; TS-1 rev 1.14 with N22-TS1-105; independent reviews UNVERIFIED. |
| 11.04 | 2026-11-13 | frick-fsw | Qualify the f07c digest and 297/297 reports as a historical snapshot after R4 identified five covered status-file checksum failures; current package status is determined only by matching the active manifest and current reports. |
| 11.03 | 2026-11-13 | frick-fsw | Bind N8/N11/N20 and §12.3 to independently attested active digest f07c2ca4… (297/297), while preserving snapshot invalidation, historical-report and no-approval boundaries. |
| 11.02 | 2026-11-13 | frick-fsw | Record bounded R4/N17 PASS with no findings for the N8/N11/N20 status correction; package integrity remains separately dependent on final exact-digest attestations. |
| 11.01 | 2026-11-13 | frick-fsw | Correct N8/N11/N20 summaries per R4 inventory review: 157-entry preview and 41-path outside inventory are historical; do not point to superseded reports as current, and require exact matching attestations for active package integrity. |
| 11.00 | 2026-11-13 | frick-fsw | Correct §12.3 per R4-AUDIT-01: distinguish prior R2/R4 checksum and documentation reviews as snapshot-bound historical results; require exact active-digest attestations before asserting current candidate integrity. |
| 10.99 | 2026-11-13 | frick-fsw | Reconcile current status wording to require exact inventory/checksum and matching post-manifest attestations without stale point-in-time pending claims after a successful freeze. |
| 10.98 | 2026-11-13 | frick-fsw | Record the R4 pre-freeze correction re-review confirming stale active manifest and pending combined freeze/attestations, accurate T3/TS-2/NC-EXT-191 limits, and no aggregate approval. |
| 10.97 | 2026-11-13 | frick-fsw | Correct the current outcome to refer neutrally to the active manifest and three independent attestation reports, avoiding stale pending text after freeze while preserving historical S1 status. |
| 10.96 | 2026-11-13 | frick-fsw | Record the completed bounded R2 candidate review, R4 status-delta pass, and correction to the UART verification-results review summary; the old package-wide manifest remains historical and attestation pending. |
| 10.95 | 2026-11-13 | frick-fsw | Label §2 explicitly as the historical 2026-11-09 rules-audit snapshot and point to §12.2/§12.3 as current candidate status, resolving stale gate/review counts without rewriting prior evidence. |
| 10.94 | 2026-11-13 | frick-fsw | Record PA/N20 substantive review with no new product finding for the bounded UART scope; the historical package-wide manifest and independent attestations remain open. |
| 10.93 | 2026-11-13 | frick-fsw | Record independent R1/R3 source/API re-reviews and R2/R4 evidence/docs delta reviews; local run-assets checks pass, while PA and package-wide inventory/manifest/attestations remain outstanding. |
| 10.92 | 2026-11-13 | frick-fsw | Resolve R3’s expired zero-length deadline edge and source-comment overclaims; add N22-TS2-86 and refresh evidence/hashes. Exact-candidate review and package-wide manifest work remain. |
| 10.91 | 2026-11-13 | frick-fsw | Close documentation/evidence corrections from first independent review round: exact hashes and focused evidence, traceability limits, tool-register delta, direct no-retry counter, and combined review/manifest state. Await exact-candidate re-reviews. |
| 10.90 | 2026-11-13 | frick-fsw | Apply bounded UART final-write deadline post-check from R3 observation; add N22-TS2-85 explicit timeout/bytes oracle and update outcome records. Target timing/readiness and manifest/re-review gaps remain. |
| 10.89 | 2026-11-13 | frick-fsw | Update the current-loop result with all-write restore checking, errno preservation, focused/full-test evidence, local gate outcomes, initial review findings and remaining re-review/manifest/target gaps; preserve the start-of-loop record separately. |
| 10.88 | 2026-11-13 | frick-fsw | Start the next N19 loop against actual HEAD and dirty candidate: select the bounded UART `send_until()` restore-failure gap under S8-PUS-FU/SRS-REQ-074; record N1-N22 status, prior manifest attestations as snapshot-bound, and strict scope boundary before behavior change. |
| --- | ---------- | --------- | ------ |
| 10.87 | 2026-11-13 | frick-fsw | Mark the PA report’s 45-entry verification as a historical snapshot and direct readers to the pre-indexed PA attestation for the eventual final digest/inventory result, avoiding a stale present-tense claim. |
| 10.86 | 2026-11-13 | frick-fsw | Align the current-loop N16/manifest protocol references with handover rev 3.74 and state that final checksum evidence is supplied by the exact PA/R2/R4 attestation records, not inferred by the status documents. |
| 10.85 | 2026-11-13 | frick-fsw | Reconcile current N8/N16/N17 audit wording with the 41-path outside inventory, four preview-correction reports and handover rev 3.73; keep the final integrity result delegated to the three post-manifest attestations without asserting approval. |
| 10.84 | 2026-11-13 | frick-fsw | Make current audit wording durable across final manifest generation: distinguish the obsolete 45-entry/history and 157-entry preview snapshots from the final package status, which is solely established by the exact PA/R2/R4 post-manifest attestation records. No checksum claim or approval is inferred by this audit. |
| 10.83 | 2026-11-13 | frick-fsw | Update the review index/results/audit/handover for the completed PA/R2/R4 preview reviews and four post-preview reports. Mark candidate-path count resolved at the review snapshot, with identity rebind/check required after these additional tracked status updates. The active manifest remains stale. |
| 10.82 | 2026-11-13 | frick-fsw | Record PA/R2/R4 final-inventory preview findings and the R2 follow-up resolving candidate identity count/hash against the refreshed 11-path diff. The 157-entry preview is historical after 4 reports and outside-path list changes; active manifest remains stale pending final generation/check. |
| 10.81 | 2026-11-13 | frick-fsw | Record the post-R4-documentation correction cycle in `verification-results.md` using current isolated Make, CI-local, smoke, docs, GSW/license, pinned changed-file format and trace logs. The full-cycle host/emulator evidence remains test-only; package-manifest and independent inventory reviews remain open. |
| 10.80 | 2026-11-13 | frick-fsw | Align N16 with handover rev 3.71 and preserve the obsolete 45-entry manifest as `assets-stale-45-entry.sha256` before replacement; it is historical evidence, not the active manifest or a source of current integrity claims. |
| 10.79 | 2026-11-13 | frick-fsw | Update current audit disposition for DOC-R4-FINAL-01: final substantive review exists and found a Major; change actual manifest inclusion to prospective until the frozen manifest is generated and checked. Align current candidate identity count and N16 handover revision. |
| 10.78 | 2026-11-13 | frick-fsw | Record completed substantive R4 review DOC-R4-FINAL-01 Major; correct prospective/actual manifest-inclusion wording and note that the current manifest is stale/incomplete. Handover was added to the tracked candidate delta, now eleven paths. Final inventory generation and independent correction reviews remain open. |
| 10.77 | 2026-11-12 | frick-fsw | Pre-index the exact R4 substantive N17 review report in the verification-results staging scope; it will review final content/process and be included in the package manifest, distinct from post-manifest hash attestations. |
| 10.76 | 2026-11-12 | frick-fsw | Reframe the N8/N9 status table so reviewed findings are identified as snapshot-scoped, while the pre-indexed final substantive R4 report and post-manifest PA/R2/R4 attestations carry their distinct current dispositions; no attestation substitutes for substantive review or NC closure. |
| 10.75 | 2026-11-12 | frick-fsw | Clarify that the final substantive R4 review (included in the manifest) and the post-manifest hash attestations have distinct authority/scope; make the PA/R2 attestations non-approving integrity evidence only, and correct N21 to describe an uncommitted candidate test. |
| 10.74 | 2026-11-12 | frick-fsw | Define the nonrecursive final-manifest evidence boundary consistently: all package evidence and substantive reviews are included; only `assets.sha256` and the three exact pre-indexed PA/R2/R4 post-manifest hash attestations are excluded. Attestations are not substantive reviews or NC closure; the enclosing Git commit is intended to bind them together. Add the separate final substantive R4 review path. |
| 10.73 | 2026-11-12 | frick-fsw | Record final-pass `make ci-local`, trace, Doxygen generation, and isolated native smoke logs in the controlled results. The final CI subset includes CTest/Lean/receipt/trace; no hardware or full sprint completion inferred. |
| 10.72 | 2026-11-12 | frick-fsw | Repeat native simulator smoke after a default-path attempt encountered the retained `/workspace` CMake cache; the isolated `BUILD_DIR` smoke passes, and the failed cache-path attempt is retained but not treated as a software test failure. Record the disposition in verification results. |
| 10.71 | 2026-11-12 | frick-fsw | Reconcile the N16 current-loop statement with the newly added project CHANGELOG entry and current audit revision. No additional product change; manifest and independent correction gates remain open. |
| 10.70 | 2026-11-12 | frick-fsw | Add the project-level CHANGELOG entry required by N16, accurately describing the test-only increment and open gates; expand the baseline/delta identity record from nine to ten tracked candidate paths. Final package-manifest verification remains pending. |
| 10.69 | 2026-11-12 | frick-fsw | Add an exact nine-path candidate diff and identity record bound to baseline HEAD; include it in verification-results and package README so the uncommitted product/document delta is explicit while unrelated untracked files remain excluded. Final package manifest is still pending. |
| 10.68 | 2026-11-12 | frick-fsw | Clarify the current-loop scope statement: r4 is the evidence package, but its package-level manifest is stale and not verified after current updates. No current package-integrity claim is made. |
| 10.67 | 2026-11-12 | frick-fsw | Update the controlled S1 CTest result row to cite persisted invocation inventory, 55 retained executable copies, per-binary hash list, and its successful 55/55 verification log. Package-level manifest remains stale pending final rebind and independent correction checks. |
| 10.66 | 2026-11-12 | frick-fsw | Correct N2 to reflect the completed R4 correction/delta review and its remaining package-manifest Major rather than stating no R4 review was obtained. No status closure beyond the explicit bounded DOC-R4-RR-02 disposition. |
| 10.65 | 2026-11-12 | frick-fsw | Update the review index, results record and r4 README for the completed PA/V&V/R4 evidence-persistence deltas; record R4 DOC-R4-RR-02 as resolved and RR-01 partial/open, V&V/R2 Major still open, and 55/55 retained CTest binaries. Current package manifest remains explicitly stale pending final freeze/rebind. |
| 10.64 | 2026-11-12 | frick-fsw | Record V&V/R2 and R4 delta reports: the 55-binary CTest inventory and current pending-manifest wording are independently confirmed; R4 DOC-R4-RR-02 is resolved, while R2 and R4 evidence-integrity Majors remain open until final package-manifest rebind and PA/R2 delta checks. No overall gate or NC closure. |
| 10.63 | 2026-11-12 | frick-fsw | Reconcile N8/N9/N11/N12/N17/N20 with the completed R1/R3 reports, fresh V&V/R2 and R4 CHANGES REQUIRED reports, retained 55-executable CTest inventory, and stale current manifest. N12 no longer says the R1/R3 confirmation is pending; unrelated findings remain open. Final report updates, manifest rebind, and PA/R2/R4 delta reviews remain required. |
| 10.62 | 2026-11-12 | frick-fsw | Align N16 with the current document revisions after the traceability mirror, audit and isolated Make-target evidence updates. No source behavior or requirement changed; final independent review and manifest rebind remain pending. |
| 10.61 | 2026-11-12 | frick-fsw | Execute required host `make build` and `make test` using isolated `BUILD_DIR` after the pre-existing root-owned `build/native` cache (created at `/workspace`) rejected the host source path. Both isolated targets pass; failed default-path attempts are retained and the existing cache was not overwritten. Update N15 and r4 evidence. |
| 10.60 | 2026-11-12 | frick-fsw | Correct the r3 evidence README to state it is superseded and has no r3 manifest; identify r4 as authoritative and hash-bound, avoiding conflicting README claims. Update N8 accordingly. Final manifest rebind and independent correction reviews remain pending. |
| 10.59 | 2026-11-12 | frick-fsw | Update N11 after PA independently verified the 45-entry manifest: the PA report and R1/R3 correction/synchronization reports, review index, traceability mirror and audit have since changed, so the final manifest rebind/check remains pending. This is currentness bookkeeping; no gate or NC closure. |
| 10.58 | 2026-11-12 | frick-fsw | Record the PA-R3 final-manifest review disposition: the 45-entry manifest and r4 evidence binding passed independently, resolving PA-N20-R3-01 in substance for the inspected bytes. Since the report itself is hash-bound and has changed, regenerate/reverify the final manifest and obtain a delta check; V&V/R2 and R4 correction re-reviews remain pending. No NC or release closure. |
| 10.57 | 2026-11-12 | frick-fsw | Record R1/R3 correction re-review PASS WITH COMMENTS for the Major coverage-attribution fix and the independent PASS confirming both Minor synchronization corrections; no NC/profile/policy closure. |
| 10.56 | 2026-11-12 | frick-fsw | Restore and validate the 11-module Lean elaboration receipt after the trace gate identified it missing (`NC-EXT-174` remains open); record `make lean-receipt` PASS in r4 and the N1 disposition. This repairs only local evidence state; no NC closure. |
| 10.55 | 2026-11-12 | frick-fsw | Address the non-blocking R1/R3 correction re-review observations by synchronizing `docs/requirements/traceability.md` rev 0.54 with the SRS-REQ-017 machine-readable mapping and by reconciling this audit's current evidence pointer to r4. R1/R3 confirmation, R4, PA and V&V correction re-reviews remain in progress; no external policy or NC closure. |
| 10.54 | 2026-11-12 | frick-fsw | Generate the 45-entry r4 `assets.sha256` binding current candidate source/docs, build cache/executable, retained gate logs, fresh N18 ELF/raw logs and current-cycle review reports; `sha256sum -c` passes for all entries. PA correction re-review and all other independent correction re-reviews remain pending; this records evidence status only, not finding/NC closure or commit approval. |
| 10.53 | 2026-11-12 | frick-fsw | Add final r4 evidence package with fresh configure/build/CTest/focused test run and source identities; record `make ci-local`, Doxygen container generation, and Renode environment self-test passes. The full pinned-format discrepancy remains NC-EXT-244 OPEN/deferred; N20/N17 correction re-reviews and the manifest remain pending. |
| 10.52 | 2026-11-12 | frick-fsw | Correct N18 evidence currency: the prior r3 invocation used a stale ELF (`55188f5a…`) and is not accepted as current evidence. Rebuild the image with the Docker RTEMS toolchain and rerun Renode; fresh ELF `e36aee30…4843a41` is bound to raw run logs/status and passes `RENODE_TEST`. Retain stale diagnosis and fresh evidence; no physical-target claim. |
| 10.51 | 2026-11-12 | frick-fsw | Record R1/R3 review SYS-R1R3-01 and correct its Major coverage-attribution finding: TS-1 `N22-TS1-23` now remains a gap with implementation-characterization evidence pending NC-EXT-13 policy; recount 104 cases as 76 covered + 28 gap and update the gap list and traceability justification. Fresh PA/R2/R1-R3/R4 re-reviews are pending. No policy decision or NC closure. |
| 10.50 | 2026-11-12 | frick-fsw | Update the current-loop audit after execution of smoke, GSW self-check and pinned-format checks. Record the clean host clang-format 23.1.3 result separately from pinned clang-format 18: full pinned check fails on unchanged initializers at `src/transport/link/link_tsn.c:18`, `src/transport/link/link_ip.c:19` and `tests/unit/test_link_tsn.c:24`, while changed `test_pus.c` passes pinned format; register NC-EXT-244 OPEN/deferred under N21. No behavior, sprint closure or approval claim changes. |
| 10.49 | 2026-11-12 | frick-fsw | Start-of-loop N19 audit for the S8-PUS-FU S1 ingestion-failure increment on committed baseline `e5f01d4`; record the exact candidate gates, review state, test-registration finding NC-EXT-240, interface-layout gap NC-EXT-241 and related deferred findings NC-EXT-242/243 and the pre-existing pinned-format discrepancy NC-EXT-244. No NC, sprint, qualification, release or safety approval is claimed; full sprint work remains open. |
| 10.48 | 2026-11-11 | frick-fsw | **Resolve NC-EXT-165: N18 re-executed on current tree HEAD `833a0e0`.** Fresh RTEMS 6.2 cross-build (`frick-rtems-toolchain:local`) configured, linked and produced `frick_fsw_renode.elf` with zero warnings; prescribed `make renode` returned `[renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)`; all 14 HAL checks PASS; ELF sha256 `55188f5a…` bound to current tree (commit `0cc455f`, NC-EXT-239 `pal_rtems.c` declaration fix, ancestor of HEAD). Companion host gates green: build exit 0 (0 warnings), CTest 56/56, Lean `lake build`, trace PASS (120/76/30/14/68), pinned clang-format 18, license OK. Independent reviews obtained: PA/N20, doc/N17-R4, V&V/R2, avionics/R1-R3 — no finding blocks resolution. Scope note: emulated Renode functional validation only — not target/HIL, not RTPFS160ZT fidelity, not WCET/timing; flight hooks remain `FF_NOT_IMPLEMENTED`; NC-PAL-01, NC-EXT-186 and NC-EXT-188 remain OPEN and are NOT closed by this resolution. NC-TS-01 ELF-freshness weakness mitigated for this run (freshly built ELF) but remains a register item for a structural script fix. NC-EXT-142 disposition corrected (sprint row no longer limits Renode/cross-build PASS to 2026-10-20 tree state). NC-EXT-165 resolved for this tree under N12 with independent R2 verification (FRICK-REVIEW-VV-R2-N18-CT-20261111). |
| 10.47 | 2026-11-11 | frick-fsw | **NC-EXT-191 backend-coverage half implemented (IP and TSN deadline writers).** Section 10 audits the increment against N1/N3/N4/N10/N11/N15/N18/N20/N21/N22: `link_ip.c`/`link_tsn.c` implement `FF_Link_t::send_until`, `link.h` documents the return contract, TS-2 rev 0.10 adds `N22-TS2-82/83`, and the mutation control shows each new assertion discriminates. Gates re-run to success (build, CTest 56/56, Lean, trace 120/76/30/14/68, clang-format 18, GSW/license, smoke, metrics 46.5%, RTEMS cross build, Renode PASS). Remaining gaps tracked: NC-EXT-191 (UART shared-descriptor concurrency, target readiness), GAP-A5/`N22-TS2-78`, T3 qualification, NC-PAL-01, NC-EXT-239. No NC closed. |
| 10.46 | 2026-11-10 | frick-fsw | **`N22-TS5-66` emulated-target gate completed; NC-EXT-239 registered.** RTEMS 6.2 cross-build PASS for the N18 profile (`build/renode`, exit 0, ELF sha256 `5557d0bd…1a0e`) and **N18 Renode PASS** (`RENODE_TEST: PASS`, container exit 0; console `SCH app started, 4 entries`), with `trace.py` (120/76/30/14/68) and pinned clang-format 18.1.3 re-run PASS. NC-EXT-239 OPEN (non-functional, N21): pre-existing `pal_rtems.c:156` implicit declaration of `ff_pal_wd_hw_feed`, prototype only in `pal_watchdog.c`; owner FSW. The S8-SCH sprint row, the closure table and the residual-items section no longer state `Renode N18 FAIL`; N18 is emulated-target evidence only, so target/HIL (GAP-A5), NC-PAL-01, NC-EXT-19/90, `N22-TS5-47`/`N22-TS5-48`, M14 and the independent reviews remain open. No NC closed, no commit/push. |
| 10.45 | 2026-11-10 | frick-fsw | **S8-SCH `N22-TS5-66` delivered (TBL load/activate → scheduler dispatch).** M17 row updated: the application registers an `FF_TblActivateCb` that re-validates and snapshots the activated image into a mutex-protected pending slot; the scheduler task applies it (`FF_SchedActivate` + `FF_SchedInit`) at the top of its next tick with a non-blocking lock and emits the re-activation event, preserving the single-writer dispatcher contract with a bounded one-tick deferral. Production-path nominal + out-of-bound host cases in `test_tbl_sched_coupling.c` (61 checks); test-delay seam made deterministic (each block notification consumed once) and the `N22-TS5-47` integration expectations corrected to the deterministic sequence. Gates: build clean, CTest 55/55 PASS, trace PASS (120/76/30/14/68), Lean PASS, pinned clang-format 18 PASS, RTEMS 6.2 cross-build PASS (N18 profile) and **N18 Renode PASS** (`RENODE_TEST: PASS`). `N22-TS5-66` closed as a test gap; NC-EXT-19/90, `N22-TS5-47`/`N22-TS5-48`, M14 memory ordering, the deferral budget and target/HIL evidence remain open. No NC closed, no commit/push. |
| 10.44 | 2026-11-09 | frick-fsw | **Emulated MPFS WDOG0 binding increment (N18 build) + provisional GEO parameters.** Implemented the test-only emulated WDOG0 binding in `src/core/pal/pal_rtems.c` for `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST` against the verified register map in `config/platform/platform_mpfs.h` (`0x20001000`; `TRIGGER -> MSVP -> TIME -> CONTROL`; read-back before the first keyed `REFRESH`), using the pinned model's emulated 156250 Hz clock, `MSVP = TIME`, `TRIGGER = 4095`; flight builds remain `FF_NOT_IMPLEMENTED` (NC-PAL-01). Moved the feeder to absolute monotonic deadlines (`FF_PalTaskDelayUntilNs`, NC-EXT-187). Set the mission-table timeout provisionally to 10 s (feed 2.5 s) as representative GEO values under new NC-EXT-238. Updated the host tests (CTest 54/54). **N18 Renode PASS 2026-11-09** (`RENODE_TEST: PASS`; console shows the WDOG0 arm/feed). Gates: native CTest 54/54, pinned clang-format 18.1.3 PASS, `lake build` PASS, elaboration receipt PASS (11 modules), trace PASS (120/76/30/14/68), `gsw-selfcheck` PASS, `metrics.py --check` PASS, RTEMS 6.2 cross-build PASS. Updated ADR-0021 rev 1.3, FRICK-ARCH-0024 rev 1.6, traceability.yaml, sprint-plan rev 1.127, handover rev 3.65 and verification-results. Emulated integration evidence only: no flight parameter, target/HIL protection, RTPFS160ZT fidelity or mission suitability is claimed. The increment still needs independent R2/R3/R4/PA review. No NC closed, no commit/push. |
| 10.43 | 2026-10-25 | frick-fsw | S8-PUS-FU NC-EXT-207 bounded increment: software-separable wrap-safe due-test correction (`time_period_due` signed-modular formulation) implemented and independently reviewed. Pre-fix `time_ge`/`time_add_period` reproduced (10 failures at wrap-straddle and forced-wrap `period_s >= 2^32-now`); three new tests added (wrap-straddle exact due, large-period held-unserved, backstep + undecided-band) with exact oracles. Mutation discrimination executed: three critical mutants (wait-band removal, tie-break >=→>, u>period→u>=period) killed at expected sites. Gates: `make build` PASS, `make test` 46/46 (`test_pus` 225/0), `make trace` PASS (120/76/30/14/68), `make verify-lean` PASS, `make format-check` PASS, `make gsw-selfcheck` PASS. **N18 Renode FAIL (NC-EXT-198) NOT RUN, NC-PAL-01 OPEN.** Authority half remains: `u >= 2^31` undecided band → `period_s >= 2^31` held unserved (formulation property, no validation/limit); ESC-HK-06 must dispose max-period/undecided-band policy; ESC-HK-03 held-unserved hazard; NC-EXT-208 untouched. No caller added (ESC-HK-02 pending). No commit/push, no NC closed, no gate/phase/qualification/release approval. Reviews: FRICK-REVIEW-0437 (V&V) and FRICK-REVIEW-0438 (PA) 2026-10-24 formulation-level; 2026-10-25 exact-candidate V&V and PA reviews both ACCEPT narrow claim. Evidence recorded in `docs/vv/evidence/2026-10-25-ncext207-wrap-due-evidence.md`. |
| 10.42 | 2026-10-22 | frick-fsw | Watchdog specification phase (owner path A, ESC-9/ESC-10): the controlled MPFS/MSS watchdog integration specification FRICK-ARCH-0024 rev 1.5 is produced, reviewed and retained (FRICK-REVIEW-0424/0425 for rev 1.0; FRICK-REVIEW-0426 embedded safety and FRICK-REVIEW-0427 V&V for the final delta) with the reference inventory (public files pinned by commit and SHA-256, not retained per the N13 boundary), the installed-RTEMS-BSP inventory (no MSS register map or driver) retained as evidence, the verified facts, recommendations, missing inputs M-1..M-16, the corrected emulator/host test sets, the target/HIL legs including the NC-PAL-01 closure conditions, and the owner decision table A, B1/B2, C, D, E, F (all OPEN). The proposal's non-fatal variant is withdrawn per the owner's rejection (ESC-10) and the alternative is keeping N18 blocked/FAIL. Documentation only: no binding, no register write, no production change, no policy decided, no NC or gate closed. NC-EXT-200/201 registered |
| 10.41 | 2026-10-21 | frick-fsw | NC-EXT-199 corrected: the prescribed N18 runner now writes into a fresh per-run directory and the verdict is taken only from it, with a fail-closed verdict unit (missing/unreadable/empty capture, failed directory/container/capture → FAIL) that keeps the container status, container exit code and bounded-wait outcome visible and distinguishes a target startup fatal exit from a timeout. The regression test (registered with CTest) uses only genuine recorded runs and is mutation-verified: a stale PASS copy plus a marker-less run FAILs, invalid captures FAIL with their reason, the recorded success PASSes, the recorded startup-fatal run FAILs with the fatal reason, and a static guard pins the runner against the fixed paths. Independent review FRICK-REVIEW-0423 confirmed the correction and its negative controls. The actual N18 run on the current tree remains **FAIL** (the target panics at start-up, NC-EXT-198). Host CTest 45/45, the regression suite 17/17, trace PASS. NC-EXT-198/199 stay OPEN; no gate or NC closed |
| 10.40 | 2026-10-21 | frick-fsw | Renode/Lean phase (owner-authorized Telegram 1397): the project-pinned Lean toolchain (`leanprover/lean4:v4.10.0`, Lean 4.10.0 commit c375e19f6b65) was installed through elan in `frick-vv:local` into the dedicated cache `~/.cache/frick-fsw/elan` with no global default change, and the prescribed `lake build` now **PASSES** on the current tree. The digest-pinned Renode image was downloaded and digest-verified against the prescribed pin, the current-tree Renode ELF was rebuilt (sha256 `23a4c8da…`), and the N18 functional validation **FAILS**: the target panics at start-up (`[FF-PANIC] watchdog: init failed` → RTEMS fatal exit 5) because the owner-approved arm-error propagation treats the unwired MPFS binding as a fatal arm failure, so the `RENODE_TEST: PASS` marker is absent from this run's process log and UART console — recorded as FAIL, not PASS and not SKIP (NC-EXT-198). The runner's own PASS verdict was a false PASS from a stale 2026-10-20 log because its fixed `/tmp` log paths are not writable here (NC-EXT-199). No watchdog policy, cadence, binding or requirement wording changed; no gate or NC closed |
| 10.39 | 2026-10-21 | frick-fsw | NC-EXT-197 correction: the UART test drain-stop flag is protected by a pthread mutex (C99 project level; C11 atomics not used), initialized before thread creation and destroyed after the join, preserving the stack-object lifetime, the bounded drain shutdown and the forced send-failure semantics. The identical forced-failure ThreadSanitizer reproducer reports 0 races after the correction (1 before), with the before/after pair and the exact pre-fix source retained; independent review FRICK-REVIEW-0422 confirms the fix and isolates the change to one test file. Nominal UART 74 checks, full CTest 44/44, trace PASS, pinned format PASS, canonical `make build`/`make test`/`make smoke` PASS. NC-EXT-197 stays OPEN pending owner disposition; no gate or NC closed |
| 10.38 | 2026-10-21 | frick-fsw | Gate-verification phase for the owner-directed increment: canonical `make build` and `make test` and `make smoke` executed with the existing pinned tools and an isolated build directory (build/acceptance) — all PASS; the earlier third-party fetch/ignored-directory blocker no longer exists (the pinned checkouts are clean at their pinned commits and the cached `frick-asn1scc:local` image is available), and the checkouts were re-verified clean afterwards. Lean: NOT RUN this cycle (no toolchain in the cached V&V image); the retained 2026-10-21T09:58Z evidence re-verifies 14/14 source-identical and is reused, with no new Lean result claimed. N18: NOT RUN — the digest-pinned `antmicro/renode@sha256:7905c32…` image is **absent** from the local store, so the prescribed `renode_vv.sh` would pull it (network, ~1 GB, not authorized here); the gate's prerequisite is now met by rebuilding the current-tree Renode ELF at the prescribed path (sha256 `23a4c8da09c37a97f4ef50aa980d68e8905e94ddca6fa785877a59fc967cdab2`), which also removes the stale-artifact trap behind NC-TS-01. The UART drain-stop flag was shown by ThreadSanitizer to be a C11 data race (`volatile` is not a synchronization primitive): registered OPEN/deferred as NC-EXT-197, and the earlier "the drain thread cannot hang" wording is superseded. Hosted CI and target/HIL remain separate and unrun. No gate or NC is closed |
| 10.37 | 2026-10-21 | frick-fsw | Handover acceptance verification: the review chronology reconciled against the file timestamps, the corrections made after FRICK-REVIEW-0419/0420 independently reviewed by a fresh narrow N20 review (FRICK-REVIEW-0421), its Major/Minor record findings applied and registered as NC-EXT-193..196, the duplicate-emission mutation retained as evidence, and the executed counts updated to 320/180/72/113 with full CTest 44/44, trace PASS, pinned format PASS, a clean RTEMS link and a 31/31 source manifest. The authority set remains open |
| 10.36 | 2026-10-21 | frick-fsw | Correction round: apply the record-currency findings of FRICK-REVIEW-0419/0420 (executed counts 320/180/71/113, the Blocker attribution to 0418, TS-5 rev 1.12 with the strengthened exactly-one oracle, ADR-0021 rev 1.2, NC-EXT-191/192, the 31-entry source manifest covering the changed V&V scripts, and the cross-build transcript with the exact invocation, exit codes, artifact identity and symbol checks). The duplicate-emission mutation confirms the corrected exactly-one oracle fails on the production wiring path. Host CTest 44/44 and the RTEMS 6.2 cross-build link cleanly; the authority set remains open and no gate or NC is closed | The duplicate-emission mutation is retained as evidence in `docs/vv/evidence/2026-10-21-s8-fdir-post-review-remediation.log`; the corrected product was independently reviewed once more in the 2026-10-21 acceptance review.
| 10.35 | 2026-10-21 | frick-fsw | Final remediation round for the same increment: correct the flight-target link defect found by PA FRICK-REVIEW-0418 (the increment had introduced `poll()` into RTEMS-compiled sources; RTEMS 6.2 implements no `poll()`, and the project cross-build failed with an undefined reference — the bounded console/UART writers now use a non-blocking descriptor with a bounded retry, and a fresh RTEMS 6.2 cross-build with the cached toolchain image configures, links and produces `frick_fsw` with no `poll` symbol and no exported test seams); apply the in-scope oracle/record corrections (loopback deadline re-check plus its own case, exactly-one-frame assertion, guarded link-time wrappers, bounded UART drain, `vsnprintf` error guard, `stderr` mode restore, extended source-identity manifest, TS-5 §3 gap mapping, traceability note, live sprint row, NC register entries NC-EXT-184..190). Host CTest 44/44 with targeted 320/172/68/113. The requirement-semantics, reset-path-liveness, target-timing, cadence and unwired-binding items remain authority decisions; no gate or NC is closed |
| 10.34 | 2026-10-21 | frick-fsw | Complete the post-review remediation round: direct whole-path console/deadline oracle with notifier-observed expired deadline and pre-reset reset counter, deterministic UART short-write injection, production reporter wiring case over the loopback deadline-aware writer, and the retained independent delta reviews R1 0414 (rejected for closure of 0409), R2 0415 and N20 PA 0413 (not approved). Host totals 320/157/68/113, full CTest 44/44, trace and pinned format PASS with a recomputed source-hash file. The exactly-one/mandatory-text policy, the bounded-formatting design, the controlled MSS refresh window and the approved jitter margin remain authority items; N18, current-tree Lean, smoke and target/HIL remain open |
| 10.33 | 2026-10-21 | frick-fsw | Begin same-increment post-review remediation: add external host syscall fault injection, full panic console-budget and overlength counterexample, UART partial/stall/retry tests, feed/reset error-state assertions; full CTest 44/44 and pinned format pass. Current cached V&V image includes lake but lacks toolchain; exact-tree Lean not rerun. Await fresh independent R1/R2/R4 and exact-product PA/N20 reviews; policy/window blockers remain |
| 10.32 | 2026-10-21 | frick-fsw | Retain fresh-context R1/R2/R3/R4 delta reports 0409–0412 and triage: report formatting/overlength semantics and actual feed cadence are unresolved; PA/N20 attempt interrupted; update R4 corrections and public vendor HAL lead without treating it as target datasheet/binding evidence. 250 ms end-to-end, watchdog refresh window, N18/Lean/smoke and all release gates remain open |
| 10.31 | 2026-10-21 | frick-fsw | Start and record the owner-authorized bounded S8-FDIR-01 delta: correct N18 watchdog-model inventory from pinned-image audit; capture current host tests/build timeout; add 250 ms report-path, UART, PAL error and CUC holdover evidence with explicit coverage limits. Feed cadence/window, hardware binding, full N15 and independent gates remain open |
| 10.30 | 2026-10-21 | frick-fsw | Audit the 2026-10-21 independent-review cycle: **record that the N1 row's earlier Lean PASS was not true** (`Frick/Watchdog.lean` was outside the build graph and did not compile; NC-EXT-120), correct the N1 and N22 rows to the measured state, and note that the requirement-based test set had four oracle gaps the previous audit counted as covered (NC-EXT-121/126/127/128) while ordering is not test-observable (NC-EXT-132). All NCs and gates remain open; N18 is pending for the current tree (NC-EXT-165) |
| 10.29 | 2026-10-20 | frick-fsw | Record R4/PA FRICK-REVIEW-0403/0404 confirmation of NC-EXT-119 historical-attribution correction; keep NC-EXT-119 OPEN pending authorized owner disposition and preserve all gates |
| 10.28 | 2026-10-20 | frick-fsw | Record PA FRICK-REVIEW-0402 historical-attribution finding NC-EXT-119 OPEN; correct handover point-in-time audit reference and retain current-pointer/gate status, with bounded R4/PA confirmation pending |
| 10.27 | 2026-10-20 | frick-fsw | Record R4/PA FRICK-REVIEW-0400/0401 stale-pointer findings under NC-EXT-116; update handover to results rev 3.00 and current loop-audit rev 10.27, with bounded confirmation pending and all N18/N15/release gates open |
| 10.26 | 2026-10-20 | frick-fsw | Record bounded R4/PA confirmations FRICK-REVIEW-0398/0399 of the NC-EXT-116 revision-reference correction; leave NC-EXT-116 OPEN pending authorized owner disposition and preserve all N18/N15/release gates |
| 10.25 | 2026-10-20 | frick-fsw | Triage R4/PA FRICK-REVIEW-0396/0397: correct NC-EXT-116's handover-revision reference from 3.27 to 3.28 in sprint-plan rev 1.87; keep NC-EXT-116 OPEN pending independent confirmation and N18/N15/release gates open |
| 10.24 | 2026-10-20 | frick-fsw | Record R4 FRICK-REVIEW-0395 partial confirmation; correct remaining handover §5 N18 pointer to verification-results 3.00 and preserve NC-EXT-116 OPEN pending bounded R4/PA confirmation; N18/N15/release gates remain open |
| 10.23 | 2026-10-20 | frick-fsw | Record R4 FRICK-REVIEW-0394 stale active N18 pointers mapped to NC-EXT-116 OPEN; correct sprint-plan and handover pointers to verification-results 3.00 / loop-audit 10.22, retain bounded R4/PA confirmation and all N18/N15/release gates as open |
| 10.22 | 2026-10-20 | frick-fsw | Record R4 delta FRICK-REVIEW-0393 confirming NC-EXT-116/117 corrections and finding stale disposition instructions; register NC-EXT-118 OPEN, correct sprint-plan wording in rev 1.84, and retain bounded R4/PA confirmation and all N18/N15/release gates as open |
| 10.21 | 2026-10-20 | frick-fsw | Record R4 FRICK-REVIEW-0391 and PA FRICK-REVIEW-0392: register NC-EXT-116/117 OPEN for the stale N18 result pointer and outdated highest-NC sequence note, correct both in sprint-plan rev 1.83, and retain the need for bounded R4/PA confirmation; N18 acceptance, N15, NCs and target/release gates remain open |
| 10.20 | 2026-10-20 | frick-fsw | Triage independent R2 FRICK-REVIEW-0390: record NC-EXT-115 OPEN for inconsistent N18 UART captures, map the SB receive-only oracle gap to existing NC-EXT-47, update the N18 result pointer to verification-results rev 3.00, and record R4/PA review as pending for this documentation delta; N18 acceptance and all NC/target/release gates remain open |
| 10.19 | 2026-10-20 | frick-fsw | Reconcile bounded N17/PA follow-ups, correct current verification/T17 references and register PA finding NC-EXT-114 for unsupported PCIe on-target claim; preserve all target/HIL, qualification and release gates |
| 10.18 | 2026-10-20 | frick-fsw | Reconcile N18 after fresh container RTEMS cross-build and prescribed Renode PASS against current S8-FDIR-01 sources; correct stale resume/sprint-status wording and clarify the source/build-control manifest scope; retain NC-TS-01, NC-EXT-40, N15, review, target/HIL and release gates as open |
| 10.17 | 2026-10-20 | frick-fsw | Record independent R4 delta review FRICK-REVIEW-0387; map findings to NC-EXT-28/35/112 and register NC-EXT-113 OPEN/deferred; preserve 2/5 host result and separate T24 PASS, leave findings uncorrected and all approval/closure gates open |
| 10.16 | 2026-10-20 | frick-fsw | Reconcile the ordered host-gate log and T24 container dry-run: 2 PASS/5 FAIL in the host run, T24 PASS in the pinned image; retain raw logs, append Lean-image evidence to NC-EXT-35, register mixed-owner gate execution NC-EXT-112 OPEN/deferred, and keep the full N15 cycle/reviews/target gates open. No NC or gate closed |
| 10.15 | 2026-10-20 | frick-fsw | Reconcile review-0386 bounded corrections in the traceability, application-count, document-history and status materials; N1/N4 counts 68 theorems and make trace PASS; full applicable cycle, R4 delta review, R2/R3/PA and target/HIL gates remain open. No NC closed or approval inferred |
| 10.14 | 2026-10-20 | frick-fsw | Continue N19 loop for current S8-FDIR-01 delta: reconcile N4 trace counts to 120 total / 76 SRS / 30 SSS / 14 GAP / 68 Lean; record R4 N17 review FRICK-REVIEW-0386 REJECTED, NC-EXT-103..111 and the F-0386-01 traceability-mirror correction (`make trace` PASS, R4 delta review pending); preserve N18 SKIP, NC-PAL-01, NC-TS-01 and pending R2/R3/PA/HIL gates. No NC closed or approval inferred |
| 10.13 | 2026-10-20 | frick-fsw | Loop start (N19) for sprint **S8-FDIR-01** (panic/reset-recovery). Added the S8-FDIR-01 objective row; recorded that the increment advances the engineering baseline (first real code change since `318f49b`) and reconciled it with the documented plan and sprint-plan row 1.74. Registered **NC-PAL-01** (MPFS MSS hardware watchdog binding NOT WIRED - Major FDIR completeness; target/HIL closure with authorised-human sign-off), **NC-EXT-102** (new ground-visible panic event id not yet in the ICD event catalogue - interface-authority disposition) and **NC-TS-01** (the Renode gate keys on ELF existence not freshness: a stale 2026-10-16 pre-change ELF produced a misleading `RENODE_TEST: PASS`; the stale ELF was removed so the increment records a true SKIP, and no RTEMS toolchain image exists here to rebuild). Recorded two audit findings in the N22 row: `N22-TS5-50` was falsely marked `covered` (function 1 never registered; corrected in TS-5 rev 1.6) and `watchdog_timeout_s` was dead configuration (now the live feeder timeout). Updated the N18 row to record the increment's N18 validation as a gap, not a PASS. Header baseline retained at `318f49b` (this increment is an uncommitted working-tree delta). No NC closed and no gate, phase, qualification, safety or release approval implied |
| 10.12 | 2026-10-19 | frick-fsw | Record the 2026-10-19 post-publication review-integration: the deferred batch-scoped N17/N20/N12 reviews were performed at HEAD 7e93325 (FRICK-REVIEW-0381 R4, 0382 R2/R3, 0383 PA; all APPROVED WITH COMMENTS, 0 Major), the bounded corrections they implied were applied (ASan duration, uncommitted label, TS-6 3.1 lwIP/ASan bullet, TS-5/traceability oracle caveat, VP 9 S7/S8 rationale, manifest addendum binding-commit, reviews/README dated historical), and the affected NC rows (NC-EXT-94, NC-EXT-99) were disposition-registered. The N7/N17/N20 rows still record these reviews as 'not obtained before commit', NC-EXT-99 remains OPEN pending N12 confirmation, and the new NC-EXT-100/101 (evidence-manifest revision/history; logs PASS-by-absence evidence record) were registered. No NC closed and no gate, phase, qualification or release approval implied |
| 10.11 | 2026-10-19 | frick-fsw | Declare that status-only commits (no code/test/requirement/evidence content) do not advance the engineering baseline, so the header baseline stays `318f49b` with `b6e1f61` as a status-only follow-up; record hosted CI run `39219583424` (all eight jobs green for `b6e1f61`) in the N7 evidence column. No NC closed and no gate, phase, qualification or release approval implied |
| 10.10 | 2026-10-19 | frick-fsw | Post-commit baseline refresh: the 2026-10-19 easy-fixes batch was committed and pushed to both remotes as `318f49b`, so the header `Baseline commit`, the working-tree scope-boundary row and the N19 row are repointed from `e7e48ee` to `318f49b` (the batch is no longer a working-tree delta); hosted GitHub CI run `39218060325` (all eight jobs green for `318f49b`) is recorded, and the N7 row now states that the batch-scoped N17/N20 reviews are deferred as NC-EXT-99. No NC closed and no gate, phase, qualification or release approval implied |
| 10.9 | 2026-10-19 | frick-fsw | Easy-fixes batch (N19): refresh the working-tree scope-boundary and N19 rows from the stale `b8c3aee`/`1fdbb83` state to the pushed baseline `e7e48ee` plus the 2026-10-19 batch, record the completed R2/R3 delta review (`FRICK-REVIEW-0379/0380`) and R4 `NC-EXT-96` confirmation, and register NC-EXT-97/98. No objective disposition changed and no gate approval implied; register NC-EXT-99 recording that this batch's own batch-scoped R4 (N17), R2/R3 (V&V confirmation) and PA (N20) reviews are deferred by explicit owner disposition to the next cycle, while every other applicable gate passed at commit |
| 10.8 | 2026-10-18 | frick-fsw | Refresh the baseline-commit row to the pushed S8 batch `1fdbb83` (S8-SCH/S8-VV-FU/S8-PUS-FU + selected N7 OBCP integration) after the documentation-vs-implementation audit `docs/reviews/2026-10-18-docs-implementation-alignment.md`; no objective/norm disposition changed and no gate approval implied |
| 10.7 | 2026-10-17 | frick-fsw | Record and process the four independent reconciliation reviews: R2 0370 (V&V delta, approved with comments) §3 gap-list synchronization and oracle-independence wording, R3 0371 (requirement grounding) ICD rev 1.3 for the provisional S3 subservice codes and S6 profile, R4 0372 (documentation) TS-1/TS-5/TS-6 history and list corrections, PA 0373 (no approval) stale candidate identity and the missing `N22-TS5-77` oracle review; register NC-EXT-93 for the review-log ID collision with a bounded mitigation and statement of the narrow-closure vs PA-acceptance taxonomy; refresh the current-tree evidence (ci-local 43/43 + Lean/trace, pinned format, docs, GSW, smoke) and candidate manifest; authority/target/provenance/release gates remain open |
| 10.6 | 2026-10-16 | frick-fsw | Reconcile N9/N12/N19/N20 and the C-07/C-15/C-16 finding rows with the completed independent R1 0355 / R2 0356 delta re-reviews: NC-EXT-88/89 and the bounded NC-EXT-23/24 defects closed narrowly (bounded NC-EXT-17 part recorded under NC-EXT-88), NC-EXT-92 registered for the pre-existing TS-4 coverage-count inconsistency, TS-1 §3 gap list corrected (removed N22-TS1-47/49 which are covered), and TS-1/TS-5 case-table counts reconciled; authority, target, hosted, provenance and release gates unchanged |
| 10.5 | 2026-10-16 | frick-fsw | Refresh current working-tree scope boundary to post-R4 CI result and narrow NC-EXT-91 closure |
| 10.4 | 2026-10-16 | frick-fsw | Record PA narrow verification of supplemental 19-file identity; close NC-EXT-91 only for PUS/Scheduler delta and retain broader candidate gates |
| 10.3 | 2026-10-16 | frick-fsw | Record provisional PUS/Scheduler implementation and fresh CI-local pass; track incomplete candidate inventory as NC-EXT-91 and keep authority/target gates open |
| 10.2 | 2026-10-16 | frick-fsw | Record R4 0369 current audit/index consistency check while retaining no-aggregate-N17 and open phase/release gates |
| 10.1 | 2026-10-16 | frick-fsw | Update current evidence revision pointer and record R4 0367/0368 reconciliation of historical handover steps vs current actions |
| 10.0 | 2026-10-16 | frick-fsw | Record R4 0365/0366 correction of the stale handover hash pointer; preserve scoped-review and no-aggregate-N17 limits |
| 9.9 | 2026-10-16 | frick-fsw | Record PA 0364 final log/cache/executable hash-record review and current R4 gate for verification-results 2.82/status delta |
| 9.8 | 2026-10-16 | frick-fsw | Record R4 0362/0363 bounded review of current evidence/status/manifest corrections; no aggregate N17 approval |
| 9.7 | 2026-10-16 | frick-fsw | Correct N8 evidence revision pointer and reconcile manifest with completed PA 0361 identity verification while retaining provenance/run-configuration limits |
| 9.6 | 2026-10-16 | frick-fsw | Record final identity-cycle CI-local pass and PA 0361 verification of the selected product-set identity; retain provenance, run/configuration and authority gates |
| 9.5 | 2026-10-16 | frick-fsw | Record current ASan/smoke/GSW/pinned format/docs/Renode passes and retain RTEMS cross-build, candidate identity, authority and target gates |
| 9.4 | 2026-10-16 | frick-fsw | Reconcile loop audit after PUS/scheduler focused re-reviews, current CI-local pass and R4 0357/0358 corrections; auxiliary cycle and product-set gates remain pending |
| 9.3 | 2026-10-16 | frick-fsw | Start next loop from R1/R2 PUS review findings, apply S3 first-error fix and coverage corrections, and record current-cycle/review gaps under N19 |
| 9.2 | 2026-10-16 | frick-fsw | Align working-tree scope boundary with PA 0345/0346 and R4 0347; retain generator, authority, target and release gates |
| 9.1 | 2026-10-16 | frick-fsw | Record PA 0346 observed generated-tree identity verification and R4 0347 narrow NC-EXT-85 closure; keep NC-EXT-84 provenance open |
| 9.0 | 2026-10-16 | frick-fsw | Correct working-tree scope gate summary after PA acceptance and track final NC-EXT-85 R4 delta plus NC-EXT-84 provenance |
| 8.9 | 2026-10-16 | frick-fsw | Record R4/PA deltas, product-set identity acceptance, refresh status/currentness and align remaining audit/header-history revisions |
| 8.8 | 2026-10-16 | frick-fsw | Reconcile stale 42/42 cycle rows, batch-review status, narrow NC-EXT-83/86/87 dispositions and current document revision histories |
| 8.7 | 2026-10-16 | frick-fsw | Record full post-0343 CI/ASan/auxiliary passes, candidate manifest and hash identity; retain PA acceptance, review, authority and target gates |
| 8.6 | 2026-10-16 | frick-fsw | Record batch reviews 0338–0340, update stale cycle/status references, and track OBCP test-oracle and trace-link corrections pending fresh cycle and delta review |
| 8.5 | 2026-10-16 | frick-fsw | Record owner direction to defer in-house M16 replacement-engine development; retain SRS-REQ-054 as separately governed open fallback |
| 8.4 | 2026-10-16 | frick-fsw | Record pinned N7 OBCP default-build integration and host connectivity test while retaining NC-S9-01 as deferred/open and M12 incomplete |
| 8.3 | 2026-10-16 | frick-fsw | Extend N19 audit to S9 OBCP intake, immutable candidate identity, limited upstream host test, missing bounded-execution API gate NC-S9-01, and final default-N7 CI/docs evidence |
| 8.2 | 2026-10-16 | frick-fsw | Re-audit owner-directed default-N7 correction, generated-source test path, CI generation steps and stale-cache failure; retain current cycle/review/sprint gates |
| 8.1 | 2026-10-16 | frick-fsw | Record final S3 provider/encoding failure oracles and updated 42/42 cycle reference; sprint remains open |
| 8.0 | 2026-10-16 | frick-fsw | Audit S3 provider-failure propagation and refreshed evidence; retain NC-EXT-17 and independent review open |
| 7.9 | 2026-10-16 | frick-fsw | Audit N7 adapter CRC correction and 42/42 verification cycle; generated SDK and independent review remain open |
| 7.8 | 2026-10-16 | frick-fsw | Audit S8-PUS-FU first increment against actual uncommitted tree, correct baseline pointer, and record absent RTEMS compiler / Renode N18 block; sprint remains in progress |
| 7.7 | 2026-10-16 | frick-fsw | Record post-deferral local verification cycle while preserving physical-target, authority, external, N20 and release limits |
| 7.6 | 2026-10-16 | frick-fsw | Record bounded target-deferral re-reviews 0336/0337; final applicable cycle and independent sprint blockers remain open |
| 7.5 | 2026-10-16 | frick-fsw | Record R4/PA target-deferral doc findings, corrected sprint-plan date, and pending bounded re-review/current cycle |
| 7.4 | 2026-10-16 | frick-fsw | Record owner decision to postpone physical-target tests and require disclaimer; scope R4/PA review and cycle for the changed docs |
| 7.3 | 2026-10-16 | frick-fsw | Record final local current-document cycle; preserve authority, external, target, N20 and release blockers |
| 7.2 | 2026-10-16 | frick-fsw | Record bounded closure of R4 0330 and PA 0331 findings via 0332/0333; final current-document cycle and authority/release gates remain open |
| 7.1 | 2026-10-16 | frick-fsw | Record R1/R2 UART corrective findings, characterization-only test mapping, candidate digest, user-authority block, and pending final current-doc cycle/R4/PA review |
| 7.0 | 2026-10-16 | frick-fsw | Re-audit current candidate after final scoped R4/PA corrections and fresh full local verification cycle; preserve unclosed authority and release gates |
| 6.9 | 2026-10-15 | frick-fsw | Record R4 0323 and PA 0324 bounded acceptance of current TS-2/NC-status corrections; retain broad assurance gates |
| 6.8 | 2026-10-15 | frick-fsw | Clarify NC-EXT-01/02 are not closed by V&V alone and remove unsupported UART test mapping to SRS-REQ-017 |
| 6.7 | 2026-10-15 | frick-fsw | Record bounded candidate manifest/logs and corrections to R4/PA findings; keep final reviews and authorities open |
| 6.6 | 2026-10-15 | frick-fsw | Correct NC-EXT-01/02 owner-disposition state and record PA/R4 delta-review findings |
| 6.5 | 2026-10-15 | frick-fsw | Re-audit the uncommitted S8-FU candidate, local 37-test cycle, UART V&V 0319, and current documentation-review findings/authority gates |
| 6.4 | 2026-10-15 | frick-fsw | Record corrected UART stream-framing implementation and bounds while preserving NC-EXT-07 authority/target open items |
| 6.3 | 2026-10-15 | frick-fsw | Record focused ASan review 0318 for NC-EXT-09 with exercised-offset-only scope |
| 6.2 | 2026-10-15 | frick-fsw | Record R2 T7 review 0317, exact-ping limitations, and NC-EXT-03 external E2E remaining open |
| 6.1 | 2026-10-15 | frick-fsw | Record bounded exact-reply deadline behavior and the head-of-line limitation for plausible incomplete GSW packets |
| 6.0 | 2026-10-15 | frick-fsw | Record the incremental live GSW decoder under NC-EXT-03 and retain external E2E/T7 qualification limits |
| 5.9 | 2026-10-15 | frick-fsw | Record bounded V&V closure of NC-EXT-01/02 and exact host receive-boundary evidence; retain target/backend limits |
| 5.8 | 2026-10-15 | frick-fsw | Record N22-TS1-96 as partial evidence under NC-EXT-02 and remove NC-EXT-30 from open R4/PA range after scoped closure |
| 5.7 | 2026-10-15 | frick-fsw | Record R4/PA sequence reviews 0311/0312, close NC-EXT-30/65 narrowly, and mark S8-FU current while retaining unrelated gates |
| 5.6 | 2026-10-15 | frick-fsw | Correct the final active summary that referred to an owner-selected sprint; consistently identify S8-FU as this plan's interpretation pending R4 review |
| 5.5 | 2026-10-15 | frick-fsw | Clarify the selected S8-FU order as the plan's interpretation of the owner's general instruction; retain NC-EXT-30 pending R4 review |
| 5.4 | 2026-10-15 | frick-fsw | Record owner direction to select S8-FU next, update NC-EXT-30 to await independent R4 review, and preserve authority/scope boundaries |
| 5.3 | 2026-10-15 | frick-fsw | Record bounded final R4/PA correction re-review 0310; close only the scoped wording findings and retain N7 historical/current-push and broader N17/N20 gates open |
| 5.2 | 2026-10-15 | frick-fsw | Record independent initial R4/PA reports 0308/0309 and applied corrections; keep delta re-reviews and N7 current-delta gates open |
| 5.1 | 2026-10-15 | frick-fsw | Apply scoped PA review corrections: distinguish historical pushed commit from unverified N7 pre-push compliance; state current docs delta is unreviewed/uncommitted; make N21/open-NC and README semaphore scope explicit |
| 5.0 | 2026-10-15 | frick-fsw | Reconcile N19 baseline and current gates to pushed HEAD 13ebf92 and green GitHub run 38793643786; record prior failed CI run/fix, Gitea verification limit, host CANopen guard evidence and open NC-S4c-04/sprint-precedence issues |
| 4.9 | 2026-10-15 | frick-fsw | Clarify N7: bounded current documentation reviews are complete through 0306, while overall N17, NC/authority, hosted and full-scope gates remain open |
| 4.8 | 2026-10-15 | frick-fsw | Record R4/PA timing chronology reviews 0306/0307 and update review pointers; retain all broader open gates |
| 4.7 | 2026-10-15 | frick-fsw | Record evidence chronology follow-ups 0304/0305 and update N9/N11/N17/N20 pointers without extending any technical or release claim |
| 4.6 | 2026-10-15 | frick-fsw | Extend M4 review pointer through 0303 after final R4/PA evidence deltas; keep NC closures and residual gates bounded |
| 4.5 | 2026-10-15 | frick-fsw | Record R4/PA final evidence reviews 0302/0303 and update N9/N11/N17/N20 pointers; preserve all broader open gates |
| 4.4 | 2026-10-15 | frick-fsw | Record R4 FRICK-REVIEW-0301 and update N9/N17 review scope; preserve overall N17, hosted, qualification, target/vendor and authority gates |
| 4.3 | 2026-10-15 | frick-fsw | Correct stale review-status wording after R4 review 0301; bounded reviews are complete while final R4/hosted/qualification/authority gates remain open; cite NC-EXT-82 deltas 0298..0300 |
| 4.2 | 2026-10-15 | frick-fsw | Record PA FRICK-REVIEW-0300 and update the remaining R4 scope and N9/N10/N11/N20 dispositions; preserve all nonlocal and authority gates |
| 4.1 | 2026-10-15 | frick-fsw | Record R2 tracked-state follow-up 0299, preserve the 0298 point-in-time boundary, and retain PA/R4/hosted/T23 qualification gates |
| 4.0 | 2026-10-15 | frick-fsw | Record FRICK-REVIEW-0298 and scoped closure of NC-S4c-02/NC-EXT-82; update N9/N10/N11/N15/N19 without claiming tool qualification, phase closure or release approval |
| 3.9 | 2026-10-15 | frick-fsw | Reconcile audit with completed reviews 0288..0297, final 171-check/full local cycle and retained emulator logs; preserve open provenance, dependency, hosted, target/vendor and authority NCs |
| 3.8 | 2026-10-15 | frick-fsw | Record final local verification cycle (36/36 Debug/Release/ASan, Lean, trace, format, docs, smoke, GSW, RTEMS cross-build, Renode and QEMU), exact artifact/image hashes and failed attempts; correct N1/N3/N4/N7/N8/N9/N10/N11/N15/N17/N18/N19/N20 dispositions; independent reviews and hosted CI remain pending |
| 3.7 | 2026-10-15 | frick-fsw | N19 re-audit at baseline 6ce6bd7 plus uncommitted scope: reconcile M4 S4c/S4d and NC-S4c-01, record ADR-0018 open-PUS/lwIP-on defaults and current-cycle gate status; local default host suite 36/36, remaining gates pending |
| 3.6 | 2026-10-15 | frick-fsw | Record hosted run #85 for the record-only commit `03366f1` (all 8 jobs) and adopt the record-only convention in N15 — record-only/evidence-only commits are verified by their own hosted run but not recursively re-recorded, terminating the record → run → record regress; the Gitea run log remains unevidenced (NC-EXT-36) and hosted-CI qualification evidence, T21, NC-EXT-43 and release gates remain open |
| 3.5 | 2026-10-15 | frick-fsw | Record the committed guard delta (`77cd548`) and its green hosted GitHub Actions run #84 (all 8 jobs, `documentation set` included) in the baseline/N15/N19 rows; the Gitea run log remains unevidenced (NC-EXT-36), and hosted-CI qualification evidence, T21, NC-EXT-43 and release gates remain open |
| 3.4 | 2026-10-15 | frick-fsw | Apply the scoped R4 FRICK-REVIEW-0285 and PA FRICK-REVIEW-0286 findings (complete the loop-audit currency and hosted-CI evidence anchor, extend N12 to NC-EXT-81, correct handover currency/NC ranges, split the guard into its own `CHANGELOG.md` section with the sync obligation) and record the bounded R4 delta re-review FRICK-REVIEW-0287 closing NC-01/NC-02; hosted-CI qualification evidence, T21, NC-EXT-36/43 and release gates remain open |
| 3.3 | 2026-10-15 | frick-fsw | Add the `make docs` output-existence guard in the `docs` target and the `docs`-job assertion + `doxygen --version` echo (PA FRICK-REVIEW-0284 Comment 5); bring the baseline-commit and §1 scope-boundary snapshot and the N7/N11/N15/N19 statements into line with the committed/pushed state (`03773f7`, `620fff8`; GitHub Actions run #83 green, Gitea run unevidenced), extend N12 to NC-EXT-81, and record NC-EXT-79 as the open `ci:docs` register mapping; scoped R4 FRICK-REVIEW-0285 and PA FRICK-REVIEW-0286 reviewed the delta APPROVED WITH COMMENTS; hosted-CI qualification evidence, T21, NC-EXT-43 and release gates remain open |
| 3.2 | 2026-10-15 | frick-fsw | New loop: fix the CI `docs` job failure (`make docs` on a fresh checkout; Doxygen 1.9.8 creates `OUTPUT_DIRECTORY` with a single-level `mkdir`, so `build/doxygen` cannot be created when `build/` is absent) by running `mkdir -p build/doxygen` in the `docs` target; verify on a pristine tree and record; hosted CI, T21, NC-EXT-43 and release gates remain open |
| 3.1 | 2026-10-14 | frick-fsw | Reconcile T22 audit status with local full-matrix CTest and scoped R2/R4/PA reviews; preserve hosted/immutable evidence and aggregate worktree-scope gates |
| 3.0 | 2026-10-14 | frick-fsw | Register the full T22 sweep runner in CTest and record 30/30 local CI pass; Doxygen container recovery is recorded; worktree scope and independent/hosted gates remain open |
| 2.9 | 2026-10-14 | frick-fsw | Re-audit exact dirty-tree scope (234 tracked + 120 untracked), correct R3 SRS/trace findings, clarify N10's actual CI-exercise gap for T22, and explicitly freeze further review expansion pending a new scope snapshot |
| 2.8 | 2026-10-14 | frick-fsw | Record final host/container Lean/trace, smoke, Renode, format, GSW and advisory scan results; retain RTEMS-cache/Doxygen permission limitations and independent review gates |
| 2.7 | 2026-10-14 | frick-fsw | Implement approved non-waiting queue-mutex behavior and partial fan-out; update SRS-REQ-001, TS-5/TS-6 and audit results while retaining NC-EXT-75/76 and SRS-REQ-072 profile/loss reporting open |
| 2.6 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0265/0266 R3 results, correct traceability counts, register NC-EXT-77, and preserve N14/N15/N17/N20 gates on the uncommitted final-tree delta |
| 2.5 | 2026-10-14 | frick-fsw | Draft SSS-REQ-006/SRS-REQ-072 event-arrival and dispatch envelope with authority-owned TBD values; add trace/test gaps; update active trace count to 117 and retain N14/NC-EXT-75/76 gates |
| 2.4 | 2026-10-14 | frick-fsw | User-approved bounded-failure correction: add PAL try-lock, pre-mutation SB timeout, EVS deferred lock timeout, synchronized contention tests, and update exact cycle evidence; preserve SRS/SSS authority boundary and NC-EXT-75/76 open |
| 2.3 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0264 and NC-EXT-76; update current NC range and preserve the unresolved no-block/SRS-REQ-011 conflict as functional work |
| 2.2 | 2026-10-14 | frick-fsw | Update current-cycle evidence for scoped API behavior; document RTEMS cache limitation and leave nonblocking-claim disposition/NC-EXT-75 open |
| 2.1 | 2026-10-14 | frick-fsw | Scope deferred delivery to explicit `FF_EvsSendDeferred` for SB overflow; restore synchronous ordinary sends; retain strict mutex/nonblocking ambiguity as open NC-EXT-75 evidence |
| 2.0 | 2026-10-14 | frick-fsw | Correct the R1-identified synchronous EVS callback defect with bounded deferred TM-output dispatch; add re-entry and queue-overflow evidence; keep NC-EXT-75 open pending independent delta review |
| 1.9 | 2026-10-14 | frick-fsw | Add FRICK-REVIEW-0263 and NC-EXT-75; update NC range and record that functional correction awaits independent delta review |
| 1.8 | 2026-10-14 | frick-fsw | Audit SB overflow behavior against FRICK-ARCH-0002, record the safe EVS correction/regression and exact cycle gate status; preserve NC-EXT-47 and review gates as open |
| 1.7 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0262 and NC-EXT-69..74; reflect scoped R4 result without claiming overall N17/N20 closure |
| 1.6 | 2026-10-14 | frick-fsw | Update N7/N21/N22 audit state, record FRICK-REVIEW-0260/0261 and NC-EXT-48..68, and preserve review/verification gates as open |
| 1.5 | 2026-10-14 | frick-fsw | Record N22 and track existing test-oracle/coverage assessment as NC-EXT-47 without stopping unrelated development |
| 1.4 | 2026-10-14 | frick-fsw | Record non-derogable N21 and track non-functional current-status findings as NC-EXT-46 without stopping development |
| 1.3 | 2026-10-14 | frick-fsw | Align current audit revision to the latest bounded R4/PA review cycle; no overall approval or NC closure |
| 1.2 | 2026-10-14 | frick-fsw | Record bounded R4/PA no-new-finding re-reviews 0258/0259; keep all applicable gates open |
| 1.1 | 2026-10-14 | frick-fsw | Record 0256/0257 status follow-ups and author corrections; no NC closure or release approval |
| 1.0 | 2026-10-14 | frick-fsw | Record 0254/0255 author dispositions and current pending re-reviews; preserve all NCs open |
| 0.99 | 2026-10-14 | frick-fsw | Record R4/PA follow-up findings 0254/0255; distinguish reported metadata discrepancy from confirmed evidence; all gates remain open |
| 0.98 | 2026-10-14 | frick-fsw | Record R4 0252 and PA 0253 status findings, corrected NC-EXT-43 scope and active metadata; retain all gates open |
| 0.97 | 2026-10-14 | frick-fsw | Record bounded R2 0249 pass and R4/PA 0250/0251 scope findings; reconcile active scope wording without closing NCs |
| 0.96 | 2026-10-14 | frick-fsw | Record owner authorization for original project content; re-scope NC-EXT-43; add PA 0248 unreadable-file finding under NC-EXT-44 |
| 0.95 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0247 scoped DDJF pass; overall N17/N20 and NC closure remain open |
| 0.94 | 2026-10-13 | frick-fsw | Record author correction for DDJF T1..T21 and pending independent R4/PA review; retain NC-EXT-45 open |
| 0.93 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0246 DDJF range discrepancy and NC-EXT-45; retain all applicable gates open |
| 0.92 | 2026-10-13 | frick-fsw | Record T21 SVG finding/correction and 0245 bounded result; refresh T21/NC-EXT-44 open status |
| 0.91 | 2026-10-13 | frick-fsw | Record T21 parser review sequence 0238..0242; 0242 bounded no-new-finding only; retain NC-EXT-44 and T21 qualification open |
| 0.90 | 2026-10-13 | frick-fsw | Record R2/R4/PA findings 0235..0237, add NC-EXT-44, and retain N10/N12/N13/N17/N19/N20 open |
| 0.89 | 2026-10-13 | frick-fsw | Start user-prioritized MPL-2.0 transition; audit actual candidate against N1-N20, identify NC-EXT-43, keep N13/N19/N20 open |
| 0.88 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0232 scoped handover pass; overall N17 remains open |
| 0.87 | 2026-10-13 | frick-fsw | Record scoped 0229..0231 outcomes and retain overall N10/N17/N20 gates open |
| 0.86 | 2026-10-13 | frick-fsw | Update N10 latest T13 review pointer from 0227 to 0228; no requalification or NC closure implied |
| 0.85 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0228 and current test-count/review-summary correction |
| 0.84 | 2026-10-13 | frick-fsw | Update handover and plan pointers after 0227; retain N10/N17/N20 open |
| 0.83 | 2026-10-13 | frick-fsw | Record latest R2 finding 0227 and make final NC summary/panel language corrections pending review |
| 0.82 | 2026-10-13 | frick-fsw | Reconcile latest T13 report IDs and handover/sprint pointers after 0224..0226 corrections |
| 0.81 | 2026-10-13 | frick-fsw | Update status pointer to sprint-plan rev 0.71; retain open NC-EXT-41/42 |
| 0.80 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0224..0226 and preserve open N17/N20 status |
| 0.79 | 2026-10-13 | frick-fsw | Reconcile current review and NC-EXT-42 status summary after T13 classifier/panel corrections |
| 0.78 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0221..0223, add NC-EXT-42, and retain R4/PA/N10 gates open |
| 0.77 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0218/0219; distinguish scoped PA pass from open N20 gate |
| 0.76 | 2026-10-13 | frick-fsw | Update handover pointer to sprint-plan rev 0.64; preserve pending review status |
| 0.75 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0216/0217; retain V&V ledger and NC-EXT-41 review gates open |
| 0.74 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0214/0215 and current corrections pending independent review |
| 0.73 | 2026-10-13 | frick-fsw | Reconcile handover pointer to sprint-plan rev 0.60; keep review findings open |
| 0.72 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0212/0213; reconcile active NC range and guide revision metadata |
| 0.71 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0210/0211 and NC-EXT-41; retain N17/N20 open |
| 0.70 | 2026-10-13 | frick-fsw | Record corrected CONTRIBUTING/driver/ADR scope wording after 0208/0209; retain NC-EXT-38 open |
| 0.69 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0206/0207; retain all NCs and current R4/PA gates open |
| 0.68 | 2026-10-13 | frick-fsw | Correct handover plan pointer to rev 0.56 and align audit NC range to 40 |
| 0.67 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0204/0205 and latest open NC-EXT-38/39/40 status |
| 0.66 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0202/0203 and NC-EXT-38; latest docs corrections remain pending independent review |
| 0.65 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0200/0201; active wording corrections await independent re-review |
| 0.64 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0198/0199 and latest active T1 wording corrections; NC-EXT-35 remains open |
| 0.63 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0195..0197 scoped outcomes; N17/N20 and NC-EXT-36 remain open |
| 0.62 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0194 as latest R4 result; NC-EXT-36 remains open pending hosted verification |
| 0.61 | 2026-10-13 | frick-fsw | Correct audit header/history revision and explicitly distinguish R2 NC-EXT-35/36 attribution |
| 0.60 | 2026-10-13 | frick-fsw | Incorporate R4/PA/R2 reports 0191..0193; retain NC-EXT-35..37 and T1 status gates open |
| 0.59 | 2026-10-13 | frick-fsw | Reconcile audit header with History and clarify NC-EXT-34 re-review scope after R4 finding |
| 0.58 | 2026-10-13 | frick-fsw | Reconcile loop-audit wording per PA-0189 while keeping NC-EXT-34 open pending fresh independent disposition |
| 0.57 | 2026-10-13 | frick-fsw | Record T1 requalification gaps NC-EXT-35/36 and add them to N10/N12 audit |
| 0.56 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0188/0189; scoped results do not close NC-EXT-34 or overall N17/N20 |
| 0.55 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0186/0187; scoped PA PASS does not close phase gates or NC-EXT-34 |
| 0.54 | 2026-10-13 | frick-fsw | Record scoped R4 approval/comments, PA FAIL, and NC-EXT-34 V&V history correction; gates remain open |
| 0.53 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0182/0183 findings; retain ICD, evidence identity and review gates open |
| 0.52 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0180/0181 and latest R4/PA findings; keep all relevant gates and NCs open |
| 0.51 | 2026-10-13 | frick-fsw | Open NC-EXT-33 for missing independent R3 review of SSS-REQ-030; correct N14 status and N12 closure criteria |
| 0.50 | 2026-10-13 | frick-fsw | Align live summaries with proposed-not-approved sprint order and N14 review publication gates; NC-EXT-30/32 remain open |
| 0.49 | 2026-10-13 | frick-fsw | Capture R4 diagram/sequence-count, stale plan pointer, sprint owner-order and review-gate wording corrections; retain affected NCs open |
| 0.48 | 2026-10-13 | frick-fsw | Record cross-repository N11 clarification in AGENTS/rules and keep NC-EXT-32 open pending re-review |
| 0.47 | 2026-10-13 | frick-fsw | Expand NC-EXT-26/27/32 corrective scope to include offsets, source-comment path and publication policy; keep findings open |
| 0.46 | 2026-10-13 | frick-fsw | Record latest R4 verdict and pending independent verification of NC-EXT-26/27 corrections |
| 0.45 | 2026-10-13 | frick-fsw | Record author corrections to ICD/PUS API documentation; NC-EXT-26/27 remain open pending independent review |
| 0.44 | 2026-10-13 | frick-fsw | Align N15 with passing pinned gates and yellow status pending NC-EXT-29 evidence identity |
| 0.43 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0178/0179 verdicts and update N17/N20 to latest rejected/failed review states |
| 0.42 | 2026-10-13 | frick-fsw | Open NC-EXT-32 for review-record publication wording; conservatively set verification semaphores yellow pending complete immutable evidence identity |
| 0.41 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0176/0177 verdicts; update N17/N20 to their latest failed reviews and retain all applicable gates open |
| 0.40 | 2026-10-13 | frick-fsw | Update N17 to identify FRICK-REVIEW-0174 as the latest rejected R4 review |
| 0.39 | 2026-10-13 | frick-fsw | Clarify N20 status from the two current PA FAIL results; retain review gate open |
| 0.38 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0174/0175 and qualify NC-EXT-13 as an authority-owned policy issue pending approved acceptance criteria and severity rationale |
| 0.37 | 2026-10-13 | frick-fsw | Incorporate latest R4/PA review verdicts; track status, evidence provenance, sprint-order and severity rationale gaps as NC-EXT-28..31 |
| 0.36 | 2026-10-13 | frick-fsw | Record D-01/D-02 documentation mismatches as NC-EXT-26/27; confirm TC header length against source constants and current ICD text |
| 0.35 | 2026-10-13 | frick-fsw | Add S8-VV-FU for GAP-R3-1 and NC-EXT-25 bounded test/fuzz work; align M17 execution order and retain all findings open pending evidence/review |
| 0.34 | 2026-10-13 | frick-fsw | Convert current-tree assessment findings C-04..C-16 into NC-EXT-14..24; map existing assurance gaps to GAP-A1..A14 without duplicate NCs; schedule autonomous PUS follow-up and distinguish PUS S11 from M17 scheduler |
| 0.33 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0173 limited parser re-review, NC-EXT-13 primary-header policy finding, and corresponding status/evidence updates. |
| 0.32 | 2026-10-13 | frick-fsw | Add C-03/NC-EXT-13 after independent R1/R2 found unsupported primary-header fields accepted without controlled profile; record FRICK-REVIEW-0173 limited GSW parser re-review and update current NC status. |
| 0.31 | 2026-10-13 | frick-fsw | Incorporate final R1/R2, R4 and PA review results: NC-EXT-11/12 added; N5 target-vs-assignment clarified; N7/N8/N9/N10/N11/N15/N17/N20 reflect open/uncommitted review state; correct stale sprint, handover, V&V, GAP-A5, README and attribution claims. |
| 0.30 | 2026-10-13 | frick-fsw | Delta-review cross-check: confirm S4b follow-up findings are still open until reports exist; correct N12/semaphore status, add S8-FU sprint mapping and disclose current GSW stream-offset fix and symmetric decoder-test evidence. |
| 0.29 | 2026-10-13 | frick-fsw | Follow-up audit reconciles status with actual review evidence: N12 remains open; no FRICK-REVIEW-0167-D1/0168-D1/0169-D1 records exist yet; NC-S4b-06..09 and NC-EXT-01..13 remain open pending delta review/authority disposition. Assessment follow-up code changes, GSW offset fix, test additions, sanitizer fixture correction, current verification record and handover updated; local Renode image now built through RTEMS Docker toolchain and functional run PASS. |
| 0.28 | 2026-10-13 | frick-fsw | Independent assessment cross-check in new §7: verify high-risk claims against the current tree; open NC-EXT-01..08 and NC-S4b-09 with sprint/owner dispositions; implement decoder bounds, max TC ingest buffer, GSW wire-format vectors, UART smoke generator and status-interpretation corrections; list unresolved requirement/profile/concurrency/framing matters without inventing project policy. |
| 0.27 | 2026-10-13 | frick-fsw | Second-pass N19 audit identified NC-S4b-06..08 and applied documentation corrections. These NCs remain OPEN pending the independent delta review; the immutable point-in-time counts in review reports are not a deviation. |
| 0.26 | 2026-10-13 | frick-fsw | S4b follow-up author corrections were applied to the initial R1+R2/R4/PA findings; the reports FRICK-REVIEW-0167/0168/0169 remain APPROVED WITH COMMENTS and their follow-up tables say delta re-review pending. The initial records do not establish N12/N17/N20 closure. |
| 0.25 | 2026-10-13 | frick-fsw | **S4b follow-up (N19 conformance audit):** five S4b deviations opened and closed — NC-S4b-01 (GAP-A5 omitted the IP/TSN on-target residual), NC-S4b-02 (**Major**: `pus_ip`/`pus_tsn` exit criterion unevidenced; `Dem` method of SRS-REQ-060/061 unmet), NC-S4b-03 (stale suite/check counts + `Verfied` typo + stale headers), NC-S4b-04 (`traceability.md` SRS-REQ-059 mirror), NC-S4b-05 (stale M4 pending/remaining lists here and in `verification-results.md`). Added the host PUS-C TM/TC round-trip demonstrations (`test_pus_ip_roundtrip`, `test_pus_tsn_roundtrip`); ctest 27/27; counts unchanged 120/76/30/14/68 |
| 0.24 | 2026-10-13 | frick-fsw | **S4b delta re-review (N12 closure):** all findings from FRICK-REVIEW-0165 (R1+R2 REJECTED) and FRICK-REVIEW-0166 (PA REJECTED) resolved — 2 Major (R1-01 seg-layer reassembly desync, R1-02 TSN ISR mutex), 10 Minor (R1-03–12), 6 V&V gaps (R2-01–06), 4 PA Minor (PA-MN-01–04), 3 Comments; 3 new findings raised and closed (NF-1 unused variable, NF-2 Major buffer overflow in `rx_partial`, NF-3 doc omission). Delta re-reviews: R1+R2 (FRICK-REVIEW-0165-D1) APPROVED; PA (FRICK-REVIEW-0166-D1) APPROVED. All gates green: build + ctest 27/27 + trace + Lean + smoke + GSW + `-Wall -Wextra -Werror` + clang-format clean |
| 0.23 | 2026-10-13 | frick-fsw | **S4b delivered:** PUS-C over IP/TSN (SRS-REQ-059/060/061) — seg/reassembly layer (`link_seg.{c,h}`, 2-byte BE length-prefix, allocation-free), IP backend (`link_ip.{c,h}`, UDP + TCP) over `FF_NetStack_t`, TSN backend (`link_tsn.{c,h}`, EtherType 0x88F5) over `FF_EthMac_t + FF_TsnBackend_t`; ctest 27/27 + trace + Lean + smoke + GSW PASS; Renode SKIP (host-only code). M4 row: restored scope PUS-C over IP/TSN row updated from "pending" to "delivered (S4b)"; on-target IP/TSN link-level = GAP-A5 (Renode GEM DMA register-only) |
| 0.22 | 2026-10-13 | frick-fsw | **S8/M8 delivered:** gPTP stack (`src/drivers/eth/gptp/ff_gptp.{c,h}`, BMCA/Sync/Follow_Up/PDelay/PI servo, 58 host checks) + 802.1Qci via GEM screener type 1 (`set_stream_filter`, 22 test_gem_tsn checks); 802.1Qav CBS + 802.1Qbb PFC = `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02); fabric CoreTSN deferred to M14; integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160) corrected; traceability + SRS updated for SRS-REQ-066/068 |
| 0.21 | 2026-10-12 | frick-fsw | NET follow-up: NC-S6-03 co-implemented (GEM 64-bit `gem_phys`/`gem_wr_qbqp`/`TBQP_HI`/`RBQP_HI`/`DMACFG.ADDR64`; per-descriptor 32-bit truncation explicit; full 3-word descriptor format deferred to M14); NC-154-01 closed (`test_lwip_netif` 26/26 host); `test_gem` extended for 64-bit register writes; Renode register-only GEM confirmed (NC-S6-01: mock-MAC loopback host test) |
| 0.20 | 2026-10-12 | frick-fsw | Sprint 7: PCIe peripheral-bound (`hal_pcie=PASS`, SRS-REQ-031/032) and SpaceWire contract-level (`hal_spacewire=CONTRACT`, SRS-REQ-032); M11 11/13 peripheral-bound; NC-S7-01 recorded (SpaceWire no emulated peripheral) |
| 0.19 | 2026-10-12 | frick-fsw | Sprint 6b-2: RTEMS object sizing wired (NC-S6-02 closed), lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS`); NC-151-02/05 closed; NC-S6-03 opened (latent GEM 64-bit descriptor DMA) |
| 0.18 | 2026-10-12 | frick-fsw | Sprint 6b-1: in-repo lwIP RTEMS sys_arch port (SRS-REQ-040); host 23/23 with FF_USE_LWIP=ON, RTEMS build produces the lwIP archive (forced-extraction link resolves the stack); NC-S6-02 (RTEMS object sizing) + NC-S7-01 (SpaceWire no model) recorded |
| 0.17 | 2026-10-12 | frick-fsw | Sprint 6a: GEM native TSN backend exercised peripheral-bound (`hal_tsn=PASS`, M11 10/13; SRS-REQ-038); NC-S6-01 opened (NET lwIP-on-GEM peripheral-bound deferred to sprint S6b) |
| 0.16 | 2026-10-12 | frick-fsw | Sprint 5: reference MSS block-device backend (SRS-REQ-070) delivered; M11 9/13 peripheral-bound (`hal_block=PASS`); counts 120/76/30/14/68 |
| 0.15 | 2026-10-12 | frick-fsw | Sprint 4: reference MSS SPI backend (SRS-REQ-069) delivered; M11 8/13 peripheral-bound (`hal_spi=PASS`); counts 113/70/29/14/51 |
| 0.14 | 2026-10-12 | frick-fsw | N19 audit: restored the M4 scopes left as prose "absorbed" notes as explicit deliverables (PUS-C over IP/TSN, CAN-FD/CANopen/RS-485 DE, PUSopen adapter; fabric contract; M8 TSN Qav/Qci/Qbb + gPTP/CoreTSN); counts 112/69/29/14/51 — FRICK-REVIEW-0137 |
| 0.13 | 2026-10-12 | frick-fsw | Sprint-3 review set closed (FRICK-REVIEW-0129..0136: R1/R2/R3/R4/PA initial + R1/R2 delta + PA delta; PA REJECTED -> APPROVED WITH COMMENTS); user-facing TCP/IP + TSN sections and the expanded framework description (FRICK-ENG-0007) added; T14 reproducible via `make misra-docker` |
| 0.12 | 2026-10-12 | frick-fsw | Sprint 3: reference MSS I2C backend (SRS-REQ-058) delivered; M11 7/13 peripheral-bound (`hal_i2c=PASS`); counts 100/59/27/14/51; `make renode` PASS on the Sprint-3 image |
| 0.11 | 2026-10-12 | frick-fsw | M10 review NCs (FRICK-REVIEW-0120..0123): N10 registry T1-T20; QEMU gap rows closed; QEMU run recorded |
| 0.10 | 2026-10-11 | frick-fsw | M10 closed: QEMU generic-BSP emulated-target run (`make qemu`, T20); M11 6/13; counts 99/58/27/14 |
| 0.9 | 2026-10-11 | frick-fsw | M4/M11: MSS CAN done (SRS-REQ-057); M11 6/13; counts 99/58/27/14; re-baselined to the CAN increment |
| 0.8 | 2026-10-11 | frick-fsw | Renode V&V environment switched to the BeagleV-Fire board repl (QSPI NOR + system services reachable) |
| 0.7 | 2026-10-11 | frick-fsw | NOR/QSPI MTD driver (SRS-REQ-056) host-verified; counts 98/57/27/14; on-SoC QSPI task opened |
| 0.6 | 2026-10-11 | frick-fsw | M11 5/13 + NVM done (SRS-REQ-055); counts 97/56/27/14; PUS-C ASN.1 row marked reconciled |
| 0.5 | 2026-10-11 | frick-fsw | Link the sprint plan (FRICK-PLAN-0002); track the ECSS traceability NC PA-67-01 and the M16 review NCs in the dispositions summary |
| 0.4 | 2026-10-11 | frick-fsw | Add the M16 objective row (in-house OBCP engine development) |
| 0.3 | 2026-10-11 | frick-fsw | N19 reconciliation: add the M15 objective row, refresh M14 wording, correct the M11 pending-class count (9) to match development-plan/roadmap |
| 0.2 | 2026-10-11 | frick-fsw | Re-baselined at e1def89: counts 94/54/26/14/51, registry T1-T19, N9 row + PA, N20 in progress, R1-02 residual tracked (PA-review loop) |
| 0.1 | 2026-10-11 | frick-fsw | First N19 loop audit: objective/rule conformance; PUS-C ASN.1 gap root cause + reconciliation; multicore M14 |


## 11. Current-loop conformance addendum — S8-PUS-FU S1 ingestion failures (2026-11-12)

**Audited baseline:** committed `e5f01d4` plus the current uncommitted test and documentation delta. The scope is one host test increment; no production C source changed. The current run is recorded in `docs/vv/verification-results.md` and evidence package `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/`; its package-level manifest is currently stale and not verified. The sprint plan remains in progress; this increment does not complete S8-PUS-FU, M17 or any release/qualification phase.

### 11.1 Objectives / milestone delta

| Objective | Actual candidate | Gap / disposition |
| --- | --- | --- |
| S8-PUS-FU Service 1 failure-path verification | Host test characterizes the illegal-APID code/payload behavior recorded in `N22-TS1-23` (`gap` pending NC-EXT-13); unknown-service/subservice branches are additionally checked at payload-octet level. Ack-zero branches are characterization-only. | NC-EXT-15 remains open; profile, requirement and interface authorities remain open. `N22-TS1-23` and `N22-TS1-104` are `gap` cases, not covered. |
| Test-registration integrity | Independent PA/R4 reviews identified `test_pus_s11_residual` declared in `FF_TEST_TARGETS`, not registered with CTest, and failing 10/1164 when invoked directly. | NC-EXT-240 OPEN, owner FSW, M17; not repaired in this increment. CTest 57/57 is registered tests only. |
| S1 verification payload authority | ICD §4 does not define the implemented 9-octet S1 payload layout or provisional destination identifier. | NC-EXT-241 OPEN, owner interface authority, unscheduled pending authority decision. No ICD change or ECSS clause-conformance claim. |
| Related independent-review comments | TS-1 ordering and `s_sequence_count` synchronization findings are recorded. | NC-EXT-242 and NC-EXT-243 OPEN/deferred under N21; no production change in scope. |

### 11.2 Norms audit N1-N22 against the candidate

| Norm | Current-loop evidence / disposition |
| --- | --- |
| N1 | No generated code changed. `lake build` PASS and 11-module Lean elaboration receipt validated in the r4 evidence package. |
| N2 | Changed product docs/comments are English; R4 correction review is CHANGES REQUIRED on the reviewed snapshot and its bounded delta resolved the N12/N14 comment, while the package-manifest Major remains open. |
| N3 | No SRS/SSS requirement changed. Traceability attribution for SRS-REQ-017 includes the PUS implementation/test as behavior characterization; the human-readable `traceability.md` mirror is synchronized in rev 0.54. The acceptance-policy basis remains open under NC-EXT-13. |
| N4 | `trace.py` PASS: 120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean links. |
| N5 | Class B remains the project target; no mission assignment or class-A closure claimed. |
| N6 | Local build/test/formal/trace/format/license and N18 legs are recorded. Dual hosted CI is not run for the uncommitted candidate; CI/CM and supply-chain gaps remain. |
| N7 | No commit/push performed. Designated FSW-agent authority and all gates remain required; full sprint scope is not complete. |
| N8 | TS-1, T3 register, verification results, sprint plan, machine-readable and human-readable traceability, review index, handover, and evidence package are updated. r3 README is explicitly superseded/unmanifested; r4 is the intended package. R4 delta resolved DOC-R4-RR-02 on its reviewed snapshot. DOC-R4-FINAL-01 and later R4-AUDIT-01 are recorded with their bounded dispositions; the 157-entry preview and 41-path outside-list inventory are historical. A prior f07c2ca4… snapshot had matching PA/R2/R4 reports (297/297), but subsequent covered status-document changes invalidated that binding. Current status must be established from the active manifest and three matching reports; this audit does not assume they match. |
| N9 | R1/R3 correction re-review passes with comments; both sync observations separately confirmed PASS. PA/R2/R4 independently confirmed the original preview for its snapshot, not final integrity. R2-FINAL-INV-02 was resolved for the refreshed 11-path identity/patch at its review snapshot; subsequent tracked-document edits require a current identity rebind and check. The package-manifest review findings remain scoped to their reports; post-manifest attestations record integrity only and are not substitutes for substantive review or N10/N22 evidence. |
| N10 | T3 `test_pus.c` hash is re-bound; remains **NOT FULLY QUALIFIED** and this exact candidate has no N10 qualification review. |
| N11 | Ordered gate logs, mutation controls, interruption record and review reports are retained. The S1 eleven-path working-tree identity, exact focused test and Renode ELF, plus 55 distinct CTest executables/invocation inventory, are included in r4; the historical per-binary check was 55/55. PA/R2/R4 checked successive preview/manifest snapshots; all superseded digests remain historical. The former 157-entry preview and 41-path outside list are not the current inventory. The prior f07c2ca4… snapshot had PA/R2/R4 inventory attestations (297/297), but covered status-document changes invalidated them. Current matching reports bind inventory/check evidence only, not substantive approval or NC closure. Any covered-file change requires a new freeze/rebind. |
| N12 | SYS-R1R3-01 is resolved in substance by reclassifying N22-TS1-23 as a gap/characterization; the correction review is PASS WITH COMMENTS and both documentation-sync comments are separately confirmed PASS. Fresh PA/R2/R4 reports have additional findings that remain open for correction/re-review. NC-EXT-240..244 remain OPEN; no NC closure is claimed. |
| N13 | Authored-file license checker PASS; third-party/generated boundaries NC-EXT-43 remain open. |
| N14 | No SSS change. Avionics R1/R3 correction re-review SYS-R1R3-20261113-PUS-S1-INGEST-RR passes with comments; SYS-R1R3-01 is resolved in substance. Two Minor documentation-sync observations were addressed and independently confirmed PASS in SYS-R1R3-20261113-PUS-S1-INGEST-SYNC-CONFIRMATION. |
| N15 | Fresh configure/build/CTest on isolated host build dir `build/doc-update`: configure exit 0, build exit 0 (0 warnings), full CTest 60/60 PASS, focused PUS tests 7/7 PASS (`test_pus`, `test_pus_s11_residual` checks=1176/0, `decoder_corpus`, `pus_event_identity`), `make verify-lean` PASS, `make trace` PASS (120/76/30/14/68), changed-file pinned clang-format 18 PASS (`src/pus/pus.c`, `tests/unit/test_pus_s11_residual.c`). `make smoke` ATTEMPTED and FAILED (CMake path mismatch: `build/native` cache bound to `/workspace` from prior Docker run, exit 2); no cache removed, no Docker workaround attempted — smoke NOT established. `make gsw-selfcheck` ATTEMPTED and FAILED (exit 2) on pre-existing third-party file `tools/patches/n7s-obcp/src/modifications/py/vm.c` missing MPL-2.0 SPDX header — unchanged from HEAD, outside this delta; GSW NOT established. N18 Renode PASS retained from prior run (`RENODE_TEST: PASS`, container exit 0); artifacts copied to `docs/vv/evidence/2026-11-14-s1-progress-failure/`. Raw logs and exit codes retained under `docs/vv/evidence/2026-11-14-s1-progress-failure/`. NC-EXT-244 OPEN/deferred under N21 (unchanged initializer files only). Dual CI and remaining plan/release checks are not claimed. **Not all gates passed; no gate is claimed unless evidenced.** |
| N16 | Project CHANGELOG and affected document histories are updated: sprint-plan rev 1.152, traceability.md rev 0.56, handover rev 3.92, loop-audit rev 10.88; TS-1 rev 1.13 and ICD rev 1.8. Current R2/R4 findings and final manifest rebind remain in progress; independent reviews for this increment are pending. |
| N17 | DOC-R4-RR-02 resolved on its reviewed snapshot; DOC-R4-FINAL-01 and its preview delta are snapshot-scoped; the R4 post-manifest attestation is designated to record the final inventory/digest check and does not substitute for N17 content review. R2-FINAL-INV-02 was resolved for an 11-path identity snapshot; refresh/recheck after controlled-document changes. Substantive reports belong in the manifest; the checksum attestation is distinct from N17 content review. |
| N18 | Fresh RTEMS image built with Docker toolchain; ELF SHA-256 `41acb18583ac25a6deb671274e1400fe4db8b84f6c105958f22959db0660cb20`, mtime 2026-11-13. Fresh `make renode` PASS (`RENODE_TEST: PASS`, container exit 0); raw run logs retained in `docs/vv/evidence/2026-11-13-icd-s3-s6-contract/renode-run/`. Emulator only; no RTPFS160ZT/HIL/timing claim. The earlier stale-ELF run is explicitly excluded. |
| N19 | This addendum audits the current objective and each norm; unresolved gaps have owners/milestones or authority dependencies. NC-EXT-244 was added after the pinned formatter check. **N19 sequencing deviation (recorded, not claimed passed):** the audit header and N1-N22 rows were refreshed at 2026-11-14 after the first source edit (`src/pus/pus.c` progress-failure move) had already been applied to the working tree; the pre-edit clean-baseline state was not audited first as N19 requires ("at the start of every development/verification loop"). `fc68b9d` is the clean committed baseline (ICD documentation only); the S1 source/test edits are uncommitted working-tree state on top of it. **Gap owner:** FSW engineer. **Milestone:** next N19 loop start, before any further source change. **Disposition:** OPEN — tracked as NC-EXT-245, open to system, non-blocking under N21. |
| N20 | **UNVERIFIED for this cycle** — independent PA/N20 review not obtained in this session (restricted to this session only; no nested Hermes/Jeeves). Prior digests/attestations are historical after covered status-document changes. Inventory evidence is not product N20 approval, risk acceptance, or release approval. N20 not passed; no closure claimed. |
| N21 | The increment adds the candidate S1 failure-path test; nonfunctional registration/interface/concurrency/documentation findings are recorded as NCs and are not silently closed. No authority-owned policy invented. |
| N22 | New test `test_s1_progress_fail_regression` compares explicit expected results for out-of-bound handler-failure inputs (malformed TC[9,1], nominal TC[9,1], no-completion-ack) through `FF_PusIngest`. S1 progress-failure regression asserts exactly one TM[1,6], zero TM[1,5], code `FF_PUS_FAIL_EXECUTION`, unchanged TM[1,8]; nominal case asserts TM[1,5]; no-completion-ack asserts no TM[1,8]. TS-1 rev 1.14 adds N22-TS1-105 (105 cases = 76 covered + 29 gap). Both the unresolved illegal-APID acceptance policy and ack-zero behavior remain characterization only and counted as `gap`; N22-TS1-36 and N22-TS1-44 remain gap rows with no retained automated check. |

**Disposition:** current-loop gates and focused reviews are evidence for this bounded candidate only. NC-EXT-240..244 remain OPEN; no NC, sprint, qualification, safety, release or flight decision is closed or approved. The remaining sprint-plan work and all listed external authority inputs remain in progress/deferred as recorded; do not declare the sprint or session complete.

**N19 sequencing gap (historical S1 loop):** the earlier N19 audit was refreshed after its first source edit; that deviation remains historical and is not retroactively passed.

## 16. Current development loop — NC-S9-01 authority decision package (2026-11-16)

### 16.1 Objective and disposition

Prepare an authority-facing package after the owner accepted the proposal to retain S18 as provisional/default-off, treat host/emulator evidence as limited, and seek written N7 license disposition. The package is `docs/status/nc-s9-01-authority-decision-package.md` rev 0.2. It requests reconciliation of the sprint-plan's recorded owner Route A selection with the comparison study's non-adoption status, without overturning or extending any decision; a mission PUS-profile decision for the current S18 proposal or replacement; target/HIL availability and evidence owners; and artifact-specific license/provenance/distribution decisions. It records current facts and explicit non-claims. **No new authority response reconciling Route A, S18 interface approval, legal conclusion, target access, qualification, review, or NC closure is recorded.** The existing sprint-plan Route A owner-selection statement remains in force as a recorded statement but its scope/reconciliation is unresolved. NC-S9-01 and NC-EXT-43 remain open.

### 16.2 N1-N22 disposition for this documentation loop

| Norm | Status / gap |
| --- | --- |
| N1 | No generated code or source artifact changed; existing Lean results are historical and not rerun for this documentation-only package. |
| N2 | Package and status updates are English. |
| N3/N4 | No requirements or traceability manifest changed in this loop; no new requirement coverage is claimed. |
| N5 | Class-B intent unchanged; target and mission gaps remain open. |
| N6 | No CI, Docker, or emulator run for this package. |
| N7 | No commit/push. |
| N8 | Added authority package and updated sprint plan, handover, loop-audit, and revision histories; no architecture/interface/mission baseline approval inferred. |
| N9 | Independent reviews of this package and affected documentation are not obtained; remain pending where applicable. |
| N10 | No V&V tool change or qualification claim. |
| N11 | No execution evidence generated; retain the current working-tree documents and their revision records. |
| N12 | NC-S9-01 and NC-EXT-43 remain open; no NC closure. |
| N13 | Package requests license authority evidence; no legal or compatibility conclusion. |
| N14 | No systems-authority decision received. |
| N15 | This documentation increment has only a local whitespace/diff check; no full project gate claimed. |
| N16 | Package revision history and handover/loop-audit histories updated. |
| N17 | Independent documentation review pending; this draft is not approved. |
| N18 | No Renode/target/HIL run; none claimed. |
| N19 | This section records the current objective and its gaps; the baseline worktree was already dirty, preserved, and is not represented as clean. |
| N20 | Independent PA review pending; no product-assurance approval claimed. |
| N21 | No functional defect or feature change in scope; authority and assurance gaps remain visible and do not imply closure. |
| N22 | No test design or coverage claim changed. |

**Disposition:** decision package prepared for authority response only. The engine option, mission S18 interface, target/HIL availability, and license/distribution disposition remain unresolved. Follow-up depends on authorized responses; retain the existing default-off/provisional boundary and all open NCs.

### 16.3 Revision history

| Revision | Date | Author | Change |
| --- | --- | --- | --- |
| 11.15 | 2026-11-16 | frick-fsw | Record current `main`/HEAD and pre-existing dirty baseline; audit N1-N22 for the NC-S9-01 authority-package loop; link draft rev0.2 reconciling the sprint-plan Route A owner-selection record with the study's non-adoption status; retain all authority, target, license, review and qualification gaps. |

## 17. Current development loop — record owner OBCP direction (2026-11-17)

### 17.1 Objective and baseline

Record the owner's explicit selection of Route A and acceptance of the S18 provisional/default-off boundary and conditional interim evidence plan. Branch `main`, HEAD `1ac2347d1f8e04a665c35c5f29134d3148efd66d`; the worktree had 54 status entries at this loop start and remains intentionally dirty. No cleanup or unrelated-content changes. Pre-edit SHA-256 snapshots for this loop: handover `f41e3b22276a507d7fa5c5f4645c3c4a0295442e1f54b08215bb9d48deb0289d`; loop audit `704237a2448d21bbebc7ada29e8d4a692c459b5e690b50f13afd001490c2c43a`; sprint plan `4ea008fb9e9c6d0b12846f69bf8bacd6ae9da572485bcca07e90e51d0cfaeb7f`; package at loop start `efd753b0255a9d76af2948e1ce43edfe14fe24b41463d8d6c7812946f28d8352`; licensing `18d866102d7b3151a035784a6e65d5e7c4a164ff0a0cb149c5966beff0894048`. These are pre-edit snapshots, not post-edit integrity attestations.

### 17.2 N1-N22 disposition for this documentation/status increment

| Norm | Status / gap |
| --- | --- |
| N1 | No generated code or source changed; no new Lean artifact or result. |
| N2 | Controlled records remain English. |
| N3/N4 | No requirement or traceability change; no new coverage claim. |
| N5 | Class-B intent unchanged; target, mission, assurance and license gaps remain open. |
| N6 | No CI, Docker, or emulator executed. |
| N7 | No commit or push. |
| N8 | Updated decision package, sprint plan, handover, and this audit with matching revision histories. |
| N9 | Independent reviews of this delta/package remain pending; no approval claimed. |
| N10 | No V&V tool changed or qualified. |
| N11 | No new test evidence generated; retain source workspace and documented revisions. |
| N12 | NC-S9-01 and NC-EXT-43 remain open; no NC closure. |
| N13 | MIT and MPL-2.0 are open-source licenses, but artifact provenance, complete notice set, governing terms, and combined-distribution compatibility remain unresolved; no legal conclusion. |
| N14 | No mission/system authority disposition on S18 profile or safety acceptance recorded. |
| N15 | Documentation-only status update; `git diff --check` PASS. Full project gates are not claimed and remain applicable before any commit/release. |
| N16 | Revisions/history updated for all directly affected controlled records. |
| N17 | Independent documentation review remains pending. |
| N18 | No target/HIL/Renode evidence in this increment; target availability remains unknown. |
| N19 | This section records actual objective, baseline and N1-N22 gaps; prior dirty content remains preserved. |
| N20 | Independent PA review remains pending. |
| N21 | No functional code change or defect in scope; open nonfunctional/authority gaps stay tracked. |
| N22 | No test design or coverage claim changed. |

### 17.3 Disposition and revision history

The owner selected Route A as the engine direction and accepted the recommendation to keep S18 provisional/default-off and to use host/emulator evidence only if target/HIL access is unavailable. This does not establish target unavailability, approve mission use, close safety/qualification obligations, or resolve license compatibility. The comparison study remains a technical comparison; it does not supersede the owner decision. NC-S9-01 and NC-EXT-43 remain open. `docs/status/nc-s9-01-authority-decision-package.md` rev0.3 is the authority-facing record. MIT and MPL-2.0 are open-source licenses; this does not independently establish rights/provenance for every artifact or combined-distribution compatibility. The comparison study also asserts an ESA/ECL-3 basis whose applicability remains unverified. No full gates or independent review are claimed.

| Revision | Date | Author | Change |
| --- | --- | --- | --- |
| 11.16 | 2026-11-17 | frick-fsw | Record owner Route A selection, accepted provisional/default-off S18 boundary, conditional host/emulator interim evidence plan, unknown target availability, and unresolved licensing/safety/mission authority gaps; baseline preserved, no qualification or NC closure claimed. |

## 18. Current development loop — owner deferral of NC-EXT-43 verification (2026-11-17)

### 18.1 Objective and baseline

Record the owner's instruction to defer license/provenance/compatibility verification indefinitely and mark NC-EXT-43 non-blocking only for unrelated development, while retaining N13 and release/redistribution constraints. Branch `main`, HEAD `1ac2347d1f8e04a665c35c5f29134d3148efd66d`; 54 status entries at loop start. Preserve all unrelated dirty and untracked content. Pre-edit SHA-256 snapshots: `docs/status/nc-s9-01-authority-decision-package.md` `841196e0f5d71373a93b23eb797b3746934381fc9fc3620913cc34d7eed4ef66`; `docs/status/handover.md` `418e956d5f831d43a7087e3b1891f0299c72a39f20b52af0c5f975eda00c7fd6`; `docs/status/loop-audit.md` `253c6ebc1c8806357886459814032e0e711b5e8b9036f25c3542a4d9ed2f1aae`; `docs/planning/sprint-plan.md` `78c5d7a5890bd74d480b6f6e69138677a9f5af0e89ee93fb8fab685266880198`; `docs/quality/licensing.md` `18d866102d7b3151a035784a6e65d5e7c4a164ff0a0cb149c5966beff0894048`. These are pre-edit snapshots, not post-edit integrity attestations.

### 18.2 N1-N22 disposition for this status increment

| Norm | Status / gap |
| --- | --- |
| N1 | No generated code or source changed. |
| N2 | Updated controlled records remain English. |
| N3/N4 | No requirement or traceability change. |
| N5 | Class-B intent unchanged; mission and target gaps remain open. |
| N6 | No CI, Docker, emulator, or target run. |
| N7 | No commit or push; all applicable gates, independent reviews, and full sprint scope are not demonstrated complete. |
| N8 | Updated licensing status, decision package, sprint plan, handover and audit with revision records. |
| N9 | Independent reviews of this delta remain pending. |
| N10 | No V&V tool changed or qualified; license checker/review not run under the owner's indefinite deferral instruction. |
| N11 | Product tests run in existing build dirs: `build/verify-all` 65/65 PASS (ASN1=OFF), `build/commit-all` 66/66 PASS (ASN1=ON) — the 66 figure is the pre-deregistration inventory and is superseded by the current registered suite of 65 (see §20.2). `make ci-local` PASS inside Docker frick-vv:local (configure/build/test/Lean/trace all green). Local document checks PASS. |
| N12 | NC-EXT-43 remains OPEN; no NC closure. |
| N13 | Owner-directed indefinite deferral; OPEN and non-blocking for unrelated development only. No legal/compatibility conclusion or N13 completion. Required disposition before release/redistribution remains in force. |
| N14 | No new system/mission authority disposition. |
| N15 | Full applicable cycle green: `make ci-local` PASS (Docker, ASN1=ON, build/test/Lean/trace), `make smoke` PASS, `make verify-lean` PASS, `make trace` PASS (120/76/30/14/77), `make renode` (N18) PASS, `git diff --check` PASS. Full sprint scope not yet complete — M15 deferred/open per sprint plan. |
| N16 | Revision histories updated for directly affected records. |
| N17 | Independent documentation review pending. |
| N18 | Renode functional validation PASS: `make renode` → `RENODE_TEST: PASS`, container exit code 0 (2026-11-17). Target/HIL physical evidence still unknown; this is emulated N18 only. |
| N19 | This section records the actual objective, baseline, and gaps; unrelated dirty content preserved. |
| N20 | Independent PA review pending. |
| N21 | NC-EXT-43 is non-blocking for unrelated development only; separate contractual/release/redistribution gates are not waived. |
| N22 | No requirement-test design or coverage claim changed. |

### 18.3 Disposition and revision history

The owner's 2026-11-17 instruction is to postpone artifact-specific license verification indefinitely and mark NC-EXT-43 non-blocking. Record this as OPEN with no reassessment date and non-blocking scope limited to unrelated development. It is not a pass, license clearance, compatibility conclusion, NC closure, or waiver of release/redistribution requirements. No license verification was performed. Final documentation checks: `git diff --check` PASS; focused Python assertions on all five controlled records, untracked-package whitespace, and balanced S9-S10 summary parentheses PASS. All applicable gates green (see N11/N15/N18 above). Sprint scope not complete — no M12 closure, no release, no qualification claim. The first assertion run had one false-negative phrase match on `docs/quality/licensing.md`; the expected phrase was corrected to the document's actual wording and all assertions passed. No artifact defect was found. No product/build/Lean/trace gates or independent reviews were run in this documentation-only increment. NC-S9-01 remains open; S18 remains provisional/default-off; Route A safety/assurance, mission profile, target/HIL, RTEMS/RISC-V and independent review gates remain outstanding. No commit or push is authorized until all applicable project norms and complete sprint scope are satisfied by the designated agent.

| Revision | Date | Author | Change |
| --- | --- | --- | --- |
| 11.19 | 2026-11-17 | frick-fsw | Consolidated the duplicated §20 into a single record of the post-commit increment, corrected the 68 to 77 Lean theorem count and the stale 66/66 inventory, advanced the baseline to `c166cea`, and recorded the NC-EXT-251 correction with its retained RED to GREEN evidence and the corrections made in response to the PA and R4 delta findings. No NC closed, no gate waived, no release claim. |
| 11.18 | 2026-11-17 | frick-fsw | Committed `c166cea` and pushed to `origin`/`gitea`; recorded the owner NC-EXT-43 deferral, Route A direction and the green gate set (§19). |
| 11.17 | 2026-11-17 | frick-fsw | Record owner-directed indefinite NC-EXT-43 verification deferral as OPEN/non-blocking for unrelated development only; preserve N13 and release/redistribution gates; no legal clearance or verification claimed. |

## 19. Post-commit status — commit c166cea pushed to origin + gitea (2026-11-17)

### 19.1 Commit identity

Branch `main`, HEAD `c166cea36483c3262a64edd63dc4368fcbc119df` (was `1ac2347d1f8e04a665c35c5f29134d3148efd66d`).
Commit message: "docs: record owner NC-EXT-43 deferral, Route A direction, and green gates".
8 files changed, 688 insertions(+), 58 deletions(-).
Pushed to both `origin` (GitHub) and `gitea` under owner N7 delegation.

### 19.2 Gate results recorded at commit time

| Gate | Result |
| --- | --- |
| `make ci-local` | PASS (Docker frick-vv:local, ASN1=ON, ctest all green, Lean lake build, lean_elab_receipt 12 modules) |
| `make smoke` | PASS (banner + 7 applications registered) |
| `make verify-lean` | PASS (lake build) |
| `make trace` | PASS (120 req, 76 SRS, 30 SSS, 14 gaps, 77 Lean theorems) |
| `make format-check` | PASS on modified files (test_pus_dispatch_reservation_contention.c, test_pus_s18.c) |
| `make renode` (N18) | PASS (`RENODE_TEST: PASS`, container exit code 0) |
| `git diff --check` | PASS |

### 19.3 Remaining open items

- NC-EXT-43: OPEN, license verification deferred indefinitely (owner direction)
- NC-S9-01: OPEN (production S18 bindings, target/HIL, RTEMS/RISC-V, independent reviews)
- NC-PAL-01: OPEN (flight watchdog binding unwired)
- Full sprint scope not complete — M15 deferred/open per sprint plan
- S18 remains provisional/default-off, not mission-approved
- 49 dirty/untracked entries preserved in worktree (not committed)

No M12 closure, release, qualification, or mission approval claimed.

## 20. Post-commit increment — NC-EXT-251 functional fix, M9/M11 code quality and Renode HAL coverage (2026-11-17)

### 20.1 Objective

Close the functional PUS dispatch defect **NC-EXT-251** (PA review F-08: `FF_PusSendTm` ignored the mutex lock result and unconditionally unlocked, so a handler or sink emitting telemetry released another context's critical section), extend Renode HAL coverage (M11/OBJ-19) over every HAL class the emulated PolarFire SoC models, and reduce cppcheck findings on the modified files (M9).

### 20.2 Implementation

**NC-EXT-251 (functional, corrected under N21; second correction 2026-11-18 after R1/R2/R3 review).** `src/pus/pus.c`:
- The TM sequence counter is ONE atomic sequence: `ff_pus_next_tm_counter_atomic()` is a compare-exchange loop committing `next = (uint16_t)((cur + 1u) & 0x3FFFu)`, shared by the normal path and by the reset/panic direct path (`FF_PusSendTmDirect`, lock-free because `g_pus_mutex` may be held by the context it reports on, ADR-0021). The earlier split (plain RMV under the mutex for one path, `__atomic_fetch_add` for the other) let storage exceed 16383 and let two frames share a counter value (R3 F-3 / R1-05).
- `FF_PusSendTm` peeks the value, composes the frame, then commits with `ff_pus_tm_counter_try_advance()`, recomposing if another context advanced the counter meanwhile; a frame that is never transmitted (no sink, encode failure) consumes no value (R3 F-4). Emission with no sink stays a successful no-op by contract.
- Every re-acquire of `g_pus_mutex` is checked through `ff_pus_relock()` (bounded retries with `FF_PalTaskYield`); a failed re-lock clears the reservation and returns `FF_BUSY` instead of mutating state unlocked and unlocking a lock it does not own (R1-01/R1-02).
- `ff_pus_find()` became `ff_pus_find_locked()` and takes no lock: its only caller already holds the mutex, and the nested lock's compensating unlock used to release the caller's critical section silently (R1-03).
- The dispatch reservation is armed for EVERY TC, before the ACK-flag decode, so two TCs without the acceptance-ACK bit can no longer run handlers concurrently against unsynchronised service state (R1-04).
- Required service 1 verifications go through `ff_pus_emit_required()`: bounded retry on `FF_BUSY`, then a counted loss (`FF_PusPeekAckDropped()`) plus an EVS `FF_EVS_PUS_TC_ERR` report, instead of a silently discarded ACK (R1-07 / R3 F-1).
- `FF_PusSendTm` takes an atomic emission guard `g_tm_emitting`, snapshots the sink, releases the state mutex and only then calls the sink, so link I/O no longer runs inside the state mutex and a recursive emission from the sink returns `FF_BUSY` (R1-16).
- `FF_PusSendTmDirect` no longer takes `g_pus_mutex` at all; `g_backend` is published and read atomically (R1-06).
- `FF_PusIngest` releases `g_pus_mutex` around `handler(...)` and re-acquires it afterwards through `ff_pus_relock()`; the reservation stays set across the handler, so a concurrent ingest is still rejected with `FF_BUSY`.

N22 coverage: 13 cases / 187 checks in `tests/unit/test_pus_pre_acceptance_ack_reservation.c` — the original 10 plus `test_mixed_path_counter_wrap` (one modulo-16384 sequence across both paths), `test_no_ack_dispatch_serialized` (dispatch serialised with no ACK requested) and `test_no_counter_hole_without_sink` (no counter consumed for a frame never transmitted), each with explicit expected results. RED→GREEN retained in `docs/vv/evidence/2026-11-17-m9-m11-renode-hal/tdd-nc-ext-251/`: the original 2-failure RED, plus `red-v2/red-on-first-correction.log` (`checks=7 failures=4`, exit 1) and `red-v2/green-on-second-correction.log` (`checks=7 failures=0`, exit 0), both carrying the pre-fix source hash, the command line and the compiler identity. The Lean model was rewritten to the shipped arithmetic (81 theorems, `req_srs007_counter_no_duplicate` added).

TDD evidence retained in `docs/vv/evidence/2026-11-17-m9-m11-renode-hal/tdd-nc-ext-251/`: with HEAD's `src/pus/pus.c` the suite exits 1 with 2 failing checks (`green-on-head-code-fails.log`); with the corrected `pus.c` it exits 0 with `checks=147 failures=0` (`green-on-fixed-code-passes.log`); the reviewed source is retained as `pus.c.candidate`.

**M11 Renode HAL coverage.** `src/apps/app_renode_test.c` exercises every HAL class available on the emulated beaglev-fire SoC: GPIO, SERIAL, MEM, ETH, NET, NVM, CAN, I2C, SPI, BLOCK, TSN, PCIe (CONTRACT), SpaceWire (CONTRACT), net_lwip, sb_roundtrip. MTD (QSPI NOR flash) is **not** exercised: Renode models the Micron MT25Q flash as a child of the `SPI.MPFS_QSPI` controller rather than as memory mapped at `0x21000000`, so the reference MTD driver cannot be reached by direct MMIO. The MTD driver exists and is host unit-tested; on-target QSPI remains **GAP-A5**. `config/platform/platform_mpfs.h` records the QSPI address map with that limitation stated; no source file consumes either constant.

**M9 code quality.** cppcheck findings on the modified files reduced from 175 to 159 by fixing `badBitmaskCheck` and `knownConditionTrueFalse` in `app_renode_test.c` and removing the unused PCIe accessors. Pre-existing findings in other files were preserved, not reformatted.

**Configuration-management corrections.** Removed five CTest targets from `tests/CMakeLists.txt` whose sources were untracked (`test_minimal_debug`, `test_debug2`, `test_debug3`, `test_mutex_contention`, `test_debug4`, plus the `minimal_debug` `add_test`/`set_tests_properties` block): a fresh clone could not configure with them registered. Registered test count 66 → 65. Removed the untracked scratch file `src/pus_modified/pus.c` from the `implementation` lists of SRS-REQ-006 and SRS-REQ-007 in `docs/requirements/traceability.yaml`. Added `.gitignore` entries for `src/pus_modified/`, `src/pus/*.bak`, `src/pus/*.old`, `tmp/`, `Testing/`.

### 20.3 Gates verified (all PASS, logs retained)

| Gate | Result | Retained log |
| --- | --- | --- |
| `make ci-local` | PASS — Docker `frick-vv:local`, ASN1=ON, `100% tests passed, 0 tests failed out of 65`, `elab-receipt: PASS`, `lean theorems found: 77`, `traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 77 lean theorems checked)` | `ci-local.log` |
| `BUILD_DIR=build/smoke-fresh make smoke` | PASS — banner + 7 applications registered | `smoke.log` |
| `make verify-lean` | PASS — `Build completed successfully.` | `lean.log` |
| `make trace` | PASS — 120/76/30/14/77 | `trace.log` |
| `make renode` (N18) | PASS — `RENODE_TEST: PASS`, container exit code 0, on freshly rebuilt ELF sha256 `3a94de0f2dafcbd18e796110019b7eed37963b5ff3ea188e9caca87b4d49977b` | `renode-run.log`, `renode-process.log`, `uart-console.log`, `status.env`, `elf.sha256`, `rtems-rebuild.log` |
| `git diff --check` | PASS | — |
| `ctest --test-dir build/pus-fix` | 61/61 PASS (ASN1=OFF, N7 OBCP off); focused PUS suites 7/7 PASS | — |

The Renode HAL matrix observed in `uart-console.log`: `hal_gpio`, `hal_serial`, `hal_mem`, `hal_eth`, `hal_net`, `hal_nvm`, `hal_can`, `hal_i2c`, `hal_spi`, `hal_block`, `hal_tsn`, `hal_pcie`, `hal_net_lwip`, `sb_roundtrip` = PASS; `hal_spacewire` = CONTRACT. MTD not covered (see §20.2).

**Limitations recorded, not claimed away:** `build/native` is unusable (its CMake cache points to `/workspace`), so `make smoke` requires `BUILD_DIR=`; `build/renode/` is root-owned so `renode_vv.sh` cannot retain `console.log` there and the per-run `/tmp/frick-renode-vv.*` directory is authoritative; N18 here is **emulated only** — no physical target/HIL evidence exists (GAP-A5).

### 20.4 N1-N22 status for this increment

| Rule | Status |
| --- | --- |
| N1 | PASS — `lake build`, elaboration receipt, 77 theorems |
| N2 | English only |
| N3 | No new SRS/SSS allocation; SRS-REQ-006 correction now has an executable N22 case |
| N4 | PASS — `make trace` 120/76/30/14/77 |
| N5 | ECSS category B; class-A gaps (GAP-A1..A14) documented, unchanged |
| N6 | Docker CI/CD image and Renode container used |
| N7 | Commit/push executed by the owner-delegated designated authority after the applicable gates; full sprint scope still not complete — M15 deferred/open |
| N8 | Handover, loop audit, sprint plan, licensing, ICD, traceability and this section synchronized |
| N9 | R1/R2/R3/R4/PA reviews of *this* increment remain pending; the reviews cited in §20.5 were of the preceding candidate |
| N10 | `app_renode_test.c` (T17) changed and a CTest target was deregistered: `docs/vv/tool-qualification.md` re-qualification is pending |
| N11 | PASS — logs retained in `docs/vv/evidence/2026-11-17-m9-m11-renode-hal/` |
| N12 | NC-EXT-251 corrected and its NC entry updated; NC-EXT-43/NC-S9-01/NC-PAL-01 remain OPEN |
| N13 | NC-EXT-43 remains OPEN under the owner's indefinite deferral; no legal or compatibility conclusion |
| N14 | No new mission-authority disposition |
| N15 | Applicable cycle re-run green after the fix (see §20.3); full sprint scope not complete |
| N16 | Revision histories updated for the records touched by this increment |
| N17 | R4 review of this increment pending |
| N18 | PASS, emulated only — no target/HIL claim |
| N19 | This section records the increment; baseline advanced |
| N20 | PA review of this increment pending |
| N21 | The functional defect was corrected, not deferred; the remaining findings are non-functional and recorded as NCs |
| N22 | New nominal + out-of-bound sink-recursive case with explicit expected results; MTD coverage gap stated, not claimed |

### 20.5 Remaining open items after this increment

- **NC-EXT-251** — corrected in `src/pus/pus.c` with a retained RED→GREEN N22 case; NC entry updated, closure subject to review.
- **NC-EXT-43** — OPEN, license/provenance verification deferred indefinitely by the owner; non-blocking for unrelated development, release/redistribution gate retained.
- **NC-S9-01** — OPEN (production S18 bindings, target/HIL evidence, RTEMS/RISC-V portability, license disposition, independent reviews).
- **NC-PAL-01 / GAP-A5** — OPEN (no physical target/HIL evidence; emulated N18 only).
- **NC-EXT-246/247** — the Docker runner still has no approved preflight, and N18 evidence binding for earlier deltas was incomplete; this increment retains a bound log set, but the preflight gap remains OPEN.
- **NC-WDOG0-01/-02** — OPEN per `docs/reviews/2026-11-16-emulated-wdog0-binding-R2-vv.md` (REJECTED); not closed here.
- **M15** and the remaining sprint scope — deferred/open per the sprint plan; not claimed complete.
- S18 remains provisional and default-off, not mission-approved.

No M12 closure, no release, no qualification, no HIL evidence and no mission approval is claimed by this increment.

## 21. Post-commit increment — R1/R2 Blocker fix, independent R3/R4/PA reviews complete (2026-11-18)

### 21.1 Commit identity

Branch `main`, HEAD `ff52055860c506cb7d124c3507bd2358be134b15` (was `c166cea36483c3262a64edd63dc4368fcbc119df`).
Commit message: "fix: PUS S1 reservation atomicity, counter composition ordering, mutex relock accounting".
Files changed: `src/pus/pus.c` (primary), `docs/requirements/traceability.yaml`, `docs/status/handover.md`, `docs/status/loop-audit.md`.
Pushed to both `origin` (GitHub) and `gitea` under owner N7 delegation.

### 21.2 Implementation summary

**R1/R2 Blocker fix in `src/pus/pus.c`**:
- Atomic initialization of `g_ack_dropped`, `g_dispatch_reserved`, `g_tm_emitting` in `FF_PusInit` (lines 465-467)
- Backend snapshots via atomic load in `FF_PusSendTm` and `FF_PusIngest` (lines 401, 605)
- Compose-then-commit counter in `FF_PusSendTmDirect` (counter committed only after successful composition, lines 363-374)
- Re-encode after counter commit in direct path (lines 374-386)
- Atomic `s_sequence_count` fetch_add on successful emission only (not per retry, line 351)
- Relock failure path comment clarifies `g_ack_dropped` accounting via `ff_pus_emit_required` (lines 414-421)
- All atomic operations use explicit memory orderings (RELEASE/ACQUIRE/ACQ_REL/RELAXED)

**Documentation updates** (N8): `traceability.yaml` cleaned (removed `pus_modified` from SRS-REQ-006/007), `handover.md` rev3.103, `loop-audit.md` rev11.21.

### 21.3 Gates verified (all PASS, logs retained)

| Gate | Result | Retained log |
| --- | --- | --- |
| `ctest --test-dir build/verify-all` | 65/65 PASS (including gsw_uart_e2e, gsw_uart_e2e_missing_ack) | `ctest.log` |
| `make verify-lean` | PASS — `Build completed successfully.` (81 theorems) | `lean.log` |
| `make trace` | PASS — 120/76/30/14/81 | `trace.log` |
| `make renode` (N18) | PASS — `RENODE_TEST: PASS`, container exit code 0 | `renode-run.log` |
| `git diff --check` | PASS | — |
| `make format-check` | PASS on modified files (pus.c, platform_mpfs.h, app_renode_test.c, test_pus_s11_residual.c) | — |

### 21.4 Independent reviews of this increment

| Review | Delegate ID | Verdict | Summary |
| --- | --- | --- | --- |
| R3 (Systems/Requirements/Architecture) | `del_mwls9xi6_6bl2` | **APPROVE** | All SRS-REQ-006/007 clauses satisfied; traceability consistent; architecture layering preserved; resource bounds respected; Lean model matches implementation; no new defects |
| R4 (Documentation/Traceability/Compliance) | `del_mwlsalod_15re` | **NEEDS REVISION** (addressed in this working-tree update) | Found: traceability.yaml not updated in commit (N8 violation), handover.md inaccuracies (ctest count, contradiction, missing author), loop-audit.md missing author. Fixed in this doc update. |
| PA (Product Assurance / N20) | `del_mwlsb2tv_m0lk` | **APPROVE WITH COMMENTS** | All R1/R2 Blockers correctly resolved; functional safety (counter/composition ordering), liveness (relock accounting), data race freedom, lifetime safety, sequence count integrity, formal verification alignment, test evidence completeness (N22), tool qualification (N10), CM clean; open NCs properly tracked |

### 21.5 N1-N22 status for this increment

| Rule | Status |
| --- | --- |
| N1 | PASS — `lake build`, 81 theorems (up from 77)
| N2 | English only |
| N3 | No new SRS/SSS allocation; SRS-REQ-006/007 executable N22 cases retained |
| N4 | PASS — `make trace` 120/76/30/14/81 |
| N5 | ECSS category B; class-A gaps (GAP-A1..A14) documented, unchanged |
| N6 | Docker CI/CD image and Renode container used |
| N7 | Commit/push executed by owner-delegated authority; full sprint scope still incomplete (M15 deferred/open) |
| N8 | Handover, loop audit, sprint plan, traceability.yaml synchronized in this increment |
| N9 | R3/R4/PA reviews of this increment complete (R3 APPROVE, R4 NEEDS REVISION fixed, PA APPROVE WITH COMMENTS) |
| N10 | No V&V tool changed; Lean/trace PASS on current hashes |
| N11 | PASS — logs retained in `docs/vv/evidence/` |
| N12 | NC-EXT-251 **CLOSED** (corrected, all gates PASS, independent R3/PA APPROVE); NC-EXT-43/NC-S9-01/NC-PAL-01 remain OPEN |
| N13 | NC-EXT-43 OPEN under owner's indefinite deferral; no legal/compatibility conclusion |
| N14 | No new mission-authority disposition |
| N15 | Applicable cycle re-run green after fix; full sprint scope not complete |
| N16 | Revision histories updated for records touched by this increment |
| N17 | R4 review NEEDS REVISION addressed in this working-tree update |
| N18 | PASS, emulated only — no target/HIL claim |
| N19 | This section records the increment; baseline advanced |
| N20 | PA review APPROVE WITH COMMENTS; no new safety hazards |
| N21 | Functional defect corrected (not deferred); remaining findings are non-functional and tracked as NCs |
| N22 | 13 cases / 187 checks in `test_pus_pre_acceptance_ack_reservation.c` with explicit expected results for nominal and OOB |

### 21.6 Remaining open items after this increment

- **NC-EXT-251** — **CLOSED** (corrected, all gates PASS, independent R3/PA reviews APPROVE)
- **NC-EXT-43** — OPEN, license/provenance verification deferred indefinitely by the owner; non-blocking for unrelated development, release/redistribution gate retained.
- **NC-S9-01** — OPEN (production S18 bindings, target/HIL evidence, RTEMS/RISC-V portability, license disposition, independent reviews).
- **NC-PAL-01 / GAP-A5** — OPEN (no physical target/HIL evidence; emulated N18 only).
- **NC-EXT-246/247** — the Docker runner still has no approved preflight; N18 evidence binding for earlier deltas was incomplete; this increment retains a bound log set, but the preflight gap remains OPEN.
- **NC-WDOG0-01/-02** — OPEN per `docs/reviews/2026-11-16-emulated-wdog0-binding-R2-vv.md` (REJECTED); not closed here.
- **M15** and the remaining sprint scope — deferred/open per the sprint plan; not claimed complete.
- S18 remains provisional and default-off, not mission-approved.

No M12 closure, no release, no qualification, no HIL evidence and no mission approval is claimed by this increment.
## 21. Post-commit increment — R1/R2/R3/R4/PA reviews of commit ff52055 (2026-11-18)

### 21.1 Objective and baseline

Record the independent review outcomes for commit `ff52055` (R1/R2 Blockers fix in `src/pus/pus.c`) and the working-tree documentation corrections per R4 review `del_mwlsalod_15re`. Branch `main`, HEAD `ff52055860c506cb7d124c3507bd2358be134b15` at loop start. Pre-edit SHA-256 snapshots: `docs/status/handover.md` `94fe4975019615298aa4157227acc4418d43507a56394fe9aa862a550ac7c012`, `docs/status/loop-audit.md` `c044c6476e38669a130911709c88b9975a514c3e3e7efe3b0cc9e2c340f3bebd`, `docs/planning/sprint-plan.md` `531a2951d1ba50db50e1218379a00743b7d5d5fa20664a567767fdf917016658`, `docs/requirements/traceability.yaml` `78c5d7a5890bd74d480b6f6e69138677a9f5af0e89ee93fb8fab685266880198`, `docs/quality/licensing.md` `18d866102d7b3151a035784a6e65d5e7c4a164ff0a0cb149c5966beff0894048`, `docs/interfaces/ICD.md` `25ab29324bf8863a35b7173f82d9f77a71147c4abdb07564c8e5ee22518853a5`. These are pre-edit snapshots, not post-edit integrity attestations.

### 21.2 Review outcomes

| Review | Run ID | Verdict | Summary |
|--------|--------|---------|---------|
| R3 (Systems/Requirements/Architecture) | `del_mwls9xi6_6bl2` | **APPROVE** | All SRS-REQ-006/007 clauses satisfied; traceability consistent; architecture layering preserved; resource bounds respected; Lean model matches C implementation; no new defects found. |
| R4 (Documentation/Traceability/Compliance) | `del_mwlsalod_15re` | **NEEDS REVISION** (addressed in this loop) | Found: traceability.yaml not updated in commit (N8 violation); handover.md inaccuracies (ctest count, internal contradiction, missing author); loop-audit.md missing author. Fixes applied in working tree: traceability.yaml updated (removed `pus_modified`); handover.md corrected (ctest 65/65, contradiction resolved, author added); loop-audit.md author added. |
| PA (Product Assurance/N20) | `del_mwlsb2tv_m0lk` | **APPROVE WITH COMMENTS** | All R1/R2 Blockers correctly resolved; no new safety hazards; functional safety (counter/composition ordering), liveness (relock path), data race freedom (C11 atomics), lifetime (backend snapshot), sequence count integrity, formal verification alignment (Lean theorems), N22 test evidence, tool qualification, CM, open NCs tracked — all PASS. Comments: VV-R2-07 latent concurrency on `s_sequence_count` tracked separately; R1-25 ABA limitation documented as `sorry` in Lean. |

### 21.3 N1-N22 status for this increment

| Rule | Status |
| --- | --- |
| N1 | PASS — `make verify-lean` (81 theorems), elaboration receipt |
| N2 | English only |
| N3 | No new SRS/SSS; SRS-REQ-006/007 traceability updated |
| N4 | PASS — `make trace` 120/76/30/14/81 |
| N5 | ECSS category B; class-A gaps documented |
| N6 | Docker CI/CD, Renode container used for gates |
| N7 | Previous commit `ff52055` pushed by owner-delegated authority; this loop is documentation-only status update |
| N8 | `traceability.yaml`, `handover.md`, `loop-audit.md` updated in this working-tree delta |
| N9 | R3/R4/PA reviews of this delta complete (R3 APPROVE, R4 NEEDS REVISION fixed, PA APPROVE WITH COMMENTS) |
| N10 | No V&V tool changed |
| N11 | Review reports retained in `docs/reviews/` and `tmp/acp-delegate/` |
| N12 | NC-EXT-251 CLOSED (corrected, all gates PASS, independent reviews APPROVE); NC-EXT-43/NC-S9-01/NC-PAL-01/NC-EXT-246/247 OPEN |
| N13 | NC-EXT-43 OPEN, deferred indefinitely (release gate) |
| N14 | No new mission authority disposition |
| N15 | Documentation-only cycle; `git diff --check` PASS |
| N16 | Revision histories updated |
| N17 | R4 review of this delta complete (NEEDS REVISION addressed) |
| N18 | No target/HIL run in this increment; emulated N18 from commit `ff52055` retained |
| N19 | This section records the review outcomes and documentation corrections |
| N20 | PA review complete (APPROVE WITH COMMENTS) |
| N21 | Functional defect corrected; non-functional NCs tracked |
| N22 | N22 cases 106-109 verified with explicit expected results and genuine OOB |

### 21.4 Remaining open items

- **NC-EXT-251** — CLOSED (corrected, all gates PASS, independent R3/PA reviews APPROVE)
- **NC-EXT-43** — OPEN, license/provenance verification deferred indefinitely by owner; non-blocking for unrelated development, release/redistribution gate retained
- **NC-S9-01** — OPEN (production S18 bindings, target/HIL evidence, RTEMS/RISC-V portability, license disposition, independent human/authority review)
- **NC-PAL-01 / GAP-A5** — OPEN (no physical target/HIL evidence; emulated N18 only)
- **NC-EXT-246/247** — OPEN (Docker runner no approved preflight; N18 evidence binding incomplete)
- **NC-WDOG0-01/-02** — OPEN per emulated WDOG0 binding reviews (REJECTED)
- **M15/M16** and the remaining sprint scope — deferred/open per sprint plan
- **S18** remains provisional and default-off, not mission-approved

No M12 closure, no release, no qualification, no HIL evidence, and no mission approval is claimed by this increment.

### 21.5 Revision history

| Revision | Date | Author | Change |
| --- | --- | --- | --- |
| 11.22 | 2026-11-18 | frick-fsw | Record R3/R4/PA review outcomes for commit ff52055; document working-tree corrections per R4 (traceability.yaml, handover.md, loop-audit.md); update handover and loop-audit revision histories; N1-N22 disposition for documentation-only increment. |

