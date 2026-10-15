/*
 * frick-fsw - CANopen application layer reference stack (PolarFire SoC), M4/S4c
 *
 * CiA 301 application layer subset (NMT receive/state, heartbeat, expedited
 * SDO server, emergency and PDO transmit) over a bound
 * HAL CAN backend (FF_CanBackend_t, SRS-REQ-063). Allocation-free, static single
 * instance (SRS-REQ-015 / norm N10). See ff_canopen.h for the contract, the NMT
 * codes and the documented limitations (GAP-A5).
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "drivers/can/canopen/ff_canopen.h"

static FF_CanopenMpfs_t *g_canopen; /* single bound instance */
/* Non-blocking serialization guard: if an RX callback preempts an API call, it
 * must not spin in the ISR or observe partially updated state. GCC/Clang
 * __atomic builtins are available in both the host and RTEMS toolchains. */
static uint8_t g_canopen_lock;
static uint32_t g_canopen_rx_contention_drops;
static void canopen_receive(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len, void *ctx);
static FF_Status_t canopen_nmt_command_locked(FF_CanopenMpfs_t *n, uint8_t command);

static uint8_t canopen_try_lock(void)
{
    return (uint8_t)!__atomic_test_and_set(&g_canopen_lock, __ATOMIC_ACQUIRE);
}

static void canopen_unlock(void)
{
    __atomic_clear(&g_canopen_lock, __ATOMIC_RELEASE);
}

static FF_Status_t canopen_contract_init_locked(FF_CanBackend_t *can, uint8_t node_id)
{
    FF_Status_t rc;

    if ((g_canopen == NULL) || (can == NULL) || (can != g_canopen->can) ||
        (node_id != g_canopen->node_id)) {
        return FF_INVALID_ARGUMENT;
    }
    rc = can->init();
    if (rc != FF_SUCCESS) {
        return rc;
    }
    if (can->set_rx_callback == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    can->set_rx_callback(canopen_receive, g_canopen);
    g_canopen->state = FF_CANOPEN_STATE_PREOP;
    g_canopen->heartbeat_sent = 0u;
    g_canopen->ready = 1u;
    return FF_SUCCESS;
}

static FF_Status_t canopen_contract_init(FF_CanBackend_t *can, uint8_t node_id)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_contract_init_locked(can, node_id);
    canopen_unlock();
    return rc;
}

/* Selected CiA 402 object dictionary contents. Values are static, owned by
 * the integrator through the binding; 402 trajectory/op-mode objects are
 * unmapped here (GAP-A5). */
static uint16_t ff_od_status_word;   /* 6041 status word */
static uint16_t ff_od_control_word;  /* 6040 control word */
static uint32_t ff_od_profile_accel; /* 6083 profile acceleration */
static uint32_t ff_od_profile_vel;   /* 6081 profile velocity */

static FF_ODRecord_t ff_canopen_reference_od_records[] = {
    {FF_CANOPEN_CI402_STATUS_WORD, 0x00u, 2u, 1u, (uint8_t *)&ff_od_status_word},   /* read-only */
    {FF_CANOPEN_CI402_CONTROL_WORD, 0x00u, 2u, 0u, (uint8_t *)&ff_od_control_word}, /* writable */
    {FF_CANOPEN_CI402_PROFILE_ACCEL, 0x00u, 4u, 0u, (uint8_t *)&ff_od_profile_accel},
    {FF_CANOPEN_CI402_PROFILE_VELOCITY, 0x00u, 4u, 0u, (uint8_t *)&ff_od_profile_vel},
};

const FF_CanOpenOD_t ff_canopen_reference_od = {
    .records = ff_canopen_reference_od_records,
    .count = (uint8_t)(sizeof(ff_canopen_reference_od_records) /
                       sizeof(ff_canopen_reference_od_records[0])),
};

/* Locate an object by (index, sub). Returns the entry or NULL. */
static const FF_ODRecord_t *od_find(const FF_CanOpenOD_t *od, uint16_t index, uint8_t sub)
{
    uint8_t i;

    if ((od == NULL) || (od->records == NULL)) {
        return NULL;
    }
    for (i = 0u; i < od->count; i++) {
        if ((od->records[i].index == index) && (od->records[i].sub == sub)) {
            return &od->records[i];
        }
    }
    return NULL;
}

static uint8_t od_has_index(const FF_CanOpenOD_t *od, uint16_t index)
{
    uint8_t i;

    if ((od == NULL) || (od->records == NULL)) {
        return 0u;
    }
    for (i = 0u; i < od->count; i++) {
        if (od->records[i].index == index) {
            return 1u;
        }
    }
    return 0u;
}

static void sdo_abort(FF_CanopenMpfs_t *n, uint16_t index, uint8_t sub, uint32_t code)
{
    uint8_t frame[8] = {0x80u,
                        (uint8_t)(index & 0xFFu),
                        (uint8_t)(index >> 8),
                        sub,
                        (uint8_t)(code & 0xFFu),
                        (uint8_t)((code >> 8) & 0xFFu),
                        (uint8_t)((code >> 16) & 0xFFu),
                        (uint8_t)(code >> 24)};

    n->sdo_errors++;
    (void)n->can->transmit(0x580u + n->node_id, 0u, frame, 8u);
}

/* Receive-side CiA 301 subset: NMT commands and expedited SDO server requests.
 * Unsupported frames/commands are ignored; work and frame sizes are bounded. */
static void canopen_receive_locked(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len,
                                   void *ctx)
{
    FF_CanopenMpfs_t *n = (FF_CanopenMpfs_t *)ctx;
    const FF_ODRecord_t *r;
    uint16_t index;
    uint8_t sub;
    uint8_t width;
    uint8_t frame[8] = {0u};
    uint8_t i;

    if ((n == NULL) || (g_canopen != n) || (n->ready == 0u) || (ext != 0u) || (data == NULL)) {
        return;
    }
    if ((id == 0u) && (len == 2u) && ((data[1] == 0u) || (data[1] == n->node_id))) {
        (void)canopen_nmt_command_locked(n, data[0]);
        return;
    }
    if ((id != (0x600u + n->node_id)) || (len != 8u)) {
        return;
    }
    if ((n->state != FF_CANOPEN_STATE_PREOP) && (n->state != FF_CANOPEN_STATE_OPERATIONAL)) {
        return; /* SDO server requests are not serviced while Stopped/Initialising */
    }
    index = (uint16_t)((uint16_t)data[1] | ((uint16_t)data[2] << 8));
    sub = data[3];
    r = od_find(&n->od, index, sub);
    if (data[0] == 0x40u) { /* expedited upload request */
        if (r == NULL) {
            sdo_abort(n, index, sub,
                      (od_has_index(&n->od, index) != 0u) ? 0x06090011u : 0x06020000u);
            return;
        }
        width = r->len;
        frame[0] = (width == 1u) ? 0x4Fu : ((width == 2u) ? 0x4Bu : 0x43u);
        frame[1] = data[1];
        frame[2] = data[2];
        frame[3] = sub;
        for (i = 0u; i < width; i++) {
            frame[4u + i] = r->value[i];
        }
        if (n->can->transmit(0x580u + n->node_id, 0u, frame, 8u) == FF_SUCCESS) {
            n->sdo_read++;
        }
        return;
    }
    if ((data[0] == 0x2Fu) || (data[0] == 0x2Bu) || (data[0] == 0x23u)) {
        width = (data[0] == 0x2Fu) ? 1u : ((data[0] == 0x2Bu) ? 2u : 4u);
        if (r == NULL) {
            sdo_abort(n, index, sub,
                      (od_has_index(&n->od, index) != 0u) ? 0x06090011u : 0x06020000u);
            return;
        }
        if (r->readonly != 0u) {
            sdo_abort(n, index, sub, 0x06010002u);
            return;
        }
        if (r->len != width) {
            sdo_abort(n, index, sub, 0x06070010u);
            return;
        }
        {
            uint8_t old_value[4];
            FF_Status_t tx_status;

            for (i = 0u; i < width; i++) {
                old_value[i] = r->value[i];
                r->value[i] = data[4u + i];
            }
            frame[0] = 0x60u;
            frame[1] = data[1];
            frame[2] = data[2];
            frame[3] = sub;
            tx_status = n->can->transmit(0x580u + n->node_id, 0u, frame, 8u);
            if (tx_status == FF_SUCCESS) {
                n->sdo_write++;
            }
            else {
                /* Backend transmit failure means the client did not receive an
                 * accepted response; roll back before returning to the owner. */
                for (i = 0u; i < width; i++) {
                    r->value[i] = old_value[i];
                }
                n->sdo_errors++;
            }
        }
        return;
    }
    sdo_abort(n, index, sub, 0x05040001u);
}

static void canopen_receive(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len, void *ctx)
{
    if (canopen_try_lock() == 0u) {
        (void)__atomic_fetch_add(&g_canopen_rx_contention_drops, 1u, __ATOMIC_RELAXED);
        return;
    }
    canopen_receive_locked(id, ext, data, len, ctx);
    canopen_unlock();
}

/* Read a little-endian value of `len` bytes from `value` into a uint32. */
static uint32_t od_unpack(const uint8_t *value, uint8_t len)
{
    uint32_t v = 0u;
    uint8_t i;

    for (i = 0u; i < len; i++) {
        v |= ((uint32_t)value[i]) << (8u * i);
    }
    return v;
}

/* Write `len` bytes of a uint32 to `value`, little-endian. */
static void od_pack(uint32_t v, uint8_t *value, uint8_t len)
{
    uint8_t i;

    for (i = 0u; i < len; i++) {
        value[i] = (uint8_t)((v >> (8u * i)) & 0xFFu);
    }
}

static FF_Status_t canopen_setup_locked(FF_CanopenMpfs_t *n, FF_CanBackend_t *can, uint8_t node_id,
                                        const FF_CanOpenOD_t *od)
{
    uint8_t i;
    uint8_t j;
    const FF_ODRecord_t *r;

    if ((n == NULL) || (can == NULL) || (od == NULL) || (od->count == 0u) ||
        (od->count > FF_OD_MAX_RECORDS) || (od->records == NULL) || (node_id == 0u) ||
        (node_id > 127u)) {
        return FF_INVALID_ARGUMENT;
    }
    if ((can->init == NULL) || (can->transmit == NULL) || (can->configure == NULL) ||
        (can->set_rx_callback == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    /* Validate the whole dictionary before binding, so a rejected Setup never
     * leaves a half-bound stack behind. */
    for (i = 0u; i < od->count; i++) {
        r = &od->records[i];
        if ((r->len != 1u) && (r->len != 2u) && (r->len != 4u)) {
            return FF_BAD_SIZE; /* expedited SDO only supports 1/2/4-byte objects */
        }
        if (r->value == NULL) {
            return FF_INVALID_ARGUMENT;
        }
        for (j = (uint8_t)(i + 1u); j < od->count; j++) {
            if ((od->records[j].index == r->index) && (od->records[j].sub == r->sub)) {
                return FF_ALREADY_EXISTS;
            }
        }
    }
    n->can = can;
    n->node_id = node_id;
    n->state = FF_CANOPEN_STATE_INIT;
    n->od = *od;
    n->guard_time_ms = FF_CANOPEN_HEARTBEAT_PERIOD_MS;
    n->heartbeat_slot = 0u;
    n->heartbeat_sent = 0u;
    n->nmt_commands = 0u;
    n->sdo_write = 0u;
    n->sdo_read = 0u;
    n->pdo_events = 0u;
    n->nmt_errors = 0u;
    n->sdo_errors = 0u;
    n->ready = 0u;
    n->open.name = "canopen-mpfs";
    n->open.init = canopen_contract_init;
    n->open.sdo_write = FF_CanOpenSdoWrite;
    n->open.sdo_read = FF_CanOpenSdoRead;
    n->open.pdo_send = FF_CanOpenPdoSend;
    g_canopen = n;
    return FF_SUCCESS;
}

FF_Status_t FF_CanopenMpfsSetup(FF_CanopenMpfs_t *n, FF_CanBackend_t *can, uint8_t node_id,
                                const FF_CanOpenOD_t *od)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_setup_locked(n, can, node_id, od);
    canopen_unlock();
    return rc;
}

/* NMT master command -> node state transition (CiA 301). */
static FF_Status_t canopen_nmt_command_locked(FF_CanopenMpfs_t *n, uint8_t command)
{
    FF_CanOpenState_t next;

    if ((n == NULL) || (g_canopen != n)) {
        return FF_INVALID_ARGUMENT;
    }
    if (n->ready == 0u) {
        return FF_NOT_READY;
    }
    switch (command) {
    case FF_CANOPEN_NMT_OPERATIONAL:
        next = FF_CANOPEN_STATE_OPERATIONAL;
        break;
    case FF_CANOPEN_NMT_STOPPED:
        next = FF_CANOPEN_STATE_STOPPED;
        break;
    case FF_CANOPEN_NMT_PREOP:
        next = FF_CANOPEN_STATE_PREOP;
        break;
    default:
        n->nmt_errors++;
        return FF_INVALID_ARGUMENT;
    }
    n->state = next;
    n->nmt_commands++;
    return FF_SUCCESS;
}

FF_Status_t FF_CanOpenNmtCommand(FF_CanopenMpfs_t *n, uint8_t command)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_nmt_command_locked(n, command);
    canopen_unlock();
    return rc;
}

/* Heartbeat producer: returns the state code the node would transmit now. The
 * `due` flag is set when `now_ms` lands on a heartbeat slot (period =
 * guard_time_ms, or never when the period is 0). */
static uint8_t canopen_heartbeat_state_locked(const FF_CanopenMpfs_t *n, uint32_t now_ms,
                                              uint8_t *due)
{
    uint32_t period;
    uint32_t elapsed;

    if ((n == NULL) || (g_canopen == NULL) || (g_canopen != n) || (n->ready == 0u)) {
        if (due != NULL) {
            *due = 0u;
        }
        return 0x00u;
    }
    period = n->guard_time_ms;
    if (period == 0u) {
        if (due != NULL) {
            *due = 0u; /* disabled: no heartbeat is ever due */
        }
        return (uint8_t)n->state;
    }
    elapsed = now_ms % period;
    if (due != NULL) {
        *due = (uint8_t)(elapsed == 0u);
    }
    return (uint8_t)n->state;
}

uint8_t FF_CanOpenHeartbeatState(const FF_CanopenMpfs_t *n, uint32_t now_ms, uint8_t *due)
{
    uint8_t state;

    if (canopen_try_lock() == 0u) {
        if (due != NULL) {
            *due = 0u;
        }
        return 0xFFu; /* outside every defined CANopen node state */
    }
    state = canopen_heartbeat_state_locked(n, now_ms, due);
    canopen_unlock();
    return state;
}

static FF_Status_t canopen_heartbeat_process_locked(FF_CanopenMpfs_t *n, uint32_t now_ms)
{
    uint8_t due = 0u;
    uint8_t state;
    uint32_t slot;
    FF_Status_t rc;

    if ((n == NULL) || (g_canopen != n)) {
        return FF_INVALID_ARGUMENT;
    }
    state = canopen_heartbeat_state_locked(n, now_ms, &due);
    if (n->ready == 0u) {
        return FF_NOT_READY;
    }
    if (due == 0u) {
        return FF_SUCCESS;
    }
    slot = (n->guard_time_ms == 0u) ? 0u : now_ms / n->guard_time_ms;
    if ((n->heartbeat_sent != 0u) && (slot == n->heartbeat_slot)) {
        return FF_SUCCESS; /* repeated scheduler invocation in the same slot */
    }
    /* Heartbeat producer COB-ID is 0x700 + node-id; payload is one state byte. */
    rc = n->can->transmit(0x700u + (uint32_t)n->node_id, 0u, &state, 1u);
    if (rc == FF_SUCCESS) {
        n->heartbeat_slot = slot;
        n->heartbeat_sent = 1u;
    }
    return rc;
}

FF_Status_t FF_CanOpenHeartbeatProcess(FF_CanopenMpfs_t *n, uint32_t now_ms)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_heartbeat_process_locked(n, now_ms);
    canopen_unlock();
    return rc;
}

/* Emit a CAN frame via the bound backend (used by PDO; SDO is local to the OD).
 * No traffic is emitted in the Stopped state (CiA 301). */
static FF_Status_t canopen_send(uint32_t id, uint8_t ext, const uint8_t *data, uint8_t len)
{
    if ((g_canopen == NULL) || (g_canopen->can == NULL) || (g_canopen->ready == 0u)) {
        return FF_NOT_READY;
    }
    if (g_canopen->state == FF_CANOPEN_STATE_STOPPED) {
        return FF_NOT_READY;
    }
    return g_canopen->can->transmit(id, ext, data, len);
}

static FF_Status_t canopen_sdo_write_locked(uint8_t node, uint16_t index, uint8_t sub,
                                            const uint8_t *data, uint8_t len)
{
    const FF_ODRecord_t *r;
    uint8_t n;
    uint32_t v;

    if ((g_canopen == NULL) || (g_canopen->ready == 0u) || (data == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (node != g_canopen->node_id) {
        return FF_NOT_FOUND; /* this reference exposes only its local node OD */
    }
    if ((len != 1u) && (len != 2u) && (len != 4u)) {
        return FF_BAD_SIZE;
    }
    r = od_find(&g_canopen->od, index, sub);
    if (r == NULL) {
        g_canopen->sdo_errors++;
        return FF_NOT_FOUND;
    }
    if (r->readonly) {
        g_canopen->sdo_errors++;
        return FF_NOT_READY; /* entry is read-only */
    }
    if (len != r->len) {
        g_canopen->sdo_errors++;
        return FF_BAD_SIZE; /* expedited transfer size must match the OD entry */
    }
    n = r->len;
    v = od_unpack(data, len);
    od_pack(v, r->value, n);
    g_canopen->sdo_write++;
    return FF_SUCCESS;
}

FF_Status_t FF_CanOpenSdoWrite(uint8_t node, uint16_t index, uint8_t sub, const uint8_t *data,
                               uint8_t len)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_sdo_write_locked(node, index, sub, data, len);
    canopen_unlock();
    return rc;
}

static FF_Status_t canopen_sdo_read_locked(uint8_t node, uint16_t index, uint8_t sub, uint8_t *data,
                                           uint8_t *len)
{
    const FF_ODRecord_t *r;
    uint8_t n;
    uint32_t v;

    if ((g_canopen == NULL) || (g_canopen->ready == 0u) || (data == NULL) || (len == NULL)) {
        return FF_INVALID_ARGUMENT;
    }
    if (node != g_canopen->node_id) {
        return FF_NOT_FOUND;
    }
    r = od_find(&g_canopen->od, index, sub);
    if (r == NULL) {
        g_canopen->sdo_errors++;
        return FF_NOT_FOUND;
    }
    v = od_unpack(r->value, r->len);
    n = (*len < r->len) ? (uint8_t)*len : r->len; /* never read past the object */
    od_pack(v, data, n);
    *len = n;
    g_canopen->sdo_read++;
    return FF_SUCCESS;
}

FF_Status_t FF_CanOpenSdoRead(uint8_t node, uint16_t index, uint8_t sub, uint8_t *data,
                              uint8_t *len)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_sdo_read_locked(node, index, sub, data, len);
    canopen_unlock();
    return rc;
}

static FF_Status_t canopen_emergency_locked(FF_CanopenMpfs_t *n, uint16_t error_code,
                                            uint8_t err_reg, const uint8_t *data, uint8_t len)
{
    uint8_t frame[8] = {0u};
    uint8_t i;

    if ((n == NULL) || (g_canopen != n)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len > 5u) {
        return FF_BAD_SIZE; /* EMCY: 2-byte error code + register + 5 manufacturer bytes */
    }
    if ((data == NULL) && (len > 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    frame[0] = (uint8_t)(error_code & 0xFFu);
    frame[1] = (uint8_t)(error_code >> 8);
    frame[2] = err_reg;
    for (i = 0u; i < len; i++) {
        frame[i + 3u] = data[i];
    }
    /* Route through canopen_send: no traffic in the Stopped state (CiA 301),
     * and the frame uses the node's emergency COB-ID (0x700 + node_id). */
    return canopen_send(0x080u + (uint32_t)n->node_id, 0u, frame, 8u);
}

FF_Status_t FF_CanOpenEmergency(FF_CanopenMpfs_t *n, uint16_t error_code, uint8_t err_reg,
                                const uint8_t *data, uint8_t len)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_emergency_locked(n, error_code, err_reg, data, len);
    canopen_unlock();
    return rc;
}

static FF_Status_t canopen_pdo_send_locked(uint8_t node, uint16_t cob_id, const uint8_t *data,
                                           uint8_t len)
{
    FF_Status_t rc;

    if (g_canopen == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if (node != g_canopen->node_id) {
        return FF_NOT_FOUND;
    }
    if ((data == NULL) || (len == 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len > 8u) {
        return FF_BAD_SIZE;
    }
    if (cob_id > 0x7FFu) {
        return FF_INVALID_ARGUMENT; /* this reference emits standard 11-bit COB-IDs */
    }
    if (g_canopen->state != FF_CANOPEN_STATE_OPERATIONAL) {
        return FF_NOT_READY; /* PDO transmission is enabled only in Operational */
    }
    rc = canopen_send((uint32_t)cob_id, 0u, data, len);
    if (rc == FF_SUCCESS) {
        g_canopen->pdo_events++;
    }
    return rc;
}

FF_Status_t FF_CanOpenPdoSend(uint8_t node, uint16_t cob_id, const uint8_t *data, uint8_t len)
{
    FF_Status_t rc;

    if (canopen_try_lock() == 0u) {
        return FF_BUSY;
    }
    rc = canopen_pdo_send_locked(node, cob_id, data, len);
    canopen_unlock();
    return rc;
}

uint32_t FF_CanOpenRxContentionDrops(void)
{
    return __atomic_load_n(&g_canopen_rx_contention_drops, __ATOMIC_RELAXED);
}
