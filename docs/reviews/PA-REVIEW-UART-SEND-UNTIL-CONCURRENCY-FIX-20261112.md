# PA Independent Review — UART `send_until` shared-descriptor concurrency fix

| Field | Value |
|---|---|
| Review ID | PA-REVIEW-UART-SEND-UNTIL-CONCURRENCY-FIX-20261112 |
| Profile | Senior SW Product Assurance (N20) |
| Date | 2026-11-12 |
| Repository | /home/nicola/fsw/frick-fsw |
| Branch | main, HEAD aa86c84 |
| Scope | Bounded functional increment: `src/transport/link/link_uart.c` only (`uart_send_until()` shared-fd `O_NONBLOCK` race) |
| Change set (uncommitted) | `src/transport/link/link_uart.c` (1 file, +13/−5 approx.) |
| Authority-owned NCs touched | None — NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47, NC-EXT-239 all confirmed NOT touched / NOT closed |
| Blocks commit/push | **No** (see findings) |

---

## 1. Scope and independence (norms N4/N20)

This review is an independent N20 product-assurance assessment of the working-tree change at HEAD `aa86c84`. The reviewer is not the author of the code change, has no schedule/financial conflict on this increment, and does not hold release, safety, or certification authority. No compliance, qualification, safety, release, or NC-closure verdict is issued here; those remain with the authorized human authority.

The change is documentation/register discipline for the code itself: exactly one source file modified (`src/transport/link/link_uart.c`), no other source change, no test-script addition, no tool change, no requirement change. It is explicitly **not** hardware/target-related (POSIX file-descriptor issue, host-executable on Linux/RTEMS), and not GAP-A5 / NC-PAL-01 / OBCP-engine related.

---

## 2. Verification of the fix (objective evidence)

### 2.1 The defect (before)

`uart_send_until()` (link_uart.c:128) previously executed:

```c
int flags = fcntl(g_fd, F_GETFL, 0);
if ((flags < 0) || (fcntl(g_fd, F_SETFL, flags | O_NONBLOCK) != 0)) {
    return FF_ERROR;
}
...
while (sent < len) {
    ... deadline check ...
    ssize_t w = write(g_fd, data + sent, len - sent);
    ...
}
(void)fcntl(g_fd, F_SETFL, flags);   /* single restore at the end */
```

`g_fd` is shared with the normal blocking path `uart_send()` (link_uart.c:107) and with the panic/FDIR reporter `FF_LinkSendUntil` → `uart_send_until`. While `send_until` held `O_NONBLOCK` for its entire (bounded-retry, deadline-bounded) duration, any concurrent `uart_send` writer on the same fd observed non-blocking mode and could receive `EAGAIN` unexpectedly — a real concurrency race in the panic/FDIR path.

### 2.2 The fix (after)

The working-tree `uart_send_until()` now:

1. Captures `flags` once at entry (`F_GETFL`, link_uart.c:133).
2. Inside the retry loop, sets `O_NONBLOCK` **per `write()` only** (link_uart.c:154).
3. Calls `write()` (link_uart.c:158).
4. Restores blocking mode **immediately after** each `write()` (link_uart.c:159: `(void)fcntl(g_fd, F_SETFL, flags);`).
5. Removed the single end-of-function restore (deleted old link_uart.c:175 line).

Effect: a concurrent writer on the shared fd never observes non-blocking mode for longer than a single `write()` call (plus a few microseconds for the restore syscall), instead of for the whole `send_until` duration. The plain `uart_send` path keeps its blocking semantics.

**Verdict on correctness: VERIFIED.** The fix directly and correctly addresses the shared-descriptor concurrency race. The plain send path retains blocking semantics; `send_until` retains its deadline-bounded non-blocking writes with immediate mode restore. This matches the change description and the test assertion in `tests/unit/test_link_uart.c:392-398` (flags restored to blocking after `send_until`).

### 2.3 Gates re-executed on this tree (build/native-pa, fresh configure)

| Gate | Command | Result | Evidence |
|---|---|---|---|
| Build | `cmake --build build/native-pa -jN` | **PASS, exit 0, 0 warnings** on `link_uart.c`; no warning/error lines in output | fresh configure at current path |
| CTest | `ctest --test-dir build/native-pa` | **PASS, 56/56, exit 0** | includes `test_link_uart` (Test #30) PASS 0.38s |
| CTest (UART focused) | `ctest -R link_uart` | **PASS, 1/1, exit 0** | asserts fd flags restored to blocking after `send_until` |
| Traceability | `python3 tools/scripts/trace.py ...` | **PASS (120/76/30/14/68), exit 0** | unchanged counts; no new requirement |
| License | `python3 tools/scripts/license_check.py` | **OK, exit 0** | MPL-2.0 SPDX header intact |
| Format | `clang-format --dry-run --Werror` (unpinned override) | **exit 0, clean** | host clang-format is 23.x; project pins 18 — see PA-002 observation |
| Lean | `lake build` in `verification/lean` | **Build completed successfully, exit 0** | no new Lean artifact from this C-only change |

Note: `make format-check` on host rejects clang-format 23.x (project pins 18); the change was verified format-clean with the available formatter via `FF_CLANG_FORMAT_ALLOW_UNPINNED=1`. Pinned clang-format 18 confirmation remains the controlled V&V-image step (N6/N10) and is not a finding against this change.

---

## 3. NC / authority-owned items — confirmed NOT touched or closed

| NC / item | Register status (sprint-plan.md) | Touched by this change? | Closed by this change? |
|---|---|---|---|
| **NC-PAL-01** | OPEN — MPFS MSS watchdog flight binding unwired (`FF_NOT_IMPLEMENTED`) | **No** — POSIX fd issue; no watchdog/PAL/RTEMS binding code | **No** |
| **GAP-A5** | OPEN — on-target/environmental/radiation qualification; Renode/QEMU do not close | **No** — no target/HIL/timing claim | **No** |
| **NC-EXT-19 / NC-EXT-90** | OPEN — mission schedule/timing/policy inputs; scheduler wrap/exhaustion | **No** — unrelated to UART link backend | **No** |
| **N22-TS5-47** | OPEN — EVS-to-HS end-to-end integration | **No** — unrelated | **No** |
| **NC-EXT-239** | OPEN — pre-existing RTEMS `pal_rtems.c:156` implicit-declaration warning | **No** — pre-existing, outside `link_uart.c` | **No** |
| **NC-EXT-191** | OPEN — UART shared-descriptor concurrency + target readiness semantics + target 250 ms bound | **Partially addressed** — exactly the `O_NONBLOCK` on shared fd sub-part is fixed | **No** — NC-EXT-191 remains OPEN (target readiness semantics and GAP-A5/N22-TS2-78 250 ms target bound remain; those are authority/design/HIL owned) |

This change addresses only the UART shared-descriptor concurrency half of NC-EXT-191. It does **not** close NC-EXT-191, and it does not close or touch NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47, or NC-EXT-239. No authority-owned NC is dispositioned here.

---

## 4. Safety / security / flight / hardware impact assessment

| Question | Answer |
|---|---|
| Target/HIL timing (250 ms bound, GAP-A5)? | No impact claimed; no target readiness or WCET evidence added or removed. |
| Watchdog / flight binding (NC-PAL-01)? | No impact; `pal_rtems.c`, `pal_watchdog.c`, `app_wd.c` untouched. |
| OBCP engine? | No impact; `n7s-obcp` untouched. |
| RTEMS target build? | Change is POSIX `fcntl`/`write` on `g_fd`; compiles under RTEMS 6.2 `riscv/mpfs64imafdc` (no `poll()`/`select()` introduced). RTEMS cross-build was green on the current tree in the NC-EXT-165/N18 re-execution record. |
| Security (cryptography, keys, supply chain)? | No impact; no new dependencies, SBOM, or attack surface. |
| Concurrency safety (shared `g_fd`)? | **Fixed** — the race is removed; the fd is restored to blocking immediately after each write. |

No safety, security, flight, or hardware impact is introduced by this change. It reduces a concurrency hazard in the panic/FDIR reporting path.

---

## 5. Findings

### PA-001 (Minor) — `F_SETFL` restore return value ignored on the shared descriptor

**Location:** `src/transport/link/link_uart.c:159`

**Observed fact:** After each `write()`, the descriptor is restored to the captured `flags` via:

```c
(void)fcntl(g_fd, F_SETFL, flags);
```

The return value of the restore `fcntl` is discarded with `(void)`.

**Gap / consequence:** `F_SETFL` can fail (e.g. `EBADF` if the descriptor were invalid, though `g_fd` is validated at entry). If the restore silently fails, `g_fd` remains in `O_NONBLOCK` mode, and a subsequent concurrent `uart_send()` writer would observe non-blocking mode and could receive `EAGAIN` — precisely the race this fix removes. Because `g_fd` is shared between the panic/FDIR path and the normal telemetry path, an undetected restore failure reintroduces the concurrency hazard on the shared descriptor.

**Risk class:** Minor. The primary defect is fixed on the success path; `fcntl(F_SETFL)` restore failure is a rare mode, and `g_fd` validity is checked at entry. Nevertheless, for ECSS Class B software, a shared-descriptor mode that must deterministically be blocking for the plain send path should not have its restoration silently ignored.

**Recommended disposition:** Capture the restore `fcntl` result; on failure, break the loop with `FF_ERROR` (or retry/restore), so a failure to restore blocking mode is detectable rather than silent. Track as an NC or address in a follow-up under N21 — non-functional/robustness finding, does not stop development.

**Blocks commit/push:** **No.**

---

### PA-002 (Observation) — Format verification used unpinned clang-format (host 23.x vs pinned 18)

**Location:** Project-wide; `Makefile:206` pins clang-format 18.

**Observed fact:** Host `clang-format` is 23.1.3; `make format-check` rejects non-pinned major 18. The change was verified format-clean via `FF_CLANG_FORMAT_ALLOW_UNPINNED=1` (`--dry-run --Werror`, exit 0). Pinned clang-format 18 confirmation is pending execution in the controlled `frick-vv:local` V&V image.

**Gap / consequence:** None against the code itself; formatting is clean. This is an environment/tool-pin observation, not a defect. Per N6/N10, the pinned-tool verification must be completed in the controlled image before any commit/push that claims format conformity under the project's tool qualification baseline.

**Recommended disposition:** Re-run `make format-check` (no override) in `frick-vv:local` with pinned clang-format 18 as part of the commit gate.

**Blocks commit/push:** **No** (condition: pinned-tool format check to be completed at commit gate; content itself is format-clean).

---

## 6. Release-assurance summary

| Item | Status |
|---|---|
| Fix correctness (shared-fd `O_NONBLOCK` race) | **Verified — addresses the defect** |
| Plain send path blocking semantics preserved | **Verified** |
| fd restored to blocking after `send_until` (test assertion) | **Verified — test_link_uart.c PASS** |
| Build (0 warnings), CTest 56/56, Lean, trace, license | **Verified green** |
| NC-PAL-01 / GAP-A5 / NC-EXT-19/90 / N22-TS5-47 / NC-EXT-239 | **Confirmed NOT touched, NOT closed** |
| NC-EXT-191 | **Partially addressed (UART concurrency sub-part); remains OPEN** — not closed here |
| Authority-owned NC disposition | **None issued** — reserved to authorized human authority |
| Findings that block commit/push | **None** |

---

## 7. PA recommendation

**Recommend commit/push: YES, with conditions.**

The `uart_send_until()` shared-descriptor concurrency fix is correct, minimal (one file), gate-green, and does not touch or close any authority-owned NC (NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47, NC-EXT-239) and does not close NC-EXT-191 (which remains open for target readiness semantics and the GAP-A5/N22-TS2-78 250 ms target bound). The single Minor finding (PA-001, restore return value ignored) is a robustness recommendation that does **not** block commit/push under N21; it should be tracked as an NC or addressed in a follow-up. The format observation (PA-002) requires the pinned-tool check at the commit gate.

Before push, ensure:
1. PA-001 is recorded/tracked (NC or follow-up item) — does not block push.
2. PA-002: pinned clang-format 18 format-check passes in `frick-vv:local`.
3. The change is reviewed and merged only by the designated `aeronautics-space-fsw-engineer` after all applicable gates/norms and full sprint scope pass (norm N7); this PA review is not a release or safety approval.

No safety-critical conclusion, waiver, residual-risk acceptance, or release decision is made by this review; those require independent qualified human authority.

---
*Report ends. Finding IDs PA-001 (Minor), PA-002 (Observation). No finding blocks commit/push.*
