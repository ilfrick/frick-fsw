# FRICK-REVIEW-0414 — S8-FDIR-01 post-review remediation delta (R1)

**Profile:** R1 FSW, fresh context  
**Disposition:** Draft review evidence. No approval, NC closure, safety
acceptance or target WCET finding.

## Scope

Post-review remediation of the working tree relative to the retained
FRICK-REVIEW-0409: PAL host-only reset-result injection and API signature, the
external `nanosleep` link-time wrapper, the saturated-stderr whole-path panic
test, the production link-wiring panic test, the exact 96/97/256-character
panic-reason counterexample, the UART partial-progress/stall tests with an
external `write` link-time wrapper, the loopback `send_until` implementation and
the related CMake and test changes. No files were edited by the reviewer.

## Findings

1. **Major — remains open (policy).** `src/core/pal/pal_posix.c` still suppresses
   the notifier when `vsnprintf` reports truncation, so an over-length reason
   yields no pre-reset report while the reset is still requested. The
   remediation adds an exact characterization test (96 characters reports, 97 and
   256 do not) but does not change the behavior. The mandatory-text envelope
   and report obligation remain an authority decision.
2. **Major — remains open (design).** Formatting is still performed before any
   deadline check and may scan an unbounded caller-provided `%s`. The
   remediation bounds the console/UART output and proves one shared deadline is
   observed by the notifier, but it does not bound the formatting work itself,
   and no target WCET is established.

## Delta confirmations

- No production bypass was introduced. The reset-result injection, the panic
  observer and the starve seam remain `FF_TEST_HOOKS`-guarded; the
  `nanosleep`/`write` wrappers are link-time options applied only to the two
  native test executables (`tests/CMakeLists.txt`), and the wrappers change
  only test-binary behavior.
- The loopback backend now implements `send_until` and honors an already
  expired deadline, so the host simulator exercises the same production
  reporter (`FF_LinkSendUntil`) as the target instead of dropping the frame;
  the new production-wiring case asserts that exactly one frame with the panic
  event id, severity and reason text reaches the link.
- The whole-path oracle is now direct rather than indirect: the notifier
  records its own entry time and the reset counter observed at notification
  time, so the harness shows that the blocked console consumed the shared
  budget before notification and that the reset request had not yet been made
  when the notification ran. The 2 s bound in that case is a hang guard, not an
  acceptance value.

## Conclusion

**REJECTED for closure of FRICK-REVIEW-0409.** Both Major findings remain
open; the characterization does not resolve the over-length policy or the
formatting deadline gap. No target WCET or hardware-reset behavior is
established.

**Confidence:** high for the source-path observations; no target behavior was
assessed. **Open gap:** authorized requirements/safety disposition of the
report-text envelope and independent target timing/reset evidence.
