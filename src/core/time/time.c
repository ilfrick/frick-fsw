/*
 * frick-fsw - Time Services implementation
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "time.h"
#include "core/pal/pal.h"

static int16_t g_leap_seconds;

FF_Status_t FF_TimeInit(void)
{
    g_leap_seconds = 0;
    /* Correlate UTC with a mission defined epoch at boot (here: Unix epoch 0). */
    return FF_PalSetUtcNs(0u);
}

uint64_t FF_TimeGetMetNs(void) { return FF_PalGetMonotonicNs(); }
uint32_t FF_TimeGetMetMs(void) { return (uint32_t)(FF_PalGetMonotonicNs() / 1000000ULL); }
uint64_t FF_TimeGetUtcNs(void) { return FF_PalGetUtcNs(); }

FF_Status_t FF_TimeSetUtcNs(uint64_t utc_ns)
{
    return FF_PalSetUtcNs(utc_ns);
}

FF_Status_t FF_TimeGetCuc(uint32_t *coarse_s, uint16_t *fine)
{
    uint64_t ns = FF_TimeGetUtcNs();
    if (coarse_s != NULL)
    {
        *coarse_s = (uint32_t)(ns / 1000000000ULL) + (uint32_t)g_leap_seconds;
    }
    if (fine != NULL)
    {
        uint64_t frac = ns % 1000000000ULL;
        *fine = (uint16_t)((frac * 65536ULL) / 1000000000ULL);
    }
    return FF_SUCCESS;
}

FF_Status_t FF_TimeSetLeapSeconds(int16_t offset_s)
{
    g_leap_seconds = offset_s;
    return FF_SUCCESS;
}

int16_t FF_TimeGetLeapSeconds(void) { return g_leap_seconds; }
