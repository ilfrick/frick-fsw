# SYS-REVIEW-R1-R3-20261112 — Avionics Systems Review (R1/R3 profile)

**Repository:** /home/nicola/fsw/frick-fsw  
**Branch:** main  
**HEAD:** b6bf060be425db99325c2d4685cd0a8b0ef104a6  
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2  
**Criticality target:** ECSS class B  
**Reviewer profile:** Senior Aeronautics and Space Avionics Systems Engineer (R1/R3) — independent review  
**Review date:** 2026-11-12  
**Change under review:** docs/interfaces/ICD.md line 106 — interface annotation only  
- Before: `| 1  | 4 / 6 | start / progress failure | TM | errcode |`  
- After:  `| 1  | 4 / 6 | start / progress failure (reserved, not emitted) | TM | errcode |`

## Change characterisation

The delta is a single-cell annotation in the PUS Service 1 subservice table of the Interface Control Document (ICD). It adds the parenthetical qualifier `(reserved, not emitted)` to the name column for Service 1 / subservice 4 / 6 ("start / progress failure").

Verified scope of the delta:
- No source-code change (`src/` untouched).
- No hardware / target / device-tree / board-support change (`board/`, `src/hal`, `src/bsp` untouched).
- No timing, scheduling, bus, network or clock change.
- No FDIR / fault-containment / watchdog / event behaviour change.
- No requirements, traceability matrix, SSS/SRS, or verification artefact change.
- ICD.md is itself the controlled interface document for this build; the annotation clarifies existing interface semantics rather than creating or removing a wire behaviour.

Gates reported as passing: `make build` exit 0 (0 warnings), CTest 56/56, Lean `lake build`, `make trace` PASS (120/76/30/14/68), format-check PASS, license OK.

## Findings

| Finding ID | Classification | File:line | Blocks commit/push? | Finding |
|---|---|---|---|---|
| SYS-R1/R3-001 | Minor | docs/interfaces/ICD.md:106 | No | ICD annotation added "(reserved, not emitted)" to Service 1 subservice 4/6. This is documentation/interface annotation only. No wire behaviour, code, hardware, timing, or FDIR is created, removed, or altered. No system-level risk is introduced. |
| SYS-R1/R3-002 | Minor | docs/interfaces/ICD.md:106; src/ (no match) | No | Confirmed no source-code emission path for S1(1,4/6) in `src/`. The annotation is consistent with the implemented behaviour ("not emitted"). No GNC, safety, FDIR, or operations interface is impacted. |
| SYS-R1/R3-003 | Minor | docs/interfaces/ICD.md:106 | No | NC-PAL-01, GAP-A5, OBCP engine, NC-EXT-19/90, N22-TS5-47, and any PAL/target/hardware item are not touched. No device-tree, BSP, driver, or FPGA/processor state changes. |
| SYS-R1/R3-004 | Minor | docs/interfaces/ICD.md:106 | No | Cross-disciplinary consistency is preserved. ICD.md is the interface document for this row; no SRS/SSS, traceability.yaml, requirements, ICD of another discipline, or verification artefact requires a corresponding change. The annotation does not alter allocated functions, budgets, partitions, failure hypothesis, or interface authority decisions. |
| SYS-R1/R3-005 | Minor | docs/interfaces/ICD.md:106 | No | No systems finding blocks commit/push. All applicable gates pass; the change is within the documentation/interface annotation scope already reviewed and accepted by the project. |

## Systems assessment against review questions

1. **System-level risk of the annotation:** None. The change narrows the documented meaning of an existing reserved subservice code point (clarifying it is reserved and not emitted). It neither opens nor closes a wire path, changes a failure condition, alters a timing budget, or modifies a fault-containment boundary. Classification: Minor, non-blocking.

2. **NC-PAL-01 / GAP-A5 / target/hardware impact:** None. The annotation touches no PAL abstraction, hardware abstraction, board support package, device tree, processor/FPGA/memory configuration, or EEE-part selection. Classification: Minor, non-blocking.

3. **GNC / safety / FDIR / operations interface impact:** None. No control law, sensor/actuator chain, command authority, fault detection/isolation/recovery behaviour, watchdog, event catalogue, maintenance mode, or ground/crew intervention assumption is changed. Classification: Minor, non-blocking.

4. **Cross-disciplinary consistency:** Consistent. The ICD is itself the controlled interface document for this table; the annotation documents an existing reserved/not-emitted code point. No requirements, SRS/SSS, traceability, other-discipline ICD, verification evidence, or interface-authority decision (e.g., NC-EXT-18 wire-code allocation) requires a companion change. Classification: Minor, non-blocking.

5. **Systems findings blocking commit/push:** None. All gates pass; the review identifies no open safety, security, reliability, configuration, or interface risk attributable to this change.

## Recommendation

From the avionics/systems perspective (R1/R3 profile), the change is acceptable: documentation/interface annotation only, cross-disciplinarily consistent, no system-level risk, no blocking finding.

**Commit/push: permitted** — provided the project's remaining applicable gates (N7 sequencing, documentation review N17, PA N20 where required) are completed by the designated authorities. This review does not waive any separate safety, security, regulatory, contractual, or release gate, and does not close any NC or approve any risk.

## Revision history

| Version | Date | Author/role | Description |
|---|---|---|---|
| 1.0 | 2026-11-12 | Senior Avionics Systems Engineer (R1/R3) | Initial independent systems review of ICD.md:106 annotation |
