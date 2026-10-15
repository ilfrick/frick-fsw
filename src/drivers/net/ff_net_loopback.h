/*
 * frick-fsw - Reference loopback network stack (FF_NetStack_t)
 *
 * A bounded, static, in-process implementation of the pluggable network-stack
 * contract (SRS-REQ-027 / SRS-REQ-039): socket-like API over a loopback pair.
 * It exercises the contract in the unit tests and the emulated-target (Renode)
 * functional check, and
 * documents how a third-party stack (lwIP) plugs in through the same vtable.
 * No dynamic memory.
 *
 * Semantics: `send` may be partial (it returns the number of bytes accepted in
 * `*sent`); `recv` is non-blocking and returns FF_SUCCESS with `*got == 0` when
 * no data is pending. The sockets are not thread-safe: calls are serialised by
 * the owning task.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_NET_LOOPBACK_H
#define FF_NET_LOOPBACK_H

#include "hal/hal_net.h"

extern FF_NetStack_t FF_NetStackLoopback;

#endif /* FF_NET_LOOPBACK_H */
