# Independent SW V&V Review — PUS service 8 execute-function unit test (`test_function_execute_bounded`)

| Field | Value |
|---|---|
| Review ID | VV-R2-20261112-PUS-SVC8-FUNCTION-EXECUTE |
| Reviewer profile | Senior Aeronautics and Space SW V&V Engineer (independent-context R2, norm N9/N10) |
| Repository | /home/nicola/fsw/frick-fsw, branch `main`, HEAD `3b04cb2` |
| Change under review | Uncommitted working tree: `tests/unit/test_pus.c` — added `test_function_execute_bounded` (19 new checks; 244 → 263 total checks; 0 failures). Verifies PUS TC[8,1] execute function end-to-end via `FF_PusRegisterFunction` + `FF_CcsdsEncodeTc` + `FF_PusIngest`. No `src/` production files modified. |
| Date | 2026-11-12 |
| Governing norms | N1-N22 (docs/rules.md); in particular N10 (test-script/tool qualification), N21 (sequencing), N22 (nominal + out-of-bound expected-result tests), N7 (commit/push authority). ECSS-E-ST-40C Rev.1 / ECSS-Q-ST-80C Rev.2 context. |
| Criticality | ECSS Class B target (project target; no controlled mission assignment evidenced — NC-EXT-5/11 class) |
| Scope | `tests/unit/test_pus.c` only (19 inserted checks, 3 new cases). Host-executable unit test; NOT target/HIL/Renode/QEMU; NOT GAP-A5/NC-PAL-01/OBCP; no production-code behavior change — verifies existing `svc8_function`. |

> **Authority statement.** This review is an independent analysis artifact. It has no approval, qualification, acceptance, certification, or flight-readiness authority. Commit/push authorization (norm N7), safety/security acceptance, PA sign-off (norm N20), and any residual-risk decision remain with the designated FSW engineer and the human authorities. This report may recommend "do not proceed"; it may not authorize proceeding.

## 1. Change under review (verified diff)

`git diff tests/unit/test_pus.c` adds `test_function_execute_bounded` (lines 1062-1147) plus recording fixtures (lines 44-60, 1062 comment block) and registers the test in `FF_TEST_MAIN` (line ~1154).

Three cases, each with explicit expected results (norm N22):

1. **Nominal execute** (lines 1085-1107): register handler for function id 7; build TC[8,1] payload `{7, 0xCC, 0xDD}`; ingest; assert `FF_PusIngest == FF_SUCCESS`, handler invoked (`g_func_exec_called`), `g_func_exec_len == 2`, `args == [0xCC, 0xDD]`, and TM[8,2] execution report emitted with `func_id=7, status=0`.
2. **Unknown function ID** (lines 1109-1127): payload `{99, 0x00}`; assert `FF_PusIngest == FF_NOT_FOUND`, handler not called.
3. **Bad size / empty payload** (lines 1129-1146): payload `NULL, 0`; assert `FF_CcsdsEncodeTc == FF_SUCCESS`, `FF_PusIngest == FF_BAD_SIZE`, handler not called.

Recording fixtures (lines 44-60): `g_func_test_handler` copies args, records `g_func_exec_id/len/args/called`, always returns `FF_SUCCESS`. `stream_has_function_report` (lines 62-90) scans the TM stream for service `FF_PUS_SERVICE_FUNCTION`, subservice `2`, and payload bytes `{func_id, status}` at secondary-header offset 19/20.

## 2. Evidence basis

| ID | Evidence | Relevance |
|---|---|---|
| E-1 | `tests/unit/test_pus.c:44-60, 1062-1147` | New test function, recording fixtures, TM-report scanner, 3 cases, 19 checks |
| E-2 | `src/pus/pus_services.c:546-564` | Production `svc8_function`: `len < 1 → FF_BAD_SIZE`; func_id dispatch; TM[8,2] `{func_id, status}` emission (`status = 0` on success, `1` otherwise); `FF_NOT_FOUND` when unregistered |
| E-3 | `src/pus/pus_services.c:164-185` | `FF_PusRegisterFunction`: duplicate → `FF_ALREADY_EXISTS`, full → `FF_NO_RESOURCE`, NULL fn → `FF_INVALID_ARGUMENT` |
| E-4 | `src/pus/pus.c:390-469` | `FF_PusIngest`: decode → find handler → invoke → return handler's `rc` (ack_flags=0 so no verification TM); mutex-guarded |
| E-5 | `src/transport/ccsds/ccsds.c:194-207` + `ccsds.h:19` | TM layout confirmed: primary header 6 bytes (`FF_CCSDS_HDR_LEN`), secondary header 13 bytes (`FF_PUS_TM_SEC_HDR_LEN`); service at byte 7, subservice at byte 8, payload at byte 19 |
| E-6 | `tests/ff_test.h:17-24` | `FF_TEST_ASSERT`: non-aborting, increments `ff_test_checks`/`ff_test_failures`; `main` returns non-zero iff any failure |
| E-7 | Reported gate results (context) | `make build` exit 0 / 0 warnings; CTest exit 0; Lean `lake build`; trace PASS (120/76/30/14/68); format-check PASS; license OK |
| E-8 | `tests/CMakeLists.txt:16, 110` | `test_pus` target from `unit/test_pus.c`, registered as CTest target; host-only |
| E-9 | Independent re-verification (this review) | Static comparison of test expected values against `svc8_function`, `FF_PusIngest`, TM encoder; all oracles confirmed byte-exact |

Rebuild/re-execution from current sources was **not** possible in this environment: `build/native` is owned by another user (`rm -rf` / reconfigure denied: "Permesso negato"), and the original CMakeCache references the stale `/workspace/build/native` path. The review therefore rests on source-level oracle verification (E-1…E-5, E-9) plus the reported gate results (E-7), which are internally consistent with the source inspection.

## 3. Findings

### VV-R2-001 — Minor — TM[8,2] error-status path (status=1) not exercised by any test

**Description.** `svc8_function` builds the TM[8,2] payload as `{func_id, status}` where `status = 0` when the handler returns `FF_SUCCESS` and `status = 1` otherwise (`src/pus/pus_services.c:556-559`). The test's recording handler `g_func_test_handler` always returns `FF_SUCCESS` (`tests/unit/test_pus.c:50-60`, line 59), so the nominal case only exercises `status = 0`. The handler-failure branch of the TM[8,2] construction — including the case where `FF_PusIngest` propagates a non-success `rc` (`src/pus/pus.c:460-469`) — is untested at unit level. For ECSS Class B service-8 execution reporting, the error-status telemetry path is safety-relevant evidence.

**File:line references.** `src/pus/pus_services.c:556-559`; `tests/unit/test_pus.c:50-60` (handler hard-coded `return FF_SUCCESS` at line 59), `1085-1107` (nominal case asserts `status=0` only).

**Observed result.** Nominal case passes: `stream_has_function_report(7u, 0u)` true (gate E-7). No test asserts `status=1` or a non-success handler return code propagated through `FF_PusIngest`.

**Blocks commit/push?** No.

**Disposition.** Add a unit case in which the registered handler returns a non-success code (e.g. `FF_ERROR` or `FF_INVALID_ARGUMENT`) and assert (a) `FF_PusIngest` returns that code and (b) TM[8,2] is emitted with `func_id` and `status=1`. Track as a verification improvement/NC under norm N21 (functional success path verified; error-reporting path tracked, does not stop this change).

---

### VV-R2-002 — Minor — Case-to-case state isolation is manual; function id 7 remains registered across error cases

**Description.** The test resets `g_func_exec_called` (lines 1079, 1110, 1130) and `g_rx_count` (lines 1099, 1124, 1143) between cases, but does **not** re-initialize the PUS function registry or unregister function id 7 between cases. `FF_PusInit()` is called once at the start (line 1072), after which id 7 stays registered for cases 2 and 3. Cases 2 (unknown id 99) and 3 (empty payload) do not depend on registration state, so there is no current interference; however, isolation relies on test ordering and manual field resets rather than a fresh registry per case. A later case that depends on "function not registered" would be order-dependent and fragile.

**File:line references.** `tests/unit/test_pus.c:1067-1084` (single `FF_PusInit` + single registration), `1109-1146` (error cases assume clean state without re-registration).

**Observed result.** All three cases pass in sequence (gate E-7); no cross-case contamination observed because cases 2/3 are registration-independent.

**Blocks commit/push?** No.

**Disposition.** Either re-run `FF_PusInit()`/`FF_PusRegisterCoreServices()` (or add an unregister seam if one exists) at the start of each case, or document that case ordering is intentional and id-7 registration is inert for cases 2 and 3. Track as a test-design improvement; norm N21 sequencing applies (functional defect none; tracking only).

---

### VV-R2-003 — Minor (informational) — `FF_TEST_ASSERT` is non-aborting; a failure in case 1 does not short-circuit cases 2/3

**Description.** `FF_TEST_ASSERT` increments a failure counter and prints but does **not** abort (`tests/ff_test.h:17-24`); `main` returns non-zero only after all tests run. A failure in the nominal case (e.g. `stream_has_function_report` false) would still allow cases 2 and 3 to execute and report. This does not produce false passes — all 19 new checks are evaluated and the binary exits non-zero if any fail — but it can obscure root-cause correlation when multiple checks fail in one run. Pre-existing harness characteristic, not introduced by this delta.

**File:line references.** `tests/ff_test.h:17-24`; `tests/unit/test_pus.c:1067-1147`.

**Observed result.** 0 failures reported (gate E-7); all 263 checks evaluated.

**Blocks commit/push?** No.

**Disposition.** Informational only; no change required for this commit. Consider abort-on-first-fail or per-case sub-reporting as a harness improvement outside this delta.

---

### VV-R2-004 — Minor (informational) — Tool qualification N10 is not triggered by this delta

**Description.** Norm N10 re-qualification triggers on a change to a qualified test script/V&V tool. This delta adds no test scripts and modifies no test script; `test_pus.c` is unit-test source (T3 context), not a script/tool. No T3 impact; no N10 re-qualification trigger. No new qualification evidence is required.

**File:line references.** `docs/vv/tool-qualification.md` (T3 row); `tests/unit/test_pus.c` (unchanged test harness, new test function only).

**Blocks commit/push?** No. No N10 action required for this change.

## 4. Confirmation observations (not findings)

- **Coverage sufficiency (question 1):** Sufficient at unit level per norm N22 — nominal execute plus two applicable out-of-bound/error cases (unknown ID → `FF_NOT_FOUND`, empty payload → `FF_BAD_SIZE`), each with explicit expected results and a traceable oracle. The only gap is the handler-failure `status=1` TM[8,2] path (VV-R2-001), tracked as an improvement, not a commit blocker.
- **Oracle correctness (question 2):** Correct and byte-exact, verified against source:
  - `FF_BAD_SIZE` for empty payload: `svc8_function` `if (len < 1u) return FF_BAD_SIZE;` (`pus_services.c:549-550`).
  - `FF_NOT_FOUND` for unknown id: `svc8_function` loop exhausts registry → `return FF_NOT_FOUND;` (`pus_services.c:563`).
  - `FF_SUCCESS` propagation on success: `FF_PusIngest` returns handler `rc` when `ack_flags == 0` (`pus.c:460-469`).
  - TM[8,2] payload `{func_id, status}`: `svc8_function` `resp[0]=func_id; resp[1]=(rc==FF_SUCCESS)?0:1; FF_PusSendTm(FUNCTION,2,resp,2)` (`pus_services.c:556-559`).
  - Byte offsets: service at byte 7, subservice at byte 8, 13-byte secondary header, payload at byte 19 — confirmed in `ccsds.c:200-207` and `ccsds.h:19` (`FF_CCSDS_HDR_LEN=6`); matches `stream_has_function_report` (`test_pus.c:67, 75-76`).
- **Determinism (question 3):** Confirmed deterministic — single-threaded host test, no concurrency, no randomness, no time-dependent assertions (`FF_TimeInit` called but timestamp never asserted), per-case reset of `g_func_exec_called`/`g_rx_count`/`g_func_exec_args`. Same inputs → same outputs every run.
- **Tool qualification (question 4):** No new test scripts/tools; no T3 impact; no N10 re-qualification trigger (VV-R2-004).
- **Existing behavior regression (question 5):** No regression — no `src/` production files modified; CTest exit 0 (gate E-7); `test_pus` is host-executable only and adds checks without altering existing cases. Check count 244 → 263 (+19) matches the 19 new `FF_TEST_ASSERT`s.
- **Scope note:** `test_pus` is host-only; no target timing, RTEMS/MPFS, Renode, or QEMU evidence is claimed or required for this test-level delta (norm N18 functional validation is not triggered by a host unit test with no production change).

## 5. Answers to the review questions

1. **Is the test coverage sufficient (nominal + out-of-bound)?** Yes for unit-level verification: nominal execute and two out-of-bound/error cases, each against explicit expected results (norm N22). Gap: handler-failure `status=1` path not exercised (VV-R2-001).
2. **Are the test oracles correct?** Yes — confirmed byte-exact against `svc8_function`, `FF_PusIngest`, and the CCSDS TM encoder (E-2…E-5).
3. **Is the test deterministic?** Yes — single-threaded, host-only, no concurrency, per-case state reset (question-3 confirmation).
4. **Tool qualification: N10 impact?** None — no new scripts/tools; no T3 re-qualification trigger (VV-R2-004).
5. **Did existing test behavior regress?** No — CTest exit 0, no `src/` changes, existing cases unchanged (question-5 confirmation).
6. **Do any V&V findings block commit/push?** **None.** VV-R2-001/002 are Minor, non-blocking, tracked as improvements/NCs under norm N21 sequencing.

## 6. Verdict

**V&V review: PASS with tracked Minor findings.** All reported gates are green (build exit 0 / 0 warnings; CTest exit 0 including `test_pus`; Lean `lake build`; trace PASS 120/76/30/14/68; format-check PASS; license OK). Independent source-level verification confirms all three test oracles are correct, the test is deterministic, no regression exists, and N10 is not triggered.

No V&V finding blocks commit/push, provided VV-R2-001/VV-R2-002 are registered as tracked NCs/verification improvements (norm N21 — functional behavior is verified for the exercised paths; verification gaps tracked, do not block this change).

Separate gates remain outside this review's authority: norm N7 commit/push authorization by the designated FSW engineer after full sprint scope and all applicable gates/norms, norm N20 Product Assurance acceptance, and any safety/security/residual-risk or release approval by the human authority. This report is evidence, not approval.

## 7. Finding register

| ID | Severity | File:line | Summary | Blocks commit/push? |
|---|---|---|---|---|
| VV-R2-001 | Minor | `src/pus/pus_services.c:556-559`; `tests/unit/test_pus.c:50-60, 1085-1107` | TM[8,2] `status=1` handler-failure path not exercised | No (track as NC/improvement) |
| VV-R2-002 | Minor | `tests/unit/test_pus.c:1067-1084, 1109-1146` | Manual per-case isolation; function id 7 remains registered across error cases | No (track as test-design improvement) |
| VV-R2-003 | Minor (informational) | `tests/ff_test.h:17-24`; `tests/unit/test_pus.c:1067-1147` | `FF_TEST_ASSERT` non-aborting; no short-circuit on failure | No |
| VV-R2-004 | Minor (informational) | `docs/vv/tool-qualification.md` (T3); `tests/unit/test_pus.c` | N10 not triggered; no new scripts/tools | No |
