# Review Report — M4 netstack **final** confirmation delta, profile R2 (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0061                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (SW V&V, N9/N10/N12)                |
| Reference revision                | commit `acff40e` ("Round 4: close R1/R2/R4 closure findings (N12)"), branch `main` |
| Predecessor under closure         | `FRICK-REVIEW-0058` (`docs/reviews/2026-10-11-m4-netstack-delta-r2-closure.md`), whose remaining R2 findings D-R2-02/06/07(a)/08/09/10 are adjudicated here |
| Items under review                | `docs/requirements/SRS.md` (SRS-REQ-039); `CHANGELOG.md`; `src/drivers/net/ff_net_loopback.{c,h}`; `tests/unit/test_netloop.c`; `tests/CMakeLists.txt`; `docs/requirements/traceability.yaml`; `docs/architecture/network-stack.md`; `docs/vv/verification-results.md`; the review records (`docs/reviews/README.md`, `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md` §7/§8) |
| Date                              | 2026-10-11                             |
| Scope                             | Independent **final** closure delta re-review (norm N12) of the R2 leftovers of `FRICK-REVIEW-0058` at `acff40e`, verified by re-execution (`test_netloop`, `ctest`, `trace.py`) and a **modulo-arithmetic mutation matrix** of the ring test; plus the explicit closure question for `FRICK-REVIEW-0032`. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N20. |
| Independence declaration          | The reviewer authored `FRICK-REVIEW-0058` and no other reviewed item; it took no part in implementing or executing the authoritative fixes. It re-verified read-only: mutations were compiled in `/tmp/opencode/r2final`; **only this report file** was written. The predecessor `FRICK-REVIEW-0058` and `FRICK-REVIEW-0032` were treated as objects of verification, not as evidence. |
| Norms applied                     | N1-N20 (see `docs/rules.md`)           |

## 1. Scope

This is the **final confirmation** delta that `FRICK-REVIEW-0058` §6 left open at
`15d1d8f`. It adjudicates each remaining R2 NC against the tree at `acff40e` and
answers the explicit question about `FRICK-REVIEW-0032`.

Verified by re-execution (not by reading prose):

- `./build/native/tests/test_netloop` → **`checks=328 failures=0`** (`exit=0`);
  `ctest --test-dir build/native` → **14/14** (0 failed).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → **PASS**
  (94 requirements, 54 srs, 26 sss, 14 gaps, 51 lean theorems checked).
- **Modulo-arithmetic mutation matrix** (the wrap test) — a mutant is built in
  `/tmp/opencode/r2final` by editing the three modulo sites of
  `src/drivers/net/ff_net_loopback.c` (`:174` send index, `:194` recv read index,
  `:195` recv head increment) and compiled with the **current** `test_netloop.c`.
  See §3 (D-R2-09) for the per-mutant result.
- `git diff 15d1d8f..acff40e` read for every reviewed file; `cat -A`/`od` used for
  the newline check; `grep -c FF_TEST_ASSERT` for the check counts.

Excluded: on-target/silicon timing and PHY/link behaviour (GAP-A5); the R1 and R4
profiles (covered by the sibling final reports `FRICK-REVIEW-0060` and
`FRICK-REVIEW-0062`); Lean and `clang-format` were not rebuilt. `metrics.py` was
**not** re-run in this session because it rewrites the tracked README status panel;
the coverage figure is a record, not a gate.

## 2. References

- Project norms: `docs/rules.md` (N1-N20); template: `docs/reviews/review-template.md`.
- Predecessor under closure: `docs/reviews/2026-10-11-m4-netstack-delta-r2-closure.md`
  (FRICK-REVIEW-0058).
- Object of closure: `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md`
  (FRICK-REVIEW-0032), §7/§8.
- Requirement/contract: SRS-REQ-039 (`docs/requirements/SRS.md:192-196`),
  SRS-REQ-027 (`docs/requirements/SRS.md:126`), `src/hal/hal_net.h`.
- Evidence records: `docs/vv/verification-results.md`,
  `docs/vv/tool-qualification.md`, `docs/reviews/README.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings — adjudication of `FRICK-REVIEW-0058`

| # | Severity | Item / Requirement | Finding / re-verdict | Evidence | Recommendation |
| - | -------- | ------------------ | -------------------- | -------- | -------------- |
| D-R2-02 | Minor → **CLOSED** | SRS-REQ-039 / `CHANGELOG.md` | **The "host simulator" claim is removed and the replacement wording is accurate for what is exercised.** `SRS.md:194-196` now reads "…exercises the pluggable network-stack interface … for the unit tests and the emulated-target (Renode) functional check"; `CHANGELOG.md:114-119` matches ("…exercising the contract in the unit tests and the emulated-target (Renode) functional check"). Verified against the code: the only non-self users of `FF_NetStackLoopback` are `tests/unit/test_netloop.c:16,57,81` and the norm-N18 Renode app `src/apps/app_renode_test.c:135-177` (`hal_net=PASS`, `:262`); the native simulator's six reference apps never instantiate it (`grep -rn FF_NetStackLoopback src tests`). The wording is therefore correct. | `SRS.md:192-196`; `CHANGELOG.md:114-119`; `git show acff40e -- docs/requirements/SRS.md CHANGELOG.md`; `grep -rn FF_NetStackLoopback src tests`; `app_renode_test.c:135-177,262` | Close D-R2-02. Residual in a *different* artifact → new Comment **FN-01** (`ff_net_loopback.h:6`). |
| D-R2-06 | Comment → **CLOSED** | `traceability.yaml`, `tests/CMakeLists.txt` (N16) | **Both named files now end with a newline**, and the wider normalisation is complete: `tail -c1` → `0x0a` for each, and a scan of **every** git-tracked text file (`git ls-files` filtered by `file --mime`) found **0 files** without a trailing newline. | `tail -c1 docs/requirements/traceability.yaml` → `0a`; `tail -c1 tests/CMakeLists.txt` → `0a`; repo-wide scan = "total missing: 0" | Close D-R2-06. |
| D-R2-07 | Comment → **(a) CLOSED** | `SRS.md:194`; (b) was already closed | **(a) The loose "TCP/IP interface" wording is gone.** `SRS.md:194` now says the reference implementation "exercises the pluggable **network-stack** interface" — the stack-independent `FF_NetStack_t` contract of SRS-REQ-027 (`SRS.md:126`); no "TCP/IP interface" string remains in SRS-REQ-039. `SRS-REQ-040`'s "lwIP TCP/IP stack" is a different, correct statement. | `SRS.md:194` (`grep -n "TCP/IP" docs/requirements/SRS.md` → only `:128` generic and `:199` lwIP); `git diff 15d1d8f..acff40e -- docs/requirements/SRS.md` | Close D-R2-07(a). |
| D-R2-08 | Comment → **CLOSED** | `docs/architecture/network-stack.md:49` | **The `test_lwip` count is correct.** `network-stack.md:49` now states **19 checks**, matching the source (`grep -c FF_TEST_ASSERT tests/unit/test_lwip.c` = 19) and consistent with `CHANGELOG.md`/`verification-results.md`. The `test_netloop` entry (`:48`) was updated to 328 in the same commit. | `docs/architecture/network-stack.md:48-49`; `grep -c FF_TEST_ASSERT tests/unit/test_lwip.c` = 19; `git show acff40e -- docs/architecture/network-stack.md`. Residual (ii): `verification-results.md:270` still prints the older M4 figure `1656/2521`; it sits under the M4-dated section and is a record, tracked as the non-blocking observation in §7 (not a V&V finding of this review). | Close D-R2-08 (count). |
| D-R2-09 | Comment → **CLOSED** (residual Comment FN-02) | `tests/unit/test_netloop.c:130-172` | **The new case genuinely crosses the 256-byte ring boundary and the modulo arithmetic is now mutation-discriminating.** After a 10-byte `recv` the write head is 10, so the subsequent full 256-byte `send` writes indices 10..255 then 0..9 (255 → 0 wrap); the drained-order assertion (`:159-161`) reads them back. Mutation matrix (current suite, `/tmp/opencode/r2final`): send-index modulo removed (`:174`) → **10 failures**; `% FF_NETLOOP_RING`→`% 255u` at `:174`/`:194`/`:195` → **12 / 1 / 11 failures**; control (no change) → 0. The wrap is real and discriminating. Residual: a *consistent* removal of the modulo on **all three** sites — the exact predecessor Mutation B — still yields **0 failures**, because the out-of-bounds indices land inside the neighbouring `g_sock` element and alias (ASan does not catch it either: object bounds, not sub-object). See FN-02. | `tests/unit/test_netloop.c:130-172` (`:144-161` the wrap block); `./build/native/tests/test_netloop` → `checks=328 failures=0`; `ctest --test-dir build/native` → 14/14; mutation matrix above; `src/drivers/net/ff_net_loopback.c:174,194,195` | Close D-R2-09; note FN-02 (non-blocking). |
| D-R2-10 | Comment → **OPEN (author action)** | `FRICK-REVIEW-0032` §7/§8; `docs/reviews/README.md`; N11/N12 | **Not recorded.** `FRICK-REVIEW-0032` §7 rows (`:183-198`) still carry `(pending)` / `open (author disposition)` / `(pending independent delta)` in every Disposition / Re-verification / Delta cell, and §8 (`:212-239`) is unchanged since `e1def89`. `docs/reviews/README.md` has **no** row for the three closure reports (`FRICK-REVIEW-0057/0058/0059`; last data row is `:60`). The closure reports themselves are committed in `acff40e`. This is a **bookkeeping/recording action** (the same is reported by R1 as `D-CL-05` of `FRICK-REVIEW-0060`). | `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md:183-198,212-239`; `git log --oneline -1 -- …/2026-10-10-m4-netstack-r1-r2-r4-review.md` → `e1def89`; `docs/reviews/README.md:58-60`; `git show --name-only acff40e` | At the closure commit: populate §7/§8 with the delta verdicts (§4.1) and append the `0057/0058/0059` rows to the log. |

**New observations (this report, non-blocking):**

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| FN-01 | Comment | `src/drivers/net/ff_net_loopback.h:6`; SRS-REQ-039 (consistency) | The driver header **still asserts the removed claim**: "It exercises the contract in unit tests and **on the host simulator**, and …". The SRS/CHANGELOG were corrected (D-R2-02) but this header comment was not, so the corrected baseline and the source disagree. No host-simulator app instantiates the stack. | `src/drivers/net/ff_net_loopback.h:6`; `grep -rn FF_NetStackLoopback src tests` (only `test_netloop.c` + `app_renode_test.c`); `SRS.md:194-196` | Reword `:6` to "in the unit tests and on the emulated target (Renode)" for consistency. Cosmetic. |
| FN-02 | Comment | `tests/unit/test_netloop.c:130-172`; `ff_net_loopback.c:174,194,195` | The wrap test is discriminating for a single-site modulo change, but a **consistent both-sided removal of the modulo survives** (`0 failures`), because the resulting out-of-bounds indices alias the adjacent `g_sock` element and the delivered byte order still matches. This is a mutation-testing alias, not a requirement defect: the loopback's specified behaviour (bytes delivered in order across the wrap) is preserved. Recorded so the "mutation-detected" statement is not overstated. | Mutation matrix in D-R2-09; `ff_net_loopback.c:174,194,195` | Optional: comment that the ring index arithmetic is verified for single-site faults; a both-sided removal is behaviourally equivalent under this stimulus. Non-blocking. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No **Major** finding is open. All prior **Minor** findings of `FRICK-REVIEW-0058`
are closed; the only unclosed prior item is the bookkeeping Comment **D-R2-10**.

## 4. Conclusion

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R2 (SW V&V, N9/N10/N12) | **APPROVED WITH COMMENTS** | No Major, no open Minor. Every prior R2 technical finding is verified closed by re-execution: D-R2-02 (accurate wording, no host-simulator use), D-R2-06 (newlines), D-R2-07(a) (contract wording), D-R2-08 (`test_lwip`=19), D-R2-09 (real boundary-crossing wrap, mutation-discriminating). The two new items are **non-blocking Comments** (FN-01 header wording, FN-02 mutation alias); D-R2-10 remains a recording action. |

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

### 4.1 Explicit statement on the verdict

All **prior R2 technical findings** of `FRICK-REVIEW-0058` are now closed (verified
above). The verdict is **APPROVED WITH COMMENTS** rather than a bare APPROVED
because this final pass raises two **brand-new, non-blocking Comments** (FN-01,
FN-02), and because the sole remaining prior R2 item — **D-R2-10** — is a
**recording action** (populate `FRICK-REVIEW-0032` §7/§8 and the review log), i.e.
the bookkeeping that *realises* the closure and is an author step, not a product
non-conformity. No new Minor is raised: no requirement or safety barrier is
defeated (the loopback delivers the specified byte stream; only the internal index
representation and a source comment are at issue). The verdict is consistent with
the sibling final profiles (`FRICK-REVIEW-0060` R1: APPROVED WITH COMMENTS with
`D-CL-05` open; `FRICK-REVIEW-0062` R4).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (SW V&V final delta re-review, N9/N12) | independent agent, fresh context (FRICK-REVIEW-0061) | 2026-10-11 | **APPROVED WITH COMMENTS** (0 Major, 0 Minor; all prior R2 technical findings closed; 2 new non-blocking Comments FN-01/FN-02; D-R2-10 recording remains the author action) |

## 6. Explicit answer: can `FRICK-REVIEW-0032` be closed from the V&V side?

**Yes, on the V&V (R2) substance — with one recording step outstanding.**

- Every R2 non-conformance of `FRICK-REVIEW-0032` is resolved with reproduced,
  objective evidence: **R2-01** (boundary/negative coverage, mutation-verified —
  D-R2-01 CLOSED), **R2-02** (wording corrected and accurate — D-R2-02 CLOSED),
  **R2-03** (closure-facing row aligned; the combined cycle row remains a tracked
  Comment, no derogation), **R2-04** (positive evidence, still valid). The residual
  Comments of the delta pass (D-R2-05..D-R2-09) are all closed.
- The **only** R2 item not yet satisfied is **D-R2-10**: the delta verdicts are not
  yet entered into `FRICK-REVIEW-0032` §7/§8 (`:183-198` still "(pending)") and the
  `FRICK-REVIEW-0057/0058/0059` rows are absent from `docs/reviews/README.md`. Per
  norm N12 the closure is *recorded* by exactly that population; it is an author
  action, not further V&V.
- Therefore: **from the V&V side, `FRICK-REVIEW-0032` may be recorded closed once
  §7/§8 and the review log are populated (D-R2-10).** No R2 technical barrier
  remains. (The R1 and R4 profile closure is outside this report; R1's `D-CL-05`
  and `D-FN-01` are the corresponding R1 items.)

## 7. Follow-up records

(non-conformances and dispositions, updated until closure; norm N12: NCs are
resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| D-R2-02 | Minor | SRS-REQ-039 / `CHANGELOG.md` | "host simulator" claim not exercised / unchanged | Reword to the exercised environments | `SRS.md:194-196`; `CHANGELOG.md:114-119`; `grep -rn FF_NetStackLoopback` → only `test_netloop.c` + `app_renode_test.c` | frick-fsw | **CLOSED** | FRICK-REVIEW-0061 (this report) |
| D-R2-06 | Comment | `traceability.yaml`, `tests/CMakeLists.txt` | Missing trailing newline | Add newline | both `tail -c1` → `0a`; repo-wide scan 0 files missing | frick-fsw | **CLOSED** | FRICK-REVIEW-0061 |
| D-R2-07 | Comment | `SRS.md:194` | Loose "TCP/IP interface" wording | Rephrase to the stack-independent contract | `SRS.md:194` "pluggable network-stack interface" (contract `FF_NetStack_t`, `:126`) | frick-fsw | **CLOSED (a)** | FRICK-REVIEW-0061 |
| D-R2-08 | Comment | `docs/architecture/network-stack.md:49` | `test_lwip` 15 vs 19 | Correct the count | `:49` = 19; `grep -c FF_TEST_ASSERT tests/unit/test_lwip.c` = 19 | frick-fsw | **CLOSED** | FRICK-REVIEW-0061 |
| D-R2-09 | Comment | `tests/unit/test_netloop.c:130-172` | "wrap-around" not exercised; modulo untested | Genuine boundary-crossing case | wrap block `:144-161`; run 328/0; mutations → 10 / 12 / 1 / 11 failures | frick-fsw | **CLOSED** (residual FN-02) | FRICK-REVIEW-0061 |
| D-R2-10 | Comment | `FRICK-REVIEW-0032` §7/§8; review log; N11 | Delta verdicts not recorded | Record at the closure commit | §7 `:183-198` still pending; §8 `:212-239` unchanged; log `:58-60` lacks `0057/0058/0059` | frick-fsw | **OPEN** (recording action) | FRICK-REVIEW-0061 |
| FN-01 | Comment | `ff_net_loopback.h:6` | Header still says "on the host simulator" | Reword for consistency | `:6` vs `SRS.md:194-196` | frick-fsw | **OPEN** (cosmetic) | new in FRICK-REVIEW-0061 |
| FN-02 | Comment | `test_netloop.c:130-172` | Consistent both-sided modulo removal survives (alias) | Optional comment | mutation matrix (0 failures; ASan also silent) | frick-fsw | **OPEN** (cosmetic) | new in FRICK-REVIEW-0061 |

Previously closed (not re-opened here): D-R2-01, D-R2-03, D-R2-04, D-R2-05.

## 8. Positive evidence reproduced in this session

- **All gates green and current at `acff40e`**: `test_netloop` **328/0**, `ctest`
  **14/14**, `trace.py` **PASS** (94/54/26/14, 51 theorems).
- **The "host simulator" claim is now correctly worded and matches the code**: the
  reference loopback is exercised by the unit tests and the Renode functional check
  (`hal_net=PASS`), and by nothing else.
- **The ring wrap is real**: the write index crosses 255 → 0 with `head=10`, and the
  drained-order assertion is discriminating (send-index / `%255` mutations fail
  10–12 checks; control passes).
- **Wording and records consistency**: SRS-REQ-039, CHANGELOG, `network-stack.md`
  and the test manifest now agree on the environments and the `test_netloop`/
  `test_lwip` counts; the whole tree is newline-normalised.

## 9. Uncertainty and confidence

```
Product/configuration and decision supported: frick-fsw M4 increment 4 (reference loopback
  network stack, SRS-REQ-039) - FINAL R2 closure adjudication of FRICK-REVIEW-0058 at acff40e
Information item/revision/status: FRICK-REVIEW-0061 (independent R2 final delta re-review), report only
Claim type: Fact (reproduced runs, git diff, quoted code/records, mutation matrix) +
  Inference (closure sufficiency, acceptability of the residual Comments) + Recommendation
Applicable baseline/tailoring: repo norms N1-N20 (docs/rules.md); ECSS-E-ST-40C, ECSS-Q-ST-80C
Sources and access level: full repository text and source (public, read directly); build, ctest,
  test_netloop, trace.py, the 15d1d8f..acff40e diff, and six mutation builds in /tmp/opencode/r2final
  all re-executed. No tracked file was modified.
Independent cross-check and conflict reconciliation: SRS wording vs code users vs Renode app vs test
  suite vs mutation; network-stack count vs grep; test suite vs predecessor Mutation B; review log vs
  0032 §7/§8; reconciled with R1's FRICK-REVIEW-0060 (same D-CL-05 recording gap).
Confidence: High for the technical closures (direct reproduction). Medium for the acceptability of the
  "host-simulator"-style residual wording (FN-01) and of the mutation alias (FN-02): both are
  judgement calls for the authority, not V&V defects.
Assumptions, TBD/TBC, detection limits: on-target/silicon PHY/link and real timing remain GAP-A5 and
  were not exercised; the Renode run is an emulated SoC, not hardware. metrics.py was not re-run (it
  rewrites a tracked file); the coverage figure was not independently re-derived. The mutation tests
  use reconstructed defects.
Consequences if the closure is granted now: the R2 findings are closed on substance; FRICK-REVIEW-0032
  would remain formally unrecorded until D-R2-10 (§7/§8 + log) is performed - a bookkeeping gap, not an
  evidence gap.
Required technical owner/reviewer/approver: frick-fsw author (D-R2-10, FN-01, FN-02); R2 final delta
  re-review per N12 (this report).
Recheck trigger/date: on the recording commit (D-R2-10) or on the next delta (norm N12).
```

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | independent reviewer agent (R2, FRICK-REVIEW-0061) | Final R2 delta re-review (norm N12) of FRICK-REVIEW-0058 at `acff40e`: D-R2-02/06/07(a)/08/09 CLOSED with reproduced evidence and a modulo mutation matrix; D-R2-10 remains a recording action; 2 new non-blocking Comments (FN-01 header wording, FN-02 mutation alias); FRICK-REVIEW-0032 closeable from the V&V side once §7/§8 + log are recorded |

---

Report written by the R2-profile reviewer only (report-only session; no other
repository file modified). Recording this delta into `FRICK-REVIEW-0032` §7/§8 and
appending the `FRICK-REVIEW-0061` row to `docs/reviews/README.md` (norm N11) is an
author action.
