/*
 * frick-fsw - HAL core registry unit tests (milestone M3)
 *
 * Verifies the bounded, allocation-free registry (SRS-REQ-025) and the
 * presence of every bus/device contract type (SRS-REQ-026..032).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "hal/hal.h"
#include "hal/hal_eth.h"
#include "hal/hal_net.h"
#include "hal/hal_tsn.h"
#include "hal/hal_can.h"
#include "hal/hal_serial.h"
#include "hal/hal_spi.h"
#include "hal/hal_pcie.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spacewire.h"
#include "hal/hal_mem.h"

static void test_registry_basic(void)
{
    static int backend_a;
    static int backend_b;

    FF_TEST_ASSERT(FF_HalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_HalTotal() == 0u);
    FF_TEST_ASSERT(FF_HalCount(FF_HAL_ETH) == 0u);

    FF_TEST_ASSERT(FF_HalRegister((FF_HalClass_t)99, "x", &backend_a) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_ETH, NULL, &backend_a) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_ETH, "eth0", NULL) == FF_INVALID_ARGUMENT);

    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_ETH, "eth0", &backend_a) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_ETH, "eth1", &backend_b) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_ETH, "eth0", &backend_b) == FF_ALREADY_EXISTS);
    FF_TEST_ASSERT(FF_HalCount(FF_HAL_ETH) == 2u);
    FF_TEST_ASSERT(FF_HalTotal() == 2u);

    FF_TEST_ASSERT(FF_HalGet(FF_HAL_ETH, "eth0") == &backend_a);
    FF_TEST_ASSERT(FF_HalGet(FF_HAL_ETH, "eth1") == &backend_b);
    FF_TEST_ASSERT(FF_HalGet(FF_HAL_ETH, "ethX") == NULL);
    FF_TEST_ASSERT(FF_HalGet((FF_HalClass_t)99, "eth0") == NULL);
    FF_TEST_ASSERT(FF_HalGet(FF_HAL_CAN, "eth0") == NULL); /* per-class namespace */
}

static void test_registry_capacity(void)
{
    static int backend[FF_HAL_MAX_BACKENDS + 1u];
    static const char *names[FF_HAL_MAX_BACKENDS + 1u] = {"n0", "n1", "n2", "n3", "n4",
                                                          "n5", "n6", "n7", "n8"};
    uint16_t i;

    FF_TEST_ASSERT(FF_HalInit() == FF_SUCCESS);
    for (i = 0u; i < (uint16_t)FF_HAL_MAX_BACKENDS; i++) {
        FF_TEST_ASSERT(FF_HalRegister(FF_HAL_CAN, names[i], &backend[i]) == FF_SUCCESS);
    }
    FF_TEST_ASSERT(FF_HalCount(FF_HAL_CAN) == (uint16_t)FF_HAL_MAX_BACKENDS);
    FF_TEST_ASSERT(FF_HalRegister(FF_HAL_CAN, names[FF_HAL_MAX_BACKENDS],
                                  &backend[FF_HAL_MAX_BACKENDS]) == FF_ERROR);
    FF_TEST_ASSERT(FF_HalCount(FF_HAL_CAN) == (uint16_t)FF_HAL_MAX_BACKENDS);
}

static void test_contract_presence(void)
{
    /* Every contract type is a usable object (compile-time presence, so that
     * each contract header is compiled by at least one translation unit). */
    FF_EthMac_t mac;
    FF_NetIf_t netif;
    FF_NetStack_t stack;
    FF_TsnBackend_t tsn;
    FF_CanBackend_t can;
    FF_CanOpenStack_t canopen;
    FF_Serial_t serial;
    FF_Spi_t spi;
    FF_I2c_t i2c;
    FF_Pcie_t pcie;
    FF_Gpio_t gpio;
    FF_SpaceWire_t spw;
    FF_Rmap_t rmap;
    FF_MemPool_t pool;
    FF_Dma_t dma;
    FF_Mtd_t mtd;
    FF_Nvm_t nvm;
    FF_BlockDev_t blk;

    (void)mac;
    (void)netif;
    (void)stack;
    (void)tsn;
    (void)can;
    (void)canopen;
    (void)serial;
    (void)spi;
    (void)i2c;
    (void)pcie;
    (void)gpio;
    (void)spw;
    (void)rmap;
    (void)pool;
    (void)dma;
    (void)mtd;
    (void)nvm;
    (void)blk;
    FF_TEST_ASSERT(sizeof(FF_EthMac_t) > 0u);
    FF_TEST_ASSERT(sizeof(FF_TsnBackend_t) > 0u);
    FF_TEST_ASSERT(sizeof(FF_BlockDev_t) > 0u);
}

FF_TEST_MAIN(test_registry_basic, test_registry_capacity, test_contract_presence)
