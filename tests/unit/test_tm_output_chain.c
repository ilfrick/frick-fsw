/*
 * frick-fsw - NC-EXT-236: Assembled TM output chain test
 *
 * Exercises the end-to-end production chain:
 *   FF_EvsSend → EVS listener (ff_event_listener) → FF_PusEmitEvent
 *   → PUS S5 encoding → FF_AppTmOutputInit/ff_to_sink → FF_LinkSend
 *   → link loopback → verification
 *
 * This is the missing oracle for NC-EXT-236: the production TM sink
 * (FF_AppTmOutputInit/ff_to_sink in src/apps/app_tm_output.c) was
 * never exercised end-to-end by any test. test_r3_gap.c covers
 * listener → injected sink; test_evs_hs_health.c covers direct
 * FF_PusEmitEvent → injected sink.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include "pus/pus.h"
#include "transport/link/link.h"
#include "transport/ccsds/ccsds.h"
#include "apps/app.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Production listener (copied from src/main/fsw_main.c)              */
/* ------------------------------------------------------------------ */

static void prod_event_listener(const FF_EvsEvent_t *evt)
{
    uint16_t text_len = 0u;

    while ((text_len < sizeof(evt->text)) && (evt->text[text_len] != '\0')) {
        text_len++;
    }
    /* FF_PusEmitEvent owns the service 5 report header (event id + type). */
    (void)FF_PusEmitEvent(evt->event_id, evt->type, (const uint8_t *)evt->text, text_len);
}

/* ------------------------------------------------------------------ */
/* Link loopback frame retrieval and decoding                         */
/* ------------------------------------------------------------------ */

static bool pop_tm_frame(uint8_t *frame, uint16_t cap, uint16_t *len)
{
    return FF_LinkLoopbackPop(frame, cap, len) == FF_SUCCESS;
}

static bool find_s5_frame(const uint8_t *frame, uint16_t len, uint8_t *out_event_id_hi,
                          uint8_t *out_event_id_lo, uint8_t *out_type, const uint8_t **out_text,
                          uint16_t *out_text_len)
{
    uint16_t apid = 0u;
    FF_Pus_TmHdr_t hdr;
    const uint8_t *d = NULL;
    uint16_t dl = 0u;

    if (FF_CcsdsDecodeTm(frame, len, &apid, &hdr, &d, &dl) != FF_SUCCESS) {
        return false;
    }
    if ((hdr.service != 5u) || (hdr.subservice < 1u) || (hdr.subservice > 3u)) {
        return false;
    }
    if (dl < 3u) {
        return false;
    }
    *out_event_id_hi = d[0];
    *out_event_id_lo = d[1];
    *out_type = d[2];
    *out_text = &d[3];
    *out_text_len = (uint16_t)(dl - 3u);
    return true;
}

/* ------------------------------------------------------------------ */
/* Test cases                                                          */
/* ------------------------------------------------------------------ */

static void test_tm_output_chain_basic(void)
{
    uint8_t frame[1024];
    uint16_t len = 0u;
    uint8_t hi, lo, type;
    const uint8_t *text = NULL;
    uint16_t text_len = 0u;

    /* Initialize the full production stack */
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkInit() == FF_SUCCESS);

    /* Install production TM sink (this is NC-EXT-236 gap closure) */
    FF_TEST_ASSERT(FF_AppTmOutputInit() == FF_SUCCESS);

    /* Install production EVS listener */
    FF_EvsSetListener(prod_event_listener);

    /* Clear any residual loopback data */
    while (FF_LinkLoopbackPop(frame, sizeof(frame), &len) == FF_SUCCESS) {
        /* drain */
    }

    /* Send an EVS event through the full chain - use custom event_id 0x1234 */
    FF_TEST_ASSERT(FF_EvsSend(0x1234u, FF_EVS_INFO, 0u, "boot test") == FF_SUCCESS);

    /* Retrieve the TM frame from the link loopback */
    FF_TEST_ASSERT(pop_tm_frame(frame, sizeof(frame), &len));
    FF_TEST_ASSERT(len >= 7u);

    /* Decode and verify PUS Service 5 */
    FF_TEST_ASSERT(find_s5_frame(frame, len, &hi, &lo, &type, &text, &text_len));
    FF_TEST_ASSERT((uint16_t)((hi << 8) | lo) == 0x1234u);
    FF_TEST_ASSERT(type == (uint8_t)FF_EVS_INFO);
    FF_TEST_ASSERT(text_len == 9u);
    FF_TEST_ASSERT(memcmp(text, "boot test", 9u) == 0);

    /* Verify no extra frames */
    FF_TEST_ASSERT(FF_LinkLoopbackPop(frame, sizeof(frame), &len) != FF_SUCCESS);
}

static void test_tm_output_chain_severities(void)
{
    uint8_t frame[1024];
    uint16_t len = 0u;
    uint8_t hi, lo, type;
    const uint8_t *text = NULL;
    uint16_t text_len = 0u;

    /* Drain */
    while (FF_LinkLoopbackPop(frame, sizeof(frame), &len) == FF_SUCCESS) {
    }

    /* INFO */
    FF_TEST_ASSERT(FF_EvsSend(0x0001u, FF_EVS_INFO, 0u, "info") == FF_SUCCESS);
    FF_TEST_ASSERT(pop_tm_frame(frame, sizeof(frame), &len));
    FF_TEST_ASSERT(find_s5_frame(frame, len, &hi, &lo, &type, &text, &text_len));
    FF_TEST_ASSERT(type == (uint8_t)FF_EVS_INFO);
    FF_TEST_ASSERT((uint16_t)((hi << 8) | lo) == 0x0001u);

    /* WARNING */
    FF_TEST_ASSERT(FF_EvsSend(0x0002u, FF_EVS_WARNING, 0u, "warn") == FF_SUCCESS);
    FF_TEST_ASSERT(pop_tm_frame(frame, sizeof(frame), &len));
    FF_TEST_ASSERT(find_s5_frame(frame, len, &hi, &lo, &type, &text, &text_len));
    FF_TEST_ASSERT(type == (uint8_t)FF_EVS_WARNING);
    FF_TEST_ASSERT((uint16_t)((hi << 8) | lo) == 0x0002u);

    /* ERROR */
    FF_TEST_ASSERT(FF_EvsSend(0x0003u, FF_EVS_ERROR, 0u, "err") == FF_SUCCESS);
    FF_TEST_ASSERT(pop_tm_frame(frame, sizeof(frame), &len));
    FF_TEST_ASSERT(find_s5_frame(frame, len, &hi, &lo, &type, &text, &text_len));
    FF_TEST_ASSERT(type == (uint8_t)FF_EVS_ERROR);
    FF_TEST_ASSERT((uint16_t)((hi << 8) | lo) == 0x0003u);

    /* CRITICAL (mapped to ERROR subservice) */
    FF_TEST_ASSERT(FF_EvsSend(0x0004u, FF_EVS_CRITICAL, 0u, "crit") == FF_SUCCESS);
    FF_TEST_ASSERT(pop_tm_frame(frame, sizeof(frame), &len));
    FF_TEST_ASSERT(find_s5_frame(frame, len, &hi, &lo, &type, &text, &text_len));
    FF_TEST_ASSERT(type == (uint8_t)FF_EVS_CRITICAL); /* CRITICAL type preserved in payload */
    FF_TEST_ASSERT((uint16_t)((hi << 8) | lo) == 0x0004u);

    /* No extra frames */
    FF_TEST_ASSERT(FF_LinkLoopbackPop(frame, sizeof(frame), &len) != FF_SUCCESS);
}

static void test_tm_output_chain_payload_transparency(void)
{
    uint8_t frame[1024];
    uint16_t len = 0u;
    uint8_t hi, lo, type;
    const uint8_t *text = NULL;
    uint16_t text_len = 0u;

    /* Drain */
    while (FF_LinkLoopbackPop(frame, sizeof(frame), &len) == FF_SUCCESS) {
    }

    /* Payload with special characters (no NUL to avoid format-security) */
    FF_TEST_ASSERT(FF_EvsSend(0xABCDu, FF_EVS_ERROR, 0u, "special: %c%c%c", 0x01, 0x02, 0x03) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(pop_tm_frame(frame, sizeof(frame), &len));
    FF_TEST_ASSERT(find_s5_frame(frame, len, &hi, &lo, &type, &text, &text_len));
    FF_TEST_ASSERT((uint16_t)((hi << 8) | lo) == 0xABCDu);
    FF_TEST_ASSERT(type == (uint8_t)FF_EVS_ERROR);
    /* Text includes the formatted characters after "special: " (9 chars) */
    FF_TEST_ASSERT(text_len >= 12u);
    FF_TEST_ASSERT(text[9] == 0x01);
    FF_TEST_ASSERT(text[10] == 0x02);
    FF_TEST_ASSERT(text[11] == 0x03);

    FF_TEST_ASSERT(FF_LinkLoopbackPop(frame, sizeof(frame), &len) != FF_SUCCESS);
}

/* ------------------------------------------------------------------ */
/* Test registration                                                   */
/* ------------------------------------------------------------------ */

FF_TEST_MAIN(test_tm_output_chain_basic, test_tm_output_chain_severities,
             test_tm_output_chain_payload_transparency)