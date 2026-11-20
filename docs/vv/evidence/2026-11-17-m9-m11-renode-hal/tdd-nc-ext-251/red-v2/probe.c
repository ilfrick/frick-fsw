/* Behavioural RED probe for NC-EXT-251 defects found by R1/R3 review.
 * Uses ONLY pre-existing public API so it links against the pre-fix code. */
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"
#include "core/pal/pal.h"
#include "core/evs/evs.h"
#include "core/time/time.h"
#include <stdio.h>
#include <string.h>

static int fails = 0;
static int checks = 0;
#define CHECK(cond, msg) do { checks++; if (!(cond)) { fails++; printf("FAIL %s:%d %s\n", __FILE__, __LINE__, msg); } } while (0)

static FF_Status_t direct_writer(const uint8_t *f, uint16_t l, uint64_t d)
{ (void)f; (void)l; (void)d; return FF_SUCCESS; }

static int nested_rc_seen = 0;
static FF_Status_t nested_handler(const FF_Pus_TcHdr_t *tc, const uint8_t *d, uint16_t l)
{
    uint8_t frame[256]; uint16_t flen = 0; FF_Pus_TcHdr_t nt;
    (void)tc; (void)d; (void)l;
    memset(&nt, 0, sizeof(nt));
    nt.version = FF_PUS_VERSION;
    nt.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    nt.service = 21; nt.subservice = 99; nt.source_id = 0x0200;
    if (FF_CcsdsEncodeTc(FF_APID_TC, &nt, (const uint8_t *)"\0", 1, frame, 256, &flen) == FF_SUCCESS) {
        FF_Status_t r = FF_PusIngest(frame, flen);
        nested_rc_seen = (r == FF_BUSY) ? 1 : 2;
    }
    return FF_SUCCESS;
}

static void build_tc(uint8_t ack, uint8_t svc, uint8_t sub, uint8_t *out, uint16_t *out_len)
{
    FF_Pus_TcHdr_t tc; memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION; tc.ack_flags = ack;
    tc.service = svc; tc.subservice = sub; tc.source_id = 0x0300;
    FF_CcsdsEncodeTc(FF_APID_TC, &tc, (const uint8_t *)"\0", 1, out, 256, out_len);
}

int main(void)
{
    uint16_t v;

    /* --- R3 F-3: the two counter paths must form ONE modulo-16384 sequence --- */
    FF_PalInit(); FF_TimeInit(); FF_EvsInit(); FF_PusInit();
    FF_PusSetBackend(&FF_PusBackendOpen);

    for (uint32_t i = 0; i < 0x3FFFu; i++) { v = FF_PusNextTmCounter(); }
    CHECK(FF_PusPeekTmCounter() == 0x3FFFu, "counter reached 0x3FFF on the normal path");

    CHECK(FF_PusSendTmDirect(0x14, 0x01, (const uint8_t *)"\1", 1, UINT64_MAX, direct_writer) == FF_SUCCESS, "direct emission 1");
    CHECK(FF_PusSendTmDirect(0x14, 0x01, (const uint8_t *)"\1", 1, UINT64_MAX, direct_writer) == FF_SUCCESS, "direct emission 2");

    CHECK(FF_PusPeekTmCounter() < 16384u, "stored counter stays within [0,16383] after the direct path");
    CHECK(FF_PusPeekTmCounter() == 1u, "stored counter wrapped to 1");
    CHECK(FF_PusNextTmCounter() == 1u, "normal path continues the same sequence");

    /* --- R1-04: dispatch must be serialized even with no ACK requested --- */
    FF_EvsInit(); FF_PusInit(); FF_PusSetBackend(&FF_PusBackendOpen);
    FF_PusRegisterService(21, 4, nested_handler);
    nested_rc_seen = 0;
    {
        uint8_t frame[256]; uint16_t flen = 0;
        build_tc(0u, 21, 4, frame, &flen);
        (void)FF_PusIngest(frame, flen);
    }
    CHECK(nested_rc_seen == 1, "nested ingest refused FF_BUSY even when no ACK was requested");

    printf("checks=%d failures=%d\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
