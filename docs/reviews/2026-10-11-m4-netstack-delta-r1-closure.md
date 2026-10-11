# Review Report - M4 netstack (FRICK-REVIEW-0032) R1 delta **closure** re-review, profile R1

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0057                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev / design + code)            |
| Reference revision                | commit `15d1d8f` ("Round 3: address R1/R2/R4 delta findings (N12)"), branch `main`; tracked tree clean (see §1 note on untracked sibling reports) |
| Items under review                | Closure delta re-review of the R1 findings of `FRICK-REVIEW-0054` (`D-R1-01`, `D-R1-02`, `D-R1-03`, `D-R1-04`) and of the held/conditional NC verdicts `R1-02` (tracking) and `R1-03` (cross-socket duplicate `bind`): `src/drivers/net/ff_net_loopback.{c,h}`, `src/hal/hal_net.h`, `tests/unit/test_netloop.c`, `docs/development-plan.md`, `docs/status/loop-audit.md`, `docs/quality/class-a-gaps.md`, `docs/vv/verification-results.md`, `docs/reviews/README.md`, `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md` (§7/§8), `docs/architecture/network-stack.md`, `docs/requirements/SRS.md`, `CHANGELOG.md` |
| Date                              | 2026-10-11                             |
| Scope                             | Independent **closure** re-review (norm N12: re-apply all rules to the affected scope and re-review the delta until compliance) of the R1 findings at `15d1d8f`, after `e1def89`. Re-executed: unit build, `./build/native/tests/test_netloop`, `ctest --test-dir build/native`, `trace.py`, an independently written ASan+UBSan behavioural probe, an independent RTEMS/RISC-V cross build of the Renode image and an independent Renode (N18) functional run. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N20. |
| Independence declaration          | The reviewer took no part in authoring, implementing, disposing or executing any reviewed item; it was spawned in a fresh context and inherited no author context. It executed only read-only verification of the repository plus throwaway artifacts outside it (probe, cross build and Renode log under `/tmp/opencode/closure-r1`; the pinned Renode container did not write into the repository). It wrote **only this report file**; no other tracked file was created or modified. `FRICK-REVIEW-0032` and the delta reports of the other profiles were read as objects of review, not endorsed. |
| Norms applied                     | N1-N20 (see `docs/rules.md`)           |

## 1. Scope

This is the closure step of the sequence
`FRICK-REVIEW-0032 → 0054 (R1 delta) / 0055 (R2 delta) / 0056 (N20 governance)`.
It adjudicates **only** the R1 items that `FRICK-REVIEW-0054` left open or
conditionally closed:

- **D-R1-01 (Minor)** - the R1-02 residual was not tracked where N19 requires,
  the `loop-audit.md` Rev 0.2 bump had no History row, the `Baseline commit` was
  stale, and the §8 disposition claimed a `development-plan.md` tracking entry
  that did not exist.
- **D-R1-02 (Comment)** - the `FF_NetStack_t` contract called the API "blocking"
  while the reference backend is non-blocking.
- **D-R1-03 (Comment)** - the R1-03 fixes carried no regression test.
- **D-R1-04 (Comment)** - `verification-results.md` wording could be read as
  closing `FRICK-REVIEW-0032`.
- **R1-02 (Minor)** - held open on D-R1-01 (design disposition accepted).
- **R1-03 (Minor)** - re-checked because `FRICK-REVIEW-0055` (R2, D-R2-04)
  recorded a divergence from my earlier `CLOSED` verdict on the same NC.

Out of this profile: the R2/R4/PA items and the closure of the review as a whole.

Commands and independent artefacts produced in this session (host Linux, GCC
15.2.0; independent of the author and of the other profiles):

- `./build/native/tests/test_netloop` -> `checks=65 failures=0` (exit 0).
- `ctest --test-dir build/native` -> `100% tests passed, 0 tests failed out of 14`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  `PASS (94 requirements, 54 srs, 26 sss, 14 gaps, 51 lean theorems)`.
- `gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror
  -fsanitize=address,undefined` over
  `src/drivers/net/ff_net_loopback.c` + an independently written probe
  (`/tmp/opencode/closure-r1/probe.c`, **not** in the repository) ->
  `PROBE PASS (0 failures)`, no sanitizer diagnostic. The probe drives: 10
  hostile/out-of-range handles x all 8 vtable entry points; the bind-rule matrix;
  a **genuine ring write-pointer wrap** (send 200, drain 150, send 200 -> writes
  indices 200..255 then 0..143) with byte-level verification; the R1-02 relay
  lifecycle; and the exact call sequence of the Renode contract check.
- Independent **RTEMS/RISC-V cross build at HEAD** in a throwaway container
  (`frick-rtems-toolchain:local` + `cmake 3.28.3`/`ninja`; `-DFF_PLATFORM=rtems
  -DFF_ENABLE_RENODE_TEST=ON`, default `FF_USE_PUS_ASN1=ON`) -> 46/46 targets
  linked; ELF `RISC-V EXEC`, entry `0x1000000000`, SHA-256
  `9d4357732a547d425bb4ed74d6692e412c0a1e30b843b670a9c9353ba7f22b37`.
- Independent **Renode functional run** (norm N18) with the pinned image
  `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`
  (the local image digest matches the script's pin) -> console:
  `hal_gpio=PASS`, `hal_serial=PASS`, `hal_mem=PASS`, `hal_eth=PASS`,
  **`hal_net=PASS`**, `sb_roundtrip=PASS`, **`RENODE_TEST: PASS`**.
- `clang-format --dry-run --Werror` (host, 23.1.3) on `ff_net_loopback.{c,h}`,
  `hal_net.h`, `test_netloop.c` -> clean.

Note on the working tree: at the time of this session `git status --short`
reported **no modified tracked file** and exactly two **untracked** sibling
reports from concurrent reviewer sessions
(`docs/reviews/2026-10-11-m4-netstack-delta-r2-closure.md` and
`docs/reviews/2026-10-11-n20-governance-r4-closure.md`). They are other
profiles' products; they were **not** used as evidence for any verdict below and
were not modified. All findings and verdicts are based on the tracked content of
`15d1d8f`.

Excluded / not executable here: on-target hardware evidence (GAP-A5);
`clang-tidy`/MISRA were not re-run (the pinned `frick-vv:local` image was not
used in this session, see §8); Lean was not rebuilt (its links are checked by the
trace gate, which passed). The Renode check is a target-execution contract check
whose two peers are the same loopback instance, i.e. target-execution evidence,
not independent validation.

## 2. References

- Project norms: `docs/rules.md` (N1-N20); N12 (NC close-out), N16 (revision
  history), N18 (Renode functional validation), N19 (loop audit).
- Predecessor reports: `docs/reviews/2026-10-11-m4-netstack-delta-r1-review.md`
  (FRICK-REVIEW-0054, adjudicated here); `docs/reviews/2026-10-11-m4-netstack-delta-r2-review.md`
  (FRICK-REVIEW-0055, D-R2-04 cross-socket `bind`); `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md`
  (FRICK-REVIEW-0032, §7 follow-up table and §8 author disposition).
- Review template: `docs/reviews/review-template.md`; review log:
  `docs/reviews/README.md`.
- Contract: `src/hal/hal_net.h` (SRS-REQ-027); requirement SRS-REQ-039
  (`docs/requirements/SRS.md:192-196`, `docs/requirements/traceability.yaml:371-378`).
- Tracking records: `docs/status/loop-audit.md`, `docs/development-plan.md`,
  `docs/quality/class-a-gaps.md`, `docs/vv/verification-results.md`,
  `docs/vv/tool-qualification.md`, `CHANGELOG.md`.
- Reviewer profiles: `docs/engineering/independent-review.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings (new, this closure re-review)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-CL-01 | Comment | `src/hal/hal_net.h:30-33`; `src/drivers/net/ff_net_loopback.c:89-95`; SRS-REQ-027 | **The new duplicate-`bind` status is backend-specific and is not stated in the contract.** The delta adds "reject a port already bound by another socket" returning `FF_ALREADY_EXISTS` (`ff_net_loopback.c:89-95`), and documents partial `send`/non-blocking `recv`/backend-specific `timeout_ms` in the contract (`hal_net.h:30-33`) - but says nothing about `bind` status codes. The sibling adapter maps *every* `lwip_bind` failure, including `EADDRINUSE`, to `FF_ERROR` (`src/hal/net/lwip/ff_net_lwip.c:71`), so the same "address in use" condition yields `FF_ALREADY_EXISTS` on the loopback and `FF_ERROR` on lwIP. `docs/architecture/network-stack.md` §2 lists no `bind` semantics either. | `ff_net_loopback.c:89-95`; `hal_net.h:30-33`; `ff_net_lwip.c:64-72`; `network-stack.md:21-26`; my probe: duplicate `bind` same socket = `-13`, cross socket = `-13`, port freed by `close` re-bindable = `FF_SUCCESS` | State the per-backend `bind` status in `hal_net.h` (and/or `network-stack.md` §2) as was done for `timeout_ms` in this delta, or make the adapters agree (`EADDRINUSE` -> `FF_ALREADY_EXISTS`). Non-blocking; no R1 NC depends on it. |
| D-CL-02 | Comment | `tests/unit/test_netloop.c:130-149`; D-R1-03 / R2-01 coverage criterion | **The new test comment claims a "wrap-around" case that the committed sequence does not exercise.** After `send 300` (`:143`, `n == 256`, ring full) and `recv 256` (`:145-146`) the peer `head` has returned to `0` and `count == 0`, so the following `send 10` (`:147`) writes indices `0..9`; **no `send` writes across the 256-byte boundary**. The head index does wrap during the drain, but the write-pointer wrap (`(head + count) % 256` with `head + count > 255`) is not covered. The **implementation** is nevertheless correct: my probe drove a real wrap (send 200; drain 150; send 200 -> indices 200..255 then 0..143) and all 250 bytes were delivered in order intact. | `test_netloop.c:130-149`; **instrumented replay of the committed sequence** (`/tmp/opencode/closure-r1/instr.c` + `seq.c`, not in the repository): `[instr] send len=10 head=0 count=0 first_idx=0 last_idx=9` - i.e. the "wrap-around" send starts at index 0; probe `/tmp/opencode/closure-r1/probe.c` -> `PROBE PASS (0 failures)`, genuine wrap verified byte-for-byte | Extend the case with a partial drain before the overflowing `send` (or reword the comment to what is actually asserted). The rest of the D-R1-03 case list is delivered (§4.1). |
| D-CL-03 | Comment | `docs/quality/class-a-gaps.md:198-199`; N16 | **Revision-history rows are out of order in the reviewed commit**: Rev `0.5` (2026-10-11) was inserted **above** Rev `0.4` (2026-10-10), so the history no longer reads chronologically (the header Revision is `0.5`). | `docs/quality/class-a-gaps.md:196-199`; `git diff e1def89..15d1d8f -- docs/quality/class-a-gaps.md` | Reorder the two rows. Cosmetic record hygiene; documentation scope (R4/PA to confirm). |
| D-CL-04 | Comment | commit message of `15d1d8f`; `docs/requirements/SRS.md:194-195`; `CHANGELOG.md:108`; N8/N16 (record integrity) | **A claim in the reviewed commit's message is not substantiated in the tree.** The message states "SRS-REQ-039 wording (pluggable network-stack; unit tests + emulated run, not host simulator) (R2-02/R4-07)". No such wording change exists: the only diff of `docs/requirements/SRS.md` in `15d1d8f` is a trailing newline, and at HEAD `SRS.md:194-195` still read "exercises the pluggable **TCP/IP** interface ... for unit tests and **the host simulator**" while `CHANGELOG.md:108` still reads "in unit tests and **on the host**". `R2-02`/`R4-07(a)` were open at `e1def89` (`FRICK-REVIEW-0055` D-R2-02/D-R2-07) and are still open at HEAD. This is the same defect class as my D-R1-01 (a record asserting an action absent from the tree). | `git diff e1def89..15d1d8f -- docs/requirements/SRS.md` (trailing-newline hunk only); `SRS.md:192-196`; `CHANGELOG.md:105-110`; `grep -rn "not host simulator"` -> 0 hits; `2026-10-11-m4-netstack-delta-r2-review.md:106,111` | Either make the rewording or correct the commit message/§8 record. **Out of the R1 profile** (routed to R2/R4/PA); no R1 NC is affected. Note: I audited only the claim adjacent to this netstack scope, not the full message (§8). |
| D-CL-05 | Comment | `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md` §7/§8; N11/N12 | **The `FRICK-REVIEW-0032` §7 table and §8 dispositions are still in the pre-delta state** ("open (author disposition)" / "(pending independent delta)" for every NC) and do not reference the delta verdicts. §8 itself is truthful ("closure is recorded only after the independent delta re-review ... populates the Disposition / Re-verification / Delta columns"), so this is a pending recording action, not a false statement. The review-log rows for 0054/0055/0056 **were** added (`docs/reviews/README.md:58-60`; `verification-results.md:336-338`). | `2026-10-10-m4-netstack-r1-r2-r4-review.md` §7 (all rows "(pending)"); §8; `docs/reviews/README.md:58-60` | Populate §7/§8 with the delta verdicts (and this closure report id) in the same cycle that records this closure (norm N11). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**No Major and no Minor finding is open in this closure delta.** All five new
findings are Comments; D-CL-04 is out of the R1 profile (record integrity routed
to R2/R4/PA) and D-CL-03 to documentation.

## 4. Verdicts and conclusion

### 4.1 Closure verdicts (independent, at `15d1d8f`)

| Item | Prior status | Verdict now | Objective re-verification evidence |
| ---- | ------------ | ----------- | ---------------------------------- |
| **D-R1-01** (R1-02 tracking; N19/N8/N16) | open (Minor) | **CLOSED** | (a) **N19 tracking exists in both named locations**: `docs/status/loop-audit.md:139` ("tracked (bounded 4-slot reference backend; `init()` resets) \| frick-fsw \| M4/M11") **and** `docs/development-plan.md:281-285` ("Tracked residuals (norm N19) ... FRICK-REVIEW-0032 R1-02; owner frick-fsw; **M4/M11**; lwIP is the production path"). The §8 claim is therefore now **substantiated**, and its wording ("tracked in `docs/status/loop-audit.md` and `docs/development-plan.md` with owner/milestone") is **accurate** - no rewording was required. (b) `loop-audit.md:147` now carries the **0.2 History row** ("Re-baselined at e1def89: counts 94/54/26/14/51, registry T1-T19, N9 row + PA, N20 in progress, R1-02 residual tracked"). (c) `loop-audit.md:7` `Baseline commit` is now `e1def89 (audited revision)` (was `e8daab3`). (d) The audit's claimed state is reproducible: `trace.py` -> 94/54/26/14/51; `loop-audit.md:53-54` show the N9 and N10/T1-T19 rows. (e) The tracked-residual **description** is truthful: my probe reproduced eager pairing, `backlog` ignored (`listen(...,0)` succeeds), `pending` overwritten (accept -> 3 after an early close) and the bound (0 of 4 slots openable after one orphan cycle, 4 after `init()`). |
| **D-R1-02** (contract wording) | open (Comment) | **CLOSED** | `hal_net.h:30-33` no longer says "blocking": "Socket-like API (bounded) ... `send` may transfer fewer bytes than requested (always check `*sent`); `recv` is non-blocking for the reference backend and `timeout_ms` semantics are backend-specific." This is consistent with the backend (`ff_net_loopback.h:10-13`, `ff_net_loopback.c:186` `(void)timeout_ms; /* loopback is non-blocking */`), with the sibling adapter's real behaviour (`ff_net_lwip.c:109-128`: `SO_RCVTIMEO` only when `timeout_ms > 0`), and with `docs/architecture/network-stack.md:21-26` ("`recv` semantics are backend-defined ..."), which already read that way at `e1def89`. The residual lwIP `timeout_ms == 0` (blocking/sticky) behaviour is unchanged by this delta and remains **FRICK-REVIEW-0044 R1-01**, not this NC. |
| **D-R1-03** (regression tests) | open (Comment) | **CLOSED** | `test_loopback_boundaries` (`test_netloop.c:79-152`) delivers the requested set: `NULL`/`addr_len == 0` (`:101-102`), duplicate `bind` same socket (`:104`) **and cross socket** (`:107`), distinct port (`:108`), `listen` without `bind` (`:120`), descriptor exhaustion + `init()` reset (`:123-128`), empty `accept` (`:110`), `NULL` out-parameters (`:111-113`), full-ring partial `send` 300 -> 256 (`:143-144`) with drain, and the head-index wrap of the 256-byte ring. Executed: `checks=65 failures=0`; the count is consistently re-recorded (`CHANGELOG.md:109`, `docs/vv/tool-qualification.md:100` (T3, Rev 0.3.3), `docs/architecture/network-stack.md:48`, `docs/vv/verification-results.md:270`) and matches the harness (`62` `FF_TEST_ASSERT` sites, one inside a 4-iteration loop). Residual: the "wrap-around" claim overstates the coverage (**D-CL-02**, Comment); the implementation itself is verified correct on a real write-pointer wrap by my probe. |
| **D-R1-04** (closure-facing wording) | open (Comment) | **CLOSED** | `docs/vv/verification-results.md:322` now reads "REJECTED; Major fixed in code, closure pending independent delta", aligned with the review log (`docs/reviews/README.md:33`: "REJECTED -> closure pending independent delta ..."). No wording at HEAD reads as a closure verdict for `FRICK-REVIEW-0032`. |
| **R1-02 (held NC)** | design disposition accepted; NC open on D-R1-01 | **CLOSED** | The bounded limitation is real, bounded and `init()`-reclaimable (independently reproduced, §4.1 D-R1-01 evidence), acceptable for a reference backend whose production path is the lwIP adapter (SRS-REQ-040), and it is **tracked with owner and milestone in both N19 locations** (`loop-audit.md:139`; `development-plan.md:281-285`) - i.e. **tracked, not waived** (the limitation is accepted, the record is now complete). |
| **R1-03 (divergence reconciliation)** | CLOSED in 0054; `FRICK-REVIEW-0055` D-R2-04 recorded a divergence | **CLOSED (and strengthened)** | Reconciliation: `FRICK-REVIEW-0054` adjudicated the three behaviours in the author's disposition (same-socket duplicate `bind`, `port` not reset on `open`, `listen` without `bind`) and verified them fixed in code (`ff_net_loopback.c:66,68`, `:86`, `:106`). `FRICK-REVIEW-0055` D-R2-04 correctly added the **wider** case (two different sockets binding the same port, both listening, first listener always selected) which my scope had not covered. At HEAD that case is **fixed** - `netloop_bind` scans the table and returns `FF_ALREADY_EXISTS` for a port already bound by another socket (`ff_net_loopback.c:89-95`) - and covered by `test_netloop.c:107`; my probe reproduced: same-socket `-13`, cross-socket `-13`, distinct port `FF_SUCCESS`, port freed by `close` re-bindable `FF_SUCCESS`, and binding to a listening socket's port rejected. Residual: D-CL-01 (status wording, Comment). No divergence remains between the R1 and R2 verdicts. |
| **R1-01, R1-04, R1-05, R1-06** | CLOSED in 0054 | **CLOSED (re-verified)** | R1-01: the signed guard is intact and unnarrowed (`ff_net_loopback.c:46-55`, check at `:48`; `FF_NetSock_t` is `int`, `hal_net.h:18`); an independent ASan+UBSan probe drove **10** handles `{65536, 65540, INT_MAX, INT_MIN, -1, 4, 3, 2, 1, 0}` x **all 8** vtable entry points -> every call rejected (`FF_INVALID_ARGUMENT` / `-1`), **no sanitizer diagnostic**; the discriminating regression case is present (`test_netloop.c:74-76`). R1-04/R1-05: `ff_net_loopback.h:10-13` documents partial `send`, non-blocking `recv` and non-reentrancy. R1-06: the translation unit has one include (`ff_net_loopback.c:10`). The file compiles warning-free under `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror`; `clang-format --dry-run --Werror` clean on the four affected files. |

### 4.2 Graded conclusion - R1 profile

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code), N9/N12 | **APPROVED WITH COMMENTS - all prior findings closed** | No Major, no Minor open. **Every** prior R1 finding is closed with objective evidence: `D-R1-01` (N19 double tracking + History row + re-baseline, §4.1), `D-R1-02`, `D-R1-03`, `D-R1-04`, plus the held `R1-02` (accepted and now fully tracked) and `R1-03` (strengthened by the cross-socket fix; the R2 divergence is reconciled). `R1-01`/`R1-04`/`R1-05`/`R1-06` remain closed. The five new findings are **Comments only** (`D-CL-01`..`D-CL-05`), one of which (`D-CL-04`) is routed outside the R1 profile. Gates reproduced in this session: `test_netloop` 65/0, `ctest` 14/14, `trace` PASS, sanitizer clean, independent cross build + **`RENODE_TEST: PASS` incl. `hal_net=PASS`**. |

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - all prior findings closed (5 new Comments, 0 Minor, 0 Major)
- [ ] REJECTED - one or more Major findings

Template note: with findings that are all Comments, the template's `APPROVED`
option ("no findings or Comments only") could also be read as applicable. This
report uses **APPROVED WITH COMMENTS** because the closure verdict carries
conditions (D-CL-05 recording) and leaves five Comments open; both readings
agree that there is no derogation and that every R1 NC is closed.

### 4.3 May the R1 subset of `FRICK-REVIEW-0032` now be signed off as closed?

- **R1 subset: YES.** All six R1 NCs of `FRICK-REVIEW-0032` (`R1-01`..`R1-06`)
  and all four R1 delta findings (`D-R1-01`..`D-R1-04`) are independently
  verified closed at `15d1d8f`; the R1-02 limitation is accepted **and** tracked
  (N19) rather than waived. The single remaining condition is administrative:
  `D-CL-05` - populate the §7/§8 columns of `FRICK-REVIEW-0032` and the
  review-log rows when recording this closure (norm N11). The review log itself
  already carries the delta reports (`docs/reviews/README.md:58-60`).
- **Review as a whole: not yet.** `FRICK-REVIEW-0032` is a three-profile review;
  its §7 table holds R2/R4 NCs outside this profile. On inspection (not R1
  adjudication) `R2-02` ("host simulator" claim / wording) is **still open** -
  at HEAD the SRS and CHANGELOG wording is unchanged (**D-CL-04**) - and the R4
  records have their own profile. The R2 and R4 closure re-reviews must sign off
  their NCs before the review can be closed as a whole (their closure reports
  exist as untracked working-tree files in this session; §1).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent, fresh context (FRICK-REVIEW-0057) | 2026-10-11 | **APPROVED WITH COMMENTS - all prior findings closed**; R1 subset of `FRICK-REVIEW-0032` may be recorded CLOSED (subject to D-CL-05 recording) |

Not signed here: R2/R4 (their NCs are out of this profile) and PA (norm N20).

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_net_loopback.c` (`sock_of`), SRS-REQ-039 | `(uint16_t)` cast defeated the descriptor bounds check -> OOB read/corruption | Fix the guard; add an out-of-range test | `ff_net_loopback.c:46-55` (signed check `:48`, no handle narrowing); `hal_net.h:18`; `test_netloop.c:74-76`; ASan+UBSan probe over 10 hostile handles x 8 vtable entry points -> clean; `test_netloop` 65/0; `ctest` 14/14 | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 / re-verified in this report |
| R1-02 | Minor | `ff_net_loopback.c` (connect/close/listen) | Relay-slot leak + `pending` overwrite (`backlog` ignored) | Bounded, documented limitation for the reference backend; **tracked** | Degradations reproduced (probe: accept -> 3 after early close; 0/4 slots openable after one orphan cycle; 4/4 after `init()`); tracked in `loop-audit.md:139` **and** `development-plan.md:281-285` (owner frick-fsw, M4/M11) | frick-fsw | **CLOSED** (accepted + tracked, not waived) | FRICK-REVIEW-0057 (this report) |
| R1-03 | Minor | `ff_net_loopback.c` (open/bind/listen) | `port` not reset; duplicate bind accepted; stale port after reopen; **cross-socket duplicate `bind`** (R2 D-R2-04) | Reset `port`; reject duplicate bind (same and cross socket); require bind before listen | `ff_net_loopback.c:66,68,86,89-95,106`; `test_netloop.c:104,107,120`; probe: same-socket `-13`, cross-socket `-13`, distinct port `FF_SUCCESS`, freed port re-bindable, listening port rejected; Renode `hal_net=PASS` | frick-fsw | **CLOSED** (strengthened; R1/R2 divergence reconciled) | FRICK-REVIEW-0057 (this report) |
| R1-04 | Comment | `ff_net_loopback.{c,h}`, `hal_net.h` | Partial `send`/empty `recv` returned as `FF_SUCCESS`; undocumented | Document the semantics | `ff_net_loopback.h:10-13`; `ff_net_loopback.c:186`; contract `hal_net.h:30-33` | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 / re-verified here |
| R1-05 | Comment | `ff_net_loopback.{h,c}` | No single-threaded/not-reentrant statement | Document | `ff_net_loopback.h:12-13`; `ff_net_loopback.c:29` (one static table, no lock) | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 / re-verified here |
| R1-06 | Comment | `ff_net_loopback.c` (include) | Unused `#include "fsw_config.h"` | Remove | `ff_net_loopback.c:10` is the only include; warning-free under `-Wconversion -Wsign-conversion -Werror` | frick-fsw | **CLOSED** | FRICK-REVIEW-0054 / re-verified here |
| D-R1-01 | Minor | `development-plan.md` / `class-a-gaps.md`; N19/N8/N16 | R1-02 residual not tracked where N19 requires; §8/commit message overstated; `loop-audit` 0.2 had no History row and a stale baseline | Add the entry with owner/milestone; add the History row; correct the wording | `development-plan.md:281-285`; `loop-audit.md:7,139,147`; §8 claim now substantiated in both files; counts 94/54/26/14/51 reproduced | frick-fsw | **CLOSED** | FRICK-REVIEW-0057 (this report) |
| D-R1-02 | Comment | `hal_net.h`; SRS-REQ-027 | Contract called the API "blocking" while the loopback is non-blocking | State the timeout/partial-transfer semantics in the contract | `hal_net.h:30-33`; consistent with `ff_net_loopback.h:10-13`, `ff_net_loopback.c:186`, `ff_net_lwip.c:109-128`, `network-stack.md:21-26` | frick-fsw | **CLOSED** | FRICK-REVIEW-0057 (this report) |
| D-R1-03 | Comment | `tests/unit/test_netloop.c`; SRS-REQ-039 | No regression test for the R1-03 fixes | Add negative/boundary cases; re-record | `test_netloop.c:79-152`; `checks=65 failures=0`; counts re-recorded in 4 documents; residual wrap-claim = D-CL-02 | frick-fsw | **CLOSED** (residual Comment D-CL-02) | FRICK-REVIEW-0057 (this report) |
| D-R1-04 | Comment | `docs/vv/verification-results.md`; N8/N11 | "Major fixed" wording could read as closing `FRICK-REVIEW-0032` | Align with the log row | `verification-results.md:322` = "REJECTED; Major fixed in code, closure pending independent delta" = `docs/reviews/README.md:33` | frick-fsw | **CLOSED** | FRICK-REVIEW-0057 (this report) |
| D-CL-01 | Comment | `hal_net.h`; `ff_net_loopback.c:89-95`; SRS-REQ-027 | New duplicate-`bind` `FF_ALREADY_EXISTS` is loopback-only and unstated in the contract; lwIP maps `EADDRINUSE` to `FF_ERROR` | Document the per-backend `bind` status, or align the adapters | `ff_net_loopback.c:89-95`; `ff_net_lwip.c:71`; `hal_net.h:30-33`; probe bind matrix | frick-fsw | **open** | FRICK-REVIEW-0057 (this report) |
| D-CL-02 | Comment | `tests/unit/test_netloop.c:130-149` | Comment claims a "wrap-around" case the sequence does not exercise (write-pointer wrap untested) | Add a partial drain before the overflowing `send`, or reword | Instrumented replay of the committed sequence (`head=0`, `first_idx=0`, `last_idx=9`); probe wrap case PASS (implementation correct) | frick-fsw | **open** | FRICK-REVIEW-0057 (this report) |
| D-CL-03 | Comment | `docs/quality/class-a-gaps.md:198-199`; N16 | History rows out of order (0.5 above 0.4) | Reorder the rows | `class-a-gaps.md:196-199`; `git diff e1def89..15d1d8f -- docs/quality/class-a-gaps.md` | frick-fsw | **open** | FRICK-REVIEW-0057 (this report) |
| D-CL-04 | Comment | `15d1d8f` commit message; `SRS.md:194-195`; `CHANGELOG.md:108`; N8/N16 | Commit message claims an SRS-REQ-039 rewording that is not in the commit or at HEAD (`R2-02`/`R4-07(a)` still open) | Make the rewording or correct the record (**R2/R4/PA scope**) | `git diff e1def89..15d1d8f -- docs/requirements/SRS.md`; `SRS.md:192-196`; `CHANGELOG.md:105-110`; `grep "not host simulator"` = 0 | frick-fsw | **open** (routed to R2/R4/PA) | FRICK-REVIEW-0057 (this report) |
| D-CL-05 | Comment | `2026-10-10-m4-netstack-r1-r2-r4-review.md` §7/§8; N11/N12 | `0032` §7/§8 still carry the pre-delta state; delta verdicts must be recorded | Populate §7/§8 + log rows with this closure | `0032` §7 (all "(pending)"), §8; `docs/reviews/README.md:58-60` | frick-fsw | **open** (recording action) | FRICK-REVIEW-0057 (this report) |

---

## 7. Independent positive evidence (reproduced in this session)

- Host unit test: `./build/native/tests/test_netloop` -> `checks=65 failures=0`;
  `ctest --test-dir build/native` -> **14/14 PASS**.
- `trace.py` -> **PASS** (94 requirements / 54 srs / 26 sss / 14 gaps / 51 lean
  theorems); SRS-REQ-039 resolves both its implementation and its test paths
  (`traceability.yaml:371-378`).
- **Memory safety under ASan+UBSan**: hostile handles `{65536, 65540, INT_MAX,
  INT_MIN, -1, 4, 3, 2, 1, 0}` x all 8 vtable entry points -> uniformly rejected,
  no diagnostic; the original attack value `65536` is rejected before indexing.
- **Ring correctness**: a genuine write-pointer wrap (send 200; drain 150; send
  200 -> indices 200..255 then 0..143) delivered all 250 bytes in order; a
  full-ring `send 300` returned `*sent == 256`; a subsequent 10-byte send/recv
  round-tripped (200/256/10 verified byte-for-byte). An **instrumented replay of
  the committed test sequence** shows the last send starts at ring index `0`
  (`head=0, count=0`), which is the evidence for D-CL-02.
- **Bind-rule matrix**: same-socket and cross-socket duplicate `bind` ->
  `FF_ALREADY_EXISTS`; distinct port -> `FF_SUCCESS`; port released by `close`
  re-bindable; `listen` without `bind` -> `FF_INVALID_ARGUMENT`; `listen(...,0)`
  accepted (documented `backlog` limitation).
- **Cross build (target realism)**: the changed sources compile and link for
  RTEMS 6.2 `riscv/mpfs64imafdc` at HEAD (46/46 targets; ELF entry
  `0x1000000000`, SHA-256 `9d435773…f22b37`).
- **Renode (norm N18) functional run at HEAD with the pinned image**:
  `RENODE_TEST: PASS`, including `hal_net=PASS` - i.e. the SRS-REQ-039 contract
  check still passes on the emulated PolarFire SoC after the new duplicate-`bind`
  rule. My host-side replay of the exact Renode call sequence
  (`app_renode_test.c:129-184`) also passes (probe), so the static reasoning and
  the target run agree.
- **Hygiene**: `clang-format --dry-run --Werror` clean on
  `ff_net_loopback.{c,h}`, `hal_net.h`, `test_netloop.c`; the changed file is
  warning-free under `-Wsign-conversion`-class flags.
- **Tracking truthfulness**: the R1-02 residual text in
  `development-plan.md:281-285` matches the independently reproduced behaviour
  (bounded by the 4-slot table, reclaimed by `init()`).

## 8. Uncertainty and confidence

- **Facts** (source-linked, date-stamped): every file:line quote above is from
  the tracked content of `15d1d8f` read directly; test/ctest/trace outputs, the
  probe result, the cross-build result and the Renode console marker were
  produced in this session on 2026-10-11; diffs were read via
  `git diff e1def89..15d1d8f` and `git show`.
- **Assumptions**: (a) the production network path is the lwIP adapter
  (SRS-REQ-040), so a bounded limitation in the reference loopback remains
  acceptable (stated by the author, `loop-audit.md:139`,
  `development-plan.md:285`); (b) R2/R4/PA own their NCs and their closure
  reports; (c) the two untracked sibling reports in the working tree are other
  profiles' products and are neither evidence for nor part of the reviewed
  baseline.
- **Inferences**: that D-CL-04 is a record-integrity defect (commit-message
  claim not present in the tree) rather than a functional defect, because the
  netstack artefacts themselves are consistent at HEAD; that D-CL-01's
  cross-backend status difference is a documentation/consistency issue until a
  programme decision fixes the contract semantics (same reasoning that closed
  D-R1-02).
- **Recommendations**: (1) record this closure into `FRICK-REVIEW-0032` §7/§8
  and the review log in the same cycle (D-CL-05, norm N11); (2) close D-CL-01
  and D-CL-02 with the next touch of the network-stack artefacts (cheap: one
  contract sentence, one test drain); (3) route D-CL-04 to R2/R4/PA and note
  that the whole commit message was **not** systematically audited in this
  netstack-scoped session (only the claim adjacent to this scope).
- **Confidence**: **High** for every closure verdict (code read, tests executed,
  sanitizer probe, cross build and emulated-target run). **High** for D-CL-02
  (sequence arithmetic + probe) and D-CL-04 (direct git/file evidence).
  **Medium** for the acceptability judgement behind D-CL-01 (depends on the
  programme's decision on cross-backend status-code uniformity).
- **Open gaps**: no on-hardware evidence (GAP-A5); the Renode run is
  target-execution evidence whose two peers are the same loopback instance (no
  independent oracle); `clang-tidy`/MISRA and Lean were not re-executed in this
  session (the pinned `frick-vv:local` image was not used; the trace gate checked
  the Lean links); the cross build used `cmake 3.28.3` installed into a throwaway
  container because `/opt/rtems` is absent on this host (the repository's
  `make renode-image` path was therefore not used verbatim).

---

Report written by the R1-profile reviewer only. No repository file other than
this report was created or modified. Recording this closure into
`FRICK-REVIEW-0032` (§7/§8) and the review log (`docs/reviews/README.md`,
norm N11) is an author action (**D-CL-05**).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | independent reviewer agent (R1, fresh context) | Closure delta re-review (norm N12) at `15d1d8f`: D-R1-01/D-R1-02/D-R1-03/D-R1-04 closed, R1-02 accepted-and-tracked (closed), R1-03 strengthened (R1/R2 divergence reconciled); APPROVED WITH COMMENTS (all prior findings closed), 5 new Comments (D-CL-01..D-CL-05); R1 subset of FRICK-REVIEW-0032 closeable |
