# R2/V&V Combined-Candidate Manifest Integrity Attestation

**Date:** 2026-11-13. **Result:** PASS for the observed working-tree inventory and byte integrity only; not product acceptance, N10 qualification, release approval, or NC closure.

- Active manifest SHA-256: `f07c2ca476abc7778ed2b643508f64371482a90891953d89eb39aa865fd21940`; **297 entries**.
- Reconstructed inventory: **298 declared paths** = 137 regular files under r4 (including manifest) + 161 unique outside-list paths. The manifest excludes itself. The three pre-registered PA/N20, R2/V&V, and R4/N17 reports are the only detached report paths. Exact eligible set: 297 paths, no missing/extra/duplicate entries.
- No symlinks, special files, missing/nonregular paths, or `build/` paths found. All 18 tracked candidate path hashes match the identity record. Baseline HEAD: `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`; patch hash `6a5b31c7d32905e9175bab2d1930eac01d10f9a743639d0f0925ff64b65c4b76` matches the tracked diff.
- Root-level `sha256sum -c`: **exit 0; 297/297 OK**. Captured stdout identity: 297 lines, 25,282 bytes, SHA-256 `ef3510ec3f86fac4ed85e37b2d2a432c974cb14d61a52829d323ffa59dc5aa61`.
- S1/48f/108e reports and manifests are included as historical evidence only; they are not used for this attestation.

This attests only inventory/identity/checksum. No files were edited by the reviewer; this report is an exact-path detached exclusion, and any covered-byte change requires a new manifest and review.
