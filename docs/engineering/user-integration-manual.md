# User & Integration Manual

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ENG-0005                         |
| Revision                          | 0.2                                    |
| Date                              | 2026-10-10                             |
| Audience                          | humans and agents integrating applications, drivers and stacks on the frick-fsw middleware |

This manual explains how to use frick-fsw as a **middleware**: you write your
own mission applications on top of it, integrate your own hardware drivers, and
optionally plug third-party stacks. Read first `AGENTS.md`, `README.md`,
`docs/development-plan.md` (product boundary and milestones) and
`docs/rules.md` (norms N1-N19).

> **frick-fsw is a framework, not an application.** The six `src/apps/*` are
> reference applications (demonstration and V&V), not mission software.

## 1. The application model

Every application is a **task** registered with Executive Services (ES) and
communicating over the Software Bus (SB). An application:

1. has an entry point `void AppMain(void *arg)` (`FF_PalTaskFunc_t`
   compatible, see `src/apps/app.h`);
2. is described by an `FF_AppConfig_t` (name, entry, stack, priority, queue
   depth; `src/core/es/es.h`);
3. is registered at start-up with `FF_EsRegisterApp(&cfg, &id)`, optionally
   gets a pipe with `FF_SbCreatePipe(id, depth)`, subscribes with
   `FF_SbSubscribe(msg_id, id)`, and is started by `FF_EsStartAll()`.

The reference wiring is in `src/main/fsw_main.c`: it initialises the core
(`FF_PalInit`, `FF_SbInit`, `FF_EvsInit`, `FF_EsInit`, `FF_TblInit`,
`FF_TimeInit`, `FF_PusInit`), registers the providers, then registers and
starts the applications.

### Minimal application

```c
#include "core/es/es.h"
#include "core/sb/sb.h"

static void MyAppMain(void *arg)
{
    (void)arg;
    FF_AppId_t me = FF_EsGetId("MY_APP");
    FF_SbSubscribe(0x1234u, me);          /* your message id(s) */
    for (;;) {
        FF_Msg_t msg;
        if (FF_SbReceive(me, &msg, 1000u) == FF_SUCCESS) {
            /* handle msg.payload[0..msg.length-1] */
        }
    }
}

static const FF_AppConfig_t my_cfg = {
    "MY_APP", MyAppMain, 4096u, 50u, 8u};
/* register: FF_EsRegisterApp(&my_cfg, &id); FF_SbCreatePipe(id, 8u); */
```

## 2. Software Bus (SB) — `src/core/sb/sb.h`

- `FF_SbPublish(const FF_Msg_t *msg)` — publish to all subscribers of
  `msg->msg_id` (wildcard `FF_SB_WILDCARD` = `0xFFFF` matches every id).
- `FF_SbSubscribe` / `FF_SbUnsubscribe` — per-application subscriptions.
- `FF_SbReceive(app, &msg, timeout_ms)` — bounded blocking receive from the
  application pipe (no dynamic allocation; fixed `FF_SB_MSG_PAYLOAD_MAX`).
- `FF_Msg_t`: `{ msg_id, apid, time_ns, length, payload[] }`.

## 3. Events (EVS) — `src/core/evs/evs.h`

- `FF_EvsSend(event_id, type, app, fmt, ...)` — emit an event (formatted
  message); `FF_EvsEvent`/`FF_EvsGetLog`/`FF_EvsGetTotalCount` for the log.
- Register a listener with `FF_EvsSetListener()` to forward events on the bus;
  the reference `TM_OUTPUT` application maps them to PUS service 5 telemetry.

## 4. Tables (TBL) — `src/core/tbl/tbl.h`

`FF_TblRegister`, `FF_TblLoad`, `FF_TblValidate`, `FF_TblActivate`,
`FF_TblRevert`, `FF_TblGet`/`FF_TblDump`. Tables are validated before atomic
activation; register a `FF_TblValidateFn_t` for domain checks.

## 5. PUS providers — `src/pus/pus.h`

Register the data your application exposes to the ground:

- Housekeeping: `FF_PusRegisterHkProvider(id, provider)` (service 3).
- Parameters: `FF_PusRegisterParamProvider` (report) and
  `FF_PusRegisterParamSetter` (set) (service 20).
- Functions: `FF_PusRegisterFunction(id, fn)` (service 8).
- Memory regions: `FF_PusRegisterMemoryRegion(base, size)` (service 6).

The reference `HK` application shows the pattern. To add a **new/standardised
PUS service**, follow `docs/engineering/service-development.md`.

## 6. Time — `src/core/time/time.h`

- `FF_TimeGetMetNs`/`FF_TimeGetMetMs` — mission elapsed time.
- `FF_TimeGetUtcNs` — UTC (correlated); `FF_TimeSetUtcNs` — correlation
  (PUS service 9,1); `FF_TimeSetSource`/`FF_TimeGetSource` — source selection.
- `FF_TimeGetCuc`/`FF_TimeCucEncode`/`FF_TimeCucDecode` — CCSDS CUC timecode.
- A PPS backend (`src/core/time/pps.h`) disciplines the tone; see
  `docs/architecture/time-and-timecode.md`.

## 7. Link and PUS dispatch

- The **link layer** (`src/transport/link/link.h`) moves CCSDS frames over a
  transport (loopback, UART). A new link is a driver: see
  `docs/engineering/driver-development.md`.
- Received frames are fed to `FF_PusIngest(raw, len)`; telemetry is emitted
  through `FF_PusSendTm(...)` and the TM sink. Program against
  `FF_Pus_Backend_t` to swap the PUS product. The **default** backend is the
  N7/ESA ASN.1 one (`FF_PusBackendN7`, build-time generation `make pus-asn1`,
  `FF_USE_PUS_ASN1`); select the open backend explicitly if needed with
  `FF_PusSetBackend(&FF_PusBackendOpen)`.
- Secure links: the CCSDS **SDLS** sub-layer (`src/security/`) wraps the
  telemetry/telecommand with a pluggable cryptographic backend
  (`FF_CryptoBackend_t`); enable/configure it where the mission link requires it.

## 8. Integrating a driver / hardware

All hardware access goes through the **PAL** (OS primitives) and the **HAL**
(bus/device contracts, milestone M3). You implement a backend behind the
contract and register it; the middleware is not modified. The procedure,
obligations (buffer ownership, DMA + cache coherency, ISR/task split, no
dynamic allocation, bounded latency) and the partition/CMake options that let
you include only the drivers you need are in
`docs/engineering/driver-development.md` and `docs/development-plan.md`.

## 9. Integrating a third-party stack (pluggable)

Network stacks are pluggable by design: the middleware talks to a
**stack-independent contract** (`FF_NetStack_t`, M3), not to a specific stack.
To adopt Tuxera TCP/IP, lwIP, RTEMS libbsd or your own, implement the contract
in an adapter; no core change is required. The same pluggable-backend principle
applies to CAN/CANopen, TSN and storage backends (see
`docs/development-plan.md` §3).

## 10. Build, test, verify

```bash
make configure build test    # host simulator + unit tests (ctest)
make verify-lean             # Lean formal verification (norm N1)
make trace                   # traceability convergence (norm N4)
make coverage metrics        # coverage + README status panel
make ci-local                # the whole cycle in the pinned Docker image
```

Register a new test in `tests/CMakeLists.txt`; add requirement links in
`docs/requirements/traceability.yaml` (norm N3/N4).

## 11. Contribution checklist (encoded in the norms)

1. English only (N2). 2. Requirements + traceability updated (N3/N4).
3. Lean model for behaviour-bearing code (N1). 4. Tests (N11).
5. Docs updated (N8), revision history (N16). 6. Full cycle green (N15).
7. Independent reviews R1/R2/R3 and documentation review R4, NCs closed
(N9/N10/N12/N14/N17). 8. Commit and push both remotes (N7).

## 12. Mathematics (libm support)

Applications that need floating-point mathematics (`sqrt/sin/cos/tan/atan2/pow/
exp/log/fabs/fmod`) use the portable facade `src/math/ff_math.h`
(`FF_MathSqrt`, `FF_MathSin`, ... plus `FF_MathDeg2Rad`, `FF_MathRad2Deg`,
`FF_MathClamp`, `FF_MathIsFinite`). The framework core uses integer arithmetic
only. Out-of-domain arguments follow the platform `libm` (`sqrt(x<0)=NaN`,
`log(x<=0)=-inf`); test results with `FF_MathIsFinite()` when a defined fallback
is required, and do not use transcendentals in hard real-time critical paths
without a WCET/stack analysis. See `docs/architecture/numerics.md` and ADR-0012.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial user & integration manual (OBJ-02, milestone M3) |
| 0.2 | 2026-10-10 | frick-fsw | M6: add mathematics (libm) support section |
