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

/* Classic (RTEMS) API objects. */
#define CONFIGURE_MAXIMUM_TASKS 16
#define CONFIGURE_MAXIMUM_TIMERS 8
#define CONFIGURE_MAXIMUM_SEMAPHORES 16

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
