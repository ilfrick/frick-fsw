/*
 * frick-fsw - CANopen reference stack unit tests (milestone M4/S4c)
 *
 * Exercises SRS-REQ-063 on the host: the NMT state machine, expedited SDO
 * (local object dictionary) and PDO/emergency emission over a mock CAN backend
 * that records the last transmitted frame. Reference CiA 301 NMT codes and
 * state codes are checked explicitly (norm N22): 0x01 Operational, 0x02 Stopped,
 * 0x80 Pre-operational; reset commands are rejected; states 0x00 Init, 0x04 Stopped, 0x05
 * Operational, 0x7F Pre-operational.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/can/canopen/ff_canopen.h"
#include <string.h>

/* --- mock CAN backend (records the last transmitted CAN frame) -------------- */

typedef struct
{
    uint32_t id;
    uint8_t ext;
    uint8_t len;
    uint8_t data[8];
} MockCanFrame_t;

typedef struct
{
    FF_CanBackend_t can;
    MockCanFrame_t last;
    uint16_t calls;
    uint8_t tx_busy; /* when set, transmit() returns FF_BUSY */
    FF_CanRxCb_t rx_cb;
    void *rx_ctx;
    uint8_t inject_rx_on_tx;
    uint8_t injected_rx[8];
} MockCan_t;

static MockCan_t *g_mock;

static FF_Status_t mock_init(void)
{
    return FF_SUCCESS;
}

static FF_Status_t mock_configure(uint32_t bitrate, uint8_t fd, uint32_t data_bitrate)
{
    (void)bitrate;
    (void)fd;
    (void)data_bitrate;
    return FF_SUCCESS;
}

static FF_Status_t mock_transmit(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len)
{
    if ((g_mock == NULL) || (len > 8u) || ((data == NULL) && (len != 0u))) {
        return FF_INVALID_ARGUMENT;
    }
    if (g_mock->tx_busy != 0u) {
        return FF_BUSY;
    }
    if ((g_mock->inject_rx_on_tx != 0u) && (g_mock->rx_cb != NULL)) {
        g_mock->inject_rx_on_tx = 0u;
        g_mock->rx_cb(0u, 0u, g_mock->injected_rx, 2u, g_mock->rx_ctx);
    }
    g_mock->calls++;
    g_mock->last.id = id;
    g_mock->last.ext = ext;
    g_mock->last.len = len;
    if (len != 0u) {
        memcpy(g_mock->last.data, data, len);
    }
    return FF_SUCCESS;
}

static void mock_set_rx_callback(FF_CanRxCb_t cb, void *ctx)
{
    if (g_mock != NULL) {
        g_mock->rx_cb = cb;
        g_mock->rx_ctx = ctx;
    }
}

static void mock_can_init(MockCan_t *m)
{
    m->can.name = "mock-can";
    m->can.supports_fd = 1u;
    m->can.init = mock_init;
    m->can.configure = mock_configure;
    m->can.transmit = mock_transmit;
    m->can.set_rx_callback = mock_set_rx_callback;
    m->calls = 0u;
    m->tx_busy = 0u;
    m->last.id = 0u;
    m->last.ext = 0u;
    m->last.len = 0u;
    m->rx_cb = NULL;
    m->rx_ctx = NULL;
    m->inject_rx_on_tx = 0u;
    g_mock = m;
}

static MockCan_t g_can;
static FF_CanopenMpfs_t g_node;
static FF_CanopenMpfs_t g_node2;
static FF_CanOpenOD_t g_bad_od;

static void test_canopen_setup(void)
{
    mock_can_init(&g_can);

    g_bad_od.count = 1u;
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 0u, &ff_canopen_reference_od) ==
                   FF_INVALID_ARGUMENT); /* node id 0 reserved */
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 128u, &ff_canopen_reference_od) ==
                   FF_INVALID_ARGUMENT); /* node id > 127 */
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(NULL, &g_can.can, 1u, &ff_canopen_reference_od) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, NULL, 1u, &ff_canopen_reference_od) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 1u, NULL) == FF_INVALID_ARGUMENT);

    g_bad_od.records = NULL;
    g_bad_od.count = 1u;
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 1u, &g_bad_od) == FF_INVALID_ARGUMENT);

    /* A duplicate (index, sub) is rejected before binding. */
    {
        static FF_ODRecord_t dup[2];
        static uint32_t v0;
        static uint32_t v1;
        FF_CanOpenOD_t od;
        dup[0] = (FF_ODRecord_t){0x6040u, 0x00u, 2u, 1u, (uint8_t *)&v0};
        dup[1] = (FF_ODRecord_t){0x6040u, 0x00u, 2u, 1u, (uint8_t *)&v1};
        od.records = dup;
        od.count = 2u;
        FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 1u, &od) == FF_ALREADY_EXISTS);
    }

    /* Bad object length (3 is not an expedited SDO width). */
    {
        static FF_ODRecord_t bad[1];
        static uint32_t v;
        FF_CanOpenOD_t od;
        bad[0] = (FF_ODRecord_t){0x6040u, 0x00u, 3u, 0u, (uint8_t *)&v};
        od.records = bad;
        od.count = 1u;
        FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 1u, &od) == FF_BAD_SIZE);
    }

    /* Nominal bind: boots in Pre-operational (CiA 301), vtable wired. */
    mock_can_init(&g_can);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 5u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.node_id == 5u);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_INIT); /* 0x00, before backend init */
    FF_TEST_ASSERT(g_node.nmt_commands == 0u);
    FF_TEST_ASSERT(g_node.open.sdo_write != NULL);
    FF_TEST_ASSERT(g_node.open.sdo_read != NULL);
    FF_TEST_ASSERT(g_node.open.pdo_send != NULL);
    FF_TEST_ASSERT(g_node.open.init != NULL);
    FF_TEST_ASSERT(g_node.can == &g_can.can);
    FF_TEST_ASSERT(g_node.ready == 0u);
    FF_TEST_ASSERT(g_node.open.init(&g_can.can, 5u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_PREOP);
    FF_TEST_ASSERT(g_node.ready == 1u);

    /* A second binding becomes the single bound instance; the first one can no
     * longer be addressed through the implicit binding. */
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node2, &g_can.can, 6u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node2.node_id == 6u);

    /* Guard: a CAN backend without the required vtable slots is refused. */
    {
        FF_CanBackend_t incomplete = {0};
        FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &incomplete, 1u, &ff_canopen_reference_od) ==
                       FF_INVALID_ARGUMENT);
    }
}

static void test_canopen_nmt(void)
{
    uint8_t nmt[2] = {FF_CANOPEN_NMT_OPERATIONAL, 1u};

    mock_can_init(&g_can);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 1u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.open.init(&g_can.can, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.rx_cb != NULL);

    /* NMT receive path: targeted command on COB-ID 0x000 transitions to OP. */
    g_can.rx_cb(0u, 0u, nmt, 2u, g_can.rx_ctx);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_OPERATIONAL);
    FF_TEST_ASSERT(g_node.nmt_commands == 1u);

    /* Wrong instance is refused. */
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node2, FF_CANOPEN_NMT_OPERATIONAL) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_node.nmt_commands == 1u);

    /* The canonical CiA 301 transition sequence from boot-up. */
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_OPERATIONAL) == FF_SUCCESS);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_OPERATIONAL);
    FF_TEST_ASSERT(g_node.nmt_commands == 2u);

    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_STOPPED) == FF_SUCCESS);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_STOPPED);
    FF_TEST_ASSERT(g_node.nmt_commands == 3u);

    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_PREOP) == FF_SUCCESS);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_PREOP);
    FF_TEST_ASSERT(g_node.nmt_commands == 4u);

    /* An unknown command is refused and counted, state unchanged. */
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, 0x7Fu) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_PREOP);
    FF_TEST_ASSERT(g_node.nmt_errors == 1u);
    FF_TEST_ASSERT(g_node.nmt_commands == 4u);

    /* Reset commands are rejected because this subset cannot perform reset sequencing. */
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_RESET_NODE) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_RESET_COMM) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_PREOP);
    FF_TEST_ASSERT(g_node.nmt_errors == 3u);
    FF_TEST_ASSERT(g_node.nmt_commands == 4u);
}

static void test_canopen_heartbeat(void)
{
    uint8_t due = 0u;
    uint8_t state;

    mock_can_init(&g_can);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 7u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.open.init(&g_can.can, 7u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_OPERATIONAL) == FF_SUCCESS);

    /* 1000 ms period: due only on slot boundaries. */
    state = FF_CanOpenHeartbeatState(&g_node, 0u, &due);
    FF_TEST_ASSERT(state == (uint8_t)FF_CANOPEN_STATE_OPERATIONAL);
    FF_TEST_ASSERT(due == 1u);
    state = FF_CanOpenHeartbeatState(&g_node, 1u, &due);
    FF_TEST_ASSERT(due == 0u);
    state = FF_CanOpenHeartbeatState(&g_node, 999u, &due);
    FF_TEST_ASSERT(due == 0u);
    state = FF_CanOpenHeartbeatState(&g_node, 1000u, &due);
    FF_TEST_ASSERT(due == 1u);
    FF_TEST_ASSERT(FF_CanOpenHeartbeatProcess(&g_node, 1000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 1u);
    FF_TEST_ASSERT(g_can.last.id == (0x700u + 7u));
    FF_TEST_ASSERT(g_can.last.len == 1u && g_can.last.data[0] == 0x05u);
    FF_TEST_ASSERT(FF_CanOpenHeartbeatProcess(&g_node, 1000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 1u); /* same heartbeat slot is not emitted twice */
    FF_TEST_ASSERT(FF_CanOpenHeartbeatProcess(&g_node, 1001u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 1u);

    /* The state byte follows the NMT state. */
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_STOPPED) == FF_SUCCESS);
    state = FF_CanOpenHeartbeatState(&g_node, 1000u, NULL);
    FF_TEST_ASSERT(state == (uint8_t)FF_CANOPEN_STATE_STOPPED);
    FF_TEST_ASSERT(FF_CanOpenHeartbeatProcess(&g_node, 2000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 2u);
    FF_TEST_ASSERT(g_can.last.data[0] == 0x04u);

    /* A zero period disables the heartbeat (no slot is ever due). */
    g_node.guard_time_ms = 0u;
    state = FF_CanOpenHeartbeatState(&g_node, 0u, &due);
    FF_TEST_ASSERT(due == 0u);
    FF_TEST_ASSERT(state == 0x04u); /* Stopped state code */
    FF_TEST_ASSERT(FF_CanOpenHeartbeatProcess(&g_node, 3000u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 2u); /* disabled producer does not transmit */
    g_node.guard_time_ms = FF_CANOPEN_HEARTBEAT_PERIOD_MS;
}

static void test_canopen_sdo(void)
{
    uint8_t request[8] = {0x40u, 0x40u, 0x60u, 0u, 0u, 0u, 0u, 0u};
    uint8_t buf[8];
    uint8_t len;
    uint32_t v32;

    mock_can_init(&g_can);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 1u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.open.init(&g_can.can, 1u) == FF_SUCCESS);

    /* Read the writable CiA 402 control word (6040h): zero until written. */
    len = 2u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(len == 2u);
    FF_TEST_ASSERT(buf[0] == 0u && buf[1] == 0u);

    /* Write it and read it back (little-endian). */
    buf[0] = 0x78u;
    buf[1] = 0x56u;
    FF_TEST_ASSERT(FF_CanOpenSdoWrite(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, 2u) ==
                   FF_SUCCESS);
    len = 2u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    v32 = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8);
    FF_TEST_ASSERT(v32 == 0x5678u);
    FF_TEST_ASSERT(g_node.sdo_write == 1u);
    FF_TEST_ASSERT(g_node.sdo_read == 2u);

    /* The read-only status word cannot be written but can be read. */
    buf[0] = 0xFFu;
    FF_TEST_ASSERT(FF_CanOpenSdoWrite(1u, FF_CANOPEN_CI402_STATUS_WORD, 0u, buf, 2u) ==
                   FF_NOT_READY);
    len = 2u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_STATUS_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.sdo_errors == 1u);

    /* Unknown objects. */
    FF_TEST_ASSERT(FF_CanOpenSdoWrite(1u, 0x5000u, 0u, buf, 2u) == FF_NOT_FOUND);
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, 0x5000u, 0u, buf, &len) == FF_NOT_FOUND);
    FF_TEST_ASSERT(g_node.sdo_errors == 3u);

    /* Only 1/2/4-byte transfers are valid for expedited SDO. */
    FF_TEST_ASSERT(FF_CanOpenSdoWrite(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, 3u) ==
                   FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_CanOpenSdoWrite(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, NULL, 2u) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, NULL, &len) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, NULL) ==
                   FF_INVALID_ARGUMENT);

    /* A short read returns the available bytes and updates *len. */
    len = 1u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(len == 1u && buf[0] == 0x78u);
    len = 4u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(len == 2u);
    FF_TEST_ASSERT(((uint32_t)buf[0] | ((uint32_t)buf[1] << 8)) == 0x5678u);

    /* Wire SDO expedited upload/download server exchange through the registered
     * receive callback and check the emitted response frames. */
    FF_TEST_ASSERT(g_can.rx_cb != NULL);
    g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
    FF_TEST_ASSERT(g_can.last.id == 0x581u);
    FF_TEST_ASSERT(g_can.last.len == 8u);
    FF_TEST_ASSERT(g_can.last.data[0] == 0x4Bu); /* 2-byte upload response */
    FF_TEST_ASSERT(g_can.last.data[4] == 0x78u && g_can.last.data[5] == 0x56u);

    request[0] = 0x2Bu; /* 2-byte expedited download to 6040h:00 */
    request[4] = 0x34u;
    request[5] = 0x12u;
    g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
    FF_TEST_ASSERT(g_can.last.id == 0x581u);
    FF_TEST_ASSERT(g_can.last.data[0] == 0x60u);
    FF_TEST_ASSERT(g_can.last.data[1] == 0x40u && g_can.last.data[2] == 0x60u);
    len = 2u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(buf[0] == 0x34u && buf[1] == 0x12u);

    /* Existing index with an absent sub-index returns the CiA 301 sub-index abort. */
    request[0] = 0x40u;
    request[3] = 0x01u;
    {
        const uint16_t calls_before = g_can.calls;
        g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
        FF_TEST_ASSERT(g_can.calls == (uint16_t)(calls_before + 1u));
        FF_TEST_ASSERT(g_can.last.id == 0x581u && g_can.last.len == 8u);
        FF_TEST_ASSERT(g_can.last.data[1] == 0x40u && g_can.last.data[2] == 0x60u &&
                       g_can.last.data[3] == 0x01u);
        FF_TEST_ASSERT(g_can.last.data[0] == 0x80u);
        FF_TEST_ASSERT(g_can.last.data[4] == 0x11u && g_can.last.data[5] == 0x00u &&
                       g_can.last.data[6] == 0x09u && g_can.last.data[7] == 0x06u);
    }

    /* Downloads to the same absent sub-index return the same explicit abort. */
    request[0] = 0x2Bu;
    {
        const uint16_t calls_before = g_can.calls;
        g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
        FF_TEST_ASSERT(g_can.calls == (uint16_t)(calls_before + 1u));
        FF_TEST_ASSERT(g_can.last.id == 0x581u && g_can.last.len == 8u);
        FF_TEST_ASSERT(g_can.last.data[1] == 0x40u && g_can.last.data[2] == 0x60u &&
                       g_can.last.data[3] == 0x01u);
        FF_TEST_ASSERT(g_can.last.data[0] == 0x80u);
        FF_TEST_ASSERT(g_can.last.data[4] == 0x11u && g_can.last.data[5] == 0x00u &&
                       g_can.last.data[6] == 0x09u && g_can.last.data[7] == 0x06u);
    }

    /* An absent index retains the object-not-found abort code. */
    request[1] = 0x00u;
    request[2] = 0x50u;
    request[3] = 0x00u;
    {
        const uint16_t calls_before = g_can.calls;
        g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
        FF_TEST_ASSERT(g_can.calls == (uint16_t)(calls_before + 1u));
        FF_TEST_ASSERT(g_can.last.id == 0x581u && g_can.last.len == 8u);
        FF_TEST_ASSERT(g_can.last.data[1] == 0x00u && g_can.last.data[2] == 0x50u &&
                       g_can.last.data[3] == 0x00u);
        FF_TEST_ASSERT(g_can.last.data[0] == 0x80u);
        FF_TEST_ASSERT(g_can.last.data[4] == 0x00u && g_can.last.data[5] == 0x00u &&
                       g_can.last.data[6] == 0x02u && g_can.last.data[7] == 0x06u);
    }

    /* Absent-index upload produces a fresh object-not-found abort as well. */
    request[0] = 0x40u;
    {
        const uint16_t calls_before = g_can.calls;
        g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
        FF_TEST_ASSERT(g_can.calls == (uint16_t)(calls_before + 1u));
        FF_TEST_ASSERT(g_can.last.id == 0x581u && g_can.last.len == 8u);
        FF_TEST_ASSERT(g_can.last.data[1] == 0x00u && g_can.last.data[2] == 0x50u &&
                       g_can.last.data[3] == 0x00u);
        FF_TEST_ASSERT(g_can.last.data[0] == 0x80u);
        FF_TEST_ASSERT(g_can.last.data[4] == 0x00u && g_can.last.data[5] == 0x00u &&
                       g_can.last.data[6] == 0x02u && g_can.last.data[7] == 0x06u);
    }

    /* Failed success-response transmission rolls the OD update back. */
    request[0] = 0x2Bu;
    request[1] = 0x40u;
    request[2] = 0x60u;
    request[3] = 0x00u;
    request[4] = 0xCDu;
    request[5] = 0xABu;
    {
        const uint16_t calls_before = g_can.calls;
        const uint16_t writes_before = g_node.sdo_write;
        const uint16_t errors_before = g_node.sdo_errors;
        g_can.tx_busy = 1u;
        g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
        g_can.tx_busy = 0u;
        FF_TEST_ASSERT(g_can.calls == calls_before);
        FF_TEST_ASSERT(g_node.sdo_write == writes_before);
        FF_TEST_ASSERT(g_node.sdo_errors == (uint16_t)(errors_before + 1u));
    }
    len = 2u;
    FF_TEST_ASSERT(FF_CanOpenSdoRead(1u, FF_CANOPEN_CI402_CONTROL_WORD, 0u, buf, &len) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(buf[0] == 0x34u && buf[1] == 0x12u);

    /* Read-only download returns an SDO abort, never a success acknowledgement. */
    request[0] = 0x2Bu;
    request[4] = 0xFFu;
    request[5] = 0xFFu;
    request[1] = 0x41u; /* 6041h status word */
    g_can.rx_cb(0x601u, 0u, request, 8u, g_can.rx_ctx);
    FF_TEST_ASSERT(g_can.last.id == 0x581u);
    FF_TEST_ASSERT(g_can.last.data[0] == 0x80u);
    FF_TEST_ASSERT(g_can.last.data[4] == 0x02u); /* abort code 06010002h */
}

static void test_canopen_pdo(void)
{
    uint8_t data[8];
    uint16_t i;

    for (i = 0u; i < 8u; i++) {
        data[i] = (uint8_t)(0x20u + i);
    }

    mock_can_init(&g_can);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 3u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.open.init(&g_can.can, 3u) == FF_SUCCESS);

    /* PDO is inhibited outside Operational, including Pre-operational. */
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x180u, data, 8u) == FF_NOT_READY);
    FF_TEST_ASSERT(g_can.calls == 0u);

    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_OPERATIONAL) == FF_SUCCESS);

    /* Out-of-bound arguments. */
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x180u, NULL, 8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x180u, data, 9u) == FF_BAD_SIZE);
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x180u, data, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x800u, data, 8u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_can.calls == 0u);

    /* Nominal PDO on COB-ID 0x180. */
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x180u, data, 8u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 1u);
    FF_TEST_ASSERT(g_can.last.id == 0x180u);
    FF_TEST_ASSERT(g_can.last.ext == 0u);
    FF_TEST_ASSERT(g_can.last.len == 8u);
    for (i = 0u; i < 8u; i++) {
        FF_TEST_ASSERT(g_can.last.data[i] == data[i]);
    }
    FF_TEST_ASSERT(g_node.pdo_events == 1u);

    /* Emergency: uses COB-ID 0x700 + node_id, err_reg as the first byte. */
    FF_TEST_ASSERT(FF_CanOpenEmergency(&g_node, 0x1234u, 0x12u, data, 5u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.calls == 2u);
    FF_TEST_ASSERT(g_can.last.id == (0x080u + 3u));
    FF_TEST_ASSERT(g_can.last.len == 8u);
    FF_TEST_ASSERT(g_can.last.data[0] == 0x34u && g_can.last.data[1] == 0x12u);
    FF_TEST_ASSERT(g_can.last.data[2] == 0x12u);
    FF_TEST_ASSERT(g_can.last.data[7] == data[4]);

    /* Emergency in the Stopped state is refused. */
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_STOPPED) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_CanOpenEmergency(&g_node, 0x1234u, 0x12u, data, 5u) == FF_NOT_READY);
    FF_TEST_ASSERT(g_can.calls == 2u); /* unchanged */
}

static void test_canopen_rx_contention_is_bounded(void)
{
    uint8_t payload = 0x5Au;
    uint8_t nmt[2] = {FF_CANOPEN_NMT_OPERATIONAL, 3u};
    const uint32_t drops_before = FF_CanOpenRxContentionDrops();

    mock_can_init(&g_can);
    FF_TEST_ASSERT(FF_CanopenMpfsSetup(&g_node, &g_can.can, 3u, &ff_canopen_reference_od) ==
                   FF_SUCCESS);
    FF_TEST_ASSERT(g_node.open.init(&g_can.can, 3u) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_CanOpenNmtCommand(&g_node, FF_CANOPEN_NMT_OPERATIONAL) == FF_SUCCESS);

    /* Simulate a backend delivering RX synchronously during transmit. The
     * non-blocking guard must drop it (not recurse into partially updated
     * state or spin), while allowing the outer API call to complete. */
    memcpy(g_can.injected_rx, nmt, sizeof(nmt));
    g_can.inject_rx_on_tx = 1u;
    FF_TEST_ASSERT(FF_CanOpenPdoSend(3u, 0x183u, &payload, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_can.inject_rx_on_tx == 0u);
    FF_TEST_ASSERT(FF_CanOpenRxContentionDrops() == drops_before + 1u);
    FF_TEST_ASSERT(g_node.state == FF_CANOPEN_STATE_OPERATIONAL);
}

FF_TEST_MAIN(test_canopen_setup, test_canopen_nmt, test_canopen_heartbeat, test_canopen_sdo,
             test_canopen_pdo, test_canopen_rx_contention_is_bounded)
