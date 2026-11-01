/*
 * frick-fsw - Requirement-keyed logical target test application (increment T1)
 *
 * Runs the T1 logical-chain cases (CCSDS -> PUS -> services -> CCSDS) and
 * prints one machine-checkable evidence line per case. The companion tools
 * decide the result:
 *   tools/scripts/target_req_verify.py   - per-case (COMPONENT) result
 *   tools/scripts/target_run_verdict.py  - COMPONENT + WHOLE-RUN result
 *
 * Evidence grammar (one line per case):
 *   TGTREQ: <SRS-ID>:<CASE>:PASS[:<detail>]
 *   TGTREQ: <SRS-ID>:<CASE>:FAIL:<detail>
 *   TGTREQ-OBS: <SRS-ID>:<CASE>:<free text>   (observation only, never coverage)
 *   TGTREQ-GAP: <SRS-ID>:<CASE>:<reason>      (no oracle: gap, never coverage)
 *
 * Scope of the evidence this module can produce (stated, not implied):
 *   - LOGICAL chain only (CCSDS framing/CRC, PUS service 1/6/9/17, and the
 *     in-image TM sink). Generic-RTEMS/QEMU evidence: not MPFS peripheral
 *     evidence, not N18, no timing/WCET claim.
 *   - Oracles are wire-byte at fixed offsets: primary version/type, secondary
 *     header flag, APID, sequence flags and counter (+1 mod 16384 in emission
 *     order), packet data length, PUS TM version/service/subservice, and the
 *     CRC. A negative control inside L1 mutates a captured frame (counter, APID,
 *     PDL) and requires the same validator to reject it.
 *   - Sub-aspects that are not controlled by a document are NOT asserted:
 *     the S1 payload prefix and the numeric failure codes are observations only
 *     (NC-EXT-14/15), and the 6-byte TC[3,1] create is an unapproved extension
 *     (NC-EXT-17) so the HK one-shot case is a declared GAP, not a PASS.
 *
 * Test-only state (documented leakage): this module registers memory region 1
 * (needed by L8) and installs its own TM sink for the duration of the suite,
 * restoring the production sink afterwards. There is no unregister API for a
 * memory region, so region 1 stays registered in this test-only image. No
 * production code path and no production image is affected (the option is OFF
 * by default).
 *
 * Compiled only with FF_ENABLE_TARGET_REQ_TEST=ON (default OFF).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "apps/app.h"
#include "core/pal/pal.h"
#include "core/time/time.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"

#include <string.h>

/* ---- evidence capture -------------------------------------------- */

#define FF_REQ_MAX_CAPTURED 48u

typedef struct
{
    uint8_t bytes[FF_CCSDS_MAX_TM_PACKET];
    uint16_t len;
} ff_req_capture_t;

static ff_req_capture_t g_capture[FF_REQ_MAX_CAPTURED];
static uint16_t g_capture_count;
static bool g_capture_on;
static uint16_t g_counter_before;

/* Region fixture owned by this module (never production state). */
static uint8_t g_req_region[64];

static void ff_req_tm_sink(const uint8_t *packet, uint16_t len)
{
    if (!g_capture_on || (packet == NULL)) {
        return;
    }
    if ((len == 0u) || (len > FF_CCSDS_MAX_TM_PACKET)) {
        return;
    }
    if (g_capture_count >= FF_REQ_MAX_CAPTURED) {
        return;
    }
    memcpy(g_capture[g_capture_count].bytes, packet, len);
    g_capture[g_capture_count].len = len;
    g_capture_count++;
}

static void ff_req_capture_begin(void)
{
    g_capture_count = 0u;
    g_counter_before = FF_PusPeekTmCounter();
    g_capture_on = true;
}

static void ff_req_capture_end(void)
{
    g_capture_on = false;
}

/* ---- fixed-offset TM frame validation ---------------------------- */

/* Service/subservice of a captured frame, read at the documented offsets. */
static uint8_t ff_req_frame_service(uint16_t i)
{
    return (i < g_capture_count) ? g_capture[i].bytes[7] : 0u;
}

static uint8_t ff_req_frame_subservice(uint16_t i)
{
    return (i < g_capture_count) ? g_capture[i].bytes[8] : 0u;
}

/* Validate one frame at fixed byte offsets (T1 F3): primary header
 * version/type/secondary-header flag/APID, sequence flags, sequence counter
 * against the expected emission order, the packet data length field, the PUS TM
 * version and service/subservice, and the appended CRC. */
static bool ff_req_frame_ok(const uint8_t *f, uint16_t len, uint16_t expect_counter, uint8_t svc,
                            uint8_t sub, const char **why)
{
    uint16_t w0;
    uint16_t seq;
    uint16_t pdl;
    uint16_t crc_calc;
    uint16_t crc_frame;

    if (len < (FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN + 2u)) {
        *why = "len_short";
        return false;
    }
    w0 = (uint16_t)(((uint16_t)f[0] << 8) | (uint16_t)f[1]);
    if (((w0 >> 13) & 0x07u) != 0u) { /* primary version */
        *why = "prim_version";
        return false;
    }
    if (((w0 >> 12) & 0x01u) != FF_CCSDS_TM_PACKET) { /* packet type = TM */
        *why = "not_tm_type";
        return false;
    }
    if (((w0 >> 11) & 0x01u) != FF_CCSDS_SEC_HDR_FLAG) {
        *why = "sec_hdr_flag";
        return false;
    }
    if ((w0 & 0x07FFu) != FF_APID_TM) {
        *why = "apid";
        return false;
    }
    seq = (uint16_t)(((uint16_t)f[2] << 8) | (uint16_t)f[3]);
    if ((seq >> 14) != 3u) { /* unsegmented */
        *why = "seq_flags";
        return false;
    }
    /* The CCSDS sequence counter is not used by this codec (it writes 0); the
     * PUS TM counter that SRS-REQ-007 constrains lives in the PUS secondary
     * header at bytes 9..10. */
    if ((seq & 0x3FFFu) != 0u) {
        *why = "ccsds_seq_not_zero";
        return false;
    }
    if ((((uint16_t)f[9] << 8) | (uint16_t)f[10]) != expect_counter) {
        *why = "pus_counter";
        return false;
    }
    pdl = (uint16_t)(((uint16_t)f[4] << 8) | (uint16_t)f[5]);
    if (((uint32_t)pdl + 1u + (uint32_t)FF_CCSDS_HDR_LEN) != (uint32_t)len) {
        *why = "pdl";
        return false;
    }
    if ((f[6] >> 4) != FF_PUS_VERSION) {
        *why = "pus_version";
        return false;
    }
    if ((f[7] != svc) || (f[8] != sub)) {
        *why = "svc_sub";
        return false;
    }
    crc_calc = FF_CcsdsCrc16(f, (uint16_t)(len - 2u));
    crc_frame = (uint16_t)(((uint16_t)f[len - 2u] << 8) | (uint16_t)f[len - 1u]);
    if (crc_calc != crc_frame) {
        *why = "crc";
        return false;
    }
    return true;
}

/* Validate every captured frame against the emission order expected from
 * `counter_before`. Returns false with a reason on the first deviation. */
static bool ff_req_validate_window(const char **why)
{
    for (uint16_t i = 0u; i < g_capture_count; i++) {
        uint16_t expect = (uint16_t)((g_counter_before + i) & 0x3FFFu);

        if (!ff_req_frame_ok(g_capture[i].bytes, g_capture[i].len, expect, ff_req_frame_service(i),
                             ff_req_frame_subservice(i), why)) {
            return false;
        }
    }
    return true;
}

/* Count captured frames by header service/subservice (decoder-validated). */
static uint16_t ff_req_count(uint8_t service, uint8_t subservice)
{
    uint16_t n = 0u;

    for (uint16_t i = 0u; i < g_capture_count; i++) {
        FF_Pus_TmHdr_t hdr;
        const uint8_t *payload = NULL;
        uint16_t payload_len = 0u;
        uint16_t apid = 0u;

        if (FF_CcsdsDecodeTm(g_capture[i].bytes, g_capture[i].len, &apid, &hdr, &payload,
                             &payload_len) != FF_SUCCESS) {
            continue;
        }
        if ((apid == FF_APID_TM) && (hdr.service == service) && (hdr.subservice == subservice)) {
            n++;
        }
    }
    return n;
}

/* First captured frame with this header service/subservice; returns its payload. */
static bool ff_req_find(uint8_t service, uint8_t subservice, const uint8_t **payload,
                        uint16_t *payload_len)
{
    for (uint16_t i = 0u; i < g_capture_count; i++) {
        FF_Pus_TmHdr_t hdr;
        const uint8_t *data = NULL;
        uint16_t len = 0u;
        uint16_t apid = 0u;

        if (FF_CcsdsDecodeTm(g_capture[i].bytes, g_capture[i].len, &apid, &hdr, &data, &len) !=
            FF_SUCCESS) {
            continue;
        }
        if ((apid == FF_APID_TM) && (hdr.service == service) && (hdr.subservice == subservice)) {
            *payload = data;
            *payload_len = len;
            return true;
        }
    }
    return false;
}

/* Print the first captured payload of a service/subservice as an observation. */
static void ff_req_obs_payload(const char *rid, const char *cid, uint8_t service,
                               uint8_t subservice)
{
    const uint8_t *payload = NULL;
    uint16_t payload_len = 0u;

    if (!ff_req_find(service, subservice, &payload, &payload_len) || (payload_len == 0u)) {
        return;
    }
    FF_PalLog("TGTREQ-OBS: %s:%s:S%u.%u_payload=", rid, cid, (unsigned)service,
              (unsigned)subservice);
    for (uint16_t i = 0u; (i < payload_len) && (i < 8u); i++) {
        FF_PalLog("%02X", (unsigned)payload[i]);
    }
    FF_PalLog(" (implementation-defined layout; NC-EXT-14/15)\n");
}

/* ---- evidence lines ---------------------------------------------- */

static void ff_req_line(const char *status, const char *rid, const char *cid, const char *detail)
{
    FF_PalLog("TGTREQ: %s:%s:%s:%s\n", rid, cid, status, detail);
}

static void ff_req_pass(const char *rid, const char *cid, const char *detail)
{
    ff_req_line("PASS", rid, cid, detail);
}

static void ff_req_fail(const char *rid, const char *cid, const char *detail)
{
    ff_req_line("FAIL", rid, cid, detail);
}

static void ff_req_gap(const char *rid, const char *cid, const char *reason)
{
    FF_PalLog("TGTREQ-GAP: %s:%s:%s\n", rid, cid, reason);
}

/* ---- telecommand helpers ----------------------------------------- */

static FF_Status_t ff_req_encode(uint16_t apid, uint8_t service, uint8_t subservice, uint8_t ack,
                                 const uint8_t *payload, uint16_t payload_len, uint8_t *frame,
                                 uint16_t *frame_len)
{
    FF_Pus_TcHdr_t hdr;

    memset(&hdr, 0, sizeof(hdr));
    hdr.version = FF_PUS_VERSION;
    hdr.ack_flags = ack;
    hdr.service = service;
    hdr.subservice = subservice;
    hdr.source_id = 0x0102u;
    return FF_CcsdsEncodeTc(apid, &hdr, payload, payload_len, frame, FF_CCSDS_MAX_TC_PACKET,
                            frame_len);
}

static FF_Status_t ff_req_send(uint16_t apid, uint8_t service, uint8_t subservice, uint8_t ack,
                               const uint8_t *payload, uint16_t payload_len, FF_Status_t *encode_st)
{
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t frame_len = 0u;
    FF_Status_t st =
        ff_req_encode(apid, service, subservice, ack, payload, payload_len, frame, &frame_len);

    if (encode_st != NULL) {
        *encode_st = st;
    }
    if (st != FF_SUCCESS) {
        return st;
    }
    return FF_PusIngest(frame, frame_len);
}

/* ---- the L1 predicate, reused by L1 and L10 ---------------------- */

static const uint8_t g_ping_payload[4] = {0x01u, 0x02u, 0x03u, 0x04u};

/* SRS-REQ-006/007/012: one accepted ping produces exactly S1.1, S1.3, S1.7 and
 * S17.2, every frame passes the fixed-offset validator, and the S17.2 payload
 * is echoed exactly. */
static bool ff_req_check_ping_set(const char **why)
{
    const uint8_t *payload = NULL;
    uint16_t payload_len = 0u;

    if (!ff_req_validate_window(why)) {
        return false;
    }
    if (ff_req_count(FF_PUS_SERVICE_VERIFICATION, 1u) != 1u) {
        *why = "S1.1_count";
        return false;
    }
    if (ff_req_count(FF_PUS_SERVICE_VERIFICATION, 3u) != 1u) {
        *why = "S1.3_count";
        return false;
    }
    if (ff_req_count(FF_PUS_SERVICE_VERIFICATION, 7u) != 1u) {
        *why = "S1.7_count";
        return false;
    }
    if ((ff_req_count(FF_PUS_SERVICE_VERIFICATION, 2u) != 0u) ||
        (ff_req_count(FF_PUS_SERVICE_VERIFICATION, 4u) != 0u) ||
        (ff_req_count(FF_PUS_SERVICE_VERIFICATION, 8u) != 0u)) {
        *why = "unexpected_S1_failure_stage";
        return false;
    }
    if (ff_req_count(FF_PUS_SERVICE_TEST, 2u) != 1u) {
        *why = "S17.2_count";
        return false;
    }
    if (!ff_req_find(FF_PUS_SERVICE_TEST, 2u, &payload, &payload_len)) {
        *why = "no_S17.2";
        return false;
    }
    if ((payload_len != sizeof(g_ping_payload)) ||
        (memcmp(payload, g_ping_payload, sizeof(g_ping_payload)) != 0)) {
        *why = "S17.2_payload_mismatch";
        return false;
    }
    return true;
}

/* SRS-REQ-006/007: one accepted ping (stimulus shared by L1 and L10). */
static FF_Status_t ff_req_ping(FF_Status_t *encode_st)
{
    return ff_req_send(FF_APID_TC, FF_PUS_SERVICE_TEST, 1u,
                       (uint8_t)(FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_START | FF_PUS_ACK_COMPLETION),
                       g_ping_payload, sizeof(g_ping_payload), encode_st);
}

/* Copy a captured frame, apply one mutation and repair the CRC so that only the
 * mutated semantic field can be responsible for a rejection. */
static bool ff_req_mutation_rejected(uint16_t src_index, uint8_t mode)
{
    uint8_t copy[FF_CCSDS_MAX_TM_PACKET];
    uint16_t len;
    uint16_t expect_counter;
    const char *why = "accepted";

    if (src_index >= g_capture_count) {
        return false;
    }
    len = g_capture[src_index].len;
    /* Expected PUS counter comes from the ORIGINAL frame: the mutated frame must
     * be refused against it (otherwise the control would validate itself). */
    expect_counter = (uint16_t)((((uint16_t)g_capture[src_index].bytes[9] << 8) |
                                 (uint16_t)g_capture[src_index].bytes[10]) &
                                0x3FFFu);
    memcpy(copy, g_capture[src_index].bytes, len);

    switch (mode) {
    case 0u: { /* PUS TM counter skip (secondary header bytes 9..10) */
        uint16_t pus = (uint16_t)(((uint16_t)copy[9] << 8) | (uint16_t)copy[10]);
        pus = (uint16_t)((pus + 1u) & 0x3FFFu);
        copy[9] = (uint8_t)(pus >> 8);
        copy[10] = (uint8_t)(pus & 0xFFu);
        break;
    }
    case 1u: { /* wrong APID */
        uint16_t w0 = (uint16_t)(((uint16_t)copy[0] << 8) | (uint16_t)copy[1]);
        w0 = (uint16_t)((w0 & 0xF800u) | 0x021u);
        copy[0] = (uint8_t)(w0 >> 8);
        copy[1] = (uint8_t)(w0 & 0xFFu);
        break;
    }
    default: { /* packet data length mismatch */
        uint16_t pdl = (uint16_t)(((uint16_t)copy[4] << 8) | (uint16_t)copy[5]);
        pdl = (uint16_t)(pdl + 1u);
        copy[4] = (uint8_t)(pdl >> 8);
        copy[5] = (uint8_t)(pdl & 0xFFu);
        break;
    }
    }

    if (len >= 3u) {
        uint16_t crc = FF_CcsdsCrc16(copy, (uint16_t)(len - 2u));
        copy[len - 2u] = (uint8_t)(crc >> 8);
        copy[len - 1u] = (uint8_t)(crc & 0xFFu);
    }
    /* The validator must reject the mutated frame. Any service/subservice is
     * acceptable here: the frame is expected to be refused before that check. */
    return !ff_req_frame_ok(copy, len, expect_counter, copy[7], copy[8], &why);
}

/* ---- cases -------------------------------------------------------- */

/* SRS-REQ-012/006/007 (L1): a valid S17.1 ping is accepted, started and
 * completed; every emitted frame is validated at fixed offsets; a mutated copy
 * of a captured frame must be rejected by the same validator. */
static void ff_req_case_l1(void)
{
    const char *why = "unknown";
    FF_Status_t st;
    uint16_t rejected = 0u;

    ff_req_capture_begin();
    st = ff_req_ping(NULL);
    ff_req_capture_end();

    if (st != FF_SUCCESS) {
        ff_req_fail("SRS-REQ-012", "L1", "ingest_status");
        return;
    }
    if (!ff_req_check_ping_set(&why)) {
        ff_req_fail("SRS-REQ-012", "L1", why);
        return;
    }

    /* Negative control: counter skip, wrong APID and PDL mismatch must each be
     * rejected by the same validator (CRC repaired so the field is the cause). */
    for (uint8_t mode = 0u; mode < 3u; mode++) {
        if (ff_req_mutation_rejected(0u, mode)) {
            rejected++;
        }
    }
    FF_PalLog("TGTREQ-OBS: SRS-REQ-012:L1:negative_control=%u/3\n", (unsigned)rejected);
    if (rejected != 3u) {
        ff_req_fail("SRS-REQ-012", "L1", "negative_control_not_discriminating");
        return;
    }
    ff_req_pass("SRS-REQ-012", "L1", "per_frame_offsets_ok_S17.2_exact_negctl_3/3");
}

/* SRS-REQ-005 (L2): an unknown non-zero service produces exactly one S1.8 and
 * no test-service telemetry. Identified by the PUS TM header only; the S1
 * payload layout and the numeric code are implementation-defined observations
 * (NC-EXT-14/15). */
static void ff_req_case_l2(void)
{
    uint16_t n18;
    uint16_t n17;

    ff_req_capture_begin();
    (void)ff_req_send(FF_APID_TC, 0x2Au, 1u, FF_PUS_ACK_COMPLETION, NULL, 0u, NULL);
    ff_req_capture_end();

    n18 = ff_req_count(FF_PUS_SERVICE_VERIFICATION, 8u);
    n17 = ff_req_count(FF_PUS_SERVICE_TEST, 2u);
    ff_req_obs_payload("SRS-REQ-005", "L2", FF_PUS_SERVICE_VERIFICATION, 8u);

    if ((n18 == 1u) && (n17 == 0u)) {
        ff_req_pass("SRS-REQ-005", "L2", "one_S1.8_by_header_no_S17.2");
    }
    else {
        ff_req_fail("SRS-REQ-005", "L2", "S1.8_or_S17.2_count");
    }
}

/* SRS-REQ-005 (L3): an unknown subservice of the test service produces exactly
 * one S1.8 and no S17.2; payload/code are observations only (NC-EXT-15). */
static void ff_req_case_l3(void)
{
    uint16_t n18;
    uint16_t n17;

    ff_req_capture_begin();
    (void)ff_req_send(FF_APID_TC, FF_PUS_SERVICE_TEST, 9u, FF_PUS_ACK_COMPLETION, NULL, 0u, NULL);
    ff_req_capture_end();

    n18 = ff_req_count(FF_PUS_SERVICE_VERIFICATION, 8u);
    n17 = ff_req_count(FF_PUS_SERVICE_TEST, 2u);
    ff_req_obs_payload("SRS-REQ-005", "L3", FF_PUS_SERVICE_VERIFICATION, 8u);

    if ((n18 == 1u) && (n17 == 0u)) {
        ff_req_pass("SRS-REQ-005", "L3", "one_S1.8_by_header_no_S17.2");
    }
    else {
        ff_req_fail("SRS-REQ-005", "L3", "S1.8_or_S17.2_count");
    }
}

/* SRS-REQ-004 (L4): a corrupted frame is rejected by the CRC check and never
 * reaches a handler. The S1.8 report for a malformed TC is a declared gap
 * (NC-EXT-04). */
static void ff_req_case_l4(void)
{
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t frame_len = 0u;
    FF_Status_t st;
    uint16_t n17;

    ff_req_capture_begin();
    st = ff_req_encode(FF_APID_TC, FF_PUS_SERVICE_TEST, 1u,
                       (uint8_t)(FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION), g_ping_payload,
                       sizeof(g_ping_payload), frame, &frame_len);
    if (st == FF_SUCCESS) {
        frame[FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN] ^= 0x01u;
        uint32_t tc_before = FF_PusGetTcCount();
        st = FF_PusIngest(frame, frame_len);
        if (FF_PusGetTcCount() != tc_before) {
            st = FF_ERROR; /* a handler side effect must not occur */
        }
    }
    ff_req_capture_end();
    n17 = ff_req_count(FF_PUS_SERVICE_TEST, 2u);

    if ((st == FF_PARSE_ERROR) && (n17 == 0u)) {
        ff_req_pass("SRS-REQ-004", "L4", "parse_error_no_handler_no_S17.2");
    }
    else {
        ff_req_fail("SRS-REQ-004", "L4", "not_rejected_by_crc");
    }
}

/* SRS-REQ-005 (L5): a frame on an illegal APID is refused with exactly one S1.2
 * and no handler is invoked. Only the PUS TM header identifies the response;
 * the S1 payload prefix and the numeric code are observations (NC-EXT-14/15). */
static void ff_req_case_l5(void)
{
    uint16_t n11;
    uint16_t n12;
    uint16_t n17;

    ff_req_capture_begin();
    (void)ff_req_send(0x021u, FF_PUS_SERVICE_TEST, 1u,
                      (uint8_t)(FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION), g_ping_payload,
                      sizeof(g_ping_payload), NULL);
    ff_req_capture_end();

    n12 = ff_req_count(FF_PUS_SERVICE_VERIFICATION, 2u);
    n11 = ff_req_count(FF_PUS_SERVICE_VERIFICATION, 1u);
    n17 = ff_req_count(FF_PUS_SERVICE_TEST, 2u);
    ff_req_obs_payload("SRS-REQ-005", "L5", FF_PUS_SERVICE_VERIFICATION, 2u);

    if ((n12 == 1u) && (n11 == 0u) && (n17 == 0u)) {
        ff_req_pass("SRS-REQ-005", "L5", "one_S1.2_by_header_no_handler");
    }
    else {
        ff_req_fail("SRS-REQ-005", "L5", "S1.2_or_counts");
    }
}

/* SRS-REQ-003 (L6): the maximum TC payload (1024) round-trips exactly, and one
 * byte more is refused by the codec. The refusal side uses the documented
 * maximum packet buffer (FF_CCSDS_MAX_TC_PACKET) as the encoder capacity. */
static void ff_req_case_l6(void)
{
    static uint8_t big[FF_PUS_MAX_TC_FRAME];
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t frame_len = 0u;
    const uint8_t *payload = NULL;
    uint16_t payload_len = 0u;
    FF_Status_t st;
    FF_Status_t encode_st = FF_ERROR;
    bool echoed;

    for (uint16_t i = 0u; i < sizeof(big); i++) {
        big[i] = (uint8_t)(i & 0xFFu);
    }

    ff_req_capture_begin();
    st = ff_req_send(FF_APID_TC, FF_PUS_SERVICE_TEST, 1u, FF_PUS_ACK_COMPLETION, big,
                     (uint16_t)sizeof(big), &encode_st);
    ff_req_capture_end();

    echoed = (st == FF_SUCCESS) && (encode_st == FF_SUCCESS) &&
             ff_req_find(FF_PUS_SERVICE_TEST, 2u, &payload, &payload_len) &&
             (payload_len == sizeof(big)) && (memcmp(payload, big, sizeof(big)) == 0);

    ff_req_capture_begin();
    FF_Status_t ok_st = ff_req_encode(FF_APID_TC, FF_PUS_SERVICE_TEST, 1u, FF_PUS_ACK_COMPLETION,
                                      big, (uint16_t)(sizeof(big) + 1u), frame, &frame_len);
    ff_req_capture_end();

    if (!echoed) {
        ff_req_fail("SRS-REQ-003", "L6", "max_payload_not_echoed");
    }
    else if ((ok_st != FF_BAD_SIZE) || (frame_len != 0u)) {
        ff_req_fail("SRS-REQ-003", "L6", "over_max_not_refused_at_encode");
    }
    else {
        ff_req_pass("SRS-REQ-003", "L6", "1024_echoed_1025_refused");
    }
}

/* SRS-REQ-010 (L7): the CUC set by S9.1 is reported by S9.2 at the documented
 * coarse-second resolution. The time source and the UTC value are saved and
 * restored, because FF_TimeSetUtcNs selects the GROUND source as a side effect. */
static void ff_req_case_l7(void)
{
    static const uint8_t set_payload[6] = {0x00u, 0x00u, 0x03u, 0xE8u, 0x00u, 0x00u}; /* 1000,0 */
    const uint8_t *payload = NULL;
    uint16_t payload_len = 0u;
    uint64_t saved_utc = FF_TimeGetUtcNs();
    FF_TimeSource_t saved_src = FF_TimeGetSource();
    uint16_t n_time2;

    ff_req_capture_begin();
    (void)ff_req_send(FF_APID_TC, FF_PUS_SERVICE_TIME, 1u, FF_PUS_ACK_COMPLETION, set_payload, 6u,
                      NULL);
    (void)ff_req_send(FF_APID_TC, FF_PUS_SERVICE_TIME, 2u, FF_PUS_ACK_COMPLETION, NULL, 0u, NULL);
    ff_req_capture_end();

    n_time2 = ff_req_count(FF_PUS_SERVICE_TIME, 2u);
    bool answered = ff_req_find(FF_PUS_SERVICE_TIME, 2u, &payload, &payload_len);

    /* Restore both the UTC value and the time source changed by S9.1. */
    (void)FF_TimeSetUtcNs(saved_utc);
    (void)FF_TimeSetSource(saved_src);

    if (answered && (payload_len == 6u)) {
        uint32_t got_coarse = ((uint32_t)payload[0] << 24) | ((uint32_t)payload[1] << 16) |
                              ((uint32_t)payload[2] << 8) | (uint32_t)payload[3];
        uint16_t got_fine = (uint16_t)(((uint16_t)payload[4] << 8) | payload[5]);
        FF_PalLog("TGTREQ-OBS: SRS-REQ-010:L7:s9.2_cuc=%lu.%u src_restored=%d\n",
                  (unsigned long)got_coarse, (unsigned)got_fine,
                  (FF_TimeGetSource() == saved_src) ? 1 : 0);
        if ((n_time2 != 1u) || (got_coarse != 1000u)) {
            ff_req_fail("SRS-REQ-010", "L7", "S9.2_CUC_coarse_mismatch");
            return;
        }
        ff_req_pass("SRS-REQ-010", "L7", "CUC_set_value_at_coarse_resolution");
        return;
    }
    ff_req_fail("SRS-REQ-010", "L7", "no_S9.2");
}

/* SRS-REQ-009 (L8): a registered region dumps exactly its bytes, and the four
 * refusal classes stay distinguishable at the handler. The on-wire refusal
 * format is an interface-authority item and is therefore a declared gap. */
static void ff_req_case_l8(void)
{
    const uint8_t *payload = NULL;
    uint16_t payload_len = 0u;
    uint8_t dump_ok[7] = {1u, 0u, 0u, 0u, 4u, 0u, 16u};     /* region 1, offset 4, 16 bytes */
    uint8_t dump_range[7] = {1u, 0u, 0u, 0u, 60u, 0u, 16u}; /* offset 60 + 16 > 64 */
    uint8_t dump_unknown[7] = {9u, 0u, 0u, 0u, 0u, 0u, 4u}; /* no such region */
    uint8_t dump_zero[7] = {1u, 0u, 0u, 0u, 0u, 0u, 0u};    /* length 0 */
    FF_Status_t st_range;
    FF_Status_t st_unknown;
    FF_Status_t st_zero;
    FF_Status_t st_ok;

    ff_req_capture_begin();
    st_ok = ff_req_send(FF_APID_TC, FF_PUS_SERVICE_MEMORY, 5u, FF_PUS_ACK_COMPLETION, dump_ok, 7u,
                        NULL);
    st_range = ff_req_send(FF_APID_TC, FF_PUS_SERVICE_MEMORY, 5u, FF_PUS_ACK_COMPLETION, dump_range,
                           7u, NULL);
    st_unknown = ff_req_send(FF_APID_TC, FF_PUS_SERVICE_MEMORY, 5u, FF_PUS_ACK_COMPLETION,
                             dump_unknown, 7u, NULL);
    st_zero = ff_req_send(FF_APID_TC, FF_PUS_SERVICE_MEMORY, 5u, FF_PUS_ACK_COMPLETION, dump_zero,
                          7u, NULL);
    ff_req_capture_end();

    bool dumped = ff_req_find(FF_PUS_SERVICE_MEMORY, 6u, &payload, &payload_len) &&
                  (payload_len == 16u) && (memcmp(payload, &g_req_region[4], 16u) == 0);

    if (!dumped) {
        ff_req_fail("SRS-REQ-009", "L8", "dump_bytes_mismatch");
    }
    else if (st_ok != FF_SUCCESS) {
        ff_req_fail("SRS-REQ-009", "L8", "valid_dump_status");
    }
    else if ((st_range != FF_INVALID_ARGUMENT) || (st_unknown != FF_NOT_FOUND) ||
             (st_zero != FF_BAD_SIZE)) {
        ff_req_fail("SRS-REQ-009", "L8", "refusal_status_not_distinguished");
    }
    else {
        ff_req_pass("SRS-REQ-009", "L8", "dump_exact_statuses_distinct");
    }
}

/* SRS-REQ-005 (L10): after a rejected (CRC-corrupted) frame the chain still
 * answers a valid ping with the full L1 set; no wedged state. */
static void ff_req_case_l10(void)
{
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t frame_len = 0u;
    const char *why = "unknown";
    FF_Status_t st;
    bool corrupt_rejected;

    ff_req_capture_begin();
    st = ff_req_encode(FF_APID_TC, FF_PUS_SERVICE_TEST, 1u, FF_PUS_ACK_COMPLETION, g_ping_payload,
                       sizeof(g_ping_payload), frame, &frame_len);
    corrupt_rejected = (st == FF_SUCCESS);
    if (corrupt_rejected) {
        frame[FF_CCSDS_HDR_LEN + FF_PUS_TC_SEC_HDR_LEN] ^= 0x80u;
        corrupt_rejected = (FF_PusIngest(frame, frame_len) == FF_PARSE_ERROR);
    }
    st = ff_req_ping(NULL);
    ff_req_capture_end();

    if (!corrupt_rejected) {
        ff_req_fail("SRS-REQ-005", "L10", "corrupt_frame_not_rejected");
    }
    else if (st != FF_SUCCESS) {
        ff_req_fail("SRS-REQ-005", "L10", "ping_after_corrupt_failed");
    }
    else if (ff_req_check_ping_set(&why)) {
        ff_req_pass("SRS-REQ-005", "L10", "chain_not_wedged");
    }
    else {
        ff_req_fail("SRS-REQ-005", "L10", why);
    }
}

/* ---- suite entry points ------------------------------------------ */

/* Runs the suite exactly once and returns; later calls are no-ops. */
static bool g_req_suite_ran;

FF_Status_t FF_AppReqTestRun(void)
{
    if (g_req_suite_ran) {
        return FF_SUCCESS;
    }
    g_req_suite_ran = true;

    for (uint16_t i = 0u; i < sizeof(g_req_region); i++) {
        g_req_region[i] = (uint8_t)(0xA5u ^ (uint8_t)(i & 0xFFu));
    }
    (void)FF_PusRegisterMemoryRegion(1u, g_req_region, (uint32_t)sizeof(g_req_region));

    /* Take over the TM sink for the duration of the suite (public API), then
     * hand it back to the production telemetry-output sink. */
    FF_PusSetTmSink(ff_req_tm_sink);

    ff_req_case_l1();
    ff_req_case_l2();
    ff_req_case_l3();
    ff_req_case_l4();
    ff_req_case_l5();
    ff_req_case_l6();
    ff_req_case_l7();
    ff_req_case_l8();
    ff_req_case_l10();

    /* Declared gaps: cases the T1 oracle does not assert, with the blocking
     * reference. These are never counted as coverage by the verifiers. */
    ff_req_gap("SRS-REQ-004", "L4-S18", "NC-EXT-04: S1.8 for a malformed TC is not asserted");
    ff_req_gap("SRS-REQ-005", "L8-WIRE",
               "NC-EXT-18: on-wire refusal failure code is interface-authority owned");
    ff_req_gap(
        "SRS-REQ-008", "L9",
        "NC-EXT-17: 6-byte TC[3,1] create is an unapproved extension (ICD defines 2 octets)");

    (void)FF_AppTmOutputInit();
    return FF_SUCCESS;
}

void FF_AppReqTestMain(void *arg)
{
    (void)arg;

    (void)FF_AppReqTestRun();

    for (;;) {
        FF_PalTaskDelayMs(5000u);
    }
}
