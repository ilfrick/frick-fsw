# FRICK-REVIEW-0283 — CI `docs`-job Doxygen output-directory fix (R4 delta)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-REVIEW-0283                      |
| Reviewer session                  | `aeronautics-space-sw-documentation-engineer` (fresh independent context) |
| Reviewer profile                  | R4 — Senior SW documentation reviewer (norm N17) |
| Reference revision                | working tree at HEAD `a4c436c` + docs delta (uncommitted) |
| Items under review                | `Makefile` (`docs` target, line ~156); `CHANGELOG.md` (`[Unreleased]` entry, 2026-10-15); `docs/vv/verification-results.md` (rev 2.7→2.8, cycle row, History row); `docs/status/loop-audit.md` (History row 3.2) |
| Date                              | 2026-10-15                             |
| Scope                             | Documentation delta only: accuracy and boundedness of the recorded root cause/fix, internal consistency of revisions/dates/counts, table well-formedness, N2/N8/N16/N17/N19/N21 conformance |
| Independence declaration          | Reviewer ran in a fresh session with no part in authoring or executing the change; every reproduced claim below was re-run independently by the reviewer on copies outside the repository |
| Norms applied                     | N2, N8, N11, N12, N16, N17, N19, N21 (N1-N22 baseline) |

## 1. Scope

Reviewed the four changed files of the delta (`Makefile`, `CHANGELOG.md`,
`docs/vv/verification-results.md`, `docs/status/loop-audit.md`) plus the
conventions they must satisfy (`docs/rules.md`, `AGENTS.md`,
`docs/reviews/README.md`, both CI `docs` jobs).

Excluded: code, requirements, traceability and tool-qualification content; no
gate closure, hosted-CI evidence, T21/NC-EXT-43 disposition, mission,
qualification or release judgement is expressed or implied. The N20 (PA) review
of this delta is **not** performed or assessed here; a separately authored PA
delta report exists in the same working tree
(`2026-10-15-ci-docs-doxygen-dir-fix-pa-delta-0284.md`, FRICK-REVIEW-0284) and is
outside this reviewer's scope.

Baseline of this review is the four-file delta. This report is itself an
addition to the tree; its own file and id are not part of the reviewed baseline
(see the id-count note in §7).

## 2. References

- `docs/rules.md` N2/N8/N11/N12/N16/N17/N19/N21; `AGENTS.md` §§1, 3, 5.
- `.github/workflows/ci.yml` `docs` job (apt `doxygen graphviz` + `make docs`);
  `.gitea/workflows/ci.yml` `docs` job (`make docs` in the V&V container).
- Commits `7cb90a6`, `57974d2` (`Makefile`, both workflow `Doxygen` steps),
  `a4c436c` (workflows switched to `make docs`; `rm -rf build/doxygen` added).
- `docs/vv/tool-qualification.md` (T1/T11 rows, history 0.1.3);
  `CHANGELOG.md` `[0.1.0]` section (CI `documentation set` entry).

## 3. Independent verification performed (all reproduced by the reviewer)

| Check | Result |
| ----- | ------ |
| Pre-fix reproduction, pristine copy without `build/`, Doxygen 1.9.8 (`frick-vv:local`) | Reproduced: `error: tag OUTPUT_DIRECTORY: Output directory 'build/doxygen' does not exist and cannot be created`, direct `doxygen` exit **1** (make exit 2) |
| Post-fix `make docs`, same pristine copy | `make docs` exit **0**; `build/doxygen/html/index.html` present (57,394 B); 2,136 entries in `build/doxygen/html/` |
| "Pre-existing warnings unchanged" | 110 Doxygen warnings before and after the docs delta; the sorted warning sets are identical except a line-number shift in `verification-results.md` (213→214) caused by the new cycle row |
| Doxygen version alignment with the GitHub job | `frick-vv:local` is Ubuntu 24.04.5 with `doxygen 1.9.8+ds-2ubuntu0.1`; the GitHub job installs the same distro package on `ubuntu-24.04` — the "Doxygen 1.9.8" claim is consistent |
| Document-id uniqueness (`docs/**/*.md`, CI check logic) | **246** unique ids, 0 duplicates — reproduces the recorded figure (point-in-time; see note in §7) |
| Reviewer-persona drift check (norm N9) | Persona bodies in sync (docs vs `.opencode`) |
| N13 licence check | OK (`license_check.py`) |
| `make gsw-selfcheck` | 5 GSW + 9 metrics + 14 licence tests — matches the recorded "5 GSW + 9 metrics + 14 license Python checks" |
| Table/record consistency | `verification-results.md` header 2.8 / date 2026-10-15 = new cycle row and History row; all cycle-log rows have 9 cells; all rows of the loop-audit History table (122 data rows) have 4 cells; no malformed row and no duplicated id |
| Boundedness of claims | Records state "Hosted CI was not run" and that no hosted-evidence, T21, NC-EXT-43 or release/qualification gate is closed; no over-claim found |
| Gate applicability (N15 interpretation note 2) | Build/ctest/Lean/trace/smoke/RTEMS are recorded `NOT RUN` with a reason ("docs/Makefile-only change"), i.e. recorded, not silently skipped |

## 4. Findings

| # | Severity | Item / Norm | Finding | Evidence | Recommendation |
| - | -------- | ----------- | ------- | -------- | -------------- |
| F-01 | Minor | `CHANGELOG.md` (new `[Unreleased]` entry); `docs/status/loop-audit.md` row 3.2; `docs/vv/verification-results.md` row 2.8 — N16 (origin/full trace), N8 | The recorded history is factually correct but **incomplete on provenance**: it explains the Doxygen mechanism and local masking ("earlier builds had already created `build/`") but does not record that the two workflows previously created the directory inline (`mkdir -p build/doxygen && doxygen Doxyfile`, added by `7cb90a6`, still present at `57974d2`), that this same CI `docs` failure class was already fixed once and recorded in the `[0.1.0]` entry ("the Doxygen step failed inside the pinned V&V image (doxygen 1.9.8) because `build/doxygen` did not exist; both workflows now create it first"), and that `a4c436c` replaced those inline steps with `make docs` while the `docs` target itself never created the parent directory. Without that sentence, a reader concludes the CI failure appeared spontaneously and cannot see that this is the second occurrence of one failure class — the audit-trail purpose of N16 and the regression-prevention value of the record. | `git show 7cb90a6 -- .github/workflows/ci.yml` (adds `mkdir -p build/doxygen && doxygen Doxyfile`); `git show 57974d2:.github/workflows/ci.yml` `Doxygen` step lines 254-258 and `git show 57974d2:.gitea/workflows/ci.yml` lines 295-299 (same inline `mkdir -p`); `git show a4c436c -- .github/workflows/ci.yml` (step becomes `sudo apt-get … doxygen graphviz` + `make docs`); `CHANGELOG.md` `[0.1.0]` section (`CI documentation set` bullet); `docs/vv/tool-qualification.md` history 0.1.3 ("Doxygen output dir created") | Add **one bounded sentence** to the new `CHANGELOG.md` entry (e.g. "The CI-visible failure is a recurrence: both workflows previously ran `mkdir -p build/doxygen && doxygen Doxyfile` inline (`7cb90a6`); the revamp `a4c436c` moved the step to `make docs`, whose target did not create the parent directory.") and mirror the recurrence in the loop-audit 3.2 row if trivially possible. Do not expand further (N21); no other wording change is required for acceptance. |
| C-01 | Comment | `docs/status/loop-audit.md` header metadata — N16/N8 internal consistency | The header still reads `Revision 3.1` / `Date 2026-10-14` / baseline commit `57974d2…(HEAD)` while a new History row `3.2 / 2026-10-15` was appended; in the same delta `verification-results.md` was bumped consistently (header 2.7→2.8 + date), so the two records now follow different conventions. | `docs/status/loop-audit.md` lines 7-9 vs line 403; `docs/vv/verification-results.md` lines 7-8 vs 644. Precedent of an accepted lag and later re-alignment in this same file: commit `e65fc1b` (header 0.23, last row 0.24) and History row 1.3 ("Align current audit revision"). | Align `Revision`/`Date` to 3.2 / 2026-10-15 (and note the audit snapshot basis if the §1/§2 tables still describe the earlier tree), or record explicitly why the header remains 3.1. No derogation to the delta; not an NC. |
| C-02 | Comment | Mechanism statement in the new `CHANGELOG.md` entry and the two record rows — label discipline (Fact/Inference) | "Doxygen 1.9.8 creates `OUTPUT_DIRECTORY` with a single-level `mkdir(2)`" asserts an internal implementation detail. What this review could verify is the observable behaviour (with the parent absent, Doxygen does not create intermediate directories, reports the quoted error and exits 1; `mkdir -p` resolves it). | Reviewer reproduction in §3; doxygen behaviour only, no Doxygen source inspected. | Optional: prefer "does not create missing parent directories" (observable) or keep the current wording as an attributed explanation. **No correction required for acceptance**; not an NC. |

No Major finding is raised. Nothing in the delta claims closure of hosted CI,
T21 tool qualification, NC-EXT-43, mission compliance or any release /
qualification gate, and no duplicate document id or malformed table row was
found.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure (NC-01)
- [ ] REJECTED — one or more Major findings

The reviewed documentation delta is **approved with comments** for this bounded
scope: the fix works from a pristine tree, the evidence claims are reproducible,
and the claims are bounded. **NC-01 (F-01) must be corrected and the correction
verified by a fresh-context R4 delta re-review before commit/push (N12/N17)**;
C-01/C-02 are comments requiring no correction for acceptance. This review is
not overall N17 approval of the repository documentation set, not the N20 PA
review, and not hosted-CI, T21, NC-EXT-43 or release/qualification closure.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | `aeronautics-space-sw-documentation-engineer` (independent context) | 2026-10-15 | APPROVED WITH COMMENTS — NC-01 tracked to closure; C-01/C-02 comments |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Minor | `CHANGELOG.md` new entry; `docs/status/loop-audit.md` 3.2; N16/N8 (F-01) | Change history omits the recurrence provenance (inline `mkdir -p` in both workflows at `57974d2`, dropped by `a4c436c` when the job moved to `make docs`) | Add the single bounded sentence described in F-01; no broader rework | Fresh-context R4 delta re-review of the added sentence(s); `git show` provenance re-checked | author (`aeronautics-space-fsw-engineer`) | **open** | pending |
| FU-02 | Comment | `docs/status/loop-audit.md` header (C-01) | Header revision/date lag the appended 3.2 row | Align header to 3.2 / 2026-10-15 or justify the lag | Inspection in the same commit | author | **open** | not required |
| FU-03 | Comment | Mechanism wording (C-02) | Internal `mkdir(2)` claim is an inference, only behaviour verified | Optional wording alignment | Inspection if changed | author | **open (optional)** | not required |
| FU-04 | Process | `docs/reviews/README.md`; N11/N17/N20 | The review log must receive rows for the reports produced in this cycle ("append a row to the log in the working tree immediately"); the log currently has no row for FRICK-REVIEW-0283 or FRICK-REVIEW-0284 | Author appends the log rows for FRICK-REVIEW-0283 (this report) and FRICK-REVIEW-0284 (PA delta report, already present in the working tree) before commit/push | `docs/reviews/README.md` log rows | author | **open** | n/a |

Note: the docs-set unique-id count of **246** in the reviewed records is the
count of the state reviewed, before the reports produced by this cycle were
stored. With this report (`FRICK-REVIEW-0283`) and the PA delta report already
present in the working tree (`FRICK-REVIEW-0284`) the count is **248**, with no
duplicate. The recorded 246 is therefore not an error (point-in-time value, like
the other cycle rows) and needs no correction to the 2026-10-15 entries.

---

Reviewer changed no repository file other than this report (N11: stored in the
working tree as produced; committed with the change only after the applicable
gates pass).
