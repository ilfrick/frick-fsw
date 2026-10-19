# Easy-fixes batch — independent V&V confirmation (R2 core + R3 requirements/interface annex)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0382 (R2 core); FRICK-REVIEW-0382A (R3 requirements/interface annex) |
| Reviewer session                  | Independent agent session, fresh context, norm N9 (no participation in authoring, executing or publishing the batch) |
| Reviewer profile                  | R2 (SW V&V) for the core; R3 (systems/requirements & interface) for the annex — one session, two annex-scoped verdicts |
| Reference revision                | `7e93325` (HEAD, `main`); reviewed batch = `318f49b`, range `e7e48ee..318f49b`; status-only follow-ups `b6e1f61`, `7e93325` |
| Items under review                | `tests/unit/test_decoder_corpus.c`, `src/transport/ccsds/ccsds.c`, `tests/CMakeLists.txt`, `docs/vv/test-spec/ts-5-core-services.md`, `docs/vv/test-spec/ts-6-sw-robustness.md`, `docs/vv/verification-results.md` rev 2.91, `docs/vv/evidence/2026-10-19-easy-fixes-*.log`, `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log`, `docs/planning/sprint-plan.md` (NC-EXT-94/97/98/99 rows), `docs/requirements/SRS.md` (SRS-REQ-018/050), `docs/requirements/traceability.yaml`, `docs/interfaces/ICD.md` (FRICK-ICD-001 §4), `README.md`, `docs/architecture/pus-c-from-scratch-assessment.md`, `src/pus/pus.c`, `docs/adr/ADR-0018-*`, `docs/adr/ADR-0019-*` |
| Date                              | 2026-10-19                             |
| Scope                             | Batch-scoped independent confirmation required by **NC-EXT-99** (the batch was published without its own reviews): seven named scope items — (1) min-PDL sub-case discrimination, (2) reproduction of the retained mutation evidence, (3) CTest counts and timeout budget, (4) status of `FRICK-REVIEW-0379` findings F-0379-01..04, (5) TS-5/TS-6 oracle-claim accuracy, (6) ASan evidence, (7) R3 annex on NC-EXT-97 and NC-EXT-94. Against `docs/rules.md` (N3/N4/N9/N10/N11/N12/N15/N19/N21/N22) and the ECSS-E-ST-40C / ECSS-Q-ST-80C / ECSS-E-ST-70-41C (PUS-C) / CCSDS 133.0-B-2 context invoked by `docs/vv/verification-plan.md` |
| Independence declaration          | The reviewer took no part in authoring, executing, reviewing or approving this batch and held no prior context of it; no conflict of interest is known. This review was performed by an AI assistant, **not** a licensed/certified engineer, designated certification representative, test witness or approval authority. It approves nothing, closes no NC, clears no gap and authorises no gate, phase, qualification, acceptance, release or flight decision |
| Norms applied                     | N3, N4, N9, N10, N11, N12, N15, N19, N21, N22 (see `docs/rules.md`); N14 applies to the requirement-semantic items of the annex, whose authority is **not** this reviewer's |

## 1. Scope

**Performed.** Independent re-verification of the seven scope items the batch
claims for itself, by direct inspection of the controlled sources and evidence
files, by running the pre-configured host build's test suite, by recomputing
every cited artifact hash, and by **one controlled mutation experiment** with a
full byte-exact revert.

**Method and commands (own execution, 2026-10-19):**

- `git rev-parse HEAD` / `git log --oneline -6` → baseline identity;
- `ctest --test-dir build/native -N`, `ctest --test-dir build/asan -N`,
  `ctest --test-dir build/native -N -L n22`, `-N -L s8-vv-fu`,
  `ctest --test-dir build/native` (one full host run);
- static tally of every `add_test(...)` and guarding `if(...)` in `tests/CMakeLists.txt`,
  cross-checked against the generated `build/native/tests/CTestTestfile.cmake`;
- `sha256sum` of the three cited evidence logs and of `src/transport/ccsds/ccsds.c`;
- `make trace`;
- one mutation of `src/transport/ccsds/ccsds.c` (min-PDL rule disabled),
  rebuild of the `test_decoder_corpus` target only, one mutant run, a
  differential probe compiled against the mutant library and against the
  restored library, then full revert.

**Explicitly excluded.** (a) `make ci-local`, Docker, RTEMS and Renode builds —
excluded by the review assignment; the batch's `ci-local`/format/smoke/GSW/docs
claims are therefore **accepted only as retained-log claims with verified
hashes**, not re-executed here (the concurrent PA review 0383 owns that remit).
(b) Editing `docs/reviews/README.md` (the batch author indexes this report), any
`git add/commit/push`, and any change to a tracked file other than this report.
(c) Re-derivation of TS-5/TS-6 coverage counts beyond arithmetic consistency,
and the R4 documentation review (concurrent report 0381).

**Working-tree state.** At the end of this review `git status --porcelain` and
`git diff --stat` show **no modified or staged tracked file**; the only
untracked entries are this report and the concurrently produced
`docs/reviews/2026-10-19-easy-fixes-product-assurance-pa-0383.md` (not authored
by this reviewer). The mutation experiment was fully reverted and re-verified
(§3.2).

**Assumptions.** The `build/native` and `build/asan` directories were configured
by the FSW engineer before this review; their recorded flags are inspected in
the CMake caches, not re-derived from a fresh configure.

## 2. References

- Project norms: `docs/rules.md` (N1–N22); `docs/vv/verification-plan.md`; `docs/vv/tool-qualification.md`.
- Traceability manifest: `docs/requirements/traceability.yaml` (`make trace` → PASS, 117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked).
- Reviews carried in the batch: `FRICK-REVIEW-0379` (R2 core) + `0380` (R3 annex), `FRICK-REVIEW-0378` (R4) — all made against the **pre-correction** tree `e7e48ee`; concurrent sibling reports 0381 (R4) and 0383 (PA) for this same batch.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

### 3.1 Scope item (1) — min-PDL guard order and sub-case discrimination (F-0379-01)

**Claim under test:** the corrected `N22-TS6-57` minimum-Packet-Data-Length
sub-case in `tests/unit/test_decoder_corpus.c` now reaches the minimum-length
rule instead of being masked by an earlier guard.

**Guard order as implemented** (`src/transport/ccsds/ccsds.c`, `FF_CcsdsDecodeTc`;
`FF_CcsdsDecodeTm` is symmetric):

1. short-frame guard — `len < FF_CCSDS_HDR_LEN + sec_len` → `FF_PARSE_ERROR` (line 144; `sec_len` = 5 TC / 13 TM, so guards 11 / 19);
2. packet-type guard — non-telecommand APID → `FF_PARSE_ERROR` (line 148);
3. **minimum-PDL rule** — `declared_data_len < minimum_data_len` → `false` → `FF_PARSE_ERROR` (`ff_ccsds_packet_data_length`, lines 54–56; `minimum = sec_len + 2` = 7 / 15);
4. declared-vs-supplied consistency (lines 59–61) → `FF_PARSE_ERROR`;
5. CRC check (lines 153–158) → `FF_CRC_ERROR`.

**The corrected vector** (`tests/unit/test_decoder_corpus.c:288–317`) is
type-aware: `minimum = sec_len + 2`, `declared = minimum - 1` (i.e. one byte
below the minimum), `supplied = FF_CCSDS_HDR_LEN + declared`, `pdl = declared - 1`
so that `declared == supplied - 6`. Observed values (probe, §3.2): **TC
supplied=12 ≥ guard 11, declared=6 < minimum=7**; **TM supplied=20 ≥ guard 19,
declared=14 < minimum=15**. The vector therefore passes guards 1 and 2 and fails
guard 3 — the branch it names. The expected result is explicit and hard-fail:
`FF_TEST_ASSERT(rc == FF_PARSE_ERROR)` at line 316, with an anomaly message that
now identifies the sub-case and template (lines 314, 54–69).

**N22 assessment:** the suite keeps seeded round-trip nominal sub-tests
(fields/payload recovered, SRS-REQ-003/005) and the out-of-bound set (random
bytes, truncation, extension, single-bit mutation, cross-type, non-TC APID,
unknown service, min-PDL), each compared against an explicit expected result —
so the "nominal and out-of-bound against explicit expected results" rule is met
for the objectives this suite claims. The new `FF_TEST_ASSERT(supplied <=
CORPUS_MAX_LEN)` at line 303 adds 15 checks (one per template), which accounts
exactly for the check-count step 14926 → 14941.

**Verdict: F-0379-01 is genuinely RESOLVED** — discrimination established by
independent mutation + differential probe (§3.2), not by reading the code alone.

### 3.2 Scope item (2) — reproduction of the retained mutation evidence

**Retained claim:** `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log`
(sha256 `70948df006385a0e79f4a00b100d042f8492c3338564cee9f5f31df9085089cb`,
recomputed — **match**) shows the minimum-PDL rule removed → **15/15 failures**.

**Independent reproduction (this review, one controlled experiment):**

| Step | Command / action | Observed |
| ---- | ---------------- | -------- |
| Baseline | build + run `tests/test_decoder_corpus` | `checks=14941 failures=0`, EXIT=0 |
| Mutate | `src/transport/ccsds/ccsds.c` min-PDL branch (lines 54–56): the `return false;` replaced by a no-op `(void)0;` (rule disabled, equivalent to the retained m3 "rule removed"); pristine copy kept at `/tmp/opencode/vv0382/ccsds.c.orig` | file differs from `HEAD` |
| Rebuild | `cmake --build build/native --target test_decoder_corpus` | rebuild only the affected target |
| Run | `build/native/tests/test_decoder_corpus` | **`checks=14941 failures=15`, EXIT=1** — all 15 failures at `test_decoder_corpus.c:316` with anomaly `declared data length below the secondary-header+CRC minimum accepted`, templates 0–8 (TC) and 0–5 (TM), seed `0x5EEDF00D` |
| Differential probe | `gcc -std=c11 -I src -I src/core -I config /tmp/opencode/vv0382/probe.c <libs>` against the mutant library and against the restored library | mutant: TC `supplied=12 short_guard=11 NO-passes declared==supplied-6=YES rc=-11 (FF_CRC_ERROR)`, TM same pattern `rc=-11`; reference: identical geometry, **`rc=-12 (FF_PARSE_ERROR)`** |
| Revert | restore pristine file; `sha256sum src/transport/ccsds/ccsds.c` | `ba8457d3e8f9f0542b8a4156baffa28cf50d5b596b3dcac2629270e1cd21068b` (= pre-experiment value) |
| Revert proof | `git diff --exit-code -- src/transport/ccsds/ccsds.c` → `0`; `git status --porcelain` → only untracked review reports; re-run suite | `checks=14941 failures=0`, EXIT=0 |

**Interpretation (discrimination, not just failure count):** with the rule
disabled the vector no longer fails at step 3, satisfies step 4 by construction
(`declared == supplied - 6`) and is rejected only at step 5 with `FF_CRC_ERROR`
(the synthesised body carries no valid CRC). The assertion demands
`FF_PARSE_ERROR`, so the sub-case discriminates **exactly** the minimum-PDL rule
and nothing else. This matches the retained 15/15 log line for line
(`...mutant-m3-discrimination.log` lines 8, 11–41).

**Verdict: the retained mutation evidence is REPRODUCED.** Limit: the exact
mutant source text was not retained by the batch (only the outcome log); this
review's own mutant was likewise reverted and only its log/binary survive in
`/tmp/opencode/vv0382/`, so future reproduction must re-apply the mutation.

### 3.3 Scope item (3) — CTest counts and timeout budget

| Claim (batch) | Independent observation | Verdict |
| ------------- | ----------------------- | ------- |
| 43 ctest tests registered | `ctest --test-dir build/native -N` → `Total Tests: 43`; `build/asan -N` → 43; `build/ci` → 43 | **CONFIRMED** |
| 14 unconditional + 29 option-guarded | static tally of `tests/CMakeLists.txt`: unconditional at lines 133–136, 138, 146–149, 160–164 = **14**; guarded at 151, 155, 158, 170, 174, 177, 181, 183–202 (20 driver tests), 205–206 (lwip pair) = **29** | **CONFIRMED** (14 + 29 = 43) |
| 41 without lwIP | 43 − 2 (`lwip`, `lwip_netif`) = 41 | **CONFIRMED** |
| TIMEOUT 60 s on the two S8-VV-FU targets | `tests/CMakeLists.txt:167–168` `set_tests_properties(es_hs_lifecycle ... TIMEOUT 60 LABELS "n22;s8-vv-fu;es-hs-lifecycle")`, `(decoder_corpus ... TIMEOUT 60 LABELS "n22;s8-vv-fu;decoder-corpus")`; generated `build/native/tests/CTestTestfile.cmake:38,40` carry `LABELS "n22;s8-vv-fu;..." TIMEOUT "60"` | **CONFIRMED** |
| Exactly the 2 claimed tests under those labels | `ctest -N -L n22` → 2 tests (`es_hs_lifecycle`, `decoder_corpus`); `ctest -N -L s8-vv-fu` → the same 2 | **CONFIRMED** |
| Suite green on host | `ctest --test-dir build/native` → `100% tests passed, 0 tests failed out of 43`, `Total Test time (real) = 19.82 sec` | **CONFIRMED** (this review's own run) |

**Verdict: all CTest count/timeout claims CONFIRMED.** Note F-0379-05 (per-test
timeout) is thereby addressed for the two named targets; other targets carry
their own budgets (lines 145, 152, 171).

### 3.4 Scope item (4) — status of F-0379-01..04 and the NC-EXT-98 residual

| Finding (FRICK-REVIEW-0379) | Status assigned by this review | Basis |
| --------------------------- | ------------------------------ | ----- |
| **F-0379-01** masked min-PDL sub-case | **RESOLVED** | corrected vector reaches the named branch (§3.1); mutant killed 15/15 and differential probe isolates the rule (§3.2); `N22-TS6-57` wording at `ts-6-sw-robustness.md:174` now matches the executable oracle |
| **F-0379-02** uncaveated "independent EVS-listener oracle" wording | **PARTIALLY RESOLVED** — residual raised as **F-0382-01** | caveats added to the four cited rows: `ts-5-core-services.md:174` (TS5-49), `:176` (TS5-51), `:201` (TS5-76), `ts-6-sw-robustness.md:172` (TS6-55), and additionally `:162` (TS6-41); **but** `ts-5-core-services.md:323–326` and `traceability.yaml:161`/`:740` still assert the oracle as independent without the coupling caveat |
| **F-0379-03** reset path unexecuted / concurrency not asserted | **NOT resolved — correctly carried** as **NC-EXT-98** | `grep -rn "FF_PalPanic\|ff_hs_fn_reset" tests/` → **no match**; the NC-EXT-98 description ("the reset path specified by `N22-TS5-50` is unexecuted … no `FF_PalPanic` stub or invocation exists in `tests/`") is **accurate at HEAD**; deferral is legitimate under N21 (a PAL panic seam does not exist yet) and `N22-TS5-50` is not over-claimed as `passing` |
| **F-0379-04** anomaly context | **PARTIALLY RESOLVED** — ES/HS residual correctly carried in **NC-EXT-98** | corpus part fixed: anomaly output now carries `template=<n>` (`test_decoder_corpus.c:54–69`, retained mutant log lines 11–39); `grep -c anomaly tests/unit/test_es_hs_lifecycle.c` → **0**, so the harness still emits no reproduction key, exactly as NC-EXT-98 records |

Spot-checked outside the mandate (no status change): **F-0379-05** resolved by
the TIMEOUT/labels (§3.3); **F-0379-06** resolved — `N22-TS6-58`
(`ts-6-sw-robustness.md:175`) now states the documented set
`{FF_PARSE_ERROR, FF_ERROR, FF_NOT_FOUND, FF_NOT_READY}`, matching the harness
status set; **F-0379-07** resolved by `verification-results.md` rev 2.91
annotating the historical 76/55/21 figure as superseded while preserving the
point-in-time record.

### 3.5 Scope item (5) — TS-5 / TS-6 oracle-claim accuracy

- `N22-TS6-57` ("rejected before any CRC access", `ts-6-sw-robustness.md:174`): **accurate and now proven** — the differential probe shows the unmutated decoder answers `FF_PARSE_ERROR` (step 3) while CRC (step 5) is reached only when the rule is removed (§3.2).
- `N22-TS6-58` status set (line 175): matches `test_decoder_corpus.c:558–559` documented-set assertion — **accurate**.
- `N22-TS6-56` CRC/oracle non-independence disclosure (line 173) unchanged and still correct — **accurate**.
- Coupling caveats present in TS-5 49/51/76 and TS-6 41/55 — **accurate**; the residual instances are **F-0382-01**.
- §3.1 count/timeout/lwIP arithmetic (lines 273–291) — **accurate** (§3.3), **except** the "Unsanitized build paths" bullet (lines 302–304), which is false against the retained ASan evidence — **F-0382-02**.
- §3.2 counts recomputed: TS-5 **77 total / 56 covered / 21 gap** and TS-6 **58 / 30 / 27 + 1 exploratory** are unchanged by this batch, consistent with the batch's "no case ID or §3.2 count changed" claim — **CONFIRMED**.

### 3.6 Scope item (6) — ASan evidence

| Claim | Independent observation | Verdict |
| ----- | ----------------------- | ------- |
| ASan log identity | `sha256sum docs/vv/evidence/2026-10-19-easy-fixes-asan.log` → `4e32e21463a363772bbdee364079afbdf60ade155d23516bf46322aea404a054` = recorded value | **CONFIRMED** |
| 43/43 pass | `asan.log:144` `100% tests passed, 0 tests failed out of 43` | **CONFIRMED** |
| Sanitizer configuration | `build/asan/CMakeCache.txt:39` `CMAKE_C_FLAGS:STRING=-fsanitize=address -fno-omit-frame-pointer`; `:248` `FF_USE_LWIP:BOOL=ON` | **CONFIRMED** |
| Labels | `asan.log:151–152` `n22 = 0.11 sec*proc (2 tests)`, `s8-vv-fu = 0.11 sec*proc (2 tests)` | **CONFIRMED** |
| Companion logs | `ci-local.log` → `92c19b3e756e1d5660a23608da5b59121702cf0157f2e1b8c8b16aa6180d4c35`; `mutant` log → `70948df0…`; both recomputed = recorded | **CONFIRMED** |
| "(166.36 s)" duration | `asan.log:157` `Total Test time (real) = 94.02 sec`; `grep -rn "166.36" docs/vv/evidence/` → **0 matches** | **NOT supported — F-0382-03** |
| lwIP targets "outside the current ASan evidence" (`ts-6-sw-robustness.md:302–304`) | `asan.log:54–55` links `test_lwip`/`test_lwip_netif`, `:139–142` runs and passes `42/43` and `43/43`; cache `FF_USE_LWIP:BOOL=ON`; `CMakeLists.txt:23` defaults lwIP **ON**, and a missing `third_party/lwip` is a `FATAL_ERROR`, not a skip (`CMakeLists.txt:235`) | **NOT supported — F-0382-02** |

**Verdict: the substantive ASan claim (43/43, correct configuration, correct
labels, correct log identity) is CONFIRMED**; two attribute/scope statements in
the surrounding records are not supported by the evidence they cite
(F-0382-02, F-0382-03). The pass/fail result, the hashes and the check counts
are unaffected.

### 3.7 Scope item (7, R3 annex) — NC-EXT-97 and NC-EXT-94

**NC-EXT-97 (SRS-REQ-018 vs SRS-REQ-050 backend default) — contradiction is
REAL; registering instead of editing was CORRECT.**

- `docs/requirements/SRS.md:500–502` (SRS-REQ-018, derived): "the reference build uses the open CCSDS backend by default; the N7/ESA ASN.1 backend and PUSopen(R) adapter are opt-in", justified by "ADR-0003/ADR-0018".
- `docs/requirements/SRS.md:271–275` (SRS-REQ-050): "shall select the N7 Space / ESA `asn1-pusc-lib` backend … with `FF_USE_PUS_ASN1=ON` by default".
- Implementation: `CMakeLists.txt:25` `option(FF_USE_PUS_ASN1 ... ON)`; `docs/adr/ADR-0019-n7-pus-backend-default.md:31` "supersedes the PUS-default portion of ADR-0018"; `docs/adr/ADR-0018-open-pus-and-lwip-defaults.md:5` status = "Superseded for PUS default by ADR-0019".

Two controlled requirements therefore disagree with each other and one of them
disagrees with the build default **and** cites a superseded ADR as its
justification. Because both statements are controlled requirement text, a
unilateral edit would change requirement semantics; norm **N14** assigns that to
the requirements authority (R3). The batch correctly recorded it (NC-EXT-97,
OPEN) and changed no requirement. **This review confirms the NC text and the
decision; it does not dispose of the contradiction** — the SRS still carries an
internal contradiction until the requirements authority acts.

**NC-EXT-94 (ICD Service-1 subservices 4/6) — description ACCURATE; registering
was CORRECT; residual divergence recorded as F-0382-04 (Comment).**

- `src/pus/pus.c:173–186`: `ACCEPTANCE` → {1,2}; `START` → {3,4}; `PROGRESS` → **5 only**; `COMPLETION` → {7,8}.
- Call sites `src/pus/pus.c:238–271`: the only `FF_PUS_STAGE_START` invocation (line 260) passes `success=1` → subservice 3; progress passes `success=1` (line 263) → 5; completion passes `success` derived from `rc` (line 269) → 7/8; acceptance is invoked with 0 (line 240) and 1 (line 246).
- Derived emitted set = **{1,2,3,5,7,8}**; **TM[1,4] and TM[1,6] have no emission path** — confirming both the batch's re-derivation and the widening of the NC to cover both codes.
- `docs/interfaces/ICD.md:96` preamble states "Rows below are the services/subservices **this build answers on the wire**", and line 106 still lists `1 | 4 / 6 | start / progress failure`. `README.md:179` and `docs/architecture/pus-c-from-scratch-assessment.md:39` were corrected to `{1,2,3,5,7,8}` in the batch, with the README cross-referencing NC-EXT-94.

The ICD is the controlled interface document; changing it is an interface-authority
(R3) act, so leaving it untouched and registering the discrepancy was the right
call, and the README/assessment corrections are consistent with the
implementation. The residual is that the ICD — including its own "answers on the
wire" preamble — still asserts a wire behaviour this build cannot produce, and
the NC records the implementation-side fact but not the now-visible
README-vs-ICD divergence (**F-0382-04**). **NC-EXT-94 must remain OPEN**; this
review recommends no closure.

### 3.8 Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F-0382-01 | Minor | `FRICK-REVIEW-0379` F-0379-02 (partially implemented); `docs/vv/test-spec/ts-5-core-services.md:323–326`; `docs/requirements/traceability.yaml:161`, `:740` | The prescribed coupling caveat was added to all four cited spec rows (TS-5 49/51/76, TS-6 55) and to TS-6-41, but **three residual claims remain uncaveated**: the TS-5 §"HS host integration evidence" bullet still says the harness provides "an independent EVS-listener oracle for FAILED-app detection", and the `lean_note` fields of `SRS-REQ-014` (line 161) and `SRS-REQ-015` (line 740) in the traceability manifest still claim an "independent EVS-listener oracle". The heartbeat id `0x0502` and the text `"app %u failed"` are `app_hs.c` literals, so the event-**identification** oracle cannot detect a defect in those shared literals — the exact asymmetry F-0379-02 was raised against, now inconsistent with the corrected rows in the same documents. | `docs/vv/test-spec/ts-5-core-services.md:174/176/201` (caveated) vs `:323–326` (not); `docs/requirements/traceability.yaml:161`/`:740`; `tests/unit/test_es_hs_lifecycle.c:20–28` (harness disclosure); `src/apps/app_hs.c:49,55` (literals) | Add the one-sentence coupling limit to the TS-5 bullet and to the two `lean_note` fields, mirroring the corrected rows. No status/count change. `ts-5-core-services.md:365` is a historical revision entry and needs no rewrite. |
| F-0382-02 | Minor | `docs/vv/test-spec/ts-6-sw-robustness.md:302–304` (§3.1 "Unsanitized build paths"); ASan evidence claim | The bullet states that the two lwIP targets "are not built by the default (sanitizer) configuration, so `test_lwip`/`test_lwip_netif` are outside the current ASan evidence". **The retained ASan evidence says the opposite:** the run links both targets (log lines 54–55), executes them and passes them (lines 139–142, `42/43`, `43/43`), and the ASan cache records `FF_USE_LWIP:BOOL=ON`; `CMakeLists.txt:23` defaults lwIP to ON and a missing third-party tree is a configure `FATAL_ERROR`, not a skip. The statement also contradicts the same subsection's own arithmetic and its "ASan leg 43/43" claim (lines 282–285), since 43 = 41 + 2 lwIP tests. A controlled test specification must state which evidence exists correctly, in either direction. | `docs/vv/evidence/2026-10-19-easy-fixes-asan.log:54–55,139–142,144`; `build/asan/CMakeCache.txt:248`; `CMakeLists.txt:23,235`; `docs/vv/test-spec/ts-6-sw-robustness.md:282–285,302–304` | Correct the bullet to state that the lwIP tests **are** included in the retained ASan evidence under `FF_USE_LWIP=ON` (or, if a genuinely unsanitized configuration is meant, name it and cite its evidence). No count/status change; re-verify §3.1 arithmetic after the edit. |
| F-0382-03 | Minor | `docs/vv/verification-results.md:77` (ASan bullet) and history row 2.91 (`:1630`) | The ASan leg is recorded as "**43/43 PASS** (166.36 s)", but the retained log ends `Total Test time (real) = 94.02 sec` (line 157) and the string `166.36` occurs **nowhere** in `docs/vv/evidence/` or `docs/vv/test-spec/`. The pass/fail claim and the log hash are correct; the duration attribute of a retained V&V record is unsupported by that record. Independently found; coincides with F-0383-01 of the concurrent PA review 0383 — same single defect, one disposition needed. | `grep -n "Total Test time" docs/vv/evidence/2026-10-19-easy-fixes-asan.log` → `157: … 94.02 sec`; `grep -rn "166.36" docs/vv/evidence/ docs/vv/test-spec/` → 0 matches | Correct the figure to the logged value, or retain a log that carries the full-leg wall time. Had a result, hash or count been unsupported instead of a duration, this would be Major. |
| F-0382-04 | Comment | `docs/interfaces/ICD.md:96` (preamble) and `:106` (row `1 \| 4 / 6`); NC-EXT-94; `README.md:179`; `docs/architecture/pus-c-from-scratch-assessment.md:39` | The batch corrected README and the assessment to the true emitted set `{1,2,3,5,7,8}` (and the README cross-references NC-EXT-94), while the ICD still declares rows to be what "this build answers on the wire" and lists `4 / 6`. The controlled documents therefore now disagree visibly, and NC-EXT-94 records the implementation-side discrepancy but not the README-vs-ICD divergence that the batch itself created. Registering rather than editing was correct (interface authority, R3); the comment is about the completeness of the NC text, not about the decision. | `src/pus/pus.c:173–186` (stage→subservice map), `:260` (only START call site, `success=1`); `docs/interfaces/ICD.md:96,106`; `README.md:179`; sprint-plan NC-EXT-94 row | Keep NC-EXT-94 OPEN and, at disposition, either annotate the ICD row/preamble as reserved-but-not-generated or add the emitted-set note; extend the NC text with the README-vs-ICD divergence so the disposition covers both documents. No closure recommended. |

**No Major findings.** None of the findings alters a requirement, a case status,
a §3.2 count, a traceability link or a test result.

### 3.9 Uncertainty and confidence

- **Facts (High confidence, directly observed):** guard order and line numbers in `ccsds.c`; corrected vector geometry and assertion; baseline 14941/0, mutant 14941/15 all at line 316, restored 14941/0; probe differential `FF_CRC_ERROR` (mutant) vs `FF_PARSE_ERROR` (reference) at identical geometry; `ctest -N` = 43 on three build trees; static tally 14 + 29; TIMEOUT/labels in `tests/CMakeLists.txt:167–168` and the generated `CTestTestfile.cmake`; full host run 43/43; `trace.py` PASS; all three cited evidence-log hashes recomputed and matched; `grep FF_PalPanic|ff_hs_fn_reset tests/` → none; `grep -c anomaly test_es_hs_lifecycle.c` → 0; stage→subservice map and all four `FF_PusSendVerification` call sites; SRS-REQ-018/050 text; `FF_USE_PUS_ASN1` default ON; ADR-0018 supersession status; ICD preamble and row text.
- **Assumptions / validity range:** host-only (`build/native`, `build/asan`), single Linux host, pre-configured trees; the batch's `ci-local`, format, smoke, GSW and Doxygen legs were **not** re-executed here — their acceptance rests on hashes that were recomputed and matched, plus the concurrent PA remit. No target, HIL, RTEMS or Renode evidence is added or assessed.
- **Measurement/oracle uncertainty:** none numeric; the discriminator is a status-code comparison (`FF_PARSE_ERROR` vs `FF_CRC_ERROR`) with no tolerance. The mutant's exact source text was not retained by the batch, so reproduction requires re-applying the edit (§3.2).
- **Untested/unobservable state space:** reset/concurrency coverage (F-0379-03) remains unexecuted; external/reference-oracle independence beyond the shared `FF_CcsdsCrc16` (NC-EXT-25) and ES/HS anomaly context (NC-EXT-98) remain undischarged; nothing here constitutes target-timing, WCET, fault-injection or operational evidence.
- **Inferences (Medium confidence):** that the TS-6 §3.1 lwIP bullet was written against a non-CI configuration assumption rather than the retained log (supported by the CI's separate `build/lwip` step); that "(166.36 s)" is a full-leg wall-clock figure recorded without its log (supported by it exceeding the ctest-internal 94.02 s).
- **Overall confidence: High** for the confirmations and the two reproduced experiments; **Medium** for severity attribution (Minor vs Comment) on F-0382-01/-02.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Bounded conclusion.** On the evidence inspected, the easy-fixes batch does what
it claims on its seven mandated scope items: the corrected minimum-PDL sub-case
is **genuinely discriminating** (established by independent mutation 15/15 plus
a differential probe isolating the rule), the retained mutation evidence is
**reproduced**, all CTest count and timeout claims are **confirmed** (43 = 14 +
29, 41 without lwIP, TIMEOUT 60 s and labels on exactly the two claimed tests),
F-0379-01 is **resolved**, F-0379-02 and F-0379-04 are **partially resolved**
with residuals raised as F-0382-01 and carried NC-EXT-98 text, F-0379-03 is
**not resolved but correctly carried**, and the ASan 43/43 claim with its
configuration, labels and log identity is **confirmed** with two unsupported
attribute statements raised as F-0382-02/-03. On the R3 annex, **NC-EXT-97's
contradiction is real** and registering it instead of editing was correct (N14),
and **NC-EXT-94's description is accurate** (emitted set {1,2,3,5,7,8}; TM[1,4]
and TM[1,6] unreachable), with the residual ICD-vs-README divergence recorded as
F-0382-04.

**Limits of this conclusion.** This review **approves nothing and closes
nothing**: it does not close NC-EXT-99 (this report satisfies only the R2/R3
part of that NC's closure condition, and only the owner/authority can dispose of
it), nor NC-EXT-94, NC-EXT-97, NC-EXT-98, NC-EXT-25, `GAP-R3-1`, `GAP-A3` or
`GAP-A11`, all of which **remain OPEN** as recorded. No gate, phase, acceptance,
qualification, tool qualification (N10), target/HIL evidence, MC/DC or
structural-coverage claim, certification finding, release or flight-readiness
decision is implied or authorised. Safety-critical conclusions require qualified
independent human review and the approvals defined by the governing baseline.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9) — core | independent agent session, FRICK-REVIEW-0382 | 2026-10-19 | APPROVED WITH COMMENTS — 0 Major, 3 Minor (F-0382-01/-02/-03), 1 Comment (F-0382-04); scope items 1–6 confirmed as stated in §3.1–§3.6; no NC/gap/gate closed |
| R3 (N9, requirements/interface annex) | independent agent session, FRICK-REVIEW-0382A | 2026-10-19 | APPROVED WITH COMMENTS — scope item 7: NC-EXT-97 contradiction verified as real and correctly registered (requirement semantics remain with the N14 authority); NC-EXT-94 description verified accurate and correctly registered, residual ICD divergence recorded (F-0382-04); no requirement, ICD or ADR text changed, no closure recommended |

## 6. Follow-up records

Norm N12: NCs are resolved by re-applying **all** rules to the affected scope and
re-reviewing the deltas until full compliance. This review registers no closure;
rows below are proposals for the owner to number and assign.

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| *(proposed, owner to assign)* | Minor | `ts-5-core-services.md:323–326`; `traceability.yaml:161`, `:740` | F-0382-01: residual uncaveated "independent EVS-listener oracle" claims after the F-0379-02 correction | Add the `app_hs.c`-literal coupling caveat to the TS-5 bullet and both `lean_note` fields; no status/count change | Re-read the three locations against `test_es_hs_lifecycle.c:20–28`; re-run `make trace` | FSW / test-spec author (V&V wording reviewed by R2) | **OPEN (proposed)** | Bounded R2 confirmation |
| *(proposed, owner to assign)* | Minor | `ts-6-sw-robustness.md:302–304` | F-0382-02: bullet wrongly states lwIP tests are outside the ASan evidence that in fact contains them | Correct the bullet to match `asan.log` (lwIP built and passing under `FF_USE_LWIP=ON`), or name the unsanitized configuration meant and cite its evidence | Re-inspect bullet vs `asan.log:54–55,139–142,144` and `build/asan/CMakeCache.txt:248`; re-check §3.1 arithmetic | FSW / test-spec author | **OPEN (proposed)** | Bounded R2 confirmation |
| *(proposed, owner to assign)* | Minor | `docs/vv/verification-results.md:77`, `:1630` | F-0382-03: recorded ASan duration "(166.36 s)" supported by no retained log (log: 94.02 s); same defect as PA F-0383-01 — **one** disposition | Correct to the logged value or retain a log carrying the full-leg wall time; recompute the log hash if the log is touched | `grep -n "Total Test time" <log>` and absence of `166.36` in `docs/vv/evidence/` | FSW / documentation | **OPEN (proposed)** | R4/N17 review of the corrected document |
| NC-EXT-94 | Minor (existing) | `FRICK-ICD-001` §4 Service 1 rows 4/6 | ICD asserts wire behaviour the implementation cannot produce; emitted set is {1,2,3,5,7,8} (confirmed §3.7); README/assessment corrected, ICD unchanged; README-vs-ICD divergence not yet in the NC text | **Remains OPEN** — disposition by the interface/requirements authority (implement, or annotate row + preamble as reserved-but-not-generated); extend the NC text with the divergence (F-0382-04). No closure recommended | ICD §4 preamble/row vs `README.md:179` and `src/pus/pus.c:173–186,260` | FSW + interface/requirements authority (R3) | **OPEN** (unchanged by this review) | Independent R3 delta review |
| NC-EXT-97 | Minor (existing) | `SRS-REQ-018` vs `SRS-REQ-050`/`ADR-0019`/`CMakeLists.txt:25` | Requirement-semantics contradiction confirmed real (§3.7); correct registration, not a unilateral edit | **Remains OPEN** — requirements authority (R3, N14) disposes, with an `ADR-0019` cross-reference | `SRS.md:500–502` vs `:271–275` re-read after disposition; `make trace` re-run | Requirements authority (R3) with FSW | **OPEN** (unchanged by this review) | Independent R3 delta review |
| NC-EXT-98 | Minor (existing) | `N22-TS5-50` reset path; ES/HS anomaly context | Residual description verified **accurate at HEAD** (§3.4): no `FF_PalPanic` stub/invocation, no `anomaly()` in the ES/HS harness | **Remains OPEN** — N21 deferral is legitimate; needs a PAL panic seam that does not exist yet | `grep -rn "FF_PalPanic\|ff_hs_fn_reset" tests/` → none; `grep -c anomaly tests/unit/test_es_hs_lifecycle.c` → 0 | FSW / V&V | **OPEN** (unchanged by this review) | Independent R2 delta review |
| NC-EXT-99 | Process (existing) | Easy-fixes batch review coverage | Batch published without its batch-scoped R4/R2-R3/PA reviews | **Remains OPEN** — this report delivers the **R2/R3** part only; closure requires all three reports and their findings processed under N12 | `docs/reviews/` presence of 0382 (this), 0381 (R4), 0383 (PA) and their dispositions | FSW (request/coordinate) + the respective independent profiles | **OPEN** | Owner disposition after 0381/0382/0383 |
| NC-EXT-25 / `GAP-R3-1` / `GAP-A3` / `GAP-A11` | (existing) | external/reference-oracle independence; ES/HS host-integration closure; organizational/model independence | Not in this review's power to clear | **Remain OPEN** — closure is an owner/authority disposition | — | Owner/authority | **OPEN** (unchanged by this review) | — |
