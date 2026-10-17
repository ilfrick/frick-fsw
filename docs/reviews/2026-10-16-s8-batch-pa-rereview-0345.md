# Bounded PA re-review 0345 — S8 candidate product-set identity

**Date:** 2026-10-16  
**Scope:** NC-EXT-83/84 and manifest `docs/vv/evidence/2026-10-16-s8-batch-candidate-manifest.md`.  
**Conclusion:** Product-set identity accepted for the 23 enumerated files and the locally listed results only. No N20 phase or release approval.

## NC-EXT-83 — accepted narrowly for enumerated local evidence

The review verified all 23 file hashes and independently recomputed the
product-set digest `6a94057d7f3b905f0b73fe54d6b686d3a5b3e5f7a2f972308a0f9e5d4657d035`.
The listed CMake cache, host executable, and three test executables matched.
The enumerated CI-local, smoke, GSW, format, docs, Renode, and ASan-retry log
hashes matched their files and their recorded outcomes. This closes the identity
finding only for those named product files/artifacts and does not create a full
repository snapshot, immutable or reproducible build, or release manifest.

## NC-EXT-84 — remains open

The original review could not reproduce the generated flat-source digest from
the then-current record. A follow-up retained the 221 per-file hashes in
`2026-10-16-n7-generated-flat-file-hashes.txt` and specifies the exact GNU
`sha256sum` serialization; its file hash reproduces the listed aggregate. The
generator/toolchain provenance remains incomplete: the `make flat` invocation
tool/version is not qualified or fully recorded. The new per-file record is
pending focused PA verification; do not interpret the earlier discrepancy as
evidence that source files changed or failed compilation.

No claim is made for target execution, reproducibility, tool qualification,
software qualification, safety acceptance, phase completion or release.
