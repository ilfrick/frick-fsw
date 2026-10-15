# Timing, Environmental, Security and Tool Qualification (N22 work package SVA-3)

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-SVA-WP3                       |
| Revision                         | 0.1                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N5, N10, N11, N15, N21, N22            |
| Umbrella                         | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) §4–7 |
| Test cases (oracles)             | work packages `ts-1-*` … `ts-6-*` (`docs/vv/test-spec/README.md`) |
| Levels / environments            | `docs/vv/test-spec/sva-1-levels-vmodel.md` (FRICK-VV-SVA-WP1) |
| Interfaces / FDIR                | `docs/vv/test-spec/sva-2-interfaces-fdir.md` (FRICK-VV-SVA-WP2) |
| Tool register                    | `docs/vv/tool-qualification.md` (T1..T21) |
| Baseline                         | Working tree at HEAD `57974d2` as read 2026-10-14 |

> **Authority statement.** This work package states the *verification
> architecture* for timing/determinism, environmental/qualification, security,
> and V&V-tool qualification. It does **not** set mission-criticality (NC-EXT-11),
> environmental/radiation levels (GAP-A6), failure-detection coverage/PSF
> (NC-EXT-31), WCET budgets (GAP-A4), fail-operational behaviour, residual risk,
> or release/flight readiness. Those are authority-owned (see §6). Nothing here is
> a compliance, qualification or airworthiness claim.

## 1. Scope, purpose and terminology

This package captures `docs/vv/system-verification-architecture.md` §4–7 at
package granularity so the per-area test cases (`ts-*`) know which environment
and oracle each aspect requires. Terminlogy follows that document and
`docs/vv/comprehensive-test-spec.md` (N22).

## 2. Timing and determinism

The target is a Microchip RT PolarFire SoC RTPFS160ZT: 5×Ri5 (RV64GC, in-order,
private L1I+D, private L2) + 1×E51 (RV64GC, dual-issue, private L1, shared L2)
under RTEMS 6.2 `riscv/mpfs64imafdc`. Timing assurance therefore *cannot* be
claimed from the emulator; HIL/silicon is required for anything that depends on
silicon behaviour.

| Aspect | Method | Environment | Can establish | Cannot establish | Status → GAP-NC |
| --- | --- | --- | --- | --- | --- |
| WCET (per task) | measurement + static analysis | SIL/PIL | relative ordering, gross regression | hardware-timed WCET, cache/pipeline effects | GAP-A4 (class-A plan) |
| Schedulability (fixed-priority preemptive) | response-time analysis | analysis (SIL model) | structural feasibility vs budget | multicore interference, priority inversion from shared HW | GAP-A4 |
| End-to-end latency | timestamp at producer, observe at consumer | PIL-Renode / HIL | nominal chain latency | jitter under contention, silicon extremes | GAP-A4 |
| Jitter / ISR latency | interrupt latency measurement | HIL target | silicon ISR latency, jitter | emulator timing only | GAP-A4 |
| Multicore interference | interference-bounding analysis (shared L2/mem ctrl) | HIL target analysis | absence of contention in nominal load | contention characterization, worst-case share | GAP-A4 |
| Watchdog cadence (kick/escalation) | timed kick + missed-kick injection | SIL/PIL/HIL | kick path + missed-kick escalation | brownout reset interaction | `ts-6` watchdog rows; GAP-A4 for silicon |

> "Average load is low" is not timing assurance; multicore shared L2/controller
> contention is first-order and must be measured on target.

## 3. Environmental and hardware qualification

All environmental/qualification **levels** are authority-owned per mission
context; the project supplies the test architecture and records the gaps.

| Item | Requirement source | Method | Environment | Status → authority/GAP |
| --- | --- | --- | --- | --- |
| Radiation: SEE/SEL/SEFI ( latchup, upset, inhibit, reset) | per mission environment | heavy-ion/thermal cycle, SEU inject | HIL target | **authority-owned (GAP-A6)** |
| Temperature (operational, survival, cycling) | per mission environment | thermal/vibration test + margins | HIL target / EUT | **authority-owned (GAP-A6)** |
| Vibration / shock (random, sine, pyrotechnic) | per mission environment | vibration/shock test | EUT/HIL | **authority-owned (GAP-A6)** |
| EMC (conducted/radiated emissions & immunity) | per mission environment | EMC test | EUT | **authority-owned (GAP-A6)** |
| Power-on reset / brownout | `src/bsp/mpfs/board.c`; FRICK-ICD-001 | reset-vector + brownout simulation | PIL/HIL | design-stage; target evidence **GAP-A5** |
| Clock-fail / redundant clock | FRICK-ICD-001 time-validity | clock-loss inject | PIL/HIL | `ts-3` time source-loss; silicon clock-switch timing **GAP-A4** |
| Watchdog / safing | `src/core/es/`; SRS-REQ-015 | missed-kick, reset escalation | SIL/PIL/HIL | `ts-6` watchdog rows; silicon reset behavior **GAP-A5** |

## 4. Security verification

Security scope is per the approved threat model and the governing security rules
(see `docs/quality/class-a-gaps.md`); until the threat model and key policy are
authority-agreed (NC-EXT-43/44), tests use documented project assumptions and
record the gap — no invented oracles.

| Domain | Requirement / NC | Test intent | Oracle | Status |
| --- | --- | --- | --- | --- |
| Secure boot chain (ROM→FSBL→SSBL→app) | NC-EXT-43 | image integrity/auth, anti-rollback | verified boot or fail-closed; no unsigned exec | design-stage; evidence pending authority agreement |
| Command auth / anti-replay / rate-limit | SRS-REQ-044; NC-EXT-44 | replayed/oversized/illegal TC rejected | TC rejected + event; no side-effect | `ts-1`/`ts-2` SDLS rows; full key-lifecycle pending NC-EXT-44 |
| Key management | NC-EXT-44 | key load/provision, expiry, revocation | key accepted/rejected per policy | pending authority key-policy agreement |
| Secure update | SRS-REQ-015; NC-EXT-43 | atomic install, rollback on failure | pre/post hash match; old image runnable | design + `ts-5` staging; rollback verification **gap** |
| Information protection / least privilege | NC-EXT-44 | partition/isolation enforcement | no cross-partition data leak | MPU/MMU region review; silicon enforcement **GAP-A5** |

**Separation note (N13):** the T21 SPDX/license check is a legal/provenance
control (license boundaries), intentionally kept separate from functional
security. T21 is **NOT FULLY QUALIFIED**.

## 5. Tool qualification summary

V&V tools registered in `docs/vv/tool-qualification.md` (T1..T21). At the
class-B target the minimum is **source review + inspection + configuration
management**; tools whose defect could not be caught by an independent check
receive qualification evidence to that effect.

Current **open** qualification items at this baseline: **T1, T13, T18, T21**
(NC-EXT-29/T21; T21 NOT FULLY QUALIFIED). Re-qualification is **required on every
qualification-affecting change** (any V&V script/tool that participates in an
acceptance decision) — a changed script invalidates its evidence until
re-qualified (per N10/T21 discipline). The Lean prover (N1) and the `trace.py`
(N4) traceability check are also treated as controlled V&V tools whose evidence
is retained per cycle (N15).

| Tool | Role | Qualification status | Gate |
| --- | --- | --- | --- |
| T1 | C coding rule checker | open | N4/N10 source review |
| T13 | static analyzer | open | N4/N10 source review |
| T18 | RTEMS cross toolchain | open | class-B minimum; NC-EXT-40 reproducibility noted |
| T21 | SPDX/license policy checker | NOT FULLY QUALIFIED | N13; blocks only when in changed scope |
| Lean prover | formal requirement theorems (N1) | qualified-per-change | N1/N15 |
| `trace.py` (N4) | bidirectional traceability | controlled | N4/N15 |
| Renode/QEMU emulators | PIL environment | environment-qualified | SVA-1 §4 (not requirement coverage) |

## 6. Authority-owned parameters (left unassigned)

| Parameter | Why authority-owned | Owner |
| --- | --- | --- |
| Mission criticality / DAL (class-A vs class-B) | risk/mission decision | program/safety (NC-EXT-11) |
| Environmental & radiation levels | mission profile & environment | program/HW (GAP-A6) |
| Failure hypotheses & detection coverage / PSF | hazard basis | safety (NC-EXT-31) |
| WCET budgets & timing allowances | performance/risk | GNC/systems (GAP-A4) |
| Safe-state policy & recovery authority | crew/public safety | safety/ops |
| Threat model, keys, crypto policy, anti-rollback authority | security/risk | security authority (NC-EXT-43/44) |
| Release / flight / launch go decision | operational readiness | ops/commander (N17/N20) |

## 7. Ownership summary

- **Project owns**: timing *architecture* (levels, methods, environments, what each
  can/cannot prove), nominal + out-of-bound test design, FDIR recovery paths,
  tool-qualification evidence, combined-coupling scenarios, and recording of the
  authority-owned gaps above.
- **Authority owns**: every item listed in §6, including the actual values that
  turn the gaps above into qualified evidence.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial timing/environmental/security/tool-qualification work package for N22; can/cannot-establish per environment, authority-owned parameters, open tool qualifications (T1/T13/T18/T21), T21 legal/security separation |
