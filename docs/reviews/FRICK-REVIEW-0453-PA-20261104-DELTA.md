# Independent PA Delta Review — 2026-11-04 Gate-Reconciliation Candidate

| Field | Value |
| --- | --- |
| **Document ID** | **FRICK-REVIEW-0453-PA-20261104-DELTA** |
| **Date / reviewer** | 2026-11-04 · Senior Aeronautics and Space SW Product Assurance Engineer (norm N20, PA profile), independent sub-agent, fresh context |
| **Scope** | Delta over `41dcbe2` = PAL test seam, T3 test-script changes, `test_evs_hs_e2e.c`, `test_frozen_utc_seam.c`, test wiring, and the 2026-11-03/2026-11-04 documentation and evidence records |
| **Base + candidate ref** | `HEAD = 41dcbe21435e14b5aaff06c63d9df6f0de04df54` + uncommitted tree, bound by `docs/vv/evidence/2026-11-04-gate-reconciliation/candidate-manifest.sha256` (62 entries, all verified) |

**Verdict: NEEDS REVISION (not approved) — 2 Major, 12 Minor, 3 Observations.**
The candidate is materially better than the state criticised by FRICK-REVIEW-0440/0441: the unsupported "51/55" figure is withdrawn with no invented substitute history, the withdrawn mutation test is preserved verbatim, and the 2026-11-04 gate set is reproducible from retained logs. It is not approvable as submitted because (i) T3 is recorded **QUALIFIED** against a test-script configuration that no cited review covers, and (ii) the sprint plan, handover and V&V record assert that *all seven* PA-NC235 Major findings are resolved while the review they cite records one of them as **PARTIALLY RESOLVED** and keeps NC-EXT-235 open. Both repeat the failure mode that created NC-EXT-235: a status claim that its own evidence does not support.

---

## 1. Scope, method and independence

**In scope (reviewed).** `src/core/pal/pal.h`, `src/core/pal/pal_posix.c`; `tests/unit/test_pus.c`, `tests/unit/test_link_uart.c`, `tests/unit/test_evs_hs_e2e.c`, `tests/unit/test_frozen_utc_seam.c`; `tests/CMakeLists.txt`; `docs/requirements/traceability.yaml`; `docs/vv/test-spec/ts-5-core-services.md`; `docs/vv/tool-qualification.md`; `docs/architecture/time-and-timecode.md`; `docs/vv/verification-results.md`; `docs/planning/sprint-plan.md`; `docs/status/handover.md`; `docs/vv/evidence/2026-11-03-gate-diagnosis/`; `docs/vv/evidence/2026-11-04-gate-reconciliation/` (including `withdrawn/`). Prior reports 0439/0440/0441/0442/0443/0444/0445/0446/0448/0449/0450/0451 were read as context.

**Method.** Recomputed all 62 hashes in `candidate-manifest.sha256` with `sha256sum -c` (**62/62 OK**) before reviewing. Inspected diffs against `41dcbe2`, file mtimes, log contents, exit-code files and review reports. Re-executed the two retained candidate binaries from `build/gate6/tests/`: `test_frozen_utc_seam` → `checks=4320 failures=0`, `test_evs_hs_e2e` → `checks=60 failures=0`, both exit 0. I did **not** re-run the container configure/build/CTest/format/Lean/cppcheck/RTEMS gates; findings on those rely on the retained logs, as declared in §9.

**Independence.** I authored none of the reviewed content, ran none of the recorded gates, and did not modify the tree. I did re-execute two already-built test binaries to check published check counts. I am not a licensed engineer, certification representative, auditor of record or approval authority.

---

## 2. What the candidate gets right (recorded so it is not re-litigated)

1. **"51/55" retraction is explicit, repeated and honest.** `docs/vv/evidence/2026-11-04-gate-reconciliation/README.md` §2, `docs/vv/verification-results.md:227-230`, `docs/planning/sprint-plan.md:322` and `docs/status/handover.md:1597` all state that no retained artifact records 55 tests and that no replacement history is invented. This is the correct disposition.
2. **`build/gate5` provenance is disclosed.** The container-only `/workspace` cache, root ownership and the as-found result (53 registered, 52 pass, 1 fail, exit 8) are recorded with the actual failing test (`pus_mutation`).
3. **Withdrawn test handled correctly** (assessed in §3, Q4): source preserved verbatim inside the evidence package and inside the manifest, sha256 recorded and re-verified (`09a7a62c…31cd9`), `SYNTAX_EXIT=1` retained with the 8 compiler errors, registration removed, and no requirement was pointing at it.
4. **Traceability and residual limits stay visible.** `N18` remains **FAIL** (NC-EXT-198/NC-PAL-01), GAP-A4 remains an advisory-checker gap, NC-EXT-19/207/208/211 remain open, and `FRICK-VV-GATE-20261104` repeats that no NC or sprint is closed.
5. **Test-only seam is correctly gated.** `FF_TEST_HOOKS` is defined only for `FF_BUILD_TESTS` on non-RTEMS (`CMakeLists.txt:218-220`); the seam is compile-time absent from flight builds and the RTEMS log shows a deliberate symbol check.

---

## 3. Findings

Severity per `docs/reviews/review-template.md`. All actions are for the owning FSW/PA process; none is a disposition I can make.

### Major

**PA-01 — Major — T3 is recorded QUALIFIED against a test configuration no cited review covers (N10, N16).**
`docs/vv/tool-qualification.md:26` declares T3 = **QUALIFIED** and its scope is "CTest / unit tests" over `tests/unit/*.c`; `:254` (rev 0.3.108, file mtime 2026-11-03 18:20:03) says qualification is complete, citing FRICK-REVIEW-0443 and the hosted record. Objective evidence:
- `FRICK-REVIEW-0443` (mtime 10:20:31) reviewed exactly two scripts, `test_pus.c` and `test_link_uart.c`; its scope §1 explicitly lists **"other T3 scripts"** as out of scope (also restated in the 0.3.108 row text).
- `tests/unit/test_frozen_utc_seam.c` is a new CTest-registered unit test under T3's declared scope, with last write **18:35:54**, i.e. after the qualification record and after both 0450 and 0451.
- A repository-wide search of `docs/reviews/` for `test_frozen_utc_seam` returns **zero** matches: no independent review of this script exists.
- The script's executed count is 4320 checks (reproduced), while `docs/status/handover.md:1596` records 2327 — so the qualified/reviewed artefact cannot be matched to the current file even by count.
Consequence: the QUALIFIED status is not tied to an identified, reviewed, configuration-controlled version of every script in its declared scope. Whether the post-qualification write was formatting-only cannot be established from the tree (the file is untracked and no pre/post hash pair is retained), which is itself the defect.
**Action:** withdraw the T3 QUALIFIED status (or narrow it explicitly per script) until `test_frozen_utc_seam.c` — and `test_evs_hs_e2e.c`, which is likewise absent from 0443 — has an independent N10 review bound to its manifest sha256, then re-issue the register entry against that hash. Record the qualification basis as "reviewed at <sha256>" rather than "tests/unit/*.c".

**PA-02 — Major — "all seven PA-NC235-01..07 resolved" contradicts the cited review (N8, N11, N12).**
`docs/planning/sprint-plan.md:322` and `docs/status/handover.md:1596` state "PA FRICK-REVIEW-0451 (PASS WITH COMMENTS, 0 Major, all seven PA-NC235-01..07 resolved)" and "NC-EXT-235 status: CORRECTED — INDEPENDENT REVIEWS COMPLETE"; `docs/vv/verification-results.md:272` repeats "PA-NC235-01..07 resolved". The cited report says otherwise in two places:
- `docs/reviews/FRICK-REVIEW-0451-PA-NC-EXT-235-REVIEW.md:87` — **PA-NC235-07 = PARTIALLY RESOLVED** (native CI and RTEMS effective `CMakeCache` values/hashes not captured).
- `:89` says "All 7 Major findings … are resolved", contradicting `:87` in the same report; `:203`/`:225` keep NC-EXT-235 **CORRECTED — PENDING INDEPENDENT REVIEW** and state it "must not be closed".
The candidate set therefore contains mutually incompatible status statements for the same NC, and the stronger statement is the one propagated into the plan, handover and V&V record. This is the same defect class as NC-EXT-235's original unsupported gate claim.
**Action:** replace the "all seven resolved" wording with the review's actual finding set (`PA-NC235-07 PARTIALLY RESOLVED`; `PA3-NC235-01` reference), state the residual open condition (effective build configuration not captured), and stop using "INDEPENDENT REVIEWS COMPLETE" as a status where the review itself withholds closure. Reconcile the internal contradiction inside FRICK-REVIEW-0451 or annotate it as superseded.

### Minor

**PA-03 — Minor — FRICK-REVIEW-0451 finding `PA3-NC235-01` is not reproducible against the reviewed tree (N8, N20).**
`FRICK-REVIEW-0451:135` states that `docs/architecture/time-and-timecode.md` "still does not document the `FF_TEST_HOOKS` frozen-UTC seam". The reviewed file (rev 0.4, mtime 2026-11-03 08:59:25, i.e. before the 18:23:38 report) documents the seam at `docs/architecture/time-and-timecode.md:35-45` and records it in the History at `:262`. A PA report that asserts a documentation gap already closed weakens the review chain in the opposite direction (false open item), and the plan then resolves it without a re-review.
**Action:** correct or retract `PA3-NC235-01` with the file state and hash that were inspected; record that the doc gap was in fact closed before the review.

**PA-04 — Minor — N22-TS5-47 PA review is misattributed to FRICK-REVIEW-0442 (N11, N16).**
`FRICK-REVIEW-0451:10-11` describes 0442 as "PASS WITH COMMENTS for N22-TS5-47 delta only". `FRICK-REVIEW-0442` does not mention `test_evs_hs_e2e.c`, N22 or TS5-47; its scope is the NC-EXT-235 candidate and it explicitly excludes untracked files. The actual N22-TS5-47 review chain is 0444/0445 (V&V) and 0446/0448/0449 (PA).
**Action:** correct the citation in 0451 or annotate it; cite 0449 (final PA) and 0445 (final V&V) for N22-TS5-47.

**PA-05 — Minor — Traceability link of `test_evs_hs_e2e.c` to SRS-REQ-074 is unsupported (N4, N22).**
`docs/requirements/traceability.yaml:695` adds `tests/unit/test_evs_hs_e2e.c` to SRS-REQ-074. SRS-REQ-074 (`docs/requirements/SRS.md:434-444`) requires exactly one pre-reset PUS-C S5 report with the panic/reset event id, lock-free and independent of EVS/SB/PUS/queue/time locks, formatting plus bounded console/UART write within one 250 ms absolute deadline, and untruncated mandatory fields. The test never enters a panic/reset path, never uses the panic event id, asserts no deadline, no lock-freedom and no truncation rule; it calls `FF_PusEmitEvent` directly with event `0x0103`. FRICK-REVIEW-0449:49 nonetheless marked the link **SUPPORTED**.
Consequence: the matrix would report coverage of a safety-relevant requirement that this test does not exercise. The other two links (SRS-REQ-014 `:159`, SSS-REQ-008 `:765`) are supported by the HS-detection assertions.
**Action:** remove the SRS-REQ-074 link, or annotate it explicitly as partial/incidental and exclude it from any closure claim; correct 0449's "SUPPORTED" row. If this link is later used at a gate as SRS-REQ-074 evidence, treat it as Major.

**PA-06 — Minor — Unresolved `NC-EXT-XXX` placeholder embedded in a controlled test (N8, N12, N16).**
`tests/unit/test_evs_hs_e2e.c:6` and `:74` record the EVS-listener → TM-sink forwarding gap as `NC-EXT-XXX`, and `:8-9` states that Part 3 of the nominal end-to-end path is not implemented. No NC with that (or any) identifier is registered for this gap in `docs/planning/sprint-plan.md`; the test spec instead says the forwarding "is a framework integration gap tracked separately" (`docs/vv/test-spec/ts-5-core-services.md:177`).
Consequence: a known functional integration gap exists without a controlled NC identity, and a placeholder token sits in reviewed source.
**Action:** register the gap as a numbered NC with owner and disposition, replace the placeholder with the real ID (or remove it and reference the NC in TS-5), and state in TS-5/`verification-results.md` that Part 3 is unverified.

**PA-07 — Minor — The `event_id` oracle cannot discriminate the production contract, and the test name overstates the path (N22).**
`src/pus/pus.c:334-347` shows `FF_PusEmitEvent` does `(void)event_id;` and forwards the caller's `data` verbatim. The test constructs `payload[0..1]` itself as the event id (`tests/unit/test_evs_hs_e2e.c:233-239`), then asserts the frame contains that same id (`:242`). The oracle therefore passes whether or not the function honours its `event_id` argument; it cannot detect the known behaviour at `pus.c:336`. Subservice selection (INFO/WARNING/CRITICAL → 1/2/3) and link loopback are genuinely exercised; the event-report identity contract is not. In addition, no out-of-bound case is present (e.g. `len > FF_PUS_MAX_TM_FRAME-3`), and Part 3 is skipped, so "e2e" is not established.
**Action:** state the test's real scope in TS-5 and in the traceability note ("direct S5 emission plumbing"), and either add a case that distinguishes the `event_id` argument or record the ignored-argument behaviour as the NC from PA-06. Do not cite this test for event-identity compliance.

**PA-08 — Minor — RTEMS seam-absence evidence is a bare exit code (N11).**
`docs/vv/evidence/2026-11-04-gate-reconciliation/candidate-rtems-symbols.log` contains one line, `NM_GREP_EXIT=1`. There is no command line, no `nm` output, no ELF hash and no negative control. Yet `tool-qualification.md:254`, `verification-results.md` (gate table) and `handover.md:1597` state the seam is "absent from the ELF" and "verified via `nm`".
Consequence: the assertion is plausible and consistent with the compile-time gating, but the retained artefact does not evidence it.
**Action:** retain the exact `nm`/`grep` command, the ELF sha256 and the full (or filtered) symbol output, plus one positive control symbol proving the test searched a real binary.

**PA-09 — Minor — Handover still carries the superseded "2327 checks"/"51/51" figures (N8, N16).**
`docs/status/handover.md:1596` (rev 3.59) states `test_frozen_utc_seam.c` has "2327 checks" and that "All gates PASS: 51/51 CTest". The retained binary reports `checks=4320 failures=0` (reproduced), and the 2026-11-04 clean run reports 52/52. Rev 3.60 (`:1597`) corrects the 51/55 and gate5 figures but does not retract or annotate 3.59's 2327/51-51 figures, so the audit trail still contains two unreconciled numbers in adjacent rows.
**Action:** annotate 3.59 as superseded with the measured values and the command used, rather than leaving both figures live.

**PA-10 — Minor — The manifest does not bind four modified build inputs that participate in the 52/52 result (N11, CM).**
`candidate-manifest.sha256` binds 62 files. `git status` shows `tests/unit/test_pus_n7_adapter.c`, `tests/unit/test_pusopen.c`, `tests/unit/test_r2_gap.c` and `tests/unit/test_sched_max_entries.c` as modified against `41dcbe2`; they are excluded by `CANDIDATE.md:31-38` and are copied into the gated binaries (`pusopen_adapter_contract`, `r2_gap`, `sched_max_entries` in `candidate-ctest.log`). FRICK-REVIEW-0451 recorded their diffs as formatting-only, which I confirmed for the four files' hunks.
Consequence: base commit + manifest alone does not reconstruct the tested configuration; the exclusion is declared, and the semantic risk is low, but the identity statement is incomplete.
**Action:** either add the four files to the manifest (cheap, and removes the caveat), or state in `CANDIDATE.md` that the gate result is only reproducible with the working tree as-is, listing the four paths and their formatting-only basis.

**PA-11 — Minor — `docs/reviews/README.md` is not updated for 0441/0443/0450/0451 (N20, N11).**
N20 requires review reports to be "logged in `docs/reviews/README.md`". The index contains no entry for any of 0441, 0443, 0450 or 0451 — including the two reports the candidate cites as its independent-review basis. There is also no entry for this report.
**Action:** add index rows with ID, date, profile, scope, verdict and the file path.

**PA-12 — Minor — `tool-qualification.md` revision history is out of order (N16).**
The History table places rev **1.0** (2026-11-02) at `:250`, then 0.3.107 and 0.3.108 (2026-11-03) at `:252`/`:254`, separated by blank lines. A reader or automated checker that scans the table in order sees the T3 QUALIFIED change as occurring before rev 1.0.
**Action:** renumber/append consistently (e.g. 1.1/1.2, or move the rows above 1.0) and remove the blank-line separators.

**PA-13 — Minor — The recorded cycle omits `make smoke` and `make gsw-selfcheck` without a declared exclusion (N15).**
`FRICK-VV-GATE-20261104` lists configure, build, CTest, format, trace, Lean, cppcheck and RTEMS cross-build. `Makefile:105` defines the full local bundle (`make check`) as build/test/Lean/trace/**smoke**/**gsw-selfcheck**, and N15 requires the complete cycle for a change. No log or statement records either gate as run, skipped or inapplicable for this candidate. (N18 is separately and correctly reported as FAIL, not skipped.)
**Action:** run and retain smoke (host boot with 7 apps) and the Python/GSW checks for a `src/`-touching candidate, or record an explicit, justified exclusion in `verification-results.md`.

**PA-14 — Minor — Review independence is asserted but not evidenced by session identity (N9).**
`FRICK-REVIEW-0450:7` and `FRICK-REVIEW-0451:7` say "Independent sub-agent session; no author context inherited", and 0441/0443 are similar, but none records a session identifier. The project already treats a recorded session ID as verifiable independence evidence elsewhere (e.g. handover 3.55 and NC-EXT-98 record `ses_…` identifiers). As submitted, a reader cannot distinguish a genuinely fresh review from a coordinator-authored report.
**Action:** record the reviewer session ID in the header of every independent report, and index it with the report.

### Observations (no derogation required)

**PA-O-01.** `src/core/pal/pal_posix.c:504-508` says the seam is "Called from the unit tests (tests/unit/test_pus.c)". It is also called by `tests/unit/test_frozen_utc_seam.c`. Update the comment so the contract description stays current (N8).
**PA-O-02.** `candidate-format.log` contains only the `clang-format --dry-run --Werror` command with no result line (the exit code is captured separately in `candidate-exits.txt`), and `candidate-lean.log` shows Lean 4.10.0 being downloaded during the gate. Version pinning makes this acceptable, but the log is weak evidence of a clean format run and the gate is not hermetic; a `FORMAT_EXIT=0` plus one explicit "no differences" line would be stronger.
**PA-O-03.** `handover.md:1596` says "Next authorized increment: documentation/evidence improvements only (per constraints)", while this delta modifies `src/core/pal/pal.h` and `pal_posix.c` (test-only, compile-time gated). The work is a legitimate N21 functional test-harness correction, but the authorization wording reads narrower than what was performed. Clarify that test-only changes under `FF_TEST_HOOKS` are within the authorized increment.

---

## 4. Answers to the seven PA questions

**Q1 — Evidence integrity.** *Substantially conformant, with one weak artefact.* All 62 manifest hashes verify (62/62). Every gate has an original exit code in `candidate-exits.txt` (configure/build/CTest/trace/format/lean/cppcheck/RTEMS all 0), no log is pipeline-masked, and the withdrawn test has a matching sha256 and a `SYNTAX_EXIT=1` proof. Weak point: the RTEMS symbol check (PA-08). The check counts I re-executed match the published `test_evs_hs_e2e` value (60) but not the handover's `test_frozen_utc_seam` value (4320 vs 2327, PA-09).

**Q2 — Configuration control.** *Partially conformant.* Base commit, manifest and container image identify most inputs; the two new tests and the PAL seam are bound by hash. Gaps: four modified build inputs are outside the manifest (PA-10); the T3 qualification is not bound to a reviewed hash (PA-01); and the effective native/RTEMS `CMakeCache` values/hashes remain uncaptured (0451's PA-NC235-07, correctly open). The `41dcbe2` base plus a clean `build/gate6` reproduces 52/52 in the container, so the *gate* is reproducible; the *qualification claim* is not.

**Q3 — Traceability and claim discipline.** *Partially conformant.* The 51/55 retraction is exemplary and repeated. Three claim defects remain: the "all seven resolved / reviews complete" statements contradict 0451 (PA-02); the SRS-REQ-074 link is unsupported (PA-05); and 0451's `PA3-NC235-01` is not reproducible (PA-03). The trace counts (120/76/30/14/68) are consistent across the gate log, plan and handover.

**Q4 — Withdrawn-test handling.** *Conformant; no finding.* The withdrawn `test_pus_mutation.c` was: preserved verbatim inside the evidence package and inside the manifest; hashed and re-verified; shown not to compile (8 errors, exit 1); removed from `tests/CMakeLists.txt`; shown to have no requirement reference by repository-wide search; and its stale-binary 13/1203 result is explicitly labelled as characterisation of a non-compiling revision, not coverage. No claimed coverage was removed. The only residual is that the archived binary result describes a revision that no longer exists anywhere — which the package states.

**Q5 — Independence of reviews 0450/0451/0441/0443.** *Partially evidenced.* All four declare fresh context and no author participation, and the review scopes are narrow and explicit. Problems: no session identifiers (PA-14); 0451 contains a non-reproducible finding and an internal contradiction (PA-03, PA-02); 0451 misattributes the N22-TS5-47 review to 0442 (PA-04); and none of the four reviews this 2026-11-04 delta or `test_frozen_utc_seam.c` (PA-01). `verification-results.md:274` correctly says the 2026-11-04 delta is "submitted for a fresh independent V&V/PA delta review" — which is this report, and it is the only review of the current content.

**Q6 — Residual-risk visibility.** *Conformant.* `CANDIDATE.md` "Facts the reviewer must not assume" and `FRICK-VV-GATE-20261104` both state: no NC closed, no sprint closed, N18 FAIL (NC-EXT-198/NC-PAL-01), NC-EXT-19 open, N22-TS5-47 partial/gap, GAP-A4 advisory checker, no hardware/HIL, NC-EXT-207/208/211 open, no commit/push. That is the right content and is not buried. Countervailing pressure: the "INDEPENDENT REVIEWS COMPLETE" label and the "all seven resolved" wording (PA-02) invite a reader to under-weight those residuals, and `NC-EXT-XXX` (PA-06) hides one integration gap from the NC register.

**Q7 — N21 sequencing.** *Conformant in substance, imprecise in wording.* The work performed is a functional test-harness correction (NC-EXT-235 Minor half) plus test evidence (N22-TS5-47) — both admissible under N21. Non-functional findings were recorded, not silently "fixed to perfection", and no NC or sprint is declared closed. The only issue is the handover's narrower "documentation/evidence improvements only" phrase (PA-O-03). The frozen seam is a `src/` edit but compile-time test-only; it does not change production time behaviour, which the diff confirms.

---

## 5. Norm conformance summary

| Norm | Assessment | Basis / gap |
| --- | --- | --- |
| **N8** — current, deployable documentation | **Partial** | Docs updated with the delta (time doc, TS-5, tool register, plan, handover, V&V record). Gaps: uncorrected 2327/51-51 figures (PA-09), stale PAL comment (PA-O-01), `NC-EXT-XXX` placeholder (PA-06), and a review-accuracy defect in 0451 (PA-03). |
| **N9** — independent-context reviews | **Partial** | Reviews exist with explicit scope and independence declarations; no session IDs (PA-14); the new `test_frozen_utc_seam.c` has no review at all (PA-01). |
| **N11** — scripts and results retained | **Partial** | Strong retention: all gate logs, original exits, failure JSONs, manifest, withdrawn source. Weak: RTEMS symbol evidence is one exit code (PA-08); the review index is not updated (PA-11); the manifest omits four build inputs (PA-10). |
| **N15** — complete cycle on every change | **Partial** | Configure/build/CTest/format/trace/Lean/cppcheck/RTEMS all recorded with exit 0; simulator smoke and GSW checks are absent with no declared exclusion (PA-13). N18 is correctly declared FAIL rather than skipped. |
| **N16** — detailed revision history | **Partial** | History rows added for the time doc, plan, handover, V&V record and tool register. Defects: tool-register rows out of order (PA-12); handover superseded figures not annotated (PA-09); misattributed review citation (PA-04). |
| **N20** — independent PA review of every phase product | **Partial** | 0451 is a genuine PA re-review of the 2026-11-03 candidate and does not pre-empt this delta. It contains an internal contradiction and one non-reproducible finding (PA-02, PA-03), is not indexed (PA-11), and no PA review covered `test_frozen_utc_seam.c` (PA-01). This report is the PA leg for the 2026-11-04 delta; it does not approve it. |
| **N21** — functional-first sequencing | **Conformant** | Test-harness functional correction handled; non-functional findings tracked; no NC/sprint closure claimed. Wording of the authorized increment is imprecise (PA-O-03, not a nonconformity). |
| **N22** — requirement-based/integration tests with explicit oracles | **Partial** | `test_evs_hs_e2e.c` exercises S5 subservice selection and link loopback with explicit expected results, but its `event_id` oracle is non-discriminating, no out-of-bound case is present, Part 3 is not exercised, and the SRS-REQ-074 link overstates scope (PA-05, PA-07). `test_frozen_utc_seam.c` (7 sub-tests, 4320 checks) is a behaviour test of the seam with explicit checks — adequate for its stated purpose. |

---

## 6. Findings register

| ID | Severity | Item / requirement | Finding | Evidence | Action |
| --- | --- | --- | --- | --- | --- |
| PA-01 | Major | N10, N16 — T3 qualification | QUALIFIED status covers an unreviewed, post-dated script under its declared scope | `tool-qualification.md:26,254`; `0443` scope; `test_frozen_utc_seam.c` mtime 18:35:54; 0 review matches | Withdraw/narrow T3 status; review new scripts; bind to sha256 |
| PA-02 | Major | N8, N11, N12 — claim discipline | "All seven PA-NC235-01..07 resolved" / "reviews complete" contradicts 0451 | `sprint-plan.md:322`; `handover.md:1596`; `verification-results.md:272` vs `0451:87,203,225` | Correct status text; record PA-NC235-07 partial |
| PA-03 | Minor | N8, N20 — review accuracy | 0451 says the time doc lacks the seam; it documents it | `time-and-timecode.md:35-45,262` (mtime 08:59) vs `0451:135` | Retract/correct the finding |
| PA-04 | Minor | N11, N16 — citation accuracy | N22-TS5-47 review attributed to 0442, which does not cover it | `0451:10-11` vs `0442` scope | Cite 0445/0449 |
| PA-05 | Minor | N4, N22 — traceability | SRS-REQ-074 link unsupported; 0449 marked SUPPORTED | `traceability.yaml:695`; `SRS.md:434-444`; `0449:49`; test `:213-272` | Remove or annotate the link |
| PA-06 | Minor | N8, N12, N16 — NC control | `NC-EXT-XXX` placeholder; gap has no registered NC | `test_evs_hs_e2e.c:6,8-9,74`; no NC row | Register NC; replace placeholder |
| PA-07 | Minor | N22 — oracle discrimination | `event_id` is ignored by production and re-supplied by the test | `pus.c:334-347`; test `:233-242` | State real scope; add discriminating case |
| PA-08 | Minor | N11 — evidence sufficiency | RTEMS "seam absent" backed only by `NM_GREP_EXIT=1` | `candidate-rtems-symbols.log:1` | Retain command, ELF hash, nm output, positive control |
| PA-09 | Minor | N8, N16 — figure correction | Handover 3.59's 2327/51-51 figures not corrected by 3.60 | `handover.md:1596-1597`; reproduced 4320 and 52/52 | Annotate as superseded |
| PA-10 | Minor | N11, CM — configuration identity | Manifest omits four modified build inputs | `CANDIDATE.md:31-38`; `git status`; `candidate-ctest.log:19,29-36` | Add to manifest or document the reproducibility limit |
| PA-11 | Minor | N20, N11 — review records | No index entries for 0441/0443/0450/0451 (or 0453) | `docs/reviews/README.md` | Add index rows |
| PA-12 | Minor | N16 — revision history order | rev 1.0 precedes 0.3.107/0.3.108 | `tool-qualification.md:250-254` | Renumber/reorder |
| PA-13 | Minor | N15 — cycle completeness | `make smoke` / `gsw-selfcheck` not run or declared excluded | `verification-results.md` gate table; `Makefile:105` | Run or record exclusion |
| PA-14 | Minor | N9 — independence evidence | No session identifiers in 0441/0443/0450/0451 | report headers | Record session IDs |

---

## 7. What remains unproven (explicit)

1. **Target/flight behaviour.** N18 remains **FAIL** (NC-EXT-198, root cause NC-PAL-01). No RTEMS target execution, no HIL, no electrical/clock-independence evidence. The RTEMS cross-build proves compilation and (weakly) symbol absence only.
2. **Seam absence on target.** Asserted, not evidenced, by the retained artefact (PA-08).
3. **EVS listener → PUS S5 TM sink → link forwarding** (the actual N22-TS5-47 end-to-end chain). Not implemented/verified; documented in the test as a known issue without an NC (PA-06).
4. **SRS-REQ-074** (pre-reset exactly-one report, 250 ms bound, lock-freedom, no truncation): unaffected and still unverified by this test (PA-05).
5. **Scheduler heartbeat/missed-slot/dispatch-failure EVS emission to the HS path** (NC-EXT-19) — counters only.
6. **T3 qualification of the current script set** — the claim cannot be matched to a reviewed configuration (PA-01).
7. **Effective build configuration** of the native and RTEMS gates (PA-NC235-07, correctly open).
8. **Reproducibility of 52/52 from base commit + manifest alone** (PA-10).
9. **Absence of defects / MISRA compliance** — cppcheck is advisory and unqualified (GAP-A4); 163 findings are a baseline, not a clean bill.
10. **Independence of the prior reviews as a verifiable fact** (PA-14).
11. **All human/authority dispositions**: NC-EXT-235 closure, residual-risk acceptance, release, and any compliance or flightworthiness statement.

---

## 8. Required independent human decision

No positive assurance recommendation is issued for release, commit or NC closure. The following require the authorized human/role owner, not this agent: withdrawal or narrowing of T3 QUALIFIED (PA-01); correction of the NC-EXT-235 status claims (PA-02); disposition of the unregistered integration gap (PA-06); acceptance of the residual target/N18 and GAP-A4 positions; and any statement of compliance, qualification or flightworthiness.

---

## 9. Confidence and limits

```text
Decision/product and configuration:
  Delta over 41dcbe21435e14b5aaff06c63d9df6f0de04df54, bound by candidate-manifest.sha256 (62/62 verified).
Claim type:
  Fact for file contents, hashes, mtimes, log contents and reproduced check counts.
  Inference for the effect of the unreviewed/post-dated script on the T3 claim.
  Recommendation for every action in §3.
Applicable baseline and approved tailoring:
  Project norms N1-N22 (docs/rules.md rev 2.8); ECSS-E-ST-40C / Q-ST-80C as invoked by the project;
  N20 PA profile; review template severity definitions.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  Live tree at HEAD 41dcbe2 + working tree; manifest 2026-11-04; gate logs 2026-11-03/04;
  tool-qualification.md rev 0.3.108 + 1.0; verification-results FRICK-VV-GATE-20261103/20261104;
  sprint-plan Rev 1.121; handover rev 3.59/3.60; reviews 0439-0451.
Cross-check and reconciliation:
  Manifest hashes recomputed; two candidate binaries re-executed; check counts cross-checked against
  handover and TS-5; review scopes cross-checked against the files they cite; time-doc content checked
  against 0451's claim. Conflicts are reported, not resolved in the candidate's favour.
Confidence:
  High for PA-01, PA-02, PA-03, PA-05, PA-06, PA-07, PA-08, PA-09, PA-11, PA-12 (direct file/log/hash evidence).
  Medium for PA-10 (formatting-only diffs verified by reading, not by independent recompilation of the excluded files).
  Medium for PA-13 (absence of a log is evidence of non-execution only if the package is complete).
  Low for anything depending on unretained pre-fix probe/reproduction runs or on the excluded untracked files.
Basis and detection limits:
  I did not re-run the container gate suite, the RTEMS cross-build, format, Lean, cppcheck or the CTest set.
  I did not inspect every line of every candidate file, only the delta and the cited evidence paths.
  No sampling claim is made beyond the named files.
Missing/inaccessible evidence (including paywalls):
  RTEMS recipient as-found reproduction log contents beyond the summary; effective CMakeCache values/hashes;
  session identifiers; the pre-fix probe logs; the 2327-check revision of test_frozen_utc_seam.c.
Assumptions and expiry/recheck date:
  Assumes the working tree is the complete candidate and the manifest is the intended binding (as CANDIDATE.md states).
  Recheck on any change to the manifest, the T3 register, the NC-EXT-235 status text, or the review set; otherwise 2026-12-04.
Consequences if wrong:
  If PA-01 is wrong (the script really was reviewed and the count discrepancy is cosmetic), the register still
  lacks a hash-bound qualification basis and must be tightened. If PA-02 is wrong (0451 intended "resolved" for all
  seven), then 0451's own PARTIALLY RESOLVED row and NC-open statement are wrong and must be corrected.
Required independent human reviewer/authority:
  Software PA authority for T3/qualification and NC status; FSW owner for corrections; mission/quality authority
  for residual-risk acceptance. No release/certification decision is made or implied here.
```

---

## 10. Authority statement

This is an independent PA assessment of evidence and process. It does **not** approve, certify, qualify, close any NC (including NC-EXT-235), accept residual risk, authorise commit/push, or declare flightworthiness. No source, test, plan or evidence file was modified; the only file created is this report. The owner and authorised human authorities retain every disposition and release decision.

**Reviewer:** Senior Aeronautics and Space SW Product Assurance Engineer (N20 PA profile), independent sub-agent, fresh context
**Date:** 2026-11-04
**Candidate identity verified:** 62/62 sha256 OK at review start; two retained binaries re-executed (`checks=4320 failures=0`, `checks=60 failures=0`)

---

## Provenance addendum — reviewer session identity (2026-11-04)

FRICK-REVIEW-0455 PA-0455-O-01 and FRICK-REVIEW-0453 PA-14 correctly noted that
session identity was asserted but not recorded. The coordinator's session log
records that this PA review ran in sub-agent session **`ses_e5a8d44f0ffeCBf7O1MGf3wfmp`**. This
addendum records that identifier; it changes no finding, no severity and no
verdict. The coordinator cannot retroactively prove the absence of shared
context, so the independence claim remains an assertion backed by a recorded
session boundary, not a formal N9 certificate.
