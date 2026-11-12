# PA Review Report — test_pus: S1 Subservice 5 (Progress Success) via FF_PusIngest

**Norm:** N20 (independent Senior SW Product Assurance review)
**Repository:** /home/nicola/fsw/frick-fsw
**Branch:** main (HEAD c3fe024)
**Change under review:** uncommitted working tree, `tests/unit/test_pus.c` only
**Reviewer:** Senior Aeronautics and Space Software Product Assurance Engineer (independent PA)
**Review date:** 2026-11-12
**Language:** English

---

## Authority statement

This report is an independent product-assurance assessment prepared by an AI assurance assistant. It is **not** a certification, qualification, compliance approval, safety/security acceptance, risk acceptance, waiver, release authorization, or commit/push approval. The designated `aeronautics-space-fsw-engineer` agent (norm N7) and the authorized human authority retain the commit/push decision, gate waivers, and any safety/security/release approval. This report recommends; it does not decide.

---

## 1. Change under review

**Scope:** a single uncommitted working-tree modification to `tests/unit/test_pus.c`. No production source files (`src/`) are modified (verified in §4.2).

**Added test:** `test_service1_progress_success` (tests/unit/test_pus.c:563-599).
- Builds a CCSDS TC[17,1] (service 17, subservice 1 = ping) with `FF_PUS_ACK_PROGRESS` only (no acceptance/start/completion flags).
- Encodes via `FF_CcsdsEncodeTc`, feeds to `FF_PusIngest(frame, frame_len)`.
- Asserts TM[1,5] (service 1 verification, subservice 5 = progress success) and TM[17,2] (ping reply) are emitted; asserts TM[1,1] (acceptance) and TM[1,7] (completion) are NOT emitted.
- Asserts exact counts: `stream_count(1u,5u)==1` and `stream_count(17u,2u)==1`.
- Registered in `FF_TEST_MAIN` at tests/unit/test_pus.c:1271-1278; executable target `test_pus`, ctest name `pus` (tests/CMakeLists.txt:16, 229).

**Check delta:** 277 → 290 checks (+13), 0 failures (context). Independently confirmed: binary reports `checks=290 failures=0`.

**Not touched:** NC-EXT-19, NC-EXT-90, N22-TS5-47, GAP-A5, NC-PAL-01 (verified in §4.3); no hardware, no mission rates, no recovery policy invented.

---

## 2. Verification records (review tasks 1–5)

Each record states the task, result, and objective evidence inspected. Claims are labelled Fact (F), Inference (I), Recommendation (R), or Open issue (O).

### 2.1 Task 1 — Test correctly exercises S1 subservice 5 (progress success) end-to-end through FF_PusIngest

**Result: VERIFIED (Fact)**

Evidence chain, source-to-implementation:

| Step | Behaviour | Source evidence |
|------|-----------|-----------------|
| TC encode | TC[17,1], `FF_PUS_ACK_PROGRESS`, service 17/subservice 1 | tests/unit/test_pus.c:577-585 |
| Ingest entry | `FF_PusIngest(frame, frame_len)` called | tests/unit/test_pus.c:587 |
| Ack flag decode | `ack_progress = (tc.ack_flags & FF_PUS_ACK_PROGRESS) != 0` | src/pus/pus.c:417 |
| Progress TM emitted | `ack_progress` → `FF_PusSendVerification(&tc, FF_PUS_STAGE_PROGRESS, 1u, 0)` | src/pus/pus.c:454-458 |
| Service 1 / subservice 5 mapping | `FF_PUS_STAGE_PROGRESS` → subservice 5 (`success ? 5u : 6`) | src/pus/pus.c:316-318 |
| Service IDs | `FF_PUS_SERVICE_VERIFICATION = 1`; `FF_PUS_SERVICE_TEST = 17` | src/pus/pus_types.h:28-29 |
| Ping reply TM | service 17 handler emits `FF_PusSendTm(FF_PUS_SERVICE_TEST, 2u, …)` → TM[17,2] | src/pus/pus_services.c:625 (service 17 ping) |
| Negative checks | acceptance/completion acks not set → no TM[1,1], TM[1,7] | src/pus/pus.c:430-434, 462-467 |

The test sends a real TC through `FF_PusIngest` (the full TC ingestion path: decode → ack-flag evaluation → verification emission → handler dispatch → TM sink) and captures emitted TM through `tm_sink` (tests/unit/test_pus.c:146-152). Empirically, `stream_has(1u,5u)`, `stream_has(17u,2u)`, `!stream_has(1u,1u)`, `!stream_has(1u,7u)`, `stream_count(1u,5u)==1`, `stream_count(17u,2u)==1` all pass (binary: checks=290, failures=0).

This closes the coverage gap noted in the existing `test_ping_roundtrip`, which covers acceptance (1,1), start (1,3), completion (1,7) but not progress (1,5).

### 2.2 Task 2 — No production source files (`src/`) modified

**Result: VERIFIED (Fact)**

`git status` on branch main (HEAD c3fe024) shows exactly one modified tracked file: `tests/unit/test_pus.c`. No file under `src/` is modified, staged, or untracked as part of this change. No hardware, configuration, tooling, or requirements files are touched.

### 2.3 Task 3 — NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 not closed

**Result: VERIFIED (Fact)**

None of these identifiers is referenced, modified, or dispositioned by the change (the change touches only `tests/unit/test_pus.c`). Their OPEN status is independently confirmed in project status records:

| Item | Status | Source evidence | Closed by this change? |
|------|--------|-----------------|------------------------|
| NC-EXT-19 | OPEN | docs/status/handover.md:169-170, :949-950, :1248; docs/status/status.json | No |
| NC-EXT-90 | OPEN | docs/status/handover.md:949-950; docs/status/status.json | No |
| N22-TS5-47 | OPEN | docs/status/handover.md:1810, :1811 | No |
| GAP-A5 | OPEN (gap) | docs/status/status.json (GAP-A5/NC-PAL-01 target/HIL evidence) | No |
| NC-PAL-01 | OPEN | docs/status/handover.md:168-170, :182; docs/status/status.json | No |

Per docs/status/status.json these remain OPEN until mission use and are **not blockers for commit or push**. This change does not alter any of them.

### 2.4 Task 4 — All gates green

**Result: VERIFIED (Fact)** — all applicable gates independently re-executed in a fresh, gitignored build directory (`build/pa-review`), except the pinned format-check gate noted below.

| Gate | Required | Result | Evidence |
|------|----------|--------|----------|
| Build (`cmake --build`) | exit 0 | PASS exit 0 | `build/pa-review`, 100% targets built, no errors |
| CTest `pus` | exit 0 | PASS | Test #11 `pus` Passed; binary `checks=290 failures=0` |
| Trace (`make trace`) | PASS 120/76/30/14/68 | PASS | `trace.py`: "traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)", exit 0 |
| Lean (`make verify-lean`) | `lake build` success | PASS | "Build completed successfully.", exit 0 |
| Format-check (pinned clang-format 18) | PASS | **Environment-limited** | Pinned clang-format 18 not installed in this host (only clang-format 23.1.3). Best-effort `--dry-run --Werror` on changed file: exit 0. Gate must be re-run in the controlled V&V image per Makefile (`FF_CLANG_FORMAT_ALLOW_UNPINNED=1` or `docker run ... frick-vv:local make format`). Not a change defect. |
| License (MPL-2.0 SPDX) | OK | PASS | `tools/scripts/license_check.py`: OK; `tests/python/test_license_check.py`: 14 tests OK |

The changed file `tests/unit/test_pus.c` carries the MPL-2.0 header (lines 9-12) consistent with project norm N13.

### 2.5 Task 5 — PA findings that block commit/push

**Result: NONE.** No Major finding and no blocking Minor finding is raised. See §3 for the three Minor findings, none of which block commit/push. The commit/push decision and any gate waiver remain with the designated `aeronautics-space-fsw-engineer` agent and the authorized human authority (norms N7, N15, N21).

---

## 3. Findings (PA-XXX)

Finding classification: **Major** = blocks commit/push until resolved; **Minor** = does not block commit/push, to be dispositioned per agreed schedule.

| ID | Class | File:line | Finding | Recommended disposition | Blocks commit/push? |
|----|-------|-----------|---------|------------------------|---------------------|
| PA-001 | Minor | tests/unit/test_pus.c:561 (header block :558-562) | Comment imprecision: "emits TM[1,5] (start/progress success)". Subservice 5 is **progress ok only**; start is subservice 3 (src/pus/pus.c:316-318). The parenthetical conflates start and progress. Code and assertions are correct. | Editorial: change comment to "progress success" (drop "start/"). No code change required. | No |
| PA-002 | Minor | tests/unit/test_pus.c:563-599; tests/CMakeLists.txt:16, 229 | Host-executable test only; no target/Renode/MPFS validation. Provides no target/HIL/WCET evidence. Consistent with existing test_pus suite; target validation remains under open NCs (NC-PAL-01, GAP-A5/N22-TS5-48, N18 emulated-target only). | Confirm scope as host unit test; do not represent as target evidence. Target validation tracked by existing open NCs. | No |
| PA-003 | Minor | tests/unit/test_pus.c:146-152 (`tm_sink`), :169-185 (`stream_count`), :586 (`g_rx_count=0u`) | Stream capture relies on resetting `g_rx_count` before ingest; `tm_sink` silently drops packets when `g_rx_count + len >= sizeof(g_rx)` (line 148). Pre-existing harness pattern, not introduced by this change. This test emits only two small frames, so overflow cannot occur here. | No action for this change. If harness extended to high-volume streams, add an overflow-fail flag. | No |

**No Major findings.** **No finding blocks commit/push.**

---

## 4. Overall verdict and recommendation

**Verdict: PA-CLEARED (no blocking findings).**

The added test `test_service1_progress_success` correctly and independently exercises PUS service 1 subservice 5 (progress success) end-to-end through `FF_PusIngest`, with correct positive and negative assertions and exact counts. No production source is modified. No open NC (NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01) is touched or closed. All applicable gates are independently green (format-check limited by missing pinned clang-format 18 in this review host; best-effort check on the changed file is clean).

**Recommendation:** the designated `aeronautics-space-fsw-engineer` agent may proceed with commit/push once all applicable gates and the full sprint scope are complete per norms N7, N15, N21. The Minor editorial finding (PA-001) should be corrected in the same or a follow-up commit as agreed; it does not block this change.

**Documentation (norms N8/N17):** this change adds verification evidence only; it does not alter behaviour, requirements, tooling, or configuration, so no documentation update is required for this change. Scope any required documentation review to the changed product.

---

## 5. Boundaries and independence

- Reporting line and independence: this review is independent of engineering implementation; the reviewer did not author the change and has no schedule/financial conflict in this item.
- Self-approval is avoided: this report does not certify, qualify, accept risk, waive, or release the product.
- PA does not substitute for engineering, V&V, system/software safety, security, configuration management, or certification/regulator authorities.
- Safety-critical conclusions, concessions, waivers, residual-risk acceptance, and release decisions require independent qualified human review (persona §4/§15).

---

## 6. Uncertainty and confidence reporting

```text
Decision/product and configuration:
  tests/unit/test_pus.c addition of test_service1_progress_success, branch main @ c3fe024, host build.
Claim type: Fact (verification results), Inference (scope/no-doc-update), Recommendation (dispositions).
Applicable baseline and approved tailoring: project norms N1-N22 (AGENTS.md); ECSS class B target; host unit-test scope.
Evidence consulted (repository artifacts):
  tests/unit/test_pus.c (:558-599, :146-185, :1271-1278)
  tests/CMakeLists.txt (:16, :229)
  src/pus/pus.c (:298-354, :417, :430-434, :454-458, :462-467)
  src/pus/pus_services.c (:625)
  src/pus/pus_types.h (:24, :28-29, :75-80)
  tests/ff_test.h
  docs/status/handover.md, docs/status/status.json (NC status)
  Fresh build in build/pa-review (gitignored): build exit 0; ctest pus Passed; binary checks=290 failures=0; trace PASS 120/76/30/14/68; lake build success; license_check OK; test_license_check 14/14 OK.
Cross-check and reconciliation: implementation trace (pus.c ack_progress → FF_PusSendVerification stage PROGRESS → subservice 5) reconciles with test assertions; gate results reconcile with context claims.
Confidence: High.
Basis and detection limits: direct source inspection plus executed binary/test/trace/lean/license evidence. Limitation: pinned clang-format 18 gate not executable in this host (clang-format 23.1.3 only); best-effort dry-run on changed file clean. No target/HIL evidence (host-only test), consistent with existing practice.
Missing/inaccessible evidence: none for this change; format gate pending controlled V&V image.
Assumptions and expiry/recheck date: format-check re-run in controlled V&V image before/at commit; no expiry for this review other than subsequent tree changes.
Consequences if wrong: none safety-critical (host unit test, no src/ change, no NC closure); incorrect formatting would fail CI gate (caught by format-check).
Required independent human reviewer/authority: designated aeronautics-space-fsw-engineer agent (commit/push, N7/N15) and authorized human authority (release/safety).
```

---

## 7. Source register (artifacts inspected, repository-local)

| Ref | Artifact | Role in review |
|-----|----------|----------------|
| R1 | tests/unit/test_pus.c (:558-599) | Changed test under review |
| R2 | tests/unit/test_pus.c (:146-185) | `tm_sink`, `stream_has`, `stream_count` harness |
| R3 | tests/unit/test_pus.c (:1271-1278) | `FF_TEST_MAIN` registration |
| R4 | tests/CMakeLists.txt (:16, :229) | `test_pus` target and ctest `pus` registration |
| R5 | src/pus/pus.c (:298-354, :417, :430-434, :454-458, :462-467) | `FF_PusSendVerification`, `FF_PusIngest` ack/progress logic |
| R6 | src/pus/pus_services.c (:625) | service 17 ping reply TM[17,2] |
| R7 | src/pus/pus_types.h (:24, :28-29, :75-80) | ack flags, service IDs, stage/subservice mapping |
| R8 | tests/ff_test.h | `FF_TEST_ASSERT`, `FF_TEST_MAIN` macros |
| R9 | docs/status/handover.md (:168-170, :949-950, :1248, :1810-1811) | NC-EXT-19/90, N22-TS5-47, NC-PAL-01 OPEN status |
| R10 | docs/status/status.json (:5) | OPEN NCs not blockers for commit/push |
| R11 | build/pa-review (fresh, gitignored) | Independently executed build/test/trace/lean/license evidence |

End of report.