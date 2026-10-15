/*
 * frick-fsw - MSS I2C driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Implements the HAL I2C master contract (FF_I2c_t) for the Microchip PolarFire
 * SoC MSS I2C controllers. The MSS I2C is the Microchip CoreI2C IP; here it is
 * driven as a **polled (non-interrupt) master**: every HAL call performs a
 * complete transaction (START, address, data, STOP) and never sleeps on an
 * ISR. I2C is also the transport for external controllers the MSS does not
 * provide natively (SRS-REQ-031).
 *
 * MMIO access is injected (FF_I2cBus_t, byte addressed) so the register/transaction
 * logic is testable on the host (mock bus) and the registers are exercised on the
 * emulated SoC in the Renode functional test (milestone M11).
 *
 * Register map (Microchip PolarFire SoC MSS I2C CoreI2C; byte registers at
 * 4-byte spacing). This layout is a **reference mapping**, not verifiable in
 * this repository: it is taken from the Microchip CoreI2C documentation and is
 * cross-checked at the two proddable register offsets (ADDR/SLAVE1_ADDR) against
 * the Renode `I2C.MPFS_I2C` model in the emulated functional test; the Microchip
 * headers (`mss_i2c.h` / `mss_i2c_regs.h`) are not revision-controlled here, so
 * on-silicon confirmation of the map is tracked under GAP-A5:
 *   - Control  (CTRL)   : 0x00 (CR2 bit7, ENS1 bit6, STA bit5, STO bit4,
 *                               SI bit3, AA bit2, CR1 bit1, CR0 bit0)
 *   - Status   (STATUS) : 0x04 (read-only state code)
 *   - Data     (DATA)   : 0x08
 *   - Addr0    (ADDR)   : 0x0C (slave address 0: ADR[7:1] + GC[0])
 *   - SMBus    (SMBUS)  : 0x10
 *   - Freq     (FREQ)   : 0x14
 *   - Glitch   (GLITCH) : 0x18
 *   - Addr1    (SLAVE1) : 0x1C (slave address 1)
 *
 * The CoreI2C serial clock is selected by the 3-bit divider code CR2:CR1:CR0 in
 * the Control register (`MSS_I2C_init` in the Microchip HAL maps the 3-bit
 * `mss_i2c_clock_divider_t` code onto those bits); the serial clock is PCLK
 * divided by the selected divider (PolarFire SoC MSS TRM: "MSS I2C uses APB
 * clock to generate the serial clock"). The 7 documented dividers and the
 * selection are a **reference derivation** from the APB reference clock
 * (FF_I2C_MPFS_CLK_HZ); the mission must confirm the configured APB frequency
 * and re-derive the divider against the datasheet before flight (per-rate
 * accuracy and on-target timing are tracked, GAP-A5). The slowest divider
 * (PCLK/960) fixes the minimum achievable SCL (FF_I2C_MIN_SCL_HZ, ~104.17 kHz at
 * the 100 MHz reference): a request below that rate is not honoured (the slowest
 * divider is used and init() returns FF_SUCCESS), so the caller must request a
 * rate >= FF_I2C_MIN_SCL_HZ.
 *
 * Known limitations of this reference backend (all bounded, allocation-free):
 *   - polled master only; there is no slave mode, no SMBus support, no clock
 *     stretching / bus-recovery / arbitration-loss handling and no interrupt
 *     path (the mission wires the ISR/task if needed);
 *   - 7-bit addressing only: 10-bit addressing is not implemented and an
 *     address above FF_I2C_ADDR7_MAX is rejected; the general-call address
 *     (0x00) is not specially treated or rejected, it is passed through as an
 *     ordinary address and no general-call semantics are implemented;
 *   - STOP is fire-and-forget: i2c_stop() writes STO and does not poll for STOP
 *     completion (STOP is issued on every terminal and error path; in a
 *     PUS-driven system the inter-transaction gap is far longer than the few
 *     microseconds a STOP takes at 100-400 kHz SCL, but STOP-completion
 *     confirmation and bus recovery are tracked as residuals, GAP-A5);
 *   - NOT re-entrant: one static instance is bound and there is no lock, so I2C
 *     calls must be serialised by the caller (a single task, or a caller-held
 *     mutex); concurrent calls interleave register accesses and corrupt the
 *     in-flight transaction;
 *   - the MMIO self-test is a bring-up check with configuration side effects (it
 *     runs init() and leaves ADDR/SLAVE1_ADDR at 0): run it before normal
 *     operation and not against concurrent traffic;
 *   - the SI poll is bounded (FF_I2C_POLL_MAX reads) and returns FF_TIMEOUT if
 *     the controller does not complete a phase (the bound is a spin count, not a
 *     calibrated wall-clock timeout, so on-target worst-case latency is GAP-A5);
 *   - the FREQ / GLITCH / SMBUS registers are left at their reset values (the
 *     reference driver programs only the Control-register serial-clock divider,
 *     exactly like `MSS_I2C_init`; the emulated MPFS_I2C does not model them);
 *   - STATIC: a single instance is bound (the HAL I2C contract carries no
 *     per-instance context), so a second I2C controller needs a separate binary
 *     or a contract extension.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_I2C_MPFS_H
#define FF_I2C_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_spi.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host).
 * Byte-wide: the CoreI2C registers are byte registers and the emulated
 * I2C.MPFS_I2C is an IBytePeripheral (double-word access is unsupported). */
typedef struct
{
    void *ctx;
    uint8_t (*read8)(void *ctx, uint32_t off);
    void (*write8)(void *ctx, uint32_t off, uint8_t val);
} FF_I2cBus_t;

/* MPFS MSS I2C register map (byte registers at 4-byte spacing). */
#define FF_I2C_REG_CTRL 0x00u
#define FF_I2C_REG_STATUS 0x04u
#define FF_I2C_REG_DATA 0x08u
#define FF_I2C_REG_ADDR 0x0Cu
#define FF_I2C_REG_SMBUS 0x10u
#define FF_I2C_REG_FREQ 0x14u
#define FF_I2C_REG_GLITCH 0x18u
#define FF_I2C_REG_SLAVE1_ADDR 0x1Cu

/* Control register bits (Microchip mss_i2c_regs.h). */
#define FF_I2C_CTRL_CR0 0x01u
#define FF_I2C_CTRL_CR1 0x02u
#define FF_I2C_CTRL_AA 0x04u
#define FF_I2C_CTRL_SI 0x08u
#define FF_I2C_CTRL_STO 0x10u
#define FF_I2C_CTRL_STA 0x20u
#define FF_I2C_CTRL_ENS1 0x40u
#define FF_I2C_CTRL_CR2 0x80u

/* Status register state codes (Microchip CoreI2C / mss_i2c.c). */
#define FF_I2C_ST_START 0x08u        /* START transmitted */
#define FF_I2C_ST_RESTART 0x10u      /* repeated START transmitted */
#define FF_I2C_ST_MT_SLA_ACK 0x18u   /* SLA+W transmitted, ACK received */
#define FF_I2C_ST_MT_SLA_NACK 0x20u  /* SLA+W transmitted, NACK received */
#define FF_I2C_ST_MT_DATA_ACK 0x28u  /* data transmitted, ACK received */
#define FF_I2C_ST_MT_DATA_NACK 0x30u /* data transmitted, NACK received */
#define FF_I2C_ST_ARB_LOST 0x38u     /* arbitration lost */
#define FF_I2C_ST_MR_SLA_ACK 0x40u   /* SLA+R transmitted, ACK received */
#define FF_I2C_ST_MR_SLA_NACK 0x48u  /* SLA+R transmitted, NACK received */
#define FF_I2C_ST_MR_DATA_ACK 0x50u  /* data received, ACK returned */
#define FF_I2C_ST_MR_DATA_NACK 0x58u /* data received, NACK returned */
#define FF_I2C_ST_IDLE 0xF8u         /* bus idle (no relevant state) */

/* 7-bit addressing. */
#define FF_I2C_ADDR7_MAX 0x7Fu

/* Bounded SI poll (spin count, not a wall-clock timeout). Each iteration is one
 * Control-register read; the controller completes a byte in well under the
 * nominal SCL period, so this is a generous bound. */
#define FF_I2C_POLL_MAX 100000u

/* MSS I2C APB reference clock used to derive the serial-clock divider. The MSS
 * I2C is clocked from the peripheral (APB) clock, which is derived on the SoC.
 * 100 MHz is the reference value used here; it is a **reference assumption**, not
 * confirmed by this repository. The MSS serial clock in
 * board/mpfs-rtpfs160zt.dts is 150 MHz, which is the MMUART clock and a
 * different MSS peripheral; the I2C APB clock must be confirmed against the
 * datasheet and the divider re-derived per rate (per-rate accuracy and on-target
 * timing = GAP-A5). */
#define FF_I2C_MPFS_CLK_HZ 100000000u

/* Minimum achievable serial clock: the slowest CoreI2C divider (PCLK/960) at the
 * reference APB clock. A requested SCL below this cannot be honoured (see the
 * header limitations); callers must request FF_I2C_MIN_SCL_HZ or faster. */
#define FF_I2C_MIN_SCL_HZ (FF_I2C_MPFS_CLK_HZ / 960u)

/* Serial clock divider table entry: the Control-register divider code
 * (CR2:CR1:CR0) and the APB PCLK divisor it selects (Microchip
 * `i2c_clock_divider_t` / `mss_i2c_clock_divider_t`). */
typedef struct
{
    uint8_t code; /* CTRL CR2:CR1:CR0 divider code */
    uint16_t div; /* PCLK divisor */
} FF_I2cDiv_t;

#define FF_I2C_DIV_COUNT 7u

/* The CoreI2C dividers, ascending by divisor (the first entry whose resulting
 * serial clock does not exceed the requested rate is selected; if none fits,
 * the slowest divider is used). Exposed for testability. */
extern const FF_I2cDiv_t ff_i2c_div_table[FF_I2C_DIV_COUNT];

/* Serial-clock target used by the MMIO self-test. */
#define FF_I2C_SELFTEST_SCL_HZ 100000u

typedef struct
{
    FF_I2c_t i2c; /* public HAL interface (must be first) */
    FF_I2cBus_t bus;
    uint8_t ctrl;  /* CTRL value with the clock divider + ENS1 (SI/STA/STO clear) */
    uint8_t ready; /* init() has completed */
} FF_I2cMpfs_t;

/* Bind the instance and complete the FF_I2c_t vtable. Returns FF_INVALID_ARGUMENT
 * if c, bus or either bus hook (read8/write8) is NULL. */
FF_Status_t FF_I2cMpfsSetup(FF_I2cMpfs_t *c, const FF_I2cBus_t *bus);

/* MMIO self-test: enable the core through the HAL init path (which programs the
 * serial-clock divider and ENS1), verify the bus is idle, and round-trip the two
 * slave-address registers (ADDR/SLAVE1_ADDR), which are plain R/W in the
 * hardware and fully modelled by the emulated I2C.MPFS_I2C. Used by the host unit
 * test and the Renode functional check (the MSS I2C0 at 0x2010A000).
 *
 * Scope note: the emulated MPFS_I2C does not model the Control-register clock
 * divider bits (CR0/CR1/CR2), so programming them emits a benign "unhandled
 * write" warning and they cannot be read back; the self-test therefore asserts
 * ENS1 only. The self-test does not run a bus transaction (no slave is attached
 * to the emulated I2C and the model advances its state machine on register
 * writes rather than on the SI clear used by real hardware): the full
 * transaction path is host-tested.
 *
 * This is a bring-up check with configuration side effects: it runs init() and
 * leaves ADDR/SLAVE1_ADDR at 0. Run it before normal operation, not against
 * concurrent traffic. */
FF_Status_t FF_I2cMpfsSelfTest(FF_I2cMpfs_t *c);

#endif /* FF_I2C_MPFS_H */
