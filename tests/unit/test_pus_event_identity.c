/*
 * frick-fsw - NC-EXT-237 discriminating oracle for FF_PusEmitEvent
 *
 * Scope: the service 5 event-report identity contract of
 * `FF_PusEmitEvent(event_id, evs_type, data, len)` (ICD §4,
 * ECSS-E-ST-70-41C service 5). The function owns the report header and must
 * encode `event id [2 octets, big-endian] | type [1]` from its `event_id` and
 * `evs_type` arguments, then append `data`/`len` as auxiliary data only.
 *
 * Why this test is discriminating: the pre-NC-EXT-237 implementation executed
 * `(void)event_id;` and forwarded `data` verbatim, so two emissions that
 * differed only in `event_id` produced identical frames with no id octets.
 * Case 1 below fails on that implementation (payload length 3 instead of 6,
 * and the first two payload octets would be the ASCII "au" of the auxiliary
 * text rather than the requested id). Cases 2-6 pin the rest of the contract:
 * big-endian order, no duplication of the id in the auxiliary data, no
 * sentinel treatment of 0x0000/0xFFFF, auxiliary data passed through
 * uninterpreted, severity-to-subservice mapping with the type octet preserved,
 * and the out-of-bound argument behaviour.
 *
 * Out of scope: the assembled listener -> production TM sink -> link chain
 * (NC-EXT-236, `test_tm_output_chain.c`) and any ground-segment behaviour.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/time/time.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* TM capture sink and independent decode oracle                       */
/* ------------------------------------------------------------------ */

#define EVT_CAP_MAX 4u
/* One complete CCSDS TM frame: primary header + PUS-C secondary header + the
 * maximum PUS user data (FF_PUS_MAX_TM_FRAME) with margin. */
#define EVT_FRAME_CAP (FF_PUS_MAX_TM_FRAME + 64u)

static uint8_t g_frames[EVT_CAP_MAX][EVT_FRAME_CAP];
static uint16_t g_frame_lens[EVT_CAP_MAX];
static uint16_t g_frame_count;

static void evt_cap_reset(void)
{
    (void)memset(g_frames, 0, sizeof(g_frames));
    (void)memset(g_frame_lens, 0, sizeof(g_frame_lens));
    g_frame_count = 0u;
}

static void evt_cap_sink(const uint8_t *frame, uint16_t len)
{
    if ((g_frame_count >= EVT_CAP_MAX) || (len > sizeof(g_frames[0]))) {
        return;
    }
    (void)memcpy(g_frames[g_frame_count], frame, len);
    g_frame_lens[g_frame_count] = len;
    g_frame_count++;
}

static bool evt_decode(uint16_t index, uint8_t *service, uint8_t *subservice, const uint8_t **data,
                       uint16_t *data_len)
{
    uint16_t apid = 0u;
    FF_Pus_TmHdr_t hdr;

    if (index >= g_frame_count) {
        return false;
    }
    if (FF_CcsdsDecodeTm(g_frames[index], g_frame_lens[index], &apid, &hdr, data, data_len) !=
        FF_SUCCESS) {
        return false;
    }
    *service = hdr.service;
    *subservice = hdr.subservice;
    return true;
}

static void evt_init(void)
{
    FF_PalTaskDelayMs(20u);
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(evt_cap_sink);
    evt_cap_reset();
}

/* ------------------------------------------------------------------ */
/* Case 1: the id on the wire comes from the argument (NC-EXT-237)     */
/* ------------------------------------------------------------------ */

static void test_event_id_is_encoded_from_argument(void)
{
    static const uint8_t aux[3] = {'a', 'u', 'x'};
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    evt_init();

    /* Same auxiliary data, two different event ids: the frames must differ
     * and each must carry its own id. */
    FF_TEST_ASSERT(FF_PusEmitEvent(0x1111u, FF_EVS_INFO, aux, (uint16_t)sizeof(aux)) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitEvent(0x2222u, FF_EVS_INFO, aux, (uint16_t)sizeof(aux)) == FF_SUCCESS);
    FF_TEST_ASSERT(g_frame_count == 2u);
    FF_TEST_ASSERT((g_frame_lens[0] == g_frame_lens[1]) &&
                   (memcmp(g_frames[0], g_frames[1], g_frame_lens[0]) != 0));

    FF_TEST_ASSERT(evt_decode(0u, &service, &subservice, &d, &dl));
    FF_TEST_ASSERT(service == 5u);
    FF_TEST_ASSERT(subservice == 1u);
    FF_TEST_ASSERT(dl == (uint16_t)(sizeof(aux) + 3u));
    FF_TEST_ASSERT((d[0] == 0x11u) && (d[1] == 0x11u));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_INFO);
    FF_TEST_ASSERT(memcmp(&d[3], aux, sizeof(aux)) == 0);

    FF_TEST_ASSERT(evt_decode(1u, &service, &subservice, &d, &dl));
    FF_TEST_ASSERT(service == 5u);
    FF_TEST_ASSERT(subservice == 1u);
    FF_TEST_ASSERT(dl == (uint16_t)(sizeof(aux) + 3u));
    FF_TEST_ASSERT((d[0] == 0x22u) && (d[1] == 0x22u));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_INFO);
    FF_TEST_ASSERT(memcmp(&d[3], aux, sizeof(aux)) == 0);

    FF_PusSetTmSink(NULL);
}

/* ------------------------------------------------------------------ */
/* Case 2: big-endian id, no duplication in the auxiliary data         */
/* ------------------------------------------------------------------ */

static void test_event_id_is_big_endian_and_not_duplicated(void)
{
    /* Auxiliary data that itself looks like an event header: 0xAB 0xCD 0x00.
     * The report header must come from the arguments, not from these octets,
     * and the auxiliary data must not be altered or prefixed. */
    static const uint8_t aux[3] = {0xABu, 0xCDu, 0x00u};
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    evt_init();

    FF_TEST_ASSERT(FF_PusEmitEvent(0x0001u, FF_EVS_ERROR, aux, (uint16_t)sizeof(aux)) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_frame_count == 1u);
    FF_TEST_ASSERT(evt_decode(0u, &service, &subservice, &d, &dl));
    FF_TEST_ASSERT(service == 5u);
    FF_TEST_ASSERT(subservice == 3u);
    /* Exactly one header: 3 octets, then the caller's octets unchanged. */
    FF_TEST_ASSERT(dl == (uint16_t)(sizeof(aux) + 3u));
    FF_TEST_ASSERT((d[0] == 0x00u) && (d[1] == 0x01u));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_ERROR);
    FF_TEST_ASSERT((d[3] == 0xABu) && (d[4] == 0xCDu) && (d[5] == 0x00u));

    FF_PusSetTmSink(NULL);
}

/* ------------------------------------------------------------------ */
/* Case 3: boundary ids are encoded, not treated as sentinels          */
/* ------------------------------------------------------------------ */

static void test_event_id_boundary_values(void)
{
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    evt_init();

    FF_TEST_ASSERT(FF_PusEmitEvent(0x0000u, FF_EVS_WARNING, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitEvent(0xFFFFu, FF_EVS_WARNING, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_frame_count == 2u);

    FF_TEST_ASSERT(evt_decode(0u, &service, &subservice, &d, &dl));
    FF_TEST_ASSERT(service == 5u);
    FF_TEST_ASSERT(subservice == 2u);
    FF_TEST_ASSERT(dl == 3u); /* header only: id(2) + type(1) */
    FF_TEST_ASSERT((d[0] == 0x00u) && (d[1] == 0x00u));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_WARNING);

    FF_TEST_ASSERT(evt_decode(1u, &service, &subservice, &d, &dl));
    FF_TEST_ASSERT(dl == 3u);
    FF_TEST_ASSERT((d[0] == 0xFFu) && (d[1] == 0xFFu));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_WARNING);

    FF_PusSetTmSink(NULL);
}

/* ------------------------------------------------------------------ */
/* Case 4: severity -> subservice, type octet preserved                */
/* ------------------------------------------------------------------ */

static void test_event_severity_and_type_octet(void)
{
    static const struct
    {
        uint8_t type;
        uint8_t expected_subservice;
    } cases[] = {
        {FF_EVS_INFO, 1u},
        {FF_EVS_WARNING, 2u},
        {FF_EVS_CRITICAL, 3u},
        {0xFFu, 3u}, /* unknown severity falls back to CRITICAL (documented) */
    };
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    evt_init();

    for (size_t i = 0u; i < (sizeof(cases) / sizeof(cases[0])); i++) {
        evt_cap_reset();
        FF_TEST_ASSERT(FF_PusEmitEvent(0x4242u, cases[i].type, NULL, 0u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_frame_count == 1u);
        FF_TEST_ASSERT(evt_decode(0u, &service, &subservice, &d, &dl));
        FF_TEST_ASSERT(service == 5u);
        FF_TEST_ASSERT(subservice == cases[i].expected_subservice);
        FF_TEST_ASSERT(dl == 3u);
        FF_TEST_ASSERT((d[0] == 0x42u) && (d[1] == 0x42u));
        /* The raw severity is preserved in the payload, including the
         * documented fallback case, so ground decoding is not ambiguous. */
        FF_TEST_ASSERT(d[2] == cases[i].type);
    }

    FF_PusSetTmSink(NULL);
}

/* ------------------------------------------------------------------ */
/* Case 5: out-of-bound arguments                                      */
/* ------------------------------------------------------------------ */

static void test_event_out_of_bound_arguments(void)
{
    static const uint8_t aux[8] = {0};
    static uint8_t aux_max[FF_PUS_MAX_TM_FRAME];
    uint8_t service = 0u;
    uint8_t subservice = 0u;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    evt_init();

    /* The auxiliary-data budget is FF_PUS_MAX_TM_FRAME - 3 (header). */
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0101u, FF_EVS_INFO, aux,
                                   (uint16_t)(FF_PUS_MAX_TM_FRAME - 2u)) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0101u, FF_EVS_INFO, aux, UINT16_MAX) == FF_BAD_SIZE);
    FF_TEST_ASSERT(g_frame_count == 0u); /* refused before framing */

    /* NULL auxiliary data is legal only with zero length. */
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0101u, FF_EVS_INFO, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_frame_count == 0u);

    /* Exactly at the budget: accepted, header intact, data preserved. */
    (void)memset(aux_max, 0x5A, sizeof(aux_max));
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0101u, FF_EVS_INFO, aux_max,
                                   (uint16_t)(FF_PUS_MAX_TM_FRAME - 3u)) == FF_SUCCESS);
    FF_TEST_ASSERT(g_frame_count == 1u);
    FF_TEST_ASSERT(evt_decode(0u, &service, &subservice, &d, &dl));
    FF_TEST_ASSERT(service == 5u);
    FF_TEST_ASSERT(dl == (uint16_t)(FF_PUS_MAX_TM_FRAME));
    FF_TEST_ASSERT((d[0] == 0x01u) && (d[1] == 0x01u));
    FF_TEST_ASSERT(d[2] == (uint8_t)FF_EVS_INFO);
    FF_TEST_ASSERT(d[3] == 0x5Au);
    FF_TEST_ASSERT(d[dl - 1u] == 0x5Au);

    FF_PusSetTmSink(NULL);
}

FF_TEST_MAIN(test_event_id_is_encoded_from_argument, test_event_id_is_big_endian_and_not_duplicated,
             test_event_id_boundary_values, test_event_severity_and_type_octet,
             test_event_out_of_bound_arguments)
