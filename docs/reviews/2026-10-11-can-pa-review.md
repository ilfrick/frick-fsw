# Review Report — Product Assurance review of the reference MSS CAN driver increment (norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0109                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | `a95cafb` + **uncommitted** working-tree increment (reference MSS CAN driver, SRS-REQ-057); HEAD `a95cafb` == `origin/main` == `gitea/main` |
| Items under review                | `src/drivers/can/mpfs/ff_can_mpfs.{c,h}` (new), `tests/unit/test_can.c` (new), `config/platform/platform_mpfs.h`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `src/apps/app_renode_test.c`; requirement SRS-REQ-057 (`docs/requirements/SRS.md` 1.15, `traceability.yaml`, `traceability.md` 0.27, SSS-REQ-020 parenting, `docs/design/software-design.md` 0.5); V&V evidence (`docs/vv/verification-results.md` 0.29, `docs/vv/renode-environment.md` 0.11, `docs/vv/tool-qualification.md` 0.3.13, `build/renode/console.log`); process/status (`CHANGELOG.md`, `README.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/planning/sprint-plan.md`, `docs/status/status.json`, `docs/rules.md`); the review evidence set for this increment |
| Date                              | 2026-10-11                             |
| Scope                             | Process and records for the SRS-REQ-057 increment (every phase product of the increment): whether the applicable process obligations are met (N15 full cycle incl. N18 Renode, N4 trace convergence, N10 tool register re-qualification, N11 records, N9/N14/N17/N20 review set); the sufficiency and integrity of the V&V evidence (reproducibility, counts, console markers, environment/limitation statements); traceability and baseline integrity (requirement ↔ design ↔ code ↔ test ↔ evidence); change/CM control (revision histories, CHANGELOG, single coherent increment, stated limitations); claim discipline; the residual-gap register. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | The reviewer took no part in authoring, implementing, executing, designing or reviewing the items; it is a fresh independent session with **no author context** (the task statement and the repository were the only inputs). Read-only for the repository: only this report file was created; **no tracked file was modified** (`git status` before/after: only the increment's own files + this report). Gates were re-executed read-only; all outputs land under the git-ignored `build/`. No commit, push, docker or Renode run was performed (another reviewer may hold the emulator). |
| Norms applied                     | N1–N20 (`docs/rules.md` 1.4); focus N4, N5, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the norm-N20 Product Assurance review of the milestone **M4/M11**
increment that adds the reference MSS CAN backend behind the HAL contract
`FF_CanBackend_t` (**SRS-REQ-057**, classic CAN 2.0 A/B), a host unit test with
a mock MMIO register file, and a `hal_can` clause to the Renode functional test.
The increment is **uncommitted**: it is a working-tree delta on top of
`a95cafb`. Part of this review is therefore to check that the **complete**
product/evidence set intended for the commit is coherent **before** it is
committed (N15 full cycle, N7 both remotes, N16 revision history, N9/N17/N20
reviews), rather than auditing a pushed revision (as the sibling PA reviews of
the eNVM/MTD increments had to do).

**Included:** every phase product of the increment (requirements, design, code,
test, configuration, documentation, verification records), the process
obligations it triggers, the factual accuracy of its recorded evidence (test
counts, cycle log, tool register, status/handover statements, README panel),
traceability convergence for SRS-REQ-057, and the honesty/limitation scoping of
the claims (emulation-only; register-level; not silicon/timing/off-chip).

**Excluded:** the internal correctness of the driver and the test (owned by R1
and R2); the Lean model (norm N1 is satisfied by an explicit `lean_note` for
this requirement, which is the project's declared route for non-Lean-checkable
artifacts); documentation adjudication owned by R4; on-target/silicon CAN
behaviour (timing, off-chip transceiver/bus, errata), which is GAP-A5 by the
increment's own scope. This review does not re-perform the R1/R2/R3/R4 reviews
and does not issue any certification, approval or release statement.

**Method.** Read the full working-tree delta and every in-scope artifact;
independently re-executed the project's local gates read-only (`trace.py`,
`metrics.py --check`, `ctest` on the native and ASan trees, `clang-format`,
`make smoke`, `make gsw-selfcheck`); measured the actual unit-test assertion
count and compared it with the recorded one; inspected the retained Renode
console capture against the claims and against the source mtimes; recomputed the
bit-timing arithmetic of the driver; and inspected the repository state
(committed vs untracked, review log, remotes) to determine whether the mandated
review evidence exists at the reviewed revision.

## 2. References

- Project norms: `docs/rules.md` (N1–N20), especially N4, N5, N8, N9, N10, N11,
  N12, N15, N16, N17, N18, N19, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-057,
  SSS-REQ-020 parenting, GAP-A1/A2/A4/A5).
- HAL contract: `src/hal/hal_can.h` (`FF_CanBackend_t`, SRS-REQ-029;
  context-free vtable — the driver's single-instance design follows from it).
- Records: `docs/vv/verification-results.md` (0.29), `docs/vv/renode-environment.md`
  (0.11), `docs/vv/tool-qualification.md` (0.3.13), `CHANGELOG.md`,
  `README.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`,
  `docs/planning/sprint-plan.md`, `docs/status/status.json`,
  `docs/development-plan.md`, `docs/roadmap.md`, `docs/quality/class-a-gaps.md`.
- Renode environment: `tools/scripts/renode_vv.sh` (pinned image digest
  `antmicro/renode@sha256:7905c3…b061`, console capture to
  `build/renode/console.log`), `tools/docker/renode/frick-renode.resc`.
- Precedents for the sequencing finding: `docs/reviews/2026-10-11-nvm-pa-review.md`
  (FRICK-REVIEW-0084, Major F1, same norm question at the prior increment) and
  `docs/reviews/2026-10-11-mtd-pa-review.md` (FRICK-REVIEW-0094, Major F1).
- ECSS-E-ST-40C Rev.1; ECSS-Q-ST-80C Rev.2 — software PA records, tool
  qualification, traceability matrices verified at each milestone.

## 3. Verification performed (independent), with results

| # | Gate / check (N) | Exact command or artifact | Result (independently reproduced) |
| - | ---------------- | ------------------------- | --------------------------------- |
| V1 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — `traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked)`, 12 CI claims; matches the README panel and `verification-results.md` 0.29 |
| V2 | Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — `metrics panel: current` |
| V3 | Unit suite, native (N6) | `ctest --test-dir build/native` | **19/19 PASS** (incl. `can`) |
| V4 | Unit suite, sanitizers (N6) | `ctest --test-dir build/asan` | **19/19 PASS** (ASan/UBSan tree contains `ff_can_mpfs.c` in `build.ninja`) |
| V5 | Test-count claim (`test_can` 60 checks) | `grep -c FF_TEST_ASSERT tests/unit/test_can.c` → 60; `./build/native/tests/test_can` | **60 asserts; `checks=60 failures=0`** — the recorded "60 checks" is exact |
| V6 | Format gate (N9/T) | `clang-format --dry-run --Werror` on the three new files | **exit 0** — clean |
| V7 | Smoke + ground segment (N6) | `make smoke`; `make gsw-selfcheck` | **PASS** (`banner + 6 applications registered`); self-check OK (CRC/frame size) |
| V8 | Renode functional evidence (N18) | `build/renode/console.log` (raw UART capture) | Contains `[renode-test] hal_can=PASS`, `sb_roundtrip=PASS`, **`RENODE_TEST: PASS`** (7 apps registered). Log mtime `21:20:33` **≥** newest source mtime (`ff_can_mpfs.h` `21:19:15`, Renode ELF `21:20:12`) → the capture corresponds to the reviewed sources |
| V9 | MISRA advisory claim (N10) | `build/misra-report.txt` (produced in the pinned V&V image, `/workspace` paths, mtime 21:22) | **13** lines matching `misra` (the script's counter) + 1 `preprocessorErrorDirective`, **none in `ff_can_mpfs.{c,h}` or `test_can.c`** — the "0 findings in the new files, 13 repo-wide pre-existing" claim is accurate (`cppcheck` is not installed on the host, so the scan itself was not re-run) |
| V10 | Lean claim (N1) | `verification/lean/.lake/build/lib/**` artifacts | Fresh `olean`/`c.o` artifacts at `21:21` (after the source edits) corroborate a successful `lake build`; the **tracked** transcript `verification/lean/lake-build.log` is **stale** (2026-10-10 16:14, last committed in `b4a1e5f`) — see PA-06 |
| V11 | Bit-timing arithmetic (product evidence) | Recompute from `ff_can_mpfs.c:66-80` + `FF_CAN_MPFS_CLK_HZ` | 125 kbit/s → 124 008 (−0.79 %); 250 kbit/s → 252 016 (+0.81 %); **500 kbit/s → 488 281 (−2.34 %)**; **1 Mbit/s → 976 563 (−2.34 %)** — see PA-02 |
| V12 | Traceability/baseline integrity (N4) | `traceability.yaml` SRS-REQ-057 block; `traceability.md:118`; SDD §6 row; SRS.md 1.15 | Requirement present, `parent_sss: SSS-REQ-020`, `derived: false`, implementation + tests non-empty, explicit `lean_note`; SSS-REQ-020 child count 9 → **10**; SDD §6 row gains `can/mpfs` + SRS-REQ-057 → README "Design traceability 58/58". **No orphan requirement, no requirement without test/evidence** |
| V13 | Change/CM control, repository state (N11/N16/N7) | `git status`, `git log`, `git branch -avv`, `git remote -v` | HEAD `a95cafb` = `origin/main` = `gitea/main`; the increment is **untracked/modified only**; 3 new files carry the SPDX `AGPL-3.0-only` header (N13) and no third-party/generated code is introduced; the review set for this increment **does not exist** (`ls docs/reviews/*can*` → none) and there are **no `docs/reviews/README.md` rows** for it (see PA-01) |
| V14 | Documentation currency (N8/N16/N19) | `grep` per document; histories | `SRS.md`, `traceability.md`, `traceability.yaml`, SDD, `verification-results.md`, `renode-environment.md`, `tool-qualification.md`, `CHANGELOG.md`, `loop-audit.md` **updated with history rows**; `development-plan.md`, `handover.md` (§4 + revision), `loop-audit.md` (revision) and `sprint-plan.md` **stale** — see PA-04 |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| PA-01 | **Major** | N15 / N9 / N17 / N20 / N11 — review set for this increment | **The N20 gate is not satisfied at the reviewed revision: the mandated review set for this increment does not exist yet**, and the increment must not be committed until it does. There is **no** R1 (code, N9), R2 (V&V/tool-qualification, N9/N10) or R4 (documentation, N17) report for SRS-REQ-057 — neither committed nor untracked — and **no `docs/reviews/README.md` log rows**; this PA report is only the fourth member of the set that the cycle requires. N15 requires the complete cycle (build/tests/Lean/trace/static analysis **plus the applicable N9/N10/N14 reviews and N12 close-out**) to finish *before* commit/push, and N17/N20 require the documentation and PA reviews to close *before* commit/push, the PA review **additional** to R1/R2/R4. Additionally `docs/status/status.json` still reports `documentation_review`, `reviews` and `product_assurance_review` as **green**, which for this increment is not yet true. This is the same gate the prior two increments breached (FRICK-REVIEW-0084 F1, FRICK-REVIEW-0094 F1); here the breach has **not** happened yet — the change is uncommitted — so the disposition is *close before commit*, not retro-fix. **This is a closure blocker, not a product defect.** | `git status` (increment untracked/modified, uncommitted); `ls docs/reviews/*can*` → none; `grep -in 'can-r1\|can-r2\|can-r4\|can-pa' docs/reviews/README.md` → 0 rows; `docs/status/status.json` semaphores; `docs/rules.md` N15/N17/N20; precedents 0084/0094 | Produce and record the complete set (**R1 0106, R2 0107, R4 0108** — plus R3 if the SSS-REQ-020 entry itself changes), append the `docs/reviews/README.md` rows, add this report (PA), regenerate the README metrics panel (`make metrics`) and reconcile `status.json` semaphores, then re-review the deltas (N12). **Downgrades to closed once committed and logged.** |
| PA-02 | Minor | SRS-REQ-057 / N10 / N11 — bit-timing accuracy | **The reference bit timing is silently inaccurate at standard CAN rates and the residual is not recorded or quantified anywhere.** With the driver's fixed 16-tq split (1 + TSEG1 13 + TSEG2 2) and the assumed 125 MHz MSS CAN clock, the nearest prescaler gives **−2.34 % bit-rate error at 500 kbit/s and 1 Mbit/s** (−0.79 %/+0.81 % at 125/250 kbit/s); 500 kbit/s is *unattainable* with 16 tq from 125 MHz (it needs prescaler 15.625, i.e. a different TSEG split). `can_configure()` returns `FF_SUCCESS` and reports nothing; the host oracle only asserts `prescaler != 0`; and SRS-REQ-057 contains **no measurable bit-timing criterion**, so the "set-up" clause is not verifiable to any bound. The header correctly flags the clock as needing datasheet confirmation (GAP-A5), but that does **not** cover the arithmetic error at the assumed clock, which no record states. | `ff_can_mpfs.c:66-80`; `ff_can_mpfs.h:118-125`; `test_can.c:168-175`; recomputation V11; `traceability.yaml` SRS-REQ-057 `lean_note` (no bit-timing statement); SRS.md:291-297 | Quantify and record the achieved bit rate/error (or the tolerance bound) in the driver header + `verification-results.md`, and let **R1** decide the technical fix (per-rate TSEG/prescaler selection, a `FF_NOT_SUPPORTED` for unattainable rates, or a stated accuracy bound). If a bounded criterion is intended, add it to SRS-REQ-057 (requirements set) so the oracle can discriminate. |
| PA-03 | Minor | `tests/unit/test_can.c` / N10 — "independent mock" claim and register-map provenance | The test header advertises "an **independent** mock double-word register file", but the mock is constructed from the **driver's own** register-map constants (`FF_CAN_REG_*`, `FF_CAN_TX_BASE`/`STRIDE`, `FF_CAN_RX_BASE`/`STRIDE`, `FF_CAN_ID_SHIFT`, DLC/IDE bit macros), so both sides share one source of truth: an error in the layout constants would pass the host suite undetected. Only four registers are cross-checked against a genuinely independent model (the Renode `MPFS_CAN` emulator, via `FF_CanMpfsSelfTest`). No record cites the **source** of the register map (datasheet section or the Renode platform model file), unlike the GPIO map which is recorded as "validated" in `handover.md` §6. | `test_can.c:40-105` (uses `FF_CAN_*` macros); `ff_can_mpfs.h:13-29` ("matches the Renode MPFS_CAN model") vs `ff_can_mpfs.c:190-233` (self-test touches Command 0x014, INTEN 0x004, TX0 ID, RX0 AMR_DATA only); emulator model lives inside the pinned image, not in-repo | Pin the addresses/bit positions from an independent source in the test (or add a compile-time/static assertion table), and cite the register-map provenance in the driver header and `handover.md` §6. Keep the Renode round trips as the independent cross-check. |
| PA-04 | Minor | N8 / N16 / N19 — documentation currency and revision history | **Four touched-or-affected documents are stale or lack their N16 history entry.** (a) `docs/development-plan.md` §6 still says the M4 residual drivers include "**CAN**… pending" and "M11 … in progress (**5/13** peripheral-bound)" with "Pending: …**CAN**…", directly contradicting the increment, `loop-audit.md` (updated) and `renode-environment.md`; its revision/History stop at 0.16 (the MTD increment). (b) `docs/status/handover.md` §4 keeps the stale snapshot "**5/13 peripheral-bound**" with a marker list **without `hal_can`** and "Requirements: **98 (57 srs** / 27 sss / 14 gap)", although the same file's §5 was edited by this increment; the **Revision stays 1.4 and no History row is added**. (c) `docs/status/loop-audit.md` content was edited (M4 row) with **no Revision bump and no History row** (history stops at 0.8; the N19 baseline commit `e1def89` is unchanged). (d) `docs/planning/sprint-plan.md` S2 still names "**CAN** … (SRS-REQ-**029**)" for this deliverable, does not record the increment, and its maintenance rule (§5) requires exactly that at loop start. | `git diff` for `loop-audit.md`/`handover.md` (content changed, header/History not); `development-plan.md:295,302,334`; `handover.md:12,97,101,120,186`; `sprint-plan.md:43,85-94`; README panel 6/13-vs-5/13 consistency | Bring `development-plan.md` (M4/M11 rows + revision/History), `handover.md` (§4 counts/markers + revision/History), `loop-audit.md` (revision/History) and `sprint-plan.md` (S2 → SRS-REQ-057, record delivered) into the same cycle (N8/N16/N19). |
| PA-05 | Minor | `ff_can_mpfs.{c,h}` / README / N8 / N11 — stated limitations | **Known limitations of the backend are not stated.** The driver never reads `TxStatus` (`FF_CAN_REG_TX_STATUS` is defined but unused) and uses a round-robin TX cursor over 32 buffers, so a still-pending TX buffer is **silently overwritten**; there is no bus-off/error/lost-arbitration handling and no `RX_MSG_LOSS` (bit 10) handling even though the header documents the bit; RX is polling-only (`FF_CanMpfsPoll`, not in the HAL contract). None of this appears in the driver header, README or `verification-results.md`; GAP-A5 covers only on-target timing/medium, not these functional limitations. The prior increments' PA reviews required exactly this "known limitations" statement for the MTD/NVM backends. | `ff_can_mpfs.c:83-113` (no TxStatus read), `:135-188` (poll ignores RX_MSG_LOSS), `ff_can_mpfs.h:59,111-116,146-150`; README:187; `verification-results.md:41`; `class-a-gaps.md:88-97` (GAP-A5 = on-target only) | Record the TX-backlog/bus-off/error-frame/RX-loss limitations (driver header comment + `verification-results.md` note), or implement the pending-buffer check; the technical disposition belongs to R1. |
| PA-06 | Comment | N10 / N11 — retained-evidence integrity | Two evidence-integrity weaknesses (both pre-existing practice, not introduced here, but they affect this increment): (a) the **tracked** Lean transcript `verification/lean/lake-build.log` is stale (2026-10-10, last committed in `b4a1e5f`) while this increment's record claims `lake build 21/21`; the claim is corroborated only by transitory artifacts under the git-ignored `.lake/`. (b) `build/renode/console.log` is git-ignored and `renode_vv.sh` retains **only the UART console** — the full Renode/docker log (which would carry the "unhandled write" warning that `ff_can_mpfs.h:111-115` claims and any model diagnostics) is discarded each run; the emulator-model statements in the driver header are therefore not verifiable from the repository. | `git log -1 -- verification/lean/lake-build.log` → `b4a1e5f` (2026-10-10); `verification/lean/.lake/build/lib/**` mtimes 21:21; `git check-ignore -v build/renode/console.log`; `tools/scripts/renode_vv.sh:52-64` | Refresh (or untrack) `lake-build.log` in the cycle; retain the full docker/Renode log (or record its hash) alongside the record, and cite/qualify the emulator-warning and model statements. |
| PA-07 | Comment | SRS-REQ-057 / N3 — verification-method precision | The declared method set is `Tst \| Dem`, but the observable "register-based and allocation-free" properties are **Inp**-type, and the Renode check is an automated **Tst**. Sibling M4/M11 driver requirements are inconsistent (SRS-REQ-055/056 declare `Tst \| Inp` for the same evidence structure). This does not invalidate the requirement (the SSS parent SSS-REQ-020 is also `Tst \| Dem`). | `SRS.md:283,288,297`; `SSS.md:89-92`; `traceability.md` verification-method mapping | Leave the adjudication to R2/R3 (requirements owners); align the declared methods across 055/056/057 or justify the difference. |
| PA-08 | Comment | N5 / N19 — residual-gap register | GAP-A5 ("On-target qualification") is present in the manifest, `class-a-gaps.md`, the compliance matrix and the V&V records, and the increment's on-target residual (silicon CAN timing, off-chip transceiver/bus) is correctly linked to it. However GAP-A5 carries **no named owner and no milestone** ("blocked on hardware"), while the `class-a-gaps.md` closure policy states each gap has an owner and an acceptance criterion — the tracking N19 asks for. Pre-existing, not specific to this increment. | `traceability.yaml:784-790`; `class-a-gaps.md:88-97,183-189` | Have the programme record owner + milestone/acceptance criterion for GAP-A5 (and confirm GAP-A1/A2/A4 remain correctly owned). |
| PA-09 | **Major** | SRS-REQ-057 (frame *receive*) / N12 / N15 — accept-all filter mis-encoded | **Added by the delta re-review (§10): the incremental "accept-all" RX acceptance mask is mis-encoded, so the receive claim of SRS-REQ-057 is not established — and the current V&V cannot detect it.** `FF_CAN_ACCEPT_ALL` is `0x1FFFFFFF` (`ff_can_mpfs.h:116`) and is written **raw** to the AMR register (`ff_can_mpfs.c:127`), but the AMR identifier-mask field is **bits 3–31** while the driver itself left-aligns identifiers with `FF_CAN_ID_SHIFT = 3` (`ff_can_mpfs.h:101`, `.c:105`). The raw value therefore sets only ID-mask bits 3–28, leaving ID-mask bits 29–31 = 0 ⇒ "must match ACR(=0)", so **extended identifiers ≥ `0x04000000` are silently rejected** although the header states "accepts every identifier" and "behaves as accept-all either way" (`ff_can_mpfs.h:110-115`). PA independently confirms the mis-encoding by bit arithmetic; the field layout is corroborated by the sibling independent sources (Renode `MPFS_CAN` AMR `WithValueField(3,29,…)`; Microchip `mss_can_filterobject.AMR {N_A:1;RTR:1;IDE:1;ID:29}` / `MSS_CAN_get_msg_filter_mask()`). The host test **cannot** catch it: the mock implements no filtering and the assertion is a tautology on the written constant (`test_can.c:208`). Reported as R2-01 (**Major, REJECTED**); adopted here as blocking at PA level (a latent receive defect the V&V gives false confidence in). | `ff_can_mpfs.h:101,116,110-115`; `ff_can_mpfs.c:105,127`; `test_can.c:208`; R2-01 (FRICK-REVIEW-0107) with its Renode/Microchip sources | Write the **left-aligned** don't-care mask (e.g. `(FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) | RTR/IDE mask`, or derive via the vendor `MSS_CAN_get_msg_filter_mask()` semantics), correct the header comment, and add a test that verifies the mask in the **identifier domain** (or a mock that models filtering); re-run `make renode`, `test_can`, `make trace` and re-review the delta (N12). Owner: R1/R2 + V&V. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive observations (no finding).** All *product-side* obligations of the
increment check out independently: the requirement (SRS-REQ-057) and its
SSS-REQ-020 parenting are present and converged (`trace.py` PASS 99/58/27/14/51,
12 CI claims — exactly as recorded); the unit suite is 19/19 natively **and**
19/19 under ASan/UBSan; `test_can` really has **60** asserted checks
(`checks=60 failures=0`); the Renode functional run is **real**, its console
contains `hal_can=PASS` and `RENODE_TEST: PASS`, and the capture post-dates the
reviewed sources; the record of the MISRA advisory scan is accurate (0 findings
in the new files, 13 pre-existing repo-wide); clang-format is clean; smoke and
the ground-segment self-check pass; the tool register (T3 gains `test_can.c`,
T17 gains `hal_can`, history 0.3.13), the cycle log (0.29), the CHANGELOG entry,
the SRS/traceability/SDD revisions and the loop-audit M4 row are updated in the
same cycle; the requirement's `lean_note` is honest and the emulation-only,
register-level scope is stated (no "silicon-validated", "certified" or
"flight-proven" claim appears anywhere in the delta). The increment is a single
coherent change with no scope creep and no new third-party/generated code. The
**only** disqualifying issue is the review-set closure gate (PA-01), which has
not yet been breached.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED at the reviewed revision** — **two Major** findings
  (**PA-01** review-set closure/log gate; **PA-09** = adopted R2-01, the
  accept-all AMR mis-encoding, independently confirmed at the code level),
  open Minors PA-02…PA-05 and Comments PA-06…PA-08. (PA-09 was added by the
  norm-N12 delta re-review in §10, after the sibling reports appeared.)

At the **initial** assessment (sections 1–9) the increment's product was sound
and its recorded evidence honest and independently reproducible, and the **only**
blocker was the missing review set — PA-01, a non-derogable N15/N17/N20 gate
that must close *before* the commit. The **delta re-review (§10)** confirms that
gate is still open (the reports now exist but are **not recorded**, and the
`metrics.py --check` "current" result is a **false green**) **and** adopts the
R2 `Major` **R2-01**, which the PA reviewer independently reproduced from the
code: the "accept-all" RX mask is written **un-shifted**, so it does not accept
every identifier and the current V&V cannot detect the defect. Both Majors must
be resolved and their deltas re-reviewed (N12) before the increment may be
committed.

**Expected conversion.** Once (i) the complete review set for this increment is
produced (R1 0106, R2 0107, R4 0108), **recorded/logged** with this report
(0109), the README panel regenerated and `status.json` reconciled (PA-01);
(ii) PA-09/R2-01 is fixed — left-aligned don't-care mask, corrected header
comment, and a behaviour-level test — and the full cycle re-run and re-reviewed;
and (iii) PA-02…PA-05 are corrected with the corrections re-reviewed, this
review **converts to APPROVED WITH COMMENTS** with PA-06…PA-08 tracked as
observations.

### 5.1 What a qualified human must still accept (this assistant cannot)

1. **Emulation-only CAN evidence.** SRS-REQ-057 is verified by a host mock and a
   register round trip on the emulated SoC. There is no CAN medium, no peer
   node, no timing/WCET and no silicon evidence: the emulated result is a
   **partial mitigation** of GAP-A5 (on-target qualification, blocked on
   hardware). A human must accept that residual for criticality B.
2. **Reference bit timing.** After PA-02 is recorded (or fixed), the programme
   must accept a bit-timing/bit-rate accuracy bound on the assumed 125 MHz MSS
   CAN clock, or require a per-rate configuration — this is a technical
   acceptance decision (R1 + system/hardware owner), not this review's.
3. **Register-map provenance and TX/error-path limitations.** A human must
   accept the cited register-map source (PA-03) and the stated limitations
   (PA-05: TX-pending overwrite, no bus-off/error/RX-loss handling,
   polling-only RX).
4. **Criticality-B residual.** GAP-A1 (deep formal verification), GAP-A2 (full
   PUS-C coverage), GAP-A4 (certified static analysis/WCET) and GAP-A5 remain
   open, with PA-08 (owner/milestone) to be assigned. No class-B claim
   (and certainly no class-A or certification claim) follows from this increment.
5. **The closure decision itself.** Committing, pushing to both remotes and
   closing the review loop (N7/N12/N20) is a human/authority decision; this
   review is an assessment, not an approval, certification or release authority.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0109 | 2026-10-11 | **REJECTED at the reviewed (uncommitted) revision — 2 Major (PA-01 review-set log gate; PA-09 = R2-01 accept-all AMR mis-encoding, confirmed); Minors PA-02…PA-05; Comments PA-06…PA-08. Expected to convert to APPROVED WITH COMMENTS once PA-01 and PA-09 close and the Minors are corrected and re-reviewed (N12).** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: reference MSS CAN driver increment (SRS-REQ-057), commit a95cafb + uncommitted working tree; HEAD == origin/main == gitea/main (nothing pushed).
Claim type: mixed (Facts on gates/records/counts; Inference on the N20-gate status; Recommendation for PA-02..PA-05).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md 1.4); ECSS-E-ST-40C Rev.1 (2025), ECSS-Q-ST-80C Rev.2 (2025); target criticality B (class-A gaps open).
Evidence consulted (issuer, ID/revision, artifact/version, date): repository working tree at a95cafb+delta; every artifact in the header table; gates re-executed read-only 2026-10-11 (see section 3).
Cross-check and reconciliation: recorded numbers (99/58/27/14/51; 19/19; 60 checks; hal_can=PASS + RENODE_TEST: PASS; 13 MISRA advisories, 0 in new files; format clean; metrics current) were all independently reproduced. The only mismatches found are documentary (PA-04) and the unprovenanced/unquantified items (PA-02, PA-03, PA-06).
Confidence: High on PA-01 (direct repository/state evidence) and on the gate reproductions; High on PA-04 (direct diff vs. header/history); Medium on PA-02 (the bit-timing arithmetic is exact for the documented assumption, but the true MSS CAN clock is not confirmed — GAP-A5); Medium on PA-03/PA-05 (code inspection only; internal design judgement is R1's).
Basis and detection limits: read-only inspection + gate re-runs; docker/Renode was NOT (re-)executed (reserved for another reviewer), so the Renode evidence was assessed from the retained capture and its timestamps, not re-run; cppcheck/lake are not installed on the host, so those scans were assessed from their artifacts; the Renode platform model (register map) is inside the pinned image and could not be opened.
Missing/inaccessible evidence (including paywalls): no CAN review reports exist (PA-01); the full Renode/docker log and the in-image Renode CAN model are not retained in-repo (PA-06); no silicon/datasheet-confirmed CAN clock (GAP-A5).
Assumptions and expiry/recheck date: the sibling R1/R2/R4 reports will be produced in this same cycle; recheck on the commit that closes them, and on any change to the bit timing, register map or the HAL CAN contract. This review is void if the sources reviewed change.
Consequences if wrong: if the sibling reviews are logged and PA-09 is fixed with the cycle re-run and re-reviewed (N12), both Majors close and the verdict becomes APPROVED WITH COMMENTS with PA-02..PA-08 tracked; if the increment is committed with the mis-encoded mask or without the recorded review set, PA-01/PA-09 stand as non-derogable-norm breaches in the revision history and a latent CAN receive defect ships.
Required independent human reviewer/authority: the project integrator/owner (and the hardware/system owner for the bit-timing and GAP-A5 acceptance) decide closure; this review confers no approval, certification or release authority.
```

## 8. What must be added/closed **before the commit** (closure checklist)

1. **PA-01** — the complete review set is now **produced** (`docs/reviews/2026-10-11-can-r1-review.md` 0106, `2026-10-11-can-r2-review.md` 0107, `2026-10-11-can-r4-review.md` 0108, this report 0109) but **not recorded**: append the four `docs/reviews/README.md` rows, regenerate the README panel (`make metrics`) and reconcile `docs/status/status.json`. If the R1/R2 reviews raise findings, close them (N12) and re-review the deltas before commit. (R3/N14 is not triggered: `SSS.md` and the manifest SSS entries were not modified, only the generated `traceability.md` mirror.)
2. **PA-09 (Major, = R2-01)** — fix the accept-all AMR write: use the left-aligned don't-care mask (or derive it via the vendor `MSS_CAN_get_msg_filter_mask()` semantics), correct the `ff_can_mpfs.h:110-115` comment, and add a test that verifies the mask in the **identifier domain** (or a mock that models filtering). Re-run `make renode`, `test_can`, `make trace` and re-review the delta (N12).
3. **Metrics/status reconciliation** — after the log rows are added, keep `python3 tools/scripts/metrics.py --check` green (`make metrics`) and reconcile the `docs/status/status.json` semaphores (`reviews`, `documentation_review`, `product_assurance_review`) with the actual state of this cycle.
4. **PA-02** — record the achieved bit-rate error / accuracy bound (or fix the TSEG/prescaler selection per rate); state the residual in the header + `verification-results.md`.
5. **PA-03** — pin/cite the register-map source independently; qualify or correct the "independent mock" wording.
6. **PA-04** — update `development-plan.md` §6 + revision/History, `handover.md` §4 + revision/History, `loop-audit.md` revision/History, `sprint-plan.md` S2 (SRS-REQ-057, delivered).
7. **PA-05** — state the TX-backlog/bus-off/error/RX-loss limitations (or implement the pending-buffer check).
8. **PA-06…PA-08 (Comments)** — refresh/untrack `lake-build.log`; retain the full Renode log (or its hash); assign owner/milestone to GAP-A5; align the SRS-REQ-057 verification-method declaration (R2/R3).
9. **N7** — after the full cycle is green, commit the coherent increment and push to **both** remotes (`origin`, `gitea`).

## 9. Evidence consulted (exact commands and results)

```text
git status --short
  → M CHANGELOG.md CMakeLists.txt README.md config/platform/platform_mpfs.h
    docs/design/software-design.md docs/requirements/SRS.md
    docs/requirements/traceability.md docs/requirements/traceability.yaml
    docs/status/handover.md docs/status/loop-audit.md docs/vv/renode-environment.md
    docs/vv/tool-qualification.md docs/vv/verification-results.md
    src/apps/app_renode_test.c tests/CMakeLists.txt
    ?? src/drivers/can/  ?? tests/unit/test_can.c        (nothing committed)
git log --oneline -1 → a95cafb tooling.md: board platform wording for the Renode env self-test
git branch -avv / git remote -v → main = origin/main = gitea/main = a95cafb (origin: github.com/ilfrick/frick-fsw; gitea: housefz.com/git/ilfrick/frick-fsw)
git diff --stat → 15 files changed, 109 insertions(+), 33 deletions(-) (+3 untracked files)

python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  → traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked); SSS-REQ-020 children 9→10
python3 tools/scripts/metrics.py --check → exit 0, "metrics panel: current"
ctest --test-dir build/native → 100% tests passed, 0 failed out of 19 (incl. #19 can)
ctest --test-dir build/asan   → 100% tests passed, 0 failed out of 19
grep -c FF_TEST_ASSERT tests/unit/test_can.c → 60
./build/native/tests/test_can → "/home/nicola/fsw/frick-fsw/tests/unit/test_can.c checks=60 failures=0"
clang-format --dry-run --Werror src/drivers/can/mpfs/ff_can_mpfs.c src/drivers/can/mpfs/ff_can_mpfs.h tests/unit/test_can.c → exit 0
make smoke → [smoke] PASS (banner + 6 applications registered)
make gsw-selfcheck → TC frame (18 B) 29100000000a020b1101010201020304cd1f; no --connect given; self-check only; exit 0
tr -d '\000' < build/renode/console.log | tail → [renode-test] hal_gpio/hal_serial/hal_mem/hal_eth/hal_net/hal_nvm/hal_can=PASS, sb_roundtrip=PASS, RENODE_TEST: PASS (7 applications registered)
stat -c '%y %n' build/renode/console.log build/renode/frick_fsw_renode.elf src/drivers/can/mpfs/ff_can_mpfs.h
  → console.log 21:20:33 ≥ ELF 21:20:12 ≥ ff_can_mpfs.h 21:19:15  (capture post-dates the sources)
cat build/misra-report.txt → 13 lines matching 'misra' (script counter) + 1 preprocessorErrorDirective; none for ff_can_mpfs*/test_can
grep -n 'malloc|calloc|realloc|free(' src/drivers/can/mpfs/* tests/unit/test_can.c → no match (allocation-free, as the record claims)
grep -c 'FF_CAN_REG_TX_STATUS|FF_CAN_INT_RX_MSG' src/drivers/can/mpfs/ff_can_mpfs.c → 0 (TX status/ RX-loss unused)

git check-ignore -v build/renode/console.log → .gitignore:2:build/ (raw Renode capture is not version-controlled)
git log -1 -- verification/lean/lake-build.log → b4a1e5f 2026-10-10 (transcript stale); .lake/build/lib/** artifacts 21:21
git show a95cafb… not needed — increment uncommitted

ls docs/reviews/ → no *can* report; grep -in 'can-r1|can-r2|can-r4|can-pa' docs/reviews/README.md → no rows (101 existing rows)
diff of docs/status/handover.md and docs/status/loop-audit.md → content changed, Revision/History NOT bumped
grep -n 'CAN' docs/development-plan.md → :295 "CAN/SPI/… pending"; :302 "5/13 … Pending: …/CAN/…" (stale)
grep -n 'SRS-REQ-029' docs/planning/sprint-plan.md → :43 S2 CAN backend (stale requirement id)
```

## 10. Delta re-review (norm N12) — sibling review set and subsequent evidence

**Re-review date 2026-10-11 (same day, after §1–§9).** Between the initial
assessment and this re-review the mandated sibling reports appeared as
**untracked** working-tree files. Norm N12 (and §8 item 1 of this report)
require the PA to re-review the deltas and record the closure state. This
section **supersedes** the initial PA-01 wording ("does not exist yet") and
adds finding **PA-09**.

### 10.1 Sibling review set — produced but **not recorded**

| Report (working tree, untracked) | Doc id | Profile | Verdict | Blocking (Major) findings |
| -------------------------------- | ------ | ------- | ------- | ------------------------- |
| `docs/reviews/2026-10-11-can-r1-review.md` | FRICK-REVIEW-0106 | R1 (SW dev, N9) | APPROVED WITH COMMENTS | none (R1-02…R1-12 Minor/Comment; no defect) |
| `docs/reviews/2026-10-11-can-r2-review.md` | FRICK-REVIEW-0107 | R2 (SW V&V, N9/N10) | **REJECTED** | **R2-01 Major** — accept-all AMR mis-encoded (adopted here as **PA-09**) |
| `docs/reviews/2026-10-11-can-r4-review.md` | FRICK-REVIEW-0108 | R4 (SW documentation, N17) | APPROVED WITH COMMENTS | none (R4-01…R4-06 Minor; R4-07…R4-13 Comment) |
| `docs/reviews/2026-10-11-can-pa-review.md` | FRICK-REVIEW-0109 | PA (N20) | **REJECTED** | **PA-01**, **PA-09** |

### 10.2 PA-01 status update — product of the gate now exists, gate **still open**

The *reports* now exist, so the initial fact "the review set does not exist yet"
no longer holds. The **gate**, however, is **not closed**:

- `docs/reviews/README.md` is **unmodified** — `git status --short
  docs/reviews/README.md` is empty and a `grep -in 'can-r[124]\|can-pa\|010[6-9]'`
  returns **0 rows**. Norm N11 requires the review results to be *recorded* and
  committed in the same cycle.
- Because the log is untouched, `python3 tools/scripts/metrics.py --check` still
  exits 0 with "metrics panel: current" — but this is a **false green**: the
  "Review streams closed 27/30" panel row is derived from
  `docs/reviews/README.md` (`metrics.py:41`), so with no rows added there is
  nothing to be stale. Adding the four rows **without** regenerating
  (`make metrics`) will make the gate fail; adding them **with** regeneration is
  required.
- `docs/status/status.json` still shows `reviews`, `documentation_review` and
  `product_assurance_review` as green (README rows 81/82/85) — not yet true for
  this cycle (R2 open, reports unrecorded).

⇒ **PA-01 remains open.** Disposition unchanged: log 0106/0107/0108/0109, run
`make metrics`, reconcile `status.json`, then commit (N7) and re-review.

### 10.3 PA-09 — the receive claim is not established (adopted R2-01, PA-confirmed)

PA independently reproduced R2-01 at the code level (see the PA-09 row in §4):
the AMR mask is written un-shifted while the identifier field is left-aligned,
so the mask covers only ID bits 0–25 and extended IDs ≥ `0x04000000` are
rejected — contradicting the header's "accepts every identifier … either way"
statement. The host oracle asserts only the literal constant, so the defect
passes CI. This is a **latent receive defect plus a verification-adequacy gap**;
it is **blocking** (Major) and cannot be dispositioned as a mere limitation. The
absolute AMR field layout on silicon remains governed by GAP-A5, but the
in-repo inconsistency (raw mask vs. `FF_CAN_ID_SHIFT`) is self-evident and must
be fixed regardless.

### 10.4 Adopted sibling findings (corroboration / cross-check)

- **R2-06 / R4-01…R4-06** are the same documentary non-conformities PA-04
  tracks (stale counts `5/13`, `57/58 srs`, `18/18`, `98 reqs`; missing
  Revision/History in `handover.md`/`loop-audit.md`/`development-plan.md`) —
  **corroborated**, PA-04 stands.
- **R2-02…R2-05** (missing boundary/negative tests, permissive mock,
  non-independent byte-order oracle) **corroborate PA-03** (the mock shares the
  driver's register-map constants) and add test-adequacy Minors owned by R2.
- **R4-09** aligns with PA-07 (`method:` convention); **R4-11** aligns with
  PA-03/PA-06 (emulator-model limitation not in the V&V records); **R4-10/R4-12**
  align with PA-02/PA-04.
- **R4-07/R4-08/R4-13** (README coverage rows from an untracked artifact,
  `ci-renode` evidence comment, `traceability.md` "six/five documented-only") are
  recorded here as corroborating observations.

### 10.5 Updated verdict

**REJECTED** at the reviewed (uncommitted) revision — **two Major** findings:
**PA-01** (review set not recorded/logged; `status.json`/panel not reconciled)
and **PA-09** (accept-all AMR mis-encoding; latent receive defect + false
V&V confidence), plus Minors PA-02…PA-05 and Comments PA-06…PA-08. Conversion to
**APPROVED WITH COMMENTS** requires PA-01 closure (log + `make metrics` +
`status.json`) and PA-09/R2-01 correction (left-aligned mask + corrected comment
+ behaviour-level test) with the full cycle re-run and the deltas re-reviewed
(N12), and correction/re-review of PA-02…PA-05.

---

Committed and pushed with the reviewed change (norm N11).
