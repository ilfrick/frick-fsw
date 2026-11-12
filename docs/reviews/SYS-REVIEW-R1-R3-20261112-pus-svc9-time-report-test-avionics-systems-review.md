# Independent Avionics Systems Review — R1/R3 Profile

**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD `29cba32`  
**Reviewer:** Senior Aeronautics and Space Avionics Systems Engineer (independent review, R1/R3 profile)  
**Review date:** 2026-11-12  
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) / RTEMS 6.2 / ECSS class B  
**Change under review:** test-only increment — **no production (`src/`) change**
  1. `tests/unit/test_pus.c` — added `test_service9_report_time` test verifying PUS service 9 subservice 2 (TC[9,2] report time → TM[9,2]) end-to-end through `FF_PusIngest`; added helper `stream_has_time_report`; registered the test in `FF_TEST_MAIN`.
**Authority:** Advisory only. This review does not approve airworthiness, human-rating, qualification, release, or NC closure. Commit/push approval rests with the `aeronautics-space-fsw-engineer` (norm N7) and authorized humans; open NCs require authorized-human disposition (N9/N20).

---

## Executive summary

The increment is test-only: it adds end-to-end coverage for PUS service 9 subservice 2 (TC[9,2] → TM[9,2]) on the host simulator, exercises production paths read-only, and modifies no `src/` file, no GNC/safety/FDIR/operations interface, and no target/hardware binding. It introduces **no system-level risk**. NC-PAL-01, GAP-A5, NC-EXT-19/90, NC-EXT-211 and N22-TS5-47 are untouched and remain OPEN. The test is consistent with existing `test_pus.c` patterns, and adding coverage for `svc9_report_time` is appropriate for host/SIL scope (production handler `svc9_report_time` at `src/pus/pus_services.c:590-602` is registered for `{9u, 2u}` at `pus_services.c:709`). All gates pass (build, CTest, Lean, trace, format, license per change context).

**Determination: no systems finding blocks commit/push.**

---

## Point-by-point review (the six review questions)

| # | Review question | Conclusion |
|---|-----------------|------------|
| 1 | System-level risk introduced? | **No.** `git diff HEAD --stat -- src/` is empty; the change touches only `tests/unit/test_pus.c`. The test exercises production paths read-only (TC encode → `FF_PusIngest` → service-9 dispatch → TM emission into the harness sink) and modifies none. All gates pass per change context. |
| 2 | NC-PAL-01 / GAP-A5 / target/hardware affected? | **No.** Host-native executable only. Uses `FF_TEST_HOOKS` PAL seams (`FF_PalTimeTestFreeze`/`Unfreeze`, `FF_TimeSetUtcNs`) that are absent from flight/RTEMS builds. No `pal_rtems`, no Renode/N18 target config, no RTPFS160ZT/MPFS/MSS binding, no RTEMS cross-build impact. NC-PAL-01 / GAP-A5 / NC-EXT-19/90 remain OPEN and are not closed or worsened. |
| 3 | GNC, safety, FDIR, operations interfaces impacted? | **No.** No production code change. The test observes `TC[9,2] → TM[9,2]` through `FF_PusIngest`/`FF_PusSetTmSink`; it does not alter service-9 framing, time authority, cadence, downlink content, or any GNC/safety/FDIR/operations interface. |
| 4 | Cross-disciplinary consistency with `test_pus.c` patterns? | **Yes.** Same harness preamble (`FF_PalInit`/`EvsInit`/`TimeInit`/`PusInit`, `FF_PusSetBackend(&FF_PusBackendOpen)`, `FF_PusSetTmSink(tm_sink)`, `g_rx_count = 0u`), same TM-stream parsing idiom as `stream_has_function_report` (lines 61-89), same frozen-time pattern as `test_service9_time`/`test_hk_periodic_emission`/`hk_wrap_test_setup`, and registered in `FF_TEST_MAIN` at line 1231. |
| 5 | `svc9_report_time` (TC[9,2] → TM[9,2]) test coverage appropriate? | **Yes**, for host/SIL scope. Production handler `svc9_report_time` (`pus_services.c:590-602`) reads CUC via `FF_TimeGetCuc`, encodes coarse[4 BE] + fine[2 BE], and emits `TM[9,2]`; dispatcher registration `{9u, 2u, svc9_report_time}` at `pus_services.c:709`. The test covers nominal report at two deterministic instants (42 s, 100 s) via `stream_has_time_report`, plus refusal of unknown subservice 99 → `FF_NOT_FOUND` and no time-report TM. Caveat: coverage is host/SIL only; deterministic time-frozen instants are synthetic test values, not mission parameters; target/RTEMS behavior is covered separately by N18, not by this test. |
| 6 | Systems findings blocking commit/push? | **None.** See SYS-R1/R3-06. |

---

## Structured findings

### SYS-R1/R3-01 — No system-level risk introduced (test-only increment)

**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** `git diff HEAD --stat -- src/` empty (no `src/` change); `tests/unit/test_pus.c:506-556` (`test_service9_report_time`) and `:91-120` (`stream_has_time_report`) are test harness code only; registration at `:1231`. All gates pass per change context (build, CTest, Lean, trace, format, license).  
**Detail:** `test_service9_report_time` feeds encoded `TC[9,2]` frames into `FF_PusIngest` and inspects the captured TM stream; no production function, interface, parameter, schedule, or hardware binding is modified. The test's observable effect is confined to `g_rx[]`/`g_rx_count` within the test binary.  
**Recommendation:** None required. The test adds meaningful end-to-end PUS service-9 evidence without production risk.

### SYS-R1/R3-02 — NC-PAL-01, GAP-A5, and target/hardware items unaffected

**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** `tests/unit/test_pus.c` is host-native; it calls `FF_PalTimeTestFreeze`/`FF_PalTimeTestUnfreeze` (lines 521, 555) and `FF_TimeSetUtcNs` (lines 522, 541) — PAL test seams present only under `FF_TEST_HOOKS` and absent from flight/RTEMS builds. No `pal_rtems`, no Renode/N18 target configuration, no RTPFS160ZT/MPFS/MSS binding, no RTEMS cross-build input touched. NC-EXT-19/90, NC-PAL-01, GAP-A5, NC-EXT-211, N22-TS5-47 remain OPEN per change context and are neither referenced nor modified by the diff.  
**Detail:** NC-PAL-01 (flight MPFS watchdog binding + target/HIL) and GAP-A5 (target evidence) are out of scope for a host-only test addition; the change is explicitly not GAP-A5 / NC-PAL-01 / OBCP-engine related (per change context).  
**Recommendation:** None required. Record that NC-PAL-01/GAP-A5/NC-EXT-19/90/NC-EXT-211/N22-TS5-47 disposition remains with authorized humans; this change neither opens nor closes them.

### SYS-R1/R3-03 — GNC, safety, FDIR, operations interfaces not impacted

**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** No `src/` modification (confirmed empty diff). `test_service9_report_time` uses `ack_flags = 0u` (`:529`) to isolate the handler and installs the production TM sink only within the test harness (`FF_PusSetTmSink(tm_sink)`, e.g. `:194` in `test_ping_roundtrip`; same idiom here).  
**Detail:** The test observes rather than commands: no GNC law, safety function, FDIR monitor, command authority, or operations/ground interface is altered, added, or removed. Service-9 TC/TM framing (ECSS: 1 = Time set, 2 = Report time; `pus_services.c:707-709`) and the PUS service-9 handler are unchanged.  
**Recommendation:** None required.

### SYS-R1/R3-04 — Cross-disciplinary consistency: test consistent with existing `test_pus.c` patterns

**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:**
- Harness preamble matches `test_service9_time` (`:460-504`) and `test_hk_periodic_emission` (`:648-807`): `FF_PalInit`/`FF_EvsInit`/`FF_TimeInit`/`FF_PusInit`, `FF_PusSetBackend(&FF_PusBackendOpen)`, `FF_PusSetTmSink(tm_sink)`, `g_rx_count = 0u` (see `:518-535`).
- TM-stream parsing idiom matches `stream_has_function_report` (`:61-89`): big-endian PDL from `[pos+4]/[pos+5]`, `step = 6 + pdl + 1`, overflow guard `(step < 7) || ((pos + step) <= pos)`, payload read at TM offset 19 (`:103-108`, consistent with `:73-76` where TM secondary header is 13 octets after the 6-octet primary header).
- Frozen deterministic time via `FF_PalTimeTestFreeze`/`FF_TimeSetUtcNs` matches `test_hk_periodic_emission` (`:666-667`) and `hk_wrap_test_setup` (`:853`); `FF_PalTimeTestUnfreeze()` at `:555` restores the seam (matching `:806`, `:946`).
- Registration in `FF_TEST_MAIN` at `:1231`, adjacent to `test_service9_time`.
**Detail:** No deviation from established test structure, naming (`test_service9_*`), comment style (`/* ---- S9 ... ----`), or assertion idiom (`FF_TEST_ASSERT` + `stream_has_*`).  
**Recommendation:** None required.

### SYS-R1/R3-05 — Adding coverage for `svc9_report_time` (TC[9,2] → TM[9,2]) is appropriate

**Classification:** Minor (confirmatory)  
**Blocks commit/push:** No  
**Evidence:** Production handler `svc9_report_time` at `src/pus/pus_services.c:590-602` reads current CUC via `FF_TimeGetCuc`, writes coarse[4 BE] + fine[2 BE] (`wr_u32`/`:599`, `wr_u16`/`:600`), and emits `TM[9,2]` via `FF_PusSendTm(FF_PUS_SERVICE_TIME, 2u, ...)` (`:601`); dispatcher entry `{9u, 2u, svc9_report_time}` at `pus_services.c:709`. `stream_has_time_report` (`test_pus.c:93-120`) validates the 6-octet CUC payload at TM payload offset 19. The test exercises nominal report at coarse=42 s/fine=0 (`:538`) and coarse=100 s/fine=0 (`:546`), plus refusal of unknown subservice 99 → `FF_NOT_FOUND` and absence of time-report TM (`:549-554`).  
**Detail:** Coverage is end-to-end through `FF_PusIngest` and appropriate for ECSS PUS-C service 9 subservice 2 (report time) on the host/SIL platform. It is **not** target/RTEMS qualification evidence and does not close GAP-A5 or NC-PAL-01; deterministic time-frozen instants (42 s, 100 s) are synthetic test values, not mission time parameters.  
**Recommendation:** None required for this cycle. Target-side service-9 behavior remains within N18/Renode scope, independently of this test.

### SYS-R1/R3-06 — No systems finding blocks commit/push

**Classification:** Minor (determination)  
**Blocks commit/push:** No — explicit determination that nothing blocks.  
**Evidence:** All gates pass per change context (build, CTest, Lean, trace, format, license); `test_service9_report_time` is registered and executable in the host CTest binary. Open NCs (NC-EXT-19/90, NC-PAL-01, GAP-A5, NC-EXT-211, N22-TS5-47) are pre-existing, untouched by the diff, and remain OPEN; they require authorized-human disposition per N7/N9/N20 but are not blockers for this test-only change.  
**Detail:** Commit/push remains subject to norm N7 (designated FSW engineer after all gates/norms and full sprint scope) and any separately applicable safety/security/regulatory/contractual gates; this avionics review raises no systems objection.  
**Recommendation:** Proceed to commit/push under the project's normal N7/N9/N20 gate ownership, noting NC-PAL-01/GAP-A5/NC-EXT-19/90/NC-EXT-211/N22-TS5-47 remain open for authorized disposition.

---

## Conclusion

From an avionics/system-level perspective, the `test_service9_report_time` increment is sound: test-only, no production risk, no target/hardware impact (NC-PAL-01/GAP-A5 unaffected), no GNC/safety/FDIR/operations interface impact, and internally consistent with existing `test_pus.c` patterns. Adding host/SIL end-to-end coverage for PUS service 9 subservice 2 (TC[9,2] → TM[9,2]) is appropriate and is anchored to the production handler at `src/pus/pus_services.c:590-602` (`{9u, 2u}` registered at `:709`). No Major findings; all six findings are Minor/confirmatory or a non-blocking determination.

**No Major findings. No systems finding blocks commit/push.**
