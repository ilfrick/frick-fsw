# Review Report: FRICK-REVIEW-0476

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0476                      |
| Reviewer session                  | independent sub-agent (session ID not recorded) |
| Reviewer profile                  | R3 (Systems Engineering / N14)         |
| Reference revision                | frick-fsw HEAD (2026-11-09 increment: emulated MPFS WDOG0 binding for N18, absolute-monotonic feeder cadence, provisional GEO values under NC-EXT-238) |
| Items under review                | SRS-REQ-073/074/075, traceability.yaml mappings, `docs/architecture/mpfs-mss-watchdog-integration-spec.md` rev 1.6, `docs/planning/sprint-plan.md` NC rows (NC-PAL-01, NC-EXT-186, NC-EXT-187, NC-EXT-188, NC-EXT-198, NC-EXT-200, NC-EXT-238), implementation in `src/core/pal/pal_rtems.c`, `src/apps/app_wd.c`, `config/platform/platform_mpfs.h`, `config/tables/fsw_tables.c`, `tests/unit/test_pal_watchdog.c`, ADR-0021 rev 1.3 |
| Date                              | 2026-11-09                             |
| Scope                             | Independent verification of systems/requirements consistency for the emulated WDOG0 watchdog binding increment. Focus areas: (1) SRS-REQ-073/074/075 traceability and test mapping accuracy; (2) derivation logic for timeout and feed cadence; (3) register/instance facts against architecture spec and source; (4) NC register correctness and honest status; (5) safety/architecture consistency of pre-reset report budget, feeder starvation/re-anchor, and flight-target runnable-state decision. ECSS references: ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C, ECSS-Q-ST-30C. |
| Independence declaration          | This reviewer had no part in authoring or executing the increment under review. Fresh context per norm N9. |
| Norms applied                     | N1-N22 (see `docs/rules.md`)           |

---

## 1. Scope

This review independently verifies the 2026-11-09 increment that:
- Implemented a **test-only emulated MPFS WDOG0 binding** for the N18 build (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`) in `src/core/pal/pal_rtems.c` against the verified register map (base `0x20001000`; `TRIGGER → MSVP → TIME → CONTROL`; read-back before first keyed `REFRESH`).
- Uses the pinned Renode model's explicitly emulated 156 250 Hz clock, `MSVP = TIME` (forbidden window disabled), `TRIGGER = 4095` (max 12-bit).
- Moved the feeder to **absolute monotonic deadlines** (`FF_PalTaskDelayUntilNs`) to eliminate cumulative drift (addressing NC-EXT-187).
- Set provisional GEO mission-table defaults: timeout **10 s**, feed **2.5 s** (T/4), pre-reset report budget **250 ms**, execution/jitter margin **100 ms** (recorded under new NC-EXT-238).
- Flight builds keep the three hooks `FF_NOT_IMPLEMENTED` under NC-PAL-01.
- N18 Renode now emits `RENODE_TEST: PASS` with the emulated binding.

The review **excludes**: target/HIL evidence, flight parameter ratification, closure of NC-PAL-01, and any safety approval of the FDIR reset-recovery behaviour. The emulated evidence is explicitly not RTPFS160ZT fidelity.

---

## 2. References

- Project norms: `docs/rules.md` (N1-N22)
- Traceability manifest: `docs/requirements/traceability.yaml`
- SRS: `docs/requirements/SRS.md` (SRS-REQ-073/074/075)
- Architecture spec: `docs/architecture/mpfs-mss-watchdog-integration-spec.md` rev 1.6
- Sprint plan: `docs/planning/sprint-plan.md` rev 1.127
- ADR: `docs/adr/ADR-0021-watchdog-reset-recovery.md` rev 1.3
- Implementation: `src/core/pal/pal_rtems.c`, `src/apps/app_wd.c`, `config/platform/platform_mpfs.h`, `config/tables/fsw_tables.c`
- Tests: `tests/unit/test_pal_watchdog.c`, `tests/unit/test_extwd_model.c`, `tests/unit/test_extwd_middleware.c`
- Lean: `verification/lean/Frick/Watchdog.lean`
- ECSS: ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C, ECSS-Q-ST-30C

---

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
|---|----------|-------------------|---------|----------|----------------|
| 1 | **Comment** | Traceability.yaml SRS-REQ-073 lean_note (line 687-688) | The lean_note still states "Renode has no watchdog model (GAP-A5)" which is outdated since the pinned model now includes `Timers.MPFS_Watchdog` and the emulated binding works. NC-EXT-200 tracks this correction for `pal_rtems.c` comment and traceability.yaml. The note was not yet updated in traceability.yaml. | traceability.yaml line 687-688 vs. `pal_rtems.c` header (lines 36-40) and FRICK-ARCH-0024 rev 1.6 §1. NC-EXT-200 is OPEN "corrected pending independent confirmation". | Update the lean_note in traceability.yaml to reflect the current emulator capability (emulated binding exists, register-level evidence only, not RTPFS160ZT fidelity) as part of NC-EXT-200 closure. |
| 2 | **Comment** | SRS-REQ-073 traceability `tests` field (line 686) | The `tests` array lists `test_extwd_model.c` and `test_extwd_middleware.c` which are XW-2 target-contract (PIL-QEMU) tests using a validated test double, not tests of the MPFS WDOG0 emulated binding. The N18 emulated binding evidence is `ci:renode` (Renode functional validation) not `ci:xw2-target`. The mapping conflates two different evidence scopes: (a) Renode emulated MPFS integration, (b) XW-2 abstract contract validation. | traceability.yaml line 686 lists `ci:xw2-target` alongside the XW-1/2 test files. The actual N18 evidence is the Renode run (`RENODE_TEST: PASS`) not the XW-2 test suite. | Separate the evidence: keep `test_pal_watchdog.c` for the portable PAL contract; add `ci:renode` for the emulated MPFS integration; move `test_extwd_model.c`/`test_extwd_middleware.c`/`ci:xw2-target` to a distinct XW-2 trace entry or annotate them as PIL-QEMU abstract-contract evidence only. |
| 3 | **Minor** | SRS-REQ-073 derivation claim in traceability lean_note (line 688) | The lean_note states "Host coverage of the derivation clauses (timeout taken from the mission table field, feed period no greater than one quarter of it) is provided by test_pal_watchdog.c cases 7-8". While cases 7-8 test `FF_AppWdPlan`, the actual feeder implementation in `FF_AppWdMain` uses absolute monotonic deadlines with a re-anchor on overrun (lines 117-127 in `app_wd.c`). The derivation test verifies the arithmetic, but the runtime behaviour (re-anchor, no catch-up burst) is not covered by the Lean model or the derivation test. | `app_wd.c` lines 105-127: `feed_ns = feed_ms * 1_000_000`, `next_ns += feed_ns`, re-anchor if `now >= next_ns`. `test_pal_watchdog.c` cases 7-8 only test `FF_AppWdPlan` arithmetic. Lean theorem `req_srs073_feed_within_period_not_expired` proves `p ≤ timeout` not `p ≤ timeout/4`. | Add a test case exercising the absolute-deadline feeder with an injected overrun to verify the re-anchor behaviour (no catch-up burst, gap stays below timeout). Note the quarter-period margin is a property of the derivation, not of the Lean model (as correctly stated in Lean scope note line 69-73). |
| 4 | **Comment** | SRS-REQ-074 traceability `implementation` (line 694) | Lists `link.c`, `link.h`, `link_uart.c`, `link_loopback.c` as implementation for the pre-reset report. The actual pre-reset direct path uses `FF_PusSendTmDirect` → `FF_LinkSendUntil` with a deadline-aware writer. The generic `link.c`/`link_loopback.c` are not on the panic path. The implementation list is overbroad. | `src/transport/link/link_uart.c` has `FF_LinkSendUntil`; `link_loopback.c` has no deadline-aware writer. `pal_posix.c` has the panic notifier and direct writer vtables. | Narrow the implementation list to the actual panic-path files: `pal_posix.c` (notifier, direct writer), `pus.c` (direct encode), `link_uart.c` (deadline-aware UART writer), `link.h` (deadline vtable). |
| 5 | **Major** | NC-EXT-186 (sprint-plan.md line 305) – 250 ms pre-reset report bound | The NC states "The 250 ms target/WCET claim remains unproven and requires target/HIL evidence; the 250 ms value is also carried as a provisional GEO parameter under NC-EXT-238." However, the watchdog reaction window at the **emulated clock** is `TRIGGER × t_tick = 4095 / 156250 ≈ 26.2 ms`. The 250 ms report budget **exceeds the emulated reaction window by ~9.5×**. The architecture spec (FRICK-ARCH-0024 R-3, §5.3 HL-2) correctly identifies this as an open question (M-13): whether the pre-reset report is required on the watchdog-timeout path at all, and with which budget. The sprint-plan NC-EXT-186 does not explicitly call out this reaction-window vs report-budget tension. | FRICK-ARCH-0024 rev 1.6 R-3 (lines 524-537): "the reaction-window ceiling would be ≈41.9 ms at 25 MHz… the 250 ms figure is an upper-bound design target… a reaction window that is shorter than the actual path duration would not [fit] — and that actual duration is unknown (M-10 and RP-1)". The emulated profile's 26.2 ms window is even smaller. | Update NC-EXT-186 to explicitly reference the reaction-window vs report-budget tension and the open owner decision M-13 (whether the watchdog-timeout path must also meet the 250 ms report budget). This is a systems-level architecture decision, not a test gap. |
| 6 | **Minor** | NC-EXT-187 (sprint-plan.md line 306) – cadence correction | The NC states "The feeder now uses absolute monotonic deadlines… a 100 ms execution/jitter budget is assumed inside the T/4 interval." The 100 ms budget is 1% of the 10 s timeout. However, the emulated reaction window is only 26.2 ms. If the feeder is starved by higher-priority work (it is the lowest-priority task), the watchdog will expire after the reaction window, not after the 2.5 s feed period. The 100 ms assumption does not account for the watchdog's own reaction window. The feeder starvation hazard (FRICK-ARCH-0024 R-5, M-16) is correctly identified but the margin analysis is incomplete. | `app_wd.c` line 26: "A 100 ms execution/jitter budget is assumed inside the T/4 interval". FRICK-ARCH-0024 R-5: "the feeder is the lowest-priority task, so a busy higher-priority workload starves it first, and a watchdog reset caused by feeder starvation is a false reset of a live system — a named hazard whose acceptable worst case and tolerance are owner inputs (M-16)". | Refine the margin analysis: the effective deadline for the feeder is `TIME × t_tick − (reaction window + MMIO latency + scheduling jitter)`, not simply T/4. The 100 ms budget must be compared against the actual reaction window (unknown for flight, 26.2 ms emulated). Record this as an open systems analysis item. |
| 7 | **Comment** | NC-EXT-198 (sprint-plan.md line 317-322) – flight-target runnable state | The NC correctly states "The owner decision on the flight-target runnable state (options a/b/c) remains open." The three options (a: emulated binding only for N18, flight builds remain FAIL; b: flight builds include the emulated binding as a degraded mode; c: defer N18 until flight binding is authorized) are not explicitly enumerated in the NC but are recorded in FRICK-ARCH-0024 agenda item F and ADR-0021. The NC status is honest: N18 PASS with emulated binding does not close NC-PAL-01. | FRICK-ARCH-0024 rev 1.6 agenda item F (line 753): "decide whether the emulator leg stays non-running… or whether a running-machine leg is authorised". ADR-0021 rev 1.3 Consequences: "Reset recovery is implemented and host-tested but not qualified… the flight-target protection is still absent until NC-PAL-01 is closed". | No action needed — the NC honestly reflects the open decision. Ensure the three options are explicitly enumerated when the owner disposes this NC. |
| 8 | **Comment** | NC-EXT-238 (sprint-plan.md line 325-326) – provisional GEO values | The NC correctly labels the 10 s / 2.5 s / 250 ms / 100 ms values as "provisional representative GEO values" and "not ratified flight thresholds". The rationale "a 10 s timeout tolerates a few missed 2.5 s feeds without nuisance resets" is a reasonable GEO heuristic but is not derived from a fault hypothesis or FMECA. The mission-authority values remain open. | sprint-plan.md line 325-326; `config/tables/fsw_tables.c` line 32: `watchdog_timeout_s = 10u`; `app_wd.c` lines 18-20. | No action needed — the provisional status is clearly documented. The owner should eventually derive the timeout from a failure hypothesis (fault detection time, recovery time, dispatch/mission-degradation strategy) rather than a GEO heuristic. |
| 9 | **Minor** | FRICK-ARCH-0024 rev 1.6 §1 (line 39-44) vs §2.3 (line 138-143) | The document states "N18 status (2026-11-09): the test-only emulated WDOG0 binding now arms and feeds in the pinned Renode image and the N18 marker is `RENODE_TEST: PASS`" but §2.3 states "The model source is not shipped in the image, and no version-bound upstream source is obtainable". These are not contradictory but the readability would improve by explicitly noting that the N18 PASS is **register-sequence integration evidence only**, not counter/timing evidence, because the probe in §2.3 was run in a non-running machine without SUT ELF. | FRICK-ARCH-0024 lines 39-44 vs 138-143. The probe (§2.3) measured register read-backs and warnings but "the counter was not running and REFRESH was never read back" (line 178-179). | Add a clarifying sentence in §1 status bullet: "This is register-sequence integration evidence from a running-machine Renode run; the retained probe (non-running) establishes model register behaviour only." |
| 10 | **Comment** | Lean theorem `req_srs073_feed_within_period_not_expired` scope note | The Lean scope note (lines 66-73) correctly states the theorem proves `p ≤ timeout` not `p ≤ timeout/4`, and that the quarter-period margin is a property of the derivation in `FF_AppWdPlan`. However, the theorem hypothesis `hp : p ≤ s.timeout` allows `p = timeout` (zero margin). The requirement SRS-REQ-073 explicitly requires `p ≤ timeout/4`. The Lean model does not capture this margin — it is correctly flagged as NC-EXT-167. | Lean lines 66-73; traceability.yaml line 687 lean_note: "The quarter-period margin is a property of the derivation in FF_AppWdPlan (checked in test_pal_watchdog.c), not of this theorem; a stronger variant with p ≤ s.timeout / 4 is tracked as NC-EXT-167." | No action needed — the gap is correctly tracked. When NC-EXT-167 is addressed, strengthen the Lean theorem to `p ≤ s.timeout / 4` or add a separate lemma. |
| 11 | **Comment** | FRICK-ARCH-0024 F-18 / R-1 instance selection | The architecture spec correctly withdrew the rev 1.3 recommendation of WDOG1 for emulated configuration after the TRM confirmed only WDOG0 (E51) can reset the MSS. The emulated configuration runs on `u54_1` (U54 core) but the binding uses WDOG0 (E51 watchdog). This is a **conscious deviation**: the emulated binding arms WDOG0 even though the running hart is U54. The spec documents this as "the emulator configuration's running hart is u54_1 and the emulator platform places WDOG1 at 0x20101000; that remains a statement about the emulator, not a protection choice" (R-1 lines 496-499). This is honest but means the emulated N18 run does **not** exercise the actual mission-hart hang detection path. | FRICK-ARCH-0024 rev 1.6 F-18 (lines 399-410), R-1 (lines 475-499). The emulated binding in `pal_rtems.c` uses `FF_WDOG0_BASE = 0x20001000`. | No action needed — the deviation is explicitly documented. The emulated N18 evidence tests the register sequence, not the mission-hart protection architecture. The owner must decide the instance/hart assignment (agenda item C). |
| 12 | **Minor** | SRS-REQ-075 traceability `lean` theorems (line 705) | Lists `req_srs075_single_reset_path` and `req_srs075_one_reset_action`. The Lean theorem `req_srs075_single_reset_path` is discharged by `rfl` between two aliases of the same list (`commandedReset = watchdogTimeout = recoverySequence`). The traceability lean_note (line 706) correctly acknowledges: "req_srs075_single_reset_path is discharged by rfl between two aliases of the same list, so its assurance content is weaker than its name suggests". This is a known weakness, not a defect. | Lean lines 99-115; traceability.yaml line 706. | No action needed — the weakness is documented. The runtime "at most once per occurrence" obligation rests on the reset-counter assertions in the C code and the XW-2 test XW-T7. |

---

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Verdict: APPROVED WITH COMMENTS**

The increment is **systems-consistent** and **honestly documented**. The emulated WDOG0 binding is correctly scoped as N18 integration evidence only, with no flight parameter, target/HIL fidelity, or mission suitability claimed. All NCs are correctly OPEN with accurate status. The provisional GEO values are clearly labelled. The safety/architecture tensions (reaction window vs report budget, feeder starvation hazard, instance/hart assignment) are explicitly surfaced in the architecture spec and NC register.

**Minor findings to track:**
1. Update traceability.yaml SRS-REQ-073 lean_note for NC-EXT-200 closure (Finding 1).
2. Separate XW-2 abstract-contract evidence from Renode emulated integration evidence in traceability (Finding 2).
3. Add feeder overrun/re-anchor test case (Finding 3).
4. Narrow SRS-REQ-074 implementation list to actual panic-path files (Finding 4).
5. Explicitly link NC-EXT-186 to reaction-window vs report-budget tension and open decision M-13 (Finding 5).
6. Refine margin analysis in NC-EXT-187 to account for watchdog reaction window (Finding 6).

---

## 5. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|------|------------------|------|------------|
| R3 (Systems Engineering / N14) | independent sub-agent (session ID not recorded) | 2026-11-09 | APPROVED WITH COMMENTS |

---

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|----|----------|-------------------|---------|-------------|--------------------------|-------|--------|-----------------|
| (new) | Comment | traceability.yaml SRS-REQ-073 lean_note | Outdated "Renode has no watchdog model" | Update as part of NC-EXT-200 closure | traceability.yaml diff | FSW | open | pending NC-EXT-200 |
| (new) | Comment | traceability.yaml SRS-REQ-073 tests | XW-2 evidence conflated with Renode emulated evidence | Separate evidence scopes | traceability.yaml diff | FSW | open | - |
| (new) | Minor | test_pal_watchdog.c | Missing feeder overrun/re-anchor test | Add test case with injected overrun | New test function + CTest pass | FSW | open | - |
| (new) | Comment | traceability.yaml SRS-REQ-074 implementation | Overbroad file list | Narrow to panic-path files | traceability.yaml diff | FSW | open | - |
| NC-EXT-186 | Major | 250 ms report bound | Missing explicit reaction-window vs budget link | Update NC text to reference M-13 and reaction window | sprint-plan.md diff | FSW + avionics | open | - |
| NC-EXT-187 | Major | feeder cadence | 100 ms budget vs reaction window | Refine margin analysis | Architecture analysis record | Systems/avionics + FSW | open | - |

---

Committed and pushed with the reviewed change (norm N11).