# TS-9 — OBCP adapter/engine connectivity

| Field | Value |
| ----- | ----- |
| Document ID | FRICK-VV-TS-9 |
| Revision | 0.4 |
| Date | 2026-10-16 |
| Status | Host connectivity test specification; limited evidence |
| Implementation | `tests/unit/test_obcp_integration.c` |
| Requirement relation | Supports integration evidence for SRS-REQ-045; does not verify requirement completion |

## 1. Test objective and boundary

Verify that the pinned N7 engine is linked through `FF_Obcp_t` adapter and that
the Python OBCP modules dispatch to the supplied C callback implementations.
Callbacks are test doubles; this test does not verify production service
bindings, engine correctness, target portability, timing/resource budgets, PUS
S18, or mission suitability. The upstream engine's own tests are not rerun.

## 2. Preconditions

- Configure with `FF_USE_N7_OBCP=ON`, `FF_BUILD_TESTS=ON`, native platform.
- Provide non-null callback functions, a caller-owned MicroPython heap and packet
  buffer, and initialized PAL.
- Run under CTest as `obcp_engine_connectivity` with a 30 s external test timeout.

## 3. Test cases

| ID | Input/condition | Expected result |
| -- | --------------- | ---------------- |
| TS9-01 | Initialize adapter with a required service callback missing | `FF_ObcpInit` returns `FF_INVALID_ARGUMENT` and does not initialize the engine |
| TS9-02 | Initialize adapter with complete callback test doubles, static heap and packet buffer | `FF_ObcpInit` returns `FF_SUCCESS`; `FF_ObcpGetStatus` reports READY before execution |
| TS9-03 | Script writes/reads integer parameter 100 = 42 | `FF_ObcpExecutePy` returns `FF_SUCCESS`; callbacks observe ID/value and Python readback equals 42 |
| TS9-04 | Script writes/reads enum 101 = 3, float 102 = 1.25, Boolean 103 = true | Each typed callback observes its ID/value and Python readback equals the written value |
| TS9-05 | Script sends event ID 513 and writes text | Event callback observes event 513 during the script; output callback receives nonzero text length |
| TS9-06 | Script invokes beginstep/endstep ID 9 with success | Step callbacks observe ID 9 and successful completion |
| TS9-07 | Script invokes wait(5), waituntil(20, 10), and gettime() | Callbacks observe exact arguments; Python receives `(12, 34)` for gettime |
| TS9-08 | Script checks packet availability and available channel 2, sends `b'OK'`, and receives `b'OK'` with timeout 7 | Packet callbacks observe channel 2, expected bytes and timeout; Python byte-string assertion completes |
| TS9-09 | Query status after script returns | State is RETURNED and `engine_reported_success` is true; this is the upstream return value, not proof of interpreter-level success |
| TS9-10 | Pass a null script pointer or zero-length script | Adapter returns `FF_INVALID_ARGUMENT` without invoking the engine |
| TS9-11 | Pass length `FF_OBCP_SCRIPT_MAX_BYTES + 1` | Adapter returns `FF_BAD_SIZE` before reading script bytes or invoking engine |
| TS9-12 | After `gettime()` assertion, script sends event ID 514 | Callback observes event 514, independently proving execution passed the final script assertion despite the upstream execute Boolean limitation |

The test passes only when all explicit assertions in `test_obcp_integration.c`
hold. It deliberately does not test cancellation during arbitrary Python loops,
failed-script result mapping, memory exhaustion, WCET, or physical/RTEMS target
behavior; those remain NC-S9-01/deferred evidence.

## 4. Evidence history

| Revision | Date | Author | Change |
| -------- | ---- | ------ | ------ |
| 0.1 | 2026-10-16 | frick-fsw | Specify first N7 OBCP adapter connectivity test and explicit scope limits |
| 0.2 | 2026-10-16 | frick-fsw | Extend connectivity coverage to all typed datapool, time, and packet callback paths |
| 0.3 | 2026-10-16 | frick-fsw | Correct receive-packet oracle to compare Python bytes with a byte string; extend assertions through the final script operations because upstream Boolean does not expose interpreter failures |
| 0.4 | 2026-10-16 | frick-fsw | Add READY-state oracle before execution and a post-final-assertion event sentinel as an independent Python completion oracle |
