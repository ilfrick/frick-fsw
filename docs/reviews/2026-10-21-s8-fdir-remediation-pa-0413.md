# FRICK-REVIEW-0413 — S8-FDIR-01 owner-directed delta, fresh N20 review

**Profile:** PA / N20, fresh context  
**Disposition:** **Not approved.** Not a phase or release approval; no NC is
closed, no gate is passed by this review, and no safety risk is accepted.

## Configuration and evidence basis

- The retained gate transcript identifies HEAD `44d2af224675cd65a66d2870eb0fc9553952799c`,
  engineering baseline `318f49b`, 88 working-tree changes and a source/config/
  tests/Lean hash manifest (`docs/vv/evidence/2026-10-21-s8-fdir-gates.log`).
  The product under review is therefore a mutable, uncommitted candidate, not
  the committed baseline.
- The owner-delta record reports a direct native build timeout during configure,
  then a direct build and 44/44 CTest; trace and pinned-format PASS; Lean
  verification failed because `lake` was unavailable; smoke failed before
  execution; N18 was not rerun (`docs/vv/verification-results.md` rev 3.02).
  These are host/emulator observations, not target timing or reset evidence.
- The retained `s8-fdir-gates.log` describes a different recorded run/state
  (it records Lean and smoke PASS) and is not evidence that those gates passed
  on the later owner-directed delta.
- Retention and side effects are recorded: raw host output retained, no cleanup
  performed, and an earlier Renode inspection pulled the missing pinned image
  (subsequent inspections used `--pull=never`). This review pulled, installed or
  altered no tools or evidence.

## Findings

| ID | Severity | Finding and location | Disposition |
|---|---|---|---|
| F-01 | Blocker — safety-relevant | The panic path does not establish that reset proceeds despite failure or absence of notification. `src/core/pal/pal_posix.c:644–672` performs variadic `vsnprintf`, then the bounded console write, then invokes the notifier before `FF_PalWatchdogReset()`. Formatting is not deadline-enforced, and an unbounded `%s` can extend the path; a bounded output buffer does not bound the work. The earlier PA report also identified potentially blocking stdio and a blocking RTEMS UART `write()` on the notifier path (NC-EXT-168). The current delta adds deadline propagation and nonblocking host UART behavior but does not establish an equivalent bounded flight path. | **Open; withhold positive recommendation.** Requirements/safety authority and the implementation owner must resolve the path so notification/formatting cannot indefinitely prevent reset, then obtain independent verification. |
| F-02 | Major — requirements/verification | SRS-REQ-074 requires "exactly one" report while also allowing absent/failed notification not to prevent reset; it does not state whether "one" means attempted, generated or successfully written, nor define required fields, maximum input length or overlength behavior (`docs/requirements/SRS.md:434–444`). The implementation suppresses the notifier when formatting is incomplete. | **Open; authority decision required.** Define the obligation and the mandatory text/input envelope before making a conformance claim. |
| F-03 | Major — timing evidence | The 250 ms local-attempt deadline is not demonstrated end-to-end. The host test additions and the reported 60,000-byte PTY partial/retry and stalled-write checks do not establish panic-entry-to-reset timing across formatting, stdio, notifier, UART, RTEMS scheduling or target hardware. | **Open.** Retain a complete host fault matrix and obtain justified target/WCET evidence under the applicable authority and approved environment. |
| F-04 | Major — watchdog cadence and configuration | `FF_AppWdMain` feeds, then calls relative `FF_PalTaskDelayMs(feed_ms)` (`src/apps/app_wd.c:92–97`). Execution and scheduling delay add to the inter-feed interval. `FF_AppWdPlan` verifies arithmetic, not runtime cadence. The 8 s value remains a placeholder; controlled MSS refresh-window/configuration and an approved margin are absent. Public vendor HAL material is an audit lead, not controlled target evidence. | **Open; do not change/select cadence by inference.** Obtain controlled hardware/configuration evidence and requirements/system authority disposition, then verify timing on target/HIL. |
| F-05 | Major — missing target protection | The RTEMS watchdog hooks are deliberately unwired (`src/core/pal/pal_rtems.c`; NC-PAL-01), so the candidate provides no demonstrated MPFS hardware watchdog reset protection. The five generic MPFS models in the pinned Renode platform do not establish RTPFS160ZT fidelity, binding or timing. | **Open; target/HIL blocker.** NC-PAL-01 and GAP-A5 remain; Renode cannot discharge this gap. |
| F-06 | Major — report/evidence currency | Current records disagree about gate outcomes and review status: verification-results rev 3.02 says Lean and smoke were not passed on this delta, while the retained gates transcript records them as PASS for its identified tree; `docs/status/status.json` says the R3/PA reports are not retained, while reports 0411 and this 0413 review are now present. | **Open.** Reconcile the current pointer/status and bind every result to its run, tree and artifact. Do not use the earlier gate transcript to claim current-tree Lean, smoke or N18 passage. |
| F-07 | Major — incomplete V&V | The owner-delta record leaves the full 250 ms nominal/boundary/stall/partial/retry matrix open and states that N18, Lean and smoke were not rerun/passed on this tree. R2 report 0410 also identifies a missing end-to-end timing oracle, narrow injected-error coverage and limited time/PPS observations. | **Open.** N15/N22 evidence is incomplete; re-run the applicable gates on the identified candidate and close the documented test gaps through the approved process. |
| F-08 | Major — authority/decision record | Review 0411 identifies unresolved requirements, safety and hardware decisions. The current evidence does not establish an approved interpretation of exactly-one/report failure, mandatory text, feed timing/margin or the hardware refresh window. | **Open.** Obtain and retain decisions from the designated human authorities. This review cannot supply those decisions or accept residual risk. |
| F-09 | Minor — evidence scope/currentness | Review 0412 notes that prior TS-5 totals are historical and that `FF_AppWdPlan` checks arithmetic only. Existing records should not present historical case totals, plan arithmetic, the Renode model inventory or public vendor material as current target evidence. | **Open for controlled disposition.** Preserve the revision/run scope wherever these artifacts are cited. |

## Current blockers and disposition

The candidate has useful host-level evidence, but **no positive N20
recommendation is warranted**. The immediate blockers are F-01 (reset-path
liveness), F-02 (unresolved requirement semantics), F-03 (no end-to-end/target
timing evidence), F-04/F-05 (unapproved cadence and unwired hardware binding)
and F-06/F-07 (current evidence and gate status not fully reconciled).

Known NCs — NC-PAL-01, NC-EXT-102 and the existing S8-FDIR-01 review NCs —
remain open. This report assigns no replacement NC identifiers and closes
none. Reports 0409–0412 are bounded review evidence; they do not replace this
fresh N20 review or provide approval.

**Confidence:** high for the cited repository-state and source/evidence
limitations; low for any claim about target operation, target timing, watchdog
reset behaviour or flight suitability, because controlled target configuration
and target/HIL evidence are unavailable.

**Required human authority:** requirements/system and safety authorities for
requirement interpretation and residual risk; avionics/hardware authority for
watchdog configuration and refresh window; authorized release authority for any
later release decision.
