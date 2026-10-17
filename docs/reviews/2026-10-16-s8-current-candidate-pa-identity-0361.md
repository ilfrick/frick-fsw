# Bounded PA identity review 0361 — current S8 candidate

**Date:** 2026-10-16  
**Scope:** 49-file selected product-set manifest and associated retained local evidence.

## Result

PA independently recomputed the manifest inventory and product-set digest. All 49 paths are unique/present, all per-file hashes match, and the digest matches `1a844759a3857cca64ac51ddf63820887cab44881ed2e55b42cd8bb4c46fe8e4`. The manifest's HEAD, CMake cache and executable hashes match the observed working tree. This is a bounded identity check; PA 0345's earlier 23-file conclusion was not transferred.

The identity-cycle `make ci-local` log reports 43/43 CTest, build, Lean and trace PASS. Separate ASan, smoke, GSW/license, pinned format/docs, Renode and RTEMS cross-build logs report their respective passes. The failed combined nested-container attempt is retained and does not replace the separate successful runs. The CI log does not itself bind the run to cache/executable hashes; configuration-to-run provenance remains a limitation.

## Open limitations

- NC-EXT-84 remains OPEN: generated-tree identity is not generator/tool provenance or reproducible generation.
- The digest excludes planning/status, reviews, evidence and generated build outputs and is not a whole-repository snapshot.
- Renode is emulator evidence; RTEMS cross-build is not target runtime or hardware qualification.
- NC-EXT-88/89/90, authority/profile inputs, OBCP mission-use gaps, hosted CI and release gates remain open.

**Disposition:** Listed-file identity is verified within scope. No N20 phase/release approval, qualification, compliance, target suitability or NC closure.
