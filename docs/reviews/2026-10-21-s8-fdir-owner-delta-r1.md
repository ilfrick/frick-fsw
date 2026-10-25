# FRICK-REVIEW-0409 — S8-FDIR owner-directed source delta

**Profile:** R1 FSW, fresh-context review  
**Disposition:** Draft review evidence; no approval, NC closure, safety acceptance,
or target WCET finding.

## Findings

1. **Major — oversized panic text suppresses the report.** In
   `src/core/pal/pal_posix.c` (current `FF_PalPanic` formatting/notifier path),
   `vsnprintf` truncation makes `message_complete` false and skips the notifier.
   This yields no S5 report for a long reason, rather than a complete mandatory
   report as the requirement currently reads. Define the input/content envelope
   and behavior with requirements/safety authority before claiming compliance.
2. **Major — formatting is not bounded by the deadline.** The deadline is set
   before `vsnprintf`, but formatting has no deadline enforcement. A bounded
   output array does not bound scanning a caller-provided `%s` argument. The
   public variadic API therefore does not establish the 250 ms bound over all
   formatting work.

## Bounded confirmations and gaps

The current source propagates one absolute deadline to the notifier/writer;
UART uses nonblocking writes with polling; notifier failure does not bypass the
subsequent reset request. The feeder propagates arm/feed errors via panic.
RTEMS hardware watchdog hooks remain unwired. This review did not establish
target timing, WCET, or hardware reset behavior.

**Review basis:** working tree inspected 2026-10-21; SRS-REQ-074, ESC decisions,
PAL/PUS/UART source. **Confidence:** high for source-path observations.
