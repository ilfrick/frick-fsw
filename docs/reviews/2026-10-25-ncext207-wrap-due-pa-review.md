# PA Review — NC-EXT-207 wrap/due software fix (independent fresh context)

**Review ID:** PA-DELTA-2026-10-25-NC-EXT-207-WRAP-DUE
**Date:** 2026-10-25
**Reviewer:** Senior Aeronautics and Space SW Product Assurance engineer (fresh-context, independent; no authorship role on any reviewed file)
**Review role:** independent PA leg (N20) of the NC-EXT-207 "software-separable half" fix; N9/N17 companion legs (V&V exact-candidate review, documentation review) are **not** closed by this report.
**Authority statement:** this report is evidence and a recommendation only. It does **not** close NC-EXT-207 or any NC, does **not** approve activation of periodic HK (ESC-HK-02 remains gated by ESC-HK-03/05/06 and NC-EXT-208), does not accept residual risk, approve a waiver, authorize commit/push, or declare release/flight readiness. Those decisions remain with the authorized human authority.

---

## 1. Candidate identity — verified byte-exact

Both declared SHA-256 digests recomputed on the working tree at review time and **match** (Fact):

```
915b62e7deb97e322198821c7dd5e5ef4f4e04a6ca30f84e32ee5e30b31e6de3  src/pus/pus_services.c
c42b1250914f84d5d6b3c9f4045cc2d1f5dcd1a46eabf6fac479ef0496041b61  tests/unit/test_pus.c
```

| Item | State |
|---|---|
| Delta confined to `pus_services.c` + `test_pus.c` (per handover; larger dirty-tree batch preserved per directive) | Confirmed; no `config/`, `hal/`, `drivers/`, `bsp/`, `board/`, emulator, or mission-table change inspected in this delta |
| Committed/pushed | **No** — working tree, no commit/push performed or authorized by this review |
| Renode (N18) | **NOT RUN per directive** — NC-EXT-198 FAIL state preserved |

---

## 2. The fix as written (what I reviewed in the source)

`time_ge()`/`time_add_period()` are removed and superseded by `time_period_due()` (`pus_services.c:406-420`):

- `u = now_coarse − last_coarse` (defined unsigned modular difference);
- `u ≥ 0x80000000` → **wait** (undecided band: backward step or ≥ 2^31 s — signed arithmetic cannot decide);
- `u > period_s` → due (late-caller single emission); `u < period_s` → not due;
- `u == period_s` → due iff `now_fine ≥ last_fine` (sub-second tie-break preserved).

My independent analysis (Formal, high confidence — same closed-form reasoning class as the two 2026-10-24 reviews, re-derived here):

1. For all `u < 2^31` the formulation is the exact signed modular difference `d = (int32_t)(now−last)`, i.e. precisely the decision-free wrap-safe comparison recommended by **both** retained reviewers: PA-HK-F01 §5.4 and its Annex A.2 `safe_signed` (`(d≥0) && ((uint32_t)d ≥ p)`), and V&V-FINAL-001 §4.1. The `u == p` fine tie-break reproduces the previous boundary exactly.
2. Wrap-straddle: after coarse wrap, `u` grows normally from the re-based instant; no premature emission, no re-emit burst. Forcing `period_s ≥ 2^32 − now` can no longer pin the pair in a permanent due state: inside the band `u < 2^31 ≤ period_s` (for the flood-forming values), so `u` can never exceed or equal `period_s` — **held unserved, no flood**.
3. `_FF_TimeGetCuc` fine-field domain (`time.c:216-227`, `fine ≤ 65535`) is unchanged; no dependency of this formulation on the fine field beyond the tie-break.

No period validation, no max-period limit, no time-base decision, no caller, no mission value was added — verified by direct read of the delta, `grep -rn FF_PusEmitPeriodicHk src/ apps/` (declaration + definition only, still **no production caller**), and absence of any validation branch in the `TC[3,1]` handler (the test `test_hk_large_period_held_unserved` asserts the extreme `period_s = 0xFFFFFFFF` is still **accepted**, pinning that no validation was added).

---

## 3. Focus verdicts

### Focus 1 — Authority boundary of the silent held-unserved consequence: **PASS (formulation property, recordable, authority-free in the software-only sense; activation-gated)**

**Inference/Recommendation.** The property "for `period_s ≥ 2^31` the due test never fires, so the report holds unserved" is:

- **Not a mission decision invented in software.** It follows necessarily from the 32-bit representation: with only coarse/fine state available (`time_period_due` has no epoch counter), an elapsed difference `u ≥ 2^31` is **mathematically undecidable** — it is indistinguishable from a backward step of `2^32 − u` seconds. No authority input can make the band decidable without new state (epoch definition, extended counter) — which is exactly the ESC-HK-06 question. Any wrap-safe implementation must resolve the band somehow; the resolution chosen ("wait") is the conservative tie-break, the opposite choice being the reproduced R-2 TM flood that the two prior reviews measured and ordered corrected. The `2^31` boundary is a property of any signed-difference formulation, and that formulation was recommended by both retained reviews **before** any code existed.
- **Not a max-period limit in the input-validation sense.** `period_s ≥ 2^31` is still accepted by `TC[3,1]` without range check (asserted by test); NC-EXT-208 remains fully open. The cap is a *service* consequence, not a *validation* decision.
- **Recorded, not smuggled:** in-file comment (`:396-401`, explicitly "NOT a maximum-period mission limit"), test comments, ADR-0020 reference, handover.
- **Limit that must not silently survive activation:** for permitted periods whose due instants can land in the undecided band during mission life, "held unserved" is a silent, unreported data-emission gap — mission-relevant. Under the current configuration this is inert (no caller, no requirement, ESC-HK-05 open). I record the condition: **ESC-HK-06 must dispose of the undecided-band/maximum-period policy, and ESC-HK-03 must add the held-unserved consequence to the hazard list, before any caller is introduced (ESC-HK-02).** Within those gates it remains a recordable formulation property, not an invented mission semantic.

### Focus 2 — NC register status: **PASS**

- `docs/planning/sprint-plan.md` NC-EXT-207 (line 326) records: code change **separable** from the epoch decision, correct form = wrap-safe comparison needing no mission input, to be made **before ESC-HK-02**; wrap **policy** remains authority-owned (ESC-HK-06). The candidate implements exactly that separable half; the register row is **unchanged**, NC-EXT-207 remains **OPEN** — this review does not close it and confirms the candidate does not close it.
- NC-EXT-208 (line 327) — **OPEN, untouched.** ESC-HK-06 (`docs/status/escalations.md:73`) — **AWAITING OWNER, untouched.** NC-PAL-01 (line 240) — **OPEN, untouched.** None of these is modified by the delta; `docs/planning/` and `docs/status/escalations.md` show no incremental changes attributable to this fix cycle (escalations.md already contained the ESC-HK-01..06 registration from 2026-10-24, rev 1.7).

### Focus 3 — Test adequacy for the hazard class (N22): **PASS, with retained limitations**

New tests in `test_pus.c` (registered as the last three cases of `FF_TEST_MAIN`, `:898-901`, so fixture leakage is contained; all use exact-count oracles and explicit expected `FF_Status_t` + `stream_count(3,25)` results):

| Class | Test | Coverage |
|---|---|---|
| Nominal | pre-existing `test_hk_periodic_emission` retained | exact due instant, re-arm, second period |
| Wrap-straddle | `test_hk_coarse_wrap_due` (`:705-789`) | not-due below/before wrap (u=5,9), fine-tie-break not-due (u==p, fine <), due exactly at wrapped instant, re-arm, late caller (u=13>10, exactly one), second period from re-based last (u==10), report parked |
| Forced-wrap-flood | `test_hk_large_period_held_unserved` (`:797-829`) | `period_s=0xFFFFFFFF`, three calls across three coarse ticks, **never due, zero emissions** (pre-fix: emit on every call); value still accepted (pinning no added validation) |
| Backstep + undecided band | `test_hk_backward_step_no_emission` (`:837-894`) | small backward step (−5 s) → wait; forward re-due at u==10; **backward into undecided band** (last=2010, now=1000 → u=2^32−1010 ≥ 2^31) → wait, zero emissions; late caller u=90 → exactly one |

Explicit expected results throughout, per N22. Limitations recorded (do not block this increment): (i) the test remains **not requirement-based** — no SRS/traceability entry exists (ESC-HK-05 open); trace PASS is structural and silent about this feature, as the 2026-10-24 PA review already noted; (ii) `period_s` exactly `2^31` and a period just under `2^31` are not separately exercised; (iii) the mutation-discrimination claims in the test comments (e.g. the band assertion killing a wrap-guard-eating mutant) are **author-reported, not evidenced** — see Focus 5. One author-reported fixture correction (late-caller/second-period instants re-based, 21→31 s) is consistent with the expected-result arithmetic in the test body as reviewed.

### Focus 4 — Gate evidence, independently re-run by this review (2026-10-25, exact results)

| Gate | Command | Result (this review) | Observed |
|---|---|---|---|
| Candidate digests | `sha256sum` | **PASS** | both byte-exact (§1) |
| Build | `cmake --build build/native` | **PASS** | exit 0, all targets built. Note: `make build` (the fetch-guarded wrapper) exits 2 at `fetch-third-party` with *"[fetch] error: n7s-obcp checkout is modified"* — a **pre-existing dirty-tree condition** of the preserved 168-entry batch, not attributable to this delta; the incremental cmake build is the evidence recorded here |
| Unit+integration ctest | `ctest --test-dir build/native` | **PASS** | `100% tests passed, 0 tests failed out of 46` |
| `test_pus` | `./build/native/tests/test_pus` | **PASS** | `checks=225 failures=0` — matches the author-reported count exactly |
| Traceability (N4) | `tools/scripts/trace.py` | **PASS** | `PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Lean (N1) | `lake build` in `frick-vv:local` | **PASS** | `Build completed successfully.` (lean4 v4.10.0 via elan; tool-input reproducibility caveats of the NC-EXT-35 class retained) |
| Format check | `make format-check` in `frick-vv:local` | **PASS** | no diagnostics, exit 0 (pinned clang-format) |
| GSW self-check | `make gsw-selfcheck` | **NOT RUN** | out of my remaining budget; author-reported PASS not independently reproduced |
| N18 Renode | — | **NOT RUN per directive** | FAIL preserved (NC-EXT-198) |
| Pre-fix reproduction (10 failures) / mutation discrimination | — | **NOT RUN** | author-reported only (see Focus 5) |

### Focus 5 — Review completeness at time of verdict

**Existing artifacts:** the two retained 2026-10-24 formulation-level reviews (V&V-FINAL-001, PA §5/Annex A.2 — read as inputs, not sources of my conclusions); the immutable handover; ESC-HK-01..06 in `docs/status/escalations.md` (rev 1.7); NC-EXT-207/208/NC-PAL-01 in the sprint-plan register; **this report**. **Pending at verdict time:** (a) mutation-discrimination run for the new formulation — **NOT RUN**; the repro→fix contrast (10→0 per handover, corroborated only to the extent of `checks=225 failures=0` on the final candidate) shows the tests execute the new branches but does **not** establish discrimination against specific mutants of `time_period_due`; (b) the independent V&V exact-candidate report — may be produced concurrently and is **not** closed by this PA leg; (c) sprint-plan NC-EXT-207 row update, ADR-0020 revision, evidence/verification-results handover/loop-audit updates — **not done** (per handover, dirty state preserved); this review updates none of them.

---

## 4. Verdict

### **ACCEPT — of this increment, for the narrow claim it supports**

**Accepted claim (narrow):** in `src/pus/pus_services.c` at SHA-256 `915b62e7…`, the due test for periodic HK emission is replaced by a wrap-safe, backward-step-safe signed-modular formulation whose `u ≥ 2^31` band resolves conservatively to "wait", removing the measured NC-EXT-207 R-1 premature-emit/burst and R-2 forced-wrap TM-flood regimes without adding a caller, period validation, a maximum-period limit, a time-base decision, or any mission value; and the three new tests in `test_pus.c` at SHA-256 `c42b1250…` exercise nominal, wrap-straddle, forced-wrap-flood, backstep, and undecided-band classes with explicit expected results. All gates I could re-run pass (`cmake --build`, ctest 46/46, `test_pus` 225/0, trace, Lean, format in the pinned image).

**Explicitly not claimed:** no NC closed (NC-EXT-207 remains OPEN in the register; only the *software-separable half* of its correction is now implemented and pending the completing V&V/mutation evidence — the *authority* half — uncertain band disposition, max-period, wrap/epoch policy — remains with ESC-HK-06 and NC-EXT-208); **no activation approval** — ESC-HK-02 cannot be satisfied while ESC-HK-03/05/06 and NC-EXT-208 are open and the held-unserved consequence is undisposed by the authority; no target evidence gained (N18 FAIL retained); no ECSS compliance claim anywhere in this delta.

**Conditions carried into the completing cycle:**

1. The completing V&V exact-candidate review and a mutation-discrimination run for `time_period_due` must land before the NC's software half is recorded as corrected in any register/ADR/evidence/handover/loop-audit document; until then only this report and the handover record the state.
2. ESC-HK-06 must decide the maximum permissible `period_s` and the undecided-band behaviour before activation; ESC-HK-03 must extend the hazard list with the held-unserved consequence; NC-EXT-208 validation remains required.
3. No activation, commit/push, or register/NC closure is authorized by this review.

**Reviewer independence statement:** fresh-context review from the declared digests; my arithmetic/decidability analysis re-derived independently rather than copied; gates re-executed by me; I share no authorship of any reviewed artifact. I read the two retained 2026-10-24 reviews and the handover before analyzing — independence here is demonstrated by the analysis being re-derived (and by the test register being read directly), not by blindness. The reviewer is an AI analysis role: separate reporting line, freedom from schedule pressure, and human accountability are organizational facts outside my visibility.

---

## 5. Confidence and limits (mandatory block)

```text
Decision/product and configuration:
  frick-fsw NC-EXT-207 wrap/due fix; src/pus/pus_services.c
  (915b62e7deb97e322198821c7dd5e5ef4f4e04a6ca30f84e32ee5e30b31e6de3),
  tests/unit/test_pus.c (c42b1250914f84d5d6b3c9f4045cc2d1f5dcd1a46eabf6fac479ef0496041b61),
  working tree uncommitted against HEAD 44d2af2 (preserved dirty batch).
Claim type: labelled Fact/Inference/Recommendation per item above.
Applicable baseline and approved tailoring:
  No requirement baseline exists for periodic HK (ESC-HK-05 open); assessed
  against the unit contract, project norms N1-N22, the retained 2026-10-24
  review findings, and the NC/escalation registers. No tailoring inferred.
Evidence consulted:
  Candidate digests recomputed 2026-10-25; pus_services.c:385-460 and
  test_pus.c:690-906 read directly; time.c:209-227 producer contract
  (unchanged); sprint-plan.md NC rows 326/327, 240; escalations.md:68-73
  (rev 1.7); two retained reviews + handover (2026-10-25).
Cross-check and reconciliation:
  My re-derivation agrees with PA-HK-F01 §5.4 / Annex A.2 safe_signed and
  V&V-FINAL-001 §4.1; no conflict found. Gate counts match author reports
  exactly (225/0; 46/46; trace 120/76/30/14/68).
Confidence: High for digest identity, gates re-run, formulation
  correctness/properties, register-untouched confirmation. Medium
  overall for increment recommendation, bounded by the pending items below.
Basis and detection limits:
  Host-native, single-threaded, synthetic time fixtures; no target/jitter/
  concurrency evidence; static reading of the delta only; no mutation run;
  sampling of the two declared files only; "no findings beyond those listed"
  is NOT claimed.
Missing/inaccessible evidence: mutation discrimination (not run); gsw-selfcheck
  and pre-fix repro not independently reproduced; no independent V&V final
  report yet; no requirement/hazard basis; Renode N18 FAIL unchanged.
Assumptions and expiry/recheck date: the two digests remain the review object
  (any byte change voids this report); caller remains absent; no register/NC
  mutated subsequently. Recheck on: any change to either file, introduction of
  a caller, any ESC-HK-06 decision, or 2026-11-24, whichever first.
Consequences if wrong: if the formulation analysis is wrong in the severe
  direction, an undetected due-test defect could reach activation; bounded
  today by no-caller inertness and the unchanged activation gates.
Required independent human reviewer/authority:
  Authorized human owner (ESC-HK-03/05/06 dispositions; NC-EXT-208 range;
  any activation authorization), completing V&V reviewer, System Safety
  Engineer for the held-unserved hazard, FSW architect for ESC-HK-02.
```

---

## 6. Retention (N11)

This report is written by this review; no other repository file was created or modified. No NC was closed, no manifest refreshed, no commit/push made, and N18 was not executed (FAIL preserved).

**End of PA review.**
