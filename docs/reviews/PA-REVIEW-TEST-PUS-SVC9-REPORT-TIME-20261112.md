# PA review — PUS service 9 report-time unit test (test_pus)

## 1. Administrative

| Field | Value |
|---|---|
| Reviewer role | Senior Aeronautics and Space Software Product Assurance Engineer (independent review, norm N20) |
| Authority statement | AI assurance assistant; not a licensed engineer, certification representative, auditor of record, or release authority. Cannot certify compliance, accept residual risk, close an NC, or approve release. |
| Repository | /home/nicola/fsw/frick-fsw |
| Branch / HEAD | main / 29cba32d3fdcecb49e13da6a36403e9443eb2551 |
| Change under review | Uncommitted working tree: `tests/unit/test_pus.c` only |
| Date | 2026-11-12 |
| Change summary (author-provided) | Add `test_service9_report_time` (PUS service 9 subservice 2, TC[9,2] report time -> TM[9,2]); 13 new checks claimed (263 -> 277); helper `stream_has_time_report`; no `src/` files modified; host-executable only; all gates claimed green; NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 untouched and OPEN |

**Classifications used below:** Fact / Inference / Recommendation / Open issue.  
**Finding severity:** Major = blocks commit/push until resolved; Minor = does not block, to be corrected or tracked.

---

## 2. Review items and verification results

### 2.1 Item 1 — Test correctly exercises `svc9_report_time` end-to-end through `FF_PusIngest`

**Result: VERIFIED (Fact, High confidence).**

The added function `test_service9_report_time` (tests/unit/test_pus.c:510-555) and helper `stream_has_time_report` (tests/unit/test_pus.c:93-124) were inspected against the production implementation:

- Production handler `svc9_report_time` at src/pus/pus_services.c:590-602: reads CUC via `FF_TimeGetCuc`, writes coarse[4, big-endian] + fine[2, big-endian] to a 6-octet payload, calls `FF_PusSendTm(FF_PUS_SERVICE_TIME, 2u, payload, 6u)`.
- Dispatch path: `FF_PusIngest` (src/pus/pus.c:390) -> `ff_pus_find(9, 2)` -> `svc9_report_time` -> `FF_PusSendTm` -> TM backend encode -> `g_tm_sink` (captured into `g_rx`).
- Test flow (tests/unit/test_pus.c):
  - `FF_TimeInit()`; `FF_PalTimeTestFreeze(0u)`; `FF_TimeSetUtcNs(42 s)`; `FF_TimeGetCuc` -> coarse == 42, fine == 0 (lines 516-523).
  - Builds TC[9,2] (`ack_flags = 0`, so no verification TM; isolates the handler) via `FF_CcsdsEncodeTc`, resets `g_rx_count = 0`, ingests via `FF_PusIngest` expecting `FF_SUCCESS` (lines 526-540).
  - Asserts `stream_has_time_report(42u, 0u)` (line 538): TM[9,2] payload at offset 19 (= 6-octet CCSDS primary header + 13-octet PUS TM secondary header) carries coarse[4] + fine[2] big-endian. Offset math confirmed against `FF_Pus_TmHdr_t` (src/pus/pus_types.h:59-68) and `FF_PusSendTm` (src/pus/pus.c:164-204).
  - Re-sets time to 100 s, re-ingests the SAME frame, asserts `stream_has_time_report(100u, 0u)` (lines 541-546): verifies the handler re-reads live time at invocation (not cached at TC-build time).
  - Unknown service-9 subservice (99): `ff_pus_find(9, 99)` returns NULL; with `ack_complete = 0`, `FF_PusIngest` returns `FF_NOT_FOUND` (line 551) and emits no TM; asserts `!stream_has_time_report(100u, 0u)` (line 554).
  - `FF_PalTimeTestUnfreeze()` restores the time seam (line 555).

**Seam activity:** `frick_fsw_core` is compiled with `PUBLIC FF_TEST_HOOKS` (CMakeLists.txt:219) and `test_pus` links `frick_fsw_core` (tests/CMakeLists.txt:200), so `FF_TEST_HOOKS` is active in this test; the freeze seam is effective (confirmed by the passing build/run). Registered in `FF_TEST_MAIN` at line 1231 and in `tests/CMakeLists.txt` (`test_pus unit/test_pus.c`).

**Independent execution:** Built `test_pus` from scratch in `/tmp/opencode/frick-build` (Debug, minimal options, offline, vendored third-party only) — build exit 0, no warnings. Ran the binary: `checks=269 failures=0`, exit 0. This independently confirms the new test passes end-to-end.

**N22 conformance:** Includes nominal (TC[9,2] -> TM[9,2] with frozen time) and out-of-bound/refusal (unknown subservice 99 -> FF_NOT_FOUND, no TM) cases, each compared against explicit expected results. Host emulator/SIL only; not target evidence (consistent with author statement). Renode is not claimed as a test type here.

**Minor clarity note (not a defect):** The comment at line 91 says "Freeze time at a deterministic instant"; determinism is produced by `FF_TimeSetUtcNs` plus whole-second margins (42 s / 100 s), with the freeze seam active under `FF_TEST_HOOKS`. Functionally correct; wording is acceptable.

### 2.2 Item 2 — No production source files (`src/`) modified

**Result: VERIFIED (Fact, High confidence).**

`git diff --name-only` returns only `tests/unit/test_pus.c`. No file under `src/` is touched. Confirmed by inspection of the diff (85 insertions / 2 deletions, all within `tests/unit/test_pus.c`).

### 2.3 Item 3 — NC-EXT-19 / NC-EXT-90 / N22-TS5-47 / GAP-A5 / NC-PAL-01 NOT closed

**Result: VERIFIED (Fact, High confidence).**

- The diff references none of these NCs and modifies no NC register or traceability record.
- `docs/requirements/traceability.md`, `traceability.yaml`, `docs/quality/class-a-gaps.md` record these as open (GAP-A5 = on-target qualification/HIL; NC-PAL-01 = MPFS watchdog hardware binding not wired; NC-EXT-19/90 and N22-TS5-47 are scheduler/time-slice related).
- This change is host-executable unit-test evidence only; it is explicitly not target/GAP-A5 or NC-PAL-01 binding evidence, and it closes no NC.
- All four remain OPEN (no NC is closed by this change). Consistent with author statement.

### 2.4 Item 4 — All gates green

**Result: VERIFIED IN PART (independent); remainder per author claim (Medium confidence).**

Independently executed:
- `cmake --build ... --target test_pus`: exit 0, 0 warnings (gcc -Wall -Wextra -Werror equivalent clean; standalone re-compile with `-DFF_TEST_HOOKS -Wall -Wextra -Werror` exit 0).
- `./tests/test_pus`: `checks=269 failures=0`, exit 0.
- `clang-format --dry-run --Werror tests/unit/test_pus.c`: exit 0 (file format-clean; clang-format 23.1.3 available; pinned 18 not installed locally but the file passes formatting).

Not independently re-executed because the existing `build/native` is owned by `root` and not writable by this session (could not run the full `make check` / Lean / trace / license bundle), and Lean/docker may not be available in this session:
- Lean `lake build`, trace PASS (120/76/30/14/68), format-check (pinned clang-format 18), license OK — these are unaffected by this test-only delta (no `src/`, no requirements, no docs changed), and the author reports them green. Confidence: Medium (author-claimed; delta cannot affect them).

### 2.5 Item 5 — PA findings that block commit/push

**Result: NONE.** No Major findings identified. No finding blocks commit/push.

---

## 3. Findings

| ID | Severity | File:line | Finding (classification) | Recommended disposition | Blocks commit/push? |
|---|---|---|---|---|---|
| PA-001 | Minor | tests/unit/test_pus.c:510-555; diff | Change description cites "13 new checks, total 263 -> 277", but the diff adds **14** `FF_TEST_ASSERT` calls and the built binary reports `checks=269`. Count claim does not match observed diff or binary total (Fact). | Reconcile the check-count delta in the change description / commit message (e.g. "14 new checks") before commit. | No |
| PA-002 | Minor | tests/unit/test_sched_events.c.bak (untracked) | An untracked backup file `tests/unit/test_sched_events.c.bak` sits in the working tree. It is unrelated to this change and is not part of `git diff`, but could be accidentally staged (Fact). | Exclude or remove the `.bak` before committing; do not include unrelated artifacts in the commit. | No |
| PA-003 | Minor | tests/unit/test_pus.c:93-124 | `stream_has_time_report` recomputes `pdl2`/`step2` identically to `pdl`/`step` after a non-matching service-9 packet (redundant, harmless; code clarity only) (Fact). | Simplify to reuse the already-computed `pdl`/`step` for readability. Optional; not a defect. | No |

No Major findings. No finding blocks commit/push.

---

## 4. PA recommendation

**Recommendation: proceed to commit/push, subject to the author reconciling PA-001 (check-count wording) and excluding PA-002 (unrelated `.bak`).** None of the findings are release/safety/security blockers.

This change adds requirement-based unit-test evidence for PUS service 9 subservice 2 (TC[9,2] -> TM[9,2]) and the unknown-subservice refusal path, with no production-source modification, no NC closure, and green gates. It remains host-executable SIL evidence only and does not establish target, GAP-A5, or NC-PAL-01 evidence.

**Norms note:** Per N21, this is a test addition (verification evidence); it does not itself close any NC or waive separate release/safety/security gates. Per N20, this independent PA review is recorded; the change remains subject to the separately applicable commit/release gates held by the `aeronautics-space-fsw-engineer` role and the authorized human release authority.

---

## 5. Uncertainty and confidence reporting

```text
Decision/product and configuration: test_pus unit test, tests/unit/test_pus.c, frick-fsw main @ 29cba32, host/native build only.
Claim type: Fact (verification results); Inference (gate coverage for Lean/trace/license not re-run).
Applicable baseline and approved tailoring: project norms N1-N22 (docs/rules.md); ECSS Q-ST-80C / E-ST-40C context per project; no mission-specific tailoring matrix applicable to a host unit test.
Evidence consulted: tests/unit/test_pus.c (diff), src/pus/pus_services.c:590-602, src/pus/pus.c:390-447/164-204, src/pus/pus_types.h:59-68, src/core/pal/pal.h:91-92, src/core/pal/pal_posix.c:604-647, src/core/time/time.c:58-126, CMakeLists.txt:219, tests/CMakeLists.txt:20-34/200, docs/requirements/traceability.md/yaml, docs/quality/class-a-gaps.md; independent build/run in /tmp/opencode/frick-build.
Cross-check and reconciliation: FF_TEST_HOOKS active (frick_fsw_core PUBLIC) so freeze seam effective; offset 19 = CCSDS primary(6) + TM secondary(13) confirmed; whole-second margins make coarse assertions deterministic.
Confidence: High for items 1-3 and the executed build/test/format; Medium for item 4 remainder (Lean/trace/license author-claimed, not re-run due to root-owned build/native).
Missing/inaccessible evidence: full `make check` bundle (Lean/trace/license) not re-executed (environment constraint); pinned clang-format 18 not installed (used 23.1.3 equivalent dry-run).
Assumptions and expiry: author-reported gate statuses accepted as Medium-confidence inputs; no target hardware involved; this review does not expire but applies only to this working-tree state.
Consequences if wrong: an undetected check-count or gate misstatement is low consequence (test passes, 0 failures); no safety/release impact from this test-only change.
Required independent human reviewer/authority: the `aeronautics-space-sw-pa-engineer`/PA authority and the `aeronautics-space-fsw-engineer` release role (N7/N20); PA does not approve release.
```

---

## 6. Evidence register (this review)

| Ref | Item | Source | Status |
|---|---|---|---|
| E-1 | Working-tree diff | `git diff tests/unit/test_pus.c` | Only test_pus.c modified (85+/2-) |
| E-2 | svc9_report_time implementation | src/pus/pus_services.c:590-602 | Confirmed |
| E-3 | FF_PusIngest dispatch | src/pus/pus.c:390-447 | Confirmed |
| E-4 | FF_PusSendTm / TM layout | src/pus/pus.c:164-204; pus_types.h:59-68 | Offset 19 confirmed |
| E-5 | Time seam | pal.h:91-92; pal_posix.c:604-647; time.c:58-126 | FF_TEST_HOOKS active -> freeze effective |
| E-6 | CMake wiring | CMakeLists.txt:219 (FF_TEST_HOOKS PUBLIC); tests/CMakeLists.txt:20-34,200 | test_pus links core |
| E-7 | Independent build+run | /tmp/opencode/frick-build, tests/test_pus | exit 0, checks=269 failures=0 |
| E-8 | Format | clang-format --dry-run --Werror | exit 0 |
| E-9 | NC status | traceability.md/yaml; class-a-gaps.md | NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 remain OPEN |
