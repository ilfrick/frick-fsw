# ADR-0006 - Backwards ECSS documentation and traceability gate (norms N3/N4)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Norms N3/N4 require ECSS-style SRS/SSS generated backwards from the code, and
bidirectional traceability converged at every commit, with derived
justifications kept in the repository.

## Decision
- Documents: `docs/requirements/SSS.md`, `SRS.md`, `traceability.md`.
- Single source of truth: `docs/requirements/traceability.yaml` (normalised
  schema: level, parent_sss, derived, justification, implementation, tests,
  lean, lean_note).
- Gate: `tools/scripts/trace.py` (self-contained, no third-party packages)
  checks document/manifest parity, hierarchy, file existence, `ci:*` evidence
  claims, Lean theorem links and GAP documentation; it exits non-zero on gaps
  (norm N4) and runs in CI and before commit (`make trace`).

## Consequences
Every commit re-verifies convergence; derived items always carry a written
justification; traceability evidence is part of the repository.

## Traceability
Norms N3/N4/N8; SSS-REQ-012/013.