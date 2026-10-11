# ADR-0011 - AGPL-3.0-only project license (norm N13)

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Norm N13 requires the whole project to be open source under AGPL-3.0 or the
maximum permitted by the licenses of any reused code parts. The earlier
Apache-2.0 decision was superseded by the rule.

## Decision
- License: **AGPL-3.0-only**; canonical text in `LICENSE`.
- All authored files carry `SPDX-License-Identifier: AGPL-3.0-only`.
- Compatibility analysis and review procedure:
  `docs/quality/licensing.md` (cFS is inspiration only and Apache-2.0; RTEMS is
  an external BSD/GPL toolchain; PUSopen is an optional commercial adapter not
  distributed).
- CI `docs` job rejects Apache-2.0 SPDX headers and non-AGPL LICENSE text.

## Consequences
Open-source distribution with copyleft obligations; attribution to NASA cFS as
inspiration preserved; commercial PUSopen integration remains possible for
licence-holders via the adapter.

## Traceability
Norm N13; SSS-REQ-012.
