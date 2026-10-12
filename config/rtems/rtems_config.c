/*
 * frick-fsw - RTEMS 6.2 application configuration (flight target)
 *
 * This is the single translation unit that carries the RTEMS Configuration
 * Table (CONFIGURE_* macros + <rtems/confdefs.h>), producing a bootable RTEMS
 * image for the Microchip RT PolarFire SoC RTPFS160ZT (BSP riscv/mpfs64imafdc).
 * The POSIX API is required by the PAL (pthreads, mutexes, condition variables,
 * message queues, keys, timers); POSIX_Init is the application entry point
 * (see src/core/pal/pal_rtems.c).
 *
 * Sizing is bounded and static (no unlimited objects); see
 * docs/architecture/rtems-integration.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include <rtems.h>

/* The POSIX initialization thread entry point (defined in
 * src/core/pal/pal_rtems.c). CONFIGURE_POSIX_INIT_THREAD_TABLE below needs the
 * declaration before <rtems/confdefs.h> (real-toolchain finding). */
void *POSIX_Init(void *argument);

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

/* Classic (RTEMS) API objects.
 *
 * lwIP (FF_USE_LWIP=ON) runs as a threaded stack (NO_SYS=0) on the RTEMS
 * target: its in-repo sys_arch port (src/hal/net/lwip/rtems/sys_arch.c) creates
 * classic RTEMS tasks, semaphores and message queues for the tcpip thread and
 * the netconn/socket objects. The counts below are sized for that stack plus
 * bounded headroom; they stay explicit and finite (deterministic sizing, no
 * CONFIGURE_UNLIMITED_OBJECTS). Derivation from the lwIP options in
 * src/hal/net/lwip/rtems/lwipopts.h (MEMP_NUM_NETCONN = 8):
 *   - message queues: 1 tcpip mailbox + up to (1 recv + 1 accept mailbox) per
 *     netconn = 1 + 2 * 8 = 17; sized 24 for headroom.
 *   - semaphores: 1 core-lock mutex + 1 memory mutex + 1 completion semaphore
 *     per netconn (<= 8) + transient port semaphores; sized 32 (was 16).
 *   - tasks: lwIP threads are classic RTEMS tasks (the framework applications
 *     run on the POSIX thread pool, CONFIGURE_MAXIMUM_POSIX_THREADS below), so
 *     the tcpip thread plus bounded worker headroom is added to the classic
 *     task pool; sized 24 (was 16).
 * Without CONFIGURE_MAXIMUM_MESSAGE_QUEUES an lwIP runtime tcpip_init() fails
 * (the count defaulted to 0): NC-S6-02, closed by S6b-2. */
#define CONFIGURE_MAXIMUM_TASKS 24
#define CONFIGURE_MAXIMUM_TIMERS 8
#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 24

/* POSIX API objects required by the PAL. Note: in RTEMS 6 POSIX mutexes and
 * condition variables are self-contained (no CONFIGURE_* needed); the
 * CONFIGURE_MAXIMUM_POSIX_MUTEXES/CONDITION_VARIABLES/MESSAGE_QUEUE_DESCRIPTORS
 * options are obsolete since RTEMS 5.1 and must not be used. */
#define CONFIGURE_MAXIMUM_POSIX_THREADS 32
#define CONFIGURE_MAXIMUM_POSIX_TIMERS 8
#define CONFIGURE_MAXIMUM_POSIX_KEYS 8
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES 8
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 16

/* POSIX initialization thread: POSIX_Init() is the application entry point. */
#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define CONFIGURE_POSIX_INIT_THREAD_STACK_SIZE (256 * 1024)

/* Tick and clock. */
#define CONFIGURE_MICROSECONDS_PER_TICK 1000
#define CONFIGURE_TICKS_PER_TIMESLICE 50

/* Deterministic sizing: no unlimited objects. */
#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_INIT

#include <rtems/confdefs.h>
