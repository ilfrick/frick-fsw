# R4 Documentation Review — frick-fsw PUS service 9 time-report test (TC[9,2] -> TM[9,2])

**Reviewer role:** Senior Aeronautics and Space Software Documentation Engineer (R4 profile), independent documentation review under norm N17.  
**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD 29cba32 (working tree).  
**Date:** 2026-11-12 (UTC).  
**Change under review:** `tests/unit/test_pus.c` — Added `test_service9_report_time` test function verifying PUS service 9 subservice 2 (TC[9,2] report time → TM[9,2]) end-to-end through `FF_PusIngest`, plus helper `stream_has_time_report`. `test_service9_report_time` registered in `FF_TEST_MAIN`. No production source files (`src/`) modified.  
**Scope:** Test-only change. No API, interface, behavior, traceability-requirement, ICD, SRS, SDD, user-manual, or release-note change. Not hardware/target related; host-executable. NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01 untouched.  
**Gate evidence reported by submitter (not re-validated by this review):** `make build` exit 0 (0 warnings); CTest exit 0 including `test_pus.c`; Lean `lake build`; `make trace` PASS; format-check PASS (pinned clang-format 18); license OK.

Norms applied: N2 (English), N8 (documentation current), N16 (revision history), N17 (documentation review gate).

---

## 1. Findings

Each finding is classified **Major** or **Minor**, carries file:line references, and states whether it blocks commit/push. Confirmations (no deficiency) are classified Minor and explicitly marked "conforming".

### DOC-R4-001 — Norm N2 (English) in new test code comments — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | `tests/unit/test_pus.c:91-92` (`stream_has_time_report` comment), `:506` (section header), `:508-509` (function comment), `:520`, `:526`, `:529`, `:531`, `:537`, `:540-541`, `:548-549` (inline comments) |
| Blocks commit/push? | No |
| Evidence | Every added comment is in English: e.g. `/* Check that TM[9,2] (time report) carries the expected 6-octet CUC payload: coarse[4, big-endian] + fine[2, big-endian] at TM payload offset 19. */` (`:91-92`), `/* Verifies that TC[9,2] (report time) emits TM[9,2] carrying the current CUC time, and that an unknown service-9 subservice is refused. */` (`:508-509`), `/* ECSS: 2 = Report time */` (`:531`), `/* Unknown service-9 subservice: refused, no time report TM. */` (`:548-549`). Code identifiers and comments remain English. No non-English content introduced. |
| Rationale | Satisfies norm N2 ("All project documentation and all code comments shall be in English", `docs/rules.md:35-44`). |

### DOC-R4-002 — Norm N8 — documentation is current — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | `tests/unit/test_pus.c:506-556` (new test + helper); `docs/architecture/pus-layer.md:108` (service 9 row); `docs/requirements/SRS.md:80-81` (SRS-REQ-010); `docs/requirements/traceability.yaml:119-126` (SRS-REQ-010 lists `tests/unit/test_pus.c`); `docs/vv/test-spec/ts-1-protocol-core.md:162` (N22-TS1-52 S9,2 report time, already documented as covered) |
| Blocks commit/push? | No |
| Evidence | The new test exercises already-documented Service 9 time-report behavior: `docs/architecture/pus-layer.md:108` lists "| 9 | Time: UTC correlation set, CUC time report |"; SRS-REQ-010 (`docs/requirements/SRS.md:80-81`) requires "the UTC correlation shall be settable from ground and reported in CUC format" (`method: Tst | Dem`); the traceability manifest already lists `tests/unit/test_pus.c` as a test of SRS-REQ-010 at file level; the test spec `docs/vv/test-spec/ts-1-protocol-core.md:162` already records N22-TS1-52 "S9,2 report time" as a nominal covered case, which this test implements. No API/interface/externally visible behavior change; no production source modified. No ICD/SRS/SDD/user-manual/release-note impact; no traceability-requirement change required. The `test_pus` target is already registered in `tests/CMakeLists.txt` (no build/CI registration documentation missing). Per norm N8 as scoped by N15/N17, no external documentation update is required for a test-only change implementing already-documented behavior. |
| Rationale | Documentation is current; no stale or dangling reference introduced. |

### DOC-R4-003 — Norm N16 — no revision history change needed — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | N/A (no document with a N16 `## History` table is touched; `tests/unit/test_pus.c` is test source) |
| Blocks commit/push? | No |
| Evidence | The change touches only test source (`tests/unit/test_pus.c`), which carries no per-document `## History` table. Per norm N16 interpretation note 3 (`docs/rules.md:495-501`), `## History` tables are required for substantive engineering/system/quality/V&V/planning documents; code-level history is carried by commit messages (norm N7 dual push). Because there is no documentation change, no history-table entry is required; the commit itself carries the change record per N16/N8. `CHANGELOG.md` is release-oriented and is not affected by a test-only change with no documentation delta. |
| Rationale | Correct application of N16 to a test-only change; no deficiency. |

### DOC-R4-004 — Test file comments are clear and accurate — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | `tests/unit/test_pus.c:91-92` (`stream_has_time_report` comment), `:506-509` (section + function comment), `:520`, `:526`, `:529`, `:531`, `:537`, `:540-541`, `:548-549` (inline comments), `:1230-1234` (`FF_TEST_MAIN` registration) |
| Blocks commit/push? | No |
| Evidence | Comments precisely state service/subservice, payload, expected result, and error-path expectations, and match the assertions in their blocks: `:537`/`stream_has_time_report(42u, 0u)` ("TM[9,2] must carry the frozen CUC time"), `:540-541`/re-report at coarse 100 ("the emitted coarse must track the new time"), `:548-549`/`FF_PusIngest(...) == FF_NOT_FOUND` and `!stream_has_time_report(100u, 0u)` ("refused, no time report TM"). The payload-offset-19 claim in `:91-92` is consistent with the sibling helper `stream_has_function_report` (`:63`, `:72`: TM secondary header is 13 octets, payload starts at offset 19 from packet start) and with CCSDS/PUS framing (6-octet primary header + 13-octet secondary header). No over-claim: the function comment (`:508-509`) scopes verification to TC[9,2]→TM[9,2] CUC payload carriage and unknown-subservice refusal; it does not claim verification-TM emission (`ack_flags = 0u`, `:529`, "isolate the handler: no verification TM"). The test is registered in `FF_TEST_MAIN` at `:1230-1234`. |
| Rationale | Clear, accurate, internally consistent with the code; no deficiency. |

### DOC-R4-005 — Documentation findings that block commit/push — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | N/A |
| Blocks commit/push? | No |
| Evidence | All findings above are Minor/conforming with no deficiency; there are no Major findings. All reported gates pass (build, CTest, Lean, trace, format, license). Norm N17 (`docs/rules.md:287-313`) requires the documentation-review loop closed before commit/push; this report closes the loop with all findings resolved/conforming. Commit/push authority remains with the `aeronautics-space-fsw-engineer` agent (norm N7) only after all applicable gates/norms pass and full sprint scope is complete. |
| Rationale | No documentation finding blocks commit/push. |

---

## 2. Overall conclusion

- **No documentation finding blocks commit/push.**
- DOC-R4-001 (N2 English), DOC-R4-002 (N8 currency), DOC-R4-003 (N16 history), DOC-R4-004 (comment clarity/accuracy), and DOC-R4-005 (blocking findings) are all **Minor — conforming**; no deficiency.
- The change is test-only, implements already-documented test case N22-TS1-52 (`docs/vv/test-spec/ts-1-protocol-core.md:162`) under SRS-REQ-010 (`docs/requirements/SRS.md:80-81`), introduces no external-documentation impact, and requires no revision-history entry under N16.
- Documentation-review conclusion: **APPROVED** (no findings requiring correction; review loop closed).

**Reviewer boundary statement:** I am the software documentation reviewer (R4) under norm N17. I have no engineering, safety, certification, release, commit/push, or risk-acceptance authority. Commit/push authority rests with the `aeronautics-space-fsw-engineer` agent (norm N7) only after all applicable gates/norms pass and the full sprint scope is complete; formal release/safety/security decisions rest with authorized humans. This report is advisory evidence for that decision.

---

## 3. Evidence consulted

- `tests/unit/test_pus.c` working-tree diff (HEAD 29cba32 → working tree): hunk `@@ -88,6 +88,37 @@` (new `stream_has_time_report` helper), `@@ -503,6 +534,58 @@` (new section header + `test_service9_report_time`), `@@ -1228,8 +1259,8 @@` (`FF_TEST_MAIN` registration).
- `docs/architecture/pus-layer.md:99-121` — implemented services; Service 9 row documents "Time: UTC correlation set, CUC time report".
- `docs/requirements/SRS.md:80-81` — SRS-REQ-010 "Time (S9): the UTC correlation shall be settable from ground and reported in CUC format. `method: Tst | Dem`."
- `docs/requirements/traceability.yaml:119-126` — SRS-REQ-010 traces tests to `tests/unit/test_pus.c` (file level).
- `docs/vv/test-spec/ts-1-protocol-core.md:160-165` — N22-TS1-50/51/52/53/55 S9 test cases; N22-TS1-52 (S9,2 report time) documented as covered; N22-TS1-55 (unexpected 9,2 payload) recorded as TBD/gap under NC-EXT-47 §7.3, consistent with this test not asserting a payload oracle.
- `docs/rules.md:35-44` (N2 English), `:110-120` (N8 documentation), `:272-286` (N16 revision history), `:287-313` (N17 documentation review), `:495-501` (interpretation note 3, per-document History scope).
- `tests/CMakeLists.txt` — `test_pus` target already registered (no build-documentation delta).

---
Committed and pushed with the reviewed change (norm N11).
