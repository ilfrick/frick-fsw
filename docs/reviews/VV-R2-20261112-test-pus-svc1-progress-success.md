# Independent SW V&V Review — PUS service 1 progress-success unit test (`test_service1_progress_success`)

| Field | Value |
|---|---|
| Review ID | VV-R2-20261112-TEST-PUS-SVC1-PROGRESS-SUCCESS |
| Reviewer profile | Senior Aeronautics and Space SW V&V Engineer (independent-context R2, norm N9/N10) |
| Repository | /home/nicola/fsw/frick-fsw, branch `main`, HEAD `c3fe024` |
| Change under review | Uncommitted working tree: `tests/unit/test_pus.c` — added `test_service1_progress_success` (lines 563-599). Verifies PUS TC[17,1] with `FF_PUS_ACK_PROGRESS` only → TM[1,5] (progress success) end-to-end through `FF_PusIngest`, plus TM[17,2] ping reply, and asserts TM[1,1] / TM[1,7] are NOT emitted (ack flags not requested) with exact `stream_count` == 1 each. Registered in `FF_TEST_MAIN` at line 1274. No `src/` production files modified — verifies existing `svc17_ping` + `FF_PusIngest` progress branch via `FF_PusSendVerification`. |
| Date | 2026-11-12 |
| Governing norms | N1-N22 (docs/rules.md); in particular N10 (test-script/tool qualification), N21 (sequencing), N22 (nominal + applicable out-of-bound cases vs explicit expected results), N7 (commit/push authority). ECSS-E-ST-40C Rev.1 / ECSS-Q-ST-80C Rev.2 context. ECSS-E-ST-70-41C (PUS-C) service 1 verification and service 17 test reference. |
| Criticality | ECSS Class B target (project target; no controlled mission assignment evidenced) |
| Scope | `tests/unit/test_pus.c` only (new test :563-599; oracles `stream_has` :154-166, `stream_count` :169-185, sink `tm_sink` :146-152). Host-executable CTest unit test; NOT target/HIL/Renode/QEMU; NOT GAP-A5/NC-PAL-01/OBCP; no production-code behavior change. Existing NCs NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 untouched. |

> **Authority statement.** This review is an independent analysis artifact. It has no approval, qualification, acceptance, certification, or flight-readiness authority. Commit/push authorization (norm N7), safety/security acceptance, PA sign-off (norm N20), and any residual-risk decision remain with the designated FSW engineer and the human authorities. This report may recommend "do not proceed"; it may not authorize proceeding.

## 1. Change under review (verified diff)

`git diff tests/unit/test_pus.c` (46 insertions, 2 deletions) adds a single test case:

1. Setup (:569-574): `FF_PalInit`, `FF_EvsInit`, `FF_TimeInit`, `FF_PusInit`, `FF_PusSetBackend(&FF_PusBackendOpen)`, `FF_PusSetTmSink(tm_sink)` — self-initializing, order-independent.
2. TC build (:577-586): TC[17,1] ping, `ack_flags = FF_PUS_ACK_PROGRESS` only (:579), payload `{0x01, 0x02}`, source_id `0x0102`, encoded via `FF_CcsdsEncodeTc`.
3. Ingest (:586-587): `g_rx_count = 0u`, then `FF_PusIngest(frame, frame_len) == FF_SUCCESS`.
4. Oracles (:591-598):
   - `stream_has(1u, 5u)` — TM[1,5] progress success present;
   - `stream_has(17u, 2u)` — TM[17,2] ping reply present;
   - `!stream_has(1u, 1u)` — acceptance TM absent (acceptance ack not requested);
   - `!stream_has(1u, 7u)` — completion TM absent (completion ack not requested);
   - `stream_count(1u, 5u) == 1u` — exactly one progress TM;
   - `stream_count(17u, 2u) == 1u` — exactly one ping reply.

## 2. Execution evidence (primary, re-executed in this review)

| ID | Evidence | Result |
|---|---|---|
| X-1 | `cmake -S . -B /tmp/opencode/build-native -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON` | exit 0 |
| X-2 | `cmake --build ... --target test_pus` then full build | exit 0, 0 errors, 0 warnings in project code (only third-party pusclib/lwip endianness notes) |
| X-3 | `/tmp/opencode/build-native/tests/test_pus` | `checks=290 failures=0`, exit 0 — includes `test_service1_progress_success` |
| X-4 | `ctest` (full suite, 57 tests) | exit 0, all tests passed — no regression |
| X-5 | Source trace: `test_pus.c:563-599` vs `src/pus/pus.c:298-469`, `src/pus/pus_services.c:622-626,717`, `src/pus/pus_types.h:24,28,35,78`, `src/transport/ccsds/ccsds.c:122-123,202-203`, `tests/ff_test.h:17-24` | All oracles confirmed byte-exact against implementation (see §4) |

The working-tree change was built and executed from the current (uncommitted) sources; this is not a stale-tree result.

## 3. Norm-by-norm confirmation (the six review questions)

| # | Review question | Result | Evidence |
|---|---|---|---|
| 1 | Coverage sufficiency (progress success via TC ingestion) | **Sufficient for claimed scope** (nominal) | X-3/X-5: TM[1,5] + TM[17,2] emitted, TM[1,1]/TM[1,7] absent, counts == 1. Minor coverage-completeness observation: nominal only; combined-ack and out-of-bound variants for this flag not exercised (VV-R2-002). |
| 2 | Oracle correctness (exact TM presence/absence, exact counts) | **Correct and byte-exact** | X-5: service at TM byte 7, subservice at byte 8 (`ccsds.c`, `pus_types.h:70-71`); `FF_PUS_ACK_PROGRESS`=0x04 (`pus_types.h:24`); `STAGE_PROGRESS`→subservice 5 (`pus.c:316-318`); `svc17_ping`→TM[17,2] (`pus_services.c:622-626`); ingestion flag parse + progress branch (`pus.c:415-418,454-458`). Absence assertions rely on `stream_has` (see VV-R2-001). |
| 3 | Determinism (single-threaded, no concurrency) | **Verified deterministic** | X-3: single-threaded host test; no threads, no randomness, no timing dependence (TM time fields not asserted); `FF_PalTimeTestFreeze` not used and not needed. `g_tm_counter`/`s_sequence_count` statics persist across tests but are never asserted by this test. |
| 4 | Tool qualification (no new scripts/tools, no T3 impact, no N10 re-qualification) | **No impact** | No new test scripts/tools added; `test_pus` is existing CTest target (T3); change context confirms no T3 impact, no N10 re-qualification trigger; `docs/vv/tool-qualification.md` register untouched. |
| 5 | Regression (existing test behavior, CTest exit 0) | **None** | X-4: ctest exit 0, all 57 tests pass; build clean (0 warnings). `test_pus` checks grew to 290 with 0 failures. (The earlier "sched_tm_chain Not Run" in a partial target-only build was an artifact of not building that unrelated binary; it is unrelated to `test_pus.c`.) |
| 6 | Blocking V&V findings | **None** | All findings Minor; no Major; no finding blocks commit/push (see §4). |

## 4. Findings

### VV-R2-001 — Minor — `stream_has` lacks the malformed-packet walk guard present in `stream_count` (oracle reliability for the absence assertions)

**Classification:** Minor — oracle-reliability observation; no defect triggered.  
**Blocks commit/push:** No.

**Description.** `stream_has` (`tests/unit/test_pus.c:154-166`) walks the captured TM stream with `pos = pos + 6u + pdl + 1u` but has no `step < 7u` / `(uint16_t)(pos + step) <= pos` break guard. `stream_count` (`:169-185`) includes that guard precisely to avoid stalling or overrunning on a truncated/malformed frame. The new test makes absence assertions (`!stream_has(1u, 1u)` at :593, `!stream_has(1u, 7u)` at :594), so if `stream_has` stopped walking early on a malformed frame it could report a false-negative absence. All frames here are well-formed (produced by the real encoder) and the test passes (X-3), so the weakness is latent, not exercised.

**File:line references.** `tests/unit/test_pus.c:154-166` (`stream_has`, no guard); `:169-185` (`stream_count`, guard present, for contrast); `:591-594` (presence/absence assertions the new test relies on).

**Observed result.** 290 checks / 0 failures (X-3); absence assertions pass because frames are well-formed.

**Disposition.** Add the same packet-walk guard to `stream_has` so the absence oracle cannot silently stop early, and re-run CTest. Track under norm N21 (verification improvement); does not stop this change.

---

### VV-R2-002 — Minor — Coverage is nominal-only for progress via ingestion; combined ack flags and out-of-bound variants not exercised (N22 completeness)

**Classification:** Minor — coverage-completeness observation; claimed scope is covered.  
**Blocks commit/push:** No.

**Description.** The test isolates `FF_PUS_ACK_PROGRESS` only (`test_pus.c:579`). Norm N22 requires nominal *and applicable out-of-bound* cases with explicit expected results. The claimed scope — "progress success via TC ingestion" — is covered nominally (TM[1,5] present, count == 1). What is not covered by this test, nor elsewhere for this flag: (a) progress combined with other ack flags in one ingestion (which would produce multiple verification TMs, e.g. progress|completion → TM[1,5] then TM[1,7]); `test_ping_roundtrip` (`:187-248`) uses acceptance|start|completion, not progress. (b) an out-of-bound ingestion variant for this flag combination. Per `FF_PusIngest` (`pus.c:454-458`) progress with `success=1` is the only production progress path (progress failure subservice 6 is never reached via ingestion), so no positive TM[1,6]-via-ingestion claim is needed or made.

**File:line references.** `tests/unit/test_pus.c:563-599` (new test, nominal only); `:579` (`ack_flags = FF_PUS_ACK_PROGRESS`); `:187-248` (`test_ping_roundtrip`, no progress flag); `src/pus/pus.c:454-458` (single production progress branch, success=1 only).

**Observed result.** Nominal progress path verified end-to-end (X-3).

**Disposition.** Extend with a combined-ack case (and/or an out-of-bound ingestion case) under norm N21; functional success path verified, does not stop this change.

---

### VV-R2-003 — Minor — `tm_sink` silently drops packets once the 1024-byte capture buffer is full; exact-count exactness assumes capture capacity is never exceeded

**Classification:** Minor — capture-capacity observation; not triggered.  
**Blocks commit/push:** No.

**Description.** `tm_sink` (`tests/unit/test_pus.c:146-152`) performs no `memcpy` and does not advance `g_rx_count` when `g_rx_count + len >= sizeof(g_rx)` (8 * 128 = 1024 bytes). `stream_count` (`:169-185`) then undercounts because emitted TM frames were silently discarded. The new test's exact-count assertions (`stream_count(...) == 1u` at :597-598) assume every emitted frame was captured. Not triggered here: exactly 2 TM frames (≈ 60 bytes total) ≪ 1024 bytes, and the test passes (X-3). This is a pre-existing harness characteristic inherited by the new test.

**File:line references.** `tests/unit/test_pus.c:146-152` (`tm_sink`, silent-drop ceiling); `:596-598` (exact-count assertions); `:586` (`g_rx_count = 0u` reset before ingest).

**Observed result.** Counts exact for this run (2 frames); no drop observed.

**Disposition.** Document the capture ceiling (or size `g_rx` to the maximum expected stream) and consider asserting no drop (`g_rx_count < sizeof(g_rx)`) where exact counts matter. Track under norm N21; non-blocking.

## 5. Traceability / requirements mapping

The test covers the PUS service 1 verification behaviour for the progress stage (TC[17,1] → TM[1,5]) and the service 17 ping reply (TC[17,1] → TM[17,2]), including the negative requirement that non-requested verification TMs (1,1 acceptance, 1,7 completion) are not emitted. It is requirements-based (ack-flag driven) with explicit expected results per norm N22. No traceability.yaml entry is required because no production requirement ID is added or changed; the test exercises existing verified behaviour (`FF_PusSendVerification` progress branch, `svc17_ping`). Bidirectional trace for the existing behaviour is unchanged (norms N3/N4 unaffected — no `src/` or requirements change).

## 6. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Summary.** The test coverage is sufficient for the claimed scope (progress success via TC ingestion), each check compared against explicit expected results per norm N22. The test oracles are correct and byte-exact: TM service byte 7 / subservice byte 8 layout (`ccsds.c`, `pus_types.h:70-71`), `FF_PUS_ACK_PROGRESS` = 0x04 (`pus_types.h:24`), `STAGE_PROGRESS` → verification subservice 5 (`pus.c:316-318`), `svc17_ping` → TM[17,2] (`pus_services.c:622-626`), and the ingestion flag parse / progress branch (`pus.c:415-418,454-458`). The test is deterministic: single-threaded, no concurrency, no timing dependence. Tool qualification is unaffected: no new scripts/tools, no T3 impact, no N10 re-qualification trigger. No regression: ctest exit 0, all 57 tests pass, build exit 0 with 0 project-code warnings. Primary execution evidence: `test_pus` 290 checks / 0 failures / exit 0 (X-3), re-executed from the current uncommitted sources.

Three Minor findings (VV-R2-001 to VV-R2-003) concern `stream_has` walk-guard robustness, nominal-only coverage completeness, and the `tm_sink` capture ceiling. None is Major; none blocks commit/push. They should be corrected or justified under norm N21 in the next compliant cycle.

No V&V finding blocks commit/push.

## 7. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|---|---|---|---|
| R2 (SW V&V, norm N9/N10) | Independent Senior SW V&V Engineer | 2026-11-12 | APPROVED WITH COMMENTS (3x Minor, none blocking) |

## 8. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|---|---|---|---|---|---|---|---|---|
| VV-R2-001 | Minor | `tests/unit/test_pus.c:154-166` | `stream_has` lacks malformed-packet walk guard (absence-oracle reliability) | Add walk guard matching `stream_count`; re-run CTest | CTest exit 0; test_pus 290/0 | V&V engineer | Open | Next cycle |
| VV-R2-002 | Minor | `tests/unit/test_pus.c:563-599` | Nominal-only coverage for progress via ingestion; combined-ack / out-of-bound not exercised | Add combined-ack and/or out-of-bound case under N21 | CTest exit 0 | V&V engineer | Open | Next cycle |
| VV-R2-003 | Minor | `tests/unit/test_pus.c:146-152` | `tm_sink` silent-drop at 1024-byte ceiling; count exactness assumes no drop | Document capture ceiling; optional no-drop assertion | CTest exit 0 | V&V engineer | Open | Next cycle |

---

Committed and pushed with the reviewed change (norm N11) — only by the designated `aeronautics-space-fsw-engineer` agent after all gates/norms pass (norm N7). This independent R2 reviewer records "do not block" and does not authorize release. N20 PA review and N9/N14 independent reviews remain separate gates.
