/*
 * frick-fsw - lwIP configuration for the frick-fsw network adapter
 *
 * Minimal lwIP options enabling the socket/netconn API and the built-in
 * loopback interface (127.0.0.1) so the adapter is self-testable on the host.
 * Used only when FF_USE_LWIP=ON; see docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_LWIPOPTS_H
#define FF_LWIPOPTS_H

#define NO_SYS 0
#define LWIP_SOCKET 1
#define LWIP_NETCONN 1
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DHCP 0
#define LWIP_IPV4 1
#define LWIP_IPV6 0
#define LWIP_HAVE_LOOPIF 1
#define LWIP_NETIF_LOOPBACK 1
#define LWIP_TCPIP_CORE_LOCKING 1
#define LWIP_COMPAT_SOCKETS 1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1
#define LWIP_SO_RCVTIMEO 1

#define MEM_SIZE (64 * 1024)
#define MEMP_NUM_NETCONN 8
#define MEMP_NUM_TCP_PCB 8
#define MEMP_NUM_TCP_PCB_LISTEN 4
#define MEMP_NUM_TCP_SEG 16
#define MEMP_NUM_SYS_TIMEOUT 16
#define TCP_MSS 1460
#define TCP_SND_BUF (2 * TCP_MSS)

#endif /* FF_LWIPOPTS_H */
