/*
 * frick-fsw - ES/HS application-lifecycle host integration harness
 * (S8-VV-FU; gap GAP-R3-1 from FRICK-REVIEW-0076 / independent assessment T-08)
 *
 * Before this suite, ES lifecycle and Health & Safety detection had only
 * simulator-smoke evidence. This harness runs the real ES registry, the real
 * POSIX PAL task layer and the real HS monitoring pass on the host, with the
 * EVS listener seam as the independent oracle (the same seam the flight wiring
 * uses to forward events to PUS service 5).
 *
 * Fault cases exercised: PAL task-create resource exhaustion driving an
 * application to FF_APP_FAILED, task exit (entry returns), and the HS
 * FAILED-application report. The oracle is the observable event stream
 * (event id, type, application id, text), not internal state alone.
 *
 * Contract limits (never asserted as requirements here): the state transition
 * on task return/abort and the invalid-id/saturation return contracts are not
 * fixed by any clause (TS-6 `N22-TS6-41`/`N22-TS6-36`, NC-EXT-47 §7.2); those
 * sub-cases are recorded as observations only. Detection latency and the
 * external watchdog window are mission inputs (TS-5 `N22-TS5-52`, TS-6
 * `N22-TS6-31`). The heartbeat event id (`0x0502`) and the "HS: app %u failed"
 * diagnostic text this harness matches on are implementation-defined literals of
 * `src/apps/app_hs.c` (no controlled requirement/interface clause fixes them),
 * so the event-identification oracle is coupled to the implementation under
 * test: it can confirm the event stream is unchanged relative to that
 * implementation, but it cannot independently detect a defect in the shared
 * literal itself. A controlled identifier/text contract would be required to
 * remove that coupling. GAP-R3-1 stays OPEN until the independent R2/R3 delta
 * review is recorded; nothing here closes an NC or approves a phase.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: MPL-2.0
 */
#include "../ff_test.h"
#include "fsw_config.h"
#include "core/common/ff_types.h"
#include "core/pal/pal.h"
#include "core/sb/sb.h"
#include "core/evs/evs.h"
#include "core/es/es.h"
#include "apps/app.h"
#include "pus/pus.h"
#include "transport/ccsds/ccsds.h"
#include "transport/link/link.h"
#include <setjmp.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Independent oracle: capture the EVS event stream at the listener    */
/* seam (the production observation point for HS output).              */
/* ------------------------------------------------------------------ */

#define HS_CAP_MAX 64u

static FF_EvsEvent_t g_cap[HS_CAP_MAX];
static uint16_t g_cap_count;

static void hs_capture(const FF_EvsEvent_t *ev)
{
    if (g_cap_count < HS_CAP_MAX) {
        g_cap[g_cap_count] = *ev;
        g_cap_count++;
    }
}

static void cap_reset(void)
{
    (void)memset(g_cap, 0, sizeof(g_cap));
    g_cap_count = 0u;
}

static uint16_t cap_count_id(uint16_t event_id)
{
    uint16_t n = 0u;
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if (g_cap[i].event_id == event_id) {
            n++;
        }
    }
    return n;
}

static uint16_t cap_count_id_type(uint16_t event_id, uint8_t type)
{
    uint16_t n = 0u;
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if ((g_cap[i].event_id == event_id) && (g_cap[i].type == type)) {
            n++;
        }
    }
    return n;
}

/* True when a captured CRITICAL application-error event reports app id `id`
 * ("HS: app <n> failed"). */
static bool cap_hs_reports(uint16_t app_id)
{
    char needle[32];
    int n = snprintf(needle, sizeof(needle), "app %u failed", (unsigned)app_id);
    if ((n < 0) || ((size_t)n) >= sizeof(needle)) {
        return false;
    }
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if ((g_cap[i].event_id == FF_EVS_APP_ERROR) && (g_cap[i].type == FF_EVS_CRITICAL) &&
            (strstr(g_cap[i].text, needle) != NULL)) {
            return true;
        }
    }
    return false;
}

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

/* Dummy application entries: a host task whose entry returns immediately is
 * the host-side model of a terminated task (PAL keeps the slot until delete). */
static void hs_dummy_entry(void *arg)
{
    (void)arg;
}

/* Fill every remaining PAL task slot so the next task create fails. Returns
 * the number of filler tasks created; the loop is bounded by the PAL slot
 * count plus one. */
static uint16_t hs_fill_pal_slots(void)
{
    uint16_t created = 0u;
    FF_TaskId_t tid = 0u;

    for (uint16_t i = 0u; i <= 64u; i++) {
        if (FF_PalTaskCreate(&tid, "filler", hs_dummy_entry, NULL, 0u, 0u) != FF_SUCCESS) {
            return created;
        }
        created++;
    }
    return created; /* PAL never reported exhaustion: bounded stop, flagged by caller */
}

static void hs_init_modules(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_SbInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EvsInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsInit() == FF_SUCCESS);
    FF_EvsSetListener(hs_capture);
    cap_reset();
}

/* ------------------------------------------------------------------ */
/* 1. Nominal: heartbeat every monitoring pass, no false FAILED report */
/* ------------------------------------------------------------------ */

static void test_hs_heartbeat_no_fault(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t ok_id = FF_INVALID_ID;
    FF_AppId_t hs_id = FF_INVALID_ID;

    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "ok_app";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &ok_id) == FF_SUCCESS);
    cfg.name = "hs_app";
    cfg.entry = FF_AppHsMain;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &hs_id) == FF_SUCCESS);

    /* Start only the worker: the HS task itself is stepped directly by the
     * harness so the pass is deterministic (no 5000 ms cadence wait). */
    FF_TEST_ASSERT(FF_EsStartApp(ok_id) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetState(ok_id) == FF_APP_RUNNING);
    FF_TEST_ASSERT(FF_EsGetState(hs_id) == FF_APP_LOADED); /* registered, not started */

    /* Nominal oracle (SRS-REQ-014 "detect failed applications"; SSS-REQ-008
     * "generate heartbeat events"): a RUNNING and a LOADED application are not
     * health anomalies, so no application-error event may be raised, and each
     * monitoring pass raises exactly one heartbeat for the HS application. */
    cap_reset();
    FF_AppHsStep(hs_id);
    FF_AppHsStep(hs_id);

    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_ERROR) == 0u);
    FF_TEST_ASSERT(cap_count_id(0x0502u) == 2u); /* one heartbeat per pass */
    FF_TEST_ASSERT(cap_count_id_type(0x0502u, FF_EVS_INFO) == 2u);
    for (uint16_t i = 0u; i < g_cap_count; i++) {
        if (g_cap[i].event_id == 0x0502u) {
            FF_TEST_ASSERT(g_cap[i].app_id == hs_id);
        }
    }

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* 2. Lifecycle on the real host PAL: start, idempotency, task exit     */
/* ------------------------------------------------------------------ */

static void test_es_lifecycle_host_tasks(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t a = FF_INVALID_ID;
    FF_AppId_t b = FF_INVALID_ID;

    FF_PalTaskDelayMs(20u); /* let previous-test tasks terminate before re-init */
    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "worker_a";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &a) == FF_SUCCESS);
    cfg.name = "worker_b";
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &b) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_EsGetAppCount() == 2u);
    FF_TEST_ASSERT(FF_EsGetId("worker_a") == a);
    FF_TEST_ASSERT(FF_EsGetState(a) == FF_APP_LOADED);

    cap_reset();
    FF_TEST_ASSERT(FF_EsStartAll() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_EsGetState(a) == FF_APP_RUNNING);
    FF_TEST_ASSERT(FF_EsGetState(b) == FF_APP_RUNNING);
    /* Independent-task oracle: one APP_STARTED event per application. */
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_STARTED) == 2u);

    /* Idempotent restart (SRS-REQ-014 state tracking; TS-5 N22-TS5-38). */
    FF_TEST_ASSERT(FF_EsStartApp(a) == FF_SUCCESS);
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_STARTED) == 2u); /* no second start */

    /* Task exit observation: both entries return immediately. Observation
     * only - no clause defines the state transition on task return/abort
     * (TS-6 N22-TS6-41, NC-EXT-47 §7.2), so this records current behaviour
     * (state stays RUNNING, HS raises no spurious report) and is not a
     * requirement oracle. */
    FF_PalTaskDelayMs(20u);
    FF_TEST_ASSERT(FF_EsGetState(a) == FF_APP_RUNNING);
    cap_reset();
    FF_AppHsStep(a);
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_ERROR) == 0u);

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* 3. Fault cases: start failure -> FAILED, HS reports it exactly once  */
/*    per monitoring pass, naming the failed application id              */
/* ------------------------------------------------------------------ */

static void test_es_start_failure_and_hs_detection(void)
{
    FF_AppConfig_t cfg;
    FF_AppId_t ok_id = FF_INVALID_ID;
    FF_AppId_t bad_id = FF_INVALID_ID;
    FF_AppId_t hs_id = FF_INVALID_ID;
    uint16_t fillers;

    FF_PalTaskDelayMs(20u); /* let previous-test tasks terminate before re-init */
    hs_init_modules();

    (void)memset(&cfg, 0, sizeof(cfg));
    cfg.name = "ok_app";
    cfg.entry = hs_dummy_entry;
    cfg.stack_bytes = 65536u;
    cfg.priority = 0u;
    cfg.queue_depth = 4u;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &ok_id) == FF_SUCCESS);
    cfg.name = "hs_app";
    cfg.entry = FF_AppHsMain;
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &hs_id) == FF_SUCCESS);

    FF_TEST_ASSERT(FF_EsStartApp(ok_id) == FF_SUCCESS);

    /* Fault injection: exhaust the PAL task slots, then start one more
     * application. The start must fail and ES must track the failure. */
    fillers = hs_fill_pal_slots();
    FF_TEST_ASSERT(fillers < 64u); /* exhaustion reached before the bounded stop */

    cfg.name = "bad_app";
    FF_TEST_ASSERT(FF_EsRegisterApp(&cfg, &bad_id) == FF_SUCCESS);
    cap_reset();
    FF_TEST_ASSERT(FF_EsStartApp(bad_id) == FF_ERROR);
    /* SRS-REQ-014: ES tracks states; a failed start is observable as
     * FF_APP_FAILED (the failed-start path, TS-5 N22-TS5-51 fault case). */
    FF_TEST_ASSERT(FF_EsGetState(bad_id) == FF_APP_FAILED);
    /* The failure is also reported at the event seam. */
    FF_TEST_ASSERT(cap_count_id(FF_EVS_APP_ERROR) >= 1u);

    /* HS detection oracle (SRS-REQ-014 "Health & Safety shall detect failed
     * applications"; SSS-REQ-008 "monitor application health"): one pass
     * reports exactly the FAILED application, by id, as a CRITICAL event, and
     * still emits exactly one heartbeat. The RUNNING application is not
     * reported (no false positive). */
    cap_reset();
    FF_AppHsStep(hs_id);
    FF_TEST_ASSERT(cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u);
    FF_TEST_ASSERT(cap_hs_reports(bad_id));
    FF_TEST_ASSERT(!cap_hs_reports(ok_id));
    FF_TEST_ASSERT(cap_count_id(0x0502u) == 1u);

    /* Second pass: the FAILED application is reported again (one report per
     * monitoring pass while the failure persists). */
    cap_reset();
    FF_AppHsStep(hs_id);
    FF_TEST_ASSERT(cap_count_id_type(FF_EVS_APP_ERROR, FF_EVS_CRITICAL) == 1u);
    FF_TEST_ASSERT(cap_hs_reports(bad_id));
    FF_TEST_ASSERT(cap_count_id(0x0502u) == 1u);

    /* Persistence: the state is unchanged by observation. */
    FF_TEST_ASSERT(FF_EsGetState(bad_id) == FF_APP_FAILED);
    FF_TEST_ASSERT(FF_EsGetState(ok_id) == FF_APP_RUNNING);

    FF_EvsSetListener(NULL);
}

/* ------------------------------------------------------------------ */
/* 4. Reset path (N22-TS5-50): PUS S8 function 1 -> FF_PalPanic ->     */
/*    pre-reset PUS S5 event report -> single watchdog reset request   */
/*                                                                     */
/* ADR-0021 / ECSS-E-ST-70-11 5.7.5.2. The test seam is FF_TEST_HOOKS   */
/* only (absent from flight builds). It lets the test observe the       */
/* pre-reset notification and the reset request without terminating    */
/* the process; the flight path continues to abort() when no hardware  */
/* reset is wired.                                                     */
/* ------------------------------------------------------------------ */

#define HS_RESET_FRAME_MAX 2048u

static uint8_t g_reset_frame[HS_RESET_FRAME_MAX];
static uint16_t g_reset_frame_len;
static uint64_t g_reset_deadline_ns;
static uint32_t g_reset_report_count; /* number of pre-reset reports emitted */
static char g_panic_reason[128];
static bool g_panic_seen;
static jmp_buf g_panic_jmp;
static uint32_t g_observed_notifier_calls;
static uint64_t g_observed_notifier_deadline;
static uint64_t g_observed_notifier_entry_ns;
static uint32_t g_observed_resets_at_notifier;
static size_t g_observed_reason_length;

static FF_Status_t hs_observe_notifier(const char *reason, uint64_t deadline_ns)
{
    g_observed_notifier_calls++;
    g_observed_notifier_deadline = deadline_ns;
    g_observed_notifier_entry_ns = FF_PalGetMonotonicNs();
    /* Observation of the reset counter inside the notifier establishes the
     * report-before-reset ordering in this host harness: the reset request has
     * not been made yet when the notification runs. */
    g_observed_resets_at_notifier = FF_PalWatchdogGetResets();
    g_observed_reason_length = (reason != NULL) ? strlen(reason) : 0u;
    return FF_TIMEOUT;
}

static FF_Status_t hs_reset_writer(const uint8_t *frame, uint16_t len, uint64_t deadline_ns)
{
    if ((frame == NULL) || (len > HS_RESET_FRAME_MAX)) {
        return FF_BAD_SIZE;
    }
    (void)memcpy(g_reset_frame, frame, len);
    g_reset_frame_len = len;
    g_reset_deadline_ns = deadline_ns;
    g_reset_report_count++;
    return FF_SUCCESS;
}

static FF_Status_t hs_failing_reporter(const char *reason, uint64_t deadline_ns)
{
    (void)reason;
    (void)deadline_ns;
    return FF_ERROR;
}

static void hs_panic_handler(const char *msg)
{
    (void)snprintf(g_panic_reason, sizeof(g_panic_reason), "%s", msg);
    g_panic_seen = true;
    longjmp(g_panic_jmp, 1);
}

/* Encode one PUS TC and inject it through the production ingest path. */
static FF_Status_t hs_pus_ingest(uint8_t service, uint8_t subservice, const uint8_t *data,
                                 uint16_t len)
{
    FF_Pus_TcHdr_t tc;
    uint8_t frame[FF_CCSDS_MAX_TC_PACKET];
    uint16_t frame_len = 0u;

    (void)memset(&tc, 0, sizeof(tc));
    tc.version = FF_PUS_VERSION;
    tc.ack_flags = FF_PUS_ACK_ACCEPTANCE | FF_PUS_ACK_COMPLETION;
    tc.service = service;
    tc.subservice = subservice;
    tc.source_id = 0x0102u;
    if (FF_CcsdsEncodeTc(FF_APID_TC, &tc, data, len, frame, sizeof(frame), &frame_len) !=
        FF_SUCCESS) {
        return FF_ERROR;
    }
    return FF_PusIngest(frame, frame_len);
}

static bool frame_contains(const uint8_t *hay, uint16_t hay_len, const uint8_t *needle,
                           uint16_t needle_len)
{
    if ((hay == NULL) || (needle == NULL) || (needle_len == 0u) || (hay_len < needle_len)) {
        return false;
    }
    for (uint16_t i = 0u; i <= (uint16_t)(hay_len - needle_len); i++) {
        if (memcmp(&hay[i], needle, needle_len) == 0) {
            return true;
        }
    }
    return false;
}

/* Nominal: the commanded reset runs the real service 8 function 1, produces
 * exactly one pre-reset S5 event report carrying the panic event id, and
 * requests exactly one watchdog reset. */
static void test_hs_reset_single_path(void)
{
    const uint8_t fn_reset[1] = {1u}; /* service 8 function identifier */
    /* Event id 0x0104, type CRITICAL, then the reason text ("HS: ..."). */
    const uint8_t expect[4] = {(uint8_t)(FF_EVS_PANIC_RESET >> 8),
                               (uint8_t)(FF_EVS_PANIC_RESET & 0xFFu), FF_EVS_CRITICAL, 'H'};
    uint32_t resets_before;
    uint64_t after_report_ns;

    hs_init_modules();
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL); /* the reset path must not depend on the TM sink */
    FF_AppHsRegister();

    /* Same wiring the flight main uses, plus the test observer seam. */
    FF_PusInstallPanicReporter(hs_reset_writer);
    FF_PalSetPanicHandler(hs_panic_handler);

    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    g_reset_frame_len = 0u;
    g_reset_deadline_ns = 0u;
    g_reset_report_count = 0u;
    g_panic_seen = false;
    g_panic_reason[0] = '\0';

    if (setjmp(g_panic_jmp) == 0) {
        (void)hs_pus_ingest(8u, 1u, fn_reset, sizeof(fn_reset));
        FF_TEST_ASSERT(false); /* the reset path must not return normally */
    }

    FF_TEST_ASSERT(g_panic_seen);
    FF_TEST_ASSERT(strstr(g_panic_reason, "reset requested") != NULL);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == (resets_before + 1u));
    /* "exactly one" pre-reset report (SRS-REQ-074): the count is asserted,
     * not only the presence, so a duplicate emission cannot pass. */
    FF_TEST_ASSERT(g_reset_report_count == 1u);
    after_report_ns = FF_PalGetMonotonicNs();
    FF_TEST_ASSERT(g_reset_deadline_ns >= after_report_ns);
    FF_TEST_ASSERT((g_reset_deadline_ns - after_report_ns) <= 250000000ULL);
    FF_TEST_ASSERT(g_reset_frame_len > 0u);
    FF_TEST_ASSERT(frame_contains(g_reset_frame, g_reset_frame_len, expect, sizeof(expect)));

    FF_PalSetPanicHandler(NULL);
    FF_EvsSetListener(NULL);
}

/* Host-only path oracle: fill stderr's nonblocking pipe, allow the console
 * attempt to consume the shared budget, then confirm that the notifier receives
 * the same expired deadline and reset is still requested. This is not WCET. */
static void test_panic_console_stall_and_report_share_one_budget(void)
{
    int pipefd[2];
    int saved_stderr;
    int flags;
    uint8_t fill[4096];
    uint64_t start_ns;
    uint64_t deadline_ns;
    uint64_t finish_ns;
    uint32_t resets_before;

    memset(fill, 0x5Au, sizeof(fill));
    FF_TEST_ASSERT(pipe(pipefd) == 0);
    flags = fcntl(pipefd[1], F_GETFL, 0);
    FF_TEST_ASSERT(flags >= 0);
    FF_TEST_ASSERT(fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK) == 0);
    while (write(pipefd[1], fill, sizeof(fill)) > 0) {
    }
    FF_TEST_ASSERT(errno == EAGAIN || errno == EWOULDBLOCK);
    saved_stderr = dup(STDERR_FILENO);
    FF_TEST_ASSERT(saved_stderr >= 0);
    FF_TEST_ASSERT(dup2(pipefd[1], STDERR_FILENO) == STDERR_FILENO);

    hs_init_modules();
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    g_observed_notifier_calls = 0u;
    g_observed_notifier_deadline = 0u;
    g_observed_notifier_entry_ns = 0u;
    g_observed_resets_at_notifier = 0u;
    FF_PalSetPanicNotifier(hs_observe_notifier);
    FF_PalSetPanicHandler(hs_panic_handler);
    start_ns = FF_PalGetMonotonicNs();
    if (setjmp(g_panic_jmp) == 0) {
        FF_PalPanic("stalled-console-report");
    }
    finish_ns = FF_PalGetMonotonicNs();

    /* Restore the real stderr before any test assertion can print. */
    FF_TEST_ASSERT(dup2(saved_stderr, STDERR_FILENO) == STDERR_FILENO);
    (void)close(saved_stderr);
    (void)close(pipefd[0]);
    (void)close(pipefd[1]);
    FF_PalSetPanicNotifier(NULL);
    FF_PalSetPanicHandler(NULL);

    FF_TEST_ASSERT(g_observed_notifier_calls == 1u);
    deadline_ns = g_observed_notifier_deadline;
    FF_TEST_ASSERT(deadline_ns >= start_ns);
    FF_TEST_ASSERT((deadline_ns - start_ns) >= 250000000ULL);
    FF_TEST_ASSERT((deadline_ns - start_ns) < 260000000ULL);
    /* The console writer must restore the descriptor mode it changed, so later
     * diagnostics keep the normal blocking semantics. */
    {
        int flags = fcntl(STDERR_FILENO, F_GETFL, 0);
        FF_TEST_ASSERT(flags >= 0);
        FF_TEST_ASSERT((flags & O_NONBLOCK) == 0);
    }
    /* The notification runs only after the blocked console attempt has
     * consumed the shared budget: the notifier observes an already-expired
     * deadline. This is the direct oracle that console and report share one
     * budget, and it is a host observation, not target WCET. */
    FF_TEST_ASSERT(g_observed_notifier_entry_ns >= deadline_ns);
    /* Report-before-reset ordering observed in this harness: the reset
     * counter had not yet been incremented when the notification ran. */
    FF_TEST_ASSERT(g_observed_resets_at_notifier == resets_before);
    FF_TEST_ASSERT(finish_ns >= g_observed_notifier_entry_ns);
    FF_TEST_ASSERT(finish_ns >= deadline_ns);
    /* Host harness sanity limit only; explicitly not a WCET acceptance value. */
    FF_TEST_ASSERT((finish_ns - start_ns) < 2000000000ULL);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == resets_before + 1u);
}

/* Production wiring check: the flight main installs FF_LinkSendUntil as the
 * panic reporter. On the host the selected link is the loopback backend, so
 * this case proves that the complete production chain (panic -> PUS service 5
 * frame -> link writer) actually delivers one frame to the link, with the
 * shared absolute deadline honoured. */
static void test_panic_production_link_wiring_delivers_one_frame(void)
{
    const uint8_t expect[3] = {(uint8_t)(FF_EVS_PANIC_RESET >> 8),
                               (uint8_t)(FF_EVS_PANIC_RESET & 0xFFu), FF_EVS_CRITICAL};
    uint8_t frame[512];
    uint16_t frame_len = 0u;
    uint32_t resets_before;

    hs_init_modules();
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);
    FF_TEST_ASSERT(FF_LinkInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkGet() == &FF_LinkLoopbackBackend);
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();

    /* Exactly the flight wiring: src/main/fsw_main.c installs the same
     * FF_LinkSendUntil writer. */
    FF_PusInstallPanicReporter(FF_LinkSendUntil);
    FF_PalSetPanicHandler(hs_panic_handler);

    if (setjmp(g_panic_jmp) == 0) {
        FF_PalPanic("production-link-wiring");
        FF_TEST_ASSERT(false); /* the reset path must not return normally */
    }

    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == (resets_before + 1u));
    FF_TEST_ASSERT(FF_LinkLoopbackPop(frame, sizeof(frame), &frame_len) == FF_SUCCESS);
    FF_TEST_ASSERT(frame_len > 0u);
    FF_TEST_ASSERT(frame_contains(frame, frame_len, expect, sizeof(expect)));
    /* The panic reason text is carried in the same frame. */
    {
        const char *reason = "production-link-wiring";
        FF_TEST_ASSERT(
            frame_contains(frame, frame_len, (const uint8_t *)reason, (uint16_t)strlen(reason)));
    }
    /* The PUS service identification of the pre-reset report: the frame is a
     * telemetry space packet whose PUS secondary header carries service 5
     * (event reporting) and subservice 3 (CRITICAL), as fixed by ADR-0021. */
    {
        uint16_t apid = 0u;
        FF_Pus_TmHdr_t hdr;
        const uint8_t *payload = NULL;
        uint16_t payload_len = 0u;
        FF_TEST_ASSERT(FF_CcsdsDecodeTm(frame, frame_len, &apid, &hdr, &payload, &payload_len) ==
                       FF_SUCCESS);
        FF_TEST_ASSERT(hdr.service == 5u);
        FF_TEST_ASSERT(hdr.subservice == 3u);
        FF_TEST_ASSERT(payload != NULL);
        FF_TEST_ASSERT(payload_len > 0u);
    }
    /* "Exactly one" frame. The loopback pop drains the whole transmit ring, so
     * a second pop would find nothing even after a duplicated emission: the
     * discriminating assertion is therefore the popped length, which must
     * equal exactly one CCSDS space packet. The primary header carries the
     * packet length (total - 1), so the expected frame size is derived from
     * the frame itself. */
    {
        /* The CCSDS primary header carries (packet data length - 1) in bytes
         * 4..5, so the total packet size is that field plus one plus the
         * primary header length. */
        uint16_t packet_field = (uint16_t)(((uint16_t)frame[4] << 8) | (uint16_t)frame[5]);
        uint16_t expected_total = (uint16_t)(packet_field + 1u + FF_CCSDS_HDR_LEN);
        FF_TEST_ASSERT(expected_total == frame_len);
        uint8_t extra[512];
        uint16_t extra_len = 0u;
        FF_TEST_ASSERT(FF_LinkLoopbackPop(extra, sizeof(extra), &extra_len) == FF_TIMEOUT);
        FF_TEST_ASSERT(extra_len == 0u);
    }

    FF_PalSetPanicHandler(NULL);
    FF_PusInstallPanicReporter(NULL);
}

/* Deadline semantics of the host link backends: an already-expired budget
 * must not report success, and a future budget must deliver the frame. This
 * covers the loopback deadline-aware writer that the production panic
 * reporter uses on the host. */
static void test_link_deadline_writer_semantics(void)
{
    const uint8_t payload[3] = {0xA5u, 0x5Au, 0x01u};
    uint8_t rx[64];
    uint16_t rx_len = 0u;

    hs_init_modules();
    FF_TEST_ASSERT(FF_LinkInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkGet() == &FF_LinkLoopbackBackend);

    /* An expired budget is refused. */
    FF_TEST_ASSERT(FF_LinkSendUntil(payload, sizeof(payload), 1u) == FF_TIMEOUT);
    /* Nothing was enqueued by the refused attempt. */
    FF_TEST_ASSERT(FF_LinkLoopbackPop(rx, sizeof(rx), &rx_len) == FF_TIMEOUT);
    FF_TEST_ASSERT(rx_len == 0u);

    /* A future budget delivers the frame. */
    FF_TEST_ASSERT(FF_LinkSendUntil(payload, sizeof(payload),
                                    FF_PalGetMonotonicNs() + 100000000ULL) == FF_SUCCESS);
    FF_TEST_ASSERT(FF_LinkLoopbackPop(rx, sizeof(rx), &rx_len) == FF_SUCCESS);
    FF_TEST_ASSERT(rx_len == sizeof(payload));
    FF_TEST_ASSERT(memcmp(rx, payload, sizeof(payload)) == 0);
}

/* Counterexample characterization, not requirement-coverage credit: the
 * current bounded buffer emits for 96 input characters but suppresses the
 * notifier for 97 and 256 characters. The mandatory-text policy is unresolved;
 * keep this test as observed behavior until authority defines the envelope. */
static void test_panic_overlength_reason_counterexample(void)
{
    char reason[257];
    uint32_t resets_before;

    memset(reason, 'R', sizeof(reason) - 1u);
    reason[sizeof(reason) - 1u] = '\0';
    hs_init_modules();
    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    FF_PalSetPanicNotifier(hs_observe_notifier);
    FF_PalSetPanicHandler(hs_panic_handler);

    g_observed_notifier_calls = 0u;
    if (setjmp(g_panic_jmp) == 0) {
        FF_PalPanic("%.*s", (int)FF_EVS_TEXT_LEN, reason);
    }
    FF_TEST_ASSERT(g_observed_notifier_calls == 1u);
    FF_TEST_ASSERT(g_observed_reason_length == FF_EVS_TEXT_LEN);

    g_observed_notifier_calls = 0u;
    if (setjmp(g_panic_jmp) == 0) {
        FF_PalPanic("%.*s", (int)(FF_EVS_TEXT_LEN + 1u), reason);
    }
    FF_TEST_ASSERT(g_observed_notifier_calls == 0u);

    g_observed_notifier_calls = 0u;
    if (setjmp(g_panic_jmp) == 0) {
        FF_PalPanic("%s", reason);
    }
    FF_TEST_ASSERT(g_observed_notifier_calls == 0u);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == resets_before + 3u);
    FF_PalSetPanicNotifier(NULL);
    FF_PalSetPanicHandler(NULL);
}

/* Out-of-bound: an unknown service 8 function is rejected and must not
 * produce a reset (a rejected command must never reset the vehicle). */
static void test_hs_reset_unknown_function(void)
{
    const uint8_t fn_unknown[1] = {99u};
    uint32_t resets_before;

    hs_init_modules();
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);
    FF_AppHsRegister();
    FF_PusInstallPanicReporter(hs_reset_writer);
    FF_PalSetPanicHandler(hs_panic_handler);

    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    g_reset_frame_len = 0u;
    g_reset_report_count = 0u;
    g_panic_seen = false;

    if (setjmp(g_panic_jmp) == 0) {
        FF_TEST_ASSERT(hs_pus_ingest(8u, 1u, fn_unknown, sizeof(fn_unknown)) == FF_NOT_FOUND);
    }
    else {
        FF_TEST_ASSERT(false); /* must not panic */
    }

    FF_TEST_ASSERT(!g_panic_seen);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == resets_before);
    FF_TEST_ASSERT(g_reset_frame_len == 0u);
    FF_TEST_ASSERT(g_reset_report_count == 0u); /* no report on a rejected command */

    FF_PalSetPanicHandler(NULL);
    FF_EvsSetListener(NULL);
}

/* Out-of-bound: a truncated service 8 command (no function identifier) is
 * rejected and must not produce a reset. */
static void test_hs_reset_missing_function_id(void)
{
    uint32_t resets_before;

    hs_init_modules();
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);
    FF_AppHsRegister();
    FF_PusInstallPanicReporter(hs_reset_writer);
    FF_PalSetPanicHandler(hs_panic_handler);

    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    g_reset_frame_len = 0u;
    g_reset_report_count = 0u;
    g_panic_seen = false;

    if (setjmp(g_panic_jmp) == 0) {
        FF_TEST_ASSERT(hs_pus_ingest(8u, 1u, NULL, 0u) == FF_BAD_SIZE);
    }
    else {
        FF_TEST_ASSERT(false); /* must not panic */
    }

    FF_TEST_ASSERT(!g_panic_seen);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == resets_before);
    FF_TEST_ASSERT(g_reset_frame_len == 0u);    /* no report on a rejected command */
    FF_TEST_ASSERT(g_reset_report_count == 0u); /* no report on a rejected command */

    FF_PalSetPanicHandler(NULL);
    FF_EvsSetListener(NULL);
}

/* Out-of-bound: the pre-reset report fails (writer/reporter error). The reset
 * must still be enforced; the notification failure must never block it. */
static void test_hs_reset_survives_report_failure(void)
{
    uint32_t resets_before;

    hs_init_modules();
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);

    /* Deliberately failing reporter instead of the flight wiring. */
    FF_PalSetPanicNotifier(hs_failing_reporter);
    FF_PalSetPanicHandler(hs_panic_handler);

    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    g_panic_seen = false;

    if (setjmp(g_panic_jmp) == 0) {
        FF_AppHsReset();
        FF_TEST_ASSERT(false); /* must not return normally */
    }

    FF_TEST_ASSERT(g_panic_seen);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == (resets_before + 1u));

    FF_PalSetPanicHandler(NULL);
    FF_PalSetPanicNotifier(NULL);
    FF_EvsSetListener(NULL);
}

/* Out-of-bound: no pre-reset reporter is installed at all (the PAL panic
 * notifier is unset). The reset must still be enforced and no report may be
 * produced: a missing notification must never prevent the reset
 * (SRS-REQ-074 "failure or absence of that notification shall not prevent the
 * reset from being enforced"). */
static void test_hs_reset_survives_missing_notifier(void)
{
    uint32_t resets_before;

    hs_init_modules();
    FF_TEST_ASSERT(FF_PusInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_PusSetBackend(&FF_PusBackendOpen) == FF_SUCCESS);
    FF_PusSetTmSink(NULL);
    FF_AppHsRegister();

    /* No reporter installed: the PAL panic notifier is explicitly unset. */
    FF_PalSetPanicNotifier(NULL);
    FF_PalSetPanicHandler(hs_panic_handler);

    FF_TEST_ASSERT(FF_PalWatchdogInit(8000u) == FF_SUCCESS);
    resets_before = FF_PalWatchdogGetResets();
    g_reset_frame_len = 0u;
    g_reset_report_count = 0u;
    g_panic_seen = false;

    if (setjmp(g_panic_jmp) == 0) {
        FF_AppHsReset();
        FF_TEST_ASSERT(false); /* must not return normally */
    }

    FF_TEST_ASSERT(g_panic_seen);
    FF_TEST_ASSERT(FF_PalWatchdogGetResets() == (resets_before + 1u));
    FF_TEST_ASSERT(g_reset_report_count == 0u); /* no reporter -> no report */
    FF_TEST_ASSERT(g_reset_frame_len == 0u);

    FF_PalSetPanicHandler(NULL);
    FF_PalSetPanicNotifier(NULL);
    FF_EvsSetListener(NULL);
}

FF_TEST_MAIN(test_hs_heartbeat_no_fault, test_es_lifecycle_host_tasks,
             test_es_start_failure_and_hs_detection, test_hs_reset_single_path,
             test_panic_console_stall_and_report_share_one_budget,
             test_panic_production_link_wiring_delivers_one_frame,
             test_link_deadline_writer_semantics, test_panic_overlength_reason_counterexample,
             test_hs_reset_unknown_function, test_hs_reset_missing_function_id,
             test_hs_reset_survives_report_failure, test_hs_reset_survives_missing_notifier)
