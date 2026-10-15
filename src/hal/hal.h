/*
 * frick-fsw - Hardware/OS abstraction layer (HAL) core registry
 *
 * Milestone M3. The HAL is the seam between the framework and every bus,
 * device and memory resource. Contracts are declared per resource
 * (hal_eth.h, hal_net.h, hal_tsn.h, hal_can.h, hal_serial.h, hal_spi.h
 * [SPI + I2C], hal_pcie.h, hal_gpio.h, hal_spacewire.h, hal_fabric.h,
 * hal_mem.h); this header
 * holds the bounded, allocation-free registry that lets a user register their
 * own backend under a stable name, per resource class, without modifying the
 * framework. See docs/architecture/hardware-abstraction.md.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_HAL_H
#define FF_HAL_H

#include "common/ff_types.h"

/* Resource classes of the HAL. One contract family per class. */
typedef enum {
    FF_HAL_ETH = 0,   /* Ethernet MAC + net interface (hal_eth.h)      */
    FF_HAL_NET,       /* Pluggable TCP/IP stack (hal_net.h)            */
    FF_HAL_TSN,       /* TSN scheduling backend (hal_tsn.h)            */
    FF_HAL_CAN,       /* CAN / CAN-FD backend (hal_can.h)              */
    FF_HAL_SERIAL,    /* UART / RS-422 / RS-485 (hal_serial.h)         */
    FF_HAL_SPI,       /* SPI master/slave (hal_spi.h)                  */
    FF_HAL_I2C,       /* I2C master (hal_spi.h)                        */
    FF_HAL_PCIE,      /* PCIe endpoint/root (hal_pcie.h)               */
    FF_HAL_GPIO,      /* GPIO + interrupt (hal_gpio.h)                 */
    FF_HAL_SPACEWIRE, /* SpaceWire + RMAP (hal_spacewire.h)            */
    FF_HAL_FABRIC,    /* Fabric-attached device via FIC/AXI (hal_fabric.h) */
    FF_HAL_MEM,       /* RAM pool + DMA (hal_mem.h)                    */
    FF_HAL_BLOCK,     /* Block device: eMMC/SD/SSD (hal_mem.h)         */
    FF_HAL_NVM,       /* Non-volatile memory: eNVM/sNVM (hal_mem.h)    */
    FF_HAL_CLASS_COUNT
} FF_HalClass_t;

/* Lifecycle: reset the registry (no dynamic allocation; norm N10). */
FF_Status_t FF_HalInit(void);

/* Register a backend under `name` for a resource class. Rejects invalid class,
 * NULL name/backend, a duplicate name (FF_ALREADY_EXISTS) and a full class
 * (FF_ERROR). Bounded by FF_HAL_MAX_BACKENDS per class. The `name` pointer
 * (and the backend) must remain valid for the lifetime of the registration
 * (typically a string literal or static storage; the registry stores the
 * pointer, it does not copy). */
FF_Status_t FF_HalRegister(FF_HalClass_t cls, const char *name, void *backend);

/* Look up a backend by class and name; NULL when absent/invalid.
 *
 * Concurrency: registration is a start-up operation (single-threaded, before
 * the applications are started); lookups after start-up are read-only and safe
 * from multiple tasks. The registry does not lock (norm N10: no hidden
 * blocking in the hot path). */
void *FF_HalGet(FF_HalClass_t cls, const char *name);

/* Number of backends registered in a class / total across all classes. */
uint16_t FF_HalCount(FF_HalClass_t cls);
uint16_t FF_HalTotal(void);

#endif /* FF_HAL_H */
