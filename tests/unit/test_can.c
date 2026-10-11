/*
 * frick-fsw - MSS CAN driver unit tests (milestone M4)
 *
 * Exercises the classic-CAN register logic on the host with a mock double-word
 * register file that mirrors the documented controller register map and models
 * the minimum controller semantics: a TX request with a loaded message raises
 * TX_MSG and self-clears; the status registers are read-only; an RX buffer MsgAv
 * flag is write-1-to-clear and clears its RX-status bit; and the RX acceptance
 * filter is applied in the identifier domain (AMR/ACR identifier field at bits
 * 3-31). The mock reuses the driver's register-map constants (it is a convenient
 * fake, not an independent oracle for the layout); the Renode MPFS_CAN model is
 * the independent cross-check of the register map. - SRS-REQ-057. The same
 * driver is exercised against the emulated SoC MMIO in the Renode functional test
 * (milestone M11).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "drivers/can/mpfs/ff_can_mpfs.h"
#include <string.h>

#define MOCK_WORDS (0x1000u / 4u)

typedef struct
{
    uint32_t regs[MOCK_WORDS];
    uint32_t amr_written;                /* raw value last written to the AMR register */
    uint8_t tx_valid[FF_CAN_TX_BUFFERS]; /* message loaded into the buffer */
    uint32_t tx_count;
    uint32_t tx_off; /* base of the TX buffer the last request went to */
    uint32_t tx_id;
    uint32_t tx_ext;
    uint32_t tx_dlc;
    uint8_t tx_data[FF_CAN_MAX_DLC];
} MockCan_t;

static MockCan_t g_m;
static FF_CanMpfs_t g_drv;

static uint32_t g_rx_count;
static uint32_t g_rx_id;
static uint8_t g_rx_ext;
static uint8_t g_rx_len;
static uint8_t g_rx_data[FF_CAN_MAX_DLC];

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    MockCan_t *m = (MockCan_t *)ctx;

    if (((off & 3u) != 0u) || (off >= 0x1000u)) {
        return 0u;
    }
    return m->regs[off / 4u];
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    MockCan_t *m = (MockCan_t *)ctx;
    uint32_t *slot;
    uint32_t rel;
    uint32_t idx;

    if (((off & 3u) != 0u) || (off >= 0x1000u)) {
        return;
    }
    /* The receive/transmit status registers are read-only. */
    if ((off == FF_CAN_REG_RX_STATUS) || (off == FF_CAN_REG_TX_STATUS)) {
        return;
    }
    slot = &m->regs[off / 4u];

    /* TX buffers: data load marks the buffer valid; a TxReq sends the loaded
     * message, self-clears and raises TX_MSG (a request without a loaded message
     * stays pending). */
    if ((off >= FF_CAN_TX_BASE) &&
        (off < (FF_CAN_TX_BASE + (FF_CAN_TX_BUFFERS * FF_CAN_TX_STRIDE)))) {
        idx = (off - FF_CAN_TX_BASE) / FF_CAN_TX_STRIDE;
        rel = (off - FF_CAN_TX_BASE) % FF_CAN_TX_STRIDE;
        if ((rel == FF_CAN_TX_DATA_LO_OFF) || (rel == FF_CAN_TX_DATA_HI_OFF)) {
            m->tx_valid[idx] = 1u;
            *slot = val;
        }
        else if (rel == FF_CAN_TX_ID_OFF) {
            *slot = val & ~0x7u; /* identifier field is bits 3-31 */
        }
        else if (rel == FF_CAN_TX_CMD_OFF) {
            if (((val & FF_CAN_TX_CMD_TXREQ) != 0u) && (m->tx_valid[idx] != 0u)) {
                uint32_t base = off;
                uint32_t idw = m->regs[(base + FF_CAN_TX_ID_OFF) / 4u];
                uint32_t lo = m->regs[(base + FF_CAN_TX_DATA_LO_OFF) / 4u];
                uint32_t hi = m->regs[(base + FF_CAN_TX_DATA_HI_OFF) / 4u];
                uint32_t i;

                m->tx_count++;
                m->tx_off = base;
                m->tx_id = (idw >> FF_CAN_ID_SHIFT) & FF_CAN_ID_MASK;
                m->tx_ext = ((val & FF_CAN_TX_CMD_IDE) != 0u) ? 1u : 0u;
                m->tx_dlc = (val >> FF_CAN_TX_CMD_DLC_SHIFT) & 0xFu;
                for (i = 0u; (i < m->tx_dlc) && (i < 4u); i++) {
                    m->tx_data[i] = (uint8_t)((lo >> (8u * i)) & 0xFFu);
                }
                for (i = 4u; i < m->tx_dlc; i++) {
                    m->tx_data[i] = (uint8_t)((hi >> (8u * (i - 4u))) & 0xFFu);
                }
                m->tx_valid[idx] = 0u;
                *slot = val & ~FF_CAN_TX_CMD_TXREQ; /* TxReq self-clears after send */
                m->regs[FF_CAN_REG_INTERRUPT_STATUS / 4u] |= FF_CAN_INT_TX_MSG;
            }
            else {
                *slot = val;
            }
        }
        else {
            *slot = val;
        }
        return;
    }

    /* RX buffers: the RTR/IDE mask bits and the AMR_DATA upper half are not
     * modelled (the emulator stores only the identifier-mask field and the low
     * 16 bits of AMR_DATA); MsgAv is write-1-to-clear and clears the RX status. */
    if ((off >= FF_CAN_RX_BASE) &&
        (off < (FF_CAN_RX_BASE + (FF_CAN_RX_BUFFERS * FF_CAN_RX_STRIDE)))) {
        idx = (off - FF_CAN_RX_BASE) / FF_CAN_RX_STRIDE;
        rel = (off - FF_CAN_RX_BASE) % FF_CAN_RX_STRIDE;
        if (rel == FF_CAN_RX_CMD_OFF) {
            uint32_t newv = (val & ~FF_CAN_RX_CMD_MSGAV) | (*slot & FF_CAN_RX_CMD_MSGAV);

            if ((val & FF_CAN_RX_CMD_MSGAV) != 0u) {
                newv &= ~FF_CAN_RX_CMD_MSGAV;
                m->regs[FF_CAN_REG_RX_STATUS / 4u] &= ~(1u << idx);
            }
            *slot = newv;
        }
        else if (rel == FF_CAN_RX_AMR_OFF) {
            m->amr_written = val; /* keep the raw write: the mask bits are not modelled */
            *slot = val & 0xFFFFFFF8u;
        }
        else if (rel == FF_CAN_RX_AMR_DATA_OFF) {
            *slot = val & 0xFFFFu;
        }
        else {
            *slot = val;
        }
        return;
    }

    /* Interrupt status: TX_MSG / RX_MSG (and RX_MSG_LOSS) are write-1-to-clear. */
    if (off == FF_CAN_REG_INTERRUPT_STATUS) {
        *slot = *slot & ~(val & (FF_CAN_INT_TX_MSG | FF_CAN_INT_RX_MSG | (1u << 10)));
        return;
    }
    *slot = val;
}

/* Controller acceptance filter over the whole identifier domain, mirroring the
 * Renode MPFS_CAN model: mask/code are the AMR/ACR identifier field (bits 3-31,
 * cf. FF_CAN_ID_SHIFT); mask = 1 means "don't care". */
static uint32_t mock_rx_accepts(uint32_t id)
{
    uint32_t amr = g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_AMR_OFF) / 4u];
    uint32_t acr = g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_ACR_OFF) / 4u];
    uint32_t mask = (amr >> FF_CAN_ID_SHIFT) & FF_CAN_ID_MASK;
    uint32_t code = (acr >> FF_CAN_ID_SHIFT) & FF_CAN_ID_MASK;

    return (((~mask) & (id ^ code)) == 0u) ? 1u : 0u;
}

static void rx_cb(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len, void *ctx)
{
    uint32_t i;

    (void)ctx;
    g_rx_count++;
    g_rx_id = id;
    g_rx_ext = ext;
    g_rx_len = len;
    for (i = 0u; (i < len) && (i < FF_CAN_MAX_DLC); i++) {
        g_rx_data[i] = data[i];
    }
}

/* Inject a received frame into RX buffer 0 (simulates the controller), applying
 * the acceptance filter: a filtered-out frame sets no MsgAv and no status bit.
 * Returns 1 when accepted, 0 when rejected. */
static uint32_t inject_rx_dlc(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len,
                              uint32_t dlc_field)
{
    uint32_t base = FF_CAN_RX_BASE;
    uint32_t lo = 0u;
    uint32_t hi = 0u;
    uint32_t i;

    for (i = 0u; (i < len) && (i < 4u); i++) {
        lo |= ((uint32_t)data[i]) << (8u * i);
    }
    for (i = 4u; i < len; i++) {
        hi |= ((uint32_t)data[i]) << (8u * (i - 4u));
    }
    g_m.regs[(base + FF_CAN_RX_ID_OFF) / 4u] = (id & FF_CAN_ID_MASK) << FF_CAN_ID_SHIFT;
    g_m.regs[(base + FF_CAN_RX_DATA_LO_OFF) / 4u] = lo;
    g_m.regs[(base + FF_CAN_RX_DATA_HI_OFF) / 4u] = hi;
    g_m.regs[(base + FF_CAN_RX_CMD_OFF) / 4u] = FF_CAN_RX_CMD_RXENBL |
                                                (dlc_field << FF_CAN_RX_CMD_DLC_SHIFT) |
                                                ((ext != 0u) ? FF_CAN_RX_CMD_IDE : 0u);
    if (mock_rx_accepts(id) == 0u) {
        return 0u;
    }
    g_m.regs[(base + FF_CAN_RX_CMD_OFF) / 4u] |= FF_CAN_RX_CMD_MSGAV;
    g_m.regs[FF_CAN_REG_RX_STATUS / 4u] |= 1u;
    return 1u;
}

static uint32_t inject_rx(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len)
{
    return inject_rx_dlc(id, ext, data, len, (uint32_t)len);
}

static void test_can(void)
{
    FF_CanBus_t bus = {&g_m, mock_read32, mock_write32};
    const uint8_t d4[4] = {0x11u, 0x22u, 0x33u, 0x44u};
    const uint8_t d8[8] = {0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u, 0x08u};

    memset(&g_m, 0, sizeof(g_m));

    /* Setup argument validation. */
    FF_TEST_ASSERT(FF_CanMpfsSetup(NULL, &bus) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanMpfsSetup(&g_drv, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.can.name != NULL);
    FF_TEST_ASSERT(g_drv.can.supports_fd == 0u);
    FF_TEST_ASSERT(g_drv.can.init != NULL);

    /* init: run bit set, interrupts disabled; idempotent on a second call. */
    FF_TEST_ASSERT(g_drv.can.init() == FF_SUCCESS);
    FF_TEST_ASSERT((g_m.regs[FF_CAN_REG_COMMAND / 4u] & FF_CAN_CMD_RUN) != 0u);
    FF_TEST_ASSERT(g_m.regs[FF_CAN_REG_INTERRUPT_ENABLE / 4u] == 0u);
    FF_TEST_ASSERT(g_drv.can.init() == FF_SUCCESS);
    FF_TEST_ASSERT((g_m.regs[FF_CAN_REG_COMMAND / 4u] & FF_CAN_CMD_RUN) != 0u);

    /* configure: zero bitrate, rate above the bus clock, prescaler overflow and
     * CAN-FD are all rejected; a reachable classic rate is accepted. */
    FF_TEST_ASSERT(g_drv.can.configure(0u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.can.configure(8000000u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.can.configure(100u, 0u, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.can.configure(500000u, 1u, 2000000u) == FF_NOT_IMPLEMENTED);
    FF_TEST_ASSERT(g_drv.can.configure(500000u, 0u, 0u) == FF_SUCCESS);
    {
        uint32_t cfg = g_m.regs[FF_CAN_REG_CONFIG / 4u];
        FF_TEST_ASSERT(((cfg >> 16) & 0x7FFFu) != 0u); /* prescaler */
        FF_TEST_ASSERT(((cfg >> 8) & 0xFu) == FF_CAN_TSEG1);
        FF_TEST_ASSERT(((cfg >> 5) & 0x7u) == FF_CAN_TSEG2);
        FF_TEST_ASSERT(((cfg >> 2) & 0x3u) == FF_CAN_SJW);
    }

    /* transmit: standard frame, 4 bytes - known-answer register words. */
    FF_TEST_ASSERT(g_drv.can.transmit(0x123u, 0u, d4, 4u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_m.tx_count == 1u);
    FF_TEST_ASSERT(g_m.tx_off == FF_CAN_TX_BASE);
    FF_TEST_ASSERT(g_m.regs[(FF_CAN_TX_BASE + FF_CAN_TX_ID_OFF) / 4u] ==
                   (0x123u << FF_CAN_ID_SHIFT));
    FF_TEST_ASSERT(g_m.regs[(FF_CAN_TX_BASE + FF_CAN_TX_DATA_LO_OFF) / 4u] == 0x44332211u);
    FF_TEST_ASSERT(g_m.regs[(FF_CAN_TX_BASE + FF_CAN_TX_DATA_HI_OFF) / 4u] == 0u);
    FF_TEST_ASSERT(g_m.tx_id == 0x123u);
    FF_TEST_ASSERT(g_m.tx_ext == 0u);
    FF_TEST_ASSERT(g_m.tx_dlc == 4u);
    FF_TEST_ASSERT(memcmp(g_m.tx_data, d4, 4u) == 0);
    FF_TEST_ASSERT((g_m.regs[FF_CAN_REG_INTERRUPT_STATUS / 4u] & FF_CAN_INT_TX_MSG) != 0u);

    /* TX_MSG is write-1-to-clear. */
    mock_write32(&g_m, FF_CAN_REG_INTERRUPT_STATUS, FF_CAN_INT_TX_MSG);
    FF_TEST_ASSERT((g_m.regs[FF_CAN_REG_INTERRUPT_STATUS / 4u] & FF_CAN_INT_TX_MSG) == 0u);

    /* transmit: extended frame, 8 bytes - known-answer register words. */
    FF_TEST_ASSERT(g_drv.can.transmit(0x1ABCDEFu, 1u, d8, 8u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_m.tx_count == 2u);
    FF_TEST_ASSERT(g_m.tx_off == (FF_CAN_TX_BASE + FF_CAN_TX_STRIDE));
    FF_TEST_ASSERT(g_m.regs[(FF_CAN_TX_BASE + FF_CAN_TX_STRIDE + FF_CAN_TX_DATA_LO_OFF) / 4u] ==
                   0x04030201u);
    FF_TEST_ASSERT(g_m.regs[(FF_CAN_TX_BASE + FF_CAN_TX_STRIDE + FF_CAN_TX_DATA_HI_OFF) / 4u] ==
                   0x08070605u);
    FF_TEST_ASSERT(g_m.tx_id == 0x1ABCDEFu);
    FF_TEST_ASSERT(g_m.tx_ext == 1u);
    FF_TEST_ASSERT(g_m.tx_dlc == 8u);
    FF_TEST_ASSERT(memcmp(g_m.tx_data, d8, 8u) == 0);

    /* transmit validation: nothing is sent for a rejected frame. */
    FF_TEST_ASSERT(g_drv.can.transmit(0x123u, 0u, d8, 9u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.can.transmit(0x123u, 0u, NULL, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.can.transmit(0x800u, 0u, d4, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.can.transmit(0x20000000u, 1u, d4, 4u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_m.tx_count == 2u);
    /* a zero-length frame with a NULL payload is legal. */
    FF_TEST_ASSERT(g_drv.can.transmit(0x1u, 0u, NULL, 0u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_m.tx_count == 3u);
    FF_TEST_ASSERT(g_m.tx_dlc == 0u);

    /* TX round-robin: successive frames use successive buffers; the 32-buffer
     * cursor wraps back to buffer 0. */
    {
        uint32_t i;

        FF_TEST_ASSERT(FF_CanMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
        FF_TEST_ASSERT(g_drv.can.transmit(0x1u, 0u, d4, 1u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_m.tx_off == FF_CAN_TX_BASE); /* cursor starts at buffer 0 */
        for (i = 1u; i < FF_CAN_TX_BUFFERS; i++) {
            FF_TEST_ASSERT(g_drv.can.transmit(0x2u, 0u, d4, 1u) == FF_SUCCESS);
        }
        FF_TEST_ASSERT(g_m.tx_off ==
                       (FF_CAN_TX_BASE + ((FF_CAN_TX_BUFFERS - 1u) * FF_CAN_TX_STRIDE)));
        FF_TEST_ASSERT(g_drv.can.transmit(0x3u, 0u, d4, 1u) == FF_SUCCESS);
        FF_TEST_ASSERT(g_m.tx_off == FF_CAN_TX_BASE); /* 32-buffer cursor wrapped to buffer 0 */
    }

    /* RX callback registration configures an accept-all RX buffer 0. The mask is
     * checked in the identifier domain: every identifier (including high
     * extended identifiers >= 0x04000000) must be accepted. */
    FF_TEST_ASSERT(FF_CanMpfsSetup(&g_drv, &bus) == FF_SUCCESS);
    g_drv.can.set_rx_callback(rx_cb, NULL);
    FF_TEST_ASSERT(g_drv.rx_cb == rx_cb);
    /* The exact register value written (including the RTR/IDE mask bits, which the
     * emulated controller does not model) is asserted on the raw write. */
    FF_TEST_ASSERT(g_m.amr_written == (uint32_t)FF_CAN_ACCEPT_ALL);
    FF_TEST_ASSERT((g_m.amr_written & 0x6u) == 0x6u); /* RTR+IDE mask bits, literal (R2-D5) */
    FF_TEST_ASSERT((g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_AMR_OFF) / 4u] &
                    (FF_CAN_ID_MASK << FF_CAN_ID_SHIFT)) == (FF_CAN_ID_MASK << FF_CAN_ID_SHIFT));
    FF_TEST_ASSERT(mock_rx_accepts(0x00000000u) == 1u);
    FF_TEST_ASSERT(mock_rx_accepts(0x000007FFu) == 1u);
    FF_TEST_ASSERT(mock_rx_accepts(0x04000000u) == 1u);
    FF_TEST_ASSERT(mock_rx_accepts(0x1FFFFFFFu) == 1u);
    FF_TEST_ASSERT((g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_CMD_OFF) / 4u] & FF_CAN_RX_CMD_RXENBL) !=
                   0u);

    /* poll: inject a standard frame, dispatch it, clear MsgAv, keep it enabled. */
    g_rx_count = 0u;
    FF_TEST_ASSERT(inject_rx(0x321u, 0u, d4, 4u) == 1u);
    {
        uint32_t got = 0u;

        FF_TEST_ASSERT(FF_CanMpfsPoll(&g_drv, &got) == FF_SUCCESS);
        FF_TEST_ASSERT(got == 1u);
        FF_TEST_ASSERT(g_rx_count == 1u);
        FF_TEST_ASSERT(g_rx_id == 0x321u);
        FF_TEST_ASSERT(g_rx_ext == 0u);
        FF_TEST_ASSERT(g_rx_len == 4u);
        FF_TEST_ASSERT(memcmp(g_rx_data, d4, 4u) == 0);
        FF_TEST_ASSERT(
            (g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_CMD_OFF) / 4u] & FF_CAN_RX_CMD_MSGAV) == 0u);
        FF_TEST_ASSERT(
            (g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_CMD_OFF) / 4u] & FF_CAN_RX_CMD_RXENBL) != 0u);
        /* a second poll finds nothing (MsgAv cleared). */
        FF_TEST_ASSERT(FF_CanMpfsPoll(&g_drv, &got) == FF_SUCCESS);
        FF_TEST_ASSERT(got == 0u);
        FF_TEST_ASSERT(g_rx_count == 1u);
    }

    /* poll: a high extended identifier, 8 bytes (accept-all in the id domain). */
    g_rx_count = 0u;
    FF_TEST_ASSERT(inject_rx(0x1ABCDEFFu, 1u, d8, 8u) == 1u);
    {
        uint32_t got = 0u;

        FF_TEST_ASSERT(FF_CanMpfsPoll(&g_drv, &got) == FF_SUCCESS);
        FF_TEST_ASSERT(got == 1u);
        FF_TEST_ASSERT(g_rx_id == 0x1ABCDEFFu);
        FF_TEST_ASSERT(g_rx_ext == 1u);
        FF_TEST_ASSERT(g_rx_len == 8u);
        FF_TEST_ASSERT(memcmp(g_rx_data, d8, 8u) == 0);
    }

    /* poll: with no callback registered the frame is still acknowledged/counted. */
    g_drv.can.set_rx_callback(NULL, NULL);
    g_rx_count = 0u;
    FF_TEST_ASSERT(inject_rx(0x111u, 0u, d4, 4u) == 1u);
    {
        uint32_t got = 99u;

        FF_TEST_ASSERT(FF_CanMpfsPoll(&g_drv, &got) == FF_SUCCESS);
        FF_TEST_ASSERT(got == 1u);
        FF_TEST_ASSERT(g_rx_count == 0u);
        FF_TEST_ASSERT(
            (g_m.regs[(FF_CAN_RX_BASE + FF_CAN_RX_CMD_OFF) / 4u] & FF_CAN_RX_CMD_MSGAV) == 0u);
    }
    g_drv.can.set_rx_callback(rx_cb, NULL);

    /* poll: an RX DLC field above 8 is clamped to the classic maximum. */
    g_rx_count = 0u;
    FF_TEST_ASSERT(inject_rx_dlc(0x222u, 0u, d8, 8u, 15u) == 1u);
    {
        uint32_t got = 0u;

        FF_TEST_ASSERT(FF_CanMpfsPoll(&g_drv, &got) == FF_SUCCESS);
        FF_TEST_ASSERT(got == 1u);
        FF_TEST_ASSERT(g_rx_len == FF_CAN_MAX_DLC);
    }

    /* poll argument validation (NULL out-counter is allowed). */
    FF_TEST_ASSERT(FF_CanMpfsPoll(NULL, NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_CanMpfsPoll(&g_drv, NULL) == FF_SUCCESS);

    /* MMIO self-test (same procedure used in the Renode functional check). */
    memset(&g_m, 0, sizeof(g_m));
    FF_TEST_ASSERT(FF_CanMpfsSelfTest(&g_drv) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_CanMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
}

FF_TEST_MAIN(test_can)
