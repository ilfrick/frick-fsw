# ADR-0010 - Independent review agents (norm N9)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
ECSS class B requires independent reviews where the applicable standards call
for independence. Norm N9 mandates reviewers instantiated as independent
agents with independent context: a Senior real-time/space/aeronautics/defense
software developer (R1) for design/code review and a Senior SW V&V engineer
(R2) for V&V review.

## Decision
- Profiles R1 (Senior real-time/space/aeronautics/defense SW developer), R2
  (Senior SW V&V engineer) and R3 (Senior Systems Engineer for SSS reviews,
  norm N14), creation instructions, scope tables and independence constraints:
  `docs/engineering/independent-review.md`.
- Review reports: `docs/reviews/` with template `docs/reviews/review-template.md`,
  committed per norm N11.
- Non-conformances: NC resolution loop per norm N12; SSS reviews are repeated
  until successful closure (norm N14).

## Consequences
Independence is technical (fresh context, no author context) and documented;
organisational independence (company-level SES/QA) remains GAP-A3.

## Traceability
Norms N9/N10/N11/N12; SSS-REQ-012.