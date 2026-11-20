# Session Handover — S1 Progress Failure and License Checker Delta

| Field | Value |
| --- | --- |
| Project | frick-fsw |
| Date | 2026-11-14 |
| Baseline HEAD | `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` |
| Worktree | Uncommitted; do not assume clean |
| Primary resume document | `docs/status/handover.md` |
| This handover | `docs/status/session-handover-2026-11-14.md` |

## 1. Immediate status

The active change combines:

1. **S1 progress-failure verification:** `FF_PusIngest` emits TM[1,5] after a
   successful handler or TM[1,6] after a failed handler, when progress ack is
   requested. It emits exactly one of these progress packets. Regression
   coverage is in `tests/unit/test_pus_s11_residual.c`.
2. **N13 SPDX correction:** the project-owned overlay
   `tools/patches/n7s-obcp/src/modifications/py/vm.c`, derived from MIT-licensed
   MicroPython code, now declares `MIT AND MPL-2.0`. The checker has a narrow
   compound-expression exception under `tools/patches/`, with MPL-2.0 required
   and the exact compound expression `MIT AND MPL-2.0` as its only exception.
   This exact-expression constraint was tightened during handover preparation:
   the earlier draft checker discarded operator distinctions and could have
   admitted `MIT OR MPL-2.0`. Added positive/negative checker tests for this.
3. **Documentation and evidence corrections:** traceability, TS-1 arithmetic,
   review/evidence records, interface documentation, NC records, and owner
   dispositions were reconciled.

**Nothing has been committed or pushed.** Do not commit or push from this
handover. The repository's `AGENTS.md` reserves commit/push to the designated
FSW agent only after all applicable gates pass and the complete sprint scope is
verified. There are outstanding gates and failed reviews.

### Owner decisions — final; do not ask again

- **AD-001:** The owner repeatedly confirmed that ICD rev 1.9 / commit
  `fc68b9d` is the authoritative disposition for NC-EXT-94/95 and explicitly
  directed that both be closed. This is recorded in
  `docs/quality/product-assurance.md` §6 and applied to both NC rows in
  `docs/planning/sprint-plan.md`. The implementation must be committed together
  with the ICD revision; closure is not approval, qualification, or a PUS-C
  compliance claim. NC-EXT-249 and NC-EXT-250 remain distinct and open.
- **AD-002:** Under owner delegation, the engineer chose the accurate compound
  SPDX expression `MIT AND MPL-2.0`, not a single MPL label that would
  misrepresent inherited MIT code. Decision and limits are recorded in
  `docs/quality/product-assurance.md` §6 and `docs/quality/licensing.md` §5.1.
  Do not request the user to choose again.

### Evidence currently recorded

Evidence directory:
`docs/vv/evidence/2026-11-14-s1-progress-failure/`.

- Fresh host configure/build in `build/s1-review`: exit 0, zero compiler
  warnings.
- CTest: 60/60 PASS.
- Focused PUS/decoder set: 7/7 PASS.
- `test_pus_s11_residual`: 1206 checks, zero failures.
- `make trace`: PASS (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean
  theorems).
- `make verify-lean`: PASS. This does **not** model the new progress-stage
  invariant; that gap is NC-EXT-248.
- `make gsw-selfcheck`: PASS after the final exact-expression restriction; the
  retained `gsw-final.log` includes 17/17 license-check tests, 12 GSW tests, and 9
  metrics tests.
- Changed-file format check used host clang-format 23.1.3 and passed. Pinned
  clang-format 18 for the changed files is not evidenced.
- Full repository format check remains failed: nine files / 39 violations;
  marker/body exit-code mismatch is documented. NC-EXT-244 remains open.
- Default `make smoke` failed due to the existing `build/native` cache path
  mismatch; safe separate `BUILD_DIR=build/smoke-safe` smoke passed.
- **N18 is not evidenced for this S1 delta.** The retained Renode run predates
  the source edit and has no candidate ELF/image identity. No fresh Docker run
  has been performed; see NC-EXT-246/247.

The candidate digest manifest is `assets.sha256` in the evidence directory.
Verify all listed hashes before using old reviews or evidence to characterize
this exact worktree. The previous five reviews are bound to an earlier
candidate; their findings are useful history, not re-review of the current
state. `gsw-fresh.log` is an earlier run of the checker before the exact
expression tightening; use `gsw-final.log` for current N13/GSW status.

## 2. Review status and interruption

Five first-round review reports exist:

- `docs/reviews/2026-11-14-s1-progress-failure-R1-fsw.md`
- `docs/reviews/2026-11-14-s1-progress-failure-R2-vv.md`
- `docs/reviews/2026-11-14-s1-progress-failure-R3-systems.md`
- `docs/reviews/2026-11-14-s1-progress-failure-R4-documentation.md`
- `docs/reviews/2026-11-14-s1-progress-failure-PA.md`

All five required corrections. A request was then launched for five independent
re-reviews on the corrected tree. All five subagent sessions failed before
producing reports because the provider returned **“Free usage limit reached.”**
There are no corrected-tree re-review reports. Do not claim N9, N17, or N20
re-review passed; no review was fabricated. The original five reports and their
index entries remain useful but are bound to the earlier hashes.

Next review action: obtain a working provider/quota, then run independent
re-reviews R1, R2, R3, R4, and PA against hashes in the current
`assets.sha256`. Reports must identify the exact hashes reviewed. Do not reuse
the earlier verdicts as current approval. N17 and N20 remain commit gates.

## 3. Current open non-conformances / gates

- **NC-EXT-43:** third-party provenance, full notice completeness, generated
  outputs, and combined-distribution compatibility remain open. The SPDX
  relabel/checker pass is not a legal or compatibility conclusion.
- **NC-EXT-44:** T21 checker change requires independent review and qualification;
  tool qualification remains open.
- **NC-EXT-244:** full repository format-check fails on nine files / 39
  violations. Do not edit unrelated files as part of this bounded increment
  without authorized scope.
- **NC-EXT-245:** N19 audit-sequencing deviation, recorded as non-blocking under
  N21.
- **NC-EXT-246:** Docker runner preflight/cleanup deviation; no further Docker
  run until the documented safe preflight is approved and applied.
- **NC-EXT-247:** N18 not evidenced for this delta; blocks claiming the
  applicable target-emulation gate complete.
- **NC-EXT-248:** Lean model omits the progress stage; the exact-one progress
  invariant is tested but not formally represented/proved.
- **NC-EXT-249:** interface authority has not disposed of the handler-TM vs
  progress-TM ordering change or handler-failure TM[1,6]+TM[1,8]
  co-occurrence. It is separate from closed NC-EXT-94/95.
- **NC-EXT-250:** SRS-REQ-006 versus handler-not-found early-return behavior.
- **NC-EXT-251:** pre-existing PUS mutex defect, functional and not introduced
  by this delta.
- Other still-open project findings include NC-S9-01, NC-PAL-01,
  NC-EXT-198/199, and T3 qualification. Consult the current sprint plan and
  primary handover for exact scope.

No release, qualification, flight, safety, security, waiver, or residual-risk
acceptance is authorized by this handover.

## 4. Agents: profiles, roles, and correct use

### Repository rules and source-of-truth profiles

`AGENTS.md` is the repository entry point and defines the non-derogable norms,
build/test commands, evidence rules, and commit restrictions. The review
process is documented in `docs/engineering/independent-review.md`. The five
authoritative Senior reviewer persona texts are versioned in:

`docs/engineering/reviewer-agents/`

The OpenCode agent definitions are also exposed in
`.opencode/agents/` (project-local) and, in the current environment,
`~/.config/opencode/agents/` (global). The repository profile documents are
the source of truth if an installed agent definition differs. The active
OpenCode `mode: all` profiles can be used as primary agents or subagents.

| Label | Agent ID | Profile file | Use for |
| --- | --- | --- | --- |
| R1 | `aeronautics-space-fsw-engineer` | `reviewer-agents/aeronautics-space-fsw-engineer.md` | Flight-software architecture, code, real-time behavior, drivers, FDIR |
| R2 | `aeronautics-space-sw-vv-engineer` | `reviewer-agents/aeronautics-space-sw-vv-engineer.md` | Requirements-based verification, test design/evidence, traceability, qualification support |
| R3 | `aeronautics-space-avionics-systems-engineer` | `reviewer-agents/aeronautics-space-avionics-systems-engineer.md` | System/interface assumptions, avionics integration, failure effects, budgets |
| R4 | `aeronautics-space-sw-documentation-engineer` | `reviewer-agents/aeronautics-space-sw-documentation-engineer.md` | Documentation completeness, consistency, revision/history, claims and records |
| PA / N20 | `aeronautics-space-sw-pa-engineer` | `reviewer-agents/aeronautics-space-sw-pa-engineer.md` | Product assurance, process/evidence integrity, anomalies, configuration and release package |

Other currently available agents:

- `explore`: rapid codebase/file/symbol exploration; specify `quick`,
  `medium`, or `very thorough`.
- `general`: multi-step research or execution tasks.

These are useful assistants, not substitutes for the five required profiles or
for independent review. A subagent review is contextual independence only; it
does not establish organizational independence or automatically satisfy N9,
N17, or N20.

### How to launch a reviewer

1. Create a genuinely fresh session/subagent for each profile; do not give it
   the author's private chain-of-thought or unrecorded context.
2. Bind the review to exact candidate hashes from
   `docs/vv/evidence/2026-11-14-s1-progress-failure/assets.sha256`.
3. Give it the relevant paths, governing requirements, prior findings (for
   re-review), and explicit instructions to stop if hashes drift.
4. Require a file-backed report under `docs/reviews/`; require prior finding
   disposition, new findings, evidence, limitations, and a clear verdict.
5. Add the report to `docs/reviews/README.md`, keep it in the working tree,
   and do not mark it as a pass unless the report actually says so.
6. If an agent/session cannot start due to provider limits, record that as
   **not performed**. Never synthesize a review or infer approval from a prior
   round.
7. Re-run affected gates after any resulting corrections. Do not commit/push
   until all applicable gates, review requirements, and sprint scope pass.

The fuller instantiation procedure and review task template are in
`docs/engineering/independent-review.md` §§2–3.

## 5. Suggested sequence for the receiving session

1. Read `AGENTS.md`, `docs/status/handover.md`, this file,
   `docs/status/loop-audit.md`, and `docs/planning/sprint-plan.md`.
2. Run `git status --short`; do not clean caches or discard changes.
3. Verify candidate hashes against `assets.sha256`. Rebuild the manifest if
   the worktree changed; re-review must bind to the new identity.
4. Check `git diff --check`, `make build`, `make test`, `make verify-lean`,
   `make trace`, and `make gsw-selfcheck` in the approved non-Docker
   configuration; preserve raw results under the evidence directory.
5. Do not run Docker/Renode until the NC-EXT-246 preflight, cleanup, mount,
   and evidence-directory controls are satisfied. N18 remains unverified for
   this delta.
6. Obtain the five required corrected-tree re-reviews. Do not ask the owner
   again about AD-001/AD-002.
7. Disposition NC-EXT-249/250 through their authorized interfaces/requirements
   owners; track, do not silently close, NC-EXT-248/251 and all licensing/tool
   qualification gaps.
8. Only after the complete sprint scope and applicable gates pass may the
   designated FSW agent consider commit/push under N7. This handover itself
   grants no authority to commit, push, release, or accept risk.

## 6. Evidence references

- `docs/vv/verification-results.md` — detailed current-cycle gate results and
  limitations.
- `docs/vv/test-spec/ts-1-protocol-core.md` — TS-1 cases and coverage totals.
- `docs/quality/product-assurance.md` §6 — final owner dispositions AD-001 and
  AD-002.
- `docs/quality/licensing.md` §5.1 — compound SPDX policy and limits.
- `docs/vv/tool-qualification.md` — T21 status; **not fully qualified**.
- `docs/reviews/README.md` — review report index.
- `docs/reviews/2026-11-14-s1-progress-failure-*.md` — first-round reports
  only; not corrected-tree re-reviews.
- `docs/vv/evidence/2026-11-14-s1-progress-failure/` — raw logs and candidate
  hash manifest.

## Facts / assumptions / confidence

- **Facts:** worktree status and review reports were inspected on 2026-11-14;
  the five re-review subagent launches returned provider “Free usage limit
  reached” errors, with no reports created. The gate values above refer to
  retained logs and the current `verification-results.md`.
- **Assumptions:** none about the owner’s decisions; AD-001/AD-002 are recorded
  as final and are not pending questions.
- **Recommendation:** preserve the worktree, obtain actual re-reviews with a
  working provider, and keep commit/push withheld until applicable gates pass.
- **Confidence:** High for recorded worktree/review status; medium for evidence
  attribution because this handover is not itself an independent review and
  the target N18 run is missing.
- **Open gaps:** N18, complete format gate, T21 independent qualification,
  external licensing/provenance disposition, formal-model progress coverage,
  interface dispositions NC-EXT-249/250, and corrected-tree re-reviews.

## Revision history

| Revision | Date | Author | Change |
| --- | --- | --- | --- |
| 1.0 | 2026-11-14 | frick-fsw | Initial focused handover for the uncommitted S1 progress-failure and SPDX-checker delta; records final owner dispositions, unavailable re-review sessions, project agent profiles and next steps. |
