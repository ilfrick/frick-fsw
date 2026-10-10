/*
 * frick-fsw - CCSDS Space Packet codec
 *
 * Encodes and decodes CCSDS Space Packets (CCSDS 133.0-B-2) carrying a PUS
 * secondary header. An optional CRC-16/CCITT trailer is appended to every
 * packet, controlled by FF_FRAME_WITH_CRC.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_CCSDS_H
#define FF_CCSDS_H

#include "common/ff_types.h"
#include "fsw_config.h"
#include "pus/pus_types.h"

#define FF_CCSDS_HDR_LEN 6u   /* CCSDS primary header          */
#define FF_CCSDS_TM_PACKET 0u /* telemetry (bit 12 = 0)        */
#define FF_CCSDS_TC_PACKET 1u /* telecommand (bit 12 = 1)      */
#define FF_CCSDS_SEC_HDR_FLAG 1u

/* Total encoded size, useful for buffers. */
#define FF_CCSDS_MAX_TC_PACKET (FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN + FF_PUS_MAX_TC_FRAME + 2u)
#define FF_CCSDS_MAX_TM_PACKET (FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN + FF_PUS_MAX_TM_FRAME + 2u)

uint16_t FF_CcsdsCrc16(const uint8_t *data, uint16_t len);

FF_Status_t FF_CcsdsEncodeTc(uint16_t apid, const FF_Pus_TcHdr_t *hdr, const uint8_t *data,
                             uint16_t data_len, uint8_t *out, size_t out_cap, uint16_t *out_len);

FF_Status_t FF_CcsdsDecodeTc(const uint8_t *raw, uint16_t len, uint16_t *apid, FF_Pus_TcHdr_t *hdr,
                             const uint8_t **data, uint16_t *data_len);

FF_Status_t FF_CcsdsEncodeTm(uint16_t apid, const FF_Pus_TmHdr_t *hdr, const uint8_t *data,
                             uint16_t data_len, uint8_t *out, size_t out_cap, uint16_t *out_len);

FF_Status_t FF_CcsdsDecodeTm(const uint8_t *raw, uint16_t len, uint16_t *apid, FF_Pus_TmHdr_t *hdr,
                             const uint8_t **data, uint16_t *data_len);

#endif /* FF_CCSDS_H */
