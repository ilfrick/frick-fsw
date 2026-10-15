# T21 PA review — unreadable managed-file false acceptance — FRICK-REVIEW-0248

| Field | Value |
| --- | --- |
| Reviewer profile | PA — Senior SW Product Assurance engineer (independent) |
| Reviewer session | `ses_ec8bdc748ffeJ63j6uM41eos5w` |
| Reference | Mutable candidate worktree at HEAD `57974d2` |
| Date | 2026-10-14 |
| Scope | T21 checker scope, execution evidence, license authority record, NC-EXT-43/44/45 status |
| Norms | N8, N10–N13, N15–N17, N19–N20 |

## Bounded verdict

**NOT APPROVED for T21 full qualification or release assurance.** NC-EXT-44
remains OPEN in S8-FU. The repository owner has separately authorized
MPL-2.0 for original project-authored content; this review does not alter that
decision or provide legal advice.

## Finding — OPEN under NC-EXT-44

`tools/scripts/license_check.py` silently skipped `UnicodeDecodeError` and
`OSError` in the managed-file pass. A temporary `src/bad.c` containing invalid
UTF-8 and no SPDX header returned no validation errors. Fail closed for
unreadable in-scope managed files and add regression tests. Owner: FSW/tool
maintainer + PA; sprint S8-FU; independent R2/PA re-review required.

## Evidence and limits

PA confirmed the CFF has no machine-readable license field, the README preserves
the earlier AGPL release distinction, and local `make gsw-selfcheck` passed at
the reviewed configuration. Both workflows call `make gsw-selfcheck` and the
standalone checker. Hosted/configuration evidence and immutable identity remain
open under NC-EXT-29. Author correction after this report is not covered.

Review was read-only; no NC closure, T21 qualification, grant authorization,
distribution compatibility, or release readiness is established.
