# ADR-0010 - Independent review agents (norm N9)

| Status | Accepted (revised 2026-10-11: five Senior personas versioned in-repo; PA review of every phase, norm N20) |
| ------ | -------- |
| Date   | 2026-10-11 |

## Context
ECSS class B requires independent reviews where the applicable standards call
for independence. Norm N9 mandates reviewers instantiated as independent
agents with independent context. Five Senior roles are used: a real-time/space
software developer (R1), a SW V&V engineer (R2), an avionics/systems engineer
(R3, for SSS reviews, norm N14), a SW documentation engineer (R4, norm N17) and
a SW product assurance engineer (PA, norm N20).

## Decision
- The five Senior **persona documents are versioned in this repository** under
  `docs/engineering/reviewer-agents/` (authoritative prompts), exposed as OpenCode
  subagents (global `~/.config/opencode/agents/`, project `.opencode/agents/`).
  Mapping, creation instructions, scope tables and independence constraints:
  `docs/engineering/independent-review.md`.
- **Norm N20:** every phase product is reviewed with independence **also by the
  Senior SW PA engineer (PA)**; findings are fixed and the reviews repeated until
  they pass, before commit/push.
- Review reports: `docs/reviews/` with template `docs/reviews/review-template.md`,
  committed per norm N11.
- Non-conformances: NC resolution loop per norm N12; SSS reviews are repeated
  until successful closure (norm N14).

## Consequences
Independence is technical (fresh context, no author context) and documented;
organisational independence (company-level SES/QA) remains GAP-A3.

## Traceability
Norms N9/N10/N11/N12/N14/N17/N20; SSS-REQ-012.
