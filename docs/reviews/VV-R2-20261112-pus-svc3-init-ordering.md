# V&V Review Report — VV-R2-20261112-PUS-SVC3-INIT-ORDERING

**Independent review under norm N10 / R2 profile — Senior Aeronautics & Space SW V&V Engineer.**
This persona has no approval, qualification, certification, or release authority. Safety-critical conclusions require qualified independent human review and the approvals defined by the governing baseline.

## Meta

| Field | Value |
|---|---|
| Repository | /home/nicola/fsw/frick-fsw |
| Branch / HEAD | main / b6bf060 (unstaged working-tree change only) |
| Change scope | `src/pus/pus_services.c` only — `svc3_create` (lines 233-239) and `svc3_enable` (lines 270-274) |
| Nature | Source-code fix; host-executable; NOT target/hardware; NOT GAP-A5/NC-PAL-01/OBCP engine; NC-EXT-19/90, N22-TS5-47 untouched |
| Review date | 2026-11-12 |
| Reviewer | Independent V&V (N10/R2 profile); no commit/push or approval authority |
| Gate results (per change author) | build exit 0 (0 warnings); CTest exit 0 incl. test_pus.c; Lean lake build; trace PASS 120/76/30/14/68; format-check PASS; license OK |

## 1. Change under review

In `svc3_create` and `svc3_enable`, the initialization order of a housekeeping report record was reversed:

- **Before:** `g_hk_reports[i].enabled = true;` (and `period_s`) was assigned **before** `(void)FF_TimeGetCuc(&last_emission_coarse, &last_emission_fine);`.
- **After:** `FF_TimeGetCuc(...)` initializes `last_emission_coarse/fine` **before** `g_hk_reports[i].enabled = true;`.

`g_hk_reports` is zero-initialized at `FF_PusInit()` (`src/pus/pus_services.c:680`, `memset(g_hk_reports, 0, ...)`), so a report's `last_emission` starts at coarse=0, fine=0.

**Defect (V&V-FIND-003 / NC-EXT-211):** A concurrent `FF_PusEmitPeriodicHk()` interleaved between `enabled=true` and `last_emission=now` could observe `enabled=true` with `last_emission=0`. `time_period_due()` (`src/pus/pus_services.c:408-422`) computes `u = now_coarse - 0 = now_coarse`; for any `period_s < now_coarse` (e.g. `now=1000 s, period_s=10 s`) it returns `true`, causing an immediate spurious (3,25) HK emission at the enable instant instead of after one full period.

`FF_PusEmitPeriodicHk` (`src/pus/pus_services.c:429-496`) reads `now` via `FF_TimeGetCuc` (`src/core/time/time.c:209-227`, always returns `FF_SUCCESS`) and skips reports where `!used || !enabled || period_s == 0` (line 440).

**Gate on this NC:** `docs/planning/sprint-plan.md:338` — NC-EXT-211 is "Minor latent concurrency NC … Latent only: the emitter has no production caller today. OPEN — DEFERRED under N21. Must be resolved (ordering or a claim of single-threaded access) before activation (ESC-HK-02); no reproduced failure exists in the current build." Confirmed: `FF_PusEmitPeriodicHk` has no production caller (only the definition at `pus_services.c:429`, declaration `pus.h:92`, and test usage). Prior reviews closed V&V-FIND-003 as FIXED/PASS once `last_emission` was initialized to now before enabling.

## 2. Evidence reviewed

| Artifact | Relevance |
|---|---|
| `src/pus/pus_services.c` | Change (lines 233-239, 270-274); struct `ff_hk_report_t` (lines 50-58); `g_hk_reports` (line 71); `time_period_due` (lines 408-422); `FF_PusEmitPeriodicHk` (lines 429-496); zero-init (line 680) |
| `tests/unit/test_pus.c` | `test_verification_missing_subservices` (lines 413-480); `test_hk_periodic_emission` (lines 499-658), registered at line 910, run as CTest `pus` |
| `tests/unit/test_r2_gap.c` | `test_pus_service3_hk` (lines 881-927) — service-3 lifecycle/return codes |
| `src/core/time/time.c` | `FF_TimeGetCuc` (lines 209-227) |
| `docs/planning/sprint-plan.md:338` | NC-EXT-211 definition and gate (ESC-HK-02) |
| `docs/reviews/2026-10-24-hk-periodic-vv-final-delta-review.md` | V&V-FIND-003 history (PASS/CLOSED) |
| `docs/reviews/2026-10-24-hk-periodic-pa-final-delta-review.md` | PA-HK-F05 (latent recurrence path, benign today) |
| `docs/vv/tool-qualification.md:252` | T3 qualification status for `test_pus.c` |

## 3. Findings

### VV-R2-001 — Test-coverage sufficiency and line-range mis-citation (Minor — does NOT block commit/push)

**Finding.** The change context states that `tests/unit/test_pus.c` lines 413-480 "cover `FF_PusEmitPeriodicHk` … asserting `== FF_SUCCESS` for all 8 subservice/stage combos." That is **inaccurate**. Lines 413-480 are `test_verification_missing_subservices`, which exercises `FF_PusSendVerification` (Service 1, subservice 0, stages START/PROGRESS/ACCEPTANCE/COMPLETION). It does **not** cover `FF_PusEmitPeriodicHk` or Service 3.

The actual Service 3 periodic-HK test is `test_hk_periodic_emission` at `tests/unit/test_pus.c:499-658`, registered at line 910. It **does** assert the behavior the fix protects:

- `test_pus.c:555-558` — immediately after enable, `FF_PusEmitPeriodicHk() == FF_ERROR` and `stream_count(3u, 25u) == 0` (no spurious emission at the enable instant).
- `test_pus.c:562-565` — one period later, `FF_PusEmitPeriodicHk() == FF_SUCCESS` and exactly one (3,25) report emitted (first emission after one full period).
- `test_pus.c:569-571` — immediate re-call is not due (no double-emit from a stale timestamp).
- `test_pus.c:574-577` — second period emits again.

**Limitation (evidence sufficiency).** The test is **single-threaded** and calls `FF_PusEmitPeriodicHk()` only after `FF_PusIngest()` returns, by which point both `enabled=true` and `last_emission=now` are set **regardless of ordering**. Therefore this test passes **both before and after** the fix. It is a valid regression guard for the intended behavior (no immediate spurious emission; first emission after one period), but it is **not discriminating for the initialization-ordering fix** and does not exercise the concurrency window (`enabled=true` observed with `last_emission=0`).

**Assessment.** Acceptable for this change because NC-EXT-211 is latent (no production caller), was already OPEN/deferred under N21, and V&V-FIND-003 was previously closed by review plus the behavioral regression test. The ordering change is verified by code inspection against `time_period_due` (`pus_services.c:408-422`) and `FF_PusEmitPeriodicHk` (`pus_services.c:429-496`).

**Recommendation (not a commit gate).** Strengthen the evidence before ESC-HK-02 activation by either:
- (a) adding a white-box assertion that `last_emission_coarse/fine` is non-zero (== current CUC) immediately after enable — requires a test-hooks accessor because `g_hk_reports` is `static` to `pus_services.c`; or
- (b) documenting review-based closure of NC-EXT-211 citing: single-threaded unit context, no production caller, ordering fix, and the `test_hk_periodic_emission` regression guard at `test_pus.c:555-577`.

### VV-R2-002 — Additional tests needed? (Minor — no action required to unblock; does NOT block commit/push)

**Finding.** No additional test is strictly required to proceed. The functional intent of the fix (no spurious emission immediately after enable; first emission after one full period) is already covered by `test_hk_periodic_emission` (`test_pus.c:555-577`), which passes. The concurrency ordering aspect cannot be discriminated by the existing single-threaded harness, and adding a deterministic inter-thread interleaving test is not practical here; the defect is latent with no production caller.

**Recommendation.** Follow VV-R2-001(a)/(b) before ESC-HK-02 activation. Do not add a test solely to satisfy a coverage metric; coverage must be traceable to a claim/hazard (`docs/rules.md` N22), and `test_pus.c` already covers the Service 3 nominal/out-of-bound emission behavior.

### VV-R2-003 — Effect on existing test behavior / regression (Minor, informational — does NOT block commit/push)

**Finding.** The change does not alter output structure. `test_hk_periodic_emission` confirms first emission occurs after one full period (`test_pus.c:562-565`), no immediate emission at enable (`:555-558`), no double-emit on immediate re-call (`:569-571`), and emission again at the second period (`:574-577`). `test_pus_service3_hk` (`tests/unit/test_r2_gap.c:881-927`) checks Service 3 return codes and one-shot TM emission; it does not assert on `last_emission` ordering or call the periodic emitter with time advance, so it is unaffected. CTest exit 0 confirms no existing test behavior regressed.

### VV-R2-004 — Tool qualification / N10 (Minor, informational — does NOT block commit/push)

**Finding.** No new test scripts were added; no T3 impact; no N10 re-qualification trigger. `test_pus.c` (including `test_hk_periodic_emission`) was already reviewed under T3 (FRICK-REVIEW-0443 covers `test_pus.c`) and T3 status is QUALIFIED (`docs/vv/tool-qualification.md:252`). No tool under `tools/` changed; no T3 criterion/oracle changed. V&V tool qualification is unaffected.

### VV-R2-005 — `(void)` discard of `FF_TimeGetCuc` status (Minor observation, pre-existing — does NOT block commit/push)

**Finding.** Both call sites use `(void)FF_TimeGetCuc(&last_emission_coarse, &last_emission_fine);` (`pus_services.c:236-237`, `:272-273`), so a time-base failure would leave `last_emission` at zero while `enabled=true` is published — re-opening the race window on a time failure. This pattern is **pre-existing** (the unchanged middle line of the diff) and is not introduced by this change. `FF_TimeGetCuc` currently always returns `FF_SUCCESS` (`time.c:209-227`) and takes the time mutex. Accepted as low risk; record as an observation rather than blocking the change.

## 4. Behavior / regression summary

| Question | Answer |
|---|---|
| Does `FF_PusEmitPeriodicHk` still return `FF_SUCCESS`/`FF_ERROR` as before? | Yes; output structure unchanged |
| First emission after enable? | After one full period (was: could be immediate if `last_emission` observed as 0) |
| Immediate re-call after emission? | Not due (no double-emit) — verified `:569-571` |
| Disabled / `period_s == 0` reports? | Skipped (line 440) — unaffected |
| Existing tests regressed? | No — CTest exit 0 |

## 5. Conclusion and commit/push recommendation

**No finding blocks commit/push.** All applicable gates pass (build, CTest incl. `test_pus.c`, Lean, trace, format, license). The change resolves the previously identified V&V-FIND-003 / NC-EXT-211 initialization-ordering defect via a verified ordering (`FF_TimeGetCuc` before `enabled=true`) protected by the `test_hk_periodic_emission` regression guard (`test_pus.c:555-577`).

**Conditions before proceeding:**
1. The change is committed/pushed **only** by the designated `aeronautics-space-fsw-engineer` after all gates/norms pass and the full sprint scope is complete (norm N7/N15); this V&V review does not waive any gate or approve risk/release.
2. Review evidence retained in the working tree immediately (this file; norm N11); revision history updated (norm N16).
3. NC-EXT-211 / V&V-FIND-003 closure basis documented before ESC-HK-02 activation (per VV-R2-001 recommendation); the NC remains latent/deferred under N21 until then.

**V&V statement:** Objective evidence supports that the delivered change correctly implements its specified intent (no spurious periodic-HK emission immediately after enable; first emission after one period) under the recorded host/unit conditions. It does not prove absence of the latent concurrency race in a multi-threaded production context, which is why NC-EXT-211 remains tracked and must be resolved before activation (ESC-HK-02).

---
*End of report. Classification key: Major = significant evidence gap or defect that may block commit; Minor = issue/observation that does not block commit but requires tracking or a documented basis.*
