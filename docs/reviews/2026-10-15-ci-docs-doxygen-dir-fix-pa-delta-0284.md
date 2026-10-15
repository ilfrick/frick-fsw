# FRICK-REVIEW-0284 — CI `docs`-job Doxygen output-directory fix (PA delta)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0284                      |
| Reviewer session                  | `aeronautics-space-sw-pa-engineer` (independent agent, norm N9) |
| Reviewer profile                  | PA — Senior SW Product Assurance engineer (norm N20) |
| Reference revision                | working tree at HEAD `a4c436c` + docs delta |
| Items under review                | `Makefile` (`docs` target), `CHANGELOG.md`, `docs/vv/verification-results.md` (rev 2.8), `docs/status/loop-audit.md` (rev 3.2) |
| Date                              | 2026-10-15                             |
| Scope                             | Bounded delta review: correctness, minimality and configuration control of the `docs` target change; adequacy, bounding and reproducibility of the recorded evidence; N21 classification of the defect; status of the open gates. Against ECSS-Q-ST-80C (§PA/V&V records, reuse of evidence), ECSS-E-ST-40C (lifecycle data) and norms N6/N8/N10/N11/N12/N15/N16/N19/N20/N21. |
| Independence declaration          | Fresh independent context; the reviewer had no part in authoring, applying or recording this change and did not review its own work. Norms N1-N22 (rules rev 2.8). |

## 1. Scope

In scope: the four modified files listed above and the change they record
(add `mkdir -p build/doxygen` to the `docs` target so that `make docs` works on
a fresh checkout without `build/`). Out of scope: the remaining material in the
uncommitted working tree (recorded as unreviewed aggregate scope in
`loop-audit.md` rev 3.1 §1 and §5); hosted CI execution; T21 qualification;
NC-EXT-43 external/generated licence boundaries; release or qualification
decisions. No approval, certification, NC closure or risk acceptance is implied
by this report.

Assumptions: the delta is exactly the four files shown by `git status`
(no new document ids, `docs/requirements/traceability.yaml` untouched);
`make docs` is the only consumer of `Doxyfile`.

**Independent verification performed by the reviewer** (no repository file
modified; the reviewed tree was exported read-only with `git archive HEAD` and
tested in throw-away copies under `/tmp/opencode`):

- Pre-fix failure reproduced on a pristine copy with no `build/` (`doxygen 1.9.8`,
  `GNU Make 4.3` in `frick-vv:local`): `error: tag OUTPUT_DIRECTORY: Output
  directory 'build/doxygen' does not exist and cannot be created`, doxygen exit
  status 1, `make` exit status 2 — matching the recorded failure.
- Post-fix, same pristine copy plus the working-tree `Makefile`: `make docs`
  exit 0, `build/doxygen/html/index.html` present (2,136 files under `html/`).
- Warning set unchanged: with the fix, the HEAD-tree run and the working-tree run
  produce the same 110 `warning:` lines; the only difference is one line-number
  shift (`verification-results.md:213` -> `:214`) caused by the added table row.
- No failure masking: `make` aborts on any non-zero recipe line, so `mkdir -p`
  cannot hide a doxygen failure. Verified adversarially — with `build` replaced
  by a regular file the target fails loudly (`mkdir: cannot create directory
  'build': Not a directory`, `make` exit 2). `mkdir -p` only creates the parent.
- GitHub-jobs version parity: `doxygen --version` in `frick-vv:local` = `1.9.8`;
  Ubuntu 24.04 (noble) ships `doxygen 1.9.8+ds-2ubuntu0.1`, which is what the
  GitHub `docs` job installs (`runs-on: ubuntu-24.04`, `apt-get install doxygen`).
  Consistent with the claim "Doxygen 1.9.8 in `frick-vv:local`, matching the
  GitHub `docs` job". The Gitea `docs` job runs `make docs` in the same image.
- Recorded CI checks re-run on the HEAD tree: unique document ids 246, no
  duplicates; reviewer-agent persona bodies in sync; `license_check.py` OK;
  `tests/python` self-checks `test_ff_gsw.py` 5, `test_metrics.py` 9,
  `test_license_check.py` 14 tests, all OK — i.e. the recorded check counts and
  results are reproducible.
- Regression origin traced with `git log -S'mkdir -p build/doxygen'`: the step was
  added inline in both workflow Doxygen steps by `7cb90a6` and was removed when
  those steps were consolidated to `make docs` by `a4c436c` (HEAD).

## 2. References

- Project norms: `docs/rules.md` (N1-N22); `AGENTS.md`; `docs/quality/product-assurance.md`
- Traceability manifest: `docs/requirements/traceability.yaml`; V&V records `docs/vv/**`; `docs/reviews/review-template.md`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C); CCSDS 133.0-B-2
- Evidence: `Makefile:155-158`; `.github/workflows/ci.yml:154-244`; `.gitea/workflows/ci.yml:187-281`; `Doxyfile:9`; `CHANGELOG.md` [Unreleased] entry; `docs/vv/verification-results.md` rev 2.8; `docs/status/loop-audit.md` rev 3.2

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `docs/status/loop-audit.md` — N8/N16/N19 currency | The History table records revision 3.2 (2026-10-15) while the document header still reads `Revision 3.1`, `Date 2026-10-14`, `Baseline commit 57974d2…`; the N19 audit header therefore does not describe this loop's baseline. | `docs/status/loop-audit.md:7-9` vs last row of the History table (line 403); the previous loop updated this header (`a4c436c`). | Update the header to rev 3.2 / 2026-10-15 / baseline `a4c436c` (working tree) in the same commit. Non-functional record correction. |
| 2 | Minor | `docs/vv/tool-qualification.md` — N10 register | The `docs` target is part of the configuration that produces `ci:docs` evidence (credited as test evidence by many SRS/SSS entries), yet the register has no entry or evidence-claim mapping for `ci:docs`/the `docs` target, and the change is not recorded there. The equivalent earlier docs-job fix was recorded in the register (rev 0.1.3, "Doxygen output dir created"). | `docs/vv/tool-qualification.md` register + evidence-mapping row; `docs/requirements/traceability.yaml` (`tests: ["ci:docs", …]`, e.g. lines 210, 433, 442, 487); `.github/workflows/ci.yml:154`, `.gitea/workflows/ci.yml:187`. | Add a bounded register entry/mapping for the `ci:docs` path (or an explicit justification that Doxygen generation is documentation-generation only and outside the register's scope) and state the re-qualification consequence of the target change. Non-functional; deferrable under N21. |
| 3 | Minor | `CHANGELOG.md` / verification record — N16 origin trace | The record states the Doxygen 1.9.8 behaviour (correct) but not why the defect appeared now: `mkdir -p build/doxygen` existed inline in both workflow Doxygen steps from `7cb90a6` and was dropped in `a4c436c` (HEAD) when they were replaced by `make docs`. Without this, the history shows the same defect fixed twice with no auditable origin. | `git log -S'mkdir -p build/doxygen'` -> `7cb90a6` (added inline) and `a4c436c` (removed, `run: make docs`); `CHANGELOG.md` [Unreleased] entry. | Add one sentence recording the regression origin (commit ids) to the CHANGELOG entry and/or the verification-results row. Non-functional. |
| 4 | Comment | Evidence-record precision (N11) | (a) The recorded failure exit status "exit 1" is doxygen's; `make docs` returns 2 (verified). (b) "246 unique document ids" is a working-tree snapshot: adding this cycle's review reports (this one included) makes it 247, so the committed tree will not reproduce 246. (c) `AGENTS.md` §3 lists `make docs` as a separate command; the "fresh clone" condition is stated there for `make ci-local`, not literally for `make docs`. | Reviewer reproduction; `docs/vv/verification-results.md` rev 2.8 row; `AGENTS.md:62-69`. | Label the counts as point-in-time snapshots and attribute the exit status to doxygen vs `make`; re-run the docs-set check after all cycle reports are added, or state the snapshot date. No functional impact. |
| 5 | Comment | CI `docs` gate strength (N6/N15) | The `docs` job asserts only the exit status of the steps: it never asserts that `build/doxygen/html/index.html` (or the warning set) was produced, so a silent generation regression would still pass. This is pre-existing and not introduced by the delta; the local pristine-tree replication in the record is stronger than the CI job itself. | `.github/workflows/ci.yml:238-244` (no output assertion); `.gitea/workflows/ci.yml:280-281`; reviewer reproduction (index.html present locally). | Consider a bounded follow-up change adding an output-existence assertion in the `docs` job (and, optionally, a `doxygen --version` echo) — as a separate scoped change, not as a condition of this fix. |

Severity basis: no finding contradicts the requirement at risk; no defect,
safety, security or evidence-integrity issue was found in the change itself.
Findings 1-3 are non-functional records/process deficiencies (N21) and are
tracked as NCs for later disposition; they do not stop unrelated development and
do not block the functional fix.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Delta disposition: APPROVED WITH COMMENTS (PA PASS, bounded to this delta).**
The change is minimal (one recipe line), configuration-controlled, correctly
placed in the shared `docs` target (covering host, GitHub and Gitea callers, and
superseding the earlier duplicated inline steps), and cannot mask a Doxygen
failure. The functional defect is correctly classified under N21 (a documented
build command and the `ci:docs` CI job failed on a fresh checkout) and was fixed
with verification appropriate to the behaviour: the unrecovered failure was
reproduced first and the fix was verified on a pristine tree, with unchanged
Doxygen output and warning set. The three Minor findings are non-functional
record items (F1-F3) tracked as NC-EXT-78..80 and do not block continuing
development (N21/N12).

Open gates are correctly stated as still open in all three records: hosted CI
was not run, and T21 tool qualification, NC-EXT-43, and release/qualification
gates are not closed by this delta. This report is not hosted-CI evidence, tool
qualification, NC closure, release approval or risk acceptance.

Norms check: N6 (Docker/CI record, bounded), N8/N16 (records updated, except
F1/F3), N10 (F2 open), N11 (evidence retained in the working tree; nothing
committed), N12 (NCs recorded for the findings), N15 (applicable gates recorded,
including explicit `NOT RUN` reasons), N19 (F1), N20 (this review), N21 (defect
classification and non-blocking non-functional findings). Requirements, Lean and
traceability were correctly recorded as not affected (`traceability.yaml`
untouched; no `src/`, `config/`, `tests/`, `verification/` or `tools/` change).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20)      | `aeronautics-space-sw-pa-engineer` (independent context) | 2026-10-15 | APPROVED WITH COMMENTS — bounded to this delta; 3 Minor findings tracked as NC-EXT-78..80 |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-EXT-78 | Minor | `loop-audit.md` currency (N8/N16/N19) | rev 3.2 History row vs header rev 3.1/2026-10-14/`57974d2` | update header to rev 3.2 / 2026-10-15 / baseline `a4c436c` | diff of the corrected header vs History row | frick-fsw (owner of the doc) | open | bounded PA delta re-review required |
| NC-EXT-79 | Minor | `tool-qualification.md` (N10) | `ci:docs` evidence path / `docs` target not registered or mapped; change not recorded | add bounded register entry or documented justification + re-qualification statement | updated register row / justification text | frick-fsw (tool owner; independent R2 at release) | open (N21: deferrable) | included in the release-scope review |
| NC-EXT-80 | Minor | `CHANGELOG.md` origin trace (N16) | regression origin (`7cb90a6` add -> `a4c436c` removal) not recorded | add one sentence with the commit ids | CHANGELOG entry | frick-fsw | open | bounded PA delta re-review required |
| — | Comment | Evidence precision | F4 (exit status, 246 snapshot, AGENTS §3 citation) | wording/precision correction at the owner's discretion, or record as accepted-as-is | — | frick-fsw | open (non-blocking) | not required |
| — | Comment | CI `docs` gate strength | F5 (no output-existence assertion) | separate bounded change proposal | — | frick-fsw | open (non-blocking) | not required |

Notices for the change owner (outside this reviewer's authority, N7/N20): this
report has been created in the working tree only; it is **not logged** in
`docs/reviews/README.md` and not committed, because the reviewer is not the
designated commit agent. The report must be appended to the review log (N20/N11)
when the cycle evidence is published. Requirements/Lean/traceability gates were
verified as not affected; the applicability judgement, the NC dispositions and
any release decision remain with the authorized owner.

---

Committed and pushed with the reviewed change (norm N11).
