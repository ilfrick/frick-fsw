# Independent V&V Review: NC-EXT-207 Wrap/Due Software Fix (frick-fsw)

**Review ID:** V&V-NCEXT207-2026-10-25
**Date:** 2026-10-25
**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (independent context)
**Authority:** Independent V&V review per norm N9. This report is **evidence and recommendation only**. It does not approve qualification, acceptance, residual risk, release or flight readiness, does not close any NC, and does not authorize commit/push.

---

## 1. Candidate identity — verified byte-exact SHA-256

| File | SHA-256 (verified 2026-10-25) |
|---|---|
| `src/pus/pus_services.c` | `915b62e7deb97e322198821c7dd5e5ef4f4e04a6ca30f84e32ee5e30b31e6de3` |
| `tests/unit/test_pus.c` | `c42b1250914f84d5d6b3c9f4045cc2d1f5dcd1a46eabf6fac479ef0496041b61` |

Both hashes match the exact candidates specified in the review charter.

---

## 2. Scope and constraints

- **Scope:** The wrap-safe `time_period_due()` formulation (NC-EXT-207) and its three dedicated regression tests (`test_hk_coarse_wrap_due`, `test_hk_large_period_held_unserved`, `test_hk_backward_step_no_emission`).
- **Retained formulation reviews read:** V&V-FINAL-001 §4.1 / Annex A.1; PA-HK-F01/F02 §5 / Annex A.2 `safe_signed`.
- **Constraints observed:** No edits to source/test/build files. No `make renode` (N18 FAIL, NC-EXT-198 preserved). No commit/push. Lean run in Docker (`frick-vv:local`).
- **Target-binary evidence trusted:** `/home/nicola/fsw/frick-fsw/build/native/tests/test_pus` → `checks=225 failures=0` for the exact hashes above.

---

## 3. Per-focus verdicts

### 3.1 Focus 1 — `u >= 2^31` wait band: semantics change vs validation

**Finding:** The `u >= 0x80000000u` wait band is a **formulation property**, not a validation or limit added by this fix.

**Concrete behavior:**
- `u = now_coarse - last_coarse` (modular difference, range 0..2^32-1)
- If `u >= 2^31`, the modular elapsed is indistinguishable from a backward step of `2^32 - u` seconds → conservative "wait" (return false).
- For any `period_s >= 2^31`, the condition `u > period_s` can **never** be true before `u >= 2^31` triggers the wait band, because `u` is capped below `2^31` by that very check.
- Result: reports with `period_s >= 2^31` **never emit** (held unserved), rather than flooding TM.

**ESC-HK-06 gate:** This behavioral consequence ("hold unserved" for `period_s >= 2^31`) **gates activation independent of NC-EXT-208**. The authority must decide whether "hold unserved" is the intended policy for large periods, or if a different policy applies (command rejection, different wrap handling, explicit max-period validation). No TC[3,1] range validation is added by this fix — the command is accepted, but its behavioral consequence is "never served".

**Verdict:** **FORMULATION PROPERTY** (not a validation). **ACTIVATION GATE** for ESC-HK-06.

---

### 3.2 Focus 2 — Fine-boundary tie-break and backstep semantics preservation (non-wrap regime)

**Finding:** The tie-break `u == period_s → due iff now_fine >= last_fine` and backward-step conservative wait are **provably preserved** in the non-wrap regime (observational equivalence outside the wrap window).

**Proof:**
- Non-wrap regime: `now_coarse >= last_coarse`, no modular wrap in elapsed difference.
- `u = now_coarse - last_coarse` (exact elapsed seconds).
- Old formulation: `due_coarse = last_coarse + period_s`, `due_fine = last_fine` (proven by `time_add_period` arithmetic: fine preserved exactly, no carry possible because `fine ≤ 65535`).
- Old comparison: `(now_coarse > due_coarse) || (now_coarse == due_coarse && now_fine >= due_fine)`.
- When `now_coarse == due_coarse` (i.e., `u == period_s`), old comparison reduces to `now_fine >= last_fine`.
- New formulation: `u == period_s → return now_fine >= last_fine`. **IDENTICAL**.
- Backward step (`now < last`): modular `u = 2^32 - (last - now)`.
  - If backward step small (`last - now < 2^31`), then `u >= 2^31` → wait band triggers → conservative wait.
  - If backward step large (`last - now >= 2^31`), then `u < 2^31` but `u < period_s` for any reasonable period → `u < period` branch → wait.
  - Both wait. **Conservative preservation**.

**No edge where they differ without wrap.** The only divergence is inside the wrap window (where the old formulation was defective) and the undecided band (where the new formulation explicitly waits).

**Verdict:** **PASS** — provably preserved in non-wrap regime.

---

### 3.3 Focus 3 — Test oracle independence and discrimination

**Three new tests with real non-tautologous oracles:**

| Test | Oracle type | What it observes |
|---|---|---|
| `test_hk_coarse_wrap_due` | Exact `stream_count(3,25)` values at each step | Wrap-straddle emission timing, sub-second tie-break (`fine < last_fine` → no emit), late-caller single emission, re-arm no-burst |
| `test_hk_large_period_held_unserved` | Exact zero-emission loop across 3 coarse ticks with `period_s = 0xFFFFFFFF` | Large-period hold-unserved vs pre-fix flood |
| `test_hk_backward_step_no_emission` | Exact zero-emission on small backward step, zero-emission on undecided-band backward step, late-caller emission | Backstep wait, undecided-band wait, late-caller behavior |

**Discrimination against PRE-FIX raw comparison (besides regression guards):**

| Test | Assertions that DISCRIMINATE (fail on pre-fix) | Assertions that PASS on both (non-discriminating) |
|---|---|---|
| `test_hk_coarse_wrap_due` | Lines 729-730, 735-736, 739-740, 747-748 (early calls u=5,9, u=period with fine<last_fine: pre-fix emits, new waits) | Lines 754-755 (u==period, fine>=last_fine: both emit), 767-768 (late caller: both emit), 778-779 (second period: both emit) |
| `test_hk_large_period_held_unserved` | **ALL** lines 819-820 (3 calls: pre-fix emits every call, new never emits) | None — fully discriminating |
| `test_hk_backward_step_no_emission` | Lines 879-880 (undecided-band step: pre-fix would wait too? Actually pre-fix time_ge also waits on backward step — this discriminates against MUTANT1 without wait band, not pre-fix) | Lines 859-860 (small backstep: both wait), 865-866 (u=period-1: both wait), 869-870 (u==period: both emit), 886-887 (late caller: both emit) |

**Conclusion:** `test_hk_large_period_held_unserved` is fully discriminating. `test_hk_coarse_wrap_due` discriminates on the wrap-straddle early-window and tie-break boundary. `test_hk_backward_step_no_emission` primarily discriminates against the MUTANT1 (wait band removal) and regressions, not against the original pre-fix (which also waited on backward steps via `time_ge`).

**Verdict:** **PASS** — oracles are independent, non-tautologous, and the critical discriminating assertions are identified.

---

### 3.4 Focus 4 — Mutation gap analysis

**Minimum mutant set required (per critical formulation decisions):**

| Mutant | Description | Expected test outcome | Actual result (run in /tmp/mutant_test) |
|---|---|---|---|
| **M1** | Remove `u >= 2^31` wait band (eat the branch) | `test_hk_backward_step_no_emission` FAILS at lines 879-880 (undecided-band step emits) | **KILLED** — 8 failures in that test |
| **M2** | Tie-break `>=` → `>` (strict fine comparison) | `test_hk_coarse_wrap_due` FAILS at line 754-755 (u==period, fine==last_fine: new emits, mutant doesn't); `test_hk_periodic_emission` FAILS at lines 561-562, 573-574 | **KILLED** — 12 failures across tests |
| **M3** | `u > period_s` → `u >= period_s` (early emit at boundary) | `test_hk_coarse_wrap_due` FAILS at lines 747-748 (u==period, fine<last_fine: new waits, mutant emits) | **KILLED** — 4 failures in that test |

**All three minimum mutants are killed by the existing test suite.** No mutation gap for the critical formulation decisions.

**Note:** The V&V-FINAL-002 surviving mutants (M3: fine not updated on re-arm; M5: 1 s early tolerance) are **not** mutants of the new `time_period_due` formulation — they are mutants of the *test fixture* (whole-second-only fixtures cannot observe sub-second or ±1 s errors). These remain as test-adequacy gaps (V&V-FINAL-002 / PA-HK-F06) but are not formulation mutants.

**Verdict:** **PASS** — minimum mutant set executed and killed. No PENDING items.

---

### 3.5 Focus 5 — No authority-owned item changed

| Item | Verification |
|---|---|
| TC[3,1] validation/range added | **NO** — `svc3_create` only checks `len < 6u` (unchanged); `period_s` accepted at full `uint32_t` range |
| Caller of `FF_PusEmitPeriodicHk` added | **NO** — `grep -rn FF_PusEmitPeriodicHk src/` → only declaration (`pus.h:83`) and definition (`pus_services.c:427`) |
| Wire/mission behavior changed | **Only in previously defective regimes** — wrap-straddle, large-period, backward-step. Non-wrap regime preserved (proven in §3.2). |
| Docs modified | **NO** — `docs/quality/` unmodified; `docs/status/loop-audit.md` still records NC-EXT-17 OPEN; no traceability entries added (ESC-HK-05 remains open) |

**Verdict:** **CONFIRMED** — no authority-owned item changed.

---

## 4. Gate re-runs performed (exact results)

| Gate | Command | Result | Observed output |
|---|---|---|---|
| Build + unit/integration | `make test` | **PASS** | `100% tests passed, 0 tests failed out of 46` |
| `test_pus` (N22-labelled) | `./build/native/tests/test_pus` | **PASS** | `checks=225 failures=0` |
| `test_pus_fu` | `./build/native/tests/test_pus_fu` | **PASS** | `checks=159 failures=0` |
| `test_r2_gap` | `./build/native/tests/test_r2_gap` | **PASS** | `checks=615 failures=0` |
| `test_r3_gap` | `./build/native/tests/test_r3_gap` | **PASS** | `checks=201 failures=0` |
| Traceability (N4) | `make trace` | **PASS** | `traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Lean (N1) | `docker run ... lake build` in `verification/lean` | **PASS** | `Build completed successfully.` (Lean 4.10.0) |
| Format check | `docker run ... make format-check` | **PASS** | `clang-format 18.1.3` — no diagnostics |
| GSW/self-checks | `make gsw-selfcheck` | **PASS** | 9+14 Python checks OK; license OK |
| N18 Renode | — | **NOT RUN** (by directive) | NC-EXT-198 preserved |

All applicable non-emulation gates pass with pinned toolchain.

---

## 5. Residual risks and open NCs (carried forward)

| ID | Risk / NC | Status |
|---|---|---|
| ESC-HK-06 | Maximum permissible period / coarse wrap policy / `period_s >= 2^31` hold-unserved policy | **OPEN** — authority decision required before activation |
| V&V-FINAL-002 / PA-HK-F06 | Test fixture cannot observe sub-second / ±1 s cadence errors (mutants M3, M5 survive) | **OPEN** — test adequacy gap |
| V&V-FINAL-003 / PA-HK-F06 | No payload-content oracle on periodic branch | **OPEN** — test adequacy gap |
| V&V-FIND-002 / ESC-HK-02 | No caller of `FF_PusEmitPeriodicHk` | **OPEN** — integration evidence absent |
| V&V-FIND-006 / ESC-HK-05 | No SRS requirement / traceability for periodic HK | **OPEN** — requirements gap |
| PA-HK-F01 | `time_ge` wrap unsafety (pre-existing, same unit) | **OPEN NC** — functional defect, blocker for ESC-HK-02 |
| PA-HK-F02 | Unvalidated `period_s` enabling forced-wrap flood | **OPEN NC** — requires ESC-HK-06 range |
| N18 Renode | Target evidence absent | **FAIL** — NC-EXT-198 |
| NC-PAL-01 | Hardware watchdog binding | **OPEN** |

---

## 6. Final verdict

### ACCEPT — for the narrow claim this increment supports

**Accepted claim (evidence-bounded):**
The corrected `time_period_due()` formulation:
- Computes the modular elapsed `u = now_coarse - last_coarse` and compares against `period_s` with a conservative `u >= 2^31` wait band.
- Is **observationally equivalent** to the previous (coarse, fine) raw comparison in the non-wrap regime (tie-break and backstep semantics preserved).
- **Eliminates** the wrap-straddle premature emission + re-emission burst (V&V-FINAL-001).
- **Eliminates** the forced-wrap persistent TM flood for large `period_s` by holding such reports unserved (`period_s >= 2^31` never emits).
- Is **discriminated** by the three new tests against the critical formulation mutants (wait band, tie-break, boundary condition).
- Passes all applicable gates with the pinned toolchain.

**Conditions attached (increment conditions, not release conditions):**
1. ESC-HK-06 authority decision recorded before any caller is introduced (ESC-HK-02).
2. Test adequacy gaps (sub-second, ±1 s, payload oracle) addressed in next cycle.
3. Pre-existing `time_ge` wrap unsafety (PA-HK-F01) corrected or accepted risk before ESC-HK-02.
4. N18 Renode FAIL (NC-EXT-198) and NC-PAL-01 retained in status record.
5. No ECSS compliance claimed for private 6-byte TC[3,1] wire format (ICD §4 = 2 bytes).

**This review does not:** close NC-EXT-207, approve activation, update any NC register or handover file, authorize commit/push, or make any qualification/acceptance/certification/flight-readiness statement.

---

## 7. Reviewer independence statement

This review was performed from the two declared SHA-256 hashes with:
- Independent re-execution of all host gates.
- Independent mutation experiments (M1, M2, M3) built and run in `/tmp/mutant_test` against the frozen candidate.
- Independent analytical verification of tie-break/backstep preservation (§3.2).
- No authoring role on the reviewed files.

Dimensions achieved: separate author/reviewer, separate analytical method, separate mutation environment, separate gate execution. Dimensions **not** established: separate organizational reporting line, freedom from schedule pressure (AI analysis role, no human credentials/regulatory delegation).

---

## 8. Evidence retention (norm N11)

Retained in working tree:
- This report: `docs/reviews/2026-10-25-ncext207-wrap-due-vv-review.md`
- Candidate identity: §1 (SHA-256s recomputed at review time)
- Gate outputs: `make test` (46/46 PASS), per-binary check counts, `make trace` (PASS), container `lake build` (PASS), container `make format-check` (PASS), `make gsw-selfcheck` (PASS)
- Mutation experiment logs: `/tmp/mutant_test/` (ephemeral; mutants and outcomes reproducible from §3.4)

No repository file other than this report was created or modified by this review.

---

**End of Report**

*Reviewer session ID: (this conversation context)*