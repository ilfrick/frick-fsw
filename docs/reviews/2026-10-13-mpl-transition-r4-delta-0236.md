# MPL transition R4 delta review — FRICK-REVIEW-0236

| Field | Value |
| --- | --- |
| Reviewer profile | R4 — Senior SW documentation engineer (independent) |
| Reviewer session | `ses_ec8eaa5acffeZE5GWzvfPU2X8h` |
| Reference | Mutable candidate worktree at `57974d2` |
| Date | 2026-10-13 |
| Scope | Active license wording, N13 traceability statement, superseded ADR-0011, sprint and status consistency |
| Norms | N8, N11–N13, N16–N17, N19 |

## Verdict

**NOT APPROVED for the requested documentation delta.** NC-EXT-43 remains open.

## Findings — all OPEN, S8-FU

1. Active `NOTICE` says the project files “are distributed under” MPL-2.0 while
   ADR-0017 prohibits representing the candidate as an authorized operative
   grant before NC-EXT-43 is resolved; CFF also declares MPL without a candidate
   note. Reconcile current statements and block release claims.
2. `docs/rules.md` says norms are covered by the traceability manifest, but N13
   has no manifest entry. Clarify that N13 is project governance enforced by
   the license CI check; do not invent an SRS/SSS requirement.
3. Superseded ADR-0011's historical decision body now says authored files used
   MPL-2.0, contradicting the decision and pre-transition evidence. Restore the
   accurate historical AGPL identifier in that decision record.

## Limits

Read-only review of a mutable candidate. No rights-holder authority, legal
conclusion, full documentation-set approval, or NC closure is established.
