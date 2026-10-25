# Final Delta V&V Review: HK Periodic Emission (NC-EXT-17) — `time_add_period` Fix + Positive-Path Evidence

**Review ID:** V&V-DELTA-2026-10-24-HK-PERIODIC-FINAL
**Date:** 2026-10-24
**Reviewer:** Senior Aeronautics and Space SW V&V Engineer (independent context)
**Review role:** third (final) delta review leg of the HK periodic emission increment
**Predecessor (this reviewer, same increment):** `docs/reviews/2026-10-24-hk-periodic-vv-delta-review.md` (V&V-DELTA-2026-10-24-HK-PERIODIC)
**Authority:** Independent V&V review per norm N9. This report is **evidence and recommendation only**. It does not approve qualification, acceptance, residual risk, release or flight readiness, does not close any NC, and does not authorize commit/push.

---

## 1. Scope, candidate identity, and constraints

### 1.1 Exact candidate verified by SHA-256 (this review)

All six hashes were recomputed on the working tree at review time and **match the stated final candidate byte-for-byte**:

```
49bc86db75762cc4face23012cd110fc921d1176fb39251f92dcb7842f6cc3cf  src/pus/pus_services.c
e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd  src/pus/pus.h
57db92de95a464aa551b6eff22bf6ece8bd120cb7dfef7e37024a20ad0eb5789  tests/unit/test_pus.c
4e7177266062288e54efc12a105a66c886cf667a551971b29a3ddb8b61c2981f  tests/unit/test_pus_fu.c
6111cd610b06039eb9140c5b53ecb42875867057fad0245fb3823fef12abfab4  tests/unit/test_r2_gap.c
4c7d6c38e37a67d2cac9ca880d1e58c4420aebcb087c1805ab5b0dfd28c31260  tests/unit/test_r3_gap.c
```

### 1.2 Delta against the previously reviewed set

| File | Previously reviewed | Final candidate | Change |
|---|---|---|---|
| `src/pus/pus_services.c` | `6f435d8f…d60ed44` | `49bc86db…2f6cc3cf` | **changed** (1 code line + explanatory comment in `time_add_period`) |
| `tests/unit/test_pus.c` | `2b9dc12f…7cc886ca` | `57db92de…0eb5789` | **changed** (positive-path test + re-arm/second-period assertions) |
| `src/pus/pus.h` | `e520bb33…` | unchanged | — |
| `tests/unit/test_pus_fu.c` | `4e717726…` | unchanged | — |
| `tests/unit/test_r2_gap.c` | `6111cd61…` | unchanged | — |
| `tests/unit/test_r3_gap.c` | `4c7d6c38…` | unchanged | — |

The reviewable delta is therefore confined to two files. Everything previously verified in `pus.h`, `test_pus_fu.c`, `test_r2_gap.c`, `test_r3_gap.c` is unchanged at the byte level and is not re-derived here.

`git diff --numstat` shows the candidate files are part of a larger uncommitted working-tree batch (`pus_services.c`: 121 added / 1 removed vs `HEAD`; `test_pus.c`: 275 / 1). **Only the six declared bytes above were reviewed.** Repository state is uncommitted; no commit or push was performed or authorized by this review.

### 1.3 Constraints observed

- N18 Renode gate remains **FAIL** (NC-EXT-198). Renode was **not** run in this review.
- NC-PAL-01 remains **OPEN** (no hardware watchdog binding). Not touched.
- No emulator, hardware, mission-parameter values, mission tables or configuration baselines were modified.
- No NC closed, no frozen-manifest refresh, no commit/push, no release/flight-readiness statement.
- Non-test findings (no caller, traceability, hazard analysis, wire-format authority) are retained as **open/deferred**.
- **No ECSS compliance is claimed for the private 6-byte `TC[3,1]` wire format.** That format remains an authority-pending extension under NC-EXT-17 (ICD §4 defines 2 bytes).

---

## 2. Verdict at a glance

| Finding | Subject | Previous state (my prior delta review) | State now | Verdict |
|---|---|---|---|---|
| V&V-FIND-001 | Positive time-due emission not exercised | NOT FIXED | Positive path exercised; exact-count oracle; kills the pre-fix mutant | **PASS** |
| V&V-FIND-002 | No caller of `FF_PusEmitPeriodicHk()` | DEFERRED | Still no caller (unchanged) | **DEFERRED (open)** |
| V&V-FIND-003 | `svc3_create` init of last-emission time | FIXED | Verified again in final candidate | **PASS** |
| V&V-FIND-004 | `time_add_period` arithmetic | FIXED (claim unconfirmed by test) | Defect **reproduced**, minimally corrected, arithmetic proven exact | **PASS (with correction to my earlier wording, §3.2)** |
| V&V-FIND-005 | 6-byte `TC[3,1]` consistency | FIXED | Verified again (impl guard + 3 test files) | **PASS (authority caveat retained)** |
| V&V-FIND-006 | Traceability for periodic emission | NOT FIXED / DEFERRED | Still zero traceability/SRS entries | **DEFERRED (open)** |
| V&V-FINAL-001 | Coarse-wrap-unsafe comparison in the `time_add_period`/`time_ge` pair (new) | — | Newly identified, reproduced numerically | **OPEN — new finding (§4.1)** |
| V&V-FINAL-002 | Test-adequacy gaps: sub-second / early-by-1 s / late-call cadence (new) | — | Surviving mutants shown by experiment | **OPEN — new finding (§4.2)** |
| V&V-FINAL-003 | No payload-content oracle on the periodic branch (new) | — | Count-only oracle | **OPEN — new finding (§4.3)** |

**Increment recommendation: ACCEPT** of the fix and of the positive-path evidence, for the **claim actually supported** (see §8), conditional on recording V&V-FINAL-001/002/003 as open NCs. **No release, qualification, flight-readiness, ECSS-compliance or NC-closure claim is made.**

---

## 3. Review focus 1 — arithmetic correctness of the corrected `time_add_period`

### 3.1 The candidate code and an independent reference

Candidate (`src/pus/pus_services.c:371-382`):

```c
static void time_add_period(uint32_t coarse, uint16_t fine, uint32_t period_s, uint32_t *out_coarse,
                            uint16_t *out_fine)
{
    uint64_t total_fine = (uint64_t)fine + ((uint64_t)period_s * 65536ULL);
    *out_coarse = coarse + (uint32_t)(total_fine >> 16);
    *out_fine = (uint16_t)(total_fine & 0xFFFFu);
}
```

I did not accept the source comment's argument; I derived the closed form and then checked it against an **independently written exact reference** (Annex A) that forms the total in units of 1/65536 s in `uint64_t` and reduces modulo 2³² s:

`ref_coarse = (uint32_t)(((uint64_t)coarse*65536 + fine + (uint64_t)period_s*65536) >> 16)`, `ref_fine = (…) & 0xFFFF`.

### 3.2 Proof for the required cases

Let `f = fine`, `P = period_s`, with the domain of `f` fixed by `FF_TimeGetCuc` (`src/core/time/time.c:209-227`): `f = (frac*65536)/10⁹`, `frac < 10⁹`, so **`0 ≤ f ≤ 65535`** (floor; `f = 65536` is impossible by construction).

`total = f + P·65536`. Since `f < 65536`, `total >> 16 = P + (f >> 16) = P` and `total & 0xFFFF = f`. Hence the candidate computes exactly `(coarse + P) mod 2³²`, `fine' = fine`.

| Required case | Result | Correctness |
|---|---|---|
| `fine = 0` | `out = (coarse+P, 0)` | exact |
| `fine = 65535` | `out = (coarse+P, 65535)`; **no carry is possible** — `f` is a fractional part, adding whole seconds cannot carry it | exact. The former implementation's assumed carry is precisely the double-count: it added `P` *and* `P` again through `total >> 16` |
| `period_s = 0` | `out = (coarse, fine)` — identity | exact (the emitter also skips `period_s == 0` at line 400, so this is a pure-function property, not a live branch) |
| large `period_s` (`0xFFFFFFFF`) | `total ≤ 65535 + (2³²−1)·65536 = 2⁴⁸−65537`; `total>>16 ≤ 0xFFFFFFFF`, so the `uint32_t` cast loses **no** bits; coarse addition wraps **once**, mod 2³² | exact modular result |
| coarse wrap near `0xFFFFFFFF` | `out_coarse = coarse + P` under defined unsigned modular arithmetic | exact **mod 2³²**, which matches the coarse definition `(uint32_t)(ns/10⁹) + leap` (`time.c:220`) — the field *is* a 32-bit wrap counter, so modular equality is semantic equality |

**Can the carry-only form be off by one?** No, within the defined domain. An off-by-one would require one of: (a) `fine ≥ 65536` (impossible: the producer floors into `uint16_t`, so `fine ≤ 65535`); (b) truncation of the `(uint32_t)` cast (impossible: the value is `≤ 0xFFFFFFFF`); (c) a coarse modulus other than 2³² (not the case: the producer truncates `ns/10⁹` into `uint32_t`). The only `±1` perturbations that remain are **external**: a leap-second adjustment (`g_leap_seconds`) shifts `coarse` by ±1 between two reads, and this can move an emission instant by 1 s (bounded, non-accumulating, because `last_emission` is re-based from the same source).

### 3.3 Exhaustive differential check (executed)

Sweep: `coarse ∈ {0, 1, 1000, 0x7FFFFFFF, 0xFFFFFFF0, 0xFFFFFFFE, 0xFFFFFFFF}`, `period_s ∈ {0, 1, 2, 10, 65535, 65536, 0x7FFFFFFF, 0xFFFFFFFE, 0xFFFFFFFF}`, **all 65536 `fine` values**:

```
exhaustive fine sweep: checked=4128768 mismatches(new)=0 mismatches(old)=3670016
fixed form is coarse+period_s / fine unchanged: YES
```

- Post-fix form: **0 mismatches in 4,128,768 combinations** against the independent reference.
- Pre-fix form: **3,670,016 mismatches** in the same sweep — i.e. the double-count was not a corner case but the **normal path for every non-zero period**, consistent with the reported symptom (10 s period, due instant 1020 s instead of 1010 s). This corroborates that the previously claimed FIND-004 fix was wrong and that the new test found a real, pervasive functional defect.

**Verdict for focus item 1: PASS.** The one-line correction is arithmetically exact over the full input domain and both modular and fractional semantics.

---

## 4. New findings introduced by this deeper analysis

### 4.1 V&V-FINAL-001 — the addition is wrap-exact, but the *comparison pair* is not wrap-safe

`time_ge` (`pus_services.c:360-366`) compares raw coarse values. When `last + period` crosses the 32-bit coarse wrap, `due` becomes a small number while `now` is still large, and the pair reports "due" immediately.

Executed (same harness, exact reference arithmetic):

```
last=0x00000000 period=10 due=0x0000000A first_due_delta=10 premature_by=0
last=0xFFFFFFF0 period=10 due=0xFFFFFFFA first_due_delta=10 premature_by=0
last=0xFFFFFFFF period=10 due=0x00000009 first_due_delta=0  premature_by=10
last=0xFFFFFFF6 period=10 due=0x00000000 first_due_delta=0  premature_by=10
last=0xFFFFFFF0 period=32 due=0x00000010 first_due_delta=0  premature_by=32
```

Consequences, quantified:

- For any report whose next due instant falls **after** the coarse wrap, the emission occurs when `now == last` (Δ = 0), i.e. **premature by up to `period_s` seconds**.
- Because `last_emission` is then re-based to `now` **inside** the pre-wrap window, the same test stays true, so the report is emitted **on every call** for the remaining `2³² − now` seconds (≤ `period_s`). For a 3600 s HK report this is up to one hour of continuous HK TM generation at call rate — a TM-bandwidth/stream-saturation side effect, not merely a cadence slip.
- Frequency: once per 2³² s ≈ **136.1 years** per coarse epoch, and only for reports straddling that instant.

**Correction to my own prior report.** My predecessor review stated "Coarse wrap handled … the arithmetic is sound". That statement was about the *addition* only and remains true (§3.3). It did **not** cover the *comparison pair*, which is not wrap-safe. I record that limitation explicitly rather than letting the earlier wording stand as a broader claim.

**Classification:** genuine functional defect in the same unit, but (a) **not introduced or worsened by this fix** (present identically in the pre-fix form), (b) reachable only at a 32-bit coarse wrap whose epoch/reachability is **not defined in the controlled baseline** (no SRS requirement exists — see FIND-006), and (c) cheaply correctable (wrap-safe comparison, e.g. comparing a 64-bit difference or an extended coarse counter).
**Severity (my assessment): Minor**, escalated to the Safety reviewer for the TM-saturation aspect. Because epoch policy is undefined I **cannot** claim unreachability within mission life — the hazard is bounded, not excluded.
**Disposition required:** open NC; either correct the comparison in a follow-up cycle, or obtain an authority decision that the 32-bit coarse domain plus a defined correlation epoch makes the wrap unreachable for the mission, and record that decision as a requirement.

### 4.2 V&V-FINAL-002 — test-adequacy gaps proven by surviving mutants

Mutants that the new test does **not** kill (executed, §5.2): fine-component not updated on re-arm (M3), a 1 s early tolerance (M5). Root cause: **every fixture instant is an exact whole second**, so sub-second and off-by-one-second behaviour is invisible to the fixture. Impact is bounded (≤ 1 s cadence error, non-accumulating), but the fix is trivial: add a `period − 1` s negative check (kills M5) and a sub-second base fixture (kills M3). **Severity: Minor; test completeness, not a product defect.**

### 4.3 V&V-FINAL-003 — no payload-content oracle on the periodic branch

The positive path asserts only `stream_count(3u, 25u) == 1`. It does not inspect the periodic packet's payload, although the periodic branch **duplicates** the payload-assembly code of the one-shot branch (`pus_services.c:423-444` vs `294-353`). A regression in the periodic payload (wrong `report_id`, dropped provider bytes, wrong length) would not be detected. Low cost to close: assert payload `{0xAA,0xBB,0xCC,0xDD}` and the report-id octet for the periodic packet. **Severity: Minor.**

---

## 5. Review focus 2 — is the positive-path test real, and is it discriminating?

### 5.1 What the test asserts (`tests/unit/test_pus.c:410-565`)

| Step | Fixture | Assertion | Oracle type |
|---|---|---|---|
| Init | `FF_TimeSetUtcNs(1000 s)` — synthetic fixture, no mission parameter | report created (6-byte `TC[3,1]`, `period_s = 10`) and enabled | setup |
| Negative at enable instant | now = 1000 s | `FF_PusEmitPeriodicHk() == FF_ERROR`; `stream_count(3,25) == 0` | exact count |
| **Positive at due instant** | now = **1010 s** | `FF_PusEmitPeriodicHk() == FF_SUCCESS`; `stream_count(3,25) == 1` | **exact count** |
| Re-arm | now unchanged (1010 s) | `== FF_ERROR`; count `== 0` | exact count |
| Second period | now = 1020 s | `== FF_SUCCESS`; count `== 1` | exact count |
| One-shot `TC[3,27]` | enabled state | still works | count |
| Disabled | disabled report | `== FF_ERROR`; count `== 0` | exact count |
| `period_s = 0` | now = 5000 s | `== FF_ERROR`; count `== 0` | exact count |

The oracle is an **exact occurrence count** over the captured TM stream (`stream_count`, lines 72-90), reset per call (`g_rx_count = 0u`), not a boolean "some packet appeared". `stream_count` fails **closed**: an oversized packet is dropped by the sink and a malformed `pdl` breaks the parse, both of which yield count 0 → assertion failure rather than a false pass.

### 5.2 Discrimination experiment (independent, executed)

I did not accept the evidence record's pre-fix failure claim; I **reproduced it** in an isolated copy of the tree at `/tmp/hk-mutants/fixcheck` (candidate bytes restored and re-hashed after every mutant):

| Mutant | Injected defect | Result | Failing assertions |
|---|---|---|---|
| **M0** | none (candidate) | **PASS** (`checks=108 failures=0`) | — |
| **M1** | pre-fix double count: `coarse + period_s + (total_fine >> 16)` | **FAIL** | `test_pus.c:472`, `test_pus.c:473` (`checks=108 failures=2`) — **identical to the evidence record**, reproduced independently |
| **M2** | last-emission timestamp not advanced after emit | **FAIL** | `:478`, `:479` |
| **M3** | coarse advanced, fine not updated | **PASS (survives)** | — → V&V-FINAL-002 |
| **M4** | `time_ge` uses `fine1 > fine2` (strict at equal coarse) | **FAIL** | `:472,:473,:478,:479,:484,:485` |
| **M5** | 1 s early tolerance (`now + 1 >= due`) | **PASS (survives)** | — → V&V-FINAL-002 |
| **M6** | `last = due` instead of `last = now` | PASS (behaviourally equivalent at this call pattern; not a defect signature) | — |

Interpretation:

- **The test demonstrably fails on the pre-fix code** (M1), at exactly the reported lines. V&V-FIND-001's requirement — "the positive branch is exercised with an oracle that can actually observe the defect" — is met.
- **The oracle tests the boundary instant** (M4): equality (`now == due`) is required for emission, so the comparison is exercised at the exact due second, not merely "some later time".
- Honest limits: two plausible small defects survive (M3, M5). These are recorded as V&V-FINAL-002, not hidden.

**Verdict for focus item 2: PASS.** Exact-count oracle, fail-closed, discriminating on the actual defect, boundary-exact.

---

## 6. Review focus 3 — are the re-arm and second-period assertions meaningful?

- **Re-arm (`:475-479`).** Meaning: after an emission, `last_emission` must have advanced, otherwise the next call re-emits. This is **not tautological**: M2 (no timestamp advance) fails here, and the *partial* update (coarse only, M3) is exactly the sub-second class that the fixture cannot see — so the assertion is meaningful for the coarse component and weak for the fine component. The comment in the test ("a stale timestamp would double-emit") accurately describes what it tests.
- **Second period (`:481-485`).** Meaning: exactly one further period must elapse before the next emission, and the re-arm must not have pushed `last_emission` forward spuriously (a mutant that sets `last_emission = due + period`, or skips a cycle, fails here). It is **partially redundant** with the re-arm check and, like the rest of the test, cannot see a ±1 s error (M5 survives). It is meaningful, but its discriminating power is cadence-level, not second-level.

**Verdict for focus item 3: PASS (meaningful, with the stated sensitivity limit recorded as V&V-FINAL-002).**

---

## 7. Review focus 4 — current state of all previously reported findings

| ID | Requirement I set previously | Evidence in final candidate | Verdict |
|---|---|---|---|
| **V&V-FIND-001** | Positive time-due path exercised with an oracle able to fail | `test_pus.c:468-485`; M1 kills at `:472,:473`; M4 kills at boundary | **PASS** |
| **V&V-FIND-002** | (deferred) scheduler/application caller | `grep -rn FF_PusEmitPeriodicHk src` → only declaration (`pus.h:83`) and definition (`pus_services.c:389`). No caller. | **DEFERRED — still open (ESC-HK-02)** |
| **V&V-FIND-003** | `svc3_create` must not leave `last_emission = 0` with `enabled = true` | `pus_services.c:233-238`: `enabled = true` then `FF_TimeGetCuc(...)`; `svc3_enable` `:268-272` same | **PASS** |
| **V&V-FIND-004** | Correct second+fraction addition with modular coarse | `:371-382`, proven in §3; defect reproduced and corrected | **PASS** (see §4.1 for the *comparison* caveat and my wording correction) |
| **V&V-FIND-005** | One consistent `TC[3,1]` payload length in impl and all tests | Guard `len < 6u` at `:217`; `test_pus.c:431` 6 bytes; `test_pus_fu.c:434,465,489,554` 6 bytes; `test_r3_gap.c:424-431` 6 bytes | **PASS — authority caveat retained.** The 6-byte form is a **private, authority-pending** extension; ICD §4 defines 2 bytes. **ECSS does not authorize this wire format** and no compliance is claimed. |
| **V&V-FIND-006** | Traceability for the periodic feature | `grep -ciE 'periodic|PusEmitPeriodicHk|time_add_period' docs/requirements/traceability.yaml` → **0**; no "periodic" mention in `docs/requirements/SRS.md` | **DEFERRED — still open (ESC-HK-05)** |

---

## 8. Review focus 5 — gates, independently re-executed

Host: Linux, `gcc`, `cmake`; container `frick-vv:local` (clang-format 18.1.3, Lean toolchain elan/lean4 v4.10.0).

| Gate | Command (as run) | Result | Observed output |
|---|---|---|---|
| Build | `make test` (builds `build/native`) | **PASS** | no build errors; binaries produced |
| Unit/integration tests | `make test` | **PASS** | `100% tests passed, 0 tests failed out of 46`, total 17.86 s |
| `pus` (N22-labelled) | `build/native/tests/test_pus` | **PASS** | `checks=108 failures=0` |
| `pus_fu` | `build/native/tests/test_pus_fu` | **PASS** | `checks=159 failures=0` |
| `r2_gap` | `build/native/tests/test_r2_gap` | **PASS** | `checks=615 failures=0` |
| `r3_gap` | `build/native/tests/test_r3_gap` | **PASS** | `checks=201 failures=0` |
| Format check (pinned tool) | `docker run … frick-vv:local make format-check` | **PASS** | `Ubuntu clang-format version 18.1.3`; `--dry-run --Werror` produced no diagnostics; exit 0 |
| Format check (host) | `make format-check` | **NOT RUN / INVALID** | `error: clang-format 23 != pinned major 18 (CI runner uses 18.x)`; guard refused to run (exit 1). Tool-version mismatch, not a formatting finding. |
| Traceability | `make trace` | **PASS** | `traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Lean | `make verify-lean` (host) | **NOT RUN** | `/bin/sh: 1: lake: not found` (no elan on host) |
| Lean | `docker run … lake build` in `verification/lean` | **PASS** | `Build completed successfully.` (toolchain lean4 v4.10.0 fetched from `releases.lean-lang.org` — see §9, tool-input note) |
| GSW/self-checks | `make gsw-selfcheck` | **PASS** | 9 + 14 Python checks `OK`; `license_check: OK`; GSW frame self-check emits TC frame |
| N18 Renode | — | **NOT RUN (by directive)** | Gate remains **FAIL** (NC-EXT-198); untouched |
| RTEMS cross-build | — | **NOT RUN** | Not affected by the delta; toolchain identity unchanged |
| MISRA / coverage | — | **NOT RUN** | Advisory, out of delta scope |

**Gate conclusion:** all applicable non-emulation gates pass with the pinned toolchain. The two "NOT RUN" entries on the host are tool-availability facts, not failures; both were re-executed successfully in the controlled image. The evidence record's gate table is therefore **corroborated**, with the one clarification that `make format-check` is only valid inside the pinned image (or with `FF_CLANG_FORMAT_ALLOW_UNPINNED=1`, which would not be evidence).

---

## 9. Review focus 6 — no mission-time parameter introduced

- The candidate delta adds **no** configuration or mission value: `config/fsw_config.h` has no periodic/HK-period/time parameter (`grep -niE 'periodic|hk_report|period_s' config/fsw_config.h` → no match), and the candidate file set excludes `config/`.
- The test uses `FF_TimeSetUtcNs(1000ULL * 1000000000ULL)` (`test_pus.c:426`, and later 1010 s, 1020 s, 5000 s) with the explicit comment *"Synthetic test value, not a mission time parameter"*. This is the **public** time-service API, used as a test fixture; it is acceptable for the intended purpose.
- Side effects are contained: (a) `FF_TimeSetUtcNs` implicitly selects `FF_TIME_SRC_GROUND` (`time.c:117-126`) — process-local to the test binary; (b) `test_hk_periodic_emission` is the **last** entry of the single `FF_TEST_MAIN` list in `test_pus.c` (line 569), so no other case in that binary inherits the fixture time; (c) each test binary is a separate process. No cross-binary contamination.
- Observation (minor, process): the test re-runs `FF_PalInit/FF_EvsInit/FF_TimeInit/FF_PusInit` mid-process although earlier cases already initialised them. It passes and is consistent with the existing file style, but it means the positive path is validated on a **re-initialised** stack rather than only at boot — worth keeping in mind if init idempotency ever changes.

**Verdict: PASS.**

---

## 10. Residual risks and uncertainty

| # | Residual risk | Basis | Bound / mitigation |
|---|---|---|---|
| R1 | **Coarse-wrap premature emission + re-emission burst** (V&V-FINAL-001) | reproduced numerically (§4.1) | ≤ `period_s` s early and ≤ `period_s` s of emission-per-call at the 2³² s wrap; once per ≈136.1 y per epoch. Reachability depends on an **undefined** CUC correlation epoch → not excluded. Requires NC disposition or authority decision. |
| R2 | **1 s resolution / floor quantization** | `FF_TimeGetCuc` floors `fine` (`time.c:224`) | due instants up to 15.26 µs early; negligible vs 1 s scheduling, no accumulation |
| R3 | **Leap-second step** shifts `coarse` by ±1 between emission instants | `g_leap_seconds` in `time.c:216-220` | ±1 s one-off per leap; not covered by any test |
| R4 | Sub-second and off-by-one-second cadence errors undetectable by the current fixture | mutants M3, M5 survive (§5.2) | bounded ≤ 1 s, non-accumulating; fix by adding a `period−1` s negative check and a sub-second fixture |
| R5 | **No target/timing evidence at all for this feature** | N18 Renode FAIL (NC-EXT-198) | all evidence is host-native; no claim about WCET, interrupt latency, bus timing, or on-target cadence |
| R6 | **Feature not exercisable in operation** | no caller (FIND-002) | the whole increment's evidence is component-level; integration evidence does not exist |
| R7 | **No requirements/hazard basis** | FIND-006 / ESC-HK-03 pending | cadence, wrap policy, missed-period policy, and provider-failure policy in the periodic path are **undocumented behaviours**; the "first error wins unless a report succeeded" return-code rule is also undocumented |
| R8 | Provider-failure and multi-report branches of the periodic path unexercised | test uses one healthy provider | error propagation, partial success, `FF_BAD_SIZE` on over-report are only covered on the one-shot path |
| R9 | Tool-input identity | container fetched lean4 v4.10.0 from the network during this review; elan/apt inputs are mutable | consistent with the already-open NC-EXT-35 class of findings; not evidence of reproducibility |

**Untested / unobservable state space (explicit):** sub-second cadence, wrap crossing, leap transitions, multi-report due in the same call, provider failure/timeout in the periodic branch, late calls (call while `now ≫ due`), scheduler jitter, on-target timing and concurrency, and any interaction with the (absent) caller. **No statement of defect absence is made.**

---

## 11. Recommendation

### ACCEPT — of this increment, for the claim it actually supports

**Accepted claim (narrow):** in the host-native unit environment, `FF_PusEmitPeriodicHk()` emits exactly one `(3,25)` report at the exact due instant `last_emission + period_s` for an enabled report with `period_s > 0`, re-arms so that an immediate second call emits nothing, emits again at the next period, and remains silent for disabled and `period_s = 0` reports; and `time_add_period()` computes `(coarse + period_s) mod 2³²` with `fine` preserved **exactly**, as shown by differential testing against an independent exact reference over 4,128,768 input combinations.

**Justification:** the previously reported defect was real, pervasive (3,670,016/4,128,768 reference mismatches in the pre-fix form), and is now correct; the new test independently reproduces the failure on the pre-fix code at the exact lines reported, uses a fail-closed exact-count oracle, and is boundary-exact. All applicable gates pass with the pinned toolchain.

**Conditions attached to this acceptance (not release or flight conditions, but increment conditions):**

1. Record **V&V-FINAL-001** (wrap-unsafe comparison), **V&V-FINAL-002** (sub-second/±1 s mutant survival), **V&V-FINAL-003** (no payload-content oracle) as **open NCs** with owner and target cycle. V&V-FINAL-001 additionally goes to the Safety reviewer for the TM-burst aspect.
2. Keep **V&V-FIND-002** (no caller) and **V&V-FIND-006** (no traceability/SRS/hazard basis) **open/deferred**; this review does not close them.
3. Retain the **no-ECSS-compliance** statement for the private 6-byte `TC[3,1]` wire format (authority pending).
4. Retain **N18 Renode FAIL (NC-EXT-198)** and **NC-PAL-01 OPEN** in the status record; the increment gains no target evidence.
5. Recommended cheap test reinforcements for the next cycle (kill M5 and M3, and close V&V-FINAL-003): a negative check at `period − 1` s; a sub-second base fixture; a payload-content assertion on the periodic packet; one call well after `due`.

**This review does not approve** qualification, acceptance, anomaly/NC closure, waiver, residual risk, certification, or flight readiness. Those remain with the authorized human authority after independent PA (N20) and safety review.

### Reviewer independence statement

This review was performed from the six declared hashes with independent derivation (closed-form proof + differential harness written for this review), independent mutation experiments (M0–M6, all executed here), and gate re-execution in both host and pinned container. It shares no authoring role on the reviewed files. Independence dimensions achieved: separate author/reviewer, separate analytical method (independent reference model rather than re-reading the implementation), separate environment (isolated tree copy for mutation). Dimensions **not** established: separate reporting line and freedom from schedule pressure are organizational facts outside this review's visibility, and the reviewer is an AI analysis role without human credentials or regulatory delegation.

---

## 12. Evidence retention (norm N11)

Retained in the working tree:

- This report: `docs/reviews/2026-10-24-hk-periodic-vv-final-delta-review.md`
- Candidate identity: SHA-256 listing in §1.1 (recomputed at review time)
- Gate outputs: `make test` (46/46 PASS), per-binary check counts, `make trace` (PASS), container `make format-check` (PASS), container `lake build` (PASS), `make gsw-selfcheck` (PASS)
- Mutation and arithmetic experiment logs: `/tmp/hk-mutants/{M0..M6}.{build,run}.log`, `/tmp/hk-mutants/time_check.c` (ephemeral, outside the repository; the harness is reproduced in Annex A so the numbers above are independently re-runnable)
- Not retained by design: Renode, RTEMS, MISRA and coverage artifacts (not run, per constraints/scope)

No file in the repository other than this report was created or modified by this review.

---

## Annex A — independent arithmetic harness used for §3.3 and §4.1

```c
/* Reference: exact arithmetic in units of 1/65536 s, reduced mod 2^32 s. */
static void add_new(uint32_t coarse, uint16_t fine, uint32_t period_s, uint32_t *oc, uint16_t *of)
{
    uint64_t total_fine = (uint64_t)fine + ((uint64_t)period_s * 65536ULL);
    *oc = coarse + (uint32_t)(total_fine >> 16);          /* candidate (post-fix) */
    *of = (uint16_t)(total_fine & 0xFFFFu);
}
static void add_old(uint32_t coarse, uint16_t fine, uint32_t period_s, uint32_t *oc, uint16_t *of)
{
    uint64_t total_fine = (uint64_t)fine + ((uint64_t)period_s * 65536ULL);
    *oc = coarse + period_s + (uint32_t)(total_fine >> 16); /* pre-fix (defective) */
    *of = (uint16_t)(total_fine & 0xFFFFu);
}
static void add_ref(uint32_t coarse, uint16_t fine, uint32_t period_s, uint32_t *oc, uint16_t *of)
{
    uint64_t total = (uint64_t)coarse * 65536ULL + fine + (uint64_t)period_s * 65536ULL;
    *oc = (uint32_t)((total >> 16) & 0xFFFFFFFFULL);
    *of = (uint16_t)(total & 0xFFFFu);
}
static bool ge(uint32_t c1, uint16_t f1, uint32_t c2, uint16_t f2)
{
    if (c1 != c2) return c1 > c2;
    return f1 >= f2;                                       /* candidate time_ge */
}
/* Sweep: coarse in {0,1,1000,0x7FFFFFFF,0xFFFFFFF0,0xFFFFFFFE,0xFFFFFFFF},
 *        period_s in {0,1,2,10,65535,65536,0x7FFFFFFF,0xFFFFFFFE,0xFFFFFFFF},
 *        fine in [0,65535];  compare add_new/add_old against add_ref; then scan
 *        forward deltas from `last` to find the first instant `ge()` reports due. */
```

Compiled with `gcc -std=c11 -Wall -Wextra -Werror -O2`; exit code 0 only if the post-fix form matches the reference everywhere.

---

**End of Final Delta Review Report**

*Reviewer note on limits of this report:* it establishes what the candidate code and tests do under the recorded conditions. It does not establish absence of defects, operational fitness, target-timing behaviour, requirements compliance, or certification. Passing tests and passing gates are bounded by the reviewed configuration, the host-native environment, and the oracles described above.
