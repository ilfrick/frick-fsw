# MISRA C:2012 Compliance

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-MISRA-001                   |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N5 (class B target / class-A gaps), N8, N10 |
| Requirement                       | SRS-REQ-041                            |

## 1. Scope and target

This document defines the MISRA C compliance process for the frick-fsw C sources
(`src/`, `config/`, `tests/`, `include`):

- **Standard**: MISRA C:2012 (including Amendment 1 and Technical Corrigendum 1).
- **Language**: C99 (the project C standard).
- **Target**: MISRA C:2012 compliance with a **documented deviation process**;
  strict compliance is required where a deviation cannot be justified.

## 2. Tooling

| Role | Tool | Notes |
| ---- | ---- | ----- |
| Open-source scanner (CI) | **cppcheck** with the MISRA addon (`--addon=misra`) | free, reproducible, advisory-then-blocking; `tools/scripts/misra.sh` |
| Certified compliance tooling | e.g. Helix QAC, Polyspace, Coverity, LDRA | **required for formal MISRA compliance**; not available here → tracked as a gap (see §5 and `docs/quality/class-a-gaps.md`, GAP-A4) |

cppcheck provides a useful automated subset of MISRA C:2012 but is **not a
qualified MISRA checker**; it is used to prevent regressions and drive
compliance, while certified tooling and manual review close the formal gap.

The scan is **advisory**: `tools/scripts/misra.sh` always exits 0 and writes the
findings + count to `build/misra-report.txt`.

## 3. How to run

```bash
make misra               # runs tools/scripts/misra.sh, writes build/misra-report.txt
```

CI runs the same script. It is **advisory initially** (the report is published,
the step does not fail the build) and becomes **blocking** once the violation
set is driven to the agreed baseline.

## 4. Deviation process

1. Every violation is triaged: fix, or raise a **deviation** with a documented
   justification (MISRA C:2012 permits deviations with a rationale).
2. Deviations are recorded in this document (table below) and reviewed at each
   cycle (norm N12/N8).
3. Suppressions in code use `// cppcheck-suppress <id>` with a justification
   comment, only via the deviation record.

| Id | Rule / directive | Location | Justification | Status |
| -- | ---------------- | -------- | ------------- | ------ |
| (none yet) | | | initial baseline being established | |

## 5. Gaps

- Certified MISRA C:2012 checker + formal compliance report (GAP-A4).
- Full rule set coverage incl. directives requiring design-level review.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial MISRA C:2012 compliance process (milestone M9) |
