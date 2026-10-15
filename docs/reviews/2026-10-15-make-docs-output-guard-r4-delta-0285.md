# FRICK-REVIEW-0285 — `make docs` output-existence guard (R4 delta)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-REVIEW-0285                      |
| Reviewer session                  | `aeronautics-space-sw-documentation-engineer` (fresh independent context) |
| Reviewer profile                  | R4 — Senior SW documentation reviewer (norm N17) |
| Reference revision                | working tree at HEAD `620fff8` + uncommitted Makefile/CI/docs delta (8 modified paths) |
| Items under review                | `Makefile` (`docs` target, line 159); `.github/workflows/ci.yml` `Doxygen` step (lines 243, 248); `.gitea/workflows/ci.yml` `Doxygen` step (lines 282, 287); `CHANGELOG.md` (`[Unreleased]`, lines 36–49); `docs/tooling.md` (lines 50, 105); `docs/vv/verification-results.md` (rev 2.8→2.9, cycle row line 25, History 2.9); `docs/vv/tool-qualification.md` (rev 0.3.72→0.3.73, History 0.3.73); `docs/status/loop-audit.md` (rev 3.2→3.3, header line 9, N11/N15/N19 rows, History 3.3) |
| Date                              | 2026-10-15                             |
| Scope                             | Bounded documentation/record delta review: N16 provenance and trace of the new entry (origin FRICK-REVIEW-0284 Comment 5; earlier fixes `7cb90a6`/`a4c436c`/`03773f7`/`620fff8`), N8 currency and internal consistency of the edited records, N11 retention visibility, N21/N22 boundedness (no release/qualification/NC/WCET/mission over-claim), and the single-source vs duplicated-guard question |
| Independence declaration          | Reviewer ran in a fresh session with no part in authoring or recording the change; every reproduced claim below was re-run independently by the reviewer on throw-away copies under `/tmp/opencode` (and via the public forge API for CI status) |
| Norms applied                     | N2, N8, N11, N12, N16, N17, N19, N21, N22 (N1-N22 baseline, rules rev 2.8) |

## 1. Scope

Reviewed the eight changed files of the delta and the conventions they must
satisfy (`docs/rules.md`, `AGENTS.md` §5, `docs/reviews/README.md`, both forge
`docs` jobs, and the origin review `docs/reviews/2026-10-15-ci-docs-doxygen-dir-fix-pa-delta-0284.md`).

Excluded: software behaviour and requirements; traceability; the PA (N20)
conclusion on this delta; T21/tool qualification, NC-EXT-43/44/78–80
dispositions; hosted-CI qualification credit; mission, WCET, release or
qualification judgement. No gate closure is expressed or implied here.

Baseline of this review is the eight-file delta. This report is itself an
addition to the tree (new id `FRICK-REVIEW-0285`); it is not part of the
reviewed baseline (see the id-count note in §7).

## 2. References

- `docs/rules.md` N8/N11/N16/N17/N19/N21/N22; `AGENTS.md` §§1, 3, 5.
- `FRICK-REVIEW-0284` Comment 5 (origin of the guard), §§F1–F3 and follow-up rows.
- Commits `7cb90a6` (inline `mkdir -p build/doxygen && doxygen Doxyfile`),
  `a4c436c` (steps consolidated to `make docs`), `03773f7` (`mkdir -p` in the
  `docs` target; carries reports 0283/0284 and the log rows), `620fff8`
  (metrics-panel regeneration; current HEAD).
- Forge evidence used only to check recorded currency statements: GitHub Actions
  runs #82 (`03773f7`), #83 (`620fff8`); `git ls-remote` on `origin` and `gitea`.

## 3. Independent verification performed (all reproduced by the reviewer)

| Check | Result |
| ----- | ------ |
| Delta extent | `git status --porcelain` = exactly the 8 recorded modified paths, no untracked path (at the time of the check; the review reports of this cycle were added to the working tree afterwards); `git diff --check` clean |
| Positive guard path (working-tree state, independent copy, `frick-vv:local`, Doxygen 1.9.8) | `make docs` exit **0**, `build/doxygen/html/index.html` **57,394 B** — reproduces the recorded figure exactly |
| Negative (silent no-op) path | With `GENERATE_HTML = NO`: `doxygen` exit **0** and `warning: No output formats selected!`, no `index.html`; `make docs` then exit **2** with `error: 'make docs' produced no build/doxygen/html/index.html (silent doxygen failure)` — the recorded PA scenario and message, reproduced verbatim |
| Guard-only path | Removing `index.html` from a previously successful tree: guard target exit **2** with the message; with a non-empty `index.html`: exit **0** |
| Warnings remain non-fatal | 111 `warning:` lines, `make docs` exit 0; `Doxyfile` has no `WARN_AS_ERROR` — consistent with the recorded "pre-existing warnings unchanged and non-fatal" |
| Workflow YAML and shell semantics | Both workflows parse (`yaml.safe_load` in `frick-vv:local`); neither sets `defaults:`, `shell:` or `continue-on-error`, so both `Doxygen` steps run under the Actions default fail-fast shell, and the added `test -s` line can fail the step (`bash -e`). Not executed by any CI run (see below) |
| Hosted-CI currency claims | GitHub run **#83** = `620fff8`, created 2026-10-15T05:45:57Z, conclusion **success**, all 8 jobs success including `documentation set` and `pipeline summary` — the loop-audit N15 statement is accurate. Run **#82** = `03773f7` = failure in the `build` job step *Metrics panel currency (norm N8)*, with `documentation set` passing — matches the retained 2.8 row. Run #83 executed the **pre-guard** target, so the workflow assertion is unexecuted at review time |
| Remote state | `git ls-remote` on `origin` and `gitea` both report `refs/heads/main = 620fff8cdf17e5f972afef189f56973ca2505db8`; the loop-audit header hash equals `git rev-parse HEAD` |
| Document-id uniqueness (CI check logic replicated) | **248** unique ids, 0 duplicates — the 248 recorded in the 2.8 row is reproducible and remains a point-in-time value (249 once this report is stored) |
| Table well-formedness of the four edited documents | Every table block has a consistent column count (9-column cycle log, 4-column history and audit rows); no malformed or duplicated row |
| N21/N22 boundedness | No statement claims release, certification, qualification, airworthiness, NC closure, requirement coverage, WCET or mission evidence for the guard; the records explicitly keep hosted-evidence, T21, NC-EXT-43 and release gates open |
| N11 retention visibility | Revisions 2.9 / 0.3.73 / 3.3 and their History rows are new; no pre-existing History row was edited or removed, and the body-row edits (header, N11, N15, N19) are declared in the 3.3 History row — not a silent rewrite |
| Residual stale statements located | `docs/status/loop-audit.md` line 46 (§1 "Working-tree scope boundary", 234 tracked + 120 untracked paths) and line 58 (N7 row, "current 354-path working tree is uncommitted" / "no commit/push authorized") still contradict the corrected header, N11 and N19 rows |

## 4. Findings

| # | Severity | Item / Norm | Finding | Evidence | Required action |
| - | -------- | ----------- | ------- | -------- | --------------- |
| F-01 | Minor | `docs/status/loop-audit.md` line 66 (N15) vs `docs/vv/verification-results.md` (N11 evidence record) — N16/N11 | The new audit statement "Hosted CI has now run: run #83 (`620fff8`) is fully green" is factually correct (independently confirmed) but has **no retained evidence anchor**: `verification-results.md` records run #82 in its 2.8 row and the new 2.9 row records no hosted run at all, so a reader cannot retrieve the run identity, commit or job set behind the audit claim | Reviewer query of the public forge API: run #83, commit `620fff8`, 2026-10-15T05:45:57Z, all 8 jobs success (incl. `documentation set`, `pipeline summary`), https://github.com/ilfrick/frick-fsw/actions/runs/38688558619; `verification-results.md` lines 25–26 | Add the run identity (forge, run number, commit, job result, URL) to the 2.9 cycle row or as a bounded hosted-CI evidence statement in `docs/vv/verification-results.md` |
| F-02 | Minor | `docs/status/loop-audit.md` lines 46 and 58 vs header line 9 and rows 62/70 — N8/N19 (internal consistency) | The delta's currency correction is **incomplete inside the same document**: the header, N11 and N19 rows now state the tree is pushed at `620fff8` with only the delta uncommitted, while the N7 row still says "current 354-path working tree is uncommitted … no commit/push authorized" and §1 still reports a snapshot of "234 tracked modified paths … and 120 untracked paths" that no longer exists. N19's purpose (no silent or misleading gap) is defeated by two contradicting live rows, and the 3.3 History row claims alignment of "the baseline-commit and N11/N15/N19 statements" only | `git status --porcelain` = 8 modified paths, no untracked; loop-audit lines 9, 46, 58, 62, 70; History row 3.3 (line 404) | Correct the two rows in a bounded way (update N7 and date-stamp the §1 snapshot as the `a4c436c`-era inventory, now committed) or record explicitly that §1/N7 describe a superseded snapshot. No broader rework |
| C-01 | Comment | `CHANGELOG.md` line 43 and `docs/vv/verification-results.md` line 25 — label discipline (Fact/Inference) | "so the CI gate fails on a silent generation regression **independently of the target's own check**" is an over-statement: the workflow `test -s` asserts the same artifact with the same predicate downstream of the `docs` target guard, so it is defence in depth, not an independent check; and the added lines are unexecuted at review time (run #83 ran the pre-guard target), which the sentence does not say | Reviewer reproduction of the guard and the workflow text; run #83 executed `make docs` without the guard | Optional: reword to "in addition to the target's guard, so its removal or regression is still caught by the CI gate", and state that the added step lines are first executed by the next hosted run. **No correction required for acceptance** |
| C-02 | Comment | `CHANGELOG.md` lines 36–49, 56 — N16 change boundary | The guard is a hardening/assurance improvement that PA requested "as a separate scoped change, not as a condition of this fix", yet it is recorded as a bullet inside the section titled "CI documentation-job fix: create the Doxygen output directory (2026-10-15)", whose closing sentence "This corrects a functional build defect (N21)" reads as covering the whole section. The change boundary and the N21 classification (the guard fixes no observed functional defect; it detects a silent-failure class) are blurred for a reader | `git diff CHANGELOG.md`; FRICK-REVIEW-0284 Comment 5 text; CHANGELOG line 56 | Optional: give the guard its own `###` heading or an explicit lead-in ("separate follow-up to the same job"), keeping the N21 sentence with the directory-creation fix. No correction required for acceptance |
| C-03 | Comment | `Makefile` line 159; `.github/workflows/ci.yml` line 248; `.gitea/workflows/ci.yml` line 287; `Doxyfile` lines 9, 15, 19 | The guard path `build/doxygen/html/index.html` is now hard-coded in **three** places and is derived from two `Doxyfile` settings (`OUTPUT_DIRECTORY`, `HTML_OUTPUT` default); the duplication is explained (primary guard + CI assertion) but the sync obligation is not documented, and the guard also makes `make docs` fail if HTML output is intentionally disabled. Risk is low (failure mode is fail-closed), so this is a documentation gap, not a hazard | `git diff` of the three files; `Doxyfile` lines 9/15/19; reviewer reproduction of the `GENERATE_HTML = NO` case | Optional: one line in `docs/tooling.md` noting that the artifact path is duplicated in the target and both workflows and must change together, or a tracked NC. No correction required for acceptance |
| C-04 | Comment | `docs/status/handover.md` (not in this delta) — N8 | The authoritative resume point (rev 2.25, 2026-10-14) was not touched and still tells a resuming agent that hosted/forge CI is unrun on the tree (lines 313, 328), although runs #82/#83 have since executed; a fresh context will not learn of the guard or of the hosted runs from the entry point | `docs/status/handover.md` lines 313, 328; runs #82/#83 | Optional and bounded: refresh the handover statement when the delta is committed. Not a condition of this delta's acceptance |

No Major finding is raised. The guard was reproduced positively, negatively and
in isolation; the recorded evidence text, the recorded HEAD hash, the recorded
248-id count and the corrected hosted-CI statements are accurate; the delta
introduces no malformed table, duplicate id or unretained overwrite.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — two Minor findings tracked to closure (NC-01, NC-02)
- [ ] REJECTED — one or more Major findings

The reviewed documentation/record delta is **approved with comments** for this
bounded scope. The change is proportional to FRICK-REVIEW-0284 Comment 5, its
evidence claims are reproducible, and its bounding statements are correct.

**NC-01 (F-01) and NC-02 (F-02) must be corrected and the corrections verified
by a fresh-context R4 delta re-review before commit/push (N12/N17)**; C-01–C-04
are comments requiring no correction for acceptance. This review is not overall
N17 approval of the repository documentation set, not the N20 PA review of this
delta, and not hosted-CI/T21/NC-EXT-43 or release/qualification closure. The
guard is a build-tooling fail-closed check: it is not WCET, timing, requirement
coverage, mission or target evidence, and no such claim is made or implied.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | `aeronautics-space-sw-documentation-engineer` (independent context) | 2026-10-15 | APPROVED WITH COMMENTS — NC-01/NC-02 tracked to closure; C-01–C-04 comments |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Minor | `docs/status/loop-audit.md` 3.3 N15 row; `docs/vv/verification-results.md` (F-01) | Hosted-CI-green claim has no retained run identity in the evidence record | Record run #83 (`620fff8`) identity/URL/result in `docs/vv/verification-results.md`; add one sentence to the N15 row | Fresh-context R4 delta re-review of the added sentence(s); forge run re-check | author (`aeronautics-space-fsw-engineer`) | **open** | pending |
| NC-02 | Minor | `docs/status/loop-audit.md` lines 46, 58 (F-02) | N7 row and §1 scope snapshot contradict the corrected header/N11/N19 within the same revision | Bounded correction or explicit superseded-snapshot date-stamp of those two rows | Fresh-context R4 delta re-review of the two rows | author | **open** | pending |
| FU-03 | Comment | `CHANGELOG.md` line 43; `verification-results.md` line 25 (C-01) | "independently of the target's own check" over-states a duplicated assertion; CI lines unexecuted | Optional rewording | Inspection if changed | author | **open (optional)** | not required |
| FU-04 | Comment | `CHANGELOG.md` structure (C-02) | Guard recorded inside the docs-dir-fix section; N21 sentence covers both | Optional separate heading/label | Inspection if changed | author | **open (optional)** | not required |
| FU-05 | Comment | `Makefile` + 2 workflows; `Doxyfile` (C-03) | Artifact path hard-coded in three places; sync obligation undocumented | Optional one-line note in `docs/tooling.md` or tracked NC | Inspection if changed | author | **open (optional)** | not required |
| FU-06 | Comment | `docs/status/handover.md` (C-04) | Resume point still states forge CI is unrun on the tree | Optional bounded refresh at commit time | Inspection if changed | author | **open (optional)** | not required |
| FU-07 | Process | `docs/reviews/README.md`; N11/N17 | The review log must receive a row for this report ("append a row to the log in the working tree immediately"); the log currently ends with the FRICK-REVIEW-0284 row | Author appends the FRICK-REVIEW-0285 row before commit/push | `docs/reviews/README.md` log row | author | **open** | n/a |

Note: the recorded documentation-set unique-id count of **248** (2.8 row,
reproduced by the reviewer with the CI check logic) is the count of the state
reviewed, before the reports of this cycle were stored. With this report
(`FRICK-REVIEW-0285`) and the concurrently produced PA delta report already in
the working tree (`FRICK-REVIEW-0286`, outside this reviewer's scope) the count
is **250** (re-checked, no duplicate). The recorded 248 is therefore not an
error (point-in-time value, like the other cycle rows) and needs no correction.

Independence note: the delta reviewed here is the eight-file Makefile/CI/docs
change; the PA delta report `FRICK-REVIEW-0286` was authored in a separate
session and its conclusion is not assessed or endorsed in this report.

---

Reviewer changed no repository file other than this report (N11: stored in the
working tree as produced; committed with the change only after the applicable
gates pass).
