/*
 * frick-fsw - lwIP configuration for the RTEMS target
 *
 * RTEMS-specific lwIP options (SRS-REQ-040, NC-S6-01). It reuses the shared
 * reference feature set (`../lwipopts.h`, identical to the host build) and adds
 * the thread/mailbox sizing that the shared file intentionally leaves at lwIP's
 * built-in defaults. Selected only for the RTEMS target (`FF_USE_LWIP=ON` with
 * the RTEMS platform); the host build keeps using the shared file unchanged.
 *
 * See docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_LWIPOPTS_RTEMS_H
#define FF_LWIPOPTS_RTEMS_H

/* Shared reference options: NO_SYS=0, socket/netconn API, loopback interface,
 * core locking. */
#include "../lwipopts.h"

/*
 * RTEMS thread priorities. The port uses the lwIP `prio` argument directly as
 * the RTEMS task priority (RTEMS: 0 is the most urgent, 255 the least urgent).
 * The values below keep the lwIP threads below the framework's high-rate tasks.
 */
#define TCPIP_THREAD_PRIO 4
#define DEFAULT_THREAD_PRIO 8

/* Thread stack sizes in bytes (rtems_task_create() takes a byte count). */
#define TCPIP_THREAD_STACKSIZE (16 * 1024)
#define DEFAULT_THREAD_STACKSIZE (8 * 1024)

/*
 * Mailbox depths. lwIP defaults these to 0, which would create zero-capacity
 * RTEMS message queues; give every mailbox a bounded depth instead.
 */
#define TCPIP_MBOX_SIZE 8
#define DEFAULT_RAW_RECVMBOX_SIZE 8
#define DEFAULT_UDP_RECVMBOX_SIZE 8
#define DEFAULT_TCP_RECVMBOX_SIZE 8
#define DEFAULT_ACCEPTMBOX_SIZE 4

#endif /* FF_LWIPOPTS_RTEMS_H */
