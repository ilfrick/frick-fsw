# S8-FU documentation delta review (FRICK-REVIEW-0320)

| Field | Value |
| --- | --- |
| Profile | Independent Senior SW Documentation Engineer (R4) |
| Date | 2026-10-15 |
| Scope | Current S8-FU doc delta: status summaries, ICD, review/evidence indexes, active loop audit |
| Baseline | Candidate at HEAD `13ebf92` plus uncommitted working tree |
| Verdict | **CHANGES REQUIRED**; current documentation baseline not coherent |

## Findings

1. **Major — Current handover/README summaries omit the UART delta.** The active
   summaries ended at NC-EXT-01/02/03 and did not mention UART framing, TS2-81,
   review 0319, or NC-EXT-07 limits.
2. **Major — ICD maximum frame wording conflicts with tested complete-packet
   capacity.** ICD §2 called the 1024-octet PUS frame constants maximum frames,
   while TS2-81 exercises a complete 1037-octet TC packet. Clarify payload vs
   complete CCSDS packet sizing.
3. **Major — UART evidence/review not fully registered or revisioned.** Review
   0319 appeared in the short index but not the detailed review log/current
   status; verification-results lacked the UART cycle and revision-history
   update; tool T3 wording did not match the then-recorded run evidence.
4. **Major — Active loop audit did not audit the current candidate.** It named
   only historical HEAD and referred to earlier review/status work rather than
   the UART S8-FU delta and its N17/N20/open-NC boundaries.

## Bounded disposition

The findings are documentation consistency/effectivity issues; no technical or
phase approval is implied. Required action: update current summaries, packet
size distinction, evidence/review registration and histories, and the active
loop-audit candidate baseline; then obtain an independent delta re-review.
NC-EXT-07 remains open, and NC-EXT-01/02 require authorized NC-owner disposition
even though the V&V report accepts bounded host evidence.
