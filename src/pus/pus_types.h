/*
 * frick-fsw - PUS packet types (ECSS-E-ST-70-41C / PUS-C)
 *
 * These structures describe the PUS secondary headers carried inside CCSDS
 * Space Packets. The exact layout is a mission tailoring; the fields below
 * follow the reference tailoring documented in docs/interfaces/icd-ground.md.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_PUS_TYPES_H
#define FF_PUS_TYPES_H

#include "common/ff_types.h"

/* --- PUS version -------------------------------------------------- */
#define FF_PUS_VER_1            1u  /* PUS-A */
#define FF_PUS_VER_2            2u  /* PUS-C */

/* --- TC acknowledgement flags (PUS-C, TC secondary header) -------- */
#define FF_PUS_ACK_ACCEPTANCE   0x01u
#define FF_PUS_ACK_START        0x02u
#define FF_PUS_ACK_PROGRESS     0x04u
#define FF_PUS_ACK_COMPLETION   0x08u

/* --- PUS service identifiers -------------------------------------- */
#define FF_PUS_SERVICE_VERIFICATION   1u
#define FF_PUS_SERVICE_HK             3u
#define FF_PUS_SERVICE_MEMORY         6u
#define FF_PUS_SERVICE_FUNCTION       8u
#define FF_PUS_SERVICE_TIME           9u
#define FF_PUS_SERVICE_SCHEDULE       11u
#define FF_PUS_SERVICE_EVENT          5u
#define FF_PUS_SERVICE_TEST           17u
#define FF_PUS_SERVICE_PARAMETER      20u

/* --- Verification (service 1) failure codes ----------------------- */
#define FF_PUS_FAIL_ILLEGAL_APID        1u
#define FF_PUS_FAIL_ILLEGAL_LENGTH      2u
#define FF_PUS_FAIL_CRC                4u
#define FF_PUS_FAIL_UNKNOWN_SERVICE     8u
#define FF_PUS_FAIL_UNKNOWN_SUBSERVICE  9u
#define FF_PUS_FAIL_BAD_DATA           12u
#define FF_PUS_FAIL_EXECUTION          20u

/* TC secondary header: version(4b)+ack(4b), service, subservice, source id */
typedef struct
{
    uint8_t  version;
    uint8_t  ack_flags;
    uint8_t  service;
    uint8_t  subservice;
    uint16_t source_id;
} FF_Pus_TcHdr_t;

/* TM secondary header: version+spare, service, subservice, counter,
 * destination id, CUC coarse (4) + fine (2). */
typedef struct
{
    uint8_t  version;
    uint8_t  service;
    uint8_t  subservice;
    uint16_t counter;
    uint16_t destination_id;
    uint32_t time_coarse_s;
    uint16_t time_fine;
} FF_Pus_TmHdr_t;

#define FF_PUS_TC_SEC_HDR_LEN 5u
#define FF_PUS_TM_SEC_HDR_LEN 13u
#define FF_PUS_TC_VERSION_OFF 0u /* documentational offsets */

/* Verification stages reported by the framework (PUS service 1) */
typedef enum
{
    FF_PUS_STAGE_ACCEPTANCE = 1,
    FF_PUS_STAGE_START      = 2,
    FF_PUS_STAGE_PROGRESS   = 4,
    FF_PUS_STAGE_COMPLETION = 8
} FF_Pus_Stage_t;

#endif /* FF_PUS_TYPES_H */
