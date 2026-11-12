# Systems Review Report — UART shared-fd concurrency fix in `link_uart.c` (R1/R3 Avionics Profile)

|                                   |                                        |
|----------------------------------:|:---------------------------------------|
| Review ID                         | SYS-REVIEW-R1-R3-20261112-link-uart-shared-fd-concurrency-avionics-systems-review |
| Reviewer profile                  | Senior Aeronautics and Space Avionics Systems Engineer (N9 R1/R3) |
| Norm profile                      | N4/N8/N11/N12/N15/N16/N17/N20/N21 |
| Repository                        | /home/nicola/fsw/frick-fsw |
| Branch                            | main |
| HEAD                              | aa86c84 |
| Working tree                      | Modified: `src/transport/link/link_uart.c` |
| Review date                       | 2026-11-12 |
| Review type                       | Avionics/systems independent review — internal fd-mode scoping change only |
| Change under review               | `uart_send_until()` in `src/transport/link/link_uart.c`: set `O_NONBLOCK` per-write and restore blocking mode immediately after, so a concurrent writer on the shared fd never observes non-blocking mode for longer than a single `write()` call |

---

## Authority statement

This review is an independent avionics/systems engineering analysis only. It does not approve airworthiness, qualification, certification, compliance, anomaly closure, release, or flight readiness. Independent qualified humans retain all safety-critical and regulatory decisions. No hardware, source-code interface, timing, FDIR, or operations interface is changed by the change under review.

---

## 1. Scope and change under review

Only `src/transport/link/link_uart.c` is modified. The diff re-scopes the `O_NONBLOCK` descriptor flag in `uart_send_until()`:

- **Before (defective):** `O_NONBLOCK` was set once on entry (`fcntl(F_SETFL, flags | O_NONBLOCK)` at ~old line 134) and held for the entire deadline-bounded retry loop; the original mode was restored only at function exit (`(void)fcntl(g_fd, F_SETFL, flags)` at ~old line 175). A concurrent writer on the shared fd (`uart_send`, line 107) therefore observed non-blocking mode for the whole duration of `uart_send_until()`.
- **After (fixed):** `O_NONBLOCK` is set immediately before each `write()` and the original mode is restored immediately after (`src/transport/link/link_uart.c:154-159`). The plain send path (`uart_send`, line 107-114) keeps its blocking semantics.

Function signature, return codes, and all external behavior are unchanged. `fcntl`/`F_GETFL`/`F_SETFL`/`O_NONBLOCK` were already used by the pre-existing code, so **no new syscall, library, or target capability is introduced**. `link_uart.c` is part of `FF_CORE_SOURCES` (`CMakeLists.txt:137`), common to the host simulator and the RTEMS 6.2 flight-shaped image (it reads `/dev/console` on target, per `docs/vv/target-io-coverage.md`), but the concurrency scenario is a shared-POSIX-fd concern and no target hardware/readiness item is touched.

---

## 2. Evidence verified

| Evidence item | Result | Reference |
|---|---|---|
| Diff scope | Only `src/transport/link/link_uart.c` modified; no `config/`, `tests/`, `tools/`, `verification/`, `board/`, ICD, SRS, SSS, or traceability change | `git diff HEAD -- src/transport/link/link_uart.c` |
| Build (user-reported) | `make build` exit 0, 0 warnings | Context (gates pass) |
| CTest (user-reported) | CTest exit 0, including `test_link_uart` | Context (gates pass) |
| Lean / trace / format / license | `lake build`, trace PASS (120/76/30/14/68), format-check PASS, license OK | Context (gates pass) |
| Blocking-restored assertion | Test asserts slave-fd `O_NONBLOCK` cleared after send operations | `tests/unit/test_link_uart.c:394-396` |
| Partial-progress / stall deadline test | Exercises `uart_send_until` short writes, stall timeout, and mode restoration | `tests/unit/test_link_uart.c`; `docs/vv/test-spec/ts-5-core-services.md:215` (N22-TS5-85) |

---

## 3. Findings

### SYS-R1/R3-001 — No system-level risk; internal fd-mode scoping change only (Compliant — no finding)

**File:line:** `src/transport/link/link_uart.c:133-176` (function `uart_send_until`); `src/transport/link/link_uart.c:107-114` (plain `uart_send`, unchanged)

**Description:** The change reduces — it does not increase — the window in which a concurrent writer on the shared fd observes non-blocking mode (from the whole function to a single `write()` call). Same function signature, same return codes (`FF_SUCCESS`/`FF_ERROR`/`FF_TIMEOUT`/`FF_INVALID_ARGUMENT`/`FF_NOT_READY`), no new syscall, no new dependency, no public API or interface change. Introduces **no system-level risk** to timing, power, thermal, radiation, EMC, FDIR, GNC, or operations.

**Classification:** Compliant — no finding.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-002 — NC-PAL-01, GAP-A5, and target/hardware items unaffected (Compliant — no finding)

**File:line:** N/A (no target/hardware file touched); `src/core/pal/pal_rtems.c` (NC-PAL-01, unwired watchdog) not modified.

**Description:** This is a POSIX shared-fd concurrency fix in the link layer. It is **not** GAP-A5 (target 250 ms bound / target readiness), **not** NC-PAL-01 (flight watchdog binding remains unwired), **not** OBCP engine related, and involves no hardware change. `fcntl`/`O_NONBLOCK` was already used by the pre-existing code, so the fix adds no new target capability or hardware dependency even though `link_uart.c` is built into the RTEMS flight-shaped image.

**Classification:** Compliant — no finding.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-003 — GNC, safety, FDIR, and operations interfaces unaffected (Compliant — no finding)

**File:line:** N/A (no interface file touched).

**Description:** No public API change: `uart_send_until(const uint8_t *, uint16_t, uint64_t)` retains its signature and semantics. No ICD, SRS, SSS, traceability, dispatch, degraded-mode, or crew/ground operations interface is modified. GNC, safety, and FDIR are not impacted.

**Classification:** Compliant — no finding.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-004 — Cross-disciplinary consistency; traceability unchanged (Compliant — no finding)

**File:line:** `docs/requirements/traceability.yaml`, SRS/SSS, ICD, SDD — untouched.

**Description:** Traceability counts unchanged (`trace` PASS 120/76/30/14/68). No cross-disciplinary interface, requirement, or traceability update is required or present for this internal locking change. No change to `docs/rules.md`, enforcement summary, or norms.

**Classification:** Compliant — no finding.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-005 — Per-write restore `fcntl` result ignored; fd may be left non-blocking on restore failure (Minor)

**File:line:** `src/transport/link/link_uart.c:159` — `(void)fcntl(g_fd, F_SETFL, flags);`

**Description:** If the restore `fcntl(F_SETFL, flags)` fails, the descriptor is left in `O_NONBLOCK` mode. A subsequent `uart_send()` (blocking path, line 107-114) would then observe non-blocking mode and could return `EAGAIN`/`EWOULDBLOCK`, reported as `FF_ERROR` (a spurious send failure) rather than blocking. This is a latent reliability issue, not a new failure introduced by this fix (the pre-existing code likewise ignored the end-of-function restore result). The unit test asserts blocking mode is restored after send (`tests/unit/test_link_uart.c:394-396`), but does not cover a restore-failure path.

**Classification:** Minor — reliability/latent-fault observation.

**Blocks commit/push:** **No.** Host gates are green and no safety-critical function depends on this path. Recommend recording as an NC for later disposition (N21) or checking/restoring the result in a subsequent pass.

---

### SYS-R1/R3-006 — Concurrency window reduced, not eliminated; no dedicated concurrent-writer test (Minor)

**File:line:** `src/transport/link/link_uart.c:154-159` — between `F_SETFL(O_NONBLOCK)` and `write()`, and between `write()` and the restore `F_SETFL`.

**Description:** A concurrent `uart_send()` on the shared fd can still observe non-blocking mode during the small window around a single `write()` call. The fix strictly improves on the prior whole-function exposure but does not eliminate the window. The test suite asserts blocking mode is restored post-send (`tests/unit/test_link_uart.c:394-396`) but does not exercise concurrent writers, so the residual race window is not directly tested.

**Classification:** Minor — verification-coverage observation.

**Blocks commit/push:** **No.** The residual window is inherent to process-global fd flags and is minimal; no new risk is introduced relative to the pre-existing design.

---

### SYS-R1/R3-007 — Per-iteration `fcntl` overhead; no functional or timing impact (Minor observation)

**File:line:** `src/transport/link/link_uart.c:154` and `:159` — two `fcntl` syscalls per `write()` iteration.

**Description:** Each `uart_send_until()` iteration now issues an extra `fcntl` pair (set `O_NONBLOCK`, restore blocking). At 115200 baud UART this is negligible and has no bearing on the 250 ms target bound (GAP-A5) or WCET; no timing guarantee is claimed or changed by this fix.

**Classification:** Minor — performance/observation only.

**Blocks commit/push:** **No.**

---

## 4. Compliance confirmations (no findings)

| ID | Confirmation | Basis |
|---|---|---|
| SYS-R1/R3-C01 | No system-level risk introduced | Diff confined to `link_uart.c:133-176`; no new syscall; exposure window narrowed |
| SYS-R1/R3-C02 | NC-PAL-01 / GAP-A5 / target-hardware unaffected | No PAL, watchdog, 250 ms bound, OBCP, or hardware change; `fcntl`/`O_NONBLOCK` pre-existing |
| SYS-R1/R3-C03 | GNC / safety / FDIR / operations interfaces unaffected | No public API/interface/ICD/SRS/SSS change |
| SYS-R1/R3-C04 | Cross-disciplinary consistency maintained; no traceability change | `trace` PASS 120/76/30/14/68; `traceability.yaml`/SRS/SSS/ICD/SDD untouched |
| SYS-R1/R3-C05 | Host gates sufficient for this change type | Build exit 0 (0 warnings); CTest exit 0 incl. `test_link_uart`; Lean, trace, format, license all pass |

---

## 5. Open safety-relevant items (unaffected, for awareness)

The following remain OPEN and escalated to the authorized human owner; they are **unaffected** by this change and are **not** closed by it:

- **NC-PAL-01** — Major FDIR: flight hardware watchdog binding unwired (`src/core/pal/pal_rtems.c` flight hooks `FF_NOT_IMPLEMENTED`).
- **NC-EXT-168** — Blocker FDIR: pre-reset panic path can block before `FF_PalWatchdogReset()`; blocks N7.
- **NC-EXT-129, NC-EXT-123, NC-EXT-166** — safety-relevant/open, awaiting owner disposition (`docs/status/escalations.md`).

None of these are touched by `link_uart.c:133-176`.

---

## 6. Conclusion

| Question | Answer |
|---|---|
| Does the fix introduce any system-level risk? | **No.** Internal fd-mode scoping change; same API/return codes; no new syscall; exposure window narrowed (`link_uart.c:154-159`). |
| Are NC-PAL-01, GAP-A5, or any target/hardware item affected? | **No.** POSIX shared-fd concurrency fix; no PAL/watchdog/250 ms/OBCP/hardware change. |
| Are GNC, safety, FDIR, or operations interfaces impacted? | **No.** No public API or interface change. |
| Is cross-disciplinary consistency maintained (no interface/requirements/traceability change needed)? | **Yes.** Traceability unchanged (120/76/30/14/68); no SRS/SSS/ICD/SDD modification. |
| Do any systems findings block commit/push? | **No.** All findings (SYS-R1/R3-005/006/007) are Minor and non-blocking; all gates pass. |

**Overall verdict:** The fix is benign from an avionics/systems perspective and introduces no system-level risk. It is a strict improvement over the prior whole-function `O_NONBLOCK` exposure. No finding blocks commit/push. Minor findings SYS-R1/R3-005 (ignored restore result) and SYS-R1/R3-006 (residual concurrency window / no concurrent-writer test) should be recorded as NCs for later review/disposition per N21; they do not stop this change. All open safety items remain escalated to the authorized human owner.

---

**Prepared by:** Independent review, Senior Aeronautics and Space Avionics Systems Engineer (R1/R3 profile)  
**Date:** 2026-11-12  
**Repository state:** HEAD aa86c84 + working-tree modification to `src/transport/link/link_uart.c` (uncommitted)
