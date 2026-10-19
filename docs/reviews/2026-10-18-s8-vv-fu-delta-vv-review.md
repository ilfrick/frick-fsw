# Independent Review FRICK-REVIEW-0379 (R2) + FRICK-REVIEW-0380 (R3) — S8-VV-FU delta: ES/HS host-integration oracle and bounded decoder corpus

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0379 (R2 core); FRICK-REVIEW-0380 (R3 requirements/interface annex) |
| Reviewer session                 | (independent agent, norm N9)           |
| Reviewer profile                 | R2 (SW V&V), with an R3 (requirements/interface) annex |
| Reference revision               | pushed HEAD `e7e48ee` on `main` (branch `main`); reviewed artifacts unmodified in the working tree |
| Items under review               | `tests/unit/test_es_hs_lifecycle.c` (sha256 `060256cd8b05bbf1cea98cdaea579f931437a152bd7708088fd33aaa58313770`); `tests/unit/test_decoder_corpus.c` (sha256 `fba54865ce562ac885adfeb5c762262dc6d0e3d682db204e58de2c93aa4f8187`); `docs/vv/test-spec/ts-5-core-services.md` rev 1.3 (sha256 `b95d5730730b1e2bbe5bec3163354d07823d075160bfe9199e516b4dffcd9941`, rows 49/50/51/76, §1.2/§1.3, §3.2); `docs/vv/test-spec/ts-6-sw-robustness.md` rev 1.5 (sha256 `d3e897cd5def7ed3e3fbf28003de8117c394edcf8c1382c461fe1fcf2626258f`, rows 35/41/55/56/57/58, §3.2); `tests/CMakeLists.txt` registrations; `docs/status/loop-audit.md` lines 63/188/189; `docs/vv/verification-results.md` S8-VV-FU section; `docs/planning/sprint-plan.md` rows 98/160/161 and history 1.68–1.70 |
| Date                             | 2026-10-18                             |
| Scope                            | Bounded R2+R3 delta review of the S8-VV-FU increment, answering six mandated questions: (1) oracle independence / non-tautology including the ES/HS heartbeat-event-id coupling; (2) reset and concurrency coverage; (3) reproducibility, bounds, time budget and anomaly retention; (4) N22 nominal + out-of-bound cases against explicit expected results; (5) honesty of the claims (no MC/DC, qualification, target, or closed `GAP-R3-1`/`NC-EXT-25`); (6) independent recomputation of the TS-5/TS-6 §3.2 counts. Against ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C, CCSDS 133.0-B-2, SRS/SSS and the project norms. |
| Independence declaration          | The reviewer had no part in authoring, editing, executing, specifying or committing any item under review, and entered the task with fresh context. Every judgement was derived from repository artifacts read directly in this session; the arithmetic was re-derived with an independent parser and by hand; the two reviewed tests and their mutants were re-run locally by the reviewer outside the repository. No subject-matter author reviewed or approved this report. |
| Norms applied                     | N1–N22 (see `docs/rules.md`)           |
| Publisher ID note (added 2026-10-19, FSW engineer — metadata only) | This report was first drafted as `FRICK-REVIEW-0378` (R2 core) / `FRICK-REVIEW-0379` (R3 annex). Those IDs collided with the concurrently produced R4 documentation-confirmation report `FRICK-REVIEW-0378` (`docs/reviews/2026-10-18-docs-alignment-r4-confirmation.md`), which had taken the first free sequential ID after `0377`. Before publication the R2 core was renumbered to **`FRICK-REVIEW-0379`** and the R3 annex to **`FRICK-REVIEW-0380`**, and this report's findings `F-0378-nn` to `F-0379-nn`, so that no review ID and no finding ID is carried by two reports. No verdict, finding, evidence citation or arithmetic in this report was changed by the renumbering. |

## 1. Scope

**In scope (reviewed).**

1. The two S8-VV-FU test artifacts (ES/HS lifecycle harness; bounded decoder corpus) — oracle grounding and non-tautology, reset/concurrency coverage, reproducibility/bounds/anomaly retention, and nominal + out-of-bound coverage against explicit expected results.
2. The TS-5 rev 1.3 and TS-6 rev 1.5 rows that cite those artifacts, their §1.2/§1.3 oracle-ground discipline, and their §3.2 coverage summaries (recomputed independently).
3. The status wording for `GAP-R3-1` / `NC-EXT-25` across `docs/vv/verification-results.md`, `docs/status/loop-audit.md` and `docs/planning/sprint-plan.md`, checked only for honesty (over/under-statement), not for disposition.
4. Traceability: `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` re-run by the reviewer.

- **Excluded / not established:** this is a bounded host-only delta review. It does **not** qualify any test, tool or process (N10), does **not** create N11-controlled run records, does **not** provide target/HIL, WCET, jitter, RTEMS-timing, radiation or integration evidence, does **not** claim MC/DC or structural coverage, and does **not** determine phase, release or flight readiness. It opens no NC and closes no NC; `GAP-A3`/`GAP-A11` (organizational/model independence) remain open and are not the reviewer's to change.
- **Assumption recorded:** the delta baseline is pushed HEAD `e7e48ee`; the two test files and the two spec documents are committed at that revision and were read there (the only uncommitted working-tree change, `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md`, was **not** touched or reviewed).
- **Non-blocking sweep:** `docs/vv/verification-results.md` line 423 was already flagged as R4/`NC-EXT-96` territory; it is raised here only as a Comment (F-0379-07), not as an R2/V&V requirement defect.

## 2. References

- Project norms: `docs/rules.md` (N1–N22); entry point `AGENTS.md`; `docs/vv/FRICK-VV-TSPEC-001` conventions cited by the specs.
- Controlled artifacts read: the two test files; `docs/vv/test-spec/ts-5-core-services.md`; `docs/vv/test-spec/ts-6-sw-robustness.md`; `tests/CMakeLists.txt`; `docs/requirements/traceability.yaml` + `docs/requirements/traceability.md`; `docs/status/loop-audit.md`; `docs/vv/verification-results.md`; `docs/planning/sprint-plan.md`; `src/apps/app_hs.c`; `src/core/es/es.c`; `src/core/pal/pal_posix.c`; `src/transport/ccsds/ccsds.c` + `ccsds.h`; `src/pus/pus.c`.
- Prior reviews whose findings frame this delta: R2 `2026-10-17-s8-vv-pus-delta-0370.md` (F-3/F-4/F-10 on oracle independence wording), R4 `2026-10-17-s8-current-docs-r4-0372.md`, PA `2026-10-17-s8-reconciliation-pa-0373.md`, R2 `2026-10-17-s8-ts5-77-oracle-r2-0374.md` (the shape template and the F-0374-01/02/03 follow-ups). Per the 0374 precedent, this review does not re-open PA 0373 F-0373-01/04.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Method and independent recomputation

Five methods were combined: (i) direct reading of code and test together; (ii) an independent parser over each spec §2 case table (scope-restricted to the `## 2. Test cases` … next `## ` block, `(?<!\|)\|` cell splitting) to recompute every count; (iii) mutation experiments compiled in `/tmp/opencode/` against the repository's real `libfrick_fsw_core.a` and driven by the reviewed test targets, to test whether the oracle is discriminating or vacuous; (iv) a stability re-run of both tests; (v) a local run of the reviewer's ASan binaries. No in-repository file was modified; all mutant/derived artifacts live under `/tmp/opencode/`.

### 3.1 Recomputed case-table counts (independent of the documents' own summaries)

| Metric | TS-5 rev 1.3 (recomputed) | TS-5 §3.2 (doc) | TS-6 rev 1.5 (recomputed) | TS-6 §3.2 (doc) |
| --- | --- | --- | --- | --- |
| Total cases | 77 | 77 ✓ | 58 | 58 ✓ |
| Nominal | 47 | 47 ✓ | 15 | 15 ✓ |
| Out-of-bound | 27 | 27 ✓ | 43 (41 `OOB` + 1 `OOB (exploratory…)` + 1 mixed counted under OOB) | 43 ✓ |
| Mixed nominal + OOB | 3 | 3 ✓ | 1 (no separate `Mixed` line in doc; disclosed in a §3.2 note) | (folded into OOB) ✓ |
| Individual | 67 | 67 ✓ | 47 | 47 ✓ |
| Combined | 10 | 10 ✓ | 11 | 11 ✓ |
| `covered` | 56 | 56 ✓ | 30 | 30 ✓ |
| `gap` | 21 | 21 ✓ | 27 | 27 ✓ |
| Exploratory | 0 | (n/a) | 1 | 1 ✓ |
| SIL / PIL / HIL | 74 / 1 / 2 (sum 77) | 74 / 1 / 2 ✓ | 53 / 1 / 4 (sum 58) | 53 / 1 / 4 ✓ |

Every recomputed count agrees with both documents. The TS-6 §3.2 environment row that read `50 / 1 / 4` in the revision reviewed by 0374 (F-0374-02) is now **53 / 1 / 4** and sums to its own `Total cases 58`; the TS-5 §3.2 environment row sums to 77. `docs/vv/verification-results.md` line 423, however, still states the older "76 total / 55 covered / 21 gap" (F-0379-07, Comment).

### 3.2 Traceability and case-ID uniqueness

`python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → `PASS (117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked)`, matching `docs/vv/verification-results.md`. TS-5 §2 states the range `N22-TS5-1 … N22-TS5-77` is unique; recomputation of the parsed table finds 77 distinct IDs and no collision. TS-6 §2 states `N22-TS6-1 … N22-TS6-58`; recomputation finds 58 distinct IDs.

### 3.3 Local execution evidence (reviewer-host, not N11-controlled)

| Target | Result | Stability |
| --- | --- | --- |
| `test_es_hs_lifecycle` | `checks=52 failures=0` | identical over 20/20 consecutive runs |
| `test_decoder_corpus` | `checks=14926 failures=0` | identical over 10/10 consecutive runs |
| ASan binaries (`build/asan/tests/`) | `52 / 14926` checks, `0` failures | — |

## 4. Scope item (1) — oracle independence and non-tautology

### 4.1 Decoder corpus (`test_decoder_corpus.c`)

- **Requirement-grounded, non-tautological oracles.** The four assertions families map to named clauses: SRS-REQ-003 (exact packet boundary, minimum PDL), SRS-REQ-004 (single-bit mutation is never accepted), SRS-REQ-005 (malformed frame not executed; unknown service not executed). Expected results are stated as outcomes (`FF_PARSE_ERROR`/`FF_CRC_ERROR`/`FF_SUCCESS`, `FF_PusGetTcCount` unchanged) rather than the implementation's internal steps, and the corpus does not re-implement the decoder to build its expected values.
- **Disclosed shared-implementation coupling.** The CRC *verdict consistency* check recomputes with `FF_CcsdsCrc16` — the same routine the decoder uses — and the suite discloses this in-line ("Internal consistency check for the CRC property … recompute" at lines 140–145; "The check shares `FF_CcsdsCrc16` with the decoder" at line 318; "Verdict consistency only: this check shares the CRC" at line 348), as do TS-6 rows 35 and 56 ("not an independent oracle … external/reference-oracle independence remains open"). The *primary* claim ("no single-bit mutation is accepted") does not depend on the shared routine: mutation experiments show strong discrimination (§4.4). This is honest coupling, correctly scoped — **not** a tautology.
- **ES/HS heartbeat-event-id coupling (mandated).** In `test_es_hs_lifecycle.c` the harness captures the EVS listener seam and asserts on `0x0502u` and on the diagnostic text `"app %u failed"` (lines 96, 185–186, 188, 302, 310). Those two tokens are literals of `src/apps/app_hs.c` (`src/apps/app_hs.c` line 55 `FF_EvsSend(0x0502u, …)`, line 49 `"HS: app %u failed"`), and the harness header (lines 20–28) discloses this in full: the event-identification oracle "is coupled to the implementation under test: it can confirm the event stream is unchanged relative to that implementation, but it cannot independently detect a defect in the shared literal itself." The coupling is real, is disclosed honestly at the harness, and **is not repeated in the TS-5 rows 49/51/76 or TS-6 row 55 that cite the harness as an "independent EVS-listener oracle"** (F-0379-02). The structural-vs-literal split is correct: the *listener seam* is independent (it is the production observation point), the *identifier* is not.

### 4.2 ES/HS lifecycle harness (`test_es_hs_lifecycle.c`)

- Requirement grounding for the FAILED-detection oracle is sound: `FF_EsGetState(bad_id) == FF_APP_FAILED` (line 288) and `cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u` (line 299/308) are checked against SRS-REQ-014 ("detect failed applications")/SSS-REQ-008, while `!cap_hs_reports(ok_id)` (line 301) is the no-false-positive half of the same clause.
- The harness deliberately labels its two *ungrounded* observations — the task-return state transition (lines 234–240) and the invalid-id/saturation return contract — as observations, not oracles, and cites TS-6 `N22-TS6-41`/`N22-TS6-36` and NC-EXT-47 §7.2. That is correct N22 gap discipline.

### 4.3 Non-tautology verdict

Neither suite asserts "the code does what the code does". Each expected result is either a requirement clause outcome or an externally-derivable property, with the shared-implementation couplings disclosed rather than hidden. The residual limit is that neither suite can detect a defect shared between the oracle and the implementation (`FF_CcsdsCrc16` for CRC; `app_hs.c` literals for the heartbeat/text). That limit is stated by the artifacts and is carried forward as open in the NC/status records, not papered over.

### 4.4 Discrimination evidence (mutation)

Compiled outside the repository against the real core library; the reviewed test target was the runner.

| Mutant | Change | Observed failures | Reading |
| --- | --- | --- | --- |
| m1 | CRC check removed from the decoder | 3764 check failures | Corpus strongly detects loss of CRC rejection |
| m2 | exact-boundary length check removed | failures at the truncation/extension sub-cases | Length-boundary oracle discriminates |
| **m3** | **minimum-PDL check removed** | **0 failures** | **The corpus sub-case that claims to exercise minimum PDL is masked — see F-0379-01** |
| a1 | heartbeat emission removed from `FF_AppHsStep` | 4 failures | Heartbeat nominal oracle discriminates |
| a2 | heartbeat id `0x0502`→`0x0503` | 4 failures | Confirms the id is asserted (and is a code literal → coupling) |
| a3 | FAILED-detection loop removed | 4 failures | FAILED-report oracle discriminates |
| a4 | diagnostic text "HS: app %u failed"→"HS: application %u failed" | 2 failures | Text is asserted (and is a code literal → coupling) |
| seed | corpus seed changed to `0x1234ABCD` | 14919 checks, 0 failures | Property holds across the input generator, not one frozen vector |

## 5. Scope item (2) — reset and concurrency coverage

### 5.1 Reset path — **not covered by any executable test**

TS-5 `N22-TS5-50` (line 175) is the commandable software reset (PUS S8 function #1 → `ff_hs_fn_reset` → `FF_PalPanic`) and its expected-result cell states *"The harness must stub/intercept `FF_PalPanic` so the reset action is observable without ending the run"*, with status `covered`. Under the TS-5 §1.2 convention, `covered` means an oracle is grounded in a controlled clause and executable in the named environment — *the test may still have to be added*. Independent search of the repository found:

- no stub, interceptor, `#define`, `--wrap` or link substitution for `FF_PalPanic` anywhere under `tests/` or `src/` (on the host path `FF_PalPanic` terminates the process — `src/core/pal/pal_posix.c` line 501ff ends in `abort()` — which is exactly why `N22-TS5-50` requires interception before the reset action can be observed without ending the run);
- no test that registers or invokes PUS Service 8 function 1 (`src/apps/app_hs.c` line 61 registers `ff_hs_fn_reset`; grep over `tests/` + `src/pus/` finds no invocation of it);
- the only site that reaches `FF_PalPanic` for the HS reset action is `ff_hs_fn_reset` itself, which no test calls (the other callers are the `src/main/fsw_main.c` startup-failure paths, which are not a ctest target).

Therefore the reset action is **specified but unexecuted**, and the sprint-plan exit gate for row 98 explicitly names "R3/R2 independently verify … **reset/concurrency coverage**". This review confirms the gap rather than a defect: `N22-TS5-50` is not over-claimed (no `passing` claim is made), but the named coverage does not exist (F-0379-03).

### 5.2 Concurrency — exercised structurally, never asserted discriminatingly

The harness does create real tasks through the real POSIX PAL (`FF_PalTaskCreate` → `pthread_create` in `src/core/pal/pal_posix.c`), so the code under test **runs concurrently** by construction. However, no assertion depends on a concurrent interleaving:

- The only concurrency-sensitive constructs are the resource-exhaustion fixture `hs_fill_pal_slots` (lines 123–135) and the assertion `FF_TEST_ASSERT(fillers < 64u)` (line 280) — a resource bound, not a race/order/atomicity property.
- The harness synchronises the *tests* by sleeping, not by joining: `FF_PalTaskDelayMs(20u) /* let previous-test tasks terminate before re-init */` at lines 206, 239 and 261. Because the POSIX PAL never frees a slot when a task entry returns, previously created tasks remain live across `hs_init_modules()`. The 20 ms sleep is a wall-clock assumption used to avoid cross-test interference — a timing-based (not deterministic) hand-off.
- No assertion is made on shared-state consistency, memory-ordering, mutex/lock behaviour, or interleaving under load; the TS-6 concurrency group (rows `N22-TS6-25…36`) is separate and was not part of this delta.

This is consistent with the sprint row's own framing (host integration, not target concurrency), but the "concurrency coverage" named in the exit gate is at best *incidental*. Recorded as part of F-0379-03.

## 6. Scope item (3) — reproducibility, bounds, anomaly retention

- **Determinism/reproducibility: good.** `CORPUS_SEED 0x5EEDF00D`, fixed iteration counts (`CORPUS_RANDOM_ITERS 2048`, `CORPUS_ROUNDTRIP_ITERS 512`), fixed buffer bound (`CORPUS_MAX_LEN 160`), no dynamic allocation, no wall-clock dependency. Re-running both targets gives byte-identical result lines (20/20 and 10/10). Changing the seed changes the input set but still passes, so the property is general, not vector-frozen.
- **Bounds: good.** Every loop is statically bounded; the PAL slot fill is bounded by `<= 64` with the exhaustion reached flagged (`fillers < 64u`).
- **Anomaly retention: partly incomplete.** Failures print `CORPUS ANOMALY seed=0x… iter=…` so an input can be regenerated from (seed, iteration). Two weaknesses (F-0379-04, Comment):
  - the *template index* is not printed, so for a template-specific anomaly the failing frame is not directly identifiable (only the iteration within the sub-test);
  - `g_iter` is stale in two sub-cases: the extension loop iterates on `extra` (lines 255–271) without setting `g_iter`, and the undersized-PDL block (lines 275–296) never sets it, so those anomalies report the previous sub-test's iteration value — the printed context can be wrong for exactly the input it is meant to identify.
  - The ES/HS harness has **no** `anomaly()` context at all (its failures are plain `FF_TEST_ASSERT`), so a failing assertion in `test_es_hs_lifecycle` does not emit a machine-readable reproduction key.
- **Time budget: not enforced (F-0379-05, Comment).** Both targets are bounded by *iteration/length constants*, which implies a bounded (and observed ~sub-second) runtime, but there is no wall-clock or per-test timeout declared: `tests/CMakeLists.txt` sets `TIMEOUT`/`PROPERTIES` for `sb_evs_structural_sweep`, `obcp_engine_connectivity` and `link_uart` but **not** for `es_hs_lifecycle` or `decoder_corpus` (lines 163–164), and the CI workflows define no per-test timeout. A regression that made the corpus loop non-terminating would hang CI rather than fail fast. Because the loop bounds are compile-time constants, the risk is low, but the "time budget … controlled" wording in sprint row 98 is not backed by an enforced budget.

## 7. Scope item (4) — N22 nominal + out-of-bound against explicit expected results

Applying the N22 test-design rule (nominal and applicable out-of-bound cases, each compared against explicit expected results) to the two suites:

| Suite | Nominal case (explicit expected result?) | Out-of-bound case (explicit expected result?) | Assessment |
| --- | --- | --- | --- |
| `test_es_hs_lifecycle` | Yes — `test_hs_heartbeat_no_fault`: exactly `2` heartbeats for two passes, `0` `FF_EVS_APP_ERROR`, heartbeat `app_id == hs_id` (lines 184–191) | Yes — `test_es_start_failure_and_hs_detection`: PAL slot exhaustion (`fillers < 64u`), `FF_EsStartApp(bad_id) == FF_ERROR`, state `FF_APP_FAILED`, exactly one CRITICAL `FF_EVS_APP_ERROR` naming only the failed id, per pass, with persistence (lines 280–314) | Satisfies N22 |
| `test_decoder_corpus` | Yes — seeded TC/TM round-trip recovers fields/payload exactly (SRS-REQ-003/005 oracle) | Yes — random-byte rejection, truncation/extension rejection, single-bit mutation rejection, undersized-PDL rejection, cross-type rejection, non-telecommand APID `FF_ERROR`, unknown service `FF_NOT_FOUND`, marker handler not invoked | Satisfies N22, with the F-0379-01 and F-0379-06 caveats |

Both suites therefore meet N22 for the objectives they claim. The two caveats (masked minimum-PDL sub-case; permissive status set) reduce the *strength* of two specific out-of-bound claims without removing nominal + out-of-bound coverage overall.

## 8. Scope item (5) — honesty of the claims

No overstatement was found in the reviewed artifacts. Specifically:

- **No MC/DC claim.** `test_decoder_corpus.c` header lines 27–28 state "This suite claims no MC/DC, no structural coverage and no qualification."; TS-6 row 35 repeats "No MC/DC or qualification is claimed from fuzzing".
- **No tool/process qualification claim.** Both headers and both spec rows stop at "covered"/host evidence; no N10/N11 qualification is asserted.
- **No target claim.** `test_es_hs_lifecycle.c` header and TS-6 row 41/55 keep detection latency, watchdog cadence and restart as `gap`; row 55 says the harness is host-only and its R2/R3 delta review "remains pending".
- **NC status: honest.** `docs/vv/verification-results.md` (S8-VV-FU §"NC disposition"), `docs/status/loop-audit.md` lines 63/188/189 and `docs/planning/sprint-plan.md` rows 160/161 all state that `GAP-R3-1` and `NC-EXT-25` are *materially closed but remain OPEN pending the independent R2/R3 delta review*. TS-5 row 51 (line 175) and TS-6 row 55 (line 172) now read "remains OPEN pending the independent R2/R3 delta review" — the "closes the gap" wording flagged by 0374 F-0374-03 has been corrected. ✅
- **GAP discipline.** The two ungrounded ES/HS observations are explicitly labelled observations; the SRS-REQ-003/004/005 oracles are requirement-cited. ✅

One internal inconsistency is a Comment, not an overstatement: TS-6 `N22-TS6-58` (line 175) says randomised streams "return only documented statuses (`FF_PARSE_ERROR`)" while the harness (lines 540–543) accepts the wider documented set `{FF_PARSE_ERROR, FF_ERROR, FF_NOT_FOUND, FF_NOT_READY}` (F-0379-06).

## 9. Scope item (6) — recomputation summary

Recomputation is reported in §3.1/§3.2: TS-5 = 77 / 47 / 27 / 3 / 67 / 10 / 56 covered / 21 gap, SIL 74 / PIL 1 / HIL 2; TS-6 = 58 / 15 / 43 / 47 / 11 / 30 covered / 27 gap / 1 exploratory, SIL 53 / PIL 1 / HIL 4. **All match the documents.** The only stale figure is the prose in `docs/vv/verification-results.md` line 423 (76 / 55 / 21) — F-0379-07.

## 10. What this review does NOT establish

- It does not qualify the tests, the tooling or the process (no N10/N11), and it does not establish an N11-controlled run: the executions recorded here are reviewer-host runs outside the evidence log.
- It does not provide target/HIL, WCET, jitter, interrupt/DMA, RTEMS or radiation evidence; none exists in the reviewed artifacts, and all such claims remain open (`GAP-A5`, `NC-EXT-47`, `NC-EXT-90`).
- It does not prove either oracle free of a defect shared with the implementation; it *records* the shared-literal/shared-routine couplings as open limits.
- It does not close, approve or disposition `GAP-R3-1` or `NC-EXT-25`, nor any other NC.
- It does not cover the reset path or discriminating concurrency assertions, because no such artifacts exist (§5).

### 10.1 `GAP-R3-1` / `NC-EXT-25` disposition asserted by this review

- **`GAP-R3-1` (ES lifecycle / HS heartbeat host integration): NOT cleared, remains OPEN.** The review finds the host-integration evidence objective is materially demonstrated and honestly described, but (a) the reset-path portion of the related coverage is still unexecuted (§5.1), (b) the "independent EVS-listener oracle" wording in the citing TS rows omits the disclosed literal coupling (F-0379-02), and (c) closure is an owner/authority disposition (§3.2), which is not this reviewer's to make. Recommendation: a *narrow, host-only* closure may be proposed to the owner **after** F-0379-02 and F-0379-03 are dispositioned.
- **`NC-EXT-25` (bounded reproducible decoder corpus + independent oracle): NOT cleared, remains OPEN.** This review supplies the independent V&V confirmation that the NC text makes a precondition ("pending independent V&V review confirmation") for the *host-only bounded corpus*, and finds that corpus requirement-grounded and discriminating except the masked minimum-PDL sub-case (F-0379-01). The NC's residual scope — independent/reference-oracle independence beyond the shared `FF_CcsdsCrc16` — is **not** discharged and remains with the owner. Recommendation: a narrow closure of the *host-only bounded-corpus* objective may be proposed to the owner **after** F-0379-01 and F-0379-06 are dispositioned; the external/reference-oracle element stays open.
- **`GAP-A3` / `GAP-A11` (organizational/model independence): not in scope; remain OPEN** and are not affected by this review.

## 11. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F-0379-01 | Minor | TS-6 `N22-TS6-57` (SRS-REQ-003); `test_decoder_corpus.c` min-PDL sub-case | The corpus sub-case described as "a Packet Data Length that cannot hold the configured secondary header + CRC trailer … rejected before any CRC access" is **masked** by its own caller: the vector supplies `supplied = FF_CCSDS_HDR_LEN + 1 = 7` so the decoder's preceding guard `len < FF_CCSDS_HDR_LEN + sec_len` (11 for TC via `FF_PUS_TC_SEC_HDR_LEN`, 19 for TM via `FF_PUS_TM_SEC_HDR_LEN`) short-circuits to `FF_PARSE_ERROR` and the minimum-PDL branch is never reached. Mutation m3 (minimum-PDL check removed) survived with **0** assertion failures. The requirement outcome (reject) is still covered elsewhere (`N22-TS6-14`), so this is an over-claim of *which* branch the corpus case exercises, not a missing requirement outcome. | `tests/unit/test_decoder_corpus.c` lines 275–296 (`supplied`, `small[4]=0`, `small[5]=0`); `src/transport/ccsds/ccsds.h` (`FF_CCSDS_HDR_LEN 6`, `FF_PUS_TC_SEC_HDR_LEN 5`, `FF_PUS_TM_SEC_HDR_LEN 13`); `src/transport/ccsds/ccsds.c` ordering of the length vs PDL checks; mutant m3 in `/tmp/opencode/corpus-mut/` (0 failures); probe: crafted `len=11, PDL=4` → reference `FF_PARSE_ERROR (−12)` vs m3 `FF_CRC_ERROR (−11)` | Either construct a sub-case that reaches the minimum-PDL branch (a *self-consistent* frame whose `len` satisfies the caller guard but whose declared PDL is below `sec_len + CRC`), or reword the `N22-TS6-57` expected result to say the rejected-by-boundary case is covered and the minimum-PDL branch is exercised by `N22-TS6-14`. No status/count change. |
| F-0379-02 | Minor | TS-5 rows 49/51/76; TS-6 row 55 — "independent EVS-listener oracle" wording | The four rows cite `test_es_hs_lifecycle.c` as an "independent EVS-listener oracle" without the coupling caveat that the harness header states in full: the heartbeat event id `0x0502` and the text `"app %u failed"` are implementation-defined literals of `src/apps/app_hs.c`, so the event-**identification** oracle is coupled to the implementation and "cannot independently detect a defect in the shared literal itself". This is asymmetric with TS-6 row 56 (which carries its shared-`FF_CcsdsCrc16` limit) and with the harness's own disclosure. The listener *seam* is independent; the *identifier* is not. No controlled clause fixes the id/text. | `tests/unit/test_es_hs_lifecycle.c` lines 20–28 (coupling disclosure), 96, 185–186, 302, 310; `src/apps/app_hs.c` lines 49, 55; `docs/vv/test-spec/ts-5-core-services.md` lines 174/175/201; `docs/vv/test-spec/ts-6-sw-robustness.md` line 172; contrast `ts-6-sw-robustness.md` line 173 (row 56 limit) | Add the one-sentence coupling limit (id/text are `app_hs.c` literals; the oracle detects drift from, not correctness of, those literals) to TS-5 rows 49/51/76 and TS-6 row 55, mirroring the harness header and row 56. No status/count change. |
| F-0379-03 | Minor | Sprint row 98 exit criterion; TS-5 `N22-TS5-50` | The exit gate names "verification of test-oracle independence, **reset/concurrency coverage** and reproducibility". Independent search finds **no** executable reset test: `N22-TS5-50` is `covered` with expected text requiring the harness to "stub/intercept `FF_PalPanic`", but no `FF_PalPanic` stub/interceptor exists in `tests/` or `src/`, and no test invokes PUS Service 8 function 1 / `ff_hs_fn_reset`. Concurrency is exercised structurally (real POSIX PAL threads) but no assertion depends on an interleaving; cross-test isolation relies on a wall-clock sleep (`FF_PalTaskDelayMs(20u)`) because the POSIX PAL never frees a returned task's slot, so the hand-off is timing-based rather than deterministic. `N22-TS5-50` is not over-claimed as *passing*, but the named coverage does not exist at this revision. | `docs/planning/sprint-plan.md` row 98 (Verify column); `docs/vv/test-spec/ts-5-core-services.md` line 175 (`N22-TS5-50`); `src/apps/app_hs.c` lines 27/31/61; `src/core/pal/pal_posix.c` (`FF_PAL_MAX_TASKS 32`, slots not freed on entry return); `tests/unit/test_es_hs_lifecycle.c` lines 123–135, 206, 239, 261, 280; grep over `tests/` for `FF_PalPanic`/`ff_hs_fn_reset` → none | Either add the stubbed-`FF_PalPanic` reset case and a deterministic, assertion-bearing concurrency case, or record the reset and concurrency coverage as explicit open limits (with owner and target milestone) so the exit gate is not read as satisfied by structure alone. Non-functional for the delta itself; tracked under N21. |
| F-0379-04 | Comment | `test_decoder_corpus.c` anomaly context | Failure messages print `seed` and `iter` only. The *template index* is never printed, and `g_iter` is stale in two sub-cases: the extension loop iterates on `extra` (lines 255–271) without updating `g_iter`, and the undersized-PDL block (lines 275–296) never sets it — so those anomalies report a context value belonging to a previous sub-test. The ES/HS harness emits no `anomaly()` context at all. | `tests/unit/test_decoder_corpus.c` lines 54, 66–69, 237, 255–271, 275–296; `tests/unit/test_es_hs_lifecycle.c` (no anomaly helper) | Optional: print the template index and the sub-case identity (or set `g_iter` per sub-case); add an anomaly-context line to the ES/HS harness. Improves reproducibility of a failure, does not change the assertions. |
| F-0379-05 | Comment | Time budget for the two targets | The corpus and harness are bounded by compile-time iteration/length constants, but no `TIMEOUT`/wall-clock budget is declared for `es_hs_lifecycle` or `decoder_corpus` (other targets set `TIMEOUT`), and the CI workflows set no per-test timeout. A regression making the corpus loop non-terminating would hang rather than fail fast. | `tests/CMakeLists.txt` lines 33–34, 41, 163–164 (no `TIMEOUT`/`PROPERTIES`) vs the three targets that set a timeout; `.github/workflows/ci.yml`/`.gitea/workflows/ci.yml` (no per-test timeout) | Optional: add a CTest `TIMEOUT` (e.g. 60 s) to the two targets and mirror it where the CI runner honours it. Low risk given the constant bounds. |
| F-0379-06 | Comment | TS-6 `N22-TS6-58` expected-result text vs harness | The row says randomised streams "return only documented statuses (`FF_PARSE_ERROR`)", while the harness treats `{FF_PARSE_ERROR, FF_ERROR, FF_NOT_FOUND, FF_NOT_READY}` as the documented set and counts anything outside it as a violation. The harness's `set ⊆ documented` intent is legitimate; the row's parenthetical narrows the set to a single member and does not match the executable oracle. | `docs/vv/test-spec/ts-6-sw-robustness.md` line 175; `tests/unit/test_decoder_corpus.c` lines 540–543 | Reword the row's parenthetical to "return only statuses from the documented set (`FF_PARSE_ERROR`, `FF_ERROR`, `FF_NOT_FOUND`, `FF_NOT_READY`), never `FF_SUCCESS`". No status/count change. |
| F-0379-07 | Comment | `docs/vv/verification-results.md` line 423 | The S8-VV-FU "Test-spec impact" prose states TS-5 §3.2 "moves to 76 total / 55 covered / 21 gap", which is stale against the committed TS-5 §3.2 (77 / 56 / 21) and against sprint-plan history 1.68 (which records the correction to 77/56/21). Understates the delta; TS-6's figures in the same sentence are correct. | `docs/vv/verification-results.md` line 423; `docs/vv/test-spec/ts-5-core-services.md` lines 336–345; `docs/planning/sprint-plan.md` line 507 (1.68) | Correct the TS-5 figures to 77 / 56 / 21. Overlaps the R4/`NC-EXT-96` documentation-drift NC; raised here as a factual-consistency Comment only, not an R2 coverage finding. |

No Major findings. No finding alters a requirement, a case status, a count, or a traceability link.

## 12. Uncertainty and confidence

- **Facts (High confidence, derived directly):** all TS-5/TS-6 §3.2 counts recompute exactly; the two tests pass on the host (52 / 14926 checks, 0 failures) and are stable across repeated runs; mutant m3 survives with 0 failures; the heartbeat id/text are `app_hs.c` literals; no `FF_PalPanic` stub or PUS-S8-fn-1 invocation exists in `tests/`; the NC/`GAP-R3-1` status wording is consistently "materially closed, remains OPEN".
- **Inferences (Medium confidence):** that the masked min-PDL sub-case is an oversight rather than a deliberate boundary choice (supported by the row's explicit "before any CRC access" wording, which the mask does not test); that the missing reset case is a scheduling deferral rather than an intentional scope removal (supported by the `N22-TS5-50` expected-result text still describing the stub the harness should have).
- **Measurement/model uncertainty:** mutation results are a sensitivity indicator, not proof of defect absence; the CRC oracle's residual coupling to `FF_CcsdsCrc16` cannot be removed by this review, only recorded.
- **Untested state space:** the reset action, deterministic concurrency/interleaving, target timing/WCET/jitter, memory protection, interrupt/DMA interaction, and the exact-boundary `skipped == room` scheduler path (0374 F-0374-06).
- **Could not verify:** whether the hosted CI ASan leg produced the same 40/40 (no N11-controlled log was re-verified here); closure authorization for `GAP-R3-1`/`NC-EXT-25` (not a reviewer authority).
- **Overall confidence:** **High** for the recomputation, reproducibility and honesty assessment; **Medium** for the severity attribution of F-0379-01 and F-0379-03.

## 13. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — no Major findings; Minor findings F-0379-01…F-0379-03 tracked to closure, Comments F-0379-04…F-0379-07 noted
- [ ] REJECTED — one or more Major findings

**Bounded conclusion (delta scope only).** The S8-VV-FU increment's two test artifacts are deterministic, bounded, requirement-grounded and honest about their limits; their nominal + out-of-bound cases satisfy N22 for the objectives they claim, and the coverage prose in TS-5 rev 1.3 / TS-6 rev 1.5 recomputes exactly. The two oracles are non-tautological *given the disclosed couplings*, which are stated at the artifacts but not fully mirrored in the citing spec rows (F-0379-02), and the minimum-PDL corpus sub-case does not exercise the branch it names (F-0379-01). The reset path is specified but unexecuted and concurrency is not asserted (F-0379-03).

This review **closes and approves nothing**: `GAP-R3-1` and `NC-EXT-25` **remain OPEN** (§10.1); `GAP-A3`/`GAP-A11` remain open and out of scope; no qualification, MC/DC, structural-coverage, target/HIL, phase, acceptance, certification or flight-readiness conclusion is implied or authorised.

## 14. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R2 (N9) | independent review agent (FRICK-REVIEW-0379) | 2026-10-18 | APPROVED WITH COMMENTS |
| R3 (N9, requirements/interface annex) | independent review agent (FRICK-REVIEW-0380) | 2026-10-18 | APPROVED WITH COMMENTS — the cited clauses (SRS-REQ-003/004/005/014, SSS-REQ-008, TS-5 §1.2/§1.3 ground, TS-6 §1.3 ground) exist and support the oracles; the only requirement-side findings are F-0379-01/02/06 (unqualified oracle wording), not a clause defect |

## 15. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| F-0379-01 | Minor | TS-6 `N22-TS6-57` / corpus min-PDL sub-case | Sub-case masked by caller length guard; m3 survives | Pending | mutant m3 0 failures (this report §4.4) | FSW (aeronautics-space-fsw-engineer) | open | — |
| F-0379-02 | Minor | TS-5 rows 49/51/76; TS-6 row 55 | "independent EVS-listener oracle" omits the disclosed heartbeat-id/text coupling | Pending | harness header lines 20–28 (this report §4.1) | FSW | open | — |
| F-0379-03 | Minor | sprint row 98; TS-5 `N22-TS5-50` | Reset path unexecuted; concurrency not asserted | Pending | grep of `tests/` for `FF_PalPanic`/`ff_hs_fn_reset` → none (this report §5) | FSW/V&V | open | — |
| F-0379-04 | Comment | corpus anomaly context | Template index unprinted; `g_iter` stale in two sub-cases; harness has no anomaly context | Pending (optional) | this report §6 | FSW | open | — |
| F-0379-05 | Comment | time budget | No CTest `TIMEOUT` on the two targets | Pending (optional) | `tests/CMakeLists.txt` 163–164 (this report §6) | FSW | open | — |
| F-0379-06 | Comment | TS-6 `N22-TS6-58` | Row's status-set parenthetical narrower than the harness oracle | Pending (optional) | this report §8 | FSW | open | — |
| F-0379-07 | Comment | `verification-results.md` line 423 | Stale TS-5 §3.2 prose (76/55/21 vs 77/56/21) | Pending (overlaps R4/`NC-EXT-96`) | this report §3.1/§9 | FSW/documentation | open | — |

**`GAP-R3-1` / `NC-EXT-25` re-verification entry:**

| NC | Item | This review's assertion | Disposition authority |
| -- | ---- | ----------------------- | --------------------- |
| GAP-R3-1 | ES/HS lifecycle + HS detection host integration | **NOT cleared — remains OPEN.** Host-integration objective materially demonstrated and honestly described; reset-path coverage (F-0379-03) and oracle-wording (F-0379-02) outstanding; closure is an owner/authority disposition | FSW/V&V owner (proposal may follow F-0379-02/-03 disposition) |
| NC-EXT-25 | Bounded reproducible decoder corpus + independent oracle | **NOT cleared — remains OPEN.** Independent V&V confirmation of the host-only bounded corpus supplied (subject to F-0379-01/-06); external/reference-oracle independence beyond the shared `FF_CcsdsCrc16` is **not** discharged | FSW owner (verifier: independent V&V) |
| GAP-A3 / GAP-A11 | Organizational/model independence | Out of scope; **remain OPEN**; unchanged by this review | not the reviewer's to change |

---

This review report is a working-tree artifact (norm N11). It modifies no other file, commits nothing, and pushes nothing. The reviewer is not the designated committing agent (N7).
