# T21 R2 file-format delta review — FRICK-REVIEW-0240

| Field | Value |
| --- | --- |
| Reviewer profile | R2 — Senior SW Verification & Validation engineer (independent) |
| Reviewer session | `ses_ec8dd2fe7ffehNKMB4uHTnMyLP` |
| Reference | Mutable candidate worktree; author correction followed review |
| Date | 2026-10-13 |
| Scope | Format-specific header comments, shebang handling, managed suffix matrix, T21 status |
| Norms | N8, N10–N13, N15–N16 |

## Verdict

**NOT APPROVED for T21 qualification.** NC-EXT-44 remains OPEN in S8-FU; fresh
independent R2 review is required after the latest correction.

## Finding — OPEN under NC-EXT-44

Shebang lines were unconditionally permitted in every managed format. A `.c` or
`.svg` file beginning with a shebang followed by an otherwise matching SPDX
declaration was accepted although the shebang is not a valid comment header for
those formats. Limit shebang handling to supported script formats and test the
full managed-format suffix/name mapping.

## Checks and limits

The checker and 8 focused tests passed during review; direct probes exposed the
false acceptance. Author changes after this review are not covered. This is not
legal authority, compatibility evidence, release approval, or NC closure.
