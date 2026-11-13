# Independent R4/N17 Combined-Candidate Pre-freeze Inventory Review

**Date:** 2026-11-13. The reviewer independently confirmed the manifest digest
`cc961a67ea8b35cffb28f2cab11906e1d8ce5a0a47c0b4a64921b18d2d9f67e2`, 287/287
checksum results, and the 288-path/287-entry declared inventory for that
snapshot. This is not the final post-manifest attestation; the documentation
finding below required correction and a new freeze.

**Finding R4-MANIFEST-01 — Moderate:** `docs/vv/verification-results.md` still
described the earlier S1-named attestation paths as the manifest exclusions in a
historical package snapshot section without sufficiently distinguishing them
from the current combined-candidate exclusions. The active combined inventory
uses only the three pre-registered `*-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`
paths. The old S1 attestation reports are present as included historical
evidence. The README's current scope was accurate; the verification-results
historical snapshot wording could mislead readers about the active exclusions.

**Required correction:** Label the old path list and 45/157-entry statuses as
historical pre-UART snapshots; explicitly identify the three current combined
report paths and state that the old reports are included, not excluded. Re-run
the complete candidate manifest and obtain a fresh R4 inventory attestation.

**Verdict:** changes required for the documentation/inventory-status claim;
the independently observed checksums passed for the inspected snapshot. No
global N17 approval, release approval, qualification, or NC closure is implied.
