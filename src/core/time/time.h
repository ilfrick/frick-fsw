/*
 * frick-fsw - Time Services
 *
 * Monotonic mission elapsed time plus a settable UTC correlation, exporting
 * CCSDS Unsegmented Time Code (CUC) used inside PUS secondary headers.
 * Analogue of the cFS Time Services, extended with a PPS / 1 Hz tone
 * discipline and leap-second scheduling (milestone M2).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_TIME_H
#define FF_TIME_H

#include "common/ff_types.h"
#include "fsw_config.h"

/* ------------------------------------------------------------------ */
/* Time source selection (SRS-REQ-021, cFS TIME heritage)              */
/* ------------------------------------------------------------------ */
typedef enum {
    FF_TIME_SRC_INTERNAL = 0, /* CPU clock only: UTC = MET + ground correlation */
    FF_TIME_SRC_TONE = 1,     /* external 1 Hz tone disciplining the time base   */
    FF_TIME_SRC_GROUND = 2    /* correlation updated from ground (PUS service 9) */
} FF_TimeSource_t;

FF_Status_t FF_TimeInit(void);

/* Mission Elapsed Time / monotonic nanoseconds since boot. */
uint64_t FF_TimeGetMetNs(void);
uint32_t FF_TimeGetMetMs(void);

/* Correlated UTC time (nanoseconds since the mission epoch). */
uint64_t FF_TimeGetUtcNs(void);
FF_Status_t FF_TimeSetUtcNs(uint64_t utc_ns);

/* Time source steering. */
FF_Status_t FF_TimeSetSource(FF_TimeSource_t src);
FF_TimeSource_t FF_TimeGetSource(void);

/* PPS / 1 Hz tone handling (SRS-REQ-022).
 * Called from the PPS backend / driver with the monotonic counter latched at
 * the tone edge; see src/core/time/pps.h and the driver development guide. */
FF_Status_t FF_TimeToneSignal(uint64_t mono_ns);

/* Leap second scheduling (SRS-REQ-023): the scheduled offset-change is applied
 * at the next tone boundary and then cleared. Only -1, 0 and +1 are legal. */
FF_Status_t FF_TimeScheduleLeap(int16_t delta_s);
FF_Status_t FF_TimeGetPendingLeap(int16_t *delta_s);

/* CUC coarse (seconds, 32 bit) plus fine (fraction, 16 bit). */
FF_Status_t FF_TimeGetCuc(uint32_t *coarse_s, uint16_t *fine);

/* Leap second offset (TAI - UTC correlation, 0 when unknown). */
FF_Status_t FF_TimeSetLeapSeconds(int16_t offset_s);
int16_t FF_TimeGetLeapSeconds(void);

/* Tone diagnostics / housekeeping data. */
uint64_t FF_TimeGetToneCount(void);

/* ------------------------------------------------------------------ */
/* CCSDS Unsegmented Time Code byte framing (SRS-REQ-024)              */
/*                                                                     */
/* P-field: 0x2E = unsegmented | agency-defined epoch | 4 coarse octets| */
/*           2 fine octets (CCSDS 301.0-B-4, section 3.2.2)            */
/* T-field: coarse[4] big-endian (seconds) + fine[2] big-endian        */
/*          (binary fraction of a second, 2^-16 s)                     */
/* ------------------------------------------------------------------ */
#define FF_TIME_CUC_LEN (1u + FF_TIME_CUC_COARSE_OCTETS + FF_TIME_CUC_FINE_OCTETS)
#define FF_TIME_CUC_PFIELD 0x2Eu

FF_Status_t FF_TimeCucEncode(uint8_t *buf, uint16_t *len);
FF_Status_t FF_TimeCucDecode(const uint8_t *buf, uint16_t len, uint32_t *coarse_s, uint16_t *fine);

#endif /* FF_TIME_H */
