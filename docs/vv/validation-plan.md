# Software Validation Plan

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-VAL-001                       |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| References                        | ECSS-E-ST-40C (validation), `docs/vv/verification-plan.md`, `docs/requirements/SSS.md` |

## 1. Verification vs validation

- **Verification** answers "did we build the product right?" — against the SRS
  (`docs/vv/verification-plan.md`, results in `verification-results.md`).
- **Validation** answers "did we build the right product?" — against the mission
  need and the SSS. This plan covers validation.

## 2. Validation objectives

| Objective | SSS | Validation activity |
| --------- | --- | ------------------- |
| The middleware provides command & data handling for a small/medium spacecraft | SSS-REQ-003/005/006 | end-to-end demonstration: TC ingest -> PUS handling -> TM on the link (simulator smoke, `ff_gsw.py` ping) |
| Deterministic execution environment | SSS-REQ-010 | analysis + inspection (no heap after init), repeated-run stability |
| Multi-target build | SSS-REQ-011 | demonstration: host + RTEMS cross build |
| Operability by humans and agents | SSS-REQ-012 | documentation deployability check (README/AGENTS quickstart), CI docs job |
| Continuous verification | SSS-REQ-013 | deployed pipeline on both forges |
| Disciplined time | SSS-REQ-016 | tone-discipline demonstration and CUC round-trip |
| Integration of user applications/drivers/stacks | SSS-REQ-017 | reference applications run; HAL contracts usable (M3) |

## 3. Validation methods

Methods: **Dem** (demonstration, e.g. simulator boot, GSW ping), **Inp**
(inspection of deliverables), **MIR** (mission-readiness review at milestones),
**Anl** (analysis where demonstration is not feasible without hardware).

## 4. Validation environment and limitations

- Validation runs on the **host simulator** (same sources) and via the
  **ground station** tool. On-target validation (flight board) is not possible
  in this repository and is tracked as GAP-A5.
- Mission-specific validation (real OPD, ground segment, end-to-end link)
  belongs to the programme, not to this framework deliverable.

## 5. Validation results

Recorded together with verification in `docs/vv/verification-results.md`
(simulator smoke, GSW self-check, dual build). Mission-level validation results
are the programme's responsibility.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial software validation plan |
