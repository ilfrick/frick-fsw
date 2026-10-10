# ADR-0005 - Lean 4 formal verification (norm N1)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Norm N1 (non-derogable) requires every generated code artifact to be verified
with Lean. The project must ship a Lean project that builds in CI and whose
theorems discharge the SRS/SSS requirements.

## Decision
- Lean 4 project in `verification/lean/` (toolchain pinned `v4.10.0` in
  `lean-toolchain`, `lake` build target `frick_verify_checks`).
- Theorems are named after requirements (`req_*`) and are linked from
  `docs/requirements/traceability.yaml`; `tools/scripts/trace.py` verifies the
  links (norm N4).
- Proofs intentionally stay in the core tactic set
  (`rfl`, `simp`, `omega`, `native_decide`) - no Mathlib dependency, so the
  project builds deterministically and fast in CI.
- `Main.lean` runs closed-form runtime checks in CI.

## Consequences
Machine-checked guarantees for CRC, bus semantics, PUS counters and length
arithmetic; deeper proofs (arbitrary-length induction, full C embedding) are
GAP-A1.

## Traceability
N1; GAP-A1.