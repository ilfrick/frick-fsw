/*
 * frick-fsw - PUS-C ASN.1 (N7 Space / ESA) default backend (SRS-REQ-050)
 *
 * The default PUS backend. It is built on the N7 Space / ESA ASN.1/ACN PUS-C
 * model (`asn1-pusc-lib`, compiled with the ESA ASN1SCC compiler) and the
 * library's C wrappers. Concretely it uses:
 *   - the ASN1SCC-generated PUS-C types (`BasicTypes.h`) as the normative PUS-C
 *     field vocabulary/widths (compile-time assertion below), and
 *   - the N7/ESA packet error control implementation
 *     (`Asn1PuscLib_Crc_calculate`, ECSS-E-ST-70-41C Annex B.1) for the CCSDS
 *     packet error-control trailer on telemetry and, when enabled, TC decode,
 * while the CCSDS Space Packet framing itself is the reference codec.
 *
 * The deep per-service ASN.1 type-codec mapping and the fundamental service
 * behaviour are milestone M15 (see docs/architecture/pus-c-from-scratch-assessment.md).
 *
 * Compiled only with FF_USE_PUS_ASN1 (build-time generation; `make pus-asn1`).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"

#include "Crc.h"        /* N7/ESA wrapper: Asn1PuscLib_Crc_calculate */
#include "asn1crt.h"    /* ASN1SCC runtime (generated)                */
#include "BasicTypes.h" /* PUS-C types (generated)                    */

/* The ASN.1 model is the normative PUS-C vocabulary: tie the build to the
 * generated model via its on-the-wire field widths (ACN encoding sizes). */
FF_STATIC_ASSERT(Pusc_PUSC_UINT8_REQUIRED_BYTES_FOR_ACN_ENCODING == 1u,
                 pusc_uint8_is_one_wire_byte);
FF_STATIC_ASSERT(Pusc_PUSC_UINT16_REQUIRED_BYTES_FOR_ACN_ENCODING == 2u,
                 pusc_uint16_is_two_wire_bytes);

static FF_Status_t n7_init(void)
{
    return FF_SUCCESS;
}

/* Telemetry: frame via the reference CCSDS codec, then (re)write the PUS-C
 * packet error-control trailer with the N7/ESA ECSS Annex B.1 CRC. */
static FF_Status_t n7_encode_tm(const FF_Pus_TmHdr_t *hdr, const uint8_t *data, uint16_t data_len,
                                uint8_t *out, size_t out_cap, uint16_t *out_len)
{
    FF_Status_t st = FF_CcsdsEncodeTm(FF_APID_TM, hdr, data, data_len, out, out_cap, out_len);

    if ((st != FF_SUCCESS) || (out == NULL) || (out_len == NULL) || (*out_len < 2u)) {
        return st;
    }
#if FF_FRAME_WITH_CRC
    /* (Re)write the PUS-C packet error-control trailer with the N7/ESA ECSS
     * Annex B.1 CRC (only when the frame carries a trailer). */
    {
        uint16_t crc = Asn1PuscLib_Crc_calculate(out, (size_t)(*out_len - 2u));
        out[*out_len - 2u] = (uint8_t)(crc >> 8);
        out[*out_len - 1u] = (uint8_t)(crc & 0xFFu);
    }
#endif
    return FF_SUCCESS;
}

/* Telecommand: validate the exact CCSDS packet boundary, then explicitly check
 * the trailer with the N7/ESA CRC implementation before reference decoding.
 * When CRC is compiled out, trailer policy remains the configured tailoring. */
static FF_Status_t n7_decode_tc(const uint8_t *raw, uint16_t len, uint16_t *apid,
                                FF_Pus_TcHdr_t *hdr, const uint8_t **data, uint16_t *data_len)
{
    if ((raw == NULL) || (len == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
#if FF_FRAME_WITH_CRC
    /* Match the core decoder's PDL boundary before using len-2: data-field
     * length is (PDL + 1), and the N7 CRC covers the packet except its trailer.
     * Widen before addition so a malformed PDL cannot wrap. */
    if (len < (FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN + 2u)) {
        return FF_PARSE_ERROR;
    }
    {
        uint16_t primary = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
        uint32_t data_field_len = (uint32_t)(((uint16_t)raw[4] << 8) | (uint16_t)raw[5]) + 1u;
        uint32_t packet_len = (uint32_t)FF_CCSDS_HDR_LEN + data_field_len;
        uint32_t minimum_data_len = (uint32_t)FF_PUS_TC_SEC_HDR_LEN + 2u;
        uint16_t expected;
        uint16_t received;

        if ((((primary >> 12) & 0x01u) != FF_CCSDS_TC_PACKET) || (hdr == NULL) ||
            (packet_len != (uint32_t)len) || (data_field_len < minimum_data_len)) {
            return FF_PARSE_ERROR;
        }
        expected = Asn1PuscLib_Crc_calculate(raw, (size_t)(len - 2u));
        received = (uint16_t)(((uint16_t)raw[len - 2u] << 8) | raw[len - 1u]);
        if (expected != received) {
            return FF_CRC_ERROR;
        }
    }
#endif
    return FF_CcsdsDecodeTc(raw, len, apid, hdr, data, data_len);
}

const FF_Pus_Backend_t FF_PusBackendN7 = {
    .name = "n7-esa-asn1",
    .init = n7_init,
    .encode_tm = n7_encode_tm,
    .decode_tc = n7_decode_tc,
};
