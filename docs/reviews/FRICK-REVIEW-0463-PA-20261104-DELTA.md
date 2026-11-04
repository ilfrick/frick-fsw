# FRICK-REVIEW-0463-PA-20261104-DELTA

**Independent PA Delta Review (norm N20) of frick-fsw commit 37aae4fff3d228046356c7728cb145b2f600b88d**

| Field | Value |
|-------|-------|
| Review ID | FRICK-REVIEW-0463-PA |
| Profile | PA / N20 (delta) |
| Session ID | (this review session) |
| Date | 2026-11-04 |
| Base commit (HEAD) | 37aae4fff3d228046356c7728cb145b2f600b88d |
| Base commit (delta baseline) | 41dcbe21435e14b5aaff06c63d9df6f0de04df54 |
| Manifest | `docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` |
| Manifest sha256 | c7b1bb7ccfb80f84ea7f106d806e4d696056d63ca003c5bafcdd4400d8f9b515 |
| Manifest entries (OK) | 194 |
| Scope | Delta since FRICK-REVIEW-0462 — exact bytes on both remotes |
| Out of scope | Engineering design review, safety case closure, tool qualification, release approval, NC closure, sprint closure |

---

## Independence Declaration

This review is performed by an independent PA agent with no engineering authorship of the committed changes, no managerial authority over the project, and no financial or schedule interest in the outcome. The reviewer has read access to the repository and evidence directories but no write/commit/push authority. Findings are reported to the human Software Product Assurance authority for disposition. No self-approval is performed or implied.

---

## Verdict

| Severity | Count |
|----------|-------|
| Blocker | 0 |
| Major | 1 |
| Minor | 2 |
| Observation | 3 |

**Overall: PASS WITH COMMENTS — one Major finding (excluded-file compliance), two Minor findings, three Observations. No Blocker. No NC closed, no sprint closed, no tool qualified, no release approval, no commit/push authorized by this review.**

---

## Finding Table

| ID | Severity | Criterion / Source | Observed Fact | Gap / Consequence | Affected Configuration | Recommended Action |
|----|----------|-------------------|---------------|-------------------|------------------------|---------------------|
| PA-0463-01 | Major | FRICK-REVIEW-0457 §6.9; FRICK-REVIEW-0459 residual 5; CANDIDATE.md claim | `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md` is tracked in HEAD (sha256 `9215342fe4cd2279802de72eed37fe19c859960deb85de11f482406413d3ba12`), identical to base commit. CANDIDATE.md states it is "excluded from both the manifest and the commit". The file IS in the commit. | Prior PA reviews required this file to be "reviewed or reverted before any commit that *includes* it". This commit includes it (it was already tracked). The requirement is violated: the file was neither reviewed nor reverted by this commit. | HEAD 37aae4f | Human PA authority must decide: (a) accept residual with documented rationale, (b) require review of the file's content modification, or (c) revert the file in a subsequent commit. |
| PA-0463-02 | Minor | Reconstruction claim (scope item 2) | Base commit + manifest does NOT reproduce the gate inputs at the base commit — the manifest records HEAD hashes. 24 of 28 managed-suffix files under `src/`, `tests/`, `tools/scripts/`, `tools/docker/` are NEW or MODIFIED relative to base commit; 4 existed in base but with different hashes. | The phrase "base commit + manifest reproduces every gate input" is true only if "reproduces" means "recovers the HEAD state of those files". The manifest is a snapshot of HEAD, not a delta from base. | HEAD 37aae4f | Clarify reconstruction semantics in CANDIDATE.md: the manifest binds the HEAD state of gate inputs; it does not imply the base commit contained those files. |
| PA-0463-03 | Minor | CANDIDATE.md accuracy | CANDIDATE.md §"Single classification of the evidence-mapping document" states the file is "excluded from ... the commit". This is factually false — the file is tracked in HEAD. | Misleading statement in the candidate description. | HEAD 37aae4f | Correct CANDIDATE.md in a follow-up commit or record an erratum. |
| PA-0463-04 | Observation | Scope item 4 — delivered vs claimed | No committed document claims gate evidence was re-run or produced *after* commit 37aae4f. Reviews 0460/0461/0462 describe independent re-execution *as part of the review*, not project re-run post-commit. FRICK-REVIEW-0462 §4.4 explicitly confirms "No document claims a gate was re-run after the freeze". | None. | HEAD 37aae4f | None. |
| PA-0463-05 | Observation | Scope item 5 — status consistency | T3 NOT FULLY QUALIFIED (tool-qualification.md rev 1.3, hash-bound); N18 FAIL (NC-EXT-198/NC-PAL-01); no NC closed; no sprint closed; hardware/human deferred — all stated correctly in sprint-plan.md rev 1.125, handover.md rev 3.64, CANDIDATE.md, verification-results.md. No contradictions found. | None. | HEAD 37aae4f | None. |
| PA-0463-06 | Observation | Scope item 6 — new defects in committed bytes | Substantive changes are test-harness fixes only: (1) frozen-UTC seam under `FF_TEST_HOOKS` in `pal.h`/`pal_posix.c` (absent from flight/RTEMS builds); (2) `test_link_uart.c` positive receive timeout raised from 20/500 ms to 2000 ms (test-timing fix, not framing logic); (3) `test_pus.c` HK tests wrap due-boundary checks with `FF_PalTimeTestFreeze/Unfreeze`. Four test files show formatting-only changes (clang-format). No production defect introduced. | None. | HEAD 37aae4f | None. |

---

## Per-Item Evidence

### 1. Manifest Integrity & Reconstruction (Scope item 1)
- **Command**: `sha256sum -c docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` (run in cloned repo at `/tmp/paa-0463`)
- **Exit code**: 0
- **OK count**: 194 (matches expected)
- **Manifest sha256**: `c7b1bb7ccfb80f84ea7f106d806e4d696056d63ca003c5bafcdd4400d8f9b515` (matches expected)
- **Self-entry check**: Manifest does NOT contain its own path (`docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256`)
- **manifest-verify.log check**: Manifest does NOT contain `manifest-verify.log`
- **Result**: **PASS** — manifest is internally consistent and correctly describes the HEAD tree.

### 2. Reconstruction Test (Scope item 2)
- **Base commit**: `41dcbe21435e14b5aaff06c63d9df6f0de04df54` (fetched from origin)
- **Managed-suffix files in manifest under `src/`, `tests/`, `tools/scripts/`, `tools/docker/`**: 28 entries
- **Comparison with base commit**:
  - 4 files exist in base with **different** hashes (modified): `src/core/pal/pal.h`, `src/core/pal/pal_posix.c`, `tests/unit/test_link_uart.c`, `tests/unit/test_pus.c`
  - 4 files exist in base with **same** hash (unchanged): `tests/unit/test_pus_n7_adapter.c`, `tests/unit/test_pusopen.c`, `tests/unit/test_r2_gap.c`, `tests/unit/test_sched_max_entries.c`
  - 20 files are **NEW** in HEAD (absent from base): `tests/unit/test_evs_hs_health.c`, `tests/unit/test_frozen_utc_seam.c`, `tools/scripts/fsw_opencode_event_bridge.py`, `tests/python/test_fsw_opencode_event_bridge.py`, `src/main/fsw_main_reqtest.c`, 12 Python test scripts, 6 shell scripts, 2 `.resc` files
- **Result**: **PASS WITH OBSERVATION** — the manifest correctly binds the HEAD state of all gate inputs. The "base commit + manifest" reconstruction works by overlaying the manifest files onto the base commit, not by verifying the base commit already contains them. This is a semantic clarification, not a defect.

### 3. Excluded File Compliance (Scope item 3)
- **File**: `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`
- **Status in HEAD**: Tracked, sha256 `9215342fe4cd2279802de72eed37fe19c859960deb85de11f482406413d3ba12`
- **Status in base commit**: Identical hash — file was NOT modified by this commit (37aae4f)
- **CANDIDATE.md claim**: "excluded from both the manifest and the commit"
- **Fact**: The file IS in the commit (HEAD tree). It is excluded from the manifest (correct), but the claim "excluded from the commit" is false.
- **Prior PA requirement**: FRICK-REVIEW-0457 §6.9 and FRICK-REVIEW-0459 residual 5: "reviewed or reverted before any commit that *includes* it"
- **Interpretation**: "Includes" in Git means the commit's tree contains the file. Since HEAD contains it, this commit includes it. The file was not reviewed or reverted by this commit.
- **Result**: **MAJOR FINDING (PA-0463-01)** — prior PA requirement violated. The file's content modification (28 insertions / 13 deletions per CANDIDATE.md) remains unreviewed and uncommitted in the working tree but the committed version is the base version. The requirement's trigger ("any commit that includes it") is met by this commit.

### 4. Delivered vs Claimed (Scope item 4)
- **Search**: All committed documents (reviews, CANDIDATE.md, evidence READMEs) for claims of gate re-run or evidence production *after* commit 37aae4f
- **Result**: No such claims found. FRICK-REVIEW-0460/0461 describe independent reviewer re-execution; FRICK-REVIEW-0462 §4.4 explicitly confirms no document claims post-freeze re-run.
- **Result**: **PASS** — no false post-commit claims.

### 5. Status Consistency (Scope item 5)
- **T3 NOT FULLY QUALIFIED**: Confirmed in `docs/vv/tool-qualification.md` rev 1.3, `sprint-plan.md` rev 1.125, `handover.md` rev 3.64, CANDIDATE.md
- **N18 FAIL**: Confirmed (NC-EXT-198 / NC-PAL-01) in all status documents
- **No NC closed**: Confirmed — all status documents state "No NC closed"
- **No sprint closed**: Confirmed — all status documents state "No sprint closed" / "genuine sprint closures = 0"
- **Hardware/human deferred**: Confirmed — NC-EXT-198 (emulated target fidelity), NC-EXT-199 (evidence integrity), NC-PAL-01 (hardware binding), NC-EXT-19 (target timing authority), NC-EXT-233 (human review deferral)
- **Result**: **PASS** — all status statements are consistent and accurate.

### 6. New Defects in Committed Bytes (Scope item 6)
- **Substantive changes** (non-formatting):
  1. `src/core/pal/pal.h` + `pal_posix.c`: Frozen-UTC test seam under `#ifdef FF_TEST_HOOKS` — absent from flight/RTEMS builds. Correctly guarded, documented as test-only.
  2. `tests/unit/test_link_uart.c`: Positive receive timeout bounds raised from 20 ms / 500 ms to 2000 ms. Comment explains CI load sensitivity; negative bounds (2 ms) unchanged. Test-harness fix only.
  3. `tests/unit/test_pus.c`: HK periodic emission tests wrap due-boundary checks with `FF_PalTimeTestFreeze(0)` / `FF_PalTimeTestUnfreeze()`. Makes UTC boundary deterministic for NC-EXT-235 correction (boundary requirement NC-EXT-207).
- **Formatting-only changes** (clang-format): `test_pus_n7_adapter.c`, `test_pusopen.c`, `test_r2_gap.c`, `test_sched_max_entries.c` — verified hunk-by-hunk in prior reviews.
- **New files**: 20 test scripts and fixtures (Python, shell, `.resc`) — gate inputs bound by manifest, not product code.
- **Result**: **PASS** — no production defect introduced. All changes are test-harness corrections or formatting.

---

## Explicit Statements (per review mandate)

- **No Nonconformance (NC) was closed by this commit or this review.**
- **No sprint was closed by this commit or this review.**
- **No tool was qualified by this commit or this review.** (T3 remains NOT FULLY QUALIFIED)
- **No release approval is granted or implied by this review.**
- **No commit or push is authorized by this review.** The human PA authority must disposition PA-0463-01 before any subsequent commit.

---

## Summary

The delta commit 37aae4f is internally consistent: the manifest verifies (194/194 OK), the committed tree matches the manifest, status claims are accurate, and the substantive changes are legitimate test-harness corrections for the NC-EXT-235 gate failures. One Major finding exists: the excluded evidence-mapping document is present in the commit (as it was in the base), violating the prior PA requirement that it be reviewed or reverted before any commit that includes it. The CANDIDATE.md misstates its exclusion from the commit. This requires human PA authority disposition.

**Recommendation**: Disposition PA-0463-01 (accept residual with rationale, require review, or revert). Correct CANDIDATE.md erratum. No other blockers to the current evidence baseline.

---

*End of FRICK-REVIEW-0463-PA-20261104-DELTA*