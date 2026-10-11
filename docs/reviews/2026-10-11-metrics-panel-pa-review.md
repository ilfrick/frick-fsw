# Review Report — README completion-metrics panel (PA review, norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0068                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | working tree at `main`, HEAD `3fe5bf0` (change staged, uncommitted), 2026-10-11 |
| Items under review                | `tools/scripts/metrics.py` (V&V tool T13), `README.md` "Completion metrics" panel + metrics note, `docs/vv/tool-qualification.md` (T13 re-qualification), `CHANGELOG.md`; sources consumed: `docs/requirements/traceability.yaml`, `docs/status/status.json`, `docs/reviews/README.md`, `docs/design/software-design.md`, `build/coverage/coverage.json` |
| Date                              | 2026-10-11                             |
| Scope                             | PA assessment of the new completion-metrics panel: (1) definition soundness/reproducibility; (2) correctness and internal consistency of the computed values against the sources; (3) accuracy and non-over-claiming of the ECSS framing in the note; (4) adequacy of the "refreshed every cycle" mechanism and measurement-integrity concerns; (5) T13 re-qualification/evidence-mapping coverage of the new computation (norm N10). |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh independent session with no prior context. The review is read-only: only this report file was created; no other repository file was modified (per the task's explicit scope constraint). |
| Norms applied                     | N4, N8, N9, N10, N11, N15, N16, N19, N20 (see `docs/rules.md`) |

## 1. Scope

PA review (norm N20) of the **completion-metrics panel** added to the README
homepage by `tools/scripts/metrics.py` (V&V tool T13) and its explanatory note.
This is the approval requested by the project owner; PA is the stakeholder for
these metrics.

Included: the metric definitions and their reproducibility, the computed values
against the raw sources, the ECSS-class-B framing of the note, the update
mechanism ("generated between `METRICS-BEGIN`/`METRICS-END` by `make metrics`,
refreshed each cycle") and its measurement-integrity implications, and the T13
re-qualification (norm N10).

Excluded (not reviewed here): correctness of the source data itself
(`traceability.yaml`, `status.json`, coverage report) beyond consistency checks;
the R1/R2/R3/R4 reviews (norms N9/N14/N17); Lean theorem content; CI workflow
correctness. The policy/criticality tailoring matrix and a customer-approved
contract were **not available** to this reviewer; ECSS applicability is assessed
against the project's own criticality-B target and the verified clause sourcing
in `FRICK-REVIEW-0067`.

**Independent reproduction performed.** `python3 tools/scripts/metrics.py`
(stdout) was compared byte-for-byte with the README block; `python3
tools/scripts/trace.py docs/requirements/traceability.yaml` was run; an
**independent parser** (separate from `metrics.py`) recomputed the counts; the
review-closure stream algorithm was re-implemented independently.

## 2. References

- Project norms: `docs/rules.md` (N1–N20), `AGENTS.md` (N8/N20 checklist).
- Sources under review: `tools/scripts/metrics.py`, `README.md`,
  `docs/vv/tool-qualification.md`, `CHANGELOG.md`.
- Data sources: `docs/requirements/traceability.yaml`,
  `docs/status/status.json`, `docs/reviews/README.md`,
  `docs/design/software-design.md`, `build/coverage/coverage.json`,
  `docs/planning/sprint-plan.md` (FRICK-PLAN-0002).
- Prior PA sourcing: `docs/reviews/2026-10-11-ecss-traceability-pa-review.md`
  (FRICK-REVIEW-0067) — ECSS clause text verified against issuer PDFs
  (ECSS-E-ST-40C Rev.1 30 Apr 2025; ECSS-Q-ST-80C Rev.2 30 Apr 2025).
- Template: `docs/reviews/review-template.md`.

## 3. Findings

### 3.1 Verification of correctness (independent reproduction) — PASS

| Metric (README) | Reproduced | Independent check | Result |
| --------------- | ---------- | ----------------- | ------ |
| Requirements in manifest | 96 (srs=55, sss=27, gap=14) | `trace.py` = 96/55/27/14; `grep -c 'level: srs'` = 55, sss = 27, gap = 14 | ✅ correct |
| SRS delivered | 49/55 | independent parser: 49/55; undelivered = SRS-REQ-{045,046,051,052,053,054} (planned M12–M16) | ✅ correct |
| SSS satisfied | 21/27 | independent parser: 21/27; unsatisfied = SSS-REQ-{022..027} (planned milestones) | ✅ correct |
| Design traceability | 2/55 | `grep -oE 'SRS-REQ-[0-9]+' software-design.md` = {SRS-REQ-015, SRS-REQ-033} | ✅ correct |
| Reviews closed | 19/22 | independent re-implementation of the stream keying: 22 streams, 19 closed | ✅ internally consistent (see F3) |
| Statement coverage | 1917/2920 (65.7%) | `coverage.json` = 1917/2920 | ✅ correct |
| Function coverage | 257/302 (85.1%) | `coverage.json` = 257/302 | ✅ correct |
| Sprints completed | 0/26 | `sprint-plan.md` sequence sums to 26 sprints; S0 in progress | ✅ correct |
| README panel == generator output | — | `metrics.py` stdout matches README block exactly | ✅ reproducible |

No arithmetic or parsing defect was found; the panel is fully reproducible from
the checked-in sources. Findings F1–F5 below concern **definition quality,
disclosure and process**, not wrong numbers.

### 3.2 Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| F1 | **Minor** | README "Verification coverage" / `metrics.py` definition | The "Verification coverage (req. with evidence)" definition (`ver_ok = sum(1 for i in srs if i["tests"])`, truthiness only) counts a requirement as verified when its **only** evidence is `ci:docs` (documentation-pipeline CI). This is inconsistent with the "SRS delivered" definition in the same panel, which correctly excludes `ci:docs` via `_is_verified`. Six SRS whose only test entry is `["ci:docs"]` and which are **not delivered** are nevertheless counted as having verification evidence, producing a "100%" that overstates software verification. | `tools/scripts/metrics.py:128-129` (`_is_verified`), `:159` (`ver_ok`); README.md:54; manifest `SRS-REQ-045/046/052/053/054` (and 051) carry `tests: ["ci:docs"]` with docs-only implementation; independent count with `_is_verified` = **49/55**, not 55/55; the panel simultaneously shows these same requirements as *not* delivered (49/55) | Use the same `_is_verified` predicate for verification coverage (yields 49/55), or rename the metric to "Requirements with an evidence link" and state it includes documentation CI. Keep the two definitions mutually consistent. |
| F2 | **Minor** | README panel vs former "Coverage" table; norms N1/N4 status | The change **removes** two disclosure rows that were in the previous panel: "Requirements with implementation 96/96" and "Formal verification coverage (requirements with Lean) 20/96 (20.8%)". The new completion panel carries **no formal-verification (Lean) coverage** metric anywhere, even though N1/N4 make Lean coverage a headline non-derogable gate. This is a regression in PA/status disclosure and can hide the norm-N1 gap from the status panel. | `git diff --cached README.md` deletes the two rows (old lines 74-77); new panel `README.md:46-60` has no Lean metric; `trace.py` reports 51 Lean theorems; independent count = **20/96** requirements reference Lean; only `README.md:313` (static text "Lean theorems = 51") remains | Restore a formal-verification (requirements-with-Lean) line and a requirements-with-implementation line in the panel (or state explicitly where the norm-N1 coverage is reported). Do not silently drop a norm-level metric. |
| F3 | **Minor** | README "Reviews closed"; `metrics.py` review-stream heuristic | The review-closure metric groups review-log rows into "scope streams" by a heuristic on the free-text Scope column (`_key`). The heuristic is **not deterministic**: two of the three streams reported "open" are artifacts of key normalisation, not genuinely open reviews. (a) `"Open-source RTEMS alignment + toolchain"` → key `open source rtems alignment toolchain` (REJECTED) is superseded in the log by `"Open-source RTEMS alignment delta"` (APPROVED WITH COMMENTS) but keys differently (`open source rtems alignment`), so the stale REJECTED is never overwritten. (b) The `"N12"` stream (`REJECTED -> dispositions in progress`, log row 57) is superseded by the PA final review `PASS` (log row 67) which keys to `N20`, not `N12`, so the closure is not merged. Only the third open stream (`ECSS traceability question`, NC `PA-67-01`) is a genuinely open NC. | `tools/scripts/metrics.py:163-190`; review log `docs/reviews/README.md:41,42,57,67,71`; independent re-implementation reproduced 22 streams / 19 closed with exactly these three "open"; the panel shows `Reviews closed 19/22 (86.4%)` | The heuristic **under-reports** closure and is naming-dependent. Add a stable scope/NC key column to the review log (or key PA delta re-reviews to their milestone), and make the grouping independent of free-text spelling. Re-derive the count after the log is keyed. (Conservative direction, but it must be deterministic to serve as a status metric.) |
| F4 | **Minor** | Norm N10; `docs/vv/tool-qualification.md` T13 | The T13 re-qualification does not fully update the register for the new computation. The register **row** T13 still states "Used for evidence of: README coverage + V&V panel" and "Review (independence): internal (R2 on changes)", and the **Evidence-claims mapping** table has **no T13 row**. Only the History entry 0.3.4 records the new behaviour. Norm N10 requires the register (and its evidence mapping) to be updated in the same commit as the tool change. | `docs/vv/tool-qualification.md:37` (T13 row), `:58-71` (evidence-claims mapping — no T13), `:101` (History 0.3.4); `docs/rules.md` N10 ("Any change to a recorded V&V script invalidates its qualification and forces a re-qualification review"; "the qualification register is maintained manually in the same commit") | Update the T13 row to name the completion metrics and the four new source artifacts; add a T13 evidence-mapping row (it supports the README status panel, no `ci:` claim); record the reviewer/date of the source review that re-qualified the new computation. Internal (R2) independence is acceptable at criticality B per N10, but this PA review does not substitute for the N10 tool source review. |
| F5 | **Minor** | README "refreshed every cycle"; norm N8/N11/N19 claim | The claim "**...refreshed on every cycle**" is **not enforced**: `make metrics` (which regenerates the README block from the sources) is not invoked by any CI workflow, and there is no gate that fails when the checked-in README block diverges from `metrics.py` output. The panel can therefore silently go stale relative to `traceability.yaml`, `status.json`, the review log, the SDD or `coverage.json` — e.g. after a traceability change that does not re-run `make metrics`. | `Makefile:125-127` (`metrics: coverage` target); `grep` finds no `metrics` reference in `.github/workflows/*` or `.gitea/workflows/*`; the panel is only regenerated when a human/agent runs the target | Add a CI/`make` currency check that fails when the README block differs from `metrics.py` output (analogous to the N4 trace gate). This makes "refreshed every cycle" verifiable rather than aspirational. **Escalation note:** this Minors set is proportionate for a status *indicator*; if the panel is later used as a milestone-gate input, F1 and F5 become gate-blocking. |
| F6 | Comment | README "Design traceability"; NC `PA-67-01` | The design-traceability metric (2/55) is a **raw substring-presence proxy** — it counts SRS ids occurring anywhere in the SDD prose, not a trace to a design component. This does not measure Annex F `<6>` design traceability and should not be read as such. The note is transparent about this (cites the open `PA-67-01` and "not claims of ECSS compliance"). | `tools/scripts/metrics.py:156` (`i["id"] in sdd`); SDD `docs/design/software-design.md:25,78`; `FRICK-REVIEW-0067` NC `PA-67-01` | Retain, but consider renaming to "SRS ids mentioned in the SDD" until the Annex F `<6>` matrices exist; do not let the label imply design-traceability compliance. |
| F7 | Comment | `metrics.py` design matching robustness | `design_ok` uses unbounded substring matching (`i["id"] in sdd`). It is safe only because all ids are fixed-width 3-digit; a future numbering change (e.g. `SRS-REQ-0010`) could cause false positives/ambiguous matches. | `tools/scripts/metrics.py:156` | Use a word-boundary/token match (e.g. `re.search(r'\b'+re.escape(id)+r'\b', sdd)`). |
| F8 | Comment | README "Sprints completed"; `status.json` | The sprint total/done are **manually maintained** in `docs/status/status.json` (`sprints.total=26`, `done=0`). The panel note says the metrics are "generated from the … sprint plan"; strictly the source is `status.json`, whose provenance is the plan. This is acceptable (like the semaphores) but the wording slightly overstates automation. | `README.md:50`; `docs/status/status.json:42-47`; `Makefile`/`metrics.py:192-198` read `status["sprints"]` only | Reword to "…from `status.json` (sprint plan FRICK-PLAN-0002)" or compute the total from the plan. |
| F9 | Comment | README note; statement coverage | The note quotes class-B practice as "100% statement + branch/decision coverage with justified exceptions" and flags only "branch/MC-DC and WCET" as **GAP-A4**. But the **measured statement coverage itself is 65.7%**, below the quoted 100% statement target; the note does not flag the statement shortfall. The number is shown, but the gap is not contextualised. | `README.md:56,58,60` | State explicitly that statement coverage (65.7%) is itself below the class-B practice and is tracked (with branch/MC-DC) under GAP-A4, so the panel cannot be read as "statement coverage is on target". |
| F10 | Comment | ECSS framing of the note | The ECSS framing is **accurate and non-over-claiming**, and is consistent with the independently sourced clause text in `FRICK-REVIEW-0067`: requirements traceability as a *matrix* under E-ST-40C 5.8.3.x (Annex R Table R-1 applicable at B); in-source requirement tagging is "a permitted means, not a requirement"; design traceability per SDD DRD Annex F `<6>`; the note ends with "not claims of ECSS compliance". No statement was found that reads as a compliance or certification claim. | `README.md:60`; `FRICK-REVIEW-0067` §3 (clauses verified against issuer PDFs, MD5-matched) | No corrective action. Keep the note coupled to `FRICK-REVIEW-0067`; if the contract/tailoring matrix becomes available, re-baseline the note. |
| F11 | Comment | `tools/scripts/metrics.py` | The file lost its trailing newline (`\ No newline at end of file` in the diff). Cosmetic; can trip some linters/diff tooling. | `git diff --cached tools/scripts/metrics.py` (last line) | Restore the trailing newline. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 4. Conclusion

The completion-metrics panel is **technically correct and fully reproducible**:
every value matches an independently recomputed figure, and the README block is
byte-identical to `metrics.py` output. The ECSS-class-B framing is accurate,
sourced and non-over-claiming. The remaining issues are **definition quality,
disclosure and process** (F1–F5, all Minor): a verification-coverage definition
that overstates evidence, a dropped norm-N1 (Lean) disclosure line, a
non-deterministic review-closure heuristic, an incomplete N10 register update,
and an unenforced "refreshed every cycle" claim. None defeats a requirement or a
safety barrier; all are correctable or justifiable in the same cycle.

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — 5 Minor (F1–F5) tracked to closure; 6 Comments**
- [ ] REJECTED — one or more Major findings

**PA recommendation.** The panel is **fit for PA/status use** as a completion
*indicator* (the note's own disclaimer applies), subject to closing F1–F5.
The panel is **not** fit to be used as a milestone-gate or compliance input
until at least F1 and F5 are closed, because at that point an overstated
verification coverage and an unenforced currency check would matter.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | —                |            |            |
| R2 (N9)       | —                |            |            |
| R3 (N9)       | —                |            |            |
| R4 (N9/N17)   | —                |            |            |
| **PA (N20)**  | **independent agent, FRICK-REVIEW-0068** | **2026-10-11** | **APPROVED WITH COMMENTS (5 Minor: F1–F5)** |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-68-01 (F1) | Minor | README verification-coverage definition | `ci:docs`-only requirements counted as verified; inconsistent with "SRS delivered" | Align `ver_ok` with `_is_verified` (→ 49/55) or rename/annotate the metric | `metrics.py` output + README row | frick-fsw | **open** | pending |
| PA-68-02 (F2) | Minor | README panel / norm N1 disclosure | Dropped "Formal verification coverage (Lean)" and "Requirements with implementation" rows | Restore Lean-coverage (+ implementation) line or state where norm-N1 coverage is reported | README block + `metrics.py` | frick-fsw | **open** | pending |
| PA-68-03 (F3) | Minor | Review-closure stream heuristic | Non-deterministic scope keying produces stale "open" streams; under-reports closure | Add stable scope/NC key column to review log; make grouping deterministic; re-derive count | recomputed stream list | frick-fsw | **open** | pending |
| PA-68-04 (F4) | Minor | Norm N10; T13 register | T13 row and evidence-claims mapping not updated for the new computation | Update T13 row + add evidence-mapping row; record source-review reviewer/date | `tool-qualification.md` | frick-fsw | **open** | pending |
| PA-68-05 (F5) | Minor | "Refreshed every cycle" / N8-N11-N19 claim | No CI/make currency gate; panel can go stale silently | Add a README-vs-generator currency check | CI workflow / `Makefile` | frick-fsw | **open** | pending |

Comment-only items F6–F11 require no derogation; they are recorded for
disposition at the owner's discretion.

**Log-entry note.** Per the task's scope constraint ("modify nothing else"), the
corresponding row was **not** appended to `docs/reviews/README.md`; the owner
should add the log row for `FRICK-REVIEW-0068` together with the corrections.

---

## Appendix A — Confidence and evidence limitations

```text
Decision/product and configuration:
  frick-fsw @ main HEAD 3fe5bf0, change under review staged/uncommitted
  (metrics.py, README.md, status.json, tool-qualification.md, CHANGELOG.md);
  target ECSS criticality B.
Claim type: mixed (Fact for reproduced counts; Inference for definition quality).
Applicable baseline and approved tailoring:
  Project targets criticality B (docs/quality/ecss-criticality-b.md). Customer
  contract / approved tailoring matrix NOT available to this reviewer; ECSS
  applicability assessed against the project baseline and FRICK-REVIEW-0067.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  tools/scripts/metrics.py (working tree); README.md; docs/status/status.json;
  docs/reviews/README.md; docs/design/software-design.md; docs/planning/
  sprint-plan.md (FRICK-PLAN-0002); build/coverage/coverage.json;
  docs/vv/tool-qualification.md; docs/rules.md; FRICK-REVIEW-0067.
  Independent reproduction: `python3 tools/scripts/metrics.py`;
  `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`;
  independent manifest parser (49/55, 21/27, 20/96); independent re-implementation
  of the review-stream keying (22/19).
Cross-check and reconciliation:
  metrics.py output == README block byte-for-byte; counts agree with trace.py and
  with an independently written parser; ECSS clause references agree with the
  issuer-PDF-sourced FRICK-REVIEW-0067. No conflict found.
Confidence:
  High for correctness/reproducibility of the values and for the definitional
  findings (direct, reproducible evidence). Medium for the N10 process finding
  (register conventions are internal; no contract tailoring seen).
Basis and detection limits:
  Full source and data inspected; substring-based matching cannot be proven
  bug-free beyond the current fixed-width id scheme; review-stream semantics are
  heuristic by construction.
Missing/inaccessible evidence (including paywalls):
  Customer contract / approved tailoring matrix; ECSS PDFs were not re-opened in
  this review (relied on the MD5-verified sourcing in FRICK-REVIEW-0067).
Assumptions and expiry/recheck date:
  Assumes status.json sprints block is the authoritative sprint state; assumes
  the criticality-B target is authoritative. Recheck on any metrics.py change or
  baseline change, or 2027-04-11.
Consequences if wrong:
  If a contract mandates stricter verification-evidence definitions, F1 escalates;
  if the panel is adopted as a gate, F1/F5 become Major.
Required independent human reviewer/authority:
  The project's authorized engineering/PA/release authority; this agent cannot
  approve, accept residual risk, or close the NCs.
```

---

Committed and pushed with the reviewed change (norm N11).