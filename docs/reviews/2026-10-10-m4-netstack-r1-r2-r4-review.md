# Review Report - M4 increment 4 (reference loopback network stack) + milestone M8

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0032                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | base commit `05c1c5e` + uncommitted working tree (M4 increment 4, 2026-10-10) |
| Items under review                | `src/drivers/net/ff_net_loopback.{h,c}`, `tests/unit/test_netloop.c`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `docs/requirements/SRS.md` (SRS-REQ-039) + `docs/requirements/traceability.{yaml,md}`, `docs/development-plan.md` (M8/OBJ-16), `docs/roadmap.md` (M8), `CHANGELOG.md`, `README.md` (counts), `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness/robustness of the loopback (ring buffer with head/count, pairing, close semantics), bounded/no-alloc, defensive programming, conformance to the `FF_NetStack_t` contract (SRS-REQ-027), requirement quality and traceability (N3/N4), documentation completeness/correctness (N8/N17), milestone-M8 definition. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N17. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh context, independent session, report-only (no repository file was modified). |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the milestone-M4 increment-4 delta (reference loopback network stack,
SRS-REQ-039) against the base commit `05c1c5e` and the uncommitted working tree,
plus the roadmap/development-plan addition of milestone M8, under three
independent profiles:

- **R1 (design/code)**: the `FF_NetStack_t` reference implementation in
  `src/drivers/net/ff_net_loopback.{c,h}` - ring buffer (`head`/`count`),
  socket-table management, connect/accept pairing, close semantics, bounded/
  static/no-alloc behaviour, defensive checks, and conformance to the contract
  in `src/hal/hal_net.h` (SRS-REQ-027).
- **R2 (V&V)**: adequacy and independence of `tests/unit/test_netloop.c`, the
  honesty of the verification claims for SRS-REQ-039 (including the `lean_note`),
  the traceability manifest entry, and the verification record.
- **R4 (documentation, N17)**: completeness/correctness/consistency of the new
  and touched documentation - counts, revisions, histories, cross-references,
  the README metrics panel, the README driver inventory and the M8 definition
  in `docs/roadmap.md` / `docs/development-plan.md`.

Executed in this session (host, Linux; independent of the author):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (74 requirements: 40 SRS / 20 SSS / 14 GAP; 51 Lean theorems;
  9 CI claims).
- `cmake -S . -B build/nlr -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/nlr && ctest --test-dir build/nlr --output-on-failure` -> **11/11
  PASS**.
- `./build/nlr/tests/test_netloop` -> `checks=26 failures=0` (matches the
  documented count).
- `python3 tools/scripts/metrics.py` -> reproduces the README panel exactly
  (74/40/20/14; tests 60/74 = 81.1%; Lean 20/74 = 27.0%; coverage
  1643/2505 = 65.6%, functions 220/264), consistent with
  `build/coverage/coverage.json`.
- clang-format gate run inside the qualified V&V image
  (`docker run --rm -v "$PWD":/workspace frick-vv:local ... clang-format
  --dry-run --Werror ...`) -> **clean** for the three touched source files.
- `gcc -Wall -Wextra -Wpedantic` on `ff_net_loopback.c` -> **no warnings**.
- Reference-quality probes built in `/tmp` (not the repo): an
  AddressSanitizer+UBSan harness for the descriptor-validation path, a
  pairing/close lifecycle probe, and a ring-buffer full/wrap probe.
- Document-id uniqueness scan (69 ids, no duplicates) and relative-link scan
  (0 broken links) over `docs/**` + `README.md`/`AGENTS.md`.

Excluded / not executable in this environment: on-target execution and
real TSN/PTP accuracy (documented GAP-A5); `lake build` was not re-run here
(Lean theorem links are checked by the trace gate); the standard build is plain
Debug, so the ASan evidence comes from a throwaway probe (see R1-01).

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- HAL network contract: `src/hal/hal_net.h` (SRS-REQ-027)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Reviewer profiles: `docs/engineering/independent-review.md`
- Predecessor increments: `docs/reviews/2026-10-10-m4-gem-tsn-*.md`
  (in particular open residual D-04 and R4-02)
- ECSS-E-ST-40C, ECSS-Q-ST-80C

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | `src/drivers/net/ff_net_loopback.c` (`sock_of`); SRS-REQ-039 (bounded/defensive) | **The descriptor bounds check is defeated by a narrowing cast, yielding an out-of-bounds array access / memory corruption for a foreign descriptor whose low 16 bits are < 4.** `sock_of` tests `((uint16_t)sd >= FF_NETLOOP_MAX)` after only `sd < 0`. For `sd == 65536`, `(uint16_t)65536 == 0`, so the check passes and `g_sock[65536].used` is read - an out-of-bounds read past the 4-entry `g_sock[]` table (and, if the garbage byte is non-zero, a write to `g_sock[65536].rx[]`). The component ships inside `frick_fsw_core` (added to `FF_CORE_SOURCES`), not only in the test binary; the guard exists precisely to reject handles not produced by `open()` and does not do so. | `ff_net_loopback.c:46-52` (check at :48); ASan+UBSan probe (`/tmp/opencode/nl_probe.oob`): `runtime error: index 65536 out of bounds for type 'FF_NetLoopSock_t [4]'` then `AddressSanitizer: SEGV ... in sock_of src/drivers/net/ff_net_loopback.c:48`. | Test the `int` handle directly, e.g. `if (sd < 0 \|\| sd >= (FF_NetSock_t)FF_NETLOOP_MAX \|\| g_sock[sd].used == 0u) return NULL;` (no `uint16_t` cast), and add a unit case passing an out-of-range descriptor (see R2-01). |
| R1-02 | Minor | `src/drivers/net/ff_net_loopback.c` (`netloop_connect`, `netloop_close`, `netloop_listen`); SRS-REQ-039 (pairing/close) | **The socket table can leak and the accept pairing is not protected.** (a) `netloop_connect` unconditionally overwrites the listener's `pending` with the new relay socket; `backlog` is ignored (`(void)backlog`), so a second connect displaces a not-yet-accepted relay which is then unreachable and never freed. (b) Closing a client before accept clears the relay's `peer` but leaves the relay `used` and still referenced by the listener's `pending`; closing a listener that has a pending relay likewise orphans it. In a 4-slot table this exhausts the pool. (c) `netloop_close` does not clear `pending`/`count`/`head` (harmless because `open` resets head/count/pending, but not `port` - see R1-03). | `ff_net_loopback.c:112-120` (pending overwrite), `:130-138` (accept), `:185-198` (close); lifecycle probe output: `connect c1=1 -> 0`, `close c1`, `connect c2=1 -> 0` (pending overwritten), `accept -> 3` (first relay orphaned), `opened 0 more before exhaustion (of 4)`. | Reject a connect while `pending != NONE` (or queue up to `backlog`); free the relay when its client closes; cancel/close pending relays in `netloop_close` of a listener; reclaim slots deterministically. |
| R1-03 | Minor | `src/drivers/net/ff_net_loopback.c` (`netloop_open`, `netloop_bind`, `netloop_listen`) | **`port` is not reset on `open`, duplicate binds are accepted, and `listen` without `bind` can expose a stale port.** `netloop_open` resets `used/listening/peer/pending/head/count` but not `port`; a socket previously bound to port P, closed and reopened, then made to `listen` without an intervening `bind` becomes discoverable by `connect` on the stale P. `netloop_bind` never checks whether the port is already bound, so two listeners can share a port (the first is always selected). | `ff_net_loopback.c:60-69` (no `port` reset), `:74-82` (no duplicate check), `:84-93` (`listen` does not require `bind`); `netloop_init` (`:37`) does reset `port`, which masks the reopen path. | Reset `port = 0` in `netloop_open`, reject (or document) duplicate binds, and either require a successful `bind` before `listen` or explicitly document the "listen on 0" semantics. |
| R1-04 | Comment | `src/drivers/net/ff_net_loopback.c` (`netloop_send`/`netloop_recv`); `src/hal/hal_net.h` | The non-blocking semantics of partial/empty transfers are undocumented and only implicit. `send` returns `FF_SUCCESS` with `*sent < len` when the peer ring is full (no `FF_NOT_READY`), and `recv` returns `FF_SUCCESS` with `*got == 0` when empty; `timeout_ms` is ignored (`(void)timeout_ms`). A socket-like contract normally distinguishes "would block" from success. | `ff_net_loopback.c:141-162` (`*sent = i`, `return FF_SUCCESS`), `:164-183` (`(void)timeout_ms`, empty -> success); ring probe: `send 300 -> sent=256`. | Document explicitly in `hal_net.h`/the header that the loopback is non-blocking with partial `send`/empty `recv` returned as `FF_SUCCESS`, or return a dedicated status on a full/empty buffer. |
| R1-05 | Comment | `src/drivers/net/ff_net_loopback.{h,c}` | No concurrency statement. The implementation uses a single static table with no locking; it is only safe single-threaded, but neither the header nor the manifest `lean_note` says so. | `ff_net_loopback.c:29` (`static g_sock[]`); no mutex/atomic. | State "single-threaded / not reentrant" in the header (a reference stack is still copied by integrators). |
| R1-06 | Comment | `src/drivers/net/ff_net_loopback.c:11` | `#include "fsw_config.h"` is not used by the file (no macro from it is referenced); harmless but noise. | Manual inspection; clean under `-Wall -Wextra -Wpedantic`. | Drop the include (or document why it is kept, as other drivers do). |
| R2-01 | Minor | `tests/unit/test_netloop.c`; SRS-REQ-039 (`method: Tst`) | **Coverage is happy-path plus three error cases only.** Untested branches/methods: `netloop_open` exhaustion (returns `-1`), `accept` with `pending == NONE`, `bind`/`connect`/`send`/`recv` with `NULL` arguments / `addr_len == 0`, a full-ring partial `send`, the ring wrap across successive send/recv, slot reclaim after close, and - critically - an out-of-range descriptor, so the suite **would not detect R1-01**. The `accept`/`connect`-failure strength and the defensive branches therefore carry no test evidence. | `test_netloop.c:14-73` (two functions); driver branches `ff_net_loopback.c:46-52,60-72,109-115,130-132,146-151,170-172`; run shows `checks=26`. | Add negative/boundary cases (null args, empty accept, table exhaustion, ring-full/partial send and multi-cycle wrap, invalid descriptor); expect the check count to grow and re-record the evidence after R1-01. |
| R2-02 | Minor | SRS-REQ-039 wording / `CHANGELOG.md`; V&V reproducibility | **The "host simulator" part of the claim is not substantiated.** SRS-REQ-039 says the reference stack is "for unit tests **and the host simulator**", and `CHANGELOG.md` says it exercises the contract "in unit tests and on the host", but `FF_NetStackLoopback` is referenced only by `test_netloop.c`; nothing in `src/main/` or `src/apps/` instantiates it, so the host simulator (`make run`) does not use it. The deliverable is *available* on the host, but the claim of active simulator use is unverified. | `grep -rn FF_NetStackLoopback src/ config/ tests/` -> only the header definition and `test_netloop.c`; no hit under `src/main/` or `src/apps/`. | Either wire the loopback into the simulator/a reference app (and record it) or reword SRS-REQ-039 / CHANGELOG to "available for unit tests and host builds" to keep the claim honest. |
| R2-03 | Comment | `docs/vv/verification-results.md` (N11/N16) | The M4 cycle row was edited **again** to fold increment 4 into a single row ("M4 drivers + RTEMS + TSN + netstack", 371 checks) instead of appending a distinct increment-4 row; the per-increment History entries (0.7..0.10) remain. This is the already-open residual D-04 from the increment-2 review. The counts themselves are correct and reproduce (15+20+9+29+101+31+20+81+23+16+26 = 371). | `git diff docs/vv/verification-results.md:27`; `docs/reviews/2026-10-10-m4-rtems-delta-review.md` (D-04, open); trace/ctest above. | Add a distinct increment-4 row (or record the explicit rationale for one row per milestone) and close D-04. |
| R2-04 | Comment | SRS-REQ-039 / manifest (positive) | Verification claims are **honest about scope**: `method: Tst`, `lean: []` with an explicit `lean_note` stating the reference stack is verified by unit tests and that third-party stacks plug in through the same vtable; no silent waiver. The `trace` gate reproduces 74/40/20/14/51 and the test count (26) reproduces. | `traceability.yaml:371-378`; `SRS.md:192-196`; `trace.py` output. | (No action; keep the note accurate after R2-01/R2-02.) |
| R4-01 | Minor | `docs/vv/verification-results.md:261` (N8/N17) | **Stale coverage figure, internally inconsistent with the same document and the README.** The "Coverage" subsection still states `` `make coverage`: lines **1533/2357 (65.0%)**, functions **207/250** `` (the increment-3 predecessor value), while the same document's cycle row (:27), the README panel (:76) and `build/coverage/coverage.json` all state **1643/2505 (65.6%)**, functions **220/264**. | `verification-results.md:261` vs `:27` and `README.md:76`; `python3 tools/scripts/metrics.py`; `build/coverage/coverage.json`. | Regenerate/update the coverage line to 1643/2505 (65.6%), functions 220/264. |
| R4-02 | Minor | `docs/status/status.json` (N8/N17) | **M8 is not reflected in the status record.** `milestones` lists only `M1..M7`; the README metrics panel and the N17 review scope explicitly include `docs/status/status.json`. Adding milestone M8 to `roadmap.md`/`development-plan.md` without adding it to the status record leaves the status set incomplete/stale. | `docs/status/status.json` (`milestones`: M1..M7 only); `roadmap.md:100-113`; `development-plan.md:164-169,206`. | Add `"M8": "planned"` to the `milestones` map. |
| R4-03 | Minor | `README.md:122-129`, `docs/engineering/driver-development.md:22-33` (N8) | **The affected "driver inventory" documentation was not updated for the new reference implementation.** The README "Low-level drivers" table and the driver-development "extension points" table (still PAL / Link / Board only) do not list `src/drivers/net/ff_net_loopback.c` / `FF_NetStackLoopback`. The latter is the **recurrence of open R4-02 from the increment-3 review** (HAL/reference backends missing from the table), now with a third reference component delivered. | `README.md:122-129`; `driver-development.md:22-33`; `docs/reviews/2026-10-10-m4-gem-tsn-r1-r2-r4-review.md` (R4-02, open). | Add the network-stack row (and a HAL/reference-driver row) to the tables and close the recurring R4-02. |
| R4-04 | Comment | `docs/vv/verification-results.md:263-267` (N8) | The "Remaining (M4)" list still names "lwIP `FF_NetStack_t` adapter + GEM TSN (`FF_TsnBackend_t`)" although the GEM native TSN backend (SRS-REQ-038) was delivered in increment 3 and the reference network stack in increment 4; only the lwIP/A third-party adapter remains. | `verification-results.md:263-267`. | Update the remaining list (and point the deferred full PTP/CoreTSN work at M8). |
| R4-05 | Comment | `docs/requirements/SRS.md`, `docs/requirements/traceability.{md,yaml}`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `tests/CMakeLists.txt` (N16) | Re-touched files still have no trailing newline (`\ No newline at end of file` in the delta diff) - pre-existing D-07 / R4-03. Cosmetic. | `git diff` (`\ No newline at end of file` markers). | Add trailing newlines. |
| R4-06 | Comment | `docs/reviews/README.md` (N11/N17) | This report's row has not been appended to the review log. This review was explicitly scoped to write the report only, so the log row (and the commit of this report) remains an author action at cycle close. | `docs/reviews/README.md:3-35` (no increment-4 row). | Append the log row and commit this report with the reviewed change (norm N11). |
| R4-07 | Comment | `docs/requirements/SRS.md:192-196`; `docs/development-plan.md:63-67,142-146,164-169` (N3/N8) | Two clarity items. (a) The SRS-REQ-039 phrase "exercises the pluggable **TCP/IP** interface" is loose: the contract is the socket-like, stack-independent `FF_NetStack_t` (SRS-REQ-027); the loopback is a byte-stream endpoint, not TCP/IP. (b) The development-plan resource catalogue labels the 802.1AS/1588/802.1CB reference backends as "M4" (:66), while M8 now owns their completion, and M8's "fabric CoreTSN" overlaps M4's "generic fabric-device (FIC/AXI) ... TSN" path (:142-146). | `SRS.md:192-196`; `development-plan.md:63-67,142-146,164-169`; `roadmap.md:100-113`. | (a) Rephrase to "the stack-independent network-stack interface". (b) State explicitly which parts of the fabric TSN path are M4 vs M8 so the two milestones do not double-claim. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Positive verification evidence (reproduced in this session)

- `trace.py`: `traceability: PASS (74 requirements, 40 srs, 20 sss, 14 gaps,
  51 lean theorems checked)`; the new SRS-REQ-039 -> SSS-REQ-020 parent link,
  the implementation paths (`ff_net_loopback.{c,h}`) and the test path
  (`test_netloop.c`) all resolve. Counts match the task baseline
  (74/40/20/14, 51 theorems).
- Host build + `ctest`: **11/11** suites PASS; `test_netloop` reports
  `checks=26 failures=0`, matching `CHANGELOG.md`, `tool-qualification.md` and
  `verification-results.md` (total 371 checks: the arithmetic
  15+20+9+29+101+31+20+81+23+16+26 = 371 is correct).
- `metrics.py` reproduces the README panel exactly (74/40/20/14; tests
  60/74 = 81.1%; Lean 20/74 = 27.0%; coverage 1643/2505 = 65.6%, functions
  220/264), consistent with `build/coverage/coverage.json`.
- `gcc -Wall -Wextra -Wpedantic` on the new translation unit: **no warnings**;
  clang-format `--dry-run --Werror` (qualified `frick-vv:local` image) is
  **clean** for `ff_net_loopback.{c,h}` and `test_netloop.c`.
- **Ring-buffer logic is correct in the normal range**: with `head`/`count`
  and a 256-byte ring, a probe sending 300 bytes delivered `sent=256` and
  `recv` returned the exact byte sequence `0..255` (modular index and
  `head+count < 512` are safe; `count` is bounded by the ring-size check).
- **No dynamic memory**: the implementation uses only a `static` 4-entry table
  of `FF_NetLoopSock_t` (≈1 KiB); no `malloc`/`free`. Bounded/static/
  in-process requirement is met.
- The HAL contract shape is conformant: every `FF_NetStack_t` member is
  implemented with the exact signature in `src/hal/hal_net.h`, and
  `open`/`accept` return `-1` on failure as the contract requires.
- Documentation set mostly updated and consistent: SRS rev 1.0 (+ history),
  traceability rev 0.9 (+ history), roadmap rev 0.8 (+ M8 + history),
  development-plan rev 0.6 (+ M8/OBJ-16 + history), tool-qualification
  rev 0.2.0 (+ T3 re-qualification row), CHANGELOG entry, verification-results
  rev 0.10 (+ history). Document-id scan: 69 ids, **no duplicates**;
  relative-link scan: **0 broken links**.
- The M8 definition is coherent and traceable: it is stated consistently in
  `roadmap.md` and `development-plan.md`, mapped to OBJ-16 (which the objective
  table and the objective-to-milestone status table both carry), scoped
  (gPTP BMCA/Sync/PDelay/servo + fabric CoreTSN/802.1CB FRER via the FIC
  contract), given a verification approach (Lean where feasible, tests, docs)
  and an explicit residual gap (GAP-A5). Its addition also resolves the
  pre-existing forward reference `OBJ-15 -> M5/M8`. Only the clarity items in
  R4-07 apply.

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **REJECTED** | One Major: R1-01 (the `sock_of` descriptor guard is defeated by a `(uint16_t)` narrowing cast, producing an out-of-bounds read/memory corruption for a foreign handle). R1-02/R1-03 are Minor, R1-04..R1-06 Comments. The ring-buffer, bounds and no-alloc behaviour are otherwise correct. |
| R2 (V&V) | **APPROVED WITH COMMENTS** | The requirement's `Tst` evidence is valid for the core behaviour and the counts/traceability reproduce; R2-01 (coverage only happy-path + 3 errors; the suite cannot detect R1-01) and R2-02 (the "host simulator" claim is not exercised) are Minor; R2-03 Comment (recurrence of open D-04); R2-04 records honest claims. |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | Minor findings R4-01 (stale coverage 1533/2357 in the same V&V record), R4-02 (M8 missing from `status.json`), R4-03 (README/driver guide inventory not updated; recurrence of open R4-02) and Comments R4-04..R4-07. Requirement counts, revisions, histories and link/id integrity are otherwise correct. |

Overall: **REJECTED**. Per norm N12 the change must not be committed/pushed
until R1-01 is resolved (correct the descriptor bounds check and add an
out-of-range case to the suite) and the delta is re-reviewed. The Minor/
Comment findings are to be dispositioned and tracked to closure; R1-02/R1-03
and R2-01 should be fixed in the same delta because they exercise the same
table-management code.

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | REJECTED (R1-01 Major) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_net_loopback.c` (`sock_of`), SRS-REQ-039 | `(uint16_t)` cast defeats the descriptor bounds check -> OOB read/corruption (ASan SEGV) | Compare the `int` handle directly; add an out-of-range test | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R1-02 | Minor | `ff_net_loopback.c` (connect/close/listen) | Relay-socket leak + `pending` overwrite (backlog ignored); orphans on early close | Reject/queue on busy pending; free relay on peer close; cancel pending | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R1-03 | Minor | `ff_net_loopback.c` (open/bind/listen) | `port` not reset on `open`; duplicate bind accepted; stale port after reopen | Reset `port`; reject duplicate bind; require bind before listen | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R2-01 | Minor | `test_netloop.c`, SRS-REQ-039 | Only happy path + 3 errors; defensive/boundary branches untested; cannot detect R1-01 | Add negative/boundary cases; re-record | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R2-02 | Minor | SRS-REQ-039 / `CHANGELOG.md` | "host simulator" claim not exercised (only `test_netloop.c` uses it) | Wire the simulator/app or reword the claim | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-01 | Minor | `verification-results.md:261` | Stale coverage 1533/2357 (65.0%), 207/250 vs 1643/2505 (65.6%), 220/264 | Update the coverage line | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-02 | Minor | `docs/status/status.json` | M8 absent from `milestones` (M1..M7 only) | Add `"M8": "planned"` | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-03 | Minor | `README.md`, `driver-development.md` | README driver inventory / driver guide omit the new reference stack (recurrence of open R4-02) | Add the network-stack + HAL rows; close R4-02 | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R1-04 | Comment | `ff_net_loopback.c`, `hal_net.h` | Partial `send`/empty `recv` returned as `FF_SUCCESS`; `timeout_ms` ignored, undocumented | Document semantics or return a dedicated status | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R1-05 | Comment | `ff_net_loopback.{h,c}` | No single-threaded/not-reentrant statement | Document | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R1-06 | Comment | `ff_net_loopback.c:11` | Unused `#include "fsw_config.h"` | Remove or justify | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R2-03 | Comment | `verification-results.md` | Combined M4 cycle row edited again (recurrence of open D-04) | Add increment row or record rationale | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-04 | Comment | `verification-results.md:263-267` | "Remaining (M4)" still lists delivered GEM TSN; point deferred PTP/CoreTSN at M8 | Update the list | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-05 | Comment | several docs / `tests/CMakeLists.txt` | Missing trailing newline (pre-existing D-07) | Add newline | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-06 | Comment | `docs/reviews/README.md` | Review-log row for this report not yet appended | Append at commit and push (N11) | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |
| R4-07 | Comment | `SRS.md:192-196`, `development-plan.md` | Loose "TCP/IP interface" wording; M4 vs M8 fabric-TSN overlap in the catalogue | Rephrase; state the M4/M8 split | (pending) | frick-fsw | **CLOSED** | FRICK-REVIEW-0057/0058/0060/0061 |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Initial milestone-M4 increment-4 review (reference loopback network stack, SRS-REQ-039) and milestone-M8 addition |

---

## 8. Author disposition (closure pending independent delta re-review)

Author-proposed dispositions at HEAD (post M11 NET), submitted for **independent**
delta re-verification (norm N12). These do **not** close the NCs: closure is recorded
only after the independent delta re-review (`FRICK-REVIEW-0054`/`0055`) populates the
Disposition / Re-verification / Delta columns.

- **R1-01 (Major) - FIXED (code):** the `sock_of` narrowing cast is corrected
  (indices are range-checked `FF_NetSock_t`), exercised by `test_netloop` (out-of-range
  case).
- **R1-03 - FIXED (code):** duplicate `bind` is rejected (`netloop_bind` returns
  `FF_ALREADY_EXISTS`).
- **R1-02 (Minor) - DOCUMENTED LIMITATION, tracked:** the reference loopback pairs at
  `connect` (eager relay slot) and keeps descriptors independent (closing one end does
  not invalidate the other's `close`, per `test_netloop.c:51`). Known degradations of
  the reference backend: `backlog` is ignored (`(void)backlog`) and a second `connect`
  before `accept` overwrites the listener's `pending` (the prior relay slot stays
  `used` and is unreachable via `accept`); an un-accepted connection retains one of
  four bounded slots until `init()` resets the table. Recorded as a bounded limitation
  (lwIP is the production path) and tracked in `docs/status/loop-audit.md` and
  `docs/development-plan.md` with owner/milestone (norm N19). A `close`-releases-peer
  change was tried and reverted because it broke the documented independent-descriptor
  semantics.
- **R2-02 - ADDRESSED:** reference semantics pinned by the NET app contract check
  (non-zero port, negative unconnected-port, accept peer) and the host tests.

Residual (R1-02) is tracked, not waived (norm N12/N19).

### Closure (norm N12)

Independently re-reviewed and closed: R1 subset FRICK-REVIEW-0057/0060; R2 subset
FRICK-REVIEW-0058/0061; documentation subset FRICK-REVIEW-0059/0062. R1-01..R1-06,
R2-01..R2-03 and R4-01..R4-07 are **CLOSED**. The R1-02 relay/`pending` orphan
remains a **bounded, documented limitation of the reference backend** (lwIP is the
production path), tracked in `docs/status/loop-audit.md` and
`docs/development-plan.md` with owner/milestone (norm N19) — not waived. **Status:
CLOSED (residual tracked).**
