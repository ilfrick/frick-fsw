# Event Envelope Requirements — Scoped R3 Review

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0265                      |
| Date                              | 2026-10-14                             |
| Reviewer profile                  | `aeronautics-space-avionics-systems-engineer` |
| Scope                             | SSS-REQ-006/SRS-REQ-001, -011, -072; trace links; TS-5/TS-6; NC-EXT-75/76 |
| Baseline                          | Uncommitted working tree at HEAD `57974d2` |
| Norms                             | N3, N4, N8, N9, N14, N15, N16, N21, N22 |

> **Authority statement.** This is a bounded requirements review by the
> configured systems-engineering agent. It is not authorized mission-parameter
> approval, human systems approval, safety acceptance, or release approval.

## 1. Bounded verdict

**NOT APPROVED for baselining or claiming verification-ready requirements.**
The initial draft correctly left all mission/profile values as TBD, but two
functional definition gaps prevented a complete testable contract. The review
does not assess overall phase/release readiness and closes no NC.

## 2. Findings

| # | Classification | Severity | Finding | Evidence | Required disposition |
|---|---|---|---|---|---|
| 1 | Functional requirement definition | Major, provisional | The loss contract did not define accounting for each failure point: EVS-lock contention before local recording, queue-full after attempted recording, and loss after queue acceptance but before ground receipt. “Not queued for forwarding” alone did not cover each failure boundary. | Initial `docs/requirements/SRS.md` SRS-REQ-011/-072; `docs/architecture/software-bus.md`; TS-5 N22-TS5-73 and TS-6 N22-TS6-51. | Define event arrival, acceptance, forwarding completion, and ground receipt; require exactly-once loss accounting across pre-acceptance and post-acceptance failure points; clarify local logs are not ground delivery. |
| 2 | Functional requirement definition | Major, provisional | Arrival, service and latency measurement boundaries were underdefined. “Accept an event-arrival envelope” did not specify whether the event was generated, accepted, or queued; `Dmin` lacked a service observation interval/measurement point; `Lmax` depended on an undefined ground-receipt boundary/link profile. | Initial `docs/requirements/SSS.md` SSS-REQ-006; SRS-REQ-072; TS-5 N22-TS5-74 and TS-6 N22-TS6-52. | Define valid EVS API submission as arrival, acceptance/dispatch/ground boundaries, service interval, and link-availability assumptions. Keep numeric values TBD until authority approval. |

## 3. Supporting observations

- The trace hierarchy under SSS-REQ-006 was valid: SRS-REQ-011 and SRS-REQ-072
  each have one SSS parent; no multiple-parent defect was identified.
- TS-5-71/72 and TS-6-50 have concrete scoped lock/queue oracles. TS-5-73,
  TS-5-74 and TS-6-52 were correctly marked gap/TBD.
- TS-6-51 establishes only local EVS-log/drop-statistics evidence, not ground
  forwarding or full loss-counter/report compliance; that distinction should be
  explicit.
- Queue capacity 64, batch 8 and nominal 1000 ms task delay are candidate
  implementation settings, not approved `Bmax`, `Rmax`, `Dmin`, or `Lmax`.
- The repository provides no approved values for the event profile, service
  interval, link availability, ground-receipt boundary, callback/link WCET, or
  target timing. Authorized numeric values cannot be set from repository
  evidence.

## 4. Conclusion and boundaries

Correct the two functional definition issues before baselining or claiming the
requirements are verification-ready. Numeric parameters remain Systems /
Operations authority-owned TBDs. This is not a finding that blocks unrelated
development under N21. NC-EXT-76 remains open until a controlled delta is
independently reviewed, the authority inputs are approved, implementation and
tests meet the final requirements, and all applicable gates pass.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.1 | 2026-10-14 | frick-fsw | Retain the initial scoped systems requirements review and two functional definition findings under existing NC-EXT-76. |
