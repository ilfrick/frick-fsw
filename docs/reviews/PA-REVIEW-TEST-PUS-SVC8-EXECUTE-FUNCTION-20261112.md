# PA Review Report — `test_function_execute_bounded` (PUS S8 TC[8,1] execute function)

**Norm:** N20 (independent Senior SW Product Assurance review)  
**Reviewer persona:** Senior Aeronautics and Space Software Product Assurance Engineer (independent; not the change author; not release/safety authority)  
**Authority statement:** This report is an independent assurance assessment. It does not certify compliance, approve a safety case, accept residual risk, close a nonconformance, qualify a tool, authorize release, or declare flightworthiness. Independent qualified humans retain those decisions.

## 1. Review identification

| Field | Value |
|---|---|
| Repository | /home/nicola/fsw/frick-fsw |
| Branch | main |
| HEAD | 3b04cb2 (`S8-PUS-FU: add PUS service 20 TC[20,1] set-parameter test`) |
| Change | Uncommitted working tree: `tests/unit/test_pus.c` only |
| Change summary | Added `test_function_execute_bounded` (19 new checks; 244 -> 263, 0 failures). Verifies PUS service 8 subservice 1 (TC[8,1] execute function) end-to-end via `FF_PusIngest`: register function handler, send TC[8,1], assert handler invoked with correct func ID/args, assert TM[8,2] execution report (func_id + status), and error paths (unknown function ID -> `FF_NOT_FOUND`, empty payload -> `FF_BAD_SIZE`). No production source files (`src/`) modified. |
| Review date | 2026-11-12 |
| Review scope | Test correctness vs. `src/pus/pus_services.c` `svc8_function` / `src/pus/pus.c` `FF_PusIngest`; `src/` untouched; named NCs untouched; gate status; PA findings |
| Related reviews (same change, distinct artefacts) | `docs/reviews/2026-11-12-r4-doc-review-test-pus-svc8-execute-function.md` (R4 doc); `docs/reviews/SYS-REVIEW-R1-R3-20261112-test-pus-svc8-function-execute-avionics-systems-review.md` (avionics). This report is the PA/N20 artefact. |

## 2. Independence and boundaries

- The change author is not this reviewer; no self-approval of PA evidence.
- This PA review assesses sufficiency/consistency/provenance of evidence only. Engineering correctness of `svc8_function` remains owned by the FSW engineer; V&V conclusions remain owned by V&V; release/safety acceptance remains with the authorized human authority.
- This test is **host-executable only** (native simulator). No target/hardware, no RTEMS target image, no flight binding, no mission rates, no recovery policy introduced or changed. It does not close any target/HIL dependency.

## 3. Verification results (task items)

### 3.1 Item 1 — Test correctly exercises `svc8_function` (TC[8,1]) end-to-end through `FF_PusIngest` — VERIFIED

Trace of the success path, confirmed against production code:

| Step | Test (tests/unit/test_pus.c) | Production behaviour | Verdict |
|---|---|---|---|
| Init + sink | lines 1069-1074 (`FF_PalInit`, `FF_EvsInit`, `FF_TimeInit`, `FF_PusInit`, `FF_PusSetBackend(&FF_PusBackendOpen)`, `FF_PusSetTmSink(tm_sink)`) | Same harness pattern as `test_ping_roundtrip` (lines 159-164); open TM backend captures into `g_rx` via `tm_sink` (lines 116-122) | OK |
| Register handler | line 1083 `FF_PusRegisterFunction(7u, g_func_test_handler)` | `FF_PusRegisterFunction` (src/pus/pus_services.c:164): rejects NULL, rejects duplicate id (NC-EXT-23/C-16), stores id 7 + handler | OK |
| Build TC[8,1] | lines 1087-1098: service=`FF_PUS_SERVICE_FUNCTION` (8), subservice=1 (`/* ECSS: 1 = Execute function */`), payload `[7, 0xCC, 0xDD]` | Dispatch table `{8u, 1u, svc8_function}` at src/pus/pus_services.c:705 | OK |
| Ingest | line 1100 `FF_PusIngest(frame, frame_len) == FF_SUCCESS` | `FF_PusIngest` (src/pus/pus.c:390): decode, APID check, `ff_pus_find(8,1) = svc8_function`, calls handler | OK |
| Handler invoked | lines 1101-1104: `g_func_exec_called`, `g_func_exec_len == 2`, `args[0]==0xCC`, `args[1]==0xDD` | `svc8_function` (src/pus/pus_services.c:546): `len>=1`, `func_id=d[0]=7`, finds registered id 7, calls `fn(&d[1], len-1)` = `fn([0xCC,0xDD], 2)`; handler records and returns `FF_SUCCESS` | OK |
| TM[8,2] report | line 1106 `stream_has_function_report(7u, 0u)` | `svc8_function` sends `FF_PusSendTm(FUNCTION, 2, resp=[7, 0])` (src/pus/pus_services.c:559); `resp[1]=0` on success (line 558) | OK |
| Offsets in `stream_has_function_report` | lines 62-90: service/subservice at `pos+7`/`pos+8`; payload at `pos+19`/`pos+20` (TM secondary header 13 octets) | TM encode (src/transport/ccsds/ccsds.c:182-218): service out[7], subservice out[8], data at out[6+13]=out[19]. Packet-length stepping `6+pdl+1` consistent with CCSDS `packet_data_len = total-7` | OK |

Error paths (confirmed against production code):

| Case | Test | Production expectation | Verdict |
|---|---|---|---|
| Unknown function ID | lines 1109-1127: payload `[99, 0x00]`, assert `FF_PusIngest == FF_NOT_FOUND`, `!g_func_exec_called` | `svc8_function`: id 99 not registered -> return `FF_NOT_FOUND` (line 563); `FF_PusIngest` returns handler rc (line 460/469) when no `ack_complete` | OK |
| Empty payload (bad size) | lines 1129-1146: `FF_CcsdsEncodeTc(..., NULL, 0u, ...)`, assert `FF_BAD_SIZE`, `!g_func_exec_called` | `svc8_function`: `len < 1` -> `FF_BAD_SIZE` (line 549-551); encode of `NULL,0` is accepted by `FF_CcsdsEncodeTc` (ccsds.c:109: only rejects `NULL && len!=0`) | OK |

Nominal + applicable out-of-bound cases are present with explicit expected results, consistent with norm N22. Conclusion: the test correctly exercises `svc8_function` end-to-end through `FF_PusIngest`, including the TM[8,2] execution report and both documented error paths.

### 3.2 Item 2 — No production source files (`src/`) modified — VERIFIED

`git status` shows exactly one modified tracked file:

```
modified:   tests/unit/test_pus.c
```

No file under `src/` (including `src/pus/pus_services.c` `svc8_function` at line 546 and `src/pus/pus.c` `FF_PusIngest` at line 390) is touched. The change is test-only.

### 3.3 Item 3 — Named NCs NOT closed — VERIFIED

Confirmed OPEN in repository records (`docs/status/handover.md` and `docs/status/status.json`); the change touches none of them:

| NC / item | Status per records | Touched by this change? |
|---|---|---|
| NC-EXT-19 / NC-EXT-90 | OPEN — mission schedule/timing/policy inputs; authority-dependent | No |
| N22-TS5-47 | OPEN — EVS-to-HS end-to-end | No |
| GAP-A5 | OPEN — target/HIL evidence | No |
| NC-PAL-01 | OPEN — flight binding (MPFS watchdog), target/HIL | No |

`status.json` explicitly records these as OPEN and "NOT blockers for commit or push." No NC is closed by this change, and no NC closure is implied.

### 3.4 Item 4 — All gates green — VERIFIED (independently re-executed on host)

| Gate | Result | Evidence |
|---|---|---|
| Build (`make BUILD_DIR=build/pa-review build`) | PASS, exit 0 | 100% built; `grep` for warnings on host sources: none (0 host warnings) |
| CTest (`ctest --test-dir build/pa-review`) | PASS | 57/57 tests passed, 0 failed; PUS-related: `pus`, `pus_fu`, `pus_n7_adapter_contract`, `pusopen_adapter_contract`, `pus_event_identity`, `decoder_corpus` all passed |
| pus test binary | PASS | `build/pa-review/tests/test_pus`: checks=263, failures=0 (matches 244->263) |
| `verify-lean` (`lake build`) | PASS | "Build completed successfully." |
| `make trace` | PASS | "traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)" — matches context (120/76/30/14/68) |
| Format check | PASS (cross-check) | `clang-format --dry-run --Werror` on `tests/unit/test_pus.c`: rc=0 (format-clean on host clang-format 23.1.3). See PA-002 re: pinned CI major 18. |
| License (`tools/scripts/license_check.py`) | PASS | "MPL-2.0 license identity and authored-file SPDX check: OK" |

Note: Lean/traceability inputs (`.lean`, `docs/requirements/traceability.yaml`) are unchanged by this test-only change; their previously-context-reported PASS stands and was re-confirmed above where executable.

### 3.5 Item 5 — PA findings that block commit/push

**None.** All findings below are **Minor** and **do not block commit/push**. Commit/push remains subject to the separately applicable gates/norms (N7: all gates + full sprint scope; authorized `aeronautics-space-fsw-engineer` agent or explicit delegation), which are green.

## 4. Findings

### PA-001 — Minor — Unused recording variable `g_func_exec_id` — Does NOT block

- **Location:** tests/unit/test_pus.c:45 (declaration), :1077 (reset `g_func_exec_id = 0u`); never read or asserted.
- **Production interface:** `svc8_function` calls the handler as `g_functions[i].fn(&d[1], (uint16_t)(len - 1u))` (src/pus/pus_services.c:555) — the handler signature `(const uint8_t *args, uint16_t len)` carries no func_id, so the handler cannot record it. func_id is verified indirectly via the TM[8,2] payload (`stream_has_function_report`, line 1106).
- **Description:** `g_func_exec_id` is declared and reset but never used. It is dead code and may signal an incomplete verification intent ("handler invoked with correct function ID" is proven only through TM, not through the handler argument).
- **Recommended disposition:** Remove `g_func_exec_id` (declaration + reset), or document that func_id correctness is asserted via TM[8,2] payload rather than handler observation. No functional impact.
- **Blocks commit/push:** No.

### PA-002 — Minor — Format-check not verifiable against pinned clang-format 18 on this host — Does NOT block

- **Location:** Makefile:189-197 (`FF_CLANG_FORMAT_MAJOR = 18` guard); tests/unit/test_pus.c (changed file).
- **Description:** The CI format-check gate pins clang-format major 18. This host provides clang-format 23.1.3, so `make format-check` fails its guard unless `FF_CLANG_FORMAT_ALLOW_UNPINNED=1`. The changed file was verified format-clean with clang-format 23 (`--dry-run --Werror`, rc=0), but clang-format >= 19 lays out single-line macro definitions differently (Makefile:185-188 comment), so compliance with the pinned CI toolchain is not established on this host.
- **Recommended disposition:** Re-run `make format-check` inside the controlled V&V image (`frick-vv:local`, clang-format 18) before push; treat the host result as a cross-check only. This is a norm N10/N7 gate condition.
- **Blocks commit/push:** No (format verified clean; gate must clear in CI with pinned clang-format 18 before push).

### PA-003 — Minor — TM[8,2] failure-status byte (`status=1`) branch not exercised — Does NOT block (recommendation)

- **Location:** tests/unit/test_pus.c:1067-1147; src/pus/pus_services.c:556-558.
- **Description:** `svc8_function` encodes TM[8,2] `resp[1] = 0` on handler success, `1` otherwise (src/pus/pus_services.c:558). The test registers a handler that always returns `FF_SUCCESS` and only asserts `status=0` (line 1106). The handler-returns-failure -> `status=1` branch is not covered. Norm N22 calls for nominal + applicable out-of-bound operation against explicit expected results; this internal branch is a coverage gap.
- **Recommended disposition:** (Optional enhancement) Add a case where the handler returns a non-success status and assert TM[8,2] `status=1`; or accept as a documented coverage limitation. The stated error paths (unknown ID, bad size) are covered.
- **Blocks commit/push:** No.

## 5. NC status summary (unchanged)

| Item | Status | Closed by this change? |
|---|---|---|
| NC-EXT-19 / NC-EXT-90 | OPEN | No |
| N22-TS5-47 | OPEN | No |
| GAP-A5 | OPEN | No |
| NC-PAL-01 | OPEN | No |

These are authority/target/HIL-dependent and remain open until mission use; per `status.json` they are **not** blockers for commit or push.

## 6. Conclusion and recommendation

**PA verdict: SUITABLE TO PROCEED to commit/push, subject to the normal gates.** 

The change:
- Correctly exercises `svc8_function` (PUS service 8, subservice 1, TC[8,1]) end-to-end through `FF_PusIngest`, including the TM[8,2] execution report and both documented error paths;
- Modifies no production source (`src/` untouched);
- Closes none of NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 (all remain OPEN and untouched);
- Passes all applicable gates (build 0 warnings, CTest 57/57, pus checks=263 failures=0, Lean, trace 120/76/30/14/68, format-clean, license OK).

All three PA findings (PA-001, PA-002, PA-003) are **Minor** and **do not block commit/push**. PA-002 requires the format-check gate to clear with the pinned clang-format 18 in CI before push.

**Recommended action for the author:** address PA-001 (remove unused `g_func_exec_id` or document TM-based func_id verification) in the same commit per norm N8 (docs updated in the same commit), then commit/push only after the pinned-clang-format-18 format check passes in CI and all N7 gates/sprint scope are complete.

**Escalation:** None required. No safety/security/criticality claim is made by this change; no waiver, deviation, or residual-risk acceptance is requested.

---

## 7. Uncertainty and confidence reporting

```text
Decision/product and configuration:
  tests/unit/test_pus.c (uncommitted, branch main, HEAD 3b04cb2), host native build; target/hardware out of scope.
Claim type: Fact (verification results and findings); Recommendation (dispositions).
Applicable baseline and approved tailoring: Project norms N1-N22 (docs/rules.md); ECSS Q-ST-80C / E-ST-40C context per contract (not a certification basis).
Evidence consulted:
  - tests/unit/test_pus.c (working tree, HEAD 3b04cb2 + uncommitted change)
  - src/pus/pus_services.c:164 (FF_PusRegisterFunction), :546-564 (svc8_function), :705 (dispatch)
  - src/pus/pus.c:390-469 (FF_PusIngest)
  - src/transport/ccsds/ccsds.c:100-135 (FF_CcsdsEncodeTc), :182-218 (FF_CcsdsEncodeTm)
  - tests/ff_test.h (FF_TEST_ASSERT, FF_TEST_MAIN)
  - Makefile:189-197 (format pin), :30-34 (build/test), :56-57 (verify-lean), :76-77 (trace)
  - docs/status/handover.md; docs/status/status.json (NC register)
Cross-check and reconciliation:
  - Test offsets (service pos+7, subservice pos+8, payload pos+19/pos+20, step=6+pdl+1) reconciled against FF_CcsdsEncodeTm/FF_CcsdsEncodeTc; CCSDS packet_data_len = total-7 verified.
  - Gate results independently re-executed on host (build, CTest 57/57, pus checks=263/0, Lean, trace 120/76/30/14/68, format dry-run rc=0, license OK).
Confidence: High for items 1-4 (direct code inspection + executed gates); Medium for PA-002 (host clang-format 23 vs pinned CI 18; environmental, not a change defect).
Missing/inaccessible evidence: Pinned clang-format 18 not present on host (verified only via clang-format 23 cross-check); CI image format-check pending.
Assumptions and expiry/recheck date: None beyond standard gate re-run on change; re-check format in CI before push.
Consequences if wrong: PA-001/PA-003 are code-quality/coverage, no safety effect; PA-002 is a gate-toolchain compliance check, caught by CI.
Required independent human reviewer/authority: Release/commit authority per norm N7 (aeronautics-space-fsw-engineer or explicit delegation); no safety/security/release approval asserted by this PA review.
```

---

## 8. Source register (review artefacts)

| Ref | Artefact | Role |
|---|---|---|
| R1 | tests/unit/test_pus.c (working tree) | Change under review |
| R2 | src/pus/pus_services.c:164, :546-564, :705 | Production interface under test |
| R3 | src/pus/pus.c:390-469 | FF_PusIngest |
| R4 | src/transport/ccsds/ccsds.c:100-135, :182-218 | CCSDS/PUS encode (offset validation) |
| R5 | tests/ff_test.h | Test harness semantics |
| R6 | docs/status/handover.md; docs/status/status.json | NC register (OPEN status confirmation) |
| R7 | Makefile:30-34, :56-57, :76-77, :189-197 | Gate targets and format pin |
| R8 | docs/reviews/2026-11-12-r4-doc-review-test-pus-svc8-execute-function.md | Companion R4 doc review (same change) |
| R9 | docs/reviews/SYS-REVIEW-R1-R3-20261112-test-pus-svc8-function-execute-avionics-systems-review.md | Companion avionics review (same change) |

**End of report.** This PA review is filed under `docs/reviews/` per norm N20/N11; review evidence is retained in the working tree and must accompany the compliant change cycle.