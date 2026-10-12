/*
 * frick-fsw - MSS SPI driver (PolarFire SoC), HAL backend (milestone M4)
 *
 * Implements the HAL SPI master contract (FF_Spi_t) for the Microchip PolarFire
 * SoC MSS SPI controllers (Microchip CoreSPI). The MSS SPI is driven here as a
 * **polled (non-interrupt) master**: transfer() writes each frame to TXDATA and
 * polls STATUS for RXDATARCVD, and never sleeps on an ISR. SPI is also a
 * transport for external controllers the MSS does not provide natively
 * (SRS-REQ-031).
 *
 * MMIO access is injected (FF_SpiBus_t, double-word addressed) so the register
 * logic is testable on the host (mock bus) and the registers are exercised on
 * the emulated SoC in the Renode functional test (milestone M11).
 *
 * Register map (Microchip PolarFire SoC MSS SPI / CoreSPI; 32-bit "DoubleWord"
 * registers). This layout is a **reference mapping**, not verifiable in this
 * repository: it is taken from the Microchip CoreSPI documentation and the
 * Renode `SPI.MPFS_SPI` model, and is cross-checked at the proddable register
 * offsets (CONTROL/FRAMESIZE/CLKRATE/STATUS) against that model in the emulated
 * functional test; the Microchip headers (`mss_spi.h` / `mss_spi_regs.h`) are
 * not revision-controlled here, so on-silicon confirmation of the map is tracked
 * under GAP-A5:
 *   - CONTROL     : 0x00 (b0 ENABLE, b1 MASTER, b2-3 MODE, b4 INTRXDATA,
 *                        b5 INTTXDATA, b6 INTRCVOVFLOW, b7 INTTXTURUN,
 *                        b8-23 FRAMECNT, b24 SPO, b25 SPH, b26 SPS,
 *                        b27 FRAMEURUN, b28 CLKMODE, b29 BIGFIFO, b30 OENOFF,
 *                        b31 RESET)
 *   - FRAMESIZE   : 0x04 (b0-5 FRAMESIZE)
 *   - STATUS      : 0x08 (b0 TXDATASENT, b1 RXDATARCVD, b2 RXOVERFLOW,
 *                        b3 TXUNDERRUN, b4 RXFIFOFULL, b5 RXFIFOFULLNEXT,
 *                        b6 RXFIFOEMPTY, b7 RXFIFOEMPTYNEXT, b8 TXFIFOFULL,
 *                        b9 TXFIFOFULLNEXT, b10 TXFIFOEMPTY, b11 (name unconfirmed, GAP-A5),
 *                        b12 FRAMESTART, b13 SSEL, b14 ACTIVE)
 *   - INT_CLEAR   : 0x0C (write-1-to-clear: b0 TXDONE, b1 RXDONE, b2 RXOVERFLOW,
 *                        b3 TXUNDERRUN, b4 CMDINT, b5 SSEND)
 *   - RXDATA      : 0x10 (read)
 *   - TXDATA      : 0x14 (write)
 *   - CLKRATE     : 0x18 (b0-7)
 *   - SSEL        : 0x1C (b0-7)
 *   - INT_MASKED  : 0x20
 *   - INT_RAW     : 0x24
 *   - CTRL_ENH    : 0x28
 *   - CMD         : 0x2C
 *   - PACKETSIZE  : 0x30
 *   - CMDSIZE     : 0x34
 *   - SLAVE_HWSTATUS : 0x38
 *   - STATUS8     : 0x3C
 *   - aliased control bytes : 0x40-0x4C
 *
 * The CoreSPI serial clock is PCLK / (2 * (CLKRATE + 1)), selected by the 8-bit
 * CLKRATE register (Microchip `MSS_SPI_configure_master_mode` sets CLKRATE from
 * the requested clock and the peripheral clock). The divider is a **reference
 * derivation** from the APB reference clock (FF_SPI_MPFS_CLK_HZ); the mission
 * must confirm the configured APB frequency and re-derive the divider against
 * the datasheet before flight (per-rate accuracy and on-target SCL are tracked,
 * GAP-A5). The slowest divider (CLKRATE = 255) fixes the minimum achievable SCL
 * (FF_SPI_MIN_SCL_HZ, ~195.31 kHz at the 100 MHz reference): a request below
 * that rate selects the slowest divider (CLKRATE = 255), and init() still
 * returns FF_SUCCESS, so the caller must request a rate >= FF_SPI_MIN_SCL_HZ.
 *
 * Known limitations of this reference backend (all bounded, allocation-free):
 *   - polled master only: there is no slave mode, no DMA path, no interrupt
 *     path and no enhanced/command (CTRL_ENH / CMD / PACKETSIZE) framing (the
 *     mission wires the ISR/task if needed);
 *   - 8-bit frames only: FRAMESIZE is fixed at 8 and the FRAMECNT /
 *     PACKETSIZE / CMDSIZE multi-frame paths are not implemented;
 *   - chip select is a plain SSEL register write (cs_assert sets the selected
 *     bit, cs_release clears SSEL); the driver neither inserts a delay between
 *     CS assertion and the first frame nor deasserts CS at the end of
 *     transfer() - the caller sequences cs_assert() / transfer() /
 *     cs_release() explicitly, matching the FF_Spi_t contract;
 *   - the received byte is always read from RXDATA (and discarded when rx is
 *     NULL) so the level-triggered RXDATARCVD flag is cleared and no stale
 *     frame can shift the next transfer; a tx-only transfer therefore still
 *     reads (and discards) one frame per transmitted frame;
 *   - init()'s cs argument is stored as the default chip-select id but SSEL is
 *     left deasserted; drive the bus with cs_assert(cs) / cs_release(cs) around
 *     transfer() (matching the FF_Spi_t contract);
 *   - STATUS bit 11 is named TXFIFOFULLNEXT in the Renode model's register
 *     field list; its exact name in the datasheet subset is unconfirmed
 *     (GAP-A5) and the driver does not depend on it;
 *   - NOT re-entrant: one static instance is bound and there is no lock, so SPI
 *     calls must be serialised by the caller (a single task, or a caller-held
 *     mutex); concurrent calls interleave register accesses and corrupt the
 *     in-flight transfer;
 *   - the injected bus descriptor is kept **by pointer** (FF_SpiMpfs_t.bus): the
 *     FF_SpiBus_t must outlive the binding (give it static storage), unlike the
 *     I2C backend which copies it by value;
 *   - the MMIO self-test is a bring-up check with configuration side effects (it
 *     runs init() and leaves CONTROL/FRAMESIZE/CLKRATE/SSEL programmed): run it
 *     before normal operation and not against concurrent traffic;
 *   - the RXDATARCVD poll is bounded (FF_SPI_POLL_MAX reads) and returns
 *     FF_TIMEOUT if the controller does not complete a frame (the bound is a
 *     spin count, not a calibrated wall-clock timeout, so on-target worst-case
 *     latency is GAP-A5);
 *   - STATIC: a single instance is bound (the HAL SPI contract carries no
 *     per-instance context), so a second SPI controller needs a separate binary
 *     or a contract extension.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_SPI_MPFS_H
#define FF_SPI_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_spi.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host).
 * Double-word (32-bit): the CoreSPI registers are 32-bit and the emulated
 * SPI.MPFS_SPI is a double-word peripheral. */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
} FF_SpiBus_t;

/* MPFS MSS SPI register map (32-bit registers). */
#define FF_SPI_REG_CONTROL 0x00u
#define FF_SPI_REG_FRAMESIZE 0x04u
#define FF_SPI_REG_STATUS 0x08u
#define FF_SPI_REG_INT_CLEAR 0x0Cu
#define FF_SPI_REG_RXDATA 0x10u
#define FF_SPI_REG_TXDATA 0x14u
#define FF_SPI_REG_CLKRATE 0x18u
#define FF_SPI_REG_SSEL 0x1Cu
#define FF_SPI_REG_INT_MASKED 0x20u
#define FF_SPI_REG_INT_RAW 0x24u
#define FF_SPI_REG_CTRL_ENH 0x28u
#define FF_SPI_REG_CMD 0x2Cu
#define FF_SPI_REG_PACKETSIZE 0x30u
#define FF_SPI_REG_CMDSIZE 0x34u
#define FF_SPI_REG_SLAVE_HWSTATUS 0x38u
#define FF_SPI_REG_STATUS8 0x3Cu

/* CONTROL register bits (Microchip mss_spi_regs.h). */
#define FF_SPI_CONTROL_ENABLE (1u << 0)
#define FF_SPI_CONTROL_MASTER (1u << 1)
#define FF_SPI_CONTROL_MODE_SHIFT 2u
#define FF_SPI_CONTROL_MODE_MASK (0x3u << FF_SPI_CONTROL_MODE_SHIFT)
#define FF_SPI_CONTROL_INTRXDATA (1u << 4)
#define FF_SPI_CONTROL_INTTXDATA (1u << 5)
#define FF_SPI_CONTROL_INTRCVOVFLOW (1u << 6)
#define FF_SPI_CONTROL_INTTXTURUN (1u << 7)
#define FF_SPI_CONTROL_SPO (1u << 24)
#define FF_SPI_CONTROL_SPH (1u << 25)
#define FF_SPI_CONTROL_RESET (1u << 31)

/* FRAMESIZE register: b0-5 FRAMESIZE (bits per frame, minus one). */
#define FF_SPI_FRAMESIZE_MASK 0x3Fu

/* STATUS register bits (Microchip mss_spi_regs.h). */
#define FF_SPI_ST_TXDATASENT (1u << 0)
#define FF_SPI_ST_RXDATARCVD (1u << 1)
#define FF_SPI_ST_RXOVERFLOW (1u << 2)
#define FF_SPI_ST_TXUNDERRUN (1u << 3)
#define FF_SPI_ST_RXFIFOFULL (1u << 4)
#define FF_SPI_ST_RXFIFOFULLNEXT (1u << 5)
#define FF_SPI_ST_RXFIFOEMPTY (1u << 6)
#define FF_SPI_ST_RXFIFOEMPTYNEXT (1u << 7)
#define FF_SPI_ST_TXFIFOFULL (1u << 8)
#define FF_SPI_ST_TXFIFOFULLNEXT (1u << 9)
#define FF_SPI_ST_TXFIFOEMPTY (1u << 10)
#define FF_SPI_ST_TXFIFOEMPTYNEXT (1u << 11)
#define FF_SPI_ST_FRAMESTART (1u << 12)
#define FF_SPI_ST_SSEL (1u << 13)
#define FF_SPI_ST_ACTIVE (1u << 14)

/* INT_CLEAR register bits (write-1-to-clear). */
#define FF_SPI_INT_TXDONE (1u << 0)
#define FF_SPI_INT_RXDONE (1u << 1)
#define FF_SPI_INT_RXOVERFLOW (1u << 2)
#define FF_SPI_INT_TXUNDERRUN (1u << 3)
#define FF_SPI_INT_CMDINT (1u << 4)
/* Command-register (0x2C) RXFIFORST bit: write-1 self-clearing, flushes the
 * receive FIFO. Used to guarantee a timed-out frame cannot shift the next
 * transfer. */
#define FF_SPI_CMD_RXFIFORST (1u << 2)
#define FF_SPI_INT_SSEND (1u << 5)

/* CLKRATE / SSEL registers: 8-bit fields. */
#define FF_SPI_CLKRATE_MASK 0xFFu
#define FF_SPI_SSEL_MASK 0xFFu

/* Frame width and chip-select limits. */
#define FF_SPI_FRAMESIZE_BITS 8u
#define FF_SPI_MODE_MAX 3u
#define FF_SPI_CS_MAX 7u

/* Bounded RXDATARCVD poll (spin count, not a wall-clock timeout). Each
 * iteration is one Status-register read; the controller completes an 8-bit
 * frame in well under the nominal SCL period, so this is a generous bound. */
#define FF_SPI_POLL_MAX 100000u

/* MSS SPI APB reference clock used to derive the serial-clock divider. The MSS
 * SPI is clocked from the peripheral (APB) clock, which is derived on the SoC.
 * 100 MHz is the reference value used here; it is a **reference assumption**,
 * not confirmed by this repository. The MSS serial clock in
 * board/mpfs-rtpfs160zt.dts is 150 MHz, which is the MMUART clock and a
 * different MSS peripheral; the SPI APB clock must be confirmed against the
 * datasheet and the divider re-derived per rate (per-rate accuracy and
 * on-target SCL = GAP-A5). */
#define FF_SPI_MPFS_CLK_HZ 100000000u

/* Minimum achievable serial clock: the slowest CoreSPI divider
 * (PCLK / (2 * 256)) at the reference APB clock. A requested SCL below this
 * cannot be honoured (see the header limitations); callers must request
 * FF_SPI_MIN_SCL_HZ or faster. */
#define FF_SPI_MIN_SCL_HZ (FF_SPI_MPFS_CLK_HZ / (2u * 256u))

/* Serial clock target used by the MMIO self-test. */
#define FF_SPI_SELFTEST_SCL_HZ 1000000u

typedef struct
{
    const FF_SpiBus_t *bus; /* injected MMIO bus; must outlive the binding */
    uint32_t clock_hz;      /* last requested SCL (Hz) */
    uint8_t mode;           /* last requested SPI mode (0-3) */
    uint8_t cs;             /* last requested chip select (0-7) */
    uint8_t ready;          /* init() has completed */
    FF_Spi_t spi;           /* public HAL interface */
} FF_SpiMpfs_t;

/* Bind the instance and complete the FF_Spi_t vtable. Returns FF_INVALID_ARGUMENT
 * if s, bus or either bus hook (read32/write32) is NULL. The bus descriptor is
 * kept by pointer: it must outlive the binding (give it static storage). */
FF_Status_t FF_SpiMpfsSetup(FF_SpiMpfs_t *s, const FF_SpiBus_t *bus);

/* MMIO self-test: enable the core through the HAL init path (CONTROL ENABLE +
 * MASTER, FRAMESIZE, CLKRATE), verify the derived CLKRATE, FRAMESIZE and the
 * empty RX FIFO (STATUS RXFIFOEMPTY), then restore by re-initialising. Used by
 * the host unit test and the Renode functional check (the MSS SPI0 at
 * 0x20108000).
 *
 * Scope note: the emulated SPI.MPFS_SPI has no registered slave, so a real
 * master transfer() would dereference a null slave and fault; the self-test
 * therefore performs register round trips only and does not run a bus
 * transaction. The full transfer path is host-tested.
 *
 * This is a bring-up check with configuration side effects: it runs init() and
 * leaves CONTROL/FRAMESIZE/CLKRATE/SSEL programmed. Run it before normal
 * operation, not against concurrent traffic. */
FF_Status_t FF_SpiMpfsSelfTest(FF_SpiMpfs_t *s);

#endif /* FF_SPI_MPFS_H */
