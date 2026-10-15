# FRICK-REVIEW-0286 — `make docs` output-existence guard + workflow `docs`-job assertions (PA delta)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0286                      |
| Reviewer session                  | `aeronautics-space-sw-pa-engineer` (independent agent, norm N9) |
| Reviewer profile                  | PA — Senior SW Product Assurance engineer (norm N20) |
| Reference revision                | working tree at HEAD `620fff8` (pushed to both remotes) + 8-file uncommitted delta |
| Items under review                | `Makefile` (`docs` target guard), `.github/workflows/ci.yml` (`docs` job), `.gitea/workflows/ci.yml` (`docs` job), `CHANGELOG.md`, `docs/tooling.md`, `docs/vv/verification-results.md` (rev 2.9), `docs/vv/tool-qualification.md` (rev 0.3.73), `docs/status/loop-audit.md` (rev 3.3) |
| Date                              | 2026-10-15                             |
| Scope                             | Bounded PA delta review: disposition of FRICK-REVIEW-0284 Comment 5; fail-closed analysis of the new assertion (`test -s` semantics, shell/quoting, `make docs` success without HTML); duplication/drift hazard of the three-place assertion; legitimacy of the loop-audit N11/N15/N19/baseline rewrites under N11; accuracy of the pushed/hosted-CI claims; evidence sufficiency and honesty of the stated limits; N10 register impact. Against ECSS-Q-ST-80C (§PA/V&V records, reuse of evidence) and ECSS-E-ST-40C (lifecycle data), with norms N6/N8/N10/N11/N12/N15/N16/N19/N20/N21. |
| Independence declaration          | Fresh independent context; the reviewer had no part in authoring, applying, testing or recording this delta and did not review its own work. Norms N1-N22 (`docs/rules.md` rev 2.8). |

## 1. Scope

In scope: the eight modified files listed above and the bounded hardening they
record — adding `test -s build/doxygen/html/index.html` after `doxygen Doxyfile`
in the `docs` target, the artifact assertion plus `doxygen --version` echo in the
two `docs` jobs, and the record/currency updates. Out of scope: the prior
`mkdir -p build/doxygen` fix (already reviewed as FRICK-REVIEW-0283/0284);
hosted-CI execution **for this delta**; T21 qualification evidence; NC-EXT-43
licence/provenance; NC-EXT-75/76 mission-authority inputs; image provenance
(NC-EXT-35/36); release or qualification decisions. No approval, certification,
NC closure or risk acceptance is implied by this report.

Assumptions: the delta is exactly the eight paths shown by `git status` (no new
document id; `docs/requirements/traceability.yaml`, `src/`, `config/`, `tests/`,
`verification/` untouched — verified); the guard asserts artifact **existence
only**, not HTML content or the warning set; GitHub Actions `run` steps execute
with `bash -e {0}`. A sibling documentation review of the same delta
(`2026-10-15-make-docs-output-guard-r4-delta-0285.md`, FRICK-REVIEW-0285, N17/R4
stream) is present in the working tree; it is a separate independent stream,
outside this report's scope and not relied upon here.

**Independent verification performed by the reviewer** (no repository file
modified; throw-away copies under `/tmp/opencode/r0286` built with
`git archive HEAD` + the working-tree `Makefile`, exercised in `frick-vv:local`):

- **Positive, pristine tree (no `build/`)**: `make docs` exit **0**;
  `build/doxygen/html/index.html` present, **57,394 B**; `doxygen --version` =
  **1.9.8**; `/bin/sh -> dash` in the image. Matches the recorded evidence
  exactly.
- **Negative — the PA scenario (silent no-op)**: same tree with
  `GENERATE_HTML = NO`; `doxygen Doxyfile` exits **0** and prints
  `warning: No output formats selected!`, no HTML is produced; the guard then
  prints `error: 'make docs' produced no build/doxygen/html/index.html (silent
  doxygen failure)` and `make` aborts with **`Makefile:159: docs` Error 1 /
  status 2**. A silent generation regression can no longer pass.
- **Guard-only**: deleting `index.html` from a successful tree makes the guard
  exit 1 (recipe shell), i.e. `make docs` fails on the next run. The target's
  `rm -rf build/doxygen` precedes generation, so a stale artifact cannot satisfy
  the guard.
- **`test -s` semantics under `dash`**: missing → 1; zero-byte file → 1;
  non-empty file → 0; a *directory* named `index.html` → 0 (theoretical only;
  Doxygen never creates that).
- **Shell/quoting**: make's default `SHELL` is `/bin/sh` (`dash` here and in the
  image); the recipe line is a single tab-indented line using only POSIX
  constructs (`test`, `||`, `{ …; }`, `exit 1`, double-quoted message containing
  single quotes). No expansion, globbing or `bash`-only syntax is involved; the
  measured behaviour confirms it runs under `dash`.
- **Duplication**: `doxygen --version` / `make docs` / `test -s
  build/doxygen/html/index.html` present in both `docs` jobs
  (`.github/workflows/ci.yml:243-248`, `.gitea/workflows/ci.yml:282-287`);
  both workflow files **parse as valid YAML** (PyYAML inside `frick-vv:local`),
  job sets unchanged (8 jobs each).
- **Hosted-CI claims re-verified at the issuer** (GitHub Actions API, public
  repository, 2026-10-15): run **#83** = `620fff8`, id `38688558619`,
  conclusion **success**, all 8 jobs success including `pipeline summary`; run
  **#82** = `03773f7`, id `38688045965`, `documentation set` success /
  `build (host Debug + Release) / unit tests` **failure**; run **#81** =
  `a4c436c`, id `38685091035`, **`documentation set` failure only**. `git
  ls-remote` returns `620fff8` for `main` on **both** `origin` and `gitea`.
  The loop-audit N11/N15 claims are therefore **true as written** (see F2 for
  the evidence-identity gap).
- **Recorded local gates re-run on the reviewed tree**: documentation-set
  id-uniqueness **248 unique ids, no duplicates**; reviewer-persona drift
  *bodies in sync*; `tools/scripts/metrics.py --check` → *metrics panel:
  current*; `make gsw-selfcheck` → 5 GSW + 9 metrics + 14 licence tests OK and
  `license_check.py` OK; `git diff --check` clean. All match the recorded
  evidence.

## 2. References

- Project norms: `docs/rules.md` (N1-N22); `AGENTS.md`; `docs/quality/product-assurance.md`
- Prior reviews: `docs/reviews/2026-10-15-ci-docs-doxygen-dir-fix-pa-delta-0284.md` (FRICK-REVIEW-0284, Comment 5 and NC-EXT-78/79/80), `…-r4-delta-0283.md`; review log `docs/reviews/README.md:280-281`
- Traceability manifest: `docs/requirements/traceability.yaml` (22 `ci:docs` evidence links); V&V records `docs/vv/**`; `docs/reviews/review-template.md`
- ECSS-E-ST-40C, ECSS-Q-ST-80C (tool qualification, V&V records/data)
- Evidence: `Makefile:155-159`; `.github/workflows/ci.yml:239-248`; `.gitea/workflows/ci.yml:280-287`; `Doxyfile:9,19-20`; `CHANGELOG.md:36-49`; `docs/tooling.md:50,105`; `docs/vv/verification-results.md:25,646`; `docs/vv/tool-qualification.md:185`; `docs/status/loop-audit.md:9,46,58,62-63,66,70,404`

## 3. Findings

| # | Severity | Location / Item | Finding | Evidence | Required action |
| - | -------- | --------------- | ------- | -------- | --------------- |
| 1 | **Minor** | `docs/status/loop-audit.md` header:9, §2 N7:58, N11:62, N12:63, N15:66, N19:70, §1 row:46 (N8/N16/N19 currency) | The currency correction is **partial and leaves the record self-contradictory**: the new header and N19 state that the audit covers "the current uncommitted delta (the `make docs` output-existence guard and its records)" and that "the worktree is clean at `620fff8`", while §1 *Working-tree scope boundary* still asserts "234 tracked modified paths (4,480 insertions/1,541 deletions) and **120 untracked paths**" and N7 still asserts "current **354-path** working tree is uncommitted"; N12 still enumerates NCs only to `NC-EXT-77` although NC-EXT-78/79/80 were recorded on 2026-10-15. The word "clean" is also wrong at audit time (8 modified paths). | `git status --porcelain` (before this report was stored) → exactly 8 modified paths, 0 untracked; `git rev-parse HEAD` = `620fff8`; `docs/status/loop-audit.md:46,58,63`; `docs/reviews/README.md:281`; FRICK-REVIEW-0284 §6. | One bounded edit (no functional rework): make the scope statements mutually consistent — e.g. state that the aggregate scope was committed at `620fff8` and the current delta is 8 paths, or qualify the 354-path statement "as of rev 3.2"; extend N12 to `NC-EXT-78..81` or point to `docs/reviews/README.md` as the NC ledger. Tracked as **NC-EXT-81**. |
| 2 | Comment | `docs/status/loop-audit.md:66` (N15), and the same pattern in `docs/vv/verification-results.md:26` (pre-existing) | Hosted-CI evidence is cited by **run number only** — no forge, workflow, run id/URL or observation date — and the N15 phrasing ("Hosted CI has now run: run #83 … is fully green") does not state that hosted CI has **not** run for the current uncommitted delta. The claim itself is verified true on GitHub Actions (API check above), but as recorded it is not independently retrievable from the record and the Gitea run remains unevidenced (NC-EXT-36). "#83" is also ambiguous between the two forges, whose run numbering is independent. | GitHub Actions API runs `38688558619` (#83, success, 8/8 jobs), `38688045965` (#82), `38685091035` (#81); `docs/status/loop-audit.md:62,66`. | At the next bounded record touch, cite the run as "GitHub Actions run #83, `<run url>` (2026-10-15)" and add one clause that the current delta has not yet run in hosted CI. No action otherwise (N21). |
| 3 | Comment | `Makefile:159` + `.github/workflows/ci.yml:244-248` + `.gitea/workflows/ci.yml:283-287` (duplication / single source of truth) | Judgement requested: the three-place assertion is **acceptable defence in depth, not an unacceptable drift hazard**. The primary control sits in the shared `docs` target, so host and both forge callers are covered even if a workflow line is later dropped or drifts; a stale duplicate **fails closed** (loud job failure), and a changed artifact path breaks the gate visibly rather than silently. Residual risks: (a) the path `build/doxygen/html/index.html` now appears in 3 files with no shared variable → 3 update sites; (b) the workflow-level line adds *independent* strength only because GitHub Actions `run` steps abort on the first failing command (`bash -e {0}`) — under a non-aborting shell, a `make docs` that failed *after* emitting `index.html` could in principle be masked by the trailing `test`, whereas the in-target guard cannot be masked (verified). | Reviewer reproduction (negative and guard-only cases); workflow text inspected; YAML parse OK. | None required now (N21). If the workflows are next touched, `make docs && test -s build/doxygen/html/index.html` (or a one-line comment on the shell assumption) removes residual (b). |
| 4 | Comment | `Makefile:159` guard semantics; record wording in `CHANGELOG.md:37-39`, `docs/vv/verification-results.md:25` | The guard is **existence-only**: it cannot detect truncated/hollow HTML, a wrong-content `index.html`, or a changed warning set (0284 marked the warning-set assertion optional; correctly not added). Verified: missing → fail, zero-byte → fail, any non-empty file → pass; a directory named `index.html` would also satisfy `test -s`. Stale-artifact masking is nevertheless prevented because `rm -rf build/doxygen` precedes generation. The records' wording ("output-existence assertion", "silent doxygen failure") does **not** over-claim content validation. | Reviewer `test -s` matrix under `dash`; `Makefile:155-157`; `Doxyfile:9,19-20`. | None; if the guard is documented again, one clause ("existence only, not content") closes the residual reader risk. |

**Severity basis:** no finding contradicts the requirement at risk and no defect,
safety, security or evidence-integrity issue was found in the change itself. The
change is minimal, configuration-controlled and strictly strengthens an existing
CI/evidence path. F1 is a record-currency non-conformity (N8/N16/N19) tracked as
NC-EXT-81; F2-F4 are observations with no required action.

### 3.1 Disposition of FRICK-REVIEW-0284 Comment 5 (fail-closed analysis)

**Dispositioned in the working tree — yes, and stronger than requested.** The
comment asked for an output-existence assertion in the CI `docs` job and
optionally a `doxygen --version` echo. Delivered: (i) the assertion is placed in
the shared `docs` target, so it covers the host, GitHub **and** Gitea callers and
supersedes the weaker per-job-only form; (ii) both `docs` jobs additionally run
the same assertion and echo the Doxygen version. Fail-closed analysis, verified
empirically rather than by reading:

- `make docs` **succeeds but HTML is absent** → guard fails, `make docs` exits 2
  with an explicit message (the exact scenario the comment described).
- make aborts on any non-zero recipe line, and `exit 1` inside the `{ …; }`
  group exits the recipe shell, so the guard cannot be routed around by later
  commands; a failing `doxygen` (e.g. missing output directory) aborts earlier
  and was reproduced in FRICK-REVIEW-0284.
- POSIX-only syntax, correct quoting, `dash`-compatible; no shell/quoting defect.
- The guard is post-generation and post-`rm -rf`, so it cannot be satisfied by a
  previously generated tree.

Not verifiable by the reviewer (and **not claimed** in any record): execution of
the modified workflow steps in hosted CI for this delta, since the delta is
uncommitted. Closure of Comment 5 as a record item remains with the owner and
requires the next pushed run (F2).

### 3.2 Loop-audit rewrites: currency correction vs. retroactive rewrite

Updating the N11/N15/N19 status rows and the baseline-commit line to the current
state is **legitimate currency maintenance**, not an unjustified retroactive
rewrite: `loop-audit.md` is by design a *current-state* audit (N19) whose
revision trail is preserved append-only in its History table, and rev 3.3 adds a
History row describing exactly these edits, while the prior revision rows (3.2,
3.1, …) remain intact. The rewritten statements were stale, and the new
pushed/hosted claims were independently verified true (both remotes at
`620fff8`; GitHub run #83 green; #82/#81 consistent with the records). The
defect is therefore not the rewrite but its **partial** execution, which leaves
`loop-audit.md` internally inconsistent in the same revision (F1). N11 is not
violated: no retained evidence was deleted, and the delta is correctly recorded
as *uncommitted* at audit time.

### 3.3 N10 / tool-qualification impact

The delta records the change in the register History (`tool-qualification.md:185`,
rev 0.3.73) and explicitly claims **no N10 qualification or release credit** —
accurate and appropriately bounded. No declared tool status is upgraded: the
`docs` target / `ci:docs` evidence path (22 traceability links) is still neither
registered nor explicitly justified, so **NC-EXT-79 remains OPEN**; the register's
own rule ("any change to a qualified tool/script invalidates its qualification …
register updated in the same commit", `docs/vv/tool-qualification.md` §Qualification
procedure step 5) will require the bounded register update for T11/`ci:docs` at
the owner's release-scope sweep. The delta neither closes nor worsens this, and
does not claim otherwise; no new NC is opened for it because it is already
tracked. Status of the other 0284 NCs observed here: **NC-EXT-78** (loop-audit
header currency) appears satisfied by `03773f7` and is further advanced by this
delta (header now 3.3 / `620fff8`); **NC-EXT-80** (origin trace) is satisfied —
`CHANGELOG.md:21` records `7cb90a6` → `a4c436c`. Closure remains an
independent-review/owner decision.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Delta disposition: APPROVED WITH COMMENTS (PA PASS, bounded to this delta).**
The change is minimal (one recipe line, three workflow lines, records),
configuration-controlled, correctly located in the shared `docs` target, and
**genuinely fail-closed**: the silent-generation-regression scenario was
independently reproduced (Doxygen 1.9.8 exits 0 with `GENERATE_HTML = NO`, no
HTML; the guard fails `make docs` with exit 2 and an explicit message), and the
`test -s` semantics, shell (`/bin/sh` → `dash`), quoting and stale-artifact
handling were checked. All recorded local evidence reproduced exactly, both
workflows parse, and the pushed/hosted-CI claims are true at the issuer. One
Minor record-currency non-conformity (F1, **NC-EXT-81**) and three Comments
(F2-F4) are recorded; none blocks continuing development (N21).

Open gates correctly remain open in all eight records: hosted-CI evidence **for
this delta** (uncommitted at review time), T21 qualification evidence,
NC-EXT-43 (licence/provenance), NC-EXT-75/76 (mission authority inputs),
image provenance (NC-EXT-35/36), NC-EXT-79, and the bounded delta re-review of
FRICK-REVIEW-0283/0284. This report is not hosted-CI evidence, tool
qualification, NC closure, release approval or risk acceptance.

Norms check: N6 (CI/Docker record, bounded), N8 (docs updated in the same delta),
N10 (register History row; NC-EXT-79 open, credit correctly not claimed),
N11 (delta retained in the working tree; reviewer created only this report),
N12 (NC-EXT-81 recorded), N15 (full applicable cycle: only the guard path is
affected; hosted CI not yet run for this delta — stated), N16 (History rows 2.9,
0.3.73, 3.3 present), N19 (F1 currency), N20 (this review), N21 (bounded
follow-up on a pre-authorised comment; no scope expansion into wording/rework).
No requirement, Lean or traceability change was asserted, and none was found
(`traceability.yaml` untouched).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20)      | `aeronautics-space-sw-pa-engineer` (independent context) | 2026-10-15 | APPROVED WITH COMMENTS — bounded to this delta; 1 Minor finding (NC-EXT-81) + 3 Comments; Comment 5 of FRICK-REVIEW-0284 dispositioned in the working tree |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-EXT-81 | Minor | `loop-audit.md` currency (N8/N16/N19) | F1: rev 3.3 header/N19 vs §1 354-path row, N7 and N12 (`NC-EXT-77` ceiling); "worktree is clean" inaccurate | bounded consistency edit as in F1 | diff of header/§1/N7/N12 vs `git status` | frick-fsw (owner of the doc) | open | bounded PA delta re-review required |
| — | Comment | Hosted-CI record identity (N11/N15) | F2 | add run URL/forge and the "not yet run for this delta" clause at the next record touch | run citation | frick-fsw | open (non-blocking) | not required |
| — | Comment | Duplication / drift of the artifact assertion (N6/N15) | F3 | none now (N21); optional `make docs && test -s …` if the workflows are next touched | workflow diff | frick-fsw | open (non-blocking) | not required |
| — | Comment | Guard detection limits (record wording) | F4 | none; optional "existence only" clause | record text | frick-fsw | open (non-blocking) | not required |
| NC-EXT-78 | Minor (prior) | `loop-audit.md` header currency | from FRICK-REVIEW-0284 | satisfied by `03773f7`, advanced by this delta (rev 3.3 / `620fff8`) | `loop-audit.md:7,9` | frick-fsw | open pending owner delta re-review | included in this report |
| NC-EXT-79 | Minor (prior) | `tool-qualification.md` register / `ci:docs` path (N10) | from FRICK-REVIEW-0284 | **not addressed by this delta** (History row only, no register/mapping or explicit scope justification) | `tool-qualification.md:83,185` | frick-fsw (tool owner; independent R2 at release) | **open** (N21: deferrable) | release-scope register update |
| NC-EXT-80 | Minor (prior) | `CHANGELOG.md` origin trace (N16) | from FRICK-REVIEW-0284 | satisfied — `CHANGELOG.md:21` records `7cb90a6` → `a4c436c` | `CHANGELOG.md:21-25` | frick-fsw | open pending owner delta re-review | included in this report |

**Notices for the change owner** (outside this reviewer's authority, N7/N20):
this report has been created in the working tree only; it is **not logged** in
`docs/reviews/README.md` and is not committed, because the reviewer is not the
designated commit agent. It must be appended to the review log (N20/N11) with
the cycle evidence. Consequence for the records: the documentation-set unique-id
count becomes **250** once this report and the sibling R4 report
(FRICK-REVIEW-0285) are both stored (it was independently re-verified as 248
before either report existed), so the committed `248` statements remain valid
point-in-time snapshots and need no correction — consistent with the
disposition of FRICK-REVIEW-0284 F4. The applicability judgement, NC dispositions, closure of
Comment 5 and any release decision remain with the authorized owner.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.1 | 2026-10-15 | frick-fsw | Bounded independent PA delta review of the `make docs` output-existence guard and workflow `docs`-job assertions; Comment 5 of FRICK-REVIEW-0284 dispositioned, NC-EXT-81 opened, three Comments recorded; no qualification, hosted-CI, NC-closure or release credit. |

---

Reviewer changed no repository file other than this report (N11: stored in the
working tree as produced; committed with the change only after the applicable
gates pass).
