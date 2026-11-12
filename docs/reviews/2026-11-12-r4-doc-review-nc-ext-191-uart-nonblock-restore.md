# R4 Independent Documentation Review — frick-fsw (norm N17)

**Reviewer profile:** Senior Aeronautics and Space Software Documentation Engineer (R4). No engineering/safety/certification/release authority; findings are advisory. Only the `aeronautics-space-fsw-engineer` agent (or an explicitly delegated human) may commit/push, and only after all applicable gates pass.

**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD 3d62d47
**Review date:** 2026-11-12
**Scope:** Uncommitted bounded functional increment in `src/transport/link/link_uart.c` (`uart_send_until()`): capture of the restore `fcntl` result so a failed restore sets `result = FF_ERROR` (the fd is not silently left non-blocking).

**Important tree-state clarification (verified against HEAD vs working tree):** HEAD 3d62d47 (`NC-EXT-191: resolve UART shared-descriptor concurrency in uart_send_until()`) already contains the explanatory comment (`link_uart.c:150-153`), the per-write `O_NONBLOCK` set, and the immediate blocking-mode restore. The uncommitted increment adds **only** the restore-result capture:
- `(void)fcntl(g_fd, F_SETFL, flags);` → `int restore_rc = fcntl(g_fd, F_SETFL, flags);`
- `if (restore_rc != 0) { result = FF_ERROR; }` after progress accounting.

There is **no API, interface, public-contract, traceability-requirement, or new-symbol change.** Not hardware/target related (POSIX fd, host-executable). NOT GAP-A5/NC-PAL-01/OBCP. NC-EXT-191 remains open (target readiness / GAP-A5 / target 250 ms) and is NOT closed by this increment.

**Gate status observed (per context):** `make build` exit 0 (0 warnings); CTest exit 0 (incl. `test_link_uart.c`); Lean `lake build` PASS; `make trace` PASS (120/76/30/14/68); format-check PASS; license OK. Norm N2: all comments English. Norm N16: no revision-history documentation files changed (only `link_uart.c` source).

---

## 1. Determinations against the review tasks

### Task 1 — Does any documentation need updating for this change?

**Determination: No.**

Rationale:
- `uart_send_until()` is static/internal; the public entry point `FF_LinkSendUntil()` (via `FF_Link_t::send_until`) returns the same statuses (`FF_SUCCESS`, `FF_TIMEOUT`, `FF_NOT_READY`, `FF_INVALID_ARGUMENT`, `FF_ERROR`) with unchanged semantics (`link.h:27-43`).
- The restore-result capture is internal error handling; it does not alter the return contract, the descriptor-mode restoration behavior, or any externally visible behavior.
- No traceability requirement change; no new public symbol; no user/operator/maintenance-visible behavior change.
- Therefore no SRS/SSS, ICD (FRICK-ICD-001), TS-x test spec, design/SDD, user/operator/maintenance doc, release note, or traceability matrix requires a change for this increment. Norm N8 (complete, current, deployable documentation) is satisfied without a documentation change.

### Task 2 — Norm N2 (English) in the added comment

**Determination: PASS.**

The comment at `src/transport/link/link_uart.c:150-153` is written in English. No non-English content.

### Task 3 — Norm N8

**Determination: PASS — no documentation change needed.**

Confirmed in Task 1. The documentation set remains complete, current, and deployable for this internal locking/error-handling change.

### Task 4 — Does the comment accurately describe the change?

**Determination: Accurate as far as it goes; incomplete regarding the restore-result capture.**

Added comment (`link_uart.c:150-153`):

> /* Set O_NONBLOCK for this write only, then restore blocking mode
>  * immediately, so a concurrent writer on the shared fd (uart_send)
>  * never observes non-blocking mode for longer than a single write()
>  * call. The plain send path keeps its blocking semantics. */

Verification of each claim against the code (`link_uart.c:154-164`):

| Comment claim | Code evidence | Verdict |
|---|---|---|
| "Set O_NONBLOCK for this write only" | `fcntl(g_fd, F_SETFL, flags | O_NONBLOCK)` inside `while (sent < len)`, immediately before each `write()` (:154) | ✓ Accurate |
| "restore blocking mode immediately" | `fcntl(g_fd, F_SETFL, flags)` executes immediately after `write()`, before any branch/continue/nanosleep (:159) | ✓ Accurate |
| "concurrent writer ... never observes non-blocking mode for longer than a single write() call" | Descriptor is non-blocking only across `uart_send_until`'s own `write()`; `uart_send()` (:107-114) performs a plain blocking write and never touches the flags | ✓ Accurate |
| "The plain send path keeps its blocking semantics" | `uart_send()` unchanged, blocking `write()` without `O_NONBLOCK` (:107-114) | ✓ Accurate |

**Gap:** The comment does **not** describe the restore-result capture introduced in this increment (`int restore_rc = ...` at :159; `if (restore_rc != 0) { result = FF_ERROR; }` at :162-164), i.e. that a failed restore surfaces as `FF_ERROR` so the fd is not silently left non-blocking. The comment is accurate for the concurrency mechanism it explains; it is simply silent on the error-handling addition.

Pre-existing header comment (`link_uart.c:116-127`) remains accurate and needs no change.

### Task 5 — Documentation findings that block commit/push

**Determination: None.**

0 Major findings. 0 Minor findings that block commit/push. One Minor observation/recommendation (DOC-R4-001) is raised; it is non-blocking.

---

## 2. Findings register

| Finding ID | Classification | File:line | Summary | Blocks commit/push? |
|---|---|---|---|---|
| DOC-R4-001 | Minor (observation/recommendation) | `src/transport/link/link_uart.c:150-153,159-164` | The comment accurately describes per-write `O_NONBLOCK` + immediate restore + shared-fd concurrency bound to a single `write()`, but does not describe the restore-result capture (`result = FF_ERROR` when the restore `fcntl` fails) introduced in this increment. Recommendation: extend the comment to note that a restore failure surfaces as `FF_ERROR` so the descriptor is not silently left non-blocking. | No |

**Major findings:** 0.
**Minor findings:** 0 blocking; 1 non-blocking observation.

Note: Existing DOC-R4-001..006 from the 2026-11-11 R4 review (NC-EXT-237/NC-EXT-191 backend-coverage cycle) are unrelated to this increment and remain as recorded; none concerns `link_uart.c` or the `O_NONBLOCK` restore behavior, and none blocks this functional increment under N21 (non-functional findings do not stop unrelated development).

---

## 3. Confirmations (no findings)

| # | Item | Checked at | Result |
|---|------|------------|--------|
| C-1 | Norm N2 (English) — comment | `src/transport/link/link_uart.c:150-153` | **Pass.** Comment is English. |
| C-2 | Comment accuracy (claims it makes) | `link_uart.c:150-164` | **Accurate.** Per-write set, immediate restore, shared-fd concurrency bound to a single `write()`, plain send path unchanged/blocking. See gap in DOC-R4-001 re: restore-result capture. |
| C-3 | Public contract unchanged | `link.h:27-43` (`send_until`/`FF_LinkSendUntil` return contract) | **Unchanged.** Same statuses and semantics; no doc edit required. |
| C-4 | Descriptor-mode restoration evidence | `docs/vv/evidence/2026-10-21-s8-fdir-uart-drain-stop-prefix.c:343-349` | **Consistent.** Asserts `(flags & O_NONBLOCK) == 0` after bounded send; holds for the per-write restore. |
| C-5 | Test-spec mode-restoration assertions | `docs/vv/test-spec/ts-5-core-services.md:215` (N22-TS5-85), `:217` (N22-TS5-87) | **Consistent.** Both assert descriptor/UART mode is restored after the bounded send; behavior unchanged by this increment. |
| C-6 | Pre-existing header comment still valid | `link_uart.c:116-127` | **Accurate.** Non-blocking used so a stalled UART cannot trap `write()`; original mode restored; blocking send path keeps semantics. No change required. |
| C-7 | NC-EXT-191 status | `docs/status/handover.md:66-67` | **Noted.** NC-EXT-191 remains OPEN (target readiness / GAP-A5 / target 250 ms); this increment does not close it. Status/context item, not a documentation finding, and does not block this functional increment. |

---

## 4. Norm N8 assessment (complete, current, deployable documentation)

N8 is satisfied for this increment without a documentation change. The changed file is source (not a controlled document/data item); no SRS/SSS/ICD/TS/spec/release-note/traceability content is affected. The only documentation-touching element is the in-code comment, which is in English (N2) and accurate for the mechanism it describes (see DOC-R4-001 for the non-blocking completeness observation).

Residual documentation-housekeeping items (DOC-R4-001..006 from the 2026-11-11 review) are out of scope for this functional increment and must be dispositioned separately before the documentation baseline/release is authorized by the appropriate authority.

---

## 5. Block/push decision

**No documentation finding blocks commit/push.**

- No documentation requires updating for this internal locking/error-handling change.
- The comment is in English (N2) and accurately describes the per-write `O_NONBLOCK` + immediate-restore behavior.
- Norm N8 is satisfied without a documentation change.
- DOC-R4-001 is a non-blocking observation/recommendation.
- NC-EXT-191 remains open (target readiness/GAP-A5/target 250 ms) and is not closed by this increment; this is context, not a documentation finding, and does not block this functional increment.

**Required before baseline/release (documentation scope):** None for this increment.

---

*Prepared by the R4 Senior Aeronautics and Space Software Documentation Engineer persona (AI documentation assistant; advisory only, no approval authority). Technical content and the commit/push decision require review and approval by the qualified authorized human/FSW engineer.*
