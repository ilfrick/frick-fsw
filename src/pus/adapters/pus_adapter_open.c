/*
 * frick-fsw - PUS open reference backend
 *
 * The default, always available backend. It maps the abstract PUS layer onto
 * the open CCSDS Space Packet codec shipped with frick-fsw. Any open PUS
 * implementation (e.g. EmbeddedPUS, LibrePUS) can be wired here instead by
 * providing the same four vtable entries.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"

static FF_Status_t open_init(void)
{
    return FF_SUCCESS;
}

static FF_Status_t open_encode_tm(const FF_Pus_TmHdr_t *hdr, const uint8_t *data, uint16_t data_len,
                                  uint8_t *out, size_t out_cap, uint16_t *out_len)
{
    return FF_CcsdsEncodeTm(FF_APID_TM, hdr, data, data_len, out, out_cap, out_len);
}

static FF_Status_t open_decode_tc(const uint8_t *raw, uint16_t len, uint16_t *apid,
                                  FF_Pus_TcHdr_t *hdr, const uint8_t **data, uint16_t *data_len)
{
    return FF_CcsdsDecodeTc(raw, len, apid, hdr, data, data_len);
}

const FF_Pus_Backend_t FF_PusBackendOpen = {
    .name      = "open",
    .init      = open_init,
    .encode_tm = open_encode_tm,
    .decode_tc = open_decode_tc,
};
