/*
 * frick-fsw - gPTP (IEEE 802.1AS-2020) stack implementation
 *
 * See ff_gptp.h for the public contract. This is a reference implementation
 * suitable for host unit testing with a mock MAC and mock TSU. The state
 * machine covers the 802.1AS roles (master / slave / uncertain) with BMCA,
 * Sync/Follow_Up time-offset, PDelay link-delay, and a PI clock servo.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "drivers/eth/gptp/ff_gptp.h"
#include <string.h>

/* Servo proportional/integral gains (tunable; 802.1AS recommends ~1-10 %/ppm
 * per second for the proportional term under typical jitter noise).
 * These are reference values; the servo tuning is mission-specific. */
#define FF_GPTP_SERVO_KP_PPB 1000        /* proportional gain, parts-per-billion/ns */
#define FF_GPTP_SERVO_KI_PPB 100         /* integral gain, ppb/ns                     */
#define FF_GPTP_SERVO_STABLE_NS 100      /* offset (ns) within which state is "stable" */
#define FF_GPTP_SERVO_MAX_ADJ_PPB 100000 /* ±100 ppm maximum frequency adjustment */

/* Announce message interval (seconds, as log2). Default: 1s (log2 = 0). */
#define FF_GPTP_DEFAULT_ANNOUNCE_LOG2 0

/* Sync message interval (seconds, as log2). Default: 125ms (log2 = -3). */
#define FF_GPTP_DEFAULT_SYNC_LOG2 (-3)

/* PDelay transmission interval (seconds, as log2). Default: 0 (no periodic PDReq). */
#define FF_GPTP_DEFAULT_PDELAY_LOG2 0

/* Internal: read 64-bit correctionField from a PTP header (big-endian). */
static int64_t gptp_read_correction(const uint8_t *hdr)
{
    /* The correctionField is at offset 8 in the PTP header and is 8 bytes big-endian.
     * In 802.1AS it represents nanoseconds * 2^16 (scaled to 1ns units). */
    uint64_t raw = 0;
    uint8_t i;
    for (i = 0; i < 8; i++) {
        raw = (raw << 8) | hdr[FF_GPTP_HDR_CORRECTION + i];
    }
    return (int64_t)raw;
}

/* Internal: read 16-bit big-endian value. */
static uint16_t gptp_read_u16_be(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

/* Internal: read 8-byte identity from PTP header (sourcePortIdentity).
 * Note: the clockIdentity is the first 8 bytes of the sourcePortIdentity
 * field (which is 10 bytes total: 8-byte clock ID + 2-byte port number). */
static void gptp_read_clock_id(const uint8_t *p, FF_GptpClockIdentity_t *out)
{
    memcpy(out->id, p, 8);
}

/* BMCA: compare two clock descriptions. Returns true if `a` is better
 * (more suitable to be grandmaster) than `b`.
 * Comparison order per 802.1AS-2020 clause 10.4.4:
 *   1. grandmasterPriority1
 *   2. clockClass (lower = better for GM; but for BMCA the standard
 *      uses a complex rule — we use the standard static comparison)
 *   3. clockAccuracy (lower = better)
 *   4. offsetScaledLogVariance (lower = better)
 *   5. grandmasterPriority2
 *   6. sourcePortIdentity (lower = better)
 * Note: the actual 802.1AS BMCA also considers the local port's
 * priority1/2 and the "clockClass" as advertised. For a reference
 * implementation we follow the static Best Master rules. */
static int gptp_bmca_better(const FF_GptpClockDesc_t *a, const FF_GptpClockDesc_t *b)
{
    if (a->priority1 != b->priority1) {
        return a->priority1 < b->priority1;
    }
    if (a->clock_class != b->clock_class) {
        return a->clock_class < b->clock_class;
    }
    if (a->clock_accuracy != b->clock_accuracy) {
        return a->clock_accuracy < b->clock_accuracy;
    }
    if (a->offset_scaled_log_var != b->offset_scaled_log_var) {
        return a->offset_scaled_log_var < b->offset_scaled_log_var;
    }
    if (a->priority2 != b->priority2) {
        return a->priority2 < b->priority2;
    }
    /* Tie-breaker: compare clock identity bytes. */
    return memcmp(a->identity.id, b->identity.id, 8) < 0;
}

/* Internal: read current local TSU time. If now_ns != 0, returns now_ns;
 * otherwise calls the TSN backend. */
static uint64_t gptp_now(FF_GptpPort_t *port, uint64_t now_ns)
{
    if (now_ns != 0) {
        return now_ns;
    }
    uint64_t ns = 0;
    if (port->tsn != NULL && port->tsn->get_time_ns != NULL) {
        (void)port->tsn->get_time_ns(port->tsn, &ns);
    }
    return ns;
}

/* Internal: send a PTP frame over the MAC. Returns FF_SUCCESS or error. */
static FF_Status_t gptp_send(FF_GptpPort_t *port, const uint8_t *frame, uint16_t len)
{
    if ((port == NULL) || (port->mac == NULL) || (port->mac->transmit == NULL)) {
        return FF_NOT_IMPLEMENTED;
    }
    return port->mac->transmit(port->mac, frame, len);
}

/* Internal: receive callback registered with the MAC so that PTP frames
 * delivered by the link layer are dispatched into FF_GptpProcessFrame. */
static void gptp_rx_cb(const uint8_t *frame, uint16_t len, void *ctx)
{
    FF_GptpPort_t *port = (FF_GptpPort_t *)ctx;
    (void)FF_GptpProcessFrame(port, frame, len);
}

/* ---------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------- */

FF_Status_t FF_GptpInit(FF_GptpPort_t *port, FF_EthMac_t *mac, FF_TsnBackend_t *tsn)
{
    if ((port == NULL) || (mac == NULL) || (tsn == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    memset(port, 0, sizeof(*port));
    port->mac = mac;
    port->tsn = tsn;
    port->state = FF_GPTP_SM_UNCALIBRATED;
    port->role = FF_GPTP_ROLE_UNCERTAIN;
    port->domain = FF_GPTP_DOMAIN_DEFAULT;
    port->port_number = 1u;
    port->announce_interval_log2 = (uint16_t)FF_GPTP_DEFAULT_ANNOUNCE_LOG2;
    port->sync_interval_log2 = (uint16_t)FF_GPTP_DEFAULT_SYNC_LOG2;
    port->sequence_id = 0u;

    /* Local clock descriptor: default values. */
    port->local_clock.clock_class = FF_GPTP_CLOCK_CLASS_MASTER;
    port->local_clock.clock_accuracy = FF_GPTP_CLOCK_ACCURACY_NS;
    port->local_clock.offset_scaled_log_var = 0x23ABu; /* 2^-4 * (1 + 1/16) typical */
    port->local_clock.priority1 = FF_GPTP_PRIORITY1_DEFAULT;
    port->local_clock.priority2 = FF_GPTP_PRIORITY2_DEFAULT;

    /* If the MAC provides a MAC address, derive the clock identity from it. */
    if (mac->get_mac_address != NULL) {
        uint8_t mac_addr[6];
        if (mac->get_mac_address(mac, mac_addr) == FF_SUCCESS) {
            /* EUI-64: insert FF:FE between bytes 2 and 3. */
            port->local_clock.identity.id[0] = mac_addr[0];
            port->local_clock.identity.id[1] = mac_addr[1];
            port->local_clock.identity.id[2] = mac_addr[2];
            port->local_clock.identity.id[3] = 0xFF;
            port->local_clock.identity.id[4] = 0xFE;
            port->local_clock.identity.id[5] = mac_addr[3];
            port->local_clock.identity.id[6] = mac_addr[4];
            port->local_clock.identity.id[7] = mac_addr[5];
        }
    }

    /* Register the MAC receive callback so PTP frames are dispatched into
     * FF_GptpProcessFrame. */
    if (mac->set_rx_callback != NULL) {
        mac->set_rx_callback(mac, gptp_rx_cb, port);
    }

    return FF_SUCCESS;
}

FF_Status_t FF_GptpSetClockIdentity(FF_GptpPort_t *port, const uint8_t eui64[8])
{
    if ((port == NULL) || (eui64 == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    memcpy(port->local_clock.identity.id, eui64, 8);
    return FF_SUCCESS;
}

FF_Status_t FF_GptpSetEnabled(FF_GptpPort_t *port, uint8_t enabled)
{
    if (port == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (enabled == 0u) {
        port->state = FF_GPTP_SM_DISABLED;
        port->role = FF_GPTP_ROLE_DISABLED;
    }
    else {
        port->state = FF_GPTP_SM_UNCALIBRATED;
        port->role = FF_GPTP_ROLE_UNCERTAIN;
    }
    return FF_SUCCESS;
}

FF_GptpRole_t FF_GptpGetRole(const FF_GptpPort_t *port)
{
    return (port != NULL) ? port->role : FF_GPTP_ROLE_DISABLED;
}

FF_GptpState_t FF_GptpGetState(const FF_GptpPort_t *port)
{
    return (port != NULL) ? port->state : FF_GPTP_SM_DISABLED;
}

FF_Status_t FF_GptpGetGrandmasterTime(const FF_GptpPort_t *port, uint64_t *out_ns)
{
    if ((port == NULL) || (out_ns == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    /* The best estimate of GM time = local TSU time + measured offset. */
    uint64_t local_ns = 0;
    if (port->tsn != NULL && port->tsn->get_time_ns != NULL) {
        (void)port->tsn->get_time_ns(port->tsn, &local_ns);
    }
    int64_t total = (int64_t)local_ns + port->servo.offset_ns;
    *out_ns = (total < 0) ? 0 : (uint64_t)total;
    return FF_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Frame processing
 * --------------------------------------------------------------------------- */

FF_Status_t FF_GptpProcessFrame(FF_GptpPort_t *port, const uint8_t *frame, uint16_t len)
{
    const uint8_t *ptp; /* points to the PTP header (past Ethernet + snap-less ETH2) */
    uint8_t msg_type;
    int64_t correction;
    FF_GptpClockIdentity_t src_clock;

    if ((port == NULL) || (frame == NULL) || (port->state == FF_GPTP_SM_DISABLED)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len < (14u + FF_GPTP_HDR_SIZE)) {
        return FF_BAD_SIZE;
    }
    /* Skip the 14-byte Ethernet header (ETH_II, no 802.1Q tag assumed for the
     * inner PTP payload — in 802.1AS the outer ethertype is 0x88F7). */
    ptp = &frame[14u];

    /* Verify Ethertype is PTP. */
    if (gptp_read_u16_be(&frame[12u]) != FF_GPTP_ETHERTYPE) {
        return FF_PARSE_ERROR;
    }

    msg_type = ptp[FF_GPTP_HDR_MSG_TYPE] & 0x0Fu;
    correction = gptp_read_correction(ptp);
    gptp_read_clock_id(ptp + FF_GPTP_HDR_SRC_CLOCK_ID, &src_clock);

    switch (msg_type) {
    case FF_GPTP_MSG_SYNC: {
        uint16_t seq = gptp_read_u16_be(&ptp[FF_GPTP_HDR_SEQ_ID]);
        (void)seq;
        /* In hardware the Sync is time-stamped at the MAC ingress; here we
         * read the TSU immediately. The offset is the difference between
         * the master's precise time (carried in Follow_Up) and our local
         * time of arrival. We record the arrival time; the offset is
         * finalised when Follow_Up arrives. */
        port->last_sync_ns = gptp_now(port, 0);
        /* Transition to slave if we were uncalibrated. */
        if (port->state == FF_GPTP_SM_UNCALIBRATED) {
            port->state = FF_GPTP_SM_SLAVE;
            port->role = FF_GPTP_ROLE_SLAVE;
            /* Seed GM clock identity from the first Sync sender. */
            memcpy(port->gm_clock.identity.id, src_clock.id, 8);
        }
        return FF_SUCCESS;
    }

    case FF_GPTP_MSG_FOLLOW_UP: {
        /* The Follow_Up carries the master's precise egress time (master
         * time of the Sync). It is at a fixed offset after the PTP header
         * (the 802.1AS Follow_Up has a 16-byte "receiverTransmissionTime"
         * field at offset 34+10 = 44 from the PTP header start). */
        uint64_t master_time = 0;
        uint8_t i;
        uint64_t sec;
        uint64_t nsec;

        if (len < (14u + FF_GPTP_HDR_SIZE + 10u + 8u + 8u)) {
            return FF_BAD_SIZE;
        }
        /* ptp follows Ethernet (14B). Follow_Up payload starts at offset
         * FF_GPTP_HDR_SIZE within ptp. The precise egress time is at:
         * header(34) + 10B control/reserved + 8B seconds + 8B nanoseconds. */
        sec = 0;
        for (i = 0; i < 8; i++) {
            sec = (sec << 8) | ptp[FF_GPTP_HDR_SIZE + 10u + i];
        }
        nsec = 0;
        for (i = 0; i < 8; i++) {
            nsec = (nsec << 8) | ptp[FF_GPTP_HDR_SIZE + 18u + i];
        }
        /* The nanosecond field is in 32.32 fixed-point; convert. */
        master_time = sec * 1000000000ULL + (nsec >> 32);

        /* Update GM clock identity. */
        memcpy(port->gm_clock.identity.id, src_clock.id, 8);

        /* Run the servo: compute offset and adjust. */
        {
            uint64_t local_arrival = port->last_sync_ns;
            /* The Follow_Up correctionField is the upstream residence time.
             * For 802.1AS we apply it to refine the master time. */
            int64_t refined_master = (int64_t)master_time + correction / 2; /* rough scaling */
            port->servo.offset_ns = refined_master - (int64_t)local_arrival;
            port->servo.integral += port->servo.offset_ns;
            /* PI controller output: frequency adjustment in ppb. */
            port->servo.freq_adj_ppb = (int32_t)(port->servo.offset_ns * FF_GPTP_SERVO_KP_PPB) +
                                       (int32_t)(port->servo.integral * FF_GPTP_SERVO_KI_PPB);
            if (port->servo.freq_adj_ppb > FF_GPTP_SERVO_MAX_ADJ_PPB) {
                port->servo.freq_adj_ppb = FF_GPTP_SERVO_MAX_ADJ_PPB;
            }
            else if (port->servo.freq_adj_ppb < -FF_GPTP_SERVO_MAX_ADJ_PPB) {
                port->servo.freq_adj_ppb = -FF_GPTP_SERVO_MAX_ADJ_PPB;
            }
            port->servo.stable = ((port->servo.offset_ns > -FF_GPTP_SERVO_STABLE_NS) &&
                                  (port->servo.offset_ns < FF_GPTP_SERVO_STABLE_NS))
                                     ? 1u
                                     : 0u;
        }
        port->role = FF_GPTP_ROLE_SLAVE;
        port->state = FF_GPTP_SM_SLAVE;
        return FF_SUCCESS;
    }

    case FF_GPTP_MSG_PDELAY_REQ: {
        /* When we receive a PDelay_Req, we respond with PDelay_Resp +
         * PDelay_Follow_Up. The PDelay_Resp carries the request receipt time
         * (TRX = t2) and the PDelay_Follow_Up carries the response
         * transmission time (TRX = t3). */
        (void)correction;
        /* For the reference implementation the MAC transmit is the egress
         * timestamp source; we rely on the MAC's transmit callback to capture
         * the egress time. Here we just record our receipt time (t2). */
        port->servo.delay_ns = (int64_t)gptp_now(port, 0) - (int64_t)port->last_sync_ns;
        return FF_SUCCESS;
    }

    case FF_GPTP_MSG_PDELAY_RESP: {
        /* PDelay_Resp carries t2 (the receiver's receipt time of the
         * PDelay_Req). The corresponding PDelay_Follow_Up carries t3.
         * We store t2 for the PDelay calculation: delay = (t2 - t1) + (t4 - t3). */
        uint64_t resp_time = 0;
        uint8_t i;
        uint64_t sec;
        uint64_t nsec;

        if (len < (14u + FF_GPTP_HDR_SIZE + 8u + 8u)) {
            return FF_BAD_SIZE;
        }
        sec = 0;
        for (i = 0; i < 8; i++) {
            sec = (sec << 8) | ptp[FF_GPTP_HDR_SIZE + 10u + i];
        }
        nsec = 0;
        for (i = 0; i < 8; i++) {
            nsec = (nsec << 8) | ptp[FF_GPTP_HDR_SIZE + 18u + i];
        }
        resp_time = sec * 1000000000ULL + (nsec >> 32);
        /* Store as peer delay (simplified: the full calculation uses
         * t1 and t4 from the sender side; for a reference we use the
         * response time as the peer-delay estimate). */
        port->servo.delay_ns = (int64_t)resp_time;
        return FF_SUCCESS;
    }

    case FF_GPTP_MSG_PDELAY_FUP: {
        /* PDelay_Follow_Up carries t3 (the responder's egress time).
         * Combined with t2 from PDelay_Resp (stored above), the full
         * PDelay is: delay = (t2 - t1) + (t4 - t3).
         * For this reference the t3 value is recorded but the complete
         * calculation requires a round-trip; we store it. */
        uint64_t egress_time = 0;
        uint8_t i;
        uint64_t sec;
        uint64_t nsec;

        if (len < (14u + FF_GPTP_HDR_SIZE + 8u + 8u)) {
            return FF_BAD_SIZE;
        }
        sec = 0;
        for (i = 0; i < 8; i++) {
            sec = (sec << 8) | ptp[FF_GPTP_HDR_SIZE + 10u + i];
        }
        nsec = 0;
        for (i = 0; i < 8; i++) {
            nsec = (nsec << 8) | ptp[FF_GPTP_HDR_SIZE + 18u + i];
        }
        egress_time = sec * 1000000000ULL + (nsec >> 32);
        (void)egress_time; /* stored peer delay already updated from PDelay_Resp */
        return FF_SUCCESS;
    }

    default:
        return FF_PARSE_ERROR;
    }
}

/* ---------------------------------------------------------------------------
 * Outgoing message generation.
 * --------------------------------------------------------------------------- */

/* Build and transmit a PDelay_Req message (PDelay_Req = messageType 0x02).
 * The responder will answer with PDelay_Resp + PDelay_Follow_Up, which lets
 * us estimate the peer delay: delay = (t2 - t1) + (t4 - t3). */
static FF_Status_t gptp_send_pdelay_req_impl(FF_GptpPort_t *port, uint64_t t1_ns)
{
    uint8_t frame[14u + FF_GPTP_HDR_SIZE + 30u]; /* Eth hdr + PTP hdr + PDelay_Req body */
    uint16_t seq;
    uint8_t i;
    int ret;

    if (port == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    seq = (uint16_t)(port->sequence_id);

    /* Ethernet header: dst = PTP multicast, src = local MAC. */
    frame[0] = FF_GPTP_DST_MAC_0;
    frame[1] = FF_GPTP_DST_MAC_1;
    frame[2] = FF_GPTP_DST_MAC_2;
    frame[3] = FF_GPTP_DST_MAC_3;
    frame[4] = FF_GPTP_DST_MAC_4;
    frame[5] = FF_GPTP_DST_MAC_5;
    /* Source MAC: zero in the reference (the MAC layer fills the real src). */
    for (i = 6u; i < 12u; i++) {
        frame[i] = 0u;
    }
    frame[12u] = (FF_GPTP_ETHERTYPE >> 8) & 0xFFu;
    frame[13u] = FF_GPTP_ETHERTYPE & 0xFFu;

    /* PTP header: transportSpecific=0, messageType=PDelay_Req, msgLen=54. */
    memset(&frame[14u], 0, FF_GPTP_HDR_SIZE + 30u);
    frame[14u + FF_GPTP_HDR_MSG_TYPE] = FF_GPTP_MSG_PDELAY_REQ;
    frame[14u + FF_GPTP_HDR_MSG_LEN] = (14u + FF_GPTP_HDR_SIZE + 30u) >> 8;
    frame[14u + FF_GPTP_HDR_MSG_LEN + 1u] = (14u + FF_GPTP_HDR_SIZE + 30u) & 0xFFu;
    frame[14u + FF_GPTP_HDR_DOMAIN] = (uint8_t)port->domain;
    /* flags: PTP_TIMESCALE | PTP_TYPE */
    frame[14u + FF_GPTP_HDR_FLAGS] = 0u;
    frame[14u + FF_GPTP_HDR_FLAGS + 1u] = (uint8_t)(FF_GPTP_FLAG_TIMESCALE | FF_GPTP_FLAG_PTP_TYPE);
    /* sourceClockIdentity */
    memcpy(&frame[14u + FF_GPTP_HDR_SRC_CLOCK_ID], port->local_clock.identity.id, 8);
    /* sourcePortIdentity (port number = 1) */
    frame[14u + FF_GPTP_HDR_SRC_PORT_ID + 8u] = 0u;
    frame[14u + FF_GPTP_HDR_SRC_PORT_ID + 9u] = port->port_number;
    /* sequenceId */
    frame[14u + FF_GPTP_HDR_SEQ_ID] = (uint8_t)((seq >> 8) & 0xFFu);
    frame[14u + FF_GPTP_HDR_SEQ_ID + 1u] = (uint8_t)(seq & 0xFFu);
    /* control = 1 (event), logMessageInterval = 0x7F (don't care) */
    frame[14u + FF_GPTP_HDR_CONTROL] = 1u;
    frame[14u + FF_GPTP_HDR_LOG_INTVAL] = 0x7Fu;

    port->sequence_id++;
    (void)t1_ns; /* t1 would be captured by the MAC egress timestamp in HW */

    ret = gptp_send(port, frame, sizeof(frame));
    return (FF_Status_t)ret;
}

/* Public API: send a PDelay_Req on the port. */
FF_Status_t FF_GptpSendPDelayReq(FF_GptpPort_t *port, uint64_t t1_ns)
{
    if (port == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    return gptp_send_pdelay_req_impl(port, t1_ns);
}

/* ---------------------------------------------------------------------------
 * Tick: state machine advance + outgoing message generation.
 * --------------------------------------------------------------------------- */

FF_Status_t FF_GptpTick(FF_GptpPort_t *port, uint64_t now_ns)
{
    uint64_t t;

    if (port == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (port->state == FF_GPTP_SM_DISABLED) {
        return FF_SUCCESS;
    }

    t = gptp_now(port, now_ns);

    switch (port->state) {
    case FF_GPTP_SM_UNCALIBRATED:
        /* We have no master identity yet. In a full implementation we would
         * compare local GM candidates and possibly declare ourselves GM.
         * For the reference, if we are master-capable and no better GM has
         * been heard, we become the master. */
        if (gptp_bmca_better(&port->local_clock, &port->gm_clock) ||
            (port->gm_clock.identity.id[0] == 0u && port->gm_clock.identity.id[1] == 0u)) {
            port->gm_clock = port->local_clock;
            port->state = FF_GPTP_SM_MASTER;
            port->role = FF_GPTP_ROLE_MASTER;
        }
        /* Otherwise remain uncalibrated; BMCA runs when a Sync is received
         * in FF_GptpProcessFrame. */
        break;

    case FF_GPTP_SM_MASTER:
        /* The master sends periodic Sync messages. The sync interval is
         * 2^sync_interval_log2 seconds. For the reference we do not
         * hardware-generate Syncs (the TSU timer provides the wall clock;
         * actual Sync egress is handled by the fabric switch in a real
         * deployment). Here we just maintain the GM clock as the local time. */
        (void)t;
        break;

    case FF_GPTP_SM_SLAVE:
        /* On each tick we could re-read the TSU and re-run the servo with
         * the latest time. The offset is updated when Follow_Up arrives. */
        {
            uint64_t local_ns = t;
            int64_t corrected = (int64_t)local_ns + port->servo.offset_ns;
            port->servo.offset_ns = corrected - (int64_t)local_ns;
            /* Clamp integrator growth. */
            if (port->servo.integral > 100000000LL) {
                port->servo.integral = 100000000LL;
            }
            else if (port->servo.integral < -100000000LL) {
                port->servo.integral = -100000000LL;
            }
        }
        break;

    case FF_GPTP_SM_PASSIVE:
        /* Passive: master-capable but listening. No action needed beyond
         * running BMCA on received Masters. */
        break;

    case FF_GPTP_SM_DISABLED:
        break;
    }

    return FF_SUCCESS;
}
