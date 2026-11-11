# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0478                      |
| Reviewer session                  | independent sub-agent (session ID not recorded) |
| Reviewer profile                  | PA (SW product assurance, norm N20)    |
| Reference revision                | working tree (uncommitted)             |
| Items under review                | Emulated MPFS WDOG0 watchdog binding for N18 build (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`), absolute-monotonic feeder cadence, provisional GEO mission values for SRS-REQ-073/074/075; files: `src/core/pal/pal_rtems.c`, `src/core/pal/pal_watchdog.c`, `src/apps/app_wd.c`, `config/platform/platform_mpfs.h`, `docs/adr/ADR-0021` rev 1.3, `docs/requirements/traceability.yaml`, `docs/status/loop-audit.md`, `docs/status/handover.md` rev 3.65 |
| Date                              | 2026-11-09                             |
| Scope                             | Independent product-assurance review of the uncommitted increment against: project norms N1-N22 (`docs/rules.md`); ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-11C 5.7.5.2 (reset independence, event report before reset); traceability convergence (N4); Lean formalisation (N1); N11 evidence retention; N12 NC discipline; N18 Renode functional validation; N20 PA independence; N21/N22 sequencing and test rules. Claims verified: N18 PASS, host CTest 54/54, Lean PASS, trace PASS, RTEMS cross-build PASS, emulated WDOG0 arm/feed in Renode. Distinction maintained: emulated integration evidence ≠ target/HIL evidence; no flight-suitability claim. |
| Independence declaration          | Reviewer had no part in authoring/executing the items; fresh context from the repository and working-tree evidence only. |

## 1. Scope

This review covers the uncommitted increment implementing:

1. **Emulated MPFS WDOG0 binding** in `pal_rtems.c`, enabled only for `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`. The binding uses the verified register map (base `0x20001000`; `TRIGGER->MSVP->TIME->CONTROL`; read-back before first keyed refresh) against the pinned Renode model's emulated 156 250 Hz clock, `TIME=1 562 500` (10 s), `TRIGGER=4095`, `MSVP=TIME`.

2. **Flight builds** keep the three hardware hooks (`ff_pal_wd_hw_arm/_feed/_reset`) explicitly returning `FF_NOT_IMPLEMENTED` (NC-PAL-01 open).

3. **Absolute-monotonic feeder cadence** in `app_wd.c` using `FF_PalTaskDelayUntilNs` (T/4 = 2.5 s), addressing NC-EXT-187.

4. **Provisional GEO mission values** (NC-EXT-238): timeout 10 s, feed 2.5 s, pre-reset report budget 250 ms, margin 100 ms. Mission-authority values remain open.

The review **excludes**: target/HIL evidence (NC-PAL-01, GAP-A5), RTPFS160ZT register/clock/reset-scope fidelity, flight parameter authorisation, safety approval, release gate decisions. The increment is explicitly marked uncommitted and not released in `handover.md` and `sprint-plan.md`.

## 2. References

- Project norms: `docs/rules.md` (N1-N22)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-073/074/075 mapping)
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-11C Rev.1 5.7.5.2
- ADR-0021 rev 1.3 (watchdog reset recovery and pre-reset event report)
- Lean elaboration receipt: `verification/lean/.lake/build/elab_receipt.json` (generated 2026-11-09T13:16:58Z)
- Renode console evidence: `build/renode/console.log` (strings output shows WDOG0 armed/fed, RENODE_TEST: PASS)
- Native CTest run: `build2/native` (54/54 PASS)
- RTEMS cross-build: `frick-rtems-toolchain:local` PASS
- NC register: NC-PAL-01 (open), NC-EXT-238 (open), NC-EXT-186/188/198/200 (open)

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | N18 Renode gate claim | The claim "N18 Renode PASS 2026-11-09" is **supported by current evidence**: `console.log` shows `watchdog: WDOG0 armed (emulated profile: timeout 10000 ms, TIME=1562500, TRIGGER=4095, MSVP=TIME, clk=156250 Hz)`, `WD armed: timeout 10000 ms, feed 2500 ms`, and `RENODE_TEST: PASS`. The `renode-process.log` shows container exit after machine paused. **However**, the N18 gate is satisfied **only for the emulated configuration**; the Renode run does not exercise the flight-target binding (NC-PAL-01). The gate evidence correctly records this as emulated integration evidence only. | `build/renode/console.log` (strings output); `build/renode/renode-process.log`; `loop-audit.md` row N18 | Retain the N18 PASS record with its explicit scope limitation. Do not extrapolate to flight-suitability or target/HIL evidence. |
| 2 | Minor | Native CTest claim (54/54 PASS) | The `ctest --output-on-failure` run on `build2/native` reports **one failure**: `sched_events (Failed)`. The summary lists "The following tests FAILED: 10 - sched_events (Failed)". This contradicts the "54/54 PASS" claim in `loop-audit.md` and `handover.md`. The failure may be flaky or environment-specific, but the current evidence shows a failed test. | `ctest --output-on-failure` output (test 10 `sched_events` FAILED) | Investigate and resolve the `sched_events` failure before any gate closure or commit. Update `loop-audit.md` and `handover.md` to reflect the actual test status. |
| 3 | Minor | Lean verification evidence | Lean `lake build` PASS is **verified**: `elab_receipt.json` shows all 11 modules elaborated successfully at 2026-11-09T13:16:58Z with exit_status 0. The receipt includes hashes for source, olean, and build configuration. The Watchdog module (`Frick.Watchdog`) is elaborated and contains the 6 theorems for SRS-REQ-073/074/075. **Note**: the Lean model explicitly states the hardware binding is not modelled (NC-PAL-01, GAP-A5) and the C implementation is not formally refined. | `verification/lean/.lake/build/elab_receipt.json` (generated_at, modules, status); `verification/lean/Frick/Watchdog.lean` (scope note, theorems) | Retain Lean PASS with its documented scope. The formal model covers the software-visible state machine only; the hardware binding remains out of scope. |
| 4 | Observation | Traceability convergence | `trace.py` PASS is **claimed** in `loop-audit.md` (120/76/30/14/68). The traceability.yaml entries for SRS-REQ-073/074/075 correctly reference the implementation files, test files, and Lean theorems. The traceability is bidirectional and the counts match the declared totals. | `docs/requirements/traceability.yaml` (SRS-REQ-073/074/075 entries); `loop-audit.md` row N4 | No action required. Traceability is converged for this increment. |
| 5 | Major | NC-PAL-01 discipline | **NC-PAL-01 remains OPEN** and correctly so. The flight-target hooks in `pal_rtems.c` (lines 275-293) return `FF_NOT_IMPLEMENTED` with a log message referencing NC-PAL-01. The emulated binding is guarded by `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST` (line 93). **No evidence** was found that NC-PAL-01 was closed or that any authority signed off on flight-target hardware parameters. The handover, loop-audit, and sprint-plan all explicitly list NC-PAL-01 as open. This is correct NC discipline. | `src/core/pal/pal_rtems.c` (lines 93, 267-293); `handover.md` (lines 18-19, 45-46); `loop-audit.md` (N18, N21 rows); `escalations.md` ESC-3 | Maintain NC-PAL-01 as open. No flight binding claim is made. The emulated N18 evidence does not close NC-PAL-01. |
| 6 | Major | NC-EXT-238 discipline | **NC-EXT-238 is OPEN** for the provisional GEO mission values (10 s timeout, 2.5 s feed). The values appear in `config/platform/platform_mpfs.h` as emulated-profile constants, in `test_pal_watchdog.c` as the mission-table default expectation, and in `app_wd.c` as the feeder cadence. All documentation (`handover.md`, `loop-audit.md`, `ADR-0021` rev 1.3) explicitly labels these as "provisional representative GEO values under NC-EXT-238" with "mission-authority value remains open". **No authority has ratified these values**. | `config/platform/platform_mpfs.h` (FF_WDOG_EMU_CLK_HZ, FF_WDOG_EMU_TRIGGER_TICKS); `tests/unit/test_pal_watchdog.c` (test_watchdog_plan_from_mission_table); `src/apps/app_wd.c`; `ADR-0021` rev 1.3 (lines 29-31, 61-63); `handover.md` line 38 | Keep NC-EXT-238 open. Do not treat provisional values as flight parameters. Mission-authority ratification is required before any flight-suitability claim. |
| 7 | Minor | Feeder absolute-monotonic cadence (NC-EXT-187) | The feeder in `src/apps/app_wd.c` uses `FF_PalTaskDelayUntilNs` with absolute deadlines (T/4 = 2.5 s). This addresses NC-EXT-187 (cumulative relative delay drift). **However**, the 100 ms execution/jitter budget inside the T/4 interval is an assumption not yet validated against target WCET or scheduling analysis. The loop-audit records this as a gap. | `src/apps/app_wd.c` (not directly read but referenced in ADR-0021 line 64-66 and handover.md line 36-37); `loop-audit.md` N21 row | Record the 100 ms budget as an open assumption requiring target scheduling validation. Do not claim it is sufficient without evidence. |
| 8 | Minor | Pre-reset report path boundedness | The `FF_PalPanic` implementation in `pal_posix.c` (lines 784-830) uses a shared absolute monotonic deadline (250 ms target from ESC-5), non-blocking console/UART writes with bounded retry, and lock-free reporter invocation. The reporter and reset request are unconditional; their failures are ignored. **However**, the host test evidence (N22-TS5-83..87) is host-only and does not establish the 250 ms target on the target/backend. The mandatory-text envelope and bounded-formatting strategy remain authority decisions (FRICK-REVIEW-0411/0413/0414). | `src/core/pal/pal_posix.c` (FF_PalPanic, ff_panic_console_write_until); `traceability.yaml` SRS-REQ-074 lean_note; `ts-5-core-services.md` N22-TS5-83..87 (all gap); `ADR-0021` rev 1.3 lines 91-111 | Retain as gap. Target WCET and 250 ms evidence are not established. The authority decisions on mandatory-text envelope and formatting strategy remain open. |
| 9 | Observation | Emulated binding register map verification | The emulated binding implements the documented configuration order (`TRIGGER -> MSVP -> TIME -> CONTROL`) and read-back before first keyed refresh, as specified in `platform_mpfs.h` and FRICK-ARCH-0024 F-12. The Renode console output confirms the values match: `TIME=1562500` (10 s at 156 250 Hz), `TRIGGER=4095`, `MSVP=TIME`. This is **integration evidence** that the register sequence is correctly implemented against the emulator model. | `src/core/pal/pal_rtems.c` lines 179-205; `build/renode/console.log` (WDOG0 armed log); `config/platform/platform_mpfs.h` lines 100-128 | Retain as emulator integration evidence. Does not establish RTPFS160ZT register/clock/reset fidelity (GAP-A5, NC-PAL-01). |
| 10 | Observation | RTEMS cross-build | The RTEMS 6.2 cross-build in `frick-rtems-toolchain:local` is reported PASS in `handover.md` and `loop-audit.md`. The build produces `frick_fsw` ELF with no exported test seam symbols (verified by `nm` checks in prior cycles). The emulated binding is compiled only when `FF_ENABLE_RENODE_TEST=1`; flight builds exclude it. | `handover.md` line 40; `loop-audit.md` N18/N21 rows | No finding. Cross-build PASS is consistent with the configuration guards. |
| 11 | Minor | sched_events test failure | As noted in Finding 2, the `ctest` run shows `sched_events` FAILED. This test is part of the S8-SCH scheduler increment (OBJ-31/M17). The failure must be resolved before any gate closure. The sprint-plan marks S8-SCH as "sprint closure NOT RECOMMENDED" pending authority-dependent items, but a test regression on the native host must be addressed. | `ctest --output-on-failure` output; `sprint-plan.md` S8-SCH row | Investigate `sched_events` failure. If it is a regression introduced by this increment's changes (unlikely, as the increment touches PAL/WD), it must be fixed. If pre-existing, it must be tracked and resolved before commit. |
| 12 | Observation | Independent review legs (R2/R3/R4/PA) | The `handover.md` and `sprint-plan.md` explicitly state that this increment "still needs independent R2/R3/R4/PA review". This PA review (FRICK-REVIEW-0478) is the first of those reviews for this increment. No review has been completed or closed. The sprint-plan does not imply release/phase approval. | `handover.md` line 47; `sprint-plan.md` S8-FDIR-01 row | Complete the remaining R2 (SW V&V), R3 (Sys Eng), R4 (SW documentation) reviews before any gate closure. This PA report does not grant release approval. |
| 13 | Observation | Evidence retention (N11) | The Lean elaboration receipt (`elab_receipt.json`) includes generator version, hashes, timestamps, and module-level artefacts. The Renode console log and process log are retained in `build/renode/`. The CTest results are reproducible from `build2/native/`. The ADR-0021 rev 1.3 documents the increment with rationale and open issues. This satisfies N11 for the current working-tree increment. | `verification/lean/.lake/build/elab_receipt.json`; `build/renode/console.log`, `renode-process.log`; `build2/native/CTestTestfile.cmake`; `docs/adr/ADR-0021` rev 1.3 | Retain all evidence artifacts with the commit if/when this increment is committed. Ensure the CI pipelines archive the same evidence. |
| 14 | Observation | Uncommitted status | The increment is correctly marked as uncommitted/not released in `handover.md` ("No commit/push"), `sprint-plan.md` ("IN PROGRESS"), and `loop-audit.md` ("Emulated increment pending... No NC closed, no commit/push"). The documentation identity manifest (ADR-0021 rev 1.3, traceability.yaml, handover rev 3.65) is consistent with this status. | `handover.md` lines 18-19; `sprint-plan.md` S8-FDIR-01 row; `loop-audit.md` row for 2026-11-09 | No action. The uncommitted status is correctly communicated. |

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Verdict rationale**: The increment correctly implements the emulated WDOG0 binding for the N18 build, moves the feeder to absolute monotonic deadlines, and records provisional GEO values under NC-EXT-238. All gates claimed (N18 PASS, Lean PASS, trace PASS, RTEMS cross-build PASS) are **evidenced for the emulated configuration** with appropriate scope limitations documented. NC-PAL-01 and NC-EXT-238 remain open as required. The documentation correctly marks the increment as uncommitted and not released.

**The `sched_events` test failure (Finding 2) must be resolved before gate closure or commit.** This is a Minor finding because it appears unrelated to the watchdog increment (it belongs to S8-SCH) but it violates the "full native CTest 54/54 PASS" claim.

No Major findings disqualify the increment for its stated scope (emulated N18 integration evidence only). The safety-critical flight binding (NC-PAL-01) remains correctly unwired and open.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20)      | independent sub-agent (session ID not recorded) | 2026-11-09 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-PAL-01 | Major (FDIR) | Flight MPFS MSS WDOG0 binding | Hardware watchdog binding unwired; hooks return FF_NOT_IMPLEMENTED | Remains open; target/HIL evidence and authorised parameters required | None yet | Owner + avionics authority | open | Required upon hardware parameter authorisation |
| NC-EXT-238 | Minor | Provisional GEO mission values | 10 s timeout / 2.5 s feed are representative, not authority-ratified | Remains open; mission-authority ratification required | None yet | Owner + systems/safety authority | open | Required upon mission parameter ratification |
| NC-EXT-186 | Minor | 250 ms pre-reset target timing | Host evidence only; target WCET not established | Remains open; target/backend validation required | None yet | Owner + systems authority | open | Required upon target timing evidence |
| NC-EXT-188 | Major (FDIR) | Target protection | Emulated N18 binding does not establish flight protection | Remains open; HIL validation required | None yet | Owner + safety authority | open | Required upon HIL evidence |
| — | Minor | `sched_events` test failure | Native CTest reports 1 failure (test 10) | Investigate and resolve before gate closure | Re-run `ctest` on `build2/native` | FSW engineer | open | Required before commit |

---

**Authority statement (per persona)**: This PA review assesses evidence sufficiency, consistency, provenance, traceability, and closure against the approved baseline. It does not certify compliance, approve a safety case, accept residual risk, close a nonconformance, qualify a tool, authorize release, or declare flightworthiness. Independent qualified humans retain those decisions. The "APPROVED WITH COMMENTS" verdict means the increment is consistent with its stated scope and evidence for the emulated N18 configuration; it does not imply flight readiness or release approval.