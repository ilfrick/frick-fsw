# TS-2 UART characterization mapping re-review (FRICK-REVIEW-0328)

| Field | Value |
| --- | --- |
| Profile | Independent Senior SW V&V Engineer (R2) |
| Date | 2026-10-16 |
| Scope | FRICK-REVIEW-0327 finding: N22-TS2-81 requirement mapping and package summaries |
| Baseline | HEAD `13ebf92` plus uncommitted candidate |
| Verdict | **Finding partly corrected; changes still required for status/count reconciliation** |

## Re-review

N22-TS2-81 now maps to `NC-EXT-07 characterization; no approved SRS oracle`;
it no longer claims SRS-REQ-016 closure. SRS-REQ-016 remains in TS-2 package
scope and on separate HIL gap N22-TS2-79. The disconnected PTY oracle remains
bounded to host status/zero-length behavior with no elapsed-time, RTEMS or
physical-UART claim.

The test table currently contains 48 `covered`, 32 `gap`, and one
characterization-only row. The TS-2 summary and SVA-1 still reported 49
covered/32 gap, and TS-2 §1.2 did not define the new status. Revise the status
convention and reconcile the summaries to 48/32/1. Total 81, nominal/OOB 44/37,
individual/combined 72/9, and SIL/PIL/HIL 76/2/3 remain consistent.

No requirement, target, qualification, N20, or release closure is inferred.
