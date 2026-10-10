/*
 * frick-fsw - Board support header (RT PolarFire SoC RTPFS160ZT)
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_BOARD_H
#define FF_BOARD_H

#include "common/ff_types.h"

const char   *FF_BoardName(void);
FF_Status_t   FF_BoardInit(void);

#endif /* FF_BOARD_H */