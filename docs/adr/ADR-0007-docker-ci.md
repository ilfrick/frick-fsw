# ADR-0007 - Docker-deployed CI/CD for both forges (norm N6)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Continuous V&V at class B needs the same pipeline to run everywhere. GitHub has
Actions; **Gitea has no built-in CI/CD**, so norm N6 mandates deploying CI/CD
via Docker.

## Decision
- Identical pipelines: `.github/workflows/ci.yml` (GitHub) and
  `.gitea/workflows/ci.yml` (Gitea Actions).
- Gitea CI is executed by a self-hosted **act_runner** deployed with Docker
  Compose (`tools/docker/act_runner/docker-compose.yml`). The runner maps the
  `ubuntu-24.04` label to the pinned V&V image
  (`ubuntu-24.04:docker://frick-vv:local`): the `gitea/act_runner` image is a
  minimal Alpine image with no toolchain, so `:host` jobs cannot build or test.
- A pinned V&V image (`tools/docker/Dockerfile`, `make docker-image` ->
  `frick-vv:local`) reproduces every verification environment for humans and
  agents (`make ci-local` runs the pipeline in the container). The image carries
  `nodejs`/`npm` so `actions/checkout` executes inside it.
- Nightly RTEMS qualification build: `.github/workflows/nightly.yml`.

## Consequences
Reproducible evidence everywhere; jobs carry `# evidence:` markers consumed by
the traceability gate; the Docker environment is a qualified V&V tool (T1,
see `docs/vv/tool-qualification.md`).

## Traceability
Norms N6/N8/N10; SRS-REQ-020, SSS-REQ-013.