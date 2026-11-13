# PA/N20 Combined-Candidate Manifest Integrity Attestation

**Date:** 2026-11-13  
**Result:** PASS for inventory and checksum integrity on the observed working-tree snapshot. This is not product approval, risk acceptance, release readiness, or final commit-tree binding.

- Active manifest: `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256`
- SHA-256: `f07c2ca476abc7778ed2b643508f64371482a90891953d89eb39aa865fd21940`
- Manifest entries: **297**. Declared paths: **298** (137 regular files under r4 including the manifest, plus 161 unique outside-list paths). The active manifest is omitted from its own hashes; the three exact pre-registered PA/R2/R4 reports are detached exclusions.
- Independently reconstructed set: **296/296 eligible paths match**, 0 missing, 0 extra, 0 duplicate. No symlink, special, missing/nonregular, or `build/` path was found.
- Root-level `sha256sum -c`: **exit 0; 297/297 OK**.
- Candidate baseline HEAD: `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`; 18 tracked paths match the identity record. The retained patch equals the current tracked diff; SHA-256 `6a5b31c7d32905e9175bab2d1930eac01d10f9a743639d0f0925ff64b65c4b76`.
- The prior S1 manifest/reports and superseded 48f/108e snapshots are included as historical evidence and were not reused.

This is a bounded configuration/inventory attestation only. Any covered-byte change invalidates it. No files were edited by the reviewer; the three detached report contents are intentionally outside this manifest.
