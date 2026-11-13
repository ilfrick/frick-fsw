# Systems Review Report — `uart_send_until()` restore-`fcntl` result capture on top of per-write `O_NONBLOCK` toggle (R1/R3 Avionics Profile)

|                                   |                                        |
|----------------------------------:|:---------------------------------------|
| Review ID                         | SYS-REVIEW-R1-R3-20261112-uart-send-until-restore-result-avionics-systems-review |
| Reviewer profile                  | Senior Aeronautics and Space Avionics Systems Engineer (N9 R1/R3) — independent review |
| Norm profile                      | N4/N8/N11/N12/N15/N16/N17/N20/N21 |
| Repository                        | /home/nicola/fsw/frick-fsw |
| Branch                            | main |
| Baseline (committed)              | HEAD 3d62d47 `NC-EXT-191: resolve UART shared-descriptor concurrency in uart_send_until()` |
| Working tree                      | ` M src/transport/link/link_uart.c` (uncommitted) |
| Review date                       | 2026-11-12 |
| Review type                       | Avionics/systems independent review — internal fd-mode scoping change only |
| Change under review               | `uart_send_until()` in `src/transport/link/link_uart.c`: on top of HEAD's per-write `O_NONBLOCK` toggle (`link_uart.c:154-157`), the restore `fcntl(F_SETFL, flags)` result is now captured (`link_uart.c:159`); if restore fails, `result = FF_ERROR` is set (`link_uart.c:162-164`) so the fd is not silently left non-blocking |

---

## Authority statement

This review is an independent avionics/systems engineering analysis only. It does not approve airworthiness, qualification, certification, compliance, anomaly closure, release, or flight readiness. Independent qualified humans retain all safety-critical and regulatory decisions. No hardware, source-code interface, timing, FDIR, or operations interface is changed by the change under review.

---

## 1. Scope and change under review

Baseline HEAD 3d62d47 already committed the core concurrency fix: `O_NONBLOCK` is set immediately before each `write()` and the original mode is restored immediately after (`link_uart.c:154-159`), so a concurrent writer on the shared fd (`uart_send`, `link_uart.c:107-114`) never observes non-blocking mode for longer than a single `write()` call. Function signature, return codes (`FF_SUCCESS`/`FF_ERROR`/`FF_TIMEOUT`/`FF_INVALID_ARGUMENT`/`FF_NOT_READY`), and all external behavior are unchanged. No new syscall, library, or target capability is introduced.

The working-tree delta on top of HEAD 3d62d47 (`git diff HEAD`):

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

Effect: HEAD ignored the restore result with `(void)`, so a restore failure left the shared fd silently in `O_NONBLOCK` mode. The working tree captures the result and records `FF_ERROR`, making a restore failure detectable rather than silent. On restore failure the loop still `continue`s (no `break`), so the fd may remain non-blocking until a later iteration's restore succeeds; see finding SYS-R1/R3-005.

`link_uart.c` is part of `FF_CORE_SOURCES`, common to the host simulator and the RTEMS 6.2 flight-shaped image (reads `/dev/console` on target, per `docs/vv/target-io-coverage.md`), but the concurrency scenario is a shared-POSIX-fd concern and no target hardware/readiness item is touched.

---

## 2. Evidence verified

| Evidence item | Result | Reference |
|---|---|---|
| Diff scope | Only `src/transport/link/link_uart.c` modified; no `config/`, `tests/`, `tools/`, `verification/`, `board/`, ICD, SRS, SSS, or traceability change | `git status --short`; `git diff HEAD` |
| Baseline content | HEAD 3d62d47 already contains per-write `O_NONBLOCK` toggle and `(void)`-ignored restore at `link_uart.c:154-159` | `git show HEAD:src/transport/link/link_uart.c` |
| Build (context/reported) | `make build` exit 0, 0 warnings | Task context; `docs/status/status.json` |
| CTest (context/reported) | CTest exit 0 including `test_link_uart.c` | Task context |
| Lean / trace / format / license | `lake build`, trace PASS (120/76/30/14/68), format-check PASS, license OK | Task context; `docs/status/status.json` |
| Open NC awareness | NC-PAL-01 (watchdog unwired), GAP-A5 (target/HIL), NC-EXT-19/90 open; unaffected by this change | `docs/status/escalations.md`; `docs/status/status.json` |
| Prior independent reviews | R1/R3, PA (N20), VV (R2) reviews for this increment exist and concur: no finding blocks commit/push | `docs/reviews/SYS-REVIEW-R1-R3-20261112-link-uart-shared-fd-concurrency-*.md`; `PA-REVIEW-UART-SEND-UNTIL-CONCURRENCY-FIX-20261112.md`; `VV-R2-20261112-link-uart-send-until-concurrency.md` |

---

## 3. Findings

### SYS-R1/R3-001 — No system-level risk; internal fd-mode scoping change only (Minor — no risk finding)

**File:line:** `src/transport/link/link_uart.c:128-176` (function `uart_send_until`); `src/transport/link/link_uart.c:107-114` (plain `uart_send`, unchanged)

**Description:** The change reduces — it does not increase — the window in which a concurrent writer on the shared fd observes non-blocking mode (whole function → single `write()` call), and it makes a restore failure detectable instead of silent. Same function signature, same return codes, no new syscall, no new dependency, no public API or interface change. Introduces **no system-level risk** to timing, power, thermal, radiation, EMC, FDIR, GNC, or operations. Strictly improves on HEAD 3d62d47, which discarded the restore result with `(void)`.

**Classification:** Minor — observation / no-risk confirmation.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-002 — NC-PAL-01, GAP-A5, and target/hardware items unaffected (Minor — no-risk confirmation)

**File:line:** N/A (no target/hardware/PAL file touched); `src/core/pal/pal_rtems.c` (NC-PAL-01, unwired watchdog) not modified.

**Description:** This is a POSIX shared-fd concurrency fix in the link layer. It is **not** GAP-A5 (target 250 ms bound / target readiness), **not** NC-PAL-01 (flight watchdog binding remains unwired), **not** OBCP engine related, and involves no hardware change. `fcntl`/`O_NONBLOCK` was already used by the pre-existing code, so the fix adds no new target capability or hardware dependency even though `link_uart.c` is built into the RTEMS flight-shaped image. No WCET, 250 ms bound, or target/HIL claim is added or removed.

**Classification:** Minor — scope confirmation.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-003 — GNC, safety, FDIR, and operations interfaces unaffected (Minor — no-risk confirmation)

**File:line:** N/A (no interface file touched).

**Description:** No public API change: `uart_send_until(const uint8_t *, uint16_t, uint64_t)` retains its signature and semantics. No ICD, SRS, SSS, traceability, dispatch, degraded-mode, or crew/ground operations interface is modified. GNC, safety, and FDIR are not impacted. The panic/FDIR reporting path (`FF_LinkSendUntil` → `uart_send_until`) benefits from the narrowed concurrency window; no safety function's behavior or allocation changes.

**Classification:** Minor — interface confirmation.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-004 — Cross-disciplinary consistency; traceability unchanged (Minor — no-risk confirmation)

**File:line:** `docs/requirements/traceability.yaml`, SRS/SSS, ICD, SDD — untouched.

**Description:** Traceability counts unchanged (`trace` PASS 120/76/30/14/68). No cross-disciplinary interface, requirement, or traceability update is required or present for this internal locking change. No change to `docs/rules.md`, enforcement summary, or norms.

**Classification:** Minor — consistency confirmation.

**Blocks commit/push:** **No.**

---

### SYS-R1/R3-005 — Restore failure sets `result = FF_ERROR` but loop continues; fd may remain non-blocking until next successful restore (Minor — latent reliability/reporting observation)

**File:line:** `src/transport/link/link_uart.c:159-164` — `int restore_rc = fcntl(g_fd, F_SETFL, flags);` followed by `if (restore_rc != 0) { result = FF_ERROR; }` inside `if (w > 0) { ... continue; }`.

**Description:** Capturing the restore result is an improvement over HEAD's `(void)` ignore: a restore failure is now reflected in `result`. However, on restore failure the loop does **not** `break`; it `continue`s, so the shared fd may remain in `O_NONBLOCK` mode until a later iteration's restore succeeds. Consequences: (a) data may be fully delivered yet reported `FF_ERROR` (spurious send failure / false negative on success), or (b) a subsequent `uart_send()` blocking-path caller could observe non-blocking mode and receive `EAGAIN`/`EWOULDBLOCK`, reported as `FF_ERROR`. `F_SETFL` failure is rare (e.g. `EBADF` only if the descriptor became invalid; `g_fd` is validated at entry) and the unit test asserts blocking mode is restored after send (`tests/unit/test_link_uart.c` flag-restoration assertion), but the restore-failure path is not exercised. This is a latent reliability/reporting issue, not a new safety hazard introduced by this fix.

**Classification:** Minor — latent fault / error-reporting observation.

**Blocks commit/push:** **No.** Host gates are green and no safety-critical function depends on this path. Recommend recording as an NC for later review/disposition (N21) or tightening restore-failure handling (break with `FF_ERROR`, or retry/restore) in a subsequent pass.

---

## 4. Compliance confirmations (no findings)

| ID | Confirmation | Basis |
|---|---|---|
| SYS-R1/R3-C01 | No system-level risk introduced | Diff confined to `link_uart.c:128-176`; no new syscall; exposure window narrowed; restore failure now detected |
| SYS-R1/R3-C02 | NC-PAL-01 / GAP-A5 / target-hardware unaffected | No PAL, watchdog, 250 ms bound, OBCP, or hardware change; `fcntl`/`O_NONBLOCK` pre-existing |
| SYS-R1/R3-C03 | GNC / safety / FDIR / operations interfaces unaffected | No public API/interface/ICD/SRS/SSS change |
| SYS-R1/R3-C04 | Cross-disciplinary consistency maintained; no traceability change | `trace` PASS (120/76/30/14/68); `traceability.yaml`/SRS/SSS/ICD/SDD untouched |
| SYS-R1/R3-C05 | Host gates sufficient for this change type | Build exit 0 (0 warnings); CTest exit 0 incl. `test_link_uart`; Lean, trace, format, license all pass |

---

## 5. Open safety-relevant items (unaffected, for awareness)

The following remain OPEN and escalated to the authorized human owner; they are **unaffected** by this change and are **not** closed by it:

- **NC-PAL-01** — Major FDIR: flight hardware watchdog binding unwired (`src/core/pal/pal_rtems.c` flight hooks `FF_NOT_IMPLEMENTED`).
- **GAP-A5** — target/HIL evidence and 250 ms target bound (N22-TS2-78); open until mission use, not a commit/push blocker.
- **NC-EXT-19/90** — mission schedule/timing/policy inputs; authority-dependent.
- **NC-EXT-191** — the UART shared-descriptor concurrency sub-part is resolved by HEAD 3d62d47 (per-write toggle); the working-tree restore-result capture is a robustness follow-up. Target-readiness semantics and on-target timing remain open and tracked separately; NC-EXT-191 is not closed by this working-tree change.

None of these are touched by `link_uart.c:128-176`.

---

## 6. Conclusion

| Question | Answer |
|---|---|
| Does the fix introduce any system-level risk? | **No.** Internal fd-mode scoping change; same API/return codes; no new syscall; exposure window narrowed; restore failure now detectable (`link_uart.c:159-164`). |
| Are NC-PAL-01, GAP-A5, or any target/hardware item affected? | **No.** POSIX shared-fd concurrency fix; no PAL/watchdog/250 ms/OBCP/hardware change. |
| Are GNC, safety, FDIR, or operations interfaces impacted? | **No.** No public API or interface change. |
| Is cross-disciplinary consistency maintained (no interface/requirements/traceability change needed)? | **Yes.** Traceability unchanged (120/76/30/14/68); no SRS/SSS/ICD/SDD modification. |
| Do any systems findings block commit/push? | **No.** All findings (SYS-R1/R3-001 to 005) are Minor and non-blocking; all gates pass. |

**Overall verdict:** The working-tree change is benign from an avionics/systems perspective and introduces no system-level risk. It is a strict robustness improvement over HEAD 3d62d47 (restore failures are now detected rather than silently ignored). Minor finding SYS-R1/R3-005 (restore failure does not break the loop; fd may remain non-blocking until next successful restore; possible spurious `FF_ERROR`) should be recorded as an NC for later review/disposition per N21; it does **not** stop this change. All open safety items remain escalated to the authorized human owner.

**Commit/push recommendation:** No systems-level objection to commit/push, subject to the separate PA (N20), documentation (N17), V&V (N10), and `aeronautics-space-fsw-engineer` gate (N7) being completed and all applicable norms passing for the full sprint scope.

---

**Prepared by:** Independent review, Senior Aeronautics and Space Avionics Systems Engineer (R1/R3 profile)  
**Date:** 2026-11-12  
**Repository state:** HEAD 3d62d47 + uncommitted working-tree modification to `src/transport/link/link_uart.c`
