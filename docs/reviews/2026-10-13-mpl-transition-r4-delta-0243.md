# MPL transition R4 delta review — FRICK-REVIEW-0243

| Field | Value |
| --- | --- |
| Reviewer profile | R4 — Senior SW documentation engineer (independent) |
| Reviewer session | `ses_ec8d7e51dffewmlpFXEtK0abF2` |
| Reference | Mutable candidate worktree at `57974d2` |
| Date | 2026-10-13 |
| Scope | Candidate/no-authorization wording, CFF license metadata, workflow descriptions, N13 traceability, NC plan/status |
| Norms | N8, N11–N13, N16–N17, N19 |

## Verdict

**NOT APPROVED for the documentation delta.** NC-EXT-43 remains OPEN; no license
grant, release authorization, or NC closure is established.

## Findings — OPEN under NC-EXT-43 / S8-FU

1. NOTICE and CFF identified candidate status and unverified authority but did
   not explicitly state that no MPL grant was authorized. CFF's `license:
   MPL-2.0` field needs an explicit explanation that it records the requested
   target only, not an authorized grant.
2. AGENTS.md claimed GitHub ran the “identical” Gitea workflow, although the
   workflow files have platform-specific differences. The narrower fact that
   both currently invoke the license checker is supportable.

## Verified and limited

ADR-0011 retains the historical AGPL decision and is marked superseded; N13 is
correctly framed as governance rather than an invented SRS/SSS requirement; the
reviewed status records keep NC-EXT-43/44 and T21 open. The candidate is mutable
and uncommitted; no history integrity or full N17/N20 approval is established.
