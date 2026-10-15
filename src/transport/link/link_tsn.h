/*
 * frick-fsw - TSN-scheduled PUS-C link backend (M4-restored, SRS-REQ-061)
 *
 * Reference FF_Link_t backend that carries PUS-C TM/TC frames over the
 * TSN-scheduled Ethernet path using FF_EthMac_t (Ethernet MAC) and
 * FF_TsnBackend_t (TSN configuration seam). Selectable via FF_LinkSelect.
 *
 * Reference encapsulation profile (over the GEM TSN backend):
 *   Ethernet frame: [dest MAC 6][src MAC 6][EtherType 0x88F5][CCSDS frame][pad>60]
 *   EtherType 0x88F5 is reserved for "frick-fsw PUS-C" (private use; mission ICD
 *   may reassign). The MAC appends the Frame Check Sequence (FCS).
 *
 * RX is callback-driven: the MAC invokes the registered callback, which pushes
 * the frame payload into a ring buffer. FF_LinkRecv reads from that buffer.
 *
 * RX length semantics: the TX path zero-pads header+payload to the 60-byte
 * Ethernet minimum (FF_LINK_TSN_MIN_FRAME, FCS excluded), and the RX path
 * delivers the received payload including that padding. A received frame can
 * therefore be longer than the PUS-C packet it carries, so a round-trip check
 * asserts the PUS header fields (service/subservice) rather than the exact
 * length and trailing bytes of the TM packet.
 *
 * No dynamic allocation (SRS-REQ-015). All buffers are static.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_LINK_TSN_H
#define FF_LINK_TSN_H

#include "link.h"
#include "hal/hal_eth.h"
#include "hal/hal_tsn.h"

/* Reference EtherType for PUS-C over Ethernet (private use range). */
#ifndef FF_LINK_TSN_ETHERTYPE
#define FF_LINK_TSN_ETHERTYPE 0x88F5u
#endif

/* Ethernet frame header (without FCS, which the MAC appends). */
#define FF_LINK_TSN_HDR_LEN 14u /* dst(6) + src(6) + type(2) */

/* RX ring buffer dimensions. */
#define FF_LINK_TSN_RX_SLOTS 16u
#ifndef FF_LINK_TSN_RX_FRAMESZ
#ifdef FF_PUS_MAX_TM_FRAME
#define FF_LINK_TSN_RX_FRAMESZ FF_PUS_MAX_TM_FRAME
#else
#define FF_LINK_TSN_RX_FRAMESZ 1024u
#endif
#endif

/* Reference TSN configuration (VLAN/PCP). */
#ifndef FF_LINK_TSN_DEFAULT_VID
#define FF_LINK_TSN_DEFAULT_VID 0u
#endif
#ifndef FF_LINK_TSN_DEFAULT_PCP
#define FF_LINK_TSN_DEFAULT_PCP 0u
#endif

/* Configuration: set via FF_LinkTsnInit() before FF_LinkSelect. */
typedef struct
{
    FF_EthMac_t *mac;     /* Ethernet MAC driver (GEM)                    */
    FF_TsnBackend_t *tsn; /* TSN backend (VLAN/PCP, 802.1AS, etc.)      */
    uint16_t vid;         /* VLAN ID (reference: 0 = no VLAN tag)        */
    uint8_t pcp;          /* priority code point (reference: 0)          */
    uint8_t dest_mac[6];  /* destination MAC (reference: broadcast ff:ff:ff:ff:ff:ff) */
} FF_LinkTsnConfig_t;

/* Default config initialiser (reference profile). */
FF_LinkTsnConfig_t FF_LinkTsnDefaultConfig(FF_EthMac_t *mac, FF_TsnBackend_t *tsn);

/* Initialise and bind the TSN backend. Must be called before FF_LinkSelect. */
FF_Status_t FF_LinkTsnInit(const FF_LinkTsnConfig_t *cfg);

/* Reference TSN link backend (selectable via FF_LinkSelect). */
extern const FF_Link_t FF_LinkTsnBackend;

#endif /* FF_LINK_TSN_H */
