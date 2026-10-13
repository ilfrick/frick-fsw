/*
 * frick-fsw - gPTP (IEEE 802.1AS-2020) grandmaster/follower stack
 *
 * Reference implementation of the 802.1AS gPTP protocol: BMCA (Best Master
 * Clock Algorithm) for grandmaster selection, Sync/Follow_Up time-offset
 * calculation, PDelay (Peer Delay / PDiff) link-delay measurement, and a
 * two-stage PI servo to adjust the local clock.
 *
 * The stack is designed to run on a single Ethernet port driven by a MAC
 * backend (FF_EthMac_t) and a TSN time backend (FF_TsnBackend_t::get_time_ns,
 * backed by the GEM 1588 TSU). On the target the TSU provides hardware-
 * timestamped Sync/Follow_Up; on the host the TSU is mock-backed for unit
 * testing.
 *
 * The fabric CoreTSN / 802.1CB (FRER) path is tracked separately (SRS-REQ-067
 * fabric-device contract; NC-S8-02).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_GPTP_H
#define FF_GPTP_H

#include "common/ff_types.h"
#include "hal/hal_eth.h"
#include "hal/hal_tsn.h"

/* ===========================================================================
 * PTP protocol constants (IEEE 1588-2019 / 802.1AS-2020).
 * =========================================================================== */

/* Ethernet PTP multicast (IEEE 802.1BA / 802.1AS). */
#define FF_GPTP_DST_MAC_0 0x01U
#define FF_GPTP_DST_MAC_1 0x80U
#define FF_GPTP_DST_MAC_2 0xC2U
#define FF_GPTP_DST_MAC_3 0x00U
#define FF_GPTP_DST_MAC_4 0x00U
#define FF_GPTP_DST_MAC_5 0x0EU

#define FF_GPTP_ETHERTYPE 0x88F7U /* PTP over Ethernet (802.1AS) */

/* Ptp messageType field values (upper nibble of the first byte). */
#define FF_GPTP_MSG_SYNC 0x00U        /* Sync */
#define FF_GPTP_MSG_FOLLOW_UP 0x08U   /* Follow_Up */
#define FF_GPTP_MSG_PDELAY_REQ 0x02U  /* PDelay_Req */
#define FF_GPTP_MSG_PDELAY_RESP 0x03U /* PDelay_Resp */
#define FF_GPTP_MSG_PDELAY_FUP 0x0AU  /* PDelay_Follow_Up */

/* PTP header (10 bytes after the 14-byte Ethernet header).
 * Fields that gPTP actually uses are parsed; the rest is skipped. */
#define FF_GPTP_HDR_TRANSP_SPEC 0u
#define FF_GPTP_HDR_MSG_TYPE 1u
#define FF_GPTP_HDR_MSG_LEN 2u
#define FF_GPTP_HDR_DOMAIN 4u
#define FF_GPTP_HDR_FLAGS 6u
#define FF_GPTP_HDR_CORRECTION 8u
#define FF_GPTP_HDR_SRC_CLOCK_ID 16u
#define FF_GPTP_HDR_SRC_PORT_ID 24u
#define FF_GPTP_HDR_SEQ_ID 30u
#define FF_GPTP_HDR_CONTROL 32u
#define FF_GPTP_HDR_LOG_INTVAL 33u
#define FF_GPTP_HDR_SIZE 34u

/* flagsBitmask in 802.1AS: bit 1 = PTP_TIMESCALE, bit 2 = PTP_TYPE (TAI)    */
#define FF_GPTP_FLAG_TIMESCALE (1U << 1)
#define FF_GPTP_FLAG_PTP_TYPE (1U << 2)

/* Clock-class and quality (802.1as-2020 Table 10-868). */
#define FF_GPTP_CLOCK_CLASS_SLAVE 0x3BU  /* 59: non-grandmaster slave    */
#define FF_GPTP_CLOCK_CLASS_MASTER 0x36U /* 54: grandmaster-capable      */
#define FF_GPTP_CLOCK_ACCURACY_NS 0x21U  /* within 100 ns                 */
#define FF_GPTP_PRIORITY1_DEFAULT 248U
#define FF_GPTP_PRIORITY2_DEFAULT 248U

/* Domain number used by the gPTP application (default 0). */
#define FF_GPTP_DOMAIN_DEFAULT 0U

/* ===========================================================================
 * gPTP data types.
 * =========================================================================== */

/* A clock identity (802.1AS): the 8-byte source clockIdentity field.
 * The reference uses the MAC address padded to 8 bytes (IEEE 802.1AS rule). */
typedef struct
{
    uint8_t id[8];
} FF_GptpClockIdentity_t;

/* Clock description as advertised in Announce/Sync messages. */
typedef struct
{
    uint8_t clock_class;             /* 0-255, lower = better master            */
    uint8_t clock_accuracy;          /* sub-nanosecond accuracy code            */
    uint16_t offset_scaled_log_var;  /* offsetScaledLogVariance (x2^16)         */
    uint8_t priority1;               /* administratively preferred              */
    uint8_t priority2;               /* path priority                           */
    FF_GptpClockIdentity_t identity; /* the EUI-64 clock identity               */
} FF_GptpClockDesc_t;

/* gPTP port role (SRS-REQ-068). */
typedef enum {
    FF_GPTP_ROLE_DISABLED = 0,
    FF_GPTP_ROLE_MASTER = 1,   /* this port is the grandmaster (or a master) */
    FF_GPTP_ROLE_SLAVE = 2,    /* synchronising to an upstream master        */
    FF_GPTP_ROLE_UNCERTAIN = 3 /* waiting for BMCA / not yet stable          */
} FF_GptpRole_t;

/* gPTP state-machine states (802.1AS-2020 clause 10.2). */
typedef enum {
    FF_GPTP_SM_DISABLED = 0,
    FF_GPTP_SM_UNCALIBRATED = 1, /* waiting for first master / master info   */
    FF_GPTP_SM_SLAVE = 2,        /* locked to grandmaster                     */
    FF_GPTP_SM_MASTER = 3,       /* this node provides the master             */
    FF_GPTP_SM_PASSIVE = 4,      /* master-capable but not selected           */
} FF_GptpState_t;

/* Servo state (PI controller). */
typedef struct
{
    int64_t offset_ns;    /* current time offset (master - local), ns       */
    int64_t delay_ns;     /* peer delay measured by PDelay, ns              */
    int64_t integral;     /* integrator accumulator, ns                     */
    int32_t freq_adj_ppb; /* frequency adjustment, parts-per-billion        */
    uint32_t stable;      /* 0 = unlocking, 1 = within jitter threshold     */
} FF_GptpServo_t;

/* gPTP port (one Ethernet port). */
struct FF_GptpPort
{
    FF_EthMac_t *mac;               /* MAC backend for send/recv                    */
    FF_TsnBackend_t *tsn;           /* TSN time backend (GEM TSU)                     */
    FF_GptpClockDesc_t local_clock; /* this port's advertised clock           */
    FF_GptpClockDesc_t gm_clock;    /* best grandmaster known                    */
    FF_GptpServo_t servo;
    FF_GptpState_t state;
    FF_GptpRole_t role;
    uint16_t sequence_id;            /* running sequence counter for outgoing msgs   */
    uint64_t last_sync_ns;           /* local time of last processed Sync             */
    uint16_t announce_interval_log2; /* seconds between Announce (log2)        */
    uint16_t sync_interval_log2;     /* seconds between Sync (log2)            */
    uint32_t domain;                 /* PTP domain number                             */
    uint8_t port_number;             /* port identity (1 = first port)                 */
};

/* Opaque handle to a gPTP port. */
typedef struct FF_GptpPort FF_GptpPort_t;

/* ===========================================================================
 * Public API.
 * =========================================================================== */

/* Initialise a gPTP port on the given MAC + TSN backend pair.
 * Returns FF_SUCCESS or FF_INVALID_ARGUMENT. */
FF_Status_t FF_GptpInit(FF_GptpPort_t *port, FF_EthMac_t *mac, FF_TsnBackend_t *tsn);

/* Set the local clock identity (derived from the MAC address, 8 bytes).
 * Must be called before the first FF_GptpTick if the MAC does not provide
 * a get_mac_address hook. Returns FF_SUCCESS or FF_INVALID_ARGUMENT. */
FF_Status_t FF_GptpSetClockIdentity(FF_GptpPort_t *port, const uint8_t eui64[8]);

/* Process one gPTP tick. Receives pending MAC frames, advances the state
 * machine, runs BMCA / servo and sends any outgoing PTP messages.
 * `now_ns` is the current local TSU time; pass 0 to read it internally.
 * Returns FF_SUCCESS. */
FF_Status_t FF_GptpTick(FF_GptpPort_t *port, uint64_t now_ns);

/* Process a raw PTP Ethernet frame received from the MAC.
 * `frame` is the full Ethernet frame (14 + PTP header + payload).
 * Returns FF_SUCCESS if the frame was a recognised PTP message, or
 * FF_PARSE_ERROR / FF_NOT_VALID for unknown types. */
FF_Status_t FF_GptpProcessFrame(FF_GptpPort_t *port, const uint8_t *frame, uint16_t len);

/* Read the best current estimate of grandmaster time (ns).
 * Returns FF_SUCCESS or FF_INVALID_ARGUMENT. */
FF_Status_t FF_GptpGetGrandmasterTime(const FF_GptpPort_t *port, uint64_t *out_ns);

/* Read the current port role (master / slave / uncertain). */
FF_GptpRole_t FF_GptpGetRole(const FF_GptpPort_t *port);

/* Read the current port state. */
FF_GptpState_t FF_GptpGetState(const FF_GptpPort_t *port);

/* Enable or disable the port (administrative master disable).
 * Returns FF_SUCCESS. */
FF_Status_t FF_GptpSetEnabled(FF_GptpPort_t *port, uint8_t enabled);

/* Send a PDelay_Req on the port (802.1AS PDelay measurement).
 * `t1_ns` is the local egress time of the request (the MAC hardware
 * captures this); pass 0 for the host to read the TSU internally.
 * Returns FF_SUCCESS, FF_NOT_IMPLEMENTED (no MAC transmit), or
 * FF_INVALID_ARGUMENT. */
FF_Status_t FF_GptpSendPDelayReq(FF_GptpPort_t *port, uint64_t t1_ns);

#endif /* FF_GPTP_H */
