# GSW UART E2E — Initial test failures

- Date: 2026-11-15
- Scope: host-SIL GSW CLI ↔ POSIX PTY bridge ↔ production UART backend ↔ PUS integration.
- Candidate base: `fc68b9d121bce2194c1beaaa9a7dc4cdab0552c` plus the current uncommitted S1/license candidate and E2E changes.
- No target, HIL, Docker, or Renode evidence is implied.

## Attempt 1 — stale CMake generation

Command:

```text
cmake --build build/s1-review --target test_gsw_uart_e2e_server -j2
```

Result: exit 2; `gmake: ***  Nessuna regola per generare l'obiettivo «test_gsw_uart_e2e_server».  Arresto.`

Cause: `build/s1-review` had not been reconfigured after adding the target. Corrective action: ran `cmake -S . -B build/s1-review`, which completed configuration and generation successfully; no source correction was needed.

## Attempt 2 — incorrect test oracle for the TM counter

Command:

```text
ctest --test-dir build/s1-review -R '^gsw_uart_e2e$' --output-on-failure
```

Initial result: failed in 0.43 s with `FAIL: TM sequence counter is not incrementing modulo 16384: [0, 0, 0, 0]`.

Cause: the test read the CCSDS primary-header sequence count, which is not the PUS TM counter. The PUS counter is in the PUS-C TM secondary header at bytes 9–10 of the packet. Corrective action: updated the independent decoder in `tests/python/test_gsw_uart_e2e.py` to extract and mask that field. No flight-software behavior was changed in response to this test-oracle error.

The corrected initial nominal CTest passed (1/1); the final focused run with nominal and OOB scenarios passed 2/2 (0.94 s); see `gsw-uart-e2e-ctest.log`.

## GSW `--ack none` advertised-option mismatch

An initial parser test exposed the exact failure `ValueError: unknown --ack stage: none`, despite the CLI help advertising `none`. The GSW parser was corrected to accept the exact `none` option as zero requested ACK stages, and `tests/python/test_ff_gsw.py` now checks both `none` and the empty value. The GSW unit suite passes 14/14; see `gsw-unit.log`.

## N22 OOB applicability

The initial E2E had only a nominal successful exchange. To add an applicable out-of-bound result, `gsw_uart_e2e_missing_ack` deliberately drops acceptance ACK TM[1,1] from the GSW receive path while retaining the server's full raw TM capture. The actual GSW CLI receives three TMs, exits 1 within 0.25 seconds, and reports the missing/duplicate/unexpected/failed response; it does not falsely succeed. This is a host-PTY fault injection, not evidence of physical UART reliability or NC-EXT-251 lock contention.

## Formatting check — initial failure and correction

Command: `clang-format --dry-run --Werror tests/integration/gsw_uart_e2e_server.c`

Initial result: failed with `tests/integration/gsw_uart_e2e_server.c:58:68: error: code should be clang-formatted [-Wclang-format-violations]`.

Corrective action: applied host clang-format 23.1.3 to the new C integration server. The targeted clang-format check, Python byte-compilation and `git diff --check` then passed. The full host build/test cycle was rerun after this formatting change.
