# Formal Verification with Lean 4 (norm N1)

| Document id | FRICK-FV-001 |
| ----------- | ------------ |
| Revision    | 0.1 |
| Date        | 2026-10-10 |

## 1. Goal

Every generated code artifact must be checked/verified with Lean (norm N1).
frick-fsw ships a Lean 4 project whose theorems discharge the requirement-level
properties; the traceability manifest links each theorem to the requirement it
supports, and `tools/scripts/trace.py` validates the links (norm N4).

## 2. Project layout

| File | Content |
| ---- | ------- |
| `verification/lean/lean-toolchain` | pins Lean 4.10.0 |
| `verification/lean/lakefile.lean`  | package `frick_verify`, executable `frick_verify_checks` |
| `Frick/Common.lean`                | shared sizes/constants, `omega` shim |
| `Frick/Ccsds.lean`                 | CRC-16/CCITT-FALSE codec mirror + transmission-check theorem |
| `Frick/SoftwareBus.lean`           | bus delivery counting, queue-depth bound |
| `Frick/Pus.lean`                   | verification-count and TM counter monotonicity (mod 16384) |
| `Frick/Requirements.lean`          | `req_*` theorems discharging SRS/SSS/GAP ids |
| `Main.lean`                        | closed-form runtime checks for CI |

## 3. Proof discipline

- Proofs use only core tactics (`rfl`, `simp`, `omega`, `native_decide`) - no
  Mathlib dependency - so `lake build` is deterministic and fast.
- Every `req_*` theorem is named after the SRS/SSS requirement; renaming
  requires updating the manifest (norm N4).
- The C codec (`src/transport/ccsds/ccsds.c`) is mirrored definitionally close,
  so theorem soundness transfers to the mirrored function; the full C-embedding
  (extraction/whole-program) is GAP-A1.

## 4. Running

```bash
make verify-lean           # cd verification/lean && lake build
```

CI job `verify-lean` runs the same command; traceability job `trace` verifies
the requirement-theorem links.

## 5. Scope and gaps

Proved today: CRC transmission check on reference frames, CCSDS length
arithmetic, bus single-publish bound, PUS verification count and TM counter
bounds. Deferred to class A (GAP-A1): arbitrary-length CRC induction, full C
memory model and dispatch formalisation.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial formal verification document |