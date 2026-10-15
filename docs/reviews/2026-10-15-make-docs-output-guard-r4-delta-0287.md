# FRICK-REVIEW-0287 — `make docs` output-existence guard (R4 delta re-review: closure of NC-01/NC-02 and NC-EXT-81)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-REVIEW-0287                      |
| Reviewer session                  | `aeronautics-space-sw-documentation-engineer` (fresh independent context) |
| Reviewer profile                  | R4 — Senior SW documentation reviewer (norm N17) |
| Reference revision                | working tree at HEAD `620fff8` + uncommitted corrective delta (11 modified paths, 2 untracked review reports) |
| Items under review                | Closure actions of FRICK-REVIEW-0285 NC-01/NC-02 and FRICK-REVIEW-0286 NC-EXT-81: `docs/status/loop-audit.md` (rev 3.3 — header:9, §1 scope-boundary row:46, N7:58, N10:61, N11:62, N12:63, N15:66, N19:70, History 3.3:404); `docs/status/handover.md` (rev 2.25→2.26, §4:91-103, 259, 263, 331, History 2.26:559); `CHANGELOG.md` (guard section, `[Unreleased]`); `docs/reviews/README.md` (log rows 0285/0286); `README.md` (status-panel row) |
| Date                              | 2026-10-15                             |
| Scope                             | Bounded **closing verification** of the three NCs raised by FRICK-REVIEW-0285/0286 only: whether loop-audit working-tree/commit/hosted-CI/NC-numbering statements are now mutually consistent and true; whether the hosted-CI citations are anchored and honestly limited; whether the §1/N7 rewrites are legitimate currency maintenance (append-only History) and consistent with `git log`; whether `handover.md` is internally consistent; whether the CHANGELOG restructure resolved C-01/C-02 and records the sync obligation; whether the review-log rows and file names are well-formed; and re-running the applicable local gates |
| Independence declaration          | Fresh session with no part in authoring the guard, the corrections or any prior review of them; every claim below was re-checked against the repository, `git` state and the public forge APIs by the reviewer. Throw-away artifacts only under `/tmp/opencode/r0287` |
| Norms applied                    | N2, N8, N11, N12, N16, N17, N19, N21, N22 (N1-N22 baseline, rules rev 2.8) |

## 1. Scope

This is the closing verification pass required by FRICK-REVIEW-0285
("NC-01 and NC-02 must be corrected and the corrections verified by a
fresh-context R4 delta re-review before commit/push") and by FRICK-REVIEW-0286
(NC-EXT-81, "bounded consistency edit … bounded PA delta re-review required").
It verifies the corrective edits listed in §3 and nothing broader.

In scope: the three NC actions; the consistency and truth of the currency
statements in the two status documents; the anchoring/limits of the hosted-CI
citations; the append-only character of the revision histories; the CHANGELOG
change boundary and sync obligation; the review-log rows; and the local gates
the delta must still pass (documentation set, reviewer-persona drift, metrics
panel, whitespace).

Out of scope (and **not** decided here): the guard's own technical merit and
duplication judgement (FRICK-REVIEW-0286 F3/F4); PA/N20 conclusion; T21 or tool
qualification; NC-EXT-43 licence/provenance; NC-EXT-75/76 mission-authority
inputs; NC-EXT-35/36 image/hosted provenance; hosted-CI qualification credit;
any release, certification, qualification, airworthiness or mission judgement.
No gate closure, NC closure or risk acceptance is expressed or implied by this
report for the items above.

Baseline of this review is the corrective delta. This report is itself an
addition to the tree (new id `FRICK-REVIEW-0287`) and is not part of the
reviewed baseline (see the id-count note in §3).

## 2. References

- `docs/rules.md` N8/N11/N12/N16/N17/N19/N21; `AGENTS.md` §§1, 3, 5, 7;
  `docs/reviews/review-template.md`.
- Origin reviews: `docs/reviews/2026-10-15-make-docs-output-guard-r4-delta-0285.md`
  (FRICK-REVIEW-0285), `…-pa-delta-0286.md` (FRICK-REVIEW-0286),
  `docs/reviews/2026-10-15-ci-docs-doxygen-dir-fix-pa-delta-0284.md`
  (FRICK-REVIEW-0284 Comment 5 and NC-EXT-78/79/80).
- Commits `a4c436c` (373 files, broad licence/revamp/stress change),
  `03773f7` (8 files, CI `docs`-job Doxygen directory fix + reports 0283/0284),
  `620fff8` (3 files, metrics-panel regeneration; HEAD, both remotes).
- Forge evidence re-read by the reviewer: GitHub Actions API runs `38688558619`
  (#83), `38688045965` (#82), `38685091035` (#81); Gitea task API
  (`www.housefz.com/git/api/v1/repos/ilfrick/frick-fsw/actions/tasks`);
  `git ls-remote origin|gitea refs/heads/main`.

## 3. Independent verification performed (all reproduced by the reviewer)

| Check | Method | Result |
| ----- | ------ | ------- |
| NC-02 residual strings | `grep -n "354"`, `grep -n "234 "`, `grep -n "120 "` in `docs/status/loop-audit.md` | `354` = **0 hits** anywhere in the file; `234`/`120` occur **only** in the retained History row 2.9 (a dated snapshot of 2026-10-14, correctly left as history). The previous live contradictions (old §1 row and N7 row) no longer exist |
| NC-02 / NC-EXT-81 live rows | `sed -n '1,75p;395,405p' docs/status/loop-audit.md` | Header:9 = rev 3.3, baseline `620fff8…db8` "(HEAD, pushed to both remotes)". §1 row:46 is now an explicitly labelled "Superseded snapshot (2026-10-14)" that records the later `a4c436c` commit. N7:58 = both remotes at `620fff8`, commit/push under explicit delegation, unmanifested aggregate scope retained as an open finding. N11:62 = prior cycle committed, delta uncommitted. N19:70 = delta inventory against `620fff8`. No live statement asserts an uncommitted 354-path tree or unauthorized commit |
| NC numbering (NC-EXT-81 second half) | `grep -n "NC-EXT-" docs/status/loop-audit.md docs/status/handover.md` | N12:63 now reads `NC-S4b-01..09`, `NC-EXT-01..81`, `NC-EXT-48..81`, with the attribution "(78/79/80 from PA FRICK-REVIEW-0284; 81 from PA FRICK-REVIEW-0286)". `handover.md` NC ranges are widened to `..81` at lines 100, 259, 263, 331. No live `..77` ceiling remains in either document |
| Commit/push currency | `git rev-parse HEAD`, `git ls-remote origin gitea`, `git status --porcelain` | HEAD = `620fff8cdf17e5f972afef189f56973ca2505db8`; both remotes' `refs/heads/main` = the same hash — matches header:9, N7:58 and `handover.md`. `git status` = 11 modified paths + 3 untracked review reports (0285, 0286 and this report), i.e. the tree is *not* literally clean and the delta *is* uncommitted — as the rows state |
| §1/N7 factual accuracy | `git show --stat a4c436c 03773f7 620fff8`; `git log -1 --format=%B a4c436c` | `a4c436c` = 373 files, +24,548/−1,550 (matches "373 files"); `03773f7` = 8 files (docs job/Doxygen directory + reports 0283/0284); `620fff8` = 3 files (metrics-panel regeneration). The `a4c436c` message states "The repository owner / systems engineer directed this commit and push" and lists the open gates — consistent with §1's "under the user's explicit authorization" and N7's "under the user's explicit delegation" |
| Hosted-CI anchor (N15:66) | GitHub Actions API, 2026-10-15 | Run **#83** = `620fff8cdf…db8`, id `38688558619`, created 2026-10-15T05:45:57Z, conclusion **success**, **8/8** jobs success including `documentation set` and `pipeline summary`; https://github.com/ilfrick/frick-fsw/actions/runs/38688558619. Run **#82** = `03773f7d4c…9b20`, id `38688045965`, failure **only** in `build (host Debug + Release) / unit tests`; run **#81** = `a4c436ceb6…0f82`, id `38685091035`, failure **only** in `documentation set` (summary skipped). Every element of the N15 sentence (forge, run number, commit hash, date, job set/result, #81/#82 retained failures) is therefore accurate |
| Hosted-CI honesty limit | GitHub API + Gitea task API + repository grep | "hosted CI has **not** yet run for this guard delta" is correct: no run exists for the uncommitted delta; the latest run is #83 at `620fff8`. "The Gitea run is not evidenced" matches the repository record (no Gitea run evidence is retained; NC-EXT-36 open) — with the precision note in RC-02 below |
| In-repo retention of #81/#82/#83 | `grep -rn "#81" "#82" "#83" "actions/runs" docs/ CHANGELOG.md` | The repo retains #82 in `docs/vv/verification-results.md` (2.8 row) and `CHANGELOG.md`; #83 and #81 are now described in `loop-audit.md` N15 and `handover.md` §4. The #83 run **URL/run id** is not retained anywhere (RC-01) |
| Retroactive-rewrite test (N11/N16) | `git diff docs/status/loop-audit.md docs/status/handover.md` | Body rows were edited and History rows **3.3** and **2.26** were **appended** describing exactly those edits; no pre-existing History row was modified or removed. The prior rows 3.2/3.1/… and 2.25/2.24/… are intact. This is currency maintenance, not a silent rewrite |
| `handover.md` internal consistency | `sed -n '1,16p;88,106p;555,560p' docs/status/handover.md` | Rev 2.26 / date 2026-10-15 / History row 2.26 present and consistent; the §4 "Update (2026-10-15)" block records the three commits, green #83, #81/#82 retention, the unevidenced Gitea run, the unchanged open gates and the guard delta with the 0285/0286 outcomes. One wording tension remains between the §4 heading and its own first bullet (RC-03) |
| CHANGELOG restructure | `sed -n '14,72p' CHANGELOG.md` | The guard now has its **own** level-3 section ("`make docs` output-existence guard (2026-10-15)"), separate from the docs-directory-fix section whose N21 sentence now applies to that fix alone (resolves C-02). The overstatement "independently of the target's own check" is gone, replaced by "defence in depth: the same existence check, downstream of the target's own" (resolves C-01). A bullet records the **sync obligation** (three assertion sites, derived from `Doxyfile` `OUTPUT_DIRECTORY` + `GENERATE_HTML = YES`; mismatch fails closed) (addresses C-03). The guard is stated to be existence-only and to close no hosted/CI/T21/NC-EXT-43/release gate |
| Review-log rows | `sed -n '1,5p;277,284p' docs/reviews/README.md`; `ls docs/reviews/` | Both new rows have the 5-column log shape (Date / Scope / Profile / Conclusion / Report) and cite `2026-10-15-make-docs-output-guard-r4-delta-0285.md` and `…-pa-delta-0286.md`; both files exist with those exact names. No malformed or duplicated row |
| Local gates re-run | CI `docs` job python extracted verbatim from `.github/workflows/ci.yml` and executed (before and after this report was stored) | `documentation set: OK (250 unique document ids)` on the corrective delta, `documentation set: OK (251 unique document ids)` with this report stored (no duplicates); `reviewer-agent personas: bodies in sync` (both exit 0) |
| Other recorded gates | — | `python3 tools/scripts/metrics.py --check` → `metrics panel: current` (exit 0); the README row `32/80 (40.0%)` is arithmetically consistent (32/80 = 40.0%); `git diff --check` → clean |
| N21/N22 boundedness | `git diff -U0` review of every added line | No added line claims release, certification, qualification, WCET, mission, hosted-CI or NC closure; every gate statement is explicitly negative ("does not close …", "remain open"). The only NC-status statement is that NC-EXT-78..81 remain open |

**Confidence and limits.** Confidence **High** for the local, git- and
forge-verifiable claims above (direct primary evidence, reproduced twice:
git/API/re-run gates). Confidence **Medium** for the Gitea mirror state: the
forge's public *task* listing is reachable and shows run #83 on `620fff8` with
all 8 tasks `success`, but the Gitea `actions/runs` endpoint requires a token
(HTTP 401) and no Gitea job log/version output is retained, so no Gitea
*qualification* evidence exists; the record's deliberately negative statement is
therefore conservative, not wrong (RC-02). No claim in this report goes beyond
what is listed in this table.

## 4. Findings

**No Major and no Minor finding.** The three previously raised non-conformances
are verified closed on their substance (see §5 and §7); the three residual
Comments below are wording/completeness notes that require **no correction for
acceptance and no further review round** (N21).

| # | Severity | Item / Norm | Finding | Evidence | Required action |
| - | -------- | ----------- | ------- | -------- | --------------- |
| RC-01 | Comment | `docs/status/loop-audit.md` N15:66 vs `docs/vv/verification-results.md` (2.9 row) — N11 | FRICK-REVIEW-0285 NC-01 asked for the run identity "**and URL**" to be recorded, preferably in the N11 evidence record. The correction anchors the claim in the audit record with forge + run number + full commit hash + date + job set/result (retrievable, so the F-01 defect is cured), but the **run id/URL is not retained** and the `docs/vv/verification-results.md` 2.9 cycle row still records no hosted run for the guard cycle | N15:66 text; forge URL `…/actions/runs/38688558619`; `docs/vv/verification-results.md:25` | None now. If the record is next touched, add the run URL — ideally as the bounded hosted-CI evidence line in the 2.9 row — so the N11 evidence record carries the same anchor as the audit. Non-blocking |
| RC-02 | Comment | `docs/status/loop-audit.md` N15:66 — hosted-CI precision | "The Gitea run is not evidenced" is correct as to **retained** repository evidence (which is what NC-EXT-36 requires: both environments' execution, version and output evidence), but it is wider than the facts: the Gitea mirror's public task listing **is** reachable and shows run #83 on `620fff8` with all 8 tasks (including `documentation set` and `pipeline summary`) in state `success`. Read literally, the sentence could be taken to mean no Gitea run evidence exists at all | Gitea task API listing (public, 2026-10-15): run 83 / `620fff8` / all tasks success / `…/actions/runs/2805`; `actions/runs` endpoint = 401 token required; no Gitea log retained; NC-EXT-36 open (`docs/planning/sprint-plan.md:149`) | None now (the statement under-claims credit, so it is safe). Recommend, when NC-EXT-36 is dispositioned, rewording to "no Gitea run evidence is retained in the repository record" and capturing the retrievable Gitea task metadata. Non-blocking |
| RC-03 | Comment | `docs/status/loop-audit.md` §1:46 / N19:70; `docs/status/handover.md` §4 heading:91 — N8/N19 | The adjective "clean" still appears next to the uncommitted-delta clause ("the worktree is now clean at `620fff8` **and** this audit's delta … is the only uncommitted change"; "The worktree is clean at `620fff8`; the guard delta is uncommitted"; heading "baseline `620fff8`, clean worktree"). FRICK-REVIEW-0286 F1 specifically flagged the word as inaccurate at audit time, and `git status` still shows uncommitted paths. The clauses that follow state the true position, so no reader is misled, but the phrasing remains loose | `git status --porcelain`; `loop-audit.md:46,70`; `handover.md:91` vs its own first bullet `handover.md:100-103` | None now. If the two files are next touched, prefer "the aggregate change is committed; only this delta is uncommitted". Non-blocking |

No malformed table, duplicate document id, broken link, unretained overwrite or
selective deletion was found in the corrective delta. The retained 2026-10-14
narrative in `handover.md` still contains present-tense "hosted CI … remains
unrun" sentences (lines ~327, ~342) but is explicitly prefixed "The narrative
below is retained as the 2026-10-14 snapshot", and the current state is stated
first in the same section; this is consistent with the document's
snapshot-plus-update style and requires no action (see RC-03 for the §4 heading
wording).

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — the FRICK-REVIEW-0285/0286 NC actions are verified closed; three residual non-blocking Comments (RC-01–RC-03)
- [ ] REJECTED — one or more Major findings

**Closing verification result.** The corrective edits are adequate and
proportionate. With the evidence in §3:

- **FRICK-REVIEW-0285 NC-01 (F-01) — CLOSED.** The hosted-CI claim is now
  anchored in the retained audit record (forge, run number, full commit hash,
  date, job set/result), the #81/#82 failures are recorded, the Gitea run is
  explicitly not claimed, and the current delta is explicitly stated not to have
  a hosted run. Residual completeness note RC-01 (no run URL; N11 evidence row
  unchanged) — non-blocking.
- **FRICK-REVIEW-0285 NC-02 (F-02) — CLOSED.** The `354`-path and
  `234 + 120`-path live statements are gone from the live rows; §1 is
  date-stamped as a superseded 2026-10-14 snapshot recording the subsequent
  `a4c436c` commit; N7 now states the true commit/push position with the
  unmanifested aggregate scope retained as an open finding. The correction is
  append-only in the History table (rev 3.3) and consistent with `git log`/
  `git rev-parse`. Residual wording note RC-03 — non-blocking.
- **FRICK-REVIEW-0286 NC-EXT-81 — CLOSED.** The audit's currency statements are
  now mutually consistent, N12 enumerates `NC-EXT-01..81` with correct
  attribution of 78/79/80 to FRICK-REVIEW-0284 and 81 to FRICK-REVIEW-0286, and
  NC-EXT-79 is recorded as the open `ci:docs` register mapping (N10 row).
  Residual precision note RC-02 (Gitea wording) — non-blocking.
- PA FRICK-REVIEW-0286 Comment F2's actionable half — the "hosted CI has not yet
  run for this delta" clause — is present; its URL request is captured in RC-01.

The CHANGELOG restructure resolves C-01 and C-02, records the three-site sync
obligation (C-03), and contains no release/qualification/NC-closure
over-claim; the review-log rows are well-formed and match the stored files; the
applicable local gates (`documentation set`, persona drift, metrics panel,
`git diff --check`) pass on the corrected tree.

**This report closes no gate.** It is not overall N17 approval of the repository
documentation set, not the N20 PA conclusion, not hosted-CI evidence or T21
qualification, not NC-EXT-36/43/75/76/78/79/80 closure, and not a release,
certification or qualification approval. RC-01–RC-03 require no further review
round; the change may proceed to the owner's normal commit/push gate (N7), which
remains the designated agent's decision.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | `aeronautics-space-sw-documentation-engineer` (independent context) | 2026-10-15 | APPROVED WITH COMMENTS — FRICK-REVIEW-0285 NC-01/NC-02 and FRICK-REVIEW-0286 NC-EXT-81 verified **closed**; RC-01–RC-03 non-blocking Comments; no further review round required |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 (0285) | Minor | `docs/status/loop-audit.md` 3.3 N15:66 | Hosted-CI-green claim had no retained evidence anchor | Corrected: N15 now cites GitHub Actions run #83 on commit `620fff8…db8` (2026-10-15), all jobs incl. the pipeline summary, retains #81/#82, states the Gitea run is unevidenced and that hosted CI has not run for this delta | `loop-audit.md:66`; GitHub API run `38688558619` (success, 8/8); `git rev-parse HEAD` | author (`aeronautics-space-fsw-engineer`) | **closed** (RC-01 residual: run URL / N11 record placement — non-blocking) | this report |
| NC-02 (0285) | Minor | `docs/status/loop-audit.md` §1:46, N7:58 | Live scope statements contradicted the corrected header/N11/N19 | Corrected: §1 is a date-stamped superseded snapshot; N7 states both remotes at `620fff8` with delegation and the open unmanifested-scope finding; History 3.3 appended | `grep 354/234/120` (live rows clean; History 2.9 retained); `git ls-remote`; `git show --stat a4c436c` | author | **closed** (RC-03 residual wording: "clean" — non-blocking) | this report |
| NC-EXT-81 (0286) | Minor | `docs/status/loop-audit.md` N12:63 + currency | Partial, self-contradictory currency correction; N12 ceiling at `NC-EXT-77` | Corrected: N12 = `NC-EXT-01..81` with 78/79/80 attributed to FRICK-REVIEW-0284 and 81 to FRICK-REVIEW-0286; NC-EXT-79 recorded as the open `ci:docs` register mapping in the N10 row; `handover.md` ranges widened to `..81` | `loop-audit.md:61,63`; `handover.md:100,259,263,331`; `docs/reviews/README.md:282-283` | author | **closed** (RC-02 residual: Gitea wording precision — non-blocking) | this report |
| FU-08 | Process | `docs/reviews/README.md`; N11/N17 | The review log must receive a row for this report ("append a row to the log in the working tree immediately"); the log currently ends with the FRICK-REVIEW-0286 row | Author appends the FRICK-REVIEW-0287 row before commit/push | `docs/reviews/README.md` log row | author | **open** | n/a |
| RC-01 | Comment | `loop-audit.md` N15:66 / `verification-results.md` 2.9 (N11) | Run id/URL not retained; N11 evidence row carries no hosted run for this cycle | Optional, at the next record touch | run citation if added | author | **open (optional, non-blocking)** | not required |
| RC-02 | Comment | `loop-audit.md` N15:66 (hosted-CI precision) | "Gitea run is not evidenced" is correct for retained evidence but wider than the reachable forge metadata | Optional rewording when NC-EXT-36 is dispositioned | Gitea task API listing | author | **open (optional, non-blocking)** | not required |
| RC-03 | Comment | `loop-audit.md` §1:46 / N19:70; `handover.md` §4:91 (N8/N19) | "clean" retained next to the uncommitted-delta clause | Optional rewording at the next touch | `git status` | author | **open (optional, non-blocking)** | not required |

Note: the recorded documentation-set unique-id count of **248** (2.8 row) and
**250** (the count reproduced by this reviewer on the corrective delta, before
this report) are point-in-time values of the states reviewed. With this report
(`FRICK-REVIEW-0287`) stored, the count becomes **251** (re-checked, no
duplicate); no recorded value needs correction.

Independence note: the deltas reviewed here are the corrective edits to
`docs/status/loop-audit.md`, `docs/status/handover.md`, `CHANGELOG.md`,
`docs/reviews/README.md` and `README.md`; the earlier guard-delta reports
FRICK-REVIEW-0285/0286 and the guard implementation itself are outside this
report's scope and their conclusions are not re-assessed or endorsed here.

---

Reviewer changed no repository file other than this report (N11: stored in the
working tree as produced; committed with the change only after the applicable
gates pass).
