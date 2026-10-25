# Formal Verification with Lean 4 (norm N1)

| Document id | FRICK-FV-001 |
| ----------- | ------------ |
| Revision    | 0.4 |
| Date        | 2026-10-21 |

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
| `Frick/LeanOmega.lean`             | `omega` helper lemmas (core Lean, no Mathlib) |
| `Frick/Ccsds.lean`                 | CRC-16/CCITT-FALSE codec mirror + transmission-check theorem |
| `Frick/SoftwareBus.lean`           | bus delivery counting, queue-depth bound |
| `Frick/Pus.lean`                   | verification-count and TM counter monotonicity (mod 16384) |
| `Frick/Time.lean`                  | M2 time base: source-set disjointness, tone discipline, leap-at-boundary, CUC round-trip/fine bound/coarse decomposition |
| `Frick/Hal.lean`                   | M3 HAL registry: bounded count, duplicate registration unchanged |
| `Frick/Sdls.lean`                  | SDLS/security model: anti-replay window and sequence discipline |
| `Frick/S4c.lean`                   | S4c parameter/statistics model |
| `Frick/Watchdog.lean`              | FDIR reset recovery (S8-FDIR-01): arm/feed/expiry state machine, report-before-reset ordering, single reset path (SRS-REQ-073/074/075) |
| `Frick/Requirements.lean`          | `req_*` theorems discharging SRS/SSS/GAP ids (imports every module above, so `lake build` elaborates all of them) |
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
bounds, and (M2) the time-base properties in `Frick.Time`: the time-source set
is disjoint, each valid tone advances the tone second counter by exactly one
second and the tone epoch strictly increases, the exported CUC seconds never
regress across a tone boundary under a non-negative scheduled leap, the pending
leap is applied/cleared exactly at a boundary, the CUC field round trip is
lossless, and the fine fraction of any sub-second count fits 16 bits (with the
coarse 4-octet decomposition recombining), and (M3) the HAL registry bound in
`Frick.Hal` (the registered count never exceeds the capacity and a duplicate
registration leaves the registry unchanged). Deferred to class A (GAP-A1):
arbitrary-length CRC induction, full C memory model and dispatch formalisation.
Lean 4 core only (no Mathlib): proofs use `rfl`, `simp`, `omega` and
`native_decide`, pinned by `lean-toolchain` (v4.10.0); no external math library
is used in either the formal model or the flight C code (integer arithmetic
only).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial formal verification document |
| 0.2 | 2026-10-10 | frick-fsw | M2: add `Frick.Time` (and `Frick.LeanOmega`) to layout/scope; note Lean 4 core / no Mathlib / no math library |
| 0.3 | 2026-10-10 | frick-fsw | M3: add `Frick.Hal` (HAL registry bound) to layout/scope |
| 0.4 | 2026-10-21 | frick-fsw | Register `Frick.Watchdog` (FDIR reset recovery, SRS-REQ-073/074/075) and the previously omitted `Frick.Sdls`/`Frick.S4c` in the module table; state that `Frick.Requirements` imports every module so the default `lake build` elaborates all of them (the gate blind spot that hid the unimported Watchdog module is NC-EXT-120/174). FRICK-REVIEW-0406 F-10 |
