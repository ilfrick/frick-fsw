# Review Report — README metrics panel, delta re-review (PA review, norm N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0069                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | working tree at `main`, HEAD `3fe5bf0` (change staged/uncommitted), 2026-10-11 |
| Items under review                | `README.md` metrics panel (consolidated), `tools/scripts/metrics.py` (V&V tool T13), `docs/vv/tool-qualification.md` (T13), `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `CHANGELOG.md`, `docs/status/status.json`; delta against FRICK-REVIEW-0068 |
| Date                              | 2026-10-11                             |
| Scope                             | Delta re-review (norm N12) of the five Minor findings (F1–F5) raised in FRICK-REVIEW-0068 and of the owner-requested consolidation (single metrics panel in README). |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh independent session with no prior context. Read-only review: only this report file was created; no other repository file was modified. |
| Norms applied                     | N1, N4, N8, N9, N10, N11, N12, N15, N16, N20 (see `docs/rules.md`) |

## 1. Scope

PA delta re-review (norms N12/N20) of the corrections applied to the README
metrics panel after FRICK-REVIEW-0068 (APPROVED WITH COMMENTS, Minors F1–F5).
Two things are verified:

1. **Resolution of F1–F5** (verification, Lean coverage, review-stream keying,
   T13 register, CI currency gate) against the new working-tree state.
2. **The owner-requested consolidation:** the README must carry **exactly one**
   metrics panel. The separate `## Requirements set (current)` prose panel must
   be gone, no duplicate/overlapping metrics panel may remain, and the single
   surviving panel must be comprehensive (completion + coverage + semaphores +
   note).

Excluded: correctness of the underlying source data (`traceability.yaml`,
`status.json`, coverage report) beyond consistency checks; Lean theorem content;
correctness of the CI pipelines as a whole; the R1/R2/R3/R4 reviews. The
policy/criticality tailoring matrix and a customer-approved contract were not
available; ECSS applicability is assessed against the project's criticality-B
target and the verified clause sourcing in FRICK-REVIEW-0067.

**Independent reproduction performed.** `python3 tools/scripts/metrics.py`
(stdout) was compared with the README block; the manifest was re-parsed with an
independent script; the review-stream keying was re-implemented independently;
the workflows were inspected; the coverage report was read directly.

## 2. References

- Project norms: `docs/rules.md` (N1–N20), `AGENTS.md` (N8/N20 checklist).
- Prior review under re-verification: `docs/reviews/2026-10-11-metrics-panel-pa-review.md`
  (FRICK-REVIEW-0068, 5 Minor).
- Prior PA sourcing: `docs/reviews/2026-10-11-ecss-traceability-pa-review.md`
  (FRICK-REVIEW-0067) — ECSS clause text verified against issuer PDFs.
- Template: `docs/reviews/review-template.md`.

## 3. Findings

### 3.1 Delta verification — resolution of FRICK-REVIEW-0068 (F1–F5)

| ID | Finding (0068) | Re-verification evidence | Result |
| -- | -------------- | ------------------------ | ------ |
| F1 | Verification coverage overstated `ci:docs`-only evidence | `README.md:64` now reads "Requirements with test/CI evidence 82/96 (85.4%) — manifest tests list non-empty (incl. planned `ci:docs`)"; the strict metric `README.md:54` "SRS requirements delivered 49/55 (89.1%) — implemented (non-doc) + verified (non-`ci:docs`)" uses `_is_real_impl AND _is_verified` (`metrics.py:121-126,180`). Independent re-parse: total tests non-empty = **82/96**; strict delivered = **49/55**; the six `ci:docs`-only SRS (045/046/051/052/053/054) are excluded from delivered. | ✅ **RESOLVED** |
| F2 | Lean coverage dropped; norm-N1 disclosure regression | `README.md:65` "Formal verification coverage (Lean) 20/96 (20.8%) — manifest Lean link non-empty (norm N1)" restored; implementation line restored at `README.md:63` (96/96). Independent re-parse: Lean non-empty = **20/96**. | ✅ **RESOLVED** |
| F3 | Non-deterministic review-stream keying; stale REJECTED streams | `metrics.py:129-168` now keys by milestone token (`\bM\d+\b`), then norm token (`\bN\d+\b`), else `PA`/normalised scope, with a prefix-collapse `canon()` and "latest row wins". Independent re-implementation reproduced **18 streams / 17 closed**; the *only* open stream is `ecss traceability question (SDD in-source traceability)` = **NC PA-67-01**. The former stale `open source rtems alignment` REJECTED is merged with its APPROVED delta; the `N12` REJECTED row keys to `PA` and is superseded by the PA PASS. `README.md:56` shows "Review streams closed 17/18 (94.4%)". | ✅ **RESOLVED** |
| F4 | T13 register row + evidence mapping + History not updated | `tool-qualification.md:37` T13 row now names "completion + coverage + semaphores" and "README completion/coverage panel + V&V semaphores"; `:70` adds a **T13 evidence-claims mapping** row (incl. the CI currency gate); `:102` History 0.3.4 and `:103` History 0.3.5 record the change. | ✅ **RESOLVED** (see D3) |
| F5 | No CI currency gate for "refreshed every cycle" | Present in **both** workflows: `.github/workflows/ci.yml:58-63` and `.gitea/workflows/ci.yml:61-66` run `make coverage`, `python3 tools/scripts/metrics.py --readme`, then `git diff --exit-code README.md` with an error on divergence. | ✅ **RESOLVED** (see D2) |
| — | Metrics note accuracy / statement-coverage gap | `README.md:93` note is accurate and non-over-claiming, ends "indicators, not claims of ECSS compliance or qualification", and now explicitly flags the statement-coverage shortfall: "the current statement coverage (65.7%) and the absent branch/MC-DC and WCET results are tracked as **GAP-A4**". | ✅ **RESOLVED** (0068 F9) |

### 3.2 Consolidation — exactly one metrics panel

- `grep -c METRICS-BEGIN` = **1** and `grep -c METRICS-END` = **1** in `README.md`
  (`README.md:40`…`:95`); the single block contains all four required parts:
  **Completion** (`:48-56`), **Coverage** (`:58-68`), **Semaphores** (`:70-87`)
  and the **Metrics note** (`:93`).
- The former `## Requirements set (current)` prose panel is **removed**
  (`git diff HEAD -- README.md`, deletion at the old §"Requirements set (current)";
  no remaining section heading). No second/overlapping metrics or coverage panel
  remains in the README.
- `python3 tools/scripts/metrics.py` stdout is byte-identical to the README block
  → the single panel is generator-owned and reproducible.
- **Residual defect (D1):** the table-of-contents entry
  `- [Requirements set (current)](#requirements-set-current)` (`README.md:31`)
  still points to the removed section (dangling anchor). The panel itself is
  also not listed in the TOC. The consolidation removed the section but not its
  TOC reference, and the `--readme` cleanup (`metrics.py:332-334`) does not touch
  the TOC, so a re-run does not self-heal it.

### 3.3 Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| D1 | **Minor** | `README.md` TOC / norm N8 (complete, current docs) | The consolidation removed the `## Requirements set (current)` section but left its table-of-contents link, which now points to a non-existent anchor. A dangling in-document link is a documentation non-conformity introduced by the very change under review. | `README.md:31` (`[Requirements set (current)](#requirements-set-current)`); no matching heading anywhere in `README.md`; `metrics.py:332-334` cleanup does not remove TOC entries | Remove the stale TOC entry (and, optionally, add a `Status panel` entry or remove the TOC line). Correct in the same commit. |
| D2 | Comment | `metrics.py` output vs CI currency gate; measurement integrity | The currency gate regenerates the panel and diffs it against the checked-in README. That gate therefore also asserts **cross-environment reproducibility of the coverage numbers**, which is not guaranteed: the committed panel's statement/function figures come from the developer's gcov run, while the two pipelines run in different environments (GitHub `ubuntu-24.04` host vs the Gitea `frick-vv:local` container). A gcov/compiler/object-set difference would make the gate fail (or, if a stale panel were regenerated in CI without being committed, silently mask drift). The gate is correct as a *staleness* check but is fragile as an environment-reproducibility check. | `.github/workflows/ci.yml:58-63`; `.gitea/workflows/ci.yml:61-66`; `README.md:66-67` (1917/2920, 257/302) vs `build/coverage/coverage.json` | Keep the gate; document that coverage is only reproducible in the pinned V&V image, or pin the coverage step to that image for both remotes, or exclude the coverage rows from the byte-diff (gate on the manifest-derived rows only). Record the environment in the coverage artifact. |
| D3 | Comment | Norm N10; `tool-qualification.md` T13 | The T13 row cites "internal (R2 on changes); PA-approved metrics (FRICK-REVIEW-0068)". FRICK-REVIEW-0068 explicitly stated it does **not** substitute for the N10 *tool-source* review. No dated R2 source-review record for the new `metrics.py` computation is entered in the register; the PA approval is being used as the re-qualifying evidence. At criticality B internal R2 independence is acceptable, so this is not a non-conformity, but the qualification evidence chain for the changed computation is thinner than N10's intent. | `tool-qualification.md:37,103`; FRICK-REVIEW-0068 follow-up PA-68-04 ("this PA review does not substitute for the N10 tool source review") | Record the reviewer/date of the R2 source review that re-qualified the new `metrics.py` computation, distinct from the PA (N20) approval. |
| D4 | Comment | Review-stream keying robustness | The re-keyed heuristic yields the correct deterministic result today, but it still depends on free-text: a row whose Scope does not contain a `M<n>`/`N<n>` token or the literal `PA` falls to a normalised-string key. E.g. the PA-67-01 row keys to its prose text, not to `PA`, which is why the PA stream is *closed* (final PASS) while the genuinely open NC surfaces as a separate string stream. Adding a stable scope/NC key column (as recommended in 0068 F3) would remove the latent naming dependence. | `metrics.py:129-168`; `docs/reviews/README.md:67,71`; independent reproduction (18/17, single open stream = PA-67-01) | Add a stable key column to the review log and key on it; treat the current heuristic as a fallback only. |
| D5 | Comment | `docs/reviews/README.md` log currency (norm N11) | The review log has no row for FRICK-REVIEW-0068 (nor for this FRICK-REVIEW-0069). Norm N11 requires review results committed/pushed each cycle; FRICK-REVIEW-0068 recorded that the logrow was deferred by its task's scope constraint. | `docs/reviews/README.md:3-71` (last row = PA-67-01); `2026-10-11-metrics-panel-pa-review.md` §6 log-entry note | Append the FRICK-REVIEW-0068 and FRICK-REVIEW-0069 rows to the review log when this change is committed. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

## 4. Conclusion

All five Minor findings of FRICK-REVIEW-0068 (F1–F5) are **resolved** with
objective evidence, and the requested **consolidation is achieved**: the README
now carries exactly one generator-owned metrics panel (completion + coverage +
semaphores + note), the separate `Requirements set (current)` prose panel is
removed, and no duplicate/overlapping panel remains. The panel is byte-identical
to `tools/scripts/metrics.py` output, and the review-stream count (17/18) is
independently reproduced with PA-67-01 as the only genuinely open stream. The
note is accurate, non-over-claiming and now surfaces the statement-coverage gap.

One new **Minor** (D1: dangling TOC anchor left by the consolidation) and four
**Comments** (D2–D5) remain, none of which defeats a requirement or a safety
barrier. D2 records a **residual measurement-integrity concern** (cross-
environment coverage nondeterminism in the CI currency gate) that must be
understood if the panel is ever promoted to a gate input.

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — 1 Minor (D1, from the consolidation); 4 Comments (D2–D5); FRICK-REVIEW-0068 Minors F1–F5 confirmed closed**
- [ ] REJECTED — one or more Major findings

**PA recommendation.** The consolidated metrics panel is **fit for PA/status
use** as a completion *indicator* (the note's own disclaimer applies), subject
to closing D1. It is still **not** fit to be used as a milestone-gate or
compliance input until the D2 coverage-reproducibility concern is settled
(together with the closed F1/F5). This PA review does not certify the panel,
does not qualify T13, and does not substitute for the N10 tool source review
(see D3).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | —                |            |            |
| R2 (N9)       | —                |            |            |
| R3 (N9)       | —                |            |            |
| R4 (N9/N17)   | —                |            |            |
| **PA (N20)**  | **independent agent, FRICK-REVIEW-0069** | **2026-10-11** | **APPROVED WITH COMMENTS (1 Minor D1; 4 Comments D2–D5)** |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-68-01 (F1) | Minor | README verification-coverage definition | `ci:docs`-only requirements counted as verified | Split into strict "SRS delivered" (implemented+verified) and labelled "Requirements with test/CI evidence (incl. `ci:docs`)" | `README.md:54,64`; independent re-parse 82/96 and 49/55 | frick-fsw | **closed** | FRICK-REVIEW-0069 §3.1 |
| PA-68-02 (F2) | Minor | README panel / norm N1 disclosure | Dropped Lean + implementation rows | Restored "Formal verification coverage (Lean) 20/96" and "Requirements with implementation 96/96" | `README.md:63,65`; independent re-parse 20/96 | frick-fsw | **closed** | FRICK-REVIEW-0069 §3.1 |
| PA-68-03 (F3) | Minor | Review-closure stream heuristic | Non-deterministic keying; stale "open" streams | Re-keyed by milestone/norm token + prefix-collapse; re-derived count | `metrics.py:129-168`; independent reproduction 18/17, only open = PA-67-01 | frick-fsw | **closed** (robustness Comment D4) | FRICK-REVIEW-0069 §3.1 |
| PA-68-04 (F4) | Minor | Norm N10; T13 register | T13 row/evidence mapping/History not updated | Updated row, added evidence-claims mapping row, History 0.3.5 | `tool-qualification.md:37,70,103` | frick-fsw | **closed** (source-review record Comment D3) | FRICK-REVIEW-0069 §3.1 |
| PA-68-05 (F5) | Minor | "Refreshed every cycle" / N8-N11-N19 | No CI/make currency gate | Added `make coverage metrics` + `git diff --exit-code README.md` to both workflows | `.github/workflows/ci.yml:58-63`; `.gitea/workflows/ci.yml:61-66` | frick-fsw | **closed** (environment reproducibility Comment D2) | FRICK-REVIEW-0069 §3.1 |
| PA-69-01 (D1) | Minor | README TOC / norm N8 | Dangling TOC link to the removed `Requirements set (current)` section | Remove the stale TOC entry | `README.md:31` | frick-fsw | **open** | pending (next cycle) |
| PA-69-D2..D5 | Comment | CI gate reproducibility / N10 record / keying robustness / log currency | See §3.3 | Owner's discretion | as cited | frick-fsw | **open** (Comment) | n/a |

Comment-only items D2–D5 require no derogation; they are recorded for
disposition at the owner's discretion.

**Log-entry note.** Per the task's scope constraint ("modify nothing else"), no
row was appended to `docs/reviews/README.md`; the owner should add the log rows
for FRICK-REVIEW-0068 and FRICK-REVIEW-0069 together with the corrections (D5).

---

## Appendix A — Confidence and evidence limitations

```text
Decision/product and configuration:
  frick-fsw @ main HEAD 3fe5bf0, change under review staged/uncommitted
  (metrics.py, README.md, status.json, tool-qualification.md, CHANGELOG.md,
  .github/workflows/ci.yml, .gitea/workflows/ci.yml); target ECSS criticality B.
Claim type: mixed (Fact for reproduced counts/consolidation; Inference for
  definition-quality and process observations).
Applicable baseline and approved tailoring:
  Project targets criticality B (docs/quality/ecss-criticality-b.md). Customer
  contract / approved tailoring matrix NOT available to this reviewer; ECSS
  applicability assessed against the project baseline and FRICK-REVIEW-0067.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  tools/scripts/metrics.py (working tree); README.md; docs/status/status.json;
  docs/reviews/README.md; build/coverage/coverage.json;
  docs/vv/tool-qualification.md; .github/workflows/ci.yml;
  .gitea/workflows/ci.yml; docs/rules.md; FRICK-REVIEW-0067/0068.
  Independent reproduction: `python3 tools/scripts/metrics.py` (stdout ==
  README block); independent manifest re-parse (tests 82/96, Lean 20/96, strict
  SRS delivered 49/55); independent re-implementation of the review-stream
  keying (18 streams, 17 closed, only open = PA-67-01).
Cross-check and reconciliation:
  README block == metrics.py stdout; counts agree with an independently written
  parser; the resolution claims for F1-F5 each map to a distinct artifact with
  matching content; the workflows contain the currency gate in both remotes. No
  conflict found; D1 is a newly introduced residual (TOC anchor).
Confidence:
  High for the F1-F5 resolutions, the consolidation and the reproduced counts
  (direct, reproducible evidence). Medium for the N10 source-review observation
  (internal register conventions; no contract tailoring seen).
Basis and detection limits:
  Full source, data and both workflows inspected; substring/token-based matching
  cannot be proven bug-free beyond the current fixed-width id scheme; the
  review-stream semantics remain heuristic by construction (D4); cross-
  environment coverage reproducibility was reasoned from the workflow
  configuration, not executed in both environments.
Missing/inaccessible evidence (including paywalls):
  Customer contract / approved tailoring matrix; execution logs proving the CI
  currency gate passes in both environments.
Assumptions and expiry/recheck date:
  Assumes status.json sprints block and the coverage.json values are the
  authoritative states. Recheck on any metrics.py / workflow change, or
  2027-04-11.
Consequences if wrong:
  If the two CI environments produce different coverage counts, the currency
  gate (F5) fails spuriously or masks drift (D2); if the panel is adopted as a
  gate, D2 escalates.
Required independent human reviewer/authority:
  The project's authorized engineering/PA/release authority; this agent cannot
  approve, accept residual risk, qualify a tool, or close the NCs.
```

---

Committed and pushed with the reviewed change (norm N11).