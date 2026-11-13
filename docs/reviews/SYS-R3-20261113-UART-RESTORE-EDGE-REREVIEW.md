# Independent R3 Systems/Avionics Re-review — UART API edges

**Date:** 2026-11-13. No new defect was found in the final-write or expired
zero-length deadline behavior. A positive final write is post-checked against
the monotonic deadline; TS-2 N22-TS2-85 observes `FF_TIMEOUT` with emitted bytes.
The expired zero-length case now checks the deadline before returning success;
N22-TS2-86 expects `FF_TIMEOUT` with no increase in write attempts.

Restoration and errno handling are bounded to the injected API subcases.
Concurrent writers may observe temporary `O_NONBLOCK`; failed restore can leave
the shared descriptor nonblocking. The current source comments, traceability,
test cases and sprint erratum retain those limitations. NC-EXT-191 remains OPEN;
no target readiness, 250 ms, WCET, HIL, or full SRS-REQ-074 conclusion is made.

This independent-context re-review is not qualification, release approval, or
NC closure.
