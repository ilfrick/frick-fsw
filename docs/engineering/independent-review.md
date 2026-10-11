# Independent Review Agents (norm N9)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-REVIEW-AGENTS-001                |
| Revision                          | 0.7                                    |
| Date                              | 2026-10-11                             |
| Applicable norms                  | N9, N10, N11, N14, N17, N20            |

## Purpose

ECSS criticality B requires independent verification wherever independence is
called for by the applicable standards (ECSS-E-ST-40C clause 5.x, ECSS-Q-ST-80C
clause on independence of verification and of the QA function). For this
project, every such independent review is performed by an **independent
agent**:

- **independent context**: the reviewer runs in a fresh session with its own
  context; it receives only the material under review plus the applicable
  reference documents. It does **not** inherit the author's conversation
  context, prior reasoning, or decision history, and it does not review work in
  which it participated.
- **profiled competence**: the reviewer agent is instantiated from the profiles
  below and must hold at least the stated skill floor.

This document is maintained and reviewed at every change (norm N8). The
profiles, creation instructions and scope are part of the repository so that
any contributor, reviewer or CI can instantiate the same reviewer reliably.

## Profiles

### Reviewer agents (Senior profiles)

The reviewer agents are five **Senior** persona documents, **versioned in this
repository** under `docs/engineering/reviewer-agents/` (authoritative prompts),
and exposed as OpenCode agents with **`mode: all`** — usable **both as
subagents** (launched by another agent for an independent review) **and as
primary/front agents** (selected directly as the session's main agent) — globally
under `~/.config/opencode/agents/` and per-project under `.opencode/agents/`.
Every review is performed by the matching Senior agent, in an independent
context. Per norm **N20**, the Senior SW Product Assurance engineer (PA)
additionally reviews **every phase product**, and findings are fixed and the
reviews repeated until they pass.

| Profile | Role | Repository persona | OpenCode agent |
| ------- | ---- | ------------------ | -------------- |
| R1 | Senior FSW / real-time / space software developer | `reviewer-agents/aeronautics-space-fsw-engineer.md` | `aeronautics-space-fsw-engineer` |
| R2 | Senior software V&V engineer | `reviewer-agents/aeronautics-space-sw-vv-engineer.md` | `aeronautics-space-sw-vv-engineer` |
| R3 | Senior avionics / systems engineer | `reviewer-agents/aeronautics-space-avionics-systems-engineer.md` | `aeronautics-space-avionics-systems-engineer` |
| R4 | Senior SW documentation engineer | `reviewer-agents/aeronautics-space-sw-documentation-engineer.md` | `aeronautics-space-sw-documentation-engineer` |
| PA | Senior SW product assurance engineer | `reviewer-agents/aeronautics-space-sw-pa-engineer.md` | `aeronautics-space-sw-pa-engineer` |

These repository persona files **supersede the previous inline R1-R4 profile
blocks** (now removed).

## How to create the reviewer agents

All profiles (R1, R2, R3, R4, PA) are instantiated **as independent
agents/sessions**, never inheriting the author's context.

1. Open a **new, empty session** (fresh context) in the AI tool being used.
   - In OpenCode: the matching Senior agent is available **either** as a
     **primary/front agent** (e.g. select `aeronautics-space-sw-vv-engineer` as
     the session's main agent) **or** as a **subagent** (launch it by name from
     another agent, e.g. `aeronautics-space-sw-vv-engineer`); the personas are
     versioned in `docs/engineering/reviewer-agents/` and exposed under
     `~/.config/opencode/agents/` and `.opencode/agents/` (`mode: all`). Otherwise
     spawn a new session and use the persona as the system prompt.
2. Use the persona document (`docs/engineering/reviewer-agents/<agent>.md`) as
   the reviewer's system prompt, add the task block below, and point to the
   repository checkout at the exact revision under review (commit hash).
3. Task block to append:

```text
REVIEW TASK (frick-fsw)
- Reference revision: <commit or ref>
- Items under review: <paths>
- Scope: <what to verify>
- Apply the project's non-derogable norms (docs/rules.md).
- Review against the applicable ECSS references listed in docs/rules.md.
- Produce the report using docs/reviews/review-template.md and store it in
  docs/reviews/ as <YYYY-MM-DD>-<scope>-<reviewer>.md.
- You have no prior context about this work; base every finding on evidence.
```

4. The author of the reviewed change must not answer reviewer questions with
   unrecorded context: clarifications are exchanged through the repository or
   a review comment file so that every decision is traceable (norm N11).

### Tooling notes

- Any current-generation large context LLM can instantiate the profiles; the
  profiles deliberately do not name a specific product so the process is
  product-agnostic and reproducible.
- The review runs **offline from the author's session** to preserve the
  independence of context. The report, once written, is committed to
  `docs/reviews/` and pushed per norm N7/N11.

## When independence is required

| Activity                                          | Profile | Independence |
| ------------------------------------------------- | ------- | ------------ |
| Design / architecture / code change review        | R1      | Required for criticality B book closure; always for class A |
| Requirements / traceability review                | R2      | Required at release milestones |
| SSS (system requirements) review                  | R3      | Required on creation and on every SSS change (norm N14) |
| Test script / V&V tool qualification review       | R2      | Required (norm N10); independent when risk or class A |
| Verification results assessment                   | R2      | Required at functional qualification milestones |
| Documentation set review (completeness/correctness) | R4    | Required on every change before commit/push (norm N17) |
| Product-assurance review of every phase product   | PA      | Required on every change before commit/push; findings fixed and reviews repeated until pass (norm N20) |
| Pre-release / build-for-flight review             | R1 + R2 + R3 + R4 + PA | Required |

Review NC resolution follows norm N12; SSS review findings force the review to
be repeated until closure (norm N14).

A review log is kept in `docs/reviews/README.md`. Every review result is
committed and pushed in the same cycle (norm N11).

## Non-conformance resolution loop (norm N12)

1. Every finding is registered as a non-conformance (NC) in the review report.
2. Resolution of an NC re-applies **all** the project norms to the affected
   scope (N1 Lean, N4 traceability, N8 documentation, N10 re-qualification of
   changed scripts/tools, N11 records) and all affected gates re-run.
3. The delta is re-reviewed (same profile, independent context) and the NC is
   closed only when the disposition, evidence and re-approval are recorded.
4. A REJECTED review blocks commit/push until every Major finding is resolved.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial profiles R1/R2 per norm N9 |
| 0.2 | 2026-10-10 | frick-fsw | Add NC resolution loop (norm N12); add profile R3 (Senior Systems Engineer, norm N14) |
| 0.3 | 2026-10-10 | frick-fsw | Add profile R4 (Senior SW documentation reviewer, norm N17); independence table extended to R4 |
| 0.4 | 2026-10-11 | frick-fsw | Map reviewer profiles to the five Senior OpenCode agents (`~/.config/opencode/agents/aeronautics-space-*`, from `~/agents/*.md`); add SW PA profile (PA) |
| 0.5 | 2026-10-11 | frick-fsw | Neutral agent names (dropped `frick-`); norm N20: independent Senior SW PA review of every phase product (fixed + repeated until pass) |
| 0.6 | 2026-10-11 | frick-fsw | Five Senior personas versioned in-repo (`reviewer-agents/`) + project `.opencode/agents/`; header norms add N20; path harmonised (PA-05) |
| 0.7 | 2026-10-11 | frick-fsw | Reviewer agents exposed with `mode: all` (usable as primary/front agents, not only subagents) |
