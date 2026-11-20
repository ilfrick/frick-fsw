# Review Report: Emulated WDOG0 Binding Increment (N18)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-2026-11-16-WDOG0-R2       |
| Reviewer session                  | Independent V&V agent (fresh context)  |
| Reviewer profile                  | R2 (Senior Aeronautics and Space Software V&V Engineer) |
| Reference revision                | b9681e8 (M12 OBCP VM budget + M16 in-house OBCP placeholder + UART/PUS fixes, includes 2026-11-09 emulated WDOG0 binding increment) |
| Items under review                | src/core/pal/pal_watchdog.c, src/core/pal/pal_posix.c (simulated backend), src/core/pal/pal_rtems.c (emulated WDOG0 binding for N18, FF_NOT_IMPLEMENTED for flight), config/platform/platform_mpfs.h (verified register map), src/apps/app_wd.c (absolute monotonic cadence), FF_PalPanic rework, verification/lean/Frick/Watchdog.lean (6 theorems), tests/unit/test_pal_watchdog.c (11 cases, 320 checks), tests/unit/test_es_hs_lifecycle.c (14 cases, 22 reset-path checks) |
| Date                              | 2026-11-16                             |
| Scope                             | Independent V&V review of the emulated WDOG0 binding increment (2026-11-09). Verify test oracle independence, reset/concurrency coverage, reproducibility, and evidence quality per N10/N22. Check that emulated N18 PASS is correctly reported and not a false PASS. Review against ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), ECSS-E-ST-70-11 5.7.5.2, and project norms N1-N22. |
| Independence declaration          | Reviewer had no part in authoring/executing the items under review; fresh context with no inheritance of author context. |
| Norms applied                     | N1-N22 (docs/rules.md rev 2.8)         |

---

## 1. Scope

**Reviewed:**
- The emulated MPFS MSS WDOG0 binding implemented in `src/core/pal/pal_rtems.c` under `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`, using the pinned Renode model's 156,250 Hz clock (FF_WDOG_EMU_CLK_HZ), MSVP=TIME, TRIGGER=4095, register order TRIGGER→MSVP→TIME→CONTROL with read-back before first REFRESH.
- The portable watchdog state machine in `src/core/pal/pal_watchdog.c` and its simulated backend in `src/core/pal/pal_posix.c`.
- The `FF_PalPanic` rework: log → lock-free pre-reset PUS S5 event report (single absolute-monotonic 250 ms deadline) → single reset path (`FF_PalWatchdogReset`) → test seam → `abort()`.
- The watchdog feeder app (`src/apps/app_wd.c`) using absolute monotonic deadlines (`FF_PalTaskDelayUntilNs`) to avoid cumulative drift.
- Lean 4 theorems in `verification/lean/Frick/Watchdog.lean` (6 theorems modelling software-visible arm/feed/expiry, report-before-reset ordering, single reset path).
- Unit test suites: `tests/unit/test_pal_watchdog.c` (11 cases, 320 checks covering nominal, out-of-bound, hardware fault injection, boundary, SRS-REQ-073 derivation, bad table rejection) and `tests/unit/test_es_hs_lifecycle.c` (14 cases, 22 reset-path checks covering HS detection, reset path, report-before-reset ordering via reset-counter observation, console stall budget sharing, production link wiring, over-length counterexample, unknown function/missing ID rejection, report failure/missing notifier survival, feed-failure reset path).
- Traceability in `docs/requirements/traceability.yaml` for SRS-REQ-073/074/075.

**Excluded:**
- Flight-target MPFS MSS watchdog hardware binding (explicitly NOT IMPLEMENTED per NC-PAL-01).
- Target/HIL evidence (GAP-A5 open).
- External watchdog (XW2) target contract evidence (PIL-QEMU only, tracked in TS-7).
- Schedulability, jitter, WCET claims (no controlled inputs).

---

## 2. References

- Project norms: `docs/rules.md` rev 2.8 (N1-N22)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-073/074/075 entries)
- ECSS-E-ST-40C (software engineering)
- ECSS-Q-ST-80C (software product assurance)
- ECSS-E-ST-70-41C (PUS-C)
- ECSS-E-ST-70-11 5.7.5.2 (watchdog reset recovery: independent hardware watchdog, ground notified before reset, single recovery path)
- Lean model: `verification/lean/Frick/Watchdog.lean`
- Renode model probe evidence: `docs/vv/evidence/2026-10-22-s8-fdir-wd-model-probe.log`
- Verification plan: `docs/vv/verification-plan.md`
- Review template: `docs/reviews/review-template.md`

---

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
|---|----------|-------------------|---------|----------|----------------|
| 1 | **Major** | SRS-REQ-073 / N22 / N18 | **Emulated N18 PASS is not target evidence; the report does not sufficiently distinguish emulator configuration from flight behaviour.** The `RENODE_TEST: PASS` marker exercises the emulated WDOG0 register sequence against the pinned Renode model (156,250 Hz clock, MSVP=TIME, TRIGGER=4095). The traceability `lean_note` for SRS-REQ-073 correctly states this is "emulator configuration and integration evidence, not RTPFS160ZT register/timing/reset fidelity" and that "target arming order and the hardware reset reaction remain HIL scope". However, the N18 gate acceptance criteria (norm N18) treat `RENODE_TEST: PASS` as a pass/fail gate without an explicit evidence classification step that prevents overclaim. The CHANGELOG and loop-audit records show `RENODE_TEST: PASS` as functional evidence without always qualifying the fidelity boundary. | `pal_rtems.c:93-248` (emulated binding under `FF_ENABLE_RENODE_TEST`), `platform_mpfs.h:155-172` (FF_WDOG_EMU_CLK_HZ = 156250 Hz, explicitly "EMULATED / PROVISIONAL N18"), `traceability.yaml:681-710` (lean_note correctly qualifies fidelity), `docs/status/loop-audit.md:77, 894` (N18 PASS recorded without fidelity disclaimer in the status panel). | Add an explicit evidence-classification gate in the N18 runner (or in `tools/scripts/renode_vv_verdict.sh`) that requires the captured log to contain the "emulated profile" log line (`watchdog: WDOG0 armed (emulated profile: ...)`) and fails if the emulated-profile qualifier is absent. Record the fidelity boundary in the N18 gate result artifact (not just in traceability). |
| 2 | **Major** | SRS-REQ-074 / ECSS-E-ST-70-11 5.7.5.2 / N22 | **Report-before-reset ordering oracle relies on reset-counter observation inside a notifier, but the notifier is not part of the flight path.** The Lean theorem `req_srs074_report_precedes_reset` proves ordering on an abstract shared list. The host test `test_hs_reset_report_before_reset_ordering` observes `FF_PalWatchdogGetResets()` inside the panic notifier (`hs_reset_writer_with_observer`) before the reset is requested. This is a valid host-only oracle (NC-EXT-132). However, the flight `FF_PalPanic` path (pal_posix.c:568-632) calls the notifier *before* `FF_PalWatchdogReset()`, but the notifier is not installed in flight builds (the flight wiring installs `FF_LinkSendUntil` as the reporter, not a notifier). The reset-counter observation is a test-seam artifact; the flight ordering is established by code inspection of the single `FF_PalPanic` function, not by an in-flight observable. | `pal_posix.c:568-632` (FF_PalPanic: notifier → FF_PalWatchdogReset → abort), `test_es_hs_lifecycle.c:543-615` (test_hs_reset_report_before_reset_ordering with notifier observation), `Watchdog.lean:56-62` (req_srs074_report_precedes_reset on abstract list), `traceability.yaml:711-750` (lean_note: "Report-before-reset ordering remains established by inspection and the Lean theorem... not the simulated backend (NC-EXT-132)"). | Document NC-EXT-132 explicitly: the report-before-reset ordering is a code-inspection/Lean-theorem claim for flight; the reset-counter oracle is a host-test artifact only. If in-flight ordering verification is required, define a flight-observable mechanism (e.g., a persistent counter in NVM read on next boot) or accept code inspection as the V&V method for this requirement. |
| 3 | **Minor** | SRS-REQ-073 / N22 / Lean scope | **Lean theorem `req_srs073_feed_within_period_not_expired` proves `p ≤ timeout`, not the required quarter-period margin (`p ≤ timeout/4`).** The theorem's scope note explicitly states: "the hypothesis is `p ≤ timeout`, so the theorem proves the *no-expiry* property for any period up to the full timeout... It does **not** prove the requirement's margin: SRS-REQ-073 requires the feed period to be at most `timeout/4`... That quarter-period margin is a property of the derivation in `FF_AppWdPlan` (checked in `test_pal_watchdog.c`), not of this theorem; a stronger variant with `p ≤ s.timeout / 4` is tracked as NC-EXT-167." This is correctly noted, but the theorem name (`feed_within_period_not_expired`) and its presence in the traceability `lean` array could mislead a reader into thinking the quarter-margin is formally verified. | `Watchdog.lean:45-70` (theorem and scope note), `traceability.yaml:681-710` (lean array includes this theorem for SRS-REQ-073), `test_pal_watchdog.c:209-252` (cases 7-8 assert `feed_ms <= (timeout_ms / 4u)`). | Rename the Lean theorem to `req_srs073_feed_within_timeout_not_expired` to match its actual hypothesis, or add the stronger `p ≤ timeout/4` variant (NC-EXT-167) to close the gap. Update traceability `lean_note` to be explicit: "Lean proves feed within full timeout; quarter margin verified by test cases 7-8." |
| 4 | **Minor** | N10 / N22 | **Test oracle independence: `test_es_hs_lifecycle.c` captures EVS events at the listener seam but matches on implementation-defined literal text ("HS: app %u failed") and event ID (0x0502).** The test file header acknowledges: "The heartbeat event id (`0x0502`) and the 'HS: app %u failed' diagnostic text this harness matches on are implementation-defined literals of `src/apps/app_hs.c` (no controlled requirement/interface clause fixes them), so the event-identification oracle is coupled to the implementation under test... A controlled identifier/text contract would be required to remove that coupling. GAP-R3-1 stays OPEN." This is correctly self-identified, but the test still uses the coupled oracle as a pass/fail criterion. | `test_es_hs_lifecycle.c:33-48` (cap_hs_reports matches "app %u failed"), `test_es_hs_lifecycle.c:10-15` (header comment acknowledging coupling), `traceability.yaml` (GAP-R3-1 open). | Close GAP-R3-1 by defining a controlled HS event contract (event ID, type, text template) in SSS/SRS or an ICD, then update the test oracle to match the controlled contract. Until then, record the test result as "oracle-coupled observation" not "independent requirement verification". |
| 5 | **Minor** | N22 / NC-EXT-125 | **Lean theorem `req_srs075_single_reset_path` is discharged by `rfl` between two aliases of the same list, providing negligible assurance.** The traceability `lean_note` for SRS-REQ-075 states: "Known weakness (NC-EXT-125): req_srs075_single_reset_path is discharged by rfl between two aliases of the same list, so its assurance content is weaker than its name suggests; the runtime 'at most once per occurrence' obligation rests on the reset-counter assertions, not on the theorem." The theorem should either be strengthened (e.g., prove that any two reset-triggering paths converge to the same sequence) or removed from the traceability `lean` array to avoid overclaim. | `Watchdog.lean:72-78` (commandedReset = watchdogTimeout by rfl; req_srs075_single_reset_path : commandedReset = watchdogTimeout := rfl), `traceability.yaml:750-780` (lean_note acknowledges NC-EXT-125). | Either strengthen the Lean model to make the theorem non-trivial (e.g., model distinct entry points that must converge), or remove it from the SRS-REQ-075 `lean` array and rely solely on the reset-counter runtime assertions (`test_pal_watchdog.c` case 6, `test_es_hs_lifecycle.c` cases) which are the actual evidence. |
| 6 | **Comment** | N18 / NC-TS-01 / NC-EXT-199 | **N18 runner ELF freshness risk is documented but not fully mitigated.** NC-TS-01 records: "`tools/scripts/renode_vv.sh` gates the N18 run on the **existence** of `build/renode/frick_fsw_renode.elf`, not on its freshness relative to the source. A stale pre-change ELF therefore satisfies the gate and yields `RENODE_TEST: PASS` that does **not** correspond to the current tree." NC-EXT-199 records a past occurrence where the runner used a stale log. The current runner (`renode_vv_verdict.sh`) validates the capture directory is fresh and non-empty, but does not verify the ELF hash matches the current source tree. | `tools/scripts/renode_vv.sh`, `tools/scripts/renode_vv_verdict.sh`, `docs/planning/sprint-plan.md` (NC-TS-01, NC-EXT-199). | Add ELF hash verification to the N18 gate: compute the expected ELF hash from the current build (or bind the run to `make renode-image` as a prerequisite) and fail if the executed ELF does not match. |
| 7 | **Comment** | N22 / test_pal_watchdog.c boundary test | **Boundary test `test_watchdog_expiry_boundary` uses 100 ms margin on 1000 ms timeout (10%), acknowledging host jitter, but does not test the exact equality boundary.** The test comment states: "The exact-equality point is not asserted: the monotonic clock advances between the injection and the check, so an 'exactly equal' sample is not reproducible (norm N22: do not test physically meaningless values)." This is a correct application of N22, but the test only covers 900 ms (inside) and 1100 ms (outside) — the transition zone is not exercised. The 100 ms margin is far above typical host jitter (<1 ms). | `test_pal_watchdog.c:150-175` (test_watchdog_expiry_boundary with 100 ms margin). | Consider adding a test with a smaller but still reproducible margin (e.g., 10 ms on 1000 ms) to better characterize the boundary, or explicitly justify why 10% is the chosen margin in the test comment. |
| 8 | **Comment** | SRS-REQ-074 / mandatory text handling | **Over-length panic reason suppresses the pre-reset notification (counterexample characterized, not resolved).** `test_panic_overlength_reason_counterexample` shows: 96 chars → notifier called; 97 chars → notifier suppressed; 256 chars → notifier suppressed. The mandatory-text policy is unresolved; the test is a "counterexample characterization, not requirement-coverage credit." The traceability notes: "Mandatory-content handling is still unresolved: an over-length reason suppresses the notification, and the formatting work itself is not deadline-enforced, so the requirement-level exactly-one obligation, the mandatory-text envelope and the bounded-formatting strategy remain authority decisions." | `test_es_hs_lifecycle.c:451-495` (test_panic_overlength_reason_counterexample), `traceability.yaml:711-750` (lean_note on mandatory-content gaps). | Resolve the mandatory-text policy with the authority (owner) and update the implementation and test oracle accordingly. Until then, the over-length suppression is an open functional gap for SRS-REQ-074 "exactly one pre-reset report". |

---

## 4. Conclusion

| Criterion | Status |
|-----------|--------|
| Test oracle independence (N10/N22) | **NOT SATISFIED** — Finding #4 (HS oracle coupled to implementation literals, GAP-R3-1 open); Finding #8 (mandatory-text policy unresolved). |
| Reset/concurrency coverage | **PARTIAL** — Finding #2 (report-before-reset ordering relies on test-seam notifier observation, not in-flight observable); Finding #5 (Lean single-reset-path theorem trivial). |
| Reproducibility (N18/N22) | **NOT SATISFIED** — Finding #1 (N18 PASS not fidelity-qualified in gate result); Finding #6 (ELF freshness not verified). |
| Evidence quality | **PARTIAL** — Finding #3 (Lean theorem hypothesis weaker than requirement margin, correctly noted but traceability array may mislead); Finding #7 (boundary margin not boundary-characterising). |
| ECSS-E-ST-70-11 5.7.5.2 compliance (sw-visible) | **PARTIAL** — Software-visible ordering (Lean + code inspection) OK; hardware independence and in-flight observability NOT established (NC-PAL-01, GAP-A5). |

**Overall:** **REJECTED** — Major findings #1 and #2 must be resolved before the emulated WDOG0 binding increment can be accepted as V&V evidence for the applicable requirements. The emulated N18 PASS is correctly achieved against the pinned model, but it is not correctly reported as emulator-only evidence, and the report-before-reset ordering lacks an in-flight observable oracle.

---

## 5. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R2 (Senior SW V&V Engineer, N9) | Independent V&V agent (fresh context) | 2026-11-16 | REJECTED — Major findings #1, #2 open |

---

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| NC-WDOG0-01 | Major | SRS-REQ-073 / N18 / N22 | Emulated N18 PASS reported without fidelity qualification in gate result | Add explicit evidence-classification gate in N18 runner requiring "emulated profile" log line; fail if absent | Updated `renode_vv_verdict.sh` or runner script with fidelity check; revised N18 gate artifact | FSW engineer | Open | Required |
| NC-WDOG0-02 | Major | SRS-REQ-074 / ECSS-E-ST-70-11 5.7.5.2 | Report-before-reset ordering oracle is test-seam only; no in-flight observable | Document NC-EXT-132 explicitly; define flight-observable mechanism or accept code inspection as V&V method | Updated traceability lean_note; if mechanism added, updated pal_posix.c and test | FSW engineer / PA | Open | Required |
| NC-WDOG0-03 | Minor | SRS-REQ-073 / Lean / N22 | Lean theorem proves `p ≤ timeout`, not quarter-margin `p ≤ timeout/4` | Rename theorem to match hypothesis or add stronger variant (NC-EXT-167); update traceability lean_note | Updated `Watchdog.lean` and `traceability.yaml` | V&V engineer | Open | Required |
| NC-WDOG0-04 | Minor | N10 / N22 / GAP-R3-1 | HS test oracle coupled to implementation-defined event ID and text | Define controlled HS event contract in SRS/SSS/ICD; update test oracle | Updated requirements, updated `test_es_hs_lifecycle.c` | SysEng / FSW engineer | Open | Required |
| NC-WDOG0-05 | Minor | SRS-REQ-075 / NC-EXT-125 | Lean single-reset-path theorem trivial (rfl) | Strengthen theorem or remove from traceability `lean` array | Updated `Watchdog.lean` and `traceability.yaml` | V&V engineer | Open | Required |
| NC-WDOG0-06 | Comment | N18 / NC-TS-01 | ELF freshness not verified in N18 gate | Add ELF hash verification to N18 runner | Updated `renode_vv_verdict.sh` or `renode_vv.sh` | Tooling engineer | Open | Recommended |
| NC-WDOG0-07 | Comment | N22 | Boundary test margin not boundary-characterising | Add smaller reproducible margin test or justify 10% choice | Updated `test_pal_watchdog.c` | V&V engineer | Open | Recommended |
| NC-WDOG0-08 | Comment | SRS-REQ-074 | Over-length reason suppresses notification; mandatory-text policy unresolved | Resolve mandatory-text policy with authority; update implementation and test | Updated requirements, `pal_posix.c`, `test_es_hs_lifecycle.c` | PA / Owner | Open | Required |

---

**Compliance with norms:** This review was conducted per N9 (independent agent, fresh context), N10 (test script review), N22 (requirement-based test oracle independence), and ECSS-E-ST-70-11 5.7.5.2. The review report is stored per N11. All findings are tracked as NCs per N12 for resolution and delta re-review.