/*
 * frick-fsw - Board support header (RT PolarFire SoC RTPFS160ZT)
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_BOARD_H
#define FF_BOARD_H

#include "common/ff_types.h"

const char *FF_BoardName(void);
FF_Status_t FF_BoardInit(void);

#endif /* FF_BOARD_H */
