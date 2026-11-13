# V&V R2 Correction Review — S8-PUS-FU S1 Final Inventory Preview

| Field | Value |
|---|---|
| Review ID | VV-R2-20261113-S8-PUS-S1-FINAL-INVENTORY-DELTA |
| Date | 2026-11-13 |
| Subject | DOC-R4-FINAL-01 / VV evidence-integrity Major — current pre-final inventory snapshot |
| Baseline | Current working tree; preview generated before this review and before subsequent inventory-correction reports |
| Scope | `r4/assets-outside-paths.txt`, `r4/assets-preview.sha256`, `r4/assets-preview-generation.log`, recursive `r4/` file types, and current README/results/audit/review index; inventory and report-set completeness only |
| Verdict | **CHANGES REQUIRED — evidence-integrity Major remains open.** Preview hashes and its generated-snapshot contents were checked, but current inventory/report additions are not covered, preview is not the active manifest, and final manifest generation/check/attestations remain outstanding. |

> **Authority and limits.** This is an AI-assisted, bounded R2 analysis. It is not human independence authority, N10 tool qualification, N22 test adequacy, NC closure, acceptance, qualification, release, or risk approval. No product docs, active manifest, or index were modified; this report is the sole created file.

## 1. Method

Inspected the declared outside-path list, 157-line preview, generation log, and all files/types under `r4/`; independently compared listed paths with the recursive package inventory and explicit outside paths; recomputed SHA-256 for each listed path. Reviewed the current r4 README integrity statement, S1 section of `docs/vv/verification-results.md`, loop-audit N8/N9/N11/N17/N20, and top review-index entries. Did not regenerate any manifest, run package-level `sha256sum -c`, execute tests, or change the active `assets.sha256`.

## 2. Results

### 2.1 Preview and filesystem checks

- `assets-preview.sha256` contains **157 unique entries**. Every listed path exists as a regular file, and all 157 SHA-256 values recompute correctly. The 37 paths in `assets-outside-paths.txt` all exist as regular non-symlink files and are present in the preview with matching hashes.
- No preview entry contains a `build/` path. Recursive inspection found 122 regular files under `r4/`, including active `assets.sha256` and `assets-preview.sha256`; there were no symlinks or special files (directories are ordinary package structure).
- Relative to the *current* `r4/` plus the 37 explicit outside paths, the preview is short by exactly `r4/assets-preview.sha256`: the current complete set would be 158 paths if every current regular file except active `assets.sha256` is expected. The generation log explains that the preview is copied into `r4/` after generation, avoiding an impossible self-hash. Thus 157 is a defensible **pre-preview-file-generation snapshot**, not an exact inventory of the current tree. The log and README say the preview file itself must be covered by the eventual final manifest; that remains essential.
- The list has no duplicate entries and no current missing/nonregular explicit outside paths. Its 37 entries include the candidate’s principal unchanged build/test inputs (`CMakeLists.txt`, `Makefile`, CMake RTEMS toolchain, `config/fsw_config.h`, `tests/CMakeLists.txt`, relevant test/source files and generation/trace scripts), as well as candidate identity/requirements and current control records. The candidate patch and baseline HEAD provide the declared delta/base identity; this is not a claim that the selected paths reproduce every toolchain or build dependency.

### 2.2 Current additions and substantive review set

The preview captures the final substantive R4 report (`2026-11-13-r4-pus-s1-final-substantive-documentation-review.md`) and the completed R2 scope-method report (`VV-R2-20261113-s8-pus-s1-manifest-scope-method-review.md`). It also includes the indexed S1 initial/correction R1/R2/PA/R4 reports and completed R1/R3 correction/synchronization confirmations present in the declared list.

However, the current working tree/index now also has inventory-correction reports not in `assets-outside-paths.txt` or this preview: `docs/reviews/PA-N20-20261113-s8-pus-s1-final-inventory-delta.md` and `docs/reviews/2026-11-13-r4-pus-s1-final-inventory-delta.md` are present; this R2 final-inventory report is being created after the preview. All three are substantive correction-review outputs, not any of the three pre-registered post-manifest hash-attestation exclusions. They must be added to the explicit outside list and bound by the final manifest after their bytes are final. Any further PA/R2/R4 inventory-correction reports produced after this preview likewise need addition before final generation. The pre-indexed attestation paths remain the only intended external report exclusions.

The review index correctly distinguishes substantive reviews from post-manifest hash attestations and the current results/audit text consistently says the active package manifest remains stale/pending. One record discrepancy remains: r4 README says the candidate patch captures **eleven** tracked candidate paths, while the current candidate identity/review records describe a ten-path tracked diff. Reconcile the path count before freezing so the report does not overstate candidate identity coverage.

### 2.3 Active manifest is a separate, stale artifact

The 157-entry preview is not `r4/assets.sha256`. The active manifest remains the obsolete **45-entry** snapshot, including transient `build/` paths, as explicitly reported by the current results, audit, README, and review index. This review did not treat the preview’s successful individual hash recomputation as active-manifest verification or package-integrity closure. The active manifest must be regenerated from the final frozen inventory and checked from repository root; the three exact PA/R2/R4 hash attestations must then bind the same final digest and inventory.

## 3. Findings

| ID | Severity | Finding | Required disposition |
|---|---|---|---|
| R2-FINAL-INV-01 | **Major — remains open** | The preview is a non-final 157-entry generated snapshot, not proof that the current complete package is bound. In the current filesystem it does not include its own preview file, and current inventory-correction reviews (PA and R4 reports already present, plus this R2 report) were produced after it and are absent from the outside list/preview. | Add every completed post-preview substantive correction report to `assets-outside-paths.txt`; freeze the complete inventory and ensure final `assets.sha256` includes the preview file, all intended recursive r4 regular files, all current explicit outside files, and all completed substantive reviews. Exclude only itself and the three exact attestation paths. Independently compare paths/types and run/retain the final checksum check, then obtain the separate attestations. Rebind after any covered-file change. |
| R2-FINAL-INV-02 | Minor | r4 README’s “eleven tracked candidate paths” conflicts with the ten tracked paths reported in the substantive R4 review/current working-tree candidate identity. | Reconcile the count and referenced identity before the final freeze; do not let this discrepancy imply an omitted/extra candidate path. |

## 4. Conclusion and claim boundary

The generated preview is internally hash-consistent for its 157 listed paths, contains no transient `build/` entries, and its 37 explicit outside files currently exist and hash correctly. It represents the pre-generation snapshot declared by its log. It does **not** establish exact current-tree completeness, because the preview itself is intentionally not self-listed and later inventory-correction reports are now present or being produced. Most importantly, the separate active `assets.sha256` remains the stale 45-entry manifest with `build/` paths. Therefore DOC-R4-FINAL-01 / the evidence-integrity Major is **not closed** and no positive package-integrity conclusion is available.

Confidence is **High** for path/count/type/hash comparisons and the current stale-manifest/index status; **Medium** for the future final inventory until frozen and independently reconciled. No N10/N22 qualification, NC closure, or release/acceptance/flight approval is made.

| Rev | Date | Reason |
|---|---|---|
| 1.0 | 2026-11-13 | Bounded R2 correction review of pre-final evidence inventory and preview. |
