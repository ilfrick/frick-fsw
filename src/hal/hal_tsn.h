/*
 * frick-fsw - HAL contract: TSN (deterministic Ethernet) backend
 *
 * Stack/PHY-independent seam for IEEE 802.1 deterministic Ethernet
 * (SRS-REQ-028): VLAN/PCP (802.1Q), time-aware scheduling (802.1Qbv),
 * credit-based shaping (802.1Qav) and time synchronisation (802.1AS).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_HAL_TSN_H
#define FF_HAL_TSN_H

#include "common/ff_types.h"

typedef struct FF_TsnBackend FF_TsnBackend_t;
struct FF_TsnBackend
{
    const char *name;
    FF_Status_t (*init)(void);

    /* 802.1Q: VLAN id (0-4094) and priority code point (0-7). */
    FF_Status_t (*set_vlan)(uint16_t vid, uint8_t pcp);

    /* 802.1Qbv: time-aware shaper; schedule is a backend-defined table. */
    FF_Status_t (*configure_tas)(const uint8_t *schedule, uint16_t len);

    /* 802.1Qav: credit-based shaper idle slope (bits/s) per traffic class. */
    FF_Status_t (*configure_cbs)(uint8_t traffic_class, uint32_t idle_slope_bps);

    /* 802.1AS: read the synchronised TSN grandmaster time (ns). */
    FF_Status_t (*get_time_ns)(uint64_t *out_ns);
};

#endif /* FF_HAL_TSN_H */
