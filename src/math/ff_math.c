/*
 * frick-fsw - Portable mathematics facade (libm) implementation
 *
 * Thin wrapper over <math.h>; no dynamic allocation. See ff_math.h and
 * docs/architecture/numerics.md for the determinism and licensing policy.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "math/ff_math.h"
#include <math.h>

#define FF_PI 3.14159265358979323846

double FF_MathSqrt(double x)
{
    return sqrt(x);
}
double FF_MathSin(double x)
{
    return sin(x);
}
double FF_MathCos(double x)
{
    return cos(x);
}
double FF_MathTan(double x)
{
    return tan(x);
}
double FF_MathAtan2(double y, double x)
{
    return atan2(y, x);
}
double FF_MathPow(double x, double y)
{
    return pow(x, y);
}
double FF_MathExp(double x)
{
    return exp(x);
}
double FF_MathLog(double x)
{
    return log(x);
}
double FF_MathFabs(double x)
{
    return fabs(x);
}
double FF_MathFmod(double x, double y)
{
    return fmod(x, y);
}

double FF_MathDeg2Rad(double deg)
{
    return deg * (FF_PI / 180.0);
}
double FF_MathRad2Deg(double rad)
{
    return rad * (180.0 / FF_PI);
}

double FF_MathClamp(double x, double lo, double hi)
{
    if (x < lo) {
        return lo;
    }
    if (x > hi) {
        return hi;
    }
    return x;
}

uint8_t FF_MathIsFinite(double x)
{
    /* finite == not (NaN or infinite) */
    return (uint8_t)(isfinite(x) ? 1 : 0);
}
