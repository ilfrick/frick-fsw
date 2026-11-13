# V&V R2 Delta Review — S1 Evidence Integrity Correction

| Field | Value |
|---|---|
| Review ID | VV-R2-20261113-S8-PUS-FU-S1-EVIDENCE-DELTA |
| Date | 2026-11-13 |
| Baseline | HEAD `e5f01d4` plus current uncommitted candidate/evidence updates |
| Parent review | `VV-R2-20261113-S8-PUS-FU-S1-INGEST`, finding VV-R2-20261113-01 (Major) |
| Scope | Focused delta on newly retained CTest inventory/binaries, isolated Make logs, current r4 README/results/audit wording, and the parent Major finding |
| Verdict | **PARTIALLY CORRECTED; Major remains OPEN pending final package-manifest rebind and independent verification.** |

> This is an AI-assisted independent analysis artifact. It has no approval authority, closes no NC, qualifies no tool, and does not approve commit, release, or flight readiness.

## 1. Review method and limits

Read the current r4 README, CTest artifact inventory, per-binary hash list, manifest, updated S1 section of `verification-results.md`, current loop-audit N12/N11/N14/N20 entries, and the current correction-review status. Ran the r4 package `sha256sum -c` from repository root. Independently ran the per-binary checksum list from `r4/artifacts/`, compared each retained executable copy with its corresponding executable in `build/evidence-20261112-pus-s1-r4/tests`, and counted/inspected the CTest JSON inventory. No product files were modified. No test, build, CTest, or Renode execution was performed.

## 2. Delta findings

### 2.1 New CTest inventory and binary retention — confirmed for the inspected files

- `artifacts/ctest-list.json` contains **57 CTest entries**. The command list represents 55 distinct compiled test executables plus the two non-binary command entries (the Python `sb_evs_structural_sweep` wrapper and shell `renode_vv_verdict` test). This is consistent with 57 registered CTest tests and the reported 55 distinct executables.
- `artifacts/ctest-inventory-summary.txt` states the scope and invocation of the separate binary-check file.
- `artifacts/ctest-binaries.sha256` contains **55 entries**. Running `sha256sum -c ctest-binaries.sha256` from the `artifacts/` directory returned **55/55 OK**.
- I independently compared the hashes of all 55 retained copies with the corresponding 55 executables in the r4 build's `tests/` directory; all matched. The retained `artifacts/test_pus` and `artifacts/frick_fsw_renode.elf` identities agree with the reported focused test / fresh ELF identities at the time of review.

This is a useful corrective improvement: it retains the actual CTest executable bytes rather than relying only on a CTest summary. It does not itself prove that every binary was invoked in the recorded CTest run unless the test-list/log association is preserved; the JSON, `gate-ctest.log`, CMake cache, and exact-byte comparisons provide that linkage for this bounded record. The 55-entry hash list is a submanifest; its own content and all retained copies still need inclusion in the package-level manifest for end-to-end package integrity.

### 2.2 Current record status and N12 — corrected in substance

The r4 README now explicitly states that the package manifest must be regenerated and independently verified after the final candidate/evidence/review set is frozen, and that package-level checksum verification must not be claimed before then. The current `verification-results.md` explicitly says the manifest is **stale and not verified**, and describes the correction reviews by snapshot. The current loop-audit N11 says the same manifest is stale; N12/N14 now correctly distinguish the completed R1/R3 correction and synchronization confirmations from unrelated open findings. I confirm that this corrects the earlier status ambiguity: SYS-R1R3-01 is reported resolved in substance by the bounded independent review, while NC-EXT-240..244, PA/R2/R4 correction findings, and final package verification remain open as applicable. No NC is closed by that distinction.

The earlier R4 correction report is from a prior snapshot and reports a stale N12/N14 mismatch; the present loop-audit text now reconciles it. Its report remains useful historical evidence and is not itself a current approval.

### 2.3 Package-level manifest — Major finding not yet resolved

The current `r4/assets.sha256` still has 45 entries and fails four listed hashes:

1. `docs/reviews/PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`
2. `docs/reviews/README.md`
3. `docs/status/loop-audit.md`
4. `docs/vv/verification-results.md`

The manifest also does not contain entries for the newly added `r4/artifacts/` files, including `ctest-list.json`, `ctest-binaries.sha256`, the 55 executable copies, `artifacts/test_pus`, and the copied Renode ELF; nor does it include the successful isolated Make logs `gate-make-build-isolated.log` and `gate-make-test-isolated.log`. The README accurately discloses that the manifest is stale/not verified, but the assets are not package-bound until the manifest is regenerated. The independent 55/55 binary submanifest check does not substitute for that package-level binding.

**Disposition of parent finding VV-R2-20261113-01: OPEN.** The root cause is now transparently disclosed, and the specific previously missing executable/log artifacts have been retained and independently checked. However, the requested final integrity result is still outstanding; the current package does not yet pass the package-level manifest check. Do not claim current r4 package integrity or close the Major until a final manifest includes the intended candidate/docs/reports, relevant logs, CTest inventory and submanifest/binary copies, focused executable, and fresh Renode ELF/raw run; then run `sha256sum -c` successfully after all included review/index/result/audit bytes are frozen and obtain the required PA/R2/R4 delta confirmations.

## 3. Focused outcome

The artifacts materially correct the missing-test-executable portion of the previous finding. The updated README, verification-results, and loop-audit now accurately present manifest verification as pending and the R1/R3 status as complete only within its bounded review scope. Those corrections prevent the previous stale manifest from being represented as a current pass. However, the Major’s final requirement—verified integrity binding for the current complete package—is not yet met. **No overall positive evidence-integrity verdict is available at this snapshot.**

Facts above are directly checked against the current files; confidence is **High** for checksum/count outcomes. I did not independently validate every CTest result beyond the retained log, or rebuild/re-execute any gate. This report does not grant N10/T3 qualification, close NCs, authorize commit/release, or approve a phase.

| Rev | Date | Reason |
|---|---|---|
| 1.0 | 2026-11-13 | Focused delta review of the response to VV-R2-20261113-01; 55 retained CTest executables independently hash-checked; package-level manifest remains stale/unverified, so the Major remains open. |
