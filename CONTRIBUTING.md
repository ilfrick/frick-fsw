# Contributing to frick-fsw

| Document revision | 0.2 |
| --- | --- |
| Date | 2026-10-13 |

Thank you for contributing. This project is governed by the non-derogable
norms N1-N20 in `docs/rules.md`; every contribution must comply with all of
them before it can be committed and pushed.

## 1. Before you start

- Read `AGENTS.md` (50% of the operational knowledge, aimed at people and
  agents) and `docs/rules.md`.
- Branches/PRs: on GitHub use pull requests; on Gitea the same pipeline runs
  via the deployed `act_runner` (norm N6).

## 2. Change checklist (norms N7/N8/N15/N16)

Every change - whatever its origin (feature, test failure fix, review NC,
requirement delta, refactor, tooling) - must:

1. keep all documentation and code comments **in English** (N2);
2. update the affected **requirements/traceability**: `traceability.yaml`,
   `SRS.md`/`SSS.md` and `traceability.md` (N3/N4);
3. update **tooling / V&V** documents when tools or scripts changed (N8/N10);
4. run the required **development & verification cycle** (N15): `make ci-local`
   covers configure/build, unit tests, Lean and trace only. It does not run
   simulator smoke, GSW self-check, static analysis/formatting, RTEMS cross-build,
   Renode, metrics, or docs; run all applicable separate gates in the verification
   plan or use the corresponding hosted CI jobs;
5. record the **evidence** in `docs/vv/verification-results.md` (N11);
6. run the **applicable reviews** (N9/N10/N14/N17/N20): R1 code/design, R2 V&V,
   R3 SSS, R4 documentation, PA product assurance; store reports in `docs/reviews/`;
   resolve NCs and re-run the cycle until closure (N12); the R4 documentation review
   loop (N17) and the PA review of every phase product (N20) must be closed before
   commit/push;
7. update the **revision history** (N16): `CHANGELOG.md` for the next release
   entry and the per-document `## History` tables that changed;
8. **Do not commit or push unless you are the designated
   `aeronautics-space-fsw-engineer` agent or the user explicitly delegated that
   authority for this scope.** That FSW engineer may commit with a detailed
   message and push to both repositories only after verifying all applicable
   gates and project norms and confirming the complete sprint scope is done
   (N7). No delegation permits bypassing a required gate.

## 3. Commit message template

```text
<scope>: <short summary>

Origin: <feature | test-failure | review-NC <id> | requirement-delta |
         refactor | tooling>
What: ...
Why: ...
Norms: N<n>, N<m>
Gates: build PASS, ctest PASS, lean PASS, trace PASS, smoke PASS
```

## 4. Reviews and independence

| Profile | Scope |
| ------- | ----- |
| R1 Senior RT/space/defense SW developer | design/architecture/code review |
| R2 Senior SW V&V engineer | verification plan, scripts/tools qualification, results |
| R3 Senior Systems Engineer (space/aero) | SSS system requirements (N14) |
| R4 Senior SW documentation reviewer | documentation completeness/correctness (N17) |
| PA Senior SW product assurance engineer | every phase product: process/records/independence (N20) |

Create reviewers per `docs/engineering/independent-review.md` (independent
context, fresh session). Record reports in `docs/reviews/` in the working tree
when produced; retain failed/intermediate reports and commit/push them with the
reviewed change only after applicable verification and independent-review gates
pass (N7/N11/N15/N17/N20).

## 5. Contact / issue tracking

Use the GitHub issues of the primary mirror; security issues go to
`SECURITY.md`. Maintainers respond in English (N2).

## History

| Rev | Date | Author | Reason |
| --- | --- | --- | --- |
| 0.1 | 2026-10-10 | frick-fsw | Initial contribution checklist and review guidance. |
| 0.2 | 2026-10-13 | frick-fsw | Correct `ci-local` scope and point to separate applicable verification gates (NC-EXT-38). |
| 0.3 | 2026-10-14 | frick-fsw | Apply user-directed N7 restriction: only designated FSW engineer commits/pushes after gates and full sprint scope; explicit delegation required for exceptions. |
