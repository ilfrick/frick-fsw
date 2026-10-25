# FRICK-REVIEW-0415 — S8-FDIR-01 post-review remediation delta (R2/N22)

**Profile:** R2 V&V, fresh context  
**Disposition:** Draft review evidence only. No testware qualification, gate
pass or NC closure.

## Findings

1. **Major — the 250 ms target is still not demonstrated.** The delta adds a
   direct whole-path oracle (the notifier observes an already-expired deadline
   after a blocked console, and the reset request is observed after
   notification) and deterministic UART partial-progress/stall cases, but all
   elapsed-time evidence is host evidence. The target/backend 250 ms
   completion, target console/UART completion and target WCET remain
   unestablished; TS-5 `N22-TS5-83` stays a gap.
2. **Minor — the UART retry path is now observed, not assumed.** The external
   `write` wrapper injects three deterministic short writes on the UART slave
   descriptor and the test asserts that all injected short writes were consumed
   by the production retry loop and that the payload arrives byte-identical.
   The limitation that remains is scope, not coverage: the wrapper proves the
   partial-progress loop executes, not the timing of a real driver.
3. **Minor — PAL fault coverage is now executable with explicit oracles.** The
   `nanosleep` wrapper injects three EINTRs (the wait still succeeds) and one
   non-EINTR EIO (the wait returns `FF_ERROR`); the failed feed leaves the feed
   counter and the software expiry reference unchanged; the reset-hook error is
   returned while the reset-request count still increments. These are host
   syscall/backend fault injections, not target evidence.
4. **Minor — status semantics.** The rows that now carry executable host
   sub-claims are still marked `gap`. That is consistent with this document's
   convention ("no controlled oracle exists" for the requirement-level claim)
   but should be read as "the requirement-level clause is unresolved while the
   host sub-claim is executed and passing".
5. **Comment — records must name the post-review run.** The earlier
   verification-results section still describes the pre-remediation counts and
   the pre-remediation tree. The post-review log, source-hash file and updated
   counts must be bound to the current tree before the cycle is presented as a
   single snapshot.

## Cases reviewed

- Whole-path shared deadline (`test_panic_console_stall_and_report_share_one_budget`):
  the notifier observes an expired deadline and the pre-reset reset counter,
  and the reset is still requested after a blocked console. Host observation.
- Production link wiring (`test_panic_production_link_wiring_delivers_one_frame`):
  the flight wiring `FF_PusInstallPanicReporter(FF_LinkSendUntil)` delivers
  exactly one frame containing the panic event id, severity and reason text to
  the selected link backend. This closes the earlier gap where the host
  simulator silently dropped the report because the loopback backend had no
  deadline-aware writer.
- Over-length characterization (`test_panic_overlength_reason_counterexample`):
  96 characters report, 97 and 256 do not, and the reset is requested in all
  three cases. Characterization, not a passing SRS oracle.
- Time/PPS: the earlier tone holdover and UTC-step host checks are unchanged
  and still establish only limited host behavior.

**Confidence:** high for the enumerated test scope. No host timing is reported
as target WCET.
