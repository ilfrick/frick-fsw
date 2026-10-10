# Software Product / Quality Assurance Plan

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-PA-001                      |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| References                        | ECSS-Q-ST-80C, `docs/rules.md`, `docs/quality/ecss-compliance-matrix.md` |

## 1. Scope and objectives

This plan states how product/quality assurance is applied to frick-fsw so that
the delivered software is a controlled, verified, documented product at ECSS
criticality B. The technical QA functions are embedded in the non-derogable
norms and the CI/CD gates; the organisational independence required at level A
is a documented gap (GAP-A3).

## 2. Assurance activities

| Activity | Mechanism | Evidence |
| -------- | --------- | -------- |
| Requirements control | norm N3/N4, traceability gate | `traceability.{yaml,md}`, `make trace` |
| Design control | SDD review (R1), docs review (R4) | `docs/design/`, `docs/reviews/` |
| Verification | plan/results, tool qualification (N10) | `docs/vv/**` |
| Validation | validation plan | `docs/vv/validation-plan.md` |
| Formal verification | Lean proofs (N1) | `verification/lean/`, transcript |
| Independence of reviews | independent-context agents (N9) | `docs/reviews/` (R1/R2/R3/R4) |
| Non-conformance control | close-out loop (N12) | review follow-up tables |
| Configuration management | SCMP | `docs/quality/configuration-management.md` |
| Licence/procurement | licence review (N13) | `docs/quality/licensing.md` |
| Safety | criticality/FDIR analysis | `docs/quality/safety-and-fdir.md` |

## 3. Metrics and records

- Code/line/function coverage and requirement coverage: `make coverage`,
  `make metrics`, README status panel.
- All records are versioned and pushed every cycle (norm N11/N7).

## 4. Independence

Technical independence of reviews is provided by independent-context reviewer
agents (profiles R1-R4). Organisational independence (a QA function outside the
development team, SES agenda, QA audits) is not available in this repository and
is tracked as **GAP-A3**.

## 5. Audits

The continuous pipeline (build, tests, Lean, traceability, format, docs,
smoke) is the automated audit; it runs on both forges (norm N6). Manual audits
are the periodic milestone reviews (R1/R2/R3/R4).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial software product/quality assurance plan |
