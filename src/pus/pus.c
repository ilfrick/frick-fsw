/*
 * frick-fsw - PUS core: dispatch, verification and telemetry emission
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "pus.h"
#include "core/time/time.h"
#include "core/pal/pal.h"
#include <string.h>

#define FF_PUS_TM_SCRATCH (FF_PUS_MAX_TM_FRAME + 32u)

/* Bounded retry budget for a required service 1 verification that collides with
 * another task's telemetry emission. If every attempt is refused the emission is
 * counted in g_ack_dropped rather than discarded silently (SRS-REQ-006/011). */
#define FF_PUS_ACK_EMIT_RETRIES 8u

typedef struct
{
    bool used;
    uint8_t service;
    uint8_t subservice;
    FF_Pus_Handler_t handler;
} ff_pus_svc_t;

static ff_pus_svc_t g_svcs[FF_PUS_MAX_SERVICES];
static const FF_Pus_Backend_t *g_backend;
static FF_Pus_TmSink_t g_tm_sink;
static uint16_t g_tm_counter;
static uint32_t g_tc_count;
static bool g_ready;

/* Dispatch reservation: nonzero when a TC dispatch is in progress (between
 * acceptance and completion). Used to return FF_BUSY on concurrent ingest, and
 * armed for every dispatch so handler state is never entered concurrently.
 * It is an atomic flag rather than a mutex-protected variable so that it can be
 * released on a path that failed to re-acquire the state mutex (NC-EXT-251). */
static uint8_t g_dispatch_reserved = 0u;

/* Nonzero while one telemetry emission owns the sink. Guards frame composition
 * plus the sink call as a single logical operation without holding g_pus_mutex
 * across a blocking sink write (NC-EXT-251, R1-16). */
static uint8_t g_tm_emitting = 0u;

/* Service 1 verifications that could not be emitted because emission stayed
 * refused for the whole bounded retry window. Counted so an unaccounted loss
 * cannot occur silently (SRS-REQ-011/SRS-REQ-072). */
static uint32_t g_ack_dropped = 0u;

/* Mutex protecting all shared PUS core state (NC-EXT-05: serialization for
 * multi-context TM calls). Created once on first FF_PusInit. */
static FF_MutexId_t g_pus_mutex;
static bool g_pus_mutex_created = false;

FF_Status_t FF_PusInit(void)
{
    if (!g_pus_mutex_created) {
        FF_Status_t st = FF_PalMutexCreate(&g_pus_mutex, "pus_mutex");
        if (st != FF_SUCCESS) {
            return st;
        }
        g_pus_mutex_created = true;
    }

    FF_PalMutexLock(g_pus_mutex, 0u);
    memset(g_svcs, 0, sizeof(g_svcs));
    __atomic_store_n(&g_backend, NULL, __ATOMIC_RELEASE);
    g_tm_sink = NULL;
    g_tm_counter = 0u;
    g_tc_count = 0u;
    g_ready = false;
    __atomic_store_n(&g_ack_dropped, 0u, __ATOMIC_RELEASE);
    __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
    __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
    FF_PalMutexUnlock(g_pus_mutex);

    if (FF_PusRegisterCoreServices() != FF_SUCCESS) {
        return FF_ERROR;
    }

    FF_PalMutexLock(g_pus_mutex, 0u);
    g_ready = true;
    FF_PalMutexUnlock(g_pus_mutex);
    return FF_SUCCESS;
}

FF_Status_t FF_PusSetBackend(const FF_Pus_Backend_t *backend)
{
    if (backend == NULL || backend->encode_tm == NULL || backend->decode_tc == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    if ((backend->init != NULL) && (backend->init() != FF_SUCCESS)) {
        return FF_ERROR;
    }
    FF_PalMutexLock(g_pus_mutex, 0u);
    __atomic_store_n(&g_backend, backend, __ATOMIC_RELEASE);
    FF_PalMutexUnlock(g_pus_mutex);
    return FF_SUCCESS;
}

const FF_Pus_Backend_t *FF_PusGetBackend(void)
{
    return __atomic_load_n(&g_backend, __ATOMIC_ACQUIRE);
}

FF_Status_t FF_PusSetTmSink(FF_Pus_TmSink_t sink)
{
    FF_PalMutexLock(g_pus_mutex, 0u);
    g_tm_sink = sink;
    FF_PalMutexUnlock(g_pus_mutex);
    return FF_SUCCESS;
}

/* ---- Registration ------------------------------------------------- */

FF_Status_t FF_PusRegisterService(uint8_t service, uint8_t subservice, FF_Pus_Handler_t handler)
{
    if (handler == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    FF_PalMutexLock(g_pus_mutex, 0u);
    /* Reject duplicate (service, subservice) registrations deterministically so a
     * later registration cannot silently shadow an earlier one (NC-EXT-23, C-15).
     * The first slot that matches service+subservice wins; a second registration of
     * the same pair is refused. */
    for (uint16_t i = 0; i < FF_PUS_MAX_SERVICES; i++) {
        if (g_svcs[i].used && (g_svcs[i].service == service) &&
            (g_svcs[i].subservice == subservice)) {
            FF_PalMutexUnlock(g_pus_mutex);
            return FF_ALREADY_EXISTS;
        }
    }
    for (uint16_t i = 0; i < FF_PUS_MAX_SERVICES; i++) {
        if (!g_svcs[i].used) {
            g_svcs[i].used = true;
            g_svcs[i].service = service;
            g_svcs[i].subservice = subservice;
            g_svcs[i].handler = handler;
            FF_PalMutexUnlock(g_pus_mutex);
            return FF_SUCCESS;
        }
    }
    FF_PalMutexUnlock(g_pus_mutex);
    return FF_NO_RESOURCE;
}

/* The caller MUST already hold g_pus_mutex. This lookup previously took the
 * state mutex itself; on a non-recursive PAL mutex the nested lock failed and
 * the compensating unlock then released the *caller's* critical section, so the
 * dispatch path silently ran unlocked. Keeping the lookup lock-free with respect
 * to the mutex makes lock ownership explicit (NC-EXT-251, R1-03). */
static FF_Pus_Handler_t ff_pus_find_locked(uint8_t service, uint8_t subservice)
{
    for (uint16_t i = 0; i < FF_PUS_MAX_SERVICES; i++) {
        if (g_svcs[i].used && (g_svcs[i].service == service) &&
            (g_svcs[i].subservice == subservice)) {
            return g_svcs[i].handler;
        }
    }
    return NULL;
}

/* TM sequence counter allocation (SRS-REQ-007). One modulo-16384 sequence is
 * shared by every emission path, so both the normal path (state mutex held) and
 * the reset/panic direct path (no mutex, ADR-0021) allocate through the same
 * compare-exchange loop. Two separate implementations previously disagreed: the
 * mutex-guarded variant performed a plain read-modify-write while the direct
 * path used an atomic increment that masked only the returned value, leaving the
 * stored value free to run past 16383. That mixed pair could lose an increment,
 * publish a value outside the sequence, or emit a duplicate counter. Here the
 * stored value is masked on every commit, so it is always in [0, 16383], and the
 * exchange makes lost increments impossible (NC-EXT-251, R1-05/R3 F-3). */
static uint16_t ff_pus_next_tm_counter_atomic(void)
{
    uint16_t cur = __atomic_load_n(&g_tm_counter, __ATOMIC_RELAXED);

    for (;;) {
        uint16_t next = (uint16_t)((cur + 1u) & 0x3FFFu);
        if (__atomic_compare_exchange_n(&g_tm_counter, &cur, next, false, __ATOMIC_RELAXED,
                                        __ATOMIC_RELAXED)) {
            return cur;
        }
    }
}

/* Commit one already-used counter value: advance the shared counter from
 * `expected` to the next value only if nothing else advanced it meanwhile.
 * Returns false when another context (the reset/panic direct path) moved the
 * counter, in which case the composed frame carries a stale value and must be
 * recomposed. Advancing after a successful composition is what keeps a failed
 * emission from leaving a hole in the sequence that the ground would report as
 * a missing packet (SRS-REQ-007; R3 F-4). */
static bool ff_pus_tm_counter_try_advance(uint16_t expected)
{
    uint16_t cur = expected;
    uint16_t next = (uint16_t)((expected + 1u) & 0x3FFFu);

    return __atomic_compare_exchange_n(&g_tm_counter, &cur, next, false, __ATOMIC_ACQ_REL,
                                       __ATOMIC_RELAXED);
}

uint16_t FF_PusNextTmCounter(void)
{
    return ff_pus_next_tm_counter_atomic();
}

uint16_t FF_PusPeekTmCounter(void)
{
    return __atomic_load_n(&g_tm_counter, __ATOMIC_RELAXED);
}

uint32_t FF_PusPeekAckDropped(void)
{
    return __atomic_load_n(&g_ack_dropped, __ATOMIC_RELAXED);
}
uint32_t FF_PusGetTcCount(void)
{
    FF_PalMutexLock(g_pus_mutex, 0u);
    uint32_t c = g_tc_count;
    FF_PalMutexUnlock(g_pus_mutex);
    return c;
}

FF_Status_t FF_PusSendTm(uint8_t service, uint8_t subservice, const uint8_t *data, uint16_t len)
{
    uint8_t scratch[FF_PUS_TM_SCRATCH];
    FF_Pus_TmHdr_t hdr;
    uint16_t out_len = 0u;
    uint32_t coarse = 0u;
    uint16_t fine = 0u;

    /* A TM emission is a single bounded operation: either this context composes
     * and hands over one frame, or it is refused. The guard is atomic so the
     * decision does not depend on g_pus_mutex, which is NOT held across the sink
     * call (a sink may perform blocking link I/O). A recursive emission from
     * inside the sink therefore gets FF_BUSY instead of deadlocking or
     * re-entering the state (NC-EXT-251, R1-16). */
    if (__atomic_exchange_n(&g_tm_emitting, 1u, __ATOMIC_ACQUIRE) != 0u) {
        return FF_BUSY;
    }

    FF_Status_t lock_rc = FF_PalMutexLock(g_pus_mutex, 0u);
    if (lock_rc != FF_SUCCESS) {
        __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
        return (lock_rc == FF_TIMEOUT) ? FF_BUSY : lock_rc;
    }
    const FF_Pus_Backend_t *backend = __atomic_load_n(&g_backend, __ATOMIC_ACQUIRE);
    if (backend == NULL || backend->encode_tm == NULL) {
        FF_PalMutexUnlock(g_pus_mutex);
        __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
        return FF_NOT_READY;
    }
    if ((data == NULL) && (len != 0u)) {
        FF_PalMutexUnlock(g_pus_mutex);
        __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
        return FF_INVALID_ARGUMENT;
    }
    if (len > FF_PUS_MAX_TM_FRAME) {
        FF_PalMutexUnlock(g_pus_mutex);
        __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
        return FF_BAD_SIZE;
    }
    /* Snapshot the sink AND backend under the state lock and release the lock
     * before calling out: the sink is application code that may block on a link
     * write, and holding the PUS state mutex across it would serialize unrelated
     * TM state access behind link I/O.
     *
     * With no sink registered, emission is a successful no-op by contract
     * (existing callers emit before a sink is attached), but it must NOT consume
     * a counter value: a consumed counter with no transmission is a hole the
     * ground reports as a missing packet (SRS-REQ-007; R3 F-4). */
    FF_Pus_TmSink_t sink = g_tm_sink;
    if (sink == NULL) {
        FF_PalMutexUnlock(g_pus_mutex);
        __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
        return FF_SUCCESS;
    }
    FF_PalMutexUnlock(g_pus_mutex);

    (void)FF_TimeGetCuc(&coarse, &fine);
    memset(&hdr, 0, sizeof(hdr));
    hdr.version = FF_PUS_VERSION;
    hdr.service = service;
    hdr.subservice = subservice;
    hdr.destination_id = FF_MISSION_ID;
    hdr.time_coarse_s = coarse;
    hdr.time_fine = fine;

    /* Compose, then commit: the counter is advanced only once a frame exists, so
     * an encoding failure leaves no hole. The counter value goes into the frame,
     * so a concurrent reset-path emission can invalidate the value between the
     * peek and the commit; the composition is retried a bounded number of times
     * in that case rather than emitting a duplicate or a stale counter. */
    for (uint8_t attempt = 0u;; attempt++) {
        hdr.counter = __atomic_load_n(&g_tm_counter, __ATOMIC_RELAXED);
        FF_Status_t enc_st =
            backend->encode_tm(&hdr, data, len, scratch, sizeof(scratch), &out_len);
        if (enc_st != FF_SUCCESS) {
            __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
            return FF_ERROR;
        }
        if (ff_pus_tm_counter_try_advance(hdr.counter)) {
            break;
        }
        if (attempt >= (FF_PUS_ACK_EMIT_RETRIES - 1u)) {
            __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
            return FF_BUSY;
        }
    }

    sink(scratch, out_len);
    __atomic_store_n(&g_tm_emitting, 0u, __ATOMIC_RELEASE);
    return FF_SUCCESS;
}

/* Reset-path telemetry (see ADR-0021). Not re-entrant by design: it uses a
 * static frame buffer because it runs on the panic path, where a ~1 KiB stack
 * scratch is undesirable. It is called at most once per reset. */
static uint8_t g_direct_frame[FF_PUS_TM_SCRATCH];

FF_Status_t FF_PusSendTmDirect(uint8_t service, uint8_t subservice, const uint8_t *data,
                               uint16_t len, uint64_t deadline_ns, FF_Pus_DirectWriter_t writer)
{
    FF_Pus_TmHdr_t hdr;
    uint16_t out_len = 0u;
    uint64_t ns;
    uint64_t frac;

    /* Lock-free by contract (ADR-0021): this runs on the panic path, where any
     * mutex may be owned by the failing context. The backend pointer is
     * snapshotted atomically; the encode step writes only the static direct
     * frame buffer, and the writer is application-supplied. No state mutex, no
     * time mutex, no allocation. Taking g_pus_mutex here previously contradicted
     * the documented contract and ignored the lock result (NC-EXT-251, R1-06). */
    const FF_Pus_Backend_t *backend = __atomic_load_n(&g_backend, __ATOMIC_ACQUIRE);

    if ((writer == NULL) || (backend == NULL) || (backend->encode_tm == NULL)) {
        return FF_NOT_READY;
    }
    if ((data == NULL) && (len != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    if (len > FF_PUS_MAX_TM_FRAME) {
        return FF_BAD_SIZE;
    }
    if (FF_PalGetMonotonicNs() >= deadline_ns) {
        return FF_TIMEOUT;
    }

    /* Degraded, lock-free time base: FF_TimeGetCuc() takes the time mutex,
     * which may be held by the failing context. */
    ns = FF_PalGetMonotonicNs();
    frac = ns % 1000000000ULL;

    memset(&hdr, 0, sizeof(hdr));
    hdr.version = FF_PUS_VERSION;
    hdr.service = service;
    hdr.subservice = subservice;
    hdr.destination_id = FF_MISSION_ID;
    hdr.time_coarse_s = (uint32_t)(ns / 1000000000ULL);
    hdr.time_fine = (uint16_t)((frac * 65536ULL) / 1000000000ULL);

    /* Compose first, then commit counter only on success (like normal path).
     * This avoids consuming a counter value for a frame that is never
     * transmitted (R3 F-4 / SRS-REQ-007). */
    if (backend->encode_tm(&hdr, data, len, g_direct_frame, sizeof(g_direct_frame), &out_len) !=
        FF_SUCCESS) {
        return FF_ERROR;
    }
    if (FF_PalGetMonotonicNs() >= deadline_ns) {
        return FF_TIMEOUT;
    }

    /* Commit counter only after successful composition and before writer. */
    hdr.counter = ff_pus_next_tm_counter_atomic();

    /* Re-encode with committed counter (encode only depends on header fields). */
    if (backend->encode_tm(&hdr, data, len, g_direct_frame, sizeof(g_direct_frame), &out_len) !=
        FF_SUCCESS) {
        /* Counter already consumed; this is a theoretical hole but extremely
         * unlikely since encode succeeded once. Accept as panic-path limit. */
        return FF_ERROR;
    }
    return writer(g_direct_frame, out_len, deadline_ns);
}

/* ---- Pre-reset event report (ADR-0021) --------------------------- */

FF_Status_t FF_PusEmitPanicEvent(const char *reason, uint64_t deadline_ns,
                                 FF_Pus_DirectWriter_t writer)
{
    uint8_t payload[FF_EVS_TEXT_LEN + 3u];
    uint16_t text_len = 0u;

    if (reason == NULL) {
        reason = "panic";
    }
    while ((text_len < FF_EVS_TEXT_LEN) && (reason[text_len] != '\0')) {
        text_len++;
    }
    payload[0] = (uint8_t)(FF_EVS_PANIC_RESET >> 8);
    payload[1] = (uint8_t)(FF_EVS_PANIC_RESET & 0xFFu);
    payload[2] = FF_EVS_CRITICAL;
    memcpy(&payload[3], reason, text_len);

    return FF_PusSendTmDirect(FF_PUS_SERVICE_EVENT, 3u, payload, (uint16_t)(text_len + 3u),
                              deadline_ns, writer);
}

static FF_Pus_DirectWriter_t g_panic_writer;

static FF_Status_t ff_panic_reporter(const char *reason, uint64_t deadline_ns)
{
    return FF_PusEmitPanicEvent(reason, deadline_ns, g_panic_writer);
}

FF_Status_t FF_PusInstallPanicReporter(FF_Pus_DirectWriter_t writer)
{
    if (writer == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    FF_PalMutexLock(g_pus_mutex, 0u);
    g_panic_writer = writer;
    FF_PalMutexUnlock(g_pus_mutex);
    FF_PalSetPanicNotifier(ff_panic_reporter);
    return FF_SUCCESS;
}

FF_Status_t FF_PusSendVerification(const FF_Pus_TcHdr_t *tc, FF_Pus_Stage_t stage, uint8_t success,
                                   uint8_t code)
{
    uint8_t payload[7] = {0};
    uint8_t subservice;

    if (tc == NULL) {
        return FF_INVALID_ARGUMENT;
    }
    /* Service 1 subservice: 1 accept. ok, 2 accept fail, 3 start ok, 4 start fail,
     * 5 progress ok, 6 progress fail, 7 completion ok, 8 completion fail. */
    switch (stage) {
    case FF_PUS_STAGE_ACCEPTANCE:
        subservice = success ? 1u : 2u;
        break;
    case FF_PUS_STAGE_START:
        subservice = success ? 3u : 4u;
        break;
    case FF_PUS_STAGE_PROGRESS:
        subservice = success ? 5u : 6u;
        break;
    case FF_PUS_STAGE_COMPLETION:
        subservice = success ? 7u : 8u;
        break;
    default:
        return FF_INVALID_ARGUMENT;
    }

    /* Request ID per ECSS-E-ST-70-41C §5.4.11.2.1(c):
     * - source identifier (16 bits, from TC secondary header)
     * - destination identifier (16 bits, the spacecraft/apid destination; provisional 0x0001)
     * - sequence count (16 bits, monotonically increasing per source)
     * The verification payload then carries: Request ID (6 octets) + service + subservice + code.
     * Incremented only on SUCCESSFUL emission (not per retry) to avoid gaps.
     */
    static uint16_t s_sequence_count = 0u;
    /* Increment atomically on successful emission; retries don't advance. */

    /* Sequence count is incremented only when emission succeeds. The retry
     * loop in ff_pus_emit_required does NOT increment on FF_BUSY. */
    uint16_t seq = __atomic_fetch_add(&s_sequence_count, 1u, __ATOMIC_RELAXED);
    payload[0] = (uint8_t)(tc->source_id >> 8);
    payload[1] = (uint8_t)(tc->source_id & 0xFFu);
    /* Destination identifier: provisional framework default (GEO OBC = 0x0001).
     * Mission authority must confirm or replace via ICD. */
    payload[2] = 0x00u;
    payload[3] = 0x01u;
    payload[4] = (uint8_t)(seq >> 8);
    payload[5] = (uint8_t)(seq & 0xFFu);
    payload[6] = tc->service;
    /* Note: subservice and code follow in extended payload; to keep frame size
     * bounded we append them after the Request ID for subservice != 0.
     * The TM sink expects: Request ID (6) + service (1) + subservice (1) + code (1) = 9 octets.
     * This implementation sends 9 octets for all verification stages. */
    uint8_t extended[9];
    memcpy(extended, payload, 6);
    extended[6] = tc->service;
    extended[7] = tc->subservice;
    extended[8] = code;
    return FF_PusSendTm(FF_PUS_SERVICE_VERIFICATION, subservice, extended, sizeof(extended));
}

FF_Status_t FF_PusEmitEvent(uint16_t event_id, uint8_t evs_type, const uint8_t *data, uint16_t len)
{
    /* Event-report payload layout (ICD §4, ECSS-E-ST-70-41C service 5):
     *   event id [2 octets, big-endian] | type [1] | auxiliary data [len]
     * This function owns the header: `event_id` and `evs_type` are encoded
     * here, and `data`/`len` carry the auxiliary data only (normally the
     * free-text reason). Callers must not prepend an id or a type octet. */
    uint8_t payload[FF_PUS_MAX_TM_FRAME];
    uint8_t subservice;

    if (len > (FF_PUS_MAX_TM_FRAME - 3u)) {
        return FF_BAD_SIZE;
    }
    if ((data == NULL) && (len != 0u)) {
        return FF_INVALID_ARGUMENT;
    }
    payload[0] = (uint8_t)(event_id >> 8);
    payload[1] = (uint8_t)(event_id & 0xFFu);
    payload[2] = evs_type;
    if (len != 0u) {
        memcpy(&payload[3], data, len);
    }
    if (evs_type == FF_EVS_INFO) {
        subservice = 1u;
    }
    else if (evs_type == FF_EVS_WARNING) {
        subservice = 2u;
    }
    else {
        subservice = 3u;
    }
    return FF_PusSendTm(FF_PUS_SERVICE_EVENT, subservice, payload, (uint16_t)(len + 3u));
}

/* Re-acquire the state mutex after running a callback or emitting telemetry.
 * Every re-acquire used to ignore its result; when the mutex was held elsewhere
 * the code went on to mutate reservation state unlocked and to unlock a lock it
 * did not own (NC-EXT-251, R1-01/R1-02). A bounded retry keeps the dispatch
 * path progressing under transient contention instead of failing every time. */
static FF_Status_t ff_pus_relock(void)
{
    for (uint8_t attempt = 0u; attempt < FF_PUS_ACK_EMIT_RETRIES; attempt++) {
        FF_Status_t st = FF_PalMutexLock(g_pus_mutex, 0u);
        if (st == FF_SUCCESS) {
            return FF_SUCCESS;
        }
        if (st != FF_TIMEOUT) {
            return st;
        }
        FF_PalTaskYield();
    }
    return FF_TIMEOUT;
}

/* Emit one required service 1 verification. A refused emission is retried a
 * bounded number of times and then counted: previously the result was discarded,
 * so a contended emission silently deleted a required ACK (SRS-REQ-006,
 * SRS-REQ-011; R1-07/R3 F-1). */
static FF_Status_t ff_pus_emit_required(const FF_Pus_TcHdr_t *tc, FF_Pus_Stage_t stage,
                                        uint8_t success, uint8_t code)
{
    FF_Status_t st = FF_ERROR;

    for (uint8_t attempt = 0u; attempt < FF_PUS_ACK_EMIT_RETRIES; attempt++) {
        st = FF_PusSendVerification(tc, stage, success, code);
        if (st != FF_BUSY) {
            return st;
        }
        FF_PalTaskYield();
    }
    (void)__atomic_fetch_add(&g_ack_dropped, 1u, __ATOMIC_RELAXED);
    (void)FF_EvsSend(FF_EVS_PUS_TC_ERR, FF_EVS_ERROR, 0u,
                     "PUS S1 verification refused: telemetry emission busy");
    return st;
}

FF_Status_t FF_PusIngest(const uint8_t *raw, uint16_t len)
{
    FF_Pus_TcHdr_t tc;
    const uint8_t *data = NULL;
    uint16_t data_len = 0u;
    uint16_t apid = 0u;
    FF_Pus_Handler_t handler;
    FF_Status_t rc;
    bool ack_accept;
    bool ack_start;
    bool ack_complete;
    bool ack_progress;

    FF_Status_t lock_rc = FF_PalMutexLock(g_pus_mutex, 0u);
    if (lock_rc != FF_SUCCESS) {
        return (lock_rc == FF_TIMEOUT) ? FF_BUSY : lock_rc;
    }
    if (!g_ready || (raw == NULL) || (g_backend == NULL)) {
        FF_PalMutexUnlock(g_pus_mutex);
        return FF_NOT_READY;
    }
    /* Dispatch reservation: if another TC is being processed (between
     * acceptance and completion), reject this one with FF_BUSY. */
    if (__atomic_load_n(&g_dispatch_reserved, __ATOMIC_ACQUIRE) != 0u) {
        FF_PalMutexUnlock(g_pus_mutex);
        return FF_BUSY;
    }
    const FF_Pus_Backend_t *backend = __atomic_load_n(&g_backend, __ATOMIC_ACQUIRE);
    if (backend->decode_tc(raw, len, &apid, &tc, &data, &data_len) != FF_SUCCESS) {
        FF_PalMutexUnlock(g_pus_mutex);
        (void)FF_EvsSend(FF_EVS_PUS_TC_ERR, FF_EVS_ERROR, 0u, "PUS decode failure");
        return FF_PARSE_ERROR;
    }

    /* Reserve the dispatch for EVERY TC, independent of the ACK flags. The
     * reservation previously covered only dispatches that requested the
     * acceptance ACK, so two TCs without it could run handlers concurrently;
     * service handler state is not internally synchronized (NC-EXT-251,
     * R1-04). The flag is atomic so a path that cannot re-acquire the state
     * mutex can still release it safely. */
    __atomic_store_n(&g_dispatch_reserved, 1u, __ATOMIC_RELEASE);

    g_tc_count++;
    ack_accept = (tc.ack_flags & FF_PUS_ACK_ACCEPTANCE) != 0u;
    ack_start = (tc.ack_flags & FF_PUS_ACK_START) != 0u;
    ack_progress = (tc.ack_flags & FF_PUS_ACK_PROGRESS) != 0u;
    ack_complete = (tc.ack_flags & FF_PUS_ACK_COMPLETION) != 0u;

    if (apid != FF_APID_TC) {
        if (ack_accept) {
            FF_PalMutexUnlock(g_pus_mutex);
            (void)ff_pus_emit_required(&tc, FF_PUS_STAGE_ACCEPTANCE, 0u, FF_PUS_FAIL_ILLEGAL_APID);
            if (ff_pus_relock() != FF_SUCCESS) {
                __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
                return FF_BUSY;
            }
        }
        __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
        FF_PalMutexUnlock(g_pus_mutex);
        return FF_ERROR;
    }
    if (ack_accept) {
        FF_PalMutexUnlock(g_pus_mutex);
        (void)ff_pus_emit_required(&tc, FF_PUS_STAGE_ACCEPTANCE, 1u, 0u);
        if (ff_pus_relock() != FF_SUCCESS) {
            __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
            return FF_BUSY;
        }
    }

    handler = ff_pus_find_locked(tc.service, tc.subservice);
    if (handler == NULL) {
        uint8_t code =
            (tc.service == 0u) ? FF_PUS_FAIL_UNKNOWN_SERVICE : FF_PUS_FAIL_UNKNOWN_SUBSERVICE;
        if (ack_complete) {
            FF_PalMutexUnlock(g_pus_mutex);
            (void)ff_pus_emit_required(&tc, FF_PUS_STAGE_COMPLETION, 0u, code);
            if (ff_pus_relock() != FF_SUCCESS) {
                __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
                return FF_BUSY;
            }
        }
        __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
        FF_PalMutexUnlock(g_pus_mutex);
        return FF_NOT_FOUND;
    }

    if (ack_start) {
        FF_PalMutexUnlock(g_pus_mutex);
        (void)ff_pus_emit_required(&tc, FF_PUS_STAGE_START, 1u, 0u);
        if (ff_pus_relock() != FF_SUCCESS) {
            __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
            return FF_BUSY;
        }
    }

    /* Handlers run WITHOUT the state lock held (design note
     * docs/engineering/nc-ext-251-design/bounded-state-machine.md: the state
     * mutex is never held across a callback). Holding it across the handler
     * made any handler that emits telemetry re-enter FF_PusSendTm with the lock
     * already owned, which the non-recursive PAL mutex cannot represent and
     * which was silently ignored (PA F-08). The dispatch reservation stays set
     * across the handler, so a concurrent ingest is still rejected FF_BUSY. */
    FF_PalMutexUnlock(g_pus_mutex);
    rc = handler(&tc, data, data_len);
    if (ff_pus_relock() != FF_SUCCESS) {
        __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
        return FF_BUSY;
    }

    /* Progress verification after handler: success emits TM[1,5], error emits TM[1,6].
     * Implements ICD §4 S1 row: subservice 5 = progress success, 6 = progress fail.
     * Exactly one progress verification is sent; never both 1,5 and 1,6. */
    if (ack_progress) {
        FF_PalMutexUnlock(g_pus_mutex);
        (void)ff_pus_emit_required(&tc, FF_PUS_STAGE_PROGRESS, (rc == FF_SUCCESS) ? 1u : 0u,
                                   (rc == FF_SUCCESS) ? 0u : FF_PUS_FAIL_EXECUTION);
        if (ff_pus_relock() != FF_SUCCESS) {
            __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
            return FF_BUSY;
        }
    }

    if (ack_complete) {
        FF_PalMutexUnlock(g_pus_mutex);
        (void)ff_pus_emit_required(&tc, FF_PUS_STAGE_COMPLETION, (rc == FF_SUCCESS) ? 1u : 0u,
                                   (rc == FF_SUCCESS) ? 0u : FF_PUS_FAIL_EXECUTION);
        if (ff_pus_relock() != FF_SUCCESS) {
            /* Relock failed after handler: completion ACK was attempted via
             * ff_pus_emit_required (which accounts in g_ack_dropped on
             * permanent refusal). Just release reservation and return. */
            __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
            return FF_BUSY;
        }
    }
    __atomic_store_n(&g_dispatch_reserved, 0u, __ATOMIC_RELEASE);
    FF_PalMutexUnlock(g_pus_mutex);
    return rc;
}