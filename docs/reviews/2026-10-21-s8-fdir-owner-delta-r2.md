# FRICK-REVIEW-0410 — S8-FDIR owner-directed V&V delta

**Profile:** R2 V&V, fresh-context review  
**Disposition:** Draft review evidence only; no testware qualification, gate
pass, or NC closure.

## Findings

1. **Major — no end-to-end timing oracle.** Tests propagate/capture a deadline
   and exercise UART nominal, expired, and disconnected results, but do not
   measure the complete panic-entry-to-local-attempt path or test stalled/
   partial writes and retries sharing that deadline. TS-5 N22-TS5-83 correctly
   remains a gap.
2. **Major — host behavior is not the target 250 ms bound.** Host `poll`, PTY,
   and unit results cannot establish RTEMS scheduling, target console/UART
   completion, or target WCET. The unwired watchdog remains a separate hardware
   gap.
3. **Minor — watchdog injected-error coverage is narrow.** Arm/feed status
   propagation is checked, but full state preservation on failed feed and reset
   hook error behavior are not covered.
4. **Minor — absolute wait fault cases are absent.** Future and expired deadline
   paths are tested; EINTR/repeated interruption and non-EINTR sleep errors are
   not.
5. **Minor — time tests are limited host observations.** Missing-edge holdover
   increase and UTC-step/nondecreasing monotonic checks do not establish
   holdover accuracy/duration or PPS hardware integration.

**Review basis:** current SRS, TS-5, T3 register, and changed tests inspected
2026-10-21. **Confidence:** high for enumerated test scope. No host timing is
reported as target WCET.
