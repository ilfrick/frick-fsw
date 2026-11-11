# Systems Review Report — Independent R1/R3 Avionics Review

**Repository:** frick-fsw (branch `main`, HEAD 833a0e0, working tree clean — no tracked modifications)
**Reviewer role:** Senior Aeronautics and Space Avionics Systems Engineer (independent review, R1/R3 profile)
**Review date:** 2026-11-11
**Target:** ECSS class B; Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2
**Increment under review:** Re-execution of norm N18 (Renode emulated-target functional validation) on the current tree; result PASS (`[renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)`). No source code change.
**NC under review:** NC-EXT-165 (Major gate NC — N18 functional validation not satisfied for the current tree; blocks N7 commit/push until satisfied; owner FSW/V&V; verifier independent R2).

## Authority statement

This review provides engineering evidence only. It does not certify, qualify, accept, waive, or approve any airworthiness, human-rating, safety, security, or mission-acceptance item. It does not close NC-EXT-165, does not accept residual risk, and does not grant commit/push authority. All safety-critical and regulatory decisions remain with authorized humans (N7). NC closure and release approval are separate authority gates (N12/N15/N20).

---

## Executive summary

The N18 re-run is **host-only emulation** against the pinned Renode PolarFire SoC model: no RTPFS160ZT silicon, no target clocks/resets, no HIL, no electrical independence, and no timing/WCET evidence. It therefore introduces **no system-level risk**, leaves **NC-PAL-01 (flight watchdog binding unwired) and GAP-A5 (target/HIL evidence) untouched**, and impacts **no GNC, safety, FDIR, or operations interface** (there is no code change).

The required action of NC-EXT-165 — a fresh RTEMS 6.2 cross-build and prescribed `make renode` on the current tree with retained logs and source/image/ELF identity — has been performed and PASSES. No finding blocks commit/push or NC-EXT-165 resolution. Final disposition of NC-EXT-165 and the commit/push decision remain with the independent R2 verifier and the N7 authority.

All host gates pass on the current tree: build exit 0 (0 warnings), CTest 56/56, Lean `lake build`, trace PASS (120/76/30/14/68), format-check PASS, license OK.

---

## Findings register

| ID | Classification | Blocks commit/push? | Blocks NC-EXT-165 resolution? | Finding (one line) |
|----|---------------|---------------------|-------------------------------|--------------------|
| SYS-R1/R3-001 | Minor | No | No | N18 re-run introduces no system-level risk — host-only emulation, no target hardware |
| SYS-R1/R3-002 | Major | No (removes the N18 blocker) | No (satisfies it) | NC-EXT-165 resolution evidence: N18 re-run PASSES on the current tree |
| SYS-R1/R3-003 | Minor | No | No | NC-PAL-01 unaffected — flight watchdog binding remains unwired |
| SYS-R1/R3-004 | Minor | No | No | GAP-A5 unaffected — N18 is not target/HIL evidence |
| SYS-R1/R3-005 | Minor (positive) | No | No | GNC, safety, FDIR, operations interfaces not impacted — no code change |
| SYS-R1/R3-006 | Minor (positive) | No | No | Cross-disciplinary consistency: emulated-target validation only; no target/HIL claim |
| SYS-R1/R3-007 | Minor | No | No | NC-TS-01 ELF-freshness weakness retained; mitigated for this run; does not block N18 |
| SYS-R1/R3-008 | Major (determination) | No | No | Determination: no blocking systems finding; commit/push unblocked subject to R2 disposition and full N7 gates |

---

## Detail: findings

### SYS-R1/R3-001 — Minor — N18 re-run introduces no system-level risk (host-only emulation, no target hardware)

**Evidence.**
- `tools/scripts/renode_vv.sh:1-128`: runs the middleware under test on the emulated PolarFire SoC via the pinned Renode image `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061`; bounded wait; container status/exit code retained; verdict taken only from a fresh per-run directory (fail-closed, NC-EXT-199).
- `tools/scripts/renode_vv_verdict.sh:1-110`: PASS only when `RENODE_TEST: PASS` is present in this run's own process log and UART console; missing/empty/unreadable capture fails closed; fatal-exit vs timeout distinguished.
- `src/apps/app_renode_test.c:1-11, 575-678`: middleware self-checks (Software Bus publish/subscribe, Event Services emit, Time Services monotonic) plus HAL MMIO checks against emulated MPFS models; prints `RENODE_TEST: PASS`/`FAIL` at line 672.
- `config/platform/platform_mpfs.h:115-128`: `FF_WDOG_EMU_CLK_HZ` (156 250 Hz) is explicitly the pinned Renode model's emulated clock, **not** a flight parameter (NC-PAL-01); TRIGGER/MSVP/TIME values are emulated-profile choices.

**Assessment.** The re-run exercises middleware and HAL seams on an emulator. It writes no registers on flight silicon, changes no flight configuration, establishes no RTPFS160ZT register/clock/reset-scope/timing behaviour, and creates no new failure mode in any deployed system. System-level risk from the re-run itself: none.

**Blocker?** No. Does not block commit/push or NC-EXT-165 resolution.

---

### SYS-R1/R3-002 — Major — NC-EXT-165 resolution evidence: N18 re-run PASSES on the current tree

**Evidence.**
- `docs/planning/sprint-plan.md:284`: NC-EXT-165 requires "fresh RTEMS 6.2 cross-build and prescribed `make renode` on the current tree with retained logs and source/image/ELF identity"; it is a gate blocking N7 commit/push until satisfied; owner FSW/V&V, verifier independent R2.
- `Makefile:148-152`: `renode-image` builds the RTEMS 6.2 image with `-DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON` and copies `build/renode/frick_fsw` to `build/renode/frick_fsw_renode.elf`.
- `Makefile:96-97`: `renode` runs `tools/scripts/renode_vv.sh` (norm N18).
- Context run evidence (review input, not re-derived here): `[renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)` on the current tree; all host gates pass (build exit 0 / 0 warnings, CTest 56/56, Lean, trace, format, license).
- Working tree clean: `git status --porcelain` shows no tracked modifications — the increment is the re-execution only.

**Assessment.** NC-EXT-165's required action is complete on the current tree. The re-run does not change source, so it cannot introduce a functional regression; it only regenerates the gate evidence that was stale after the 2026-10-21 corrections (`src/apps/app_wd.c`, `src/apps/app.h`, `tests/unit/test_pal_watchdog.c`, per sprint-plan.md:284).

**Blocker?** No for commit/push once the independent R2 verifier accepts the evidence; it **removes** the N18 blocker registered by NC-EXT-165. It does not block NC-EXT-165 resolution — it is the resolution evidence. Final NC closure and release approval remain with the R2 verifier and N7 authority (N12/N15/N20).

---

### SYS-R1/R3-003 — Minor — NC-PAL-01 unaffected: flight watchdog binding remains unwired

**Evidence.**
- `src/core/pal/pal_rtems.c:93-100`: the WDOG0 binding is compiled only for `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST` (the emulated N18 profile); flight builds use the `#else` branch.
- `src/core/pal/pal_rtems.c:278-304`: flight hooks `ff_pal_wd_hw_arm/feed/reset` return `FF_NOT_IMPLEMENTED` with `(NC-PAL-01)`; comments state the flight-target clock, gating, mission parameters and HIL evidence are not authorised.
- `src/core/pal/pal_watchdog.c:12-13`: "On RTEMS the MPFS MSS watchdog binding is deferred and tracked by NC-PAL-01; the hooks report FF_NOT_IMPLEMENTED."
- `pal.h:120-123`: the RTEMS/MPFS hardware binding is not wired (NC-PAL-01).
- `src/apps/app_wd.c:96-98`: on target, a failed `FF_PalWatchdogInit` panics (`watchdog: init failed`) — the flight path still fails explicitly until NC-PAL-01 is closed.

**Assessment.** N18 used the emulated profile only (`FF_ENABLE_RENODE_TEST`). The flight build path, the unwired MPFS WDOG0 binding, and the NC-PAL-01 Major NC are unchanged. NC-PAL-01 remains open as a separate item and is not resolved by N18.

**Blocker?** No. NC-PAL-01 remains open independently, but it is not affected by this increment and does not block N18 resolution.

---

### SYS-R1/R3-004 — Minor — GAP-A5 unaffected: N18 is not target/HIL evidence

**Evidence.**
- `src/apps/app_renode_test.c:126-135`: GEM check is an MMIO Module ID/revision identification only; comment notes the full GEM driver/DMA/link path is host-tested and on-target PHY/link is GAP-A5.
- `src/apps/app_renode_test.c:266-326`: network check uses the reference loopback stack (in-process contract check), not an emulated peripheral data path.
- `src/apps/app_renode_test.c:378-404` (CAN), `406-435` (I2C), `437-465` (SPI), `543-571` (block/MMC): each check is register/MMIO round-trip or self-test on emulated models with explicit "on-target … is GAP-A5" caveats; no bus transaction runs where no real slave/medium is attached.
- `README.md:174`: HWIL = GAP-A5; Renode row is functional validation only.
- `docs/vv/renode-environment.md:12-19`: Renode runs the same RTEMS binary "without hardware (partial GAP-A5 mitigation)" — mitigation, not substitution.

**Assessment.** GAP-A5 (target/HIL evidence for peripheral-bound paths: PHY/link, DMA, real bus timing, PTP accuracy, flash endurance/retention, SD behaviour) remains open. N18 makes no target/HIL claim and consumes none of the GAP-A5 evidence budget.

**Blocker?** No.

---

### SYS-R1/R3-005 — Minor (positive) — GNC, safety, FDIR, operations interfaces not impacted (no code change)

**Evidence.**
- `git diff HEAD --stat`: empty — no source code change in this increment.
- `src/apps/app_renode_test.c:621-622`: the only PUS-relevant emission is `FF_EvsSend(0x7E01u, FF_EVS_INFO, me, "renode-test")` — informational telemetry, telemetry-only; no GNC function consumes service 5 events.
- `src/core/pal/pal_watchdog.c:39-57, 80-88`: watchdog arm/feed/check contract unchanged; `FF_PalWatchdogCheck` has no production call site (pre-existing NC-EXT-129, unrelated to this increment).

**Assessment.** No functional chain, control law, safety behaviour, FDIR recovery path, or ground/operations interface is altered. GNC: none. Safety: no requirement weakened; no new hazard introduced. FDIR: the watchdog reset-recovery path and its unwired state are unchanged. Operations: event IDs remain uncontrolled (pre-existing NC-EXT-102/SYS-R1/R3-006 of the earlier review); no change.

**Blocker?** No.

---

### SYS-R1/R3-006 — Minor (positive) — Cross-disciplinary consistency: emulated-target validation only; no target/HIL claim

**Evidence.**
- `tools/scripts/renode_vv.sh:4-5`: "Runs the middleware under test on the emulated PolarFire SoC (Renode image, pinned)".
- `Makefile:172-179`: XW-2 target contract is explicitly "NOT norm N18: the production watchdog hooks are replaced by a validated test double … a PASS proves … nothing about a device, reset scope, electrical independence, timing or the flight path."
- `src/core/pal/pal_rtems.c:34-40`: authorization boundary comment — the emulated configuration "establishes no target behaviour."
- `README.md:354-363` (Renode V&V environment section): distinguishes emulated validation from target/HIL.

**Assessment.** Architecture, scripts, Makefile, app code, and documentation are mutually consistent: N18 = emulated-target functional validation; target/HIL/fidelity claims are expressly disclaimed. No inconsistency between the N18 PASS and any target-qualification position.

**Blocker?** No.

---

### SYS-R1/R3-007 — Minor — NC-TS-01 ELF-freshness weakness retained; mitigated for this run

**Evidence.**
- `docs/planning/sprint-plan.md:284` (NC-EXT-165 row): "the NC-TS-01 ELF-freshness weakness still applies" — `renode_vv.sh:41-44` gates on ELF existence, not freshness; a fresh build for this run mitigates staleness but the structural weakness remains.
- `tools/scripts/renode_vv.sh:34`: `ELF="${ROOT}/build/renode/frick_fsw_renode.elf"`.

**Assessment.** The weakness does not invalidate this run (context states the fresh re-run passes on the current tree), but it should be corrected before the next N18 cycle so the gate cannot pass against a stale ELF. It is a verification-infrastructure item, not an avionics design finding.

**Blocker?** No. Does not block this N18 resolution; recommended fix before the next cycle.

---

### SYS-R1/R3-008 — Major (determination) — No blocking systems finding; commit/push unblocked subject to R2 disposition and full N7 gates

**Evidence.** Consolidation of SYS-R1/R3-001..007 plus context host-gate results.

**Assessment.**
- Systems-level risk from the increment: none (SYS-R1/R3-001).
- NC-EXT-165 resolution precondition met: N18 re-run PASSES on current tree (SYS-R1/R3-002).
- NC-PAL-01 and GAP-A5 unaffected and remain open as separate Major items (SYS-R1/R3-003/004) — they are not preconditions for N18 and are not changed here.
- GNC/safety/FDIR/operations: no impact (SYS-R1/R3-005).
- Cross-disciplinary consistency: maintained (SYS-R1/R3-006).

**Blocker?** No finding blocks commit/push or NC-EXT-165 resolution. Commit/push remains subject to: (1) independent R2 verifier acceptance of the N18 evidence and disposition of NC-EXT-165; (2) full N7 gate compliance (all applicable gates green on the current tree); (3) separate open gates NC-PAL-01, GAP-A5, NC-TS-01, NC-EXT-129/166/168/169 etc. where applicable to the release scope. This review does not close any NC or waive any gate.

---

## Cross-disciplinary consistency (GNC / safety / FDIR / operations)

| Discipline | N18 re-run effect | Assessment |
|---|---|---|
| **GNC** | None. No control law, sensor, actuator, or guidance interface touched. | No impact. |
| **Safety** | None. No safety requirement changed; no new failure mode created by emulation. | Safety-neutral. |
| **FDIR** | None. Watchdog arm/feed/reset contract unchanged; binding remains unwired (NC-PAL-01). | No new FDIR claim made. |
| **Operations** | None. Event reporting unchanged; ground interface (PUS S5 profile) provisional and uncontrolled as before (NC-EXT-102). | No operational interface change. |

---

## Verification basis reviewed

- `tools/scripts/renode_vv.sh` (lines 1-128) — norm N18 runner, fail-closed capture, fresh per-run directory.
- `tools/scripts/renode_vv_verdict.sh` (lines 1-110) — verdict logic, marker-only PASS, fatal-exit/timeout classification.
- `tools/scripts/renode_env_selftest.sh` (lines 1-40) — norm N18 environment self-test.
- `src/apps/app_renode_test.c` (lines 1-678) — Renode test application and HAL self-checks.
- `src/core/pal/pal_rtems.c` (lines 93-306) — MPFS WDOG0 binding, emulated vs flight branches, NC-PAL-01 boundary.
- `src/core/pal/pal_watchdog.c` (lines 1-120) — portable watchdog harness; RTEMS binding deferred.
- `pal.h` (lines 113-144) — watchdog contract abstraction; NC-PAL-01 noted.
- `config/platform/platform_mpfs.h` (lines 100-128) — WDOG0 register map and emulated clock.
- `src/apps/app_wd.c` (lines 80-128) — watchdog feeder; T/4 cadence; panic on init failure.
- `src/main/fsw_main.c` (line 89) — pre-reset panic reporter wiring (unchanged).
- `Makefile` (lines 96-101, 148-152, 172-179) — renode/renode-image/xw2 targets; xw2 not N18.
- `docs/vv/renode-environment.md` (lines 1-19) — Renode V&V environment; partial GAP-A5 mitigation only.
- `docs/planning/sprint-plan.md` (line 284) — NC-EXT-165 register entry and required action.
- Context host-gate results: build exit 0 / 0 warnings; CTest 56/56; Lean `lake build`; trace PASS (120/76/30/14/68); format-check PASS; license OK; working tree clean.

---

## Determination

**No finding blocks commit/push.** **No finding blocks NC-EXT-165 resolution.** The N18 re-run on the current tree PASSES and satisfies the required action of NC-EXT-165. Final disposition of NC-EXT-165 (closure), and the commit/push decision under N7, rest with the independent R2 verifier and the authorized human authority. NC-PAL-01 (flight watchdog binding unwired) and GAP-A5 (target/HIL evidence) remain open and unaffected by this increment.

This review does not close any NC, waive any gate, establish target/HIL/fidelity evidence, or approve any release. All safety-critical and regulatory decisions remain with authorized humans.

---

**End of report.**