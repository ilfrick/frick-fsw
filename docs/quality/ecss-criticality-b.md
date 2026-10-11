# ECSS Criticality Class B Baseline

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-B-001                       |
| Revision                          | 0.4                                    |
| Date                              | 2026-10-10                             |
| Target                            | ECSS software criticality level B (norm N5) |
| Gaps to class A                   | `docs/quality/class-a-gaps.md` (GAP-A1..A14) |
| Compliance matrix                  | `docs/quality/ecss-compliance-matrix.md` |

## 1. Applicable references

- ECSS-E-ST-40C *Space engineering - Software general requirements*.
- ECSS-Q-ST-80C *Space product assurance - Software product assurance*.
- ECSS-E-ST-70-41C *Ground systems and operations - Telemetry and telecommand
  packet utilisation* (PUS-C).
- CCSDS 133.0-B-2 *Space Packet Protocol*; CCSDS 130.0-G.
- RTEMS 6.2 (open-source RTEMS project) and the
  `riscv/mpfs64imafdc` BSP for the Microchip RT PolarFire SoC RTPFS160ZT.

## 2. Class B baseline statement

The framework and its engineering data are produced to be deliverable at ECSS
criticality **B**. This means in particular:

- a full SSS/SRS/documentation set exists and is traced (norms N3/N4);
- verification covers every requirement by test, analysis, inspection or
  demonstration, with recorded results (norms N6/N10/N11);
- verification tools are qualified and configuration-managed (norm N10);
- design/code reviews are performed; items that the applicable standards mark
  as independent verification are reviewed by independent agents (norm N9);
- non-conformances are tracked to closure by re-applying all norms (norm N12);
- configuration management, traceability and the verification evidence are kept
  in the repository and pushed on both mirrors at every cycle (norm N7/N11).

## 3. Engineering data set (current baseline)

| Deliverable | Location |
| ----------- | -------- |
| Non-derogable norms | `docs/rules.md` |
| Approach and tooling | `docs/tooling.md` |
| SSS | `docs/requirements/SSS.md` |
| SRS | `docs/requirements/SRS.md` |
| Traceability manifest + matrices | `docs/requirements/traceability.yaml`, `traceability.md` |
| Software design (SDD) | `docs/design/software-design.md` |
| Architecture decisions | `docs/adr/` |
| Interfaces (ICD) | `docs/interfaces/` |
| Hardware abstraction | `src/hal/`, `docs/architecture/hardware-abstraction.md` |
| V&V plan / validation plan / results | `docs/vv/verification-plan.md`, `docs/vv/validation-plan.md`, `docs/vv/verification-results.md` |
| V&V tool qualification | `docs/vv/tool-qualification.md` |
| Configuration management | `docs/quality/configuration-management.md` |
| Product/quality assurance | `docs/quality/product-assurance.md` |
| Safety / FDIR | `docs/quality/safety-and-fdir.md` |
| Compliance matrix | `docs/quality/ecss-compliance-matrix.md` |
| Independent review process | `docs/engineering/independent-review.md`, `docs/reviews/` |
| Code, tests, formal proofs | `src/`, `tests/`, `verification/lean/` |
| Ground support tool | `tools/gsw/ff_gsw.py` |

## 4. Verification methods applicable

| Method | Code | Used for |
| ------ | ---- | -------- |
| Test | Tst | unit tests, simulator smoke, S17 ping |
| Analysis | Anl | Lean theorems, arithmetic bounds, determinism arguments |
| Inspection | Inp | static analysis, deterministic memory, docs |
| Demonstration | Dem | simulator boot, dual-build, RTEMS cross build |
| Mission readiness | MIR | flight/qualification milestones |

## 5. Qualification claim

Nothing in this document supersedes the customer-criticality classification:
re-classification to a higher category obliges the project to redefine the
verification baseline and close the listed gaps (norm N5).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial class B target baseline |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: class-A gap reference updated to GAP-A1..A7 (R3 review NC-5) |
| 0.3 | 2026-10-10 | frick-fsw | ECSS compliance assessment: engineering data set extended (SDD, CM, PA, safety, validation, compliance matrix); gaps GAP-A1..A13 |
| 0.4 | 2026-10-10 | frick-fsw | M7: gap reference GAP-A1..A14 (SDLS security) |
