# Independent SW V&V Review — UART shared-descriptor concurrency fix (`uart_send_until()`)

| Field | Value |
|---|---|
| Review ID | VV-R2-20261112-UART-SEND-UNTIL |
| Reviewer profile | Senior Aeronautics and Space SW V&V Engineer (independent-context R2, norm N9/N10) |
| Repository | /home/nicola/fsw/frick-fsw, branch `main`, HEAD `3d62d47` |
| Change under review | Commit `3d62d47` "NC-EXT-191: resolve UART shared-descriptor concurrency in uart_send_until()" |
| Date | 2026-11-12 |
| Governing norms | N8, N10, N15, N20, N21, N22 (docs/rules.md); ECSS-E-ST-40C Rev.1 / ECSS-Q-ST-80C Rev.2 context |
| Criticality | ECSS Class B target (project target; no controlled mission assignment evidenced — NC-EXT-5/11 class) |
| Scope | `src/transport/link/link_uart.c` only (4 insertions, 1 deletion). Host/POSIX fd issue; NOT target/HIL; NOT GAP-A5/NC-PAL-01/OBCP; NC-EXT-191 UART part |

> **Authority statement.** This review is an independent analysis artifact. It has no approval, qualification, acceptance, certification, or flight-readiness authority. Commit/push authorization (norm N7), safety/security acceptance, PA sign-off (norm N20), and any residual-risk decision remain with the designated FSW engineer and the human authorities. This report may recommend "do not proceed"; it may not authorize proceeding.

## 1. Change under review (verified diff)

`git diff HEAD -- src/transport/link/link_uart.c` (4 insertions, 1 deletion):

```c
         ssize_t w = write(g_fd, data + sent, (size_t)(len - sent));
-        (void)fcntl(g_fd, F_SETFL, flags);
+        int restore_rc = fcntl(g_fd, F_SETFL, flags);
         if (w > 0) {
             sent = (uint16_t)(sent + (uint16_t)w);
+            if (restore_rc != 0) {
+                result = FF_ERROR;
+            }
             continue;
         }
```

Effect: `O_NONBLOCK` is set per write (`link_uart.c:154-157`) and blocking mode is restored immediately afterwards; the restore `fcntl` result is now captured (`:159`) and, on restore failure, `result = FF_ERROR` (`:162-164`) so the shared fd is not silently left non-blocking for the concurrent blocking `uart_send()` path (`uart_send` is unchanged at `:107-114`). The deadline-bounded retry loop (`:139-177`) and entry validation (`:130-136`) are unchanged.

## 2. Evidence basis

| ID | Evidence | Relevance |
|---|---|---|
| E-1 | `src/transport/link/link_uart.c:128-179` | Production function under review |
| E-2 | `tests/unit/test_link_uart.c:307-410` | Existing `send_until` cases: deadline pass/timeout, forced short writes, stall/timeout, past-deadline, zero deadline, flag restoration, closed-fd error |
| E-3 | `tests/unit/test_link_uart.c:394-396` | Flag-restoration assertion: `fcntl(F_GETFL)` on `g_uart_slave_fd`, `(flags & O_NONBLOCK) == 0` |
| E-4 | `tests/CMakeLists.txt:45-46` | `FF_TEST_HAVE_WRITE_WRAP=1` + `-Wl,--wrap=write` enabled for `test_link_uart`, so E-2/E-3 are active in the CI build |
| E-5 | `tests/CMakeLists.txt:33-36, 276-277` | `test_link_uart` target with `-Wall -Wextra -Werror`, registered as CTest `link_uart` |
| E-6 | `docs/vv/tool-qualification.md` rev 1.7, T3 row | T3 (unit tests incl. `test_link_uart.c`) status: NOT FULLY QUALIFIED for candidate (legacy NC-EXT-235); R2 delta review FRICK-REVIEW-0443 covers `test_link_uart.c` within its narrow scope |
| E-7 | `docs/requirements/traceability.yaml:571, 580, 697` | SRS-REQ-074 / IP/TSN lean_notes: "the UART shared-descriptor concurrency ... remain open", "NC stays open" |
| E-8 | `docs/rules.md` | N8 (docs updated same commit), N10 (tool/script qualification), N21 (sequencing), N22 (nominal + out-of-bound expected-result tests) |
| E-9 | `docs/vv/verification-results.md` (NC-EXT-191 / NC-EXT-237 cycle sections) | Reported cycle gates: build exit 0 / 0 warnings, CTest 56/56, Lean, trace PASS (120/76/30/14/68), format, license, RTEMS cross-build, Renode N18 PASS |
| E-10 | Independent re-execution | Fresh native build `/tmp/opencode/frick-build` from current sources, `test_link_uart`: `checks=77 failures=0`, exit 0; container prior build `build/ci/tests/test_link_uart`: `checks=77 failures=0`, exit 0 |

## 3. Findings

### VV-R2-001 — Minor — Concurrency race is covered at mechanism level, not by a multi-writer exercise

**Description.** The defect being fixed is a shared-fd race: `uart_send()` (blocking writer) vs `uart_send_until()` (briefly non-blocking writer) on the same descriptor `g_fd`. The fix (per-write `O_NONBLOCK` + immediate restore) is sound, but the existing test is **single-writer**: it deterministically forces short writes through `__wrap_write` (E-2, lines 321-343, `g_uart_forced_short_writes = 3`) and asserts flag restoration (lines 394-396), but it never runs a concurrent `uart_send()` writer alongside `uart_send_until()` on the shared fd. The interleaving the fix targets is therefore not directly exercised; coverage is mechanism-level (restore behavior, retry loop, deadline, error paths).

**File:line references.** `src/transport/link/link_uart.c:128-179`; `tests/unit/test_link_uart.c:307-346, 390-398`; `tests/CMakeLists.txt:45-46`.

**Observed result.** Independent run on current sources: `checks=77 failures=0`, exit 0 (E-10). All exercised cases pass.

**Blocks commit/push?** No.

**Disposition.** Track as an NC/verification improvement: either add a concurrent-writer soak test (one thread `uart_send`, one thread `uart_send_until`, shared fd; assert delivery integrity and flag restoration) or record an analysis justification that the single-writer retry/restore evidence suffices for the host POSIX increment. The UART concurrency portion of NC-EXT-191 should not be claimed as test-verified until this is dispositioned; target readiness semantics remain open regardless (E-7).

### VV-R2-002 — Minor — New restore-failure branch is not exercised by any test

**Description.** The change introduces a new error branch (`link_uart.c:159-164`): when the mode-restore `fcntl(F_SETFL, flags)` fails, `result = FF_ERROR` is set so the fd is not silently left non-blocking. No test seam can fail `fcntl` restore (no `__wrap_fcntl`), and `tests/unit/test_link_uart.c` contains no fcntl-fault injection. This branch is untested — a structural-coverage gap on the newly added code. A restore failure on a valid open fd is exceptional, so analysis justification is plausible, but it must be recorded rather than assumed.

**File:line references.** `src/transport/link/link_uart.c:159-164`; `tests/unit/test_link_uart.c` (no fcntl-fault seam).

**Blocks commit/push?** No.

**Disposition.** Add an `__wrap_fcntl` fault-injection case that forces restore failure and asserts `FF_ERROR`, or document the path as analysis-justified (exceptional condition, errno preserved, no silent state change). Register as a tracked NC pending next cycle. Does not affect the exercised-path evidence (E-10).

### VV-R2-003 — Minor — Traceability consistency risk under norm N8

**Description.** Norm N8 requires behavior/requirement-touching changes to update affected docs in the same commit. Commit `3d62d47` changes the behavior of `uart_send_until()` and claims to resolve the UART shared-descriptor concurrency part of NC-EXT-191, but `docs/requirements/traceability.yaml` (SRS-REQ-074 lean_note at line 697, and the IP/TSN lean_notes at lines 571 and 580) still states "the UART shared-descriptor concurrency ... remain open" / "NC stays open". Only the target-readiness-semantics part genuinely remains open. Leaving the note stale misstates the evidence state at the traceability gate (`trace.py` PASS 120/76/30/14/68 is structural convergence, not a correctness statement on these notes).

**File:line references.** `docs/requirements/traceability.yaml:571, 580, 697`; norm N8 (`docs/rules.md`).

**Blocks commit/push?** Conditionally no. If `traceability.yaml` is updated in this commit (UART concurrency marked resolved; target readiness explicitly remains open), this finding is closed and commit/push is not blocked by V&V. If left stale, it is an N8 non-compliance and must block the pre-push gate until corrected.

**Disposition.** Update the affected lean_notes in the same commit cycle (or as a tied doc change verified before push), then re-run `make trace`.

### VV-R2-004 — Minor (informational) — Tool qualification N10 is not triggered by this delta

**Description.** Norm N10 re-qualification triggers on a change to a qualified test script/V&V tool. This delta adds no test scripts and modifies no test script (`tests/unit/test_link_uart.c` is unchanged; context and E-2/E-3 confirm existing test only). T3's qualification status is unchanged: pre-existing NOT FULLY QUALIFIED for the candidate (legacy NC-EXT-235, E-6), independent delta review FRICK-REVIEW-0443 covered `test_link_uart.c` within its narrow scope; the unrelated `test_pus.c` sha mismatch in T3's register does not involve this change. No new qualification evidence or T3 re-qualification is required for this delta.

**File:line references.** `docs/vv/tool-qualification.md` rev 1.7 (T3 row); `tests/unit/test_link_uart.c` (unchanged).

**Blocks commit/push?** No. No N10 action required for this change. Pre-existing T3 qualification gaps remain owned elsewhere and are not introduced or worsened by this delta.

## 4. Confirmation observation (not a finding)

**Existing test behavior is unaffected.** `uart_send()` is untouched (E-1, lines 107-114). The write-wrap harness still targets `g_uart_slave_fd`, which resolves to the same production fd `g_fd` opened by `uart_init()` via the `FRICK_TEST_UART_DEVICE` symlink (test lines 193-206). The deadline/short-write/short-read cases (lines 307-376), past-deadline and zero-deadline cases (lines 377-389), the flag-restoration assertion (lines 394-396), and the error case on master-close (line 410) all retain their semantics: flags are restored to blocking after `send_until`, and no test expects the fd to remain non-blocking. Independent rebuild/run from current sources under `-Wall -Wextra -Werror`: 0 warnings, `checks=77 failures=0`, exit 0 (E-10); consistent with the reported CTest exit 0 including `test_link_uart.c`.

## 5. Answers to the review questions

1. **Is existing `test_link_uart.c` coverage sufficient?** For the host-level POSIX mechanism: yes, as unit-level verification evidence — nominal delivery, forced short-write/retry, deadline expiry, past-deadline/zero-deadline rejection, error path, and flag restoration are all covered with explicit expected-result oracles (norm N22). It is **not** sufficient to claim the concurrency race itself is proven fixed (no multi-writer interleaving) — see VV-R2-001.

2. **Are additional tests needed?** Recommended, not mandatory for this commit: (a) restore-failure fault injection or documented justification (VV-R2-002); (b) concurrent-writer soak or analysis justification for the race (VV-R2-001). No new test script is required, so no N10 trigger arises.

3. **Does the change affect existing test behavior?** No — confirmed by independent execution on current sources (E-10): identical `checks=77 failures=0`, exit 0; `uart_send` and the write-wrap target fd are unchanged.

4. **Tool qualification:** No new test scripts; no T3 impact; no N10 re-qualification trigger (VV-R2-004). T3's pre-existing "NOT FULLY QUALIFIED" status is unaffected by this delta.

5. **V&V findings blocking commit/push?** **None.** VV-R2-001/002/003 are Minor, non-blocking, with tracked dispositions. VV-R2-003 becomes a gate blocker only if `traceability.yaml` is not corrected (norm N8).

## 6. Verdict

**V&V review: PASS with tracked Minor findings.** All reported gates are green (build exit 0 / 0 warnings; CTest exit 0 including `test_link_uart.c`; Lean `lake build`; trace PASS 120/76/30/14/68; format-check PASS; license OK), and independent re-execution of `test_link_uart` from current sources passes (77 checks, 0 failures, exit 0).

No V&V finding blocks commit/push, provided VV-R2-003's traceability note is corrected in the same commit cycle (norm N8) and VV-R2-001/VV-R2-002 are registered as tracked NCs/improvements (norm N21 sequencing — functional defect corrected; verification gaps tracked, do not stop unrelated development).

Separate gates remain outside this review's authority: norm N7 commit/push authorization by the designated FSW engineer after full sprint scope, norm N20 Product Assurance acceptance, and any safety/security/residual-risk or release approval by the human authority. This report is evidence, not approval.

## 7. Finding register

| ID | Severity | File:line | Summary | Blocks commit/push? |
|---|---|---|---|---|
| VV-R2-001 | Minor | `src/transport/link/link_uart.c:128-179`; `tests/unit/test_link_uart.c:307-346, 390-398` | Race mechanism covered single-writer only; no concurrent-writer exercise | No (track as NC/improvement) |
| VV-R2-002 | Minor | `src/transport/link/link_uart.c:159-164` | Restore-failure `FF_ERROR` branch untested | No (add seam or justify; track NC) |
| VV-R2-003 | Minor | `docs/requirements/traceability.yaml:571, 580, 697`; norm N8 | Traceability notes stale vs. resolved UART concurrency | Conditional — yes if not corrected in same commit |
| VV-R2-004 | Minor | `docs/vv/tool-qualification.md` rev 1.7 (T3) | N10 not triggered; T3 status unchanged | No |
