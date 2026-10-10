# Numerics and Mathematics Support

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0009                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N3, N4, N8, N13                        |
| Requirements                      | SSS-REQ-018, SRS-REQ-033               |
| Decision                          | `docs/adr/ADR-0012-libm-math.md`       |

## 1. Purpose

This note documents the numerical policy of frick-fsw and the maths facade over
the platform `libm` (milestone M6), so that users know what is available, how it
behaves, and its determinism and licensing implications.

## 2. Core vs applications

- The **framework core** (PAL, SB, EVS, ES, TBL, Time, CCSDS, PUS, HAL) uses
  **integer arithmetic only**; it does not call `libm` and stays deterministic
  (SSS-REQ-010).
- **Applications** may use floating-point mathematics through the facade
  `src/math/ff_math.h` (SSS-REQ-018 / SRS-REQ-033). The dependency on `libm` is
  explicit (`FF_USE_LIBM`, `FRICK_USE_LIBM=1`) and centrally managed.

## 3. Facade

`src/math/ff_math.h` provides: `FF_MathSqrt/Sin/Cos/Tan/Atan2/Pow/Exp/Log/
Fabs/Fmod` (IEEE-754 `double`), and the helpers `FF_MathDeg2Rad`,
`FF_MathRad2Deg`, `FF_MathClamp`, `FF_MathIsFinite`.

**Domain behaviour** (platform `libm` semantics): `sqrt(x<0) = NaN`,
`log(x<=0) = -inf/NaN`, etc. Callers needing a defined fallback shall test the
result with `FF_MathIsFinite()`.

## 4. Determinism policy

- The core's determinism guarantee (SSS-REQ-010) is unaffected: no floating
  point is used there.
- Transcendental functions are **not guaranteed bit-for-bit identical** across
  toolchains/CPUs (host glibc vs target newlib vs a future certified library).
  They **shall not be used in hard real-time critical paths** without a
  WCET/stack and error analysis (GAP-A4).
- Unit tests use **tolerances**, never exact equality, for transcendental
  results (`tests/unit/test_math.c`).
- On the RISC-V RV64GC target the FPU is used; floating-point context save/
  restore on task switch is the RTEMS BSP's responsibility (GAP-A5).
- The facade is **reentrant and thread-safe**: the functions are stateless
  wrappers with no shared mutable state and no dynamic allocation; callers need
  no locking.
- **Error signalling**: out-of-domain or non-representable results are
  signalled through the IEEE-754 value itself (`NaN`/`+-inf`). The underlying
  platform `libm` may set `errno` (e.g. `EDOM`/`ERANGE`); the facade does not
  clear it, and callers shall **not** rely on `errno` — they shall test the
  result with `FF_MathIsFinite()` when a defined fallback is required.

## 5. Licensing

`libm` is part of the C toolchain already in use (glibc host / newlib RTEMS);
no new third-party component is distributed by frick-fsw. See
`docs/quality/licensing.md` and ADR-0012.

## 6. Verification

- `tests/unit/test_math.c` (SRS-REQ-033): elementary functions within tolerance,
  helper behaviour, domain/finiteness.
- Inspection: the facade is a thin wrapper (no allocation, no hidden state).
- The facade is third-party-backed and therefore not Lean-checkable; the
  manifest records an explicit `lean_note` for SRS-REQ-033.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial numerics note and libm facade policy (milestone M6) |
