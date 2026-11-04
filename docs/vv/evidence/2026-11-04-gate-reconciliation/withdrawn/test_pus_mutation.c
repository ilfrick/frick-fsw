/*
 * frick-fsw - PUS layer mutation testing (NC-EXT-235 follow-on)
 *
 * Systematic mutation testing for PUS layer critical functions.
 * Complements test_decoder_corpus.c single-bit mutation testing
 * by targeting semantic mutations in PUS business logic.
 *
 * Copyright (C) 2026 Ilfrick
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/sb/sb.h"
#include "core/time/time.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

static void init_core_modules(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TimeInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
}

/* Dummy HK provider that returns fixed data. */
static FF_Status_t dummy_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    if (*len < 4u) { return FF_BAD_SIZE; }
    out[0] = 0xAAu; out[1] = 0xBBu; out[2] = 0xCCu; out[3] = 0xDDu;
    *len = 4u;
    return FF_SUCCESS;
}



/* ------------------------------------------------------------------ */
/* 1. time_period_due boundary testing                               */
/* ------------------------------------------------------------------ */

static void test_time_period_due_boundary(void)
{
    hk_test_setup();
    /* Register HK report: report_id=10, provider=1, period=10s */
    { uint8_t payload[6] = {10u, 1u, 0u, 0u, 0u, 10u};
      hk_send_tc(1u, payload, 6u); }
    /* Enable HK report (report_id=10) */
    { uint8_t payload[1] = {10u}; hk_send_tc(5u, payload, 1u); }
    FF_PalTimeTestFreeze(1000000000000ULL);
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_PalTimeTestFreeze(1010000000000ULL);
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_PalTimeTestFreeze(1011000000000ULL);
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_PalTimeTestFreeze(1050000000000ULL);
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_PalTimeTestUnfreeze();
}

/* ------------------------------------------------------------------ */
/* 2. FF_CcsdsEncodeTc/DecodeTc round-trip mutation testing         */
/* ------------------------------------------------------------------ */

static void test_ccsds_encode_decode_mutation_roundtrip(void)
{
    FF_Pus_TcHdr_t tc;
    uint8_t payload[64], frame[256];
    uint16_t frame_len;
    for (uint16_t payload_len = 0; payload_len <= 32; payload_len += 7) {
        for (uint16_t i = 0; i < payload_len; i++) payload[i] = (uint8_t)(i * 37 + 17);
        FF_Pus_TcHdr_t tc_init = {0}; tc_init.version = FF_PUS_VERSION;
        tc_init.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
        tc_init.service = FF_PUS_SERVICE_HK; tc_init.subservice = 1u; tc_init.source_id = 0x0102u;
        tc = tc_init;
        FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc, payload, payload_len, frame, sizeof(frame), &frame_len) == FF_SUCCESS);
        uint16_t decoded_apid; FF_Pus_TcHdr_t decoded_tc; const uint8_t *decoded_data; uint16_t decoded_len;
        FF_TEST_ASSERT(FF_CcsdsDecodeTc(frame, frame_len, &decoded_apid, &decoded_tc, &decoded_data, &decoded_len) == FF_SUCCESS);
        FF_TEST_ASSERT(decoded_apid == FF_APID_TC); FF_TEST_ASSERT(decoded_tc.version == tc.version);
        FF_TEST_ASSERT(decoded_tc.service == tc.service); FF_TEST_ASSERT(decoded_tc.subservice == tc.subservice);
        FF_TEST_ASSERT(decoded_tc.source_id == tc.source_id); FF_TEST_ASSERT(decoded_len == payload_len);
        FF_TEST_ASSERT(memcmp(decoded_data, payload, payload_len) == 0);
        for (uint16_t i = 0; i < frame_len; i++) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint8_t mutated[256]; memcpy(mutated, frame, frame_len);
                mutated[i] ^= (1u << bit);
                uint16_t mutated_apid; FF_Pus_TcHdr_t mutated_tc; const uint8_t *mutated_data; uint16_t mutated_len;
                FF_Status_t rc = FF_CcsdsDecodeTc(mutated, frame_len, &mutated_apid, &mutated_tc, &mutated_data, &mutated_len);
                FF_TEST_ASSERT(rc != FF_SUCCESS);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* 3. FF_PusEmitEvent mutation testing                               */
/* ------------------------------------------------------------------ */

static void test_pus_emit_event_mutation(void)
{
    init_core_modules();
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);
    uint8_t data[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0103u, FF_EVS_CRITICAL, data, 8) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0103u, FF_EVS_CRITICAL, NULL, 0) == FF_SUCCESS);
    uint8_t oversized[2048]; memset(oversized, 0xAA, sizeof(oversized));
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0103u, FF_EVS_CRITICAL, oversized, 2048) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_PusEmitEvent(0x0103u, 0xFF, data, 4) == FF_SUCCESS);
}



/* ------------------------------------------------------------------ */
/* 4. FF_PusEmitPeriodicHk mutation testing                          */
/* ------------------------------------------------------------------ */

static void test_pus_emit_periodic_hk_mutation(void)
{
    hk_test_setup();
    /* Register HK report: report_id=10, provider=1, period=10s */
    { uint8_t payload[6] = {10u, 1u, 0u, 0u, 0u, 10u};
      hk_send_tc(1u, payload, 6u); }
    /* Enable HK report (report_id=10) */
    { uint8_t payload[1] = {10u}; hk_send_tc(5u, payload, 1u); }
    FF_PalTimeTestFreeze(1000000000000ULL); FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_PalTimeTestFreeze(1010000000000ULL); FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_ERROR);
    FF_PalTimeTestFreeze(1011000000000ULL); FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_PalTimeTestFreeze(1050000000000ULL); FF_TEST_ASSERT(FF_PusEmitPeriodicHk() == FF_SUCCESS);
    FF_PalTimeTestUnfreeze();
}

/* ------------------------------------------------------------------ */
/* 5. FF_PusIngest mutation testing                                  */
/* ------------------------------------------------------------------ */

static void test_pus_ingest_mutation(void)
{
    init_core_modules(); FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS); FF_PusSetTmSink(NULL);
    { FF_Pus_TcHdr_t tc_init = {0}; tc_init.version = FF_PUS_VERSION; tc_init.ack_flags = 0u;
      tc_init.service = FF_PUS_SERVICE_HK; tc_init.subservice = 1u; tc_init.source_id = 0x0102u;
      uint8_t payload[6] = {1u, 1u, 0u, 0u, 0u, 10u}; uint8_t frame[128]; uint16_t frame_len = 0u;
      FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc_init, payload, sizeof(payload), frame, sizeof(frame), &frame_len) == FF_SUCCESS);
      FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS); }
    { uint8_t frame[10] = {0}; FF_TEST_ASSERT(FF_PusIngest(frame, 10) != FF_SUCCESS); }
    { FF_Pus_TcHdr_t tc_init = {0}; tc_init.version = FF_PUS_VERSION; tc_init.ack_flags = 0u;
      tc_init.service = FF_PUS_SERVICE_HK; tc_init.subservice = 1u; tc_init.source_id = 0x0102u;
      uint8_t payload[6] = {1u, 1u, 0u, 0u, 0u, 10u}; uint8_t frame[128]; uint16_t frame_len = 0u;
      FF_TEST_ASSERT(FF_CcsdsEncodeTc(0xFFFF, &tc_init, payload, sizeof(payload), frame, sizeof(frame), &frame_len) == FF_SUCCESS);
      FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) != FF_SUCCESS); }
    { FF_Pus_TcHdr_t tc_init = {0}; tc_init.version = 0xFF; tc_init.ack_flags = 0u;
      tc_init.service = FF_PUS_SERVICE_HK; tc_init.subservice = 1u; tc_init.source_id = 0x0102u;
      uint8_t payload[6] = {1u, 1u, 0u, 0u, 0u, 10u}; uint8_t frame[128]; uint16_t frame_len = 0u;
      FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc_init, payload, sizeof(payload), frame, sizeof(frame), &frame_len) == FF_SUCCESS);
      FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) != FF_SUCCESS); }
}

/* ------------------------------------------------------------------ */
/* 6. FF_PusRegisterHkProvider mutation testing                      */
/* ------------------------------------------------------------------ */

static void test_pus_register_hk_provider_mutation(void)
{
    init_core_modules(); FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS); FF_PusSetTmSink(NULL);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(1u, dummy_hk_provider) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(1u, dummy_hk_provider) != FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(0u, dummy_hk_provider) != FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(255u, dummy_hk_provider) != FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(2u, NULL) != FF_SUCCESS);
}

static FF_Status_t dummy_hk_provider(uint8_t id, uint8_t *out, uint16_t *len)
{
    (void)id;
    if (*len < 4u) { return FF_BAD_SIZE; }
    out[0] = 0xAAu; out[1] = 0xBBu; out[2] = 0xCCu; out[3] = 0xDDu;
    *len = 4u;
    return FF_SUCCESS;
}



FF_TEST_MAIN(test_time_period_due_boundary, test_ccsds_encode_decode_mutation_roundtrip,
             test_pus_emit_event_mutation, test_pus_emit_periodic_hk_mutation,
             test_pus_ingest_mutation, test_pus_register_hk_provider_mutation)

static void hk_send_tc(uint8_t subservice, const uint8_t *payload, uint16_t payload_len)
{
    FF_Pus_TcHdr_t tc_init = {0};
    tc_init.version = FF_PUS_VERSION;
    tc_init.ack_flags = 0u;
    tc_init.service = FF_PUS_SERVICE_HK;
    tc_init.subservice = subservice;
    tc_init.source_id = 0x0102u;
    uint8_t frame[128];
    uint16_t frame_len = 0u;
    FF_TEST_ASSERT(FF_CcsdsEncodeTc(FF_APID_TC, &tc_init, payload, payload_len, frame, sizeof(frame), &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusIngest(frame, frame_len) == FF_SUCCESS);
}

static void hk_test_setup(void)
{
    init_core_modules();
    FF_TEST_ASSERT(FF_PusRegisterHkProvider(1u, dummy_hk_provider) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);
}

FF_TEST_MAIN(test_time_period_due_boundary, test_ccsds_encode_decode_mutation_roundtrip,
             test_pus_emit_event_mutation, test_pus_emit_periodic_hk_mutation,
             test_pus_ingest_mutation, test_pus_register_hk_provider_mutation)
