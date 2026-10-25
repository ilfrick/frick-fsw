# FRICK-REVIEW-0416 — S8-FDIR-01 final delta review (R1)

**Profile:** R1 FSW, fresh context  
**Disposition:** Review evidence. No approval, NC closure, safety acceptance or
target WCET finding.

## Scope

Final delta since FRICK-REVIEW-0414: the loopback `send_until` implementation,
the production-wiring panic test, the direct whole-path oracle additions and
the UART short-write injection. The reviewer made no edit to the repository and
reproduced its observations on an out-of-tree copy with enumerated mutations.

## Findings

1. **Major — remains open (policy).** `src/core/pal/pal_posix.c` still suppresses
   the notifier when `vsnprintf` reports truncation, so an over-length reason
   yields no pre-reset report while the reset is still requested. The delta adds
   an exact characterization test (96 characters report, 97 and 256 do not) but
   does not change the behavior. The mandatory-text envelope and report
   obligation remain an authority decision (NC-EXT-184).
2. **Major — remains open (design).** Formatting is still performed before any
   deadline check and may scan an unbounded caller-provided `%s`. The delta
   bounds the console/UART output and proves that one shared deadline is
   observed by the notifier, but it does not bound the formatting work itself,
   and no target WCET is established (NC-EXT-185).
3. **Minor — loopback deadline honoured on entry only.** `loop_send_until`
   checked the deadline once and then called `loop_send`, which takes the mutex
   with `FF_PAL_WAIT_FOREVER` and copies without a further deadline
   evaluation, so the host backend was not deadline-equivalent to the target
   backend. **Corrected in this round:** the deadline is re-checked after the
   copy, and a dedicated case covers the expired/future budget behavior.
4. **Minor — the expired-deadline behavior had no test.** Deleting the
   loopback deadline check left the whole 44-test suite green. **Corrected in
   this round** by `test_link_deadline_writer_semantics`.
5. **Minor — the production-wiring case did not verify "exactly one frame".**
   It popped one frame and checked its content, so a duplicated emission could
   still pass. **Corrected in this round:** a second pop must return
   `FF_TIMEOUT`.
6. **Minor — the UART short-write case fails by hang, not by a located
   failure.** When the sender returned early, the drain thread kept waiting and
   the case ended as a CTest timeout. **Corrected in this round:** the drain
   thread is released on a failed send.
7. **Minor — the compiler guard on the link-time wrappers was ineffective.**
   The test sources referenced `__real_write`/`__real_nanosleep`
   unconditionally, so a toolchain without `--wrap` would fail to link instead
   of degrading gracefully. **Corrected in this round:** the sources are guarded
   by the same condition as the linker option.
8. **Minor — a `vsnprintf` error could leave an unspecified buffer in use.**
   On a negative return the buffer contents are unspecified, yet the text was
   still formatted into the console message. **Corrected in this round:** the
   buffer is cleared and the reason is treated as incomplete.
9. **Minor — `FF_LinkLoopbackPop` dereferenced `len` unconditionally.**
   **Corrected in this round:** the output pointers are validated.
10. **Comment — the host simulator still does not render the pre-reset report.**
    The frame is enqueued in the loopback transmit ring, which only the test
    harness drains. The records were corrected to state this precisely.
11. **Comment — the retained RTEMS artifact was stale** and still exported the
    test seams. **Corrected in this round:** a fresh RTEMS 6.2 cross-build with
    the cached toolchain image produces a flight artifact that exports neither
    the test seams nor `poll`.

## Delta confirmations

- **No production bypass.** Every new seam is `FF_TEST_HOOKS`-guarded and the
  flight build defines that macro only for non-RTEMS targets; the link-time
  wrappers are attached only to the two host test executables.
- The whole-path oracle is direct and sensitive: making the console attempt
  return immediately, or moving the reset request before the notification,
  fails the case. The 2 s bound in that case is a hang guard, not an
  acceptance value.
- The UART short-write injection is correctly scoped to the PTY slave
  descriptor, is fail-closed, and cannot corrupt the other cases in that
  binary.

## Conclusion

**REJECTED for closure of FRICK-REVIEW-0409/0414.** Both Major findings remain
open; the delta resolves neither the over-length policy nor the unbounded
formatting work. The Minor findings of this round were corrected in the
follow-up round and are recorded as NC-EXT-190.

**Confidence:** high for the source-path, mutation and symbol observations. No
target behavior was assessed.
