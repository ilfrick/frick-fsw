# ECSS Compliance Matrix (deliverables and gaps)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-COMP-001                    |
| Revision                          | 0.4                                    |
| Date                              | 2026-10-11                             |
| Target                            | ECSS software criticality **B** (norm N5) |
| References                        | ECSS-E-ST-40C (software general requirements), ECSS-Q-ST-80C (software product assurance), ECSS-E-ST-10C (system engineering) |
| Gap tracker                       | `docs/quality/class-a-gaps.md` (GAP-A*) |

This matrix lists the software process **outputs and activities** expected for a
software deliverable and maps each to a project artifact or to a documented
gap. It is the answer to "what is missing for an ECSS class-B FSW". Status
legend: **present**, **partial**, **gap**.

## 1. Software engineering outputs (ECSS-E-ST-40C)

| Output | Role | Project artifact | Status |
| ------ | ---- | ---------------- | ------ |
| Software Development Plan (SDP) | planning, lifecycle, organisation, tailoring | `docs/development-plan.md` (+ `docs/roadmap.md`, `docs/tooling.md`) | partial: content present, not titled "SDP" |
| Software System Specification (SSS) / customer baseline | end-item behaviour, top of trace | `docs/requirements/SSS.md` | present |
| Software Requirements Specification (SRS) | software requirements, verifiable | `docs/requirements/SRS.md` | present |
| Requirements verification matrix | SRS -> method -> evidence | `docs/requirements/traceability.yaml`/`.md` | present |
| **Software Design Document (SDD)** | architectural + detailed design | `docs/design/software-design.md` | present (added for this matrix) |
| Interface Control Document (ICD) | TM/TC, link, time interfaces | `docs/interfaces/ICD.md` | present |
| Hardware/OS abstraction interfaces | device/driver integration contracts | `src/hal/*.h`, `docs/architecture/hardware-abstraction.md` | present |
| Code quality (MISRA + style) | coding standard + coding style applied codebase-wide | `docs/quality/misra-c.md`, `docs/engineering/coding-style.md`, `.clang-format` | present (certified MISRA checker = GAP-A4) |
| Security (SDLS) | link-layer security, SA + key management | `src/security/`, `docs/architecture/security-sdls.md` | present (algorithm pluggable; GAP-A14) |
| Software Verification Plan | verification approach and methods | `docs/vv/verification-plan.md` | present |
| Software Validation Plan | validation against mission need | `docs/vv/validation-plan.md` | present (added for this matrix) |
| Verification/Validation report | executed evidence | `docs/vv/verification-results.md` | present |
| V&V tool qualification | tool correctness/traceability/CM | `docs/vv/tool-qualification.md` | present |
| Software User Manual / integration manual | use and integration | `docs/engineering/user-integration-manual.md` | present |
| Operations manual (OPD / TM/TC database) | on-orbit operations | (PUS catalogue in README; no OPD) | gap: GAP-A12 |
| Software Configuration Management Plan (SCMP) | baselines, change control, releases | `docs/quality/configuration-management.md` | present (added for this matrix) |
| Software Product/Quality Assurance Plan | PA/SQA organisation and audits | `docs/quality/product-assurance.md` | partial: plan present, independent QA organisation is a gap (GAP-A3) |
| Software Criticality & hazard analysis / FDIR | safety, failure detection and recovery | `docs/quality/safety-and-fdir.md` | partial (added; on-target fault injection is GAP-A4/A5) |
| Software metrics | process/product measures | README status panel, `tools/scripts/metrics.py` | present |
| Software reuse / COTS analysis | reused components, licence, pedigree | `docs/quality/licensing.md`, ADR-0002/0003/0012 | present |
| Delivery / acceptance (DRDs) | deliverable list and acceptance | this matrix §3; `docs/quality/licensing.md` | partial |
| Software maintenance plan | corrective/adaptive maintenance | `docs/quality/configuration-management.md` §6 | present |

## 2. Product assurance outputs (ECSS-Q-ST-80C)

| Output | Project artifact | Status |
| ------ | ---------------- | ------ |
| Software product assurance plan | `docs/quality/product-assurance.md` | present |
| Configuration management | `docs/quality/configuration-management.md` | present |
| Non-conformance / problem reporting and closure | `docs/rules.md` N12, `docs/reviews/` | present |
| Independent verification | `docs/engineering/independent-review.md`, `docs/reviews/` (R1/R2/R3/R4/PA, norm N20) | present (technical); organisational independence is GAP-A3 |
| Verification tool qualification | `docs/vv/tool-qualification.md` | present |
| Software criticality categorisation | `docs/quality/safety-and-fdir.md` | present |
| Procurement / COTS pedigree | `docs/quality/licensing.md`, ADRs | partial (no supplier audits) |

## 3. Deliverable list (DRD set)

The repository delivers, versioned and traced: `docs/rules.md`,
`docs/tooling.md`, `docs/requirements/{SSS,SRS,traceability}.{md,yaml}`,
`docs/design/software-design.md`, `docs/architecture/**`, `docs/adr/**`,
`docs/interfaces/ICD.md`, `docs/quality/**`, `docs/vv/**`,
`docs/engineering/**`, `docs/reviews/**`, `docs/development-plan.md`,
`docs/roadmap.md`, `src/`, `config/`, `tests/`, `verification/lean/`, `tools/`,
`README.md`, `AGENTS.md`, `CHANGELOG.md`. Every change is committed and pushed
to both mirrors (norm N7) and reviewed (norms N9/N10/N14/N17/N20).

## 4. Residual gaps and dispositions (class A / programme)

Items that cannot be closed by software/documentation alone, or that are
process/mission artefacts, are tracked in `docs/quality/class-a-gaps.md` and
summarised here (this includes the class-A gaps GAP-A1/A2 and the residual
items GAP-A3..A13):

| Gap | Nature | Why not closable here |
| --- | ------ | --------------------- |
| GAP-A1 | deep formal verification (arbitrary-length CRC induction, C embedding) | formal work package |
| GAP-A2 | full PUS-C service coverage (formal + per-service tests) | incremental work package |
| GAP-A3 | organisational independence (SES/QA) | company/programme function |
| GAP-A4 | MISRA, WCET/stack, fault injection, toolchain "lowest terms" | needs target toolchain + hardware campaigns |
| GAP-A5 | on-target qualification / HIL | needs the flight board |
| GAP-A6 | radiation/environmental | out of software scope |
| GAP-A7 | on-target PPS capture, TAI/CCSDS epoch | needs the flight board |
| GAP-A8 | SDP formalisation/tailoring record | process artefact, closable by re-titling/extending |
| GAP-A9 | SDD detailed design completeness for every module | closable incrementally (documentation WP) |
| GAP-A10 | software safety analysis completeness (hazard/FMEA) | needs system hazard inputs |
| GAP-A11 | independent verification of unit tests by a third party | organisational (GAP-A3) |
| GAP-A12 | operations manual / TM/TC database (OPD) | mission tailoring WP |
| GAP-A13 | maintenance & obsolescence plan beyond CM | programme WP |
| GAP-A14 | SDLS cryptographic algorithm qualification, HSM/keystore, IV uniqueness, ground key management | mission/HW/ground artefacts |

## 5. Conclusion

The class-B software baseline is **complete for a framework deliverable**:
requirements, design, interfaces, V&V plan/validation plan/results, tool
qualification, CM, PA plan, safety/FDIR analysis, user manual and metrics are
all present and traced. The SDP content exists (in `docs/development-plan.md`)
and the SDD covers the architecture and module design; per-function detailed
design and the formal/coverage items above are the documented residual
work packages. Nothing in the class-B claim supersedes the customer
classification (norm N5). The remaining items are either organisational
(GAP-A3, A11), hardware-dependent (GAP-A4, A5, A6, A7), formal/coverage work
packages (GAP-A1, A2), or process/mission documents (GAP-A8, A9, A10, A12,
A13), and are documented rather than silently omitted (norm N5).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial ECSS compliance matrix: outputs mapped, gaps GAP-A8..A13 identified |
| 0.2 | 2026-10-10 | frick-fsw | M6: reuse row references ADR-0012 (libm) |
| 0.3 | 2026-10-10 | frick-fsw | M7: security (SDLS) output added; GAP-A14 |
| 0.4 | 2026-10-11 | frick-fsw | Independent verification/review rows add the N20 PA review; review norms N9/N10/N14/N17/N20 (PA-07) |
