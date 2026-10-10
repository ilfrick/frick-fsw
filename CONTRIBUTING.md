# Contributing to frick-fsw

Thank you for contributing. This project is governed by the non-derogable
norms N1-N18 in `docs/rules.md`; every contribution must comply with all of
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
4. run the **complete development & verification cycle** (N15):
   `make ci-local` (build + unit tests + simulator smoke + Lean + trace +
   static + docs) or the equivalent CI jobs;
5. record the **evidence** in `docs/vv/verification-results.md` (N11);
6. run the **applicable reviews** (N9/N10/N14/N17): R1 code/design, R2 V&V,
   R3 SSS, R4 documentation; store reports in `docs/reviews/`; resolve NCs and
   re-run the cycle until closure (N12); the R4 documentation review loop must
   be closed before commit/push (N17);
7. update the **revision history** (N16): `CHANGELOG.md` for the next release
   entry and the per-document `## History` tables that changed;
8. commit with a **detailed message** (what, why, origin of the change, norm
   links) and push to **both repositories** (N7).

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

Create reviewers per `docs/engineering/independent-review.md` (independent
context, fresh session). Reports use `docs/reviews/review-template.md` and are
committed with the change (N11).

## 5. Contact / issue tracking

Use the GitHub issues of the primary mirror; security issues go to
`SECURITY.md`. Maintainers respond in English (N2).