# ADR-0021 - Watchdog reset recovery and the pre-reset event report

| Status | Accepted for framework implementation; reset-recovery behaviour is not safety-approved |
| ------ | ----------------------------------------------------------------------------------- |
| Date   | 2026-10-21 |

## Context

The framework had no reset-recovery mechanism: `FF_PalPanic` formatted a
message and called `abort()`, `FF_BoardInit` only logged (the watchdog was a
comment), no hardware watchdog was ever armed or fed, and the mission
configuration field `watchdog_timeout_s` was dead configuration consumed by
nothing. Sprint `S8-FDIR-01` (see `docs/planning/sprint-plan.md`) implements
FDIR reset recovery in line with the intent of ECSS-E-ST-70-11C Rev.1 5.7.5.2 (the recovery action
is hardware-generated so that it is independent of the on-board software it
resets; an event report is generated before the reset) and the SAVOIR FDIR
handbook guidance that monitors/recovery trace to the FMECA/failure hypothesis.

The user (project owner) authorized the original scope below on 2026-10-20. A
follow-up owner response was received 2026-10-21 at 15:49 (Telegram 1317/1318;
recorded in `docs/status/escalations.md`): 8 s is an arbitrary placeholder,
with no mission justification, and is not a ratified flight threshold.
The user (project owner) authorised, on 2026-10-20: a controlled `FF_PalPanic`
as the single recovery entry; an independent hardware watchdog that is kicked
and, on loss of kicking, resets the processor; **one** reset path shared by a
hang and a commanded software reset; a timeout of 8 s fed every 2 s, driven by
the mission configuration table; and a pre-reset notification sent to ground as
a **PUS service 5 event report (D3)**.

## Decision

### Hardware watchdog (SRS-REQ-073)

- The platform-independent arm/feed/expiry state machine and its counters live
  in `src/core/pal/pal_watchdog.c`; the hardware binding is an abstraction with
  three hooks (`ff_pal_wd_hw_arm`/`_feed`/`_reset`).
- The host simulator provides a **simulated backend** that records arm, feed and
  reset requests and performs no hardware reset, so the mechanism can be
  stimulated deterministically in tests (owner decision B2-b).
- The RTEMS/MPFS MSS watchdog binding remains **not wired**: the hooks report
  `FF_NOT_IMPLEMENTED` (NC-PAL-01). The Renode BeagleV-Fire board description
  imports the PolarFire SoC platform and instantiates five MPFS watchdog models;
  model availability is not evidence of RTPFS160ZT register/timing/reset
  fidelity. The target binding still requires a verified Microchip BSP/datasheet
  reference and target/HIL validation (GAP-A5).
- A dedicated lowest-priority task (`FF_AppWdMain`, `src/apps/app_wd.c`) arms
  the watchdog after framework initialisation. The 8 s default and 2 s interval
  are framework placeholders only; preserve actual inter-feed ≤ T/4, use
  absolute monotonic deadlines with execution/jitter margin, and audit the
  earliest valid hardware refresh window before selecting cadence. No mission
  timing or flight suitability is established.

### Pre-reset event report (SRS-REQ-074)

- `FF_PalPanic` now runs, in order: bounded format and log; the registered
  pre-reset reporter; the single reset request; (test builds only) the
  `FF_TEST_HOOKS` observer; then `abort()`. The notifier and the reset request
  are unconditional and their failures are ignored — a failed report must never
  prevent the reset.
- The reporter is **lock-free by construction** (owner decision B1): it uses
  `FF_PusSendTmDirect`, which builds the frame with the selected backend's pure
  encoder into a static buffer and hands it to a direct writer
  (`FF_LinkSendUntil`, the deadline-aware link writer installed by
  `fsw_main.c`; the plain `FF_LinkSend` carries no deadline and is not used on
  the reset path),
  taking **no** EVS/SB/PUS/queue/time lock and performing no allocation. This is
  required because `FF_PalPanic` may be entered while any of those locks is
  held, and `FF_TimeGetCuc` in particular takes the time mutex.
- Consequence: the pre-reset frame carries a **degraded time base** (lock-free
  monotonic nanoseconds since boot, not the disciplined UTC time). This is an
  accepted, documented limitation of the reset path. The worst-case emission
  time is **not** analysed per link backend (recorded as a limitation, not a
  claim).
- Owner response (2026-10-21, Telegram 1317/1318; ESC-1/ESC-5/ESC-6): implement
  one absolute monotonic deadline of 250 ms for the entire local attempt,
  including formatting, console/UART wait-for-completion-or-timeout and all
  retries. Mandatory S5 content must not be truncated; no remote-delivery
  guarantee is assumed; reporting failure must not prevent reset. This is an
  initial target, not measured WCET or safety acceptance. The shared deadline is
  now implemented end to end (`FF_Pal_PanicNotifier_t`, `FF_Pus_DirectWriter_t`
  and `FF_Link_t::send_until` all carry an absolute deadline) and the
  boundary/fault cases exist (TS-5 `N22-TS5-83`..`87`), but they are host
  observations only: the target/backend completion time and the target WCET are
  not established, and the mandatory-text envelope and the bounded-formatting
  strategy remain authority decisions.
- **Writer implementation (2026-10-21, corrected).** The console and UART
  bounded writers use a non-blocking descriptor with a short back-off and a
  retry against the shared deadline, and restore the descriptor mode
  afterwards. A `poll()`-based readiness wait was implemented first and then
  **removed**: RTEMS 6.2 provides the `poll.h` types but implements no
  `poll()`, so the flight target did not link. The current form is a busy
  retry, not a readiness wait; the target driver's readiness semantics are
  unverified, and the shared-descriptor mode change races with the normal
  telemetry path (registered as NC-EXT-191).
- The event identifier is the mission-defined `FF_EVS_PANIC_RESET` (`0x0104`)
  with `CRITICAL` severity, sent on service 5 subservice 3. The ground-visible
  event-id catalogue is an interface item; its confirmation is tracked by
  **NC-EXT-102**.

### Single reset path (SRS-REQ-075)

- `FF_PalWatchdogReset` is the only reset action. A watchdog expiry (when the
  platform is wired) and a commanded reset (`FF_AppHsReset`, exposed as PUS
  service 8 function 1) both reach it through `FF_PalPanic`. A rejected or
  malformed service 8 command must not reset.

## Consequences

- Reset recovery is **implemented and host-tested but not qualified**: the
  hardware protection is absent until NC-PAL-01 is closed, and the FDIR
  monitors/thresholds and the recovery budget need FMECA linkage and
  system/HIL validation before any behaviour claim. No self-approval of
  FDIR/reset-recovery behaviour is asserted.
- `FF_TEST_HOOKS` makes the panic path observable in tests without terminating
  the process; the flight build excludes it and still falls through to `abort()`
  when no hardware reset occurs (never silently continues).
- The smoke/QEMU V&V scripts were updated to the new registered-application
  count (7); they are V&V tools and their register entries are updated per N10.

## Alternatives considered

- **Queue the pre-reset report through EVS/SB/PUS.** Rejected: re-entering those
  locks from a panic taken while holding one of them can deadlock and the reset
  would never be enforced.
- **Have the software itself perform the reset.** Rejected: ECSS requires the
  recovery action to be independent of the software it resets, and a hung
  software stack cannot reset itself.
- **Abort only (status quo).** Rejected: no ground notification and no
  recovery path.
- **Wire the MPFS watchdog by assumption.** Rejected: the register/sequence must
  not be fabricated; an unverified binding would be unsafe and unevidenced.

## References

- ECSS-E-ST-70-11C Rev.1 5.7.5.2 (recovery independence, event report before
  reset); ECSS-Q-ST-30C (HSIA / FDIR-vs-FMECA); SAVOIR FDIR handbook.

  **Conformance limitation (NC-EXT-161):** the cited clause text was not read
  against a controlled copy in this cycle, so this ADR records a design intent,
  not established conformance. Conformance cannot be claimed while the hardware
  binding is unwired (NC-PAL-01), the pre-reset path can block (NC-EXT-168) and
  the report-before-reset ordering is established only by inspection and a
  definitional Lean lemma (NC-EXT-167/132). Clause-level verification is an R3 /
  authority input.
- `docs/requirements/SRS.md` SRS-REQ-073/074/075;
  `docs/requirements/traceability.yaml`.
- `docs/vv/test-spec/ts-5-core-services.md` N22-TS5-50; NC-PAL-01;
  `docs/status/loop-audit.md`.

## Renode watchdog model reuse audit (2026-10-21)

Read-only inspection of the pinned image
`antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`
identified Renode v1.16.1.16973. The loaded `beaglev-fire.repl` imports
`platforms/cpus/polarfire-soc.repl`, which instantiates five
`Timers.MPFS_Watchdog` devices at 0x20001000, 0x20101000, 0x20103000,
0x20105000 and 0x20107000. The platform sets model frequency to an estimated
156250 Hz. This establishes model availability only, not RTPFS160ZT register,
clock, reset-scope or target fidelity; upstream model register-locking is a
known TODO. No target PAL binding was inferred or wired by this audit.

## Public vendor-reference audit lead (2026-10-21)

The project owner supplied a public Microchip MPFS HAL watchdog header at
`https://raw.githubusercontent.com/polarfire-soc/polarfire-soc-bare-metal-examples/refs/heads/main/driver-examples/mss/mpfs-hal/mpfs-hal-run-from-ddr-u54-2/src/platform/drivers/mss/mss_watchdog/mss_watchdog.h`.
The retrieved header describes five MSS watchdog instances, MVRP-configurable
refresh-forbidden behavior (disabled by default; refresh forbidden while the
counter is greater than MVRP), an AXI/256 frequency example, and timeout IRQ/NMI
before reset at counter zero. These are vendor-driver documentation claims and
audit leads only. The URL points to mutable `main`; no commit-pinned driver,
exact RTPFS160ZT datasheet/register mapping, RTEMS BSP integration, selected
watchdog instance, clock rate, clock/power/reset independence, or configured
counter/MVRP/timeout values have been verified. Therefore this reference does
not establish the earliest valid refresh time, units for a mission feed
cadence, hardware binding, or reset coverage. The project-owner-supplied RT
PolarFire SoC product URL explicitly names RTPFS160ZT, but was not retrievable
from this environment (HTTP 403); it is not used as datasheet evidence:
`https://www.microchip.com/en-us/products/fpgas-and-plds/radiation-tolerant-fpgas/rt-polarfire-soc`.
Do not map the host/Renode/driver model to the target or choose a feeder cadence
until controlled target documentation and configuration are audited.

## History

| Revision | Date | Change |
|---|---|---|
| 1.0 | 2026-10-20 | Initial watchdog reset recovery and pre-reset report decision record |
| 1.1 | 2026-10-21 | Record owner follow-up: 8 s is arbitrary; 250 ms local attempt target and monotonic timing direction; correct Renode model inventory; add public HAL/product audit leads with explicit target-variant and configuration limitations |
| 1.2 | 2026-10-21 | frick-fsw | Record the implemented shared deadline and its host evidence, and correct the writer design: the `poll()`-based readiness wait was replaced by a non-blocking descriptor with a bounded retry because RTEMS 6.2 implements no `poll()` and the flight target did not link; the target readiness semantics, the mandatory-text envelope and the bounded-formatting strategy remain open |
