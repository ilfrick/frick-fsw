# Documentation Set Review (profile R4, norm N17)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0005                      |
| Reviewer session                  | independent agent (fresh context, norm N9) |
| Reviewer profile                  | R4 - Senior SW documentation reviewer  |
| Reference revision                | working tree on base commit `a69732f` (branch `main`, uncommitted M2/doc-program delta) |
| Items under review                | Full documentation set: `README.md`, `AGENTS.md`, `CONTRIBUTING.md`, `CHANGELOG.md`, `docs/**` (requirements SSS/SRS/traceability.yaml+md, architecture, ADRs, `interfaces/ICD.md`, quality, `vv/`, `engineering/`, `reviews/`, `roadmap.md`, `status/status.json`, `glossary.md`, `tooling.md`, `verification/`), `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml` |
| Date                              | 2026-10-10                             |
| Scope                             | Completeness and correctness of the documentation set (norm N8/N17): every required document current; requirement/theorem/CI/coverage counts; revision numbers and history tables; milestone and status records; internal consistency; coherence and cross-referencing of norm N17, profile R4, the `AGENTS.md` checklist and the `status.json` `documentation_review` gate; agreement with code/manifest/CI/status; no stale references; no "TBD" in closed artifacts; English (N2); SPDX/licence (N13); links resolve |
| Independence declaration          | Reviewer took no part in authoring, executing or reviewing this work; fresh context; every claim below is based solely on the repository evidence at the stated revision and on `docs/rules.md` (N1-N17) |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Independent documentation review (norm N17) of the whole documentation set of
frick-fsw at the reference revision (working tree on `a69732f`, i.e. the
uncommitted milestone-M2 time-base delta plus the documentation-program delta
that introduces norm N17, profile R4 and the `documentation_review` gate).

In scope:

- **Normative and meta documents**: `docs/rules.md` (N1-N17 and the
  enforcement summary), `AGENTS.md`, `README.md`, `CONTRIBUTING.md`,
  `CHANGELOG.md`.
- **Requirements and traceability**: `docs/requirements/{SSS,SRS,traceability}.{md,yaml}`.
- **Design**: `docs/architecture/**`, `docs/adr/**`, `docs/interfaces/ICD.md`,
  `docs/glossary.md`, `docs/tooling.md`.
- **Quality/planning/status**: `docs/quality/**`, `docs/roadmap.md`,
  `docs/status/status.json`.
- **V&V**: `docs/vv/**`, `docs/verification/formal-verification.md`.
- **Engineering/reviews**: `docs/engineering/**`, `docs/reviews/**`.
- **CI**: workflow header comments and `# evidence:` markers in both forges.

Verification methods used (read-only): inspection, grep-based cross-checks,
`git status`/`git diff --stat`/`git log`, `python3 tools/scripts/trace.py
docs/requirements/traceability.yaml` (exit 0), `python3
tools/scripts/metrics.py` (panel regeneration to stdout only), reading
`build/coverage/coverage.json`, execution of the pre-built unit-test binaries to
confirm the documented check counts, and a relative-link existence scan over
all Markdown.

Excluded from this review: the correctness of the flight code itself (reviewed
by R1/R2), the substantive technical content of the M2 mitigations (reviewed by
R1/R2/R3), and any file not present in the repository at the reference revision.

Assumptions: the reference revision is the working tree as-is (the M2 delta and
the documentation-program delta are pending a single commit/push, norms N7/N15);
counts and revision numbers are checked against the machine-readable sources
(`traceability.yaml`, `build/coverage/coverage.json`, `status.json`) and the
gates that derive them.

## 2. References

- Project norms: `docs/rules.md` (N1-N17; interpretation notes).
- Traceability manifest / matrices: `docs/requirements/traceability.yaml`,
  `docs/requirements/traceability.md`.
- Independent review process and profiles: `docs/engineering/independent-review.md`.
- Status record: `docs/status/status.json`; panel generator `tools/scripts/metrics.py`;
  coverage artefact `build/coverage/coverage.json`.
- CI: `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `.github/workflows/nightly.yml`.
- Review template: `docs/reviews/review-template.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2,
  CCSDS 301.0-B-4.

## 3. Verified without finding

The following were verified and found compliant and consistent at the reference
revision:

1. **Traceability gate converges.** `python3 tools/scripts/trace.py
   docs/requirements/traceability.yaml` exits 0: **48 requirements (srs=25,
   sss=16, gap=7), 41 Lean theorems checked, 9 CI evidence claims**; every
   SRS has exactly one SSS parent and every SSS at least one child
   (`traceability.yaml`; `trace.py` output). SRS ids are complete
   (`FRICK-SRS-REQ-000`, `SRS-REQ-001..024`) and SSS ids complete
   (`SSS-REQ-001..016`); GAP ids complete (`GAP-A1..A7`).
2. **Counts in `traceability.md` are correct and self-labelling.**
   `docs/requirements/traceability.md:18-20` states SRS=25, SSS=16, GAP=7,
   Lean=41 and "CI evidence claims = 9 unique (8 referenced by the manifest;
   `ci-lean` reported but not claimed)" — matching the gate output, the manifest
   and both workflows (`# evidence: ci-lean … not claimed in manifest but
   reported`).
3. **Coverage figures match the artefact.** `README.md:68` and
   `docs/vv/verification-results.md:107` state **lines 843/1446 (58.3%),
   functions 113/156**, matching `build/coverage/coverage.json`
   (`lines_total=1446, lines_covered=843, funcs_total=156, funcs_covered=113`).
4. **ctest counts and 5/5 suites match the documentation.** Executing the
   pre-built suites reproduces the documented per-suite check counts exactly:
   `test_ccsds` 15, `test_sb` 20, `test_tbl` 9, `test_pus` 29, `test_time` 101
   (5/5 suites), matching `docs/vv/verification-results.md:23,99-103`.
5. **Lean module count is re-derivable.** `verification/lean/lake-build.log`
   records `[17/17] Built frick_verify_checks` / "Build completed
   successfully", supporting the documented `lake build 17/17` claim
   (`verification-results.md:22-23,88-90`); `Frick/Time.lean` contains 11
   theorems matching the CHANGELOG M2 note.
6. **The N17 machinery is coherent and cross-referenced.** Norm N17
   (`docs/rules.md:253-279,301`), profile R4
   (`docs/engineering/independent-review.md:125-168,218`), AGENTS checklist
   item 7 (`AGENTS.md:96-100`), the `documentation_review` gate
   (`docs/status/status.json:24`) and the V&V record (`verification-results.md:135-141,153`)
   agree on scope, trigger and closure rule, and the enforcement summary lists
   N17.
7. **CI claims and workflow markers are consistent.** Both workflows declare the
   same 9 evidence markers; `trace.py` accepts them; `tool-qualification.md`
   T11 maps the six workflow-supported claims correctly; the only asymmetry is
   the documented `ci-lean` marker (intended, N1).
8. **No broken relative links.** A recursive scan of all Markdown links in
   `docs/**`, `README.md`, `AGENTS.md`, `CONTRIBUTING.md`, `CHANGELOG.md`
   resolves every non-external target.
9. **Language (N2) and licence (N13).** All reviewed content is in English; CI
   job `docs` performs the AGPL-3.0 `LICENSE` check and the
   "no `Apache-2.0` SPDX header" scan (`.github/workflows/ci.yml:140-159`).
10. **Milestone statuses agree across records** (`roadmap.md:17-91`,
    `status.json:8-14`, README PUS section): M1 done, M2 done, M3/M4 planned,
    M5 in progress.
11. **Class-A gap tracking is consistent**: `class-a-gaps.md` lists GAP-A1..A7,
    the manifest and `traceability.md:103-109` list the same, and
    `ecss-criticality-b.md:10,75` now references GAP-A1..A7 (the former R3 NC-5
    stale reference is corrected).
12. **ICD/architecture CUC description is internally consistent with the
    implementation and SRS-REQ-024** (`ICD.md:75-119`,
    `time-and-timecode.md:147-210`): the explicit 7-octet P+T form and the
    6-octet PUS on-wire T-field are distinguished and labelled with the R3
    NC-1 disposition.
13. **M2 code/doc fixes corresponding to the R1/R2 findings are present in the
    tree**: `pps.c` state handling (`g_sw_running=false` on create failure),
    `time.c` `_Static_assert`s for the CUC macros, a PAL mutex around the time
    state, and a file-scope `FF_TEST_MAIN` in `test_time.c` (so the suite
    really executes and reports 101 checks).

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F-1 | **Major** | `docs/reviews/2026-10-10-m2-{time-base-r1,vv-r2,sss-r3}-review.md` (norm N9/N11/N12) vs `docs/vv/verification-results.md` | **M2 review closure is contradictory and unrecorded.** The three M2 independent review reports still carry every non-conformance as **open** with **"TBD"** dispositions and no re-verification evidence, and the R2/R3 conclusions are still **REJECTED**; yet `verification-results.md` asserts "All M2 NCs (R1/R2/R3) resolved and delta-re-reviewed", lists them as "Resolved", and states "Open non-conformances: none". This defeats the norm-N12 closure criterion (disposition + re-verification evidence + re-approval recorded in the report) and violates the R4 rule "no TBD dispositions in closed artifacts"; the delivered record misrepresents the review/NC status (N11 record integrity). | R1 report `§6` rows NC-01..NC-08 "open"/"TBD" (`2026-10-10-m2-time-base-r1-review.md:132-141`); R2 report `§7` rows M2-01..M2-06 "open" with TBD dispositions and conclusion "REJECTED" (`2026-10-10-m2-vv-r2-review.md:84,103-110`); R3 report `§6` rows NC-1..NC-5 "open"/"TBD", conclusion "REJECTED" (`2026-10-10-m2-sss-r3-review.md:119,143-149`); `verification-results.md:23` ("All M2 NCs (R1/R2/R3) resolved and delta-re-reviewed"), `:125-133` (NC closure table "Resolved" / "Open non-conformances: none"), `:150-152` ("NCs closed after fix"). | Update each M2 report's follow-up table to the true terminal state (disposition, re-verification evidence — ctest check counts, `trace.py` output, coverage artefact, transcript — owner and `closed`), record the delta re-review outcome per profile, and make `verification-results.md` reference that evidence. Re-run the full cycle (N12/N15) and re-verify before committing. |
| F-2 | Minor | `README.md` "Requirements set (current)" (norm N8/N17) | The human entry point states **stale requirement/theorem counts**: "SRS = 21 · SSS = 15 · GAP (to class A) = 6 · Lean theorems = 20", contradicting the machine-generated status panel **in the same file** (srs=25, sss=16, gap=7) and the traceability gate (41 Lean theorems). | `README.md:251` vs `README.md:64-68`; `trace.py` output (48 reqs, srs=25, sss=16, gap=7, 41 theorems); `traceability.md:18-20`. | Regenerate/replace the paragraph with 25/16/7/41 (or point to the metrics panel and manifest as the single source of truth). |
| F-3 | Minor | `README.md` status panel vs `docs/status/status.json` (norm N17/N8) | The README semaphore panel **omits the `documentation review` gate** that exists in `status.json` and is emitted by `metrics.py`. Norm N17 explicitly states the gate "is reported in the README metrics panel (`documentation_review` semaphore in `docs/status/status.json`)". The panel therefore does not match its own generator/status source. | `README.md:43-54` (ten gates, none named "documentation review"); `status.json:15-27` (eleven gates incl. `"documentation_review": "green"`); `python3 tools/scripts/metrics.py` output (emits the `documentation review` row). | Re-run `make metrics` (`metrics.py --readme`) so the panel is regenerated from `status.json`, and keep the two in sync each cycle (the injectable panel is meant to be machine-owned). |
| F-4 | Minor | Revision headers vs History tables (norm N16/N8) | **Eight current documents have a revision header older than their own latest `History` row**, so the front-matter revision is wrong/ambiguous: SRS (`0.2` vs `0.3`), SSS (`0.2` vs `0.3`), `traceability.md` (`0.1` vs `0.2`), `roadmap.md` (`0.1` vs `0.2`), `driver-development.md` (`0.1` vs `0.2`), `licensing.md` (`0.1` vs `0.2`), `tool-qualification.md` (`0.1` vs `0.1.4`), `verification-results.md` (`0.1` vs `0.3`). | `SRS.md:7` vs `:145`; `SSS.md:7` vs `:95`; `traceability.md:7` vs `:124`; `roadmap.md:5` vs `:98`; `driver-development.md:5` vs `:195`; `licensing.md:7` vs `:79`; `tool-qualification.md:7` vs `:72`; `verification-results.md:7` vs `:162`. | Align each header `Revision` with the max history row, or make the header explicit that it denotes the initial basis; add a CI/checklist check for header-vs-history parity (norm N16). |
| F-5 | Minor | Norm N17 / profile R4 not propagated to current documents (norm N8/N17/N16) | The new norm/profile/gate is coherent where introduced but **absent or contradicted in several current documents**: `README.md` still says "N1-N16" and never mentions N17/documentation review; `AGENTS.md §1` heading says "norm N1-N12"; `CONTRIBUTING.md` says "N1-N16", lists reviews as "N9/N10/N14" and its profile table gives only R1/R2/R3; `docs/reviews/review-template.md` header profiles are "R1 (SW dev) / R2 (SW V&V) / R3 (Sys Eng)" and norms "N1-N12" with references "N1-N11"; `docs/vv/verification-plan.md` §7 names only "R1 or R2" and its per-change-cycle/GATES text omits N17; `CHANGELOG.md` has no entry for the N17/R4/documentation-review change. | `README.md:13,270` (N1-N16); `AGENTS.md:12`; `CONTRIBUTING.md:4,27,50-54`; `review-template.md:7,13,22`; `verification-plan.md:10,63,161-166`; `CHANGELOG.md:8-55` (no N17/R4 entry); `rules.md:253-279` (N17 requirement). | Update every current document that enumerates norms/profiles to N1-N17 and add R4 (review template profile/norm rows, CONTRIBUTING review list, AGENTS heading, README norm reference and V&V plan cycle), and add a CHANGELOG `[Unreleased]` entry recording norm N17, profile R4 and the `documentation_review` gate (norm N16). |
| F-6 | Minor | Document identifiers (norm N8/N16) | **Document-id collision and inconsistent review ids.** `FRICK-ARCH-0006` is used by both `docs/architecture/time-and-timecode.md` and `docs/architecture/mapping-cfs.md`; the review ids are inconsistent and near-colliding (`FRICK-REVIEW-004` for the R3 M2 review vs `FRICK-REVIEW-0004` for the R2 M2 review; the R1 M2 review uses the non-numeric `FRICK-REVIEW-M2-TIME-R1-2026-10-10`), breaking the single-namespace id convention used elsewhere. | `time-and-timecode.md:3`; `mapping-cfs.md:3`; `m2-sss-r3-review.md:5`; `m2-vv-r2-review.md:5`; `m2-time-base-r1-review.md:5`; other reviews use `FRICK-REVIEW-0001..0004`. | Assign `mapping-cfs.md` a unique `FRICK-ARCH-00xx` id and normalise the review ids to the `FRICK-REVIEW-00nn` scheme; add a uniqueness check to the `docs` CI job. |
| F-7 | Minor | `docs/reviews/README.md` review log (norm N11/N17) | The review log is **incomplete**: it omits the M2 R1 report row (`2026-10-10-m2-time-base-r1-review.md`) even though the report exists and is referenced elsewhere, and the profile column/log has no R4 documentation-review entry; the log therefore does not reflect the actual set of reviews under `docs/reviews/`. | `docs/reviews/README.md:3-9` (five rows); directory contains six review reports; `verification-results.md:150` references the M2 R1 review and `:153` pre-references the R4 report. | Add the M2 R1 row and the R4 documentation-review row, extend the profile column, and treat the log as part of the same commit (norm N11/N17). |
| F-8 | Minor | `docs/tooling.md`, `docs/glossary.md` (norm N16) | Current engineering/reference documents have **no `## History` table**, although norm N16 requires "a `## History` table in each engineering document". (`docs/tooling.md` and `docs/glossary.md` are in the reviewed delta; the architecture notes `applications.md`, `platform.md`, `software-bus.md`, `mapping-cfs.md` and the ADRs likewise lack one.) | `docs/tooling.md` (no History section); `docs/glossary.md` (none); `docs/architecture/{applications,platform,software-bus,mapping-cfs}.md` (none); `docs/rules.md:243-251` (N16 requirement). | Add `## History` tables (rev/date/author/reason) to `tooling.md` and `glossary.md` at minimum, and decide/document the exemption for ADRs and short architecture notes; enforce in the `docs` gate. |
| F-9 | Minor | `docs/verification/formal-verification.md` (norm N8) | The formal-verification document is **stale after M2**: its project-layout table omits the `Frick/Time.lean` model (and `Frick/LeanOmega.lean`) and its scope section does not mention the M2 time/theorem set, although the M2 delta adds `Frick.Time` and ten requirement wrappers linked in the manifest. | `formal-verification.md:17-26` (lists only Common, Ccsds, SoftwareBus, Pus, Requirements, Main); `formal-verification.md:47-52` (scope omits time); actual modules `verification/lean/Frick/{Time,LeanOmega}.lean`; CHANGELOG M2 note (`CHANGELOG.md:23-24`) and `traceability.yaml` SRS-REQ-021..024. | Add `Frick/Time.lean` (and `Frick/LeanOmega.lean`) to the layout table and extend the scope paragraph (time-source disjointness, tone discipline, leap-at-boundary, CUC round-trip/fine bound/coarse decomposition). |
| F-10 | Minor | `README.md` structure (norm N8) | The README contains a **duplicated, empty `## Repository layout` heading**: the real section (heading + table) is followed later by a second empty `## Repository layout` immediately before `## Contributing`, leaving an empty section in the rendered document. | `README.md:254-266` (real section) and `README.md:302-304` (empty duplicate heading). | Delete the empty duplicate heading. |
| C-1 | Comment | `docs/status/status.json` (norm N17) | The `documentation_review` gate is set to `green` in the delta under review, but the corresponding R4 report did not exist before this review and (as delivered) does not satisfy the N17 closure rule (F-1/N12). The status record therefore anticipates a closure that is not yet evidenced. | `status.json:24`; absence of the report before this run; this report's conclusion (REJECTED). | Only set `documentation_review` to `green` once the R4 report exists and its NCs are closed (or set it to the true state); keep status honest per cycle (norm N8/N11). |
| C-2 | Comment | `README.md` documentation index (norm N8) | The documentation index omits several delivered artifacts a fresh user/agent would need: `docs/status/status.json`, `docs/glossary.md` and `CHANGELOG.md` (the repository map in `AGENTS.md` also lists `docs/status` only generically). | `README.md:268-291`; `AGENTS.md:36`. | Add index entries for the status record, glossary and CHANGELOG. |
| C-3 | Comment | `docs/tooling.md` tool inventory (norm N8/N10) | The "Verification & validation tools" table lists T1-T10 but omits the qualified tools **T12 `coverage.py`** and **T13 `metrics.py`**, although `docs/vv/tool-qualification.md:36-37` registers them and both are used to produce evidence recorded in the docs. | `tooling.md:51-64`; `tool-qualification.md:36-37,71`. | Add the coverage collector and metrics generator rows (with their qualification refs) to the tooling inventory. |
| C-4 | Comment | `docs/architecture/overview.md` vs `README.md`/`roadmap.md` (norm N8) | The overview's PUS service lists (`S1 S3 S5 S6 S8 S9 S17 S20`) omit the implemented **service 11 stub**, which the README PUS-C table and the M1 roadmap scope both list. | `overview.md:27,58`; `README.md:102`; `roadmap.md:20`. | Add S11 (stub) to the architecture service lists, or note the intentional omission. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings

Rationale: the documentation set is largely complete and the machine-checked
counts (48 requirements, srs=25/sss=16/gap=7, 41 Lean theorems, 9 CI claims,
coverage 843/1446 with functions 113/156, ctest 5/5 with the documented per-suite
checks) and the newly added N17/R4/status-gate machinery are internally
coherent and cross-referenced. However, **F-1 is a Major non-conformity**: the
three independent M2 review reports record all their non-conformances as open
with "TBD" dispositions and no re-verification evidence (R2/R3 still REJECTED),
while the V&V results log and review-evidence table declare them resolved and
state that no non-conformances are open. This contradicts norm N12 and the R4
requirement that closed artifacts carry no "TBD" dispositions, and it
misrepresents the review outcome in the delivered record (norms N11/N12/N17).
Under norm N17 a REJECTED documentation review blocks commit/push until all
Major findings are resolved and the deltas re-reviewed (norm N12). The Minor
findings (stale README counts, README panel vs `status.json`, stale revision
headers, unpropagated N17/R4 references and missing CHANGELOG entry,
document-id collision, incomplete review log, missing History tables, stale
formal-verification doc, duplicate README heading) must be corrected in the same
cycle; the Comments are recommendations.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 / N9 / N17 | independent documentation reviewer agent (profile R4) | 2026-10-10 | REJECTED (1 Major; Minors + Comments tracked) |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| DOC-01 | Major | M2 review reports vs `verification-results.md` (N11/N12/N17) | M2 NCs recorded open/"TBD"; R2/R3 conclusions REJECTED; V&V log claims resolved/none open (F-1) | Update the three M2 follow-up tables with terminal dispositions, evidence and re-approval; reconcile `verification-results.md` | Updated M2 report NC tables (closed) + linked ctest/trace/coverage/transcript evidence; re-run full cycle | frick-fsw (M2/doc owner) | open | required (R4, independent) |
| DOC-02 | Minor | `README.md` requirements-set counts | Stale SRS=21/SSS=15/GAP=6/Lean=20 contradicts panel and gate (F-2) | Replace with 25/16/7/41 or defer to the panel/manifest | README diff; `trace.py` + `metrics.py` output | frick-fsw | open | required |
| DOC-03 | Minor | `README.md` panel vs `status.json` | `documentation review` gate missing from panel (F-3) | Re-run `make metrics`/`metrics.py --readme` | Diff between README panel and `metrics.py` stdout | frick-fsw | open | required |
| DOC-04 | Minor | Revision headers (8 docs) | Header revision older than latest History row (F-4) | Align headers with History (or clarify semantics) | Per-file header/history diff | frick-fsw | open | required |
| DOC-05 | Minor | N17/R4 propagation | README/AGENTS/CONTRIBUTING/review-template/verification-plan/CHANGELOG not updated for N17/R4 (F-5) | Update norm/profile lists and add CHANGELOG entry | Grep for N1-N17/R4 across current docs | frick-fsw | open | required |
| DOC-06 | Minor | Document ids | `FRICK-ARCH-0006` collision; review ids inconsistent (F-6) | Reassign unique ids; normalise review ids; add uniqueness check | Unique-id scan | frick-fsw | open | required |
| DOC-07 | Minor | `docs/reviews/README.md` | Review log missing M2 R1 and R4 rows (F-7) | Complete the log | Log vs directory listing | frick-fsw | open | required |
| DOC-08 | Minor | `tooling.md`/`glossary.md` History | No `## History` tables (F-8) | Add History tables (and document ADR/architecture exemption) | Presence check | frick-fsw | open | required |
| DOC-09 | Minor | `formal-verification.md` | Layout/scope omit `Frick/Time.lean` and M2 theorems (F-9) | Add Time/LeanOmega and M2 scope | Doc vs `verification/lean/**` and manifest | frick-fsw | open | required |
| DOC-10 | Minor | `README.md` | Duplicate empty `## Repository layout` heading (F-10) | Delete the duplicate | README diff | frick-fsw | open | required |
| DOC-11 | Comment | `status.json` | `documentation_review` green before closure (C-1) | Set the true gate state until closure | status.json + report existence | frick-fsw | open | on change |
| DOC-12 | Comment | `README.md`/`AGENTS.md` index | Missing status/glossary/CHANGELOG index entries (C-2) | Add index entries | README/AGENTS diff | frick-fsw | open | on change |
| DOC-13 | Comment | `tooling.md` | T12/T13 tools not listed (C-3) | Add coverage/metrics rows | tooling vs tool-qualification | frick-fsw | open | on change |
| DOC-14 | Comment | `overview.md` | S11 stub omitted from service lists (C-4) | Add S11 or note the omission | overview vs README/roadmap | frick-fsw | open | on change |

---

Committed and pushed with the reviewed change (norm N11).
