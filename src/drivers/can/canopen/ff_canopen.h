/*
 * frick-fsw - CANopen application layer reference stack (PolarFire SoC), M4/S4c
 *
 * A small, allocation-free reference implementation of the CANopen application
 * layer subset (CiA 301 NMT receive/state, heartbeat, EMCY, expedited SDO
 * server, PDO transmit) over a binding of the HAL CAN backend (FF_CanBackend_t,
 * SRS-REQ-063). The object dictionary contains selected CiA 402 drive objects.
 * This is not a complete CiA 301/302/402 implementation: remote SDO client,
 * PDO mapping, SDO segmented/block transfers and CiA 402 motion/profile logic
 * are out of scope and tracked as gaps.
 *
 * The NMT command and state codes follow CiA 301:
 *   master command -> node state: 0x01 Operational, 0x02 Stopped,
 *   0x80 Pre-operational. Reset-node/reset-communication commands are rejected
 *   because platform reset sequencing is outside this reference subset.
 *   node-reported state (in the heartbeat second byte): 0x00 Boot-up/
 *   Initialising, 0x04 Stopped, 0x05 Operational, 0x7F Pre-operational.
 *
 * A CANopen node owns one CAN backend; the single-instance binding mirrors the
 * HAL contracts (no instance context). Multiple nodes need multiple instances
 * (GAP-A5).
 *
 * Public API calls are serialized with a non-blocking internal guard. If an RX
 * callback arrives while an API call is in progress (or another callback owns
 * the guard), that frame is dropped rather than spinning in interrupt context;
 * the drop counter below records this case. Callers must still handle FF_BUSY
 * and the heartbeat-state contention sentinel (0xFF). This is not a substitute
 * for platform-specific ISR/atomic verification or CAN receive backpressure.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef FF_CANOPEN_H
#define FF_CANOPEN_H

#include "common/ff_types.h"
#include "hal/hal_can.h"

/* NMT node states (CiA 301, the byte reported in the heartbeat second byte /
 * boot-up). Kept as a small enum so the state machine is testable directly. */
typedef enum {
    FF_CANOPEN_STATE_INIT = 0x00u,        /* boot-up / Initialising */
    FF_CANOPEN_STATE_STOPPED = 0x04u,     /* Stopped                  */
    FF_CANOPEN_STATE_OPERATIONAL = 0x05u, /* Operational              */
    FF_CANOPEN_STATE_PREOP = 0x7Fu        /* Pre-operational          */
} FF_CanOpenState_t;

/* NMT master command codes (the first byte of an NMT message). */
#define FF_CANOPEN_NMT_OPERATIONAL 0x01u
#define FF_CANOPEN_NMT_STOPPED 0x02u
#define FF_CANOPEN_NMT_PREOP 0x80u
/* Node/communication reset commands (0x81/0x82) are intentionally unsupported:
 * a reset requires application/platform reset sequencing outside this subset. */
#define FF_CANOPEN_NMT_RESET_NODE 0x81u
#define FF_CANOPEN_NMT_RESET_COMM 0x82u

/* Heart-beat consumer/producer period in milliseconds; 0 disables it. */
#define FF_CANOPEN_HEARTBEAT_PERIOD_MS 1000u

/* --- object dictionary ------------------------------------------------------ */

/* Length, in bytes, of an object (1/2/4): only the expedited SDO set is
 * supported (GAP-A5: SDO block/strided transfers are out of scope). */
typedef uint8_t FF_ODLen_t;
typedef uint16_t FF_ODIndex_t;
typedef uint8_t FF_ODSub_t;

#define FF_OD_MAX_RECORDS 16u /* bounded reference dictionary */
#define FF_OD_MAX_LEN 4u      /* maximum length of a single object (bytes) */

/* A dictionary entry is a (de)serialised reference: the value storage is
 * caller-provided (the integrator maps a real variable into an OD entry), so
 * the stack owns no memory and the reference OD is built from plain data. */
typedef struct
{
    FF_ODIndex_t index;
    FF_ODSub_t sub;   /* sub-index; 0x00 for a simple 32-bit object     */
    FF_ODLen_t len;   /* 1, 2 or 4                                      */
    uint8_t readonly; /* 1: SDO write refuses this entry                  */
    uint8_t *value;   /* caller-owned storage (NOT copied; must outlive)   */
} FF_ODRecord_t;

typedef struct
{
    const FF_ODRecord_t *records; /* bounded array, not copied           */
    uint8_t count;                /* number of entries in `records`      */
} FF_CanOpenOD_t;

/* --- stack instance --------------------------------------------------------- */
/* CiA 402 drive-profile object indexes used by the reference dictionary. */
#define FF_CANOPEN_CI402_CONTROL_WORD 0x6040u
#define FF_CANOPEN_CI402_STATUS_WORD 0x6041u
#define FF_CANOPEN_CI402_PROFILE_VELOCITY 0x6081u
#define FF_CANOPEN_CI402_PROFILE_ACCEL 0x6083u
/* 402 trajectory generation (op. mode, target pos/vel/torque) is out of scope (GAP-A5). */

typedef struct
{
    FF_CanOpenStack_t open;  /* public HAL interface (must be first) */
    FF_CanBackend_t *can;    /* bound CAN backend                    */
    uint8_t node_id;         /* this node's CANopen node id (1..127) */
    FF_CanOpenState_t state; /* current NMT state                    */
    uint32_t guard_time_ms;  /* heartbeat producer period, ms        */
    uint32_t heartbeat_slot; /* last slot successfully transmitted    */
    FF_CanOpenOD_t od;       /* reference object dictionary          */
    uint16_t nmt_commands;   /* diagnostics: NMT commands processed  */
    uint16_t sdo_write;      /* diagnostics: accepted SDO writes     */
    uint16_t sdo_read;       /* diagnostics: accepted SDO reads      */
    uint16_t pdo_events;     /* diagnostics: PDO events sent         */
    uint16_t nmt_errors;     /* diagnostics: invalid NMT frames      */
    uint16_t sdo_errors;     /* diagnostics: refused SDO transfers   */
    uint8_t ready;           /* bound CAN backend init() completed   */
    uint8_t heartbeat_sent;  /* at least one heartbeat was sent      */
} FF_CanopenMpfs_t;

/* Bind the instance to a CAN backend + node id + reference dictionary.
 * Rejects NULL/incomplete arguments, a node id outside 1..127, a dictionary with
 * a NULL record array when count != 0, or an entry whose length is not in
 * {1,2,4} or whose storage pointer is NULL. */
FF_Status_t FF_CanopenMpfsSetup(FF_CanopenMpfs_t *n, FF_CanBackend_t *can, uint8_t node_id,
                                const FF_CanOpenOD_t *od);

/* NMT master command -> state transition (CiA 301). Invalid commands, or a
 * command that does not apply to the current state, is refused and counted;
 * the state machine is idempotent for a no-op command. */
FF_Status_t FF_CanOpenNmtCommand(FF_CanopenMpfs_t *n, uint8_t command);

/* Heartbeat producer: returns the state byte and a due flag. The process
 * function transmits on 0x700+node-id when due, based on the supplied ms clock. */
uint8_t FF_CanOpenHeartbeatState(const FF_CanopenMpfs_t *n, uint32_t now_ms, uint8_t *due);
FF_Status_t FF_CanOpenHeartbeatProcess(FF_CanopenMpfs_t *n, uint32_t now_ms);

/* SDO expedited local-object helpers; `node` must equal the bound node id
 * (remote-node client transfers are not implemented, GAP-A5). Wire requests
 * received on 0x600+node-id are handled by the registered CAN receive callback.
 * `data` length is one of {1,2,4}; object bytes are little-endian. */
FF_Status_t FF_CanOpenSdoWrite(uint8_t node, uint16_t index, uint8_t sub, const uint8_t *data,
                               uint8_t len);
FF_Status_t FF_CanOpenSdoRead(uint8_t node, uint16_t index, uint8_t sub, uint8_t *data,
                              uint8_t *len);

/* Emergency object (CiA 301 1003h-style): an 8-byte CAN frame is emitted with
 * the supplied error register + data; the CAN backend is only asked to send
 * when it is not in the Stopped state. */
FF_Status_t FF_CanOpenEmergency(FF_CanopenMpfs_t *n, uint16_t error_code, uint8_t err_reg,
                                const uint8_t *data, uint8_t len);

/* PDO: send `len` (1..8) bytes on the mapped COB-ID. Refused in the Stopped
 * state; the frame is handed to the backend's transmit(). The instance is the
 * single bound binding. */
FF_Status_t FF_CanOpenPdoSend(uint8_t node, uint16_t cob_id, const uint8_t *data, uint8_t len);

/* Number of RX callbacks dropped because the stack serialization guard was
 * already held. Monotonic modulo 2^32; intended for diagnostics. */
uint32_t FF_CanOpenRxContentionDrops(void);

/* Reference dictionary of the selected CiA 402 objects. */
extern const FF_CanOpenOD_t ff_canopen_reference_od;

#endif /* FF_CANOPEN_H */
