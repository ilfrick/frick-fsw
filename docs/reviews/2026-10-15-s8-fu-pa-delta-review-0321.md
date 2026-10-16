# S8-FU product-assurance delta review (FRICK-REVIEW-0321)

| Field | Value |
| --- | --- |
| Profile | Independent Senior SW Product Assurance Engineer (PA) |
| Date | 2026-10-15 |
| Scope | Current S8-FU product/evidence delta at HEAD `13ebf92` plus uncommitted changes |
| Verdict | **Bounded evidence accepted with findings; N20 phase review NOT PASSED** |

## Bounded assessment

Candidate test and build evidence is explicitly associated with the uncommitted
worktree rather than hosted run `38793643786`. The records distinguish Renode
emulation from physical hardware and RTEMS cross-build from target runtime. The
GSW external ground E2E/profile/correlation gap and NC-EXT-07 timeout/recovery
and target limits remain visible. NC-S4c-04 remains open pending independent
review/integration; no basis was found to close it.

## Findings

1. **Moderate — NC-EXT-01/02 closure not owner-authorized.** FRICK-REVIEW-0316
   states its bounded closure recommendation is conditional on the authorized
   project NC owner recording that scope. At review, sprint-plan §3 and README
   stated these NCs closed. Change the active register/summaries to “V&V accepted
   bounded evidence; NC-owner disposition pending” until that authorized action
   is recorded.
2. **Moderate — candidate effectivity is not fully reproducible.** The current
   verification/review records correctly admit no complete immutable source,
   configuration, artifact and tool-input identity. This remains under
   NC-EXT-29. Capture a bounded candidate snapshot/manifest including source
   digest/file inventory, relevant configuration/artifacts, and observed tool
   or container identities, while preserving the mutable-input limitation.
3. **Minor — TS-2 scope omits SRS-REQ-017.** Case N22-TS2-81 cites SRS-REQ-017
   but TS-2 §1.1 did not include it. Add the requirement to the package scope or
   correct the case mapping; reconcile trace/test-spec summaries.

## Assurance disposition

The findings concern scope and evidence assurance; no flight/software safety
approval or risk acceptance is made. NC-EXT-03 and NC-EXT-07 remain open.
Human NC owner and PA/release authorities retain all approval decisions.
