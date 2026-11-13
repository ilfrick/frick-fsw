# Independent Avionics Systems Review — R1/R3 Profile

**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD `93be62b`
**Reviewer:** Senior Aeronautics and Space Avionics Systems Engineer (independent review, R1/R3 profile)
**Review date:** 2026-11-12
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) / RTEMS 6.2 / ECSS class B
**Change under review:** test-only increment — **no production (`src/`) change**
  1. `tests/unit/test_sched_tm_chain.c` (new)
  2. `tests/CMakeLists.txt` (test registration)
  3. `docs/vv/test-spec/ts-5-core-services.md` (TS-5 spec: `N22-TS5-47` `partial`→`covered`, rev 1.17)
  4. `docs/vv/verification-results.md` (`sched_tm_chain` PASS entry)
**Authority:** Advisory only. This review does not approve airworthiness, human-rating, qualification, release, or NC closure. Commit/push approval rests with the `aeronautics-space-fsw-engineer` (norm N7) and authorized humans; open NCs require authorized-human disposition (N9/N20).

---

## Executive summary

The increment is test-only documentation plus verification evidence. It introduces **no system-level risk**, touches **no target/hardware item** (NC-PAL-01 / GAP-A5 unaffected), and impacts **no GNC / safety / FDIR / operations interface**. The TS-5 test spec and `docs/vv/verification-results.md` are consistent on the new test evidence (`test_sched_tm_chain.c`, 50 checks, 0 failures). The `N22-TS5-47` `partial`→`covered` status change is appropriate under the spec's own §1.2 convention and correctly does **not** close NC-EXT-19 (which remains open pending independent review). Two **Minor** documentation-consistency items warrant reconciliation but do not block commit/push.

**Determination: no systems finding blocks commit/push.**

---

## Point-by-point review (the six review questions)

| # | Review question | Conclusion |
|---|-----------------|------------|
| 1 | System-level risk introduced? | **No.** Test-only; `git diff --stat -- src/` is empty; all gates (build/CTest/Lean/trace/format/license) pass. The test exercises production paths read-only and modifies none. |
| 2 | NC-PAL-01 / GAP-A5 / target/hardware affected? | **No.** Host-executable only (native simulator). Uses `FF_TEST_HOOKS` seams (`FF_PalTaskDelayMsTestEnable/Step/Wait`, `FF_PalMonotonicTestSet`, `FF_SchedTestRequestExit`, `FF_SbTestLock`) that are absent from flight/RTEMS builds. No RTPFS160ZT / MPFS / MSS binding, no Renode N18 target config, no RTEMS cross-build impact. Pre-existing `NC-EXT-239` / `NC-PAL-01` note (verification-results.md:417-421) is unrelated to this change. |
| 3 | GNC, safety, FDIR, operations interfaces impacted? | **No.** No production code change. The test observes scheduler EVS events (heartbeat / missed-slot / dispatch-failure) and the PUS service 5 TM downlink but does not alter their content, cadence, authority, or downlink behavior; health-monitoring path is exercised read-only in the harness. |
| 4 | TS-5 spec consistent with verification-results? | **Mostly consistent.** Both cite `test_sched_tm_chain.c`, 50 checks, 0 failures (ts-5:177; verification-results.md:409). Two Minor documentation-consistency items (see SYS-R1/R3-04). |
| 5 | `N22-TS5-47` `partial`→`covered` appropriate? | **Yes**, with caveats. Appropriate under §1.2 (`covered` = requirement-grounded oracle + executable in named environment + PASS tracked; anchors `SRS-REQ-071`/`SSS-REQ-030`, ts-5:177). Correctly does **not** claim NC closure ("NC-EXT-19 remains open pending independent review; no NC closed", ts-5:440). One Minor status-token conflict (see SYS-R1/R3-05). |
| 6 | Systems findings blocking commit/push? | **None.** See SYS-R1/R3-06. |

---

## Structured findings

### SYS-R1/R3-01 — No system-level risk introduced (test-only)
**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** `git diff HEAD --stat -- src/` empty (no `src/` change); `tests/CMakeLists.txt:29-32` and `:108-110` register only a new host test target; `docs/vv/test-spec/ts-5-core-services.md:7` revision 1.17 and `:440` history entry 1.17 describe added test evidence only.  
**Detail:** `test_sched_tm_chain.c` exercises production paths (`FF_AppSchedulerMain` → `FF_EvsSend` → production listener → `FF_PusEmitEvent` → PUS S5 → `FF_AppTmOutputInit`/`ff_to_sink` production sink → `FF_LinkSend` → link loopback; lines 46-56, 63-92, 127-142, 157-181 incl. `FF_AppTmOutputInit()` at line 175; assertions at lines 255-256, 281-283, 313-314) but modifies none. All `FF_TEST_HOOKS` seams are host-only.  
**Recommendation:** None required. The test adds meaningful end-to-end health-monitoring-path evidence without production risk.

### SYS-R1/R3-02 — NC-PAL-01, GAP-A5, and target/hardware items unaffected
**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** `tests/unit/test_sched_tm_chain.c` is host-native (no `pal_rtems`, no Renode/N18 target config); `tests/CMakeLists.txt:30-31` compile definitions are `FF_TEST_HOOKS` only; `docs/vv/verification-results.md:417-421` records pre-existing `NC-EXT-239`/`NC-PAL-01` as unrelated to this cycle.  
**Detail:** NC-PAL-01 (flight MPFS watchdog binding + target/HIL), GAP-A5 (target evidence), NC-EXT-19/90 (scheduler mission timing/policy) remain open and are **not** closed or worsened by this change. The change is explicitly not GAP-A5 / NC-PAL-01 / OBCP-engine related (per change context).  
**Recommendation:** None required. Record that NC-PAL-01/GAP-A5/NC-EXT-19/90 disposition remains with authorized humans; this change neither opens nor closes them.

### SYS-R1/R3-03 — GNC, safety, FDIR, operations interfaces not impacted
**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** No `src/` modification (confirmed empty diff). `test_sched_tm_chain.c:175` installs the production TM sink only within the test harness; no change to scheduler event IDs (`0x0603` missed-slot / `0x0604` heartbeat / `0x0605` dispatch-failure), severities, cadence, or PUS service 5 framing.  
**Detail:** The test observes rather than commands. No GNC law, safety function, FDIR monitor, or operations/ground interface is altered, added, or removed. Event-identity semantics are separately covered by `test_pus_event_identity.c` (TS-1 `N22-TS1-58`, ts-5:177).  
**Recommendation:** None required.

### SYS-R1/R3-04 — Cross-disciplinary consistency: TS-5 spec vs verification-results
**Classification:** Minor  
**Blocks commit/push:** No  
**Evidence:**
- Consistent core: `test_sched_tm_chain.c` cited with "50 checks, 0 failures" in `docs/vv/test-spec/ts-5-core-services.md:177` and `docs/vv/verification-results.md:409`.
- Item (a) — `docs/vv/verification-results.md:320` states "NC-EXT-19 remains open (scheduler EVS end-to-end path not verified)". With `test_sched_tm_chain.c` now evidencing the **host** end-to-end path, the parenthetical "path not verified" is stale for host scope. NC-EXT-19 itself remains open pending independent review (per change context and ts-5:177/440), so clarify rather than delete.
- Item (b) — `docs/vv/verification-results.md:389-415` (S8-SCH cycle section dated 2026-11-10) now lists `sched_tm_chain` PASS (`:409`, test added 2026-11-12) while reporting CTest count "55" (`:406`). Reconcile section date/CTest count if this table is intended as current-tree status.
**Detail:** No functional or gate inconsistency; only documentation currency. `ts-5-core-services.md:129` (§1.2) defines only `covered`/`gap` statuses; execution status is tracked in `verification-results.md` per §1.2.  
**Recommendation:** Update `:320` parenthetical to reflect host end-to-end evidence delivered with NC-EXT-19 still open pending independent review; reconcile the S8-SCH section date and CTest count (`:406`) with the current tree. Non-blocking.

### SYS-R1/R3-05 — `N22-TS5-47` `partial`→`covered` appropriateness; status-token conflict
**Classification:** Minor  
**Blocks commit/push:** No  
**Evidence:**
- Appropriateness: `ts-5-core-services.md:129` (§1.2) — `covered` = oracle grounded in a controlled requirement/architecture/interface clause and executable in the named environment; `N22-TS5-47` anchors `SRS-REQ-071`/`SSS-REQ-030` (`:177`), test added and executable in SIL, PASS tracked in `verification-results.md:409`. Row text and history (`:440`) correctly state "NC-EXT-19 remains open pending independent review; no NC closed" — so `covered` does **not** imply NC closure (consistent with §1.2).
- Conflict: `N22-TS5-47` row (`:177`) inline marker reads `**covered**` but the Status column (col 10) reads `**mapped**`; `mapped` is not a §1.2-defined status (only `covered`/`gap`). History entry `:440` says "moves from `partial` to `covered`."  
**Detail:** The evidence fully supports `covered` per §1.2. The `mapped` token is leftover/ambiguous and contradicts the inline marker and history. Aligning Status column col 10 to `covered` is consistent with NC-EXT-19 remaining open (since `covered` ≠ NC closure).  
**Recommendation:** Align `ts-5-core-services.md:177` Status column col 10 to `**covered**` (author/PA discretion). Leave the explicit "NC-EXT-19 remains open" note intact. Non-blocking.

### SYS-R1/R3-06 — No systems finding blocks commit/push
**Classification:** Minor (determination)  
**Blocks commit/push:** No — explicit determination that nothing blocks.  
**Evidence:** All gates pass (build, CTest, Lean, trace, format, license per change context; `verification-results.md:409` `sched_tm_chain` checks=50 failures=0; `ts-5-core-services.md:411` trace PASS 120/76/30/14/68; Lean `lake build` PASS). Open NCs (`NC-EXT-19`, `NC-EXT-19/90`, `NC-PAL-01`, `GAP-A5`, `NC-EXT-237`) are pre-existing, not closed or worsened by this change; they require authorized-human disposition per N7/N9/N20 but are not blockers for this test-only documentation + evidence commit.  
**Detail:** Commit/push remains subject to norm N7 (designated FSW engineer after all gates/norms and full sprint scope) and any separately applicable safety/security/regulatory/contractual gates; this avionics review raises no systems objection.  
**Recommendation:** Proceed to commit/push under the project's normal N7/N9/N20 gate ownership, noting NC-EXT-19/NC-PAL-01/GAP-A5 remain open for authorized disposition.

---

## Conclusion

From an avionics/system-level perspective, the `sched_tm_chain` test increment is sound: test-only, no production risk, no target/hardware impact, no GNC/safety/FDIR/operations impact, and internally consistent verification documentation. The `N22-TS5-47` `partial`→`covered` change is appropriate under the project's own §1.2 convention and correctly preserves NC-EXT-19 as open. Two Minor documentation-consistency items (`verification-results.md:320` parenthetical; `:406` CTest count/section date; `ts-5-core-services.md:177` status-column token) should be reconciled but do not block commit/push.

**No Major findings. No systems finding blocks commit/push.**
