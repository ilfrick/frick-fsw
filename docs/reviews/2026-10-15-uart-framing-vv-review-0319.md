# UART stream-framing delta review (FRICK-REVIEW-0319)

| Field | Value |
| --- | --- |
| Profile | Independent SW V&V review |
| Date | 2026-10-15 |
| Scope | NC-EXT-07; `link_uart.c`; TS2-81 host PTY framing, timeout retention, ambiguous PDL handling |
| Baseline | Source HEAD `13ebf92` plus uncommitted UART and test changes |
| Verdict | **ACCEPTED for enumerated host PTY framing behavior; NC-EXT-07 remains OPEN** |

## Evidence

- Fresh focused CTest: `link_uart` passed 1/1; executable reported **46 checks,
  0 failures**.
- The PTY test uses the production `FF_LinkUartBackend` implementation. It
  covers split packet retention across timeout, two coalesced frames, preserving
  a complete packet after `FF_BAD_SIZE`, over-limit PDL bytewise recovery, a
  1037-octet configured maximum packet, delayed data with a `UINT32_MAX`
  timeout, and a plausible incomplete PDL candidate followed by a valid-looking
  packet. It confirms no inner packet is emitted and later confirms the retained
  candidate is returned only after its declared length is completed.
- `make rtems-docker` compiled and linked the RTEMS 6.2 target after replacing
  unavailable `poll()`/`select()` calls with the final termios VMIN/VTIME path.
  This is cross-build evidence only, not RTEMS runtime evidence.

## Disposition and limits

Accept the listed host PTY behaviors. Keep NC-EXT-07 OPEN because the termios
timeout is decisecond-granular (short requested timeouts can overrun by less
than 100 ms plus scheduling/driver latency), timeout tolerance and ambiguous
candidate discard/recovery are not authority-approved, and RTEMS termios
runtime/timing and physical UART behavior have not been verified. The receiver
state is single-consumer and callers must serialize. A plausible incomplete
candidate can hold later bytes across receive calls; this is explicitly tested
and is not claimed as general malformed-stream recovery.

Prior RTEMS builds of intermediate `poll()` and `select()` candidates failed
to link because those symbols are unavailable in the project toolchain. Those
attempts are superseded by the final successful termios cross-build and are not
success evidence for the final candidate.
