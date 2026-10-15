# ADR-0007 - Docker-deployed CI/CD for both forges (norm N6)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Continuous V&V at class B requires documented CI workflows and local checks.
GitHub has Actions; **Gitea has no built-in CI/CD**, so norm N6 mandates
deploying CI/CD via Docker. Local `make ci-local` is a bounded subset, not the
complete CI workflow or the full N15 verification cycle.

## Decision
- Identical pipelines: `.github/workflows/ci.yml` (GitHub) and
  `.gitea/workflows/ci.yml` (Gitea Actions).
- Gitea CI is executed by a self-hosted **act_runner** deployed with Docker
  Compose (`tools/docker/act_runner/docker-compose.yml`). The runner maps the
  `ubuntu-24.04` label to the local V&V image
  (`ubuntu-24.04:docker://frick-vv:local`): the `gitea/act_runner` image is a
  minimal Alpine image with no toolchain, so `:host` jobs cannot build or test.
- A controlled V&V image (`tools/docker/Dockerfile`, `make docker-image` ->
  `frick-vv:local`) provides a common verification environment for humans and
  agents. `make ci-local` runs configure/build, unit tests, Lean and trace in
  the container; it is not the full pipeline. The image carries
  `nodejs`/`npm` so `actions/checkout` executes inside it. Its base image, apt
  package versions and elan installer source are currently mutable; full
  reproducibility is not established and remains open under NC-EXT-35.
- Nightly RTEMS qualification build: `.github/workflows/nightly.yml`.

## Consequences
Common Docker-based jobs and `# evidence:` markers consumed by the traceability
gate. T1 qualification status and environment provenance are controlled in
`docs/vv/tool-qualification.md`; do not infer reproducibility while NC-EXT-35 is
open.

## Traceability
Norms N6/N8/N10; SRS-REQ-020, SSS-REQ-013.

## Amendment history

| Date | Change |
| --- | --- |
| 2026-10-13 | Clarify bounded `make ci-local` scope and distinguish it from hosted CI and the complete N15 cycle (NC-EXT-38). |
