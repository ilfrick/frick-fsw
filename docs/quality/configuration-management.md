# Software Configuration Management Plan (SCMP)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-CM-001                      |
| Revision                          | 0.2                                    |
| Date                              | 2026-10-11                             |
| References                        | ECSS-E-ST-40C, ECSS-Q-ST-80C, `docs/rules.md` (N7/N11/N16) |

## 1. Scope

This plan defines how frick-fsw configuration items are identified, baselined,
changed, and released, and how the configuration is kept consistent across the
two mirrors (GitHub `origin` and Gitea).

## 2. Configuration items

| CI | Location |
| -- | -------- |
| Requirements baseline | `docs/requirements/{SSS,SRS,traceability}.{md,yaml}` |
| Design baseline | `docs/design/`, `docs/architecture/`, `docs/adr/`, `docs/interfaces/ICD.md` |
| Source baseline | `src/`, `config/` |
| Test baseline | `tests/`, `verification/lean/` |
| V&V records | `docs/vv/`, `docs/reviews/` |
| Tooling | `tools/`, `CMakeLists.txt`, `Makefile`, `.github/`, `.gitea/` |
| Quality/assurance | `docs/quality/`, `docs/rules.md`, `docs/tooling.md` |

## 3. Baseline and version control

- The single source of truth is the Git repository; `main` is the integration
  branch. Every change is a commit with a detailed message (norm N16).
- Version identity: commit hash + semantic project version (`project(frick_fsw
  VERSION ...)`). Release tags mark baselines.
- Dual remote: every compliant change is pushed to **both** `origin` and
  `gitea` (norm N7); CI runs on both.

## 4. Change control

- Every change (feature, defect, review NC, requirement delta) follows the full
  cycle (norm N15), the documentation review loop (norm N17) and the
  product-assurance review (norm N20) before commit.
- Non-conformances are recorded and closed per norm N12; review reports are
  configuration items (norm N11).
- The requirements manifest is the traceability gate (norm N4); it converges at
  every commit.

## 5. Status accounting

- `CHANGELOG.md` is the project-level revision history; each engineering
  document carries a `## History` table (norm N16).
- The README status panel (`tools/scripts/metrics.py`) reports coverage and the
  V&V/review semaphores each cycle.

## 6. Maintenance

- Corrective maintenance: defect/NC fixes through the same cycle.
- Adaptive maintenance: requirement/tooling changes through the same cycle.
- Reuse/obsolescence of third-party components (RTEMS, PUSopen, Lean) is
  tracked in `docs/quality/licensing.md` and the ADRs.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial configuration management plan |
| 0.2 | 2026-10-11 | frick-fsw | Review list adds the N20 product-assurance review (PA-07) |
