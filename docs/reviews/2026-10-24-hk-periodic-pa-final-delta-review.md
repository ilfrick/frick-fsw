# PA Final Delta Review — HK Periodic Emission (NC-EXT-17)

**Review ID:** PA-DELTA-2026-10-24-HK-PERIODIC-FINAL
**Date:** 2026-10-24
**Reviewer:** Senior Aeronautics and Space SW Product Assurance / Safety Engineer (independent context)
**Review role:** third (final) PA leg of the HK periodic emission increment, norm N20
**Predecessors (same reviewer, same increment):**

- `docs/reviews/2026-10-24-hk-periodic-safety-review.md` (initial safety review, SF-01..SF-08)
- `docs/reviews/2026-10-24-hk-periodic-safety-delta-review.md` (delta review: SF-08 PARTIALLY FIXED, SF-04 "fixed" but unconfirmed by test)

**Companion leg:** `docs/reviews/2026-10-24-hk-periodic-vv-final-delta-review.md` (V&V, ACCEPT narrow)

**Authority statement.** This report is independent assurance evidence and a recommendation only. It does not certify compliance, approve or close an NC, accept residual risk, qualify a tool, approve a waiver or deviation, approve release or flight readiness, or authorize commit/push. Those decisions remain with the authorized human authority after review of this report.

---

## 1. Candidate identity — verified

All six declared SHA-256 digests were recomputed on the working tree at review time and **match the stated final candidate byte-for-byte** (Fact, high confidence):

```
49bc86db75762cc4face23012cd110fc921d1176fb39251f92dcb7842f6cc3cf  src/pus/pus_services.c
e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd  src/pus/pus.h
57db92de95a464aa551b6eff22bf6ece8bd120cb7dfef7e37024a20ad0eb5789  tests/unit/test_pus.c
4e7177266062288e54efc12a105a66c886cf667a551971b29a3ddb8b61c2981f  tests/unit/test_pus_fu.c
6111cd610b06039eb9140c5b53ecb42875867057fad0245fb3823fef12abfab4  tests/unit/test_r2_gap.c
4c7d6c38e37a67d2cac9ca880d1e58c4420aebcb087c1805ab5b0dfd28c31260  tests/unit/test_r3_gap.c
```

| Item | State | Evidence |
|---|---|---|
| Candidate differs from the set I last reviewed | Confirmed | `pus_services.c` `6f435d8f…` → `49bc86db…`; `test_pus.c` `2b9dc12f…` → `57db92de…` |
| Reviewable delta confined | Confirmed | `git diff` hunks for `pus_services.c` touch only the Service-3 HK region (`@@ -54,0 +55,3`, `@@ -211`, `@@ -215`, `@@ -226`, `@@ -256`, `@@ -340,0 +357,104`); no `config/`, `hal/`, `drivers/`, `bsp/`, `board/`, `tools/docker/`, `tools/scripts/renode*` hunk exists in the delta |
| Committed? | **No** | working-tree modification against `HEAD` (44d2af2); candidate files show as modified, not committed |
| Evidence record hash-linked | Confirmed | `docs/vv/evidence/2026-10-24-hk-periodic-ncext17-evidence.md` §4/§5 |
| Prior review reports retained and unaltered | Confirmed — all four recorded digests recomputed and match | §5 of the evidence record |

**Scope limitation (Open issue, low materiality).** I verified the *six declared* files and their delta. The working tree also carries unrelated uncommitted changes from other increments (watchdog/S8-FDIR, docs). No statement in this report applies to those files.

---

## 2. What this review did and did not do

**Performed (independent of the V&V leg):**

1. Recomputed all six candidate digests.
2. Read the UUT (`pus_services.c:211-460`) and the positive-path test (`test_pus.c:410-570`) directly.
3. Re-derived the time arithmetic from the *producer* contract (`FF_TimeGetCuc`, `src/core/time/time.c:209-227`) rather than from the source comment, and ran an **independently written** differential harness against an exact reference (Annex A), 4,128,768 combinations.
4. Independently modelled the `time_add_period`/`time_ge` **pair** as a state machine (emission instant, burst duration, backward-step behaviour), and modelled forced-wrap inputs.
5. Re-ran the host gates I can run (`make test`, `make trace`, per-binary check counts).
6. Audited the scope claims: wire-format compliance, mission values, hardware binding, emulator change, NC closure, commit.

**Not performed / not re-run (by constraint):** Renode (N18 remains FAIL, NC-EXT-198), RTEMS cross-build, Lean (no `lake` on host), coverage/MISRA. No emulator, hardware or mission parameter was touched.

---

## 3. Per-finding verdicts (SF-01 … SF-08)

| ID | Subject | My prior state | State in this candidate | Verdict |
|---|---|---|---|---|
| **SF-01** | TC[3,1] length guard before payload access | FIXED | `if (len < 6u) return FF_BAD_SIZE;` at `:217`, precedes every payload read (`d[0]` `:219`, `d[1]` `:220`, `rd_u32(&d[2])` `:222`) | **PASS** (with PA-HK-F04) |
| **SF-02** | `svc3_create` left `last_emission = 0` with `enabled = true` | FIXED | `FF_TimeGetCuc()` at `:237-238` (create) and `:271-272` (enable) | **PASS** (with PA-HK-F05) |
| **SF-03** | No caller of `FF_PusEmitPeriodicHk()` | DEFERRED | `grep -rn FF_PusEmitPeriodicHk` → declaration `pus.h:83` and definition `pus_services.c:389` only; no caller in `src/` | **DEFERRED — open (ESC-HK-02)** |
| **SF-04** | `time_add_period` arithmetic | FIXED (claim unconfirmed) | Defect **reproduced**, corrected at `:371-386`; arithmetic exact over the full domain (§3.1) | **PASS** |
| **SF-05** | Software-only time source ⇒ ground TM misinterpretation | DEFERRED | No ICD/ground change; comment at `:214-216` retained | **DEFERRED — open (ESC-HK-04)** |
| **SF-06** | ICD §4 does not authorize 6-byte TC[3,1] | DEFERRED | No ICD change; private-format comment retained; **no ECSS compliance claimed** | **DEFERRED — open (ESC-HK-01)** |
| **SF-07** | No hazard analysis for periodic HK | DEFERRED | No hazard artifact; `docs/quality/safety-and-fdir.md` does not cover the periodic HK path | **DEFERRED — open (ESC-HK-03)** |
| **SF-08** | Test evidence gaps (positive path unexercised) | PARTIALLY FIXED | Positive path now exercised with an exact-count oracle at the exact due instant, re-arm, and second period (`test_pus.c:468-486`) | **PASS** (residual gaps → PA-HK-F06) |

### 3.1 Independent confirmation of the SF-04 correction (the reason for this re-review)

Producer contract (`time.c:216-227`): `coarse = (uint32_t)(ns/1e9) + leap`; `fine = floor(frac*65536/1e9)` with `frac < 1e9`, hence **`fine ∈ [0, 65535]` and `fine = 65536` is unconstructible**.

Candidate (`:377-386`): `total_fine = fine + period_s*65536`; `out_coarse = coarse + (total_fine>>16)`; `out_fine = total_fine & 0xFFFF`.
Since `fine < 65536`, `total_fine>>16 == period_s` and `total_fine & 0xFFFF == fine`; the result is therefore exactly `(coarse + period_s) mod 2^32` with `fine` preserved, and the `(uint32_t)` cast loses no bits because `period_s ≤ 2^32-1`.

My harness (Annex A), swept over `coarse ∈ {0, 1, 1000, 0x7FFFFFFF, 0xFFFFFFF0, 0xFFFFFFFE, 0xFFFFFFFF}`, `period_s ∈ {0, 1, 2, 10, 65535, 65536, 0x7FFFFFFF, 0xFFFFFFFE, 0xFFFFFFFF}`, **all 65536 `fine` values**:

```
add sweep: combos=4128768 mismatches_fixed=0 mismatches_prefix=3670016
```

(Fact, high confidence.) The corrected form is exact on every combination including coarse wrap and maximum period; the pre-fix form is wrong on **88.9 %** of the swept domain. This **independently corroborates** the V&V leg's numbers, which were produced by a separately written harness — two independent implementations, identical counts. The pre-fix defect was therefore not a corner case but the normal path for every non-zero period.

**PA position on my own earlier wording.** My delta review recorded SF-04 as FIXED on the strength of source inspection plus the source comment ("Handles fine-field overflow … the carry is the only coarse contribution"). That comment was wrong, and inspection did not catch the double count. I record the correction explicitly: **source inspection plus a plausible comment is not evidence of arithmetic correctness; differential execution against an independent reference is.** That is now the expectation I apply to this class of finding.

---

## 4. Review focus 2 — safety significance of the double-count defect (now fixed)

**Fact — what the defect actually did (measured, Annex A §1-2):** with a 1 Hz caller, base 1000 s, `period_s = 10`:

```
1) pre-fix, 1 Hz caller, P=10: emissions at 1020, 1040, 1060 -> intervals 20 s, 20 s
```

The cadence was not "one emission late once"; because `last_emission` is re-based to `now` after each emission, the **effective period doubled permanently** — half the configured HK sample rate for the whole life of the report. The evidence record's phrase "one full period late for every report" describes the same behaviour and is not misleading; the operational consequence (a **sustained 50 % loss of HK sampling**) should be stated in those terms, because it is a persistent degradation rather than a transient slip.

**Fact — a second, more severe failure mode existed in the same defect (measured):**

```
2) pre-fix, last=1000, P=0x80000000: due=0x000003E8 immediately_due=YES
```

For `period_s = 2^31` (= 2147483648, a value trivially produced by a signed/unsigned conversion or a typo), `2·period_s ≡ 0 (mod 2^32)` and the pre-fix due instant equals `last` **exactly**, at any epoch. The report is then due on **every** call and `last` never advances, i.e. an unbounded TM emission flood from one valid-format command. Because `period_s` is an unvalidated 32-bit field (`:222`, no range check anywhere), this was reachable without any epoch coincidence.

**Safety characterisation (Fact/Inference):**

| Attribute | Assessment |
|---|---|
| Hazard contribution, cadence mode | Persistent 2× period (50 % HK rate loss), non-accumulating; degrades trend resolution and roughly doubles fault-detection latency on every HK-monitored parameter; can also invalidate a ground FDS "housekeeping expected by T" timeout budget |
| Hazard contribution, flood mode | TM stream saturation at call rate; loss of the very HK reports used to observe the platform; risk of starving other TM/TC work depending on the (absent) caller's task budget |
| Criticality | Consistent with a **Major** functional finding in a class-B software context, not Minor |
| Actual exposure in this increment | **Nil in operation**: the function has no caller (SF-03/ESC-HK-02), nothing in the delivered configuration references it, and the code is uncommitted |
| Detection | Not detected by any prior review or by the test suite as it stood; detected only when a discriminating positive-path oracle was added |
| Status now | **Corrected and proven exact** (§3.1) |

**PA conclusion (focus 2): PASS.** The correction is exact, and the episode yields two durable lessons that I require to be carried forward rather than treated as closed history:

1. A test that cannot fail is not evidence. The increment's earlier "PASS" was produced by a negative-only suite; the defect survived it. This vindicates the N22 requirement for nominal **and** out-of-bound cases against explicit expected results, and the V&V reviewer's insistence on a fail-closed exact-count oracle.
2. The defect class is **arithmetic on a 32-bit modular time field without a defined wrap domain**. That class is not exhausted by this fix — see §5.

---

## 5. Review focus 3 — the `time_ge()` wrap finding: classification, significance, deferral

### 5.1 What the code does

`time_ge()` (`:360-366`) compares raw coarse values lexicographically (`coarse1 > coarse2`, then `fine1 >= fine2`). `time_add_period()` is wrap-exact in the *addition*, but after a coarse wrap the due instant is numerically smaller than `now`, so the pair reports "due".

My independent state-machine model (Annex A §2), 1 s call steps:

```
last=0xFFFFFFF0 p=10    due=0xFFFFFFFA  first_due_delta=10  premature_by=0     burst_calls=5
last=0xFFFFFFFF p=10    due=0x00000009  first_due_delta=0   premature_by=10    burst_calls=0
last=0xFFFFFFF6 p=10    due=0x00000000  first_due_delta=0   premature_by=10    burst_calls=9
last=0xFFFFFFF0 p=32    due=0x00000010  first_due_delta=0   premature_by=32    burst_calls=15
last=0xFFFFFF00 p=3600  due=0x00000D10  first_due_delta=0   premature_by=3600  burst_calls=255
```

This independently reproduces V&V-FINAL-001: **premature by up to `period_s` seconds**, followed by an **emit-on-every-call** burst that lasts until the coarse value passes the wrapped `due`, i.e. **≤ `period_s` seconds** for these inputs (measured 5 s for `P=10`, 255 s for `P=3600`).

### 5.2 (b) Significance — the V&V frequency estimate is too optimistic and must be widened

The V&V leg characterised the event as "once per 2^32 s ≈ 136.1 years, only for reports straddling that instant". That is correct **for a bounded period and a fixed epoch**. It is not the whole reachable set, because `period_s` is a **32-bit ground-commandable field with no validation** (`:222`; no bound, no plausibility check, no SRS range). In the addition, `last + period_s` is modular, so the wrap can be **forced at any epoch** by choosing a large enough period. Measured (Annex A §3-4):

```
now=0x000003E8 (1000)        p=0xFFFFFFFF  due=0x000003E7  immediately_due=YES
now=0x000003E8 (1000)        p=0xFFFFFF00  due=0x000002E8  immediately_due=YES
now=0x000003E8 (1000)        p=0xFFFFFC00  due=0xFFFFFFE8  immediately_due=no
now=1000, p=0xFFFFFFFF: burst still due after 3.0e8 simulated seconds (sampling bound)
```

Analytically, with `period_s = 2^32-1` the due instant is `now-1` on **every** call after the first emission, so the state never leaves "due": the burst is not bounded by `period_s` but persists for the remainder of the coarse epoch (measured to a 3·10⁸ s sampling limit; bounded only by the epoch wrap ≈ 136 years). Reachability threshold at `now = 1000 s` is `period_s ≥ 2^32 − 1000`; the threshold widens as the epoch advances (e.g. any `period_s ≥ 2^32 − now`).

Therefore the finding has **two regimes**:

| Regime | Trigger | Consequence | Reachability |
|---|---|---|---|
| R-1 epoch wrap | report straddles the 32-bit coarse wrap | premature by ≤ `period_s`, burst ≤ `period_s`, then recovery with a phase shift | once per 2^32 s per epoch; epoch **undefined** in the controlled baseline |
| R-2 forced wrap | one TC[3,1] with `period_s ≥ 2^32 − now` (e.g. `0xFFFFFFFF`, or a signed→unsigned artifact) | **persistent** emit-on-every-call TM flood for the rest of the epoch | any time; one valid-format command |

### 5.3 (a) Is it a functional defect? — **Yes**

**Fact.** The unit's documented contract (`pus.h:81-83`: "emits … reports whose period has elapsed") is violated for inputs in the defined domain of the function: the comparison decides "due" when the elapsed period is negative/zero, and then re-decides "due" because the state update (`:453-454`) does not escape. This is not a coding-style or performance matter, and it is in the same unit as, and the same class as, the defect just proven by test. I classify it **functional**, not non-functional. Severity: **Major latent** (Minor while unreachable) — it becomes Major on activation.

### 5.4 (c) Is deferral pending a CUC-epoch/time-base authority decision correct? — **Partly. I do not accept the finding as indefinitely deferred.**

The correct part (Recommendation): the following are genuinely outside FSW authority and must be decided by the time-base/requirements authority — what the 32-bit coarse epoch *is*, how ground correlation is established, whether the time source may step backwards (leap-second removal, correlation update), and what the maximum permissible `period_s` and the missed-period/wrap policy are. No FSW engineer may invent these (and N21 forbids inventing policy).

The incorrect part (Fact + Recommendation): a comparison formulation exists that is **both wrap-safe and backward-step-safe and requires no mission input** — a signed modular difference:

```c
int32_t d = (int32_t)(now - last);      /* wrap-safe within ±2^31 s; negative = backward step */
return (d >= 0) && ((uint32_t)d >= period_s);
```

Measured behaviour of that form (Annex A §5): `at_last → 0`, `at_last+10 → 1`, `at_last-1 → 0` (waits, preserving the benign backward-step behaviour of the current raw comparison, measured `4) current raw time_ge on backward step … due_reported=no`), wrap crossing → `1`, huge `period_s` at `last` → `0` (no spurious flood). It needs only the bounded assumption that the interval of interest is < 2^31 s, which is a self-evident property of a housekeeping cadence and not a mission decision; the assumption itself is recordable in the SRS.

Consequently: **the epoch decision and the comparison fix are separable**, and the deferral rationale as written ("cannot be fixed without an authority decision") does not hold for the comparison. Leaving it as "deferred, no owner, no target" also conflicts with N21's requirement that functional bugs be corrected — the only reason it does not block today is that the code is unreachable.

### 5.5 Explicit safety position (as requested)

> **Safety position — `time_ge()` wrap unsafety (PA-HK-F01).**
>
> 1. It is a **genuine functional defect** (Fact, reproduced numerically by two independent harnesses), of the same root-cause class as the double-count defect that this increment just fixed.
> 2. Its operational significance is **currently not credible** in the delivered configuration, because `FF_PusEmitPeriodicHk()` has **no caller** (SF-03 / ESC-HK-02), the file is uncommitted, and no requirement, table or scheduler entry binds it. On that basis it does **not** block acceptance of *this* increment.
> 3. Its operational significance is **credible and potentially severe on activation** (`≤ period_s` premature emission and burst at the epoch wrap; and, independently of any epoch coincidence, a **persistent TM flood** from a single command with an extreme `period_s`, because `period_s` is unvalidated). A TM flood on the housekeeping path degrades the very observability used to detect and respond to platform anomalies, and interacts with the ground TM budget and with any FDS "housekeeping expected" timer. That is a safety-relevant consequence, not merely a comfort issue.
> 4. Therefore the correct disposition is **"acceptable only while inert; a blocker for activation"**: the defect must be corrected, or a formally accepted residual-risk position recorded by the authority, **before the first caller is introduced** (ESC-HK-02). I do not accept an unbounded deferral, and I do not accept "authority decision pending" as the sole basis for leaving the code uncorrected, because the comparison fix requires no authority input (§5.4).
> 5. The wrap/epoch **policy** question (time-base representation, correlation epoch, step/leap behaviour, permissible `period_s`, missed-period policy) remains with the authority and is raised as **ESC-HK-06**; it does not gate the comparison fix, but it does gate any claim that the cadence is *specified* behaviour.

---

## 6. New PA findings

| ID | Finding | Classification | Severity | Owner / target | Disposition |
|---|---|---|---|---|---|
| **PA-HK-F01** | `time_ge()` is not wrap-safe; premature emission + re-emission burst at the coarse wrap; **amplified** to a persistent TM flood by a large `period_s`. Confirms and widens V&V-FINAL-001 (R-1 and R-2 regimes, §5.2). | **Functional** | Major latent / Minor while unreachable | FSW engineer | **OPEN NC.** Correction required **before ESC-HK-02 (any caller)**. A wrap- and step-safe formulation exists and needs no mission input (§5.4). The epoch/wrap *policy* is ESC-HK-06. |
| **PA-HK-F02** | `period_s` is accepted with **no range or plausibility validation** (`:222`; no bound in `svc3_create`), enabling the R-2 flood and any degenerate cadence. | Functional/robustness (input validation) | Major latent | FSW engineer + requirements authority (range) | **OPEN NC** + **ESC-HK-06**. Note the engineering fact: the rejected region is forced by the 32-bit representation; only the numeric maximum needs requirements input. |
| **PA-HK-F03** | **ESC-HK-01..05 exist only in review reports.** `docs/status/escalations.md` (FRICK-ESCAL-001 rev 1.6) contains ESC-1..ESC-10 and **zero** `ESC-HK` entries (`grep -c ESC-HK` = 0). The claim "escalated to the authorized human owner" is therefore not evidenced in the controlled register — the same defect class as NC-EXT-172. | Process/evidence | Major (evidence integrity) | Project/PA | **OPEN NC.** Record ESC-HK-01..06 in FRICK-ESCAL-001 with receiving authority, date raised, question, status, and the NC-EXT-172 evidence limitation. |
| **PA-HK-F04** | Guard is `len < 6u`, so payloads **longer** than 6 bytes are silently accepted (my earlier finding asked `!= 6u`; `svc6_dump` uses `len != 7u`). Not a memory-safety issue (all reads are ≤ `d[5]`), but the fixed-format interface is not strict. | Non-functional (interface strictness/consistency) | Minor | FSW engineer | **OPEN NC** (N21: does not block). |
| **PA-HK-F05** | In `svc3_create` the record is published (`used = true; enabled = true` `:232-233`) **before** `FF_TimeGetCuc()` (`:237-238`). With a scheduler caller in another task, an emitter could observe `enabled` with the previous/zero `last_emission` — a recurrence path for SF-02 and a data race on the report fields (no lock covers `g_hk_reports`). Benign today (no caller, single-threaded host). | Non-functional (concurrency/architecture) → concurrency input to hazard analysis | Minor latent | FSW engineer / architect | **OPEN NC**; must be considered in the ESC-HK-02 integration and ESC-HK-03 hazard analysis. |
| **PA-HK-F06** | Test-adequacy residuals: (i) no case with `period_s` near/above `2^32 − now` or a wrap-straddling base (the exact class that produced the flood); (ii) surviving mutants M3/M5 from the V&V leg (sub-second and ±1 s); (iii) no payload-content oracle on the periodic branch; (iv) **no requirement exists, so the test is not requirement-based** (N22 presupposes a requirement baseline). | Non-functional (test completeness) / requirements gap | Minor | V&V + requirements authority | **OPEN NC**; (iv) is covered by ESC-HK-05. |
| **PA-HK-F07** | Retention/registration: the cycle evidence sits in `docs/vv/evidence/2026-10-24-hk-periodic-ncext17-evidence.md` but `docs/vv/verification-results.md` contains no entry for this increment (AGENTS.md §5 step 4 asks for that record), and the HK reviews are not indexed in `docs/reviews/README.md`. N11 retention itself is satisfied (files present, hash-linked). | Non-functional (documentation/process) | Minor | Documentation reviewer | **OPEN NC** (N21: does not block). |

**No finding in this list is a reproduced functional failure of the *delta under review*.** PA-HK-F01/F02 are pre-existing functional defects in the same unit; under N21 they are corrected now or recorded as NCs for later disposition — they do not stop unrelated development, and they do not invalidate the correction this increment delivered. They are, however, **activation blockers** in the sense of §5.5.4.

---

## 7. Review focus 4 — escalation status

| Escalation | Question for the authority | Status in this candidate | Change since my delta review |
|---|---|---|---|
| **ESC-HK-01** | ICD §4 TC[3,1] format: authorize the 6-byte extension or adopt a compliant format | **OPEN** — no ICD change; private-format comment retained (`:214-216`) | None |
| **ESC-HK-02** | Scheduler/application integration: who calls `FF_PusEmitPeriodicHk()`, from which task, at what cadence, with what concurrency/locking model | **OPEN** — still no caller | None (now also the **gate for PA-HK-F01/F02/F05**) |
| **ESC-HK-03** | Hazard analysis for periodic HK emission | **OPEN** — no artifact | None. **Scope must be widened** (see below) |
| **ESC-HK-04** | Ground-system acceptance / interpretation of the private format and the software-only time source | **OPEN** — no evidence of ground-system position | None |
| **ESC-HK-05** | SRS requirement + traceability for periodic emission | **OPEN** — `grep -ciE 'periodic\|PusEmitPeriodicHk\|time_add_period' docs/requirements/traceability.yaml` = **0** | None |
| **ESC-HK-06** *(new, recommended)* | **Time-base and cadence policy**: definition of the CUC correlation epoch; behaviour and representation at the 32-bit coarse wrap; permitted backward steps/leap handling; maximum permissible `period_s`; missed-period and send-failure policy | **NEW** | Raised by this review (independent amplification analysis, §5.2) |

**ESC-HK-03 scope addition (Recommendation).** The hazard analysis must now cover the hazards demonstrated in this increment, with their measured consequences: `H_SW_CADENCE_HALVED` (effective period 2×, 50 % HK rate loss — measured), `H_SW_TM_FLOOD` (large `period_s` / wrap: emission at every call, epoch-scale duration — measured), `H_SW_PREMATURE_EMIT` (≤ `period_s` early at the wrap), and the concurrency window in PA-HK-F05. These are not speculative: three of them are reproduced numerically.

**Register gap.** ESC-HK-01..06 are not in `docs/status/escalations.md` (PA-HK-F03). Until they are recorded there with a receiving authority and date, the repository cannot support a statement that these questions were escalated to a human authority — only that an AI review wrote them down.

---

## 8. Review focus 5 — authority and claim audit

| Claim to verify | Finding | Verdict |
|---|---|---|
| No ECSS compliance claimed for the private 6-byte TC[3,1] wire format | In-code caveat at `:214-216` ("Do not claim ECSS compliance for this wire format"); evidence record §6; no compliance-matrix or ICD row added (`docs/quality/ecss-compliance-matrix.md` unmodified) | **CONFIRMED — no claim made.** The format remains an authority-pending private extension (ICD §4 = 2 bytes). ECSS-E-ST-70-41C/PUS-C does not authorize it. |
| No mission values introduced | Delta contains no `config/` change; `grep -niE 'periodic\|hk_report\|period_s' config/` → no match; the only time values are the test fixture `FF_TimeSetUtcNs(1000/1010/1020/5000 s)` through the public API, labelled "Synthetic test value, not a mission time parameter" (`test_pus.c:425-426`). `test_hk_periodic_emission` is the **last** entry of `FF_TEST_MAIN` (`:570`), so the fixture does not leak to other cases | **CONFIRMED** |
| No hardware binding | No `hal/`, `drivers/`, `bsp/`, `board/`, `config/` hunk in the delta; no register/MSS/watchdog content | **CONFIRMED** |
| No emulator change | No `tools/docker/`, `tools/scripts/renode*`, `board/` change in the delta; Renode not run | **CONFIRMED** |
| No NC closed | `docs/quality/` **unmodified** in the working tree (`git status --porcelain docs/quality/` empty); `docs/status/loop-audit.md:200,205` still records NC-EXT-17 **OPEN**; `docs/vv/verification-results.md:1122` still records NC-EXT-17 among OPEN | **CONFIRMED** |
| No commit/push | Candidate files are working-tree modifications against `HEAD` 44d2af2 | **CONFIRMED** |
| N18 Renode FAIL retained | NC-EXT-198 unchanged; Renode not re-run per directive | **CONFIRMED** |
| NC-PAL-01 OPEN retained | Unchanged; not touched | **CONFIRMED** |

---

## 9. Gates — independently re-executed by this review

| Gate | Command | Result | Observed |
|---|---|---|---|
| Build + unit/integration | `make test` | **PASS** | `100% tests passed, 0 tests failed out of 46` (23.9 s) |
| `test_pus` (N22-labelled) | `./build/native/tests/test_pus` | **PASS** | `checks=108 failures=0` |
| `test_pus_fu` | `./build/native/tests/test_pus_fu` | **PASS** | `checks=159 failures=0` |
| `test_r2_gap` | `./build/native/tests/test_r2_gap` | **PASS** | `checks=615 failures=0` |
| `test_r3_gap` | `./build/native/tests/test_r3_gap` | **PASS** | `checks=201 failures=0` |
| Traceability (N4) | `make trace` | **PASS** | `traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Lean (N1) | `make verify-lean` | **NOT RUN here** | no `lake` on host; V&V re-executed it in the pinned `frick-vv:local` image (PASS). Corroboration accepted, not independently reproduced by me. |
| Format check | `make format-check` | **NOT RUN here** | host clang-format major mismatch (23 vs pinned 18) — the host guard correctly refuses; V&V PASS in the pinned image |
| N18 Renode | — | **FAIL — unchanged (NC-EXT-198)** | not run, per constraint |
| RTEMS / coverage / MISRA | — | **NOT RUN** | out of delta scope |

My per-binary check counts match the V&V leg exactly, and `make test`/`make trace` pass. The evidence record's gate table is corroborated for the gates I could run.

**Gate honesty note (Fact).** The trace PASS is a *structural* result: it reports 120 requirements, none of which is the periodic HK feature (ESC-HK-05, count 0). Traceability cannot pass or fail for a feature that has no requirement — it is silent about it. Likewise, `test_pus` passing is not evidence of requirement coverage for this feature (PA-HK-F06 iv).

---

## 10. Residual risks and uncertainty

| # | Residual risk | Basis | Bound / mitigation |
|---|---|---|---|
| R1 | **`time_ge` wrap unsafety incl. forced-wrap TM flood** (PA-HK-F01/F02) | reproduced numerically, two harnesses | Bounded by "no caller"; becomes epoch-scale TM flood on activation. Requires correction or accepted risk **before ESC-HK-02** |
| R2 | **Unvalidated `period_s`** | `:222`, no bound | Same as R1; requirements range needed (ESC-HK-06) |
| R3 | **No target evidence whatsoever** (N18 FAIL, NC-EXT-198) | all evidence host-native | No claim about WCET, jitter, concurrency, on-target cadence, or driver/bus behaviour |
| R4 | **Feature not exercisable** (no caller) | SF-03 | Component-level evidence only; no integration evidence exists |
| R5 | **No requirement / hazard basis** | ESC-HK-05 / ESC-HK-03 | Cadence, wrap, missed-period, send-failure and provider-failure policies in this path are **undocumented behaviours**, including the "first error wins unless a report succeeded" return rule |
| R6 | **Concurrency window in create/enable** | PA-HK-F05 | Benign while single-threaded and uncalled; must be addressed with the caller |
| R7 | **Sub-second / ±1 s sensitivity of the fixture** | V&V mutants M3/M5 | ≤ 1 s, non-accumulating |
| R8 | **Leap-second step (±1 s) uncovered** | `g_leap_seconds` in `time.c:216-220` | Bounded one-off; part of ESC-HK-06 policy |
| R9 | **Tool-input reproducibility** (container-fetched Lean toolchain, mutable base images) | V&V §10 R9 | Already-open NC-EXT-35 class; not evidence of reproducibility |

**Explicit non-claim.** Passing gates and passing tests establish what this candidate does under the recorded host conditions and oracles. They do **not** establish absence of defects, operational fitness, on-target behaviour, requirement compliance, or certification.

---

## 11. Recommendation

### ACCEPT — of this increment, for the narrow claim it supports

**Accepted claim (narrow, Fact-backed):**

1. The SF-04 arithmetic defect is **genuinely corrected** in this exact candidate: `time_add_period()` computes exactly `(coarse + period_s) mod 2^32` with `fine` preserved, verified against an independently written exact reference over 4,128,768 input combinations (0 mismatches; pre-fix 3,670,016 mismatches). This also confirms, retrospectively and with correction to my own earlier wording, that SF-04 was **not** fixed in the set I reviewed previously.
2. SF-01 (guard before payload access) and SF-02 (`last_emission` initialised to `now` in both create and enable) are **genuinely fixed**.
3. SF-08 is now **closed as an evidence gap**: the positive time-due path is exercised with a fail-closed exact-count oracle at the exact due instant, on the re-arm, and on the second period, and the test demonstrably fails on the pre-fix code.
4. The increment introduces **no** mission value, **no** hardware binding, **no** emulator change, makes **no** ECSS compliance claim for the private wire format, closes **no** NC, and performs **no** commit/push.

**I therefore ACCEPT the correction and the added evidence, and REJECT any wider reading** — this is not an acceptance of the HK periodic feature, not a statement that the periodic path is safe or specified, not target evidence, and not a release input.

**Conditions attached (increment conditions, not release conditions):**

| # | Condition |
|---|---|
| C1 | Register **ESC-HK-01..05** in `docs/status/escalations.md` with receiving authority, date, question and status, and add **ESC-HK-06** (time-base/wrap/cadence policy). Until then no repository statement may claim these were escalated to an authority (PA-HK-F03). |
| C2 | Raise **PA-HK-F01** as an open **functional** NC owned by the FSW engineer with a hard target of *before the first caller is introduced* (ESC-HK-02). Record explicitly that the deferral covers the **wrap policy**, not the comparison formulation, which requires no mission input. |
| C3 | Raise **PA-HK-F02** (unvalidated `period_s`) as an open NC; the requirements authority must supply a permissible range (ESC-HK-06) and the implementation must enforce it. |
| C4 | Widen **ESC-HK-03** hazard-analysis scope with the measured hazards `H_SW_CADENCE_HALVED`, `H_SW_TM_FLOOD`, `H_SW_PREMATURE_EMIT`, plus the PA-HK-F05 concurrency window. |
| C5 | Record PA-HK-F04/F05/F06/F07 as open NCs under N21 (non-functional; they do not stop development and do not block this increment). |
| C6 | Add to the next test cycle: a `period_s` near `2^32 − now` and a wrap-straddling base (the class that produced the flood), a `period − 1 s` negative check, a sub-second fixture, a payload-content oracle, and one call well after `due`. |
| C7 | Retain **N18 FAIL (NC-EXT-198)** and **NC-PAL-01 OPEN** in the status record; this increment gains no target evidence. |
| C8 | Retain the no-ECSS-compliance statement for the private 6-byte `TC[3,1]` wire format until ESC-HK-01/04 are decided. |

**Explicitly not done by this review:** no NC closed, no waiver, no risk acceptance, no release/qualification/flight-readiness statement, no commit/push authorization.

### Reviewer independence statement

This review was performed from the six declared digests, with my own analytical method (derivation from the CUC producer contract plus two harnesses written for this review, not a re-read of the implementation), my own modelling of the comparison state machine, and gate re-execution on the host. It shares no authoring role on the reviewed files.

Transparency on method order: the V&V leg's report was an input to this review and I read it before performing my analysis. I therefore do **not** claim blindness to its conclusions. Instead, independence is demonstrated by divergence where it matters: my arithmetic harness was written independently and produced the same counts (4,128,768 / 3,670,016) — two implementations agreeing exactly; and my wrap modelling **contradicts and widens** the V&V leg's "once per 136 years" characterisation by identifying the forced-wrap regime (§5.2), which I would not have produced by deferring to that report. Two of its statements are also corrected above (its "wrap handled" inheritance, and its period-bounded burst duration). Dimensions **not** established: separate reporting line and freedom from schedule pressure are organisational facts outside my visibility; I am an AI analysis role with no human credentials, no regulatory delegation, and no authority to approve, waive or accept risk.

---

## 12. Confidence and limits (mandatory block)

```text
Decision/product and configuration:
  frick-fsw HK periodic emission (NC-EXT-17), six declared files at the SHA-256
  listed in §1, working tree uncommitted against HEAD 44d2af2.
Claim type: Fact | Inference | Recommendation | Open issue  (labelled per item)
Applicable baseline and approved tailoring:
  No approved requirement exists for this feature (ESC-HK-05). ECSS-E-ST-40C
  Rev.1 / Q-ST-80C Rev.2 are invoked by the project, but this increment is
  assessed against the unit contract, the project norms N1-N22, and the
  ICP/ICD (which does NOT define this payload). No tailoring decision is
  inferred by me.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  Candidate source/tests at the six digests (2026-10-24); src/core/time/time.c
  (CUC producer contract); docs/vv/evidence/2026-10-24-hk-periodic-ncext17-
  evidence.md; docs/reviews/2026-10-24-hk-periodic-{vv-review,safety-review,
  vv-delta-review,safety-delta-review,vv-final-delta-review}.md; docs/status/
  escalations.md rev 1.6; docs/status/loop-audit.md; docs/vv/verification-
  results.md; docs/quality/* (unmodified). Host gate runs executed by me.
Cross-check and reconciliation:
  Arithmetic: my harness vs the V&V harness — identical counts (0 / 3,670,016
  of 4,128,768). Wrap analysis: my state-machine model vs V&V numeric cases —
  identical premature/burst values, EXTENDED by the forced-wrap regime, which
  supersedes the "once per 136 years" phrasing for uncapped period_s.
  Correction recorded: my earlier "SF-04 FIXED" and V&V's earlier "wrap
  handled" statements were both too broad; both corrected explicitly.
Confidence: High  for SF-01/02/04/08 verdicts, for the rejection of any wider
  claim, and for the arithmetic proof.  Medium  for the hazard significance of
  the wrap defect, because no hazard analysis exists and the caller is absent.
Basis and detection limits:
  Host-native, single-threaded, synthetic time fixture; no target timing, no
  concurrency, no bus/link evidence; no requirement-based oracle exists; the
  test fixture cannot see sub-second or ±1 s errors. Sampling-based review of
  the six files only. "No findings beyond those listed" is NOT claimed.
Missing/inaccessible evidence (including paywalls):
  Renode/target evidence (N18 FAIL, NC-EXT-198); scheduler integration (absent);
  SRS requirement and hazard analysis (absent); ground-system position on the
  private format (absent); ICD control-board decision (absent); Lean gate not
  re-run by me (re-executed in the pinned container by the V&V leg).
Assumptions and expiry/recheck date:
  Assumes the six digests remain the review object (any byte change voids this
  report); assumes the CUC producer contract in time.c is current; assumes the
  caller remains absent. Recheck on: any change to pus_services.c, on
  introduction of a caller, on any ICD/epoch decision, or 2026-11-24.
Consequences if wrong:
  If the wrap analysis is wrong in the benign direction, the residual is a
  bounded ≤ period_s cadence error (minor). If wrong in the severe direction,
  a TM flood could go to activation without correction: loss of housekeeping
  observability and TM budget exhaustion on a class-B platform.
Required independent human reviewer/authority:
  Authorised human owner (accept PA-HK-F01/F02 risk or direct the fix and its
  due date), System Safety Engineer (ESC-HK-03 and the hazards above),
  Requirements/Systems authority (ESC-HK-05, ESC-HK-06 period range),
  ICD Control Board + Ground System Engineer (ESC-HK-01/04),
  FSW architect (ESC-HK-02 integration and locking model).
```

---

## 13. Evidence retention (N11)

Retained in the working tree by this review:

- This report: `docs/reviews/2026-10-24-hk-periodic-pa-final-delta-review.md`
- Candidate identity: §1 (digests recomputed here)
- Independent arithmetic/comparison harness: `/tmp/pa_hk_time/pa_check.c`, `/tmp/pa_hk_time/pa_check2.c` (ephemeral, outside the repository; reproduced in Annex A so every number in §3.1/§4/§5 is re-runnable)
- Gate outputs: `make test` (46/46), per-binary counts (108/159/615/201), `make trace` (PASS 120/76/30/14/68)

No other repository file was created or modified by this review. No NC was closed; no manifest was refreshed; no commit or push was made.

---

## 14. Revision history

| Rev | Date | Author | Change |
|---|---|---|---|
| 0.1 | 2026-10-24 | Senior SW PA / Safety Engineer (independent) | Final N20 delta review of the HK periodic increment: verified the six candidate digests, confirmed SF-01/SF-02/SF-04/SF-08, quantified the double-count defect (2× period cadence; `period_s = 2^31` flood mode), adjudicated the `time_ge` wrap finding (functional; deferral of the policy correct, of the whole finding not), raised PA-HK-F01..F07 and ESC-HK-06, and recorded ACCEPT-narrow with conditions C1..C8. |

---

## Annex A — independent harnesses (re-runnable)

A.1 `pa_check.c` — reference vs candidate vs pre-fix addition, then the comparison state machine.

```c
/* Reference: exact total in 1/65536 s, mod 2^32 s. */
static void ref_add(uint32_t c, uint16_t f, uint32_t p, uint32_t *oc, uint16_t *of)
{
    uint64_t t = (uint64_t)c * 65536ULL + (uint64_t)f + (uint64_t)p * 65536ULL;
    *oc = (uint32_t)((t >> 16) & 0xFFFFFFFFULL);
    *of = (uint16_t)(t & 0xFFFFu);
}
static void cand_add(uint32_t c, uint16_t f, uint32_t p, uint32_t *oc, uint16_t *of) /* candidate */
{
    uint64_t total_fine = (uint64_t)f + ((uint64_t)p * 65536ULL);
    *oc = c + (uint32_t)(total_fine >> 16);
    *of = (uint16_t)(total_fine & 0xFFFFu);
}
static void old_add(uint32_t c, uint16_t f, uint32_t p, uint32_t *oc, uint16_t *of)  /* pre-fix */
{
    uint64_t total_fine = (uint64_t)f + ((uint64_t)p * 65536ULL);
    *oc = c + p + (uint32_t)(total_fine >> 16);
    *of = (uint16_t)(total_fine & 0xFFFFu);
}
static bool time_ge(uint32_t c1, uint16_t f1, uint32_t c2, uint16_t f2)  /* UUT comparison */
{
    if (c1 != c2) return c1 > c2;
    return f1 >= f2;
}
/* Sweep: coarse in {0,1,1000,0x7FFFFFFF,0xFFFFFFF0,0xFFFFFFFE,0xFFFFFFFF},
 *        period in {0,1,2,10,65535,65536,0x7FFFFFFF,0xFFFFFFFE,0xFFFFFFFF},
 *        fine in [0,65535]; then scan forward from `last` at 1 s steps for the
 *        first instant time_ge() reports due, and continue scanning to measure
 *        the emit-every-call burst with last := now after each emission. */
```

Observed: `combos=4128768 mismatches_fixed=0 mismatches_prefix=3670016`; burst observations as quoted in §5.1; forced-wrap observations as quoted in §5.2.

A.2 `pa_check2.c` — pre-fix cadence, pre-fix large-period flood, and the decision-free comparison.

```c
static void add_prefix(uint32_t c, uint32_t p, uint32_t *oc) { *oc = c + p + p; }
static bool safe_signed(uint32_t now, uint32_t last, uint32_t p)
{
    int32_t d = (int32_t)(now - last);   /* wrap-safe within +-2^31 s; negative = backstep */
    return (d >= 0) && ((uint32_t)d >= p);
}
```

Observed: `pre-fix, 1 Hz caller, P=10: emissions at 1020, 1040, 1060 -> intervals 20 s, 20 s`; `pre-fix, last=1000, P=0x80000000: due=0x000003E8 immediately_due=YES`; `safe_signed: at_last=0 at_last+10=1 at_last-1=0 wrap_ok=1 huge_p=0 backstep=0`.

Both compiled with `gcc -std=c11 -Wall -Wextra -Werror -O2`.

---

**End of PA Final Delta Review.**

*This report records an objective-evidence assessment of the identified configuration only. It does not certify compliance, approve a safety case, accept residual risk, close a nonconformance, qualify a tool, authorise release, or declare flightworthiness. Independent qualified humans retain those decisions.*
