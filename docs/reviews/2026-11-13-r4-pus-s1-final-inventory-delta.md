# R4 Inventory-Correction Re-review — S8-PUS-FU S1

| Field | Value |
|---|---|
| Review ID | DOC-R4-20261113-PUS-S1-FINAL-INVENTORY-DELTA |
| Review date | 2026-11-13 |
| Candidate | HEAD `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83` plus the current working-tree delta and r4 evidence snapshot |
| Scope | Focused N17 correction re-review of DOC-R4-FINAL-01: candidate identity, r4 inventory rule and README, outside-path inventory, generated preview and generation record, current status wording, and substantive-report coverage |
| Verdict | **PARTIAL CONFIRMATION; CHANGES REQUIRED for final package-integrity readiness.** The current preview corrects the identified inventory defects for its snapshot. It is explicitly not the active/final manifest, and future correction-review reports are not yet in its outside-path inventory. |

> **Authority statement.** This AI-assisted review has no engineering, PA, CM, release, or commit authority. It closes no NC, approves no baseline or package, accepts no risk, and makes no sprint-completion claim.

## 1. Review basis

Inspected the eleven-path `candidate-identity.txt` and `candidate-working-tree.patch`; r4 `README.md`, `assets-outside-paths.txt`, `assets-preview-generation.log`, `assets-preview.sha256`, and active `assets.sha256`; current `docs/reviews/README.md`, `docs/vv/verification-results.md`, `docs/status/loop-audit.md`, and `docs/status/handover.md`; and the initial substantive report `2026-11-13-r4-pus-s1-final-substantive-documentation-review.md`. Compared preview paths with the current r4 files and explicitly listed outside paths. No product files, controlled status records, inventory files, active manifest, tests, or build outputs were changed by this review.

## 2. Re-review results

### Candidate identity and evidence boundary — confirmed for stated scope

The candidate patch enumerates eleven tracked paths, matching `candidate-identity.txt`: `CHANGELOG.md`; sprint plan; requirements traceability YAML and mirror; review index; handover; loop audit; TS-1; tool qualification; verification results; and `tests/unit/test_pus.c`. The identity record binds those paths and the patch to the stated baseline. The r4 README correctly distinguishes these tracked candidate paths from the larger evidence/package inventory and excludes unrelated pre-existing untracked material.

The outside-path list contains the changed candidate files and relevant unchanged evidence/build inputs, including `src/pus/pus.c`, `tests/CMakeLists.txt`, `CMakeLists.txt`, `Makefile`, `config/fsw_config.h`, the RTEMS toolchain file, Doxygen inputs, relevant test/trace/Lean/ASN.1 scripts, and `third_party/README.md`. These are appropriate identity anchors for the stated test/build evidence; they do not replace the baseline commit identity. All 37 paths currently listed in `assets-outside-paths.txt` exist as regular files.

### Exact manifest rule and preview — snapshot coverage confirmed, final not established

The current rule is consistent across the r4 README, generation record and current status records: the final package manifest is to contain every regular file recursively under r4 plus every explicitly enumerated regular outside path; its only exclusions are itself and the three exact pre-indexed post-manifest attestation reports. Symlinks/special files and missing/non-regular listed paths are generation errors. The preserved `assets-stale-45-entry.sha256` is historical, not active evidence. Transient `build/` paths are explicitly disallowed; retained copies under r4 are the evidence sources.

The generated preview contains 157 unique entries. It covers all 37 currently explicit outside paths and all current regular r4 files other than the active `assets.sha256` and the preview file itself (120 of 121 such r4 files). The preview's own omission is expected: the generation record says the preview is copied into r4 after its generation and will itself be covered by the eventual final manifest. No preview entry starts with `build/` or contains `/build/`. The preview includes the candidate identity/patch, r4 README and cache, CTest invocation inventory, binary hashes and 55 executable copies, focused test and fresh ELF, raw Renode records, gate logs, and the currently completed substantive review reports identified in the outside-path list.

The preview is a generated inventory aid, not a checksum pass: the active `assets.sha256` remains the obsolete 45-entry manifest described by the original DOC-R4-FINAL-01, and the preview generation record explicitly labels itself non-final. The initial report's 41/45 result describes that obsolete manifest snapshot only; it is not evidence that the preview or final package has been verified. I did not treat the preview as `sha256sum -c` evidence.

### Status wording and review records — current text is prospective

Current handover, verification-results, loop-audit N8/N11/N17 rows, and the review index distinguish the obsolete active manifest from the non-final preview and do not claim final generation or verification. Current wording says the substantive R4 report and R2 scope-method report must/will be included, rather than claiming they are already bound. The earlier loop-audit history row 10.75 uses “included in the manifest”; as a historical revision entry it does not override the current N8/N11/N17 status, but should not be reused as a current-state assertion.

The completed substantive R4 report and completed R2 scope-method report are present in `assets-outside-paths.txt` and in the preview. The review index pre-registers the substantive correction re-reviews from R4, V&V/R2, and PA/N20 as separate reports that must be included in the final manifest. Those reports do not yet exist and are not currently listed in `assets-outside-paths.txt`; this is a required pre-final-generation action, not an allowed exclusion. This report is also outside r4 and is not yet in that list or the already-generated preview.

## 3. Finding and disposition

### DOC-R4-FINAL-01 — partially resolved; remains open for final binding

The preview and its generation record substantively address the original inventory defects for the present snapshot: no transient `build/` entries, broad r4 recursive coverage, key outside candidate/config/build/source inputs, and the currently completed substantive reviews. The prior “is included” wording is corrected in current-state records. This is a partial correction confirmation, not closure of the original finding.

Before final generation, append this report and each completed correction-review report to `assets-outside-paths.txt` (at minimum the pre-registered R4, V&V/R2, and PA/N20 inventory-delta reports), verify the report/index set is complete, then freeze and regenerate the final `assets.sha256` under the exact rule. Ensure that the final manifest includes the preview and generation record as ordinary r4 files, all substantive review reports, and no transient `build/` paths. Independently compare its path inventory to the declared rule and run `sha256sum -c` from repository root. Only then may the three exact post-manifest attestations report the final digest, complete inventory/exclusions and checksum result. Any covered-file change requires a new freeze/rebind and applicable review.

**Disposition: DOC-R4-FINAL-01 remains OPEN.** The preview does not replace the active manifest, establish package integrity, or authorize an attestation. No NC, qualification, release, commit, or sprint status is changed by this report.

```text
Product/configuration and decision supported: S8-PUS-FU S1 candidate; stated baseline plus current pre-final r4 inventory snapshot.
Information item/status: focused R4/N17 inventory-correction re-review.
Claim type: Facts (inspected repository paths and records); assessment; Recommendation (complete the explicit inventory and final independent binding).
Applicable baseline: repository N8/N11/N12/N16/N17/N19/N20; no external-standard interpretation.
Sources/access: repository files and retained evidence; no external sources.
Independent cross-check: candidate patch vs identity list; preview path set vs on-disk r4/outside inventory; review index and current controlled status wording.
Confidence: High for the eleven candidate paths, 157 preview entries, current outside-path existence, and absence of build paths; medium for future final-manifest completeness because future reviews and final generation/check are pending.
Limitations: No product/test/build/Lean/trace/Renode execution; no active/final manifest generation or checksum attestation performed.
Required authorities: authorized project/CM/PA/review authorities as applicable.
Recheck trigger: after all substantive correction reports are completed and explicitly inventoried, before final manifest attestations.
```

## 4. Follow-up re-review — Rev 1.1 (2026-11-13)

Re-read the current review index, verification-results section, loop-audit N8/N9/N11/N16/N17/N20 rows and history, handover, r4 README, outside-path list, generation record, and candidate identity after the latest status edits. Also inspected the R2 candidate-identity delta and confirmed the three exact attestation files are pre-registered but not yet present.

### Snapshot/status distinctions — confirmed, with no final result asserted

- The obsolete 45-entry `assets.sha256`/preserved stale copy and the 157-entry preview are consistently described in current status text as historical snapshots, not the current complete inventory or final integrity result. The preview's generation/check reviews are snapshot-scoped; the 41-path outside list and later report additions supersede its 37-path generation input. The final-generation log records the intended exact rule and expected count/method, but explicitly contains no digest or pass result.
- Current controlled wording assigns the eventual final digest, inventory/exclusion determination, and `sha256sum -c` outcome to the three exact pre-indexed PA/N20, V&V/R2, and R4/N17 post-manifest attestations. Those paths are the only external review-report exclusions, along with the manifest itself. At this inspection the attestation files are not yet present, so no current final-manifest result can be stated. The active `assets.sha256` remains the old 45-entry file; generation-method text is not generation or verification evidence.
- The substantive reports—including the three inventory-correction reports (PA, R2, R4) and the R2 candidate-identity delta—are listed among the 41 outside paths and are intended to be bound by the final manifest. They are not attestation exclusions. The README additionally requires all regular r4 files, including the 157-entry preview, updated preview log, and final-generation log, to be in the final inventory. The four post-preview reports are thereby distinguished from the three later hash-only attestation outputs.

### Candidate identity — snapshot-qualified

The current candidate identity record and patch still name eleven tracked paths, and their exact path set matches the current eleven-path tracked diff. A read-only `sha256sum -c` check at this follow-up reports all eleven candidate-file hashes and the patch hash `OK` (with the known warning for non-checksum metadata lines). This confirms local byte consistency now; it does not itself constitute an independent review. R2-FINAL-INV-02 was resolved in the R2 report for its reviewed snapshot (Rev 1.3). The current handover/audit explicitly require the latest identity rebind to receive an independent R2 recheck after the subsequent controlled-status edits and before freeze. I therefore do not extend the prior R2 conclusion to that later snapshot or claim the independent recheck complete.

### Disposition

The current wording correctly separates historical manifest/preview snapshots, substantive reports intended for inclusion, the exact three attestation exclusions, and the not-yet-reported final integrity result. No contradiction requiring an edit to controlled records was identified within this requested scope. DOC-R4-FINAL-01 remains open for the final frozen inventory, independent path/check verification, and attestation sequence; the latest candidate identity rebind also remains pending the independent R2 recheck recorded in current status. No approval or NC closure is made.

```text
Claim types: Facts (reviewed repository status and read-only identity checksum/path comparison); assessment; Open issue (fresh independent R2 identity recheck and final package attestations).
Confidence: High for current path counts/list membership, current status wording, active-file/preview distinction, and identity checksum results; no conclusion on a future final manifest.
Limitations: No manifest regeneration or package checksum run; no status/index/inventory edits; no builds, tests, Lean, trace, or Renode execution.
Recheck trigger: after the latest candidate identity is independently rechecked by R2 and the exact final-manifest attestation reports are produced against a frozen inventory.
```

## 5. Follow-up re-review — Rev 1.2 (2026-11-13)

Reviewed Rev 1.3 of `VV-R2-20261113-s8-pus-s1-candidate-identity-delta.md` and the current r4 candidate-identity record. R2 reports that, after the status edits and identity regeneration, the exact 11-path sets agree and all 11 file hashes and the candidate-patch digest verify.

This later independent R2 recheck **supersedes Rev 1.1's pending status for candidate-identity consistency**; R2-FINAL-INV-02 is resolved for the reviewed current identity snapshot. This is limited to the candidate path set and identity hashes. It does **not** resolve DOC-R4-FINAL-01 or establish package integrity: the final manifest and its exact PA/N20, V&V/R2, and R4/N17 attestations remain the separate outstanding evidence-integrity result. No approval or NC closure is made.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.0 | 2026-11-13 | Senior SW Documentation reviewer profile | Focused correction re-review of DOC-R4-FINAL-01 against the current non-final inventory preview. |
| 1.1 | 2026-11-13 | Senior SW Documentation reviewer profile | Follow-up after status edits; distinguish historical snapshots, final attestation result, substantive-report inclusion, and latest identity recheck status. |
| 1.2 | 2026-11-13 | Senior SW Documentation reviewer profile | Record R2 Rev 1.3 identity recheck as superseding the pending identity status; retain the separate open package-manifest/attestation issue. |
