# Review Report - M4 netstack (FRICK-REVIEW-0032) **final** R1 confirmation delta, profile R1

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0060                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev / design + code)            |
| Reference revision                | commit `acff40e` ("Round 4: close R1/R2/R4 closure findings (N12)"), branch `main`; tracked tree clean |
| Items under review                | **Final confirmation** of the five residual Comments `D-CL-01`..`D-CL-05` of `FRICK-REVIEW-0057` (`docs/reviews/2026-10-11-m4-netstack-delta-r1-closure.md`): `src/hal/hal_net.h`, `tests/unit/test_netloop.c`, `src/drivers/net/ff_net_loopback.c`, `docs/requirements/SRS.md`, `CHANGELOG.md`, `docs/quality/class-a-gaps.md`, `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md` (§7/§8), `docs/reviews/README.md` |
| Date                              | 2026-10-11                             |
| Scope                             | Independent **final delta confirmation** (norm N12) at `acff40e`, after `15d1d8f`, of the residual Comments of the R1 closure report `FRICK-REVIEW-0057`. Re-executed: unit build, `./build/native/tests/test_netloop`, `ctest --test-dir build/native`, `trace.py`, and an independent **mutation test** of the ring write-index arithmetic (proving the new wrap case discriminates). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N20. |
| Independence declaration          | The reviewer took no part in authoring, implementing, disposing or executing any reviewed item; it was spawned in a fresh context and inherited no author context. It executed only read-only verification of the repository plus throwaway artifacts under `/tmp/opencode/final-r1` (mutation builds; nothing written into the repository). It wrote **only this report file**; no other tracked file was created or modified (`FRICK-REVIEW-0032` was read as an object of review, not modified). |
| Norms applied                     | N1-N20 (see `docs/rules.md`)           |

## 1. Scope

This is the **final confirmation step** of the sequence
`FRICK-REVIEW-0032 → 0054 (R1 delta) → 0057 (R1 closure) → 0060 (this report)`.
It verifies, one by one and against the tracked content of `acff40e`, whether the
five residual Comments that `FRICK-REVIEW-0057` left open have been addressed:

- **D-CL-01** (Comment) - the duplicate-`bind` `FF_ALREADY_EXISTS` status was
  backend-specific and unstated in the contract `hal_net.h`.
- **D-CL-02** (Comment) - the new test comment claimed a "wrap-around" case the
  committed sequence did not exercise (write-pointer wrap untested).
- **D-CL-03** (Comment) - `docs/quality/class-a-gaps.md` History rows were out of
  ascending revision order.
- **D-CL-04** (Comment) - the `15d1d8f` commit message claimed an SRS-REQ-039
  rewording that was absent from the tree ("TCP/IP" / "host simulator").
- **D-CL-05** (Comment) - the `FRICK-REVIEW-0032` §7/§8 tables were still in the
  pre-delta state ("(pending)" for every NC).

Out of this profile: the R2/R4/PA items and the closure of `FRICK-REVIEW-0032`
as a whole. No other file was modified.

Commands and independent artefacts produced in this session (host Linux, GCC;
independent of the author and of the other profiles):

- `./build/native/tests/test_netloop` -> `checks=328 failures=0` (exit 0).
- `ctest --test-dir build/native` -> `100% tests passed, 0 tests failed out of 14`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  `PASS (94 requirements, 54 srs, 26 sss, 14 gaps, 51 lean theorems checked)`.
- **Independent mutation test** of the ring write index (source copied to
  `/tmp/opencode/final-r1`, compiled with `gcc -Isrc -Isrc/core -Iconfig -Itests`
  against the unmodified `test_netloop.c`):
  - baseline (unmutated) -> `checks=328 failures=0`;
  - **Mutation A** - write index modulo removed
    (`p->rx[(p->head + p->count)]`) -> `checks=328 failures=10` (`test_netloop.c:160`);
  - **Mutation B** - modulo changed to `FF_NETLOOP_RING - 1u` (255)
    -> `checks=328 failures=12` (`test_netloop.c:160`);
  - **Mutation C** - full-ring test `p->count >= FF_NETLOOP_RING` -> `>`
    -> `checks=328 failures=2` (`test_netloop.c:168`).
  A mutated modulo is therefore now detected, i.e. the wrap case is
  discriminating (D-CL-02).

Excluded / not executable here: on-target hardware evidence (GAP-A5); the Renode
run and `clang-format`/`clang-tidy` were not re-executed in this session (they are
unaffected by the D-CL-01..05 doc/test deltas; `FRICK-REVIEW-0057` re-ran them at
`15d1d8f`).

## 2. References

- Project norms: `docs/rules.md` (N1-N20); N9 (independent review), N11 (records
  each cycle), N12 (NC close-out), N16 (revision history), N19 (loop audit).
- Predecessor reports: `docs/reviews/2026-10-11-m4-netstack-delta-r1-closure.md`
  (FRICK-REVIEW-0057, the report under final confirmation); `...-delta-r1-review.md`
  (0054); `...-delta-r2-closure.md` (0058, read only as an object); `...-n20-governance-r4-closure.md`
  (0059, read only as an object); `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md`
  (FRICK-REVIEW-0032).
- Template and log: `docs/reviews/review-template.md`, `docs/reviews/README.md`.
- Contract/requirement: `src/hal/hal_net.h` (SRS-REQ-027), SRS-REQ-039
  (`docs/requirements/SRS.md:192-196`), manifest `docs/requirements/traceability.yaml`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings (new, this final confirmation)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-FN-01 | Comment | `tests/unit/test_netloop.c:162-166` | **A leftover clause in the new test comment is not asserted.** The comment reads "a partial send on a full ring transfers 0, and a **300-byte send caps at 256**"; the following assertions are `send 256 -> n == 256` (issued on an **empty** ring - it fills it) and `send 300 -> n == 0` (issued on a **full** ring). No assertion in this block shows a 300-byte send capping at 256 - that behaviour (the pre-delta `send 300 -> 256` on a non-full ring) was **removed** by this very delta. The first clause ("partial send on a full ring transfers 0") is the one actually asserted. | `test_netloop.c:162` (comment) vs `:163-166` (assertions); mutation C shows the full-ring branch is exercised (`:168`) | Delete the stale "caps at 256" clause (or add the intended capped-partial-fill case). Cosmetic comment-accuracy only; the implementation and its partial-send semantics (`send` transfers fewer than requested) are correct and exercised. Non-blocking. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**No Major and no Minor finding is raised in this confirmation.** The single new
finding (`D-FN-01`) is a Comment; it defeats no requirement and no safety barrier.

## 4. Verdicts and conclusion

### 4.1 Final confirmation of `FRICK-REVIEW-0057` Comments (independent, at `acff40e`)

| Item | Prior status | Verdict now | Objective confirmation evidence |
| ---- | ------------ | ----------- | ------------------------------- |
| **D-CL-01** | open (Comment) | **CLOSED** | `src/hal/hal_net.h:36-38` now documents the `bind` status: "`bind` fails if the address/port is already in use: the reference backend returns `FF_ALREADY_EXISTS` (same socket or another socket on the backend); adapters may map a platform error (e.g. `EADDRINUSE`) to `FF_ERROR`." Consistent with the backend (`ff_net_loopback.c:86-95`: same-socket `:86-87`, cross-socket `:89-95`) and with the sibling adapter that maps `lwip_bind` failure to `FF_ERROR`. The delta was applied in `acff40e` (`git diff 15d1d8f..acff40e -- src/hal/hal_net.h`, +3 lines). |
| **D-CL-02** | open (Comment) | **CLOSED** | `test_netloop.c:142-161` now genuinely crosses the ring boundary. Sequence: 10-byte send/recv advances the peer ring `head` to `10`; then `send 256` (`:154`) writes indices `(10 + 0..255) % 256 = 10..255` then `0..9` - the write pointer crosses `255 -> 0`; `recv 256` (`:157-158`) drains and the loop at `:159-161` asserts the byte-exact order `buf[i] == i` for all 256 bytes. Executed here -> `checks=328 failures=0`. **Discriminating**: independent mutation of the write index (Mutation A = modulo removed; Mutation B = modulo 255) makes the test fail (10 / 12 failures at `:160`), so a mutated modulo is now detected. `head + count` reaches `10 + 255 = 265 > 255`, i.e. the wrap is real and exercised. |
| **D-CL-03** | open (Comment) | **CLOSED** | `docs/quality/class-a-gaps.md:195-199` now lists the History rows in ascending revision order `0.1, 0.2, 0.3, 0.4, 0.5` (the `0.5` row moved below `0.4`); header `Revision 0.5` equals the latest row. Applied in `acff40e`. |
| **D-CL-04** | open (Comment) | **CLOSED** | `docs/requirements/SRS.md:192-196` SRS-REQ-039 now reads "exercises the pluggable **network-stack interface** without external dependencies (bounded, static, in-process loopback), for the **unit tests and the emulated-target (Renode) functional check**" - no "TCP/IP", no "host simulator". `CHANGELOG.md:115-119` was aligned ("in the unit tests and the emulated-target (Renode) functional check"). Applied in `acff40e` (verified with `git log -L '/SRS-REQ-039/,+4:docs/requirements/SRS.md'` -> the rewording is in the `acff40e` hunk). |
| **D-CL-05** | open (Comment) | **NOT YET ADDRESSED - still to be recorded** | The `FRICK-REVIEW-0032` report is **unchanged since `e1def89`** (`git diff 15d1d8f..acff40e -- docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md` is empty). §7 (`:183-198`) still carries `open (author disposition)` / `(pending independent delta)` for every NC, and §8 (`:212-239`) still lacks the delta verdicts. The review log (`docs/reviews/README.md`) has **no** rows for `FRICK-REVIEW-0057/0058/0059`, and `docs/vv/verification-results.md` has none either, although the three closure reports are now committed by `acff40e`. §8 already *states* (pre-existing text) that closure is recorded only after the delta re-review populates the columns; that statement is truthful but does not record the completed deltas. `D-CL-05` was formulated to allow either recording **or** stating that the delta verdicts are still to be recorded; the reviewer confirms they are **still to be recorded**. This is a bookkeeping/Comment item - no requirement or safety barrier is defeated. |

### 4.2 Graded conclusion - R1 profile

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code), N9/N12 | **APPROVED WITH COMMENTS** | No Major, no Minor. **Four** of the five prior Comments are closed with objective evidence (`D-CL-01`..`D-CL-04`); `D-CL-05` remains **open** as a non-blocking recording action (the delta verdicts are still to be recorded, which the Comment explicitly permits). One brand-new non-blocking Comment (`D-FN-01`, a stale test-comment clause; the implementation is correct). Every R1 non-conformity of `FRICK-REVIEW-0032` (`R1-01`..`R1-06`) and every R1 delta finding (`D-R1-01`..`D-R1-04`) was already closed and re-verified in `FRICK-REVIEW-0054`/`0057`; none is reopened here. Gates reproduced in this session: `test_netloop` 328/0, `ctest` 14/14, `trace` PASS, and a discriminating mutation test of the ring wrap. |

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - one prior Comment (`D-CL-05`) still open + one new Comment (`D-FN-01`); 0 Minor, 0 Major
- [ ] REJECTED - one or more Major findings

Note on the verdict: the prompt's `APPROVED` condition ("all prior R1 findings,
incl. `D-CL-01`..`05`, closed") is **not** met while `D-CL-05` remains unrecorded,
so the template-consistent verdict is **APPROVED WITH COMMENTS**. There is no
derogation: `D-CL-05` is a bookkeeping record and `D-FN-01` is a comment accuracy
item; neither defeats a requirement or a safety barrier.

### 4.3 Can `FRICK-REVIEW-0032` now be signed closed for the R1/R4-code subset?

- **R1 subset (design/code): YES - closeable.** All six R1 NCs (`R1-01`..`R1-06`)
  and all four R1 delta findings (`D-R1-01`..`D-R1-04`) are independently closed at
  `acff40e` (re-verified in `FRICK-REVIEW-0054`/`0057` and unaffected by this
  delta); the R1-02 limitation is accepted **and** tracked (N19), not waived. The
  only condition attached by `FRICK-REVIEW-0057` - populate §7/§8 and the log - is
  `D-CL-05`, an administrative recording step. The three closure reports
  (`FRICK-REVIEW-0057/0058/0059`) are now committed, so the R1 subset may be
  recorded **CLOSED** once `D-CL-05` is performed.
- **R4 subset (documentation of the code): the netstack R4 items appear closed at
  `acff40e`, but the signature is out of this profile.** Observed (not adjudicated):
  R4-01/R4-02/R4-03/R4-06 were closed earlier; R4-05 (missing trailing newlines,
  incl. `tests/CMakeLists.txt`) is now satisfied by the repo-wide final-newline
  normalization (`tests/CMakeLists.txt` ends in `0x0a`; `docs/status/status.json`,
  `docs/requirements/traceability.yaml` likewise); R4-07(a) ("TCP/IP" wording) is
  fixed by the SRS-REQ-039 rewording (`SRS.md:194`). R4 must issue its own
  confirmation.
- **Review as a whole: not yet.** Two cross-profile conditions remain: (i) the
  §7/§8 + log recording (`D-CL-05` here, `D-R2-10` in `FRICK-REVIEW-0058`); and
  (ii) the R2 subset is not closeable from the V&V side per `FRICK-REVIEW-0058`
  (its `D-R2-02` "host simulator" wording - note its substance now appears fixed
  by `acff40e`'s SRS/CHANGELOG rewording, which R2 must re-adjudicate). Per norm
  N12, `FRICK-REVIEW-0032` may be recorded closed only when every NC of every
  profile is resolved **and** the tables/log record the closure.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent, fresh context (FRICK-REVIEW-0060) | 2026-10-11 | **APPROVED WITH COMMENTS** - `D-CL-01`..`D-CL-04` CLOSED, `D-CL-05` still to be recorded, 1 new Comment (`D-FN-01`); R1 subset of `FRICK-REVIEW-0032` closeable |

Not signed here: R2/R4 (their NCs are out of this profile) and PA (norm N20).

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| D-CL-01 | Comment | `hal_net.h`; `ff_net_loopback.c:86-95`; SRS-REQ-027 | Duplicate-`bind` `FF_ALREADY_EXISTS` backend-specific and unstated in the contract | Document the per-backend `bind` status | `hal_net.h:36-38`; consistent with `ff_net_loopback.c:86-95` and the lwIP adapter (`EADDRINUSE` -> `FF_ERROR`) | frick-fsw | **CLOSED** | FRICK-REVIEW-0060 (this report) |
| D-CL-02 | Comment | `tests/unit/test_netloop.c` | "wrap-around" claim not exercised (write-pointer wrap untested) | Add a partial drain before the overflowing send | `test_netloop.c:142-161`; run 328/0; mutation A/B/C -> 10/12/2 failures (wrap is real, `head=10`, indices cross 255 -> 0) | frick-fsw | **CLOSED** | FRICK-REVIEW-0060 (this report) |
| D-CL-03 | Comment | `docs/quality/class-a-gaps.md`; N16 | History rows out of ascending order (0.5 above 0.4) | Reorder | `class-a-gaps.md:195-199` -> `0.1,0.2,0.3,0.4,0.5` | frick-fsw | **CLOSED** | FRICK-REVIEW-0060 (this report) |
| D-CL-04 | Comment | `docs/requirements/SRS.md`; `CHANGELOG.md`; N8 | Commit claimed an SRS-REQ-039 rewording absent from the tree | Make the rewording (or correct the record) | `SRS.md:192-196`; `CHANGELOG.md:115-119`; in commit `acff40e` | frick-fsw | **CLOSED** | FRICK-REVIEW-0060 (this report) |
| D-CL-05 | Comment | `2026-10-10-m4-netstack-r1-r2-r4-review.md` §7/§8; `docs/reviews/README.md`; N11 | `0032` §7/§8 still "(pending)"; delta verdicts and log rows not recorded | Populate §7/§8 + log rows at the closure commit | `0032` §7 `:183-198` (all pending), §8 `:212-239`; report unchanged since `e1def89`; no `0057/0058/0059` rows in `docs/reviews/README.md` or `verification-results.md` | frick-fsw | **open** (recording action) | FRICK-REVIEW-0060 (this report) |
| D-FN-01 | Comment | `tests/unit/test_netloop.c:162` | Leftover comment clause "a 300-byte send caps at 256" is not asserted (the 300-byte send is issued on a full ring -> `n == 0`) | Reword/remove the clause (or add the intended capped-partial-fill case) | `test_netloop.c:162` vs `:163-166`; implementation correct | frick-fsw | **open** (cosmetic) | new in FRICK-REVIEW-0060 |

---

## 7. Independent positive evidence (reproduced in this session)

- Host unit test at `acff40e`: `./build/native/tests/test_netloop` ->
  `checks=328 failures=0`; `ctest --test-dir build/native` -> **14/14 PASS**.
- `trace.py` -> **PASS** (94 requirements / 54 srs / 26 sss / 14 gaps / 51 lean
  theorems); SRS-REQ-039 resolves to both its implementation and its test paths.
- **Fit-for-purpose coverage of the ring wrap (mutation testing)**: removing the
  write-index modulo (Mutation A) or changing it to mod 255 (Mutation B) makes the
  new wrap/order assertions fail (10 / 12 failures at `test_netloop.c:160`); an
  off-by-one on the full-ring guard (Mutation C) fails at `:168` (2 failures). The
  new case is therefore discriminating, not decorative.
- **Count consistency**: `test_netloop` = 328 checks is recorded consistently in
  `docs/architecture/network-stack.md:48`, `CHANGELOG.md:16,119`,
  `docs/vv/tool-qualification.md:100` (T3, Rev 0.3.3) and
  `docs/vv/verification-results.md:270` - and reproduces.
- **Record hygiene of the delta**: `class-a-gaps.md` History ordered;
  `SRS-REQ-039`/`CHANGELOG` wording accurate; the `acff40e` message matches the
  tree for the netstack scope (the earlier `D-CL-04` commit-message claim is now
  substantiated).

## 8. Uncertainty and confidence

- **Facts** (source-linked, date-stamped): every `file:line` quote above is from
  the tracked content of `acff40e`, read directly; the test/ctest/trace outputs and
  the four mutation builds were produced in this session on 2026-10-11; diffs were
  read via `git diff 15d1d8f..acff40e` and `git log -L`.
- **Assumptions**: (a) the production network path is the lwIP adapter
  (SRS-REQ-040), so the bounded reference-loopback limitation remains acceptable;
  (b) R2/R4/PA own their NCs and closure reports; (c) `D-CL-05` is a bookkeeping
  action, not a functional defect.
- **Inferences**: that `D-FN-01` is a stale comment fragment carried over from the
  pre-delta `send 300 -> 256` case (the sequence now issues the 300-byte send on a
  full ring), i.e. a comment-accuracy defect rather than a behaviour defect.
- **Recommendations**: (1) perform `D-CL-05` and `D-R2-10` (populate `0032` §7/§8
  and append the `0057/0058/0059` rows to `docs/reviews/README.md` and the
  `verification-results.md` review-evidence table) in the closure commit; (2) with
  that recording and the R2 confirmation, the R1 subset of `FRICK-REVIEW-0032` may
  be recorded CLOSED; (3) optionally fix `D-FN-01` on the next touch of the test.
- **Confidence**: **High** for `D-CL-01`..`D-CL-04` (direct code/doc reads plus a
  discriminating mutation test) and for `D-CL-05` (direct git/file evidence that
  the `0032` report is unmodified since `e1def89`). **High** for `D-FN-01` (the
  comment text and the assertions are both quoted).
- **Open gaps**: no on-hardware evidence (GAP-A5); the Renode run and
  `clang-format`/`clang-tidy` were not re-executed in this session (they were not
  touched by the `D-CL-01`..`05` deltas; last re-run in `FRICK-REVIEW-0057`); the
  whole `acff40e` commit message was not systematically audited in this
  netstack-scoped session.

---

Report written by the R1-profile reviewer only. No repository file other than
this report was created or modified. Recording the `FRICK-REVIEW-0032` §7/§8
delta verdicts and the review-log rows (`D-CL-05`, norm N11) is an author action.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | independent reviewer agent (R1, fresh context) | Final R1 confirmation delta (norm N12) at `acff40e`: `D-CL-01`..`D-CL-04` CLOSED (bind doc, real ring wrap with discriminating mutation test, History order, SRS-REQ-039/CHANGELOG rewording in the commit); `D-CL-05` still to be recorded; 1 new Comment `D-FN-01`; verdict APPROVED WITH COMMENTS; R1 subset of FRICK-REVIEW-0032 closeable |
