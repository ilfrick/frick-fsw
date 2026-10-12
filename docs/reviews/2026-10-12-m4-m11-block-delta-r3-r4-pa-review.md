# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0147                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2 (delta) + R3 + R4 + PA** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) **delta closure** of FRICK-REVIEW-0146, **R3** (Senior avionics / systems engineer, N14), **R4** (Senior SW documentation engineer, N17) and **PA** (Senior SW Product Assurance engineer, N20) |
| Reference revision                | `737c805` (HEAD) + **uncommitted** Sprint-5 working-tree increment (reference MSS block-device backend, SRS-REQ-070) |
| Items under review                | `src/drivers/block/mpfs/ff_block_mpfs.{c,h}` (new); `tests/unit/test_block.c` (new); the `hal_block` deltas in `src/apps/app_renode_test.c`; `config/platform/platform_mpfs.h` (`FF_MMC0_BASE`); `CMakeLists.txt`, `tests/CMakeLists.txt`; requirement **SRS-REQ-070** and its chain in `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`; the contract `FF_BlockDev_t` (`src/hal/hal_mem.h`); `docs/design/software-design.md`; `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/quality/class-a-gaps.md`; `docs/engineering/software-description.md`; `CHANGELOG.md`; `README.md`; `docs/status/{handover,loop-audit,status.json}`; `docs/planning/sprint-plan.md`; `docs/development-plan.md`; the `ci-renode` markers in both CI workflows; and the delta dispositions of FRICK-REVIEW-0146 (R1-01/R1-02/R1-03/R2-01/SCOPE-01) |
| Date                              | 2026-10-12                             |
| Scope                             | Combined delta-closure + systems/requirements + documentation + product-assurance review of the reference MSS MMC/SD (SDHCI-style) HAL block-device backend and its evidence, against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA), the project norms N1–N20 (`docs/rules.md`), the reviewer contract `docs/engineering/independent-review.md` (rev 0.7), the HAL contract `FF_BlockDev_t` (`src/hal/hal_mem.h`), and the authoritative register map of the Renode `SD.MPFS_SDController` model. |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery or previously reviewing the items under review. This review ran in a fresh context and every declared claim was re-executed or read first-hand (see §12). |
| Norms applied                    | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **Part A — R1+R2 delta closure.** First-hand re-verification of the five dispositions of
  FRICK-REVIEW-0146: R1-01 (Major, `block_size = 4096` vs the 12-bit TBS field), R1-02 (timeout
  path status clear), R1-03 (`ErrorNormalStatusEnable` programming), R2-01 (test discrimination),
  SCOPE-01 (plan doc currency); and a regression check of the whole block increment
  (driver/test/Renode/build/trace).
- **Part B — R3 (systems/requirements, N14).** SRS-REQ-070 well-formedness (singular, testable,
  parented to SSS-REQ-020, method-annotated, no over-claim); the contract `FF_BlockDev_t`
  (`src/hal/hal_mem.h`) against the delivered backend; trace manifest / matrix / top-down /
  bottom-up agreement (counts **114/71/29/14/51**); the block-size bound (≤2048) documented
  consistently.
- **Part C — R4 (documentation, N17).** Currency and internal consistency of
  `docs/vv/{verification-results,renode-environment,tool-qualification}.md`,
  `docs/design/software-design.md`, `docs/quality/class-a-gaps.md`,
  `docs/engineering/software-description.md`, `docs/status/{handover,loop-audit,status.json}`,
  `docs/planning/sprint-plan.md`, `docs/development-plan.md`, `CHANGELOG.md`, `README.md`, both
  `ci-renode` markers; per-document header `Revision`/`Date` vs last `## History` row (N16); stale
  counts / "pending" rows.
- **Part D — PA (product assurance, N20).** Review set/log, semaphores, records integrity, claim
  discipline and the residual-gap register (GAP-A5 block residual).
- Independent re-execution of the full local gate set (see §12).

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour (configured SD clock, bus width, card initialisation/type, real card
  timing, on-target data path and wall-clock latency) — tracked to **GAP-A5**, explicitly not
  verifiable here.
- The `ff_block_mpfs.h` register map vs `mss_sd.h`/`mss_sd_regs.h` on silicon (headers are not
  revision-controlled in this repository) — GAP-A5; the emulated-oracle match was already
  established in FRICK-REVIEW-0146 and is re-asserted by the passing Renode check.
- Unrelated doc-only deltas of other review scopes; unrelated increments re-run only as aggregate
  regressions.

**Assumptions recorded:** (a) the assigned document id `FRICK-REVIEW-0147` was supplied by the review
lead (the highest committed id observed in `docs/reviews/` is `FRICK-REVIEW-0145`); (b) the item
under review is the uncommitted working tree atop `737c805`; (c) the 12-bit TBS width is
authoritative (model `.WithValueField(0, 12)`), as accepted in FRICK-REVIEW-0146.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract: `docs/engineering/independent-review.md` (rev 0.7); template: `docs/reviews/review-template.md`.
- Contract: `src/hal/hal_mem.h` (`FF_BlockDev_t`, lines 65–74).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-070 lines 336–347; SRS-REQ-032), `docs/requirements/SSS.md` (SSS-REQ-020 lines 89–92), `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`.
- Scope documents: `docs/roadmap.md` (M4/M11), `docs/development-plan.md` (OBJ-10/OBJ-19), `docs/planning/sprint-plan.md` (S5), `docs/vv/renode-environment.md`.
- Gaps: `docs/quality/class-a-gaps.md` (GAP-A5, lines 88–115).
- Prior review: `docs/reviews/2026-10-12-m4-m11-block-r1-r2-review.md` (FRICK-REVIEW-0146).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Part A — R1+R2 delta closure

| NC (FRICK-REVIEW-0146) | Declared fix | First-hand verification | Status |
| ---------------------- | ------------ | ----------------------- | ------ |
| **R1-01** (Major) | `FF_MMC_BLOCK_SIZE_MAX` → 2048; add `FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK, …)`; `Setup` rejects 4096; header comments updated; test asserts 4096 → `FF_INVALID_ARGUMENT`, 2048 → success | `ff_block_mpfs.h:231` `#define FF_MMC_BLOCK_SIZE_MAX 2048u`; `ff_block_mpfs.c:24` `FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK, block_size_fits_tbs)` (macro is a real compile-time assert: `ff_types.h:41`); `ff_block_mpfs.c:259-262` rejects `block_size > FF_MMC_BLOCK_SIZE_MAX`; `test_block.c:255` asserts 4096 → `FF_INVALID_ARGUMENT`, `:257-258` asserts 2048 → success. `4096 & FF_MMC_TBS_MASK (= 0x0FFF) == 0` re-derived; `2048 & 0x0FFF == 0x800` (representable). No caller passes 4096 (`app_renode_test.c:353` uses 512; the mock uses 512). **Residual (documentation only):** `ff_block_mpfs.h:248` and `:256` still document `512..4096` → new finding **B-01** (Minor). | **CLOSED** (code defect resolved and regression-tested; documentation residual raised as B-01) |
| **R1-02** (Minor) | On `FF_TIMEOUT` clear the whole status word (`FF_MMC_INT_CLEAR_ALL`) before returning | `ff_block_mpfs.c:69-72`: timeout path now `block_wr(FF_MMC_REG_ERR_NORMAL_INT_STATUS, FF_MMC_INT_CLEAR_ALL)` (`0xFFFFFFFF`, `ff_block_mpfs.h:212`) before `return FF_TIMEOUT`; success path clears the completion bits (`:65`), error path clears the observed status (`:61`). Every exit path of `block_wait_done` now clears status → no stale completion/error bit survives a call. | **CLOSED** (code) — see **R2-01-R** (no regression test asserts the clear) |
| **R1-03** (Minor) | `block_issue` programs `ErrorNormalStatusEnable` (0x234) with the completion + error bits before issuing | `ff_block_mpfs.c:82-84`: `block_wr(FF_MMC_REG_ERR_NORMAL_INT_ENABLE, FF_MMC_INT_DONE \| FF_MMC_INT_ERROR_MASK)` is written before BlockSizeBlockCount/Argument1/CommandTransferMode. Consistent with the emulated model's enable-gated completion bits. | **CLOSED** (code) — see **C-02** (comment: the mock does not observe/assert the 0x234 write) |
| **R2-01** (Minor) | Test now covers the 4096 rejection and the 2048 boundary (82 checks) | `test_block.c:253-258` present; `./build/native/tests/test_block` → `checks=82 failures=0` (74 static assert sites; `mock_bind`'s single assert executes 9× = +8, so 82 is arithmetically consistent). **The second half of R2-01 is not addressed:** no test pre-sets a stale status bit and asserts the timeout clear; `mock_exec_cmd` forces `err_status = 0` in `never_ready` mode (`test_block.c:92-94`), so the mock still cannot model a stale bit. | **PARTIALLY CLOSED** → residual **R2-01-R** (Minor) |
| **SCOPE-01** (Minor) | `docs/development-plan.md` §6 M4/M11 rows updated (BLOCK done, M11 9/13) + history row 0.22 | `development-plan.md:310` M4 row now "…MSS SPI, **MSS block-device (MMC/SD) done**; eMMC/storage/PCIe/SpaceWire pending"; `:317` M11 row "**in progress (9/13 peripheral-bound)** … `hal_block=PASS` … Pending: NET(lwIP-on-GEM)/TSN/PCIE/SPACEWIRE"; history `:355` row 0.22 present, in order, header `Revision 0.22` matches (N16). Consistent with README/handover/loop-audit/sprint-plan/status.json/software-description/verification-results/renode-environment (all 9/13). | **CLOSED** — with new **B-02** (loop-audit M4 row self-contradiction) |

**Regression check (no regression introduced by the delta):**

- `FF_STATIC_ASSERT` bounds pair intact (`ff_block_mpfs.c:21,24,25`); `FF_MMC_BLOCK_SIZE_MIN 512 ≤ MAX 2048 ≤ TBS_MASK 4095`.
- Driver unchanged in the register map, command encoding, bounds, bounded/allocation-free structure, and strict-warning cleanliness (re-verified, §12).
- 22/22 ctest (native **and** ASan/UBSan), `test_block` 82 checks, `trace.py` PASS 114/71/29/14, `metrics --check` current, clang-format clean, Renode `RENODE_TEST: PASS` incl. `hal_block=PASS`, QEMU PASS, `lake build` PASS.

## 4. Part B — R3 (systems / requirements) findings

- **B-SRS — SRS-REQ-070 well-formedness: compliant (no finding).** `docs/requirements/SRS.md:336-347`. Singular (one backend behind one contract); testable (host unit test + emulated register check; failure mapping to `FF_TIMEOUT`/`FF_ERROR`/`FF_INVALID_ARGUMENT`/`FF_NOT_READY` is observable); parented to **SSS-REQ-020** (`parent_sss: SSS-REQ-020`, `derived: false`; SSS-REQ-020 = "reference low-level implementations" — the block device is a HAL contract reference implementation, in scope); method-annotated (`method: Tst | Dem`); no over-claim (it claims host test + emulated **register** check only; the on-target SD clock/bus-width/card-init residuals are explicitly tracked to GAP-A5, and the emulated board has no SD card). The requirement does not restate the 2048 block-size bound — acceptable (a driver-level bound, not a system requirement).
- **B-CONTRACT — `FF_BlockDev_t` conformance: compliant (no finding).** `src/hal/hal_mem.h:65-74` declares `name`, `block_size`, `num_blocks`, `read(uint64_t,uint8_t*,uint32_t)`, `write(uint64_t,const uint8_t*,uint32_t)`, `flush(void)`. `FF_BlockMpfsSetup` (`ff_block_mpfs.c:266-277`) populates every member (`name="mpfs-mmc"`, `block_size`, `num_blocks`, `read`/`write`/`flush`); `Setup`/`SelfTest` are driver-specific, outside the vtable (consistent with the sibling backends). No type or semantic mismatch.
- **B-TRACE — manifest / matrix / top-down / bottom-up: converged (no finding).** `trace.py` PASS with **114 requirements (71 srs / 29 sss / 14 gaps), 51 Lean theorems, 12 CI claims**; `traceability.md` (rev 0.32) counts SRS=71/SSS=29/GAP=14/Lean=51; `README.md` panel 114 (srs=71, sss=29, gap=14). Top-down: SRS-REQ-070 → SSS-REQ-020 (`traceability.md:50`); bottom-up: `traceability.md:135` SRS-REQ-070 → `drivers/block/mpfs/ff_block_mpfs.c/h`, `config/platform/platform_mpfs.h` → `test_block.c`, `ci:renode`. SDD design matrix `software-design.md:113` allocates SRS-REQ-070 to `drivers/block/mpfs`.
- **B-01 — Minor — block-size bound documented inconsistently (N8).** `ff_block_mpfs.h:231` defines `FF_MMC_BLOCK_SIZE_MAX 2048u` and `:228-229` correctly explains the 12-bit TBS limit, but **`:248`** (`uint32_t block_size; /* bytes per block (power of two, 512..4096) */`) and **`:256`** (Setup doc: "if block_size is not a power of two in **[512, 4096]**") still advertise **4096** as accepted. An integrator following the contract comment would pass 4096 and receive `FF_INVALID_ARGUMENT`. The R1-01 fix was applied to the code, the bound macro and two of the four comment sites, but not to these two. This is the *only* place the "≤2048" bound is inconsistent (SRS, traceability, Renode/SDD/DDJF docs, README and the test all agree). Owner: authoring agent. Sprint: **S5 delta (pre-commit)**.
- **B-02 — Minor — `docs/status/loop-audit.md` M4 row contradicts itself (N8/N19).** `loop-audit.md:27` now reads "…MSS SPI done (SRS-REQ-069), **MSS block-device done (SRS-REQ-070); MMC/eMMC/storage/PCIe/SpaceWire pending**" — i.e. the same row asserts the MSS **MMC/SD** block device is done *and* that **MMC** is pending, while the M11 row in the same table (`:34`) records "…/SPI/**BLOCK** done (9/13)". This is the recurring M4/M11 inconsistency class flagged for SPI (FRICK-REVIEW-0142/0146 PA-03). Fix: drop `MMC` from the pending list (`eMMC/storage/PCIe/SpaceWire pending`), matching `development-plan.md:310`. Owner: authoring agent. Sprint: **S5 delta (pre-commit)**.
- **B-03 — Comment — "eMMC/storage pending" residual wording.** `development-plan.md:310` keeps "eMMC/storage/PCIe/SpaceWire pending" while the driver header calls the controller "eMMC/SD". The residual is defensible (eMMC-specific features — boot partition, HS400 tuning, CQ — are not implemented; SRS-REQ-070 covers the SDHCI-style MMC/SD controller), but the plan could state which eMMC features remain. No correction required.

## 5. Part C — R4 (documentation) findings

**Header/history consistency (N16) — all touched documents compliant:** for every touched document with a `## History` table the header `Revision`/`Date` equals the last history row and the row is in ascending order — `SRS.md` 1.20/2026-10-12 (`:454`), `traceability.md` 0.32 (`:238`), `development-plan.md` 0.22 (`:355`), `sprint-plan.md` 0.8 (`:110`), `loop-audit.md` 0.16 (`:171`), `verification-results.md` 0.37 (`:441`), `renode-environment.md` 0.16 (`:166`), `tool-qualification.md` 0.3.20 (`:129`), `software-design.md` 0.8 (`:155`), `class-a-gaps.md` 0.10 (`:224`), `software-description.md` 0.5 (`:466`), `handover.md` 1.12 (`:214`). `README.md`/`CHANGELOG.md` are not `## History`-bearing (CHANGELOG is the project-level history, `[Unreleased]` entry present).

**Content currency — compliant:** `verification-results.md:49` block row (S5, PASS, 22/22, `test_block` 82, 114/71/29/14, REAL Renode incl. `hal_block=PASS`, scope limits + GAP-A5); `renode-environment.md:95,105` "Peripheral-bound (9/13)", BLOCK enumerated (SRS-REQ-070) with the no-SD-card scope-limit note; `tool-qualification.md:27,41` T3 gains `test_block.c`, T17 gains `hal_block`; `software-design.md:113` design matrix; `class-a-gaps.md:114-117` GAP-A5 block residual; `software-description.md:181,283` BLOCK implemented; `handover.md` current state at `737c805` + S5, M11 9/13, counts 114, next S6; `sprint-plan.md:47,110` S5 "delivered" (`hal_block=PASS` met; M11 9/13; ctest 22/22; test_block 82); `status.json` `sprints.done=5`; `CHANGELOG.md` S5 entry; both `ci-renode` markers (`.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`) now list **SRS-REQ-070**.

- **C-01 — Minor — false V&V evidence count in `docs/vv/verification-results.md` (N8/N11).** Line **247** (M7 SDLS section, "### Unit tests (Tst)") now reads "8/8 suites; `sdls` **82 checks**". The actual SDLS suite reports **81 checks** (`./build/native/tests/test_sdls` → `checks=81 failures=0`), and `tool-qualification.md:96` (rev 0.1.7) and the M7 cycle row (`verification-results.md:29`, "sdls 81") both record 81. The increment's diff changed exactly this "81" → "82", i.e. an **unrelated historical count was overwritten** (collateral of the `test_block` 82-check count). A V&V results record must not carry a count that no longer reproduces. Fix: restore `sdls` to **81 checks** in `:247`. Owner: authoring agent. Sprint: **S5 delta (pre-commit)**.
- **C-02 — Comment — `verification-results.md:247` "8/8 suites".** Pre-existing M7-era statement (the ctest set is now 22 tests / 13 unit suites). Not introduced by this increment, but since line 247 is being corrected it should be qualified as historical (e.g. "8/8 suites at M7"). No correction strictly required.
- **C-03 — Comment — `test_block.c` mock does not observe the `ErrorNormalStatusEnable` (0x234) write.** `mock_write32` ignores 0x234 (default case, `test_block.c:192-194`), so the R1-03 fix is not asserted by any test; a regression that dropped the 0x234 write would pass. Recommend the mock record the last enable value and a test assert it contains the completion + error bits. Advisory (R1-03's fix is correct by inspection).

## 6. Part D — PA (product assurance, N20) findings

- **Records / evidence (N11).** The increment adds the S5 row to `verification-results.md` (rev 0.37), the T3/T17 re-qualification to `tool-qualification.md` (rev 0.3.20), the S5 entry to `CHANGELOG.md`, the GAP-A5 block residual, and the S5 history rows to every touched document. Records are configuration-identified, dated and consistent — **except** the corrupted `sdls` count (**C-01**) and the two residual doc inconsistencies (**B-01**, **B-02**).
- **Claim discipline.** SRS-REQ-070, the driver header limitations, the README/SDD/DDJF scope statements, the Renode scope-limit note and the GAP-A5 entry are **consistent and free of over-claim**: "polled buffer mode only", "no SD card attached → register round-trips only", "transfer path host-tested", "on-target SD clock/bus-width and card init = GAP-A5". The Renode PASS marker is claimed as a real run (norm N18) and was reproduced first-hand. The `ci-renode` marker lists SRS-REQ-070 (documentation-level; `trace.py` validates only the *claim name*, so the per-requirement list inside the marker is not machine-gated — noted, not a finding).
- **Residual-gap register (N5/N19).** GAP-A5 gains the MSS block-device on-target residual (SD clock/bus-width, card init, polled buffer mode) at `class-a-gaps.md:114-117`; GAP-A1..A14 unchanged; `trace.py` reports 14 gaps. The R2-02 oracle-independence concern (no independent oracle for the transfer/status path; register offsets only cross-checked by Renode) remains an open, correctly-tracked residual.
- **Semaphores.** `status.json` gates and the README panel are all GREEN, including `documentation_review`, `product_assurance_review` and `reviews`, while the block review loop is **still open** (FRICK-REVIEW-0146 REJECTED at the time of that report; R4/PA reviews only now being performed). The semaphores reflect the *last closed* cycle (S4) and must be refreshed (with the review-log rows) in the corrective commit.
- **D-01 — Minor — review-log rows for this increment are not yet present (N11/N20).** `docs/reviews/README.md` has **no** row for FRICK-REVIEW-0146 or FRICK-REVIEW-0147 (its last row is the Sprint-4 SPI R4+PA delta closure). Norm N20 requires the reports to be logged in `docs/reviews/README.md`; the reports themselves are present as untracked files. Owner: authoring agent/coordinator. Sprint: **S5 commit (same cycle)**. (This is a close-out step of the loop, not a product defect; it does not by itself block the technical conclusion.)
- **D-02 — Comment — semaphore refresh.** As above: refresh `status.json`/README semaphores at commit so the `reviews`/`documentation_review`/`product_assurance_review` states match the closed block loop (and the `docs/reviews/README.md` count). Advisory.
- **D-03 — Comment — `docs/reviews/README.md` review-stream metric.** The README "Review streams closed 31/36" is derived from the review log; adding the block rows will change it. `metrics.py --check` is current **against the present log**, so no action now — confirm after the log rows are added.

**What a qualified human must still accept (PA statement).** This agent cannot certify, approve, waive, accept residual risk or authorise release. The following require independent qualified human acceptance:

1. **Residual risk of the on-target block path (GAP-A5)** — the SD clock/bus-width programming, card initialisation/type and the real FIFO-based SDHCI transfer path are **not** verified by any evidence in this repository; the host mock is a behavioural fake (not an independent oracle) and the Renode check is register round-trips only.
2. **The declared design limitations** — polled buffer mode, no DMA/ADMA/CQ/SDIO/PHY tuning, no error recovery (no CMD12/bus reset), non-re-entrant single instance, `flush()` a no-op, no card init — accepted as bounded reference-backend scope (SRS-REQ-070 states them; GAP-A5 tracks the on-target residuals).
3. **The residual error-bit-map uncertainty** (FRICK-REVIEW-0146 R1-C1: model-only `ResponseError` b27 outside the mask) — confirm against `mss_sd_regs.h` on target.
4. **Closure of the four Minor findings** (B-01, B-02, C-01, R2-01-R) and of the two open-loop items (D-01, C-02/C-03/D-02/D-03 comments) before commit/push (N12/N7).

## 7. Scope conformance (standing rule)

**Implemented content vs documented scope.**

- **Within scope, no over-implementation.** The driver implements exactly the `FF_BlockDev_t` contract plus a driver-specific `Setup`/`SelfTest` (outside the vtable, consistent with the sibling backends). No DMA/ADMA, command queuing, SDIO, PHY tuning, card init or interrupt path is implemented — matching SRS-REQ-070 and the header limitations. The additional register `#define`s are map documentation only. The MMIO self-test is a documented bring-up check with disclosed side effects. `docs/roadmap.md` M4, `docs/development-plan.md` OBJ-10/OBJ-19 and `docs/planning/sprint-plan.md` S5 list the MMC/SD block-device class; the implementation matches that scope.
- **Contract conformance.** `FF_BlockDev_t` satisfied (B-CONTRACT); the SRS-REQ-070 failure mapping is implemented and host-tested; the `block_size ≤ 2048` bound now matches the 12-bit TBS field in code, but two header comments still advertise 4096 (**B-01**).
- **Deviation found → none new (scope NC).** The FRICK-REVIEW-0146 **SCOPE-01** plan-currency deviation is **closed** (`development-plan.md` M4/M11 rows + history 0.22, §3). Two *documentation-consistency* deviations remain (**B-01** header comments; **B-02** loop-audit M4 row) — these are doc-currency defects (N8/N19), **not** out-of-scope implementation.

**Scope NC: none** (SCOPE-01 closed; B-01/B-02 are documentation-consistency Minors, not scope deviations).

## 8. Findings summary

| # | Severity | Profile | Item | Finding | Evidence | Required correction |
| - | -------- | ------- | ---- | ------- | -------- | ------------------- |
| B-01 | **Minor** | R3/R4 | `ff_block_mpfs.h` block-size contract | `FF_MMC_BLOCK_SIZE_MAX = 2048` but the struct comment (`:248`) and the `Setup` doc comment (`:256`) still say `512..4096` | `ff_block_mpfs.h:231` vs `:248,:256` | Update both comments to `[512, 2048]` (N8) |
| B-02 | **Minor** | R3/R4/PA | `loop-audit.md` M4 row | Row asserts "MSS block-device done (SRS-REQ-070)" **and** "MMC… pending" (self-contradiction; M11 row says BLOCK done) | `loop-audit.md:27` vs `:34` | Remove `MMC` from the pending list (N8/N19) |
| C-01 | **Minor** | R4 | `verification-results.md:247` | `sdls` recorded as **82 checks**; actual `test_sdls` = **81** (unrelated historical count overwritten) | `verification-results.md:247` vs `test_sdls` output / `:29` / `tool-qualification.md:96` | Restore `sdls` to 81 checks (N8/N11) |
| R2-01-R | **Minor** | R2 | `test_block.c` discrimination | The stale-status/timeout-recovery regression test (second half of FRICK-REVIEW-0146 R2-01) is absent; the mock zeroes `err_status` in `never_ready` mode, so the R1-02 clear is untested | `test_block.c:92-94,386-394`; `ff_block_mpfs.c:69-72` | Pre-set a stale status bit, model it, and assert the driver clears it after `FF_TIMEOUT` |
| D-01 | **Minor** | PA | Review log (N11/N20) | No `docs/reviews/README.md` row for FRICK-REVIEW-0146/0147 | `docs/reviews/README.md` last row = Sprint-4 SPI closure | Add both rows in the corrective commit |
| C-02 | Comment | R4 | `verification-results.md:247` | "8/8 suites" is an M7-era count (now 22 ctest tests) | `:247` | Qualify as historical (optional) |
| C-03 | Comment | R2/PA | `test_block.c` mock | The 0x234 enable write is ignored by the mock → R1-03 untested | `test_block.c:192-194`; `ff_block_mpfs.c:84` | Record/assert the enable value (advisory) |
| B-03 | Comment | R3 | `development-plan.md:310` | "eMMC/storage pending" while the controller is "eMMC/SD" | `:310`; `ff_block_mpfs.h:6` | Clarify which eMMC features remain (optional) |
| D-02 | Comment | PA | Semaphores | `status.json`/README show reviews/docs/PA GREEN while the block loop is open | `status.json` gates; README panel | Refresh at commit |
| D-03 | Comment | PA | Review-stream metric | 31/36 derived from the log; will change with the block rows | README panel; `metrics.py` | Re-confirm after the log rows |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier. *(None open.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified. *(B-01, B-02, C-01, R2-01-R, D-01.)*
- **Comment**: observation/recommendation, no derogation. *(C-02, C-03, B-03, D-02, D-03.)*

## 9. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The single Major finding of FRICK-REVIEW-0146 (**R1-01**) is **closed**: the block-size
contract now excludes 4096, a real compile-time `FF_STATIC_ASSERT` guards the TBS field, `Setup`
rejects 4096, and a discriminating regression test (4096 → `FF_INVALID_ARGUMENT`, 2048 → success)
covers it; the full gate set (22/22 native and ASan, `test_block` 82 checks, `trace.py` PASS
114/71/29/14, `metrics --check` current, clang-format clean, Renode `RENODE_TEST: PASS` incl.
`hal_block=PASS`, QEMU PASS, `lake build` PASS) was reproduced first-hand with no regression. R1-02,
R1-03 and SCOPE-01 are closed. Five **Minor** items remain — two documentation-consistency defects
(**B-01**, **B-02**), one false V&V count (**C-01**), one residual test-coverage item (**R2-01-R**)
and one records close-out step (**D-01**) — none of which invalidates the requirement or a safety
barrier. Because no Major is open, the delta is **APPROVED WITH COMMENTS**; per norm N12 the five
Minors must be closed and the deltas re-reviewed (same profiles, fresh context) before commit/push,
and the review-log rows (D-01) added in the same cycle (norms N7/N11/N20). R3 (N14) and R4 (N17) find
no requirement or documentation defect that blocks the increment; PA (N20) records the residual
on-target risk for human acceptance (§6).

## 10. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1+R2 (delta) (N9) | Senior FSW engineer **and** Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — Major R1-01 closed; R1-02/R1-03/SCOPE-01 closed; R2-01 partial (R2-01-R) |
| R3 (N14)      | Senior avionics / systems engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — SRS-REQ-070/contract/trace conform; B-01/B-02/B-03 |
| R4 (N17)      | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — B-01/B-02/C-01/C-02 |
| PA (N20)      | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — records/semaphores/claims; D-01/D-02/D-03; residual risk for human acceptance |

## 11. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| R1-01 (0146) | **Major** | SRS-REQ-070 block size | 4096 truncated to TBS=0 | Fixed: `MAX=2048`, static assert, Setup rejects 4096, regression test | `ff_block_mpfs.h:231`; `ff_block_mpfs.c:24,259-262`; `test_block.c:255-258`; `test_block` 82 checks | frick-fsw | **closed** | this report (§3) | S5 delta |
| R1-02 (0146) | Minor | SRS-REQ-070 timeout | status uncleared on timeout | Fixed: clear whole status word on timeout | `ff_block_mpfs.c:69-72` | frick-fsw | **closed** (code; test residual R2-01-R) | this report (§3) | S5 delta |
| R1-03 (0146) | Minor | SRS-REQ-070 completion poll | 0x234 not programmed | Fixed: program `ErrorNormalStatusEnable` with completion+error bits | `ff_block_mpfs.c:82-84` | frick-fsw | **closed** (code; test residual C-03) | this report (§3) | S5 delta |
| R2-01 (0146) | Minor | V&V test discrimination | mock cannot discriminate 4096/stale status | 4096 + 2048 boundary tests added; stale-status test still missing | `test_block.c:253-258`; `test_block` 82 checks | frick-fsw | **partial** → R2-01-R | this report (§3) | S5 delta |
| SCOPE-01 (0146) | Minor | Scope/N8/N16/N19 | `development-plan.md` stale | Fixed: M4/M11 rows + history 0.22 | `development-plan.md:310,317,355` | frick-fsw | **closed** — with B-02 | this report (§3) | S5 delta |
| **B-01** | Minor | `ff_block_mpfs.h:248,256` | block-size comments still `512..4096` | Update both comments to `[512, 2048]` | (delta) header diff | frick-fsw | **open** | pending | **S5 delta (pre-commit)** |
| **B-02** | Minor | `loop-audit.md:27` | M4 row asserts MMC done **and** pending | Remove `MMC` from the pending list | (delta) `loop-audit.md` diff | frick-fsw | **open** | pending | **S5 delta (pre-commit)** |
| **C-01** | Minor | `verification-results.md:247` | `sdls` recorded 82 checks; actual 81 | Restore `sdls` to 81 checks | (delta) `verification-results.md` diff + `test_sdls` output | frick-fsw | **open** | pending | **S5 delta (pre-commit)** |
| **R2-01-R** | Minor | `test_block.c` | stale-status/timeout-recovery test absent; mock zeroes the stale bit | Add the test (pre-set a stale status bit; assert the clear after `FF_TIMEOUT`) | (delta) `test_block.c` diff + failing-pre-fix/passing-post-fix run | frick-fsw | **open** | pending | **S5 delta (pre-commit)** |
| **D-01** | Minor | `docs/reviews/README.md` | no log row for FRICK-REVIEW-0146/0147 | Add both rows in the corrective commit | (delta) `README.md` diff | frick-fsw | **open** | pending | **S5 commit (same cycle)** |
| C-02, C-03, B-03, D-02, D-03 | Comment | — | Advisory (§5/§6) | Accepted / optional | — | frick-fsw | n/a | — | — |

## 12. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native build | `cmake --build build/native` | up to date (green) |
| Native ctest | `ctest --test-dir build/native` | **22/22 pass**, incl. `block` |
| ASan/UBSan ctest | `ctest --test-dir build/asan` (rebuilt) | **22/22 pass**, incl. `block` |
| `test_block` checks | `./build/native/tests/test_block` (rebuilt after `touch`) | `checks=82 failures=0` |
| SDLS checks (C-01) | `./build/native/tests/test_sdls` | `checks=81 failures=0` (doc says 82) |
| Traceability | `trace.py docs/requirements/traceability.yaml` | **PASS (114 requirements, 71 srs, 29 sss, 14 gaps, 51 lean theorems, 12 CI claims)** |
| Metrics panel | `metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror` on `ff_block_mpfs.{c,h}`, `test_block.c`, `app_renode_test.c`, `platform_mpfs.h` | clean (exit 0) |
| Warning hygiene | strict set `-Werror -fsyntax-only` on the driver | clean (exit 0) |
| Renode functional run (N18) | `make renode` | **PASS** (`RENODE_TEST: PASS`; `hal_gpio…hal_block=PASS`) |
| QEMU emulated target | `make qemu` | PASS (banner + 6 applications registered) |
| Lean (N1) | `lake build` in `frick-vv:local` | `Build completed successfully` |
| Static asserts | `FF_STATIC_ASSERT` macro (`ff_types.h:41`) + driver asserts | real compile-time asserts; `512 ≤ 2048 ≤ 4095` |
| FORBIDDEN constructs | `grep -nE '\bwhile\b\|\bgoto\b\|\bmalloc\b\|\bcalloc\b\|\brealloc\b\|\bfree\('` | none in code (one word in a comment) |
| SPDX headers (N13) | grep in the three new files | `AGPL-3.0-only` present |
| Contract | `hal_mem.h:65-74` vs `ff_block_mpfs.c:266-277` | all members populated; no mismatch |
| R1-01 fix | `ff_block_mpfs.h:231`, `.c:24,259-262`, `test_block.c:255-258` | 4096 rejected; 2048 accepted; static assert present |
| R1-02 fix | `ff_block_mpfs.c:69-72` | timeout clears `FF_MMC_INT_CLEAR_ALL` |
| R1-03 fix | `ff_block_mpfs.c:82-84` | 0x234 programmed with `FF_MMC_INT_DONE \| FF_MMC_INT_ERROR_MASK` |
| SCOPE-01 fix | `development-plan.md:310,317,355` | M4 BLOCK done; M11 9/13; history 0.22 (header matches) |
| B-01 | `ff_block_mpfs.h:248,256` | stale `512..4096` |
| B-02 | `loop-audit.md:27` vs `:34` | M4 row self-contradiction |
| C-01 | `verification-results.md:247` vs `:29`, `tool-qualification.md:96` | `sdls` 82 vs 81 |
| N16 header/history | 12 touched docs | header `Revision`/`Date` = last history row, in order |
| ci-renode markers | both workflows | SRS-REQ-070 listed |

> Note: the gate commands regenerate build artifacts under `build/` (git-ignored) only; **no tracked
> source or documentation file was modified** by this review. The reviewer wrote **exactly one file**
> (this report); nothing was committed or pushed.

## 13. Norm-by-norm audit

| Norm | Status | Evidence / note |
| ---- | ------ | --------------- |
| N1 Lean | **pass** | `lake build` PASS; SRS-REQ-070 carries an explicit `lean_note` (no silent waiver). |
| N2 English | **pass** | All new/changed content is English. |
| N3 backwards SRS/SSS | **pass** | SRS-REQ-070 added, parented to SSS-REQ-020; chain verified. |
| N4 traceability | **pass** | `trace.py` PASS 114/71/29/14, top-down + bottom-up converged. |
| N5 class-B/A gaps | **pass** | GAP-A5 block residual recorded; GAP-A1..A14 unchanged. |
| N6 V&V + deployed env | **pass** | CI + V&V image + Renode; `ci-renode` markers updated. |
| N7 dual push | **n/a (pre-commit)** | Change is uncommitted; commit/push blocked until N12 close-out. |
| N8 docs current | **partial** | B-01 (stale block-size comments), B-02 (loop-audit M4 row), C-01 (false sdls count). |
| N9 independent review | **pass** | This report (fresh context, independent profiles). |
| N10 test scripts/tools | **pass** | T3 gains `test_block.c`, T17 gains `hal_block`; re-qualification registered; C-03 advisory. |
| N11 records in repo | **partial** | Results/reports present; **D-01** log rows pending; **C-01** count wrong. |
| N12 NC closure | **in progress** | Five Minors open, tracked with owner + sprint; deltas to be re-reviewed. |
| N13 licensing | **pass** | SPDX `AGPL-3.0-only` in the three new files. |
| N14 SSS review | **pass** | R3 finds no requirement/scope defect (B-01/B-02 are doc-currency). |
| N15 full cycle | **pass (this cycle)** | Build/tests/Lean/trace/Renode/QEMU reproduced green. |
| N16 revision history | **pass** | All touched docs' header = last history row; CHANGELOG S5 entry present. |
| N17 documentation review | **pass with comments** | R4 performed; B-01/B-02/C-01/C-02. |
| N18 Renode functional | **pass** | `make renode` → `RENODE_TEST: PASS` incl. `hal_block=PASS` (reproduced). |
| N19 loop audit | **partial** | `loop-audit.md` refreshed but its M4 row self-contradicts (**B-02**). |
| N20 PA review | **pass with comments** | PA performed; D-01/D-02/D-03; residual risk flagged for human acceptance. |

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- R1-01 fix is present and effective: `FF_MMC_BLOCK_SIZE_MAX = 2048`, `FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK, …)`, `Setup` rejects `>2048`, and `test_block` asserts 4096 → `FF_INVALID_ARGUMENT` / 2048 → success.
- R1-02 fix is present: the timeout path clears the whole status word (`FF_MMC_INT_CLEAR_ALL`).
- R1-03 fix is present: `block_issue` programs `ErrorNormalStatusEnable` (0x234) with the completion + error bits before issuing.
- SCOPE-01 is closed: `development-plan.md:310,317` (BLOCK done, M11 9/13) and history row 0.22 (header matches).
- `test_block` reports **82 checks** (74 assert sites; `mock_bind`'s assert runs 9×); `test_sdls` reports **81 checks**, while `verification-results.md:247` says 82.
- `ff_block_mpfs.h:248,256` still document `512..4096` against `FF_MMC_BLOCK_SIZE_MAX = 2048`.
- `loop-audit.md:27` asserts both "MSS block-device done" and "MMC… pending".
- `FF_BlockDev_t` (`hal_mem.h:65-74`) is fully populated by `Setup`; no contract mismatch.
- All gates reproduce: 22/22 native + ASan, `trace.py` PASS 114/71/29/14 (51 theorems), `metrics --check` current, clang-format clean, strict-warning clean, Renode PASS incl. `hal_block=PASS`, QEMU PASS, `lake build` PASS.
- `docs/reviews/README.md` has no row for FRICK-REVIEW-0146/0147.

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0147`* — owner: review lead; impact: report identity only.
- *The uncommitted working-tree delta is the item under review atop `737c805`.*
- *On real SDHCI the normal/error status bits are set regardless of the enable register* — owner: hardware/mission (GAP-A5); validation: `mss_sd_regs.h` on target.
- *The 12-bit TBS width is authoritative* — validation: confirm on silicon (GAP-A5).

**Inferences**

- Because the code, the bound macro and the test now agree on `≤2048` while two header comments still advertise 4096, an integrator following the comments would receive `FF_INVALID_ARGUMENT` (B-01) — a documentation hazard, not a code defect.
- Because the mock zeroes `err_status` in `never_ready` mode, the R1-02 timeout-clear cannot be discriminated by the current test suite (R2-01-R); a regression dropping the clear would pass unnoticed.
- Because `verification-results.md:247` records `sdls` 82 checks while the suite reports 81 and every other record says 81, the count was overwritten by a blind replace during this increment (C-01).
- Because `loop-audit.md`'s M4 row retains `MMC` in the pending list after recording the block device done, the M4/M11 inconsistency class recurs (B-02).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. Close **B-01** (both header comments → `[512, 2048]`) and **B-02** (drop `MMC` from the loop-audit M4 pending list) in the corrective commit; re-run `metrics --check` and the doc-consistency checks.
2. Close **C-01** (restore `sdls` 81 checks) and confirm `verification-results.md` M7 section consistency (C-02).
3. Close **R2-01-R**: extend the mock to preserve/observe a pre-set status bit and add a test asserting the driver clears it after `FF_TIMEOUT`; optionally add a C-03 assertion for the 0x234 write.
4. Add the `docs/reviews/README.md` rows for FRICK-REVIEW-0146/0147 and refresh `status.json`/README semaphores (D-01/D-02/D-03) in the same cycle (N11/N20).
5. Keep the GAP-A5 on-target residuals (SD clock/bus-width, card init, error-bit map incl. b27, wall-clock latency) and the R2-02 oracle-independence concern tracked; obtain a datasheet-derived oracle for the transfer/status path before any flight claim.
6. Re-review the deltas (same profiles, fresh context) before commit/push (N12/N7).

**Confidence:** **High** for the R1-01/R1-02/R1-03/SCOPE-01 closures and the gate results (all reproduced first-hand, internally consistent, and arithmetically checked). **High** for B-01, B-02 and C-01 (direct source reads + reproducible test counts). **Medium** for R2-01-R's significance (it rests on the mock's design and on the on-target semantics of the timeout path, a GAP-A5 residual). Confidence is not a probability of safety.

**Open gaps**

- **B-01, B-02, C-01, R2-01-R (Minor)** open — must be closed before commit/push (N12).
- **D-01** — review-log rows for FRICK-REVIEW-0146/0147 not yet present (N11/N20).
- **No independent oracle** for the transfer/status path (FRICK-REVIEW-0146 R2-02); register offsets only are cross-checked (by Renode).
- **On-target evidence absent** — SD clock/bus-width, card init, error-bit map (incl. b27), wall-clock latency (GAP-A5); the residual risk is for **human acceptance**.
- **Declared design limitations** (polled buffer mode, no DMA/CQ/SDIO/PHY, no error recovery, non-re-entrant, `flush()` no-op) — accepted as bounded reference-backend scope; human acceptance of the residual risk required.

---

*This report is authored to be committed and pushed with the reviewed change (norm N11). The review was read-only with respect to repository content: the reviewer wrote exactly one file (this report); no other tracked file was modified, created or deleted, and nothing was committed or pushed. The gate commands regenerate only git-ignored `build/` artifacts.*
