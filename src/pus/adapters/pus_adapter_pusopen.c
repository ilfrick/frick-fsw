/*
 * frick-fsw - PUSopen(R) commercial backend adapter
 *
 * PUSopen (https://pusopen.com) is a commercial, MISRA-C coded CCSDS/PUS
 * library with space heritage. This file is the integration point: it is only
 * compiled when FRICK_USE_PUSOPEN=1 and the vendor headers/library are present.
 *
 * The commercial sources are NOT part of this repository. See
 * docs/architecture/pus-layer.md, section "PUSopen integration", for how to
 * drop the PUSopen SDK into third_party/pusopen/ and enable the option.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"

#if defined(FRICK_USE_PUSOPEN) && (FRICK_USE_PUSOPEN == 1)

#include <pusopen.h> /* provided by the PUSopen SDK */

static FF_Status_t pusopen_init(void)
{
    /* PUSopen_Config_t cfg = {0}; cfg.source_id = ...; */
    return (PUSopen_Init(/*&cfg*/) == PUSOPEN_OK) ? FF_SUCCESS : FF_ERROR;
}

static FF_Status_t pusopen_encode_tm(const FF_Pus_TmHdr_t *hdr, const uint8_t *data,
                                     uint16_t data_len, uint8_t *out, size_t out_cap,
                                     uint16_t *out_len)
{
    PUSopen_Packet_t pkt = {0};
    size_t written = 0u;

    if ((hdr == NULL) || (out == NULL) || (out_len == NULL) ||
        ((data == NULL) && (data_len != 0u))) {
        return FF_INVALID_ARGUMENT;
    }
    pkt.service = hdr->service;
    pkt.subservice = hdr->subservice;
    pkt.counter = hdr->counter;
    pkt.time_s = hdr->time_coarse_s;
    pkt.time_f = hdr->time_fine;
    pkt.data = (uint8_t *)data;
    pkt.data_len = data_len;

    if (PUSopen_EncodeTm(&pkt, out, out_cap, &written) != PUSOPEN_OK) {
        return FF_ERROR;
    }
    if ((written > out_cap) || (written > UINT16_MAX)) {
        return FF_BAD_SIZE;
    }
    *out_len = (uint16_t)written;
    return FF_SUCCESS;
}

static FF_Status_t pusopen_decode_tc(const uint8_t *raw, uint16_t len, uint16_t *apid,
                                     FF_Pus_TcHdr_t *hdr, const uint8_t **data, uint16_t *data_len)
{
    PUSopen_Packet_t pkt = {0};
    uint16_t primary;
    uint16_t packet_apid;

    if ((raw == NULL) || (hdr == NULL) || (len < FF_CCSDS_HDR_LEN)) {
        return FF_PARSE_ERROR;
    }
    primary = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    if (((primary >> 12) & 0x01u) != FF_CCSDS_TC_PACKET) {
        return FF_PARSE_ERROR;
    }
    packet_apid = (uint16_t)(primary & 0x07FFu);
    if (packet_apid != FF_APID_TC) {
        return FF_PARSE_ERROR; /* do not replace a received APID with the expected value */
    }
    if (PUSopen_DecodeTc(raw, len, &pkt) != PUSOPEN_OK) {
        return FF_PARSE_ERROR;
    }
    if ((pkt.data_len > UINT16_MAX) || ((pkt.data == NULL) && (pkt.data_len != 0u))) {
        return FF_BAD_SIZE;
    }
    hdr->version = FF_PUS_VERSION;
    hdr->ack_flags = pkt.ack_flags;
    hdr->service = pkt.service;
    hdr->subservice = pkt.subservice;
    hdr->source_id = pkt.source_id;
    if (apid != NULL) {
        *apid = packet_apid;
    }
    if (data != NULL) {
        *data = pkt.data;
    }
    if (data_len != NULL) {
        *data_len = (uint16_t)pkt.data_len;
    }
    return FF_SUCCESS;
}

const FF_Pus_Backend_t FF_PusBackendPusopen = {
    .name = "pusopen",
    .init = pusopen_init,
    .encode_tm = pusopen_encode_tm,
    .decode_tc = pusopen_decode_tc,
};

#endif /* FRICK_USE_PUSOPEN */
