# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0145                      |
| Reviewer session                  | independent agent (norm N9/N20), fresh context |
| Reviewer profile                  | **R4 + PA (delta)** — R4 (Senior SW documentation engineer) **and** PA (Senior SW Product Assurance engineer) |
| Reference revision                | `5ca2045` (HEAD) + **uncommitted** Sprint-4 working-tree increment (reference MSS SPI backend, SRS-REQ-069) |
| Items under review                | `src/drivers/spi/mpfs/ff_spi_mpfs.{c,h}` (new); `tests/unit/test_spi.c` (new); SPI deltas in `src/apps/app_renode_test.c`, `config/platform/platform_mpfs.h`, `CMakeLists.txt`, `tests/CMakeLists.txt`; requirement **SRS-REQ-069** and its chain in `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`; `docs/design/software-design.md`; `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/quality/class-a-gaps.md`; `docs/engineering/software-description.md`; `docs/planning/sprint-plan.md`; `docs/development-plan.md`; `docs/status/{handover.md,loop-audit.md,status.json}`; `CHANGELOG.md`; `README.md`; `docs/reviews/README.md`; the prior review set `docs/reviews/2026-10-12-m4-m11-spi-{r1-r2,delta-r3-r4,pa}-review.md`; the `ci-renode` markers in both CI workflows |
| Date                              | 2026-10-12                             |
| Scope                             | Combined **R4 (documentation, norm N17) + PA (product assurance, norm N20) delta-closure** review of the Sprint-4 reference MSS SPI increment **after** the fixes for `FRICK-REVIEW-0144` (PA-01..PA-07) were applied: first-hand verification that each claimed fix is present and correct; existence/logging/closure of the review set; truthfulness of the `status.json`/README semaphores; header↔`## History` agreement (N16); count consistency (113/70/29/14/51; `test_spi` = 120 checks); absence of stale counts/wording outside historical rows; absence of capability over-claim; **scope conformance** (implemented content vs the documented M4/M11 scope — standing rule); consistency of the SPI scope/limitation statements (register round-trips only in Renode; transfer path host-tested); residual-gap tracking (GAP-A5 SPI); and independent re-execution of the applicable gates. Assessed against ECSS-E-ST-40C Rev.1 (software development/V&V, information/data items), ECSS-Q-ST-80C Rev.2 (software product assurance, records, tool qualification) and the project norms N1–N20 (`docs/rules.md` 1.4). |
| Independence declaration          | The reviewer had **no part** in authoring, executing, building-for-delivery, or previously reviewing the items under review; this review ran in a fresh context and every declared claim was re-executed or read first-hand (see §3). The reviewer wrote **exactly one file** — this report — and modified no other repository file; nothing was committed or pushed. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- The fixes claimed for `FRICK-REVIEW-0144` (PA-01..PA-07) and the carried
  finding D-01, plus the previously-closed R1-01/R1-02/R2-01/R2-02/SCOPE-01 and
  B-01..B-03 (re-verified, not re-derived).
- The new driver `src/drivers/spi/mpfs/ff_spi_mpfs.c` / `.h`: register map/bit
  positions, `init`/`transfer`/`cs_assert`/`cs_release`, CLKRATE derivation,
  argument/`NULL` validation, pre-init behaviour, the bounded status poll and the
  **timeout path (the PA-04/D-01 fix)**, the MMIO self-test and the documented
  limitations.
- The new unit test `tests/unit/test_spi.c` (behavioural 32-bit mock; **120**
  checks), including whether the PA-04/D-01 timeout-path fix is discriminated.
- The SPI functional check in `src/apps/app_renode_test.c` (register
  round-trips only) and its term in the `RENODE_TEST` verdict.
- Documentation set touched by the increment: `README.md`, `CHANGELOG.md`,
  `docs/requirements/{SRS.md,traceability.md,traceability.yaml}`,
  `docs/design/software-design.md`, `docs/development-plan.md`,
  `docs/engineering/software-description.md`, `docs/planning/sprint-plan.md`,
  `docs/quality/class-a-gaps.md`, `docs/status/{handover.md,loop-audit.md,status.json}`,
  `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`,
  both `ci-renode` markers.
- **Scope conformance** (standing rule): implemented content vs the documented
  scope (SRS-REQ-069, `hal_spi.h`, `docs/roadmap.md` M4/M11,
  `docs/development-plan.md` OBJ-10/OBJ-19, `docs/vv/renode-environment.md`) — §6.
- Independent re-execution of the applicable gates (§3).

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour (configured MSS SPI APB clock, per-rate SCL
  accuracy, real-slave timing, FIFO depth, wall-clock latency) — tracked to
  GAP-A5 and not verifiable here.
- The technical content already reviewed by R1/R2/R3/R4 (0142/0143) and PA
  (0144); this report re-verifies their *closure claims* and assesses the
  R4/PA-relevant currency/records/scope dimensions; it does not re-derive the
  register map or the divider numerics from the Microchip headers (not
  revision-controlled here).
- The `docs/reviews/README.md` log row for **this** report (owner: authoring
  agent/coordinator, norm N11).

**Assumption recorded:** the assigned document id `FRICK-REVIEW-0145` was
supplied by the review lead; the highest id observed in `docs/reviews/` is
`FRICK-REVIEW-0144`, so the id is taken as assigned and not re-derived here.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract:
  `docs/engineering/independent-review.md` (rev 0.7).
- Report format: `docs/reviews/review-template.md`.
- Prior reviews: `docs/reviews/2026-10-12-m4-m11-spi-r1-r2-review.md`
  (`FRICK-REVIEW-0142`, REJECTED), `docs/reviews/2026-10-12-m4-m11-spi-delta-r3-r4-review.md`
  (`FRICK-REVIEW-0143`, APPROVED WITH COMMENTS) and
  `docs/reviews/2026-10-12-m4-m11-spi-pa-review.md` (`FRICK-REVIEW-0144`,
  APPROVED WITH COMMENTS; PA-01..PA-07, D-01).
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
| Native ctest (working tree) | `ctest --test-dir build/native` | **21/21 pass**, incl. `spi` |
| ASan/UBSan ctest | `ctest --test-dir build/asan` | **21/21 pass**, incl. `spi` |
| `test_spi` checks | `./build/native/tests/test_spi` | **`checks=120 failures=0`** |
| Traceability | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS (113 requirements, 70 srs, 29 sss, 14 gaps, 51 lean theorems)**; SSS-REQ-020 → **14** SRS children |
| Metrics panel | `python3 tools/scripts/metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror` on `ff_spi_mpfs.{c,h}`, `test_spi.c`, `app_renode_test.c` (clang-format 23.1.3) | clean (exit 0) |
| Header↔History (N16) | script over `| Revision`/`| Date` + last `## History` row | **all agree**: SRS 1.19 (`## 8. History`), traceability.md 0.31, SDD 0.7, software-description 0.4, verification-results 0.36, renode-environment 0.15, tool-qualification 0.3.19, class-a-gaps 0.9, handover 1.11, loop-audit 0.15, sprint-plan 0.7, development-plan 0.21 |
| Count consistency | `grep` across README/`traceability.md`/`verification-results`/`handover`/`loop-audit`/`status.json` | 113/70/29/14/51 consistent; only historical rows carry old counts (100/59, 98/57, 112/69, 7/13, 109) |
| Stale "absorbed" wording | `grep -rni absorb docs/ README.md CHANGELOG.md` | only `## History` rows / review reports / change descriptions — none live |
| `status.json` semaphores | read `docs/status/status.json:26-47` | `sprints.done=4`; all gates `green` (incl. `product_assurance_review`, `reviews`) — PA-05 (deferred to commit) |
| PA-01 fix | read `docs/vv/renode-environment.md:95,100,121-125,159` | **(8/13)**, SPI enumerated (SRS-REQ-069), SPI scope-limit note present; history 0.15 |
| PA-02 fix | read `README.md:417-420` | "Sprint 4 **complete** … M11 8/13 … `hal_spi=PASS` … Next Sprint 5 (BLOCK)" |
| PA-03 fix | read `docs/status/loop-audit.md:27,34` | M4 row "MSS SPI done (SRS-REQ-069)"; M11 row "8/13" |
| PA-04/D-01 fix | read `ff_spi_mpfs.c:122-128`; `test_spi.c:152-157` | `FF_TIMEOUT` → `spi_wr(FF_SPI_REG_CMD, FF_SPI_CMD_RXFIFORST)`; mock models the CMD RXFIFORST bit; comment reworded |
| PA-06 fix | read `docs/quality/class-a-gaps.md:108-112` | GAP-A5 SPI item now names the STATUS b11 residual |
| PA-07 fix | read `docs/status/loop-audit.md:9` | line refreshed ("ce0bff3 + Sprint-4 SPI increment (audited revision; pre-commit)") — still not `5ca2045` |
| B-01 fix | `grep "120 checks"` + `test_spi` run | `test_spi` = 120 checks recorded in `verification-results.md:48`, `sprint-plan.md:109`, `handover.md:212`, `CHANGELOG.md:19`; no stale "109" outside historical rows |
| B-02 fix | read `docs/development-plan.md` header + tail | order 0.19 → 0.20 → 0.21; header `0.21` = last row |
| B-03 fix | read `docs/planning/sprint-plan.md:45,109` | S4 "— **delivered**", cites SRS-REQ-069, exit "`hal_spi=PASS` (met); M11 8/13" |
| Contract match | `src/hal/hal_spi.h` vs `ff_spi_mpfs.{c,h}` | exact `FF_Spi_t` vtable (`name`, `init`, `transfer`, `cs_assert`, `cs_release`) |
| No other file modified | `git status --short` before/after | identical tracked/untracked set; gate commands regenerated only git-ignored `build/` artifacts |

## 4. Closure of the prior findings (first-hand verification)

| NC | Severity | Disposition verified first-hand | Status |
| -- | -------- | ------------------------------- | ------ |
| **PA-01** | Minor | `renode-environment.md` §HAL coverage now reads **Peripheral-bound (8/13)**, enumerates **SPI** (MSS SPI register round trips, SRS-REQ-069) and carries the SPI scope-limit note ("no registered slave → register round-trips only; the transfer path is host-tested", `:121-125`); history 0.15 present and header-matching. | **CLOSED** |
| **PA-02** | Minor | `README.md:417-420` "Current focus: **Sprint 4** complete … M11 peripheral-bound HAL coverage **8/13**, with `hal_spi=PASS` … Next per plan: **Sprint 5** (BLOCK eMMC/SD)". | **CLOSED** |
| **PA-03** | Minor | `loop-audit.md:27` M4 row now lists "… MSS SPI done (SRS-REQ-069); MMC/eMMC/storage/PCIe/SpaceWire pending"; consistent with the M11 row `:34` (8/13). | **CLOSED** |
| **PA-04 / D-01** | Minor | On `FF_TIMEOUT` the driver now writes `FF_SPI_CMD_RXFIFORST` (deterministic RX-FIFO reset) instead of a single `RXDATA` read (`ff_spi_mpfs.c:122-128`); the mock models the CMD RXFIFORST bit (`test_spi.c:152-157`); the old absolute comment was replaced. The primary failure mode is addressed (the timeout branch no longer returns without clearing the FIFO). Residuals (bounded, error path) recorded as **PA-08** below. | **CLOSED** (residual tracked as PA-08) |
| **PA-05** | Comment | `status.json:36`/`README.md:82` still `green` at the instant of this review; the coordinator reconciles the `product_assurance_review`/`reviews` semaphore **at commit** (disposition recorded). Not verifiable pre-commit; no derogation. | **CLOSED (disposition)** |
| **PA-06** | Comment | GAP-A5 now itemizes the STATUS b11 name residual (`class-a-gaps.md:110-112`). Residual: the driver header register-map line 32 still states `b11 TXFIFOEMPTYNEXT` as fact while the limitation (`:79-81`) says the model names it `TXFIFOFULLNEXT` — see **PA-10**. | **CLOSED** (residual tracked as PA-10) |
| **PA-07** | Comment | The `loop-audit.md:9` baseline line was refreshed but still cites `ce0bff3` (the pre-`5ca2045` commit) rather than the actual audited base `5ca2045` + increment — see **PA-11**. | **PARTIALLY CLOSED** (residual tracked as PA-11) |
| **R1-01** | Major | `spi_transfer` reads `RXDATA` on **every** frame and discards it when `rx == NULL` (`ff_spi_mpfs.c:130-137`); the mock is a non-overwriting one-deep RX FIFO (`test_spi.c:143-150`); `test_transfer_drains_rx` (`:363-383`) returns the **new** frame. | **CLOSED** |
| **R1-02** | Minor | `cs_assert`/`cs_release` gate on `spi_check_ready()` → `FF_NOT_READY` pre-init (`ff_spi_mpfs.c:142-173`); `test_cs_preinit` (`test_spi.c:385-398`) asserts it. | **CLOSED** |
| **R2-01** | Minor | The mock discriminates R1-01 (non-overwriting FIFO guard + regression test); still honestly declared a behavioural fake, not an independent oracle. | **CLOSED** |
| **R2-02** | Minor | SRS-REQ-069 reads "Validated by a host unit test (behavioural MMIO mock)" (`SRS.md:333-334`), matching `test_spi.c:10-18`. | **CLOSED** |
| **SCOPE-01** | Minor | `development-plan.md` M4 row "MSS SPI done … SPI removed from pending"; M11 row "8/13 … `hal_spi=PASS`" (`:310,317`); history 0.21 in order and header-matching. | **CLOSED** |
| **B-01** | Minor | `test_spi` = 120 checks recorded in the four documents; no stale "109" outside historical rows. | **CLOSED** |
| **B-02** | Minor | `development-plan.md` History 0.19 → 0.20 → 0.21; header `0.21` = last row. | **CLOSED** |
| **B-03** | Minor | `sprint-plan.md` §2 S4 "— **delivered**", cites SRS-REQ-069, exit annotated. | **CLOSED** |

**Closure summary:** PA-01, PA-02, PA-03, PA-04/D-01, R1-01, R1-02, R2-01, R2-02,
SCOPE-01 and B-01..B-03 are **verified CLOSED**. PA-05 is closed by recorded
disposition (semaphore reconciled at commit). PA-06 and PA-07 are closed for
their primary ask, with bounded residuals tracked as **PA-10** and **PA-11**.

## 5. New findings (R4 documentation + PA assurance dimensions)

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation (required correction) |
| - | ------------------------------ | ------------------ | ------- | -------- | ------------------------------------ |
| **PA-08** | **Comment** | N5/N10/N15/N19 — D-01 closure evidence | **The D-01 closure (the new `FF_TIMEOUT` RX-FIFO reset) is neither cross-checked nor discriminated by any test, and the `CMD` RXFIFORST bit is a new load-bearing register-map assumption.** The driver now depends on `FF_SPI_REG_CMD` (0x2C) bit 2 `FF_SPI_CMD_RXFIFORST` on the error path (`ff_spi_mpfs.c:127`), but (a) the emulated Renode check round-trips only CONTROL/FRAMESIZE/CLKRATE/STATUS (`renode-environment.md:121-125`; `ff_spi_mpfs.h:19-22`), so the CMD bit is **not** cross-checked; (b) no host test asserts the reset — `test_transfer_timeout` uses `no_rx=1` (no frame ever arrives), so the mock's RXFIFORST handling (`test_spi.c:152-157`) is executed but its effect is unobserved (`test_spi.c:336-346`); and (c) GAP-A5's SPI item does **not** name the CMD bit (it names the CLKRATE divider, the 100 MHz APB reference, per-rate SCL and the b11 name only, `class-a-gaps.md:108-112`). The in-code comment asserts the failure mode is "close[d] … on the error path" (`ff_spi_mpfs.c:123-126`), which is only established if the CMD bit is correct and the frame does not complete after the reset. Low materiality (error path; the caller already receives `FF_TIMEOUT`; the primary idiom is verified). | `ff_spi_mpfs.c:122-128`; `ff_spi_mpfs.h:177-180`; `test_spi.c:152-157,336-346`; `class-a-gaps.md:108-112`; `renode-environment.md:121-125` | Itemize the CMD RXFIFORST bit (0x2C b2) in GAP-A5 (as for the b11 name), add a mock mode/test that completes a frame after the poll bound (or explicitly declare the timeout-path reset unverified), and tighten the comment to "clears any frame already in the RX FIFO at the timeout instant". |
| **PA-09** | **Minor** | N8/N16 — documentation currency (resume point) | **`docs/status/handover.md` §4 "Current state" is headed "(as of commit `db8261f` + the Sprint-3 (M4/M11 MSS I2C) increment …)", which is stale by two increments** — `db8261f` precedes the Sprint-3 I2C commit `ce0bff3`, and the restored-M4-scope commit `5ca2045` and the Sprint-4 SPI increment are not named — while the same section's body was advanced to Sprint 4 (`handover.md:103-116,140-142`). The `handover.md` was modified in this increment (rev 1.11), so its current-state marker must match the body (norm N8); the same defect class was raised as PA-02. | `docs/status/handover.md:87` vs `:103-116,140-142`; `git show HEAD:docs/status/handover.md:87` (unchanged by the increment) | Advance the §4 heading to the actual state (e.g. "as of `5ca2045` + the Sprint-4 SPI increment") so the resume point is accurate. |
| **PA-10** | Comment | N5/N8 — register-map naming (PA-06 residual) | The driver header register-map line still states `b11 TXFIFOEMPTYNEXT` as fact (`ff_spi_mpfs.h:32`; `#define … TXFIFOEMPTYNEXT (1u << 11)` at `:166`), while the limitations section says the Renode model names b11 `TXFIFOFULLNEXT` and that the name is "unconfirmed (GAP-A5)" (`:79-81`). PA-06 asked to reword line 32; only the GAP-A5 item was added. The bit is unused, so there is no functional impact. | `ff_spi_mpfs.h:32,79-81,166`; `class-a-gaps.md:110-112` | Mark b11 as unconfirmed on the map line (or align the two names) so the map and the limitation agree. |
| **PA-11** | Comment | N16/N19 — audit-record precision (PA-07 residual) | The `loop-audit.md` `Baseline commit` reads "`ce0bff3` + Sprint-4 SPI increment (audited revision; pre-commit)" (`:9`), but the audited revision is HEAD `5ca2045` (the restored-M4-scope commit) plus the uncommitted increment. The line was refreshed but still omits `5ca2045`. | `docs/status/loop-audit.md:9` | State the audited base as `5ca2045` + the Sprint-4 increment. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(none)*
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified. *(PA-09.)*
- **Comment**: observation/recommendation, no derogation. *(PA-08, PA-10, PA-11.)*

## 6. Scope conformance (standing rule)

**Implemented content vs documented scope.**

- **Within scope, no over-implementation.** The driver implements exactly the
  `FF_Spi_t` contract (`init`, `transfer`, `cs_assert`, `cs_release`) and nothing
  beyond it: no DMA, interrupt, slave or enhanced/command framing. The additional
  register `#define`s document the map; all are unused except the newly-used
  `FF_SPI_CMD_RXFIFORST` (still within the polled-master scope). The MMIO
  self-test is a documented bring-up check with disclosed side effects. No
  out-of-scope feature was found.
- **No under-implementation.** The `FF_Spi_t` vtable is complete and bound by
  `FF_SpiMpfsSetup`; the requirement SRS-REQ-069 is implemented and traced
  (implementation, tests and `ci:renode` links present in
  `traceability.yaml`).
- **Consistent with the scope documents.** `docs/roadmap.md` M4/M11 list SPI among
  the native MSS peripheral layers; `docs/development-plan.md` OBJ-10/OBJ-19 and
  the M11 objective cover the SPI class; the implementation matches.
- **No capability over-claim.** The SPI limitation (emulated `SPI.MPFS_SPI` has no
  registered slave → **register round-trips only**; the **transfer path is
  host-tested**) is stated consistently in the driver header
  (`ff_spi_mpfs.h:237-240`), SRS-REQ-069, the `verification-results.md` row, the
  Renode check comment (`app_renode_test.c:298-304`) and `renode-environment.md`
  (now with the SPI scope-limit note, PA-01 closed).

No scope deviation (over- or under-implementation) was identified; the standing
scope-conformance rule is **satisfied**. The residuals PA-08..PA-11 are
records/verification and currency observations, not out-of-scope features.

## 7. Norm-by-norm audit (N1–N20)

| Norm | Requirement | Verification | Status |
| ---- | ----------- | ------------ | ------ |
| N1 | Lean verification of generated code | SPI is a bus driver with no Lean theorem; `traceability.yaml` SRS-REQ-069 carries an explicit `lean_note` justification | compliant |
| N2 | English only | all new/changed content English | compliant |
| N3 | Backwards ECSS SRS/SSS | SRS-REQ-069 present, parented to SSS-REQ-020; SSS unchanged (no N14 trigger) | compliant |
| N4 | Bidirectional traceability | `trace.py` PASS 113/70/29/14/51; SSS-REQ-020 → 14 children; bottom-up + manifest + SDD matrix agree | compliant |
| N5 | Class B target; class-A gaps documented | GAP-A5 gains the SPI residual incl. the b11 name; the CMD RXFIFORST bit is not itemized (**PA-08**) | compliant with a records gap (PA-08) |
| N6 | Full SW V&V; Docker-deployed environments | gates re-run (native/ASan/trace/metrics/format) | compliant |
| N7 | Dual push | **not performed** — this review is a pre-commit gate; commit/push is the human/authority action | not applicable to the reviewer |
| N8 | Complete, current documentation | PA-01/PA-02/PA-03 closed; residual currency defect in `handover.md` §4 (**PA-09**) and the header map naming (**PA-10**) | **partial — one Minor (PA-09) open** |
| N9 | Independent review agents | 0142/0143/0144 by independent profiles; this R4+PA delta review independent | compliant |
| N10 | Test-script/V&V-tool review & qualification | T3 includes `test_spi.c`; T17 includes `hal_spi`; register updated 0.3.19; the timeout-path reset is not discriminated (**PA-08**) | compliant with a V&V gap (PA-08) |
| N11 | Scripts + review results in the repo | reports 0142/0143/0144 in `docs/reviews/`; log rows present; this report's row pending (owner: coordinator) | compliant (this report's log row pending) |
| N12 | NC resolution to full compliance | PA-01..PA-04/D-01 closed; new PA-09 (Minor) + PA-08/PA-10/PA-11 (Comments) to close/justify | **in progress** |
| N13 | AGPL-3.0-only, SPDX | SPDX headers present in the new files | compliant |
| N14 | SSS review by Senior Systems Engineer | SSS.md not modified in this increment | not triggered |
| N15 | Full cycle on every change | build/test/trace/format re-run; Lean/RTEMS/Renode/QEMU recorded (not re-run here) | compliant |
| N16 | Detailed revision history | header↔History agree for every touched doc; `handover.md` §4 marker stale (**PA-09**); `loop-audit.md` base imprecise (**PA-11**) | compliant with PA-09/PA-11 |
| N17 | R4 documentation review before commit | 0143 was a combined R3+R4 review; this report is the **R4 delta re-review** over the fixed docs | compliant (this report) |
| N18 | Renode functional validation | recorded `make renode` → `RENODE_TEST: PASS` incl. `hal_spi=PASS` (read, not re-run — outside the requested gate set) | compliant (recorded) |
| N19 | Loop audit vs declared objectives/norms | `loop-audit.md` M4/M11 rows now consistent (PA-03 closed); baseline-commit still imprecise (**PA-11**) | compliant with PA-11 |
| N20 | Independent PA review of every phase product | this report; findings must be corrected and the review repeated until it passes | **in progress** (this report) |

## 8. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** Every fix claimed for `FRICK-REVIEW-0144` is present and verified
first-hand: PA-01 (`renode-environment.md` §HAL coverage 8/13 + SPI + scope-limit
note), PA-02 (README "Sprint 4 complete"), PA-03 (loop-audit M4 row), and
PA-04/D-01 (deterministic `FF_TIMEOUT` RX-FIFO reset, mock models the CMD bit,
comment reworded). PA-05 is closed by recorded disposition; PA-06 and PA-07 are
closed for their primary ask with bounded residuals. The previously-closed
R1-01/R1-02/R2-01/R2-02/SCOPE-01 and B-01..B-03 are re-verified closed. The
increment is **within scope** (no over- or under-implementation), the requirement
chain converges at **113/70/29/14/51**, and every requested gate passes first-hand
(**21/21 native + ASan**, `test_spi` **120 checks**, `trace.py` PASS,
`metrics --check` current, clang-format clean; all touched-document headers match
their last `## History` row). **No Major finding is open.** One **new Minor**
(**PA-09**, the stale `handover.md` §4 "as of" marker — N8 currency) and three
**Comments** (**PA-08** the untracked/untested `CMD` RXFIFORST bit and the
timeout-path comment; **PA-10** the header b11 naming; **PA-11** the loop-audit
base commit) remain to be corrected or justified before commit/push. Because a
Minor is open, the N20/N12 loop is **not fully closed**: PA-09 (and the Comments)
must be closed and, per norm N12, the affected deltas re-reviewed before the
increment is committed and pushed.

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| **R4 (N17)** | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — PA-01/PA-02/PA-03 closed; PA-09 (Minor) + PA-10/PA-11 (Comments) to closure |
| **PA (N20)** | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — PA-01..PA-07/D-01 closed; PA-08/PA-09/PA-10/PA-11 (1 Minor + 3 Comments) to closure; delta re-review required before commit/push (N12/N20) |

## 10. Follow-up records

(norm N12: NCs are resolved by re-applying ALL norms to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| PA-01 | Minor | N8/N16/N18 — V&V environment record | `renode-environment.md` §HAL coverage stale | Fix count/enumeration/note | `renode-environment.md:95,100,121-125,159` | frick-fsw | **closed** | done (0145) | S4 |
| PA-02 | Minor | N8/N16/N19 — documentation currency | README "Current focus" stale | Advance to Sprint 4 | `README.md:417-420` | frick-fsw | **closed** | done (0145) | S4 |
| PA-03 | Minor | N8/N19 — loop-audit consistency | M4 row listed SPI pending | Update the M4 row | `loop-audit.md:27,34` | frick-fsw | **closed** | done (0145) | S4 |
| PA-04 / D-01 | Minor | N8 — comment accuracy / timeout path | Timeout drain not robust; comment over-claimed | Deterministic RX-FIFO reset + mock + comment | `ff_spi_mpfs.c:122-128`; `test_spi.c:152-157` | frick-fsw | **closed** (residual PA-08) | done (0145) | S4 |
| PA-05 | Comment | N8/N11 — records truthfulness | Semaphore green while PA review pending | Set green at commit | `status.json`/README (at commit) | frick-fsw (coordinator) | **closed (disposition)** | — | S4 (commit) |
| PA-06 | Comment | N5/N8/N19 — residual tracking | b11 name referenced but not itemized | Itemize in GAP-A5 | `class-a-gaps.md:110-112` | frick-fsw | **closed** (residual PA-10) | — | S4 |
| PA-07 | Comment | N16/N19 — audit-record precision | loop-audit baseline omitted `5ca2045` | State the audited base precisely | `loop-audit.md:9` | frick-fsw | **open** (residual PA-11) | — | S4/S5 |
| R1-01 | Major | SRS-REQ-069 `transfer` RX | Undrained RX FIFO → next transfer mis-reads | Drain `RXDATA` on every frame + discriminating test | `ff_spi_mpfs.c:130-137`; `test_spi.c:363-383` | frick-fsw | **closed** | done (0143) | S4 |
| R1-02 | Minor | SRS-REQ-069 pre-init | `cs_assert`/`cs_release` bypassed readiness | Gate on `spi_check_ready()` | `ff_spi_mpfs.c:142-173`; `test_spi.c:385-398` | frick-fsw | **closed** | done (0143) | S4 |
| R2-01 | Minor | V&V test discrimination | Mock could not model a non-overwriting FIFO | Non-overwriting FIFO + regression | `test_spi.c:143-150,363-383` | frick-fsw | **closed** | done (0143) | S4 |
| R2-02 | Minor | SRS-REQ-069 evidence wording | "master/slave" overstated the evidence | Align SRS text | `SRS.md:333-334` | frick-fsw | **closed** | done (0143) | S4 |
| SCOPE-01 | Minor | Scope conformance (N8/N16/N19) | `development-plan.md` M4/M11 stale | Update rows + history | `development-plan.md:310,317` | frick-fsw | **closed** | done (0143) | S4 |
| B-01 | Minor | R4 — evidence accuracy | `test_spi` recorded as 109; actual 120 | Update the count in four docs | `verification-results.md:48`, `sprint-plan.md:109`, `handover.md:212`, `CHANGELOG.md:19` | frick-fsw | **closed** | done (0144) | S4 |
| B-02 | Minor | R4 — revision history (N16) | `development-plan.md` history out of order | Reorder; last row = header | `development-plan.md` tail (0.19→0.20→0.21) | frick-fsw | **closed** | done (0144) | S4 |
| B-03 | Minor | R4 — sprint-plan consistency | S4 row not marked delivered | Mark delivered; cite SRS-REQ-069 | `sprint-plan.md:45,109` | frick-fsw | **closed** | done (0144) | S4 |
| **PA-08** | **Comment** | N5/N10/N15/N19 — D-01 closure evidence | Timeout RX-FIFO reset depends on the untracked/un-cross-checked `CMD` RXFIFORST bit and is not discriminated by any test; comment asserts closure | Itemize the CMD bit in GAP-A5; add a discriminating test (or declare unverified); tighten the comment | (delta) `class-a-gaps.md`/`ff_spi_mpfs.{c,h}`/`test_spi.c` diff | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| **PA-09** | **Minor** | N8/N16 — documentation currency | `handover.md` §4 "as of commit `db8261f` + Sprint-3 increment" stale by two increments | Advance the §4 heading to the actual state | (delta) `handover.md` diff | frick-fsw | **open** | pending | S4 delta (pre-commit) |
| **PA-10** | Comment | N5/N8 — register-map naming | Header map line 32 states b11 `TXFIFOEMPTYNEXT` as fact vs the limitation `TXFIFOFULLNEXT` | Mark b11 unconfirmed / align names | (delta) `ff_spi_mpfs.h` diff | frick-fsw | **open** | — | S4/S5 |
| **PA-11** | Comment | N16/N19 — audit-record precision | loop-audit baseline still cites `ce0bff3`, not `5ca2045` | State the audited base precisely | (delta) `loop-audit.md` diff | frick-fsw | **open** | — | S4/S5 |
| LOG-01 | Comment | N11/N20 — records | Review-log row for FRICK-REVIEW-0145 still `(pending)` | Coordinator sets the final verdict after closure | `docs/reviews/README.md` diff | frick-fsw (coordinator) | **open** | — | S4 (commit) |

> This report is authored to be committed and pushed with the reviewed change
> (norm N11). The review was **read-only** with respect to repository content: the
> reviewer wrote exactly one file (this report); no other tracked file was
> modified, created or deleted, and nothing was committed or pushed. The gate
> commands regenerated only git-ignored `build/` artifacts.

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- PA-01 is closed: `renode-environment.md` §HAL coverage reads **(8/13)**,
  enumerates **SPI** (SRS-REQ-069) and carries the SPI scope-limit note; history
  0.15 matches the header.
- PA-02 is closed: `README.md` "Current focus" says **Sprint 4 complete** (M11
  8/13, `hal_spi=PASS`, next Sprint 5 BLOCK).
- PA-03 is closed: `loop-audit.md` M4 row records "MSS SPI done (SRS-REQ-069)",
  consistent with the M11 row (8/13).
- PA-04/D-01 is closed: on `FF_TIMEOUT` the driver writes
  `FF_SPI_CMD_RXFIFORST` (deterministic RX-FIFO reset); the mock models the CMD
  RXFIFORST bit; the old absolute comment was replaced.
- PA-06's primary ask is met (GAP-A5 itemizes the b11 name); PA-07's line was
  refreshed but still cites `ce0bff3`.
- R1-01/R1-02/R2-01/R2-02/SCOPE-01 and B-01/B-02/B-03 are re-verified closed.
- All touched documents' headers match their last `## History` row (N16),
  including SRS.md (its History heading is `## 8. History`).
- Counts are consistent at **113/70/29/14/51**; `test_spi` = **120** checks; no
  stale counts/"absorbed" wording outside historical rows.
- All requested gates pass first-hand: **21/21** native + ASan, `test_spi` 120
  checks, `trace.py` PASS, `metrics --check` current, clang-format clean.
- The new timeout-path reset depends on the `CMD` (0x2C b2) RXFIFORST bit, which
  is not cross-checked by the emulated check and is not discriminated by any host
  test (PA-08).
- `handover.md` §4 is still headed "as of commit `db8261f` + the Sprint-3
  increment" (PA-09); `ff_spi_mpfs.h:32` still states b11 `TXFIFOEMPTYNEXT` as
  fact vs the limitation (PA-10).

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0145`* — owner: review lead; impact: report identity
  only; validation: lead renumbers if needed.
- *The uncommitted working-tree delta is the item under review atop `5ca2045`.*
- *`FF_SPI_MPFS_CLK_HZ = 100 MHz` is the reference APB clock* — owner:
  hardware/mission (GAP-A5); validation: confirm against the RTPFS160ZT datasheet.
- *The `CMD` (0x2C) RXFIFORST bit position (b2) is as documented in the driver
  header* — owner: hardware/driver (GAP-A5); validation: confirm against the
  CoreSPI register map (the Renode model / datasheet).

**Inferences**

- Because the timeout-path reset's register bit is neither cross-checked nor
  tested, its closure of the R1-01 failure mode on the error path rests on an
  unverified assumption and a comment that asserts more than the evidence shows
  (PA-08).
- Because `handover.md` §4's "as of" marker was not advanced with the increment
  while its body was, the resume point misstates the current state by two
  increments (PA-09).
- Because only the GAP-A5 item was added (not the map-line reword), the header's
  b11 name and its limitation remain inconsistent (PA-10).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. Correct PA-09 (Minor) — advance the `handover.md` §4 heading to the actual
   state — in the corrective commit; re-run the full gate set (N15) and update the
   affected history (N8/N16). Owner: authoring agent; sprint: **S4 delta
   (pre-commit)**.
2. Address PA-08: itemize the `CMD` RXFIFORST bit in GAP-A5 (as for the b11
   name), add a mock mode/test that completes a frame after the poll bound (or
   explicitly declare the timeout-path reset unverified), and tighten the comment
   wording.
3. Address PA-10 (align the b11 map line with the limitation) and PA-11 (state
   the audited base as `5ca2045`).
4. Set the `product_assurance_review`/`reviews` semaphores at commit (PA-05) and
   add the review-log row (LOG-01); re-review the PA/R4 deltas (N12/N17/N20) until
   no Minor is open before commit/push.
5. Keep the GAP-A5 on-target residuals (APB clock, per-rate SCL, real-bus/FIFO
   behaviour, wall-clock latency) tracked.

**Confidence:** **High** for the closure determinations of PA-01/PA-02/PA-03/
PA-04/D-01/R1-01/R1-02/R2-01/R2-02/SCOPE-01 and B-01..B-03, the count/trace
convergence, the header↔History agreement and the gate results (all reproduced
first-hand by direct source/document reads and command re-execution). **High** for
PA-09/PA-10/PA-11 (direct document/source reads). **Medium** for PA-08's
materiality (the error path is rarely taken and the caller already receives
`FF_TIMEOUT`; the exact `CMD`/RXFIFORST semantics are a GAP-A5 residual).

**Open gaps**

- **PA-09 (Minor) open** — `handover.md` §4 "as of" marker stale; to close before
  commit (N8/N12/N20).
- **PA-08/PA-10/PA-11 (Comments) open** — timeout-path closure evidence, header
  b11 naming, audit base commit.
- **PA-05, LOG-01 (Comments)** — semaphore timing and this report's log row.
- **No independent oracle** for the transfer path (accepted limitation); register
  offsets/reset values only are cross-checked (by Renode).
- **On-target evidence absent** — APB clock, per-rate SCL accuracy, FIFO depth,
  wall-clock latency (GAP-A5).
- **This report's `docs/reviews/README.md` log row** and the final verdict are to
  be set by the coordinator after the delta closure (norm N11/N20).
- **Document id** `FRICK-REVIEW-0145` retained as assigned.
