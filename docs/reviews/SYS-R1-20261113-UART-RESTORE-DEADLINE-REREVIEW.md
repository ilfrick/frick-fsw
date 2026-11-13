# Independent R1/FSW Re-review — UART restore/deadline correction

**Date:** 2026-11-13. **Disposition:** no functional finding on the bounded
restore, errno, post-write deadline, or expired zero-length cases. This is not
qualification, NC closure, or release approval.

The reviewer inspected the current implementation, test, API contract,
traceability, sprint record and TS-2 cases 84–86, and ran the retained focused
test: **104 checks, 0 failures**. Restoration is checked before interpreting
write results; failure returns `FF_ERROR` and stops this call. Saved write errno
survives `fcntl`; successful positive writes receive a deadline post-check;
expired zero-length requests return `FF_TIMEOUT` before any write. Source comments
and traceability now state concurrent writers may observe temporary
nonblocking mode, and failed restoration can leave the descriptor mode uncertain.

**Residuals:** NC-EXT-191 remains OPEN for shared-descriptor effects and target
readiness/250 ms evidence. Exact package-wide manifest and other review-profile
dispositions remain separate, pending evidence. No target/WCET/SRS-REQ-074
compliance claim is made.
