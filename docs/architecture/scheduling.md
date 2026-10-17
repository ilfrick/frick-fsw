# Application Scheduling Architecture and Trade Study

| Document id | FRICK-ARCH-0018 |
| --- | --- |
| Revision | 0.3 |
| Date | 2026-10-16 |
| Status | Design input with a partially implemented dispatcher; the implementation is not an assurance, schedulability or qualification claim |

## 1. Scope and terminology

The project needs an **application-level schedule dispatcher**, not a second
RTOS scheduler. The dispatcher releases configured Software Bus messages at
mission-selected times/rates. RTEMS remains responsible for selecting which
ready task runs, priority preemption, and synchronization. The dispatcher is
itself an ordinary Executive Services task and therefore receives its CPU time
under RTEMS' configured native scheduler.

The former `src/apps/app_scheduler.c` reference stub polled at 100 ms, used one
shared `g_last_run` timestamp for all entries, ignored Table Services
activation/error results, and had no missed-slot, heartbeat or recovery
semantics. One entry resetting the shared timestamp starved entries with longer
periods (tracked as NC-EXT-19). That stub has been replaced by the bounded
dispatcher described in section 5.1; mission suitability is still not claimed.

## 2. Industrial/flight-software options reviewed

| Option | Function and evidence | Fit / disposition |
| --- | --- | --- |
| Native RTEMS scheduler | Configurable kernel task schedulers, including deterministic priority and EDF variants; task scheduling/preemption is the OS responsibility. RTEMS has documented space-sector adoption/qualification activity (RTEMS Project manual; ESA overview). | **Selected foundation.** Retain RTEMS for task scheduling; do not replace its scheduler with an app-level loop. Select the actual algorithm only from approved RTEMS/BSP configuration and verified workload analysis. |
| NASA cFS SCH application | Flight-software application that emits Software Bus messages on configured time slots in a time-division/multi-rate schedule, typically aligned to a time-service major-frame signal; its own description makes clear it is an application plugged into cFE, not the RTOS dispatcher. NASA publishes the source under its own license. | **Primary behavioral reference.** Port the concepts (bounded schedule table, major/minor time slots, SB message release, schedule synchronization, status/diagnostics) into frick-fsw's contracts. Do not copy source until a separate license/provenance review approves any code reuse; use independently authored implementation by default. |
| NASA/JPL F Prime RateGroupDriver + Active/PassiveRateGroup | Flight-software framework pattern: a timing source drives a rate-group divider; active groups execute on threads, passive groups execute synchronously in caller context. F´ documents slip detection and warns that synchronous work delays following groups. | **Useful comparator**, especially for fixed periodic activity and overrun observability. It is not a direct drop-in: C++/F´ component topology differs from this C/SB framework. |
| RTEMS Rate Monotonic Manager | Kernel-provided periodic period object useful for implementing periodic releases and collecting period status; not an application schedule table or a replacement for RTEMS task dispatch. | **Candidate RTEMS primitive**, subject to compatibility, target test, API review, and WCET/tick-resolution assessment. Compare with absolute monotonic deadlines plus task delay during M17. |
| Wind River VxWorks | Commercial native RTOS with extensive vendor-reported flight heritage (e.g. Mars InSight, Juno, and other spacecraft) and proprietary delivery/support. | Valid industrial alternative for a platform selection trade, not a scheduler add-on. Not selected because RTEMS 6.2 is the controlled target baseline; migration would require platform, licensing, supplier, and assurance re-baselining. Vendor heritage does not qualify this application. |

### Decision

Implement an in-house, CFS-SCH-inspired **table-driven SB dispatcher** in M17,
using the RTEMS 6.2 native scheduler and RTEMS clock/task services. Keep the
dispatcher bounded, static-memory, independent from task preemption, and
instrumented for Health & Safety. Use F´ rate-group concepts as a design
cross-check, not as an implementation dependency. Mission table rates,
major-frame synchronization source, allowed jitter, task priority/stack,
late-slot policy, and recovery authority remain `TBD` until supplied by the
system/mission authority; do not invent values or imply schedulability.

## 3. cFS SCH concepts to carry over

The NASA SCH model is a time-division schedule: a major frame (commonly tied to
cFE Time Services synchronization) contains minor frames/slots, and table
entries cause software-bus messages to be generated at pre-determined times.
This separates **when application work is released** from **which RTOS task
runs next**. Carry the model, not a literal source transplant:

- fixed-capacity, validated schedule image with explicit slot/entry bounds;
- schedule epoch/synchronization and monotonic execution clock kept distinct
  from UTC/ground time correlation;
- message dispatch through the existing SB interface, bounded per tick;
- explicit enable/disable and atomic staged-table activation using Table
  Services;
- counters/events for missed or late ticks, invalid tables, dispatch errors,
  and schedule synchronization state;
- table-level verification for rate divisibility, duplicate/conflicting
  entries, arithmetic overflow, and worst-case dispatch work.

Detailed cFS semantics, current release compatibility, configuration/profile,
and license terms are to be checked against the exact cFS source version before
any compatibility claim or code reuse. The project target here is behavioral
architecture, not cFS conformance.

## 4. Fault response and backup strategy

There is no justified claim of a second RTEMS task scheduler that takes over
automatically when the native scheduler or its dispatch machinery encounters a
fatal exception. RTEMS documents that fatal conditions (including processor
exceptions) enter the Fatal Error Manager; configured fatal extensions may
perform platform-specific recovery, and the BSP commonly supplies a reset
handler. A reset is containment/recovery, **not** scheduler failover.

Planned layered response, subject to system-safety authority approval:

1. **Application-level schedule fault** (late/missed tick, invalid table,
   publish failure): latch diagnostics, stop/hold schedule dispatch according
   to an authority-defined policy, maintain the H&S heartbeat/status report,
   and avoid unbounded catch-up bursts.
2. **Scheduler task stopped/hung**: independent H&S monitoring detects stale
   heartbeat. A task restart is permitted only if RTEMS/ES supports it and the
   scheduler's state is reset to a defined, table-validated state; dispatch must
   be inhibited until restart completion and recovery confirmation. No blind
   restart loop.
3. **RTEMS fatal/internal error or processor exception**: invoke the configured
   RTEMS/BSP fatal extension and the approved watchdog/reset path. Do not try to
   switch to a second scheduler from a potentially corrupted execution context.
4. **Reset/restart recovery**: boot validates image and schedule table, reports
   reset cause, starts in an inhibited/safe schedule state, and requires the
   approved re-enable condition. This behavior must be defined with platform,
   safety, and operations authorities before flight use.

Independent watchdog hardware, whether windowed and its timeout, reset
sequencing, reset-cause persistence, restart budget, safe-state behavior, and
mission operations procedure are all `TBD` pending the platform/mission
authority. The scheduler must never service a watchdog in a way that masks its
own loss of progress; watchdog ownership/health aggregation is an explicit
design item.

## 5. M17 implementation and verification gate

- Define schedule-table schema, exact integer time units, epoch, slot ordering,
  sync-loss behavior, and error semantics; acquire all mission-specific values
  from controlled inputs.
- Replace the current reference stub only after requirements and review
  approval. Use monotonic time and absolute release targets (or a justified
  RTEMS period primitive) to avoid cumulative relative-delay drift.
- Prove static bounds for number of entries, work per release, queue/publish
  interactions and memory. Define no-catch-up / skip / inhibit behavior only
  with authority-approved requirements.
- Test several independent periods, phase alignment, slot rollover, clock
  discontinuity, table validation/activation, repeated ticks, missed/late
  release, full SB queue, publish error, task restart, and fatal/reset
  simulation where the environment can support it.
- Run target RTEMS timing and overload tests; host tests do not establish
  target jitter, WCET, RTEMS fatal-extension behavior, or watchdog correctness.
- Record test-environment fidelity, scheduler configuration, binary/source
  identity and timing assumptions. Closure requires R1/R2/R3/R4/PA gates as
  applicable; none of this plan alone authorizes flight use.

## 5.1 Implementation status (first S8-SCH increment, 2026-10-16)

Delivered and host-verified (not an approved mission profile):

- `src/core/sched/ff_sched.{c,h}`: fixed-capacity (`FF_SCHED_MAX_ENTRIES` = 16),
  statically allocated schedule image; no dynamic allocation anywhere.
- Validation (`FF_SchedValidate`): image size, magic, entry-count bound,
  non-zero period, payload bound, and duplicate `msg_id` rejection. Rate
  divisibility and major-frame alignment are deliberately **not** checked —
  they are mission inputs that remain `TBD`.
- Atomic activation (`FF_SchedActivate`): a rejected image leaves the active
  table untouched; accepted images are published through a single-writer
  double buffer. Interrupt/multi-core memory-ordering beyond that single-writer
  publish is not established and is tracked with the platform/SMP work (M14).
- Per-entry **absolute** release targets: each entry keeps its own next-due
  instant, so a short period cannot starve a longer one (corrects NC-EXT-19)
  and relative-delay drift does not accumulate.
- Checked deadline arithmetic: if the first or next target cannot be represented
  in `uint64_t` monotonic nanoseconds, that entry is disabled until explicit
  `FF_SchedInit` and `time_exhaustions` increments. Activation clears targets
  and must be followed by `FF_SchedInit` to resume dispatch; the missed-slot
  diagnostic saturates at `UINT32_MAX`. No wrapped target is dispatched. This is
  the provisional public-reference policy in ADR-0020, not a mission-approved
  clock-domain or recovery requirement; NC-EXT-90 stays open for authority/target
  evidence.
- Bounded work per poll: at most one release per entry per call; slots crossed
  while late are counted (`missed`) and never burst-released. The provisional
  no-catch-up/skip behaviour is a project selection and still requires an
  authority-approved late-slot policy (NC-EXT-07 scope).
- Diagnostics for the §5 evidence requirement: release/missed/late/publish-error
  counters exposed by `FF_SchedGetStats`, plus EVS heartbeat, missed-slot and
  dispatch-failure events emitted by `src/apps/app_scheduler.c`.
- Host verification: `tests/unit/test_sched.c` (nominal and out-of-bound cases
  with explicit expected results, injected clock) plus real SB delivery.

Explicitly **not** established: mission rates/jitter/tolerances, WCET, target
RTEMS timing, watchdog interaction, clock-discontinuity behaviour, table
load/activate-over-SB operational flow, and recovery authority. Section 5's
gate ("replace the stub only after requirements and review approval") is **not
yet satisfied**: the independent R/PA review of this increment is pending, so
NC-EXT-19 is not closed and no flight-use authorization is implied.

## 6. Sources checked (accessed 2026-10-13)

- NASA, `nasa/SCH`, README: https://github.com/nasa/SCH — states SCH is a cFS
  application producing SB messages at predetermined timing intervals, with
  configurable major/minor frames; NOSA license noted for provenance review.
- NASA GSFC ETD, Core Flight System:
  https://etd.gsfc.nasa.gov/capabilities/core-flight-system/ — describes cFS
  as a reusable flight-software framework and reports broad NASA mission use;
  this is institutional heritage evidence, not qualification of this port.
- RTEMS Project, RTEMS 6.1 Classic API, Scheduling Concepts:
  https://docs.rtems.org/docs/6.1/c-user/scheduling-concepts/index.html — lists
  priority/EDF and SMP scheduler variants. The project baseline is RTEMS 6.2;
  consult the controlled 6.2 manual/configuration before implementation.
- RTEMS Project, RTEMS 6.1 Classic API, Fatal Error Manager background:
  https://docs.rtems.org/docs/6.1/c-user/fatal-error/background.html — fatal
  errors include RTEMS/application/processor exceptions; fatal extensions may
  run, and BSP often resets. This is not scheduler redundancy.
- NASA/JPL, F´ Rate Groups and Timeliness:
  https://fprime.jpl.nasa.gov/latest/docs/user-manual/design-patterns/rate-group/
  — documents rate group driver, active/passive execution, and slip concerns.
- ESA, RTEMS:
  https://www.esa.int/Enabling_Support/Space_Engineering_Technology/Software_Systems_Engineering/RTEMS
  — describes ESA space-mission use and qualification activity. This does not
  qualify this project's RTEMS build or application.
- Wind River, Space Missions:
  https://www.windriver.com/inspace — vendor-reported VxWorks use on named
  spacecraft including InSight/Juno. Treat as supplier heritage evidence, not
  independent qualification or evidence for this project.

## History

| Rev | Date | Author | Reason |
| --- | --- | --- | --- |
| 0.1 | 2026-10-13 | frick-fsw | Record scheduler option trade, cFS-SCH behavior, RTEMS ownership boundary, recovery limits and M17 plan |
| 0.2 | 2026-10-16 | frick-fsw | Record the delivered S8-SCH dispatcher increment (ff_sched validation/atomic activation/absolute per-entry targets, bounded per-poll work, missed-slot and heartbeat diagnostics) and state explicitly that the section 5 review gate, mission timing inputs and target evidence remain open |
| 0.3 | 2026-10-16 | frick-fsw | Select checked, non-wrapping deadline arithmetic with per-entry disable/counter as provisional framework behavior; cite ADR-0020 and keep NC-EXT-90/mission authority limits open |
