# S8 provisional PUS/Scheduler delta assessments — 2026-10-16

These are bounded, independent-context engineering assessments of the current
uncommitted working tree. They are not NC-owner dispositions, certification,
qualification, phase approval, mission-profile acceptance, or release approval.

## Flight-software delta review

**Scope:** Service 3 request-list parser, one-shot semantics, revised TC
subservice registration, test vectors in `test_r2_gap.c`, `test_r3_gap.c`, and
`test_pus_fu.c`. Service 6 and scheduler arithmetic were spot-checked.

**Result:** No functional findings in the reviewed Service 3 delta. The reviewer
confirmed the `N:u8` plus exactly N IDs parser, one-shot reporting independent
of periodic-enable state, first-error retention with continued list processing,
and revised subservice IDs. Focused host tests passed (`test_pus_fu`,
`test_r2_gap`, `test_r3_gap`, `test_pus`, `test_sched`).

**Limit:** The checks verify provisional ADR-0020 behavior only. Mission profile
acceptance, target behavior, and external protocol interoperability were not
established.

## Product-assurance delta review

**Scope:** Candidate identity/effectivity, evidence linkage, public-reference
identity, failure/retest record, and disclaimer boundaries.

**Finding PA-S8-D-01 — initially OPEN; narrowly dispositioned by PA review
2026-10-16 (tracked as NC-EXT-91):** The existing 49-file
selected-product manifest omits `src/core/sched/ff_sched.c`,
`src/core/sched/ff_sched.h`, and ADR-0020. Its digest therefore does not identify
the exact current Scheduler implementation and policy record. The fresh
CI-local pass is evidence of a run but is not correlated by that manifest to an
exact source/policy set, configuration and executable.

**Evidence needed:** A supplemental, scope-appropriate inventory including
changed scheduler source/header, PUS/profile policy, changed tests/specs and
run/configuration/executable references; recomputed hashes/digest; independent
PA verification of the inventory and run linkage.

Other assessment: the ADR disclaimer clearly separates public references from
mission authority. ECSS PDF SHA-256 is recorded; PUSopen and online Linux/RTEMS
documentation are not archived or hashed. No mission approval, target evidence,
or compliance claim follows.

## Documentation delta review

The scoped R4 review identified and the working tree corrected:

- TS1-43's obsolete TC[3,27] stimulus, now recorded as `{N=1, ID=1}` under the
  provisional ADR-0020 profile;
- handover current-action wording, reconciled to the corrected CI-local result;
- sprint-plan sequence guidance, updated to identify NC-EXT-91 as the highest
  registered NC.

The R4 reviewer confirmed TS1/TS5 summary arithmetic and the revised TS1-88
S6 logical-region stimulus. A separate scoped re-review confirmed TS1-43 and
the current CI/handover status and identified the sequence/current-action
corrections above. This does not establish aggregate N17 approval.

## Format-wrapper delta review

An independent bounded FSW review found no functional defect in the revised
`find -print0 | xargs -0 clang-format -i` recipe. It confirmed the source roots
and C/H selection match `format-check`; no-match behavior is not independently
tested. The latest retained pinned-container transcript demonstrates successful
`make format` followed by successful `make format-check`. This is a bounded
functional review, not formal N10 tool qualification or hosted execution.

## Current disposition

- PA independently verified the supplemental 19-file PUS/Scheduler inventory,
  digest, and cited CI log/cache/executable hashes; NC-EXT-91 is closed narrowly
  for this delta only. See
  `docs/reviews/2026-10-16-s8-pus-scheduler-delta-pa-identity.md`.
- The broader S8/OBCP candidate, generated-source provenance, immutable/reproducible
  build provenance, and full source-to-binary chain are not established by the
  19-file delta inventory; NC-EXT-84 and separate gates remain open.
- NC-EXT-14..18 and NC-EXT-90 remain open for mission authority/profile
  disposition and applicable evidence.
- No commit/push, mission-procedure enablement, PUS compliance claim, scheduler
  schedulability claim, or flight/release authorization is made.
