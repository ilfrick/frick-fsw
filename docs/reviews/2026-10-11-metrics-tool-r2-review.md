# Review Report — V&V tool T13 `tools/scripts/metrics.py` (N10 source review, R2)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0070                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (Senior SW V&V engineer)            |
| Reference revision                | `main`, HEAD `3fe5bf0`, change staged/uncommitted (2026-10-11) |
| Items under review                | `tools/scripts/metrics.py`; `docs/vv/tool-qualification.md` (T13 row, evidence-claims mapping, History); `README.md` metrics panel; `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml` (currency gate); `Makefile` (`coverage`/`metrics`) |
| Date                              | 2026-10-11                             |
| Scope                             | Norm N10 independent source review + qualification inspection of V&V tool T13 (single status/completion-metrics panel generator). Correctness, robustness, determinism, definition-to-implementation consistency, evidence/qualification adequacy, and measurement integrity. |
| Independence declaration          | Reviewer wrote none of `metrics.py`, the panel, the workflows or the T13 register; fresh session with no author context. Read-only review: only this report file was created; no other repository file was modified. |
| Norms applied                     | N1, N4, N8, N9, N10, N11, N12, N15, N16, N19, N20 (see `docs/rules.md`, `docs/engineering/independent-review.md`) |

## 1. Scope

Independent R2 **source review** of V&V tool T13 `tools/scripts/metrics.py`
(the single completion/coverage/semaphore panel generator) and inspection of its
qualification evidence, as required by norm N10 ("source review + inspection +
configuration management"; independent at the criticality/risk warrant, R2
profile). This review is the dated R2 source-review record requested by
FRICK-REVIEW-0069 Comment **D3** (PA-68-04), which stated that neither the PA
approval nor FRICK-REVIEW-0068 substitutes for the N10 *tool-source* review.

**In scope.**

1. Source correctness and robustness of `metrics.py` (missing files, malformed
   manifest/status/coverage, edge cases, division-by-zero, inline-vs-block YAML
   list parsing, determinism).
2. Consistency between the *stated* definitions (module docstring, README panel
   header, Definition column, metrics note) and the *implemented* computation:
   **implemented / verified / delivered**, **SSS satisfied**, **design
   traceability**, **review streams**, **coverage**.
3. Qualification evidence: `docs/vv/tool-qualification.md` T13 register row +
   evidence-claims mapping + History; the README panel T13 renders; the CI
   currency gate (`make coverage` + `metrics.py --check`) in both workflows.
4. Measurement integrity: metrics that could be gamed or that hide a gap; the
   `--check` normalisation that ignores code-coverage rows.

**Excluded.** Correctness of the *underlying source data* (`traceability.yaml`,
`status.json`, `coverage.json`, review log prose) beyond consistency checks;
`coverage.py`/gcov correctness (separate tool T12); the CI pipelines as a whole;
Lean theorem content; the R1/R3/R4 reviews and the PA (N20) reviews. No customer
contract or approved tailoring matrix was available; criticality B is taken from
the project baseline (`AGENTS.md`, `docs/quality/`).

**Independent reproduction performed (2026-10-11).**

- `python3 tools/scripts/metrics.py` → exit 0; stdout compared **byte-for-byte**
  with the README block between the markers.
- `python3 tools/scripts/metrics.py --check` → exit 0 ("metrics panel: current").
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → exit 0;
  `PASS (96 requirements, 55 srs, 27 sss, 14 gaps, 51 lean theorems checked)`.
- An independent manifest parser (written from the YAML shape, not importing
  `metrics.py`) reproduced: total 96 / srs 55 / sss 27 / gap 14; SRS delivered
  **49/55**; SSS satisfied **21/27**; with-implementation **96/96**; with-tests
  **82/96**; with-Lean **20/96**; design-traceability **2/55**; `ci:docs`-only
  SRS = {045,046,051,052,053,054}.
- An independent re-implementation of the review-stream keying reproduced
  **18 streams / 17 closed**, the only open stream being the PA-67-01 NC prose
  stream.
- The `--check` normalisation was exercised directly (see finding R2-01).

Seed/generator/file-hash concerns: none — the tool is a pure read-only
transformation of controlled text/JSON inputs; no RNG, network or clock.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); `AGENTS.md` (N8/N10/N20 checklist).
- N9 profiles: `docs/engineering/independent-review.md`.
- Template: `docs/reviews/review-template.md`.
- Prior reviews: FRICK-REVIEW-0067 (ECSS traceability, PA-67-01),
  FRICK-REVIEW-0068 (metrics panel PA; F1–F5), FRICK-REVIEW-0069 (metrics panel
  PA delta; D1–D5).
- ECSS context: ECSS-Q-ST-80C (tool and tool qualification), ECSS-E-ST-40C
  (verification tools derived from standards) as cited in the qualification
  register.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R2-01 | **Minor** | `metrics.py --check` / T13 evidence claim; norm N8/N19 "refreshed every cycle" | The `--check` currency gate deliberately **drops both code-coverage rows before comparison** (`_normalize` removes every line containing `Code statement`/`Code function`). The gate therefore cannot detect coverage drift: I changed/removed the coverage rows and `_normalize(block)==_normalize(fresh)` still held. The T13 evidence-claims mapping states the gate covers the "README completion/**coverage** panel", but coverage is effectively ungated. A code change that lowers coverage and is committed without re-running `make metrics` passes CI silently, and the note's figure even stays self-consistent (see R2-02). | `metrics.py:316-319` (`_normalize`), `:330-340` (`check`); direct experiment: rendered panel vs. panel with coverage rows altered → `full equal False` but `normalized equal True`; `tool-qualification.md:70`; `.github/workflows/ci.yml:60-61`; `.gitea/workflows/ci.yml:63-64` | Narrow the normalisation to what is genuinely environment-dependent, or state explicitly that coverage rows are *not* gated and that the panel is an indicator only. If coverage is ever a gate input, remove the coverage rows from `_normalize` and pin `make coverage` to the T1 image in both remotes. |
| R2-02 | **Minor** | Rendered metrics note vs. `coverage.json`; measurement integrity | The note text embeds a **hardcoded** statement-coverage value, `the current statement coverage (65.7%)`, as a source literal — it is not read from `coverage` (`c['cov']`). If the real coverage moves, the panel row (computed) and the note (literal) diverge, and because `_normalize` keeps the note line, `--check` compares the literal against itself and never catches the divergence. This is a mechanism by which the panel can display a stale/misleading coverage figure. | `metrics.py:296-298` (literal `65.7%`); `README.md:65` (computed row `1917/2920 (65.7%)`) vs `README.md:92` (literal); `coverage.json` = 1917/2920 (65.7%) today (they happen to agree) | Derive the note figure from `coverage` (or drop the numeric literal from the note and reference the row). |
| R2-03 | **Minor** | `docs/vv/tool-qualification.md`; norm N16 (revision history) | The register's own **header `Revision` is `0.3.4`, while its History table's latest entry is `0.3.5`** (0.3.5 records exactly the change under review). The revision header was not advanced with the last history row, so the controlled document is internally inconsistent. | `tool-qualification.md:7` (`0.3.4`) vs `:103` (`0.3.5`); `git diff HEAD` shows the header was bumped only `0.3.3 → 0.3.4` while two history rows (0.3.4, 0.3.5) were added | Set the header `Revision` to `0.3.5` (or reconcile the history rows) in the same commit. |
| R2-04 | **Minor** | T13 evidence-claims mapping vs. implemented gate | The register describes the CI currency gate as `make coverage metrics` + `git diff --exit-code README.md`. The workflows actually run `make coverage` then `python3 tools/scripts/metrics.py --check` — there is **no `make metrics` step and no `git diff --exit-code`**; the comparison is internal to `--check`. The documented qualification evidence does not match the implemented gate (N10 traceability of the tool to its evidence claim). | `tool-qualification.md:70`; `.github/workflows/ci.yml:58-61`; `.gitea/workflows/ci.yml:61-64`; `Makefile:125-127` (`metrics` target distinct from the gate) | Correct the evidence-claims wording to the actual command (`make coverage` + `metrics.py --check`). |
| R2-05 | **Minor** | Sprint metric provenance; stated-vs-implemented | The panel attributes "Sprints completed" to `docs/planning/sprint-plan.md` (FRICK-PLAN-0002), and the panel header says metrics are generated "from the traceability manifest, **the sprint plan**, the review log and the coverage report". The code never reads the sprint plan: the values come from the hand-maintained `status.json` `sprints` block (`spr.get("total"/"done")`). Semaphores likewise come from `status.json`, which the header does not mention. A reader can therefore believe the sprint figure is derived/reconciled when it is a manual entry. | `metrics.py:201,207-208` (`spr = status.get("sprints", {})`); `status.json:42-47`; `README.md:43,51` | State `status.json` as the sprint/semaphore source (and keep the plan only as the human-facing definition), or actually derive the sprint count from the plan. |
| R2-06 | **Minor** | Design-traceability metric; measurement integrity | `design_ok` counts an SRS whose **id appears as a substring anywhere** in the SDD (`i["id"] in sdd`). That measures "id mentioned in the SDD", not a requirements→design trace, and is trivially gameable: pasting the full SRS id list into the SDD would yield 55/55 without any design element. The Definition column reads "SRS traced to the SDD (ECSS design DRD Annex F)". The metric note candidly records the open NC `PA-67-01`, so no over-claim is made *in prose*, but the numeric definition is weaker than the label. | `metrics.py:198`; `README.md:54`; independent check: SDD contains only `SRS-REQ-015`, `SRS-REQ-033` → 2/55 | Rename the metric to "SRS ids referenced in the SDD" or define it against an explicit design-trace matrix (the PA-67-01 deliverable). |
| R2-C1 | Comment | `_review_keys` / `canon` prefix-collapse; robustness | The `canon()` prefix-collapse (`x.startswith(k) or k.startswith(x)`) over-merges disjoint identifiers that share a prefix: e.g. a future `M1` key would merge with `M10`, and `N1` with `N16`. Not triggered by the current log (keys are M2/M3/M4/M6/M7/M10/M11/M12/M16, N1/N7/N20, plus prose), but it makes the stream count data-dependent in a non-obvious way. Echoes PA Comment D4. | `metrics.py:159-161`; independent reproduction listing the 18 keys | Key on a stable explicit column, or bound the collapse to exact token equality (`M1 != M10`). |
| R2-C2 | Comment | Review-stream keying of PA-opened NC | A row whose Scope is an ECSS/PA question but does not contain `M<n>`/`N<n>` or start with `PA` falls to the normalised-prose key. Consequently the PA-67-01 NC row keys to `ecss traceability question sdd in source traceability`, not to `PA`; the genuinely open NC surfaces as a separate string stream while the `PA` stream is closed. This is why the single open stream is correct *today*, but by heuristic, not by design. | `metrics.py:129-139,152-155`; `docs/reviews/README.md:71`; independent reproduction | Add a stable key/NC column to the review log (PA D4). |
| R2-C3 | Comment | Robustness to malformed inputs | `main()` validates only file existence, not JSON shape; `render()` indexes `status["gates"]`/`status["deferred"]` and `coverage['lines_covered']` directly. A malformed `status.json`/`coverage.json` (or a missing key) raises an uncaught traceback rather than a controlled `error:` + exit code. `check()` opens `README` without a guard. | `metrics.py:214-215,350-358,262-266,324` | Guard the JSON shape and print a controlled error; wrap file reads. |
| R2-C4 | Comment | Code hygiene | `parse_manifest` (`:70`) and `review_streams` (`:145`) open files without context managers; `_toc_cleanup` (`:307-313`) is now dead code (the `Requirements set (current)` section and its TOC entry are already absent from `README.md`). | `metrics.py:70,145,307-313`; `grep "Requirements set" README.md` → none | Use `with open(...)`; remove `_toc_cleanup` or keep it only if it still guards a live transition. |
| R2-C5 | Comment | "Requirements with implementation" strength | `with_impl` counts any non-empty implementation list, **including `docs/`**, so 96/96 is a "has a truthy list" indicator rather than an implementation metric. The Definition column says so honestly ("incl. `docs/`"). | `metrics.py:176`; `README.md:62` | Keep the honest label; optionally show the strict (non-doc) count alongside. |
| R2-C6 | Comment | Prior PA dispositions | PA D1 (dangling `README.md` TOC anchor) is now **resolved** in the working tree (the stale TOC entry is removed), and PA D5 (log rows for FRICK-REVIEW-0068/0069) is **resolved** (`docs/reviews/README.md:72-73`). PA D3 (dated R2 source-review record) is addressed by *this* report: the T13 register row should cite **FRICK-REVIEW-0070** and its date, distinct from the PA (N20) approvals. | `README.md` (no `Requirements set (current)`); `docs/reviews/README.md:72-73`; `tool-qualification.md:37` still cites only "PA-approved metrics (FRICK-REVIEW-0068)" | Update `tool-qualification.md:37` to record the R2 source review (FRICK-REVIEW-0070, 2026-10-11). |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

### 3.1 Positive verification (no finding)

- **Definition-to-implementation consistency** holds for the core semantics:
  `_is_real_impl` = non-`docs/`, non-`ci:` (`metrics.py:121-122`); `_is_verified`
  = non-empty and not exactly `["ci:docs"]` (`:125-126`); `delivered` = both
  (`:180-181`); SSS satisfied = all children delivered else self (`:187-193`);
  these match the module docstring (`:18-24`) and README rows (`:52-53`).
- **Parsing** of inline flow lists and block lists is correct for the manifest
  shape, including quoted tokens containing commas and block items whose value
  contains `-` (split on the first marker only): `_split_list` (`:53-62`),
  block branch (`:92-98`). Independently reproduced counts match exactly.
- **Edge cases:** division-by-zero is avoided by `pct` (`:217`, returns `"n/a"`
  when `d == 0`); missing `coverage.json` renders "not measured" (`:267-268`);
  missing `DESIGN`/`REVIEWS` are guarded (`:196,200`).
- **Determinism:** line-ordered parse, `dict` insertion order for gates,
  `min(cands, key=len)` on an ordered list — the panel is reproducible
  (`--check` "current").
- **Reproduction** of `trace.py` convergence (`96/55/27/14/51`) and of the panel
  counts (see §1).
- **T13 evidence chain exists:** register row (`tool-qualification.md:37`),
  evidence-claims mapping (`:70`), History 0.3.4/0.3.5 (`:102-103`), and the
  currency-gate job in both workflows (`.github/workflows/ci.yml:58-61`,
  `.gitea/workflows/ci.yml:61-64`).

## 4. Conclusion

The T13 source is **correct and reproducible**: every panel figure I recomputed
with an independent parser agrees with `metrics.py` output, the README block is
byte-identical to stdout, `--check` exits 0, `trace.py` reports PASS at
96/55/27/14/51, and the inline/block list parsing, edge-case guards and
determinism behave as documented. No Major finding: none of the six Minors
defeats a requirement or a safety barrier, and the panel is explicitly and
accurately disclaimed in its own note as an *indicator, not a claim of ECSS
compliance or qualification*.

The Minors concentrate on **measurement integrity and stated-vs-implemented
consistency**: the coverage rows that the `--check` gate normalises away
(R2-01) combined with the hardcoded coverage figure in the note (R2-02) mean the
panel's coverage content can go stale silently while the gate reports "current";
the register's revision header (R2-03) and its description of the gate (R2-04)
do not match the controlled artifacts; the sprint figure is manually sourced but
attributed to the plan (R2-05); and design traceability is a substring-presence
metric weaker than its label (R2-06). These require correction or explicit
justification before the tool's coverage-related evidence can be relied upon.

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — 6 Minor (R2-01…R2-06) tracked to closure; Comments R2-C1…R2-C6; prior PA D1/D5 confirmed resolved**
- [ ] REJECTED — one or more Major findings

**N10 source-review statement.** The N10 independent source review of T13 is
**satisfied by this record (FRICK-REVIEW-0070, 2026-10-11)**: the tool has been
source-reviewed for correctness, exercised for qualification (reproduced panel +
PASS trace), and traced to its evidence claims. The Minors above are findings to
close (norm N12) in the same cycle; the qualification register must be updated to
cite this R2 record and correct R2-03/R2-04. This reviewer confirms correctness
of the tool's *logic*; this report does **not** qualify the tool, approve a
milestone gate, or accept residual risk — those require the project authority
(norm N10/N9).

**Recommended path for the coverage gate.** Either (a) explicitly scope the
`--check` gate to the manifest/status/review-derived rows and state that coverage
is not gated, or (b) make coverage gated by deriving the note figure from
`coverage.json` and pinning the `make coverage` step to the T1 V&V image in both
remotes. Option (a) is acceptable at criticality B given the tool is an
indicator; option (b) is required before the panel is used as any gate input.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | —                |            |            |
| **R2 (N9)**   | **independent agent, FRICK-REVIEW-0070** | **2026-10-11** | **APPROVED WITH COMMENTS (6 Minor, 6 Comments)** |
| R3 (N9)       | —                |            |            |
| R4 (N9/N17)   | —                |            |            |
| PA (N20)      | —                |            |            |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| T13-R2-01 | Minor | `--check` normalisation vs T13 coverage claim | Coverage rows excluded from the currency gate → coverage drift undetected | Scope the gate to non-coverage rows and label it, or gate coverage explicitly | TBD | frick-fsw | open | pending |
| T13-R2-02 | Minor | Metrics note hardcoded `65.7%` | Literal coverage figure not derived from `coverage.json`; can silently diverge | Derive from `coverage` or remove the numeric literal | TBD | frick-fsw | open | pending |
| T13-R2-03 | Minor | `tool-qualification.md` header revision | Header `0.3.4` vs History latest `0.3.5` (norm N16) | Advance header to 0.3.5 | TBD | frick-fsw | open | pending |
| T13-R2-04 | Minor | T13 evidence-claims mapping | Gate described as `make coverage metrics` + `git diff`; actual is `make coverage` + `metrics.py --check` | Correct the wording | TBD | frick-fsw | open | pending |
| T13-R2-05 | Minor | Sprint metric source | Attributed to the sprint plan; actually read from `status.json` | State `status.json` as source or derive from the plan | TBD | frick-fsw | open | pending |
| T13-R2-06 | Minor | Design traceability definition | Substring presence in SDD labelled "traced to the SDD"; gameable | Rename or define against a real design trace (PA-67-01) | TBD | frick-fsw | open | pending |
| T13-R2-D3 | Comment | N10 record (PA D3) | Register lacked a dated R2 source-review record | Cite FRICK-REVIEW-0070 (2026-10-11) in `tool-qualification.md:37` | this report | frick-fsw | open (Comment) | n/a |
| T13-R2-C1..C6 | Comment | keying robustness / malformed inputs / hygiene / metric strength / PA D1-D5 | See §3 | Owner's discretion | as cited | frick-fsw | open (Comment) | n/a |

Comment-only items require no derogation; they are recorded for disposition at
the owner's discretion. Per the task's scope constraint ("modify nothing else"),
no row was appended to `docs/reviews/README.md`; the owner should add the
FRICK-REVIEW-0070 log row together with the corrections.

---

## Appendix A — Confidence and evidence limitations

```text
Decision/product and configuration:
  frick-fsw @ main HEAD 3fe5bf0; change staged/uncommitted (metrics.py,
  README.md, status.json, tool-qualification.md, docs/reviews/README.md,
  .github/workflows/ci.yml, .gitea/workflows/ci.yml); target ECSS criticality B.
Claim type: mixed — Fact for reproduced counts, byte-identity and exit codes;
  Inference for definition-quality and measurement-integrity observations.

Applicable baseline and approved tailoring:
  Project targets criticality B (docs/quality/). Customer contract / approved
  tailoring matrix NOT available; N10 independence assessed against the project
  norm and the R2 profile in docs/engineering/independent-review.md.

Evidence consulted (issuer, ID/revision, artifact, date):
  tools/scripts/metrics.py (working tree, HEAD 3fe5bf0); README.md (panel
  :39-94); docs/status/status.json (:42-47); build/coverage/coverage.json;
  docs/vv/tool-qualification.md (rev header 0.3.4 / History to 0.3.5);
  docs/reviews/README.md (:72-73); .github/workflows/ci.yml (:58-61);
  .gitea/workflows/ci.yml (:61-64); Makefile (:120-127); docs/rules.md N9/N10/N11;
  FRICK-REVIEW-0067/0068/0069.
  Independent reproduction: metrics.py stdout == README block; metrics.py --check
  exit 0; trace.py PASS 96/55/27/14/51; independent manifest parser (49/55, 21/27,
  96/96, 82/96, 20/96, design 2); independent review-stream re-implementation
  (18/17, only open = PA-67-01 prose stream); direct `_normalize` experiment.

Cross-check and reconciliation:
  All reproduced counts agree across three independent methods (metrics.py,
  independent parser, trace.py). Findings R2-03/R2-04 reconcile the register text
  against the actual files; R2-01/R2-02 reconcile the stated gate with the
  implemented normalisation. No conflicting evidence found.

Confidence:
  High for correctness/robustness, the reproductions, and the stated-vs-
  implemented mismatches (direct, reproducible evidence). Medium for the
  measurement-integrity *consequences* (they depend on how the panel is used,
  which is a governance decision, not a code property).

Basis and detection limits:
  Full source, the manifest shape, both workflows, status.json and coverage.json
  inspected. The parser's line-oriented regexes cannot be proven robust beyond
  the current fixed-width id/field scheme; review-stream semantics are heuristic
  by construction. Cross-environment coverage reproducibility was reasoned from
  the workflow configuration, not executed in both environments.

Missing/inaccessible evidence (including paywalls):
  Customer contract / approved tailoring matrix; execution logs proving the CI
  gate passes in both environments; the T1 image's exact gcov/compiler versions.

Assumptions, expiry/recheck date:
  Assumes status.json and coverage.json are the authoritative states. Recheck on
  any metrics.py / workflow / register change, or 2027-04-11.

Consequences if wrong:
  If coverage is later treated as a gate input while R2-01 stands, coverage
  regressions pass undetected; if the note literal (R2-02) is relied on, a stale
  figure may be read as current.

Required independent human reviewer/authority:
  The project's authorized engineering/PA/release authority; this agent cannot
  qualify a tool, approve a gate, or accept residual risk.
```

---

Committed and pushed with the reviewed change (norm N11).