# Documentation Set Review — Delta Re-Review (profile R4, norm N17)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                       | FRICK-REVIEW-0009                      |
| Reviewer session                  | independent agent (fresh context, norm N9) |
| Reviewer profile                  | R4 - Senior SW documentation reviewer  |
| Predecessor report                | `docs/reviews/2026-10-10-doc-set-r4-review.md` (FRICK-REVIEW-0005, **REJECTED**, NCs DOC-01..DOC-14) |
| Reference revision                | working tree on base commit `a69732f` (branch `main`, uncommitted M2/doc-program delta) |
| Items under review                | Disposition of DOC-01..DOC-14 (FRICK-REVIEW-0005) and the new documents `docs/development-plan.md` (FRICK-PLAN-0001) and `docs/roadmap.md` rev 0.3, plus their cross-references (`README.md`, `AGENTS.md`, `docs/rules.md`) |
| Date                              | 2026-10-10                             |
| Scope                             | Delta verification (norm N12/N17): re-apply all rules to the changed scope, confirm each predecessor NC's claimed disposition against the working tree, re-check the machine-checked counts (trace gate, coverage artefact, ctest, CI claims, Lean transcript) and the internal/cross-document consistency of the new planning documents |
| Independence declaration          | Reviewer took no part in authoring, fixing or reviewing this work; fresh context; every claim below is based solely on the repository evidence at the stated revision and on `docs/rules.md` (N1-N17) |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

This report re-reviews **only** the dispositions claimed for the fourteen
non-conformances of the predecessor R4 documentation review
(`docs/reviews/2026-10-10-doc-set-r4-review.md`, REJECTED, 1 Major + 9 Minor +
4 Comments) and the two documents introduced since that review
(`docs/development-plan.md`, `docs/roadmap.md` rev 0.3). It does not re-audit the
M2 code base, the R1/R2/R3 technical reviews or the M2 delta re-reviews, which
are recorded closed in their own reports (and were spot-checked where they bear
on DOC-01).

Read-only checks performed by this reviewer:

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` (exit 0;
  48 requirements, srs=25/sss=16/gap=7, 41 Lean theorems, 9 CI evidence claims).
- `ctest --test-dir build/native` (5/5 PASS) and direct execution of the five
  prebuilt suite binaries to re-derive the documented per-suite check counts.
- Reading `build/coverage/coverage.json` and cross-checking it against the README
  metrics panel, `docs/vv/verification-results.md` and `docs/status/status.json`.
- `python3 tools/scripts/metrics.py` (panel regeneration to stdout only; no file
  written except this report) compared byte-for-byte with the README panel block.
- Grep sweeps for `N1-N1x`/`R4`, for `FRICK-*` document ids, for `# evidence:`
  markers in both workflows, and for `## History` across `docs/**`.
- A recursive relative-link resolution scan over every Markdown file (0 broken).

Excluded: the correctness of the flight code and of the M2 mitigations
(owned by R1/R2/R3), on-target hardware bring-up (GAP-A7), and any file not
present at the reference revision. No file was modified except this report.

## 2. References

- Project norms: `docs/rules.md` (N1-N17; interpretation notes; N12/N16/N17).
- Predecessor review: `docs/reviews/2026-10-10-doc-set-r4-review.md` (FRICK-REVIEW-0005).
- M2 reviews + deltas: `docs/reviews/2026-10-10-m2-{time-base-r1,vv-r2,sss-r3}{,-delta}-review.md`.
- New documents: `docs/development-plan.md` (FRICK-PLAN-0001), `docs/roadmap.md` (FRICK-RM-0001).
- Records: `docs/vv/verification-results.md`, `docs/status/status.json`,
  `build/coverage/coverage.json`, `verification/lean/lake-build.log`.
- CI: `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `tools/scripts/trace.py`.
- Review template: `docs/reviews/review-template.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2, CCSDS 301.0-B-4.

## 3. Verified without finding

Independent re-runs/observations at the reference revision:

1. **Traceability gate converges.** `trace.py` exits 0: **48 requirements
   (srs=25, sss=16, gap=7), 41 Lean theorems, 9 CI evidence claims** —
   matching `README.md:72`, `docs/requirements/traceability.md:18-20` and
   `docs/vv/verification-results.md:23,115-118`.
2. **ctest and per-suite counts.** `ctest --test-dir build/native` reports
   **5/5 PASS**; the binaries report `test_ccsds` 15, `test_sb` 20, `test_tbl`
   9, `test_pus` 29, `test_time` 101 (`failures=0`), exactly as recorded in
   `docs/vv/verification-results.md:23,99-103`.
3. **Coverage matches the artefact.** `build/coverage/coverage.json` is
   `lines 845/1446, funcs 114/156`; `README.md:76` and
   `verification-results.md:107,23` state 845/1446 (58.4%) / 114/156. (Stale
   *other* instances of 843/1446 are raised as D-4.)
4. **README panel is machine-generated and current.** `metrics.py` stdout is
   byte-identical to the README `<!-- METRICS-BEGIN -->…<!-- METRICS-END -->`
   block, including the `documentation review` semaphore and the
   `documentation_review` gate present in `status.json:24`.
5. **CI claims and markers are consistent.** Both workflows carry the same nine
   distinct markers (`ci-host-build`, `ci-dual-build`, `ci-static-analysis`,
   `ci-simulator-smoke`, `ci-lean`, `ci-trace`, `ci-docs`, `ci-rtems-cross-build`,
   `ci-ci-pipeline`); `ci-lean` is present in **both** files
   (`.github/workflows/ci.yml:92`, `.gitea/workflows/ci.yml:109`); the manifest
   references 8 (all declared) — the intended N1 asymmetry.
6. **Lean transcript is re-derivable.** `verification/lean/lake-build.log` ends
   with `[17/17] Built frick_verify_checks` / `Build completed successfully`;
   `Frick/Time.lean` contains 11 `theorem` declarations.
7. **No broken relative links.** A recursive scan over all Markdown (including
   the two new documents) resolves every non-external target (0 broken).
8. **N17 machinery coherent.** Norm N17 (`rules.md` N17 section + enforcement
   summary), profile R4 (`engineering/independent-review.md`), the AGENTS
   checklist item 7 (`AGENTS.md:96-100`), the `documentation_review` gate
   (`status.json:24`) and the V&V record (`verification-results.md:135-141`)
   agree on scope/trigger/closure.
9. **M2 delta reports exist and match the log.** The three delta reports
   (`2026-10-10-m2-*-delta-review.md`) exist and are listed in
   `docs/reviews/README.md:12-14` and `verification-results.md:150-155`.
10. **New documents are internally sound as drafts** (see §6):
    `development-plan.md` objectives/milestones and the `roadmap.md` M1..M5
    statuses agree with `status.json:8-14` (M1 done, M2 done, M3/M4 planned,
    M5 in progress).

## 4. Predecessor NC disposition (DOC-01..DOC-14)

| NC | Prev. sev. | Claimed disposition | Independent verification | Status |
| -- | ---------- | ------------------- | ------------------------ | ------ |
| DOC-01 | Major | M2 reports given terminal dispositions + delta refs | `m2-time-base-r1-review.md:139-146`, `m2-vv-r2-review.md:111-116`, `m2-sss-r3-review.md:151-155` now carry `closed` rows with evidence and delta pointers; the three delta reports exist; `verification-results.md:23,125-133,150-155` reference them consistently. One residual record contradiction (NF-01) — see D-6. | **CLOSED** (Major resolved; residual Minor D-6) |
| DOC-02 | Minor | README counts = 25/16/7/41 | `README.md:259` states "SRS = 25 · SSS = 16 · GAP = 7 · Lean theorems = 41", matching the gate and `traceability.md:18-20`. | **CLOSED** |
| DOC-03 | Minor | README panel includes documentation review gate | `README.md:60` lists `documentation review`; identical to `metrics.py` stdout and `status.json:24`. | **CLOSED** |
| DOC-04 | Minor | Revision headers aligned with History | 9/10 named docs aligned (SRS 0.3, SSS 0.3, traceability 0.2, driver-development 0.2, licensing 0.2, tool-qualification 0.1.4, verification-results 0.3, tooling 0.2, formal-verification 0.2). **`roadmap.md` still has header 0.2 vs its History 0.3** — see D-2. | **PARTIAL** (D-2) |
| DOC-05 | Minor | N17/R4 propagated (README/AGENTS/CONTRIBUTING/template/plan/CHANGELOG) | README N1-N17, CONTRIBUTING N1-N17 + R4 row (`:4,27,56`), review-template R4 + N1-N17, verification-plan, CHANGELOG N17/R4 entry (`:31-37`) all correct. **Residual: `AGENTS.md:12` heading still "norm N1-N12"; `driver-development.md:11` and `service-development.md:11` still "(N1-N16)"** — see D-3. | **PARTIAL** (D-3) |
| DOC-06 | Minor | `FRICK-ARCH-0006` collision fixed; review ids normalised; uniqueness check added | ARCH collision **fixed** (`mapping-cfs.md:3` = `FRICK-ARCH-0007`; ids unique across `docs/architecture/`). But review ids were **not** normalised: `FRICK-REVIEW-0005` is used by both the R2 delta and the R4 doc-set report; `FRICK-REVIEW-0008` by both the R1 delta and the R3 delta; and no id-uniqueness check exists in the `docs` CI job. See D-1. | **PARTIAL** (D-1) |
| DOC-07 | Minor | Review log gets M2 R1 + R4 + delta rows | `docs/reviews/README.md:9,11` add the M2 R1 and R4 rows; `:12-14` add the three delta rows. (The R4 delta row is added by this report.) | **CLOSED** |
| DOC-08 | Minor | History tables in tooling.md + glossary.md | `tooling.md:116-121` and `glossary.md:37-41` now have `## History`. Residual: the recommendation to document the ADR/short-architecture-note exemption is not done — see D-7. | **CLOSED** (residual Comment D-7) |
| DOC-09 | Minor | formal-verification.md adds Time/LeanOmega + M2 scope | `formal-verification.md:22,26` add `Frick/LeanOmega.lean` and `Frick/Time.lean`; `:49-64` extends scope with the M2 time properties; rev 0.2 + History. | **CLOSED** |
| DOC-10 | Minor | Duplicate README `## Repository layout` removed | `README.md` has a single `## Repository layout` (line 264) followed by `## Documentation index` (line 278). | **CLOSED** |
| DOC-11 | Comment | `documentation_review` gate true state | `status.json:24` now has a real counterpart report (FRICK-REVIEW-0005 exists), so setting it `green` is no longer premature-by-absence; residual argument that the state is optimistic while open Minors remain — see D-9. | **CLOSED** (residual Comment D-9) |
| DOC-12 | Comment | README/AGENTS index entries | `README.md:303-304` add `docs/status/status.json`, `docs/glossary.md` and `CHANGELOG.md`. | **CLOSED** |
| DOC-13 | Comment | tooling.md T12/T13 | `tooling.md:65-66` add `coverage.py` (T12) and `metrics.py` (T13); History rev 0.2 records it. | **CLOSED** |
| DOC-14 | Comment | overview.md S11 stub | `overview.md:27` (ASCII) and `:58` (Mermaid) both list `S11`; consistent with `README.md:110` and `roadmap.md`. | **CLOSED** |

Summary: **DOC-01 (Major) resolved; DOC-02/03/07/08/09/10/11/12/13/14 closed;
DOC-04/05/06 partially closed** (residuals D-1..D-3), plus new Minor D-4/D-5/D-6
and Comments D-7..D-10.

## 5. New findings (delta)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-1 | Minor | Review-report document ids (norm N8/N16; DOC-06 residual) | The id normalisation introduced **two collisions** and added no guard: `FRICK-REVIEW-0005` is claimed by both `docs/reviews/2026-10-10-m2-vv-r2-delta-review.md:5` and `docs/reviews/2026-10-10-doc-set-r4-review.md:5`; `FRICK-REVIEW-0008` by both `docs/reviews/2026-10-10-m2-time-base-r1-delta-review.md:5` and `docs/reviews/2026-10-10-m2-sss-r3-delta-review.md:5`. `CHANGELOG.md:83-84` asserts "review ids normalised to `FRICK-REVIEW-00nn`", which is inaccurate, and the `docs` CI job (`.github/workflows/ci.yml:120-138`) checks only file existence — no uniqueness check was added. | The four id lines above; `CHANGELOG.md:83-84`; `.github/workflows/ci.yml:120-138`. | Renumber the colliding reports to unique `FRICK-REVIEW-00nn` ids (0005/0008 free slots), add a document-id uniqueness check to the `docs` job, and correct the CHANGELOG wording. |
| D-2 | Minor | Revision headers vs History (norm N16; DOC-04 residual) | `docs/roadmap.md:5` still shows `Revision 0.2` while its own History table has a `0.3` row (`roadmap.md:101`). (`docs/quality/licensing.md:79-80` also lists its History rows out of chronological order — 0.2 before 0.1 — cosmetic.) | `roadmap.md:5` vs `:101`; `licensing.md:77-80`. | Bump `roadmap.md` front-matter to 0.3 (and order the licensing rows). |
| D-3 | Minor | N17/R4 propagation (norm N8/N17; DOC-05 residual) | Three current documents still enumerate the old norm set: `AGENTS.md:12` heading reads "norm N1-N12"; `docs/engineering/driver-development.md:11` and `docs/engineering/service-development.md:11` read "(N1-N16)". (The M2 point-in-time review reports legitimately retain N1-N16 as their historical scope.) | `AGENTS.md:12`; `driver-development.md:11`; `service-development.md:11`. | Update the three references to N1-N17 (the AGENTS §7 table and checklist item 7 already list N17). |
| D-4 | Minor | Coverage figures (norm N11) | The current artefact is **845/1446 (58.4%), funcs 114/156** (`build/coverage/coverage.json`; `README.md:76`; `verification-results.md:23,107`), but three places still record **843/1446 (58.3%), functions 113/156**: `CHANGELOG.md:28`, `docs/reviews/2026-10-10-m2-vv-r2-delta-review.md:111`, and the `verification-results.md:165` History row 0.3. | `build/coverage/coverage.json`; `CHANGELOG.md:28`; `m2-vv-r2-delta-review.md:111`; `verification-results.md:165`. | Align the three stale instances with the regenerated artefact (845/1446, 114/156, 58.4%). |
| D-5 | Minor | README cross-references vs roadmap/development-plan | `README.md:114-115` states "PUS service 23 file management and CFDP (M3); TCP/IP and TSN transport over PUS-C (M4)", but `roadmap.md:80-82` and `development-plan.md:137-139` move the virtual filestore / PUS service 23 / CFDP / IP+TSN into **M4**. Also `README.md:130` calls the PPS/driver work "planned milestones (M2/M4)" although M2 is done and the PPS software backend is delivered (`roadmap.md:24-47`). | `README.md:114-115,130`; `roadmap.md:80-82`; `development-plan.md:137-139`. | Update the README PUS/driver text to the rev-0.3 plan (file transfer set in M4; M2 delivered). |
| D-6 | Minor | M2 R1 NC-03/NF-01 record (norm N11/N12; DOC-01 residual) | The R1 **delta** report records NC-03 as "**NOT FULLY CLOSED**" with NF-01 "open" (`m2-time-base-r1-delta-review.md:59,110`) and its conclusion requires NF-01 resolved "before the M2 time base is committed/pushed" (`:87-96`), yet the R1 **main** report now marks NC-03 `closed` with "NF-01 resolved" (`m2-time-base-r1-review.md:141`), and `verification-results.md:127,131` states the R1 findings are resolved and "Open non-conformances: none". The NF-01 documentation fix **is present** in the tree (`time.c:12-15` "task context only … the ISR defers to a task"; `driver-development.md` §3 step 1 "must **not** call FF_TimeToneSignal directly"; `time-and-timecode.md:120-128`), but no delta entry verifies it, so the closure is asserted without recorded re-verification. | `m2-time-base-r1-delta-review.md:59,87-96,110`; `m2-time-base-r1-review.md:141`; `verification-results.md:127,131`; `time.c:12-15`; `driver-development.md` §3. | Record an NF-01 closure entry (in the delta report or a short addendum) with the task-context-only evidence, so NC-03 has a verified terminal disposition consistent with the main report. |
| D-7 | Comment | Architecture notes / ADR History (norm N16; DOC-08 residual) | `docs/architecture/{applications,mapping-cfs,platform,software-bus}.md` still have no `## History` table (overview, time-and-timecode, pus-layer do), and the recommended ADR/short-note exemption is not documented in the N16 section of `rules.md`. | `find docs -name '*.md'` vs the `## History` sweep; `rules.md` N16 section. | Add the four History tables (cheap) or document an explicit exemption in the N16 text; enforce in the `docs` gate. |
| D-8 | Comment | New-document cross-references | `docs/development-plan.md` (FRICK-PLAN-0001) is referenced by `README.md:16` and `roadmap.md:66` but not by `AGENTS.md`; the plan itself does not link `roadmap.md`; and its §6 M4 row (`:172`) omits OBJ-07 although §2 (`:41`) scopes OBJ-07 as "M3 (structure) / M4 (implementations)". | `development-plan.md:41,46,172`; `AGENTS.md:36`; `README.md:16`; `roadmap.md:66`. | Add the plan to the AGENTS repository map / docs list, cross-link roadmap <-> plan, and list OBJ-07 implementations in the §6 M4 row. |
| D-9 | Comment | `status.json` `documentation_review` | `status.json:24` is `green` while the predecessor R4 review is REJECTED and this delta review still carries open Minor findings (D-1..D-6); the gate is at best "green with tracked Minors" until those close. | `status.json:24`; this report §4-§5. | Keep the gate honest per cycle: close D-1..D-6 and re-review, or annotate the deferred/tracked state, then commit. |
| D-10 | Comment | `m2-time-base-r1-review.md` header | The document-id row is malformed (`:5` ends with a stray `\|     \|`), leaving a broken table cell in the rendered report. | `docs/reviews/2026-10-10-m2-time-base-r1-review.md:5`. | Remove the trailing empty cell. |

**Severity definition** (per template): Major = non-conformity that disqualifies
the item; Minor = must be corrected/justified but does not invalidate the
requirement; Comment = observation, no derogation. No Major finding was raised:
no D-x item defeats a requirement, a safety barrier or a machine-checked count.

## 6. New documents (development-plan.md, roadmap.md rev 0.3)

Reviewed for completeness, correctness, internal consistency and
cross-references (development-plan <-> roadmap <-> README <-> rules.md):

- **Completeness.** `development-plan.md` (FRICK-PLAN-0001, rev 0.1) states the
  middleware boundary (§1), 12 traceable objectives OBJ-01..OBJ-12 (§2), the
  HAL resource/bus catalogue (§3), milestones M1..M5 with deliverables (§4), the
  hardware-free V&V summary (§5), an objective-to-milestone status table (§6)
  and a History table (§7). `roadmap.md` rev 0.3 carries the M1..M5 scope and a
  History row 0.3 describing the re-plan. Both are linked from `README.md`
  (lines 16, 301-302) and `roadmap.md:66` links the plan.
- **Consistency.** Milestone titles and statuses agree between the two documents
  and with `status.json:8-14` (M1/M2 done, M3/M4 planned, M5 in progress). The
  M3 ("interfaces/documentation only, no drivers") vs M4 ("reference
  implementations behind the M3 contracts") split is coherent across both.
- **Residual issues** (raised above, non-blocking): D-2 (`roadmap.md` revision
  header), D-5 (README's "M3" file-transfer claim vs the rev-0.3 M4 re-plan) and
  D-8 (plan not in AGENTS; plan does not link roadmap; §6 M4 omits OBJ-07).
  A new normative doc should also be added to the `docs`-job required list if it
  is to be gated like the rest of the set.

## 7. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Graded conclusion: APPROVED WITH COMMENTS.**

Rationale: the predecessor **DOC-01 Major is resolved** — the three M2 review
reports now carry terminal `closed` dispositions with re-verification evidence
and delta re-review references, the three delta reports exist, and
`verification-results.md` references them consistently (residual D-6 is a Minor
record-sync issue about NF-01 only). DOC-02, DOC-03, DOC-07, DOC-09, DOC-10,
DOC-12, DOC-13 and DOC-14 verify closed with direct evidence; DOC-08/DOC-11 are
closed with residual Comments (D-7/D-9). The machine-checked facts are all
consistent and independently reproduced: trace gate 48/25/16/7 + 41 theorems + 9
CI claims (exit 0), ctest 5/5 with 15/20/9/29/101 checks, coverage
845/1446 + 114/156 matching the artefact, README panel byte-identical to
`metrics.py`, the Lean transcript at 17/17, no broken links. No Major
non-conformity remains, so the set is no longer blocked at Major level.

However, the documentation review is **not fully closed**: DOC-04, DOC-05 and
DOC-06 remain partially closed and six new Minor findings (D-1..D-6) are open —
the review-id collisions and missing uniqueness guard (D-1), the `roadmap.md`
revision header (D-2), the residual N1-N12/N1-N16 references (D-3), the
inconsistent coverage figure (D-4), the stale README cross-references (D-5) and
the NF-01 closure record (D-6). Under norms N12/N17 these must be corrected and
recorded before commit/push; the Comments D-7..D-10 are recommendations. The
graded conclusion is therefore **APPROVED WITH COMMENTS with the Minor findings
tracked to closure**, not a clean APPROVED.

## 8. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 / N9 / N17 | independent documentation reviewer agent (profile R4) | 2026-10-10 | APPROVED WITH COMMENTS (0 Major; DOC-04/05/06 partial + D-1..D-6 Minors tracked to closure; D-7..D-10 Comments) |

## 9. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| DOC-01 | Major | M2 reports vs `verification-results.md` (N11/N12/N17) | M2 NCs recorded open/"TBD"; R2/R3 conclusions REJECTED; V&V log claimed resolved (F-1) | Closed as Major: M2 follow-up tables now terminal + delta refs; delta reports exist; V&V reconciled | `m2-*-review.md` NC tables; `2026-10-10-m2-*-delta-review.md`; `verification-results.md:23,125-133,150-155` | frick-fsw | **closed** (residual D-6) | done (this report) |
| DOC-02 | Minor | README requirements-set counts | Stale 21/15/6/20 (F-2) | Replaced with 25/16/7/41 | `README.md:259`; trace gate | frick-fsw | **closed** | done (this report) |
| DOC-03 | Minor | README panel vs `status.json` | `documentation review` gate missing (F-3) | Panel regenerated | `README.md:60` == `metrics.py` stdout; `status.json:24` | frick-fsw | **closed** | done (this report) |
| DOC-04 | Minor | Revision headers (10 docs) | Header older than History (F-4) | 9/10 aligned; `roadmap.md` header 0.2 vs History 0.3 | per-file header/history diff | frick-fsw | **open** (D-2) | required |
| DOC-05 | Minor | N17/R4 propagation | Old norm sets in current docs (F-5) | Mostly done; `AGENTS.md:12`, `driver-development.md:11`, `service-development.md:11` still old | grep N1-N1x/R4 | frick-fsw | **open** (D-3) | required |
| DOC-06 | Minor | Document ids | `FRICK-ARCH-0006` collision; ids inconsistent (F-6) | ARCH fixed; review ids **collide** (`0005`×2, `0008`×2); no uniqueness check | id scan; `.github/workflows/ci.yml:120-138` | frick-fsw | **open** (D-1) | required |
| DOC-07 | Minor | `docs/reviews/README.md` | Review log missing M2 R1/R4 (F-7) | Log completed (M2 R1 + R4 + deltas) | `docs/reviews/README.md:9,11-14` | frick-fsw | **closed** | done (this report) |
| DOC-08 | Minor | `tooling.md`/`glossary.md` History | No `## History` (F-8) | Added to both; exemption not documented | `tooling.md:116-121`; `glossary.md:37-41` | frick-fsw | **closed** (Comment D-7) | done (this report) |
| DOC-09 | Minor | `formal-verification.md` | Layout/scope omitted Time/M2 (F-9) | Time + LeanOmega + M2 scope added | `formal-verification.md:22,26,49-64` | frick-fsw | **closed** | done (this report) |
| DOC-10 | Minor | `README.md` | Duplicate `## Repository layout` (F-10) | Duplicate removed | `README.md:264,278` | frick-fsw | **closed** | done (this report) |
| DOC-11 | Comment | `status.json` | Gate green before closure (C-1) | Report now exists; state still optimistic while Minors open | `status.json:24`; this report | frick-fsw | **closed** (Comment D-9) | done (this report) |
| DOC-12 | Comment | README/AGENTS index | Missing status/glossary/CHANGELOG (C-2) | Index entries added | `README.md:303-304` | frick-fsw | **closed** | done (this report) |
| DOC-13 | Comment | `tooling.md` | T12/T13 missing (C-3) | Rows added | `tooling.md:65-66` | frick-fsw | **closed** | done (this report) |
| DOC-14 | Comment | `overview.md` | S11 stub omitted (C-4) | S11 added (ASCII + Mermaid) | `overview.md:27,58` | frick-fsw | **closed** | done (this report) |
| D-1 | Minor | Review-report ids (N8/N16) | `FRICK-REVIEW-0005` ×2 and `0008` ×2; no uniqueness check | Renumber; add id check; fix CHANGELOG wording | id lines cited in §5 | frick-fsw | **open** | required |
| D-2 | Minor | Revision headers (N16) | `roadmap.md` 0.2 vs History 0.3 | Bump to 0.3 | `roadmap.md:5,101` | frick-fsw | **open** | required |
| D-3 | Minor | N17/R4 propagation (N8) | Three current docs still N1-N12/N1-N16 | Update to N1-N17 | §5 D-3 | frick-fsw | **open** | required |
| D-4 | Minor | Coverage figures (N11) | 843/1446 recorded vs artefact 845/1446 | Align three instances | `coverage.json`; §5 D-4 | frick-fsw | **open** | required |
| D-5 | Minor | README cross-refs (N8) | Service 23/CFDP "M3" vs roadmap M4; PPS "M2/M4" | Update README | `README.md:114-115,130` | frick-fsw | **open** | required |
| D-6 | Minor | M2 R1 NC-03/NF-01 record (N11/N12) | Delta says NF-01 open, main/V&V say closed; fix present but unverified | Record NF-01 closure with evidence | §5 D-6 | frick-fsw | **open** | required |
| D-7 | Comment | Architecture/ADR History (N16) | Four notes lack History; exemption undocumented | Add tables or document exemption | §5 D-7 | frick-fsw | open | on change |
| D-8 | Comment | New-doc cross-references | Plan not in AGENTS; plan/roadmap not interlinked; §6 M4 omits OBJ-07 | Cross-link and complete §6 | §5 D-8 | frick-fsw | open | on change |
| D-9 | Comment | `status.json` gate | `documentation_review` green with Minors open | Re-state honestly per cycle | `status.json:24` | frick-fsw | open | on change |
| D-10 | Comment | R1 review header | Malformed table row | Remove stray cell | `m2-time-base-r1-review.md:5` | frick-fsw | open | on change |

---

Report document id `FRICK-REVIEW-0009` was chosen as the next free number in the
`FRICK-REVIEW-00nn` namespace precisely because `0005` and `0008` are duplicated
(D-1); once those collisions are renumbered this report's id remains unique.
This report is committed and pushed with the reviewed change (norm N11).
