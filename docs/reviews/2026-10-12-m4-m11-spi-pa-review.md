# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0144                      |
| Reviewer session                  | independent agent (norm N9/N20), fresh context |
| Reviewer profile                  | **PA** — Senior SW Product Assurance engineer |
| Reference revision                | `5ca2045` (HEAD) + **uncommitted** Sprint-4 working-tree increment (reference MSS SPI backend, SRS-REQ-069) |
| Items under review                | `src/drivers/spi/mpfs/ff_spi_mpfs.{c,h}` (new); `tests/unit/test_spi.c` (new); SPI deltas in `src/apps/app_renode_test.c`, `config/platform/platform_mpfs.h`, `CMakeLists.txt`, `tests/CMakeLists.txt`; requirement **SRS-REQ-069** and its chain in `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`; `docs/design/software-design.md`; `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/quality/class-a-gaps.md`; `docs/engineering/software-description.md`; `docs/planning/sprint-plan.md`; `docs/development-plan.md`; `docs/status/{handover.md,loop-audit.md,status.json}`; `CHANGELOG.md`; `README.md`; `docs/reviews/README.md`; the prior review set `docs/reviews/2026-10-12-m4-m11-spi-{r1-r2,delta-r3-r4}-review.md`; the `ci-renode` markers in both CI workflows |
| Date                              | 2026-10-12                             |
| Scope                             | Independent **Product-Assurance review of every phase product** (norm N20) of the Sprint-4 reference MSS SPI increment: the implemented content vs the documented scope (standing scope-conformance rule); existence, logging and closure of the review set; truthfulness of the `status.json` semaphores; header↔`## History` agreement (N16); count consistency (113/70/29/14/51; `test_spi` = 120 checks); absence of stale "absorbed"/old counts outside historical rows; absence of capability over-claim; consistency of the SPI scope/limitation statements (register round-trips only in Renode; transfer path host-tested); residual-gap tracking (GAP-A5 SPI SCL/APB; STATUS b11 name); and independent re-execution of the applicable gates. Assessed against ECSS-E-ST-40C Rev.1 (software development/V&V, information/data items), ECSS-Q-ST-80C Rev.2 (software product assurance, records, tool qualification) and the project norms N1–N20 (`docs/rules.md` 1.4). |
| Independence declaration          | The reviewer had **no part** in authoring, executing, building-for-delivery, or previously reviewing the items under review; this review ran in a fresh context and every declared claim was re-executed or read first-hand (see §3). The reviewer wrote exactly one file — this report — and modified no other repository file. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- The new driver `src/drivers/spi/mpfs/ff_spi_mpfs.c` / `.h`: register map and bit
  positions, `init`/`transfer`/`cs_assert`/`cs_release`, CLKRATE derivation,
  argument/`NULL` validation, pre-init behaviour, the bounded status poll and the
  timeout path (incl. the R1-01/R1-02/D-01 fixes), the MMIO self-test and the
  documented limitations.
- The new unit test `tests/unit/test_spi.c` (behavioural 32-bit mock; **120**
  checks) and whether the R1-01/R1-02 regressions genuinely discriminate.
- The SPI functional check in `src/apps/app_renode_test.c`
  (`mpfs_spi_read32/write32`, `renode_hal_spi_check`, `hal_spi=PASS`) and its term
  in the `RENODE_TEST` verdict — specifically that it is register round-trips only.
- Build/test wiring (`CMakeLists.txt`, `tests/CMakeLists.txt`) and platform
  constants `FF_SPI0_BASE`/`FF_SPI1_BASE`.
- Requirement/evidence chain: SRS-REQ-069 (parent SSS-REQ-020),
  `traceability.yaml`/`traceability.md`, `verification-results.md`,
  `tool-qualification.md` (T3/T17), `class-a-gaps.md` (GAP-A5),
  `development-plan.md`, `sprint-plan.md`, `handover.md`, `loop-audit.md`,
  `status.json`, `software-description.md`, `README.md`, `CHANGELOG.md`, both
  `ci-renode` markers.
- The review set `FRICK-REVIEW-0142` / `FRICK-REVIEW-0143`: existence, log rows
  (`docs/reviews/README.md`), and closure of every finding first-hand.
- **Scope conformance** (standing rule): implemented content vs the documented
  scope (SRS-REQ-069, `hal_spi.h`, `docs/roadmap.md` M4, `docs/development-plan.md`
  OBJ-10/OBJ-19, `docs/vv/renode-environment.md`) — see §6.
- Independent re-execution of the applicable gates (§3).

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour (configured MSS SPI APB clock, per-rate SCL
  accuracy, real-slave timing/eye, FIFO depth, wall-clock latency) — tracked to
  GAP-A5 and not verifiable here.
- The engineering/technical content already reviewed by R1/R2/R3/R4 (0142/0143);
  this report re-verifies their *closure claims* and assesses the PA-relevant
  records/scope/truthfulness dimensions, it does not re-derive the register map or
  the divider numerics from the Microchip headers (not revision-controlled here).
- The `docs/reviews/README.md` log row for **this** report (owner: authoring
  agent/coordinator, norm N11).

**Assumption recorded:** the assigned document id `FRICK-REVIEW-0144` was supplied
by the review lead; the highest id observed in `docs/reviews/` is
`FRICK-REVIEW-0143`, so the id is taken as assigned and not re-derived here.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract:
  `docs/engineering/independent-review.md` (rev 0.7).
- Report format: `docs/reviews/review-template.md`.
- Prior reviews: `docs/reviews/2026-10-12-m4-m11-spi-r1-r2-review.md`
  (`FRICK-REVIEW-0142`, REJECTED) and
  `docs/reviews/2026-10-12-m4-m11-spi-delta-r3-r4-review.md`
  (`FRICK-REVIEW-0143`, APPROVED WITH COMMENTS).
- Contract: `src/hal/hal_spi.h` (`FF_Spi_t`).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-069, SRS-REQ-031),
  `docs/requirements/SSS.md` (SSS-REQ-020), `docs/requirements/traceability.yaml`,
  `docs/requirements/traceability.md`.
- Scope documents: `docs/roadmap.md` (M4, M11), `docs/development-plan.md`
  (OBJ-10, OBJ-19), `docs/planning/sprint-plan.md`, `docs/vv/renode-environment.md`.
- Gaps: `docs/quality/class-a-gaps.md` (GAP-A5).
- Evidence: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`
  (T3/T17), `CHANGELOG.md`, `README.md`, `docs/status/*`.
- Authoritative hardware oracle: Renode `SPI.MPFS_SPI` / Microchip CoreSPI
  register map and reset values (SPI0 `@0x20108000` PLIC 54, SPI1 `@0x20109000`
  PLIC 55).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Verification performed (evidence ledger — all obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native build (working tree) | `cmake --build build/native` | up to date (green; 25/25 targets linked) |
| Native ctest | `ctest --test-dir build/native` | **21/21 pass**, incl. `spi` |
| ASan/UBSan ctest | `ctest --test-dir build/asan` | **21/21 pass**, incl. `spi` |
| `test_spi` checks | `./build/native/tests/test_spi` | **`checks=120 failures=0`** |
| Traceability | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS (113 requirements, 70 srs, 29 sss, 14 gaps, 51 lean theorems)**; SSS-REQ-020 → 14 SRS children |
| Metrics panel | `python3 tools/scripts/metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror` on `ff_spi_mpfs.{c,h}`, `test_spi.c`, `app_renode_test.c` (clang-format 23.1.3) | clean (exit 0) |
| Renode functional validation (N18) | `make renode` (RTEMS ELF present → gate **in force**) | **PASS** (`RENODE_TEST: PASS`; console shows `hal_spi=PASS`, `hal_i2c=PASS`, …, `sb_roundtrip=PASS`) |
| QEMU emulated target | `make qemu` (T20, `frick-rtems-toolchain:local`) | PASS (banner + 6 applications registered) |
| Lean (N1) | `lake build` in `verification/lean` via `frick-vv:local` | Build completed successfully |
| RTEMS cross build | `cmake` + `cmake --build` via `frick-rtems-toolchain:local` (`FF_PLATFORM=rtems`) | links `frick_fsw` (100%, green) |
| MISRA (T14, advisory) | `make misra-docker` | 13 findings, **0 in the SPI files** (reproducible) |
| R1-01 fix (source) | read `ff_spi_mpfs.c:119-139` | `RXDATA` read on **every** frame, discarded when `rx == NULL` (`:133-136`) |
| R1-01 regression | read `test_spi.c:143-150,357-377` | non-overwriting one-deep RX FIFO + `test_transfer_drains_rx` (returns the **new** frame) |
| R1-02 fix | read `ff_spi_mpfs.c:141-172`; `test_spi.c:379-392` | `cs_assert`/`cs_release` gate on `spi_check_ready()` → `FF_NOT_READY` pre-init; `test_cs_preinit` asserts it |
| R2-02 fix | read `docs/requirements/SRS.md:333-334` | "Validated by a host unit test (behavioural MMIO mock)" |
| SCOPE-01 fix | read `docs/development-plan.md:310,317` | M4 row: "MSS SPI done; MMC/eMMC/storage/PCIe/SpaceWire pending"; M11 row: "8/13 … `hal_spi=PASS`" |
| D-01 (timeout drain) | read `ff_spi_mpfs.c:122-127` vs the in-code comment | single `RXDATA` read on `FF_TIMEOUT`; comment claims "a timeout cannot leave a stale byte" → not established (PA-04) |
| B-01 (check count) | `grep -rn 109 docs/ README.md CHANGELOG.md` (excl. reviews/history) | none stale; `test_spi` 120 checks recorded in `verification-results.md:48`, `sprint-plan.md`, `handover.md:212`, `CHANGELOG.md:19` |
| B-02 (history order) | read `docs/development-plan.md` header + tail | order 0.19 → 0.20 → 0.21; header `0.21` = last row |
| B-03 (sprint-plan S4) | read `docs/planning/sprint-plan.md:45,109` | S4 "— **delivered**", cites SRS-REQ-069, exit "`hal_spi=PASS` (met); M11 8/13" |
| Header↔History (N16) | script over `| Revision`/`| Date` + last `## History` row | **all agree** for SRS 1.19, traceability.md 0.31, SDD 0.7, software-description 0.4, verification-results 0.36, renode-environment 0.15, tool-qualification 0.3.19, class-a-gaps 0.9, handover 1.11, loop-audit 0.15, sprint-plan 0.7, development-plan 0.21 |
| Count consistency | `grep` across README/`traceability.md`/`verification-results`/`handover`/`loop-audit`/`status.json` | 113/70/29/14/51 consistent; only historical rows carry old counts (112/69, 100/59, 20/20) |
| Stale "absorbed" wording | `grep -rni absorb docs/ README.md CHANGELOG.md` | only `## History` rows / review reports / change descriptions — none live |
| SPI scope statement | `ff_spi_mpfs.h:72-75,233-236`; `renode-environment.md:103`; `verification-results.md:48`; SRS-REQ-069 | "register round-trips only; transfer path host-tested" stated consistently in the driver/SRS/V&V row; **`renode-environment.md` §HAL coverage is stale** (PA-01) |
| SPDX (N13) | head of the three new files | `AGPL-3.0-only` present |
| Review set logged | read `docs/reviews/README.md:142-144` | rows for 0142 (REJECTED→fixed), 0143 (APPROVED WITH COMMENTS), 0144 (pending) present |
| Review reports exist | `ls docs/reviews/2026-10-12-m4-m11-spi-*.md` | 0142 and 0143 present (untracked, pre-commit) |
| `status.json` semaphores | read `docs/status/status.json:26-47` | `sprints.done=4`; all gates `green` — `product_assurance_review: green` while this PA review is pending (PA-05) |
| No other file modified | `git status --short` before/after the review | identical tracked/untracked set; gate commands regenerated only git-ignored `build/` artifacts |

## 4. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation (required correction) |
| - | ------------------------------ | ------------------ | ------- | -------- | ------------------------------------ |
| **PA-01** | **Minor** | N8/N16/N18 — V&V environment record | **`docs/vv/renode-environment.md` §HAL coverage is stale and its History row over-claims.** The section still reads "**Peripheral-bound** (7/13)" and its enumeration lists only GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C — **SPI is missing** — although the same paragraph's marker list already contains `hal_spi=PASS` and every other document records **M11 8/13**. The History row 0.15 states "note the emulated `SPI.MPFS_SPI` scope limit (no registered slave → register round-trips only; the transfer path is host-tested)", but the body has **no** SPI scope-limit paragraph (the scope-limit notes cover CAN and I2C only). The increment therefore (a) left the coverage count and enumeration stale and (b) recorded a body change that was not made. | `docs/vv/renode-environment.md:95-100` ("(7/13)"), `:103` (`hal_spi=PASS`), `:150` (history 0.15); all other docs M11 8/13 | Update the §HAL coverage count to **8/13**, add **SPI** (MSS SPI register round trips, SRS-REQ-069) to the peripheral-bound enumeration, and add the SPI scope-limit paragraph (or reword the History row to match the actual body change). Re-verify with the R4/N17 delta. |
| **PA-02** | **Minor** | N8/N16/N19 — documentation currency | **`README.md` "Current focus" is stale for the delivered increment.** The prose still says "**Sprint 3** complete … M11 peripheral-bound HAL coverage **7/13**, with `hal_i2c=PASS` … Next per plan: **Sprint 4** (SPI backend)", while this increment delivers Sprint 4 (SPI, M11 8/13, `hal_spi=PASS`). The generated metrics panel (lines 39–94) was refreshed, but this hand-maintained paragraph (outside the `METRICS` block) was not; the same defect was previously raised for the I2C cycle (R4-03) and has recurred. | `README.md:417-420`; `docs/planning/sprint-plan.md:45,109`; `docs/status/status.json` `sprints.done=4` | Advance the "Current focus" paragraph to **Sprint 4 delivered** (SPI, M11 8/13, `hal_spi=PASS`; next S5 BLOCK eMMC/SD), consistent with `handover.md`/`sprint-plan.md`. |
| **PA-03** | **Minor** | N8/N19 — internal inconsistency (loop audit) | **`docs/status/loop-audit.md` contradicts itself.** The **M4** objective row still lists "… MSS I2C done (SRS-REQ-058); **SPI**/MMC/eMMC/storage/PCIe/SpaceWire **pending**", while the **M11** row in the same table records "GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C/**SPI** done (**8/13**)". The M4 row was not updated when the M11 row was. | `docs/status/loop-audit.md:27` (M4) vs `:34` (M11) | Update the M4 row to record MSS SPI done (SRS-REQ-069) and remove SPI from the pending list; keep the N19 audit internally consistent. |
| **PA-04** | **Minor** | N8 — code-comment accuracy; D-01 disposition | **The D-01 disposition does not fully close the error-path failure mode and the comment over-claims.** On `FF_TIMEOUT` the driver reads `RXDATA` **once** *before* returning. A frame that completes *after* the poll bound (the very condition that produced the timeout) has not yet arrived at that instant, so the read cannot clear a `RXDATARCVD` set later; the one-frame RX shift flagged by R1-01 therefore remains reachable on the timeout sub-case. The in-code comment nevertheless asserts "so a timeout **cannot** leave a stale byte for the next transfer", which is not established. No host test discriminates this path (`test_transfer_timeout` uses `no_rx=1`, where no frame ever arrives). Classified **Minor** (error path; the caller already receives `FF_TIMEOUT`; the primary idiom is fixed). | `ff_spi_mpfs.c:121-128`; `test_spi.c:330-340`; FRICK-REVIEW-0143 D-01 | Either flush `RXDATA` until `RXFIFOEMPTY` **at transfer start** (robust against a late frame), or keep the single drain and (a) reword the comment to state it clears only an already-arrived frame and (b) document the required caller recovery after `FF_TIMEOUT`. Add a mock mode that completes a frame after the poll bound if the flush is to be tested. |
| **PA-05** | Comment | N8/N11 — records truthfulness | The `status.json`/README semaphore **`product_assurance_review` is `green`** (and `reviews` `green`) while the PA review of *this* increment is still **pending** (`docs/reviews/README.md:144` `(pending)`, and 0142 was REJECTED / 0143 APPROVED WITH COMMENTS with D-01 still open). The semaphore over-states the gate state at this instant. | `docs/status/status.json:36`; `README.md:82`; `docs/reviews/README.md:142-144` | Set `product_assurance_review`/`reviews` green only after this review's findings are closed and the PA delta re-review passes, or annotate the semaphore as "PA delta pending". |
| **PA-06** | Comment | N5/N8/N19 — residual-gap tracking | The driver header states the STATUS **b11 name** is "unconfirmed (**GAP-A5**)" (`ff_spi_mpfs.h:79-81`), but `docs/quality/class-a-gaps.md` GAP-A5 itemizes only the SPI CLKRATE divider/APB reference and per-rate SCL accuracy — the b11 register-map name is **not** itemized in the register. The header map line 32 also states `b11 TXFIFOEMPTYNEXT` as fact while the limitation says the Renode model names it `TXFIFOFULLNEXT`. | `ff_spi_mpfs.h:32,79-81,166`; `docs/quality/class-a-gaps.md:108-110` | Add the SPI register-map/b11-name confirmation to the GAP-A5 item; reword line 32 to mark b11 as unconfirmed. |
| **PA-07** | Comment | N16/N19 — audit record precision | The `loop-audit.md` header `Baseline commit` reads "`ce0bff3` (Sprint-3) + Sprint-4 SPI increment (audited revision)", which omits the intervening restored-M4-scope commit (HEAD `5ca2045`) that is the actual audit base. | `docs/status/loop-audit.md:9` | State the audited base as `5ca2045` + the Sprint-4 increment (or name both increments). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(none)*
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified. *(PA-01, PA-02, PA-03, PA-04.)*
- **Comment**: observation/recommendation, no derogation. *(PA-05, PA-06, PA-07.)*

## 5. Closure of the prior findings (first-hand verification)

| NC | Severity | Disposition verified first-hand | Status |
| -- | -------- | ------------------------------- | ------ |
| **R1-01** | Major | `spi_transfer` reads `RXDATA` on **every** frame and discards it when `rx == NULL` (`ff_spi_mpfs.c:133-136`); the mock is a non-overwriting one-deep RX FIFO (`test_spi.c:143-150`); `test_transfer_drains_rx` (`:357-377`) returns the **new** frame. Reasoned discrimination: a driver without the drain leaves `rx_valid=1`/`rx_fifo=0x11` and the test would read `0x11` (≠ `0x22`). | **CLOSED** |
| **R1-02** | Minor | `cs_assert`/`cs_release` call `spi_check_ready()` and return `FF_NOT_READY` pre-init (`ff_spi_mpfs.c:151-154,166-169`); `test_cs_preinit` (`test_spi.c:379-392`) asserts both, then `FF_SUCCESS` post-init. | **CLOSED** |
| **R2-01** | Minor | The mock discriminates R1-01 (non-overwriting FIFO guard + regression test); still honestly declared a behavioural fake, not an independent oracle. | **CLOSED** |
| **R2-02** | Minor | SRS-REQ-069 now reads "Validated by a host unit test (behavioural MMIO mock)" (`SRS.md:333-334`), matching `test_spi.c:10-18`. | **CLOSED** |
| **SCOPE-01 (NC-01)** | Minor | `development-plan.md` M4 row now "MSS SPI done … SPI pending removed"; M11 row "8/13 … `hal_spi=PASS`" (`:310,317`); History 0.21 present and in order. | **CLOSED** |
| **D-01** | Minor | Timeout branch drains `RXDATA` once (`ff_spi_mpfs.c:122-127`) — present, but the failure mode is not fully closed and the comment over-claims (see **PA-04**). | **OPEN** (residual, tracked as PA-04) |
| **B-01** | Minor | `test_spi` = 120 checks recorded in `verification-results.md:48`, `sprint-plan.md`, `handover.md:212`, `CHANGELOG.md:19`; no stale "109" outside historical rows. | **CLOSED** |
| **B-02** | Minor | `development-plan.md` History now 0.19 → 0.20 → 0.21; header `0.21` = last row. | **CLOSED** |
| **B-03** | Minor | `sprint-plan.md` §2 S4 row "— **delivered**", cites SRS-REQ-069, exit "`hal_spi=PASS` (met); M11 8/13". | **CLOSED** |
| B-04/B-05/B-06/B-08, LOG-01 | Comment | `status.json` `sprints.done` advanced to 4 (B-04 resolved); `software-description.md` trailing newline present (B-05); loop-audit baseline partially clarified (B-06 → PA-07); review-log rows 0142/0143/0144 added (LOG-01); "controllers" plural still present (B-08, non-blocking). | **CLOSED / tracked** |

## 6. Scope conformance (standing rule)

**Implemented content vs documented scope.**

- **Within scope, no over-implementation.** The driver implements exactly the
  `FF_Spi_t` contract (`init`, `transfer`, `cs_assert`, `cs_release`) and nothing
  beyond it: no DMA, interrupt, slave or enhanced/command framing. The additional
  register `#define`s document the map only and are unused. The MMIO self-test is a
  documented bring-up check with disclosed side effects. No out-of-scope feature
  was found.
- **Consistent with the scope documents.** `docs/roadmap.md` M4 lists SPI among
  the native MSS peripheral layers; `docs/development-plan.md` OBJ-10/OBJ-19 and
  the M11 objective cover the SPI class; the implementation matches.
- **No capability over-claim.** The SPI limitation (emulated `SPI.MPFS_SPI` has no
  registered slave → **register round-trips only**; the **transfer path is
  host-tested**) is stated consistently in the driver header, SRS-REQ-069, the
  `verification-results.md` row and the Renode check comment. The one deviation is
  the *stale* §HAL-coverage count/enumeration in `renode-environment.md`
  (**PA-01**) — a currency defect, not an over-claim.
- **Deviations found → PA-01/PA-02/PA-03** (documentation currency/inconsistency
  in `renode-environment.md`, `README.md`, `loop-audit.md`; N8/N16/N19). The
  implemented SPI is within the documented M4/M11 scope; these are records
  defects, not out-of-scope features.

No other scope deviation was identified.

## 7. Norm-by-norm audit (N1–N20)

| Norm | Requirement | Verification | Status |
| ---- | ----------- | ------------ | ------ |
| N1 | Lean verification of generated code | SPI is a bus driver with no Lean theorem; `traceability.yaml` SRS-REQ-069 carries an explicit `lean_note` justification; `lake build` PASS | compliant |
| N2 | English only | all new/changed content English | compliant |
| N3 | Backwards ECSS SRS/SSS | SRS-REQ-069 present, parented to SSS-REQ-020; SSS unchanged (no N14 trigger) | compliant |
| N4 | Bidirectional traceability | `trace.py` PASS 113/70/29/14/51; SSS-REQ-020 → 14 children; bottom-up + manifest + SDD matrix agree | compliant |
| N5 | Class B target; class-A gaps documented | GAP-A5 gains the SPI on-target residual; b11 name not itemized (**PA-06**) | compliant with a records gap (PA-06) |
| N6 | Full SW V&V; Docker-deployed environments | gates re-run; Renode/QEMU/RTEMS/Lean executed in the pinned images | compliant |
| N7 | Dual push | **not performed** — this review is a pre-commit gate; commit/push is the human/authority action | not applicable to the reviewer |
| N8 | Complete, current documentation | three stale/inconsistent records (**PA-01, PA-02, PA-03**) + comment accuracy (**PA-04**) | **partial — Minors open** |
| N9 | Independent review agents | 0142/0143 by independent profiles; this PA review independent | compliant |
| N10 | Test-script/V&V-tool review & qualification | T3 gains `test_spi.c`; T17 gains `hal_spi`; register updated 0.3.19 | compliant |
| N11 | Scripts + review results in the repo | reports 0142/0143 in `docs/reviews/`; V&V results row added; log rows present | compliant (this report's log row pending, owner = coordinator) |
| N12 | NC resolution to full compliance | 0142 Majors/Minors closed (0143); D-01 open (**PA-04**); new Minors PA-01..PA-04 to close + PA delta re-review | **in progress** |
| N13 | AGPL-3.0-only, SPDX | SPDX headers present in the three new files | compliant |
| N14 | SSS review by Senior Systems Engineer | SSS.md not modified in this increment | not triggered |
| N15 | Full cycle on every change | build/test/Lean/trace/format/Renode/QEMU/RTEMS/MISRA re-run | compliant |
| N16 | Detailed revision history | header↔History agree for every touched doc; `development-plan.md` order fixed (B-02); `renode-environment.md` 0.15 row misstates the body (**PA-01**) | compliant with PA-01 |
| N17 | R4 documentation review before commit | 0143 includes the combined R3+R4 review (APPROVED WITH COMMENTS) — but it did **not** detect PA-01/PA-02/PA-03; a R4 delta re-review is recommended | partial (see Recommendations) |
| N18 | Renode functional validation | `make renode` re-run → `RENODE_TEST: PASS` incl. `hal_spi=PASS` (RTEMS ELF present → gate in force) | compliant |
| N19 | Loop audit vs declared objectives/norms | `loop-audit.md` updated; M4 row stale (**PA-03**), baseline-commit imprecise (**PA-07**) | compliant with PA-03/PA-07 |
| N20 | Independent PA review of every phase product | this report; findings must be corrected and the review repeated until it passes | **in progress** (this report) |

## 8. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The increment is **technically sound and traceable**: the
`transfer` RX-drain fix (R1-01 Major) is effective and genuinely discriminated by
`test_transfer_drains_rx`; R1-02/R2-01/R2-02/SCOPE-01 are verified closed; the
check count is 120 everywhere; the trace chain converges at 113/70/29/14/51; the
review set exists and is logged; and every requested gate passes first-hand
(21/21 native + ASan, Renode `hal_spi=PASS`, QEMU PASS, Lean PASS, RTEMS cross
PASS, MISRA advisory 0 in the SPI files, clang-format clean, `metrics --check`
current). **No Major finding is open**, so the review is **APPROVED WITH
COMMENTS**. However, four **Minor** findings (PA-01..PA-04) and three Comments
(PA-05..PA-07) must be closed (corrected or justified) and the delta re-reviewed
per norm N12/N20 **before commit/push**: three are documentation-currency
non-conformities in files this increment touched (`renode-environment.md`,
`README.md`, `loop-audit.md`) plus the carried-over D-01 error-path residual and
its over-claiming comment. These are records/accuracy defects and do not
invalidate the requirement, but N8/N16/N19 and the N20 "no open Minor before
commit" rule require their closure.

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| **PA (N20)** | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — no Major open; Minors PA-01..PA-04 + Comments PA-05..PA-07 and residual D-01 to closure; PA delta re-review required before commit/push (N12/N20) |

## 10. Follow-up records

(norm N12: NCs are resolved by re-applying ALL norms to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| R1-01 | Major | SRS-REQ-069 `transfer` RX | Undrained RX FIFO → next transfer mis-reads | Drain `RXDATA` on every frame + discriminating test | `ff_spi_mpfs.c:133-136`; `test_spi.c:357-377`; `test_spi` 120 checks | frick-fsw | **closed** | done (0143) | S4 |
| R1-02 | Minor | SRS-REQ-069 pre-init | `cs_assert`/`cs_release` bypassed readiness | Gate on `spi_check_ready()` | `ff_spi_mpfs.c:151-154,166-169`; `test_spi.c:379-392` | frick-fsw | **closed** | done (0143) | S4 |
| R2-01 | Minor | V&V test discrimination | Mock could not model a non-overwriting FIFO | Non-overwriting FIFO + regression | `test_spi.c:143-150,357-377` | frick-fsw | **closed** | done (0143) | S4 |
| R2-02 | Minor | SRS-REQ-069 evidence wording | "master/slave" overstated the evidence | Align SRS text | `SRS.md:333-334` | frick-fsw | **closed** | done (0143) | S4 |
| SCOPE-01 | Minor | Scope conformance (N8/N16/N19) | `development-plan.md` M4/M11 stale | Update rows + history | `development-plan.md:310,317`, history 0.21 | frick-fsw | **closed** | done (0143) | S4 |
| D-01 | Minor | `transfer` timeout path | Timeout drain present but not robust; comment over-claims; untested | Flush at transfer start, or reword + document recovery (see PA-04) | `ff_spi_mpfs.c:121-128` diff + optional test | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| B-01 | Minor | R4 — evidence accuracy | `test_spi` recorded as 109; actual 120 | Update the count in the four docs | `verification-results.md:48`, `sprint-plan.md`, `handover.md:212`, `CHANGELOG.md:19` | frick-fsw | **closed** | done | S4 |
| B-02 | Minor | R4 — revision history (N16) | `development-plan.md` history out of order; header ≠ last row | Reorder; last row = header | `development-plan.md` tail (0.19→0.20→0.21) | frick-fsw | **closed** | done | S4 |
| B-03 | Minor | R4 — sprint-plan consistency | S4 row not marked delivered; cited only SRS-REQ-031 | Mark delivered; cite SRS-REQ-069; annotate exit | `sprint-plan.md:45,109` | frick-fsw | **closed** | done | S4 |
| **PA-01** | **Minor** | N8/N16/N18 — V&V environment record | `renode-environment.md` §HAL coverage "(7/13)", SPI missing from the enumeration, History 0.15 over-claims a body note | Fix count → 8/13, add SPI, add the SPI scope-limit note (or reword the history) | (delta) `renode-environment.md` diff + R4/N17 re-review | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| **PA-02** | **Minor** | N8/N16/N19 — documentation currency | `README.md` "Current focus" stale (Sprint 3, M11 7/13, next S4) | Advance to Sprint 4 delivered (SPI, 8/13, `hal_spi=PASS`; next S5) | (delta) `README.md` diff | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| **PA-03** | **Minor** | N8/N19 — loop-audit consistency | M4 row lists SPI pending; M11 row records SPI done (8/13) | Update the M4 row | (delta) `loop-audit.md` diff | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| **PA-04** | **Minor** | N8 — comment accuracy / D-01 | Timeout drain not robust; comment over-claims; untested | Flush at transfer start, or reword + document recovery | (delta) `ff_spi_mpfs.c` diff (+ optional test) | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| PA-05 | Comment | N8/N11 — records truthfulness | `product_assurance_review`/`reviews` semaphore green while the PA review is pending | Set green only after PA closure, or annotate | (delta) `status.json`/README | frick-fsw | open | — | S4 (commit) |
| PA-06 | Comment | N5/N8/N19 — residual tracking | STATUS b11 name referenced as GAP-A5 but not itemized; header map vs limitation mismatch | Add the b11 item to GAP-A5; reword the map line | (delta) `class-a-gaps.md`/`ff_spi_mpfs.h` diff | frick-fsw | open | — | S4/S5 |
| PA-07 | Comment | N16/N19 — audit record precision | loop-audit baseline-commit omits `5ca2045` | State the audited base precisely | (delta) `loop-audit.md` diff | frick-fsw | open | — | S4/S5 |
| LOG-01 | Comment | N11/N20 — records | Review-log row for FRICK-REVIEW-0144 still `(pending)` | Coordinator sets the final verdict after the delta re-review | `docs/reviews/README.md` diff | frick-fsw (coordinator) | open | — | S4 (commit) |

> This report is authored to be committed and pushed with the reviewed change
> (norm N11). The review was **read-only** with respect to repository content: the
> reviewer wrote exactly one file (this report); no other tracked file was
> modified, created or deleted, and nothing was committed or pushed. The gate
> commands regenerated only git-ignored `build/` artifacts.

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- The R1-01 Major is effectively fixed: `spi_transfer` reads `RXDATA` on every
  frame and discards it when `rx == NULL`; the mock is a non-overwriting one-deep
  RX FIFO; `test_transfer_drains_rx` returns the new frame (discriminating).
- R1-02 (readiness gate), R2-01 (mock discrimination), R2-02 (SRS wording) and
  SCOPE-01 (`development-plan.md` rows) are all closed as claimed.
- B-01/B-02/B-03 are fixed: `test_spi` = 120 checks everywhere; the
  `development-plan.md` History is ordered and header-matching; the `sprint-plan.md`
  S4 row is marked delivered and cites SRS-REQ-069.
- All touched documents' headers match their last `## History` row (N16), except
  the defects in PA-01.
- Counts are consistent at **113/70/29/14/51**; no stale "absorbed"/old counts
  outside historical rows; no capability over-claim.
- All requested gates pass first-hand: 21/21 native + ASan, `test_spi` 120 checks,
  `trace.py` PASS, `metrics --check` current, clang-format clean, Renode
  `hal_spi=PASS`, QEMU PASS, Lean PASS, RTEMS cross PASS, MISRA advisory 0 in the
  SPI files.
- `renode-environment.md` §HAL coverage still says "(7/13)" and omits SPI from the
  enumeration, while its marker list already contains `hal_spi=PASS` (PA-01).
- `README.md` "Current focus" and `loop-audit.md`'s M4 row are stale for the
  delivered increment (PA-02, PA-03).
- On `FF_TIMEOUT` the driver reads `RXDATA` once before returning; the in-code
  comment claims a timeout cannot leave a stale byte (PA-04).

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0144`* — owner: review lead; impact: report identity
  only; validation: lead renumbers if needed.
- *The uncommitted working-tree delta is the item under review atop `5ca2045`.*
- *`FF_SPI_MPFS_CLK_HZ = 100 MHz` is the reference APB clock* — owner:
  hardware/mission (GAP-A5); validation: confirm against the RTPFS160ZT datasheet.
- *`RXDATARCVD` is a level bit cleared by reading `RXDATA`* — as modelled by the
  mock and stated in the driver header; validation: CoreSPI datasheet under GAP-A5.

**Inferences**

- Because `renode-environment.md` §HAL coverage was only partially updated (marker
  list yes, count/enumeration/scope-note no) while its History row claims the
  scope note, the document is internally inconsistent and its History over-states
  the change (PA-01).
- Because the README "Current focus" and the loop-audit M4 row were not advanced
  with the delivered increment, the documentation set is not fully current (N8) at
  the instant of this review (PA-02, PA-03).
- Because the timeout drain is issued before a late frame can complete, the D-01
  error-path failure mode is reduced but not eliminated; the comment's absolute
  claim is therefore inaccurate (PA-04).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. Correct PA-01/PA-02/PA-03 and resolve/justify PA-04 (and D-01) in the
   corrective commit; re-run the full gate set (N15) and update the affected
   histories (N8/N16). Owner: authoring agent; sprint: **S4 delta (pre-commit)**.
2. Extend the **R4 (N17)** delta re-review to the three documentation-currency
   defects (PA-01/02/03) — the 0143 combined review did not detect them — and
   re-review the PA delta (N12/N20) until no Minor is open before commit/push.
3. Address PA-05 (set the PA/reviews semaphore after closure), PA-06 (itemize the
   b11 residual in GAP-A5) and PA-07 (precise audit base).
4. Keep the GAP-A5 on-target residuals (APB clock, per-rate SCL, real-bus/FIFO
   behaviour, wall-clock latency) tracked.

**Confidence:** **High** for the closure determinations of R1-01/R1-02/R2-01/
R2-02/SCOPE-01 and B-01..B-03, the count/trace convergence, the gate results and
the documentation-currency findings PA-01/PA-02/PA-03 (all reproduced first-hand
by direct source/document reads and command re-execution). **High** for PA-04's
source observation (the single read and the comment text) and **Medium** for its
on-target materiality (rests on the documented `RXDATARCVD`/FIFO semantics; the
exact post-timeout completion behaviour is a GAP-A5 residual). **Medium** for
PA-05 (the semaphore may follow a loop-baseline convention).

**Open gaps**

- **PA-01/PA-02/PA-03/PA-04 (Minors) open** — documentation currency/consistency
  and the timeout-path residual; to close before commit (N8/N12/N20).
- **D-01 (Minor) open** — timeout path does not robustly drain; comment over-claims.
- **PA-05/PA-06/PA-07, LOG-01 (Comments)** — semaphore timing, GAP-A5 b11 item,
  audit-base precision, this report's log row.
- **No independent oracle** for the transfer path (accepted limitation); register
  offsets/reset values only are cross-checked (by Renode).
- **On-target evidence absent** — APB clock, per-rate SCL accuracy, FIFO depth,
  wall-clock latency (GAP-A5).
- **This report's `docs/reviews/README.md` log row** and the final PA verdict are
  to be set by the coordinator after the delta re-review (norm N11/N20).
- **Document id** `FRICK-REVIEW-0144` retained as assigned.
