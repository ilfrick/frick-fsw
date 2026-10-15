# ADR-0017 — MPL-2.0 project-file license transition (norm N13)

| Status | Owner-authorized for original project code and materials; external/generated distribution scope remains open |
| ------ | ------------------------------------------------------------------------------ |
| Date   | 2026-10-14 |

## Context

ADR-0011 selected AGPL-3.0-only for the repository. On 2026-10-14 the repository
owner authorized MPL-2.0 for original project-authored code and materials
created for frick-fsw, limited to content original to this project. No
additional project authorization is required for that scope. This record does
not independently verify provenance or make a legal determination. Third-party,
generated, mission-supplied, commercial and other externally controlled material
remains outside the authorization and retains its own terms. Existing
distributions and grants are not represented as revoked or altered.

## Decision

- The current tree designates original project-authored code and materials under
  `MPL-2.0` within the owner's stated scope; canonical text is the official
  Mozilla license in `LICENSE`.
- In-scope comment-capable source/configuration/build/test/tool and authored
  asset-source files use `SPDX-License-Identifier: MPL-2.0`. Documentation and
  non-comment formats are covered by `LICENSE`/`NOTICE`.
- Third-party, generated, mission-supplied, and commercial components retain
  their own licenses and notices. The earlier AGPL compatibility analysis is
  not reused as an MPL disposition.
- Historical review reports retain point-in-time AGPL observations. Prior
  license grants are outside the scope of this current-tree metadata update.
- `docs/quality/licensing.md` records the component inventory, official
  license-text source, controlled check, and limits.
- **Open disposition:** NC-EXT-43 tracks third-party/generated-component
  boundaries and compatibility for combined distribution profiles. The owner's
  authorization for original project content is recorded; independent PA/R4
  review and applicable project gates remain open. This ADR is not legal advice
  or an independent provenance finding.

## Consequences

Original project-authored code and materials are owner-authorized for MPL-2.0
under the scope above. Third-party/generated components retain their terms.
Compatibility of combined or generated
distributions remains under NC-EXT-43. Existing project assurance, sprint, and
release gates remain in force.

## Traceability

Norm N13; transition work item NC-EXT-43; supersedes ADR-0011 for original
project-authored code and materials within the owner's stated scope; external
component terms and prior distributions remain unchanged.
