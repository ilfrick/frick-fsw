# S8-PUS-FU S1 final manifest — independent inventory/hash attestation

| Field | Record |
|---|---|
| Review | VV-R2-20261113-S8-PUS-S1-FINAL-MANIFEST-ATTESTATION |
| Reviewer | Independent V&V/R2 subagent (AI-assisted; not a human authority) |
| Date | 2026-11-13 |
| Snapshot | Repository `/home/nicola/fsw/frick-fsw`; `HEAD=e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`; working tree was not clean. Evidence below is for the observed tree at execution, not a later tree or commit. |
| Active manifest | `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256` |

## Result

**Inventory and hash checks passed for the observed snapshot.** Independently calculated SHA-256 of the active manifest:

```text
c09c96f867744ade53e2802f6c5979157443e65bd9affe1131fb4602519d9e96
```

This matches the expected digest. The manifest contains **175 entries**, with **0 duplicate paths**. Independent exact-set comparison found 134 regular files recursively under `r4/` other than `assets.sha256`, plus all 41 unique regular paths in `assets-outside-paths.txt` = **175 expected paths**. The manifest set matched exactly: **0 missing, 0 unlisted, 0 out-of-scope paths**. No symlinks or special files were found in the recursively inventoried `r4/` tree or on the included paths; no included path is under a transient `build/` path.

The only exclusions, in addition to `assets.sha256` excluding itself, were the three named post-manifest reports below. At check time those report files were absent and not listed in the manifest:

1. `docs/reviews/PA-N20-20261113-s8-pus-s1-final-manifest-attestation.md`
2. `docs/reviews/VV-R2-20261113-s8-pus-s1-final-manifest-attestation.md`
3. `docs/reviews/R4-N17-20261113-s8-pus-s1-final-manifest-attestation.md`

## Commands and evidence

Run from the repository root:

```sh
sha256sum docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
```

Output was the digest recorded above. The independent path-set check parsed the manifest as `64-lowercase-hex + two spaces + repository-relative path`, rejected duplicate entries, read the 41 non-comment lines from `assets-outside-paths.txt`, recursively enumerated `r4/`, removed only the active manifest itself, and compared the resulting union with the manifest path set. It checked regular-file existence, symlink/special-file conditions, path traversal/absolute paths, the three exact absent exclusions, and prohibited any inventory path containing a `build` component. Result:

```text
PASS: 175 unique manifest paths; 134 regular r4 files + 41 outside files; exact-set match; 0 symlinks/specials/build paths; exactly 3 named exclusions absent/unlisted
```

Then, also from repository root, ran:

```sh
sha256sum -c docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
```

**Result: 175 `OK`; exit status 0.** The complete command output was captured at `/tmp/opencode/s8-pus-s1-sha256-check.txt` for the execution; the check reported each listed path `OK`.

## Scope and limitation

This is hash and inventory evidence only. It does **not** establish N10 tool qualification, N22 test adequacy, anomaly/non-conformance closure, acceptance, qualification, release, flight readiness, or approval by any authority. No included file or index was edited for this attestation; only this post-manifest report is created by this action. Any change to a manifest-covered file requires a new freeze/rebind and re-attestation.
