# MPL transition R4 delta review — FRICK-REVIEW-0244

| Field | Value |
| --- | --- |
| Reviewer profile | R4 — Senior SW documentation engineer (independent) |
| Reviewer session | `ses_ec8d3b548ffe4U5hKv3ymalUvL` |
| Reference | Mutable candidate worktree at `57974d2` |
| Date | 2026-10-13 |
| Scope | Candidate-only license wording, CFF metadata, CI/test claims, status pointers, actual checker behavior |
| Norms | N8, N10–N13, N16–N17, N19 |

## Verdict

**NOT APPROVED for the documentation delta.** NC-EXT-43/44 remain OPEN; T21 is
not qualified and no license or release authority is granted.

## Findings — OPEN in S8-FU

1. `assets/logo.svg` and `assets/logo-wordmark.svg` contain valid SPDX
   declarations inside XML comments following an XML declaration, but the
   checker stopped at the prolog. `make gsw-selfcheck` therefore failed its
   current-repository policy test. Correct SVG prolog/comment parsing and rerun
   checker/tests; retain T21 unqualified under NC-EXT-44.
2. README title/callout labeled v0.1.0 and the project license MPL-2.0 without
   distinguishing the earlier AGPL-3.0-only release from the unapproved current
   candidate. Clarify both.
3. `docs/status/status.json` omitted the latest R4 review 0243 and pending delta
   review, although the handover/review index recorded it.

## Limits

The review is read-only and scoped; no full N17/N20 approval, legal authority,
distribution compatibility, immutable configuration, or NC closure is
established. Author changes after this review are not covered.
