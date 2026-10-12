/*
 * frick-fsw - Cadence GEM register map (Microchip PolarFire SoC MSS)
 *
 * Offsets and bit positions for the two Gigabit Ethernet MACs (GEM) in the
 * PolarFire SoC MSS. Source of truth: the Cadence GEM layout as used by the
 * Linux `macb` driver (drivers/net/ethernet/cadence/macb.h) and the Linux
 * binding `microchip,mpfs-macb`; the same layout is described in the
 * Microchip PolarFire SoC MSS Technical Reference Manual (DS60001702).
 * Only the subset used by the driver is defined here.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_GEM_REGS_H
#define FF_GEM_REGS_H

/* Register offsets (32-bit registers). */
#define FF_GEM_NCR 0x0000u    /* network control                    */
#define FF_GEM_NCFGR 0x0004u  /* network configuration              */
#define FF_GEM_NSR 0x0008u    /* network status                     */
#define FF_GEM_DMACFG 0x0010u /* DMA configuration                  */
#define FF_GEM_TSR 0x0014u    /* transmit status                    */
#define FF_GEM_RBQP 0x0018u   /* receive buffer queue base address  */
#define FF_GEM_TBQP 0x001Cu   /* transmit buffer queue base address */
/* High 32 bits of the queue base address (present when DMACFG.ADDR64=1).
 * Co-implemented for NC-S6-03; the full 64-bit descriptor format (3-word
 * descriptors with per-descriptor addr_hi) is deferred to M14. */
#define FF_GEM_RBQP_HI 0x00C8u /* receive buffer queue base addr hi  */
#define FF_GEM_TBQP_HI 0x00CCu /* transmit buffer queue base addr hi */
#define FF_GEM_RSR 0x0020u     /* receive status                     */
#define FF_GEM_ISR 0x0024u
#define FF_GEM_IER 0x0028u
#define FF_GEM_IDR 0x002Cu
#define FF_GEM_IMR 0x0030u
#define FF_GEM_MAN 0x0034u  /* PHY maintenance                    */
#define FF_GEM_SA1B 0x0088u /* specific address 1 bottom          */
#define FF_GEM_SA1T 0x008Cu /* specific address 1 top             */

/* TSN: 1588/TSU time (802.1AS). */
#define FF_GEM_TSH 0x01C0u /* timer seconds high (bits 47:32) */
#define FF_GEM_TSL 0x01D0u /* timer seconds low  (bits 31:0)  */
#define FF_GEM_TN 0x01D4u  /* timer nanoseconds (30 bits)     */
#define FF_GEM_TSH_SHIFT 32u
#define FF_GEM_TN_MASK 0x3FFFFFFFu

/* TSN: 802.1Qbv Enhanced Scheduled Traffic (ENST), per queue `q`. */
#define FF_GEM_ENST_CONTROL 0x0880u
#define FF_GEM_ENST_START_TIME(q) (0x0800u + ((uint32_t)(q) << 2))
#define FF_GEM_ENST_ON_TIME(q) (0x0820u + ((uint32_t)(q) << 2))
#define FF_GEM_ENST_OFF_TIME(q) (0x0840u + ((uint32_t)(q) << 2))
#define FF_GEM_ENST_START_SEC_SHIFT 30u
#define FF_GEM_ENST_START_NSEC_MASK 0x3FFFFFFFu
#define FF_GEM_ENST_TIME_MASK 0x1FFFFu /* on/off time field (17 bits) */

/* TSN: 802.1Q screener type 2 (VLAN/PCP -> queue). */
#define FF_GEM_SCRT2 0x0540u
#define FF_GEM_SCRT2_QUEUE_SHIFT 0u
#define FF_GEM_SCRT2_VLANPR_SHIFT 4u
#define FF_GEM_SCRT2_VLANEN_SHIFT 8u

/* NCR bit positions (macb layout; GEM has no software reset). */
#define FF_GEM_NCR_LLB 1u /* loopback local       */
#define FF_GEM_NCR_RE 2u  /* receive enable       */
#define FF_GEM_NCR_TE 3u  /* transmit enable      */
#define FF_GEM_NCR_MPE 4u /* management port enable */
#define FF_GEM_NCR_CLRSTAT 5u
#define FF_GEM_NCR_BP 8u     /* back pressure        */
#define FF_GEM_NCR_TSTART 9u /* start transmission   */
#define FF_GEM_NCR_THALT 10u

/* NCFGR bit positions. */
#define FF_GEM_NCFGR_SPD 0u    /* speed (0: 10/100 use, see GBE) */
#define FF_GEM_NCFGR_FD 1u     /* full duplex          */
#define FF_GEM_NCFGR_BIG 8u    /* receive 1536-byte frames */
#define FF_GEM_NCFGR_GBE 10u   /* gigabit mode enable  */
#define FF_GEM_NCFGR_DRFCS 17u /* FCS remove          */
#define FF_GEM_NCFGR_DBW 21u   /* data bus width (0=32,1=64,2=128) */
#define FF_GEM_NCFGR_CLK 18u   /* MDC clock division (3 bits) */

/* DMACFG fields. */
#define FF_GEM_DMACFG_RXBS 16u /* DMA receive buffer size (8 bits) */
#define FF_GEM_DMACFG_RXEXT 28u
#define FF_GEM_DMACFG_TXEXT 29u
#define FF_GEM_DMACFG_ADDR64 30u

/* TSR bit positions. */
#define FF_GEM_TSR_UBR 0u
#define FF_GEM_TSR_TGO 3u
#define FF_GEM_TSR_COMP 5u /* transmit complete */

/* RSR bit positions (no frame length in RSR; length is in the descriptor). */
#define FF_GEM_RSR_BNA 0u
#define FF_GEM_RSR_REC 1u
#define FF_GEM_RSR_OVR 2u

/* DMA descriptor (2 words: addr, ctrl), macb layout. */
#define FF_GEM_TX_USED 31u /* TX ctrl: 1 = software owns, 0 = DMA */
#define FF_GEM_TX_WRAP 30u /* TX ctrl: wrap bit (last descriptor) */
#define FF_GEM_TX_LAST 15u /* TX ctrl: last buffer of frame */
#define FF_GEM_TX_FRMLEN_MASK 0x07FFu
#define FF_GEM_RX_USED 0u /* RX addr word: 1 = frame ready */
#define FF_GEM_RX_WRAP 1u /* RX addr word: wrap bit */
#define FF_GEM_RX_FRMLEN_MASK 0x0FFFu

/* Design config not used here; see macb.h for the full GEM map. */

#endif /* FF_GEM_REGS_H */
