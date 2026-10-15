/*
 * frick-fsw - Time Services unit tests (milestone M2: PPS tone, leap, CUC)
 *
 * Determinism rules (R1 F1 / R2 M2-06): never assert fine/coarse equality
 * across two separate wall-clock reads. Round-trips are anchored at a fixed
 * UTC value captured at the instant of set, the byte decode runs on the fixed
 * reference buffer, and tone edges are always freshly sampled monotonic
 * timestamps (never future-dated).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/time/time.h"
#include "core/time/pps.h"
#include "core/pal/pal.h"
#include <string.h>

/* Freshly sample a monotonic timestamp strictly later than *prev. Never
 * future-dates: the exported TONE-mode UTC reads (now - epoch) against the
 * same clock, so a future-dated edge would fall back to raw PAL UTC. */
static uint64_t later_than(uint64_t prev)
{
    uint64_t t = FF_PalGetMonotonicNs();
    while (t <= prev) {
        t = FF_PalGetMonotonicNs();
    }
    return t;
}

static void test_init_and_basic_cuc(void)
{
    uint32_t coarse = 999u;
    uint16_t fine = 999u;
    int16_t pend = 99;

    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetSource() == FF_TIME_SRC_INTERNAL);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == 0);
    FF_TEST_ASSERT(FF_TimeGetToneCount() == 0u);
    FF_TEST_ASSERT(FF_TimeGetPendingLeap(&pend) == FF_SUCCESS);
    FF_TEST_ASSERT(pend == 0);
    /* Coarse is deterministic (0 whole seconds since boot); the fine fraction
     * is wall-clock sensitive and is exercised only through byte round-trip. */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 0u);
    /* The fine fraction is a uint16_t: a `< 65536` range assert is tautological
     * (and -Wtype-limits rejects it); its lossless recovery is covered by the
     * CUC byte round-trip in test_cuc_encode_decode below. */
}

static void test_utc_and_leap_correlation(void)
{
    uint32_t coarse;
    uint16_t fine;

    FF_TimeInit();
    FF_TEST_ASSERT(FF_TimeSetUtcNs(FF_TIME_TONE_NS) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetSource() == FF_TIME_SRC_GROUND); /* implicit switch */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1u);

    FF_TEST_ASSERT(FF_TimeSetLeapSeconds(37) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == 37);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 38u);

    /* CUC coarse tracks the correlation (service 9,1); the fine fraction is
     * environment-timing sensitive, so it is exercised via byte round-trip. */
    FF_TimeSetUtcNs(5u * FF_TIME_TONE_NS + 250000000ULL); /* 5.25 s */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 5u + 37u);
}

static void test_cuc_bytes(void)
{
    uint8_t buf[FF_TIME_CUC_LEN];
    uint16_t len = 0u;
    uint32_t c1 = 0u, c1b = 0u, c2 = 0u;
    uint16_t f1 = 0u, f1b = 0u, f2 = 0u;

    FF_TimeInit();
    /* Anchor the round-trip at a fixed coarse value: from set to encode the
     * elapsed time is a sub-second fraction, so coarse is exact and the fine
     * field is the value captured at the very same instant. */
    FF_TimeSetUtcNs(5u * FF_TIME_TONE_NS);

    FF_TEST_ASSERT(FF_TimeCucEncode(NULL, &len) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeCucEncode(buf, NULL) == FF_INVALID_ARGUMENT);

    /* Encode is bit-exact: P-field + big-endian T-field, owned by the
     * payload length FF_TIME_CUC_LEN (7 octets). */
    FF_TEST_ASSERT(FF_TimeCucEncode(buf, &len) == FF_SUCCESS);
    FF_TEST_ASSERT(len == 7u);
    FF_TEST_ASSERT(buf[0] == FF_TIME_CUC_PFIELD);

    /* Lossless same-instant round-trip: decode(encode(now)) recovers the
     * coarse value set above and a valid sub-second fine fraction, without
     * any second wall-clock read. */
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, len, &c1, &f1) == FF_SUCCESS);
    FF_TEST_ASSERT(c1 == 5u);
    /* f1 is a uint16_t fine fraction (range implied by the type, so no
     * `< 65536` assert; -Wtype-limits rejects that comparison). */

    /* Decode is deterministic on a given buffer. */
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, len, &c1b, &f1b) == FF_SUCCESS);
    FF_TEST_ASSERT(c1b == c1 && f1b == f1);

    /* Deterministic byte-level decode of a fixed, valid reference buffer. */
    {
        const uint8_t fixed[FF_TIME_CUC_LEN] = {
            FF_TIME_CUC_PFIELD, 0x00, 0x00, 0x00, 0x05, 0x40, 0x00}; /* 5 s + 0.25 s */
        FF_TEST_ASSERT(FF_TimeCucDecode(fixed, sizeof(fixed), &c2, &f2) == FF_SUCCESS);
        FF_TEST_ASSERT(c2 == 5u);
        FF_TEST_ASSERT(f2 == 16384u);
    }

    /* decode rejects wrong length, wrong P-field and null arguments */
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, 6u, &c1, &f1) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, 8u, &c1, &f1) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_TimeCucDecode(NULL, len, &c1, &f1) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, len, NULL, &f1) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, len, &c1, NULL) == FF_INVALID_ARGUMENT);
    buf[0] = 0x00u;
    FF_TEST_ASSERT(FF_TimeCucDecode(buf, len, &c1, &f1) == FF_PARSE_ERROR);
}

static void test_source_selection(void)
{
    FF_TimeInit();
    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_TONE) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetSource() == FF_TIME_SRC_TONE);
    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_GROUND) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_INTERNAL) == FF_SUCCESS);
    /* out-of-enum value rejected */
    FF_TEST_ASSERT(FF_TimeSetSource((FF_TimeSource_t)99) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeGetSource() == FF_TIME_SRC_INTERNAL);
}

static void test_tone_discipline(void)
{
    uint64_t now, now2;
    uint32_t coarse;
    uint16_t fine;

    FF_TimeInit();
    now = FF_PalGetMonotonicNs();

    FF_TEST_ASSERT(FF_TimeToneSignal(now) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeToneSignal(now) == FF_INVALID_ARGUMENT); /* must strictly increase */
    FF_TEST_ASSERT(FF_TimeToneSignal(now - 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeGetToneCount() == 1u);

    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_TONE) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    /* One disciplined second: the first edge establishes the first second
     * boundary (latched correlation + 1 s), per SRS-REQ-022. */
    FF_TEST_ASSERT(coarse == 1u);

    /* A second edge on a freshly sampled (strictly later) timestamp advances
     * the tone second counter by exactly one more. */
    now2 = later_than(now);
    FF_TEST_ASSERT(FF_TimeToneSignal(now2) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetToneCount() == 2u);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 2u);
}

static void test_leap_at_tone_boundary(void)
{
    uint64_t now;
    int16_t pend = 0;
    uint32_t coarse;
    uint16_t fine;

    FF_TimeInit();
    /* scheduling is restricted to single seconds (FF_TIME_CUC_MAX_LEAP_S) */
    FF_TEST_ASSERT(FF_TimeScheduleLeap((int16_t)(FF_TIME_CUC_MAX_LEAP_S + 1)) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeScheduleLeap((int16_t)(-FF_TIME_CUC_MAX_LEAP_S - 1)) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_TimeScheduleLeap(0) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeScheduleLeap(1) == FF_SUCCESS);
    FF_TimeGetPendingLeap(&pend);
    FF_TEST_ASSERT(pend == 1);

    now = FF_PalGetMonotonicNs();
    FF_TEST_ASSERT(FF_TimeToneSignal(now) == FF_SUCCESS);

    /* applied and cleared exactly at the boundary */
    FF_TimeGetPendingLeap(&pend);
    FF_TEST_ASSERT(pend == 0);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == 1);

    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_TONE) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    /* coarse = 1 tone second + 1 leap second */
    FF_TEST_ASSERT(coarse == 2u);
}

/* A scheduled leap stays pending when no tone ever arrives: it is silently
 * inert and is applied only at the first valid tone that follows (SRS-REQ-023
 * no-tone clause, R3 F-4). */
static void test_leap_pending_without_tone(void)
{
    int16_t pend = 0;
    uint32_t coarse;
    uint16_t fine;
    uint64_t now;

    FF_TimeInit();
    FF_TEST_ASSERT(FF_TimeScheduleLeap(1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetPendingLeap(&pend) == FF_SUCCESS);
    FF_TEST_ASSERT(pend == 1);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == 0); /* never applied without a tone */
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 0u);

    now = FF_PalGetMonotonicNs();
    FF_TEST_ASSERT(FF_TimeToneSignal(now) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == 1); /* applied at the first tone */
    FF_TEST_ASSERT(FF_TimeGetPendingLeap(&pend) == FF_SUCCESS);
    FF_TEST_ASSERT(pend == 0);
}

/* Exported CUC seconds never regress across a tone boundary that carries a
 * non-negative scheduled leap (Lean req_srs021_utc_non_regression, R1 C2). */
static void test_leap_non_regression_at_boundary(void)
{
    uint32_t coarse;
    uint16_t fine;
    uint64_t t1, t2;

    FF_TimeInit();
    t1 = FF_PalGetMonotonicNs();
    FF_TEST_ASSERT(FF_TimeToneSignal(t1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_TONE) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1u);

    FF_TEST_ASSERT(FF_TimeScheduleLeap(1) == FF_SUCCESS);
    t2 = later_than(t1);
    FF_TEST_ASSERT(FF_TimeToneSignal(t2) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    /* 2 tone seconds + 1 unapplied-before +1 leap, cleared at the boundary */
    FF_TEST_ASSERT(coarse == 3u);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == 1);
}

/* A negative leap implements the ITU-skipped second: the tone epoch advances
 * but the exported CUC second does not (R1 C2 additional coverage). */
static void test_leap_negative_skipped_second(void)
{
    uint32_t coarse;
    uint16_t fine;
    uint64_t t1, t2;

    FF_TimeInit();
    t1 = FF_PalGetMonotonicNs();
    FF_TEST_ASSERT(FF_TimeToneSignal(t1) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeSetSource(FF_TIME_SRC_TONE) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1u);

    FF_TEST_ASSERT(FF_TimeScheduleLeap(-1) == FF_SUCCESS);
    t2 = later_than(t1);
    FF_TEST_ASSERT(FF_TimeToneSignal(t2) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeGetLeapSeconds() == -1);
    FF_TEST_ASSERT(FF_TimeGetCuc(&coarse, &fine) == FF_SUCCESS);
    FF_TEST_ASSERT(coarse == 1u); /* second skipped */
}

static void test_pps_backend(void)
{
    FF_TimeInit();
    FF_TEST_ASSERT(FF_PpsRegister(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_PpsStart() == FF_NOT_READY); /* nothing registered yet */
    FF_TEST_ASSERT(FF_PpsStop() == FF_NOT_READY);
    FF_TEST_ASSERT(FF_PpsRegister(&FF_PpsSwBackend) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PpsRegister(&FF_PpsSwBackend) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(FF_PpsStart() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PpsStop() == FF_SUCCESS);
}

FF_TEST_MAIN(test_init_and_basic_cuc, test_utc_and_leap_correlation, test_cuc_bytes,
             test_source_selection, test_tone_discipline, test_leap_at_tone_boundary,
             test_leap_pending_without_tone, test_leap_non_regression_at_boundary,
             test_leap_negative_skipped_second, test_pps_backend)
