/*
 * frick-fsw - CAN-FD fabric backend (PolarFire SoC), HAL backend (M4/S4c)
 *
 * Implements the HAL CAN backend contract (FF_CanBackend_t, SRS-REQ-029) for a
 * CAN-FD controller sitting in the FPGA fabric and reached through the generic
 * fabric-device contract (FF_Fabric_t, SRS-REQ-067) - SRS-REQ-062. Classic CAN
 * on the MPFS is an MSS resource (drivers/can/mpfs/ff_can_mpfs.c); CAN-FD is not
 * a feature of that controller, so it is a fabric device here.
 *
 * MMIO access goes through the fabric window (injected backend + injected
 * fabric bus). Frame/timing logic and register accesses are host-tested with
 * mocks; the available Renode SoC model has no fabric CAN-FD peripheral
 * (NC-S4c-01/GAP-A5).
 *
 * Reference register layout of the fabric CAN-FD core (contract-level default):
 *   MODE 0x00, STATUS 0x04, CMD 0x08, BTP 0x0C, DBTP 0x10,
 *   TX_ID 0x14, TX_DLC 0x18, TX_DATA 0x1C..0x5B (64 bytes),
 *   RX_ID 0x5C, RX_DLC 0x60, RX_DATA 0x64..0xA3 (64 bytes), RX_AVAIL 0xA4.
 * No vendor IP for this core is pinned in this repository (the FIC-attached
 * catalogue is board data), so these offsets are a reference mapping confirmed
 * by the host unit test through the injected fabric bus; the on-target layout
 * must be confirmed against the board's fabric IP - GAP-A5. The catalogue entry
 * for the core must expose at least FF_CANFD_WIN_SIZE bytes.
 *
 * Known limitations of this reference backend (all bounded, allocation-free):
 *   - single instance bound statically (the HAL CAN contract carries no context);
 *   - payload transfer is byte-wise through the fabric window: no burst/DMA path
 *     (a DMA-capable variant is GAP-A5);
 *   - reception is polled (FF_FabricCanFdProcess): interrupts are not wired
 *     (fabric IRQ lines are board work, GAP-A5) and the backend buffers nothing
 *     itself - the controller FIFO is the only queue;
 *   - bit timing requires an exactly representable bit time (integer prescaler,
 *     8..20 time quanta per bit divisible by five, exact sample point at 80%), so a clock/bit-rate
 * pair that needs fractional programming is refused with FF_INVALID_ARGUMENT rather than silently
 * programmed with a wrong bit rate;
 *   - no CAN-FD transmitter-delay-compensation (TDC) programming, no error-
 *     counter/state reporting beyond the status bits used here (GAP-A5).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_CANFD_FABRIC_H
#define FF_CANFD_FABRIC_H

#include "common/ff_types.h"
#include "hal/hal_can.h"
#include "hal/hal_fabric.h"

/* Reference register offsets (window-relative). */
#define FF_CANFD_REG_MODE 0x00u     /* mode/enable, FD, bit-rate switch        */
#define FF_CANFD_REG_STATUS 0x04u   /* FIFO readiness                          */
#define FF_CANFD_REG_CMD 0x08u      /* FIFO reset, transmit request            */
#define FF_CANFD_REG_BTP 0x0Cu      /* nominal bit timing                      */
#define FF_CANFD_REG_DBTP 0x10u     /* data-phase bit timing (CAN-FD)          */
#define FF_CANFD_REG_TX_ID 0x14u    /* transmit identifier + frame flags       */
#define FF_CANFD_REG_TX_DLC 0x18u   /* transmit DLC                            */
#define FF_CANFD_REG_TX_DATA 0x1Cu  /* transmit payload window (64 bytes)      */
#define FF_CANFD_REG_RX_ID 0x5Cu    /* receive identifier + frame flags        */
#define FF_CANFD_REG_RX_DLC 0x60u   /* receive DLC                             */
#define FF_CANFD_REG_RX_DATA 0x64u  /* receive payload window (64 bytes)       */
#define FF_CANFD_REG_RX_AVAIL 0xA4u /* receive FIFO fill level                */

/* Minimum window the catalogue entry must expose for the core. */
#define FF_CANFD_WIN_SIZE 0x100u

/* MODE bits. */
#define FF_CANFD_MODE_ENABLE 0x01u
#define FF_CANFD_MODE_FD 0x02u
#define FF_CANFD_MODE_BRS 0x04u /* bit-rate switch (FD data phase) */

/* STATUS bits. */
#define FF_CANFD_STATUS_TX_READY 0x01u
#define FF_CANFD_STATUS_RX_EMPTY 0x02u

/* CMD bits. */
#define FF_CANFD_CMD_RESET_FIFOS 0x01u
#define FF_CANFD_CMD_TX_REQUEST 0x02u
#define FF_CANFD_CMD_POP_RX 0x04u /* release the head receive frame */

/* Maximum payload of a CAN-FD frame (ISO 11898-1:2015). */
#define FF_CANFD_MAX_PAYLOAD 64u

typedef struct
{
    FF_CanBackend_t can;   /* public HAL interface (must be first)  */
    FF_Fabric_t *fabric;   /* fabric backend that owns the window   */
    FF_FabricWindow_t win; /* open window on the CAN-FD core        */
    FF_CanRxCb_t rx_cb;    /* receive callback (deferred to a task) */
    void *rx_ctx;
    uint32_t clk_hz;       /* core clock driving the bit timing     */
    uint32_t bitrate;      /* configured nominal bit rate (0 = none) */
    uint32_t data_bitrate; /* configured data-phase bit rate        */
    uint16_t tx_frames;    /* diagnostics: frames handed to the core */
    uint16_t tx_busy;      /* diagnostics: refused, no TX room      */
    uint16_t rx_frames;    /* diagnostics: frames delivered         */
    uint16_t rx_errors;    /* diagnostics: malformed/refused frames  */
    uint8_t fd;            /* CAN-FD mode enabled                   */
    uint8_t ready;         /* init() completed                      */
} FF_FabricCanFd_t;

/* Bind the instance: resolve and open the fabric window on `dev_id`, then
 * complete the FF_CanBackend_t vtable. `clk_hz` is the core clock used for bit
 * timing. The fabric backend must already be initialised: the window is opened
 * here and a failure is propagated (FF_NOT_FOUND for an unknown device id,
 * FF_BAD_SIZE when the catalogue entry is smaller than FF_CANFD_WIN_SIZE).
 * Returns FF_INVALID_ARGUMENT for NULL/incomplete arguments or a zero clock. */
FF_Status_t FF_FabricCanFdSetup(FF_FabricCanFd_t *c, FF_Fabric_t *fabric, uint32_t dev_id,
                                uint32_t clk_hz);

/* CAN-FD DLC <-> payload length (ISO 11898-1:2015). 0..8 map to themselves, then
 * 12, 16, 20, 24, 32, 48, 64. FF_BAD_SIZE for a length CAN-FD cannot encode
 * (9..11, 13..15, and every non-canonical length above 8). Pure functions,
 * modelled in Frick/CanFd.lean. */
FF_Status_t FF_CanFdDlcToLen(uint8_t dlc, uint8_t *len);
FF_Status_t FF_CanFdLenToDlc(uint8_t len, uint8_t *dlc);

/* Derive a bit timing (integer prescaler, 8..20 time quanta per bit, sample
 * point at 80%, synchronisation jump width of 1 quantum). Returns
 * FF_INVALID_ARGUMENT when the clock/bit-rate pair has no exact solution, so an
 * unreachable bit rate is refused instead of programmed wrongly. Pure function,
 * modelled in Frick/CanFd.lean. */
FF_Status_t FF_FabricCanFdBitTiming(uint32_t clk_hz, uint32_t bitrate, uint32_t *prescaler,
                                    uint32_t *seg1, uint32_t *seg2);

/* Drain up to `budget` received frames from the controller FIFO and deliver each
 * to the registered callback (a callback that is not set still drains, so the
 * FIFO cannot wedge). Bounded: at most `budget` frames, no allocation. */
FF_Status_t FF_FabricCanFdProcess(uint16_t budget);

#endif /* FF_CANFD_FABRIC_H */
