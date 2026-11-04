/*
 * frick-fsw - Frozen UTC seam stress/quality tests (NC-EXT-235)
 *
 * Verifies the compile-time-guarded frozen UTC seam (FF_TEST_HOOKS) behaves
 * correctly under stress, multiple freeze/unfreeze cycles, and edge cases.
 * The seam exists only in test builds: FF_TEST_HOOKS is defined for
 * FF_BUILD_TESTS on non-RTEMS platforms and is absent from flight/RTEMS
 * builds. It changes no production timing behaviour when unfrozen.
 *
 * The frozen UTC seam is designed to freeze time DURING a test, not to
 * permanently change the clock. After unfreeze, time resumes from the
 * original offset + elapsed monotonic time.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/time/time.h"
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

static void init_modules(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
}

/* ------------------------------------------------------------------ */
/* 1. Basic freeze/unfreeze cycle                                     */
/* ------------------------------------------------------------------ */

static void test_basic_freeze_unfreeze(void)
{
    init_modules();

    /* Freeze at a known time */
    FF_PalTimeTestFreeze(2000000000000ULL); /* 2000 s */
    uint64_t frozen_utc = FF_PalGetUtcNs();
    FF_TEST_ASSERT(frozen_utc == 2000000000000ULL);

    /* Time should not advance while frozen */
    for (int i = 0; i < 100; i++) {
        FF_TEST_ASSERT(FF_PalGetUtcNs() == 2000000000000ULL);
    }

    /* Unfreeze - time resumes from original offset (not frozen value) */
    FF_PalTimeTestUnfreeze();
    uint64_t after_unfreeze = FF_PalGetUtcNs();
    /* Should NOT be the frozen value */
    FF_TEST_ASSERT(after_unfreeze != 2000000000000ULL);
    /* Should be a valid timestamp */
    FF_TEST_ASSERT(after_unfreeze > 0);
}

/* ------------------------------------------------------------------ */
/* 2. Multiple freeze/unfreeze cycles                                 */
/* ------------------------------------------------------------------ */

static void test_multiple_cycles(void)
{
    uint64_t last_after = 0;
    for (int cycle = 0; cycle < 50; cycle++) {
        uint64_t freeze_time = 1000000000000ULL + (uint64_t)cycle * 1000000000ULL;
        FF_PalTimeTestFreeze(freeze_time);
        FF_TEST_ASSERT(FF_PalGetUtcNs() == freeze_time);
        FF_PalTimeTestUnfreeze();
        uint64_t after = FF_PalGetUtcNs();
        FF_TEST_ASSERT(after > 0);
        FF_TEST_ASSERT(after < 4000000000000ULL); /* Not absurdly far in future */
        /* Time should be monotonic (not decrease) */
        if (last_after > 0) {
            FF_TEST_ASSERT(after >= last_after);
        }
        /* Time should not jump to the frozen value */
        FF_TEST_ASSERT(after != 1000000000000ULL + (uint64_t)cycle * 1000000000ULL);
    }
}

/* ------------------------------------------------------------------ */
/* 3. Frozen time is quantized deterministically by FF_TimeGetCuc     */
/* ------------------------------------------------------------------ */

/* The frozen seam must make the production CUC conversion deterministic. This
 * sub-test exercises FF_TimeGetCuc itself (not local arithmetic): a frozen
 * whole-second instant must decode to an exact coarse second with a zero fine
 * fraction, and advancing the freeze must move the coarse field by exactly the
 * advanced number of seconds. The former "due logic" block was removed: it
 * asserted arithmetic on values the test had just set and called no production
 * due predicate (FRICK-REVIEW-0452 F-07). The periodic due predicate itself is
 * exercised end-to-end through FF_PusEmitPeriodicHk in tests/unit/test_pus.c. */
static void test_frozen_cuc_advance(void)
{
    uint32_t coarse = 0u;
    uint16_t fine = 0xFFFFu;

    FF_PalTimeTestFreeze(1000000000000ULL); /* 1000 s */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1000u);
    FF_TEST_ASSERT(fine == 0u);

    FF_PalTimeTestFreeze(1010000000000ULL); /* 1010 s */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1010u);
    FF_TEST_ASSERT(fine == 0u);

    /* A sub-second freeze must land in the fine field, not the coarse one:
     * 0.25 s of a 1 s second = 16384 in 2^-16 units. */
    FF_PalTimeTestFreeze(1010250000000ULL); /* 1010.25 s */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1010u);
    FF_TEST_ASSERT(fine == 16384u);

    FF_PalTimeTestUnfreeze();
}

/* ------------------------------------------------------------------ */
/* 4. Freeze with FF_TimeSetUtcNs interaction                         */
/* ------------------------------------------------------------------ */

static void test_freeze_with_setutcns(void)
{
    FF_PalTimeTestFreeze(5000000000000ULL); /* 5000 s */
    FF_TEST_ASSERT(FF_PalGetUtcNs() == 5000000000000ULL);

    /* FF_TimeSetUtcNs while frozen should update frozen value, not offset */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(6000000000000ULL) == FF_SUCCESS); /* 6000 s */
    FF_TEST_ASSERT(FF_PalGetUtcNs() == 6000000000000ULL);

    FF_PalTimeTestUnfreeze();
    uint64_t after = FF_PalGetUtcNs();
    FF_TEST_ASSERT(after > 0);
}

/* ------------------------------------------------------------------ */
/* 5. Stress: rapid freeze/unfreeze                                   */
/* ------------------------------------------------------------------ */

static void test_stress_rapid_cycles(void)
{
    uint64_t last_after = 0;
    for (int i = 0; i < 1000; i++) {
        uint64_t t = 1000000000000ULL + (uint64_t)i * 1000ULL;
        FF_PalTimeTestFreeze(t);
        FF_TEST_ASSERT(FF_PalGetUtcNs() == t);
        FF_PalTimeTestUnfreeze();
        uint64_t after = FF_PalGetUtcNs();
        FF_TEST_ASSERT(after > 0);
        FF_TEST_ASSERT(after < 4000000000000ULL); /* Not absurdly large */
        /* Time should be monotonic */
        if (last_after > 0) {
            FF_TEST_ASSERT(after >= last_after);
        }
        last_after = after;
    }
}

/* ------------------------------------------------------------------ */
/* 5b. Verify FF_TimeSetUtcNs updates frozen value when frozen        */
/* ------------------------------------------------------------------ */

static void test_setutcns_while_frozen(void)
{
    FF_PalTimeTestFreeze(1000000000000ULL);
    FF_TEST_ASSERT(FF_PalGetUtcNs() == 1000000000000ULL);

    /* FF_TimeSetUtcNs while frozen updates the frozen value */
    FF_TEST_ASSERT(FF_TimeSetUtcNs(2000000000000ULL) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalGetUtcNs() == 2000000000000ULL);

    FF_TEST_ASSERT(FF_TimeSetUtcNs(3000000000000ULL) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PalGetUtcNs() == 3000000000000ULL);

    FF_PalTimeTestUnfreeze();
}

/* ------------------------------------------------------------------ */
/* 6. Verify monotonic clock is unaffected by freeze                  */
/* ------------------------------------------------------------------ */

static void test_monotonic_unaffected(void)
{
    uint64_t mono_before = FF_PalGetMonotonicNs();
    FF_PalTimeTestFreeze(9999999999999ULL);
    uint64_t mono_during = FF_PalGetMonotonicNs();
    FF_PalTimeTestUnfreeze();
    uint64_t mono_after = FF_PalGetMonotonicNs();

    /* Monotonic should always advance */
    FF_TEST_ASSERT(mono_during >= mono_before);
    FF_TEST_ASSERT(mono_after >= mono_during);
    FF_TEST_ASSERT(mono_after > mono_before); /* Some time passed */
}

/* ------------------------------------------------------------------ */

FF_TEST_MAIN(test_basic_freeze_unfreeze, test_multiple_cycles, test_frozen_cuc_advance,
             test_freeze_with_setutcns, test_stress_rapid_cycles, test_setutcns_while_frozen,
             test_monotonic_unaffected)
