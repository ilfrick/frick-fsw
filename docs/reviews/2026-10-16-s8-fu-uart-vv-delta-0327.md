# UART disconnect/test-path V&V delta review (FRICK-REVIEW-0327)

| Field | Value |
| --- | --- |
| Profile | Independent Senior SW V&V Engineer (R2) |
| Date | 2026-10-16 |
| Scope | UART zero-byte disconnect behavior and test-only per-run PTY path; TS-2 N22-TS2-81 mapping |
| Baseline | HEAD `13ebf92` plus uncommitted candidate |
| Verdict | **Host behavior accepted with documentation finding; no broader requirement/target approval** |

## Bounded acceptance

The reviewer rebuilt `test_link_uart` and ran focused CTest; 1/1 passed. The
backend returns `FF_ERROR` for a substantially early zero-byte read and retains
ordinary VTIME expiry toward timeout. The disconnected-PTY test asserts error
status and zero length; it does not establish an elapsed-time bound or RTEMS
behavior. The test-only environment override and `mkdtemp` directory avoid the
previous shared fixed path. Malformed plausible-PDL recovery remains open under
NC-EXT-07; no inner-boundary heuristic is accepted.

## Finding

1. **Minor — N22-TS2-81 is mapped to the wrong requirement.** SRS-REQ-016 states
   portability (host and RTEMS build); the host PTY case characterizes PDL
   framing/disconnect behavior and does not verify that portability requirement.
   The case should be marked as NC-driven characterization without SRS credit,
   or mapped to a controlled behavior requirement. Keep the open NC-EXT-07 and
   do not imply mission UART acceptance.

This review is limited to the test/source correction. It does not close
NC-EXT-07, establish target or physical-UART behavior, qualify a tool, or pass
N20/release gates.
