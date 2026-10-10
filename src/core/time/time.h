/*
 * frick-fsw - Time Services
 *
 * Monotonic mission elapsed time plus a settable UTC correlation, exporting
 * CCSDS Unsegmented Time Code (CUC) used inside PUS secondary headers.
 * Analogue of the cFS Time Services.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_TIME_H
#define FF_TIME_H

#include "common/ff_types.h"
#include "fsw_config.h"

FF_Status_t FF_TimeInit(void);

/* Mission Elapsed Time / monotonic nanoseconds since boot. */
uint64_t FF_TimeGetMetNs(void);
uint32_t FF_TimeGetMetMs(void);

/* Correlated UTC time (nanoseconds since the Unix epoch). */
uint64_t FF_TimeGetUtcNs(void);
FF_Status_t FF_TimeSetUtcNs(uint64_t utc_ns);

/* CUC coarse (seconds, 32 bit) plus fine (fraction, 16 bit). */
FF_Status_t FF_TimeGetCuc(uint32_t *coarse_s, uint16_t *fine);

/* Leap second announcement (seconds offset, 0 when unknown). */
FF_Status_t FF_TimeSetLeapSeconds(int16_t offset_s);
int16_t FF_TimeGetLeapSeconds(void);

#endif /* FF_TIME_H */
