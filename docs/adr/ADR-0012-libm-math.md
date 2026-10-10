# ADR-0012 - Mathematics library integration (libm)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Applications on top of the middleware (geometry, attitude, coordinate
transforms, engineering conversions) need floating-point mathematics. The
framework core uses only integer arithmetic (`uint64_t` nanoseconds, bounded
counters) to stay deterministic, but users need a documented, portable way to
call elementary math functions. Reaching `libm` directly from applications
scatters the dependency, the determinism implications and the licence analysis.

## Decision
- Add a **portable mathematics facade** (`src/math/ff_math.{c,h}`, milestone M6)
  over the platform `libm`, exposing `sqrt/sin/cos/tan/atan2/pow/exp/log/fabs/
  fmod` plus angle and clamp helpers and a finiteness check.
- Link the platform `libm` (`-lm`) in the build (`FF_USE_LIBM`, default ON;
  `FRICK_USE_LIBM=1`). On the host this is glibc; on RTEMS 6 / RISC-V it is
  newlib's `libm`.
- Keep the core free of floating point; the facade is a user-facing utility.
- Document the **determinism policy** and the **licensing** implications
  (`docs/architecture/numerics.md`, `docs/quality/licensing.md`).

## Rationale
- One contract, one dependency, one licence analysis; the implementation can be
  swapped (fixed-point or a certified library) behind the same façade.
- `libm` is part of the C toolchain already used (glibc host / newlib RTEMS),
  so no new third-party component is introduced.

## Consequences
- Applications may use floating point; the core remains integer/deterministic.
- Iterative transcendental functions are not bit-for-bit identical across
  toolchains; the policy states they must not be used in hard real-time
  critical paths without a WCET/stack analysis (GAP-A4).
- Unit tests must use tolerances, not exact equality, for transcendentals.

## Traceability
SSS-REQ-018, SRS-REQ-033; norms N3/N4/N8/N13.
