/*
 * frick-fsw - MSS MMC/SD block-device driver (PolarFire SoC), HAL backend (M4)
 *
 * Implements the HAL generic block-device contract (FF_BlockDev_t: read/write/
 * flush, addressed in blocks) for the Microchip PolarFire SoC MSS MMC/SD
 * controller (an SDHCI-style controller; eMMC/SD) (SRS-REQ-070; SRS-REQ-032
 * contract). The device is driven as a **polled (non-interrupt) host**: every
 * HAL call programs the transfer, issues a single/multi-block command and spins
 * on the interrupt-status register; it never sleeps on an ISR.
 *
 * MMIO access is injected (FF_BlockBus_t, double-word addressed) so the register/
 * transfer logic is testable on the host (mock bus) and the registers are
 * exercised on the emulated SoC in the Renode functional test (milestone M11).
 *
 * Register map (Microchip PolarFire SoC MSS MMC/SD, SDHCI-style 32-bit
 * registers). This layout is a **reference mapping**, not verifiable in this
 * repository: it is taken from the SDHCI specification and the Renode
 * `SD.MPFS_SDController` model, and is cross-checked at the proddable register
 * offsets (BlockSizeBlockCount / Argument1 / Capabilities1 / HostControl2)
 * against that model in the emulated functional test; the Microchip headers
 * (`mss_sd.h` / `mss_sd_regs.h`) are not revision-controlled here, so on-silicon
 * confirmation of the map is tracked under GAP-A5:
 *   - GeneralInformation_HRS00   : 0x000
 *   - Debounce_HRS01             : 0x004
 *   - BusSetting_HRS02           : 0x008
 *   - AXIE_HRS03                 : 0x00C
 *   - PhySettings_HRS04          : 0x010
 *   - EMMCControl_HRS06          : 0x018
 *   - IODelay_HRS07              : 0x01C
 *   - HostCapability_HRS30       : 0x078
 *   - HostControllerVersion_HRS31: 0x07C
 *   - FSMMonitor_HRS32           : 0x080
 *   - BootStatus_HRS36           : 0x090
 *   - SlotInterruptStatus_CRS63  : 0x0FC
 *   - SDMASystemAddressArgument2_SRS00 : 0x200
 *   - BlockSizeBlockCount_SRS01  : 0x204 (b0-11 Transfer Block Size TBS,
 *                                        b16-31 Block Count BCCT)
 *   - Argument1_SRS02            : 0x208
 *   - CommandTransferMode_SRS03  : 0x20C (four byte fields packed in the word:
 *                                        0x20C, 0x20D, 0x20E, 0x20F)
 *   - Response1..4_SRS04..07     : 0x210..0x21C
 *   - DataBuffer_SRS08           : 0x220 (32-bit data port)
 *   - PresentState_SRS09         : 0x224
 *   - HostControl1_SRS10         : 0x228
 *   - HostControl2_SRS11         : 0x22C (b1 Internal Clock Stable)
 *   - ErrorNormalInterruptStatus_SRS12 : 0x230 (write-1-to-clear)
 *   - ErrorNormalStatusEnable_SRS13    : 0x234
 *   - ErrorNormalSignalEnable_SRS14    : 0x238
 *   - SRS15                      : 0x23C
 *   - Capabilities1_SRS16        : 0x240
 *   - Capabilities2..4_SRS17..19 : 0x244..0x24C
 *   - ForceEvent_SRS20           : 0x250
 *   - ADMAErrorStatus_SRS21      : 0x254
 *   - DmaSystemAddressLow_SRS22  : 0x258
 *   - DmaSystemAddressHigh_SRS23 : 0x25C
 *   - PresetValue_SRS24..27      : 0x260..0x26C
 *   - SRS29                      : 0x274
 *   - Command Queuing (CQ)       : 0x400+
 *
 * CommandTransferMode byte fields (packed little-endian into the 32-bit word at
 * 0x20C): byte 0x20C = b0 DMA Enable, b1 Block Count Enable, b2-3 Auto CMD,
 * b4 Data Transfer Direction Select (0=write, 1=read), b5 Multi/Single Block
 * Select, b6 RECT, b7 RECE; byte 0x20D = b0 RID; byte 0x20E = b0-1 Response Type
 * Select (0=none, 1=136, 2=48, 3=48+busy), b3 CRCCE, b4 CICE, b5 Data Present
 * Select, b6-7 Command Type; byte 0x20F = b0-5 Command Index.
 *
 * Interrupt status bits (ErrorNormalInterruptStatus, 0x230, write-1-to-clear):
 * b0 CommandComplete, b1 TransferComplete, b4 BufferWriteReady, b5 BufferReadReady
 * (error bits b15-25). This backend uses only the two completion bits and the
 * error mask; the error bits are treated as FF_ERROR.
 *
 * Known limitations of this reference backend (all bounded, allocation-free):
 *   - polled single/multi-block **buffer mode** only: no DMA/ADMA/ADMA2 (the DMA
 *     address registers are left at reset), no command queuing (CQ), no SDIO and
 *     no PHY tuning; bus width and clock are whatever the boot path programmed
 *     (the driver does not program HostControl1/HostControl2 or the preset
 *     values, and only reads HostControl2 to confirm the internal clock is
 *     stable in the self-test);
 *   - the transfer is a simplified model: the driver waits for CommandComplete +
 *     TransferComplete and then moves the whole block payload through the 32-bit
 *     DataBuffer port (little-endian, 4 bytes at a time). Real SDHCI is FIFO
 *     based and requires BufferReadReady / BufferWriteReady handshaking while
 *     the transfer is in flight; that handshaking, the interrupt path and the
 *     DMA path are out of scope here and are tracked under GAP-A5 (the on-target
 *     data path and timing are not exercised by the emulated board, which has no
 *     SD card attached);
 *   - a block count is limited to the 16-bit BCCT field (FF_MMC_BCCT_MAX = 65535
 *     blocks per call) and an LBA must fit the 32-bit Argument1 register; larger
 *     requests are rejected with FF_INVALID_ARGUMENT;
 *   - no card-initialisation (CMD0/CMD8/ACMD41/CMD2/CMD3/CMD7), no
 *     high-capacity vs byte addressing selection and no CSD/SCR parsing: the
 *     driver assumes the card is already initialised and uses block addressing
 *     (SDHC/SDXC/eMMC); bring-up of the card is the mission/boot responsibility;
 *   - no error recovery beyond reporting FF_ERROR/FF_TIMEOUT: no CMD12
 *     stop-transmission, no bus reset, no re-initialisation. flush() is a no-op
 *     (FF_SUCCESS) because there is no write-back cache;
 *   - NOT re-entrant: one static instance is bound and there is no lock, so
 *     block calls must be serialised by the caller (a single task, or a
 *     caller-held mutex); concurrent calls interleave register accesses and
 *     corrupt the in-flight transfer;
 *   - the injected bus descriptor is kept **by pointer** (FF_BlockMpfs_t.bus):
 *     the FF_BlockBus_t must outlive the binding (give it static storage);
 *   - the MMIO self-test is a bring-up check with configuration side effects (it
 *     round-trips BlockSizeBlockCount and Argument1 and leaves them cleared):
 *     run it before normal operation and not against concurrent traffic;
 *   - the completion poll is bounded (FF_MMC_POLL_MAX reads) and returns
 *     FF_TIMEOUT if the controller does not complete the transfer (the bound is
 *     a spin count, not a calibrated wall-clock timeout, so on-target worst-case
 *     latency is GAP-A5);
 *   - the reference clock / bus-width / card programming are **reference
 *     assumptions** (GAP-A5): the mission must confirm the configured SD clock,
 *     bus width and card type before flight;
 *   - STATIC: a single instance is bound (the HAL block-device contract carries
 *     no per-instance context), so a second controller needs a separate binary
 *     or a contract extension.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_BLOCK_MPFS_H
#define FF_BLOCK_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_mem.h"

/* MMIO access injected by the platform (real MMIO on target, mock on host).
 * Double-word (32-bit): the MSS MMC/SD registers are 32-bit and the emulated
 * SD.MPFS_SDController is a double-word peripheral. */
typedef struct
{
    void *ctx;
    uint32_t (*read32)(void *ctx, uint32_t off);
    void (*write32)(void *ctx, uint32_t off, uint32_t val);
} FF_BlockBus_t;

/* MPFS MSS MMC/SD register map (32-bit registers, SDHCI-style). */
#define FF_MMC_REG_GENERAL_INFO 0x000u
#define FF_MMC_REG_DEBOUNCE 0x004u
#define FF_MMC_REG_BUS_SETTING 0x008u
#define FF_MMC_REG_AXIE 0x00Cu
#define FF_MMC_REG_PHY_SETTINGS 0x010u
#define FF_MMC_REG_EMMC_CONTROL 0x018u
#define FF_MMC_REG_IO_DELAY 0x01Cu
#define FF_MMC_REG_HOST_CAPABILITY 0x078u
#define FF_MMC_REG_HOST_VERSION 0x07Cu
#define FF_MMC_REG_FSM_MONITOR 0x080u
#define FF_MMC_REG_BOOT_STATUS 0x090u
#define FF_MMC_REG_SLOT_INT_STATUS 0x0FCu
#define FF_MMC_REG_SDMA_SYS_ADDR2 0x200u
#define FF_MMC_REG_BLOCK_SIZE_COUNT 0x204u
#define FF_MMC_REG_ARGUMENT1 0x208u
#define FF_MMC_REG_CMD_XFER_MODE 0x20Cu
#define FF_MMC_REG_RESPONSE1 0x210u
#define FF_MMC_REG_RESPONSE2 0x214u
#define FF_MMC_REG_RESPONSE3 0x218u
#define FF_MMC_REG_RESPONSE4 0x21Cu
#define FF_MMC_REG_DATA_BUFFER 0x220u
#define FF_MMC_REG_PRESENT_STATE 0x224u
#define FF_MMC_REG_HOST_CONTROL1 0x228u
#define FF_MMC_REG_HOST_CONTROL2 0x22Cu
#define FF_MMC_REG_ERR_NORMAL_INT_STATUS 0x230u
#define FF_MMC_REG_ERR_NORMAL_INT_ENABLE 0x234u
#define FF_MMC_REG_ERR_NORMAL_INT_SIGNAL 0x238u
#define FF_MMC_REG_SRS15 0x23Cu
#define FF_MMC_REG_CAPABILITIES1 0x240u
#define FF_MMC_REG_CAPABILITIES2 0x244u
#define FF_MMC_REG_CAPABILITIES3 0x248u
#define FF_MMC_REG_CAPABILITIES4 0x24Cu
#define FF_MMC_REG_FORCE_EVENT 0x250u
#define FF_MMC_REG_ADMA_ERROR_STATUS 0x254u
#define FF_MMC_REG_DMA_SYS_ADDR_LOW 0x258u
#define FF_MMC_REG_DMA_SYS_ADDR_HIGH 0x25Cu
#define FF_MMC_REG_PRESET_VALUE 0x260u

/* BlockSizeBlockCount register (0x204): b0-11 TBS, b16-31 BCCT. */
#define FF_MMC_TBS_MASK 0x0FFFu
#define FF_MMC_BCCT_SHIFT 16u
#define FF_MMC_BCCT_MASK 0xFFFFu

/* CommandTransferMode (0x20C) packed 32-bit word fields. */
#define FF_MMC_XFER_DMA_ENABLE (1u << 0)
#define FF_MMC_XFER_BLOCK_COUNT_ENABLE (1u << 1)
#define FF_MMC_XFER_AUTO_CMD_SHIFT 2u
#define FF_MMC_XFER_AUTO_CMD_MASK (0x3u << FF_MMC_XFER_AUTO_CMD_SHIFT)
#define FF_MMC_XFER_DATA_DIR_READ (1u << 4) /* 0 = write, 1 = read */
#define FF_MMC_XFER_MULTI_BLOCK (1u << 5)
#define FF_MMC_XFER_RECT (1u << 6)
#define FF_MMC_XFER_RECE (1u << 7)
#define FF_MMC_CMD_RID (1u << 8)       /* byte 0x20D b0 */
#define FF_MMC_CMD_RESP_TYPE_SHIFT 16u /* byte 0x20E b0-1 */
#define FF_MMC_CMD_RESP_TYPE_MASK (0x3u << FF_MMC_CMD_RESP_TYPE_SHIFT)
#define FF_MMC_CMD_RESP_NONE 0u
#define FF_MMC_CMD_RESP_136 1u
#define FF_MMC_CMD_RESP_48 2u
#define FF_MMC_CMD_RESP_48B 3u
#define FF_MMC_CMD_CRCCE (1u << 19)
#define FF_MMC_CMD_CICE (1u << 20)
#define FF_MMC_CMD_DATA_PRESENT (1u << 21)
#define FF_MMC_CMD_TYPE_SHIFT 22u
#define FF_MMC_CMD_INDEX_SHIFT 24u /* byte 0x20F b0-5 */
#define FF_MMC_CMD_INDEX_MASK 0x3Fu

/* HostControl2 (0x22C) bits. */
#define FF_MMC_HC2_CLOCK_STABLE (1u << 1) /* Internal Clock Stable */

/* ErrorNormalInterruptStatus (0x230) bits. */
#define FF_MMC_INT_COMMAND_COMPLETE (1u << 0)
#define FF_MMC_INT_TRANSFER_COMPLETE (1u << 1)
#define FF_MMC_INT_BUFFER_WRITE_READY (1u << 4)
/* Write-1-to-clear of the whole status word (timeout recovery). */
#define FF_MMC_INT_CLEAR_ALL 0xFFFFFFFFu
#define FF_MMC_INT_BUFFER_READ_READY (1u << 5)
#define FF_MMC_INT_DONE (FF_MMC_INT_COMMAND_COMPLETE | FF_MMC_INT_TRANSFER_COMPLETE)
/* Error bits b15-25 of the interrupt-status register. */
#define FF_MMC_INT_ERROR_MASK 0x03FF8000u

/* Capabilities1 (0x240) bits. */
#define FF_MMC_CAP1_SDMA (1u << 22)        /* SDMA Support */
#define FF_MMC_CAP1_VOLTAGE_3V3 (1u << 24) /* Voltage Support 3.3 V */

/* Commands used by this backend. */
#define FF_MMC_CMD_READ_SINGLE_BLOCK 17u
#define FF_MMC_CMD_READ_MULTIPLE_BLOCKS 18u
#define FF_MMC_CMD_WRITE_SINGLE_BLOCK 24u
#define FF_MMC_CMD_WRITE_MULTIPLE_BLOCKS 25u

/* Block-size bounds accepted by Setup(): a power of two in [512, 2048] (the
 * 12-bit TBS field cannot hold a 4096-byte block). */
#define FF_MMC_BLOCK_SIZE_MIN 512u
#define FF_MMC_BLOCK_SIZE_MAX 2048u

/* Maximum blocks per call: the Block Count field (BCCT) is 16 bits. */
#define FF_MMC_BCCT_MAX 0xFFFFu

/* Bounded completion poll (spin count, not a wall-clock timeout). Each
 * iteration is one interrupt-status read; the controller completes a buffered
 * transfer in well under the nominal SD clock period, so this is a generous
 * bound. */
#define FF_MMC_POLL_MAX 100000u

/* Block size used by the MMIO self-test. */
#define FF_MMC_SELFTEST_BLOCK_SIZE 512u

typedef struct
{
    const FF_BlockBus_t *bus; /* injected MMIO bus; must outlive the binding */
    uint32_t block_size;      /* bytes per block (power of two, 512..2048) */
    uint64_t num_blocks;      /* device capacity in blocks */
    uint8_t ready;            /* Setup() has completed */
    FF_BlockDev_t dev;        /* public HAL interface */
} FF_BlockMpfs_t;

/* Bind the instance and complete the FF_BlockDev_t vtable. Returns
 * FF_INVALID_ARGUMENT if d, bus or either bus hook (read32/write32) is NULL, if
 * block_size is not a power of two in [512, 2048], or if num_blocks is 0. The
 * bus descriptor is kept by pointer: it must outlive the binding (give it static
 * storage). The HAL block-device contract has no init(), so Setup() also marks
 * the instance ready. */
FF_Status_t FF_BlockMpfsSetup(FF_BlockMpfs_t *d, const FF_BlockBus_t *bus, uint32_t block_size,
                              uint64_t num_blocks);

/* MMIO self-test: round-trip BlockSizeBlockCount (0x204) and Argument1 (0x208),
 * confirm Capabilities1 (0x240) advertises SDMA and 3.3 V support and that
 * HostControl2 (0x22C) reports the internal clock stable. Used by the host unit
 * test and the Renode functional check (the MSS MMC at 0x20008000).
 *
 * Scope note: the emulated board has **no SD card attached**, so a real command
 * would log "no SD card" and do nothing; the self-test therefore performs
 * register round trips only and does not issue a command. The full transfer path
 * is host-tested.
 *
 * This is a bring-up check with configuration side effects: it leaves
 * BlockSizeBlockCount and Argument1 cleared. Run it before normal operation, not
 * against concurrent traffic. */
FF_Status_t FF_BlockMpfsSelfTest(FF_BlockMpfs_t *d);

#endif /* FF_BLOCK_MPFS_H */
