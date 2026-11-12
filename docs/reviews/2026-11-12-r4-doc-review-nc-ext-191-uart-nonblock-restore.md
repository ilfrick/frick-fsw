# R4 Independent Documentation Review — frick-fsw (norm N17)

**Reviewer profile:** Senior Aeronautics and Space Software Documentation Engineer (R4). No engineering/safety/certification/release authority; findings are advisory. Only the `aeronautics-space-fsw-engineer` agent (or an explicitly delegated human) may commit/push, and only after all applicable gates pass.

**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD aa86c84
**Review date:** 2026-11-12
**Scope:** Uncommitted bounded functional increment in `src/transport/link/link_uart.c` (`uart_send_until()`): per-write `O_NONBLOCK` with immediate restore of blocking mode on the shared UART file descriptor, to fix NC-EXT-191 UART shared-descriptor concurrency. No API, interface, public-contract, traceability-requirement, or new-symbol change. Not hardware/target related (POSIX fd, host-executable). NOT GAP-A5/NC-PAL-01/OBCP. NC-EXT-191 remains open (target readiness / GAP-A5 / target 250 ms) and is NOT closed by this increment.

**Gate status observed (per context):** `make build` exit 0 (0 warnings); CTest exit 0 (incl. `test_link_uart.c`); Lean `lake build` PASS; `make trace` PASS (120/76/30/14/68); format-check PASS; license OK. Norm N2: all comments English. Norm N16: no revision-history documentation files changed (only `link_uart.c` source).

---

## 1. Determinations against the review tasks

### Task 1 — Does any documentation need updating for this change?

**Determination: No.**

Rationale:
- `uart_send_until()` is static/internal; the public entry point `FF_LinkSendUntil()` returns the same statuses (`FF_SUCCESS`, `FF_TIMEOUT`, `FF_NOT_READY`, `FF_INVALID_ARGUMENT`, `FF_ERROR`) with unchanged semantics.
- `src/transport/link/link.h:30-41` documents the `send_until` return contract and backend-capability statement; none of it depends on when `O_NONBLOCK` is set/restored internally. No interface change.
- No traceability requirement change; no new public symbol; no user/operator/maintenance-visible behavior change.
- Therefore no SRS/SSS, ICD, TS-x test spec, design/SDD, user/operator/maintenance doc, release note, or traceability matrix requires a change for this increment. Norm N8 (complete, current, deployable documentation) is satisfied without a documentation change.

### Task 2 — Norm N2 (English) in the added comment

**Determination: PASS.**

The added comment at `src/transport/link/link_uart.c:150-153` is written in English. No non-English content.

### Task 3 — Norm N8

**Determination: PASS — no documentation change needed.**

Confirmed in Task 1. The documentation set remains complete, current, and deployable for this internal locking change.

### Task 4 — Does the added comment accurately describe the change?

**Determination: Yes — accurate.**

Added comment (`link_uart.c:150-153`):

> /* Set O_NONBLOCK for this write only, then restore blocking mode
>  * immediately, so a concurrent writer on the shared fd (uart_send)
>  * never observes non-blocking mode for longer than a single write()
>  * call. The plain send path keeps its blocking semantics. */

Code it describes (`link_uart.c:154-159`):

```c
if (fcntl(g_fd, F_SETFL, flags | O_NONBLOCK) != 0) {   // set per-write
    result = FF_ERROR;
    break;
}
ssize_t w = write(g_fd, data + sent, (size_t)(len - sent));
(void)fcntl(g_fd, F_SETFL, flags);                       // restore immediately after write
```

Verification of each claim:
- "Set O_NONBLOCK for this write only": `O_NONBLOCK` is set inside the `while (sent < len)` loop, immediately before each `write()`, and was removed from the function-entry point. ✓
- "restore blocking mode immediately": `(void)fcntl(g_fd, F_SETFL, flags);` executes immediately after `write()`, before any branch/continue/nanosleep. ✓
- "concurrent writer ... never observes non-blocking mode for longer than a single write() call": the descriptor is non-blocking only across uart_send_until's own `write()` call; `uart_send()` (link_uart.c:107-114) performs a plain blocking write and never touches the flags, so a concurrent writer on the shared fd observes non-blocking mode only within uart_send_until's single write window. ✓
- "The plain send path keeps its blocking semantics": `uart_send()` is unchanged and performs blocking `write()` without `O_NONBLOCK`. ✓

All exit paths of `uart_send_until()` leave the descriptor in its original (blocking) mode (verified: `flags < 0` return before any set; deadline/clock break before any set; `fcntl` set failure leaves flags unset; every `write()` is followed by restore; loop completion returns after the last restore). The pre-existing header comment at `link_uart.c:116-127` ("The descriptor is switched to non-blocking mode ... and the original mode is restored afterwards ... so the normal (blocking) send path keeps its semantics") remains accurate and needs no change.

### Task 5 — Documentation findings that block commit/push

**Determination: None.**

No Major findings. No Minor findings. No documentation finding blocks commit/push.

---

## 2. Findings register

| Finding ID | Classification | File:line | Summary | Blocks commit/push? |
|---|---|---|---|---|
| — | — | — | No documentation findings raised for this increment. | No |

**Major findings:** 0.
**Minor findings:** 0.

Note: Existing DOC-R4-001..006 from the 2026-11-11 R4 review (NC-EXT-237/NC-EXT-191 backend-coverage cycle) are unrelated to this increment and remain as recorded; none of them concerns `link_uart.c` or the `O_NONBLOCK` restore behavior, and none blocks this functional increment under N21 (non-functional findings do not stop unrelated development).

---

## 3. Confirmations (no findings)

| # | Item | Checked at | Result |
|---|------|------------|--------|
| C-1 | Norm N2 (English) — added comment | `src/transport/link/link_uart.c:150-153` | **Pass.** Comment is English. |
| C-2 | Comment accuracy vs code | `link_uart.c:150-159` | **Accurate.** Per-write set, immediate restore, shared-fd concurrency bound to a single write(), plain send path unchanged/blocking. |
| C-3 | Public contract unchanged | `link.h:30-41` (`FF_LinkSendUntil`/`send_until` return contract) | **Unchanged.** Same statuses and semantics; no doc edit required. |
| C-4 | Descriptor-mode restoration evidence | `docs/vv/evidence/2026-10-21-s8-fdir-uart-drain-stop-prefix.c:343-349` | **Consistent.** Asserts `(flags & O_NONBLOCK) == 0` after bounded send; holds for per-write restore. |
| C-5 | Test-spec mode-restoration assertion | `docs/vv/test-spec/ts-5-core-services.md:217` (N22-TS5-87) | **Consistent.** "console and UART descriptor modes are restored after their bounded writes." |
| C-6 | Pre-existing header comment still valid | `link_uart.c:116-127` | **Accurate.** Non-blocking used so a stalled UART cannot trap write(); original mode restored; blocking send path keeps semantics. No change required. |
| C-7 | NC-EXT-191 status | `docs/status/handover.md:66-67` | **Noted.** NC-EXT-191 remains OPEN (target readiness / GAP-A5 / target 250 ms); this increment does not close it. Not a documentation finding. |

---

## 4. Block/push decision

**No documentation finding blocks commit/push.**

- No documentation requires updating for this internal locking change.
- The added comment is in English (N2) and accurately describes the per-write `O_NONBLOCK` + immediate-restore behavior.
- Norm N8 is satisfied without a documentation change.
- NC-EXT-191 remains open (target readiness/GAP-A5/target 250 ms) and is not closed by this increment; this is a status/context item, not a documentation finding, and does not block this functional increment.

**Required before baseline/release (documentation scope):** None for this increment. Existing documentation-housekeeping items (DOC-R4-001..006 from the 2026-11-11 review) are out of scope for this functional increment and must be dispositioned separately before the documentation baseline/release is authorized by the appropriate authority.

---
*Prepared by the R4 Senior Aeronautics and Space Software Documentation Engineer persona (AI documentation assistant; advisory only, no approval authority). Technical content and the commit/push decision require review and approval by the qualified authorized human/FSW engineer.*
