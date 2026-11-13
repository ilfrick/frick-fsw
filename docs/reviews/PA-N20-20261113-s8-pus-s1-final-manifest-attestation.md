# PA / N20 — S8-PUS-FU S1 final package-manifest integrity attestation

| Field | Value |
| --- | --- |
| Review ID | PA-N20-20261113-S8-PUS-S1-FINAL-MANIFEST-ATTESTATION |
| Reviewer identity | PA / N20 independent-assurance subagent (AI); not a human approval authority |
| Review date | 2026-11-13 |
| Candidate | Frozen S8-PUS-FU S1 candidate, evidence package `r4` |
| Active manifest | `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256` |
| Review scope | Independently recompute active-manifest digest; compare manifest path set to the declared package inventory and exclusions; check path types and build-path exclusion; verify listed file hashes from repository root |
| Out of scope | Substantive N20 product/process assessment, V&V adequacy, tool qualification, anomaly disposition, NC closure, qualification, acceptance, release, and flight approval |

## Result

**Integrity checks passed for the observed working-tree snapshot.** The independently recomputed SHA-256 of the active manifest is:

```text
c09c96f867744ade53e2802f6c5979157443e65bd9affe1131fb4602519d9e96
```

This exactly matches the expected digest supplied for this attestation. The manifest contains 175 entries and its path set exactly matches the declared inventory: 134 regular files recursively beneath `r4/`, excluding `assets.sha256` itself, plus the 41 regular paths enumerated in `r4/assets-outside-paths.txt`. The three pre-registered external post-manifest attestation reports are excluded from the manifest by design:

- `docs/reviews/PA-N20-20261113-s8-pus-s1-final-manifest-attestation.md` (this report)
- `docs/reviews/VV-R2-20261113-s8-pus-s1-final-manifest-attestation.md`
- `docs/reviews/R4-N17-20261113-s8-pus-s1-final-manifest-attestation.md`

The path-set comparison found **0 duplicates, 0 missing paths, and 0 out-of-scope paths**. The outside-path inventory itself had 0 duplicate entries. The `r4` tree contained 135 regular files including the manifest; it contained no symlink or special-file entries. The manifest contained no `build/` paths. `sha256sum -c` reported **175/175 `OK`**.

## Commands and observed results

Commands were run with the repository root as the working directory. Temporary comparison files were created under `mktemp -d` and removed on exit; no manifest-covered file, index, or other document was edited for this attestation.

1. Independently recompute the active manifest's own digest:

   ```sh
   sha256sum docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
   ```

   Output digest: `c09c96f867744ade53e2802f6c5979157443e65bd9affe1131fb4602519d9e96` (expected digest matched).

2. Count regular files and inspect for symlinks/special entries:

   ```sh
   find docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4 -type f | wc -l
   find docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4 -mindepth 1 ! -type d ! -type f -print
   ```

   Results: `135`; no output from the second command (zero symlink/special entries).

3. Independently compare path sets. The comparison built a sorted list of all regular files beneath `r4/` except the active manifest, added the nonblank/noncomment paths from `assets-outside-paths.txt`, checked duplicate inventory paths, and compared the resulting sorted unique set with paths parsed from the active manifest. Counts/results:

   ```text
   r4 regular files including active manifest: 135
   r4 regular files excluding active manifest: 134
   outside inventory entries: 41
   expected unique inventory paths: 175
   manifest entries: 175
   duplicate expected paths: 0
   duplicate outside-list paths: 0
   duplicate manifest paths: 0
   missing manifest paths: 0
   out-of-scope manifest paths: 0
   manifest paths containing build/: 0
   ```

   The three named post-manifest reports were not present in the outside-path inventory or active manifest and remain excluded as pre-registered attestation outputs. The active manifest itself was excluded from its own inventory. The substantive R4 review is not one of these excluded attestation outputs and is included in the manifest inventory.

4. Verify each listed file hash from the repository root:

   ```sh
   sha256sum -c docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
   ```

   Result: exit status 0; 175 entries reported `OK` (175/175).

## Assurance boundary and limitations

**Fact:** These checks establish that, at the time of execution, the supplied manifest digest matched the expected value, its path set matched the declared inventory and exclusions, and every listed path's current bytes matched its recorded SHA-256.

This is **integrity evidence only**. It is not N20 approval or a substantive product-assurance conclusion. It does not accept risk, close any NC, establish test adequacy or tool qualification, demonstrate qualification or conformity, authorize acceptance or release, or establish flight approval. A later change to any manifest-covered file invalidates the corresponding file-hash result and requires re-verification; a change to the manifest also invalidates this manifest-digest attestation. The report itself is intentionally outside the manifest it attests.

```text
Claim type: Fact (observed integrity-check results)
Configuration: Working-tree snapshot of the frozen S8-PUS-FU S1 r4 package on 2026-11-13
Evidence: Active assets.sha256; assets-outside-paths.txt; filesystem inventory; sha256sum -c output
Confidence: High for the stated snapshot integrity and path-set checks
Detection limits: Does not assess semantic correctness, completeness of requirements/tests, or authenticity beyond local file hashes
Required human authority: Qualified project N20/product-assurance and release authorities retain all approval and risk decisions
```
