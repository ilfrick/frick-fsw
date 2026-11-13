# PA/N20 Review — S8-PUS-FU S1 r4 Preview Inventory Delta

| Field | Value |
|---|---|
| Review date | 2026-11-13 |
| Scope | Read-only assessment of the r4 manifest preview inventory, candidate identity, and current S8-PUS-FU S1 review-report inclusion |
| Candidate baseline | HEAD `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83` plus the 11-path working-tree delta recorded in r4 |
| Bounded verdict | The preview matches the declared inventory for the files present at generation and verifies 157/157 entries against the current tree. It is not the active package manifest and does not establish final package integrity. Final correction-review reports must be added to the outside-path list and final manifest. |

> **Authority and limits.** This is an AI-assisted product-assurance assessment, not approval, certification, qualification, NC closure, risk acceptance, release authorization, or a flightworthiness decision. It does not approve the package or supersede required independent human authority.

## 1. Assessment basis

Inspected the r4 `README.md`, `assets-outside-paths.txt`, `assets-preview.sha256`, `assets-preview-generation.log`, `candidate-identity.txt`, `candidate-working-tree.patch`, `assets.sha256`, `assets-stale-45-entry.sha256`, and current relevant PA, R1/R3, R2, and R4 reports and review index. From the repository root, independently checked the preview with `sha256sum -c`; also compared its path set against regular files under r4 and the explicit outside-path list, checked for transient `build/` entries, and compared candidate identity paths/hashes with the current diff and files. No builds, tests, or product-document edits were made.

## 2. Inventory verification

**Facts observed:**

- The preview contains **157 entries** and `sha256sum -c` reports **157/157 OK** against the current repository tree.
- The explicit list contains **37 unique paths**; all currently exist as regular non-symlink files. No duplicate paths were found.
- The r4 tree currently has 121 regular files other than the active `assets.sha256`. The preview covers every then-existing eligible r4 file and every listed outside file, except the preview file itself. That self-omission is expected: the generation log says the preview is copied into r4 after generation. The preview is now present as a regular r4 file and must be included in the final manifest.
- No preview entry points into `build/`; no symlink or special file was found in the declared r4 scope. The current preview does not include the active `assets.sha256` or any of the three pre-registered post-manifest attestation paths.
- The README and generation log define exactly four final-manifest exclusions: `assets.sha256` itself and these three reports:
  1. `docs/reviews/PA-N20-20261113-s8-pus-s1-final-manifest-attestation.md`
  2. `docs/reviews/VV-R2-20261113-s8-pus-s1-final-manifest-attestation.md`
  3. `docs/reviews/R4-N17-20261113-s8-pus-s1-final-manifest-attestation.md`
  The three report paths are not present in this preview snapshot; their future exclusion is not evidence of a completed final inventory check.
- The preview covers the completed S1-ingestion substantive reports currently identified in the review index and explicit list, including PA-R2/PA-R3 and the initial PA review; R1/R3 initial, correction re-review, and synchronization confirmation; R2 initial, evidence delta, ingestion correction review, and scope-method review; and R4 initial, correction re-review, delta, and final substantive documentation review. No completed in-scope substantive report among those inspected is missing from the preview.
- `candidate-identity.txt` and `candidate-working-tree.patch` identify the same **11 tracked paths** as `git diff --name-only HEAD`. The recorded SHA-256 values for all 11 identity paths match the current files. The identity record separately identifies unchanged production `src/pus/pus.c` via the package evidence; it is not an additional changed candidate path.

## 3. Findings / required follow-up

### PA-INV-01 — Final inventory remains prospective

The preview is an accurate checksum/path snapshot for its generation point, but its own digest is absent by construction and future review output is not yet present. Before final generation, add this report and each completed substantive correction-review report produced after this preview (including the planned R2/V&V and R4 inventory-delta reports) to `assets-outside-paths.txt`; then regenerate the final manifest from the frozen current inventory. Preserve the exact four exclusions above, include `assets-preview.sha256` and its generation log, and independently check that no `build/` paths occur and that the final path set equals the declared scope. Do not treat this preview or its 157/157 check as the final manifest attestation.

### PA-INV-02 — Preserve the historical 45-entry distinction

The current `assets.sha256` and `assets-stale-45-entry.sha256` are byte-identical 45-entry files (SHA-256 `3639fc2b1fe32d1dc08dc47ae63256e2c44587bcda090fc16ca53744a684550b`). They include transient `build/` paths and represent the obsolete prior snapshot, not the declared final inventory. The separately named stale copy preserves that history; neither file is proof of integrity for the current candidate. The preview is a separate 157-entry review aid and is not the active `assets.sha256`.

## 4. Bounded conclusion

**Assessment:** No discrepancy was found between the generated preview path set and the declared inventory at generation, subject to the expected omission of the preview’s own digest and future reports. The preview hashes verify at the time of this review. Current S1 substantive reports and the 11-path candidate identity are accounted for. Final manifest completeness, final exclusions, package-level binding, and post-manifest checksum attestations remain unverified until the correction reports and this report are added, the final manifest is regenerated, and its complete path/checksum set is independently checked.

This conclusion does not clear prior findings, close an NC, qualify a tool, validate test adequacy, or authorize commit/release. No product docs, review index, or manifest were changed; this report is the only file created for this task.

```text
Claim types: Facts (file/path counts, hashes and direct checksum result); assessment (bounded inventory consistency); recommendation (final correction and rebind sequence); open issue (final package manifest and attestations).
Confidence: High for the inspected path sets, candidate path/hash comparison, and 157/157 checksum result; not a confidence statement about final package integrity.
Detection limits: Read-only inspection of the stated r4 snapshot and S1-ingestion review set; no execution or full project-wide review-report inventory.
Required reviewer/authority: Independent PA, R2/V&V, R4, and authorized project/CM authorities for their respective decisions.
Recheck trigger: After all correction reports and index/list updates are frozen and the final manifest has been generated.
```
