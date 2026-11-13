# Independent R2/V&V Review — UART-inclusive candidate

**Date:** 2026-11-13. **Disposition:** bounded UART code/test/host-evidence scope
acceptable; candidate-wide integrity is not yet verified. Not N10 qualification,
release acceptance, or approval.

The reviewer verified the UART-local run-assets list (22/22), source/header/test/
CMake identities, and retained test binary. Re-running the focused executable
produced 104 checks, 0 failures; isolated CTest passed 57/57. TS-2 rev 0.13 cases
84–86 provide explicit host API expected results for failed flag restore/no retry,
errno preservation, final-write deadline overrun with emitted bytes, and expired
zero-length rejection with no write. `tool-qualification.md` correctly keeps T3
NOT FULLY QUALIFIED. These results do not establish target readiness, target
timing/WCET, HIL, full SRS-REQ-074 compliance, or NC-EXT-191 closure.

The older r4 package manifest has 11 current-tree checksum failures and is a
historical snapshot; its attestations do not bind this candidate. A complete
combined-candidate inventory/manifest and independent attestations remain a
separate open CM/evidence gate. The local run-assets manifest is not
candidate-wide integrity.
