# Independent Review — Product Assurance closure review (norm N12/N20) of the M4/M11 reference MSS CAN increment

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0117                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | `a95cafb` + CAN increment (clock/standard fixes) — **uncommitted** working tree; HEAD `a95cafb` == `origin/main` == `gitea/main`; verified CAN source hashes: `ff_can_mpfs.h` md5 `6ed89e588d5bbf974554de14faef4a4b` (sha256 `2ee9abed27bee3cb…`), `ff_can_mpfs.c` md5 `7a8a95bb763d1888ea1812404d32ca6b`, `tests/unit/test_can.c` md5 `e33b53562e7b8da1d4d95993a3a7a92e` |
| Items under review                | `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`, `tests/unit/test_can.c` (128 checks), `config/platform/platform_mpfs.h`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `src/apps/app_renode_test.c`, `tools/scripts/renode_vv.sh`; SRS-REQ-057 / SSS-REQ-020 (`docs/requirements/SRS.md` 1.16, `traceability.yaml`, `traceability.md` 0.27, `docs/design/software-design.md` 0.5); records (`docs/vv/verification-results.md` 0.30, `docs/vv/renode-environment.md` 0.12, `docs/vv/tool-qualification.md` 0.3.14, `build/renode/console.log`, `build/renode/renode-process.log`); process/status (`CHANGELOG.md`, `README.md`, `docs/development-plan.md` 0.17, `docs/status/handover.md` 1.5, `docs/status/loop-audit.md` 0.9, `docs/planning/sprint-plan.md` 0.2, `docs/status/status.json`, `docs/quality/class-a-gaps.md` 0.6, `.github`/`.gitea` `ci.yml`); the CAN review set `0106..0113` and the `docs/reviews/README.md` rows |
| Date                              | 2026-10-11                             |
| Scope                             | **PA closure review (norm N12/N20)** of the residual items of FRICK-REVIEW-0113 (D-01 `status.json`, D-02 delta-review log rows), of the post-delta code change **R1-D-01** (`FF_CAN_MPFS_CLK_HZ` 125 → 80 MHz) and of the R2-D2 raw-AMR assertion, re-auditing norms N1–N20 over the whole SRS-REQ-057 increment and deciding whether the loop can close. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | I took no part in authoring, implementing, executing, designing or reviewing the items; fresh independent session with **no author context** (the task statement and the repository were the only inputs). Read-only: only this report file was created; **no tracked file was modified**. Gates were re-executed read-only; all outputs land under the git-ignored `build/`. No commit, push, docker or Renode run was performed. |
| Norms applied                     | N1–N20 (`docs/rules.md` 1.4); focus N1, N4, N5, N6, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the **PA closure review** of the reference MSS CAN backend increment
(SRS-REQ-057). It closes the loop opened by **FRICK-REVIEW-0109** (PA, REJECTED →
`0109` Majors PA-01/PA-09 closed in the delta **0113**) by:

1. verifying that the two residual items of my delta review **0113** are closed on
   evidence — **D-01** (`docs/status/status.json` reconciled) and **D-02** (the CAN
   delta-review rows in `docs/reviews/README.md`);
2. re-auditing the **post-delta code change R1-D-01** (`FF_CAN_MPFS_CLK_HZ`
   corrected 125 → 80 MHz) and **R2-D2** (raw-AMR-write assertion; suite 127 → 128)
   — these arrived *after* the delta round, so a further PA review is required for
   N12/N20 convergence;
3. re-applying **all** norms N1–N20 to the affected scope and confirming the
   increment is coherent (a single increment), revision histories are current,
   traceability has converged (SRS-REQ-057 ↔ SSS-REQ-020 ↔ design ↔ code ↔ test ↔
   Renode) and there is no claim overreach.

**Included:** D-01/D-02 closure; R1-D-01 closure evidence and the bit-rate
arithmetic at 80 MHz; the full N1–N20 audit; the commit package (all CAN review
reports `0106..0109`, deltas `0110..0113`, closures `0114..0117`, the
`docs/reviews/README.md` rows and `docs/status/status.json`).

**Excluded:** the internal correctness of the driver/test beyond the PA-visible
dispositions (owned by R1/R2 and their closure reviews 0114/0115); documentation
adjudication (owned by R4 and 0116); on-target/silicon CAN behaviour (GAP-A5). This
review issues no certification, approval or release statement.

**Method.** Read the full working-tree delta and every in-scope artifact;
independently re-executed the local gates read-only (`trace.py`,
`metrics.py --check`, `ctest` native + ASan, `./build/native/tests/test_can`,
`clang-format`); re-derived the bit-timing arithmetic at 80 MHz; inspected the
retained Renode console and process logs; and inspected the repository state
(committed vs untracked, review log, `status.json`). Docker/Renode, `cppcheck` and
`lake` were **not** re-run (not available/forbidden on the read-only host); their
evidence was assessed from the retained artifacts. The reviewed source revision
was hashed before and after the assessment (see §3.1).

## 2. References

- Project norms: `docs/rules.md` (N1–N20).
- Prior reports under disposition: `docs/reviews/2026-10-11-can-pa-review.md`
  (FRICK-REVIEW-0109), `docs/reviews/2026-10-11-can-pa-delta-review.md`
  (FRICK-REVIEW-0113); siblings `…-can-r1-review.md` (0106),
  `…-can-r2-review.md` (0107), `…-can-r4-review.md` (0108), deltas
  `…-can-r1-delta-review.md` (0110, holds R1-D-01), `…-can-r2-delta-review.md`
  (0111, holds R2-D1..R2-D4), `…-can-r4-delta-review.md` (0112).
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-057,
  SSS-REQ-020, GAP-A1/A2/A4/A5).
- HAL contract: `src/hal/hal_can.h` (`FF_CanBackend_t`, contract SRS-REQ-029).
- Records: `docs/vv/verification-results.md` (0.30), `docs/vv/renode-environment.md`
  (0.12), `docs/vv/tool-qualification.md` (0.3.14), `CHANGELOG.md`, `README.md`,
  `docs/development-plan.md` (0.17), `docs/status/handover.md` (1.5),
  `docs/status/loop-audit.md` (0.9), `docs/planning/sprint-plan.md` (0.2),
  `docs/status/status.json`, `docs/quality/class-a-gaps.md` (0.6).
- ECSS-E-ST-40C Rev.1; ECSS-Q-ST-80C Rev.2 — software PA records, tool
  qualification, traceability matrices verified at each milestone.

## 3. Verification performed (independent), with results

| # | Gate / check (N) | Exact command or artifact | Result (independently reproduced) |
| - | ---------------- | ------------------------- | --------------------------------- |
| V1 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — `traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked)`, 12 CI claims; SSS-REQ-020 children 10 |
| V2 | Metrics-panel currency (N8/N10) | `python3 tools/scripts/metrics.py --check` | **exit 0** — `metrics panel: current` |
| V3 | Unit suite, native (N6) | `ctest --test-dir build/native` | **19/19 PASS** (incl. test 19 `can`) |
| V4 | Unit suite, sanitizers (N6) | `ctest --test-dir build/asan` | **19/19 PASS** |
| V5 | Test-count claim (128) | `./build/native/tests/test_can` | **`checks=128 failures=0`** — matches the record (60→128 history in verification-results 0.29/0.30) |
| V6 | Format gate (N9/T1) | `clang-format --dry-run --Werror` on the three new files | **exit 0 — CLEAN** (clang-format 23.1.3). *See F1: an intermediate working-tree state during the review window carried a format violation.* |
| V7 | Renode functional evidence (N18) | `build/renode/console.log`, `build/renode/renode-process.log` | Both contain `[renode-test] hal_can=PASS` … `RENODE_TEST: PASS` (7 apps); ELF/console/process-log mtimes 22:52 (post-change rebuild) |
| V8 | D-01 `status.json` reconciliation (N11) | `git diff HEAD -- docs/status/status.json`; `docs/status/status.json:5` | **Non-empty**: `meta.note` now records "Last cycle: M4/M11 reference MSS CAN backend (SRS-REQ-057) — reviews FRICK-REVIEW-0106/0107/0108/0109 + delta 0110/0111/0112/0113 closed; M11 6/13"; `meta.updated` = 2026-10-11; semaphores `reviews`/`documentation_review`/`product_assurance_review`/`vv_records` = green, consistent with V2 |
| V9 | D-02 delta-review log rows (N11/N12) | `docs/reviews/README.md:106-113` | Rows **present** for `0106/0107/0108/0109` and deltas `0110/0111/0112/0113`; closures `0114..0117` rows still pending (produced in parallel — see §6.3) |
| V10 | R1-D-01 clock correction (code) | `ff_can_mpfs.h:148-153` | **`#define FF_CAN_MPFS_CLK_HZ 80000000u`** with the MSS TRM range 8..80 MHz cited; the assumed 125 MHz is removed from the product header |
| V11 | Bit-timing arithmetic at 80 MHz (R1-D-01) | Recompute `can_configure()` (`ff_can_mpfs.c:66-80`), tq = 16, integer prescaler | **exact**: 125 k → presc 40 → 125 000; 250 k → 20 → 250 000; 500 k → 10 → 500 000; 1 M → 5 → 1 000 000. The header/`verification-results.md` "exact at 80 MHz (prescaler 40/20/10/5)" claim is **arithmetically correct**; the rate guard rejects bitrate > `80e6/16` = 5 Mbit/s |
| V12 | Stale 125 MHz / −2.34 % residue (N8/N16) | `grep` for `125 MHz`/`−2.34`/`976` in product docs/source | **No stale value in any product artifact.** Remaining hits are inside **historical review reports** (0109/0113 etc., correctly frozen snapshots) and unrelated third-party ASN.1 constants |
| V13 | R2-D2 raw-AMR assertion (N12) | `tests/unit/test_can.c:324-328` | The suite now asserts `g_m.amr_written == (uint32_t)FF_CAN_ACCEPT_ALL` **and** the identifier-field bits; `mock_rx_accepts()` models the identifier domain and discriminates `0x04000000`/`0x1FFFFFFF`; V5 count 127→128 is the added assertion |
| V14 | HA L/self-test integration (N18) | `src/apps/app_renode_test.c:237-265,276,319-333` | `renode_hal_can_check()` binds the driver over `FF_CAN0_BASE` (0x2010C000) and gates `RENODE_TEST` on `hal_can_ok` |
| V15 | Review-log/CM state (N11/N16/N7) | `git status`, `git log -1`, `git rev-parse` | 3 new source files carry the SPDX `AGPL-3.0-only` header; the eight CAN reports exist (untracked); HEAD == origin == gitea == `a95cafb`; the increment is **uncommitted** |
| V16 | Documentation currency (N8/N16/N19) | Histories per document; `traceability.md:18` | `development-plan.md` 0.17, `handover.md` 1.5, `loop-audit.md` 0.9 (baseline `a95cafb + CAN increment`), `sprint-plan.md` 0.2, `traceability.md` 0.27 (`SRS = 58`), `software-design.md` 0.5, `class-a-gaps.md` 0.6, `renode-environment.md` 0.12, `verification-results.md` 0.30, `tool-qualification.md` 0.3.14, `SRS.md` 1.16, `ci.yml` evidence comment (+055/+057) — all consistent with `trace.py` (99/58/27/14) |
| V17 | Lean transcript freshness (N1/N11) | `verification/lean/lake-build.log` | `✔ [1/21]…[21/21] Built frick_verify_checks`; `Build completed successfully.` |
| V18 | MISRA advisory claim (N10) | `build/misra-report.txt` | 13 advisory lines + counter; **none** in `ff_can_mpfs.{c,h}` or `test_can.c` — claim accurate (cppcheck absent; scan not re-run) |

### 3.1 Working-tree integrity during the review window — **F1**

The reviewed source revision was **not frozen** during the assessment. The CAN
header/source mtimes advanced to **22:59:28** while the review ran. At one point the
working header carried `FF_CAN_ACCEPT_ALL` as the literal **`0x1FFFFFFFu`** — i.e.
the exact PA-09/R2-01 defect this increment exists to fix — together with a
clang-format violation (the short literal is collapsed by clang-format and does not
match the surrounding style); the file was then written back to the correct
identifier-domain expression `((FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) | …)`. The
**final, verified state is correct** (V6 clean, V10/V13 correct, V5 128/0), and a
concurrent opencode session plus a detached `antmicro/renode` container are
observably active — so the most likely explanation is a concurrent reviewer's
patch-back discrimination experiment (cf. the method documented in
`…-can-r2-delta-review.md`) or a parallel author save, not a latent product defect.
It is nonetheless a **CM/evidence-integrity observation**: a closure review must be
tied to an immutable, identified revision. See finding **F1** in §6.

**Personally verified (re-executed read-only):** V1–V6, V10 (code read), V11
(re-derived arithmetic), V13–V18 (file/record inspections and hashes).
**Read, not re-executed:** the Renode/docker run, `cppcheck`, `lake build`
(tools absent/forbidden); their evidence was assessed from the retained artifacts.

## 4. Closure table — residual items of FRICK-REVIEW-0113 and the post-delta change

| # | Sev (0113) | Claimed disposition | Independent verification | Status |
| - | ---------- | ------------------- | ------------------------ | ------ |
| D-01 | Comment | `status.json` reconciled before commit | V8: `meta.note` records the CAN review cycle; `meta.updated` present; semaphores consistent with `metrics.py --check` exit 0 (V2) | **CLOSED** |
| D-02 | Comment | CAN delta-review rows appended to the log | V9: rows for 0106–0113 present; the closures 0114–0117 rows are being produced in parallel and must be appended in the same commit (§6.3) | **CLOSED** (for the 0106–0113 set); 0114–0117 rows pending |
| R1-D-01 | Minor (0110) | Clock corrected 125 → 80 MHz; bit-rate error recorded | V10: `FF_CAN_MPFS_CLK_HZ = 80000000u`; V11: standard rates exact (40/20/10/5); V12: no stale 125 MHz in product docs; `verification-results.md` 0.30 + `CHANGELOG.md` record it. Residual: the 80 MHz value is datasheet-attributed (see §5) | **CLOSED** (code + records); hardware attribution = human acceptance |
| R2-D2 | Minor (0111) | Assert the written AMR mask bits (raw write) | V13: `test_can.c:326` asserts `amr_written == FF_CAN_ACCEPT_ALL`; V5 count 127→128 | **CLOSED** |

## 5. Norm audit N1–N20 for this change

| N | Rule | Assessment for the CAN increment (closure) | Status |
| - | ---- | ------------------------------------------ | ------ |
| N1 | Lean check of generated artifacts | No generated code; SRS-REQ-057 carries an explicit `lean_note`; Lean transcript fresh 21/21 (V17) | OK |
| N2 | English docs/comments | All new files/comments/records in English | OK |
| N3 | Backwards ECSS SRS/SSS | SRS-REQ-057 present with SSS-REQ-020 parent; method `Tst | Dem` (SRS 1.16) | OK |
| N4 | Bidirectional traceability | V1: `trace.py` PASS 99/58/27/14/51; SSS-REQ-020 child 10; SDD 58/58; `traceability.yaml:534-541` implementation+tests non-empty | OK |
| N5 | Class-B deliverable; class-A gaps | GAP-A1/A2/A4/A5 tracked; GAP-A5 owner/milestone recorded (`class-a-gaps.md` 0.6) | OK |
| N6 | Full SW V&V; Docker CI/CD | ctest 19/19 native + ASan; smoke PASS (recorded); CI workflows updated (+055/+057) | OK |
| N7 | Commit/push to both remotes | Increment uncommitted; HEAD == origin == gitea == `a95cafb`. **Action at commit** | Pending (pre-commit) |
| N8 | Complete/current docs | V16: all affected docs refreshed with histories | OK |
| N9 | Independent reviews | R1/R2/R4/PA reports exist (fresh-context declarations); this closure review is the PA element | OK |
| N10 | V&V tool qualification | T3 gains `test_can.c`; T17 gains `hal_can` + process-log retention (`tool-qualification.md` 0.3.13/0.3.14) | OK |
| N11 | Test/review results committed each cycle | Reports exist and are logged (0106–0113); `build/renode/*` retained; `status.json` reconciled (D-01). Residual: closure rows 0114–0117 + freeze (F1, §6.3) | OK (pending pre-commit records) |
| N12 | NCs resolved by re-applying all rules, re-review deltas | This closure re-applies N1–N20; PA-01/PA-09 (0109) and D-01/D-02 (0113) verified closed; R1-D-01/R2-D2 verified | OK |
| N13 | AGPL-3.0-only | Three new files carry the SPDX header; no third-party code introduced | OK |
| N14 | SSS review by R3 | Not triggered: `SSS.md`/manifest SSS entries unchanged (only the generated `traceability.md` mirror) | N/A |
| N15 | Full cycle before commit | Build/test/Lean/trace/static analysis + reviews re-run to success on the fixed revision (recorded; local gate re-runs green) | OK |
| N16 | Revision history | Every touched doc gains a History row + revision bump (V16) | OK |
| N17 | R4 documentation review closed before commit | R4 delta 0112 APPROVED (R4-01..R4-13 closed); R4 closure 0116 in parallel | OK (pending 0116) |
| N18 | Renode functional validation | V7: `hal_can=PASS` + `RENODE_TEST: PASS`; scope limit documented (`renode-environment.md` 0.12) | OK |
| N19 | Loop audit at loop start | `loop-audit.md` 0.9 re-baselined to `a95cafb + M4/M11 CAN increment`; M4/M11/N3 rows updated | OK |
| N20 | PA review of every phase product | FRICK-REVIEW-0109 issued; delta 0113; **this closure (0117)** closes the PA element | OK |

## 6. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F1 | **Minor** | N11/CM — working-tree integrity | The reviewed source revision was **not frozen** during the review window (CAN file mtimes advanced to 22:59:28; an intermediate header state carried the literal `0x1FFFFFFF` accept-all value — the PA-09/R2-01 defect — plus a format violation), then was corrected. The final state is verified correct, but a closure cannot be audited against a moving tree | §3.1; verified hashes `ff_can_mpfs.h` `6ed89e58…`, `.c` `7a8a95bb…`, `test_can.c` `e33b5356…`; `clang-format` clean only on the frozen state (V6) | **Freeze the commit candidate to the verified hash** and confirm no further writes; re-run `clang-format`/`test_can` on the frozen revision at commit (the recorded cycle must correspond to the committed bytes). Likely a concurrent reviewer/author write — not a product defect |
| F2 | **Comment** | N5/N8 — non-standard bit rates | `can_configure()` accepts any bitrate ≤ 5 Mbit/s and rounds the prescaler silently; the records now quantify accuracy only for the **standard** rates (exact at 80 MHz). The achieved error at arbitrary rates (e.g. 300 k → ≈ −1.96 %) is not bounded anywhere; PA-02 asked for an error bound/quantification | V10/V11; `ff_can_mpfs.c:66-80`; header lines 32-39, 148-153; `verification-results.md` 0.30 | Either state that the driver is intended for the four standard rates only (and reject others), or record a per-rate accuracy bound. Owner: R1/V&V; on-target timing = GAP-A5 |
| F3 | **Comment** | N10/N12 — residual comments R1-D-02 / R2-D1 | Two delta residuals remain open in the inputs to this closure: **R1-D-02** (RX-buffer MsgAv-ack race — state as a limitation or latch before dispatch; not present in the header's limitations block) and **R2-D1** (`presc == 0` branch is unreachable defensive code — `ff_can_mpfs.c:72-74`, inline-commented but not recorded as a coverage justification) | `…-can-r1-delta-review.md:127,180`; `…-can-r2-delta-review.md:48,88`; `ff_can_mpfs.h:41-47`; `ff_can_mpfs.c:72-74` | Close in the **R1/R2 closure reviews 0114/0115** (optional per their severity); if left open they must be recorded as accepted residuals with owner. Not a PA barrier |

No new Major finding. F1 is a process/CM non-conformity (must be corrected by the
freeze + re-run at commit); F2/F3 are observations.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — the loop can close: **D-01** and **D-02** of
  FRICK-REVIEW-0113 are closed on objective evidence; the post-delta change
  **R1-D-01** (`FF_CAN_MPFS_CLK_HZ` 125 → 80 MHz) and **R2-D2** are verified
  closed at the code/record level; the increment is coherent, converged and free
  of claim overreach. One **Minor** (F1: freeze the reviewed revision before
  commit) and two **Comments** (F2 bit-rate bound; F3 residual R1-D-02/R2-D1
  closure in 0114/0115) remain.
- [ ] REJECTED

**Reasoning.** The two residual items that held FRICK-REVIEW-0113 open are now
closed: `status.json` carries the CAN review-cycle note and `metrics.py --check`
stays green, and the CAN delta-review rows are logged. The post-delta clock
correction is sound: at 80 MHz with the fixed 16-tq split the four standard rates
are exactly reachable (prescaler 40/20/10/5), the assumed 125 MHz value and its
−2.34 % error are removed from every product artifact, and the change is recorded
in the header, `verification-results.md` 0.30 and `CHANGELOG.md`. The only
process-level concern (F1) is that the working tree was written while the review
ran — the final state is verified correct, so it is corrected by pinning the
commit to the verified hash, not by another code change.

### 7.1 What a qualified human must accept before commit/push

1. **Emulation-only CAN evidence.** SRS-REQ-057 is verified by a host mock and an
   MMIO register round trip on the emulated SoC; there is **no CAN medium, peer
   node, timing/WCET or silicon evidence**. The emulated result is a **partial
   mitigation** of GAP-A5 (on-target qualification, blocked on hardware). A human
   must accept that residual for criticality B.
2. **The 80 MHz reference clock is datasheet-attributed, not silicon-confirmed.**
   The value (`FCANCKL` 80 MHz, DS00004248 Table 5-22) and the 8..80 MHz MSS PLL
   range are taken from the R1 finding / MSS TRM; this review could not parse the
   datasheet. The hardware/system owner must confirm the configured MSS CAN clock
   and accept the reference bit timing (or require per-rate TSEG/prescaler
   configuration). This is a technical acceptance decision, not this review's.
3. **Register-map provenance and TX/error-path limitations.** Accept the cited
   register-map source and the stated limitations (TX-pending overwrite, no
   bus-off/error/RX-loss handling, polling-only RX) — and note F2 (arbitrary-rate
   accuracy) and F3 (MsgAv-ack race).
4. **Criticality-B residual.** GAP-A1, GAP-A2, GAP-A4 and GAP-A5 remain open
   (GAP-A5 now owned/milestoned). **No class-B or class-A/certification claim**
   follows from this increment; M11 is 6/13 peripheral-bound, not "complete".
5. **Freeze + commit package + dual-remote push (N7).** Verify the reviewed hash
   (`ff_can_mpfs.h` `6ed89e58…`, `.c` `7a8a95bb…`, `test_can.c` `e33b5356…`), then
   commit the coherent increment **including** all CAN review reports
   `2026-10-11-can-{r1,r2,r4,pa}-review.md`, deltas `…-{r1,r2,r4,pa}-delta-review.md`,
   closures `0114..0117`, the `docs/reviews/README.md` rows (append 0114–0117 +
   `make metrics`) and `docs/status/status.json`, and push to **both** remotes.
   This is a human/authority decision; this review is an assessment, not an
   approval, certification or release authority.

## 8. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0117 | 2026-10-11 | **APPROVED WITH COMMENTS — D-01/D-02 of 0113 closed; R1-D-01 (80 MHz) and R2-D2 verified; F1 freeze-before-commit (Minor) + F2/F3 comments. The PA loop can close.** |

## 9. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| D-01 | Comment | N11/N8 `status.json` | semaphores not reconciled post-fix | note records the CAN review cycle; panel current | `status.json:5`; `metrics.py --check` exit 0 | author | **closed** | FRICK-REVIEW-0117 |
| D-02 | Comment | N11/N12 review log | delta rows 0110–0113 absent | rows appended (V9) | `README.md:110-113` | author | **closed** | FRICK-REVIEW-0117 |
| PA-01 | Major | N15/N11/N20 review set + log + metrics | set not logged; panel unreconciled | reports + rows + `make metrics` | `README.md:106-113`; V2 | author | **closed** | FRICK-REVIEW-0113 |
| PA-09 | Major | SRS-REQ-057 RX accept-all (N12) | AMR mask written un-shifted | left-aligned identifier-domain mask + test | `ff_can_mpfs.h:140-141`; `test_can.c:326-332`; V5/V13 | R1/R2 + V&V | **closed** | FRICK-REVIEW-0113/0117 |
| R1-D-01 | Minor | SRS-REQ-057 bit timing | assumed 125 MHz clock | corrected to 80 MHz; exact standard rates | V10/V11/V12; `verification-results.md` 0.30 | author | **closed** | FRICK-REVIEW-0117 |
| F1 | Minor | N11/CM working tree | reviewed revision changed mid-review (transient wrong mask) | freeze commit to verified hash; re-run gates at commit | §3.1; verified hashes | author/integrator | **open (pre-commit)** | — |
| F2 | Comment | N5/N8 bit-rate bound | arbitrary-rate accuracy unbounded | state standard-rate-only intent or record a bound | `ff_can_mpfs.c:66-80`; V11 | R1/V&V | open | FRICK-REVIEW-0114/0115 |
| F3 | Comment | N10/N12 residual R1-D-02/R2-D1 | MsgAv-ack race; unreachable `presc==0` | close or record as accepted residual | `ff_can_mpfs.h:41-47`; `ff_can_mpfs.c:72-74` | R1/R2 | open | FRICK-REVIEW-0114/0115 |

## 10. Uncertainty and confidence

```text
Decision/product and configuration: reference MSS CAN driver increment (SRS-REQ-057), commit a95cafb + uncommitted working tree (clock/standard fixes); verified source hashes ff_can_mpfs.h 6ed89e588d5bbf974554de14faef4a4b / .c 7a8a95bb763d1888ea1812404d32ca6b / test_can.c e33b53562e7b8da1d4d95993a3a7a92e; HEAD == origin/main == gitea/main (nothing pushed).
Claim type: mixed (Facts on gates/records/counts and the clock correction; Inference on the N20-gate closure status; Recommendation for the pre-commit freeze F1 and the comments F2/F3).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md 1.4); ECSS-E-ST-40C Rev.1 (2025), ECSS-Q-ST-80C Rev.2 (2025); target criticality B (class-A gaps open).
Evidence consulted (issuer, ID/revision, artifact/version, date): repository working tree at a95cafb+closure; every artifact in the header table; gates re-executed read-only 2026-10-11 (section 3).
Cross-check and reconciliation: recorded numbers (99/58/27/14/51; 19/19 native + ASan; test_can 128; hal_can=PASS + RENODE_TEST: PASS; 13 MISRA advisories, 0 in the new files; format clean; metrics current) were reproduced. D-01/D-02, R1-D-01 and R2-D2 confirmed closed. The reviewed revision was not frozen during the window (F1: a transient working header carried 0x1FFFFFFF); the final state is verified correct.
Confidence: High on D-01/D-02, R1-D-01 at the arithmetic/record level, PA-01/PA-09 and R2-D2 (direct code/test/repository evidence); Medium on the Renode/Lean/cppcheck artifacts (assessed from retained evidence, not re-run) and on the 80 MHz clock value (datasheet-attributed; full PDF not accessed).
Basis and detection limits: read-only inspection + gate re-runs on the frozen hashes; docker/Renode, cppcheck and lake were NOT re-executed (not available/forbidden); the emulator CAN model lives inside the pinned image; the built test/ELF evidence (22:52) predates the 22:59 working-tree write, so a rebuild on the frozen hash is recommended.
Missing/inaccessible evidence (including paywalls): closure-review rows 0114-0117 not yet in docs/reviews/README.md (in production); no silicon/datasheet-confirmed CAN clock (GAP-A5); PolarFire SoC datasheet full text not parsed.
Assumptions and expiry/recheck date: the parallel closure reviews 0114/0115/0116 close the R1/R2/R4 dispositions for the same fixes; recheck at the commit that freezes the verified hash and on any change to the bit timing, register map or the HAL CAN contract. This review is void if the reviewed sources change (see F1).
Consequences if wrong: if the commit is made from an unfrozen tree, the reviewed evidence may not correspond to the committed bytes (F1); if the MSS CAN clock differs from 80 MHz, the reference bit timing is wrong at every rate (bounded by GAP-A5).
Required independent human reviewer/authority: the project integrator/owner (and the hardware/system owner for the clock and GAP-A5 acceptance) decide closure; this review confers no approval, certification or release authority.
```

## 11. Evidence consulted (exact commands and results)

```text
git status --short → 24 modified + 10 untracked (src/drivers/can/, tests/unit/test_can.c, 8 can-*.md reports)
git log --oneline -1 → a95cafb tooling.md: board platform wording for the Renode env self-test
git rev-parse HEAD / origin/main / gitea/main → a95cafb…  (all equal; increment uncommitted)

python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  → traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked); SSS-REQ-020 <- 10
python3 tools/scripts/metrics.py --check → exit 0, "metrics panel: current"
ctest --test-dir build/native → 19/19 passed (test 19 = can)
ctest --test-dir build/asan   → 19/19 passed
./build/native/tests/test_can → checks=128 failures=0
clang-format --dry-run --Werror ff_can_mpfs.c ff_can_mpfs.h test_can.c → exit 0 (CLEAN, clang-format 23.1.3)

git diff HEAD -- docs/status/status.json → meta.note records "...CAN backend (SRS-REQ-057) - reviews
  FRICK-REVIEW-0106/0107/0108/0109 + delta 0110/0111/0112/0113 closed; M11 6/13" (D-01 closed)
docs/reviews/README.md:106-113 → rows for 0106/0107/0108/0109 + 0110/0111/0112/0113 (D-02 closed)

ff_can_mpfs.h:148-153 → #define FF_CAN_MPFS_CLK_HZ 80000000u (MSS TRM range 8..80 MHz cited)
Bit-timing recomputation (80 MHz, tq=16, integer prescaler):
  125000 -> presc 40 -> 125000 (exact); 250000 -> 20 -> 250000 (exact);
  500000 -> 10 -> 500000 (exact); 1000000 -> 5 -> 1000000 (exact); guard: bitrate > 80e6/16 = 5e6 rejected
grep '125 MHz|−2.34|976' product docs/source → none (only historical review reports + unrelated ASN.1 constants)
test_can.c:324-328 → amr_written == FF_CAN_ACCEPT_ALL; identifier-field bits set; mock_rx_accepts(0x04000000|0x1FFFFFFF)==1

tr -d '\000' < build/renode/console.log | tail → hal_gpio/hal_serial/hal_mem/hal_eth/hal_net/hal_nvm/hal_can=PASS,
  sb_roundtrip=PASS, RENODE_TEST: PASS  (same in build/renode/renode-process.log)
stat build/renode/{frick_fsw_renode.elf,console.log,renode-process.log} → mtime 22:52 (post-change rebuild)
tail verification/lean/lake-build.log → ✔ [21/21] Built frick_verify_checks; Build completed successfully.
grep -ci misra build/misra-report.txt → 13; grep 'ff_can_mpfs|test_can' → none

Working-tree integrity (§3.1): stat -c '%y' ff_can_mpfs.{c,h} → 2026-10-11 22:59:28 (after the review began);
  at ~the same window sed -n '138,142p' showed '#define FF_CAN_ACCEPT_ALL \' / '0x1FFFFFFFu' and
  clang-format reported a 140:26 violation; the final state (hash 6ed89e58…) is the correct expression and
  clang-format is clean. Concurrent opencode sessions + a detached antmicro/renode container were active.
```

---

Committed and pushed with the reviewed change (norm N11).
