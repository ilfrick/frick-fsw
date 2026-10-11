# Non-derogable Norms (Rules N1-N18)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-RULES-001                        |
| Revision                          | 1.3                                    |
| Date                              | 2026-10-10                             |
| Applicable standards              | ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C) |
| Applicability                     | Every change to this repository, every generated artifact, every commit |

The norms below are **non-derogable** (ND) for this project: none of them can be
waived by any contributor, agent, or automated tool. Any change that does not
satisfy all applicable norms is not compliant and must not be committed or
pushed (norm N7). The norms are maintained here and are covered by the
traceability manifest (`docs/requirements/traceability.yaml`) and the CI gates.

## N1 - Formal verification with Lean

> Every generated code artifact (implementation or requirement-level model)
> must be checked/verified with Lean 4.

- The Lean 4 project lives in `verification/lean/` and is built with
  `lake build` (toolchain pinned in `lean-toolchain`).
- Verified-by-Lean code is mirrored by machine-checked theorems; CI runs
  `verify-lean`.
- Where a requirement is verified by analysis/test evidence instead of a Lean
  theorem, the traceability manifest records an explicit `lean_note`
  justification (there is no silent waiver).

## N2 - English only

> All project documentation and all code comments shall be in English.

- Code identifiers, comments, commit messages, issues, PRs and documentation
  are written in English.
- The user-facing chat language may differ (e.g. Italian); repository content
  stays English.

## N3 - Backwards ECSS documentation

> The project shall carry a backwards (retroactively generated) ECSS-compatible
> SRS and SSS covering all generated code, themselves verified as much as
> possible with Lean.

- `docs/requirements/SRS.md` and `docs/requirements/SSS.md` are the generated,
  ECSS-style documents; they are traced from code and architecture.
- Each requirement is linked to implementation, tests and Lean theorems through
  the manifest `docs/requirements/traceability.yaml`.

## N4 - Bidirectional traceability

> Before committing SSS/SRS/code, trace bidirectionally (top-down and
> bottom-up), resolve traceability gaps/conflicts, write the justification for
> derived items, and converge. Traceability matrices/evidence and derived
> justifications must live in the repository and be reviewed/updated on every
> change.

- `tools/scripts/trace.py` is the gate: it validates the manifest, the
  SRS/SSS documents, file existence, CI evidence claims and Lean theorem links,
  and the SSS<->SRS hierarchy. It exits non-zero on any gap.
- Derived items (`derived: true` / `gap: true`) must carry a `justification`.
- The gate runs in CI (`trace`) and locally before every commit (`make trace`).

## N5 - ECSS criticality target

> Artifacts shall be suitable for ECSS criticality level B; the gaps to reach
> level A shall be documented.

- `docs/quality/ecss-criticality-b.md` is the level B target baseline.
- `docs/quality/class-a-gaps.md` and the `GAP-A*` manifest entries document the
  residual gaps to level A.

## N6 - Software V&V and deployed test environments

> Everything shall be subjected to Software V&V to reach the target
> level/criticality per ECSS. All necessary test environments and tools shall
> be deployed. Docker shall be used to deploy CI/CD where no forge CI exists
> (Gitea has no built-in CI/CD) and for any other required test environment or
> tool.

- V&V lifecycle: `docs/vv/verification-plan.md`, `docs/vv/verification-results.md`.
- The full pipeline (build, unit tests, simulator smoke, Lean, traceability,
  static analysis) runs on GitHub Actions and, for Gitea, via a self-hosted
  `act_runner` deployed with Docker Compose (`tools/docker/`).
- A Docker-based V&V image (`tools/docker/Dockerfile`) pins the toolchain so any
  human or agent reproduces the same verification environment.

## N7 - Dual push

> Every change compliant with all the norms shall be committed and pushed to
> both repositories.

- Origin repositories: GitHub (`ilfrick/frick-fsw`) and Gitea
  (`ilfrick/frick-fsw`, https://www.housefz.com/git).
- A commit is a candidate for push only if the full local pipeline passes
  (`make build test trace verify-lean` or their CI equivalents).

## N8 - Complete, current, deployable documentation

> The whole project, including approach and tools, shall be accurately
> documented; documentation shall be stored in the repository, reviewed and
> completely updated at every change so that it stays current. The docs must
> make the project deployable by humans and by agents.

- Entry points: `README.md` (human) and `AGENTS.md` (agents).
- Every change that affects behaviour, tooling, requirements or configuration
  must update the affected docs in the same commit.
- CI checks documentation against the traceability manifest (`ci:docs`, `ci:trace`).

## N9 - Independent review agents

> Whenever an independent review is required by the applicable criticality, it
> shall be performed by an independent agent with independent context, with a
> Senior real-time/space/aeronautics/defense software developer profile for
> design/code review, and a Senior SW V&V engineer profile for V&V review.
> Reviewer agent profiles, instructions on how to create them, and descriptions
> of their purpose shall be stored in the project and kept current.

- Profiles, creation instructions and scope: `docs/engineering/independent-review.md`.
- Review evidence (report files) is stored under `docs/reviews/` with the
  template `docs/reviews/review-template.md`.
- Independence requirements: a reviewer agent must run in its **own context**
  (fresh session, no inheritance of author context) and must not review its own
  work. See the profile document for precise constraints.

## N10 - Review of test scripts and V&V tools

> All test scripts of any kind created for V&V shall be reviewed, if necessary
> with independence, as required by ECSS for the applicable criticality level.

- Every script, harness, tool or configuration used as V&V evidence is a
  **verification tool** in the sense of ECSS-Q-ST-80C and shall be qualified:
  correctness by review, traceability to the evidence claims it supports, and
  version control.
- At criticality B the minimum qualification for scripts is: source review +
  inspection + configuration management. Where the criticality or the risk
  warrants it (or level A is targeted), the review is **independent** and uses
  the N9 agent profiles.
- Register of V&V tools and their qualification status: `docs/vv/tool-qualification.md`.
- Any change to a recorded V&V script invalidates its qualification and forces
  a re-qualification review before the evidence produced with it can be reused.
- The gate `make trace` checks that every script referenced as evidence exists
  and is listed in the qualification register.

## N11 - Test scripts and review results in the repository

> All test scripts and the results of all reviews (at every level) shall be
> stored in the project repository and committed and pushed in every
> development and verification cycle.

- Test scripts and V&V harnesses live in `tests/`, `tools/scripts/`,
  `tools/gsw/` and `verification/`; they are versioned like the code they
  verify.
- Review reports (including independent agent reviews, norm N9) are stored in
  `docs/reviews/` using `docs/reviews/review-template.md` and are committed in
  the same cycle as the reviewed change.
- Verification results are recorded in `docs/vv/verification-results.md` and
  updated in every cycle (norm N6/N10/N11).
- Test scripts and review results follow norm N7: compliant changes are pushed
  to both repositories.

## N12 - Non-conformance resolution to full compliance

> If a review raises non-conformances, they shall be resolved and ALL the rules
> re-applied, until full compliance is reached.

- Any finding (regardless of severity) opens a non-conformance (NC) tracked in
  the review report (`docs/reviews/review-template.md`, follow-up records).
- Resolution of an NC restarts the full compliance gate for the affected scope:
  code changes, documentation (norm N8), traceability (norm N4), Lean
  verification (norm N1) and the test scripts/V&V tools used (norm N10) are all
  re-run and re-reviewed as applicable.
- An NC is **closed** only when the disposition is recorded, the re-verification
  evidence is attached, and the review conclusion for the deltas is APPROVED or
  APPROVED WITH COMMENTS (no Major findings open).
- A REJECTED review blocks commit/push (norm N7) until all Major findings are
  resolved and the deltas are re-reviewed.

## N13 - Open source licensing

> The whole project shall be open source under AGPL-3.0 (or the maximum
> permitted by the licenses of any reused code parts). The license shall be
> documented in the project and reviewed/updated when the project changes.

- The project license is **AGPL-3.0-only** (GNU Affero General Public License
  v3); the canonical text is in `LICENSE`.
- All authored **source, configuration and build files** carry the SPDX header
  `SPDX-License-Identifier: AGPL-3.0-only` (already applied repo-wide; new such
  files must add it). Engineering documentation is covered by the project
  `LICENSE`/`NOTICE` and the licence review (`docs/quality/licensing.md`); the
  CI licence job remains authoritative.
- The license context, compatibility analysis for reused/derived components and
  the review procedure are documented in `docs/quality/licensing.md`.
- Any change that touches licence-relevant material (new dependency, reused
  code, third-party components) triggers a license review and update of
  `docs/quality/licensing.md` and `LICENSE` in the same commit (norm N8).
- The CI `docs` job verifies that `LICENSE` is the AGPL-3.0 text and that no
  `Apache-2.0` SPDX headers remain (norm N13 enforcement).

## N14 - Senior systems-engineering review of SSS requirements

> All developed SSS requirements shall be reviewed, as required by the
> applicable ECSS criticality, by a **Senior Systems Engineer** in
> space/aeronautics/aerospace. The independent-agent profile shall be created
> and documented in the project like the other reviewer profiles.
> Non-conformances raised by the reviews shall be resolved and the reviews
> repeated until they close successfully. The reviews shall be included in the
> project.

- The SSS (system level) requirements are reviewed by profile **R3 - Senior
  Systems Engineer (space/aeronautics/aerospace)** defined in
  `docs/engineering/independent-review.md`.
- Trigger: on creation and on every modification of `docs/requirements/SSS.md`
  or of the SSS entries in the traceability manifest.
- Review outcome and NC resolution follow norms N9/N12: the reviewer runs with
  independent context, findings are resolved by re-applying all norms, and the
  review is repeated until the conclusion is APPROVED or APPROVED WITH COMMENTS
  with all NCs closed (norm N12).
- The review reports are stored in `docs/reviews/` and committed and pushed
  with the reviewed change (norm N11).

## N15 - Full development/verification cycle on every change

> Every change, whatever its origin (bug fixed after a test, review finding,
> requirement update, refactoring, tooling change...), regenerates a complete
> development and verification cycle, repeated in all its parts until it
> completes successfully.

- A change is **any** modification to `src/`, `config/`, `tests/`,
  `verification/`, `tools/`, CI files or the requirements/documentation set
  (norm N8).
- The affected scope goes through the complete cycle again: build (host +
  cross proxy), unit tests, simulator smoke, Lean (norm N1), traceability
  (norm N4), static analysis and documentation consistency, plus the
  applicable reviews (norms N9/N10/N14) and NC close-out (norm N12).
- The cycle ends only when every gate passes; evidence is recorded in
  `docs/vv/verification-results.md` (norm N11) and the change is committed and
  pushed to both mirrors (norm N7).

## N16 - Detailed revision history

> For all changes, a detailed revision history shall be written and maintained
> and shall be included in the project.

- A **revision history** is maintained for every document and for the project:
  - project-level: `CHANGELOG.md` (release-oriented, detailed entries);
  - document-level: a `## History` table in each engineering document records
    rev, date, author and reason;
  - code-level: commit messages in the repository (norm N7 dual push).
- Every change updates the relevant history entries in the same commit (norm N8).
- The history includes the origin of the change (test failure, review finding,
  requirement change, refactor, tooling) so the full trace of every change is
  auditable (norm N12/N15).

## N17 - Documentation review by the Senior SW documentation reviewer

> Before every commit/push, the project documentation shall be reviewed for
> completeness and correctness by an independent **Senior SW documentation
> reviewer** agent. The review loop (review -> correction -> re-review) shall
> be closed with all findings resolved before the change is committed and
> pushed; a REJECTED documentation review blocks commit/push.

- The reviewer agent profile is **R4 - Senior SW documentation reviewer**
  defined in `docs/engineering/independent-review.md`, instantiated with
  independent context (norm N9) and stored evidence under `docs/reviews/`
  (norm N11).
- Scope: the documentation set affected by any change — `docs/**`, `README.md`,
  `AGENTS.md`, status records (`docs/status/status.json`), CI workflow
  comments, and the engineering guides — checked for completeness, correctness,
  accuracy against the code/requirements/traceability (norm N8), internal
  consistency (including counts, revisions, history tables and the README
  metrics panel), and applicability to both human and agent users.
- Trigger: on **every change** (norm N15), together with the other applicable
  reviews (norms N9/N10/N14); at minimum one documentation-review loop per
  committed change.
- NC resolution follows norm N12: review -> correction -> re-review (fresh
  context) until the conclusion is APPROVED or APPROVED WITH COMMENTS with all
  NCs closed. All findings are tracked to closure in the review report.
- The gate is reported in the README metrics panel (`documentation_review`
  semaphore in `docs/status/status.json`) and reflected in the enforcement
  summary below.

## N18 - Functional validation in the Renode emulated PolarFire SoC

> The functional validation of every change shall pass in the Renode emulated
> Microchip PolarFire SoC environment.

- The environment is `docs/vv/renode-environment.md` (pinned Renode image,
  `polarfire-soc` platform; middleware test application `app_renode_test.c`
  enabled with `FF_ENABLE_RENODE_TEST=ON`; run via `make renode` /
  `tools/scripts/renode_vv.sh`, CI evidence `ci:renode`).
- The gate passes when the emulated run produces the `RENODE_TEST: PASS` marker.
- The gate is **in force**: verified end-to-end on the real open-source RTEMS 6.2
  image (`make renode-image` then `make renode` -> `RENODE_TEST: PASS`). When the
  RTEMS image is present, a missing marker **fails** the gate (exit 1); when it is
  absent (per-commit runners without the toolchain image) the run is a recorded
  SKIP proxy (never counted as a pass). The nightly workflow performs the real
  run.

## N19 - Development-loop objective/rule conformance audit

> At the start of every development/verification loop, the declared project
> objectives (the milestones/OBJs of `docs/development-plan.md` / `docs/roadmap.md`)
> and adherence to **all** non-derogable rules (N1-N19) shall be compared against
> the **actual implementation** reached up to that point; the gaps and
> discrepancies so identified shall be reconciled within the loop or explicitly
> tracked with a disposition.

- The audit is recorded in `docs/status/loop-audit.md`, objective-by-objective and
  rule-by-rule, with `status / gap / disposition`; each loop starts by updating it.
- A gap is either **closed in the loop**, or **tracked** in
  `docs/quality/class-a-gaps.md` (class A) / `docs/development-plan.md` with an
  owner and a milestone. No gap is left silent.
- Rationale: this norm prevents "declared intent not implemented" discrepancies
  (e.g. a third-party integration recorded in a decision but never wired into the
  build/runtime, or an objective claimed but not delivered) from surviving
  unnoticed across loops.

## Enforcement summary

| Norm | Gate / evidence |
| ---- | --------------- |
| N1   | `make verify-lean` (CI job `verify-lean`), traceability `lean` links |
| N2   | repository language policy, review checklist |
| N3   | SRS.md / SSS.md present and linked in `traceability.yaml` |
| N4   | `make trace` / CI job `trace` exits 0 before push |
| N5   | `ecss-criticality-b.md`, `class-a-gaps.md`, `GAP-A*` manifest entries |
| N6   | CI workflows + Docker V&V image + `docs/vv/**` records |
| N7   | release/contribution process in `CONTRIBUTING.md`; dual remote push |
| N8   | README + AGENTS quickstart; docs updated per change (contribution checklist) |
| N9   | `docs/engineering/independent-review.md`, evidence under `docs/reviews/` |
| N10  | `docs/vv/tool-qualification.md`, re-qualification on any script change |
| N11  | scripts + review reports + V&V results committed/pushed every cycle |
| N12  | NC close-out loop: finding -> resolution -> full re-application of all norms -> re-review -> close |
| N13  | AGPL-3.0-only project license + `docs/quality/licensing.md` + CI check |
| N14  | SSS requirements reviewed by Senior Systems Engineer (R3), NCs closed by repeated reviews (N12), reports in repo (N11) |
| N15  | every change re-runs the complete dev+V&V cycle until success; evidence recorded |
| N16  | detailed revision history maintained (CHANGELOG + per-doc history + commits) |
| N17  | documentation reviewed by the Senior SW documentation reviewer (R4) and the review loop closed before every commit/push |
| N18  | functional validation of every change in the Renode emulated PolarFire SoC environment shall pass; the gate is **in force** (verified on the real open RTEMS 6.2 image: `make renode` -> `RENODE_TEST: PASS`); per-commit runners without the toolchain image record a SKIP proxy (never counted as a pass) and the nightly performs the real run |
| N19  | at the start of every development loop, `docs/status/loop-audit.md` compares declared objectives and adherence to N1-N19 against the actual implementation; every gap is closed in the loop or tracked with owner/milestone (no silent gap) |

## Interpretation notes (non-derogable content unchanged)

The following interpretation notes fix the scope of two norms whose literal
wording is otherwise ambiguous; they result from the independent rules review
`docs/reviews/2026-10-10-rules-conflict-review.md` (norm N12 close-out of the
NCs) and are reviewed on every change (norm N8/N16).

1. **Scope of "verified with Lean" (N1/N3).** "Generated code" means the
   flight-software sources mirrored by the Lean model, i.e. executable C code
   whose properties admit a formal model (codec, bus, PUS dispatch, counters,
   bounds) and the requirement-level theorem statements. Non-executable
   artifacts (prose requirements, manifests, CI files, scripts) are not
   Lean-checkable; they are verified by: formal analysis where implemented
   (N1), tool-qualification review (N10), traceability gate (N4) and the
   applicable R2/R3 reviews (N9/N14). Every such artifact carries an explicit
   `lean_note` or a qualification entry, never a silent waiver.

2. **Scope of the "complete cycle" (N15).** The complete cycle includes every
   gate **applicable to the change**, as defined by `docs/tooling.md` and the
   V&V plan: build (host + crossing proxy), unit tests, simulator smoke,
   Lean, traceability, static analysis, documentation consistency, applicable
   reviews (N9/N10/N14) and NC closure (N12). Steps that require flight
   hardware (GAP-A5) or a very long toolchain build use documented proxies
   (e.g. the RTEMS cross-build proxy and the nightly build) reported in
   `docs/vv/verification-results.md`. A gate not applicable to a change is
   recorded, not silently skipped.

3. **Scope of the per-document history (N16).** The `## History` table is
   required for the substantive engineering, system, quality, V&V and planning
   documents (requirements, architecture notes, ICD, quality, V&V, engineering
   guides, roadmap, development plan). ADRs (`docs/adr/`) record their evolution
   through their `Status`/`Date` front-matter and the git history; a separate
   `## History` table is not duplicated there. This keeps N16 authoritative for
   auditable documents without duplicating the ADR status field.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial rules N1-N8 |
| 0.2 | 2026-10-10 | frick-fsw | Add N9 (independent review agents) |
| 0.3 | 2026-10-10 | frick-fsw | Add N10 (review of test scripts / V&V tools) |
| 0.4 | 2026-10-10 | frick-fsw | Add N11 (scripts and review results in repo, committed every cycle) |
| 0.5 | 2026-10-10 | frick-fsw | Add N12 (NC resolution loop to full compliance) |
| 0.6 | 2026-10-10 | frick-fsw | Add N13 (AGPL-3.0 open source licensing) |
| 0.7 | 2026-10-10 | frick-fsw | Add N14 (Senior Systems Engineer review of SSS) |
| 0.8 | 2026-10-10 | frick-fsw | Add N15 (full cycle on every change) and N16 (detailed revision history) |
| 0.9 | 2026-10-10 | frick-fsw | Add N17 (documentation review by the Senior SW documentation reviewer R4 before every commit/push) |
| 1.0 | 2026-10-10 | frick-fsw | Add interpretation note 3: per-document History scope (ADRs exempt) — R4 review DOC-08 |
| 1.1 | 2026-10-10 | frick-fsw | Scope N13 SPDX requirement to source/config/build files; docs covered by LICENSE/NOTICE (R4 M3 C-2) |
| 1.2 | 2026-10-10 | frick-fsw | Add N18 (Renode functional-validation gate) |
| 1.3 | 2026-10-11 | frick-fsw | Add N19 (development-loop objective/rule conformance audit; `docs/status/loop-audit.md`) |