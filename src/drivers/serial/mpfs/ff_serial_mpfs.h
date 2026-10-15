/*
 * frick-fsw - MSS MMUART serial driver (PolarFire SoC), HAL backend (M4)
 *
 * Implements the HAL serial contract (FF_Serial_t) for the Microchip PolarFire
 * SoC MSS MMUART peripherals (NS16550 in wide-register mode), used for
 * RS-422/RS-485 (with an external transceiver). MMIO access is injected
 * (FF_SerialBus_t, byte-wide) so the register logic is testable on the host and
 * exercised on the emulated SoC in the Renode functional test (milestone M11).
 * The RS-485 half-duplex direction line (DE/RE) is injected the same way
 * (FF_SerialDeRe_t), so the turnaround logic is host-testable and the board
 * integrator binds the real transceiver control pin (SRS-REQ-064).
 *
 * MPFS MMUART register map (Microchip MSS_UART, 32-bit registers, 4-byte
 * spacing; the Renode `UART.NS16550` model runs with `wideRegisters: true`):
 *   RBR/THR 0x00, IER 0x04, IIR/FCR 0x08, LCR 0x0C, MCR 0x10, LSR 0x14,
 *   MSR 0x18, SCR 0x1C.
 *
 * The HAL serial contract has no per-instance context, so the driver is a
 * bounded single-instance backend (FF_SerialMpfsSetup binds it).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_SERIAL_MPFS_H
#define FF_SERIAL_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_serial.h"

typedef struct
{
    void *ctx;
    uint8_t (*read8)(void *ctx, uint32_t off);
    void (*write8)(void *ctx, uint32_t off, uint8_t val);
} FF_SerialBus_t;

/* RS-485 half-duplex transceiver direction line (DE/RE). The pin is board
 * specific (an MSS GPIO driven through FF_Gpio_t on target, a mock on the host),
 * so it is injected exactly like the MMIO bus. `write` drives the line
 * (1 = transmit, DE asserted; 0 = receive, RE) and `delay_us` applies a bounded
 * microsecond hold. Both hooks are mandatory: the HAL serial contract carries
 * the requested de-assert timing (SRS-REQ-064), so a control line that cannot
 * honour it is not accepted. The descriptor must outlive the binding. */
typedef struct
{
    void *ctx;
    void (*write)(void *ctx, uint8_t level);
    void (*delay_us)(void *ctx, uint32_t us);
} FF_SerialDeRe_t;

/* Bounded transmit-drain poll used when the transceiver is turned around: the
 * driver polls the transmitter-empty bit a small, fixed number of times before
 * applying the `de_assert_us` hold supplied on that release call (the value on
 * an assert call is not cached for later use). The hold is the authoritative
 * turnaround; the poll only shortens it when the shift register is already
 * empty (it is never allowed to spin unboundedly in the release path). */
#define FF_SERIAL_DRAIN_SPIN_LIMIT 4096u

/* MPFS MMUART register offsets (4-byte spaced). */
#define FF_UART_REG_THR 0x00u /* transmit holding / receive buffer */
#define FF_UART_REG_IER 0x04u
#define FF_UART_REG_FCR 0x08u
#define FF_UART_REG_LCR 0x0Cu
#define FF_UART_REG_MCR 0x10u
#define FF_UART_REG_LSR 0x14u
#define FF_UART_REG_MSR 0x18u
#define FF_UART_REG_SCR 0x1Cu /* scratch (loopback for the MMIO self-test) */

#define FF_UART_LSR_DATA_READY 0x01u
#define FF_UART_LSR_THR_EMPTY 0x20u
#define FF_UART_LSR_TX_EMPTY 0x40u

/* LCR fields. */
#define FF_UART_LCR_WLEN_MASK 0x03u
#define FF_UART_LCR_STOP2 0x04u
#define FF_UART_LCR_PEN 0x08u /* parity enable */
#define FF_UART_LCR_EPS 0x10u /* parity select: 0 = odd, 1 = even */

typedef struct
{
    FF_Serial_t serial; /* public HAL interface (must be first) */
    FF_SerialBus_t bus;
    const FF_SerialDeRe_t *dere; /* bound DE/RE direction line (NULL = unbound) */
    uint8_t rs485;               /* last requested half-duplex enable        */
    uint8_t de_line;             /* last level driven on the DE/RE line      */
    uint16_t drain_polls;        /* polls used by the last turnaround        */
    uint16_t drain_timeouts;     /* turnarounds that exhausted the poll      */
    uint32_t de_assert_us;       /* last requested de-assert hold (us)       */
} FF_SerialMpfs_t;

/* Bind the instance and complete the FF_Serial_t vtable. */
FF_Status_t FF_SerialMpfsSetup(FF_SerialMpfs_t *s, const FF_SerialBus_t *bus);

/* Bind the RS-485 DE/RE direction line (SRS-REQ-064). `dere`, `dere->write` and
 * `dere->delay_us` must all be non-NULL (FF_INVALID_ARGUMENT otherwise). The
 * line starts in receive (0). Rebinding replaces the previous line. */
FF_Status_t FF_SerialMpfsBindDeRe(FF_SerialMpfs_t *s, const FF_SerialDeRe_t *dere);

/* MMIO self-test: write/read the NS16550 scratch register. Used by the Renode
 * functional test (not part of the HAL contract). */
FF_Status_t FF_SerialMpfsScratchSelfTest(FF_SerialMpfs_t *s);

#endif /* FF_SERIAL_MPFS_H */
