/*
 * frick-fsw - deterministic bounded CCSDS/PUS decoder fuzz/property corpus
 * (S8-VV-FU; NC-EXT-25, independent assessment T-01/T-05)
 *
 * Before this suite the decoder was covered only by selected boundary vectors
 * (test_ccsds.c). This corpus is deterministic (fixed xorshift32 seed), and
 * resource-bounded: fixed iteration counts, fixed buffer bounds, no dynamic
 * allocation, no wall-clock dependency. It runs as an ordinary ctest target,
 * so it is executed by every CI leg, including the AddressSanitizer leg of
 * .github/workflows/ci.yml.
 *
 * Oracles are requirement-grounded, not "did not crash":
 *  - SRS-REQ-003: the primary-header Packet Data Length must describe exactly
 *    the supplied packet boundary and must be able to hold the configured PUS
 *    secondary header plus CRC trailer; anything else is rejected (FF_PARSE_ERROR).
 *  - SRS-REQ-004: frames whose CRC does not match are rejected (FF_CRC_ERROR);
 *    a single-bit change anywhere in the CRC-covered region therefore cannot
 *    be accepted.
 *  - SRS-REQ-005: a malformed frame must not be executed (marker handler not
 *    invoked); an unknown service/subservice is not executed (FF_NOT_FOUND).
 *
 * Reproducibility/anomaly retention: every assertion failure prints
 * "CORPUS ANOMALY seed=0x... iter=..." so the exact input can be regenerated
 * from (seed, iteration) - no input files are kept in flight.
 *
 * Limits: profile/oracle-dependent header cases stay with NC-EXT-04/06/13 and
 * external interoperability with NC-EXT-03. This suite claims no MC/DC, no
 * structural coverage and no qualification. NC-EXT-25 remains open until the
 * independent V&V review of this evidence is recorded.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include "transport/ccsds/ccsds.h"
#include "pus/pus.h"
#include <string.h>

/* ---- bounded corpus configuration -------------------------------- */

#define CORPUS_SEED 0x5EEDF00Du /* fixed: the corpus is reproducible forever */
#define CORPUS_RANDOM_ITERS 2048u
#define CORPUS_ROUNDTRIP_ITERS 512u
#define CORPUS_MAX_LEN 160u /* covers 6 + 13 + 96 payload + 2 CRC */
#define CORPUS_MAX_TEMPLATE 16u

static uint32_t g_rng;
static uint32_t g_iter; /* iteration of the running sub-test (anomaly context) */
static uint16_t g_tmpl; /* template index in the running sub-test (anomaly context) */

static uint32_t rnd(void)
{
    uint32_t x = g_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng = x;
    return x;
}

static void anomaly(const char *what)
{
    (void)fprintf(stderr, "CORPUS ANOMALY seed=0x%08X template=%u iter=%u: %s\n",
                  (unsigned)CORPUS_SEED, (unsigned)g_tmpl, (unsigned)g_iter, what);
}

/* ---- template packets -------------------------------------------- */

typedef struct
{
    uint8_t bytes[CORPUS_MAX_LEN];
    uint16_t len;
} pkt_t;

/* Build the deterministic template set: valid CRC-carrying TC and TM frames
 * spanning the smallest and representative payload lengths and apid extremes. */
static void build_templates(pkt_t *tc_out, uint16_t *n_tc_out, pkt_t *tm_out, uint16_t *n_tm_out)
{
    static const uint16_t apids[] = {0x000u, 0x010u, 0x7FFu};
    static const uint16_t tm_apids[] = {0x000u, 0x421u};
    static const uint8_t payload_lens[] = {0u, 1u, 32u};
    static const uint8_t tm_payload_lens[] = {0u, 16u, 64u};
    uint16_t n_tc = 0u;
    uint16_t n_tm = 0u;
    uint8_t payload[96];

    for (uint16_t i = 0u; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(i * 7u + 3u);
    }

    for (size_t a = 0; a < (sizeof(apids) / sizeof(apids[0])); a++) {
        for (size_t p = 0; p < (sizeof(payload_lens) / sizeof(payload_lens[0])); p++) {
            FF_Pus_TcHdr_t hdr;
            pkt_t *pkt = &tc_out[n_tc];
            uint16_t out_len = 0u;

            (void)memset(&hdr, 0, sizeof(hdr));
            hdr.version = 1u;
            hdr.ack_flags = 0u;
            hdr.service = 0xEEu;
            hdr.subservice = 0x01u;
            hdr.source_id = 0x0042u;
            FF_TEST_ASSERT(FF_CcsdsEncodeTc(apids[a], &hdr, payload, payload_lens[p], pkt->bytes,
                                            sizeof(pkt->bytes), &out_len) == FF_SUCCESS);
            pkt->len = out_len;
            n_tc++;
        }
    }

    for (size_t a = 0; a < (sizeof(tm_apids) / sizeof(tm_apids[0])); a++) {
        for (size_t p = 0; p < (sizeof(tm_payload_lens) / sizeof(tm_payload_lens[0])); p++) {
            FF_Pus_TmHdr_t hdr;
            pkt_t *pkt = &tm_out[n_tm];
            uint16_t out_len = 0u;

            (void)memset(&hdr, 0, sizeof(hdr));
            hdr.version = 1u;
            hdr.service = 0x03u;
            hdr.subservice = 0x01u;
            hdr.counter = 0x3FFFu;
            hdr.destination_id = 0x0001u;
            hdr.time_coarse_s = 0x12345678u;
            hdr.time_fine = 0x9ABCu;
            FF_TEST_ASSERT(FF_CcsdsEncodeTm(tm_apids[a], &hdr, payload, tm_payload_lens[p],
                                            pkt->bytes, sizeof(pkt->bytes),
                                            &out_len) == FF_SUCCESS);
            pkt->len = out_len;
            n_tm++;
        }
    }
    *n_tc_out = n_tc;
    *n_tm_out = n_tm;
}

/* Internal consistency check for the CRC property (SRS-REQ-004): recompute
 * using the project CRC routine and compare with the trailer. This is not an
 * independent oracle and cannot detect a common-mode defect in that routine. */
static bool crc_matches(const uint8_t *raw, uint16_t len)
{
    uint16_t expect = FF_CcsdsCrc16(raw, (uint16_t)(len - 2u));
    uint16_t got = (uint16_t)(((uint16_t)raw[len - 2u] << 8) | raw[len - 1u]);
    return expect == got;
}

/* ---- 1. random-byte robustness ------------------------------------ */

/* Seeded pseudo-random byte streams of every length in [0, CORPUS_MAX_LEN]
 * must be rejected unless the frame is genuinely self-consistent (a random
 * stream is never expected to be accepted). */
static void test_random_bytes_rejected(void)
{
    static uint8_t buf[CORPUS_MAX_LEN];
    uint16_t accepted_tc = 0u;
    uint16_t accepted_tm = 0u;

    g_rng = CORPUS_SEED;
    for (g_iter = 0u; g_iter < CORPUS_RANDOM_ITERS; g_iter++) {
        uint16_t len = (uint16_t)(rnd() % (CORPUS_MAX_LEN + 1u));
        FF_Pus_TcHdr_t tc;
        FF_Pus_TmHdr_t tm;
        const uint8_t *data = NULL;
        uint16_t data_len = 0u;
        uint16_t apid = 0u;
        FF_Status_t rc;

        for (uint16_t i = 0u; i < len; i++) {
            buf[i] = (uint8_t)rnd();
        }

        rc = FF_CcsdsDecodeTc(buf, len, &apid, &tc, &data, &data_len);
        if (rc == FF_SUCCESS) {
            accepted_tc++;
            anomaly("random frame accepted by FF_CcsdsDecodeTc");
            /* If it was accepted it must be genuinely CRC-consistent
             * (SRS-REQ-004) and internally bounded (SRS-REQ-003). */
            FF_TEST_ASSERT(crc_matches(buf, len));
            FF_TEST_ASSERT((data != NULL) && (data + data_len <= buf + len));
        }
        else {
            FF_TEST_ASSERT((rc == FF_PARSE_ERROR) || (rc == FF_CRC_ERROR));
        }

        rc = FF_CcsdsDecodeTm(buf, len, &apid, &tm, &data, &data_len);
        if (rc == FF_SUCCESS) {
            accepted_tm++;
            anomaly("random frame accepted by FF_CcsdsDecodeTm");
            FF_TEST_ASSERT(crc_matches(buf, len));
            FF_TEST_ASSERT((data != NULL) && (data + data_len <= buf + len));
        }
        else {
            FF_TEST_ASSERT((rc == FF_PARSE_ERROR) || (rc == FF_CRC_ERROR));
        }
    }

    /* Expected result: a random stream is not a valid space packet. */
    FF_TEST_ASSERT(accepted_tc == 0u);
    FF_TEST_ASSERT(accepted_tm == 0u);
}

/* NULL and degenerate inputs are rejected without dereference (ASan leg). */
static void test_degenerate_inputs_rejected(void)
{
    FF_Pus_TcHdr_t tc;
    FF_Pus_TmHdr_t tm;
    const uint8_t *data = NULL;
    uint16_t data_len = 0u;
    uint8_t one[1] = {0xA5u};

    FF_TEST_ASSERT(FF_CcsdsDecodeTc(NULL, 16u, NULL, &tc, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTm(NULL, 16u, NULL, &tm, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(one, 0u, NULL, &tc, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTm(one, 0u, NULL, &tm, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(one, 1u, NULL, &tc, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTm(one, 1u, NULL, &tm, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(one, 5u, NULL, &tc, &data, &data_len) == FF_PARSE_ERROR);
    /* NULL secondary-header pointer is a rejected argument. */
    FF_TEST_ASSERT(FF_CcsdsDecodeTc(one, 5u, NULL, NULL, &data, &data_len) == FF_PARSE_ERROR);
    FF_TEST_ASSERT(FF_CcsdsDecodeTm(one, 5u, NULL, NULL, &data, &data_len) == FF_PARSE_ERROR);
}

/* ---- 2. length-boundary corpus (SRS-REQ-003) ---------------------- */

/* For every valid template: every truncated length and every extended length
 * must be rejected with FF_PARSE_ERROR - the primary-header length must
 * describe exactly the supplied packet boundary. */
static void test_length_boundary_exact(const pkt_t *pkts, uint16_t n, bool is_tc)
{
    for (uint16_t t = 0u; t < n; t++) {
        const pkt_t *pkt = &pkts[t];
        g_tmpl = t;

        /* Truncation at every length 0 .. total-1. */
        for (g_iter = 0u; g_iter < pkt->len; g_iter++) {
            FF_Pus_TcHdr_t tc;
            FF_Pus_TmHdr_t tm;
            const uint8_t *data = NULL;
            uint16_t data_len = 0u;
            FF_Status_t rc =
                is_tc ? FF_CcsdsDecodeTc(pkt->bytes, g_iter, NULL, &tc, &data, &data_len)
                      : FF_CcsdsDecodeTm(pkt->bytes, g_iter, NULL, &tm, &data, &data_len);
            if (rc != FF_PARSE_ERROR) {
                anomaly("truncated frame not rejected with FF_PARSE_ERROR");
            }
            FF_TEST_ASSERT(rc == FF_PARSE_ERROR);
        }

        /* Extension: trailing bytes past the declared boundary are rejected. */
        {
            static uint8_t ext[CORPUS_MAX_LEN + 8u];
            uint8_t extra;

            for (extra = 1u; extra <= 4u; extra++) {
                g_iter = extra;
                FF_Pus_TcHdr_t tc;
                FF_Pus_TmHdr_t tm;
                const uint8_t *data = NULL;
                uint16_t data_len = 0u;
                uint16_t len = (uint16_t)(pkt->len + extra);
                FF_Status_t rc;

                (void)memcpy(ext, pkt->bytes, pkt->len);
                (void)memset(&ext[pkt->len], 0x5Au, extra);
                rc = is_tc ? FF_CcsdsDecodeTc(ext, len, NULL, &tc, &data, &data_len)
                           : FF_CcsdsDecodeTm(ext, len, NULL, &tm, &data, &data_len);
                if (rc != FF_PARSE_ERROR) {
                    anomaly("extended frame not rejected with FF_PARSE_ERROR");
                }
                FF_TEST_ASSERT(rc == FF_PARSE_ERROR);
            }
        }

        /* Packet Data Length field that declares less than one secondary
         * header plus CRC trailer must be rejected by the minimum-length rule
         * itself. The frame is sized to exactly the declared length so that the
         * minimum-length rule (not the preceding "frame shorter than the
         * secondary header" guard and not the "declared length must equal the
         * supplied length" rule) is the only failing condition. Before this
         * correction the vector supplied only `FF_CCSDS_HDR_LEN + 1` octets, so
         * the decoder rejected it on the short-frame guard and the
         * minimum-length rule was never exercised (FRICK-REVIEW-0379
         * F-0379-01). */
        if (pkt->len >= FF_CCSDS_HDR_LEN) {
            g_iter = t;
            FF_Pus_TcHdr_t tc;
            FF_Pus_TmHdr_t tm;
            const uint8_t *data = NULL;
            uint16_t data_len = 0u;
            static uint8_t small[CORPUS_MAX_LEN];
            FF_Status_t rc;
            uint16_t sec_len =
                is_tc ? (uint16_t)FF_PUS_TC_SEC_HDR_LEN : (uint16_t)FF_PUS_TM_SEC_HDR_LEN;
            uint16_t minimum = (uint16_t)(sec_len + (FF_FRAME_WITH_CRC ? 2u : 0u));
            uint16_t declared = (uint16_t)(minimum - 1u); /* one byte below minimum */
            uint16_t supplied = (uint16_t)(FF_CCSDS_HDR_LEN + declared);
            uint16_t pdl = (uint16_t)(declared - 1u);

            FF_TEST_ASSERT(supplied <= CORPUS_MAX_LEN);
            /* Copy only the primary header from a real frame (preserving the
             * packet-type and version bits) and synthesise the rest, so the
             * sub-case is independent of the template's own length. */
            (void)memcpy(small, pkt->bytes, FF_CCSDS_HDR_LEN);
            (void)memset(&small[FF_CCSDS_HDR_LEN], 0, (size_t)(supplied - FF_CCSDS_HDR_LEN));
            small[4] = (uint8_t)(pdl >> 8);
            small[5] = (uint8_t)(pdl & 0xFFu);
            rc = is_tc ? FF_CcsdsDecodeTc(small, supplied, NULL, &tc, &data, &data_len)
                       : FF_CcsdsDecodeTm(small, supplied, NULL, &tm, &data, &data_len);
            if (rc != FF_PARSE_ERROR) {
                anomaly("declared data length below the secondary-header+CRC minimum accepted");
            }
            FF_TEST_ASSERT(rc == FF_PARSE_ERROR);
        }
    }
}

static void test_length_boundary(void)
{
    static pkt_t tc[CORPUS_MAX_TEMPLATE];
    static pkt_t tm[CORPUS_MAX_TEMPLATE];
    uint16_t n_tc = 0u;
    uint16_t n_tm = 0u;

    build_templates(tc, &n_tc, tm, &n_tm);
    FF_TEST_ASSERT((n_tc > 0u) && (n_tm > 0u));
    test_length_boundary_exact(tc, n_tc, true);
    test_length_boundary_exact(tm, n_tm, false);
}

/* ---- 3. single-bit mutation corpus (SRS-REQ-004) ------------------ */

/* Flip every bit of every byte of every template: no mutated frame may be
 * accepted (CRC-16/CCITT-FALSE detects all single-bit errors in the covered
 * region), and an FF_CRC_ERROR verdict must be consistent with an internal
 * CRC recomputation. The check shares FF_CcsdsCrc16 with the decoder. */
static void test_single_bit_mutations_rejected(const pkt_t *pkts, uint16_t n, bool is_tc)
{
    static uint8_t mutated[CORPUS_MAX_LEN];
    uint32_t cases = 0u;

    for (uint16_t t = 0u; t < n; t++) {
        const pkt_t *pkt = &pkts[t];

        for (uint16_t i = 0u; i < pkt->len; i++) {
            for (uint8_t bit = 0u; bit < 8u; bit++) {
                FF_Pus_TcHdr_t tc;
                FF_Pus_TmHdr_t tm;
                const uint8_t *data = NULL;
                uint16_t data_len = 0u;
                FF_Status_t rc;

                g_iter = cases;
                (void)memcpy(mutated, pkt->bytes, pkt->len);
                mutated[i] ^= (uint8_t)(1u << bit);
                cases++;

                rc = is_tc ? FF_CcsdsDecodeTc(mutated, pkt->len, NULL, &tc, &data, &data_len)
                           : FF_CcsdsDecodeTm(mutated, pkt->len, NULL, &tm, &data, &data_len);
                if (rc == FF_SUCCESS) {
                    anomaly("single-bit mutation accepted (SRS-REQ-004 violation)");
                    FF_TEST_ASSERT(rc != FF_SUCCESS);
                    continue;
                }
                if (rc == FF_CRC_ERROR) {
                    /* Verdict consistency only: this check shares the CRC
                     * implementation with the decoder and is not independent. */
                    if (crc_matches(mutated, pkt->len)) {
                        anomaly("FF_CRC_ERROR reported although CRC matches");
                    }
                    FF_TEST_ASSERT(!crc_matches(mutated, pkt->len));
                }
                else {
                    FF_TEST_ASSERT(rc == FF_PARSE_ERROR);
                }
            }
        }
    }
    FF_TEST_ASSERT(cases > 0u); /* the corpus actually exercised cases */
}

static void test_single_bit_mutations(void)
{
    static pkt_t tc[CORPUS_MAX_TEMPLATE];
    static pkt_t tm[CORPUS_MAX_TEMPLATE];
    uint16_t n_tc = 0u;
    uint16_t n_tm = 0u;

    build_templates(tc, &n_tc, tm, &n_tm);
    FF_TEST_ASSERT((n_tc > 0u) && (n_tm > 0u));
    test_single_bit_mutations_rejected(tc, n_tc, true);
    test_single_bit_mutations_rejected(tm, n_tm, false);
}

/* Cross-type rejection: a TC packet presented to the TM decoder (and vice
 * versa) is rejected on the packet-type bit. */
static void test_cross_type_rejected(void)
{
    static pkt_t tc[CORPUS_MAX_TEMPLATE];
    static pkt_t tm[CORPUS_MAX_TEMPLATE];
    uint16_t n_tc = 0u;
    uint16_t n_tm = 0u;
    FF_Pus_TcHdr_t tc_hdr;
    FF_Pus_TmHdr_t tm_hdr;
    const uint8_t *data = NULL;
    uint16_t data_len = 0u;

    build_templates(tc, &n_tc, tm, &n_tm);
    for (uint16_t t = 0u; t < n_tc; t++) {
        FF_TEST_ASSERT(FF_CcsdsDecodeTm(tc[t].bytes, tc[t].len, NULL, &tm_hdr, &data, &data_len) ==
                       FF_PARSE_ERROR);
    }
    for (uint16_t t = 0u; t < n_tm; t++) {
        FF_TEST_ASSERT(FF_CcsdsDecodeTc(tm[t].bytes, tm[t].len, NULL, &tc_hdr, &data, &data_len) ==
                       FF_PARSE_ERROR);
    }
}

/* ---- 4. seeded encode/decode round-trip property ------------------- */

/* Randomised-but-seeded valid frames: the decoder must recover exactly what
 * the encoder produced (fields, payload, bounds). */
static void test_roundtrip_property(void)
{
    static const uint16_t apids[] = {0x000u, 0x010u, 0x7FFu, 0x123u};
    static const uint8_t services[] = {1u, 3u, 5u, 8u, 9u, 0xF3u, 0xEEu};
    static uint8_t payload[96];
    static uint8_t out[CORPUS_MAX_LEN];

    g_rng = CORPUS_SEED ^ 0xA5A5A5A5u;
    for (g_iter = 0u; g_iter < CORPUS_ROUNDTRIP_ITERS; g_iter++) {
        uint16_t apid = apids[rnd() % (sizeof(apids) / sizeof(apids[0]))];
        uint16_t plen = (uint16_t)(rnd() % (sizeof(payload) + 1u));
        uint16_t out_len = 0u;
        uint16_t dec_len = 0u;
        uint16_t dec_apid = 0u;
        const uint8_t *data = NULL;
        uint8_t service = services[rnd() % (sizeof(services) / sizeof(services[0]))];
        uint8_t subservice = (uint8_t)rnd();
        uint8_t ack = (uint8_t)(rnd() & 0x0Fu);
        bool is_tc = ((g_iter & 1u) == 0u);

        for (uint16_t i = 0u; i < plen; i++) {
            payload[i] = (uint8_t)rnd();
        }

        if (is_tc) {
            FF_Pus_TcHdr_t hdr;
            FF_Pus_TcHdr_t dec;

            (void)memset(&hdr, 0, sizeof(hdr));
            hdr.version = 1u;
            hdr.ack_flags = ack;
            hdr.service = service;
            hdr.subservice = subservice;
            hdr.source_id = (uint16_t)rnd();

            FF_TEST_ASSERT(FF_CcsdsEncodeTc(apid, &hdr, payload, plen, out, sizeof(out),
                                            &out_len) == FF_SUCCESS);
            if (FF_CcsdsDecodeTc(out, out_len, &dec_apid, &dec, &data, &dec_len) != FF_SUCCESS) {
                anomaly("valid TC frame not decoded");
                FF_TEST_ASSERT(false);
                continue;
            }
            FF_TEST_ASSERT(dec_apid == apid);
            FF_TEST_ASSERT(dec.version == 1u);
            FF_TEST_ASSERT(dec.ack_flags == ack);
            FF_TEST_ASSERT(dec.service == service);
            FF_TEST_ASSERT(dec.subservice == subservice);
            FF_TEST_ASSERT(dec.source_id == hdr.source_id);
            FF_TEST_ASSERT(dec_len == plen);
            FF_TEST_ASSERT((data != NULL) && (data + dec_len <= out + out_len));
            if (plen > 0u) {
                FF_TEST_ASSERT(memcmp(data, payload, plen) == 0);
            }
            FF_TEST_ASSERT(crc_matches(out, out_len));
        }
        else {
            FF_Pus_TmHdr_t hdr;
            FF_Pus_TmHdr_t dec;

            (void)memset(&hdr, 0, sizeof(hdr));
            hdr.version = 1u;
            hdr.service = service;
            hdr.subservice = subservice;
            hdr.counter = (uint16_t)(rnd() & 0x3FFFu);
            hdr.destination_id = (uint16_t)rnd();
            hdr.time_coarse_s = rnd();
            hdr.time_fine = (uint16_t)rnd();

            FF_TEST_ASSERT(FF_CcsdsEncodeTm(apid, &hdr, payload, plen, out, sizeof(out),
                                            &out_len) == FF_SUCCESS);
            if (FF_CcsdsDecodeTm(out, out_len, &dec_apid, &dec, &data, &dec_len) != FF_SUCCESS) {
                anomaly("valid TM frame not decoded");
                FF_TEST_ASSERT(false);
                continue;
            }
            FF_TEST_ASSERT(dec_apid == apid);
            FF_TEST_ASSERT(dec.version == 1u);
            FF_TEST_ASSERT(dec.service == service);
            FF_TEST_ASSERT(dec.subservice == subservice);
            FF_TEST_ASSERT(dec.counter == hdr.counter);
            FF_TEST_ASSERT(dec.destination_id == hdr.destination_id);
            FF_TEST_ASSERT(dec.time_coarse_s == hdr.time_coarse_s);
            FF_TEST_ASSERT(dec.time_fine == hdr.time_fine);
            FF_TEST_ASSERT(dec_len == plen);
            FF_TEST_ASSERT((data != NULL) && (data + dec_len <= out + out_len));
            if (plen > 0u) {
                FF_TEST_ASSERT(memcmp(data, payload, plen) == 0);
            }
            FF_TEST_ASSERT(crc_matches(out, out_len));
        }
    }
}

/* ---- 5. bounded FF_PusIngest corpus (SRS-REQ-005) ----------------- */

static uint16_t g_marker_calls;

static FF_Status_t corpus_marker_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *data,
                                         uint16_t len)
{
    (void)tc;
    (void)data;
    (void)len;
    g_marker_calls++;
    return FF_SUCCESS;
}

/* Returns 0 if every observed status is in the documented set, else the
 * offending iteration + 1 (0 means "no violation" so it is safe as a count). */
static uint32_t ingest_observed_violations(void)
{
    static uint8_t buf[CORPUS_MAX_LEN];
    uint32_t accepted = 0u;
    uint32_t tc_before;
    uint32_t violations = 0u;

    (void)FF_PusInit();
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    tc_before = FF_PusGetTcCount();

    g_rng = CORPUS_SEED ^ 0x0BADF00Du;
    for (g_iter = 0u; g_iter < CORPUS_RANDOM_ITERS; g_iter++) {
        uint16_t len = (uint16_t)(rnd() % (CORPUS_MAX_LEN + 1u));
        FF_Status_t rc;

        for (uint16_t i = 0u; i < len; i++) {
            buf[i] = (uint8_t)rnd();
        }
        rc = FF_PusIngest(buf, len);
        if (rc == FF_SUCCESS) {
            accepted++;
        }
        else if ((rc != FF_PARSE_ERROR) && (rc != FF_ERROR) && (rc != FF_NOT_FOUND) &&
                 (rc != FF_NOT_READY)) {
            violations++;
            anomaly("FF_PusIngest returned an undocumented status for a random buffer");
        }
    }

    FF_TEST_ASSERT(accepted == 0u);   /* expected result: no random frame is executed */
    FF_TEST_ASSERT(violations == 0u); /* every other status is from the documented set */
    FF_TEST_ASSERT(FF_PusGetTcCount() == tc_before); /* no random frame was counted */
    return violations;
}

static void test_pus_ingest_corpus(void)
{
    static const uint16_t apids[] = {0x010u, 0x7FFu};
    static uint8_t out[CORPUS_MAX_LEN];
    uint8_t payload[8] = {0xDEu, 0xADu, 0xBEu, 0xEFu, 1u, 2u, 3u, 4u};

    (void)FF_PalInit();
    (void)FF_SbInit();
    (void)FF_EvsInit();
    (void)FF_PusInit();
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);

    /* Marker service: proves execution / non-execution directly (SRS-REQ-005). */
    g_marker_calls = 0u;
    FF_TEST_ASSERT(FF_PusRegisterService(0xEEu, 0x02u, corpus_marker_handler) == FF_SUCCESS);

    for (size_t a = 0; a < (sizeof(apids) / sizeof(apids[0])); a++) {
        FF_Pus_TcHdr_t hdr;
        uint16_t out_len = 0u;
        uint32_t marker_before;

        (void)memset(&hdr, 0, sizeof(hdr));
        hdr.version = 1u;
        hdr.ack_flags = 0u; /* no acknowledgement traffic: no TM is generated */
        hdr.service = 0xEEu;
        hdr.subservice = 0x02u;
        hdr.source_id = 0x0007u;

        FF_TEST_ASSERT(FF_CcsdsEncodeTc(apids[a], &hdr, payload, sizeof(payload), out, sizeof(out),
                                        &out_len) == FF_SUCCESS);
        marker_before = g_marker_calls;

        if (apids[a] == 0x010u) {
            /* Valid TC on the telecommand APID: the handler executes. */
            FF_TEST_ASSERT(FF_PusIngest(out, out_len) == FF_SUCCESS);
            FF_TEST_ASSERT(g_marker_calls == marker_before + 1u);

            /* Same frame, corrupted trailer: rejected (SRS-REQ-004) and the
             * handler is NOT executed (SRS-REQ-005 "malformed frame shall not
             * be executed"). */
            out[out_len - 1u] ^= 0x5Au;
            FF_TEST_ASSERT(FF_PusIngest(out, out_len) == FF_PARSE_ERROR);
            FF_TEST_ASSERT(g_marker_calls == marker_before + 1u);

            /* Same frame, truncated: not executed either. */
            out[out_len - 1u] ^= 0x5Au; /* restore */
            FF_TEST_ASSERT(FF_PusIngest(out, out_len - 1u) == FF_PARSE_ERROR);
            FF_TEST_ASSERT(g_marker_calls == marker_before + 1u);
        }
        else {
            /* Valid CRC on a non-telecommand APID: rejected as illegal APID
             * without execution. */
            FF_TEST_ASSERT(FF_PusIngest(out, out_len) == FF_ERROR);
            FF_TEST_ASSERT(g_marker_calls == marker_before);
        }

        /* Unknown service/subservice on the telecommand APID, ack=0: the frame
         * is decoded and CRC-valid, but the service is not registered, so it is
         * not executed (SRS-REQ-005 -> FF_NOT_FOUND; no completion ack was
         * requested, so no service 1 packet is generated either). */
        if (apids[a] == 0x010u) {
            FF_Pus_TcHdr_t hdr2;
            uint16_t out2_len = 0u;
            uint8_t out2[CORPUS_MAX_LEN];

            (void)memset(&hdr2, 0, sizeof(hdr2));
            hdr2.version = 1u;
            hdr2.ack_flags = 0u;
            hdr2.service = 0xEFu; /* never registered */
            hdr2.subservice = 0x03u;
            hdr2.source_id = 0x0007u;
            marker_before = g_marker_calls;
            FF_TEST_ASSERT(FF_CcsdsEncodeTc(apids[a], &hdr2, payload, sizeof(payload), out2,
                                            sizeof(out2), &out2_len) == FF_SUCCESS);
            FF_TEST_ASSERT(FF_PusIngest(out2, out2_len) == FF_NOT_FOUND);
            FF_TEST_ASSERT(g_marker_calls == marker_before); /* still not executed */
        }
    }

    /* Randomised byte streams through the full ingest path. Re-initialises the
     * PUS service table (services are reset), so the marker service registered
     * above is gone there - the random-input oracle therefore does not depend on
     * handler execution. */
    (void)ingest_observed_violations();
}

FF_TEST_MAIN(test_degenerate_inputs_rejected, test_random_bytes_rejected, test_length_boundary,
             test_single_bit_mutations, test_cross_type_rejected, test_roundtrip_property,
             test_pus_ingest_corpus)
