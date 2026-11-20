# Independent Review Report — S1 progress-failure emission (`FF_PusIngest`), NC-EXT-94 (R1 FSW profile)

|                                   |                                                                                                                         |
| --------------------------------: | :---------------------------------------------------------------------------------------------------------------------- |
| Review ID                         | FSW-REVIEW-R1-20261114-s1-progress-failure                                                                              |
| Reviewer profile                  | Senior Aeronautics and Space Flight Software (FSW) Engineer — norm N9 reviewer profile **R1**                            |
| Norm profile                      | N1/N3/N4/N8/N9/N11/N12/N15/N16/N21/N22 (assessed only)                                                                    |
| Repository                        | `/home/nicola/fsw/frick-fsw`                                                                                            |
| Branch                            | `main`                                                                                                                  |
| Baseline (committed)              | `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` — "ICD: document TM[1,6] emission support and TC[20,1] set (NC-EXT-94/95)"     |
| Working tree under review         | 9 modified files + 1 untracked evidence directory (see §1.1)                                                            |
| Review date                       | 2026-11-14                                                                                                              |
| Review type                       | Bounded independent delta review of an uncommitted working-tree change (read-only)                                       |
| Change under review               | `FF_PusIngest` progress verification moved after `handler(...)`; handler success → exactly one TM[1,5] (code 0), failure → exactly one TM[1,6] (code `FF_PUS_FAIL_EXECUTION` = 20); plus regression tests and documentation records |

---

## Authority and independence statement

This is an AI expert-persona review. It is **contextual independence only** (a separate session with no authorship of the delta), **not organizational independence**. It is not an approval, certification, qualification, compliance finding, release authorization, airworthiness finding, safety-case input, or NC closure, and it does not close NC-EXT-94 or any other NC. No flight, target, hardware, timing, radiation, or security qualification is established or claimed. A qualified authorized human retains every safety-critical, interface-authority, and release decision. Norm N9 independence in the project's own sense (independent reviewer agent, recorded in `docs/reviews/`) is only partly satisfied: this review was performed by a distinct context, but it is not a substitute for the project's designated independent reviewer assignment, and `docs/reviews/README.md` was **not** updated (this review was restricted to creating this one file).

## 1. Binding hash verification (performed before any review activity)

All nine candidate hashes matched exactly on 2026-11-14; `git rev-parse HEAD` returned `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c`. Review proceeded.

| File | Declared sha256 | Measured | Result |
| ---- | --------------- | -------- | ------ |
| `src/pus/pus.c` | `195b8cf5…8c22293b` | identical | MATCH |
| `tests/unit/test_pus_s11_residual.c` | `33b43b96…3b42a13` | identical | MATCH |
| `CHANGELOG.md` | `a35e9ea4…0de8a4d7` | identical | MATCH |
| `README.md` | `5b119888…a5de9b6` | identical | MATCH |
| `docs/planning/sprint-plan.md` | `27a42bd3…7a29985d` | identical | MATCH |
| `docs/status/handover.md` | `ac374ec6…f4767cc11` | identical | MATCH |
| `docs/status/loop-audit.md` | `c36a9e64…8d4b2b41` | identical | MATCH |
| `docs/vv/test-spec/ts-1-protocol-core.md` | `7ceaa4b5…9bacce95` | identical | MATCH |
| `docs/vv/verification-results.md` | `8ab0fab7…010450bd` | identical | MATCH |

## 1.1 Delta inventory

```text
 M CHANGELOG.md
 M README.md
 M docs/planning/sprint-plan.md
 M docs/status/handover.md
 M docs/status/loop-audit.md
 M docs/vv/test-spec/ts-1-protocol-core.md
 M docs/vv/verification-results.md
 M src/pus/pus.c
 M tests/unit/test_pus_s11_residual.c
?? docs/vv/evidence/2026-11-14-s1-progress-failure/
```

Production delta is 11 changed lines in one function (`src/pus/pus.c:449-474`). Everything else is tests and records.

## 2. Scope

**In scope.** (a) Correctness and mutual exclusivity of the S1 progress-verification emission in `FF_PusIngest`; (b) preservation of acceptance/start/completion and early-return behaviour; (c) re-entrancy, ordering and side effects introduced by moving the call after `handler(...)`; (d) discriminating power and production-path fidelity of the new tests; (e) conformance to the project's error-code, range and telemetry conventions; (f) consistency of the accompanying records with the implemented behaviour and with each other.

**Out of scope.** Requirement or interface-authority decisions (whether TM[1,6] *should* be emitted, the PUS-C profile, failure-code allocation); start-failure (TM[1,4]) behaviour; target/RTEMS/Renode/N18 evidence; timing, WCET, radiation, security; PA/N20 disposition; closure of NC-EXT-94; anything outside the listed files.

## 3. Method (read-only)

1. Full read of `FF_PusIngest`, `FF_PusSendVerification`, `FF_PusSendTm`, `ff_pus_find`, `FF_PalMutexLock`, the registered core handlers, `pus_types.h` and the ICD §4 S1/S6/S20 rows.
2. `git diff HEAD` review of every changed file; cross-check of each claim in `CHANGELOG.md`, `README.md`, `docs/vv/test-spec/ts-1-protocol-core.md`, `docs/vv/verification-results.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`.
3. **Empirical differential probe (no repository modification).** The HEAD version of `pus.c` was extracted with `git show HEAD:src/pus/pus.c`, compiled into `/tmp/opencode`, linked against the project archive with `pus.c.o` removed, and driven through `FF_PusIngest` with identical TCs as the working-tree build. This establishes, rather than assumes, that the new tests discriminate the defect (§5, Q5).
4. **Runtime inspection** with `gdb` of the PUS mutex state (`g_mutexes[g_pus_mutex].mtx.__data.__lock/__owner`) at handler entry and immediately before `pus.c:461`.
5. Independent re-execution of the retained test binaries (`build/doc-update/tests/test_pus_s11_residual`, `build/smoke-safe/tests/test_pus_s11_residual`) and of `tools/scripts/trace.py`.
6. `gcc -std=c11 -Wall -Wextra -Werror` compilation of both changed C files to `/tmp/opencode` (0 warnings), and `clang-format --dry-run --Werror` on both changed files.

**Not re-executed by me** (so not re-verified here): the full CMake configure/build, CTest 60/60, `lake build`, Renode/N18, Docker-based pinned clang-format 18, `make smoke`, `make gsw-selfcheck`. Those remain the author's claims, corroborated only where stated. I did not build into any repository build directory and did not clean anything.

## 4. Answers to the review questions

### Q1 — Is the emission logic correct and mutually exclusive for all handler outcomes?

**Yes.** `src/pus/pus.c:455-465` evaluates `rc` exactly once, after the handler, and performs **one** call:

```c
rc = handler(&tc, data, data_len);
...
if (ack_progress) {
    FF_PalMutexUnlock(g_pus_mutex);
    (void)FF_PusSendVerification(&tc, FF_PUS_STAGE_PROGRESS, (rc == FF_SUCCESS) ? 1u : 0u,
                                 (rc == FF_SUCCESS) ? 0u : FF_PUS_FAIL_EXECUTION);
    FF_PalMutexLock(g_pus_mutex, 0u);
}
```

- `rc == FF_SUCCESS` → `success=1, code=0` → `FF_PUS_STAGE_PROGRESS` maps to subservice 5 (`pus.c:316-318`).
- **Any** other value (`FF_FAILURE`, `FF_NOT_FOUND`, `FF_INVALID_ARGUMENT`, `FF_NOT_IMPLEMENTED`, `FF_BAD_SIZE`, `FF_TIMEOUT`, …) → `success=0, code=20` → subservice 6.
- The ternary is total over `FF_Status_t`; there is no unhandled outcome and no fall-through to a second emission.
- `ack_progress == false` → the block is skipped entirely (no 1,5, no 1,6). Verified empirically.

Measured (working tree, `FF_PusBackendOpen`, sink counting TM service/subservice):

| Scenario | 1,1 | 1,2 | 1,3 | 1,5 | 1,6 (code) | 1,7 | 1,8 (code) |
| -------- | --- | --- | --- | --- | ---------- | --- | ---------- |
| TC[17,1] success, all acks | 1 | 0 | 1 | **1** | 0 | 1 | 0 |
| registered handler → `FF_BAD_SIZE`, all acks | 1 | 0 | 1 | **0** | **1 (20)** | 0 | 1 (20) |
| handler → `FF_INVALID_ARGUMENT`, progress OFF | 1 | 0 | 0 | 0 | 0 | 0 | 1 (20) |
| handler → `FF_INVALID_ARGUMENT`, progress only | 0 | 0 | 0 | 0 | **1 (20)** | 0 | 0 |
| S11 stub → `FF_NOT_IMPLEMENTED`, all acks | 1 | 0 | 1 | 0 | **1 (20)** | 0 | 1 (20) |
| TC[3,2] handler → `FF_NOT_FOUND`, all acks | 1 | 0 | 1 | 0 | **1 (20)** | 0 | 1 (20) |
| unknown service 77, all acks | 1 | 0 | 0 | 0 | 0 | 0 | 1 (9) |
| illegal APID, all acks | 0 | 1 | 0 | 0 | 0 | 0 | 0 |

### Q2 — Re-entrancy, ordering, side effects from moving the call after `handler(...)`

**No new re-entrancy hazard; one real, unrecorded ordering change.**

- The handler still runs inside the same nominal critical section as before (the move does not change when the handler executes relative to the lock/unlock sequence at `pus.c:449-453`), and the progress emission still uses the same `unlock → send → relock` pattern that the acceptance, start and completion stages already use. The handler is not re-entered; `FF_PusSendVerification` is called exactly once, after the handler returns, on the same thread.
- **Ordering change (measured).** TM[1,5] now follows any telemetry the handler emits, instead of preceding it:

  | TC | Pre-change (HEAD) order | Post-change order |
  | -- | ----------------------- | ----------------- |
  | TC[17,1] ping, all acks | `[1,1] [1,3] [1,5] [17,2] [1,7]` | `[1,1] [1,3] [17,2] [1,5] [1,7]` |
  | TC[9,2] report, all acks | `[1,1] [1,3] [1,5] [9,2] [1,7]` | `[1,1] [1,3] [9,2] [1,5] [1,7]` |

  Every core handler emits TM (`src/pus/pus_services.c:350, 486, 544, 564, 606, 630, 672, 736, 759, 778`), so this affects S3, S6, S8, S9, S17 and S20 on the wire. It is a consequence of the chosen semantics and not itself a defect, but it is **not recorded** in the controlled ICD, the CHANGELOG or the TS-1 case (finding **M-02**). No test or ground-station code depends on the old order (`tools/gsw/ff_gsw.py` has no S1 subservice logic; `tests/unit/test_pus.c:558-600` asserts presence/counts only, but its comment still reads "1,5 progress success - 17,2 ping reply", now factually reversed).
- **No duplication or interleaving of verification TM**: the emission is a single call site, and `FF_PusSendVerification` builds one frame per call.
- **Pre-existing lock-discipline defect surfaced by this review** (not introduced by the delta): see **M-03**. The PUS "mutex" is not effective during handler dispatch; a handler that emits TM silently releases the ingest critical section. The delta neither fixes nor materially worsens this (it substitutes one `unlock → send → relock` window for another), but the change narrative should not be read as strengthening serialization.

### Q3 — Can any path emit both 1,5 and 1,6, or zero when one is required?

**Never both.** One call site, one ternary, one frame. Measured across eight scenarios: no scenario produced both, and none produced two of the same subservice.

**Zero progress TM occurs in exactly two situations, both pre-existing and both unchanged by this delta:**
1. handler not found (`pus.c:437-447`, early return before the progress stage) — even with `ack_progress` set, only TM[1,8] (or nothing) is emitted;
2. `FF_PusSendVerification`/`FF_PusSendTm` fails internally (encode error, `g_backend == NULL`, no sink) — the status is discarded by `(void)`.

Situation 1 is intentional and documented in the test comment (`tests/unit/test_pus_s11_residual.c:193-196`); situation 2 is a pre-existing framework-wide property, not specific to this delta.

### Q4 — Are the early returns (unknown service/subservice, illegal APID) genuinely unchanged?

**Yes.** `git diff HEAD` touches only (a) the removal of `rc = handler(...)` from below the progress block and its insertion above it, and (b) the two arguments of the progress call. The illegal-APID block (`pus.c:420-429`) and the handler-not-found block (`pus.c:437-447`) are byte-identical to HEAD. Confirmed behaviourally: illegal APID → TM[1,2] only (no 1,5/1,6); unknown service → TM[1,8] only (no 1,5/1,6). The existing test oracle `test_s1_missing_subservices_coverage` (`test_pus_s11_residual.c:201-205`) still asserts `1,3 == 0` and `1,5 == 0` for the unknown-subservice path and still passes.

### Q5 — Do the tests exercise the production path, and do the oracles discriminate? Are existing assertions weakened?

**Production path: yes.** All cases go through `FF_PusIngest` with the open CCSDS backend and the *production* core-service registration; no handler is re-implemented and no verification function is called directly to fabricate the result. `stream_code_byte` parses the emitted frame stream, so the oracle reads the wire artefact rather than a return value.

**Discrimination: proven, not assumed.** I built the HEAD version of `pus.c` (pre-change) into an isolated probe and drove the same failing-handler TC through both builds:

| Build | 1,5 | 1,6 |
| ----- | --- | --- |
| HEAD (pre-change) | **1** | **0** |
| Working tree (post-change) | **0** | **1** |

So `FF_TEST_ASSERT(stream_count(1u, 6u) == 1u)` and `stream_count(1u, 5u) == 0u` fail against the pre-change code and pass against the new code: the regression genuinely pins the defect rather than the implementation. The code-octet assertion (`== FF_PUS_FAIL_EXECUTION`) also discriminates, because the pre-change 1,5 packet carries code 0 and the frame would not match subservice 6 at all.

**Existing assertions: none weakened.** The diff replaces one assertion (`stream_count(1u, 8u) == 1u`) with four stricter ones and adds a new test function; no assertion anywhere in the file was deleted or relaxed. `FF_TEST_MAIN` registration is correct (`test_pus_s11_residual.c:706-712`).

**Gaps in test completeness** (see m-03, m-04): the `ack_progress == false` branch is not asserted at ingest level, and the helper hardcodes frame offsets.

### Q6 — Error-code, range and telemetry conventions

- Error code: `FF_PUS_FAIL_EXECUTION` (`20u`) is the existing macro (`src/pus/pus_types.h:46`) and matches the completion-stage convention already used at `pus.c:469-470` and documented for S6 refusals in ICD §4. Consistent.
- Range: `success` is `uint8_t` 1/0, `code` `uint8_t`; no narrowing, no signed/unsigned mix, no overflow path.
- Telemetry: `FF_PusSendVerification` already maps PROGRESS→{5,6}; TM[1,5] carries code 0 as the ICD row requires (`docs/interfaces/ICD.md:105`); payload layout and Request-ID construction are untouched.
- The choice of a *single* failure code for every handler outcome is consistent with the project's existing (interface-authority-owned) convention, though codes 2/8/9/12 exist and remain unused for handler results (comment **c-01**).

### Q7 — Deviations from the stated scope in docs/README/CHANGELOG

The stated scope is respected: one production function, one test file, no requirement text, no `traceability.yaml` change (I confirmed `make trace`-equivalent: `trace.py` PASS, 120/76/30/14/68), no NC closure claimed, NC-EXT-94 explicitly left OPEN, no PUS-C compliance claim, no start-failure behaviour invented. `README.md:184` and `ICD.md` rev 1.9 were correctly de-scoped to "subservice 4 has no emission path".

Deviations found are **record accuracy**, not scope creep: the TS-1 coverage totals (**M-01**), the recorded test check count (m-01), the "repo-committed" evidence directory wording (m-02), and the unrecorded TM[1,5] ordering/semantics change (**M-02**).

## 5. Findings

| ID | Severity | Location | Summary |
| -- | -------- | -------- | ------- |
| M-01 | **Major** | `docs/vv/test-spec/ts-1-protocol-core.md:289-303`, `:349`; `docs/vv/verification-results.md:993-994`; `docs/status/loop-audit.md:458`, `:802` | TS-1 §3.1 coverage summary not updated, and the covered/gap split asserted in three records is arithmetically wrong |
| M-02 | **Major** | `docs/interfaces/ICD.md:105`; `CHANGELOG.md` (new NC-EXT-94 section); `docs/vv/test-spec/ts-1-protocol-core.md:147` | TM[1,5] ordering and semantics redefined on the wire without an interface record update |
| M-03 | **Major** (pre-existing, out of delta) | `src/pus/pus.c:172`, `:202`, `:127-136`; `src/core/pal/pal_posix.c:350-365` | PUS core mutex is not effective: nested `FF_PusSendTm` releases the caller's critical section; every lock result is ignored |
| m-01 | Minor | `docs/vv/verification-results.md:1050`; `docs/status/loop-audit.md:795` | Recorded check count (1176) does not match the executed artifact (1206) |
| m-02 | Minor | `docs/vv/verification-results.md:1011` | Evidence directory described as "repo-committed" while it is untracked |
| m-03 | Minor | `tests/unit/test_pus_s11_residual.c:58-78` | `stream_code_byte` hardcodes frame offsets, uses a 0xFF sentinel, and contains a dead loop guard |
| m-04 | Minor | `tests/unit/test_pus_s11_residual.c:393-472` | No ingest-level case for `ack_progress` clear with a failing handler (N22 out-of-bound completeness) |
| m-05 | Minor | `src/pus/pus.c:457-459` | Comment states an unconditional invariant that has two exceptions |
| m-06 | Minor | `docs/architecture/pus-c-from-scratch-assessment.md:7` vs `:284` | Header still says Revision 0.3 while the history (and the CHANGELOG citation) says rev 0.4 |
| m-07 | Minor | `docs/interfaces/ICD.md` rev 1.9 (HEAD `fc68b9d`); `docs/architecture/pus-c-from-scratch-assessment.md` rev 0.4 | Controlled records were committed asserting the new wire behaviour while HEAD's code did not implement it |
| c-01 | Comment | `src/pus/pus.c:462-463`, `:469-470` | All handler errors collapse to code 20; codes 2/8/9/12 remain unused |
| c-02 | Comment | `src/pus/pus.c:460-471` | One command can now produce two failure TM (1,6 then 1,8); ground must not treat 1,6 as terminal |
| c-03 | Comment | `src/pus/pus.c:437-447`; `docs/interfaces/ICD.md:105-107` | 1,5/1,6 are emitted only when a handler is dispatched; the ICD row does not say so |
| c-04 | Comment (pre-existing, out of delta) | `src/pus/pus.c:438-439` | Unknown *service* is reported as `FF_PUS_FAIL_UNKNOWN_SUBSERVICE` (9) unless `service == 0`; measured for service 77 |
| c-05 | Comment | `tests/unit/test_pus_s11_residual.c:437-447` | Third sub-case re-implements `build_tc` instead of parameterising its ack flags |

### M-01 — TS-1 coverage summary is stale and the new covered/gap split is wrong (Major)

`docs/vv/test-spec/ts-1-protocol-core.md:349` (revision 1.14) states: "§3.1 totals updated (105 cases = 76 `covered` + 29 `gap`)". The §3.1 table at `:289-303` was **not** updated: it still reads `Total cases 104`, `Nominal 48`, `Out-of-bound 51`, `Mixed nominal + out-of-bound 5`, `covered 76`, `gap 28`.

Counting the case rows in the reviewed file gives **105 rows**: 77 `covered` (10 `host covered (…)` + 67 `covered`/`covered (…)`) and 28 `gap`. The new row `N22-TS1-105` (`:147`) carries the status `host covered (NC-EXT-94 implementation; …)`, which the document's own convention (`:305-308`) counts as `covered`. Therefore the correct totals are **105 = 77 covered + 28 gap**, `Mixed` 6, `OOB` unchanged — not `76 + 29`.

The same wrong split is repeated in `docs/vv/verification-results.md:993-994` ("raising TS-1 to 105 cases = 76 covered + 29 gap") and in `docs/status/loop-audit.md:458` and `:802`. Three records therefore assert a coverage figure that contradicts the table they cite. Because §3.1 is the summary a PA/IV&V reader uses, and because revision 1.14 explicitly claims the update was made, this must be corrected before the delta is committed (N8/N16, and the `N22` row of the loop-audit). Fix: update the §3.1 table to 105/49/51/6 (nominal/OOB/mixed as applicable), set `covered 77` and `gap 28`, and correct the three prose statements.

### M-02 — TM[1,5] ordering and semantics changed on the wire without an interface record update (Major)

The delta is presented as "acceptance, start and completion behaviour unchanged; TM[1,6] is now emitted on error". Two interface-visible consequences of the same edit are not recorded anywhere:

1. **Ordering.** TM[1,5] now follows the handler's data telemetry (measured: `[1,1][1,3][17,2][1,5][1,7]` vs the previous `[1,1][1,3][1,5][17,2][1,7]`). A ground segment or procedure that treats 1,5 as "request accepted, data TM will follow" now observes the inverse for S3/S6/S8/S9/S17/S20.
2. **Semantics.** TM[1,5] changes meaning from "in progress" to "executed successfully". For long-running handlers (S6 memory dump, S8 function call) progress telemetry no longer gives an intermediate indication, and when both `ack_progress` and `ack_completion` are set, 1,5 and 1,7 are now redundant.

The controlled ICD row (`docs/interfaces/ICD.md:105`) still reads `1 | 3 / 5 | start / progress success | TM | errcode 0` with no ordering or timing statement, and TS-1 `N22-TS1-105` does not mention ordering. Recommendation: add one sentence to the ICD S1 row (e.g. "progress verification is emitted after handler execution; when the handler emits telemetry, TM[1,5]/TM[1,6] follow it") and record the semantic redefinition in the CHANGELOG entry, then confirm with the interface authority, since ICD §4 is interface-authority owned. No code change is required for this finding.

### M-03 — PUS core mutex is not effective (Major, pre-existing; not introduced by this delta)

Found while answering Q2; reported because it bears directly on "re-entrancy and side effects" and on the framework's NC-EXT-05 serialization claim (`src/pus/pus.c:28-30`). Three interacting defects:

1. `FF_PusSendTm` (`pus.c:172`) calls `FF_PalMutexLock(g_pus_mutex, 0u)` and **ignores the result**, then unconditionally calls `FF_PalMutexUnlock` (`pus.c:202`). `FF_PalMutexLock` with `timeout_ms = 0` never blocks (`pal_posix.c:356-364`: `pthread_mutex_timedlock` with a deadline computed from `CLOCK_MONOTONIC` while glibc compares against `CLOCK_REALTIME` → immediate `ETIMEDOUT`), so a same-thread re-lock fails silently and the following unlock **succeeds** — releasing the lock that the caller (`FF_PusIngest`) believes it holds.
2. `ff_pus_find` (`pus.c:127-136`) locks a mutex that `FF_PusIngest` already holds and then unlocks it unconditionally, silently ending the ingest critical section.
3. Every other caller of `FF_PalMutexLock(…, 0u)` likewise ignores the status, so under cross-thread contention the "critical sections" are not enforced at all.

Measured with `gdb` on the reviewed build: at handler entry `__lock = 1` with `__owner` equal to the current LWP (the handler is invoked with the mutex held); immediately before `pus.c:461`, for a handler that emits telemetry, `__lock = 0` — the nested emission has already released the ingest lock. Observable single-threaded behaviour remains correct (the `unlock → send → relock` pairs are balanced from the application's point of view, because the failed second unlock returns `EPERM` and is ignored), which is why this has not surfaced in tests.

Impact: concurrent `FF_PusIngest`/`FF_PusSendTm` calls from different tasks can interleave TM encoding, the TM counter (`FF_PusNextTmCounter`) and the Request-ID counter, defeating the stated serialization. This is a separate, pre-existing defect and must not be booked against this delta; it should be opened as an NC with the same discipline as NC-EXT-243 (which already records the unsynchronised `s_sequence_count`). It does not block the S1 progress-failure delta, whose single-threaded behaviour is correct.

### Minor findings

- **m-01 — Recorded check count does not match the artifact.** `docs/vv/verification-results.md:1050` and `docs/status/loop-audit.md:795` record `checks=1176, failures=0` for `test_pus_s11_residual`. Both retained binaries report `checks=1206 failures=0` (I executed them: `build/doc-update` and `build/smoke-safe`, exit 0). 1176 is the pre-change count carried over from the previous cycle (`:956`, `:982`). Correct the number, or state that it is the previous-cycle figure.
- **m-02 — "repo-committed evidence directory".** `docs/vv/verification-results.md:1011` describes `docs/vv/evidence/2026-11-14-s1-progress-failure/` as a "repo-committed evidence directory"; the directory is untracked in the reviewed working tree. Reword to "retained evidence directory, to be committed with this delta" until it is actually committed (N11).
- **m-03 — `stream_code_byte` robustness.** `tests/unit/test_pus_s11_residual.c:58-78`: offsets `pos+7`, `pos+8`, `pos+27` are hardcoded and undocumented against the type constants; a change to the verification payload layout would silently read a different octet (the `0xFF` sentinel would only catch a *missing* packet, not a wrong offset). The guard `if ((pos + 6u) > g_rx_count) break;` is dead, because the loop condition already requires `pos + 28 <= g_rx_count`. Recommend named offsets derived from `FF_PUS_TM_SEC_HDR_LEN`/`FF_PUS_TC_SEC_HDR_LEN` plus a comment tying octet 27 to `extended[8]` in `FF_PusSendVerification`.
- **m-04 — Missing negative case for `ack_progress` clear.** The delta's stated contract includes "when `ack_progress` is set"; the complementary branch (failing handler with `ack_progress` clear → no 1,5 and no 1,6, completion failure still emitted) is not asserted anywhere at ingest level. I verified it empirically (measured: `1,6 = 0`, `1,8 = 1`), but N22 asks for the applicable out-of-bound case with an explicit expected result. One three-line sub-case in `test_s1_progress_fail_regression` closes this.
- **m-05 — Overstated invariant in the code comment.** `src/pus/pus.c:457-459` states "Exactly one progress verification is sent; never both 1,5 and 1,6." True for the handler-found path; the early return at `:437-447` sends none, and a failed `FF_PusSendVerification` sends none. Scope the sentence ("when a handler is dispatched and the verification is emitted successfully").
- **m-06 — Revision header lags its own history.** `docs/architecture/pus-c-from-scratch-assessment.md:7` still reads `Revision 0.3` while `:284` records revision 0.4 dated 2026-11-14, and `CHANGELOG.md` cites "rev 0.4". Update the header.
- **m-07 — Records committed ahead of the code.** `fc68b9d` (HEAD) committed ICD rev 1.9 (`docs/interfaces/ICD.md:255`) and the assessment rev 0.4 stating "Wire behaviour changed: TM[1,6] is now emitted on handler error with `ack_progress`" while HEAD's `pus.c` did not implement it. The reviewed delta closes the gap, but the intermediate committed baseline was internally inconsistent; for a class-B baseline, documentation asserting an unimplemented wire behaviour is exactly the class of defect NC-EXT-94 was opened for. Prefer committing code and interface record together, or annotate such commits as "implementation pending".

### Comments

- **c-01 — Failure-code granularity.** Every handler outcome maps to code 20. `FF_PUS_FAIL_ILLEGAL_LENGTH` (2), `FF_PUS_FAIL_BAD_DATA` (12), `FF_PUS_FAIL_UNKNOWN_SERVICE` (8) and `UNKNOWN_SUBSERVICE` (9) remain unused for handler results, so a `FF_BAD_SIZE` refusal and an execution failure are indistinguishable on the wire. This matches the existing S6 convention and the SRS-REQ-005 "gap" rows, and separating the causes is an interface-authority decision (NC-EXT-18 pattern) — recorded here so it is not mistaken for a defect of this delta.
- **c-02 — Two failure TM per command.** With both acks set, a failing handler now produces TM[1,6] *and* TM[1,8] (measured for the S11 stub, `FF_NOT_IMPLEMENTED`, `FF_INVALID_ARGUMENT`, `FF_NOT_FOUND`). PUS-C permits this, but ground procedures must not treat 1,6 as terminal or the completion stage will be lost. Worth a sentence in the operations/ground documentation.
- **c-03 — Dispatch-failure path emits no progress TM.** `pus.c:437-447` returns before the progress stage; a TC with `ack_progress` set for an unknown service/subservice yields only TM[1,8]. Intentional and unchanged, but the ICD S1 rows do not state that 1,5/1,6 exist only for dispatched commands.
- **c-04 — Pre-existing wrong code for unknown service.** `pus.c:438-439` selects `FF_PUS_FAIL_UNKNOWN_SERVICE` only when `service == 0`; any other unregistered service is reported as `UNKNOWN_SUBSERVICE` (measured: service 77 → TM[1,8] code 9). Out of scope for this delta and untested (`test_unknown_service_dispatch` asserts only the count), but it should be captured as an NC.
- **c-05 — Test helper duplication.** The third sub-case (`test_pus_s11_residual.c:437-447`) hand-builds the TC header because `build_tc` hardcodes all four ack flags. Adding an `ack_flags` parameter to `build_tc` would remove the duplication and keep the test focused on the oracle.

## 6. Verdict

**NEEDS REVISION** — the revisions required are **documentation-only** (M-01, M-02, m-01, m-02, m-06, m-07); no code change is required by this review.

The production change itself is **accepted with comments**: the emission is correct, total over all handler outcomes, mutually exclusive, correctly gated by `ack_progress`, preserves every early return, respects the project's error-code and telemetry conventions, and is pinned by tests that I proved to discriminate the pre-change defect. M-03 is a pre-existing framework defect that this delta neither introduces nor fixes and that must not block it.

I do not approve the delta for commit; commit authority and gate adjudication remain with the `aeronautics-space-fsw-engineer` (N7) and, for the interface record, with the interface authority.

## 7. Residual risks

1. **Interface record gap (M-02).** Until the ICD S1 row states the post-handler emission and the resulting ordering, a ground segment built from the ICD can still mis-correlate TM[1,5] with the service telemetry that now precedes it. This is an interface-authority risk, not a code risk.
2. **Unrecorded semantic redefinition of TM[1,5].** For long-running handlers, progress telemetry no longer indicates that execution started. If operations rely on that, the loss is silent (no TM is missing — only its timing changes).
3. **PUS serialization is not delivered (M-03).** Any future multi-task use of `FF_PusIngest`/`FF_PusSendTm` (e.g. command ingest task plus periodic HK emission) can interleave TM encoding and counter updates. Untested and unqualified; a concurrency fault here would be intermittent and target-dependent.
4. **Failure-code information loss (c-01).** Every handler refusal is code 20; a ground operator cannot distinguish a malformed payload from an execution failure. Accepted project-wide today, but it limits anomaly diagnosis.
5. **Test-oracle fragility (m-03).** The code-octet oracle depends on undocumented frame offsets; a future payload-layout change could make the assertion pass or fail for the wrong reason.
6. **Evidence records not re-verified by me.** CTest 60/60, `lake build`, pinned clang-format 18, Renode/N18, smoke and GSW results are the author's claims. I independently confirmed only: both changed files compile warning-free under `-Wall -Wextra -Werror`; `clang-format 23.1.3 --dry-run --Werror` passes on both; `trace.py` reports PASS (120/76/30/14/68); the retained `test_pus_s11_residual` binaries pass with 1206 checks.
7. **Scope of this review.** No target, timing, WCET, stack, radiation, security, RTEMS, Renode-fidelity or qualification conclusion is drawn, and no start-failure (TM[1,4]) behaviour was assessed for adequacy (it is intentionally not emitted).

## 8. Reporting per the FSW review protocol

**Facts** (verified in this session): all nine candidate hashes and HEAD match; the emission is a single call site with a total ternary; no scenario emits both 1,5 and 1,6; `ack_progress == false` suppresses both; early returns are byte-identical to HEAD; the pre-change build emits TM[1,5] for a failing handler and the post-change build emits TM[1,6] (code 20) — hence the new assertions discriminate; TM[1,5] ordering relative to handler telemetry changed; the TS-1 §3.1 table is stale and the 76/29 split contradicts the table's own status tokens; the recorded check count is 1176 against an artifact that reports 1206; the PUS mutex is released by a nested `FF_PusSendTm` inside a handler.

**Assumptions** (owner, impact, validation): (a) `build/smoke-safe` and `build/doc-update` were built from the reviewed working tree — supported by mtimes (binaries 12:44/12:47, `pus.c` 12:21) and by the pre/post differential behaviour; if either binary were stale, the discrimination proof would need re-running (owner: FSW). (b) The untracked evidence directory is intended to be committed with this delta (owner: FSW; if not, N11 retention is incomplete). (c) The ICD S1 row is interface-authority owned and must not be edited unilaterally (owner: interface authority).

**Inferences**: the 1176 figure was carried over from the previous cycle rather than measured, because it matches the pre-change assertion count exactly and the delta adds ~30 assertions. The 76/29 split appears to have been derived by assuming the new row is a `gap` row without checking its status token. Both are inferences from arithmetic and are correctable without further evidence.

**Recommendations** (decision owner, prerequisites, trade-offs): fix M-01 and the record figures before commit (FSW, ~15 minutes, no code change); raise M-02 with the interface authority and record the ordering/semantics sentence (interface authority + FSW); open M-03 as an NC with the same treatment as NC-EXT-243 (FSW + PA); add the `ack_progress`-clear sub-case and parameterise `build_tc` (FSW, optional under N21 but cheap).

**Confidence**: **High** on the code-correctness conclusions (Q1–Q5), because they are backed by differential execution of both builds plus static reading. **High** on M-01 and m-01 (direct counting and execution). **Medium** on M-03's practical impact, because it is established by symbol-level inspection of a single-threaded build; the cross-thread consequence is reasoned, not measured. **Medium** on M-02's operational consequence, because no ground-segment behaviour was available to assess.

**Open gaps**: no target/HIL evidence; no Renode re-run; pinned clang-format 18 and `lake build` not re-executed; ICD/interface-authority disposition of the S1 row still pending; NC-EXT-94 remains OPEN; `docs/reviews/README.md` index entry for this report not created (outside this review's write restriction).
