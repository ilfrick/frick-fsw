/*
 * frick-fsw - MSS MMC/SD block-device driver unit tests (milestone M4)
 *
 * Exercises the SDHCI-style register/transfer logic on the host with a mock
 * double-word register file that mirrors the documented MSS MMC/SD register map
 * and models the command/data path: a write to CommandTransferMode (0x20C)
 * decodes the command index / data direction / block count and, for a read,
 * copies the addressed blocks from a backing store into an internal transfer
 * buffer and raises CommandComplete+TransferComplete; a read of the DataBuffer
 * port (0x220) pops the next little-endian word, a write pushes one and, when
 * the whole payload has been moved, commits it to the backing store. The
 * driver's polled sequence (program BlockSizeBlockCount/Argument1, issue the
 * command, poll ErrorNormalInterruptStatus, move the DataBuffer) is thus driven
 * end-to-end. The mock reuses the driver's register-map constants: it is a
 * convenient behavioural fake, not an independent oracle (it cannot discriminate
 * a wrong register offset or a wrong command-bit assignment, and it models an
 * idealised buffer rather than a real FIFO-based SDHCI controller). The layout
 * is cross-checked only at the proddable register offsets by the Renode
 * `SD.MPFS_SDController` model (the Microchip `mss_sd.h` / `mss_sd_regs.h`
 * headers are not revision-controlled in this repository) and the Renode
 * functional test exercises the emulated MMIO (milestone M11); see
 * docs/vv/renode-environment.md for the emulator scope limits.
 * - SRS-REQ-070 (MMC/SD backend; SRS-REQ-032 contract).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "drivers/block/mpfs/ff_block_mpfs.h"
#include <string.h>

#define MOCK_NUM_BLOCKS 8u
#define MOCK_BLOCK_SIZE 512u
#define MOCK_STORE_BYTES (MOCK_NUM_BLOCKS * MOCK_BLOCK_SIZE)

typedef struct
{
    uint32_t block_size_count; /* 0x204 */
    uint32_t argument1;        /* 0x208 */
    uint32_t cmd_xfer_mode;    /* 0x20C */
    uint32_t err_status;       /* 0x230 (write-1-to-clear) */
    uint32_t cap1;             /* 0x240 (read-only) */
    uint32_t hc2;              /* 0x22C (read-only here) */
    uint8_t store[MOCK_STORE_BYTES];
    uint8_t xfer[MOCK_STORE_BYTES]; /* in-flight payload */
    uint32_t xfer_len;              /* bytes expected for the in-flight command */
    uint32_t xfer_pos;              /* bytes moved through the DataBuffer so far */
    uint32_t last_cmd;              /* last decoded command index */
    uint32_t last_dir_read;         /* last decoded data direction (1 = read) */
    uint32_t tbs;                   /* last decoded transfer block size */
    uint32_t block_count;           /* last decoded block count */
    uint32_t data_words;            /* DataBuffer accesses */
    uint32_t never_ready;           /* fault injection: status never completes */
    uint32_t keep_status;           /* fault injection: leave a stale status word */
    uint32_t error_status;          /* fault injection: error bit set */
} MockBlock_t;

static MockBlock_t g_mock;
static FF_BlockMpfs_t g_drv;
/* static storage: FF_BlockMpfs_t keeps a pointer to the bus descriptor */
static FF_BlockBus_t g_bus;

static void mock_reset(MockBlock_t *m)
{
    (void)memset(m, 0, sizeof(*m));
    m->cap1 = FF_MMC_CAP1_SDMA | FF_MMC_CAP1_VOLTAGE_3V3;
    m->hc2 = FF_MMC_HC2_CLOCK_STABLE;
}

static void mock_fill_store(MockBlock_t *m, uint8_t seed)
{
    uint32_t i;

    for (i = 0u; i < MOCK_STORE_BYTES; i++) {
        m->store[i] = (uint8_t)((seed + (uint8_t)i) & 0xFFu);
    }
}

/* Decode and "execute" a command written to CommandTransferMode (0x20C). */
static void mock_exec_cmd(MockBlock_t *m, uint32_t val)
{
    m->cmd_xfer_mode = val;
    m->last_cmd = (val >> FF_MMC_CMD_INDEX_SHIFT) & FF_MMC_CMD_INDEX_MASK;
    m->last_dir_read = (val & FF_MMC_XFER_DATA_DIR_READ) ? 1u : 0u;
    m->tbs = m->block_size_count & FF_MMC_TBS_MASK;
    m->block_count = (m->block_size_count >> FF_MMC_BCCT_SHIFT) & FF_MMC_BCCT_MASK;
    m->xfer_len = m->tbs * m->block_count;
    m->xfer_pos = 0u;
    if (m->xfer_len > MOCK_STORE_BYTES) {
        m->xfer_len = MOCK_STORE_BYTES;
    }
    if (m->never_ready != 0u) {
        if (m->keep_status == 0u) {
            m->err_status = 0u; /* no completion ever */
        }
        return;
    }
    if (m->error_status != 0u) {
        m->err_status = FF_MMC_INT_ERROR_MASK;
        return;
    }
    m->err_status = FF_MMC_INT_DONE;
    if (m->last_dir_read != 0u) {
        uint32_t base = m->argument1 * m->tbs;
        if ((base + m->xfer_len) <= MOCK_STORE_BYTES) {
            (void)memcpy(m->xfer, &m->store[base], m->xfer_len);
        }
    }
}

static uint32_t mock_read32(void *ctx, uint32_t off)
{
    MockBlock_t *m = (MockBlock_t *)ctx;
    uint32_t v;

    if ((off & 3u) != 0u) {
        return 0u;
    }
    switch (off) {
    case FF_MMC_REG_BLOCK_SIZE_COUNT:
        v = m->block_size_count;
        break;
    case FF_MMC_REG_ARGUMENT1:
        v = m->argument1;
        break;
    case FF_MMC_REG_CMD_XFER_MODE:
        v = m->cmd_xfer_mode;
        break;
    case FF_MMC_REG_ERR_NORMAL_INT_STATUS:
        v = m->err_status;
        break;
    case FF_MMC_REG_CAPABILITIES1:
        v = m->cap1;
        break;
    case FF_MMC_REG_HOST_CONTROL2:
        v = m->hc2;
        break;
    case FF_MMC_REG_DATA_BUFFER: {
        uint32_t w = 0u;
        if ((m->xfer_pos + 4u) <= m->xfer_len) {
            w = (uint32_t)m->xfer[m->xfer_pos + 0u] | ((uint32_t)m->xfer[m->xfer_pos + 1u] << 8) |
                ((uint32_t)m->xfer[m->xfer_pos + 2u] << 16) |
                ((uint32_t)m->xfer[m->xfer_pos + 3u] << 24);
            m->xfer_pos = m->xfer_pos + 4u;
        }
        m->data_words = m->data_words + 1u;
        v = w;
        break;
    }
    default:
        v = 0u;
        break;
    }
    return v;
}

static void mock_write32(void *ctx, uint32_t off, uint32_t val)
{
    MockBlock_t *m = (MockBlock_t *)ctx;

    if ((off & 3u) != 0u) {
        return;
    }
    switch (off) {
    case FF_MMC_REG_BLOCK_SIZE_COUNT:
        m->block_size_count = val;
        break;
    case FF_MMC_REG_ARGUMENT1:
        m->argument1 = val;
        break;
    case FF_MMC_REG_CMD_XFER_MODE:
        mock_exec_cmd(m, val);
        break;
    case FF_MMC_REG_ERR_NORMAL_INT_STATUS:
        m->err_status = m->err_status & ~val; /* write-1-to-clear */
        break;
    case FF_MMC_REG_DATA_BUFFER: {
        if ((m->xfer_pos + 4u) <= m->xfer_len) {
            m->xfer[m->xfer_pos + 0u] = (uint8_t)(val & 0xFFu);
            m->xfer[m->xfer_pos + 1u] = (uint8_t)((val >> 8) & 0xFFu);
            m->xfer[m->xfer_pos + 2u] = (uint8_t)((val >> 16) & 0xFFu);
            m->xfer[m->xfer_pos + 3u] = (uint8_t)((val >> 24) & 0xFFu);
            m->xfer_pos = m->xfer_pos + 4u;
            if (m->xfer_pos >= m->xfer_len) {
                uint32_t base = m->argument1 * m->tbs;
                if ((base + m->xfer_len) <= MOCK_STORE_BYTES) {
                    (void)memcpy(&m->store[base], m->xfer, m->xfer_len);
                }
            }
        }
        m->data_words = m->data_words + 1u;
        break;
    }
    default:
        /* read-only / unused register: ignore */
        break;
    }
}

static void mock_bus_init(void)
{
    g_bus.ctx = &g_mock;
    g_bus.read32 = mock_read32;
    g_bus.write32 = mock_write32;
}

static void mock_bind(void)
{
    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&g_drv, &g_bus, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) ==
                   FF_SUCCESS);
}

static void test_setup_and_validate(void)
{
    /* static: Setup() binds the instance into the module-global g_block, so the
     * object must outlive the binding (avoids a dangling pointer after return). */
    static FF_BlockMpfs_t drv;
    FF_BlockBus_t bad;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_BlockMpfsSetup(NULL, &g_bus, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) ==
                   FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, NULL, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) ==
                   FF_INVALID_ARGUMENT);
    /* A partially populated bus (missing hooks) is rejected. */
    bad.ctx = &g_mock;
    bad.read32 = NULL;
    bad.write32 = mock_write32;
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &bad, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) ==
                   FF_INVALID_ARGUMENT);
    bad.read32 = mock_read32;
    bad.write32 = NULL;
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &bad, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) ==
                   FF_INVALID_ARGUMENT);
    /* block_size must be a power of two in [512, 2048]. */
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, 256u, MOCK_NUM_BLOCKS) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, 0u, MOCK_NUM_BLOCKS) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, 1000u, MOCK_NUM_BLOCKS) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, 8192u, MOCK_NUM_BLOCKS) == FF_INVALID_ARGUMENT);
    /* num_blocks must be > 0. */
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, MOCK_BLOCK_SIZE, 0u) == FF_INVALID_ARGUMENT);

    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.dev.name != NULL);
    FF_TEST_ASSERT(strcmp(drv.dev.name, "mpfs-mmc") == 0);
    FF_TEST_ASSERT(drv.dev.block_size == MOCK_BLOCK_SIZE);
    FF_TEST_ASSERT(drv.dev.num_blocks == MOCK_NUM_BLOCKS);
    FF_TEST_ASSERT(drv.dev.read != NULL);
    FF_TEST_ASSERT(drv.dev.write != NULL);
    FF_TEST_ASSERT(drv.dev.flush != NULL);
    FF_TEST_ASSERT(drv.ready == 1u);
    /* A 4096-byte block does not fit the 12-bit TBS field and is rejected
     * (regression for the R1-01 truncation). */
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, 4096u, MOCK_NUM_BLOCKS) == FF_INVALID_ARGUMENT);
    /* 2048 is the largest accepted block size. */
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, 2048u, MOCK_NUM_BLOCKS) == FF_SUCCESS);
    FF_TEST_ASSERT(drv.dev.block_size == 2048u);
}

static void test_not_ready(void)
{
    /* static: Setup() binds the instance into the module-global g_block. */
    static FF_BlockMpfs_t drv;
    FF_Status_t (*rd)(uint64_t, uint8_t *, uint32_t);
    FF_Status_t (*wr)(uint64_t, const uint8_t *, uint32_t);
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) == FF_SUCCESS);
    rd = drv.dev.read;
    wr = drv.dev.write;
    /* The HAL block contract has no init(): Setup() binds and marks ready.
     * Clearing ready exercises the FF_NOT_READY gate. (The g_block == NULL
     * never-bound guard is defensive: an unbound vtable has no callable
     * pointer; it is covered by the SelfTest unbound check below.) */
    drv.ready = 0u;
    FF_TEST_ASSERT(rd(0u, buf, 1u) == FF_NOT_READY);
    FF_TEST_ASSERT(wr(0u, buf, 1u) == FF_NOT_READY);
    drv.ready = 1u;
    FF_TEST_ASSERT(rd(0u, buf, 1u) == FF_SUCCESS);
}

static void test_read_single_block(void)
{
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_bind();
    mock_fill_store(&g_mock, 0x10u);
    (void)memset(buf, 0, sizeof(buf));
    FF_TEST_ASSERT(g_drv.dev.read(2u, buf, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.last_cmd == FF_MMC_CMD_READ_SINGLE_BLOCK);
    FF_TEST_ASSERT(g_mock.last_dir_read == 1u);
    FF_TEST_ASSERT(g_mock.tbs == MOCK_BLOCK_SIZE);
    FF_TEST_ASSERT(g_mock.block_count == 1u);
    FF_TEST_ASSERT(g_mock.data_words == (MOCK_BLOCK_SIZE / 4u));
    FF_TEST_ASSERT(memcmp(buf, &g_mock.store[2u * MOCK_BLOCK_SIZE], MOCK_BLOCK_SIZE) == 0);
    /* The driver cleared the completion bits (write-1-to-clear). */
    FF_TEST_ASSERT(g_mock.err_status == 0u);
}

static void test_read_multiple_blocks(void)
{
    uint8_t buf[3u * MOCK_BLOCK_SIZE];

    mock_bind();
    mock_fill_store(&g_mock, 0x22u);
    (void)memset(buf, 0, sizeof(buf));
    FF_TEST_ASSERT(g_drv.dev.read(3u, buf, 3u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.last_cmd == FF_MMC_CMD_READ_MULTIPLE_BLOCKS);
    FF_TEST_ASSERT(g_mock.block_count == 3u);
    FF_TEST_ASSERT(memcmp(buf, &g_mock.store[3u * MOCK_BLOCK_SIZE], sizeof(buf)) == 0);
}

static void test_write_single_block(void)
{
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_bind();
    (void)memset(&g_mock.store, 0, sizeof(g_mock.store));
    (void)memset(buf, 0x5Au, sizeof(buf));
    FF_TEST_ASSERT(g_drv.dev.write(1u, buf, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.last_cmd == FF_MMC_CMD_WRITE_SINGLE_BLOCK);
    FF_TEST_ASSERT(g_mock.last_dir_read == 0u);
    FF_TEST_ASSERT(memcmp(&g_mock.store[1u * MOCK_BLOCK_SIZE], buf, MOCK_BLOCK_SIZE) == 0);
}

static void test_write_multiple_blocks(void)
{
    uint8_t buf[2u * MOCK_BLOCK_SIZE];
    uint32_t i;

    mock_bind();
    (void)memset(&g_mock.store, 0, sizeof(g_mock.store));
    for (i = 0u; i < sizeof(buf); i++) {
        buf[i] = (uint8_t)((0x80u + (uint8_t)i) & 0xFFu);
    }
    FF_TEST_ASSERT(g_drv.dev.write(4u, buf, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_mock.last_cmd == FF_MMC_CMD_WRITE_MULTIPLE_BLOCKS);
    FF_TEST_ASSERT(g_mock.block_count == 2u);
    FF_TEST_ASSERT(memcmp(&g_mock.store[4u * MOCK_BLOCK_SIZE], buf, sizeof(buf)) == 0);
}

static void test_write_read_roundtrip(void)
{
    uint8_t out[2u * MOCK_BLOCK_SIZE];
    uint8_t in[2u * MOCK_BLOCK_SIZE];
    uint32_t i;

    mock_bind();
    for (i = 0u; i < sizeof(out); i++) {
        out[i] = (uint8_t)((0xC3u + (uint8_t)i) & 0xFFu);
    }
    FF_TEST_ASSERT(g_drv.dev.write(0u, out, 2u) == FF_SUCCESS);
    (void)memset(in, 0, sizeof(in));
    FF_TEST_ASSERT(g_drv.dev.read(0u, in, 2u) == FF_SUCCESS);
    FF_TEST_ASSERT(memcmp(in, out, sizeof(out)) == 0);
}

static void test_args_validation(void)
{
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_bind();
    /* NULL buffer. */
    FF_TEST_ASSERT(g_drv.dev.read(0u, NULL, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.dev.write(0u, NULL, 1u) == FF_INVALID_ARGUMENT);
    /* Zero blocks. */
    FF_TEST_ASSERT(g_drv.dev.read(0u, buf, 0u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.dev.write(0u, buf, 0u) == FF_INVALID_ARGUMENT);
    /* Block count beyond the 16-bit BCCT field. */
    FF_TEST_ASSERT(g_drv.dev.read(0u, buf, (FF_MMC_BCCT_MAX + 1u)) == FF_INVALID_ARGUMENT);
    /* LBA at / beyond capacity. */
    FF_TEST_ASSERT(g_drv.dev.read(MOCK_NUM_BLOCKS, buf, 1u) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(g_drv.dev.read(MOCK_NUM_BLOCKS + 100u, buf, 1u) == FF_INVALID_ARGUMENT);
    /* Last valid LBA but the request runs past the end. */
    FF_TEST_ASSERT(g_drv.dev.read(MOCK_NUM_BLOCKS - 1u, buf, 2u) == FF_INVALID_ARGUMENT);
    /* The last full-capacity request is accepted. */
    FF_TEST_ASSERT(g_drv.dev.read(0u, buf, 1u) == FF_SUCCESS);
    FF_TEST_ASSERT(g_drv.dev.read(MOCK_NUM_BLOCKS - 1u, buf, 1u) == FF_SUCCESS);
    /* LBA beyond the 32-bit Argument1 register. */
    FF_TEST_ASSERT(g_drv.dev.read(0x100000000ull, buf, 1u) == FF_INVALID_ARGUMENT);
}

static void test_timeout(void)
{
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_bind();
    g_mock.never_ready = 1u; /* the controller never signals completion */
    FF_TEST_ASSERT(g_drv.dev.read(0u, buf, 1u) == FF_TIMEOUT);
    FF_TEST_ASSERT(g_drv.dev.write(0u, buf, 1u) == FF_TIMEOUT);
}

static void test_timeout_clears_stale_status(void)
{
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_bind();
    /* A stale status bit from a previous transfer is left set and the command
     * must not overwrite it (keep_status); the driver polls to the bound and
     * must clear the stale word on the timeout path (regression for R1-02). */
    g_mock.err_status = FF_MMC_INT_BUFFER_WRITE_READY;
    g_mock.keep_status = 1u;
    g_mock.never_ready = 1u;
    FF_TEST_ASSERT(g_drv.dev.read(0u, buf, 1u) == FF_TIMEOUT);
    FF_TEST_ASSERT(g_mock.err_status == 0u);
}

static void test_error_status(void)
{
    uint8_t buf[MOCK_BLOCK_SIZE];

    mock_bind();
    g_mock.error_status = 1u; /* an error bit is set in the status register */
    FF_TEST_ASSERT(g_drv.dev.read(0u, buf, 1u) == FF_ERROR);
    FF_TEST_ASSERT(g_drv.dev.write(0u, buf, 1u) == FF_ERROR);
    /* The driver cleared the whole status word on the error path. */
    FF_TEST_ASSERT(g_mock.err_status == 0u);
}

static void test_flush(void)
{
    mock_bind();
    /* No write-back cache: flush() is a no-op that always succeeds. */
    FF_TEST_ASSERT(g_drv.dev.flush() == FF_SUCCESS);
}

static void test_self_test(void)
{
    /* static: Setup() binds the instance into the module-global g_block. */
    static FF_BlockMpfs_t drv;

    mock_reset(&g_mock);
    mock_bus_init();
    FF_TEST_ASSERT(FF_BlockMpfsSelfTest(NULL) == FF_INVALID_ARGUMENT);
    FF_TEST_ASSERT(FF_BlockMpfsSetup(&drv, &g_bus, MOCK_BLOCK_SIZE, MOCK_NUM_BLOCKS) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_BlockMpfsSelfTest(&drv) == FF_SUCCESS);
    /* The self-test leaves BlockSizeBlockCount / Argument1 cleared. */
    FF_TEST_ASSERT(g_mock.block_size_count == 0u);
    FF_TEST_ASSERT(g_mock.argument1 == 0u);

    /* Capabilities1 without 3.3 V support fails the self-test. */
    g_mock.cap1 = FF_MMC_CAP1_SDMA;
    FF_TEST_ASSERT(FF_BlockMpfsSelfTest(&drv) == FF_ERROR);
    g_mock.cap1 = FF_MMC_CAP1_SDMA | FF_MMC_CAP1_VOLTAGE_3V3;
    /* HostControl2 without the stable-clock bit fails the self-test. */
    g_mock.hc2 = 0u;
    FF_TEST_ASSERT(FF_BlockMpfsSelfTest(&drv) == FF_ERROR);
    g_mock.hc2 = FF_MMC_HC2_CLOCK_STABLE;

    /* A self-test on an unbound instance is rejected. */
    {
        FF_BlockMpfs_t other;
        (void)memset(&other, 0, sizeof(other));
        FF_TEST_ASSERT(FF_BlockMpfsSelfTest(&other) == FF_INVALID_ARGUMENT);
    }
}

FF_TEST_MAIN(test_setup_and_validate, test_not_ready, test_read_single_block,
             test_read_multiple_blocks, test_write_single_block, test_write_multiple_blocks,
             test_write_read_roundtrip, test_args_validation, test_timeout,
             test_timeout_clears_stale_status, test_error_status, test_flush, test_self_test)
