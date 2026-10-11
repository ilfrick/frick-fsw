# Review - Non-derogable Norms N1-N16: conflict analysis

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0002                      |
| Reviewer                          | independent review session (meta-review of the norms) |
| Reviewer profile                  | R1 + R3 (requirements/system quality triad) |
| Reference revision                | pre-commit baseline (2026-10-10)       |
| Items under review                | docs/rules.md (N1-N16)                 |
| Date                              | 2026-10-10                             |
| Scope                             | mutual consistency of the non-derogable norms; no derivation is waived |
| Independence declaration          | reviewer used only the written norm text and the project context |
| Norms applied                     | N1-N16 (with the review provisions N9/N11/N12/N14/N16) |

## 1. Method

Each norm was read literally and checked pairwise against the others, then
against the operational reality documented in `docs/tooling.md`
(hardware availability, Lean model scope, dual push, review agents). Findings
are classified Major / Minor / Comment.

## 2. Findings

| # | Severity | Norms involved | Finding |
| - | -------- | -------------- | ------- |
| 1 | Major    | N1, N3, N6, N10 | The predicate "every generated code artifact verified with Lean" (N1) and "SRS/SSS themselves verified with Lean" (N3) lacks a definition of *scope*: prose requirements, YAML manifests, Markdown, CI YAML and Python/Shell scripts are not Lean-checkable today. Taken literally, N1/N3 are unattainable for non-executable artifacts, creating an internal inconsistency and a tension with N6/N10 (which prescribe V&V *and* tool review for the same artifacts). Resolution used today: formalizable properties get Lean theorems, non-formal evidence gets a documented `lean_note` in the traceability manifest, tools get qualification review (N10). The norms must carry this interpretation explicitly (see interpretation notes). |
| 2 | Major    | N7, N15, GAP-A5 | N15 requires every change to complete the full development/verification cycle "until success"; N7 then pushes only compliant changes. Some cycle steps are physically impossible without flight hardware (on-target RTEMS/HWIL, GAP-A5) or take very long (RSB toolchain). A literal reading blocks all progress for those changes. Resolution needed: the "complete cycle" must be scoped to the gates applicable to the change (with documented proxies, e.g. the RTEMS cross-build proxy and the nighties build), and the class-B baseline fixes it (interpretation notes). |
| 3 | Minor    | N13, N5 | AGPL-3.0 (N13) is the strongest open-source licence; its network copyleft (Section 13) can conflict with future proprietary mission distribution or productisation wishes (N5 deliverability context). The "maximum permitted by reused licences" clause and the documented compatibility analysis (`docs/quality/licensing.md`) mitigate; this is a legal/business tension, not a technical inconsistency. |
| 4 | Minor    | N7, N4, N8, N15 | The gating stack (traceability, documentation-current, full cycle) means a persistently failing gate freezes pushes on both mirrors. Tension between cadence and rigour; mitigated by Docker reproducibility and CI. Not a contradiction. |
| 5 | Comment  | N9, N14 | N9 makes independent review conditional on the applicable criticality; N14 mandates R3 review of all SSS requirements unconditionally. N14 is stricter, not contradictory. Recommend keeping N14 as the SSS-specific minimum. |
| 6 | Comment  | N3, N4 | N3 (backward SSS/SRS generated) and N4 (bidirectional traceability) overlap in intent; the manifest is the single source of truth. No conflict; consolidation prevents drift. |
| 7 | Comment  | N11, N16 | N11 (scripts + review results in repo) and N16 (detailed revision history) partially overlap. Complementary; no conflict. |
| 8 | Comment  | N2, ECSS | ECSS standards are written in English; norm N2 (English only) is compatible with ECSS practice. |
| 9 | Comment  | N1, N13 | Lean and the V&V toolchain are Apache-2.0 build-time tools; using them does not contaminate the AGPL-3.0 distribution of frick-fsw (documented in licensing.md). No conflict. |

## 3. Conclusions and recommendations

- No **contradictory pair** was found: no two norms demand mutually exclusive
  behaviours. The norms are strictly reinforcing (stacked gates).
- Two **unresolved scope ambiguities** (findings 1 and 2) must be pinned down in
  `docs/rules.md` as *interpretation notes* (non-derogable content unchanged) so
  that the rules remain simultaneously satisfiable in every development cycle:
  1. scope of "Lean-verified code" (formalizable properties vs. non-executable
     artifacts and tooling);
  2. scope of the "complete cycle" (gates applicable to the change, documented
     hardware/gate proxies).
- **Recommended dispositions** (norm N12): adopt interpretation notes; keep
  GAP-A5 as the tracked hardware dependency; keep `lean_note` and the tool
  qualification register as the evidence paths for non-Lean artifacts.

## 4. Conclusion

- [x] **APPROVED WITH COMMENTS** - no major finding invalidates any norm;
  findings 1-2 are resolved by the interpretation notes. All notes committed
  with this review (N11/N16).

## 5. Follow-up records (norm N12)

| NC | Severity | Disposition | Status |
| -- | -------- | ----------- | ------ |
| 1  | Major    | interpretation note in rules.md (scope of Lean verification) + kept lean_note/qualification evidence paths | closed |
| 2  | Major    | interpretation note in rules.md (complete-cycle scope, hardware proxies, GAP-A5) | closed |
| 3  | Minor    | recorded in licensing review procedure (N13) | closed |
| 4  | Minor    | recorded in tooling.md (Docker reproducibility) | closed |
