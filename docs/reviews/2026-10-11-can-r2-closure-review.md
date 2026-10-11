# Independent Closure Review — Reference MSS CAN driver (SRS-REQ-057), profile R2 (SW V&V)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0115                      |
| Reviewer session                  | Independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (Senior SW V&V engineer)            |
| Reference revision                | `a95cafb` + CAN increment (clock/std fixes) (working tree) |
| Items under review                | Closure of the R2 residuals **R2-D1** and **R2-D2** of FRICK-REVIEW-0111, and disposition of R2-D3/R2-D4; artefacts `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`, `tests/unit/test_can.c` (128 checks), `config/platform/platform_mpfs.h`, the changed V&V/records docs (`docs/vv/verification-results.md` 0.30, `docs/vv/tool-qualification.md` 0.3.14, `docs/vv/renode-environment.md` 0.12), `CHANGELOG.md`, `docs/requirements/traceability.{yaml,md}`, SRS-REQ-057; prior reports FRICK-REVIEW-0107 / 0111 |
| Date                              | 2026-10-11                             |
| Scope                             | Delta closure of the R2 findings of FRICK-REVIEW-0111 against ECSS-E-ST-40C (verification), ECSS-Q-ST-80C (software PA), ECSS-E-ST-10-02C (product verification), IEEE 1012 (V&V process framing) |
| Independence declaration          | I did not author, design, execute or disposition this change and had no access to the author's conversation, reasoning or intent beyond the records in the repository and the prior reviews. Every conclusion below is grounded in a command I re-ran, a controlled repository artefact, or a primary source already established in FRICK-REVIEW-0107. |
| Norms applied                     | N1–N20 (`docs/rules.md`); focus N4 (traceability), N8 (docs per change), N10/T3+T17 (tool qualification), N11 (evidence), N12 (NC closure), N15 (full cycle), N18 (Renode functional validation), N19 (loop audit) |

## 1. Scope

Re-verification of the author's dispositions for the two R2 residuals of
FRICK-REVIEW-0111 — **R2-D1** (`presc == 0` unreachable branch undispositioned)
and **R2-D2** (RTR/IDE mask bits of the accept-all AMR unprotected by the
regression) — and confirmation that **R2-D3/R2-D4** (Comments, optional) are
appropriately documented or accepted. Included: independent re-execution of the
named gates, an independent discrimination experiment on the R2-D2 regression in
a private build directory, and a consistency read of the evidence documents.

Excluded: the R1 code-quality remit (including the R1-D-01 clock correction,
which is R1 scope), the R3 systems and R4 documentation remits, the RTEMS/Lean
build chain, and any on-silicon claim (governed by GAP-A5). Per the task
constraints I did **not** run Docker or `make renode`, and did not modify any
tracked file other than this report. I used a **private** build directory
(`build/rev-r2-closure`, removed after use) and temporarily patched the driver
header/`.c` under verified backups, restoring both to their original SHA-256.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); `docs/engineering/independent-review.md` (profiles, independence).
- Prior reviews: `docs/reviews/2026-10-11-can-r2-review.md` (FRICK-REVIEW-0107); `docs/reviews/2026-10-11-can-r2-delta-review.md` (FRICK-REVIEW-0111).
- Requirement: SRS-REQ-057 (SRS rev 1.16), parent SSS-REQ-020.
- Primary technical references (from FRICK-REVIEW-0107, re-applied): Renode `MPFS_CAN.cs` filter semantics `(~AcceptanceMask & (Id ^ AcceptanceCode)) == 0`, AMR `{N_A:1, RTR:1, IDE:1, ID:29}`; Microchip `mss_can.h`/`mss_can.c` (`MSS_CAN_get_msg_filter_mask()` left-aligns the ID and sets the RTR/IDE mask bits).
- Gap: `docs/quality/class-a-gaps.md` GAP-A5 (on-target qualification).

## 3. Closure of FRICK-REVIEW-0111 residuals

| NC | Severity | Disposition claimed | Independent re-verification | Status |
| -- | -------- | ------------------- | --------------------------- | ------ |
| R2-D1 | Minor | `ff_can_mpfs.c:72-73` now carries `/* defensive: unreachable, as presc >= 1 whenever bitrate <= clk/tq */`, recording the branch as unreachable defensive code | Verified `ff_can_mpfs.c:70-74`. Reachability argument independently re-checked: the guard at `:67` (`bitrate <= FF_CAN_MPFS_CLK_HZ / tq`) with `divisor = bitrate * tq` (`:70`) implies `divisor <= CLK`, hence `presc = (CLK + divisor/2) / divisor >= (divisor + divisor/2) / divisor = 1.5` → integer `>= 1`. The `presc == 0` branch is unreachable **and** is now explicitly dispositioned as such (coverage justification), not claimed as a tested boundary. gcov still marks `:73` uncovered, which is now expected and explained. | **CLOSED** |
| R2-D2 | Minor | The mock captures the raw AMR write (`test_can.c:138`) and the suite asserts `g_m.amr_written == (uint32_t)FF_CAN_ACCEPT_ALL` (`:326`), protecting the RTR/IDE mask bits the emulator drops | The assertion is present and the write is genuinely captured. **Discrimination is only partial** — see §5: it fails (128/1) when the driver **write site** drops the bits, but it passes (128/0) when the **macro definition** drops them, because the expected value is derived from the same macro. The specific regression described in R2-D2 ("a future regression that dropped the `FF_CAN_RX_AMR_RTR`/`FF_CAN_RX_AMR_IDE` mask bits") is therefore not fully protected. Recorded as residual **R2-D5**. | **CLOSED (partial) — new residual R2-D5** |
| R2-D3 | Comment | Not addressed (optional) | The byte-order known-answer words remain the driver's own convention, unanchored to a vendor/emulator vector (`test_can.c:261-265,279-284`). No new evidence; documented in FRICK-REVIEW-0111. Accepted as an open optional Comment. | **Accepted (open Comment)** |
| R2-D4 | Comment | Not addressed (optional) | RX callback re-entrancy and multi-buffer dispatch order remain untested; only buffer 0 is exercised (`test_can.c`), and the R2-09 re-read robustness fix (`ff_can_mpfs.c:183-185`) is still not covered by a test. No new evidence; documented in FRICK-REVIEW-0111. Accepted as an open optional Comment. | **Accepted (open Comment)** |

No Major or new code defect was found. R2-D1 is fully closed; R2-D2 is closed as a
finding (the requested assertion was added and does discriminate the write-site
regression) with a bounded hardening residual R2-D5.

## 4. New residual findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R2-D5 | Minor | SRS-REQ-057 / T3 (regression independence) | The new R2-D2 assertion compares the captured raw AMR write to `FF_CAN_ACCEPT_ALL`, the same macro the driver writes at `ff_can_mpfs.c:127`. The RTR/IDE mask bits live only in that macro, so a regression that edits the macro definition (the most direct way to "drop the mask bits") changes both the driver's write and the test's expected value together and is **not** caught. Empirically: macro edited to drop the bits → suite still **128/0**; driver write site edited to drop the bits (macro kept) → suite **128/1** at `:326`. The identifier-domain semantic checks (via `mock_rx_accepts`) still independently guard the `<< FF_CAN_ID_SHIFT` fix (original R2-01 bug → 128/9), but no independent anchor guards the RTR/IDE bits. | `test_can.c:138,326`; `ff_can_mpfs.h:116-117,139-141`; `ff_can_mpfs.c:127`; private-build experiment §5 | Assert the bits against independent literals rather than the shared macro, e.g. `FF_TEST_ASSERT((g_m.amr_written & 0x6u) == 0x6u);` (bits 1=RTR, 2=IDE), or derive the expected mask without `FF_CAN_RX_AMR_RTR/FF_CAN_RX_AMR_IDE`. Optional but cheap. |

## 5. Independent discrimination experiment (private build directory)

Baseline: `cmake -S . -B build/rev-r2-closure -DCMAKE_BUILD_TYPE=Debug` + build
`test_can` → `checks=128 failures=0` (matches the author's claim). Header/`.c`
backed up; every patch was applied to a single site and restored afterwards
(header SHA-256 `2ee9abed…`, `.c` SHA-256 `e3b72ecb…` re-verified identical to
the pre-experiment values).

| Variant | Patch | Result | Interpretation |
| ------- | ----- | ------ | -------------- |
| Baseline | none | **128 / 0** | Suite green as claimed |
| R2-01 reintroduced | `FF_CAN_ACCEPT_ALL` = literal `0x1FFFFFFFu` | **128 / 9** | The original receive defect is still independently caught (identifier-domain semantic checks) |
| R2-D2 write-site drop | driver writes `FF_CAN_ACCEPT_ALL & ~(RTR|IDE)` (macro kept) | **128 / 1** (`:326`) | The new assertion catches a write-path regression → the fix has real value |
| R2-D2 macro drop | `FF_CAN_ACCEPT_ALL` = `(FF_CAN_ID_MASK << FF_CAN_ID_SHIFT)` | **128 / 0** | **Not** caught → residual R2-D5 |

## 6. Evidence-document accuracy (independently checked)

- **`docs/vv/verification-results.md` 0.30** is accurate: the MSS CAN row records
  ctest **19/19**, `test_can` **128 checks**, trace **99/58/27/14/51**, M11
  **6/13**, `hal_can=PASS`, and the "9 failures at the old constant" figure (I
  reproduced exactly 9). The History 0.30 entry (60→128 checks, clock 125→80 MHz)
  matches the code.
- **`docs/vv/tool-qualification.md` 0.3.14** correctly lists `test_can.c` under
  T3 and `hal_can` among the T17 HAL checks; the T17 row records retention of the
  full Renode process log. The file `build/renode/renode-process.log` is present.
- **`CHANGELOG.md`** correctly records the R2-01/PA-09 mask fix and the R1-D-01
  clock correction (125 → 80 MHz), with on-target confirmation left to GAP-A5.
- **Clock standard-rate arithmetic** (R1 scope, sanity-checked for consistency):
  with `FF_CAN_MPFS_CLK_HZ = 80000000` and `tq = 16`, the standard rates give
  prescaler 40/20/10/5 for 125/250/500 kbit/s and 1 Mbit/s, matching the header
  and CHANGELOG text.
- **Renode retained evidence**: `build/renode/console.log` contains
  `[renode-test] hal_can=PASS` and `RENODE_TEST: PASS`; `build/renode/renode-process.log`
  ends with the same marker. (Retained log; I did not re-run `make renode` per
  instruction.)
- **`docs/reviews/README.md`** now lists FRICK-REVIEW-0110–0111 (R1/R2 delta) and
  the R4/PA delta reviews. This closure report (FRICK-REVIEW-0115) is **not yet
  listed** — a follow-up index update is required with the change (norm N8/N11),
  as `docs/reviews/README.md` is a tracked file outside this review's edit scope.

## 7. Verdict

- [ ] APPROVED
- [x] **APPROVED WITH COMMENTS** — R2-D1 fully closed; R2-D2 closed as a finding
  with a bounded hardening residual **R2-D5** (Minor); R2-D3/R2-D4 accepted as
  open optional Comments. No Major.
- [ ] REJECTED

The two R2 residuals that gated FRICK-REVIEW-0111 are dispositioned: the
unreachable `presc == 0` branch is now explicitly recorded as unreachable
defensive code (R2-D1, closed), and the regression suite now captures and asserts
the raw AMR write, which I verified discriminates a write-site regression
(R2-D2, closed). One bounded gap remains: because the expected value is derived
from the same macro that defines the RTR/IDE mask bits, a macro-level edit that
drops those bits still passes the 128-check suite (R2-D5, Minor). This does not
defeat SRS-REQ-057 — the shipped macro is correct and the identifier-domain
semantic checks still catch the original R2-01 defect — but it leaves the exact
regression class named in R2-D2 only partially protected. The fix is a one-line
literal-bit assertion; it should be applied before commit or tracked to closure
(norm N12).

## 8. Sign-off

| Role            | Agent / Reviewer | Date       | Conclusion |
| --------------- | ---------------- | ---------- | ---------- |
| R2 (SW V&V, N9) | Independent agent (fresh context) | 2026-10-11 | APPROVED WITH COMMENTS (R2-D1 closed; R2-D2 closed with residual R2-D5 Minor; R2-D3/R2-D4 accepted) |

## 9. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R2-D1 | Minor | SRS-REQ-057 / T3 | unreachable `presc==0` branch undispositioned | documented as unreachable defensive code (`:73`) | `ff_can_mpfs.c:70-74`; reachability re-derived | frick-fsw | **closed** | this report |
| R2-D2 | Minor | SRS-REQ-057 / T3 | RTR/IDE mask bits unprotected by the regression | fixed (raw-write capture + assertion); residual R2-D5 | `test_can.c:138,326`; discrimination matrix §5 | frick-fsw | **closed (partial)** | this report |
| R2-D5 | Minor | SRS-REQ-057 / T3 | R2-D2 assertion is not independent of the macro definition: a macro-level drop of the RTR/IDE bits still passes (128/0) | open — add a literal-bit assertion (e.g. `(amr_written & 0x6u) == 0x6u`) | private-build experiment §5 | frick-fsw | **open** | — |
| R2-D3 | Comment | T3 (oracle) | KAT convention not vendor-anchored | accepted (optional) | `test_can.c:261-265,279-284` | frick-fsw | **accepted (open)** | — |
| R2-D4 | Comment | SRS-REQ-057 (dispatch) | callback re-entrancy / multi-buffer dispatch order untested | accepted (optional) | `test_can.c` (buffer 0 only); `ff_can_mpfs.c:146-188` | frick-fsw | **accepted (open)** | — |

## 10. Evidence consulted (exact commands and results)

Reference revision: `git status` / `git diff --stat` — branch `main` at `a95cafb`,
23 modified tracked files + untracked `src/drivers/can/`, `tests/unit/test_can.c`,
`docs/reviews/2026-10-11-can-*.md`.

Verification gates (independently re-run on the working tree):

- `ctest --test-dir build/native` → **100% tests passed, 0 failed out of 19**.
- `./build/native/tests/test_can` → `checks=128 failures=0`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  `traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked)`.
- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`.
- `grep -a -nE "hal_can|RENODE_TEST" build/renode/console.log` →
  `[renode-test] hal_can=PASS` (line 34), `RENODE_TEST: PASS` (line 36);
  `tail build/renode/console.log` → the HAL check list ending in
  `[renode-test] hal_can=PASS` + `RENODE_TEST: PASS`.

R2-D2 discrimination experiment (private build dir `build/rev-r2-closure`, removed
after use; header/`.c` restored and SHA-256 re-verified):

- `cmake -S . -B build/rev-r2-closure -DCMAKE_BUILD_TYPE=Debug` + build `test_can`
  → baseline `checks=128 failures=0`.
- Macro patched to `(FF_CAN_ID_MASK << FF_CAN_ID_SHIFT)` (drop RTR/IDE) → `checks=128 failures=0`.
- Write site patched to `FF_CAN_ACCEPT_ALL & ~(FF_CAN_RX_AMR_RTR | FF_CAN_RX_AMR_IDE)` →
  `checks=128 failures=1` (`test_can.c:326`).
- Macro patched to `0x1FFFFFFFu` (original R2-01 bug) → `checks=128 failures=9`.

Files read: `src/drivers/can/mpfs/ff_can_mpfs.h` (full), `.../ff_can_mpfs.c`
(`:1-194`), `tests/unit/test_can.c` (full), `config/platform/platform_mpfs.h`,
`docs/reviews/review-template.md`, `docs/reviews/2026-10-11-can-r2-review.md`,
`docs/reviews/2026-10-11-can-r2-delta-review.md`, `docs/reviews/2026-10-11-can-r4-delta-review.md`,
`docs/vv/verification-results.md` (diff/0.30), `docs/vv/tool-qualification.md`
(diff/0.3.14), `CHANGELOG.md` (diff), `docs/reviews/README.md` (diff),
`docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/rules.md`.

Not executed / not re-run: Docker, `make renode`, `make renode-env`, ASan/UBSan,
Lean, `clang-format`, `misra.sh` (author evidence accepted; not required by the
closure scope). No tracked file was modified other than this report; no commit or
push performed.

---

Uncertainty statement (R2): facts above are traceable to the commands and
artefacts listed. The R2-D1 closure rests on the reachability argument at
`ff_can_mpfs.c:67-74`, re-derived independently. The R2-D2/R2-D5 conclusions rest
on the reproduced 128/0 → 128/1 → 128/9 discrimination matrix, which is
deterministic on the host mock. The Renode evidence is a **retained log** — I did
not re-execute `make renode` (Docker avoided per instruction) — so its
authenticity rests on the retained files and the recorded run. Absolute
register-level confirmation on silicon, and the corrected 80 MHz reference clock,
remain governed by GAP-A5 and require a qualified human before flight. Confidence:
**High** for the gate re-runs, the R2-D1 closure and the R2-D2 discrimination
result; **High** that R2-D5 is a real, bounded residual. This report is an AI
analysis and carries no approval, qualification or certification authority.
