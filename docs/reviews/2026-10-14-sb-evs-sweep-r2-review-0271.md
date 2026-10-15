# SB/EVS structural sweep — R2 verification review

**Review ID:** FRICK-REVIEW-0271  
**Date:** 2026-10-14  
**Profile:** Independent R2 software V&V review  
**Scope:** `test_sb_sweep.c`, sweep runner, CMake/Make registration, and retained CSV.  
**Disposition:** Initial review NOT APPROVED for tool qualification; corrected delta is acceptable for exploratory native-host structural characterization only.

## Findings and correction

- **F-01, major:** Dispatch limits originally had exactly the limit number of
  notifications pending, so an implementation ignoring the limit could pass.
  Corrected test cases queue one more event than each limit 1–63, assert exactly
  the limit is dispatched, and then assert the remaining event is dispatched.
  At limit 64, the configured 64-entry queue is filled and drained.
- **F-02, minor:** The initial runner did not robustly reject inconsistent
  capacity/configuration and result data. The corrected runner obtains compiled
  limits using `--describe`, rejects invalid limits before output, checks each
  result against expected parameters/arithmetic, audits the serialized CSV for
  count, uniqueness, capacity, arithmetic, and status, then atomically replaces
  the output.

The final independent delta review reran the sweep and reproduced the retained
CSV byte-for-byte. It confirmed 9,984 unique profiles, 9,985 data records
including dispatch sweep, 1,697,329 assertion checks, zero failures, 2,207
callbacks in the explicit dispatch sweep, and matching hashes as recorded in
`docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.md`.

## Bounded conclusion

The corrected package is adequate for the stated exploratory characterization.
This is not a target timing/WCET result, mission arrival/service envelope,
ground receipt, requirement compliance, NC closure, formal tool-owner N10
acceptance, or release approval. T22 remains restricted accordingly.
