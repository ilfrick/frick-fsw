# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0143                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2 (delta closure) + R3+R4 combined** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) for the delta; R3 (Senior avionics / systems engineer) + R4 (Senior SW documentation engineer) for the increment |
| Reference revision                | `5ca2045` (HEAD) + **uncommitted** Sprint-4 working-tree increment (reference MSS SPI backend, SRS-REQ-069) |
| Items under review                | `src/drivers/spi/mpfs/ff_spi_mpfs.{c,h}` (new); `tests/unit/test_spi.c` (new); SPI deltas in `src/apps/app_renode_test.c`, `config/platform/platform_mpfs.h`, `CMakeLists.txt`, `tests/CMakeLists.txt`; requirement **SRS-REQ-069** and its chain in `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`; `docs/design/software-design.md`; `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/quality/class-a-gaps.md`; `docs/engineering/software-description.md`; `docs/status/{handover.md,loop-audit.md,status.json}`; `docs/planning/sprint-plan.md`; `docs/development-plan.md`; `CHANGELOG.md`; `README.md`; the `ci-renode` markers in both CI workflows |
| Date                              | 2026-10-12                             |
| Scope                             | Combined **delta closure** of the R1+R2 review `FRICK-REVIEW-0142` (findings R1-01/R1-02/R2-01/R2-02/SCOPE-01) **and** the **R3 systems/requirements (N14)** and **R4 documentation (N17)** review of the same increment, against ECSS-E-ST-40C (SW development/V&V, requirements), ECSS-Q-ST-80C (SW PA), the project norms N1–N20 (`docs/rules.md`), the HAL contract `FF_Spi_t` (`src/hal/hal_spi.h`), and the authoritative Renode `SPI.MPFS_SPI` / Microchip CoreSPI register model. |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery, or previously reviewing the items under review. This review ran in a fresh context; every declared claim was re-executed or read first-hand (see §9). |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **Part A — delta closure.** The fixes claimed for `FRICK-REVIEW-0142`:
  - R1-01 (Major): `spi_transfer` now always reads `RXDATA` (discarding when `rx == NULL`); the mock is a one-deep **non-overwriting** RX FIFO; regression test `test_transfer_drains_rx`; updated `test_transfer_tx_only` expectations.
  - R1-02 (Minor): `cs_assert`/`cs_release` gate on readiness → `FF_NOT_READY` pre-init; `test_cs_preinit`.
  - R2-01 (Minor): the mock discriminates R1-01.
  - R2-02 (Minor): SRS-REQ-069 wording "mock MMIO master/slave" → "behavioural MMIO mock".
  - SCOPE-01 (Minor): `docs/development-plan.md` §6 M4/M11 rows updated + history row.
  - Comment items R1-C2/C4/C5 addressed in the driver header; R1-C1/C3/C6 and R2-C1 accepted as documented.
- **Part B — R3 (systems/requirements).** SRS-REQ-069 well-formedness (singular, testable, parented to SSS-REQ-020, method-annotated, free of over-claim); the trace manifest/matrix/top-down/bottom-up agreement (counts 113/70/29/14/51); the contract `FF_Spi_t` (`src/hal/hal_spi.h`) vs the delivered backend; scope statements accurate and free of over-claim (SPI implemented, M11 8/13; the emulated model has no registered slave → register round-trips only).
- **Part B — R4 (documentation).** Currency/consistency of `docs/vv/{verification-results,renode-environment,tool-qualification}.md`, `docs/design/software-design.md`, `docs/quality/class-a-gaps.md`, `docs/engineering/software-description.md`, `docs/status/{handover,loop-audit,status.json}`, `docs/planning/sprint-plan.md`, `docs/development-plan.md`, `CHANGELOG.md`, `README.md`, both `ci-renode` markers; per-doc header `Revision`/`Date` vs the last `## History` row (N16).
- Independent re-execution of the requested gate set (see §9).

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour (configured MSS SPI APB clock, per-rate SCL accuracy, real-slave timing/eye, FIFO depth, wall-clock latency) — tracked to GAP-A5 and not verifiable here.
- Renode/QEMU/Lean/RTEMS-cross/MISRA re-execution (not in the requested gate set); the recorded evidence was read but not re-run.
- The PA (N20) review of the same increment, and the review-log row in `docs/reviews/README.md` (owner: authoring agent; norms N11/N20).

**Assumption recorded:** the assigned document id `FRICK-REVIEW-0143` was supplied by the review lead; the highest committed/observed id in `docs/reviews/` is `FRICK-REVIEW-0142` (the initial SPI review), so the id is taken as assigned and not re-derived here.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract: `docs/engineering/independent-review.md` (rev 0.7).
- Report format: `docs/reviews/review-template.md`.
- Prior review: `docs/reviews/2026-10-12-m4-m11-spi-r1-r2-review.md` (`FRICK-REVIEW-0142`, REJECTED).
- Contract: `src/hal/hal_spi.h` (`FF_Spi_t`).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-069, SRS-REQ-031), `docs/requirements/SSS.md` (SSS-REQ-020), `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`.
- Scope documents: `docs/roadmap.md` (M4, M11), `docs/development-plan.md` (OBJ-10, OBJ-19), `docs/planning/sprint-plan.md`, `docs/vv/renode-environment.md`.
- Gaps: `docs/quality/class-a-gaps.md` (GAP-A5).
- Evidence: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` (T3/T17), `CHANGELOG.md`, `README.md`, `docs/status/*`.
- Authoritative hardware oracle: Renode `SPI.MPFS_SPI` model / Microchip CoreSPI register map and reset values (SPI0 `@0x20108000` PLIC 54, SPI1 `@0x20109000` PLIC 55).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Part A — R1+R2 delta closure

| NC | Severity | Item / Requirement | Fix claimed | First-hand evidence | Regression check | Status |
| -- | -------- | ------------------ | ----------- | ------------------- | ---------------- | ------ |
| **R1-01** | **Major** | SRS-REQ-069 `transfer` RX (stale `RXDATARCVD` / undrained RX FIFO) | `spi_transfer` always reads `RXDATA` (discards when `rx == NULL`); mock non-overwriting one-deep RX FIFO; `test_transfer_drains_rx`; updated `test_transfer_tx_only` | `ff_spi_mpfs.c:119-133` reads `RXDATA` on every frame and discards when `rx == NULL` (l.129-132); mock `test_spi.c:143-150` only refreshes `rx_fifo`/`rx_valid` when `rx_valid == 0` (non-overwriting); `test_transfer_drains_rx` (`test_spi.c:357-377`) tx-only then full-duplex → `rx[0] == 0x22` (the new frame); `test_transfer_tx_only` (`306-320`) asserts `rx_count == 2`, `rx_valid == 0` | Discrimination confirmed by reading: with a driver that skips the drain, the mock keeps `rx_valid=1`/`rx_fifo=0x11`, the next full-duplex poll returns immediately and `rx[0] == 0x11` → the new test fails. No regression: 21/21 native + ASan, `test_spi` 120 checks | **CLOSED (primary path)** — with residual **D-01** (see Part B) |
| **R1-02** | Minor | SRS-REQ-069 pre-init | `cs_assert`/`cs_release` call the ready check → `FF_NOT_READY`; `test_cs_preinit` | `spi_cs_assert` (`ff_spi_mpfs.c:137-153`) and `spi_cs_release` (`155-168`) now call `spi_check_ready()`; `test_cs_preinit` (`test_spi.c:379-392`) asserts `FF_NOT_READY` pre-init for both, then `FF_SUCCESS` post-init | No regression | **CLOSED** |
| **R2-01** | Minor | V&V — test discrimination | Mock discriminates R1-01 | Non-overwriting FIFO guard (`test_spi.c:143-150`) + `test_transfer_drains_rx` (`357-377`); the mock is still honestly declared a behavioural fake, not an independent oracle (`test_spi.c:10-18`) | The regression test fails against the pre-fix driver (reasoned) and passes now | **CLOSED** |
| **R2-02** | Minor | SRS-REQ-069 evidence wording | "mock MMIO master/slave" → "behavioural MMIO mock" | `docs/requirements/SRS.md:320,334` now reads "Validated by a host unit test (behavioural MMIO mock)"; matches `test_spi.c:10-18` | No regression | **CLOSED** |
| **SCOPE-01 (NC-01)** | Minor | Scope conformance / N8 / N16 / N19 | `development-plan.md` §6 M4/M11 rows updated (SPI done, M11 8/13) + history row | `docs/development-plan.md:310` now lists "MSS SPI done; MMC/eMMC/storage/PCIe/SpaceWire pending"; `:317` now "in progress (8/13 peripheral-bound) … `hal_spi=PASS`"; history row 0.21 added | Consistent with README/handover/loop-audit/sprint-plan/status.json/software-description/traceability (all 8/13). **However** the added history row is out of order and the header/last-row disagree → new **B-02** | **CLOSED** — with new **B-02** |

**Delta regression assessment.** No regression introduced by the fixes. `cmake --build build/native` links 25/25 targets; `ctest --test-dir build/native` = **21/21** (incl. `spi`); `ctest --test-dir build/asan` = **21/21**; `./build/native/tests/test_spi` = **checks=120 failures=0**; `trace.py` = PASS 113/70/29/14; `metrics.py --check` = current; `clang-format --dry-run --Werror` on the changed C files = clean (exit 0). The two added tests and the updated expectations are the only behavioural changes in the test; the driver change is the intended drain.

> **Note on the R1-01 residual (D-01).** The chosen fix drains `RXDATA` on the *success* path only. On the `FF_TIMEOUT` branch `spi_transfer` returns at `ff_spi_mpfs.c:122-124` **before** the `RXDATA` read at l.129, so a frame that the controller completes *after* the poll bound leaves a stale `RXDATARCVD`/RX-FIFO byte — the same one-frame shift that R1-01 flagged, on the timeout sub-case. The in-code comment (`ff_spi_mpfs.c:125-128`) claims "a tx-only (rx == NULL) or **timed-out** transfer must not leave a stale received byte", which the code does not implement. Because this is an error path (the caller already receives `FF_TIMEOUT`) and the primary idiom is fixed, D-01 is classified **Minor**, not a re-opened Major.

## 4. Part B — R3 + R4 findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| **B-01** | **Minor** | R4/N8/N11/N16 — V&V evidence accuracy | The `test_spi` check count is recorded as **109** in four documents of this increment, but the test now reports **120** checks. The count was written before the R1-01/R1-02 regression tests were added and was not refreshed. | `docs/vv/verification-results.md:48` and history `:439` ("`test_spi` 109 checks"); `docs/planning/sprint-plan.md:109`; `CHANGELOG.md:19`; `docs/status/handover.md:143,212`; first-hand `./build/native/tests/test_spi` → `checks=120 failures=0` | Update the four documents to **120 checks** in the corrective commit (N8/N11/N16). |
| **B-02** | **Minor** | R4/N16 — revision history | `docs/development-plan.md`'s `## History` table is **out of ascending order** (… 0.19, **0.21**, **0.20**), and the header `Revision` (**0.21**) does not equal the last history row (**0.20**). The new 0.21 row was inserted above 0.20. | `docs/development-plan.md` header `| Revision | 0.21 |`; history tail order `0.19 → 0.21 → 0.20` | Reorder so 0.20 precedes 0.21 and the last row is 0.21 (matching the header). |
| **B-03** | **Minor** | R4/N8/N16/N19 — sprint-plan internal consistency | `docs/planning/sprint-plan.md` §2 row **S4** was **not** updated to mark delivery (no "— **delivered**", unlike S1/S2/S3) and still cites only the contract **SRS-REQ-031**, not the delivered **SRS-REQ-069**, while the same document's History row 0.7 states "S4 delivered" and the exit criterion remains unannotated. | `docs/planning/sprint-plan.md:45` vs `:44` (S3 pattern) and History `:109` | Mark S4 delivered, cite SRS-REQ-069 (contract SRS-REQ-031), and annotate the exit criterion "(`hal_spi=PASS` met)". |
| **B-04** | Comment | R4/N8/N19 — status metric | `docs/status/status.json` `sprints.done = 3` and the README panel "Sprints completed 3/29" were **not advanced** after Sprint 4, while the same increment's `handover.md`/`sprint-plan.md` state "S4 delivered". Possibly intentional under the sprint-plan maintenance rule (sprints ticked at *loop start*); if so it should be stated. | `docs/status/status.json` `sprints` block (unchanged from `5ca2045`); `README.md` panel (regenerated, matches status.json); `docs/status/handover.md:142`; `docs/planning/sprint-plan.md` §5 | Either advance `done` to 4 (with S5 current) or add a one-line note that the sprint counter is re-baselined at loop start. |
| **B-05** | Comment | R4/N16 — file hygiene | `docs/engineering/software-description.md` has **no trailing newline** (both the 0.3 and 0.4 rows end without a final `\n`). | `tail -c1` on the file is not a newline | Add the trailing newline. |
| **B-06** | Comment | R4/N19 — loop audit | `docs/status/loop-audit.md` `Baseline commit` still reads "ce0bff3 (Sprint-3) + restored-M4-scope increment", while the body was updated to the post-S4 state (M11 8/13, counts 113). | `docs/status/loop-audit.md:9` vs body rows `:31,:46` and History 0.15 | Clarify the audited revision (pre-S4 `5ca2045`) or note that the audit covers the S4 loop. |
| **B-07** | Comment | R4/N11/N20 — records | `docs/reviews/README.md` (review log) has **no row** for `FRICK-REVIEW-0142` (nor 0143). Per norm N11/N20 the log row is added by the authoring agent/coordinator with the change. | `docs/reviews/README.md` last rows (through the M4-restored-scope closure) | Add the SPI review rows (0142, 0143, plus the PA review) before commit. |
| **B-08** | Comment | R3 — requirement wording | SRS-REQ-069 says "the … MSS SPI (CoreSPI) **controllers**" (plural) "behind `FF_Spi_t`", but the backend is a single static instance (only SPI0 exercised); the driver header documents this. Not a defect; a wording precision point. | `docs/requirements/SRS.md:322-323`; `ff_spi_mpfs.h:96-98` | Optionally reword to "controller" or note single-instance in the requirement. |

**Part B positive confirmations (R3/R4):**

- **SRS-REQ-069 is well-formed.** Singular (one reference SPI backend), testable (host unit test + emulated register check, `method: Tst | Dem`), correctly **parented to SSS-REQ-020** ("Reference low-level implementations") in both the manifest (`traceability.yaml:642-649`) and the SRS, method-annotated, and free of over-claim: the validation text now matches the actual evidence (behavioural MMIO mock + emulated-MMIO register check), and the on-target residuals (APB clock, per-rate SCL) are explicitly deferred to GAP-A5.
- **Trace chain agrees.** `trace.py docs/requirements/traceability.yaml` = **PASS (113 requirements, 70 srs, 29 sss, 14 gaps, 51 lean theorems)**; SSS-REQ-020 now shows **14** SRS children (was 13); the bottom-up (`traceability.md:50`), the manifest entry (`:134`) and the SDD matrix (`software-design.md`) all include SRS-REQ-069; counts 113/70/29/14/51 are consistent across README, `traceability.md`, `verification-results.md`, `handover.md`, `loop-audit.md`.
- **Contract `FF_Spi_t` matches the backend.** `src/hal/hal_spi.h` (`name`, `init(clock_hz,mode,cs)`, `transfer(tx,rx,len)`, `cs_assert(cs)`, `cs_release(cs)`) is implemented exactly by `ff_spi_mpfs.{c,h}` via the `FF_Spi_t spi` member and `FF_SpiMpfsSetup`; the driver-specific self-test is outside the vtable.
- **Scope statements accurate / no over-claim.** "SPI implemented, M11 8/13" is correct (GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C/SPI = 8 peripheral-bound); the emulated `SPI.MPFS_SPI` **no-registered-slave** limitation (register round-trips only, transfer path host-tested) is stated consistently in `renode-environment.md:0.15`, `verification-results.md`, the driver header (`ff_spi_mpfs.h:233-236`), SRS-REQ-069 and the README driver table.
- **Header/History agreement** holds for every touched document except `docs/development-plan.md` (B-02): SRS 1.19=1.19, traceability 0.31=0.31, SDD 0.7=0.7, software-description 0.4=0.4, verification-results 0.36=0.36, renode-environment 0.15=0.15, tool-qualification 0.3.19=0.3.19, class-a-gaps 0.9=0.9, handover 1.11=1.11, loop-audit 0.15=0.15, sprint-plan 0.7=0.7.
- **CI markers** in both `.github/workflows/ci.yml` and `.gitea/workflows/ci.yml` add `SRS-REQ-069` to the `ci-renode` evidence comment; `trace.py` validates them (PASS).

## 5. Scope conformance

**Implemented content vs documented scope (standing rule).**

- **Within scope, no over-implementation.** The driver implements exactly the `FF_Spi_t` contract (`init`, `transfer`, `cs_assert`, `cs_release`) and nothing beyond it: no DMA, interrupt, slave or enhanced/command framing. The extra register `#define`s (`INT_MASKED`, `INT_RAW`, `CTRL_ENH`, `CMD`, `PACKETSIZE`, `CMDSIZE`, `SLAVE_HWSTATUS`, `STATUS8`) document the map only and are unused. The MMIO self-test is a documented bring-up check with disclosed side effects.
- **Consistent with the scope documents.** `docs/roadmap.md` M4 lists SPI among the native MSS peripheral layers; `docs/development-plan.md` OBJ-10/OBJ-19 and the M11 objective cover the SPI class; the implementation matches.
- **SCOPE-01 resolved.** The `development-plan.md` M4/M11 status rows now agree with the implementation (SPI done, M11 8/13).
- **No scope NC raised in this review.** The increment is a reference driver within the documented M4/M11 scope; the scope statements are accurate and free of over-claim.

No deviation was identified.

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The R1-01 Major is resolved for the primary failure mode (tx-only / `rx == NULL` → subsequent full-duplex), with a genuinely discriminating regression test (`test_transfer_drains_rx`); R1-02, R2-01, R2-02 and SCOPE-01 are all closed. The increment is within scope, the requirement is well-formed and correctly parented, the contract matches, the trace chain converges (113/70/29/14/51), and the requested gate set passes first-hand (21/21 native + ASan, `test_spi` 120 checks, `trace.py` PASS, `metrics --check` current, `clang-format` clean). No Major finding is open, so the delta review is **APPROVED WITH COMMENTS**. The three Minor findings (B-01 check-count, B-02 history order/header, B-03 sprint-plan S4 row) and the residual D-01 (timeout drain) must be corrected/justified before commit (norm N12).

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1+R2 (delta) + R3+R4 (N9/N14/N17) | Senior FSW engineer, Senior SW V&V engineer, Senior avionics/systems engineer and Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — no Major open; Minors B-01/B-02/B-03 + residual D-01 to closure (N12) |

## 8. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| R1-01 | **Major** | SRS-REQ-069 `transfer` RX | Undrained RX FIFO / stale `RXDATARCVD` → next full-duplex transfer mis-reads | Drain `RXDATA` when `rx == NULL` + discriminating test | `test_transfer_drains_rx` + full gate set (120 checks, 21/21) | frick-fsw | **closed (primary)** | done | S4 delta |
| R1-01-res (D-01) | Minor | `transfer` timeout path | On `FF_TIMEOUT` the driver returns **before** draining `RXDATA`; the code comment claims timed-out transfers leave no stale byte (untrue) | Drain/clear `RXDATA` (or flush at transfer start, or document post-timeout recovery) and align the comment | (delta) driver comment + optional test using a mock that completes a frame after the poll bound | frick-fsw | open | pending | S4 delta (pre-commit) |
| R1-02 | Minor | SRS-REQ-069 pre-init | `cs_assert`/`cs_release` bypassed the ready check | Gate on `spi_check_ready()` + `test_cs_preinit` | `ff_spi_mpfs.c:147,162`; `test_spi.c:379-392` | frick-fsw | closed | done | S4 delta |
| R2-01 | Minor | V&V test discrimination | Mock could not model a non-overwriting RX FIFO | Non-overwriting FIFO + regression test | `test_spi.c:143-150,357-377` | frick-fsw | closed | done | S4 delta |
| R2-02 | Minor | SRS-REQ-069 evidence wording | "mock MMIO master/slave" overstated the evidence | Align SRS text | `SRS.md:320,334` | frick-fsw | closed | done | S4 delta |
| SCOPE-01 | Minor | Scope conformance (N8/N16/N19) | `development-plan.md` M4/M11 status stale | Update rows + history | `development-plan.md:310,317` + history 0.21 | frick-fsw | closed | done | S4 delta |
| B-01 | Minor | R4 — evidence accuracy | `test_spi` recorded as 109 checks; actual 120 | Update the count in `verification-results.md`, `sprint-plan.md`, `CHANGELOG.md`, `handover.md` | (delta) diff + `test_spi` re-run | frick-fsw | open | pending | S4 delta (pre-commit) |
| B-02 | Minor | R4 — revision history (N16) | `development-plan.md` history out of order; header 0.21 ≠ last row 0.20 | Reorder; make the last row 0.21 | (delta) diff | frick-fsw | open | pending | S4 delta (pre-commit) |
| B-03 | Minor | R4 — sprint-plan consistency | §2 S4 row not marked delivered; cites SRS-REQ-031 not SRS-REQ-069 | Mark S4 delivered; cite SRS-REQ-069; annotate exit criterion | (delta) diff | frick-fsw | open | pending | S4 delta (pre-commit) |
| B-04..B-08 | Comment | R4/R3 | status.json sprint counter, trailing newline, loop-audit baseline header, review-log row, "controllers" plural | Correct where cheap / justify | (delta) diff | frick-fsw | n/a | — | S4/S5 |
| LOG-01 | Comment | N11/N20 — records | Review-log row for FRICK-REVIEW-0142/0143 not yet in `docs/reviews/README.md` | Add rows with the commit | `docs/reviews/README.md` diff | frick-fsw (authoring agent/coordinator) | open | — | S4 (commit) |

> Norm N20 (PA) review of this increment remains **out of scope of this combined report** and is to be run separately before commit/push. This reviewer wrote only this report.

## 9. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native build | `cmake --build build/native` | green (25/25 targets linked, incl. `test_spi`) |
| Native ctest | `ctest --test-dir build/native` | **21/21 pass**, incl. `spi` |
| ASan/UBSan ctest | `ctest --test-dir build/asan` | **21/21 pass**, incl. `spi` |
| `test_spi` checks | `./build/native/tests/test_spi` | `checks=120 failures=0` |
| Traceability | `trace.py docs/requirements/traceability.yaml` | **PASS (113 requirements, 70 srs, 29 sss, 14 gaps, 51 theorems)** |
| SSS-REQ-020 children | `trace.py` bottom-up summary | **14** srs children (incl. SRS-REQ-069) |
| Metrics panel | `metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror` on `ff_spi_mpfs.{c,h}`, `test_spi.c`, `app_renode_test.c` | clean (exit 0) |
| R1-01 drain (source) | read `ff_spi_mpfs.c:119-133` | `RXDATA` read on every frame; discarded when `rx == NULL` |
| R1-01 regression | read `test_spi.c:143-150,357-377` | non-overwriting FIFO + `test_transfer_drains_rx` (new frame, not stale) |
| R1-01 residual (timeout) | read `ff_spi_mpfs.c:122-124` vs comment `125-128` | returns before drain on `FF_TIMEOUT`; comment over-claims |
| R1-02 | read `ff_spi_mpfs.c:137-168`; `test_spi.c:379-392` | ready check present; `FF_NOT_READY` pre-init |
| R2-02 | read `docs/requirements/SRS.md:320,334` | "behavioural MMIO mock" |
| SCOPE-01 | read `docs/development-plan.md:310,317` + history | M4 SPI done; M11 8/13 |
| Check-count claim | `grep "109"` + `test_spi` run | docs say 109; actual 120 → B-01 |
| development-plan history | read `development-plan.md` header + tail | order 0.19→0.21→0.20; header 0.21 ≠ last row 0.20 → B-02 |
| sprint-plan §2 S4 | read `sprint-plan.md:45` vs `:44,:109` | S4 not marked delivered; cites SRS-REQ-031 → B-03 |
| Header/History (all touched docs) | script over `| Revision`/`| Date` + last history row | all agree except `development-plan.md` |
| Contract match | `src/hal/hal_spi.h` vs `ff_spi_mpfs.h`/`.c` | exact `FF_Spi_t` vtable |
| CI markers | both `ci.yml` comments | `SRS-REQ-069` present in `ci-renode` |
| Scope limit (emulated) | `ff_spi_mpfs.h:233-236`, `renode-environment.md:0.15`, `verification-results.md` | no registered slave → register round-trips only (consistent) |

> The gate commands regenerate build artifacts under `build/` (git-ignored) only; **no tracked source or documentation file was modified** by this review.

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- `spi_transfer` reads `RXDATA` on every frame and discards it when `rx == NULL`; the mock is a non-overwriting one-deep RX FIFO; `test_transfer_drains_rx` demonstrates the new frame is returned (R1-01 primary fix effective and discriminating).
- `cs_assert`/`cs_release` now gate on readiness and return `FF_NOT_READY` pre-init; `test_cs_preinit` asserts it (R1-02).
- On the `FF_TIMEOUT` branch the driver returns before reading `RXDATA`, while the code comment claims a timed-out transfer leaves no stale byte (D-01).
- SRS-REQ-069 validation wording is "behavioural MMIO mock"; the requirement is parented to SSS-REQ-020, method-annotated `Tst | Dem`, and free of over-claim (R2-02).
- `development-plan.md` M4/M11 rows are updated to SPI-done / M11 8/13 (SCOPE-01).
- `test_spi` reports **120** checks; four documents record **109** (B-01).
- `development-plan.md` history is out of order and its header `Revision` (0.21) ≠ last history row (0.20) (B-02).
- `sprint-plan.md` §2 S4 row is not marked delivered and cites only SRS-REQ-031 (B-03).
- The contract `FF_Spi_t` is implemented exactly; the trace chain converges at 113/70/29/14/51; all requested gates pass.

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0143`* — owner: review lead; impact: report identity only.
- *The uncommitted working-tree delta is the item under review atop `5ca2045`.*
- *`FF_SPI_MPFS_CLK_HZ = 100 MHz` is the reference APB clock* — owner: hardware/mission (GAP-A5); validation: confirm the configured MSS SPI APB clock against the RTPFS160ZT datasheet.
- *`RXDATARCVD` is a level bit cleared by reading `RXDATA`* — as modelled by the mock and stated in the driver header; validation: CoreSPI datasheet under GAP-A5.
- *The Renode/Lean/QEMU/RTEMS/MISRA results in `verification-results.md` were not re-run here* (outside the requested gate set); they were read, not reproduced.

**Inferences**

- Because the timeout branch returns before draining, a frame completed after the poll bound can leave a stale byte that the next full-duplex transfer reads first (D-01) — the R1-01 failure mode persists on that error path; the in-code comment is therefore inaccurate.
- Because the `test_spi` count grew from 109 to 120 with the two added regression tests and the doc count was not refreshed, the recorded V&V evidence understates the executed checks (B-01).
- Because the new `development-plan.md` history row was inserted above 0.20, the file's history ordering and header/last-row invariant (N16) are violated (B-02).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. Correct B-01/B-02/B-03 (Minors) and D-01 in the corrective commit; re-run the full gate set (N15) and update the affected histories (N8/N16).
2. Resolve D-01 by draining/clearing `RXDATA` on the timeout path (or flushing at transfer start) and aligning the comment; add a mock mode that completes a frame after the poll bound if the fix is to be tested.
3. Add the `docs/reviews/README.md` log rows (LOG-01) and run the PA (N20) review before commit/push.
4. Keep the GAP-A5 on-target residuals (APB clock, per-rate SCL, real-bus/FIFO behaviour, wall-clock latency) tracked.

**Confidence:** **High** for the Part A closure determinations, the contract match, the trace convergence and the gate results (all reproduced first-hand and internally consistent). **High** for B-01/B-02/B-03 (direct document/source reads). **Medium** for D-01's on-target materiality (rests on the documented `RXDATARCVD`/FIFO semantics; the exact post-timeout completion behaviour is a GAP-A5 residual). **Medium** for B-04 (the loop-start re-baseline convention may make it intentional).

**Open gaps**

- **D-01 (Minor) open** — timeout path does not drain `RXDATA`; comment over-claims.
- **B-01/B-02/B-03 (Minors) open** — evidence count, history ordering/header, sprint-plan S4 row.
- **B-04..B-08, LOG-01 (Comments)** — status counter, trailing newline, loop-audit baseline header, review-log row, "controllers" plural.
- **No independent oracle** for the transfer path (accepted limitation); register offsets/reset values only are cross-checked (by Renode).
- **On-target evidence absent** — APB clock, per-rate SCL accuracy, FIFO depth, wall-clock latency (GAP-A5).
- **PA (N20) review** of this increment not part of this report.
- **Document id** `FRICK-REVIEW-0143` retained as assigned.

---

*This report is authored to be committed and pushed with the reviewed change (norm N11). The review was read-only with respect to repository content: the reviewer wrote exactly one file (this report); no other tracked file was modified, created or deleted, and nothing was committed or pushed. The gate commands regenerate only git-ignored `build/` artifacts.*
