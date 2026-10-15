/*
 * frick-fsw - SpaceWire driver (contract-level backend, milestone M4/M11)
 *
 * Implements the HAL SpaceWire contract (FF_SpaceWire_t) and RMAP contract
 * (FF_Rmap_t) for ECSS-E-ST-50-12/51/52/53 compliance at the API level.
 * There is no emulated SpaceWire peripheral in the Renode `beaglev-fire`
 * repl (GAP-A5), so this backend provides a **contract-level implementation
 * only**: a deterministic, bounded mock that exercises the API surface and
 * verifies the RMAP protocol state machine without binding to hardware.
 *
 * This is analogous to the network loopback check (hal_net=CONTRACT) where
 * the FF_NetStack_t contract is exercised in-process without an emulated
 * peripheral. The SpaceWire check in the Renode functional test emits
 * `[renode-test] hal_spacewire=CONTRACT`.
 *
 * Design:
 * - FF_SpaceWire_t: send/send_pid with protocol ID, receive callback.
 * - FF_Rmap_t: read/write/rmw with target logical address, key, reply address.
 * - All buffers are caller-owned, bounded, no allocation.
 * - The implementation tracks sequence numbers, keys, and reply addresses
 *   per ECSS-E-ST-50-52; it validates RMAP command/reply formats.
 * - No hardware timing, link-layer, or physical-layer behaviour is modelled.
 *
 * Known limitations (all bounded, allocation-free):
 *   - No emulated peripheral (GAP-A5): this is a contract-level mock only.
 *   - Single logical address / target supported (static instance).
 *   - No link-layer error injection (disconnect, parity, ESC errors).
 *   - No timeouts or retry logic (the caller sequences transactions).
 *   - RMAP buffer size capped at FF_SPW_MAX_PAYLOAD (256 bytes per ECSS).
 *   - Not re-entrant: single static instance, caller must serialise.
 *   - The MMIO self-test concept does not apply; a contract self-test
 *     (FF_SpaceWireMpfsSelfTest) exercises the API and RMAP state machine.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_SPACEWIRE_MPFS_H
#define FF_SPACEWIRE_MPFS_H

#include "common/ff_types.h"
#include "hal/hal_spacewire.h"

/* ECSS-E-ST-50-12/51/52/53 constants. */
#define FF_SPW_PROTOCOL_ID_CCSDS 0x01u                 /* ECSS-E-ST-50-53: CCSDS packet transfer */
#define FF_SPW_PROTOCOL_ID_RMAP 0x02u                  /* ECSS-E-ST-50-52: RMAP */
#define FF_SPW_MAX_PAYLOAD 256u                        /* Max data payload per ECSS-E-ST-50-52 */
#define FF_SPW_MAX_RMAP_LEN (FF_SPW_MAX_PAYLOAD + 16u) /* Header + data + CRC */

/* RMAP command codes (ECSS-E-ST-50-52 Table 5-1). */
#define FF_RMAP_CMD_READ 0x01u
#define FF_RMAP_CMD_WRITE 0x02u
#define FF_RMAP_CMD_RMW 0x03u

/* RMAP reply status codes (ECSS-E-ST-50-52 Table 5-4). */
#define FF_RMAP_REPLY_OK 0x00u
#define FF_RMAP_REPLY_INVALID_CMD 0x01u
#define FF_RMAP_REPLY_UNSUPPORTED 0x02u
#define FF_RMAP_REPLY_KEY_ERROR 0x03u
#define FF_RMAP_REPLY_ADDR_ERROR 0x04u
#define FF_RMAP_REPLY_CRC_ERROR 0x05u
#define FF_RMAP_REPLY_EEP 0x06u /* Early EOP */
#define FF_RMAP_REPLY_ELP 0x07u /* Early LP */
#define FF_RMAP_REPLY_TOO_MUCH_DATA 0x08u
#define FF_RMAP_REPLY_VERIFY_ERROR 0x09u
#define FF_RMAP_REPLY_RMW_ERROR 0x0Au
#define FF_RMAP_REPLY_AUTH_ERROR 0x0Bu

/* RMAP header size (bytes): Protocol ID (1) + Header CRC (2, split MSB/LSB) +
 * Command (1) + Target Logical Addr (1) + Key (1) + Reply Addr (1) +
 * Transaction ID (2) + Address (4) + Data Length (3, 24-bit) = 16 bytes. */
#define FF_RMAP_HEADER_SIZE 16u

/* Contract-level mock bus: the platform injects a "link" that the driver uses
 * to simulate send/receive. In the real system this would be a hardware FIFO
 * or DMA descriptor ring. Here it is a simple function-pointer interface for
 * the contract-level check. */
typedef struct
{
    void *ctx;
    FF_Status_t (*send)(void *ctx, const uint8_t *data, uint16_t len);
    FF_Status_t (*recv)(void *ctx, uint8_t *data, uint16_t *len);
} FF_SpwMockBus_t;

typedef struct
{
    FF_SpaceWire_t spw;   /* public HAL interface (must be first) */
    FF_Rmap_t rmap;       /* public RMAP interface */
    FF_SpwMockBus_t bus;  /* injected mock link */
    uint8_t logical_addr; /* this node's logical address */
    uint8_t target_addr;  /* default RMAP target */
    uint8_t key;          /* RMAP key */
    uint8_t reply_addr;   /* RMAP reply address */
    uint16_t tx_tid;      /* transmit transaction ID counter */
    uint16_t rx_tid;      /* receive transaction ID (for replies) */
    uint8_t rx_cb_set;    /* callback registered */
    FF_SpwRxCb_t rx_cb;   /* receive callback */
    void *rx_cb_ctx;      /* callback context */
    uint8_t ready;        /* init() completed */
} FF_SpaceWireMpfs_t;

/* Bind the instance and complete the FF_SpaceWire_t and FF_Rmap_t vtables.
 * Returns FF_INVALID_ARGUMENT if s, bus, or either bus hook is NULL. */
FF_Status_t FF_SpaceWireMpfsSetup(FF_SpaceWireMpfs_t *s, const FF_SpwMockBus_t *bus,
                                  uint8_t logical_addr);

/* Contract-level self-test: exercises the send/receive/RMAP API surface and
 * verifies the RMAP command/reply format, sequence numbers, keys, and CRC.
 * Used by the host unit test and the Renode functional check (contract-level
 * only, no emulated peripheral).
 *
 * Returns FF_SUCCESS if all API contracts are satisfied. */
FF_Status_t FF_SpaceWireMpfsSelfTest(FF_SpaceWireMpfs_t *s);

#endif /* FF_SPACEWIRE_MPFS_H */