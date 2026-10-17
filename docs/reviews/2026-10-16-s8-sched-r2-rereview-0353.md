# Bounded R2 re-review 0353 — S8-SCH test-oracle corrections

**Date:** 2026-10-16  
**Scope:** `tests/unit/test_sched.c`, TS5-44/45/46, TS6-33 following R2 scheduler evidence findings.

## Disposition

The corrections address the reviewed test-evidence gaps:

- TS5-44 now asserts exact `FF_ERROR` results for invalid table inputs.
- TS5-45 now polls after rejecting a table with different first-entry ID/payload and verifies the prior image's tested first entry is dispatched. Scope language was narrowed to that actual oracle; it does not claim every table entry was compared.
- TS5-46/TS6-33 now use the tested 1000 ms period for the 4500 ms jump and check the emitted message ID, length, payload and time, three missed slots, and next release at 5000 ms.

The focused scheduler test passed 1/1 in `2026-10-16-s8-sched-r2-correction-focused-retry.log`. The first attempt is retained separately and failed because Ninja could not write its build log (`Permission denied`); it is an environment/permissions failure, not a product test failure.

**Finding 0353-01 (minor document control):** TS6 history had duplicate 1.4 entries. They have been combined into one history entry. This report does not establish target timing, schedulability, approved mission policy, or phase/release approval. NC-EXT-19 and the time-range behavior gap remain open.
