# Review Report: independent SW V&V (norm N10 / R2 profile)

|                                   |                                        |
| ---------------------------------:| :------------------------------------- |
| Document id                      | FRICK-REVIEW-VV-R2-20261112            |
| Reviewer session                  | Independent agent (fresh context; no part in authoring/executing the increment), norm N9 |
| Reviewer profile                  | R2 - Senior SW V&V Engineer            |
| Reference revision                | `main`, HEAD `aa86c84a42a5f90a1447fe322cee020ad0f4925e` |
| Date                              | 2026-11-12                             |
| Items under review                | `src/transport/link/link_uart.c` (`uart_send_until()` per-write `O_NONBLOCK` toggle + restore); existing coverage in `tests/unit/test_link_uart.c:307-410` |
| Scope                             | V&V evidence sufficiency for the bounded host-executable POSIX-fd concurrency fix; need for additional tests; effect on existing tests; T3/tool-qualification trigger; findings that block commit/push. Excluded: target/HIL, RTEMS target timing, Renode N18, and any requirement outside the link UART `send_until` path. |
| Norms applied                     | N1-N22 (see `docs/rules.md`); ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C) |

## 1. Scope and increment summary

**Increment (bounded functional change).** Only `src/transport/link/link_uart.c` is modified (`git status`: ` M src/transport/link/link_uart.c`; `git diff` = 10 insertions, 2 deletions). No new test scripts; `tests/unit/test_link_uart.c` is unchanged.

The defect: `uart_send_until()` previously switched the shared descriptor `g_fd` to `O_NONBLOCK` once at entry and restored blocking mode only at function exit. While `send_until` was in progress (bounded by its deadline, with retries), any concurrent writer on the same fd — notably the plain blocking `uart_send()`/`FF_LinkUartBackend.send` path — could observe non-blocking mode for the whole duration, breaking `uart_send`'s blocking contract.

The fix: `F_SETFL(flags | O_NONBLOCK)` is now issued immediately before each `write()` (link_uart.c:154-157) and `F_SETFL(flags)` restores blocking mode immediately after each `write()` (link_uart.c:159), so a concurrent writer never observes non-blocking mode for longer than a single `write()` call. The function-exit restore was removed as redundant (every loop iteration restores; every `break` exits with blocking mode already restored).

**Evidence base relied upon by the increment (per the task context and verified against the tree):**

| Gate | Result | Reference |
|---|---|---|
| `make build` | exit 0, 0 warnings | task context; `docs/vv/verification-results.md` NC-EXT-191 cycle |
| CTest (incl. `test_link_uart.c`) | exit 0, 56/56 PASS | NC-EXT-191 cycle gate-ctest.log |
| `make verify-lean` (`lake build`) | exit 0 | verification-results.md NC-EXT-191 cycle |
| `make trace` | PASS (120/76/30/14/68) | verification-results.md NC-EXT-191 cycle |
| `make format-check` | exit 0 | verification-results.md NC-EXT-191 cycle |
| N13 license | OK | verification-results.md NC-EXT-191 cycle |
| T3 / N10 re-qualification | No trigger (no script changed) | `docs/vv/tool-qualification.md` T3 row |

## 2. Existing coverage in `tests/unit/test_link_uart.c` (verified by inspection)

| Lines | Case | What it evidences |
|---|---|---|
| 307-311 | `send_until` success + short-read verification; `FF_TIMEOUT` on already-expired deadline | nominal delivery + deadline rejection on entry |
| 312-346 | 60000-byte tx with `g_uart_forced_short_writes = 3` via `__wrap_write`; drain thread; `FF_TIMEOUT` guard; asserts all injected short writes consumed and all bytes received | partial-progress/retry loop (`w > 0`, `EAGAIN`/`EWOULDBLOCK` back-off) under forced short writes |
| 347-376 | stalled UART (no drain) → `FF_TIMEOUT` before deadline; observes partial emission (`0 < total < sizeof(stalled)`) | bounded timeout + partial emission characterized, not hidden |
| 377-384 | deadline in the past → `FF_TIMEOUT` immediately | entry deadline rejection |
| 385-389 | deadline == 0 → `FF_TIMEOUT` immediately | degenerate deadline |
| 394-396 | `fcntl(g_uart_slave_fd, F_GETFL)`; asserts `(flags & O_NONBLOCK) == 0` | **restore-after-write behavior** (end state after `send_until` returns) |
| 404-411 | closed PTY master → `FF_ERROR` | disconnected-UART failure path |

Note: `g_uart_slave_fd` capture (204-207) and the flag-restoration assertion (392-398) are guarded by `#ifdef FF_TEST_HAVE_WRITE_WRAP`. The test file already uses pthreads elsewhere (`uart_delayed_writer`, lines 145-152; writer thread at 274-277) but only for the **receive** path (writing to `g_master_fd`); there is no concurrent writer on the **shared slave fd** during `send_until`.

## 3. Findings (VV-R2-XXX)

Severity definitions (per `docs/reviews/review-template.md`):
- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified.
- **Confirmation**: no non-conformity; evidence checked and satisfactory.

### VV-R2-001 — Minor — BLOCKS COMMIT/PUSH: No

**Item / requirement:** Concurrency-safety evidence for the per-write `O_NONBLOCK` toggle — `src/transport/link/link_uart.c:128-176` (fix; intent comment at :150-153); coverage `tests/unit/test_link_uart.c:307-410`.

**Finding:** The fix's stated purpose is concurrency safety: a concurrent writer on the shared fd must never observe non-blocking mode for longer than a single `write()` call. All existing `send_until` cases are **single-threaded**. No case runs two writers concurrently on `g_uart_slave_fd` (e.g., `FF_LinkUartBackend.send`/`uart_send` alongside `send_until`) during `send_until`. Therefore the concurrency property itself is supported by code inspection plus the end-state flag assertion, not by a concurrent-execution oracle. The flag-restoration assertion at :394-396 validates restore-after-write **at return**, not per-write restore under simultaneous writers.

**Evidence:** `test_link_uart.c` has pthread infrastructure (lines 145-152, 274-277) but uses it only for the receive path; no `send` + `send_until` concurrency case exists. `link_uart.c:150-153` documents the concurrency intent as a comment/oracle, not as a test assertion.

**Recommendation / disposition:** Add a concurrent-writer regression case, **or** document why it is not deterministically achievable on the host PTY harness and record it as a tracked NC/improvement under norms N12/N21 (deferred, non-functional; does not stop this change). The end-state contract is directly asserted and all gates pass, so this is a verification-coverage gap, not a functional defect.

**Status:** Open (tracked NC/improvement). Does not block commit/push.

### VV-R2-002 — Minor — BLOCKS COMMIT/PUSH: No

**Item:** Scope of the flag-restoration assertion — `tests/unit/test_link_uart.c:392-398` (and `g_uart_slave_fd` capture at :204-207).

**Finding:** The restoration assertion `(flags & O_NONBLOCK) == 0` (:394-396) is guarded by `#ifdef FF_TEST_HAVE_WRITE_WRAP` (:392); `g_uart_slave_fd` capture is likewise conditional (:204-207). On hosts/toolchains without linker `--wrap` support, the restore-after-write behavior is **not asserted** by `test_link_uart.c`. Coverage of the key behavioral contract is therefore conditional on the build configuration.

**Evidence:** `test_link_uart.c:39-46` (`__wrap_write` only active under `FF_TEST_HAVE_WRITE_WRAP`); :204-207; :392-398. The CI/gate host uses `--wrap` (the wrap-based short-write cases at :321-343 execute), so the gate evidence exercises the assertion, but the conditional scope is not reflected in the V&V evidence map.

**Recommendation / disposition:** Make the restoration assertion unconditional, or record the conditional scope (and the configurations in which restore-after-write is asserted) in the V&V evidence map / traceability justification.

**Status:** Open (tracked NC/improvement). Does not block commit/push.

### VV-R2-003 — Confirmation (no NC) — BLOCKS COMMIT/PUSH: No

**Item:** Tool qualification / T3 re-qualification trigger — `docs/vv/tool-qualification.md` (T3 row); `tests/unit/test_link_uart.c` (unchanged).

**Finding:** Confirmed. No new test scripts are added and `test_link_uart.c` is unchanged (`git status`: only `src/transport/link/link_uart.c` modified). `test_link_uart.c` remains within the previously R2-reviewed T3 scope (FRICK-REVIEW-0443 covered `tests/unit/test_link_uart.c`; see `docs/vv/tool-qualification.md` T3 row). Per norm N10 ("any change to a recorded V&V script invalidates its qualification"), since no V&V script/tool changed, no T3 re-qualification trigger arises. No new evidence-claim dependency on an unqualified tool is introduced.

**Status:** Satisfied. Does not block commit/push.

### VV-R2-004 — Confirmation (no NC) — BLOCKS COMMIT/PUSH: No

**Item:** Effect of the change on existing test behavior — `tests/unit/test_link_uart.c:39-46` (`__wrap_write` target), :307-410 (send_until cases); `src/transport/link/link_uart.c:157-159` (`write` + immediate restore).

**Finding:** Confirmed no adverse effect. `uart_send_until` still issues a single `write()` to `g_fd`/`g_uart_slave_fd` — the same call-site wrapped by `__wrap_write` (test_link_uart.c:39-46), so the forced-short-write oracle still targets the production fd. `O_NONBLOCK` is restored immediately after each `write()` (:159) and the end state is asserted after `send_until` returns (:394-396), so `uart_send`/`FF_LinkUartBackend.send` retains blocking semantics. Gate evidence (NC-EXT-191 cycle): 56/56 CTest pass including `test_link_uart`, build exit 0 / 0 warnings, Lean, trace PASS (120/76/30/14/68), format-check PASS, license OK.

**Status:** Satisfied. Does not block commit/push.

### VV-R2-005 — Context / pre-existing open item — BLOCKS COMMIT/PUSH: No

**Item:** NC-EXT-191 residual parts — `docs/requirements/traceability.yaml:697` (SRS-REQ-074 lean_note); `docs/vv/verification-results.md` (NC-EXT-191 cycle, 2026-11-11).

**Finding:** Context only, not a finding caused by this increment. This increment addresses the **UART shared-descriptor concurrency** part of NC-EXT-191 (per-write non-blocking toggle + restore). The **target-readiness-semantics** part (no `poll()`/`select()` on RTEMS 6.2; readiness semantics unverified, see `link_uart.c:122-126` and ADR-0021) and on-target link timing/HIL (GAP-A5 / N22-TS2-78) remain open and tracked separately. NC-EXT-191 therefore remains open overall; this increment does not close it, and it is not claimed to.

**Status:** Pre-existing open item (tracked elsewhere). Does not block commit/push.

## 4. Sufficiency conclusion

**1. Is existing `test_link_uart.c` coverage sufficient verification evidence for this concurrency fix?**
Yes for the bounded, host-executable, POSIX-fd-internal change at the current gate state. The evidence directly covers every behavior the fix touches: send/short-write/short-read deadline cases (:307-376), entry-expired and zero deadlines (:377-389), disconnected-UART failure (:404-411), and — most importantly — the restore-after-write behavior the fix is designed to preserve (:394-396, end-state `(flags & O_NONBLOCK) == 0`). The partial-progress retry loop is deterministically exercised via forced short writes (:321-343). All gates pass.

The one gap is **direct concurrency evidence**: no test runs two writers simultaneously on the shared fd, so the stated concurrency intent ("never observes non-blocking mode for longer than a single `write()` call") rests on code inspection + end-state assertion rather than a concurrent-execution oracle. This is recorded as VV-R2-001 (Minor, non-blocking).

**2. Are additional tests needed?**
Not as a condition of commit/push for this bounded increment. The existing flag-restoration assertion validates the restore-after-write behavior, and the short-write retry loop validates the loop the fix preserves. Recommended (not required for this cycle): a concurrent-writer regression case or a documented justification/tracked NC (VV-R2-001), plus making the restoration assertion unconditional or scoping it in the evidence map (VV-R2-002).

**3. Does the change affect existing test behavior?** No — confirmed by VV-R2-004; write wrap still targets `g_uart_slave_fd`; flags restored after `send_until`; no CTest regression.

**4. Tool qualification / T3:** No new test scripts; no T3 re-qualification trigger; confirmed by VV-R2-003.

**5. Findings that block commit/push:** None. No Major findings. Two Minor findings (VV-R2-001, VV-R2-002) are recorded as tracked NCs/improvements under norms N12/N21; they do not block commit/push because all gates pass, the change is bounded/internal, and N21 permits deferring non-functional findings without stopping development.

**Verdict:** V&V evidence is sufficient for this increment; no finding blocks commit/push. Commit/push authority remains with the `aeronautics-space-fsw-engineer` agent under norm N7 (all gates/norms + full sprint scope), and any applicable PA (N20) / documentation (N17) reviews remain separate gates. This report is a recommendation and evidence record, not an approval or release authority.

## 5. Follow-up NC register (norm N12)

| NC | Severity | Item / requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|----|----------|--------------------|---------|-------------|--------------------------|-------|--------|-----------------|
| VV-R2-001 | Minor | `link_uart.c:128-176`; `test_link_uart.c:307-410` | Concurrency property not directly exercised (no concurrent writer on shared fd) | Add concurrent-writer case, or document host-PTY limitation + tracked improvement | Concurrent-writer test run / documented justification | FSW engineer | Open | Required at closure |
| VV-R2-002 | Minor | `test_link_uart.c:392-398` | Flag-restoration assertion conditional on `--wrap` | Make assertion unconditional or scope conditional coverage in evidence map | Unconditional assertion run, or evidence-map entry | FSW engineer | Open | Required at closure |

Committed and pushed with the reviewed change (norm N11) once all applicable gates pass.
