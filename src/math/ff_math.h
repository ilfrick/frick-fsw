/*
 * frick-fsw - Portable mathematics facade (libm)
 *
 * Milestone M6. Applications that need floating-point math (geometry,
 * attitude, transforms, engineering conversions) use this facade instead of
 * calling the platform libm directly, so that:
 *   - the dependency and its licence are explicit and centrally managed
 *     (norm N13; docs/quality/licensing.md);
 *   - the determinism policy is documented (see docs/architecture/numerics.md);
 *   - the implementation can be swapped (host glibc libm, RTEMS/newlib libm,
 *     or a fixed-point / mission-certified replacement) behind one contract.
 *
 * This is a thin, allocation-free wrapper over <math.h>. It is NOT intended for
 * hard real-time critical paths without a WCET/stack analysis (GAP-A4); the
 * integer arithmetic used by the framework core is unaffected.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_MATH_H
#define FF_MATH_H

#include "common/ff_types.h"

/* Build-time switch (CMake FF_USE_LIBM, default ON). */
#if !defined(FRICK_USE_LIBM) || (FRICK_USE_LIBM == 0)
#error "frick-fsw math facade requires FRICK_USE_LIBM=1 (link libm)"
#endif

/* Elementary functions (IEEE-754 double). Out-of-domain arguments follow the
 * platform libm: e.g. sqrt(x<0) = NaN, log(x<=0) = -inf/NaN. Callers that need
 * a defined fallback shall check FF_MathIsFinite() on the result. */
double FF_MathSqrt(double x);
double FF_MathSin(double x);
double FF_MathCos(double x);
double FF_MathTan(double x);
double FF_MathAtan2(double y, double x);
double FF_MathPow(double x, double y);
double FF_MathExp(double x);
double FF_MathLog(double x);
double FF_MathFabs(double x);
double FF_MathFmod(double x, double y);

/* Helpers (implemented in the facade, not from libm).
 * FF_MathClamp requires lo <= hi; a NaN x is not clamped (returned as-is). */
double FF_MathDeg2Rad(double deg);
double FF_MathRad2Deg(double rad);
double FF_MathClamp(double x, double lo, double hi);

/* 1 when x is finite (neither NaN nor infinity), 0 otherwise. */
uint8_t FF_MathIsFinite(double x);

#endif /* FF_MATH_H */
