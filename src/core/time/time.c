/*
 * frick-fsw - Time Services implementation
 *
 * Three time sources are supported (cFS TIME heritage, milestone M2):
 *   - INTERNAL: UTC = monotonic clock + ground correlation (the baseline),
 *   - TONE:     an external 1 Hz tone latches the monotonic counter and the
 *               UTC interval between two tones is forced to exactly one
 *               second (oscillator drift is bounded per tone),
 *   - GROUND:   the correlation is updated by ground (PUS service 9,1).
 * Leap-second changes are scheduled and applied at the tone boundary.
 *
 * The tone state is a multi-word structure written by FF_TimeToneSignal
 * (task context only: the software backend task or a task fed by the board
 * capture ISR; a mutex is not ISR-safe, so the ISR defers to a task) and read
 * by the consumers; a PAL mutex makes every state transition and read atomic
 * (norm N1/N3, R1 review NC-03).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "time.h"
#include "core/pal/pal.h"
#include "time/pps.h"

/* CUC tailoring macros are configuration points: enforce the values that the
 * CCSDS 301.0-B-4 P-field 0x2E implies (4 coarse + 2 fine octets) and the
 * single-second leap granularity, so the frame layout cannot silently drift
 * from the configuration (R1 review NC-05).
 */
FF_STATIC_ASSERT(FF_TIME_CUC_COARSE_OCTETS == 4u, cuc_coarse_octets_is_four);
FF_STATIC_ASSERT(FF_TIME_CUC_FINE_OCTETS == 2u, cuc_fine_octets_is_two);
FF_STATIC_ASSERT(FF_TIME_CUC_MAX_LEAP_S == 1, cuc_max_leap_is_one_second);

static FF_MutexId_t g_time_mtx = FF_INVALID_ID;
static int16_t g_leap_seconds;
static int16_t g_pending_leap;
static FF_TimeSource_t g_source;
static bool g_tone_valid;
static uint64_t g_tone_epoch_ns;
static uint64_t g_tone_utc_ns;
static uint64_t g_tone_count;

static void ff_time_lock(void)
{
    if (g_time_mtx != FF_INVALID_ID) {
        (void)FF_PalMutexLock(g_time_mtx, FF_PAL_WAIT_FOREVER);
    }
}

static void ff_time_unlock(void)
{
    if (g_time_mtx != FF_INVALID_ID) {
        (void)FF_PalMutexUnlock(g_time_mtx);
    }
}

/* Raw UTC computation; the caller must hold g_time_mtx. Never nests, so it is
 * safe to call from both FF_TimeGetUtcNs and FF_TimeGetCuc. */
static uint64_t ff_time_utc_ns_locked(void)
{
    if ((g_source == FF_TIME_SRC_TONE) && g_tone_valid) {
        uint64_t now = FF_PalGetMonotonicNs();
        if (now >= g_tone_epoch_ns) {
            /* Time since the last tone in the monotonic domain, scaled to the
             * disciplined UTC epoch: intervals between tones are one second. */
            return g_tone_utc_ns + (now - g_tone_epoch_ns);
        }
    }
    return FF_PalGetUtcNs();
}

FF_Status_t FF_TimeInit(void)
{
    FF_Status_t st;

    if (g_time_mtx == FF_INVALID_ID) {
        st = FF_PalMutexCreate(&g_time_mtx, "time_core");
        if (st != FF_SUCCESS) {
            return st;
        }
    }
    ff_time_lock();
    g_leap_seconds = 0;
    g_pending_leap = 0;
    g_source = FF_TIME_SRC_INTERNAL;
    g_tone_valid = false;
    g_tone_epoch_ns = 0u;
    g_tone_utc_ns = 0u;
    g_tone_count = 0u;
    /* Correlate UTC with a mission defined epoch at boot (here: Unix epoch 0). */
    st = FF_PalSetUtcNs(0u);
    ff_time_unlock();
    return st;
}

uint64_t FF_TimeGetMetNs(void)
{
    return FF_PalGetMonotonicNs();
}

uint32_t FF_TimeGetMetMs(void)
{
    return (uint32_t)(FF_PalGetMonotonicNs() / 1000000ULL);
}

uint64_t FF_TimeGetUtcNs(void)
{
    uint64_t utc;

    ff_time_lock();
    utc = ff_time_utc_ns_locked();
    ff_time_unlock();
    return utc;
}

FF_Status_t FF_TimeSetUtcNs(uint64_t utc_ns)
{
    ff_time_lock();
    if (g_source == FF_TIME_SRC_INTERNAL) {
        /* A ground/ops correlation implicitly selects the GROUND source. */
        g_source = FF_TIME_SRC_GROUND;
    }
    ff_time_unlock();
    return FF_PalSetUtcNs(utc_ns);
}

FF_Status_t FF_TimeSetSource(FF_TimeSource_t src)
{
    if ((src != FF_TIME_SRC_INTERNAL) && (src != FF_TIME_SRC_TONE) && (src != FF_TIME_SRC_GROUND)) {
        return FF_INVALID_ARGUMENT;
    }
    ff_time_lock();
    g_source = src;
    ff_time_unlock();
    return FF_SUCCESS;
}

FF_TimeSource_t FF_TimeGetSource(void)
{
    FF_TimeSource_t src;

    ff_time_lock();
    src = g_source;
    ff_time_unlock();
    return src;
}

FF_Status_t FF_TimeToneSignal(uint64_t mono_ns)
{
    ff_time_lock();
    if (g_tone_valid && (mono_ns <= g_tone_epoch_ns)) {
        ff_time_unlock();
        return FF_INVALID_ARGUMENT; /* non-monotonic tone rejected */
    }
    if (!g_tone_valid) {
        /* First edge: latch the current correlation and establish the first
         * disciplined second boundary ("time at tone will be", cFS
         * CFE_MISSION_TIME_AT_TONE_WILL_BE): UTC just after the edge reads
         * latched + 1 s. Pinned by SRS-REQ-022 and the unit tests. */
        g_tone_utc_ns = FF_PalGetUtcNs();
    }
    g_tone_utc_ns += FF_TIME_TONE_NS; /* exactly one second per tone */
    g_tone_epoch_ns = mono_ns;
    g_tone_valid = true;
    g_tone_count++;

    /* Leap-second application at the tone boundary (cFS TIME heritage). */
    if (g_pending_leap != 0) {
        g_leap_seconds += g_pending_leap;
        g_pending_leap = 0;
    }
    ff_time_unlock();
    return FF_SUCCESS;
}

uint64_t FF_TimeGetToneCount(void)
{
    uint64_t count;

    ff_time_lock();
    count = g_tone_count;
    ff_time_unlock();
    return count;
}

FF_Status_t FF_TimeScheduleLeap(int16_t delta_s)
{
    if ((delta_s > FF_TIME_CUC_MAX_LEAP_S) || (delta_s < -FF_TIME_CUC_MAX_LEAP_S)) {
        return FF_INVALID_ARGUMENT; /* leap changes are single seconds */
    }
    ff_time_lock();
    g_pending_leap = delta_s;
    ff_time_unlock();
    return FF_SUCCESS;
}

FF_Status_t FF_TimeGetPendingLeap(int16_t *delta_s)
{
    if (delta_s == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    ff_time_lock();
    *delta_s = g_pending_leap;
    ff_time_unlock();
    return FF_SUCCESS;
}

FF_Status_t FF_TimeGetCuc(uint32_t *coarse_s, uint16_t *fine)
{
    uint64_t ns;
    int16_t leap;

    ff_time_lock();
    ns = ff_time_utc_ns_locked();
    leap = g_leap_seconds;
    ff_time_unlock();

    if (coarse_s != NULL) {
        *coarse_s = (uint32_t)(ns / FF_TIME_TONE_NS) + (uint32_t)leap;
    }
    if (fine != NULL) {
        uint64_t frac = ns % FF_TIME_TONE_NS;
        *fine = (uint16_t)((frac * 65536ULL) / FF_TIME_TONE_NS);
    }
    return FF_SUCCESS;
}

FF_Status_t FF_TimeSetLeapSeconds(int16_t offset_s)
{
    ff_time_lock();
    g_leap_seconds = offset_s;
    ff_time_unlock();
    return FF_SUCCESS;
}

int16_t FF_TimeGetLeapSeconds(void)
{
    int16_t leap;

    ff_time_lock();
    leap = g_leap_seconds;
    ff_time_unlock();
    return leap;
}

FF_Status_t FF_TimeCucEncode(uint8_t *buf, uint16_t *len)
{
    uint32_t coarse;
    uint16_t fine;
    uint16_t i;

    if ((buf == NULL) || (len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (FF_TimeGetCuc(&coarse, &fine) != FF_SUCCESS) {
        return FF_ERROR;
    }
    /* Explicit 7-octet self-describing CUC (CCSDS 301.0-B-4): 1-octet P-field
     * followed by the coarse[4] + fine[2] T-field, big-endian. */
    buf[0] = FF_TIME_CUC_PFIELD;
    for (i = 0; i < FF_TIME_CUC_COARSE_OCTETS; i++) {
        buf[1u + i] = (uint8_t)(coarse >> (8u * (FF_TIME_CUC_COARSE_OCTETS - 1u - i)));
    }
    for (i = 0; i < FF_TIME_CUC_FINE_OCTETS; i++) {
        buf[1u + FF_TIME_CUC_COARSE_OCTETS + i] =
            (uint8_t)(fine >> (8u * (FF_TIME_CUC_FINE_OCTETS - 1u - i)));
    }
    *len = FF_TIME_CUC_LEN;
    return FF_SUCCESS;
}

FF_Status_t FF_TimeCucDecode(const uint8_t *buf, uint16_t len, uint32_t *coarse_s, uint16_t *fine)
{
    uint32_t coarse = 0u;
    uint16_t fine_out = 0u;
    uint16_t i;

    if ((buf == NULL) || (coarse_s == NULL) || (fine == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len != FF_TIME_CUC_LEN) {
        return FF_BAD_SIZE;
    }
    if (buf[0] != FF_TIME_CUC_PFIELD) {
        return FF_PARSE_ERROR;
    }
    for (i = 0; i < FF_TIME_CUC_COARSE_OCTETS; i++) {
        coarse = (coarse << 8) | (uint32_t)buf[1u + i];
    }
    for (i = 0; i < FF_TIME_CUC_FINE_OCTETS; i++) {
        fine_out = (uint16_t)((fine_out << 8) | (uint16_t)buf[1u + FF_TIME_CUC_COARSE_OCTETS + i]);
    }
    *coarse_s = coarse;
    *fine = fine_out;
    return FF_SUCCESS;
}