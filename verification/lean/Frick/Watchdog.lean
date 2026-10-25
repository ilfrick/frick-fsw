-- frick-fsw - Formal model: watchdog reset recovery (FDIR)
--
-- Mirrors src/core/pal/pal_watchdog.c, src/core/pal/pal_posix.c
-- (FF_PalPanic) and ADR-0021:
--   * the watchdog is armed with a timeout and only an armed watchdog can
--     expire;
--   * a successful feed restarts the deadline, and feeding at a period no
--     larger than the timeout keeps the watchdog from expiring (this is the
--     "feed interval << timeout" requirement, here proved as
--     period <= timeout);
--   * the pre-reset event report is ordered before the reset action;
--   * an explicit software reset request and a watchdog timeout use one
--     shared recovery sequence (single reset path).
--
-- Scope of the model: these are the software-visible ordering/counting
-- properties. The hardware watchdog itself (arm/feed MMIO, independent clock,
-- reset assertion) is not modelled: it is a platform binding, not yet wired
-- for the MPFS target (NC-PAL-01), and Renode has no watchdog model (GAP-A5).
-- The C implementation is not formally refined against this model.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0
import Frick.Common

namespace Frick.Watchdog

-- =====================================================================
-- Arming / feeding / expiry (SRS-REQ-073)
-- =====================================================================

/-- Software-visible watchdog state. `last` is the time of the last accepted
feed and `timeout` the configured interval, both in the same monotonic unit. -/
structure WdState where
  armed : Bool
  timeout : Nat
  last : Nat
deriving Repr, DecidableEq

/-- A feed is accepted only when the watchdog is armed; `FF_PalWatchdogFeed`
returns `FF_NOT_READY` and leaves the state untouched otherwise. -/
def feed (s : WdState) (now : Nat) : WdState :=
  if s.armed then { s with last := now } else s

/-- Expiry uses a strict comparison, exactly like `FF_PalWatchdogCheck`
(`elapsed > timeout`), and only an armed watchdog can expire. -/
def expired (s : WdState) (now : Nat) : Bool :=
  s.armed && decide (now - s.last > s.timeout)

/-- A disarmed watchdog never reports expiry (out-of-bound case: feeding or
checking before arming must not trigger the reset path). -/
theorem req_srs073_not_armed_never_expires (s : WdState) (now : Nat) (h : s.armed = false) :
    expired s now = false := by
  simp [expired, h]

/-- An accepted feed restarts the deadline: immediately after feeding, the
watchdog is not expired. -/
theorem req_srs073_feed_clears_expiry (s : WdState) (now : Nat) (h : s.armed = true) :
    expired (feed s now) now = false := by
  simp [expired, feed, h]

/-- If the feed period `p` does not exceed the configured timeout, then a feed
at `now0` followed by any observation `now` with `now - now0 <= p` is not an
expiry.

Scope note (do not over-read this theorem): the hypothesis is `p ≤ timeout`, so
the theorem proves the *no-expiry* property for any period up to the full
timeout, including the boundary case `p = timeout` where the margin is zero. It
does **not** prove the requirement's margin: SRS-REQ-073 requires the feed period
to be at most `timeout/4`, which leaves `3*timeout/4` of slack for scheduling
jitter and feed-execution time. That quarter-period margin is a property of the
derivation in `FF_AppWdPlan` (checked in `test_pal_watchdog.c`), not of this
theorem; a stronger variant with `p ≤ s.timeout / 4` is tracked as NC-EXT-167.
The model is also purely discrete: it does not model the task's blocking delay or
the execution time between feeds (NC-EXT-166). -/
theorem req_srs073_feed_within_period_not_expired (s : WdState) (now0 now p : Nat)
    (harmed : s.armed = true) (hp : p ≤ s.timeout) (hgap : now - now0 ≤ p) :
    expired (feed s now0) now = false := by
  have hle : now - now0 ≤ s.timeout := Nat.le_trans hgap hp
  have hne : ¬ (now - now0 > s.timeout) := Nat.not_lt.mpr hle
  simp [expired, feed, harmed, hne]

-- =====================================================================
-- Ordering and single reset path (SRS-REQ-074, SRS-REQ-075)
-- =====================================================================

/-- The two actions of the recovery path, in execution order. -/
inductive PanicAction where
  | report
  | reset
deriving Repr, DecidableEq

/-- One shared recovery sequence: the pre-reset event report is generated
first, then the reset is enforced. Both the commanded reset and a watchdog
timeout are defined from this single sequence (`FF_PalWatchdogReset` is the
only reset action in the C). -/
def recoverySequence : List PanicAction := [PanicAction.report, PanicAction.reset]

def commandedReset : List PanicAction := recoverySequence

def watchdogTimeout : List PanicAction := recoverySequence

/-- The report is generated before the reset is enforced (ECSS-E-ST-70-11
5.7.5.2: ground is notified before the reset). The report occupies the lower
index of the shared sequence, so it is emitted first. -/
theorem req_srs074_report_precedes_reset : recoverySequence[0]? = some .report ∧ recoverySequence[1]? = some .reset := by
  decide

/-- A commanded software reset and a watchdog timeout take the same path. -/
theorem req_srs075_single_reset_path : commandedReset = watchdogTimeout := rfl

/-- The recovery sequence contains exactly one reset action, so the reset is
not requested twice on one occurrence. -/
theorem req_srs075_one_reset_action : (recoverySequence.filter (· == .reset)).length = 1 := by
  decide

end Frick.Watchdog
