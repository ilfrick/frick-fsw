# R4/N17 Combined-Candidate Manifest Inventory Attestation

**Date:** 2026-11-13. **Result:** PASS for the specified inventory, documentation status, and checksum on the observed working-tree snapshot. This is not aggregate N17 approval, qualification, release approval, or NC closure.

- Active manifest SHA-256: `f07c2ca476abc7778ed2b643508f64371482a90891953d89eb39aa865fd21940`; **297 entries**.
- Inventory: 298 declared paths—137 regular files below r4, including the manifest, plus 161 outside-list paths. The 297 eligible manifest paths match exactly; no missing, extra, duplicate, malformed, symlink, special, or `build/` paths were found. Root-level `sha256sum -c` exited 0, **297/297 OK**.
- Candidate identity baseline: `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`; all 18 tracked-file hashes match, and the patch equals the current tracked diff (SHA-256 `6a5b31c7d32905e9175bab2d1930eac01d10f9a743639d0f0925ff64b65c4b76`).
- The corrected loop-audit §12.3 and N8/N11/N20 matrix distinguish prior results as historical; R4-AUDIT-01 is resolved for bounded wording. Full sprint scope remains incomplete; no commit/push disposition is changed. Earlier S1/48f/108e files are included as historical evidence, not active attestations.

This is an inventory/documentation attestation only. The three current combined-candidate report paths are excluded from the manifest and must be bound by the final Git tree. Any covered-byte change invalidates this result. No files were edited by the reviewer.
