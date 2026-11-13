# R4/N17 Combined-Candidate Manifest Inventory Attestation

**Date:** 2026-11-13  
**Result:** PASS for the specified combined-candidate path inventory and checksum. This is not aggregate N17 approval, release approval, qualification, or NC closure.

- Active `r4/assets.sha256` SHA-256: `108e945d48b5e6bed56add8d1729bfc6bc89aefb51d011d4aa176b16f59b773b`.
- Manifest: **289 unique entries**; repository-root `sha256sum -c`: **exit 0, 289/289 OK**.
- Inventory: **290 declared paths**—137 regular files in r4 including the manifest, plus 153 outside-list paths. Excluding the active manifest and exactly the three pre-registered combined-candidate reports yields the exact 289-entry path set. No missing/extra/duplicate entries, symlinks, special files, or `build/` paths were found.
- Candidate identity: all 18 tracked-path hashes and the patch match the current diff; baseline HEAD is `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`.
- Documentation scope: current combined exclusions are the three `*-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md` paths. Older S1-named attestation reports are included as historical evidence, not excluded. R4-MANIFEST-01 was corrected and re-reviewed; the older 45-entry manifest/157-entry preview are explicitly historical.

This report attests only to the stated inventory/documentation and checksum observations. It does not qualify T3, close findings, or approve product, release, or flight. No files were edited by the reviewer.
