# Test Specification TS-5 — Core services (Software Bus, Tables, Events, Executive, scheduler, HS/FDIR, math)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-TS5                     |
| Revision                         | 0.9                                    |
| Date                             | 2026-10-16                             |
| Norms                            | N10, N11, N22                          |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |
| Work-package index               | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-5 |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |
| Scope                            | Core services: Software Bus (`sb`), Table Services (`tbl`), Event Services (`evs`), Executive Services (`es`), the scheduler application, the Health & Safety (HS/FDIR) application, and the mathematics facade (`math`) |
| Baseline                         | Working tree 2026-10-16 (S8-SCH dispatcher increment); previous baseline HEAD `57974d2` read 2026-10-14 |
| Modules under test               | `src/core/sb/sb.c`, `src/core/tbl/tbl.c`, `src/core/evs/evs.c`, `src/core/es/es.c`, `src/core/sched/ff_sched.c`, `src/apps/app_scheduler.c`, `src/apps/app_hs.c`, `src/math/ff_math.c` |
| Interfaces cited                 | `src/core/sb/sb.h`, `src/core/tbl/tbl.h`, `src/core/evs/evs.h`, `src/core/es/es.h`, `src/apps/app.h`, `src/math/ff_math.h`, `src/core/common/ff_types.h`, `config/fsw_config.h`; `docs/architecture/software-bus.md` (FRICK-ARCH-0002), `docs/architecture/applications.md` (FRICK-ARCH-0005), `docs/architecture/numerics.md` (FRICK-ARCH-0009), `docs/architecture/scheduling.md` (FRICK-ARCH-0018) |

> **Authority statement.** This work package defines verification *cases and
> oracles* for the current baseline. It does not assign criticality, accept
> residual risk, qualify tools, close an NC, or approve release/flight
> readiness. Requirement/interface gaps are recorded (never resolved by
> inventing an oracle); disposition and closure require the authorized
> authorities.

## 1. Scope and method

### 1.1 In scope

Cases derived from the requirements in this work package's area that are present
in `docs/requirements/traceability.yaml`:

`SRS-REQ-001` (Software Bus delivery semantics, including the wildcard
subscription), `SRS-REQ-002` (Software Bus FIFO ordering), `SRS-REQ-011` (every
on-board event forwarded to ground as PUS service 5), `SRS-REQ-013` (atomic
table activation), `SRS-REQ-014` (application lifecycle: independent tasks,
Executive Services states, Health & Safety detection of failed applications),
`SRS-REQ-015` (deterministic memory, no dynamic allocation after
initialisation), `SRS-REQ-019` (configuration concentrated in
`config/fsw_config.h` plus mission tables), `SRS-REQ-033` (portable mathematics
facade over `libm`), `SRS-REQ-044` (Renode functional-validation environment,
cited only by the PIL gap case `N22-TS5-67`), and `SRS-REQ-071` (RTEMS-backed,
table-driven schedule dispatch; dispatcher delivered and host-verified, with
mission rates/tolerances/target timing still TBD). SSS parents carried by the same
modules: `SSS-REQ-002` (publish/subscribe bus with configurable pipe depth),
`SSS-REQ-008` (Health & Safety: application health monitoring, heartbeat and a
commandable software reset), `SSS-REQ-009` (table load/validate/activate),
`SSS-REQ-018` (numerical support) and `SSS-REQ-030` (bounded, table-driven
application scheduling).

Out of scope (owned by other work packages): CCSDS/PUS/time (TS-1),
link/SDLS (TS-2), the I/O drivers and HAL seams (TS-3/TS-4) and the
cross-cutting software-only robustness programme (TS-6). `SRS-REQ-008` (service
3 housekeeping) and `SRS-REQ-005` (TC acceptance) are owned by TS-1; `app_hk.c`
appears here only as a fixture of the SB fan-out scenario `N22-TS5-62`.

### 1.2 Class, scope, environment and status conventions

- **Class**: `nominal` = a legal input inside the specified operating envelope
  (this includes legal minimum/maximum boundary values); `OOB` = a
  range-violating, malformed, out-of-envelope, resource-exhausted or
  prohibited-ordering input.
- **Scope**: `individual` = one module surface is under test (the other modules
  are only fixtures); `combined` = the case deliberately asserts coupling across
  two or more modules in one scenario (e.g. ES-created pipes carrying SB
  traffic, EVS through PUS, scheduler through TBL).
- **Env**: `SIL` = native host unit/integration suite (`make test`, ctest);
  `PIL` = Renode emulated PolarFire SoC; `HIL` = target silicon
  (RTPFS160ZT). Per N22 and FRICK-VV-TSPEC-001 §1.3, an emulator establishes
  only what its model represents.
- **Status**: `covered` = the case is specified with an oracle grounded in a
  controlled requirement, architecture or interface clause and is executable in
  the named environment (the test may still have to be added; execution status
  is tracked by the umbrella and `docs/vv/verification-results.md`). `gap` = no
  controlled oracle exists (the requirement/interface is silent, contradictory,
  mission-tailored, or the feature is not delivered), so the case is recorded
  against **NC-EXT-47** and **not** executed to a fabricated oracle.
- **Gap discipline (N22)**: every `OOB` row (and any nominal row) whose expected
  behaviour is not defined by a requirement/architecture/interface clause is
  `gap`; the oracle cell names the open NC/TBD and, where applicable, an
  uncontrolled *observation* (explicitly not an oracle). The Gaps list (§3) maps
  every gap row to an NC-EXT-47 sub-item (FRICK-VV-TSPEC-001 §7).
- **Cross-package boundary**: TS-6 owns the cross-cutting robustness property
  and its oracle (memory safety, overflow, concurrency, fault injection,
  resource bounds, sanitizer legs). Where a TS-5 boundary row touches a surface
  TS-6 already owns, the TS-5 oracle is restricted to the requirement-grounded
  functional contract and the TS-6 case is cited (e.g. SB full-pipe drop and EVS
  error event = `N22-TS6-44`; ES saturation return contract = `N22-TS6-36`;
  scheduler starvation = `N22-TS6-33`; math inverted clamp = `N22-TS6-21`; EVS
  ring wrap = `N22-TS6-7`). Overlapping rows are not duplicated.

### 1.3 Interface clauses used as oracle ground

| Clause | Content used as oracle ground |
| --- | --- |
| `SRS.md` §2 | `SRS-REQ-001` (a published message is copied into the input queue of every pipe whose subscription matches the message id **and** of every wildcard subscription), `SRS-REQ-002` (messages on a single pipe are received in FIFO order) |
| `SRS.md` §4/§5 | `SRS-REQ-011` (every on-board EVS event forwarded to ground as PUS service 5), `SRS-REQ-013` (table activation is atomic; a staged image becomes active only on explicit activate), `SRS-REQ-014` (applications run as independent tasks, ES tracks states, HS detects failed applications), `SRS-REQ-015` (no dynamic allocation after initialisation) |
| `SRS.md` §6 | `SRS-REQ-019` (config concentrated in `config/fsw_config.h` plus mission tables), `SRS-REQ-033` (portable facade over platform `libm`; documented out-of-domain behaviour, e.g. `sqrt(x<0)=NaN`, `log(x<=0)=-inf`; results within a documented tolerance ≤ 1e-9 for the covered inputs) |
| `SRS.md` §6 | `SRS-REQ-071` (allocation-free bounded schedule table dispatching SB messages against monotonic time; runs as an ordinary RTEMS task with no competing preemption scheduler; table images validated before activation and activated atomically; heartbeat/late-missed-slot/dispatch-failure evidence; numeric rates/tolerances overrun response are mission-controlled inputs, TBD) and `SRS-REQ-044` (Renode environment with a middleware test application that emits a checkable result) |
| `SSS.md` §2 | `SSS-REQ-002` (publish/subscribe bus with configurable pipe depth), `SSS-REQ-008` (monitor application health, generate heartbeat events, support a commandable software reset), `SSS-REQ-009` (load/validate/activate of on-board configuration tables), `SSS-REQ-018` (documented portable numerical interface), `SSS-REQ-030` (bounded table-driven SB dispatch; report scheduler liveness and dispatch overruns to health monitoring) |
| FRICK-ARCH-0002 §2 | SB model: ports follow the cFS `0xMMSS` convention, `FF_SB_WILDCARD` (`0xFFFF`) matches any message id; pipes are bounded FIFO queues owned by one subscriber task; **overflow policy** — publish never blocks the publisher; a full pipe drops the new message and raises an EVS error event |
| FRICK-ARCH-0002 §4 | All SB pools are static tables sized at compile time; publish/lock are guarded by the PAL mutex; single-publish losslessness for pipes bounded at `FF_SB_QUEUE_DEPTH_MAX` |
| FRICK-ARCH-0005 §1 | The six reference applications are each an independent task (fixed stack, priority, pipe depth) registered by ES; the current scheduler app is demonstration code (shared last-run timestamp; no controlled table activation, missed-slot handling or scheduler-fault recovery) |
| FRICK-ARCH-0009 §3/§4 | Facade domain behaviour follows platform `libm`; callers requiring a fallback test with `FF_MathIsFinite()`; the facade is reentrant and thread-safe; transcendental functions are not guaranteed bit-for-bit identical across toolchains and are not for hard real-time paths without WCET/stack analysis (GAP-A4) |
| FRICK-ARCH-0018 §1/§3/§5 | The dispatcher is an ordinary ES task; RTEMS retains task selection/preemption; the current stub uses one shared `g_last_run` and ignores Table Services activation/error results; the planned dispatcher validates a fixed-capacity schedule image, activates it atomically through Table Services, and emits missed/late/dispatch-error evidence; all mission rates/tolerances/recovery authority are TBD |
| `sb.h`, `tbl.h`, `evs.h`, `es.h`, `app.h`, `ff_math.h`, `ff_types.h`, `fsw_config.h` | Declared API/contract: status codes (`FF_SUCCESS=0` and the negative set `FF_INVALID_ARGUMENT`/`FF_NO_RESOURCE`/`FF_NOT_FOUND`/`FF_ALREADY_EXISTS`/`FF_BAD_SIZE`/`FF_NOT_READY`/`FF_NAME_TOO_LONG`/`FF_TIMEOUT`/`FF_ERROR`), `FF_Msg_t` shape and `FF_SbStats_t`, the TBL load/validate/activate/revert/dump lifecycle with an optional `NULL` validator and `FF_TBL_INVALID_HANDLE`, EVS "circular on-board log" and optional listener, ES state enum and 1-based app ids, and the bounds `FF_SB_MAX_PIPES`=32, `FF_SB_MAX_SUBSCRIPTIONS`=96, `FF_SB_QUEUE_DEPTH_MAX`=32, `FF_SB_DEFAULT_DEPTH`=16, `FF_SB_WILDCARD`=0xFFFF, `FF_ES_MAX_APPS`=32, `FF_ES_APP_NAME_LEN`=16, `FF_EVS_LOG_DEPTH`=64, `FF_EVS_TEXT_LEN`=96, `FF_TBL_MAX_TABLES`=32, `FF_TBL_MAX_SIZE`=4096 |

## 2. Test cases

Case IDs `N22-TS5-1` … `N22-TS5-76`; groups: `1–15` Software Bus, `16–29`
Table Services, `30–34` Event Services, `35–42` Executive Services, `43–48`
scheduler application, `49–53` Health & Safety (HS/FDIR), `54–61` mathematics
facade, `62–68` combined scenarios and environment-bound gaps, `69–70` core-
services memory determinism and statistics-counter wrap boundaries, `71–73`
SB/EVS bounded lock contention and SRS-REQ-011 forwarding under EVS lock contention;
`74` is the authority-profile event-rate/dispatch/latency case (currently gap/TBD);
`75` is the SB queue-mutex contention case and `76` is the new ES/HS lifecycle +
health-detection host integration harness (S8-VV-FU).

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| N22-TS5-1 | SRS-REQ-001 | `sb.c` exact-match fan-out to every subscribed pipe | nominal | two pipes subscribed to `0x0A01`; publish one message id `0x0A01`, length 2, payload {DE,AD} | one copy in each matching pipe; both `FF_SbReceive` return the same msg_id and length and byte-identical payload; `delivered` increases by 2 (SRS-REQ-001 "copied into the input queue of every pipe whose subscription matches"; Lean `req_srs001_exact_and_wildcard`) | SIL | individual | covered |
| N22-TS5-2 | SRS-REQ-001 | `sb.c` wildcard subscription receives every message id | nominal | one pipe subscribed to `FF_SB_WILDCARD` (`0xFFFF`); publish ids `0x0000`, `0x0A01`, `0xFFFF` in turn | each publish is copied into the wildcard pipe; `FF_SbReceive` returns the matching id and payload for all three (SRS-REQ-001 wildcard clause; FRICK-ARCH-0002 §2 `FF_SB_WILDCARD` matches any id) | SIL | individual | covered |
| N22-TS5-3 | SRS-REQ-001 | `sb.c` no-match publish delivers to no pipe | nominal | pipe subscribed to `0x0A02`; publish id `0x0A03` (never subscribed) | publish returns `FF_SUCCESS` with zero copies (`FF_SbReceive` on the pipe = `FF_TIMEOUT`); `published` increments but `delivered` does not (SRS-REQ-001 "every pipe whose subscription matches"; Lean `req_srs001_no_match_no_delivery`) | SIL | individual | covered |
| N22-TS5-4 | SRS-REQ-002 | `sb.c` FIFO ordering on a single pipe | nominal | one pipe subscribed to `0x0B03`; publish three messages with payload bytes 0,1,2 | the three `FF_SbReceive` calls return payloads in the publish order 0,1,2 (FIFO, SRS-REQ-002; asserted by `test_r3_gap.c` `test_sb_fifo_order`) | SIL | individual | covered |
| N22-TS5-5 | SRS-REQ-002, SSS-REQ-002 | `sb.c` receive from an empty subscribed pipe returns no message | nominal | pipe created and subscribed, queue empty; `FF_SbReceive(app, &msg, 0)` | the call returns no message and leaves the pipe/counters unchanged; the receive code is `FF_TIMEOUT` in the baseline (SRS-REQ-002 bounded FIFO queue is empty; SSS-REQ-002 configurable pipe depth). The exact timeout/deadline contract is owned by TS-6 `N22-TS6-38` | SIL | individual | covered |
| N22-TS5-6 | SRS-REQ-002 | `sb.c` pipe reuse after full drain preserves FIFO (ring wrap) | nominal | depth-2 pipe subscribed to `0x0B04`; publish two, drain two, publish two more, drain two | the second set of receives returns the second set of payloads in order; no stale first-set message resurfaces and no message is lost (FIFO ring semantics; SRS-REQ-002) | SIL | individual | covered |
| N22-TS5-7 | SRS-REQ-001, SSS-REQ-002 | `sb.c` publish to a full pipe never blocks and does not disturb other subscribers | nominal | pipe A filled to `FF_SB_QUEUE_DEPTH_MAX`; pipe B subscribed to the same id and empty; publish one more message with an EVS listener configured | publish returns `FF_SUCCESS` before any listener callback; pipe A retains its buffered messages unchanged, pipe B receives the new message, and the SB error is in the EVS log/notification queue for later dispatch (SRS-REQ-001 per-pipe copy; SSS-REQ-002 bounded queue; FRICK-ARCH-0002 §2). Detailed drop/statistics/event-delivery oracle is owned by TS-6 `N22-TS6-44` | SIL | individual | covered |
| N22-TS5-8 | SRS-REQ-001 | `sb.c` statistics accounting (`FF_SbStats_t`) | nominal | publish N messages to one exact subscriber with no drop; read `FF_SbGetStats` | `published` equals the number of publishes; `delivered` equals the number of copies; `dropped == 0`; the struct fields match `FF_SbStats_t` (declared interface `sb.h`; SRS-REQ-001 delivery) | SIL | individual | covered |
| N22-TS5-9 | SSS-REQ-002, SRS-REQ-019 | `sb.c` pipe registry capacity (`FF_SB_MAX_PIPES` = 32, plus one) | OOB | create pipes for 32 distinct apps, then a 33rd | the first 32 pipes exist and remain usable; the 33rd is not created and no existing pipe is displaced (bounded static pool, `FF_SB_MAX_PIPES` in `config/fsw_config.h` = SRS-REQ-019 single configuration point; SSS-REQ-002). The exact refusal status is not fixed by a clause (the implementation returns `FF_NO_RESOURCE`); it is recorded as an uncontrolled observation, not the oracle | SIL | individual | covered |
| N22-TS5-10 | SRS-REQ-001 | `sb.c` duplicate pipe creation for the same application | OOB | `FF_SbCreatePipe(a, 4)` then `FF_SbCreatePipe(a, 4)` again | **gap** — no requirement/interface clause defines the duplicate-pipe policy or the effect of a rejected second create on the existing pipe. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_ALREADY_EXISTS`, the first pipe unchanged. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-11 | SSS-REQ-002 | `sb.c` pipe depth 0 and depth above `FF_SB_QUEUE_DEPTH_MAX` | OOB | `FF_SbCreatePipe(a, 0)`; `FF_SbCreatePipe(b, FF_SB_QUEUE_DEPTH_MAX + 1)` | **gap** — SSS-REQ-002 requires a configurable pipe depth but no clause defines the out-of-range/zero depth policy (clamp, refuse or default). Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: both fall back to `FF_SB_DEFAULT_DEPTH` (16). No oracle is asserted here | SIL | individual | gap |
| N22-TS5-12 | SRS-REQ-001 | `sb.c` `FF_SbPublish`/`FF_SbReceive`/`FF_SbGetStats` with a NULL pointer | OOB | `FF_SbPublish(NULL)`; `FF_SbReceive(app, NULL, 0)`; `FF_SbGetStats(NULL)` | **gap** — no SRS/architecture/interface clause defines the NULL-argument contract of the SB API. Open TBD (NC-EXT-47 §7.2/§7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT` for all three. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-13 | SRS-REQ-001 | `sb.c` subscribe/unsubscribe to a pipe that does not exist | OOB | `FF_SbSubscribe(0x0010, 3)` with no pipe for app 3; `FF_SbUnsubscribe(0x0010, 3)` | **gap** — the delivery clause covers only matching subscriptions of existing pipes; no clause fixes the return contract for a subscription referencing an unregistered subscriber. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NOT_FOUND`. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-14 | SRS-REQ-001 | `sb.c` one pipe holding both an exact and a wildcard subscription | OOB | pipe subscribed to `0x0B05` and to `FF_SB_WILDCARD`; publish id `0x0B05` | **gap** — SRS-REQ-001 requires a copy for every matching subscription "and of every wildcard subscription", but does not state whether a pipe that holds both receives one copy or two. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: two copies (one per subscription row). No oracle is asserted here | SIL | individual | gap |
| N22-TS5-15 | SRS-REQ-001 | `sb.c` SB operations before `FF_SbInit` | OOB | `FF_SbCreatePipe`/`FF_SbPublish`/`FF_SbReceive` before `FF_SbInit` | **gap** — no clause defines the pre-initialisation contract of the SB API (nor bounded, idempotent re-init). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: the API reports not-ready / invalid. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-16 | SRS-REQ-013, SSS-REQ-009 | `tbl.c` register then `FF_TblGet` returns the active image | nominal | register a table with name, image and size; `FF_TblGet(h, &size)` | register returns a valid handle (`!= FF_TBL_INVALID_HANDLE`); `FF_TblGet` returns the registered image pointer and the registered size (SRS-REQ-013; SSS-REQ-009 load/validate/activate; `tbl.h`) | SIL | individual | covered |
| N22-TS5-17 | SRS-REQ-013 | `tbl.c` load stages but does not activate (active image unchanged) | nominal | load a valid image different from the active bytes; `FF_TblGet` before activate | `FF_TblLoad` returns `FF_SUCCESS`; `FF_TblGet` still returns the original active image (staging is isolated until activate) (SRS-REQ-013 "a staged image becomes active only on explicit activate") | SIL | individual | covered |
| N22-TS5-18 | SRS-REQ-013 | `tbl.c` activate commits the staged image atomically | nominal | load a valid image, then `FF_TblActivate(h)`; `FF_TblGet` after | activate returns `FF_SUCCESS` and `FF_TblGet` then returns the staged bytes in full (atomic commit; SRS-REQ-013; `test_tbl.c` `test_load_activate`) | SIL | individual | covered |
| N22-TS5-19 | SRS-REQ-013 | `tbl.c` a second staged load does not disturb the active image | nominal | after an activate, load a second valid image; read `FF_TblGet` before activating it | the active image retains the first committed bytes until the second load is explicitly activated (SRS-REQ-013 atomicity) | SIL | individual | covered |
| N22-TS5-20 | SRS-REQ-013 | `tbl.c` revert discards the staged image and clears "staged" | nominal | load a valid image; `FF_TblRevert(h)`; `FF_TblActivate(h)` | revert returns `FF_SUCCESS`, staging is re-seeded from the active image, and a subsequent activate returns `FF_NOT_READY` (nothing is staged) (SRS-REQ-013 explicit-activate; `test_r2_gap.c` `test_tbl_errors_and_revert`) | SIL | individual | covered |
| N22-TS5-21 | SRS-REQ-013, SRS-REQ-019 | `tbl.c` `FF_TblDump` returns the active image and its exact size | nominal | activate a known image; `FF_TblDump(h, out, &len)` with `len` = table size | dump returns `FF_SUCCESS`; `*len` equals the table size and `out` holds the active bytes (never the uncommitted staged image) (SRS-REQ-013; `tbl.h` `FF_TblDump`) | SIL | individual | covered |
| N22-TS5-22 | SSS-REQ-009, SRS-REQ-019 | `tbl.c` maximum-size image (`FF_TBL_MAX_SIZE` = 4096) round trip | nominal | register a table of exactly 4096 bytes; load a full 4096-byte image; activate; dump | load/activate/dump all return `FF_SUCCESS` and the dumped 4096 bytes equal the staged image (bounded by `FF_TBL_MAX_SIZE` in `config/fsw_config.h` = SRS-REQ-019; SSS-REQ-009) | SIL | individual | covered |
| N22-TS5-23 | SSS-REQ-009, SRS-REQ-019 | `tbl.c` table registry capacity (`FF_TBL_MAX_TABLES` = 32, plus one) | OOB | register 32 tables, then a 33rd | the first 32 handles are valid and remain usable; the 33rd registration is refused with `FF_TBL_INVALID_HANDLE` (bounded static registry, `FF_TBL_MAX_TABLES` = SRS-REQ-019 config point; SSS-REQ-009; `test_r2_gap.c` `test_tbl_capacity`) | SIL | individual | covered |
| N22-TS5-24 | SSS-REQ-009, SRS-REQ-019 | `tbl.c` registration size boundary (exact max and max+1) | OOB | register with size `FF_TBL_MAX_SIZE` (4096) then with size `FF_TBL_MAX_SIZE + 1` | the exact-max registration returns a valid handle; the over-max registration returns `FF_TBL_INVALID_HANDLE` and no slot is consumed (bounded by `FF_TBL_MAX_SIZE` = SRS-REQ-019; SSS-REQ-009) | SIL | individual | covered |
| N22-TS5-25 | SRS-REQ-013 | `tbl.c` load with a wrong length is rejected | OOB | `FF_TblLoad(h, src, size - 1)` and `FF_TblLoad(h, src, size + 1)` | both return `FF_BAD_SIZE`; the active image and the "staged" flag are unchanged (size must equal the registered table size before validation; SRS-REQ-013; `tbl.h`) | SIL | individual | covered |
| N22-TS5-26 | SRS-REQ-013 | `tbl.c` activate before any load returns `FF_NOT_READY` | OOB | activate a freshly registered table with nothing staged | activate returns `FF_NOT_READY` and does not modify the active image (staged image becomes active only on explicit activate of a validated staged image; SRS-REQ-013) | SIL | individual | covered |
| N22-TS5-27 | SRS-REQ-013 | `tbl.c` validator rejection invalidates the staged image (rejected image never committed) | OOB | register with a rejecting validator; load a rejected image; then `FF_TblActivate` | load returns `FF_ERROR`; the staged image is invalidated so activate returns `FF_NOT_READY` and the previously active image is never overwritten by the rejected bytes (SRS-REQ-013; regression `FRICK-REVIEW-0077`; `test_r3_gap.c` `test_tbl_atomic_activate_revert`) | SIL | individual | covered |
| N22-TS5-28 | SSS-REQ-009 | `tbl.c` registration with an incomplete definition | OOB | `FF_TblRegister(NULL)`; def with `name = NULL`; `image = NULL`; `size = 0` | **gap** — SSS-REQ-009 requires load/validate/activate but no clause fixes the registration-rejection contract for an incomplete definition. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_TBL_INVALID_HANDLE` for all four. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-29 | SRS-REQ-013 | `tbl.c` invalid handle and invalid dump arguments | OOB | `FF_TblLoad(0xFFFE, …)`; `FF_TblActivate(0xFFFE)`; `FF_TblRevert(0xFFFE)`; `FF_TblGet(0xFFFE, NULL)`; `FF_TblDump(h, NULL, &len)`; `FF_TblDump(h, out, NULL)`; `FF_TblDump(h, out, &len)` with `len < size` | **gap** — no requirement/interface clause defines the invalid-handle or invalid-argument return contract of the TBL API, nor the undersized-dump outcome. Open TBD (NC-EXT-47 §7.2/§7.3). Uncontrolled observation only: `FF_NOT_FOUND` / `NULL` / `FF_INVALID_ARGUMENT` / `FF_BAD_SIZE`. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-30 | SRS-REQ-011 | `evs.c` ordinary event delivered synchronously to the listener with all fields | nominal | `FF_EvsSetListener(l)`; `FF_EvsSend(0x1234, FF_EVS_WARNING, 7, "x=%u", 42)` | before `FF_EvsSend` returns, the listener is invoked exactly once with `event_id = 0x1234`, `type = FF_EVS_WARNING`, `app_id = 7`, `text = "x=42"`; the send returns `FF_SUCCESS` (SRS-REQ-011 event record forwarded; `evs.h` optional listener; `test_r2_gap.c` `test_evs_listener_and_ring`) | SIL | individual | covered |
| N22-TS5-31 | SRS-REQ-011 | `evs.c` every severity is forwarded once and the total count increments | nominal | send one event of each `FF_EVS_INFO`/`WARNING`/`ERROR`/`CRITICAL`; `FF_EvsGetTotalCount` | the listener receives exactly one call per event (4 total); `FF_EvsGetTotalCount` increases by 4 and the log retains the events (SRS-REQ-011 "every on-board event"; `evs.h`) | SIL | individual | covered |
| N22-TS5-32 | SRS-REQ-011 | `evs.c` log returns retained events in order and honours `max_events` | nominal | read the log with `max_events` smaller than, equal to and larger than the number of retained events | `FF_EvsGetLog` returns at most `max_events` events in the order they were sent, and the count returned equals the number of entries actually copied (SRS-REQ-011; `evs.h` "circular on-board log"; `test_r3_gap.c` `test_pus_event_s5_propagation`) | SIL | individual | covered |
| N22-TS5-33 | SRS-REQ-011 | `evs.c` event text at the `FF_EVS_TEXT_LEN` boundary | nominal | send an event whose formatted text is exactly `FF_EVS_TEXT_LEN - 1` characters, then one character shorter | both are logged as NUL-terminated strings of the exact expected length and forwarded to the listener unchanged (SRS-REQ-011; `FF_EVS_TEXT_LEN` = 96 boundary in `config/fsw_config.h`) | SIL | individual | covered |
| N22-TS5-34 | SRS-REQ-011 | `evs.c` API before init, NULL arguments and over-bound text | OOB | `FF_EvsGetLog(NULL, n)`; `FF_EvsSend` before `FF_EvsInit`; `FF_EvsSend` with `fmt = NULL`; text longer than the buffer | **gap** — `evs.h` declares the API but no clause fixes the pre-init, NULL-destination/NULL-format or text-truncation contract of EVS. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `0u` returned, truncation to the buffer. No oracle is asserted here. The bounded-ring overwrite property is owned by TS-6 `N22-TS6-7` and `N22-TS6-8` | SIL | individual | gap |
| N22-TS5-35 | SRS-REQ-014 | `es.c` register a valid application (1-based id, state LOADED, name resolvable) | nominal | register an app config `{name, entry, stack_bytes, priority, queue_depth}` | register returns `FF_SUCCESS`; the first app id is `1` and ids are consecutive 1-based; `FF_EsGetState(id)` = `FF_APP_LOADED`; `FF_EsGetId(name)` returns `id`; `FF_EsGetAppCount` increments (SRS-REQ-014; `es.h`; `test_r2_gap.c` `test_es_registry_and_lifecycle`) | SIL | individual | covered |
| N22-TS5-36 | SRS-REQ-014, SRS-REQ-019 | `es.c` application name length boundary (`FF_ES_APP_NAME_LEN` = 16) | nominal | register with a 15-character name, then with a 16-character name | the 15-character name is accepted; the 16-character name (including NUL, `>= FF_ES_APP_NAME_LEN`) is refused with `FF_NAME_TOO_LONG` and no app slot is consumed (bounded name field, `FF_ES_APP_NAME_LEN` in `config/fsw_config.h` = SRS-REQ-019; SRS-REQ-014; `test_r2_gap.c`) | SIL | individual | covered |
| N22-TS5-37 | SRS-REQ-014, SSS-REQ-008 | `es.c` start an application task and move it to RUNNING | nominal | register a valid app, then `FF_EsStartApp(id)` | start returns `FF_SUCCESS`; `FF_EsGetState(id)` = `FF_APP_RUNNING`; the app task is created independently and an `FF_EVS_APP_STARTED` info event is raised (SRS-REQ-014 "applications run as independent tasks"; SSS-REQ-008; `es.c`) | SIL | individual | covered |
| N22-TS5-38 | SRS-REQ-014 | `es.c` starting an already-RUNNING application is idempotent | nominal | `FF_EsStartApp(id)` twice for the same app | the second call returns `FF_SUCCESS` and the app remains `FF_APP_RUNNING` with no second task created (SRS-REQ-014; `test_r2_gap.c`) | SIL | individual | covered |
| N22-TS5-39 | SRS-REQ-014 | `es.c` `FF_EsStartAll` starts every registered application independently | nominal | register two apps; `FF_EsStartAll()` | returns `FF_SUCCESS` and both apps reach `FF_APP_RUNNING` as separate tasks (SRS-REQ-014 independent tasks / SS-REQ-008) | SIL | individual | covered |
| N22-TS5-40 | SRS-REQ-014 | `es.c` state progression LOADED → RUNNING is observable | nominal | read `FF_EsGetState` before and after start | the state is `FF_APP_LOADED` before start and `FF_APP_RUNNING` after, for each registered app (SRS-REQ-014 "Executive Services shall track states") | SIL | individual | covered |
| N22-TS5-41 | SRS-REQ-014, SRS-REQ-019 | `es.c` application registry capacity (`FF_ES_MAX_APPS` = 32, plus one) | OOB | register 32 apps, then a 33rd | the registry does not exceed `FF_ES_MAX_APPS` and the 32 existing apps are unchanged; the 33rd registration adds no app and does not displace an existing one (bounded registry, `FF_ES_MAX_APPS` = SRS-REQ-019 config point; SRS-REQ-014). The exact refusal status and state transition are not fixed by a clause and are owned by TS-6 `N22-TS6-36` | SIL | individual | covered |
| N22-TS5-42 | SRS-REQ-014 | `es.c` invalid configuration, unknown id and invalid lookup | OOB | `FF_EsRegisterApp(NULL, &id)`; cfg with `name = NULL`; `entry = NULL`; `FF_EsStartApp` for a never-registered id; `FF_EsGetState(0xFFFD)`; `FF_EsGetId(NULL)`; `FF_EsGetId("missing")` | **gap** — SRS-REQ-014 requires registration/lifecycle tracking but no clause fixes the invalid-argument, unknown-id or invalid-lookup return contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_INVALID_ARGUMENT` / `FF_NOT_FOUND` / `FF_APP_FAILED` / `FF_INVALID_ID`. No oracle is asserted here. The failed-start/rollback path is owned by TS-6 `N22-TS6-37` | SIL | individual | gap |
| N22-TS5-43 | SRS-REQ-071, SSS-REQ-030 | `ff_sched.c` schedule-table image validation (magic, size, entry count ≤ 16, payload bound) | nominal | a schedule table with `magic = FF_SCHED_MAGIC`, `count <= FF_SCHED_MAX_ENTRIES` (16), `payload_len <= FF_SCHED_PAYLOAD_MAX`, and size equal to `sizeof(FF_SchedTable_t)`; also the zero-entry and full-capacity images; run `FF_SchedValidate` | the validator accepts each well-formed bounded image (`FF_SUCCESS`) (SRS-REQ-071 "allocation-free, bounded schedule table … validated before activation"; SSS-REQ-030 bounded table-driven service). Evidence: `tests/unit/test_sched.c` `test_validate_rejects` (accept path) | SIL | individual | covered |
| N22-TS5-44 | SRS-REQ-071 | `ff_sched.c` rejected schedule-table images | OOB | images with NULL pointer, `size` ± 1, wrong magic, `count = FF_SCHED_MAX_ENTRIES + 1`, zero period, `payload_len > FF_SCHED_PAYLOAD_MAX`, and duplicate `msg_id` | each image is rejected (`FF_ERROR`) and no entry of a rejected image is ever dispatched (bounded schedule image, SRS-REQ-071). Evidence: `tests/unit/test_sched.c` `test_validate_rejects` | SIL | individual | covered |
| N22-TS5-45 | SRS-REQ-071 | `ff_sched.c` validated-before-activation and atomic schedule-table activation | nominal | activate an invalid image, then a valid one; observe `FF_SchedActiveCount` and the rejection counter | activation validates first: an invalid image leaves the previously active table untouched (count unchanged) and increments `invalid_activations`; a valid image becomes active (`FF_SchedActiveCount` = new count) (SRS-REQ-071 "validated before activation and activated atomically"; SSS-REQ-030). Evidence: `tests/unit/test_sched.c` `test_activate_atomic`. **Scope limit:** atomicity is established for the single-writer double-buffer publish only; cross-core/interrupt memory ordering is not established and is deferred to the multicore work (M14) | SIL | individual | covered |
| N22-TS5-46 | SRS-REQ-071, SSS-REQ-030 | `ff_sched.c` table-driven dispatch of configured SB messages (injected clock) | nominal | run the dispatcher with three entries (1000/5000/10000 ms) over 30 s of injected time; also poll at 137 ms (drift) and across a 4500 ms jump (missed slots) | fixed results with explicit expected values: 30/6/3 releases over 30 s with zero misses (per-entry absolute targets, no starvation — NC-EXT-19 corrected); 10 releases in 10.001 s for the 137 ms case (no cumulative drift); a 4500 ms jump on a 1500 ms entry yields exactly one release plus 3 counted missed slots and no catch-up burst; `max_out` bounds releases per poll (SRS-REQ-071/SSS-REQ-030). Evidence: `tests/unit/test_sched.c` (`test_no_starvation`, `test_no_cumulative_drift`, `test_first_release_boundary`, `test_missed_slots_bounded`, `test_max_out_bound`, `test_null_out_allowed`, `test_real_sb_delivery`). **Scope limit:** host execution with an injected clock; the *mission* rates/tolerances remain TBD (row N22-TS5-48) and target/RTEMS timing evidence is not available | SIL | individual | covered |
| N22-TS5-47 | SRS-REQ-071, SSS-REQ-030 | `app_scheduler.c` / `ff_sched.c` heartbeat, late/missed-slot and dispatch-failure evidence | nominal | force a late release and an SB publish failure and observe the health-monitoring path | **partial** — the mechanism is delivered: `FF_SchedGetStats` exposes release/`missed`/`late`/`publish_errors` counters (asserted in host test: `test_missed_slots_bounded`, `test_no_starvation`), and `app_scheduler.c` emits EVS heartbeat/missed-slot/dispatch-failure events (`0x0601`–`0x0605`). The host test verifies the counter semantics but **not** the EVS emission to the Health & Safety listener, which requires the running application set (integration/simulator smoke). No end-to-end health-path oracle is asserted here; M17/S8-SCH follow-up. NC-EXT-19 remains open pending independent review | SIL | individual | gap |
| N22-TS5-48 | SRS-REQ-071 | `app_scheduler.c` mission-controlled rates, timing tolerances and overrun response | nominal | apply a mission rate/jitter/overrun profile | **gap** — SRS-REQ-071 states numeric rates, timing tolerances and the operational response to overruns are mission-controlled inputs, TBD until an approved profile exists. Open TBD (NC-EXT-47 §7.3; FRICK-ARCH-0018 §1/§5). No oracle is asserted here | SIL | individual | gap |
| N22-TS5-49 | SSS-REQ-008, SRS-REQ-014 | `app_hs.c` periodic heartbeat event | nominal | run `FF_AppHsStep` (one monitoring pass) and observe EVS over at least one heartbeat period | the HS monitoring pass emits a heartbeat event (id `0x0502`, `FF_EVS_INFO`) once per pass (independent EVS-listener oracle), and remains alive (SSS-REQ-008 heartbeat; SRS-REQ-014; `app_hs.c` / `FF_AppHsStep`; host integration harness `tests/unit/test_es_hs_lifecycle.c`). The 5000 ms cadence itself follows from the `FF_PalTaskDelayMs(5000u)` loop and is not re-timed here; cadence vs watchdog window remains `gap` (N22-TS5-52) | SIL | individual | covered |
| N22-TS5-50 | SSS-REQ-008 | `app_hs.c` commandable software reset registered as PUS S8 function #1 | nominal | start HS; invoke PUS service 8 function id 1 | HS registers `ff_hs_fn_reset` as PUS S8 function 1 and invoking it performs the on-board reset action (`FF_PalPanic`) as the commandable software reset (SSS-REQ-008; `app_hs.c`). The harness must stub/intercept `FF_PalPanic` so the reset action is observable without ending the run | SIL | individual | covered |
| N22-TS5-51 | SRS-REQ-014, SSS-REQ-008 | `app_hs.c` detects a FAILED application and reports it | OOB | inject a PAL task-create failure to place an application into `FF_APP_FAILED`, then run one HS monitoring pass (`FF_AppHsStep`) | HS observes `FF_EsGetState(id) == FF_APP_FAILED` and raises exactly one `FF_EVS_APP_ERROR` critical event per pass naming the failed id (SRS-REQ-014 "Health & Safety shall detect failed applications"; SSS-REQ-008); the RUNNING application is not reported (no false positive). Independent EVS-listener oracle in the host integration harness `tests/unit/test_es_hs_lifecycle.c` (S8-VV-FU, closes the `GAP-R3-1` simulator-smoke gap). Detection latency/recovery budget is mission-controlled and owned by TS-6 `N22-TS6-41`/`N22-TS6-31` | SIL | individual | covered |
| N22-TS5-52 | SSS-REQ-008 | `app_hs.c` heartbeat cadence versus the external watchdog window | nominal | stop/lose the HS heartbeat and observe the external watchdog reaction | **gap** — SSS-REQ-008 requires a heartbeat but no clause fixes the missed-kick escalation latency or the external watchdog window; HS uses a 5000 ms cadence and the watchdog is external (RTEMS/monitor), TBD. Open TBD (NC-EXT-47 §7.5; GAP-A5). No oracle is asserted here | SIL | individual | gap |
| N22-TS5-53 | SRS-REQ-014 | `app_hs.c` treatment of non-RUNNING, non-FAILED applications | OOB | an application in `FF_APP_STOPPED` or `FF_APP_LOADED` during an HS loop | **gap** — SRS-REQ-014 requires detecting *failed* applications; no clause defines whether a STOPPED or LOADED application is a health anomaly for HS. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: HS flags only `FF_APP_FAILED`. No oracle is asserted here | SIL | individual | gap |
| N22-TS5-54 | SRS-REQ-033, SSS-REQ-018 | `math` facade elementary functions within tolerance | nominal | `FF_MathSqrt(4)`, `FF_MathSin/Cos/Tan(0)`, `FF_MathTan(π/4)`, `FF_MathPow(2,10)`, `FF_MathExp(0)`, `FF_MathLog(1)`, `FF_MathFabs(-3.5)`, `FF_MathFmod(7,3)` | each result is within the documented tolerance of the reference value (≤ 1e-9 for the covered inputs) (SRS-REQ-033 "within the accuracy of the platform libm … tolerance (<= 1e-9)"; SSS-REQ-018; `test_math.c`) | SIL | individual | covered |
| N22-TS5-55 | SRS-REQ-033 | `math` facade angle helpers and in-range clamp | nominal | `FF_MathDeg2Rad(180)`, `FF_MathRad2Deg(π)`, `FF_MathAtan2(1,1)`, `FF_MathClamp(0.5,0,1)` | `Deg2Rad(180) = π`, `Rad2Deg(π) = 180`, `Atan2(1,1) = π/4` (within tolerance), and the in-range clamp returns the input unchanged (SRS-REQ-033 helpers; `ff_math.h`) | SIL | individual | covered |
| N22-TS5-56 | SRS-REQ-033 | `math` facade clamp saturation boundaries | nominal | `FF_MathClamp(-1, 0, 1)`; `FF_MathClamp(5, 0, 1)`; `FF_MathClamp(0, 0, 1)`; `FF_MathClamp(1, 0, 1)`; `FF_MathClamp(x, lo, lo)` | below `lo` returns `lo`, above `hi` returns `hi`, exactly `lo`/`hi` return `lo`/`hi` (saturation at the boundaries) (SRS-REQ-033 clamp helper; `ff_math.h`) | SIL | individual | covered |
| N22-TS5-57 | SRS-REQ-033 | `math` facade documented out-of-domain results and finiteness | nominal | `FF_MathSqrt(-1)`; `FF_MathLog(0)`; `FF_MathLog(-1)`; `FF_MathIsFinite(NaN)`; `FF_MathIsFinite(+inf)`; `FF_MathIsFinite(1.0)` | `sqrt(x<0) = NaN`; `log(0) = -inf` (and `log(x<0) = NaN`); `FF_MathIsFinite` returns 0 for NaN/+inf and 1 for a finite value (SRS-REQ-033 "documented behaviour for out-of-domain arguments"; `ff_math.h`; FRICK-ARCH-0009 §3) | SIL | individual | covered |
| N22-TS5-58 | SRS-REQ-033 | `math` facade passes NaN through the clamp unclamped | nominal | `FF_MathClamp(FF_MathSqrt(-1), 0, 1)` | the result is NaN (a NaN input is not clamped), so callers must test with `FF_MathIsFinite` (SRS-REQ-033 documented behaviour; `ff_math.h` "a NaN x is not clamped"; `test_r2_gap.c` `test_math_nan_inf`) | SIL | individual | covered |
| N22-TS5-59 | SRS-REQ-033 | `math` facade overflow/underflow follows platform `libm` | nominal | `FF_MathExp(1000)`; `FF_MathExp(-1000)`; `FF_MathPow(0, -1)`; `FF_MathFabs(-inf)` | out-of-range results follow IEEE-754/`libm`: `+inf` / `0` / `+inf` (or NaN) signalled through the value itself; `errno` is not relied upon (SRS-REQ-033; FRICK-ARCH-0009 §3/§4) | SIL | individual | covered |
| N22-TS5-60 | SRS-REQ-033 | `math` facade links `libm` and enforces the build-time guard | nominal | compile `ff_math.c`/`ff_math.h` with `FRICK_USE_LIBM=1` and observe the guard when it is not defined | the facade builds and links `libm` with `FRICK_USE_LIBM=1`; without that macro the header emits a compile error (`#error`), and the core does not call `libm` (SRS-REQ-033 "shall link libm"; FRICK-ARCH-0009 §2/§3) | SIL | individual | covered |
| N22-TS5-61 | SRS-REQ-033 | `math` facade on-target FPU determinism and WCET/stack budget | nominal | on the RTPFS160ZT: run the transcendentals and measure bit patterns, WCET and stack | **gap** — FRICK-ARCH-0009 §4 states transcendental results are not guaranteed bit-for-bit identical across toolchains and shall not be used in hard real-time paths without WCET/stack and error analysis; that analysis and the on-target FPU context behaviour are GAP-A4/GAP-A5 with no controlled oracle. Open TBD (NC-EXT-47 §7.5). No oracle is asserted here | HIL | individual | gap |
| N22-TS5-62 | SRS-REQ-001, SRS-REQ-014 | SB over ES-created pipes: an ES-registered application receives a published message | nominal | `FF_EsRegisterApp` (which creates the app pipe) for two apps; subscribe; publish from one app; receive on the other | the ES-created pipe carries the message: `FF_SbReceive` on the subscriber returns the published message (ES registers each app with a pipe, `es.c`; SRS-REQ-001 delivery; SRS-REQ-014 independent tasks) | SIL | combined | covered |
| N22-TS5-63 | SRS-REQ-011 | EVS event forwarded end-to-end to PUS service 5 | nominal | wire the EVS listener to `FF_PusEmitEvent` as in `fsw_main.c`; raise one event of each severity | each EVS event becomes a PUS service 5 telemetry packet whose payload carries the event id and type (SRS-REQ-011; the PUS S5 encoding itself is owned by TS-1) | SIL | combined | covered |
| N22-TS5-64 | SRS-REQ-014, SSS-REQ-008 | ES start plus HS detection of the same application set | OOB | register and start two apps; force one to `FF_APP_FAILED`; run an HS loop | ES reports `FF_APP_RUNNING` for the healthy app and `FF_APP_FAILED` for the failed app in the same run, and HS raises the critical event for the failed id (SRS-REQ-014 state tracking + HS detection; SSS-REQ-008) | SIL | combined | covered |
| N22-TS5-65 | SRS-REQ-014, SRS-REQ-001 | core-service initialisation order with ES-created pipes | nominal | initialise EVS, TBL, SB and ES; register apps (each creating a pipe); start all; publish | initialisation order is honoured with no resource clash; every registered app has a working pipe after `FF_EsStartAll` (SRS-REQ-014 lifecycle; SRS-REQ-001 pipes; FRICK-ARCH-0005 §1 app set) | SIL | combined | covered |
| N22-TS5-66 | SRS-REQ-071, SRS-REQ-013 | scheduler dispatch from a validated, atomically-activated schedule table | nominal | deploy a schedule image through Table Services, activate it, and let the scheduler dispatch its entries onto the SB | **gap** — SRS-REQ-071 requires the dispatcher to consume a validated, atomically-activated table; the app now registers the schedule image with Table Services and wires `FF_SchedValidate` as the validator, and activates it atomically through `FF_SchedActivate`, but the TBL-driven operational flow (`FF_TblLoad`/`FF_TblActivate` → dispatcher re-activation and dispatch coupling) is not delivered. Open NC-EXT-19 / M17 (NC-EXT-47 §7.3). No oracle is asserted here | SIL | combined | gap |
| N22-TS5-67 | SRS-REQ-044, SRS-REQ-014 | core services on the emulated PolarFire SoC (Renode) | nominal | Renode `polarfire-soc` run of `app_renode_test` exercising the core services | **gap** — SRS-REQ-044 requires a checkable result per change, but the `RENODE_TEST: PASS` aggregate marker is not a per-requirement oracle and no per-check core-service observable is emitted. Open NC-EXT-47 §7.6. No oracle is asserted here | PIL | combined | gap |
| N22-TS5-68 | SRS-REQ-014, SRS-REQ-071 | core services on target silicon (HIL) | nominal | RTPFS160ZT target: application start, heartbeats, SB/EVS/TBL operation under RTEMS 6.2, watchdog/reset path | **gap** — on-silicon timing, watchdog escalation, scheduler liveness and HS reset behaviour require flight hardware; target evidence remains GAP-A5 (NC-EXT-31). Open TBD (NC-EXT-47 §7.5). No oracle is asserted here | HIL | combined | gap |
| N22-TS5-69 | SRS-REQ-015 | core services stay allocation-free after initialisation | nominal | initialise EVS, TBL, SB and ES; register and start the app set; run a bounded publish/load/activate/event workload; observe the process heap via an interposed allocator | no heap allocation occurs during or after the workload — every SB pipe/queue, the TBL registry and the EVS log are static tables sized at compile time, and the allocation counter is unchanged after `FF_EsStartAll` (SRS-REQ-015 "shall not allocate dynamic memory after initialisation"; FRICK-ARCH-0002 §4 static pools; FRICK-ARCH-0009 §4 facade is allocation-free). The sanitizer/leak leg over the whole registered ctest set is owned by TS-6 `N22-TS6-5` | SIL | individual | covered |
| N22-TS5-70 | SRS-REQ-001 | `sb.c` statistics counters at the unsigned-wrap boundary | OOB | drive the `FF_SbStats_t` counters `published`/`delivered`/`dropped` to the `UINT32_MAX` boundary and publish once more (analysis / white-box seeding) | **gap** — SRS-REQ-001 defines delivery semantics but no clause defines the width or the wrap/saturation behaviour of the `FF_SbStats_t` counters, so no controlled oracle exists. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the counters are plain `uint32_t` and wrap modulo 2^32. No oracle is asserted here. The EVS total-count wrap across the ring is owned by TS-6 `N22-TS6-7` | SIL | individual | gap |
| N22-TS5-71 | FRICK-ARCH-0002 §2 | SB publish lock contention rejects atomically | OOB | acquire the SB mutex using the synchronized test hook; snapshot stats and subscriber queue; call `FF_SbPublish`; release mutex | exact result is `FF_TIMEOUT`; `published`/`delivered`/`dropped` remain byte-for-byte unchanged and receive finds no new message; lock acquisition is controlled, not induced by sleeps | SIL | individual | covered |
| N22-TS5-72 | FRICK-ARCH-0002 §2 | deferred EVS notification lock contention is observable without false enqueue | OOB | acquire EVS mutex using synchronized test hook; snapshot EVS log count and notification stats; call `FF_EvsSendDeferred`; release mutex | exact result is `FF_TIMEOUT`; EVS log, queue-derived `queued`/`dispatched`/`dropped` statistics do not change; after release a fresh call returns `FF_SUCCESS` and exactly one event is logged | SIL | individual | covered |
| N22-TS5-73 | SRS-REQ-011 | SB overflow forwarding when EVS lock is contended | OOB | fill the one-entry subscriber pipe, hold EVS mutex with synchronized hook, and publish a second matching message | **gap** — current code completes SB publish/drop accounting but cannot log/enqueue the EVS report; SRS-REQ-011 requires ground forwarding and no authority-owned overload policy defines the bounded failure outcome. Do not treat this observation as an acceptable requirement oracle; NC-EXT-76 remains open until maximum rate/burst, minimum dispatch service, maximum forwarding latency and overload policy are controlled | SIL | individual | gap |
| N22-TS5-74 | SRS-REQ-072, SSS-REQ-006 | Event-arrival envelope, dispatch service and ground-forwarding latency | nominal + OOB | timestamp each valid EVS API submission, acceptance/queueing, PUS S5 frame acceptance by selected link backend, and ground receipt; inject arrivals satisfying approved `A(Δ) ≤ Bmax + Rmax·Δ`, then exceed each approved bound under approved link profile | **gap/TBD** — in-envelope accepted events must reach the approved ground boundary within `Lmax`, with dispatch completion ≥ `Dmin` over each approved service interval; rejection, queue loss, and post-acceptance loss must increment the independent loss counter exactly once and be reported when possible. Local EVS log alone is not ground receipt. Parameters/profile and counter-reporting oracle are not approved/implemented; no numeric pass/fail oracle exists yet (NC-EXT-76). Queue=64, batch=8 and nominal 1000 ms task delay are candidates only; HIL/target ground-receipt fidelity remains TBD | SIL | combined | gap |
| N22-TS5-75 | SRS-REQ-001 | SB queue-mutex contention skips only the contended copy and reports incomplete fan-out | OOB | create two matching pipes in deterministic subscription order; hold the first pipe's queue mutex with the test hook; publish while it remains held; then release it | `FF_SbPublish` returns exactly `FF_TIMEOUT` before the held mutex is released; `published` and `dropped` each increase by one, `delivered` increases by one; contended pipe remains empty, second pipe receives the exact message; one deferred `FF_EVS_SB_ERR`/`FF_EVS_ERROR` identifies the busy subscriber/message and contains `queue busy`, with no inline listener call. Full queue is separately covered by TS-6 `N22-TS6-44` and keeps `FF_SUCCESS` | SIL | combined | covered |
| N22-TS5-76 | SRS-REQ-014, SSS-REQ-008 | ES/HS lifecycle + health-detection host integration harness | nominal + OOB | register a RUNNING application and a registered-but-unstarted HS task, step HS (observes only `FF_APP_LOADED`, no FAILED report); then inject a PAL task-create failure to drive an application to `FF_APP_FAILED`, step HS, and step again | each HS pass raises exactly one `0x0502` `FF_EVS_INFO` heartbeat for the HS application and exactly one `FF_EVS_APP_ERROR` `CRITICAL` naming the FAILED id (one per pass while the failure persists); the RUNNING application is never reported (no false positive); `FF_EsGetState` reflects `FF_APP_LOADED`/`FF_APP_RUNNING`/`FF_APP_FAILED` deterministically (SRS-REQ-014 "applications run as independent tasks… track states"; SSS-REQ-008 "monitor application health"). Independent EVS-listener oracle in `tests/unit/test_es_hs_lifecycle.c` (S8-VV-FU). Detection latency, external-watchdog cadence and the task-return/abort restart contract remain `gap` (N22-TS6-41/N22-TS5-52/N22-TS6-31); the harness records only the grounded detection oracle here | SIL | combined | covered |

## 3. Gaps (mapping to NC-EXT-47 sub-items)

All gaps are recorded, not resolved (N21). Each row below maps a `gap` case to
the NC-EXT-47 sub-item in FRICK-VV-TSPEC-001 §7 that it belongs to, and names
the specific open NC where one exists. Gap rows assert **no** oracle.

**NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause
(unresolved/absent contract).**

- `N22-TS5-10` — SB duplicate pipe creation policy (TBD; NC-EXT-47 §7.3).
- `N22-TS5-11` — SB pipe depth 0 / above `FF_SB_QUEUE_DEPTH_MAX` (TBD;
  NC-EXT-47 §7.3).
- `N22-TS5-13` — SB subscribe/unsubscribe to a non-existent pipe (TBD;
  NC-EXT-47 §7.3).
- `N22-TS5-14` — one pipe holding both an exact and a wildcard subscription
  (copy count) (TBD; NC-EXT-47 §7.3).
- `N22-TS5-28` — TBL registration with an incomplete definition (TBD;
  NC-EXT-47 §7.3).
- `N22-TS5-45` — scheduler atomic activation: single-writer double-buffer only;
  cross-core/interrupt memory ordering deferred to M14 (NC-EXT-47 §7.3).
- `N22-TS5-46` — mission dispatch rates/jitter tolerances remain
  mission-controlled TBD; host-verified dispatcher behavior is covered but no
  target/RTEMS timing evidence exists (NC-EXT-19 / M17; NC-EXT-47 §7.3).
- `N22-TS5-47` — scheduler heartbeat / late-missed-slot / dispatch-failure
  counters are host-verified, but the EVS-to-Health&Safety end-to-end oracle
  requires the running application set (integration/simulator smoke;
  NC-EXT-19 / M17; NC-EXT-47 §7.3).
- `N22-TS5-48` — scheduler mission rates, tolerances and overrun response TBD
  (NC-EXT-47 §7.3; FRICK-ARCH-0018 §1/§5).
- `N22-TS5-53` — HS treatment of STOPPED/LOADED (non-RUNNING, non-FAILED)
  applications (TBD; NC-EXT-47 §7.3).
- `N22-TS5-66` — scheduler ↔ TBL validated/activated dispatch coupling not
  delivered (registration and validator are wired; the TBL load/activate →
  dispatch path is missing) (NC-EXT-19 / M17; NC-EXT-47 §7.3).
- `N22-TS5-70` — SB `FF_SbStats_t` counter width and unsigned wrap/saturation
- `N22-TS5-73` — SRS-REQ-011 outcome when SB overflow event cannot acquire EVS
  lock (authority policy gap; TBD; NC-EXT-47 §7.3).

**NC-EXT-47 §7.2 — software-only robustness oracles not specified as a
controlled, requirement-mapped set.**

- `N22-TS5-12` — SB `FF_SbPublish`/`FF_SbReceive`/`FF_SbGetStats` NULL
  arguments (TBD; NC-EXT-47 §7.2/§7.3).
- `N22-TS5-15` — SB API before `FF_SbInit` (pre-init contract) (TBD;
  NC-EXT-47 §7.2).
- `N22-TS5-29` — TBL invalid handle and invalid dump arguments (TBD;
  NC-EXT-47 §7.2/§7.3).
- `N22-TS5-34` — EVS pre-init, NULL/over-bound arguments and text truncation
  (TBD; NC-EXT-47 §7.2).
- `N22-TS5-42` — ES invalid configuration / unknown id / invalid lookup (TBD;
  NC-EXT-47 §7.2).

**NC-EXT-47 §7.5 — target (HIL) evidence remains GAP-A5.**

- `N22-TS5-52` — HS heartbeat cadence versus the external watchdog window
  (GAP-A5; NC-EXT-47 §7.5).
- `N22-TS5-61` — math facade on-target FPU determinism and WCET/stack budget
  (GAP-A4 / GAP-A5; NC-EXT-47 §7.5).
- `N22-TS5-68` — core services on target silicon (GAP-A5 / NC-EXT-31;
  NC-EXT-47 §7.5).

**NC-EXT-47 §7.6 — emulated-target aggregate marker is not a per-requirement
oracle.**

- `N22-TS5-67` — core services on Renode: `RENODE_TEST: PASS` does not evidence
  the SRS-REQ-044/SRS-REQ-014 core-service observables (TBD; NC-EXT-47 §7.6).

### 3.1 Current automated evidence (as-is) versus specified-not-yet-run

The cases above are **specified**; the following records what is actually
automated on the baseline, so that `covered` is not read as `passing`:

- **Registered core-service suites**: `test_sb.c`, `test_tbl.c`, `test_r2_gap.c`,
  `test_r3_gap.c` and `test_sched.c` already assert several of these oracles (SB
  fan-out / FIFO / no-match / wildcard / saturation / drop, TBL lifecycle /
  revert / dump / capacity, EVS listener and ring, ES registry/lifecycle, TBL
  atomic activate, scheduler image validation / atomic activation / per-entry
  release targets / missed-slot accounting).
  `test_math.c` covers SRS-REQ-033 tolerance, helpers and domain behaviour. The
  SB/TBL/ES/EVS/HS row-to-suite mapping must be confirmed by the runner before
  any statement of "verified".
- **SB full-pipe correction (implemented and host-tested this increment)**:
  the baseline divergence recorded for `N22-TS5-7`/`N22-TS6-44` is corrected
  in `src/core/sb/sb.c`; `tests/unit/test_sb.c` now asserts the EVS log event,
  dropped statistic, retained queued message and publish return. The applicable
  SIL run passed (`test_sb`, 48 checks; `make test`, 27/27) and is recorded in
  `docs/vv/verification-results.md`; broader NC-EXT-47 remains open.
- **Scheduler (`SRS-REQ-071`) dispatcher delivered and host-tested**: the
  bounded, allocation-free `src/core/sched/ff_sched.c` replaces the shared
  last-run stub; `tests/unit/test_sched.c` asserts validation accept/reject,
  atomic activation, per-entry absolute release targets (no starvation —
  NC-EXT-19 corrected), no cumulative drift, bounded `max_out`, missed-slot
  accounting and real SB delivery, with explicit expected values. Rows
  `N22-TS5-43/44/45/46` move to `covered`. Mission rates/tolerances, RTEMS/target
  timing, the EVS-to-HS end-to-end oracle (`N22-TS5-47`) and the TBL
  load/activate→dispatch coupling (`N22-TS5-66`) remain `gap`, so NC-EXT-19 is
  **not** closed here.
- **HS (`SSS-REQ-008`) has no dedicated automated suite** on the baseline
  (lifecycle/HS detection is currently simulator-smoke only); rows `N22-TS5-49`
  to `N22-TS5-51` are specified and instrumentable on the host but must be added.
- **Runner note**: `covered` requires the case to be executed in the named
  environment and its raw evidence retained (N11); this document assigns the
  oracle, not the run status.

### 3.2 Coverage summary

| Metric | Count |
| --- | --- |
| Total cases | 76 |
| Nominal | 48 |
| Out-of-bound | 26 |
| Mixed nominal + out-of-bound | 2 |
| Individual | 66 |
| Combined | 10 |
| `covered` (oracle grounded in a requirement/architecture/interface clause) | 55 |
| `gap` (no controlled oracle; recorded under NC-EXT-47) | 21 |
| SIL / PIL / HIL (primary assignments; target remains gap) | 73 / 1 / 2 |

Requirement coverage in this scope: `SRS-REQ-001/002/011/013/014/015/019/033/
044/071` and `SSS-REQ-002/008/009/018/030` all have at least one case. Every
in-scope module surface has nominal and (where a contract exists) out-of-bound
cases; the scheduler rows are predominantly `gap` because `SRS-REQ-071` is a
planned milestone (M17) whose implementation and numeric profile are not yet
delivered.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial TS-5 work package per N22 and the FRICK-VV-TSPEC-002 chunk plan: nominal + out-of-bound case table with explicit oracles for the core services (Software Bus, Table Services, Event Services, Executive Services, scheduler application, HS/FDIR and the mathematics facade) and the combined scenarios; gaps mapped to NC-EXT-47 sub-items with the open NC/TBD named |
| 0.2 | 2026-10-14 | frick-fsw | Update the SB full-pipe oracle cross-reference and record the regression test execution; broader NC-EXT-47 remains open |
| 0.3 | 2026-10-14 | frick-fsw | Update the SB full-pipe oracle for deferred EVS listener delivery after R1 finding FRICK-REVIEW-0263; NC-EXT-75 remains open |
| 0.4 | 2026-10-14 | frick-fsw | Restore synchronous ordinary `FF_EvsSend` oracle; restrict deferred dispatch to explicit EVS deferred API and SB overflow |
| 0.5 | 2026-10-14 | frick-fsw | Add SRS-REQ-072 event-envelope and synchronized SB/EVS lock-contention cases with explicit TBD parameters, failure boundary and candidate-versus-oracle distinction |
| 0.6 | 2026-10-14 | frick-fsw | Specify deterministic multiple-subscriber queue-lock contention oracle, continued partial fan-out and exact return/statistics/event behavior; keep SRS-REQ-072 profile/loss-reporting gap open |
| 0.7 | 2026-10-14 | frick-fsw | Reconcile case summary to all 75 table rows, use one primary environment token per case and retain HIL/target ground receipt as a gap |
| 0.8 | 2026-10-16 | frick-fsw | Update scheduler rows 43–47/66 for the delivered S8-SCH dispatcher (`ff_sched` validation, atomic activation, absolute per-entry targets, bounded per-poll work, missed-slot/heartbeat counters) with host oracles in `tests/unit/test_sched.c`; retain mission-rate/tolerance, target-timing, EVS-to-HS integration and TBL load/activate coupling gaps; NC-EXT-19 remains open |
| 0.9 | 2026-10-16 | frick-fsw | S8-VV-FU: upgrade HS rows 49/51 to a grounded host integration oracle and add `N22-TS5-76` (ES/HS lifecycle + FAILED-application detection with an independent EVS-listener oracle), closing `GAP-R3-1`'s host-integration gap; `FF_AppHsStep` extracted as a non-blocking test seam. Detection latency, watchdog cadence (TS-5/TS-6 `N22-TS5-52`/`N22-TS6-31`) and the task-return/abort restart contract (TS-6 `N22-TS6-41`) remain `gap`; no NC closed or phase approved |
