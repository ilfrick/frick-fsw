# R4/N17 Final Manifest Integrity Attestation — S8-PUS-FU S1

| Field | Value |
|---|---|
| Report ID | R4-N17-20261113-S8-PUS-S1-FINAL-MANIFEST-ATTESTATION |
| Date | 2026-11-13 |
| Candidate | Frozen S8-PUS-FU S1 package represented by the manifest below |
| Scope | Independent checksum recomputation and path/type inventory comparison for the final r4 package manifest |
| Result | **PASS — 175/175 manifest checks; exact declared path set confirmed** |

> **Scope and authority limits.** This is an integrity/inventory attestation only. It is not the substantive N17 documentation review (recorded separately in `2026-11-13-r4-pus-s1-final-substantive-documentation-review.md` and its correction re-review), nor approval of requirements, content, a baseline, a phase, NC closure, qualification, acceptance, commit, or release. It does not establish test adequacy or product correctness. Qualified authorized humans retain those decisions.

## 1. Manifest identity and commands

From the repository root, inspected the active manifest at:

`docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256`

The independently recomputed SHA-256 of the manifest file is:

`c09c96f867744ade53e2802f6c5979157443e65bd9affe1131fb4602519d9e96`

Commands run from the repository root:

```sh
sha256sum docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
sha256sum -c docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
```

For the independent inventory check, a read-only `python3 - <<'PY' ... PY` check parsed the manifest and `r4/assets-outside-paths.txt`, recursively `lstat`-enumerated r4, and compared the manifest path set with regular r4 files other than `assets.sha256` plus the listed outside paths, less the three named attestation exclusions. It also counted duplicate paths and rejected missing/non-regular outside entries, r4 symlink/special files, and manifest paths containing `build/`.

## 2. Inventory and checksum results

- Manifest entries: **175**; duplicate manifest paths: **0**.
- Recursive r4 regular files other than the manifest: **134**.
- Explicit outside paths: **41**, all unique and regular; duplicate outside paths: **0**.
- Derived required inventory: **175** paths. Set comparison: **0 missing, 0 extra**.
- Symlinks or special files under r4: **0**. Missing or non-regular explicit outside paths: **0**.
- Manifest paths containing transient `build/` paths: **0**.
- Independent `sha256sum -c` from repository root: **175/175 OK**, exit status **0**.

The exact three external exclusions are the pre-indexed post-manifest attestation reports below; none is included as a manifest entry. At inventory-check time the PA/N20 and V&V/R2 reports existed and were excluded as specified; this R4/N17 report was the attestation being created and is likewise excluded:

1. `docs/reviews/PA-N20-20261113-s8-pus-s1-final-manifest-attestation.md`
2. `docs/reviews/VV-R2-20261113-s8-pus-s1-final-manifest-attestation.md`
3. `docs/reviews/R4-N17-20261113-s8-pus-s1-final-manifest-attestation.md`

The manifest itself is the fourth, distinct exclusion from the recursive r4 inventory rule. The three report paths above are the only external report exclusions; the 41 outside paths are otherwise included.

## 3. Bounded conclusion

**Fact:** the manifest file digest matches the expected value, its 175 listed checksums verify, and its exact path set matches the stated inventory/exclusion rule for the inspected frozen package. No missing/extra/duplicate paths, symlinks/special files, or transient build paths were found.

This result attests only bytes and inventory at this check point. Any change to a covered file requires regeneration/rebinding and a new check. This report does not replace or repeat the substantive N17 review; it grants no approval, closes no NC, and authorizes no release or commit.

```text
Claim type: Facts (manifest digest, path/type counts, exclusions, and checksum output).
Confidence: High for the direct hash and filesystem/path checks performed.
Limitations: No substantive documentation review, builds, tests, Lean, trace, Renode, product-correctness assessment, or human approval performed by this attestation.
Recheck trigger: Any change to a manifest-covered file or the declared inventory/exclusion rule.
```
