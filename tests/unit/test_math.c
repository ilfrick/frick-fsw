/*
 * frick-fsw - Mathematics facade unit tests (milestone M6)
 *
 * Verifies the libm facade (SRS-REQ-033): elementary functions within
 * tolerance, domain behaviour and the facade helpers.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "math/ff_math.h"

static int approx(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0.0) {
        d = -d;
    }
    return d <= eps;
}

static void test_elementary(void)
{
    FF_TEST_ASSERT(approx(FF_MathSqrt(4.0), 2.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathSin(0.0), 0.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathCos(0.0), 1.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathTan(0.0), 0.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathTan(0.7853981633974483), 1.0, 1e-9));
    FF_TEST_ASSERT(approx(FF_MathPow(2.0, 10.0), 1024.0, 1e-9));
    FF_TEST_ASSERT(approx(FF_MathExp(0.0), 1.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathLog(1.0), 0.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathFabs(-3.5), 3.5, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathFmod(7.0, 3.0), 1.0, 1e-12));
}

static void test_helpers_and_domain(void)
{
    FF_TEST_ASSERT(approx(FF_MathAtan2(1.0, 1.0), 0.7853981633974483, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathDeg2Rad(180.0), 3.14159265358979323846, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathRad2Deg(3.14159265358979323846), 180.0, 1e-9));
    FF_TEST_ASSERT(approx(FF_MathClamp(5.0, 0.0, 1.0), 1.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathClamp(-1.0, 0.0, 1.0), 0.0, 1e-12));
    FF_TEST_ASSERT(approx(FF_MathClamp(0.5, 0.0, 1.0), 0.5, 1e-12));

    FF_TEST_ASSERT(FF_MathIsFinite(1.0) == 1u);
    FF_TEST_ASSERT(FF_MathIsFinite(FF_MathSqrt(4.0)) == 1u);
    FF_TEST_ASSERT(FF_MathIsFinite(FF_MathSqrt(-1.0)) == 0u); /* NaN */
    FF_TEST_ASSERT(FF_MathIsFinite(FF_MathLog(0.0)) == 0u);   /* -inf */
}

FF_TEST_MAIN(test_elementary, test_helpers_and_domain)
