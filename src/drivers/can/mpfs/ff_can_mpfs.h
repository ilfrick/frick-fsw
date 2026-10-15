/*
 * frick-fsw - MSS CAN driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Implements the HAL CAN contract (FF_CanBackend_t, classic CAN 2.0 A/B) for the
 * Microchip PolarFire SoC MSS CAN controllers (CAN.MPFS_CAN). CAN-FD is not a
 * feature of the MSS CAN (it is a fabric device reached through the FIC), so
 * `supports_fd` is 0 and a CAN-FD configure request is rejected.
 *
 * MMIO access is injected (FF_CanBus_t, double-word addressed) so the register
 * logic is testable on the host (mock bus) and exercised on the emulated SoC in
 * the Renode functional test (milestone M11).
 *
 * Register map (Microchip PolarFire SoC MSS CAN; matches the Renode MPFS_CAN
 * model and the Microchip `mss_can.h` filter layout AMR/ACR = {N_A:1, RTR:1,
 * IDE:1, ID:29}, cf. MSS_CAN_get_msg_filter_mask()):
 *   - InterruptStatus : 0x000 (RX_MSG_LOSS bit10, TX_MSG bit11, RX_MSG bit12;
 *                              write-1-to-clear)
 *   - InterruptEnable : 0x004 (INT_ENBL bit0, TX_MSG_ENBL bit11, RX_MSG_ENBL bit12)
 *   - RxStatus        : 0x008 (read-only, bit i = RX buffer i has a message)
 *   - TxStatus        : 0x00C (read-only, bit i = TX buffer i request pending)
 *   - Command         : 0x014 (run/stop bit0)
 *   - Config          : 0x018 (bit timing: SJW[3:2], TSEG2[7:5], TSEG1[11:8],
 *                              BITRATE[30:16]; SWAP_ENDIAN bit13)
 *   - TX buffer i     : 0x020 + i*0x10 -> Cmd+0x0 (TxReq bit0, DLC[19:16],
 *                              IDE bit20), Id+0x4 (ID[31:3]), DataHi+0x8,
 *                              DataLo+0xC
 *   - RX buffer i     : 0x220 + i*0x20 -> Cmd+0x0 (MsgAv bit0 w1c, RxBufferEbl
 *                              bit3, RxIntEbl bit5, DLC[19:16], IDE bit20),
 *                              Id+0x4, DataHi+0x8, DataLo+0xC, AMR+0x10, ACR+0x14,
 *                              AMR_DATA+0x18, ACR_DATA+0x1C
 *
 * Bit timing is a reference calculation from the MSS CAN clock
 * (FF_CAN_MPFS_CLK_HZ) with a fixed 87.5% sample point (1 + 13 + 2 = 16 tq).
 * The MSS CAN clock is derived from the MSS PLL and is one of 8/16/24/32/40/48/
 * 56/64/72/80 MHz (PolarFire SoC MSS Technical Reference Manual); 80 MHz is used
 * as the reference, at which the standard rates 125/250/500 kbit/s and 1 Mbit/s
 * are exact (prescaler 40/20/10/5). The mission must confirm the configured MSS
 * CAN clock and re-derive the TSEG/prescaler split per rate against the datasheet
 * before flight (on-target timing is GAP-A5).
 *
 * Known limitations of this reference backend: TX is round-robin over 32 buffers
 * and the TX status register is not polled, so a still-pending buffer can be
 * overwritten if the caller outruns the controller; there is no bus-off /
 * error-frame / lost-arbitration handling and no RX_MSG_LOSS (bit 10) handling;
 * and receive is polling-only through FF_CanMpfsPoll (which is driver-specific,
 * not part of the HAL contract). Static, no allocation; bounded buffers. Single
 * instance (the HAL CAN contract carries no context).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_CAN_MPFS_H
#define FF_CAN_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_can.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host). */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
} FF_CanBus_t;

/* MPFS MSS CAN register map. */
#define FF_CAN_REG_INTERRUPT_STATUS 0x000u
#define FF_CAN_REG_INTERRUPT_ENABLE 0x004u
#define FF_CAN_REG_RX_STATUS 0x008u
#define FF_CAN_REG_TX_STATUS 0x00Cu
#define FF_CAN_REG_COMMAND 0x014u
#define FF_CAN_REG_CONFIG 0x018u

#define FF_CAN_TX_BASE 0x020u
#define FF_CAN_TX_STRIDE 0x10u
#define FF_CAN_TX_CMD_OFF 0x0u
#define FF_CAN_TX_ID_OFF 0x4u
#define FF_CAN_TX_DATA_HI_OFF 0x8u
#define FF_CAN_TX_DATA_LO_OFF 0xCu

#define FF_CAN_RX_BASE 0x220u
#define FF_CAN_RX_STRIDE 0x20u
#define FF_CAN_RX_CMD_OFF 0x0u
#define FF_CAN_RX_ID_OFF 0x4u
#define FF_CAN_RX_DATA_HI_OFF 0x8u
#define FF_CAN_RX_DATA_LO_OFF 0xCu
#define FF_CAN_RX_AMR_OFF 0x10u
#define FF_CAN_RX_ACR_OFF 0x14u
#define FF_CAN_RX_AMR_DATA_OFF 0x18u
#define FF_CAN_RX_ACR_DATA_OFF 0x1Cu

/* Command register: run/stop bit. */
#define FF_CAN_CMD_RUN (1u << 0)

/* Interrupt-enable / -status bits. */
#define FF_CAN_INT_ENBL (1u << 0)
#define FF_CAN_INT_TX_MSG (1u << 11)
#define FF_CAN_INT_RX_MSG (1u << 12)

/* TX buffer control bits. */
#define FF_CAN_TX_CMD_TXREQ (1u << 0)
#define FF_CAN_TX_CMD_DLC_SHIFT 16u
#define FF_CAN_TX_CMD_IDE (1u << 20)

/* RX buffer control bits. */
#define FF_CAN_RX_CMD_MSGAV (1u << 0)
#define FF_CAN_RX_CMD_RXENBL (1u << 3)
#define FF_CAN_RX_CMD_DLC_SHIFT 16u
#define FF_CAN_RX_CMD_IDE (1u << 20)

/* RX acceptance-mask register (AMR) bit layout (Microchip MSS CAN filter: N_A,
 * RTR, IDE, then the 29-bit identifier). Bits 0-2 are the reserved/RTR-mask/
 * IDE-mask bits; bits 3-31 are the identifier mask, left-aligned exactly as the
 * identifier is in the ID register (see FF_CAN_ID_SHIFT). */
#define FF_CAN_RX_AMR_RTR (1u << 1)
#define FF_CAN_RX_AMR_IDE (1u << 2)

/* Identifier field (ID[31:3]) and limits. */
#define FF_CAN_ID_SHIFT 3u
#define FF_CAN_ID_MASK 0x1FFFFFFFu
#define FF_CAN_STD_ID_MAX 0x7FFu
#define FF_CAN_EXT_ID_MAX 0x1FFFFFFFu

/* Frame limits and buffer counts. */
#define FF_CAN_MAX_DLC 8u
#define FF_CAN_TX_BUFFERS 32u
#define FF_CAN_RX_BUFFERS 32u

/* RX acceptance mask that accepts every frame: the 29 identifier-mask bits are
 * all "don't care" (left-aligned with FF_CAN_ID_SHIFT, exactly as the identifier
 * itself is written to the ID register) plus the RTR and IDE mask bits, so both
 * standard and extended data/remote frames match. The reserved N_A bit is left
 * clear. Without the << FF_CAN_ID_SHIFT the mask would cover only identifier
 * bits 0-25, silently rejecting extended identifiers >= 0x04000000 - the mask
 * and the identifier must live in the same (identifier) domain. The emulated
 * MPFS_CAN does not implement the AMR RTR/IDE mask bits (bits 1-2), so
 * configuring this filter on the emulator emits a benign "unhandled write"
 * warning; the Renode self-test does not exercise this path. */
#define FF_CAN_ACCEPT_ALL                                                                          \
    ((FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) | FF_CAN_RX_AMR_RTR | FF_CAN_RX_AMR_IDE)

/* Reference bit-timing: 87.5% sample point (1 sync + TSEG1 + TSEG2 = 16 tq). */
#define FF_CAN_SJW 1u
#define FF_CAN_TSEG1 13u
#define FF_CAN_TSEG2 2u

/* MSS CAN reference clock: the MSS CAN clock is derived from the MSS PLL and is
 * one of 8/16/24/32/40/48/56/64/72/80 MHz (PolarFire SoC MSS TRM); 80 MHz is the
 * reference value and makes the standard rates exact with a 16 tq split. The
 * mission must confirm the configured clock against the datasheet and re-derive
 * the split per rate; bit timing derived from it is a reference value (GAP-A5). */
#define FF_CAN_MPFS_CLK_HZ 80000000u

typedef struct
{
    FF_CanBackend_t can; /* public HAL interface (must be first) */
    FF_CanBus_t bus;
    FF_CanRxCb_t rx_cb;
    void *rx_ctx;
    uint32_t next_tx; /* round-robin TX buffer cursor */
} FF_CanMpfs_t;

/* Bind the instance and complete the FF_CanBackend_t vtable. */
FF_Status_t FF_CanMpfsSetup(FF_CanMpfs_t *c, const FF_CanBus_t *bus);

/* MMIO self-test (register round trips on the Control/Interrupt-enable/TX-ID/
 * RX-acceptance-mask-data registers, all fully implemented by the emulated
 * controller) used by the host unit test and the Renode functional check (the
 * MSS CAN at 0x2010C000). Interrupts are left disabled and the interrupt status
 * is not touched, so no IRQ is raised. */
FF_Status_t FF_CanMpfsSelfTest(FF_CanMpfs_t *c);

/* Driver-specific: dispatch every received frame in the enabled RX buffers to
 * the registered callback (clearing its MsgAv flag) and count them in *got.
 * The ISR / polling loop is mission specific, so this is not part of the HAL
 * contract. Returns FF_INVALID_ARGUMENT on a NULL instance. */
FF_Status_t FF_CanMpfsPoll(FF_CanMpfs_t *c, uint32_t *got);

#endif /* FF_CAN_MPFS_H */
