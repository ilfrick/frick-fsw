# PA Independent Review — UART `send_until` restore-result capture (THIS CHANGE)

| Field | Value |
|---|---|
| Review ID | PA-REVIEW-UART-SEND-UNTIL-RESTORE-RESULT-20261112 |
| Profile | Senior SW Product Assurance (N20) |
| Date | 2026-11-12 |
| Repository | /home/nicola/fsw/frick-fsw |
| Branch | main, HEAD 3d62d47 |
| Scope | Bounded functional increment: `src/transport/link/link_uart.c` only — working-tree change capturing the `fcntl(F_SETFL, flags)` restore return value in `uart_send_until()` |
| Change set (uncommitted) | `src/transport/link/link_uart.c` (1 file): `int restore_rc = fcntl(g_fd, F_SETFL, flags);` replacing `(void)fcntl(g_fd, F_SETFL, flags);`, plus `if (restore_rc != 0) result = FF_ERROR;` inside the `w > 0` branch |
| Authority-owned NCs touched | None — NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47 all confirmed NOT touched / NOT closed |
| Blocks commit/push | **No** (see findings; Finding PA-001 is Minor, non-blocking under N21) |
| Supersedes / relates to | PA-REVIEW-UART-SEND-UNTIL-CONCURRENCY-FIX-20261112.md (reviewed the first fix at HEAD aa86c84; raised PA-001 restore-result-ignored). THIS CHANGE partially addresses PA-001 but incompletely — see PA-001 below. |

---

## 1. Scope and independence (norms N4/N20)

This review is an independent N20 product-assurance assessment of the working-tree change at HEAD `3d62d47`. The reviewer is not the author of the code change, has no schedule/financial conflict on this increment, and does not hold release, safety, or certification authority. No compliance, qualification, safety, release, or NC-closure verdict is issued here; those remain with the authorized human authority.

The change is exactly one source file modified (`src/transport/link/link_uart.c`), no other source change, no test-script addition, no tool change, no requirement change. It is explicitly **not** hardware/target-related (POSIX file-descriptor issue, host-executable on Linux/RTEMS), and not GAP-A5 / NC-PAL-01 / OBCP-engine related.

**Baseline context:** Commit `3d62d47` (first fix, now in HEAD) changed `uart_send_until()` from holding `O_NONBLOCK` for the entire call duration to setting `O_NONBLOCK` per `write()` and restoring blocking mode immediately after — with the restore result discarded via `(void)`. THIS CHANGE (working tree) captures the restore return value so a restore failure is no longer silently ignored. THIS CHANGE does **not** re-introduce the end-of-function single restore; the per-write + immediate-restore structure from `3d62d47` is retained.

---

## 2. Verification of the fix (objective evidence)

### 2.1 What THIS CHANGE does

In `uart_send_until()` (link_uart.c:128-179), each loop iteration now:

1. Sets `O_NONBLOCK` for the write only (line 154; unchanged from `3d62d47`).
2. Calls `write()` (line 158).
3. Calls `fcntl(g_fd, F_SETFL, flags)` to restore blocking mode **immediately after** the write (line 159), capturing the return in `restore_rc`.
4. If `w > 0` (lines 160-166): advances `sent`, and **if `restore_rc != 0`, sets `result = FF_ERROR`** (lines 162-164) — then `continue`s.
5. If `w <= 0` (EINTR / EAGAIN / EWOULDBLOCK / other / zero): `continue` or `break` **without inspecting `restore_rc`** (lines 167-176).

Effect on the success path (`w > 0`): a restore failure is detected and reported as `FF_ERROR`. The concurrent writer never observes non-blocking mode longer than a single `write()` call, and a restore failure on that path is visible.

### 2.2 What THIS CHANGE does NOT do (gap)

`restore_rc` is inspected **only** inside the `w > 0` branch. On every other iteration outcome:

| Path | Code lines | `restore_rc` checked? | fd left in non-blocking mode if restore failed? |
|---|---|---|---|
| `w > 0` | 160-166 | **Yes** (result = FF_ERROR) | No — detected |
| `w == 0` (zero-byte write) | falls to 175-176 | **No** | **Yes — undetected** |
| `w < 0, EINTR` | 167-168 (`continue`) | **No** | **Yes — undetected** |
| `w < 0, EAGAIN/EWOULDBLOCK` | 170-173 (`continue`) | **No** | **Yes — undetected** |
| `w < 0, other error` | 175-176 (`break`) | **No** | **Yes — undetected** |

`EAGAIN`/`EWOULDBLOCK` is the expected retry condition in non-blocking mode when the UART transmit buffer is full — precisely the condition this loop is built to handle. If `fcntl(F_SETFL, flags)` restore fails on an `EAGAIN` iteration, the shared descriptor is left in non-blocking mode and the next concurrent `uart_send()` writer observes it — **the exact race this fix family removes**, now silent again on that path.

### 2.3 Verdict on correctness

The **primary defect** (shared-fd `O_NONBLOCK` held for the whole `send_until` duration) is correctly fixed by commit `3d62d47` (per-write set + immediate restore). THAT fix is verified and is the dominant risk reduction.

**THIS CHANGE** (restore-result capture) is **partially correct**: it detects restore failures only on the `w > 0` path. It does **not** deliver on its stated full purpose ("silent restore failure no longer leaves the shared fd in non-blocking mode undetected") because write-failure paths bypass the check. See Finding PA-001.

---

## 3. Gates re-executed on this tree (independent verification)

Fresh out-of-tree configure at `/home/nicola/fsw/frick-fsw/build/native_pa` (stale `/workspace`-rooted cache removed; not a code change).

| Gate | Command | Result | Evidence |
|---|---|---|---|
| Build | `cmake --build build/native_pa` | **PASS, exit 0, 0 warnings** on `link_uart.c` | no warning/error lines in output |
| CTest (all) | `ctest --test-dir build/native_pa` | **PASS, 56/56, exit 0** | includes `test_link_uart` PASS |
| CTest (UART focused) | `ctest -R link_uart` | **PASS, 1/1** | asserts fd flags restored to blocking after `send_until` |
| Traceability | `python3 tools/scripts/trace.py ...` | **PASS (120/76/30/14/68), exit 0** | unchanged counts; no new requirement |
| License | `python3 tools/scripts/license_check.py` | **OK, exit 0** | MPL-2.0 SPDX header intact |
| Format | `clang-format --dry-run --Werror` (unpinned override) | **exit 0, clean** | host clang-format is 23.x; project pins 18 — see PA-003 |
| Lean | `lake build` in `verification/lean` | **Build completed successfully, exit 0** | no new Lean artifact from this C-only change |

Note: `make format-check` on host rejects clang-format 23.x (project pins 18). The change was verified format-clean via `FF_CLANG_FORMAT_ALLOW_UNPINNED=1`. Pinned clang-format 18 confirmation remains the controlled V&V-image step (N6/N10).

---

## 4. NC / authority-owned items — confirmed NOT touched or closed

| NC / item | Register status | Touched by this change? | Closed by this change? |
|---|---|---|---|
| **NC-PAL-01** | OPEN — MPFS MSS watchdog flight binding unwired (`FF_NOT_IMPLEMENTED`) | **No** — POSIX fd issue; no watchdog/PAL/RTEMS binding code | **No** |
| **GAP-A5** | OPEN — on-target/environmental/radiation qualification; Renode/QEMU do not close | **No** — no target/HIL/timing claim | **No** |
| **NC-EXT-19 / NC-EXT-90** | OPEN — mission schedule/timing/policy inputs; scheduler wrap/exhaustion | **No** — unrelated to UART link backend | **No** |
| **N22-TS5-47** | OPEN — EVS-to-HS end-to-end integration | **No** — unrelated | **No** |
| **NC-EXT-191** | OPEN — UART shared-descriptor concurrency + target readiness semantics + target 250 ms bound | **Partially addressed** — exactly the `O_NONBLOCK` on shared fd sub-part (fixed by `3d62d47`; restore detection added by THIS CHANGE) | **No** — NC-EXT-191 remains OPEN (target readiness semantics and GAP-A5/N22-TS2-78 250 ms target bound remain; authority/design/HIL owned) |

This change addresses only the UART shared-descriptor concurrency half of NC-EXT-191. It does **not** close NC-EXT-191, and it does not close or touch NC-PAL-01, GAP-A5, NC-EXT-19/90, or N22-TS5-47. No authority-owned NC is dispositioned here.

---

## 5. Safety / security / flight / hardware impact assessment

| Question | Answer |
|---|---|
| Target/HIL timing (250 ms bound, GAP-A5)? | No impact claimed; no target readiness or WCET evidence added or removed. |
| Watchdog / flight binding (NC-PAL-01)? | No impact; `pal_rtems.c`, `pal_watchdog.c`, `app_wd.c` untouched. |
| OBCP engine? | No impact; `n7s-obcp` untouched. |
| RTEMS target build? | Change is POSIX `fcntl`/`write` on `g_fd`; compiles under RTEMS 6.2 `riscv/mpfs64imafdc` (no `poll()`/`select()` introduced). RTEMS cross-build was green on the current tree in the NC-EXT-165/N18 re-execution record. |
| Security (cryptography, keys, supply chain)? | No impact; no new dependencies, SBOM, or attack surface. |
| Concurrency safety (shared `g_fd`)? | **Improved** — the primary race is removed; restore-failure detection added on the `w > 0` path, but incomplete on write-failure paths (see PA-001). |

No safety, security, flight, or hardware impact is introduced by this change. It reduces a concurrency hazard in the panic/FDIR reporting path.

---

## 6. Findings

### PA-001 (Minor) — `F_SETFL` restore result inspected only on the `w > 0` path

**Location:** `src/transport/link/link_uart.c:159-165` (capture at line 159; check at lines 162-164, gated by `w > 0` at line 160).

**Observed fact:** `int restore_rc = fcntl(g_fd, F_SETFL, flags);` captures the restore result, but `if (restore_rc != 0) result = FF_ERROR;` appears only inside `if (w > 0)`. On `w == 0`, `EINTR`, `EAGAIN`/`EWOULDBLOCK`, and other write-error paths, execution `continue`s or `break`s without inspecting `restore_rc`.

**Gap / consequence:** `F_SETFL` can fail (e.g. `EBADF` if the descriptor were invalid). If the restore silently fails on a write-failure iteration, `g_fd` remains in `O_NONBLOCK` mode, and a subsequent concurrent `uart_send()` writer observes non-blocking mode and may receive `EAGAIN` — precisely the race this fix removes. Because `g_fd` is shared between the panic/FDIR path (`FF_LinkSendUntil` → `uart_send_until`) and the normal telemetry path (`uart_send`), an undetected restore failure reintroduces the concurrency hazard on the shared descriptor. The stated purpose of THIS CHANGE ("silent restore failure no longer leaves the shared fd in non-blocking mode undetected") is therefore only partially delivered.

**Risk class:** Minor. The primary defect is fixed on the success path by commit `3d62d47`; `fcntl(F_SETFL)` restore failure is a rare mode on a valid descriptor, and `g_fd` validity is checked at entry. Nevertheless, for ECSS Class B software, a shared-descriptor mode that must deterministically be blocking for the plain send path should have its restoration verified on every path, not only the success path.

**Recommended disposition:** Move the `restore_rc` check outside the `w > 0` branch — e.g. check `if (restore_rc != 0) result = FF_ERROR;` immediately after the `fcntl` call at line 159, before the `write` outcome branching — so a failure to restore blocking mode is detectable on every iteration. Track as an NC or address in a follow-up under N21 — non-functional/robustness finding, does not stop development.

**Blocks commit/push:** **No.**

---

### PA-002 (Minor) — No test coverage for the restore-failure path

**Location:** `tests/unit/test_link_uart.c` (no new test; `link_uart.c:159-164` new logic).

**Observed fact:** The existing test asserts fd flags are restored to blocking after a successful `send_until` (lines 392-398, under `FF_TEST_HAVE_WRITE_WRAP`). The test harness wraps `write()` but not `fcntl()`, so there is no deterministic way to inject an `fcntl(F_SETFL, flags)` restore failure. The new `result = FF_ERROR` branch on restore failure (link_uart.c:162-164) is therefore unexercised by tests.

**Gap / consequence:** The error-handling path added by THIS CHANGE has no test coverage. If the restore-check logic were later regressed, no test would catch it.

**Risk class:** Minor. The success-path restoration is tested; the untested path is an error-injection path requiring `fcntl` mocking not present in the harness.

**Recommended disposition:** Either extend the test harness to wrap `fcntl` and inject a restore failure, or record the coverage gap as an NC/follow-up item under N21. Does not block commit/push.

**Blocks commit/push:** **No.**

---

### PA-003 (Observation) — Format verification used unpinned clang-format (host 23.x vs pinned 18)

**Location:** Project-wide; `Makefile:206` pins clang-format 18.

**Observed fact:** Host `clang-format` is 23.1.3; `make format-check` rejects non-pinned major 18. The change was verified format-clean via `FF_CLANG_FORMAT_ALLOW_UNPINNED=1` (`--dry-run --Werror`, exit 0). Pinned clang-format 18 confirmation is pending execution in the controlled `frick-vv:local` V&V image.

**Gap / consequence:** None against the code itself; formatting is clean. This is an environment/tool-pin observation, not a defect. Per N6/N10, the pinned-tool verification must be completed in the controlled image before any commit/push that claims format conformity under the project's tool-qualification baseline.

**Recommended disposition:** Re-run `make format-check` (no override) in `frick-vv:local` with pinned clang-format 18 as part of the commit gate.

**Blocks commit/push:** **No** (condition: pinned-tool format check to be completed at commit gate; content itself is format-clean).

---

## 7. Release-assurance summary

| Item | Status |
|---|---|
| Primary fix correctness (shared-fd `O_NONBLOCK` race) | **Verified — commit 3d62d47, per-write set + immediate restore** |
| THIS CHANGE (restore-result capture) | **Partially verified — detects restore failure only on `w > 0` path (PA-001)** |
| Plain send path blocking semantics preserved | **Verified** |
| fd restored to blocking after `send_until` (test assertion) | **Verified — test_link_uart.c PASS** |
| Build (0 warnings), CTest 56/56, Lean, trace, license | **Verified green** |
| Format (pinned clang-format 18) | **Clean with available formatter; pinned-tool check pending at commit gate (PA-003)** |
| NC-PAL-01 / GAP-A5 / NC-EXT-19/90 / N22-TS5-47 | **Confirmed NOT touched, NOT closed** |
| NC-EXT-191 | **Partially addressed (UART concurrency sub-part); remains OPEN** — not closed here |
| Authority-owned NC disposition | **None issued** — reserved to authorized human authority |
| Findings that block commit/push | **None** |

---

## 8. PA recommendation

**Recommend commit/push: YES, with conditions.**

The `uart_send_until()` shared-descriptor concurrency fix is correct and minimal (one file), gate-green, and does not touch or close any authority-owned NC (NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47) and does not close NC-EXT-191 (which remains open for target readiness semantics and the GAP-A5/N22-TS2-78 250 ms target bound). THIS CHANGE adds restore-failure detection on the success path but incompletely (PA-001); this is a Minor, non-blocking robustness finding under N21.

Before push, ensure:

1. **PA-001** is recorded/tracked (NC or follow-up item) — restore check should cover all write outcomes, not only `w > 0`. Does **not** block push.
2. **PA-002** is recorded/tracked — restore-failure path is untested. Does **not** block push.
3. **PA-003**: pinned clang-format 18 format-check passes in `frick-vv:local`.
4. The change is reviewed and merged only by the designated `aeronautics-space-fsw-engineer` after all applicable gates/norms and full sprint scope pass (norm N7); this PA review is not a release or safety approval.

No safety-critical conclusion, waiver, residual-risk acceptance, or release decision is made by this review; those require independent qualified human authority.

---

*Report ends. Finding IDs: PA-001 (Minor), PA-002 (Minor), PA-003 (Observation). No finding blocks commit/push.*
