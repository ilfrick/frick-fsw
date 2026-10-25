# NC-EXT-207 (S3 periodic due-test wrap/TM-flood) — reproduced defect and fix evidence

Date: 2026-10-25
Tree: `/home/nicola/fsw/frick-fsw` (uncommitted working tree)
Sprint: S8-PUS-FU
Constraints retained: **N18 Renode remains FAIL (NC-EXT-198)** and was **NOT RUN**;
**NC-PAL-01 remains OPEN**. No emulator, hardware, mission parameter or wire-format
decision was made. No NC was closed. No frozen manifest was refreshed. No commit/push.

Independent reviews:
- `docs/reviews/2026-10-25-ncext207-wrap-due-vv-review.md` (V&V exact-candidate review)
- `docs/reviews/2026-10-25-ncext207-wrap-due-pa-review.md` (PA exact-candidate review)
Both **ACCEPT** narrow claim.

## 1. Scope actually addressed

NC-EXT-207 disposition reads: wrap-safe comparison needing no mission input exists
and must be made **before the first caller is introduced (ESC-HK-02)**; the
correct disposition of the time-base (CUC epoch, 32-bit coarse wrap
representation, permitted backward steps, maximum `period_s`) is
authority-owned (ESC-HK-06). Only the software-separable correction was addressed:

- `time_ge`/`time_add_period` (raw coarse comparison + addition) replaced by
  `time_period_due` (signed modular elapsed-vs-period);
- `u >= 2^31` undecided band resolves to "wait" (conservative tie-break);
- `period_s >= 2^31` reports hold unserved (no flood, no emission) — formulation
  property, NOT a max-period validation/limit (NC-EXT-208, ESC-HK-06 remain
  authority-owned);
- Wire behavior unchanged outside previously defective regimes; no caller added.

## 2. Defect reproduced before the fix

Pre-fix `time_ge`/`time_add_period` in `src/pus/pus_services.c` (sha256
`05508611…`): raw coarse comparison caused (a) premature emission + re-emit burst
at the epoch wrap (R-1 regime) and (b) forced-wrap TM flood from a single
`period_s >= 2^32 - now` command (R-2 regime). Extended test suite (`test_pus`
with three new tests) against pre-fix source:

```
checks=225 failures=10 (make test: 98%, 1 of 46 failed)
```

The pre-existing test `test_hk_periodic_emission` encoded the defect by asserting
the exact due instant without covering wrap/large-period/backstep cases.

## 3. Minimal fix

`src/pus/pus_services.c` — removed `time_ge`/`time_add_period`, added
`time_period_due()` (lines 406-420):

```c
static bool time_period_due(uint32_t now_coarse, uint16_t now_fine,
                            uint32_t last_coarse, uint16_t last_fine,
                            uint32_t period_s)
{
    uint32_t u = now_coarse - last_coarse; /* defined unsigned modular diff */
    if (u >= 0x80000000u) {
        return false; /* undecided band: backward step or >= 2^31 s -> wait */
    }
    if (u > period_s) {
        return true;  /* interval elapsed (late caller emits once) */
    }
    if (u < period_s) {
        return false; /* not yet due */
    }
    return now_fine >= last_fine; /* u == period: sub-second tie-break */
}
```

Properties:
- For all `u < 2^31`: exact signed modular difference `d = (int32_t)(now-last)`,
  matching the decision-free wrap-safe form recommended by both retained
  reviewers (PA Annex A.2 `safe_signed`, V&V §4.1).
- Wrap-straddle: after coarse wrap, `u` grows normally from the re-based instant;
  no premature emission, no re-emit burst.
- Forced-wrap flood: for `period_s >= 2^32 - now`, `u < 2^31 <= period_s`, so
  `u` can never exceed or equal `period_s` — held unserved, no flood.
- Tie-break `u == period`: `now_fine >= last_fine` reproduces previous boundary
  exactly.
- `period_s >= 2^31`: held unserved (formulation property, no validation added).

## 4. Tests added

Three new tests in `tests/unit/test_pus.c` (registered in `FF_TEST_MAIN`):

| Test | Coverage | Oracle type |
|------|----------|-------------|
| `test_hk_coarse_wrap_due` | Wrap-straddle at `last=0xFFFFFFFE` p=10: not-due u=5,9; fine tie-break not-due (u==p, fine<); due exactly at wrapped instant; re-arm; late caller u=13>10 (one emission); second period from re-based last | Exact `stream_count(3,25)` + `FF_Status_t` per step |
| `test_hk_large_period_held_unserved` | `period_s=0xFFFFFFFF` accepted, three calls across three ticks: **never due, zero emissions** (pre-fix: emit every call) | Zero-emission loop + value-acceptance pin |
| `test_hk_backward_step_no_emission` | Small backstep (-5s) → wait; forward re-due at u==10; backstep into undecided band (last=2010, now=1000 → u=2^32-1010 ≥ 2^31) → wait; late caller u=90 → exactly one | Exact counts per regime |

All use real non-tautologous oracles (explicit expected results, per N22).
Limitation: not requirement-based (ESC-HK-05 open).

## 5. Mutation discrimination (executed 2026-10-25)

| Mutant | Change | Expected kill site |
|--------|--------|-------------------|
| M1: wait band removal (`u >= 2^31` → `u >= 0xFFFFFFFF`) | `test_hk_backward_step_no_emission` emits in undecided band |
| M2: tie-break `>=` → `>` | `test_hk_coarse_wrap_due` fails at `now_fine == last_fine` |
| M3: `u > period` → `u >= period` | `test_hk_coarse_wrap_due` fires early at u==period (pre-tie) |

All three mutants built under `/tmp/mutant_test`, linked against test binary,
**killed** (failures at expected lines). No mutation gap for critical
formulation decisions.

## 6. Gates on the final candidate — exact results

| Gate | Command | Result |
|------|---------|--------|
| Candidate digests | `sha256sum` | PASS (byte-exact match) |
| Build | `cmake --build build/native` | PASS (exit 0) |
| | `make build` | PASS (exit 0; `fetch-third-party` clean) |
| Unit+integration ctest | `ctest --test-dir build/native` | PASS (46/46, exit 0) |
| `test_pus` | `./build/native/tests/test_pus` | PASS (225 checks, 0 failures) |
| Traceability (N4) | `tools/scripts/trace.py` | PASS (120 req, 76 SRS, 30 SSS, 14 gaps, 68 theorems) |
| Lean (N1) | `lake build` in `frick-vv:local` | PASS |
| Format check | `make format-check` in `frick-vv:local` | PASS |
| GSW self-check | `make gsw-selfcheck` | PASS |
| N18 Renode | — | NOT RUN (FAIL preserved, NC-EXT-198) |
| Pre-fix reproduction | — | 10 failures (author-reported, corroborated by 225/0 contrast) |

## 7. Candidate hashes (SHA-256, final)

```
915b62e7deb97e322198821c7dd5e5ef4f4e04a6ca30f84e32ee5e30b31e6de3  src/pus/pus_services.c
c42b1250914f84d5d6b3c9f4045cc2d1f5dcd1a46eabf6fac479ef0496041b61  tests/unit/test_pus.c
```

## 8. What remains open after this increment

- **Authority half:** ESC-HK-06 (max-period/undecided-band policy), ESC-HK-03
  (held-unserved hazard), NC-EXT-208 (period validation) — all authority-owned.
- **Activation gate:** ESC-HK-02 (caller) cannot be satisfied while above are open.
- **Test adequacy:** ESC-HK-05 (requirement baseline) open; `period_s` exactly
  `2^31` and just-under not separately exercised; test not requirement-traced.
- **N18 Renode:** FAIL (NC-EXT-198), NOT RUN. NC-PAL-01 OPEN.
- **Doc updates:** loop-audit.md not updated in this turn (N19).

## 9. Revision history

| Rev | Date | Author | Change |
| --- | --- | --- | --- |
| 0.1 | 2026-10-25 | frick-fsw (delegated) | Record reproduced defect, minimal fix, mutation discrimination, full gate results, authority residuals, and final candidate hashes. |
