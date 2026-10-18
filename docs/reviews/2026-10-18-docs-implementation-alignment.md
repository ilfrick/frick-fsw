# Documentation-vs-Implementation Alignment (2026-10-18)

**Document id:** FRICK-REVIEW-20261018-DOCS-ALIGN  
**Reviewer session:** R4 documentation engineer (independent audit)  
**Reviewer profile:** R4 (SW documentation), with cross-check against FSW source evidence  
**Reference revision:** commit `1fdbb83` (S8 reconciliation batch, pushed to origin/main + gitea/main)  
**Items under review:** all living `.md` docs in `/home/nicola/fsw/frick-fsw` (focused subset of descriptive/navigational and governed artifacts)  
**Date:** 2026-10-18  
**Scope:** factual alignment between implemented code and the controlled documentation set. Governed artifacts (ICD FRICK-ICD-001) were **not edited**; discrepancies are recorded as NCs. Requirement IDs/semantics/traceability links were left unchanged.  
**Independence declaration:** fresh R4 review context; no part in authoring the FSW code paths cited below. No approval/certification/compliance/qualification claim is made.  
**Norms applied:** N1–N22 (see `docs/rules.md`), with emphasis on N4 (traceability), N7 (commit authority boundary), N8 (current/deployable docs), N11 (evidence retention), N16 (revision history), N17 (documentation review), N21 (non-functional findings as NCs; fix factual errors).

## 1. Docs reviewed (living set; subset checked in detail)

**Status/trace/repo surfaces:** `README.md`, `docs/requirements/SSS.md`, `docs/requirements/SRS.md`, `docs/design/software-design.md`, `docs/architecture/component-gap-analysis.md`, `docs/architecture/pus-layer.md`, `docs/vv/verification-plan.md`, `docs/quality/licensing.md`, `docs/planning/sprint-plan.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/status/status.json`, `docs/vv/test-spec/ts-9-obcp-integration.md`, `docs/interfaces/ICD.md` (governed; not edited). Other living docs were scanned for high-confidence mismatches and no material factual errors were identified (e.g. `docs/engineering/software-description.md` matches N7 default backend).

## 2. Primary source evidence (cited)

| Evidence | File:line(s) | What it shows |
|---|---|---|
| N7/ESA PUS ASN.1 backend default | `CMakeLists.txt:25-26` | `FF_USE_PUS_ASN1 ON`, `FF_USE_N7_OBCP ON` |
| Main uses N7 PUS | `src/main/fsw_main.c:82` | `ff_pus_handle_packet(&bus, &req)` (N7/ESA path) |
| PUS backend/type | `src/pus/pus.h:10,81` | PUS backend descriptor includes N7/ESA types |
| S1 verification subservice mapping | `src/pus/pus.c:178-179,263` | `FF_PusSendVerification` maps `FF_PUS_STAGE_PROGRESS` → subservice 5 only; no subservice 6 emission path exists |
| S3/S6/S11/S20 registration | `src/pus/pus_services.c:495,517-522` | Registered services include S3 (subservices 1,3,5,6,27), S6/11 present, S20 registers `{20,1}` and returns TM[20,4] for `{20,3}` reports |
| Unit/integration tests count | `tests/CMakeLists.txt` | 43 CTest registrations observed (matches `verification-plan` §9 after correction) |
| OBCP code exists and default-enabled | `src/obcp/ff_obcp.c`, `src/obcp/ff_obcp.h`, `CMakeLists.txt:26` | Implementation present; `FF_USE_N7_OBCP=ON` by default; deferred integration tracked under NC-S9-01 |
| Link backends | `src/transport/link/` | `ff_link_ip.c`, `ff_link_tsn.c`, `ff_link_seg.c` + base API present |

## 3. Findings (traceable)

| ID | doc | claimed statement | evidence file:line | severity | fix/NC |
|---|---|---|---|---|---|
| F-018-01 | `README.md` (PUS implementation row) | Default PUS-C backend described as "open CCSDS codec" | `CMakeLists.txt:25-26`, `src/main/fsw_main.c:82`, `src/pus/pus.h:10,81` | Minor | **Fixed:** corrected to N7/ESA PUS backend (ASN.1 codec path enabled by `FF_USE_PUS_ASN1=ON`); header/row revised to reflect N7/ESA default with open-CCSDS backend available as an alternative. |
| F-018-02 | `README.md` (S3 subservices row) | Listed S3 subservices as `1, 2, 3, 4, 27` | `src/pus/pus_services.c:517-518` | Minor | **Fixed:** corrected to registered set `{1,3,5,6,27}`. |
| F-018-03 | `docs/design/software-design.md` | OBCP marked "planned / no code" | `src/obcp/ff_obcp.c`, `src/obcp/ff_obcp.h`, `CMakeLists.txt:26` | Minor | **Fixed:** updated status to reflect implementation present (default-built, `FF_USE_N7_OBCP=ON`); note deferred integration (NC-S9-01) preserved. |
| F-018-04 | `docs/design/software-design.md` (link backends) | Link backends bullet missing IP/TSN/SEG entries | `src/transport/link/` (directory contents) | Minor | **Fixed:** added `ff_link_ip`, `ff_link_tsn`, `ff_link_seg`. |
| F-018-05 | `docs/architecture/pus-layer.md` (§6) | S20 row omitted TC[20,1] set side | `src/pus/pus_services.c:495,521-522` | Minor | **Fixed:** added TC[20,1] set; TM[20,4] mapping noted. |
| F-018-06 | `docs/vv/verification-plan.md` (§9) | CTest count stated 40/40 | `tests/CMakeLists.txt` (43 CTest registrations) | Minor | **Fixed:** corrected to 43/43 in §9. |
| F-018-07 | Header metadata (6 docs) | Revision headers lagged their own last history row (pure header-sync) | existing `History` table for each doc | Minor (editorial) | **Fixed (metadata only):** `docs/architecture/component-gap-analysis.md` 0.2→0.4; `docs/quality/licensing.md` 1.6→1.7; `docs/requirements/SSS.md` 0.19→0.20; `docs/requirements/SRS.md` 1.35→1.36; `docs/vv/verification-plan.md` 0.1.6→0.1.7; `docs/vv/test-spec/ts-9-obcp-integration.md` 0.1→0.4. No requirement content changed. |
| F-018-08 | `docs/status/handover.md` (§4), `docs/status/loop-audit.md`, `docs/status/status.json` | Baseline/commit state referenced S8 batch as uncommitted or stale baseline | git HEAD `1fdbb83` (pushed to both remotes); working tree modified but HEAD is committed | Minor | **Fixed:** updated baseline-commit references to `1fdbb83`, clarified hosted-CI wording, and bumped `updated` to 2026-10-18. |
| F-018-09 | `docs/planning/sprint-plan.md`, `docs/status/loop-audit.md`, `docs/status/handover.md` | NC/loop history needed to record audit | audit findings | Minor | **Fixed:** added history rows for 2026-10-18; registered NC-EXT-94/95/96 in `sprint-plan.md`. |
| F-018-10 | `docs/interfaces/ICD.md` (governed) | FRICK-ICD-001 §4 lists S1 subservice 6 (progress failure); §4 S20 row omits TC[20,1] | `src/pus/pus.c:178-179,263` (no S1 subservice 6); `src/pus/pus_services.c:495,521-522` (S20 has TC[20,1]) | Minor (governed) | **NC recorded (not edited):** NC-EXT-94 (S1,6 never emitted); NC-EXT-95 (S20 TC[20,1] missing). No wire behaviour or requirement semantics changed. |
| F-018-11 | Cross-cutting documentation drift | PUS default/backend, S3 codes, OBCP/link, CTest count, headers, baseline state collectively inconsistent | union of above evidence | Minor | **NC recorded:** NC-EXT-96 (corrected in this audit; independent R4 confirmation requested). No requirement IDs/semantics/traceability links changed. |

## 4. Non-conformances registered

The following NCs were registered in `docs/planning/sprint-plan.md` (sequence continues from NC-EXT-93):

| NC | Source/Review context | Severity | Finding | Disposition/plan | Owner | Verifier | Status |
|---|---|---|---|---|---|---|---|
| NC-EXT-94 | ICD §4 S1 subservice 6 vs `src/pus/pus.c:178-179,263` | Minor | ICD lists S1 subservice 6 (progress failure) as answered on the wire, but `FF_PusSendVerification` maps `FF_PUS_STAGE_PROGRESS` → subservice 5 only and no code path emits TM[1,6]. | **OPEN — spec-vs-implementation discrepancy.** Either implement the progress-failure emission path or annotate the ICD row as reserved-but-not-generated. No wire behaviour changed; no PUS-C compliance claim made. Non-functional under N21. | FSW + interface/requirements authority | independent R3/R4 | OPEN |
| NC-EXT-95 | ICD §4 S20 row vs `src/pus/pus_services.c:495,521-522` | Minor | ICD S20 row lists only the parameter report (TM) side although code registers `{20,1}` set and `{20,3}` report (`TM[20,4]`). | **OPEN — controlled wire-profile completeness.** Add the registered TC[20,1] set subservice (and its payload) to the ICD row. `docs/architecture/pus-layer.md` §6 was corrected in this audit. Non-functional under N21. | FSW + interface authority | independent R3/R4 | OPEN |
| NC-EXT-96 | Descriptive/status docs drift vs implementation and pushed baseline `1fdbb83` | Minor | README PUS default-backend/S3 codes; SDD OBCP/link; pus-layer S20; verification-plan §9; six revision headers; status/handover/loop-audit baseline state were stale. | **CORRECTED in this audit (this report + 13 modified files). OPEN for independent R4 confirmation.** No requirement IDs/semantics/traceability links changed; no NC closed; no gate/phase/release approval implied. Non-functional under N21. | FSW/documentation | independent R4 | OPEN (confirmation only) |

## 5. Residual gaps

- **ICD authority boundary (governed):** S1,6 and S20 TC[20,1] require interface authority disposition; intentionally not edited in this audit (respect for governed artifacts and to avoid crossing authority boundaries). 
- **Review indexing:** `docs/reviews/README.md` was not indexed with this report (intentional to respect immutability of existing `docs/reviews/*.md` as declared; FSW engineer should index it as part of commit/closeout).
- **Scope deferrals remain:** OBCP is a candidate integration (host-callback only) tracked under NC-S9-01; PUS-C S3/S6/S11 profile is **PROVISIONAL** (ADR-0020); target ECSS criticality B is an aspiration only. No new deferrals introduced.
- **Traceability/Lean:** `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` must remain green; no Lean links were modified (all edits are descriptive/navigational or metadata with exception of controlled architecture row fix in pus-layer which is consistent with implementation).

## 6. Uncertainty, confidence, and open questions

**Confidence:** **Medium–High** for the descriptive/navigational and implementation-coupled docs reviewed in detail. 
- High: direct source evidence for all code-coupled claims (backend, services, counts, files).
- Medium: governed ICD items require authority confirmation (not implementation change alone); header-sync was mechanical but correctness of history continuity is assumed; baseline identity (pushed HEAD `1fdbb83`) is factual from git.
- Low: no low-confidence material claims made.

**Assumptions, TBD/TBC, stale-data risk:**
- Assumed existing `History` tables are the canonical source for each doc's last revision/date (used for header-sync to avoid changing history content).
- Recheck trigger: any code change affecting PUS verification mapping (S1), S20 service set, OBCP default, or CTest set.
- Consequences if wrong: ICD/README would misstate implemented wire behaviour (mitigated by NC-EXT-94/95; no behaviour change). Documentation drift could confuse operators/developers (corrected; confirmation pending).

**Open questions:**
1. For NC-EXT-94: does the project require TM[1,6] (progress failure) emission in this phase? If not, should ICD mark S1,6 as "reserved/not generated" or remove?
2. For NC-EXT-95: confirm TC[20,1] payload/constraints are the registered form; add to ICD with correct effectivity.
3. For NC-EXT-96: independent R4 confirmation requested (no requirement/trace changes made). Is the header-sync policy acceptable as pure metadata correction?
4. Should `docs/reviews/README.md` be updated to index this report now (immovable constraint noted), or deferred to FSW engineer closeout?

## 7. Conclusion

- [x] Factual errors fixed where not governed: README, SDD, pus-layer, verification-plan, headers (6), status/handover/loop-audit, sprint-plan, ts-9 header.
- [x] Governed ICD mismatches registered as NC-EXT-94/95 (not edited). 
- [x] Cross-cutting drift recorded as NC-EXT-96 (corrected; confirmation pending).
- [ ] Independent R4/R3 confirmation for NC-EXT-94/95/96 (recommended). 
- [ ] Index report in `docs/reviews/README.md` (deferred to respect stated immutability; FSW engineer action).

**No compliance/certification/qualification/approval claimed.** All changes are descriptive/navigational or metadata-corrective except non-governed implementation-consistent corrections; no requirement IDs/semantics/traceability links were changed; `trace.py` remains expected to pass on current manifest.

---

*Retained report written 2026-10-18 by R4 documentation engineer audit (non-functional under N21). Evidence preserved in working tree; commit/push remains the exclusive authority of the `aeronautics-space-fsw-engineer` per N7.*