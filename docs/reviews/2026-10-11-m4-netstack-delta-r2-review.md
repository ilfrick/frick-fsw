# Review Report - FRICK-REVIEW-0032 independent delta re-review (V&V / profile R2)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0055                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (SW V&V, N9/N10/N12)                |
| Reference revision                | commit `e1def89` ("PA review dispositions round 2 (N12/N20): truthful records + propagation + 0032 pending independent delta"), branch `main`, clean tree |
| Items under review                | Delta re-review of the **V&V non-conformances** of `FRICK-REVIEW-0032`: `tests/unit/test_netloop.c`; `src/drivers/net/ff_net_loopback.{c,h}`; `src/hal/hal_net.h`; `src/apps/app_renode_test.c`; `docs/requirements/SRS.md` (SRS-REQ-039) + `docs/requirements/traceability.yaml`; `docs/vv/verification-results.md`; `docs/vv/tool-qualification.md`; `CHANGELOG.md`; `README.md`; `docs/status/status.json`; `docs/status/loop-audit.md`; `docs/engineering/driver-development.md`; `.github/workflows/ci.yml`; and `FRICK-REVIEW-0032` §7 (follow-up) and §8 (author disposition) as the object of re-verification |
| Date                              | 2026-10-11                             |
| Scope                             | Independent delta re-review (norm N12) of the V&V criteria of R1-01, R2-01, R2-02, R2-03, R2-04 and of the R4-01..R4-06 items whose *criterion is V&V evidence* (coverage line, `status.json`, review-log row, documentation newlines), plus the R1-04/R1-05/R1-06 Comment items where the criterion is recorded evidence. Re-executed: unit build + `ctest`, `test_netloop`, `trace.py`, `metrics.py`, a **mutation test** that re-introduces the reviewed R1-01 defect, an independent behavioural probe of the untested branches, and an **independent RTEMS/RISC-V cross-build + pinned-Renode functional run** of the SRS-REQ-039 contract check. ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N20. |
| Independence declaration          | The reviewer took no part in authoring, implementing, disposing or executing any reviewed item; it was spawned in a fresh, independent context and inherited no author context. Verification was read-only with respect to tracked files: probes were built in `/tmp/opencode` and the Renode ELF in the gitignored `build/renode-r2`; **only this report file** was written. `FRICK-REVIEW-0032` and its §8 author disposition were treated as the object of verification, not as evidence. |
| Norms applied                     | N1-N20 (see `docs/rules.md`)           |

## 1. Scope

This is the **R2-profile delta re-review** that `FRICK-REVIEW-0032` §8 declares
pending ("closure is recorded only after the independent delta re-review
(`FRICK-REVIEW-0054`/`0055`)"). It adjudicates the V&V non-conformances and the
documentation items whose acceptance criterion is V&V evidence. The R1-profile
adjudication is owned by `FRICK-REVIEW-0054`
(`docs/reviews/2026-10-11-m4-netstack-delta-r1-review.md`) and is cross-referenced,
not duplicated — except where a code/behaviour claim has a **V&V evidence**
criterion (R1-01 regression coverage, R1-03 fix coverage), where this report
records its own independent result.

Verified (by re-execution, not by reading prose):

1. **R1-01 closure** — that `sock_of` no longer narrows the descriptor, that an
   out-of-range case exists, and **that the new case actually discriminates**:
   the reviewed defect was re-introduced into a throwaway copy and the suite was
   re-run against it (mutation test).
2. **R2-01** — which of the requested negative/boundary branches the suite now
   covers (`tests/unit/test_netloop.c`, 29 assertions) and which remain
   uncovered; the behaviour of the uncovered branches was independently probed.
3. **R2-02** — whether the SRS-REQ-039 / CHANGELOG "host simulator" claim is now
   exercised: source/`grep`/build-file analysis of the native simulator, plus an
   independent cross-build and Renode run of the SRS-REQ-039 contract check.
4. **R2-03 / R4-01** — the accuracy of `docs/vv/verification-results.md`
   (cycle rows, "Remaining (M4)", coverage, review-evidence table) against
   re-run `ctest`/`metrics.py`/`coverage.json`.
5. **R4-02..R4-06** — `status.json` milestones, README/driver-guide inventory,
   review-log row and trailing newlines.

Commands executed in this session (host, Linux; independent of the author):

- `cmake --build build/native` -> `ninja: no work to do` (build current) and
  `./build/native/tests/test_netloop` -> `checks=29 failures=0`.
- `ctest --test-dir build/native` -> `100% tests passed, 0 tests failed out of 14`.
- Per-suite check totals: ccsds 15, sb 20, tbl 9, pus 36, time 101, hal 31,
  math 20, sdls 81, gem 23, gem_tsn 16, netloop 29, gpio 30, serial 30, mem 23.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  `PASS (94 requirements, 54 srs, 26 sss, 14 gaps, 51 lean theorems checked)`,
  12 CI evidence claims.
- `python3 tools/scripts/metrics.py` (print-only; `git status` clean afterwards)
  -> README panel reproduces: 94 (54/26/14); tests 80/94 (85.1%); Lean 20/94;
  code line coverage **1880/2848 (66.0%), functions 256/301**, matching
  `build/coverage/coverage.json`.
- **Mutation test (R1-01)**: faithful re-introduction of the reviewed `sock_of`
  (narrowing check + un-narrowed `g_sock[sd]` index) in `/tmp/opencode/nlmut2`,
  compiled with the **current** suite -> `runtime error: index 65536 out of bounds
  for type 'FF_NetLoopSock_t [4]'` (UBSan) and `AddressSanitizer: SEGV ... in
  sock_of ... ff_net_loopback.c:54` (plain build: segmentation fault). The
  **fixed** code passes the same suite under ASan+UBSan with `checks=29
  failures=0`.
- **Independent behavioural probe** (`/tmp/opencode/nl_probe*.c`, not in the
  repository): duplicate/`NULL`/zero-length arguments, table exhaustion, empty
  `accept`, full-ring partial `send`, ring wrap across the 256-byte boundary,
  slot reclaim, the R1-02 relay orphan, and all eight vtable entry points with
  hostile handles.
- **Independent Renode reproduction (norm N18)**: cross-build in
  `frick-rtems-toolchain:local` (`-DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON`)
  -> **46/46 targets**, ELF SHA-256
  `d4e0799abfd55d2fb45e12dbd8c2735ddba4cc6ff079f4dd9c7f73479362e38c`, with
  `FF_NetStackLoopback` and `netloop_send` present in the target symbol table;
  run under the pinned image
  `antmicro/renode@sha256:7905c329…` -> console shows
  `hal_gpio/serial/mem/eth/net=PASS`, `sb_roundtrip=PASS`, `RENODE_TEST: PASS`.

Excluded / not executable here: real-silicon timing, PHY/link behaviour
(GAP-A5); `clang-format` and Lean were not re-run (format/Lean are not
implicated by any V&V NC of this delta; Lean links are covered by `trace.py`,
which passed). `FRICK-REVIEW-0032`'s R1 code findings other than the regression
evidence are adjudicated in `FRICK-REVIEW-0054`.

## 2. References

- Project norms: `docs/rules.md` (N1-N20); template:
  `docs/reviews/review-template.md`
- Predecessor under re-verification: `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md`
  (FRICK-REVIEW-0032), §7 and §8
- Parallel profile delta: `docs/reviews/2026-10-11-m4-netstack-delta-r1-review.md`
  (FRICK-REVIEW-0054, R1)
- Requirement/contract: SRS-REQ-039 (`docs/requirements/SRS.md:192-196`),
  `src/hal/hal_net.h` (SRS-REQ-027),
  `docs/requirements/traceability.yaml:371-378`
- Evidence records: `docs/vv/verification-results.md`,
  `docs/vv/tool-qualification.md`; `docs/status/{status.json,loop-audit.md}`
- ECSS-E-ST-40C, ECSS-Q-ST-80C

## 3. Findings (delta findings on the disposition)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-R2-01 | Minor | `tests/unit/test_netloop.c`; SRS-REQ-039 (`method: Tst`) | **R2-01 is only partially closed: the critical case was added, the rest of the requested branch set was not.** The suite grew from 26 to exactly **29** checks, i.e. **three** assertions, all of them the out-of-range handle cases (`test_netloop.c:74-76`); there is still **no** case for `bind`/`connect`/`send`/`recv` with `NULL` or `addr_len == 0`, `open` exhaustion, `accept` with `pending == NONE`, a full-ring partial `send`, the ring wrap across the 256-byte boundary, slot reclaim after `close`, or the new R1-03 defensive branches (`FF_ALREADY_EXISTS`, `listen` without `bind`). The suite still has two test functions only (`:14-53`, `:55-77`). My probe confirms the *implementation* behaves correctly on all of them (`NULL`/`addr_len==0` -> `FF_INVALID_ARGUMENT`; 5th `open` -> `-1`; empty `accept` -> `-1`; `send 300` -> `sent=256`, then `sent=0` on a full ring; wrap drains in order; `open` reuses slot 0) — so this is a **test-evidence gap, not a defect**. Additionally, `verification-results.md:270` claims `test_netloop` exercises "close unpairing", but no case asserts any unpairing effect (close is called at `:50-52` without an assertion on the resulting `peer` state). | `tests/unit/test_netloop.c:14-79` (29 `FF_TEST_ASSERT`); `./build/native/tests/test_netloop` -> `checks=29 failures=0`; `ff_net_loopback.c:63-77,85-87,97-99,139-141,162-168,186-189`; probe output (see §1); `docs/vv/verification-results.md:270` | Add the negative/boundary cases requested by R2-01 (they now exist behaviourally and are cheap to assert); add the three R1-03 cases; correct or drop the "close unpairing" coverage wording. Re-record the count in `CHANGELOG.md`, `docs/vv/tool-qualification.md` (T3) and `docs/architecture/network-stack.md` afterwards. |
| D-R2-02 | Minor | SRS-REQ-039 wording / `CHANGELOG.md`; R2-02 disposition ("ADDRESSED") | **R2-02 is not closed: the "host simulator" claim is still not exercised.** The new `renode_hal_net_check` (`src/apps/app_renode_test.c:123-183`) is real and reproduces (**I independently cross-built the RTEMS/RISC-V ELF and re-ran the pinned Renode image: `hal_net=PASS`, `RENODE_TEST: PASS`**), but it is compiled only under `#if defined(FRICK_PLATFORM_MPFS)` (`:26`, `:207`), and a grep of `build/native/build.ninja` for `FRICK_PLATFORM_MPFS`/`FF_ENABLE_RENODE_TEST`/`app_renode_test` returns **0** hits: the native host simulator (`make run`, `src/main/main_native.c`) links only the six reference apps (see `CMakeLists.txt:85-92`) and never instantiates `FF_NetStackLoopback`. `FF_ENABLE_RENODE_TEST` is also OFF by default (`CMakeLists.txt:24`). The SRS and CHANGELOG wording was **not** changed: `SRS.md:195` still reads "for unit tests and the host simulator" and `CHANGELOG.md:108` "in unit tests and on the host". `grep -rn FF_NetStackLoopback` still finds only the header, the driver, `test_netloop.c` and the platform-guarded app. A second, recorded caveat: the target check's two peers are the **same** loopback instance (no independent oracle; `ff_net_loopback.c:150-192`), so it adds target-execution proof, not independent validation. | `SRS.md:192-196` (`:194` "TCP/IP", `:195` "host simulator"); `CHANGELOG.md:105-110`; `app_renode_test.c:26,123-183,207,252-267`; `CMakeLists.txt:24,85-92,96-98`; `grep -c` over `build/native/build.ninja` = 0; my Renode run (SHA-256 `d4e0799a…`) | Either wire the loopback into the host simulator/reference app (`make run`) and record it, **or** reword SRS-REQ-039 and `CHANGELOG.md` to "unit tests and the emulated-target (Renode) functional check". Scope the added evidence as a target-execution contract check (per the phrasing already used in `verification-results.md:36`). |
| D-R2-03 | Comment | `docs/vv/verification-results.md` (N11/N16) | **R2-03 / residual D-04 is still open, and the combined row has grown again.** The M4 cycle row is still a single row edited in place — now "M4 drivers + RTEMS + TSN + netstack + lwIP" (`:27`, 374 checks) — folding **five** increments; no distinct increment-4 row was added and no explicit rationale for one row per milestone is recorded, which is exactly the open D-04 from the increment-2 RTEMS review. The counts themselves are correct (15+20+9+29+101+31+20+81+23+16+29 = 374 for the 11 suites of that cycle). Separate accuracy point (also raised by R1 as D-R1-04): the review-evidence table row reads `REJECTED, Major fixed (netstack bounds)` (`:321`) while the review log correctly says "closure pending independent delta" (`docs/reviews/README.md:33`) and the report's own §7 table says `open`. | `docs/vv/verification-results.md:27,321`; `docs/reviews/README.md:33`; `docs/reviews/2026-10-10-m4-rtems-delta-review.md:74` (D-04); `docs/reviews/2026-10-10-m4-gem-tsn-delta-review.md:173` | Add a distinct increment-4 cycle row (or record the explicit one-row-per-milestone rationale and close D-04), and align `:321` with the log ("Major fixed in code; delta re-review pending / R2-01..R2-03 open"). |
| D-R2-04 | Minor | `tests/unit/test_netloop.c`; R1-03 disposition ("FIXED (code): duplicate `bind` is rejected") | **The R1-03 disposition is overstated and carries no test evidence; the cross-socket case is still unfixed.** `netloop_bind` rejects only a **re-bind of the same socket** (`s->bound != 0` -> `FF_ALREADY_EXISTS`, `ff_net_loopback.c:85-87`). Two **different** sockets may still bind the **same port** and both `listen`; `netloop_connect` then selects the lowest-index listener (`:113-117`). Independently reproduced: `bind(a,7)=0`, `bind(b,7)=0` (cross-socket duplicate accepted), `listen(a)=0`, `listen(b)=0`, then `connect -> 0`, `accept(a)=3`, `accept(b)=-1` — i.e. the R1-03 sub-finding "two listeners can share a port (the first is always selected)" persists. None of the three R1-03 behaviours has a regression case in the suite. **This diverges from `FRICK-REVIEW-0054`, which recorded R1-03 `CLOSED`** on same-socket evidence; the divergence must be reconciled under N12. | `ff_net_loopback.c:79-91,113-117`; probe output (`bind(b,7)=0`, `listen(b)=0`, `accept(a)=3`, `accept(b)=-1`); `tests/unit/test_netloop.c:14-79` (no `bind`/`listen` error case); `2026-10-11-m4-netstack-delta-r1-review.md` §4.1 (R1-03 CLOSED) | Reject a `bind` to an already-bound port (scan the table for `bound != 0 && port == addr[0]`), or document the "port sharing is accepted; first listener wins" semantics explicitly in `hal_net.h`; add the three R1-03 regression cases. Record the R1-03 verdict consistently in both delta reports. |
| D-R2-05 | Comment | `docs/vv/verification-results.md` (R4-04) | **R4-04 is still open and now contradicts the same document's own cycle row.** "Remaining (M4)" (`:272-276`) still lists the **lwIP `FF_NetStack_t` adapter** and the **GEM TSN (`FF_TsnBackend_t`)**, i.e. two items the M4 row itself records as delivered (SRS-REQ-038 `test_gem_tsn` 16 checks; SRS-REQ-040 lwIP adapter), and also MMUART/GPIO, both delivered and Renode-verified (SRS-REQ-047/048). | `docs/vv/verification-results.md:27` vs `:272-276`; `CHANGELOG.md:100-116`; per-suite runs (gem_tsn 16, gpio 30, serial 30) | Rewrite the remaining list to the genuinely outstanding items (third-party adapters, CAN/SPI/I2C/QSPI/MMC/CFDP/service 23) and point the deferred PTP/CoreTSN work at M8. |
| D-R2-06 | Comment | several docs / `tests/CMakeLists.txt` (R4-05) | **R4-05 is still open**: `docs/requirements/SRS.md`, `docs/requirements/traceability.md`, `docs/requirements/traceability.yaml`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` and `tests/CMakeLists.txt` all still lack a trailing newline (verified with `tail -c1`). Cosmetic. | `tail -c1` on the six files (all non-empty) | Add the trailing newlines. |
| D-R2-07 | Comment | `docs/requirements/SRS.md:194`; `docs/development-plan.md` (R4-07) | R4-07(a) **open**: SRS-REQ-039 still says the reference stack "exercises the pluggable **TCP/IP** interface"; the contract is the stack-independent `FF_NetStack_t` and the loopback is a byte-stream endpoint, not TCP/IP. R4-07(b) appears **addressed**: `development-plan.md:150` (M4: generic fabric-device contract, no FPGA IP) vs `:171-176` (M8: gPTP + fabric CoreTSN) now describes the split, and `:292` records M8 `planned`. | `SRS.md:192-196`; `docs/development-plan.md:150,171-176,292` | Rephrase to "the stack-independent network-stack interface (`FF_NetStack_t`)". |
| D-R2-08 | Comment | `docs/vv/verification-results.md`; `docs/architecture/network-stack.md` (evidence-record consistency) | **R4-01 as written is closed, with a labelling residual.** The stale line was corrected from 1533/2357 to **1656/2521 (65.7%), functions 220/264** (`:270`) and now agrees with the same document's M4 cycle row (`:27`); the current README/`coverage.json` value **1880/2848 (66.0%)** is reproduced by `metrics.py` and is the later-cycle value. Residual: the M4 note does not label its figure as a milestone snapshot, so it reads as current. Unrelated but adjacent evidence-accuracy defect found while cross-checking counts: `docs/architecture/network-stack.md:49` states `test_lwip` **15** checks while the current source yields **19** (CHANGELOG `:115` and `verification-results.md:27` say 19). | `docs/vv/verification-results.md:27,270`; `python3 tools/scripts/metrics.py` and `build/coverage/coverage.json` (1880/2848, 256/301); `grep -c FF_TEST_ASSERT tests/unit/test_lwip.c` = 19; `docs/architecture/network-stack.md:48-49` | Label the M4 coverage figure as the milestone-cycle snapshot; correct `network-stack.md:49` to 19 (or record the `FF_USE_LWIP` build/version that yields 15). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No **Major** finding is open in this profile's delta.

## 4. NC verdicts (independent, at `e1def89`)

| NC | Profile | Verdict | Objective re-verification evidence |
| -- | ------- | ------- | ---------------------------------- |
| **R1-01** | R1 (V&V evidence criterion) | **CLOSED** | The narrowing cast is gone: `if ((sd < 0) \|\| (sd >= (FF_NetSock_t)FF_NETLOOP_MAX)) return NULL;` (`ff_net_loopback.c:48`), `FF_NetSock_t` is `int` (`hal_net.h:18`), and the index reaching `g_sock[]` (`:51,:54`) is provably in `[0,4)`. A regression case exists (`test_netloop.c:74-76`) and is **discriminating**: re-introducing the reviewed defect into a throwaway copy made the current suite crash — UBSan `index 65536 out of bounds for type 'FF_NetLoopSock_t [4]'` and ASan `SEGV ... in sock_of ff_net_loopback.c:54`, called from `test_netloop.c:74` — while the fixed code passes the same suite under ASan+UBSan (`checks=29 failures=0`). No sanitizer diagnostic for the fixed code; `ctest` netloop PASS. The check count was re-recorded consistently (29) in `CHANGELOG.md:110`, `tool-qualification.md:87` (T3, rev 0.2.0), `network-stack.md:48` and `verification-results.md:270`. |
| **R2-01** | R2 | **OPEN (partially closed)** | The critical item (out-of-range descriptor; "the suite would not detect R1-01") is covered and mutation-effective. The remaining requested categories (null/zero-length args, exhaustion, empty `accept`, full-ring partial `send`, wrap, slot reclaim, the new R1-03 branches) are still absent from the 29-check suite — see **D-R2-01**. |
| **R2-02** | R2 | **OPEN (partially addressed)** | The Renode emulated-target contract check exists and reproduces (`hal_net=PASS`), but the **host simulator** (`make run`) still does not instantiate the loopback and the SRS/CHANGELOG "host simulator"/"on the host" wording is unchanged — see **D-R2-02**. |
| **R2-03** | R2 | **OPEN** | No distinct increment-4 cycle row; the single M4 row was edited again (now five increments); no recorded rationale; `:321` wording can be read as closure — see **D-R2-03**. |
| **R2-04** | R2 (positive) | **CLOSED (no action) — claim confirmed** | The SRS-REQ-039 evidence is honest and now better traced: `method: Tst`, `lean: []` (`traceability.yaml:377`) with an explicit `lean_note` (`:378`) that no longer says "end-to-end"; `tests: ["tests/unit/test_netloop.c", "ci:renode"]` (`:376`) and the `ci:renode` CI marker now lists **SRS-REQ-039** and SSS-REQ-020 in both workflows (`.github/workflows/ci.yml:76`, `.gitea/workflows/ci.yml:79`); `ci:renode` is mapped to qualified tool T17, whose location list now includes `app_renode_test.c` (`tool-qualification.md:41,70`). `trace.py` PASS (94/54/26/14, 51 theorems, 12 CI claims). |
| **R4-01** | R4 (V&V evidence criterion) | **CLOSED** (residual Comment D-R2-08) | `verification-results.md:270` = 1656/2521 (65.7%), 220/264, consistent with the M4 cycle row `:27`; current value 1880/2848 reproduced by `metrics.py`. |
| **R4-02** | R4 | **CLOSED** | `docs/status/status.json:16` now contains `"M8": "planned"`, matching `development-plan.md:292` and `roadmap`. |
| **R4-03** | R4 | **CLOSED** (Comment) | The README inventory now carries the reference stack: `README.md:168` "Net stack (loopback) … `src/drivers/net/ff_net_loopback.c`" in the low-level driver table and `README.md:125` "Networking … lwIP adapter; loopback stack" in the HAL table; `driver-development.md:136-139` names the Cadence GEM, the GEM TSN backend and the loopback `FF_NetStack_t` under `src/drivers/`. Residual (non-blocking): the §1 extension-point table (`driver-development.md:23-28`) still says "three narrow interfaces" (PAL/Link/Board) while 13 HAL contract classes ship. |
| **R4-04** | R4 (V&V evidence criterion) | **OPEN** | See **D-R2-05** — "Remaining (M4)" still names delivered items. |
| **R4-05** | R4 | **OPEN** | See **D-R2-06** — trailing newlines still missing in six files. |
| **R4-06** | R4 | **CLOSED** | The review-log row exists (`docs/reviews/README.md:33`) and honestly flags "closure pending independent delta (R1-01/R1-03 fixed; R1-02 residual tracked)"; the report is committed (norm N11). |
| **R4-07** | R4 (V&V evidence criterion) | **OPEN (partially)** | (a) "TCP/IP interface" wording persists (`SRS.md:194`); (b) the M4/M8 fabric-TSN split is now stated — see **D-R2-07**. |
| R1-04 / R1-05 / R1-06 | R1 | *(out of profile; evidence spot-checked)* | `ff_net_loopback.h:10-13` documents partial `send` / non-blocking `recv` / non-thread-safety (R1-04, R1-05); `ff_net_loopback.c:10` has a single include, no `fsw_config.h` (R1-06). Consistent with `FRICK-REVIEW-0054`'s CLOSED verdicts. |
| R1-02 | R1 | *(out of profile; tracking verified)* | The limitation is real and independently reproduced (connect cli1, close, connect cli2 -> listener `pending` overwritten; `accept -> 3`; one relay orphaned; further `open -> -1`), and it is tracked at `docs/status/loop-audit.md:139` with owner and milestone (M4/M11). Note `FRICK-REVIEW-0054` D-R1-01 found the §8 claim of *also* tracking it in `development-plan.md` unsubstantiated; I confirm no such entry exists there. |
| R1-03 | R1 (V&V evidence criterion) | **still not fully fixed** | See **D-R2-04** — cross-socket duplicate `bind` accepted; no regression test. Diverges from `FRICK-REVIEW-0054` (CLOSED). |

## 5. Positive evidence reproduced in this session

- **The one Major of `FRICK-REVIEW-0032` is genuinely closed with a discriminating
  regression test** (mutation test above) — the strongest single result of this
  delta.
- Host gates green and current: `ctest` **14/14**, `test_netloop` **29/0**,
  `trace.py` **PASS** (94/54/26/14, 51 theorems, 12 CI claims), `metrics.py`
  reproduces the README panel exactly (1880/2848, 256/301) and `git status`
  stayed clean.
- **Renode/R18 functional evidence reproduces independently**: fresh cross-build
  (46/46), `FF_NetStackLoopback`/`netloop_send` in the target ELF
  (SHA-256 `d4e0799a…`), pinned Renode image -> `hal_net=PASS`,
  `RENODE_TEST: PASS`. The SRS-REQ-039 contract is really exercised on the
  emulated target; the `hal_net_ok` result is ANDed into the single marker
  (`app_renode_test.c:257,267`), so the gate faithfully reflects the NET check.
- Ring-buffer semantics are correct in the normal and wrap cases (probe:
  partial `send` 300 -> 256, full ring -> 0, wrap drains in order); slot reclaim
  works; `NULL`/zero-length arguments and hostile handles are uniformly rejected.
- Tool qualification is kept current for the changed testware: T3 rev 0.2.0
  records `test_netloop.c` with 29 checks (`tool-qualification.md:87`); T17 now
  covers `app_renode_test.c` (`:41`).
- Review record extended and mostly accurate: the review-evidence table now has
  the netstack row and the M11 NET R1/R2/R4 + delta rows (`:321,329-332`), and
  all referenced report paths resolve on disk (45/45 checked).

## 6. Conclusion

Graded conclusion for this profile's delta:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R2 (SW V&V, N9/N10/N12) | **APPROVED WITH COMMENTS** | No Major finding. The Major that caused the REJECTED verdict (R1-01) is **closed with objective, mutation-verified regression evidence**. R2-04 is confirmed; R4-01/R4-02/R4-03/R4-06 are closed from the V&V side. The remaining findings are Minor/Comments (D-R2-01, D-R2-02, D-R2-04 Minor; D-R2-03, D-R2-05..D-R2-08 Comment) and must be tracked to closure. |

**Can `FRICK-REVIEW-0032` be closed from the V&V side? No.**

- R2-01, R2-02 and R2-03 (this profile's own NCs) are **not closed**; R4-04,
  R4-05 and R4-07(a) are open; and the R1-03 disposition is **overstated** and
  diverges from `FRICK-REVIEW-0054` (D-R2-04). Per norm N12 a delta may be
  approved with comments, but `FRICK-REVIEW-0032` may be marked closed only when
  every NC of all profiles is resolved and the §7/§8 tables and the review log
  record the closure (norm N11). Recommended minimum before closure:
  (i) add the remaining R2-01 cases plus the three R1-03 cases; (ii) reword the
  SRS-REQ-039/CHANGELOG "host simulator" and "TCP/IP" claims (or wire the
  simulator); (iii) add the distinct increment-4 cycle row and the rationale;
  (iv) fix the R1-03 cross-socket `bind` or document the semantics; (v) correct
  "Remaining (M4)", add the trailing newlines and align `verification-results.md:321`
  with the review log.

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (SW V&V delta re-review, N9/N12) | independent agent (FRICK-REVIEW-0055) | 2026-10-11 | APPROVED WITH COMMENTS (R1-01 closed; R2-01/R2-02/R2-04 verdicts; D-R2-01/D-R2-02/D-R2-04 Minor; D-R2-03, D-R2-05..D-R2-08 Comment) |

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_net_loopback.c` (`sock_of`) | Narrowing cast defeated the descriptor guard (OOB read/corruption) | Fix the guard; add an out-of-range case; re-record | `ff_net_loopback.c:46-55`; `test_netloop.c:74-76`; **mutation test crashes / fixed code ASan-clean**; 29/0; ctest 14/14 | frick-fsw | **CLOSED** | `FRICK-REVIEW-0055` (this report) |
| R2-01 | Minor | `test_netloop.c`, SRS-REQ-039 | Coverage was happy-path + 3 errors; could not detect R1-01 | Add negative/boundary cases; re-record | Critical case added and mutation-verified; **7 remaining categories still untested** (D-R2-01); probe shows correct behaviour | frick-fsw | **OPEN (critical part closed)** | `FRICK-REVIEW-0055` |
| R2-02 | Minor | SRS-REQ-039 / `CHANGELOG.md` | "host simulator" claim not exercised | Wire the simulator/app or reword | Renode emulated-target check **exists and reproduces** (`hal_net=PASS`), but `build/native` contains no `FRICK_PLATFORM_MPFS`/`FF_ENABLE_RENODE_TEST`; SRS/CHANGELOG wording unchanged (D-R2-02) | frick-fsw | **OPEN** | `FRICK-REVIEW-0055` |
| R2-03 | Comment | `docs/vv/verification-results.md` | Combined M4 cycle row (recurrence of D-04) | Add increment row or record rationale | Row now "…+ netstack + lwIP" (374 checks); no increment-4 row; `:321` wording (D-R2-03) | frick-fsw | **OPEN** | `FRICK-REVIEW-0055` |
| R2-04 | Comment | SRS-REQ-039 / manifest | Claims honest about scope | Keep accurate | `traceability.yaml:371-378`; `ci:renode` marker lists SRS-REQ-039/SSS-REQ-020; T3/T17 current; `trace.py` PASS | frick-fsw | **CLOSED (no action)** | `FRICK-REVIEW-0055` |
| R4-01 | Minor | `verification-results.md` | Stale coverage figure | Update | `:270` = 1656/2521 (65.7%) = `:27`; current 1880/2848 reproduced by `metrics.py` (D-R2-08 labelling residual) | frick-fsw | **CLOSED** | `FRICK-REVIEW-0055` |
| R4-02 | Minor | `docs/status/status.json` | M8 absent | Add `"M8": "planned"` | `status.json:16` | frick-fsw | **CLOSED** | `FRICK-REVIEW-0055` |
| R4-03 | Minor | `README.md`, `driver-development.md` | Inventory omits the reference stack | Add rows | `README.md:125,168`; `driver-development.md:136-139` | frick-fsw | **CLOSED** | `FRICK-REVIEW-0055` |
| R4-04 | Comment | `verification-results.md:272-276` | "Remaining (M4)" lists delivered items | Update the list | Still names lwIP adapter + GEM TSN (delivered in `:27`) + MMUART/GPIO (D-R2-05) | frick-fsw | **OPEN** | `FRICK-REVIEW-0055` |
| R4-05 | Comment | several docs / `tests/CMakeLists.txt` | Missing trailing newline | Add newline | Six files still lack it (`tail -c1`) (D-R2-06) | frick-fsw | **OPEN** | `FRICK-REVIEW-0055` |
| R4-06 | Comment | `docs/reviews/README.md` | Review-log row missing | Append at commit | `docs/reviews/README.md:33` | frick-fsw | **CLOSED** | `FRICK-REVIEW-0055` |
| R4-07 | Comment | `SRS.md:194`, `development-plan.md` | "TCP/IP interface" wording; M4/M8 overlap | Rephrase; state the split | (a) `SRS.md:194` unchanged; (b) split stated at `development-plan.md:150,171-176,292` (D-R2-07) | frick-fsw | **OPEN (partially)** | `FRICK-REVIEW-0055` |
| R1-02 | Minor | `ff_net_loopback.c` (connect/close/listen) | Relay-socket leak / `pending` overwrite | Documented bounded limitation, tracked | Reproduced (probe); tracked at `loop-audit.md:139`; §8 `development-plan.md` claim unsubstantiated (`FRICK-REVIEW-0054` D-R1-01) | frick-fsw | **OPEN (tracked, not closed)** | `FRICK-REVIEW-0055` + `FRICK-REVIEW-0054` |
| R1-03 | Minor | `ff_net_loopback.c` (`open`/`bind`/`listen`) | `port` not reset; duplicate `bind`; stale port | Fix code | Same-socket `bind` -> `FF_ALREADY_EXISTS`, `listen` without `bind` -> `FF_INVALID_ARGUMENT`, `port` reset on `open`; **cross-socket duplicate `bind` still accepted and untested** (D-R2-04) | frick-fsw | **OPEN (partially fixed)** | `FRICK-REVIEW-0055` |
| D-R2-01 | Minor | `test_netloop.c` | Remaining branch set untested | Add cases; re-record counts | Probe evidence (behaviour correct) | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-02 | Minor | SRS-REQ-039 / `CHANGELOG.md` | "host simulator" wording vs native build | Reword or wire the host simulator | `grep -c` over `build/native/build.ninja` = 0; SRS/CHANGELOG unchanged | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-03 | Comment | `verification-results.md` | Cycle-row mutability + closure-facing wording | Add row/rationale; align wording | `:27,321`; `docs/reviews/README.md:33` | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-04 | Minor | `ff_net_loopback.c`; `test_netloop.c` | Cross-socket duplicate `bind` unfixed and untested; disposition overstated | Reject duplicate port bind or document; add tests; reconcile with `FRICK-REVIEW-0054` | Probe: `bind(b,7)=0`, `accept(a)=3`, `accept(b)=-1` | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-05 | Comment | `verification-results.md:272-276` | Stale "Remaining (M4)" | Update | `:27` vs `:272-276` | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-06 | Comment | six docs / `tests/CMakeLists.txt` | Missing trailing newline | Add newline | `tail -c1` | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-07 | Comment | `SRS.md:194`; `development-plan.md` | "TCP/IP" wording | Rephrase | `SRS.md:194` | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |
| D-R2-08 | Comment | `verification-results.md`; `network-stack.md:49` | Snapshot not labelled; `test_lwip` count 15 vs 19 | Label snapshot; correct the count | `grep -c FF_TEST_ASSERT tests/unit/test_lwip.c` = 19; `CHANGELOG.md:115` = 19 | frick-fsw | **OPEN** | new in `FRICK-REVIEW-0055` |

### Uncertainty and confidence

```
Product/configuration and decision supported: frick-fsw M4 increment 4 (reference loopback
  network stack, SRS-REQ-039) - V&V delta adjudication of FRICK-REVIEW-0032 at e1def89
Information item/revision/status: FRICK-REVIEW-0055 (independent R2 delta re-review), report only
Claim type: Fact (reproduced runs, quoted code/records, mutation result) +
  Inference (evidence-sufficiency of the remaining coverage, host-simulator claim) + Recommendation
Applicable baseline/tailoring: repo norms N1-N20 (docs/rules.md); ECSS-E-ST-40C, ECSS-Q-ST-80C
Sources and access level: full repository text and source (public, read directly); build, ctest,
  test_netloop, trace.py, metrics.py, ASan/UBSan builds, mutation build, behavioural probes and the
  RTEMS cross build + pinned Renode run all re-executed. Build artefacts were written only to the
  gitignored build/ (build/renode-r2) and /tmp/opencode; no tracked file was modified.
Independent cross-check and conflict reconciliation: code vs unit test vs mutation vs probe vs
  app check vs native build file vs traceability/CI marker vs qualification register vs cycle row;
  conflict with FRICK-REVIEW-0054 recorded (R1-03).
Confidence: High (reproduction of the fix, the mutation, the Renode run and the records; the
  uncovered-branch inventory is directly countable). Medium where "intended use" of the SRS-REQ-039
  wording is a scope decision for the authority (host simulator vs emulated target).
Assumptions, TBD/TBC, detection limits: on-target/silicon PHY/link and real timing remain GAP-A5
  and were not exercised; the Renode run is an emulated SoC, not hardware. The mutation test used a
  reconstructed defect (faithful to the reviewed text and reproducing the documented ASan/UBSan
  signature), not the author's original working-tree revision, which no longer exists in git.
Consequences if wrong: an NC set could be recorded closed while a claimed environment (host
  simulator) is not exercised and a defended behaviour (duplicate bind) is still unfixed; a
  milestone row could be mutated again without a recorded rationale.
Required technical owner/reviewer/approver: frick-fsw author (fixes); R1/R2/R4 delta re-review per
  N12; the host-simulator-vs-Renode wording decision is for the authority.
Recheck trigger/date: on the next delta (norm N12) or at the closure bookkeeping of FRICK-REVIEW-0032.
```

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | independent reviewer agent (R2, FRICK-REVIEW-0055) | Independent V&V delta re-review of FRICK-REVIEW-0032: R1-01 closed (mutation-verified), R2-01/R2-02/R2-03/R4-04/R4-05/R4-07 open, R1-03 partially fixed |

---

The author must append the FRICK-REVIEW-0055 row to `docs/reviews/README.md` and
update the §7/§8 tables of `FRICK-REVIEW-0032` in the same commit (norm N11);
this review was scoped to write this report only.
