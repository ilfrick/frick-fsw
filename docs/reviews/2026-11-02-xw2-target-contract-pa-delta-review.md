# Independent SW PA Delta Review — XW-2 Target Contract Post-Correction Evidence

| Field | Value |
| --- | --- |
| Review ID | `PA-REVIEW-XW2-DELTA-20261102` |
| Date | 2026-11-02 |
| Reviewer profile | Senior Aeronautics and Space Software Product Assurance (PA) Engineer |
| Configured subagent | `aeronautics-space-sw-pa-engineer` |
| Harness session | `ses_e663d6753ffeUrSrkkLe6Fj7IK` (original PA review session; this delta review is a new task) |
| Model / route | `openrouter/nvidia/nemotron-3-ultra-550b-a55b:free` |
| Increment under review | Corrected XW-2 target contract (commit `ae278ef8734d12e69a20d41ba367c12c16155afe`) — delta since original PA review (`PA-REVIEW-XW2-20261102`) |
| Original verdict | **PASS WITH COMMENTS — conditional for commit** (13 findings: PA-01…PA-13, three Majors PA-01/PA-02/PA-03) |
| Review mode | Read-only; no build, test, formatter, or XW-2 runner executed |

> **Authority statement.** AI expert-persona review. It is not a human credential, not a certification, qualification, airworthiness finding, waiver or release approval, and it closes no non-conformance. Human approval remains mandatory for every decision gate.

---

## 1. Summary of the delta

The FSW engineer applied corrections to address all 13 findings from the original PA review. The corrections required editing `tools/scripts/qemu_xw2_vv.sh` (nm-segregation assertions, elapsed-time recording, `FF_USE_PUS_ASN1=OFF` parameterization, control shape check) and rebuilding the XW-2 image. This invalidated the pre-correction run `20261101T172050Z`. A fresh post-correction run was executed:

- **Run directory:** `build/xw2-run-20261102T011328Z` (archived at `docs/vv/evidence/2026-11-02-xw2-postfix/`)
- **Commit:** `ae278ef8734d12e69a20d41ba367c12c16155afe` (on both origin and gitea)
- **Result:** 13/13 required cases, SRS-REQ-073 PASS=12, SRS-REQ-075 PASS=1, WHOLE-RUN PASS, no-wrap control **EXPECTED FAILURE** (watchdog init panic + component verifier exit 1)
- **ELF positive:** `a9dc29d4ac147bcc0c07b9d307892797f7ae7d9a7b88e052312758ab0256c920`
- **ELF control:** `7f56b90e1a5716cee0c78b46ccadd26709f2edc6428734364eafb087da954a93`
- **Backend:** `docker`, toolchain `frick-rtems-toolchain:local` id `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896`
- **Config:** `FF_USE_PUS_ASN1=OFF`, QEMU timeout 25 s, elapsed 25,360,537,941 ns
- **nm-segregation:** asserted (3/0 and 0/3)
- **Run nonce:** `ae2cca72f04721807c0ef680df8b5111`
- **Archive hash verification:** 23/23 files OK (`sha256sum -c assets.sha256`)

The pre-correction runs (`20261101T170112Z`, `20261101T170759Z`, `20261101T172050Z`) are retained as **superseded history** in `docs/vv/evidence/2026-11-01-xw2-target-final/` (INDEX.md marked SUPERSEDED HISTORY). The authoritative evidence of record is the post-correction run.

---

## 2. Per-finding disposition

| Finding | Original severity | Disposition | Evidence of correction |
| --- | --- | --- | --- |
| **PA-01** Evidence pointers cited superseded run (`170112Z` / `65e4a91`) | **Major** | **CORRECTED** | All pointers in `verification-results.md`, `sprint-plan.md`, `tool-qualification.md` (T25 register), and the PA review itself now reference the post-correction run `20261102T011328Z`, commit `ae278ef`, and the archive `docs/vv/evidence/2026-11-02-xw2-postfix/`. The `INDEX.md` of the old archive is marked SUPERSEDED HISTORY. |
| **PA-02** Blocking gate consumes unqualified T25 with no deviation record | **Major** | **CORRECTED — deviation recorded, approval outstanding** | `docs/vv/tool-qualification.md` §82–100 now contains **DEV-T25-01** (bounded deviation). Fields: tool T25 NOT QUALIFIED; bounded use = exit-status gating only; compensating controls (1) fail-closed runner, (2) pre-existing fail-closed verifiers, (3) no-wrap control discrimination, (4) nm counts asserted, (5) no requirement claim from gate status. **Approval owner = Software PA authority** (not self-approved). Status = OPEN, approval outstanding. |
| **PA-03** Gitea `xw2-target` job runs `make pus-asn1` inside container without docker CLI | **Major** | **CORRECTED** | `.gitea/workflows/ci.yml` job `xw2-target` (lines 377–406) no longer runs `make pus-asn1`. Step "XW-2 target contract" sets `XW2_USE_PUS_ASN1=OFF` (contract-equivalent; XW-2 image contains no PUS codec). The job runs `make xw2` inside the pre-seeded `frick-rtems-toolchain:local` container. Commentary at lines 395–402 explains the rationale. |
| **PA-04** nm-segregation counts printed, not asserted; "proven" overstates observation | Minor | **CORRECTED** | `qemu_xw2_vv.sh` lines 166–185: `seg_count()` extracts the four keys; loop asserts exact values (`XW2_WRAP_CALL_SITES:3`, `XW2_REAL_CALL_SITES:0`, `FLIGHT_WRAP_DEFINED:0`, `FLIGHT_REAL_CALL_SITES:3`). On mismatch, exits 1 with diagnostic. `nm-segregation.exit` (0/1) recorded. Terminology: "asserted/observed" replaces "proven" in `verification-results.md` (line 105) and TS-7 §6.1 (line 167). Run directory `nm-segregation.txt` shows the four counts. |
| **PA-05** Declared gaps XW-G1..G5 absent from TS-7 | Minor | **CORRECTED** | `ts-7-xw2-target-contract.md` §5.1 (lines 134–148) adds table `XW-G1`…`XW-G5` with requirement, why it is a gap on this environment, and closure authority. Manifest `xw2_manifest.json` retains the five declared gaps; verifier rejects undeclared gaps only, consistent with TS-7. |
| **PA-06** Enforcement split between verifiers not stated in TS-7 | Minor | **CORRECTED** | TS-7 §6.1 (lines 161–172) adds "Enforcement split" table mapping each layer (`target_req_verify.py`, `target_run_verdict.py`, `qemu_xw2_vv.sh`, `app_xw2_test.c`) to what is checked and what is **not** checked. Clarifies that the console `detail` field is not re-verified on PASS. |
| **PA-07** XW-2 results lack document ID/revision/owner; SUPERSEDED statement unreconciled | Minor | **CORRECTED** | `verification-results.md` lines 46–54: XW-2 section now carries Document ID `FRICK-VV-XW2-20261102`, Revision 1.0, Date 2026-11-02, Author FSW engineer. The prior "marked SUPERSEDED (G6)" statement is removed; the section explicitly references the post-correction run and archive. |
| **PA-08** `checks=73/49` not backed by archived `LastTest.log`; format exit statuses not captured | Minor | **CORRECTED** | `verification-results.md` §117–124: XW-1 `LastTest.log` now archived in post-correction evidence. `format-apply.exit` and `format-check.exit` (both 0) captured in archive. The prior archive's unsupported exit-0 assertion is acknowledged. |
| **PA-09** Sprint plan scope misstated (XW-3), V&V review missing, row order not reordered | Minor | **CORRECTED** | `sprint-plan.md` Rev 1.116 (date 2026-11-01): Sprint S8-FU row (line 97) scope corrected to XW-1/XW-2; V&V review recorded; rows reordered (1.113/1.114/1.115/1.116 sequence consistent). |
| **PA-10** QEMU timeout hardcoded; elapsed not recorded | Observation | **CORRECTED** | `qemu_xw2_vv.sh` lines 20–21, 54–57, 95: `XW2_QEMU_TIMEOUT` parameterised (default 25 s), recorded in run directory `qemu-timeout.txt`. `positive-elapsed-ns.txt` records wall-clock elapsed (25,360,537,941 ns). |
| **PA-11** XW-T7 at-most-once enforced by test double, not software | Observation | **CARRIED** | Unchanged; explicitly disclosed in TS-7 §108 and `verification-results.md`. Authority item, no correction required. |
| **PA-12** Cadence/authoritative CI not stated; toolchain provenance open | Observation | **DOCUMENTED** | `verification-results.md` and `tool-qualification.md` T25 row state: Gitea per-commit `xw2-target` is authoritative; GitHub nightly only (weekly). Toolchain image provenance remains NC-EXT-35/40 (mutable base/apt/RSB). |
| **PA-13** Evidence archive and reviews untracked; extra dirs not gitignored | Minor | **CORRECTED** | `verification-results.md` line 112: post-correction archive at `docs/vv/evidence/2026-11-02-xw2-postfix/` with `README.md` and `assets.sha256`. Both the original PA review and this delta review are in `docs/reviews/`. Explicit path staging confirmed. |

---

## 3. New findings introduced by the corrections

| ID | Severity | Finding | Disposition required |
| --- | --- | --- | --- |
| **PA-D-01** | **Minor** | **DEV-T25-01 approval owner is "Software PA authority" but the deviation record does not name a specific human individual or role mapping.** The project norms (N9, N20) require independent PA review; the approval owner must be traceable to a named authority (e.g., the human PA lead, not an agent profile). The deviation cannot be closed by the FSW engineer or an AI persona. | Record the specific human authority (name/role) who will approve/reject DEV-T25-01. Update `tool-qualification.md` deviation table with the named approver. |
| **PA-D-02** | **Observation** | **The post-correction run `20261102T011328Z` was produced by the FSW engineer (author of the increment) and not by an independent execution.** The original PA review accepted the technical result on the basis of retained evidence, but independence of execution is not established. For a blocking gate (norm N20), independent re-execution or witnessing is expected before the deviation is approved. | Document whether an independent re-run (by a different agent/person, or witnessed by PA) will be performed as a condition of DEV-T25-01 approval. If not, record the rationale and residual risk. |
| **PA-D-03** | **Minor** | **The `verification-results.md` document now contains two major result sections (Renode verdict and XW-2) under a single document ID/revision.** The XW-2 section has its own Document ID (`FRICK-VV-XW2-20261102`), but the parent file does not. This creates a CM ambiguity: which document ID governs the file? The TS-7 and tool-qualification register each carry their own IDs, but `verification-results.md` is a composite. | Either (a) give `verification-results.md` a single governing Document ID with section-level IDs as sub-identifiers, or (b) split XW-2 results into a separate CM-identified file (e.g., `verification-results-xw2.md`) and cross-reference. Apply consistently to future multi-cycle result files. |
| **PA-D-04** | **Observation** | **The `qemu_xw2_vv.sh` script now contains both the positive and control build logic, the nm-segregation assertion, the QEMU run, the verifier calls, and the composite verdict — a single script doing all gate functions.** This concentrates risk: a defect in the script could affect all layers. The TS-7 §6.1 enforcement split table reflects the logical separation, but the physical script is monolithic. | Consider factoring the script into smaller, independently qualified units (build, segregation check, run, verify, verdict) per N10, or record the monolithic structure as an accepted deviation with compensating review. |

---

## 4. Resolution of original Majors

| Major | Resolved? | Comment |
| --- | --- | --- |
| **PA-01** (evidence pointers to superseded run) | **YES** | All pointers redirected to post-correction run `20261102T011328Z`, commit `ae278ef`, archive `docs/vv/evidence/2026-11-02-xw2-postfix/`. Superseded archive marked SUPERSEDED HISTORY. |
| **PA-02** (unqualified T25 used as blocking gate without deviation) | **YES — deviation recorded, approval outstanding** | DEV-T25-01 recorded with bounded use, compensating controls, and **Software PA authority** as approval owner. The deviation is **OPEN**; the gate may run in this bounded mode until the PA authority accepts or rejects it. The FSW engineer did not self-approve. |
| **PA-03** (Gitea job cannot execute due to `make pus-asn1` in container) | **YES** | Job `xw2-target` no longer runs `make pus-asn1`; uses `XW2_USE_PUS_ASN1=OFF` inside the pre-seeded toolchain container. Commentary documents the contract-equivalent rationale. |

**All three original Majors are resolved in the technical sense.** PA-02 remains administratively open pending human PA authority approval of the deviation.

---

## 5. Non-claims verification (unchanged)

The following non-claims remain explicitly stated and verified in the corrected artifacts:

- **NOT N18** — FAIL (NC-EXT-198, root cause NC-PAL-01)
- **NOT device / HIL / reset-scope / electrical-clock independence / timing / WCET / mission timeout**
- **8 s timeout remains ESC-7 placeholder**
- **Test double NOT reused to close NC-PAL-01**
- **T25 NOT QUALIFIED** — `DEV-T25-01` deviation approval outstanding (owner: Software PA authority)

These are documented in `verification-results.md`, `ts-7-xw2-target-contract.md`, `tool-qualification.md` (T25 row), and the PA review. No correction weakened these boundaries.

---

## 6. Verdict

**PASS WITH COMMENTS — conditional for commit**

### Conditions

1. **DEV-T25-01 approval:** The Software PA authority (named human) must approve or reject the bounded deviation before the XW-2 gate is relied upon for any downstream commitment. The gate may execute per-commit in its current bounded mode, but a positive gate status must not be cited as requirement satisfaction or tool qualification.
2. **PA-D-01:** Name the specific human approval authority for DEV-T25-01 in `tool-qualification.md`.
3. **PA-D-03:** Resolve the CM ambiguity in `verification-results.md` (single governing Document ID or split file) before the next cycle that adds result sections.
4. **Independent re-execution (PA-D-02):** Document whether an independent re-run/witness is required for DEV-T25-01 approval; if not, record rationale.

### Confidence

| Aspect | Confidence | Basis |
| --- | --- | --- |
| Technical result (13/13, control EXPECTED FAILURE) | **High** | Direct archive inspection: 23/23 hashes verified, ELF identities match, nm-segregation counts asserted, console/verifier/verdict JSON consistent. |
| PA-01/03/04/05/06/07/08/09/10/13 corrections | **High** | All referenced files inspected; corrections present and consistent across documents. |
| PA-02 deviation recording | **High** | DEV-T25-01 table complete with all required fields; approval owner identified as role (not self). |
| DEV-T25-01 approval authority specificity (PA-D-01) | **Medium** | Role stated but individual not named; traceability to human authority incomplete. |
| Independence of post-correction run execution (PA-D-02) | **Medium** | Run produced by increment author; independent re-execution not evidenced. |
| CM ambiguity in `verification-results.md` (PA-D-03) | **High** | File structure inspected; two document IDs in one file. |

### Missing/inaccessible evidence

- Gitea job execution log (not executed in this review; static inspection only)
- Branch-protection required-status configuration naming `xw2-target` (not in tree)
- Clean-build provenance for `frick-rtems-toolchain:local` (NC-EXT-35/40 open)
- Named human PA authority for DEV-T25-01 approval

### Assumptions and expiry/recheck date

- The post-correction run archive is byte-identical to the live run directory (verified by `sha256sum -c assets.sha256` = 23/23 OK).
- The commit `ae278ef` is the same on both remotes (stated; not independently verified against remote).
- The `FRICK-REVIEW-XW2-20261102` original PA review session (`ses_e663d6753ffeUrSrkkLe6Fj7IK`) is the basis for the 13 findings; no new findings from that session were added after the delta.
- Recheck required if `qemu_xw2_vv.sh`, `xw2_manifest.json`, `target_req_verify.py`, `target_run_verdict.py`, or the target test binary change (per DEV-T25-01 revisit trigger).

### Consequences if wrong

- If DEV-T25-01 is rejected by PA authority: the Gitea `xw2-target` job must be withdrawn from required-status checks until T25 is qualified.
- If the post-correction run evidence is not representative (e.g., environment drift): the technical PASS does not hold for subsequent commits.
- If CM ambiguity in `verification-results.md` persists: traceability gate (N4) may flag duplicate/ambiguous document IDs.

### Required independent human reviewer/authority

- **Software PA authority** (named human) for DEV-T25-01 approval and PA-D-01 resolution.
- **Configuration management authority** for `verification-results.md` CM structure (PA-D-03).
- **V&V independent reviewer** (R2 profile) for any independent re-execution witness (PA-D-02).

---

## 7. Evidence consulted (claim-to-source trace)

| Claim | Source |
| --- | --- |
| Post-correction run result (13/13, WHOLE-RUN PASS, control EXPECTED FAILURE) | `docs/vv/evidence/2026-11-02-xw2-postfix/README.md`, `verdict.json`, `console.log`, `control-result.txt` |
| ELF hashes (positive/control) | `docs/vv/evidence/2026-11-02-xw2-postfix/elf.sha256`, `control-elf.sha256` |
| nm-segregation counts asserted (3/0 and 0/3) | `docs/vv/evidence/2026-11-02-xw2-postfix/nm-segregation.txt`, `nm-segregation.exit` |
| Run nonce, elapsed, timeout, backend, toolchain ID | `docs/vv/evidence/2026-11-02-xw2-postfix/run-nonce.txt`, `positive-elapsed-ns.txt`, `qemu-timeout.txt`, `backend.txt`, `toolchain-id.txt` |
| Archive hash verification (23/23 OK) | `docs/vv/evidence/2026-11-02-xw2-postfix/assets.sha256` |
| DEV-T25-01 deviation record | `docs/vv/tool-qualification.md` lines 86–101 |
| Gitea `xw2-target` job corrected (no `make pus-asn1`, `XW2_USE_PUS_ASN1=OFF`) | `.gitea/workflows/ci.yml` lines 377–406 |
| XW-G1..G5 gap table in TS-7 | `docs/vv/test-spec/ts-7-xw2-target-contract.md` lines 134–148 |
| Enforcement split in TS-7 | `docs/vv/test-spec/ts-7-xw2-target-contract.md` lines 161–172 |
| XW-2 document ID/revision/owner in verification-results | `docs/vv/verification-results.md` lines 46–54 |
| Sprint plan scope/review/order corrections | `docs/planning/sprint-plan.md` Rev 1.116, lines 97–98 |
| Superseded archive marked SUPERSEDED HISTORY | `docs/vv/evidence/2026-11-01-xw2-target-final/INDEX.md` lines 11–22 |
| Original PA review findings and dispositions | `docs/reviews/2026-11-02-xw2-target-contract-pa-review.md` |

---

## 8. Archive

This delta review is archived at:

**`docs/reviews/2026-11-02-xw2-target-contract-pa-delta-review.md`**

---

**End of delta review.**