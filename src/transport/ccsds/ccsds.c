/*
 * frick-fsw - CCSDS Space Packet codec implementation
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "ccsds.h"
#include <string.h>

/* ---- big endian helpers ------------------------------------------ */

static void ff_put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFFu);
}

static uint16_t ff_get_u16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static void ff_put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v & 0xFFu);
}

static uint32_t ff_get_u32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* ---- CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) --------------- */

uint16_t FF_CcsdsCrc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFu;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)((uint16_t)data[i] << 8);
        for (uint8_t b = 0; b < 8u; b++) {
            if (crc & 0x8000u) {
                crc = (uint16_t)((uint16_t)(crc << 1) ^ 0x1021u);
            }
            else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

/* ---- primary header ---------------------------------------------- */

static void ff_write_primary(uint8_t *out, uint8_t packet_type, uint16_t apid, uint16_t seq_count,
                             uint16_t packet_data_len)
{
    uint16_t w0 = (uint16_t)((FF_CCSDS_SEC_HDR_FLAG << 11) | (apid & 0x07FFu));
    w0 = (uint16_t)(w0 | ((uint16_t)packet_type << 12));
    ff_put_u16(&out[0], w0);
    ff_put_u16(&out[2], (uint16_t)(0xC000u | (seq_count & 0x3FFFu))); /* seq flags = standalone */
    ff_put_u16(&out[4], (uint16_t)(packet_data_len - 1u));
}

/* ---- telecommand ------------------------------------------------- */

FF_Status_t FF_CcsdsEncodeTc(uint16_t apid, const FF_Pus_TcHdr_t *hdr, const uint8_t *data,
                             uint16_t data_len, uint8_t *out, size_t out_cap, uint16_t *out_len)
{
    uint16_t total;
    uint8_t *sec;

    if ((hdr == NULL) || (out == NULL) || (out_len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((data == NULL) && (data_len != 0u)) {
        return FF_INVALID_ARGUMENT;
    }

    total = (uint16_t)(FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN + data_len +
                       (FF_FRAME_WITH_CRC ? 2u : 0u));
    if (out_cap < total) {
        return FF_BAD_SIZE;
    }

    ff_write_primary(out, FF_CCSDS_TC_PACKET, apid, 0u, (uint16_t)(total - FF_CCSDS_HDR_LEN));
    sec = &out[FF_CCSDS_HDR_LEN];
    sec[0] = (uint8_t)(((hdr->version & 0x0Fu) << 4) | (hdr->ack_flags & 0x0Fu));
    sec[1] = hdr->service;
    sec[2] = hdr->subservice;
    ff_put_u16(&sec[3], hdr->source_id);

    if (data_len > 0u) {
        memcpy(&out[FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN], data, data_len);
    }
    if (FF_FRAME_WITH_CRC) {
        uint16_t crc = FF_CcsdsCrc16(out, (uint16_t)(total - 2u));
        ff_put_u16(&out[total - 2u], crc);
    }
    *out_len = total;
    return FF_SUCCESS;
}

FF_Status_t FF_CcsdsDecodeTc(const uint8_t *raw, uint16_t len, uint16_t *apid, FF_Pus_TcHdr_t *hdr,
                             const uint8_t **data, uint16_t *data_len)
{
    uint16_t w0, pdl, sec_len = FF_PUS_TC_SEC_HDR_LEN;
    uint16_t total_data;
    const uint8_t *sec;

    if ((raw == NULL) || (hdr == NULL) || (len < FF_CCSDS_HDR_LEN + sec_len)) {
        return FF_PARSE_ERROR;
    }
    w0 = ff_get_u16(&raw[0]);
    if (((w0 >> 12) & 0x01u) != FF_CCSDS_TC_PACKET) {
        return FF_PARSE_ERROR; /* not a telecommand */
    }
    pdl = ff_get_u16(&raw[4]);
    total_data = (uint16_t)(pdl + 1u);
    if ((uint16_t)(FF_CCSDS_HDR_LEN + total_data) > len) {
        return FF_PARSE_ERROR;
    }
    if (FF_FRAME_WITH_CRC) {
        uint16_t expect = FF_CcsdsCrc16(raw, (uint16_t)(FF_CCSDS_HDR_LEN + total_data - 2u));
        uint16_t got = ff_get_u16(&raw[FF_CCSDS_HDR_LEN + total_data - 2u]);
        if (expect != got) {
            return FF_CRC_ERROR;
        }
    }
    sec = &raw[FF_CCSDS_HDR_LEN];
    hdr->version = (uint8_t)(sec[0] >> 4);
    hdr->ack_flags = (uint8_t)(sec[0] & 0x0Fu);
    hdr->service = sec[1];
    hdr->subservice = sec[2];
    hdr->source_id = ff_get_u16(&sec[3]);

    if (apid != NULL) {
        *apid = (uint16_t)(w0 & 0x07FFu);
    }
    if (data_len != NULL) {
        *data_len = (uint16_t)(total_data - sec_len - (FF_FRAME_WITH_CRC ? 2u : 0u));
    }
    if (data != NULL) {
        *data = &raw[FF_CCSDS_HDR_LEN + sec_len];
    }
    return FF_SUCCESS;
}

/* ---- telemetry --------------------------------------------------- */

FF_Status_t FF_CcsdsEncodeTm(uint16_t apid, const FF_Pus_TmHdr_t *hdr, const uint8_t *data,
                             uint16_t data_len, uint8_t *out, size_t out_cap, uint16_t *out_len)
{
    uint16_t total;
    uint8_t *sec;

    if ((hdr == NULL) || (out == NULL) || (out_len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((data == NULL) && (data_len != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    total = (uint16_t)(FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN + data_len +
                       (FF_FRAME_WITH_CRC ? 2u : 0u));
    if (out_cap < total) {
        return FF_BAD_SIZE;
    }
    ff_write_primary(out, FF_CCSDS_TM_PACKET, apid, 0u, (uint16_t)(total - FF_CCSDS_HDR_LEN));
    sec = &out[FF_CCSDS_HDR_LEN];
    sec[0] = (uint8_t)(((hdr->version & 0x0Fu) << 4) & 0xF0u);
    sec[1] = hdr->service;
    sec[2] = hdr->subservice;
    ff_put_u16(&sec[3], hdr->counter);
    ff_put_u16(&sec[5], hdr->destination_id);
    ff_put_u32(&sec[7], hdr->time_coarse_s);
    ff_put_u16(&sec[11], hdr->time_fine);

    if (data_len > 0u) {
        memcpy(&out[FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN], data, data_len);
    }
    if (FF_FRAME_WITH_CRC) {
        uint16_t crc = FF_CcsdsCrc16(out, (uint16_t)(total - 2u));
        ff_put_u16(&out[total - 2u], crc);
    }
    *out_len = total;
    return FF_SUCCESS;
}

FF_Status_t FF_CcsdsDecodeTm(const uint8_t *raw, uint16_t len, uint16_t *apid, FF_Pus_TmHdr_t *hdr,
                             const uint8_t **data, uint16_t *data_len)
{
    uint16_t w0, pdl, total_data;
    const uint8_t *sec;

    if ((raw == NULL) || (hdr == NULL) || (len < FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN)) {
        return FF_PARSE_ERROR;
    }
    w0 = ff_get_u16(&raw[0]);
    if (((w0 >> 12) & 0x01u) != FF_CCSDS_TM_PACKET) {
        return FF_PARSE_ERROR; /* not telemetry */
    }
    pdl = ff_get_u16(&raw[4]);
    total_data = (uint16_t)(pdl + 1u);
    if ((uint16_t)(FF_CCSDS_HDR_LEN + total_data) > len) {
        return FF_PARSE_ERROR;
    }
    if (FF_FRAME_WITH_CRC) {
        uint16_t expect = FF_CcsdsCrc16(raw, (uint16_t)(FF_CCSDS_HDR_LEN + total_data - 2u));
        uint16_t got = ff_get_u16(&raw[FF_CCSDS_HDR_LEN + total_data - 2u]);
        if (expect != got) {
            return FF_CRC_ERROR;
        }
    }
    sec = &raw[FF_CCSDS_HDR_LEN];
    hdr->version = (uint8_t)(sec[0] >> 4);
    hdr->service = sec[1];
    hdr->subservice = sec[2];
    hdr->counter = ff_get_u16(&sec[3]);
    hdr->destination_id = ff_get_u16(&sec[5]);
    hdr->time_coarse_s = ff_get_u32(&sec[7]);
    hdr->time_fine = ff_get_u16(&sec[11]);

    if (apid != NULL) {
        *apid = (uint16_t)(w0 & 0x07FFu);
    }
    if (data_len != NULL) {
        *data_len = (uint16_t)(total_data - FF_PUS_TM_SEC_HDR_LEN - (FF_FRAME_WITH_CRC ? 2u : 0u));
    }
    if (data != NULL) {
        *data = &raw[FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN];
    }
    return FF_SUCCESS;
}
