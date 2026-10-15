# Event Envelope Requirements — R3 Delta Review

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0266                      |
| Date                              | 2026-10-14                             |
| Reviewer profile                  | `aeronautics-space-avionics-systems-engineer` |
| Scope                             | Corrective delta to FRICK-REVIEW-0265 event-envelope requirements |
| Baseline                          | Uncommitted working tree at HEAD `57974d2` |
| Norms                             | N3, N4, N8, N9, N14, N15, N16, N21, N22 |

> **Authority statement.** Bounded independent systems-requirements review only.
> This is not approval of mission values, operational risk, safety, qualification,
> or release.

## 1. Verdict

**APPROVED WITH COMMENTS for continued requirement development; NOT APPROVED for
baseline or verification-ready claims.** The two functional definition issues in
FRICK-REVIEW-0265 are substantially addressed by the author delta. The independent
R3 gate N14 is **not passed** for baselining or NC closure because authority
inputs and implementation evidence remain outstanding.

## 2. Findings and dispositions

| # | Classification | Severity | Finding | Evidence | Disposition |
|---|---|---|---|---|---|
| 1 | Non-functional documentation | Minor | Human-readable traceability count prose was stale: it reported 71 SRS/29 SSS while the controlled manifest and trace run report 73 SRS/30 SSS/14 GAP, 117 total. | `docs/requirements/traceability.md` prior current-count lines; `traceability.yaml`; `trace.py` output. | Registered as NC-EXT-77. Current count prose was corrected in rev 0.42; verify through the trace gate. Do not rewrite historical counts. |
| 2 | Open functional requirement-realization comment | Comment | Exactly-once loss accounting still needs detailed counter range, wrap/saturation and reset semantics, and a controlled point for declaring an accepted-but-not-yet-ground-received event lost. The implementation does not yet provide the required independent end-to-end loss counter. | SSS-REQ-006/SRS-REQ-072; `src/core/evs/evs.c`; NC-EXT-76. | Keep within NC-EXT-76; Systems/Operations with FSW/V&V must define the counter/loss-detection semantics before baseline and verify implementation. No duplicate NC created. |
| 3 | Test/status comment | Comment | The revised tests properly distinguish local event-log/notification evidence from ground receipt; numeric-envelope cases remain gap/TBD and must not be upgraded. | TS-5 N22-TS5-73/74; TS-6 N22-TS6-51/52; FRICK-ARCH-0002. | No new finding. Keep test gaps and NC-EXT-47/76 open until approved values and evidence exist. |

## 3. Confirmed delta properties

- Valid EVS API submission, acceptance/queueing, link-backend dispatch
  completion, and ground-receipt boundaries are now distinguished.
- The event-arrival envelope, dispatch service measurement interval, and
  `Lmax` endpoint are explicit, pending approval of values and profiles.
- Exactly-once loss accounting covers rejection before acceptance, bounded
  queue loss, and post-acceptance loss; local log evidence is not represented as
  ground forwarding.
- Each relevant SRS requirement has the single parent SSS-REQ-006 in the
  manifest; trace convergence is reported PASS at 117 requirements.
- Current queue 64, dispatch batch 8, and nominal 1000 ms delay are candidates,
  not approved `Bmax`, `Rmax`, `Dmin`, or `Lmax` evidence.

## 4. Authority inputs and limitations

The repository does not provide approved values for `Bmax`, `Rmax`, `Dmin`,
`Lmax`, service interval, burst/link-availability profile, ground-receipt
boundary, or counter/loss-detection semantics. Those remain Systems/Operations
authority-owned TBDs. No numeric value is inferred or approved by this review.
The implementation does not yet provide the required end-to-end loss counter
and report behavior. No NC, overall phase, safety, or release gate is closed.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.1 | 2026-10-14 | frick-fsw | Retain R3 delta result for the corrected event-envelope requirements; approve continued development only, not baseline or N14 closure. |
