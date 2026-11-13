# PA/N20 Combined-Candidate Manifest Integrity Attestation

**Date:** 2026-11-13  
**Review type:** Independent post-manifest CM/evidence-binding check  
**Result:** PASS for inventory and checksum integrity on the observed working-tree snapshot. This does not establish final commit-tree binding, release readiness, or product approval.

## Manifest and identity

- Active manifest: `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256`
- SHA-256: `108e945d48b5e6bed56add8d1729bfc6bc89aefb51d011d4aa176b16f59b773b`
- Entries: **289**
- Candidate baseline: `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`
- Candidate identity: 18 modified tracked paths; recorded patch matches the current binary `git diff HEAD --binary`; reverse application check succeeds. Patch SHA-256: `c2d023d7dc3fcf5aebdfcd37553559b833c701097e80428b2153acecfab2747b`.

## Inventory and exclusion result

The declared set comprises 137 regular files below r4 (including the active manifest) plus 153 unique outside-list paths: **290 declared paths**. The only exclusions are the active `assets.sha256` itself and the three pre-registered combined-candidate reports named in the review index. Excluding these gives 289 expected hash entries, exactly matching the manifest. Path-set comparison found 0 duplicates, 0 missing paths, and 0 extra paths. No symlinks or special files were found; all external paths are regular files; no included path is under `build/`.

From repository root, `sha256sum -c` returned **289/289 OK, exit 0**. The prior pre-UART S1 manifest and its reports are included as historical evidence and are not reused for this candidate.

## Scope limits

This attestation addresses inventory/configuration integrity only. It does not assess product correctness or V&V adequacy, qualify T3/N10, close an NC, accept risk, or approve release or flight. Any covered-byte change invalidates this attestation and requires a new freeze and review. No files were edited by the reviewer.
