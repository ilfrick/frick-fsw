# MPL transition PA audit — FRICK-REVIEW-0233

| Field | Value |
| --- | --- |
| Reviewer profile | PA — Senior SW Product Assurance engineer (independent) |
| Reviewer session | `ses_ec8fb5586ffetgFt70x7JgTg3W` |
| Reference | Mutable worktree at HEAD `57974d2`; pre-transition inventory |
| Date | 2026-10-13 |
| Scope | Read-only inventory of license policy, rights/authority evidence, SPDX, CI, dependencies, planning and status implications |
| Norms | N8, N11–N13, N15–N17, N19–N20 |

## Verdict

**OPEN — transition audit only; no license approval, legal conclusion, NC
closure, or release authorization.**

## Findings

1. **Major / severity unassigned — N13 baseline conflict.** `AGENTS.md`,
   `docs/rules.md`, ADR-0011, licensing documentation and two CI workflows
   enforce AGPL-3.0-only. A controlled N13/ADR decision and CI/document update
   are required.
2. **Major / severity unassigned — rights and authority evidence absent.**
   Copyright notices name Ilfrick and inspected Git history shows a single
   `ilfrick` author identity, but neither observation establishes legal
   ownership, contributor status, assignment, or authority to relicense all
   files. Qualified rights-holder/IP-counsel disposition is required.
3. **Major / severity unassigned — third-party/generated boundary unverified.**
   The licensing analysis describes fetched GPL-3.0 N7/ESA ASN.1 material,
   generated output, BSD lwIP, toolchain components, and optional commercial
   PUSopen. Exact distribution/linkage and applicable terms need controlled
   review; third-party files must not receive project MPL identifiers.
4. **Minor — implementation/control coverage gap.** The current check in both
   workflows asserts AGPL text and only excludes Apache SPDX tags; it does not
   verify approved project-file coverage or third-party exclusions.

All findings remain **OPEN** and are assigned to the license-transition work
package in S8-FU pending owner/rights-holder disposition.

## Evidence observed

- 204 files carried `SPDX-License-Identifier: AGPL-3.0-only`; zero MPL-2.0
  references existed before implementation.
- Git author history exposed one `ilfrick` identity; this is not proof of rights.
- Active files named above conflict with the requested target; historical review
  reports record past AGPL checks and should remain historically accurate.

## Limits

Read-only repository assessment. No source-code license scan beyond repository
text, contributor agreements, contracts, external legal review, dependency
license verification, or legal authority evidence was available. No finding is
closed by this report.
