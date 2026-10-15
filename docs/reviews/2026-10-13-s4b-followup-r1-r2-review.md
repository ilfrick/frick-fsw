# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0167                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1 + R2 combined** — R1 (Senior FSW engineer) **and** R2 (Senior SW V&V engineer) |
| Reference revision                | `frick-fsw` working tree on `main`, baseline commit `57974d2` (uncommitted "S4b follow-up" change; 12 modified files) |
| Items under review                | `tests/unit/test_link_ip.c` (new `test_pus_ip_roundtrip`; 20 test funcs / 77 checks) and `tests/unit/test_link_tsn.c` (new `test_pus_tsn_roundtrip`; 15 test funcs / 135 checks); the documentation deltas: `docs/vv/verification-results.md` (corrected S4b rows + new follow-up row), `docs/requirements/traceability.yaml` (SRS-REQ-059 typo, SRS-REQ-060/061 `lean_note`), `docs/requirements/traceability.md` (rev 0.38, SRS-REQ-059 mirror row), `docs/quality/class-a-gaps.md` (rev 0.11, GAP-A5 IP/TSN residual), `docs/planning/sprint-plan.md` (rev 0.17, S4b exit wording + NC-S4b-01..05), `docs/status/loop-audit.md` (rev 0.25), `docs/status/status.json`, `docs/vv/tool-qualification.md` (T3 entry 0.3.25), `CHANGELOG.md`, `README.md`; requirements SRS-REQ-059/060/061 in `docs/requirements/SRS.md` |
| Date                              | 2026-10-13                             |
| Scope                             | Independent combined R1+R2 (norm N9) review of the uncommitted S4b follow-up: does the actual implementation match the documented scope, is the new host PUS-C TM/TC round-trip evidence genuine (method `Dem` of SRS-REQ-060/061), and is the corrected/added documentation accurate and current? Against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA / Class B), ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 (space packets), the `FF_Link_t` vtable (`src/transport/link/link.h`), the link backends (`src/transport/link/link_ip.c`, `src/transport/link/link_tsn.c`), `FF_PusIngest`/S17 ping (`src/pus/pus.c`, `src/pus/pus_services.c`), the reference loopback stack (`src/drivers/net/ff_net_loopback.h`), and the project norms N1–N20 (`docs/rules.md`). |
| Independence declaration          | The reviewer had no part in authoring, executing for delivery, or previously reviewing this change. This review ran in a fresh context; the two modified test files, the full diff and the referenced source files were read first-hand, and every gate listed in §1 was re-run by the reviewer in this session. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

**Point-in-time counts (audit D1).** The counts recorded in this report (e.g.
`test_link_ip` 77 checks) are the values the reviewer measured at review time,
before the `-Wall -Wextra -Werror` corrective delta added the IP send-leg and
PUS-TM-sink assertions; the current values are `test_link_ip` 20 funcs / **79**
checks and `test_time` 99 checks. A review report is an immutable record of the
state it examined and is deliberately not retro-edited; the current counts are
carried by `docs/vv/verification-results.md` (rev 0.45+) and by the delta
re-review `FRICK-REVIEW-0167-D1`.

## 1. Scope

**In scope (verified first-hand):**

- `tests/unit/test_link_ip.c` — the new `test_pus_ip_roundtrip` (lines 725–808): a PUS-C S17 ping TC is decoded from the IP `FF_Link_t` backend, ingested by `FF_PusIngest`, the TM reply is captured by the installed `FF_PusSetTmSink` sink, re-injected into the mock net-stack RX buffer and received again via `FF_LinkRecv`.
- `tests/unit/test_link_tsn.c` — the new `test_pus_tsn_roundtrip` (lines 444–509) and the new mock-MAC echo (`g_mock_echo`, lines 31/64–66/135): the same PUS-C exchange over the TSN backend with the mock MAC looped back (`mock_mac_transmit` → `tsn_eth_rx_cb` → RX ring → `FF_LinkRecv`).
- The documentation deltas that claim this evidence and the corrected stale counts (see Items under review), cross-checked against the actual measured state.
- Requirements SRS-REQ-059/060/061 (`docs/requirements/SRS.md` lines 353–367) and their traceability rows (`docs/requirements/traceability.yaml` lines 552–579).

**Out of scope / excluded (not re-reviewed here):**

- The S4b source code itself (`link_seg.c`, `link_ip.c`, `link_tsn.c`, `link.h`) — unchanged by this follow-up and already reviewed/closed in FRICK-REVIEW-0165/0166 and their delta re-reviews.
- On-target Renode/QEMU validation (N18) — the change is host-only code plus docs; the follow-up row correctly records `SKIP (host-only code; no RTEMS image change — norm N18 proxy)`.
- R3/R4/PA reviews of the follow-up (norms N14/N17/N20) — separate reviewer profiles, not part of this combined R1+R2 report.
- The lwIP adapter and the real GEM/MSS driver implementations — separate deliverables (GAP-A5).

**Gates re-run by the reviewer in this session (independent evidence):**

| Gate | Command | Result |
| ---- | ------- | ------ |
| Build | `cmake --build build/native` | PASS |
| ctest | `ctest` in `build/native` | **27/27 PASS** |
| `test_link_ip` | `./build/native/tests/test_link_ip` | **checks=77 failures=0** (20 test funcs) |
| `test_link_tsn` | `./build/native/tests/test_link_tsn` | **checks=135 failures=0** (15 test funcs) |
| Baseline counts | `57974d2` worktree (`/tmp/opencode/frick-baseline`) | `test_link_ip` **58** checks, `test_link_tsn` **122** checks → the corrected S4b-row numbers (19 funcs/58 checks; 14 funcs/122 checks) are accurate |
| Traceability (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (114 reqs / 71 srs / 29 sss / 14 gaps / 51 theorems) |
| Metrics (N10/T13) | `python3 tools/scripts/metrics.py --check` | `metrics panel: current` |
| clang-format | `clang-format --dry-run --Werror` on both modified tests | both **clean** |
| ASan/UBSan | `build/review-asan/tests/test_link_{ip,tsn}` | clean, **77 / 135** checks, no memory errors |
| Native smoke | `./build/native/frick_fsw` | `frick-fsw: 6 applications registered`, TM→link output |
| GSW self-check | `python3 tools/gsw/ff_gsw.py --ping` | PASS (TC frame + CRC self-check) |
| `-Wall -Wextra -Werror` | `gcc -std=gnu99 -Wall -Wextra -Werror -c tests/unit/test_link_tsn.c …` | **FAILS** at lines 327/336 (`-Werror=sign-compare`, pre-existing `test_tsn_ring_full_drop`; baseline fails at 317/326) — see R2-F02 |

Not claimed: `make renode` (N18) and `make verify-lean` (N1) were **not** run — Renode and the Lean toolchain are unavailable in this host-only review window (the follow-up rows correctly mark them SKIP/proxy and reuse the unchanged `lake build 21/21`).

**Review focus questions (raised for this review), answered:**

1. *Are the new round-trips genuine, or tautological?* — **Genuine.** Both drive a real PUS-C S17 ping TC built by `FF_CcsdsEncodeTc` through the real link recv path into the real `FF_PusIngest`, and the real service-17/subservice-2 TM reply is captured and re-read through the link. They are not self-fulfilling mocks of the unit under test. **Caveat:** the IP test does not close the loop on the send leg (R1-F01).
2. *Does the TSN mock-MAC echo affect other tests?* — **No.** `g_mock_echo` is guarded (only `mock_mac_transmit` consults it), reset to `false` in `reset_mocks()`, and the new test is last in `FF_TEST_MAIN`; the 135 checks include all pre-existing TSN tests unchanged.
3. *Missing negative cases (link full, timeout, wrong EtherType, partial send)?* — Backend negatives exist and are unchanged (`test_tsn_ring_full_drop`, `test_tsn_recv_wrong_etype_dropped`, `test_tsn_short_frame_dropped`, `test_tsn_oversized_payload_dropped`, `test_seg_read_error_propagation`, `test_ip_tcp_partial_write`). The **new Dem** itself has only the happy path (R2-F05).
4. *Memory safety / arithmetic / MISRA?* — ASan/UBSan clean; no new arithmetic or indexing hazards in the added test code; no MISRA-relevant construct introduced. The only strict-compile issue is pre-existing (R2-F02).
5. *Documented scope vs implementation?* — One scope mismatch (undocumented `README.md` edit, R2-F01) and two documentation-accuracy mismatches (R2-F03, R2-F04).
6. *Are the func/check counts independent?* — Yes: 20 funcs/77 checks and 15 funcs/135 checks measured; baseline 19/58 and 14/122 → the corrected S4b-row counts are accurate.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest and mirror: `docs/requirements/traceability.yaml` (SRS-REQ-059/060/061), `docs/requirements/traceability.md` (rev 0.38)
- Requirements: `docs/requirements/SRS.md` (rev 1.23; lines 353–367)
- V&V evidence: `docs/vv/verification-results.md` (S4b rows), `docs/vv/tool-qualification.md` (T3, entry 0.3.25)
- Link contract/backends: `src/transport/link/link.h`, `src/transport/link/link_ip.c`, `src/transport/link/link_tsn.c`
- Reference loopback stack: `src/drivers/net/ff_net_loopback.h`
- PUS-C: `src/pus/pus.c`, `src/pus/pus_services.c` (`FF_PusIngest`, S17 ping → TM service 17 subservice 2)
- Planning/status: `docs/planning/sprint-plan.md` (rev 0.17), `docs/status/loop-audit.md` (rev 0.25), `docs/status/status.json`, `CHANGELOG.md`, `docs/quality/class-a-gaps.md` (rev 0.11)
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Prior review for format/severity reference: `docs/reviews/2026-10-13-s4b-r1-r2-review.md`

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence (file:line) | Recommendation |
|---|----------|-------------------|---------|----------------------|----------------|
| **R1-F01** | **Minor** | SRS-REQ-060 — `test_pus_ip_roundtrip` evidence | **The IP "round trip" is not closed on the send leg.** The sink (`pus_ip_tm_sink`) copies the TM packet it is *given* into `g_pus_reply` independently of `FF_LinkSend`, and the test then re-injects `g_pus_reply` into `g_mock_net_rx` and calls `FF_LinkRecv`. It never asserts what `FF_LinkSend` actually wrote to the net stack (`g_mock_net_tx` / `g_mock_net_tx_len` are left unread). The only send-side assertion is `g_pus_tx_count >= 2u`, which counts successful sink invocations, not the transmitted bytes. A defect in `ip_send`'s byte copy would not be detected by this test. | `tests/unit/test_link_ip.c:730–740` (sink stores the argument), `:794` (`g_pus_tx_count >= 2u`), `:798–804` (re-inject `g_pus_reply`, no `g_mock_net_tx` check) | Assert the sent bytes (`g_mock_net_tx_len`/`memcmp` on the TM the sink transmits) or run the exchange through `FF_NetStackLoopback` so the send leg feeds the recv leg (a genuinely closed loop). The existing `test_ip_udp_send_recv_roundtrip` already asserts `g_mock_net_tx` (`test_link_ip.c:534–535`), so the backend send leg is covered elsewhere — only the *Dem* is incomplete. |
| **R1-F02** | Comment | V&V — test isolation | `test_pus_ip_roundtrip` installs `FF_PusSetTmSink(pus_ip_tm_sink)` and does not reset it (`FF_PusSetTmSink(NULL)`), unlike `test_pus_tsn_roundtrip` (`test_link_tsn.c:509`). Harmless today (the test is last in `FF_TEST_MAIN`), but fragile if test order changes or a test is appended. | `tests/unit/test_link_ip.c:792` (install), no reset before `FF_TEST_MAIN` (`:812–819`) | Add `FF_PusSetTmSink(NULL)` at the end of the test. |
| **R1-F03** | Comment | SRS-REQ-061 — TSN echo / re-entrancy | The new mock-MAC echo executes `tsn_eth_rx_cb` from inside `mock_mac_transmit`, which is called by `tsn_link_send`. This is safe **only** because `tsn_link_send` does not hold `g_lock` while calling `transmit` and `tsn_eth_rx_cb` takes a non-blocking try-lock (`FF_PalMutexLock(g_lock, 0u)`). If either property changes (blocking lock in send, or a blocking lock in the callback), the echoed path self-deadlocks. | `tests/unit/test_link_tsn.c:64–66` (echo); `src/transport/link/link_tsn.c:179–218` (send does not hold `g_lock`), `:60–63` (callback try-lock) | Document the invariant in the test comment and/or `link_tsn.h`; ideally assert it (e.g., a test-local flag proving the lock is not held during `transmit`). |
| **R1-F04** | Comment | SRS-REQ-061 — TSN round-trip byte fidelity | The TSN round-trip asserts only the TM secondary header (`tm_rx[7]`/`tm_rx[8]`) and never byte equality. This is caused by design: `tsn_link_send` pads frames to 60 bytes and `tsn_link_recv` returns payload+padding, so the TM byte length is not preserved across the TSN round trip. Acceptable for Ethernet min-frame padding, but the padding semantics should be explicit. | `tests/unit/test_link_tsn.c:497–508` (service/subservice only); `src/transport/link/link_tsn.c:211–216` (pad to 60), `:242–252` (recv returns `payload_len`) | State the padding/RX-length semantics in `link_tsn.h`; optionally assert the padding bytes and the exact received length. |
| **R2-F01** | **Minor** | N8/N16/N19 — documented scope vs actual | **`README.md` was modified but is not in the documented change scope.** The metrics panel changes (`5552/7779 → 5618/7877`, `678/698 → 682/702`) although the S4b follow-up only added test functions — adding tests does not change measured `src/` coverage. The delta (+66 covered / +98 total lines / +4 functions) is therefore **environment drift**, and none of the change records (`CHANGELOG.md`, `loop-audit.md` 0.25, `sprint-plan.md` 0.17, `status.json`, `verification-results.md`) mention the README edit. (`metrics.py --check` reports "current", so the panel matches the local `coverage.json`; the discrepancy is against the committed baseline panel.) | `git diff README.md` (coverage rows); `git status` shows `M README.md`; documented scope in `CHANGELOG.md` follow-up entry / `docs/planning/sprint-plan.md` rev 0.17 (no README) | Either record the README metrics refresh explicitly in the follow-up change note (as the baseline commit `57974d2` "regenerate stale README metrics panel" did), or revert `README.md`. Coverage panels must not silently change in a tests-only change. |
| **R2-F02** | **Minor** | N11 — evidence claim accuracy | **The documented "`-Wall -Wextra -Werror` clean (host)" claim is not reproducible for `tests/unit/test_link_tsn.c`.** `gcc -std=gnu99 -Wall -Wextra -Werror -c tests/unit/test_link_tsn.c …` fails with `-Werror=sign-compare` at lines **327** and **336** (`test_tsn_ring_full_drop`, `for (int i = 0; i < FF_LINK_TSN_RX_SLOTS; i++)`; the baseline fails at 317/326 — pre-existing, not introduced here). The project build does not enforce this claim on tests either: `-Wall -Wextra` is applied only to the lwIP glue (`CMakeLists.txt:187,205`) and `-Werror` only to `frick_fsw_core` (`CMakeLists.txt:248–250`), not to the test targets (`tests/CMakeLists.txt`). The **new** test code is clean. | `docs/vv/verification-results.md` follow-up row (`clang-format + -Wall -Wextra -Werror clean (host)`); `CHANGELOG.md` "all PASS"; compile output at `tests/unit/test_link_tsn.c:327,336`; `CMakeLists.txt:187,205,248–250` | Qualify the evidence ("authored source clean; `test_link_tsn.c` retains pre-existing sign-compare warnings at 327/336") or fix the two `int` loop counters to an unsigned type so the claim becomes true. |
| **R2-F03** | **Minor** | N8 — documentation accuracy | **The docs overstate the environment.** The follow-up row and `CHANGELOG.md` describe the IP round-trip as running "on the reference loopback stack", but `test_pus_ip_roundtrip` uses the bespoke `g_mock_net` `FF_NetStack_t` fake; `FF_NetStackLoopback` (`src/drivers/net/ff_net_loopback.h:25`) is not referenced by the test. | `CHANGELOG.md` follow-up entry ("on the reference loopback stack"); `docs/vv/verification-results.md` follow-up row; `tests/unit/test_link_ip.c:256–268` (`g_mock_net`), `:759` (`FF_LinkIpDefaultConfig(&g_mock_net)`); `src/drivers/net/ff_net_loopback.h:25` | Correct the wording to "mock `FF_NetStack_t` (`g_mock_net`)" or route the test through `FF_NetStackLoopback`. |
| **R2-F04** | **Minor** | N10 — tool/test-script qualification | **The N10 disposition is not explicit.** T3 entry 0.3.25 records changed test scripts (`tests/unit/test_link_ip.c`, `tests/unit/test_link_tsn.c`) yet concludes "no V&V tool changed". The T3 *runner* is unchanged, but the changed test scripts are the V&V artifacts N10 governs; the record should state the qualification/re-qualification basis explicitly rather than a blanket "no tool changed". | `docs/vv/tool-qualification.md` entry 0.3.25 | State explicitly that T3 (host unit-test runner) is unchanged and the changed test scripts are (re-)qualified by this review plus the gate evidence, or re-qualify them under T3. |
| **R2-F05** | Comment | V&V — negative coverage of the new Dem | The two new tests exercise only the happy path (a valid S17 ping TC; a successful TM sink send). No negative case is added at the Dem level (malformed/short TC, unknown service, TM-sink `FF_LinkSend` failure, ring-full/timeout during the Dem, receive-buffer too small). Backend-level negatives exist and are unchanged, so this is not a backend gap — only the Dem lacks an error path. | `tests/unit/test_link_ip.c:743–808`, `tests/unit/test_link_tsn.c:449–509` | Add one or two negative Dem cases (e.g. a malformed TC rejected by `FF_PusIngest`, or an RX buffer too small on the TM leg). |
| **R2-F06** | Comment | N16 — per-document revision history | `SRS.md` (rev 1.23) was not updated by the follow-up. Its latest history row (1.22) records `test_link_ip (19) + test_link_tsn (14)` (now 20/15) and does not record that the `Dem` method of SRS-REQ-060/061 is now satisfied. The requirement text is unchanged, so this is a history-currency observation, not a requirement defect. | `docs/requirements/SRS.md` (rev 1.23; history rows 1.22–1.23) | Add an SRS history row recording the follow-up (Dem evidence for SRS-REQ-060/061, counts 20/15). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier. *(None.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified. *(R1-F01, R2-F01, R2-F02, R2-F03, R2-F04.)*
- **Comment**: observation/recommendation, no derogation. *(R1-F02, R1-F03, R1-F04, R2-F05, R2-F06.)*

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

The S4b follow-up **achieves its stated objective**: it supplies the missing host `pus_ip`/`pus_tsn` demonstration evidence for the `Dem` method of SRS-REQ-060/061 and corrects the stale counts/typo/mirror-row/status-list defects recorded as NC-S4b-01..05. The two new tests are **genuine** end-to-end PUS-C exchanges (not tautologies): a real S17 ping TC is encoded by `FF_CcsdsEncodeTc`, received through the real link backend, ingested by the real `FF_PusIngest`, and the real service-17/subservice-2 TM reply is captured and re-read through the link. All gates re-run by the reviewer pass (ctest 27/27; `test_link_ip` 20 funcs/77 checks; `test_link_tsn` 15 funcs/135 checks; trace 114/71/29/14/51; `metrics.py --check` current; clang-format clean; ASan/UBSan clean; native smoke 6 apps; GSW self-check). The corrected S4b-row counts (19 funcs/58 checks; 14 funcs/122 checks) are independently confirmed against baseline `57974d2`.

No Major finding was raised, so the increment is **APPROVED WITH COMMENTS**. Five Minor findings must be corrected or justified before the change is closed under norm N12: R1-F01 (IP Dem send leg not asserted), R2-F01 (undocumented `README.md` metrics edit / environment drift), R2-F02 (`-Wall -Wextra -Werror` claim not reproducible for `test_link_tsn.c`), R2-F03 ("reference loopback stack" wording vs `g_mock_net`), R2-F04 (N10 tool-qualification disposition). The six Comments are recorded for the author's consideration. The on-target Renode link-level round-trip remains **GAP-A5** (register-only GEM, NC-S6-01 precedent) and is correctly excluded from this host-only evidence.

## 5. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R1 (Senior FSW engineer) | Independent agent (norm N9, fresh context) | 2026-10-13 | **APPROVED WITH COMMENTS** — 1 Minor (R1-F01) + 3 Comments (R1-F02–F04); round-trips genuine |
| R2 (Senior SW V&V engineer) | Independent agent (norm N9, fresh context) | 2026-10-13 | **APPROVED WITH COMMENTS** — 4 Minor (R2-F01–F04) + 2 Comments (R2-F05–F06); gate evidence independently reproduced |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R1-F01 | Minor | SRS-REQ-060 | IP Dem send leg not asserted; TM is taken from the sink argument and re-injected, `g_mock_net_tx` never checked; `g_pus_tx_count >= 2u` counts calls, not bytes | Assert the transmitted bytes (`g_mock_net_tx_len`/`memcmp`) or route the test through `FF_NetStackLoopback` for a closed loop | Updated `test_pus_ip_roundtrip` + `test_link_ip` green | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R1-F02 | Comment | V&V | `test_pus_ip_roundtrip` leaves the TM sink installed (no `FF_PusSetTmSink(NULL)`) | Add the reset (mirror `test_pus_tsn_roundtrip`) | Test diff | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R1-F03 | Comment | SRS-REQ-061 | Mock-MAC echo re-enters `tsn_eth_rx_cb` from `transmit`; safe only while `tsn_link_send` holds no lock and the callback uses a try-lock | Document the invariant in the test/`link_tsn.h` | Comment in test/header | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R1-F04 | Comment | SRS-REQ-061 | TSN round-trip asserts only service/subservice; 60-byte padding means TM length/bytes are not preserved | State the padding/RX-length semantics in `link_tsn.h` | Header note | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R2-F01 | Minor | N8/N16/N19 | `README.md` metrics panel changed (5552/7779→5618/7877; 678/698→682/702) but not in the documented scope; delta not attributable to a tests-only change | Record the metrics refresh in the change note or revert `README.md` | `git diff README.md` + updated change record | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R2-F02 | Minor | N11 | "`-Wall -Wextra -Werror` clean" not reproducible for `test_link_tsn.c` (sign-compare at 327/336; tests are not compiled with these flags) | Qualify the claim or fix the `int` loop counters | Recompile log + updated evidence row | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R2-F03 | Minor | N8 | Docs say the IP round-trip runs "on the reference loopback stack"; the test uses `g_mock_net`, not `FF_NetStackLoopback` | Correct the wording or use `FF_NetStackLoopback` | `CHANGELOG.md`/`verification-results.md` + test diff | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R2-F04 | Minor | N10 | T3 entry 0.3.25 records changed test scripts but says "no V&V tool changed"; disposition not explicit | State the T3 qualification/re-qualification basis | `tool-qualification.md` diff | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R2-F05 | Comment | V&V | New Dem has no negative/error case (malformed TC, sink send failure) | Add one or two Dem negative cases | Test diff | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |
| R2-F06 | Comment | N16 | `SRS.md` history not updated for the follow-up (latest row 1.22 records 19/14) | Add an SRS history row | `SRS.md` diff | frick-fsw authoring agent | **closed** (authoring correction; see §6.1) | pending delta re-review |

### 6.1 Closure record (authoring agent, 2026-10-13)

Every finding above is corrected in the S4b follow-up review-loop increment
(base `57974d2` + working tree); the full pipeline was re-run (norm N15).

| NC | Corrective action (file) | Re-verification |
| -- | ------------------------ | --------------- |
| R1-F01 | `tests/unit/test_link_ip.c` — the sink records `g_pus_reply_off` (= `g_mock_net_tx_len` before `FF_LinkSend`) and the test asserts the reply lies inside `g_mock_net_tx` and `memcmp`s byte-for-byte against it (the send leg, not just the sink call) | `test_link_ip` 20 funcs / **79** checks (was 77), 0 failures; ctest 27/27 |
| R1-F02 | `FF_PusSetTmSink(NULL)` at the end of `test_pus_ip_roundtrip` | `test_link_ip` re-run; later suites unaffected |
| R1-F03 | Echo invariant documented in the `mock_mac_transmit` comment (no lock held in `tsn_link_send`; `tsn_eth_rx_cb` try-lock) | source read + `test_link_tsn` re-run |
| R1-F04 | Padding/RX-length semantics documented in `src/transport/link/link_tsn.h` (60-byte zero-pad; payload delivered including padding) | header diff; comment-only, no behaviour change |
| R2-F01 | `README.md` metrics refresh recorded in `CHANGELOG.md`, `verification-results.md` (0.44) and `status.json`; the test sources *are* instrumented (`coverage.py` aggregates them) | `metrics.py --check` = current |
| R2-F02 | Both `int` loop counters fixed to `unsigned`; the `-Wall -Wextra -Werror` gate is now applied to every test target in `tests/CMakeLists.txt`, which required four pre-existing test defects to be fixed (`test_link_tsn.c`, `test_spacewire.c`, `test_time.c`, `test_gptp.c`) | every `tests/unit/*.c` compiles clean under `gcc -std=gnu99 -Wall -Wextra -Werror -fsyntax-only`; `make build` + `make test` green |
| R2-F03 | "reference loopback stack" corrected to the mock `FF_NetStack_t` in `CHANGELOG.md`, `verification-results.md` (S4b rows) and the test comment; `traceability.yaml` SRS-REQ-060 now states which stack is verified where | diff + `make trace` PASS |
| R2-F04 | `tool-qualification.md` 0.3.25 rewritten as a T3 re-qualification record; 0.3.26 added for this delta | register diff |
| R2-F05 | Disposition: **accepted** — the `Dem` exercises the reference path by design; malformed-input and error paths of the same backends are already covered by the `Tst` negative cases (short/oversized/error tests in `test_link_ip`/`test_link_tsn`) | existing negative tests, re-run green |
| R2-F06 | `SRS.md` history advanced (`1.24`); `1.22` annotated as superseded | SRS diff |

---

Committed and pushed with the reviewed change (norm N11). The reviewer wrote exactly one file (this report); no tracked source, test or documentation file was modified by this review. Gate commands regenerate only git-ignored `build/` artifacts (`build/native`, `build/review-asan`).
