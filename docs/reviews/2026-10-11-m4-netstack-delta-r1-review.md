# Review Report - M4 netstack (FRICK-REVIEW-0032) independent delta re-review, profile R1

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0054                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev / design + code)            |
| Reference revision                | commit `e1def89` ("PA review dispositions round 2 ... 0032 pending independent delta"), branch `main`, clean tree |
| Items under review                | Delta re-review of the R1 non-conformances of `FRICK-REVIEW-0032`: `src/drivers/net/ff_net_loopback.{c,h}`, `src/hal/hal_net.h`, `tests/unit/test_netloop.c`, `docs/status/loop-audit.md`, `docs/development-plan.md`, `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md` (§7 table, §8 author disposition), SRS-REQ-039 (`docs/requirements/SRS.md`, `docs/requirements/traceability.yaml`) |
| Date                              | 2026-10-11                             |
| Scope                             | Independent delta re-review (norm N12: re-apply all rules to the affected scope and re-review the delta) of NCs **R1-01, R1-02, R1-03, R1-04, R1-05, R1-06** of `FRICK-REVIEW-0032` against the code, tests and tracking records at `e1def89`. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N20. |
| Independence declaration          | The reviewer took no part in authoring, implementing, disposing or executing any reviewed item; it was spawned in a fresh, independent context and inherited no author context. It executed only read-only verification (git reads, unit tests, `trace.py`, an independently written ASan/UBSan probe in `/tmp/opencode`) and wrote **only this report file**; no other tracked file was modified. `FRICK-REVIEW-0032` was read as an object of review, not endorsed. |
| Norms applied                     | N1-N20 (see `docs/rules.md`)           |

## 1. Scope

This is the **R1-profile delta re-review** that `FRICK-REVIEW-0032` §8 declares
pending ("closure is recorded only after the independent delta re-review
(`FRICK-REVIEW-0054`/`0055`)"). It adjudicates the six R1 non-conformances only.

In scope:

- **R1-01 (Major)** - `sock_of` narrowing cast defeating the descriptor bounds
  check (`ff_net_loopback.c`, `hal_net.h`, `test_netloop.c`).
- **R1-03 (Minor)** - duplicate `bind`, `port` not reset on `open`,
  `listen` without `bind`.
- **R1-02 (Minor)** - relay-slot leak / `pending` overwrite / `backlog`
  ignored, dispositioned by the author as a **bounded limitation, tracked**.
  Adjudicated for (a) technical acceptability for a reference backend and
  (b) N19 tracking with owner/milestone.
- **R1-04 / R1-05 / R1-06 (Comments)** - `send`/`recv` semantics documented,
  non-reentrancy documented, unused include removed.

Explicitly **out of this profile** (R2/R4 NCs, to be adjudicated by their own
delta re-reviews): R2-01, R2-02, R2-03 (V&V) and R4-01..R4-07 (documentation).
Observations made on those while reading the same files are labelled as such and
are **not** R1 adjudications.

Commands executed in this session (host, Linux; independent of the author):

- `./build/native/tests/test_netloop` -> `checks=29 failures=0`.
- `ctest --test-dir build/native` -> `100% tests passed, 0 tests failed out of 14`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  `PASS (94 requirements, 54 srs, 26 sss, 14 gaps, 51 lean theorems)`.
- `gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -c
  src/drivers/net/ff_net_loopback.c` -> **exit 0, no warnings**.
- An independently written AddressSanitizer+UBSan probe
  (`/tmp/opencode/delta-r1/probe.c`, **not** in the repository) driving **all
  eight** `FF_NetStack_t` entry points with hostile handles
  `{65536, 65540, INT_MAX, INT_MIN, -1, 4, 3}` -> every call returned
  `FF_INVALID_ARGUMENT`/`-1` and **no sanitizer diagnostic** was produced.
- A second probe (`/tmp/opencode/delta-r1/probe3.c`) reproducing the R1-02
  lifecycle (backlog, `pending` overwrite, orphan, slot accounting).

Excluded / not executable here: on-target execution and real TSN/PTP accuracy
(GAP-A5); `clang-format`/`clang-tidy` were not re-run (the pinned
`frick-vv:local` image was not used in this session); Lean was not rebuilt
(its links are checked by the trace gate, which passed).

## 2. References

- Project norms: `docs/rules.md` (N1-N20); N12 (NC close-out), N19 (loop audit).
- Predecessor review under re-verification: `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md`
  (FRICK-REVIEW-0032), §7 follow-up table and §8 author disposition.
- Review template: `docs/reviews/review-template.md`.
- Contract: `src/hal/hal_net.h` (SRS-REQ-027); requirement SRS-REQ-039
  (`docs/requirements/SRS.md:192-196`, `docs/requirements/traceability.yaml:371-377`).
- Tracking records: `docs/status/loop-audit.md`, `docs/development-plan.md`,
  `docs/quality/class-a-gaps.md`.
- Reviewer profiles: `docs/engineering/independent-review.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings (delta findings on the disposition)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-R1-01 | Minor | `docs/development-plan.md`, `docs/quality/class-a-gaps.md`; **N19 / N8 / N16** | **The R1-02 tracking claimed by the author disposition is only half-substantiated, and the commit message repeats the unsubstantiated half.** The §8 disposition states the residual is "tracked in `docs/status/loop-audit.md` **and** `docs/development-plan.md` with owner/milestone (norm N19)", and the `e1def89` commit message says "tracked in loop-audit + development plan". At HEAD the residual exists **only** in `loop-audit.md:139`; `development-plan.md` and `class-a-gaps.md` contain **no** R1-02 / relay-orphan entry (N19 names `class-a-gaps.md` / `development-plan.md` as the tracking location for a non-class-A gap). Separately, `loop-audit.md` was modified in `e1def89` to Rev 0.2 but has **no 0.2 History row** (N16), and its `Baseline commit` remains `e8daab3`. | `docs/status/loop-audit.md:7,139,142-146`; `git show e1def89 -- docs/status/loop-audit.md` (adds `:139`, bumps Rev to 0.2, no History row); `git show e1def89 -- docs/development-plan.md` (no residual entry added); `grep -rn "R1-02\|relay\|orphan" docs/development-plan.md docs/quality/class-a-gaps.md` -> no match; `2026-10-10-m4-netstack-r1-r2-r4-review.md:224-234` (claim). | Add the one-line residual to `docs/development-plan.md` (M4/M11) **or** to `class-a-gaps.md`, keep `loop-audit.md:139` (owner/milestone already present), add the 0.2 History row, and correct the §8 wording so it states the true tracking location. |
| D-R1-02 | Comment | `src/hal/hal_net.h:30-31`; SRS-REQ-027 | **Contract/implementation wording tension on blocking semantics.** The `FF_NetStack_t` contract comment still describes the API as "Socket-like API (blocking, bounded)", while the reference backend at `ff_net_loopback.h:10-12` now correctly documents that `send` may be partial and `recv` is non-blocking (and `ff_net_loopback.c:178` ignores `timeout_ms`). Cross-referenced with FRICK-REVIEW-0044 R1-01 (the lwIP backend's `timeout_ms == 0` semantics diverge from the loopback). An integrator reading only the contract may assume blocking `recv`. | `src/hal/hal_net.h:30-31`; `src/drivers/net/ff_net_loopback.h:10-13`; `src/drivers/net/ff_net_loopback.c:178`; `docs/reviews/2026-10-10-m9-codequality-r1-r2-r4-review.md` (R1-01). | Define the `recv`/`timeout_ms` and partial-`send` semantics **in the contract** (per-backend allowed), so both the loopback and the lwIP adapter are judged against one statement. |
| D-R1-03 | Comment | `tests/unit/test_netloop.c`; SRS-REQ-039 (`method: Tst`) | **The R1-03 fixes carry no regression test.** `test_netloop.c` grew by exactly three assertions (26 -> 29), all of them the R1-01 out-of-range handle cases (`:73-76`); there is still **no** case for duplicate `bind` (`FF_ALREADY_EXISTS`), `listen` without `bind`, or `open` exhaustion. The suite cannot detect a regression that re-introduces the R1-03 defect. (This is the substance of R2-01, which remains open in the R2 profile - see §4.) | `tests/unit/test_netloop.c:14-79` (two functions only); `./build/native/tests/test_netloop` -> `checks=29`; `src/drivers/net/ff_net_loopback.c:85-87,97-98`. | In the R2-01 scope, add: duplicate `bind` -> `FF_ALREADY_EXISTS`; `listen` without `bind` -> `FF_INVALID_ARGUMENT`; `open` exhaustion -> `-1`; ring-full partial `send`; `accept` empty; `NULL`-argument paths. |
| D-R1-04 | Comment | `docs/vv/verification-results.md:321`; N8/N11 | **A closure-facing record can be read as closing FRICK-REVIEW-0032.** The review-evidence table states `M4 netstack + M8 review ... REJECTED, Major fixed (netstack bounds)`, while the review log (`docs/reviews/README.md:33`) correctly says "closure pending independent delta" and the report's own table says `open (author disposition)`. The wording "Major fixed" refers to the code fix (true) but reads as a closure verdict. | `docs/vv/verification-results.md:321`; `docs/reviews/README.md:33`; `2026-10-10-m4-netstack-r1-r2-r4-review.md:183`. | Align the wording with the log (e.g. "REJECTED; Major fixed in code, closure pending delta") as part of the R2/R4 delta and the closure bookkeeping. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No **Major** finding is open in this delta.

## 4. NC verdicts and conclusion

### 4.1 R1 non-conformance verdicts (independent, at `e1def89`)

| NC | Verdict | Objective re-verification evidence |
| -- | ------- | ---------------------------------- |
| **R1-01** | **CLOSED** | `sock_of` now compares the **signed** handle against the table bound with no narrowing of the handle: `if ((sd < 0) \|\| (sd >= (FF_NetSock_t)FF_NETLOOP_MAX)) return NULL;` (`ff_net_loopback.c:48`); the cast is applied to the constant (`4u` -> `4`), and `FF_NetSock_t` is `int` (`hal_net.h:18`), so indices reaching `g_sock[(uint16_t)sd]` (`:51,:54`) are provably in `[0,4)`. Independent ASan+UBSan probe driving **all eight** vtable entry points with `{65536, 65540, INT_MAX, INT_MIN, -1, 4, 3}` produced no diagnostic and uniformly rejected them; the exact original attack value `65536` (which the predecessor review showed SEGV'ing) is now rejected. Out-of-range tests exist: `test_netloop.c:73-76` (`close`/`recv`/`send` with `65536` -> `FF_INVALID_ARGUMENT`). `test_netloop` `checks=29 failures=0`; `ctest` 14/14; `-Wconversion -Wsign-conversion` clean. |
| **R1-03** | **CLOSED** | `open` resets `bound` and `port` (`ff_net_loopback.c:66,68`); duplicate `bind` returns `FF_ALREADY_EXISTS` (`:85-87`); `listen` without a successful `bind` returns `FF_INVALID_ARGUMENT` (`:97-98`). Independently reproduced: duplicate `bind` -> `-13` (`FF_ALREADY_EXISTS`); after close+reopen, `listen` without `bind` -> `-2` (`FF_INVALID_ARGUMENT`) and a subsequent `connect` to the previously used port `7` -> `FF_ERROR` (the stale-port exposure is gone). |
| **R1-02** | **design disposition ACCEPTED; NC OPEN (tracking incomplete)** | The described degradations are **true** at HEAD, independently reproduced: `listen(backlog=0)` succeeds and a connection is still accepted (`(void)backlog`, `ff_net_loopback.c:96`) -> `backlog` is not honoured; closing a client before `accept` and connecting again reuses the freed slot and overwrites the listener's `pending` (`:127-128`), so `accept` returns the new relay and the first relay slot stays `used` and unreachable (orphan); after **one** orphan cycle **0** of the 4 slots remain openable, and `init()` reclaims all 4 (`:31-44`) - i.e. the leak is **bounded by the table and reclaimed by `init()`**; closing a listener with a pending relay leaks that one slot (3 of 4 free). Independent-descriptor semantics hold: `close(acc)` then `close(cli)` both succeed (`test_netloop.c:50-52`), so the "free the relay when its peer closes" change would indeed have altered documented behaviour. For a **bounded, static, in-process reference backend** whose production path is the lwIP adapter (SRS-REQ-040), a documented bounded limitation is an **acceptable** disposition (no memory corruption, no unbounded growth, no safety effect; the defect requires repeated misuse of a reference-only API). **However**, the NC is not yet fully closed: N19 requires the non-class-A gap to be tracked in `class-a-gaps.md`/`development-plan.md` and the §8 disposition asserts `development-plan.md` tracking that does not exist (**D-R1-01**). |
| **R1-04** | **CLOSED** | `ff_net_loopback.h:10-12` documents partial `send` (`*sent < len` returned as success) and non-blocking `recv` (`*got == 0` returned as success); `ff_net_loopback.c:178` marks `timeout_ms` ignored with the reason. The requested documentation exists in the implementation header; the residual contract-wording tension is D-R1-02 (Comment). |
| **R1-05** | **CLOSED** | `ff_net_loopback.h:12-13` states "The sockets are not thread-safe: calls are serialised by the owning task", which is consistent with the single `static g_sock[]` table and absence of locks (`ff_net_loopback.c:29`). |
| **R1-06** | **CLOSED** | The unused `#include "fsw_config.h"` is gone; the translation unit has exactly one include (`ff_net_loopback.c:10`). Compiles clean with `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion`. |

### 4.2 Graded conclusion - R1 profile

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code), N9 | **APPROVED WITH COMMENTS** | No Major open. R1-01 (the only Major) is closed with independent code + sanitizer + test evidence; R1-03/R1-04/R1-05/R1-06 are closed. R1-02's bounded-limitation disposition is accepted, but its closure is held open on the N19 tracking defect D-R1-01 (Minor). D-R1-02/D-R1-03/D-R1-04 are Comments. |

### 4.3 May `FRICK-REVIEW-0032` now be marked closed?

- **For the R1 subset: yes, once (i) this delta is recorded in the §7/§8 tables
  and the review log (norm N11) and (ii) D-R1-01 is fixed** (the one-line
  `development-plan.md`/`class-a-gaps.md` entry with owner frick-fsw and
  milestone M4/M11, the `loop-audit.md` 0.2 History row, and truthful §8
  wording). R1-01, R1-03, R1-04, R1-05 and R1-06 may be recorded **CLOSED** now;
  R1-02 may be recorded **CLOSED** when D-R1-01 is closed (it must be **tracked,
  not waived** - the limitation is accepted, the record is not).
- **For the review as a whole: not yet.** `FRICK-REVIEW-0032` is a three-profile
  review (R1+R2+R4) and its §7 table holds R2/R4 NCs that are outside this
  profile. On inspection (not R1 adjudication), **R2-01 remains substantively
  open**: the suite grew only by the three R1-01 out-of-range assertions
  (documented count 29 reproduces), and none of the other requested negative/
  boundary cases (null arguments, empty `accept`, table exhaustion, ring-full
  partial `send`, ring wrap, slot reclaim) exist. Closing the review while an
  open Minor of another profile is unverified would repeat the record-integrity
  failure class already raised by the PA reviews. The R2 and R4 delta
  re-reviews must sign off their NCs first.

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure (R1 profile)
- [ ] REJECTED - one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-11 | APPROVED WITH COMMENTS (R1-01 Major closed; R1-02 held on D-R1-01) |

Not signed here: R2/R4 (their NCs are out of this profile) and PA (norm N20).

## 6. Follow-up records

(non-conformances and their dispositions; norm N12. These are the R1 NCs of
`FRICK-REVIEW-0032` re-verified independently at `e1def89`, plus the delta
findings D-R1-01..D-R1-04 raised by this re-review. Recording them into the
`FRICK-REVIEW-0032` §7 table is an author action, norm N11.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_net_loopback.c` (`sock_of`), SRS-REQ-039 | `(uint16_t)` cast defeated the descriptor bounds check -> OOB read/corruption | Compare the signed handle directly; add an out-of-range test | `ff_net_loopback.c:46-55` (signed check `:48`, no handle narrowing); `hal_net.h:18` (`FF_NetSock_t` is `int`); `test_netloop.c:73-76`; `test_netloop` `checks=29 failures=0`; `ctest` 14/14; independent ASan+UBSan probe over 7 hostile handles x 8 vtable entry points -> clean; `-Wconversion` clean | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 (this report) |
| R1-02 | Minor | `ff_net_loopback.c` (connect/close/listen) | Relay-slot leak + `pending` overwrite (`backlog` ignored) | Bounded, documented limitation for the reference backend; tracked | Degradations reproduced at HEAD (backlog=0 accepted; orphan after early close; 0/4 slots openable after one cycle; 4/4 after `init()`; 3/4 after listener+client close; independent descriptors per `test_netloop.c:50-52`); tracked in `loop-audit.md:139` (owner frick-fsw, M4/M11) but **not** in `development-plan.md`/`class-a-gaps.md` (**D-R1-01**) | frick-fsw | **OPEN** (design disposition accepted; record/tracking incomplete) | FRICK-REVIEW-0054 (this report) |
| R1-03 | Minor | `ff_net_loopback.c` (open/bind/listen) | `port` not reset; duplicate bind accepted; stale port after reopen | Reset `port`; reject duplicate bind; require bind before listen | `ff_net_loopback.c:66,68` (reset), `:85-87` (`FF_ALREADY_EXISTS`), `:97-98` (bind required); probe: duplicate bind `-13`, `listen` w/o bind after reopen `-2`, `connect` to stale port `FF_ERROR` | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 (this report) |
| R1-04 | Comment | `ff_net_loopback.{c,h}`, `hal_net.h` | Partial `send`/empty `recv` returned as `FF_SUCCESS`; undocumented | Document the semantics | `ff_net_loopback.h:10-12`; `ff_net_loopback.c:178`; contract wording tension remains (D-R1-02) | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 (this report) |
| R1-05 | Comment | `ff_net_loopback.{h,c}` | No single-threaded/not-reentrant statement | Document | `ff_net_loopback.h:12-13`; `ff_net_loopback.c:29` (single static table, no lock) | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 (this report) |
| R1-06 | Comment | `ff_net_loopback.c` (include) | Unused `#include "fsw_config.h"` | Remove | `ff_net_loopback.c:10` is the only include; `-Wall -Wextra -Wpedantic -Wconversion` clean | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 (this report) |
| D-R1-01 | Minor | `development-plan.md` / `class-a-gaps.md`; N19/N8/N16 | R1-02 residual not tracked where N19 requires; §8 and the commit message claim it is; loop-audit Rev 0.2 has no 0.2 History row | Add the entry with owner/milestone; add the History row; correct the wording | `loop-audit.md:7,139,142-146`; `git show e1def89 -- docs/development-plan.md` (no entry); `grep` -> no R1-02/relay/orphan match in plan or class-a-gaps | frick-fsw | **open** | FRICK-REVIEW-0054 (this report) |
| D-R1-02 | Comment | `hal_net.h:30-31`; SRS-REQ-027 | Contract calls the API "blocking" while the loopback is non-blocking | State the timeout/partial-transfer semantics in the contract | `hal_net.h:30-31`; `ff_net_loopback.h:10-13`; `ff_net_loopback.c:178`; FRICK-REVIEW-0044 R1-01 | frick-fsw | **open** | FRICK-REVIEW-0054 (this report) |
| D-R1-03 | Comment | `tests/unit/test_netloop.c`; SRS-REQ-039 | No regression test for the R1-03 fixes (R2-01 still open) | Add negative/boundary cases in the R2-01 scope | `test_netloop.c:14-79`; `checks=29`; `ff_net_loopback.c:85-87,97-98` | frick-fsw | **open** | FRICK-REVIEW-0054 (this report) |
| D-R1-04 | Comment | `docs/vv/verification-results.md:321`; N8/N11 | "Major fixed" wording can read as closing `0032` | Align with the log row ("closure pending delta") | `verification-results.md:321` vs `docs/reviews/README.md:33` | frick-fsw | **open** | FRICK-REVIEW-0054 (this report) |

---

## 7. Independent positive evidence (reproduced in this session)

- Host unit test: `./build/native/tests/test_netloop` -> `checks=29 failures=0`;
  the documented count (`CHANGELOG.md:109`, `docs/vv/tool-qualification.md:87`,
  `docs/vv/verification-results.md:270`) matches the code.
- `ctest --test-dir build/native` -> **14/14 PASS**, 0 failed.
- `trace.py` -> **PASS** (94/54/26/14, 51 lean theorems); the SRS-REQ-039 entry
  resolves its implementation and test paths
  (`traceability.yaml:371-377`).
- Compiler hygiene: `ff_net_loopback.c` is warning-free under
  `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion`.
- **R1-01 safe under a memory-safety tool**: an independently authored
  ASan+UBSan probe (harness in `/tmp/opencode/delta-r1`, not committed) calling
  `close/bind/listen/connect/accept/send/recv` with
  `{65536, 65540, INT_MAX, INT_MIN, -1, 4, 3}` returned
  `FF_INVALID_ARGUMENT`/`-1` for every call with **no** sanitizer diagnostic.
  The original attack value (`65536`) that produced an OOB access in the
  predecessor review is now rejected before indexing.
- Ring/lifecycle behaviour: the R1-02 degradations were reproduced
  independently (see §4.1), which confirms the disposition's description is
  accurate and bounded rather than a hidden corruption.

## 8. Uncertainty and confidence

- **Facts** (source-linked, date-stamped): all file:line quotes above are from
  `e1def89` read directly; test/ctest/trace outputs were produced in this
  session on 2026-10-11; the `e1def89` file diffs were read via `git show`.
- **Assumptions**: (a) the production path for the network stack is the lwIP
  adapter (SRS-REQ-040), so a bounded limitation in the loopback is acceptable -
  this is stated by the author and by `loop-audit.md`/`development-plan.md` M4;
  (b) the R2/R4 NCs remain their profiles' responsibility (not re-adjudicated).
- **Inferences**: that D-R1-01 is a records/N19 defect rather than a functional
  defect (the limitation itself is bounded and `init()`-reclaimable).
- **Recommendations**: fix D-R1-01 (one line + History row + wording), then
  record the R1 NCs as in §6; drive R2-01/R2-02 to closure before closing the
  review as a whole.
- **Confidence**: **High** for the R1-01/R1-03/R1-04/R1-05/R1-06 verdicts
  (code read, tests run, sanitizer probe). **High** for the R1-02 behavioural
  description (independently reproduced). **High** for D-R1-01 (direct file and
  `git show` evidence). **Medium** for the acceptability judgement of the R1-02
  bounded limitation (depends on the programme's validation of "reference-only
  backend" and on lwIP being the production path, per the assumptions above).
- **Open gaps**: no independent target/on-hardware evidence (GAP-A5); no
  R2/R4 delta adjudication in this report; `clang-format`/`clang-tidy` were not
  re-executed in this session (the predecessor review recorded them clean and the
  touched file was reformatted only in the deltas that added the guard).

---

Report written by the R1-profile reviewer only (report-only session; no other
repository file modified). Recording this delta into `FRICK-REVIEW-0032` (§7/§8)
and the review log (`docs/reviews/README.md`, norm N11) is an author action.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | independent reviewer agent (R1, fresh context) | Independent delta re-review (norm N12) of the R1 NCs of FRICK-REVIEW-0032 at `e1def89`: R1-01/R1-03/R1-04/R1-05/R1-06 closed; R1-02 bounded limitation accepted but held open on the N19 tracking defect (D-R1-01) |
