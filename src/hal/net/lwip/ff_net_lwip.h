/*
 * frick-fsw - lwIP network-stack adapter (FF_NetStack_t)
 *
 * Implements the pluggable network-stack contract over the lwIP socket API
 * (SRS-REQ-040). lwIP is a third-party BSD-3 stack (fetched, pinned) whose
 * `contrib` unix port is part of the lwIP tree. Addresses are 6 octets: IPv4[4]
 * (big-endian) followed by port[2] (big-endian). Built only when FF_USE_LWIP=ON.
 * See docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_NET_LWIP_H
#define FF_NET_LWIP_H

#include "hal/hal_net.h"

extern FF_NetStack_t FF_NetStackLwip;

#endif /* FF_NET_LWIP_H */
