# R4 Documentation Review — frick-fsw PUS service 3 ordering fix (NC-EXT-211)

**Reviewer role:** Senior Aeronautics and Space Software Documentation Engineer (R4 profile), independent documentation review under norm N17.  
**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD b6bf060 (working tree).  
**Date:** 2026-11-12 (UTC).  
**Change under review:** `src/pus/pus_services.c` — reorder so `FF_TimeGetCuc()` is called BEFORE `g_hk_reports[i].enabled = true;` in both `svc3_create` and `svc3_enable`, to prevent a concurrent `FF_PusEmitPeriodicHk()` from observing `enabled=true` with a zero-initialized `last_emission`.  
**Scope:** Source-code fix only. No API change, no interface change, no externally visible behavior change, no new public symbol, no traceability-requirement change. Not hardware/target related; host-executable. NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01, OBCP engine not touched.

**Gate evidence reported by submitter (not re-validated by this review):** `make build` exit 0 (0 warnings); CTest exit 0 including `test_pus.c` (exercises `FF_PusEmitPeriodicHk`); Lean `lake build`; `make trace` PASS (120/76/30/14/68); format-check PASS (pinned clang-format 18); license OK.

---

## 1. Findings

Each finding is classified **Major** or **Minor**, carries file:line references, and states whether it blocks commit/push. Confirmations (no deficiency) are classified Minor and explicitly marked "conforming".

### DOC-R4-001 — Norm N2 (English) — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | `src/pus/pus_services.c:233-235` and `:269-271` (added comments) |
| Blocks commit/push? | No |
| Evidence | Both added comments are in English: "Initialize last emission time before enabling so a concurrent `FF_PusEmitPeriodicHk()` never observes `enabled=true` with a stale (zero-initialized) `last_emission`. (NC-EXT-211)". |
| Rationale | Satisfies norm N2 (all repo content in English). No non-English content introduced. |

### DOC-R4-002 — Added comments accurately describe the change (NC-EXT-211 ordering fix) — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | `src/pus/pus_services.c:233-235` and `:269-271`; compare `docs/planning/sprint-plan.md:338` |
| Blocks commit/push? | No |
| Evidence | The comments state that the last-emission time is initialized **before** enabling, cite `(NC-EXT-211)`, and name the hazard (a concurrent `FF_PusEmitPeriodicHk()` observing `enabled=true` with a stale/zero-initialized `last_emission`). The diff confirms `FF_TimeGetCuc()` now precedes `g_hk_reports[i].enabled = true;` in both `svc3_create` and `svc3_enable`. `docs/planning/sprint-plan.md:338` defines NC-EXT-211 identically: "`svc3_create`/`svc3_enable` set `enabled = true` and then read the current CUC time, so a future caller of `FF_PusEmitPeriodicHk()` interleaved between those two steps could observe an enabled report whose `last_emission` is still uninitialised." |
| Rationale | The comments are accurate, correctly cite NC-EXT-211, and make no over-claim (no claim of target/HIL validation; the emitter remains inert with no production caller). |

### DOC-R4-003 — Norm N8 / external documentation impact — Minor — conforming (no deficiency)

| Field | Value |
|---|---|
| Classification | Minor — conforming |
| File:line | Change is source-only; cross-checked `docs/architecture/time-and-timecode.md:48` and `docs/requirements/traceability.yaml` (0 mentions of "periodic"; V&V-FIND-006 deferred under ESC-HK-05). |
| Blocks commit/push? | No |
| Evidence | No API/interface/behavior change visible externally; no ICD/SRS/SDD/user-manual/release-note impact; no traceability-requirement change; no document prescribes the previous ordering as required behavior. `docs/architecture/time-and-timecode.md:48` discusses the test-harness frozen-UTC seam, not the `svc3` ordering, so it is not invalidated by this fix. |
| Rationale | No external documentation update is required for this internal ordering fix, consistent with norm N8 as scoped by the submitter. |

### DOC-R4-004 — NC-EXT-211 register currency (norms N8 / N16) — Minor — documentation currency / tracking

| Field | Value |
|---|---|
| Classification | Minor — documentation currency / tracking (not a correctness defect in the change) |
| File:line | `docs/planning/sprint-plan.md:338` (NC register); related `CHANGELOG.md [Unreleased]` (N16 per-change entries) |
| Blocks commit/push? | No |
| Evidence | `docs/planning/sprint-plan.md:338` records NC-EXT-211 as **OPEN — DEFERRED under N21**, requiring resolution (ordering or a claim of single-threaded access) **before activation (ESC-HK-02)**; owner: FSW with concurrency/architecture agreement; verifier: independent V&V. PA reviews `FRICK-REVIEW-0440`/`0442`/`0451`/`0457` confirm NC-EXT-211 remains OPEN/authority-owned and that a test-comment mis-citation "must not be read as progress on NC-EXT-211". This change implements the ordering remedy; per the existing governance record, closure additionally requires concurrency/architecture agreement + independent V&V concurrence, and resolution is a precondition for activation, not for this source commit. |
| Rationale | Per norm N8 (complete, current documentation) and N16 (detailed revision history), recommend updating `docs/planning/sprint-plan.md:338` (and considering a brief `CHANGELOG.md [Unreleased]` entry) to record the implemented remedy and its pending disposition. This is a tracking item, not a gate: all gates pass, the emitter remains inert (no production caller), and NC-EXT-211 is explicitly deferred under N21. |
| Recommendation | Update `docs/planning/sprint-plan.md:338` in the same or the immediately following commit to reflect the implemented fix and pending owner/V&V disposition. Do **not** hold this source-only commit for it. |

---

## 2. Overall conclusion

- **No documentation finding blocks commit/push.**
- DOC-R4-001 (N2 English), DOC-R4-002 (comment accuracy vs NC-EXT-211), and DOC-R4-003 (N8 external-doc impact) are conforming; no deficiency.
- DOC-R4-004 is a **Minor** documentation-currency tracking item: the NC-EXT-211 register still shows OPEN although the ordering remedy is implemented. It should be closed out promptly (same or following commit) to satisfy N8/N16, but it does not block this source-only commit/push.

**Reviewer boundary statement:** I am the software documentation reviewer (R4) under norm N17. I have no engineering, safety, certification, release, commit/push, or risk-acceptance authority. Commit/push authority rests with the `aeronautics-space-fsw-engineer` agent (norm N7) only after all applicable gates/norms pass and the full sprint scope is complete; formal release/safety/security decisions rest with authorized humans. This report is advisory evidence for that decision.

---

## 3. Evidence consulted

- `src/pus/pus_services.c` working-tree diff (HEAD → working tree): two hunks, `@@ -230,12 +230,13 @@` and `@@ -265,11 +266,12 @@`.
- `docs/planning/sprint-plan.md:338` — NC-EXT-211 register entry.
- `docs/reviews/FRICK-REVIEW-0440-PA-NC-EXT-235.md:115,181`; `FRICK-REVIEW-0442-PA-NC-EXT-235-REVIEW.md:124,171`; `FRICK-REVIEW-0451-PA-NC-EXT-235-REVIEW.md:127,174`; `FRICK-REVIEW-0457-PA-20261104-REREVIEW3.md:141` — NC-EXT-211 remains OPEN/authority-owned.
- `docs/architecture/time-and-timecode.md:48` — test-seam discussion (not prescribing `svc3` ordering).
- `docs/reviews/2026-10-24-hk-periodic-vv-final-delta-review.md:217` — historical V&V-FIND-003 record (pre-fix state).
- `docs/requirements/traceability.yaml` — no periodic/HK-emission requirement (0 mentions).
- `CHANGELOG.md` — per-change `[Unreleased]` entries maintained per norm N16.
