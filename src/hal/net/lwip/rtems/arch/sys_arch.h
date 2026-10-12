/*
 * frick-fsw - lwIP system abstraction types for the RTEMS target
 *
 * lwIP `arch/sys_arch.h` (NO_SYS=0 build) for the in-repo RTEMS `sys_arch`
 * port (SRS-REQ-040, NC-S6-01). The sys objects are value types: lwIP provides
 * the storage for them, so the port allocates nothing beyond the RTEMS kernel
 * objects created by sys_arch.c. This header is used only for the RTEMS target;
 * the host build keeps using the upstream lwIP unix port.
 *
 * See docs/architecture/network-stack.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_LWIP_RTEMS_ARCH_SYS_ARCH_H
#define FF_LWIP_RTEMS_ARCH_SYS_ARCH_H

#include "lwip/arch.h"
#include "lwip/opt.h"

#include <rtems.h>

/*
 * Critical-section protection level. sys_arch_protect() returns the previous
 * RTEMS interrupt level so nested protect/unprotect pairs restore correctly
 * (see sys_arch.c).
 */
typedef rtems_interrupt_level sys_prot_t;

/* lwIP sys object handles (value types; storage is owned by lwIP). */
typedef struct
{
    rtems_id id;
} sys_sem_t;

typedef struct
{
    rtems_id id;
} sys_mutex_t;

typedef struct
{
    rtems_id id;
} sys_mbox_t;

typedef struct
{
    rtems_id id;
} sys_thread_t;

/* "Null" handle values (an invalid RTEMS object id). lwIP core uses the
 * sys_*_valid()/sys_*_set_invalid() macros below; these definitions exist for
 * port convention. */
#define SYS_SEM_NULL ((sys_sem_t){RTEMS_ID_NONE})
#define SYS_MUTEX_NULL ((sys_mutex_t){RTEMS_ID_NONE})
#define SYS_MBOX_NULL ((sys_mbox_t){RTEMS_ID_NONE})
#define SYS_THREAD_NULL ((sys_thread_t){RTEMS_ID_NONE})

/* Semaphores. */
#define sys_sem_valid_val(sem) ((sem).id != RTEMS_ID_NONE)
#define sys_sem_valid(sem) (((sem) != NULL) && sys_sem_valid_val(*(sem)))
#define sys_sem_set_invalid_val(sem) ((sem).id = RTEMS_ID_NONE)
#define sys_sem_set_invalid(sem)                                                                   \
    do {                                                                                           \
        if ((sem) != NULL) {                                                                       \
            sys_sem_set_invalid_val(*(sem));                                                       \
        }                                                                                          \
    } while (0)

/* Mutexes. */
#define sys_mutex_valid_val(mutex) ((mutex).id != RTEMS_ID_NONE)
#define sys_mutex_valid(mutex) (((mutex) != NULL) && sys_mutex_valid_val(*(mutex)))
#define sys_mutex_set_invalid_val(mutex) ((mutex).id = RTEMS_ID_NONE)
#define sys_mutex_set_invalid(mutex)                                                               \
    do {                                                                                           \
        if ((mutex) != NULL) {                                                                     \
            sys_mutex_set_invalid_val(*(mutex));                                                   \
        }                                                                                          \
    } while (0)

/* Mailboxes. */
#define sys_mbox_valid_val(mbox) ((mbox).id != RTEMS_ID_NONE)
#define sys_mbox_valid(mbox) (((mbox) != NULL) && sys_mbox_valid_val(*(mbox)))
#define sys_mbox_set_invalid_val(mbox) ((mbox).id = RTEMS_ID_NONE)
#define sys_mbox_set_invalid(mbox)                                                                 \
    do {                                                                                           \
        if ((mbox) != NULL) {                                                                      \
            sys_mbox_set_invalid_val(*(mbox));                                                     \
        }                                                                                          \
    } while (0)

/* Thread-creation diagnostics (NC-151-05). sys_thread_new() records task-create
 * failures instead of failing silently; these accessors expose the count and the
 * last RTEMS status code (see sys_arch.c). */
unsigned int frick_lwip_port_thread_failures(void);
int frick_lwip_port_last_thread_status(void);

#endif /* FF_LWIP_RTEMS_ARCH_SYS_ARCH_H */
