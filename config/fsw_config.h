/*
 * frick-fsw - Master flight software configuration
 *
 * Every tunable of the framework lives here (or in a mission table), mirroring
 * the cFS philosophy of a single, reviewable configuration point.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_FSW_CONFIG_H
#define FF_FSW_CONFIG_H

#if defined(FRICK_PLATFORM_MPFS)
#include "platform/platform_mpfs.h"
#else
#include "platform/platform_native.h"
#endif

/* ------------------------------------------------------------------ */
/* Software Bus                                                        */
/* ------------------------------------------------------------------ */
#define FF_SB_MAX_PIPES 32u
#define FF_SB_MAX_SUBSCRIPTIONS 96u
#define FF_SB_MSG_PAYLOAD_MAX 512u
#define FF_SB_DEFAULT_DEPTH 16u
#define FF_SB_QUEUE_DEPTH_MAX 32u

/* Software Bus message identifiers (cFS style: 0xMMSS) */
#define FF_SB_WILDCARD 0xFFFFu

/* ------------------------------------------------------------------ */
/* Executive Services                                                  */
/* ------------------------------------------------------------------ */
#define FF_ES_MAX_APPS 32u
#define FF_ES_APP_NAME_LEN 16u
#define FF_ES_MAX_RESETS 4u

/* ------------------------------------------------------------------ */
/* Event Services                                                      */
/* ------------------------------------------------------------------ */
#define FF_EVS_LOG_DEPTH 64u
#define FF_EVS_TEXT_LEN 96u

/* Event identifiers (mission defined) */
#define FF_EVS_APP_STARTED 0x0101u
#define FF_EVS_APP_EXIT 0x0102u
#define FF_EVS_APP_ERROR 0x0103u
#define FF_EVS_STARTUP_ERR 0x0201u
#define FF_EVS_SB_ERR 0x0301u
#define FF_EVS_PUS_TC_ERR 0x0401u

/* ------------------------------------------------------------------ */
/* Table Services                                                      */
/* ------------------------------------------------------------------ */
#define FF_TBL_MAX_TABLES 32u
#define FF_TBL_NAME_LEN 16u
#define FF_TBL_MAX_SIZE 4096u

/* ------------------------------------------------------------------ */
/* Time Services                                                       */
/* ------------------------------------------------------------------ */
#define FF_TIME_MAX_CORRELATIONS 8u

/* One nominal second in the monotonic domain (tone period, 1 Hz). */
#define FF_TIME_TONE_NS 1000000000ULL
/* CCSDS Unsegmented Time Code tailoring: 4 coarse + 2 fine octets. */
#define FF_TIME_CUC_COARSE_OCTETS 4u
#define FF_TIME_CUC_FINE_OCTETS 2u
/* Leap-second schedule granularity (single seconds, ITU/ECSS practice). */
#define FF_TIME_CUC_MAX_LEAP_S 1
/* Software PPS backend tuning (host simulator / tests). */
#define FF_TIME_SW_TONE_PERIOD_MS 1000u
#define FF_TIME_TONE_TASK_PRIORITY 40u

/* ------------------------------------------------------------------ */
/* PUS                                                                 */
/* ------------------------------------------------------------------ */
#define FF_PUS_MAX_SERVICES 32u
#define FF_PUS_MAX_TM_FRAME 1024u
#define FF_PUS_MAX_TC_FRAME 1024u

/* PUS version implemented (ECSS-E-ST-70-41C is PUS-C, version 2) */
#define FF_PUS_VERSION 2u

/* Mission identifiers */
#define FF_MISSION_ID 0x01u
#define FF_APID_TC 0x010u
#define FF_APID_TM 0x020u

/* ------------------------------------------------------------------ */
/* Hardware abstraction layer (HAL, milestone M3)                     */
/* ------------------------------------------------------------------ */
/* Bounded registry capacity per resource class (no dynamic allocation). */
#define FF_HAL_MAX_BACKENDS 8u

/* Optional CRC-16/CCITT trailer on transfer frames */
#ifndef FF_FRAME_WITH_CRC
#define FF_FRAME_WITH_CRC 1
#endif

/* ------------------------------------------------------------------ */
/* General                                                             */
/* ------------------------------------------------------------------ */
#define FF_VERSION_STRING "0.1.0"
#define FF_CODENAME "Eärendil"

/* Enable runtime assertions in development builds */
#ifndef FF_ENABLE_ASSERT
#define FF_ENABLE_ASSERT 1
#endif

#endif /* FF_FSW_CONFIG_H */
