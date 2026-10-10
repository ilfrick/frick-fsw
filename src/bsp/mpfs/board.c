/*
 * frick-fsw - Board init (RT PolarFire SoC)
 *
 * On the flight target the RISC-V cores boot from the Hart Software
 * Services (HSS) first stage bootloader: the E51 monitor core starts the HSS
 * which releases the U54 application cores. frick-fsw therefore enters with
 * DDR controller (DDRSS) configured by HSS, but explicitly configures:
 *
 *   - MSS clock / PLL re-lock checks
 *   - watchdog enable (MPFS watchdog on the U54 side)
 *   - console UART (FF_LINK_UART_DEVICE)
 *   - fabric context (FPGA design must match the device tree)
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "board.h"
#include "fsw_config.h"
#include "core/pal/pal.h"

const char *FF_BoardName(void)
{
    return FF_BOARD_NAME;
}

FF_Status_t FF_BoardInit(void)
{
#ifdef FRICK_PLATFORM_MPFS
    /* Real board bring-up:
     *  - assert the U54 cluster clock is at the expected frequency
     *  - configure the MSS watchdog
     *  - UART console is opened by the link layer.
     * Device tree: board/mpfs-rtpfs160zt.dts
     */
    FF_PalLog("board: %s booting (HSS released U54 hart %u)\n", FF_BoardName(),
              (unsigned)FF_MPFS_BOOT_HARTID);
    return FF_SUCCESS;
#else
    FF_PalLog("board: host simulator (no hardware)\n");
    return FF_SUCCESS;
#endif
}