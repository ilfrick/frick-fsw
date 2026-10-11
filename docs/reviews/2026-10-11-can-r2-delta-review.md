# Independent Delta Re-Review — Reference MSS CAN driver (SRS-REQ-057), profile R2 (SW V&V)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0111                      |
| Reviewer session                  | Independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (Senior SW V&V engineer)            |
| Reference revision                | `a95cafb` + CAN increment (post-NC fixes, working tree) |
| Items under review                | `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`, `tests/unit/test_can.c` (127 checks), the changed V&V/evidence docs (`docs/vv/verification-results.md` 0.30, `docs/vv/renode-environment.md` 0.12, `docs/vv/tool-qualification.md` 0.3.14/T17), `docs/requirements/traceability.{yaml,md}`, SRS-REQ-057, and the status/plan doc refresh (`development-plan.md`, `status/handover.md`, `status/loop-audit.md`); prior report FRICK-REVIEW-0107 |
| Date                              | 2026-10-11                             |
| Scope                             | Delta closure of the R2 findings of FRICK-REVIEW-0107 (Major **R2-01** + Minors R2-02..R2-07, Observations R2-08/R2-09). Verification adequacy of the increment against ECSS-E-ST-40C (verification), ECSS-Q-ST-80C (software PA), ECSS-E-ST-10-02C (product verification), IEEE 1012 (V&V process framing) |
| Independence declaration          | I did not author, design, execute or disposition this change and had no access to the author's conversation, reasoning or intent beyond the records in the repository and the prior review. Every conclusion below is grounded in a command I re-ran, a controlled repository artefact, or a primary source already established in FRICK-REVIEW-0107. |
| Norms applied                     | N1–N20 (`docs/rules.md`); focus N4 (traceability), N8 (docs per change), N10/T3+T17 (tool qualification), N11 (evidence), N12 (NC closure by full re-application of the rules), N15 (full cycle), N16 (revision history), N18 (Renode functional validation), N19 (loop audit) |

## 1. Scope

Re-verification of the author's dispositions for the findings of FRICK-REVIEW-0107 (REJECTED on the Major R2-01), plus assessment of whether the new regression test genuinely discriminates the R2-01 fix and whether the mock is now faithful. Excluded: the R1 code-quality remit, the R3 systems and R4 documentation remits (separate reviews), the RTEMS/Lean build chain, and any on-silicon claim (governed by GAP-A5).

Per the task constraints I did **not** run docker or `make renode`; I relied on the retained Renode logs and the author's recorded run. I used a **private** build directory (`build/rev-r2-delta`, removed after use) and temporarily patched one header under a verified backup, restoring it to the original SHA-256. No tracked file was modified other than this report.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); `docs/engineering/independent-review.md` (profiles, independence).
- Prior review: `docs/reviews/2026-10-11-can-r2-review.md` (FRICK-REVIEW-0107).
- Requirement: SRS-REQ-057 (SRS rev 1.16), parent SSS-REQ-020.
- Primary technical references (from FRICK-REVIEW-0107, re-applied): Renode `MPFS_CAN.cs` filter semantics `(~AcceptanceMask & (Id ^ AcceptanceCode)) == 0` with AMR `{N_A:1, RTR:1, IDE:1, ID:29}`; Microchip `mss_can.h`/`mss_can.c` (`MSS_CAN_get_msg_filter_mask()` left-aligns the ID and sets the RTR/IDE mask bits).
- Gap: `docs/quality/class-a-gaps.md` GAP-A5 (on-target qualification).

## 3. Closure of FRICK-REVIEW-0107 findings

| NC | Severity | Disposition claimed | Independent re-verification | Status |
| -- | -------- | ------------------- | --------------------------- | ------ |
| R2-01 | Major | `FF_CAN_ACCEPT_ALL` now `((FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) \| FF_CAN_RX_AMR_RTR \| FF_CAN_RX_AMR_IDE)`; new RTR/IDE mask macros; header comment corrected; regression test added | Verified in `ff_can_mpfs.h:115-140`: identifier-mask field (bits 3–31) is written in the identifier domain (`0x1FFFFFFF << 3`), RTR (`1<<1`) and IDE (`1<<2`) mask bits set, reserved N_A bit clear. **Discrimination proved**: I backed up the header, patched the constant back to `0x1FFFFFFFu`, rebuilt `test_can` in a private build dir → **checks=127 failures=9** (the accept-all identifier-domain assertions and the `0x1ABCDEFF` dispatch fail); restored the header (SHA-256 `013def7f…` re-verified) → **checks=127 failures=0**. Renode check exercised by the fix is not (see R2-08) — the fix is validated by the host test as documented | **CLOSED** |
| R2-02 | Minor | Negative/boundary cases added (bitrate>clock, prescaler overflow, len>8, NULL, bad std/ext IDs, 0-length, DLC>8 clamp, no-callback, init idempotence) | Verified `test_can.c:242-255` (bitrate 0/8 MHz/100/500 k/CAN-FD), `:289-297` (len 9, NULL+len, std>0x7FF, ext>0x1FFFFFFF, zero-length NULL), `:368-380` (no callback), `:383-392` (DLC 15 clamp), `:235-240` (init idempotent). gcov confirms lines 68/76/165/203 are now executed. One listed case (`presc` rounding `:73`) remains uncovered because the branch is unreachable (see **R2-D1**) | **CLOSED** (one residual, R2-D1) |
| R2-03 | Minor | TX round-robin/wraparound assertions added | Verified `test_can.c:299-314`: asserts the buffer base per transmit and the 32-buffer cursor wrapping to buffer 0. Residual "don't overwrite a still-pending buffer" remains an untested, documented limitation (header lines 40-42) | **CLOSED** |
| R2-04 | Minor | Mock hardened (read-only status regs, TX requires loaded data, AMR/AMR_DATA field masking) | Verified `test_can.c:67-70` (RX_STATUS/TX_STATUS writes ignored), `:73-112` (TX requires `tx_valid`, else pending and no TX_MSG), `:136-141` (AMR `& 0xFFFFFFF8`, AMR_DATA `& 0xFFFF`). The mock is no longer more permissive in those respects | **CLOSED** |
| R2-05 | Minor | Byte-order oracle independence | Known-answer literal register words now asserted (`test_can.c:261-264,279-282`). Residual: the reference convention is still the driver's own (no independent vendor vector) — see **R2-D3** | **CLOSED** (residual R2-D3) |
| R2-06 | Minor | Stale status/plan docs refreshed | `development-plan.md:302` = "6/13 peripheral-bound"; `status/handover.md:97` = "6/13", `:190` counts 99/58/27/14; `status/loop-audit.md:34` = "6/13 … 7 classes pending", `:49` = "99 reqs". Remaining "5/13"/"98" occurrences are inside History tables only. Cross-document state now consistent | **CLOSED** |
| R2-07 | Minor | SRS-REQ-057 reworded (driver-specific poll; bounded buffers; bit-timing/GAP-A5) | SRS-REQ-057 (`SRS.md:292-302`) now states the receive path is the driver-specific `FF_CanMpfsPoll`, bounded 32 TX/RX buffers, and bit-timing reference/GAP-A5; `method: Tst \| Dem`. Consistent with `hal_can.h` (no receive entry) and `traceability.yaml:534-541` | **CLOSED** |
| R2-08 | Observation | Renode check scope (no action) | `renode-environment.md:104-107` adds an explicit CAN scope limit: the emulated `MPFS_CAN` models only the AMR identifier field, not the RTR/IDE mask bits; the check uses the fully-modelled registers and the filter is host-verified. Honest and consistent with `FF_CanMpfsSelfTest` (which uses AMR_DATA, not AMR) | **CLOSED** |
| R2-09 | Observation | Poll re-read before MsgAv ack | Verified `ff_can_mpfs.c:179-185`: the control word is re-read after the callback before the MsgAv acknowledge. Residual: callback re-entrancy / multi-buffer dispatch order still untested — see **R2-D4** | **CLOSED** (residual R2-D4) |

## 4. New residual findings (post-fix)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R2-D1 | Minor | SRS-REQ-057 / T3 (coverage) | The `presc == 0` branch (`ff_can_mpfs.c:72-73`) is **unreachable** given the rate guard at `:67-69`: `bitrate ≤ FF_CAN_MPFS_CLK_HZ / tq` implies `divisor = bitrate・tq ≤ CLK`, hence `presc = (CLK + divisor/2)/divisor ≥ 1`. The branch remains uncovered (gcov `:73 #####`) and is not dispositioned as defensive/dead code, so the R2-02 item "presc rounding edge" is not, and cannot be, established by a test. | `ff_can_mpfs.c:66-77`; gcov `ff_can_mpfs.c.gcov:101` (`73: #####`) | Record `:73` as unreachable defensive code (coverage justification) or remove it; do not claim it as a tested boundary. |
| R2-D2 | Minor | SRS-REQ-057 / T3 (regression completeness) | The regression test asserts only the AMR **identifier** field (`test_can.c:322-323`) and the mock discards AMR bits 0–2 (`:137`), so a future regression that dropped the `FF_CAN_RX_AMR_RTR`/`FF_CAN_RX_AMR_IDE` mask bits would still pass the 127-check suite. This matches the emulator (which does not model those bits either), so no false claim is made, but the mask bits set by the R2-01 fix are not themselves protected. | `ff_can_mpfs.h:139-140`; `test_can.c:322-327`; `renode-environment.md:104-107` | Add one assertion that the written AMR has the RTR/IDE mask bits set (host-only, independent of the emulator limitation). |
| R2-D3 | Comment | T3 (oracle independence) | The byte-order known-answer words (`0x44332211`, `0x04030201`, …) are asserted literally, which is stronger than before, but the convention (little-endian data packing, ID `<<3`) is still the driver's own and is not anchored to an independent vendor/emulator vector. | `test_can.c:261-264,279-282` vs `ff_can_mpfs.c:26-36` | Optional: add one vendor/emulator-derived word to break the shared-assumption path. |
| R2-D4 | Comment | SRS-REQ-057 (dispatch) | RX callback re-entrancy and multi-buffer dispatch order (RX_STATUS multi-bit iteration) remain untested; only buffer 0 is exercised. The R2-09 code robustness fix (re-read) is not itself covered by a test. | `test_can.c` (buffer 0 only); `ff_can_mpfs.c:146-188` | Optional: add a two-buffer dispatch-order test and a callback that reconfigures the buffer. |

## 5. Evidence-document accuracy (independently checked)

- **`docs/vv/verification-results.md` 0.30** (row + History) is accurate: `test_can` 127 checks, ctest 19/19, trace 99/58/27/14, `hal_can=PASS`, and the "9 failures at the old constant" figure — I reproduced exactly 9. M11 6/13 consistent.
- **`docs/vv/renode-environment.md` 0.12** scope note (CAN) matches the emulator model and the self-test design; the CAN row in §5 is honest ("MMIO identification/round-trip checks; full driver/data path host-tested").
- **`docs/vv/tool-qualification.md` 0.3.14 / T17** correctly lists `test_can.c` under T3, lists `hal_can` among the T17 HAL checks, and records the retained full Renode process log (`build/renode/renode-process.log`) — the file is present.
- **Traceability**: `trace.py` PASS; SRS-REQ-057 → SSS-REQ-020; tests `test_can.c` + `ci:renode` declared in the manifest and present in both CI workflows.
- **Renode retained evidence**: `build/renode/console.log` contains `[renode-test] hal_can=PASS` and `RENODE_TEST: PASS`; `build/renode/renode-process.log` likewise; log mtimes are later than the rebuilt `frick_fsw_renode.elf` mtime, consistent with a rebuild-then-run order. (I could not rebuild per constraint.)

## 6. Verdict

- [ ] APPROVED
- [x] **APPROVED WITH COMMENTS** — the Major R2-01 is genuinely closed and independently discriminated; the remaining items are Minor/Comment residuals tracked to closure (R2-D1, R2-D2 Minor; R2-D3, R2-D4 Comment). No new Major.
- [ ] REJECTED

The R2-01 rejection barrier is lifted: the accept-all mask is now written in the identifier domain with the RTR/IDE mask bits set, the header comment is corrected, and the new host regression test fails (9 failures) when the defect is reintroduced and passes (127/0) after the fix — I reproduced both states independently. The mock hardening closes the false-confidence path that allowed the defect to pass. The v0.1.0-class residual items (unreachable `presc==0` branch, unasserted RTR/IDE mask bits, non-vendor KAT convention, untested callback re-entrancy) are Minor/Comment and do not defeat SRS-REQ-057; they must be dispositioned (norm N12/N8) before commit. Note: `docs/reviews/README.md` does not yet list this delta report and must be updated with the change.

## 7. Sign-off

| Role            | Agent / Reviewer | Date       | Conclusion |
| --------------- | ---------------- | ---------- | ---------- |
| R2 (SW V&V, N9) | Independent agent (fresh context) | 2026-10-11 | APPROVED WITH COMMENTS (R2-01 closed; R2-D1/R2-D2 Minor; R2-D3/R2-D4 Comment) |

## 8. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R2-01 | Major | SRS-REQ-057 / `ff_can_mpfs.h` | accept-all AMR mis-encoded | fixed: mask in identifier domain + RTR/IDE bits | patch-back experiment: 127/9; fixed: 127/0; mask read at `:115-140` | frick-fsw | **closed** | this report |
| R2-02 | Minor | SRS-REQ-057 / T3 | missing negative/boundary tests | fixed (cases added); residual R2-D1 | `test_can.c:242-297,368-392`; gcov 68/76/165/203 now covered | frick-fsw | **closed** | this report |
| R2-03 | Minor | SRS-REQ-057 | TX round-robin/wraparound unverified | fixed | `test_can.c:299-314` | frick-fsw | **closed** | this report |
| R2-04 | Minor | T3 (mock fidelity) | mock more permissive than emulator | fixed | `test_can.c:67-141` | frick-fsw | **closed** | this report |
| R2-05 | Minor | T3 (oracle) | byte-order oracle non-independence | fixed (literal KAT words); residual R2-D3 | `test_can.c:261-264,279-282` | frick-fsw | **closed** | this report |
| R2-06 | Minor | N8/N16/N19 | stale status/plan docs | fixed | `development-plan.md:302`; `handover.md:97,190`; `loop-audit.md:34,49` | frick-fsw | **closed** | this report |
| R2-07 | Minor | SRS-REQ-057 wording | requirement text vs HAL contract | fixed | `SRS.md:292-302` | frick-fsw | **closed** | this report |
| R2-08 | Observation | N18 | Renode check scope | documented | `renode-environment.md:104-107` | frick-fsw | **closed** | — |
| R2-09 | Observation | `FF_CanMpfsPoll` | poll ack clobber | fixed (re-read); residual R2-D4 | `ff_can_mpfs.c:179-185` | frick-fsw | **closed** | this report |
| R2-D1 | Minor | SRS-REQ-057 / T3 | unreachable `presc==0` branch (`:73`) uncovered/undispositioned | open — document as defensive/dead code or remove | gcov `:73 #####`; reachability argument `:67-77` | frick-fsw | open | — |
| R2-D2 | Minor | SRS-REQ-057 / T3 | RTR/IDE mask bits not asserted by the regression test | open — add a mask-bits assertion | `test_can.c:322-327`; `:137` | frick-fsw | open | — |
| R2-D3 | Comment | T3 (oracle) | KAT convention not vendor-anchored | open — optional | `test_can.c:261-264` | frick-fsw | open | — |
| R2-D4 | Comment | SRS-REQ-057 | callback re-entrancy/dispatch order untested | open — optional | `test_can.c` (buffer 0 only) | frick-fsw | open | — |

## 9. Evidence consulted (exact commands and results)

Reference revision:
- `git -C /home/nicola/fsw/frick-fsw status` / `git diff --stat` — branch `main` at `a95cafb`; 23 modified files + untracked `src/drivers/can/`, `tests/unit/test_can.c`, `docs/reviews/2026-10-11-can-*.md`.

Verification gates (independently re-run on the working tree):
- `ctest --test-dir build/native` → **100% tests passed, 0 failed out of 19**.
- `./build/native/tests/test_can` → `checks=127 failures=0`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → `traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked)`; SRS-REQ-057 → SSS-REQ-020.
- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`.
- `grep -a -nE "hal_can|RENODE_TEST" build/renode/console.log` → `[renode-test] hal_can=PASS`, `RENODE_TEST: PASS`; `tail build/renode/renode-process.log` → same markers.
- `ls -la --time-style=full-iso build/renode/{console.log,frick_fsw_renode.elf,renode-process.log}` → ELF 22:34:26, console 22:34:46, process log 22:34:47 (rebuild-then-run order).

R2-01 discrimination experiment (private build dir, header restored):
- `cmake -S . -B build/rev-r2-delta -DCMAKE_BUILD_TYPE=Debug` + `--build … --target test_can` → OK.
- Patched `FF_CAN_ACCEPT_ALL` back to `0x1FFFFFFFu` → `./build/rev-r2-delta/tests/test_can` → `checks=127 failures=9`.
- Restored header from `/tmp/opencode/ff_can_mpfs.h.bak` → SHA-256 `013def7f…` identical to original; rebuild → `checks=127 failures=0`. Private build dir removed.

Coverage (gcov artefact):
- `build/coverage/gcov/…ff_can_mpfs.c.gcov` → uncovered lines 41, 73, 91, 120, 206, 210, 217, 225, 234 (all defensive/unreachable returns; `:73` analysed in R2-D1). Previously-uncovered 68/76/165/203 are now executed.

Documents read: `docs/reviews/2026-10-11-can-r2-review.md` (FRICK-REVIEW-0107), `docs/reviews/review-template.md`, `docs/vv/verification-results.md` (0.30), `docs/vv/renode-environment.md` (0.12), `docs/vv/tool-qualification.md` (0.3.14), `docs/requirements/SRS.md` (1.16), `docs/requirements/traceability.{yaml,md}`, `docs/development-plan.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/reviews/README.md`, `docs/rules.md`, and the diffs of `CMakeLists.txt`, `tests/CMakeLists.txt`, `src/apps/app_renode_test.c`, `tools/scripts/renode_vv.sh`, `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `config/platform/platform_mpfs.h`.

---

Uncertainty statement (R2): facts above are traceable to the commands/artefacts listed. The R2-01 closure is grounded in the filter semantics shared by the Renode model and the Microchip MSS CAN API (established in FRICK-REVIEW-0107) and in the reproduced 127/9 → 127/0 discrimination; absolute register-level confirmation on silicon remains governed by GAP-A5 and requires a qualified human before flight. The Renode evidence is a **retained log** — I did not re-execute `make renode` (docker avoided per instruction) — so its authenticity rests on the retained files and recorded run, not on an independent re-run. Confidence: **High** for the R2-01 closure, the gate re-runs and the document accuracy; **Medium** for the residual test-completeness items (source/reading-grounded). This report is an AI analysis and carries no approval, qualification or certification authority.
